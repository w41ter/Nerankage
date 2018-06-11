#include <nerangake/generation_gc.h>

#include <assert.h>
#include <string.h> // memcpy

#include <algorithm>
#include <stdexcept> // exception

#include <nerangake/context.h>

namespace nrk {
namespace gc {

static size_t Align4K(size_t size) { return size & ~0xFFF; }

static uint32_t Align(uint32_t value) { return (value + 0x3) & ~0x3; }

static uint8_t *const Offset(uint8_t *const add, double factor, size_t length) {
    assert(factor < 1.0);
    uint32_t size = Align(length * factor);
    return add + size;
}

GenerationGC::GenerationGC(size_t size)
    : space_size_(Align4K(size))
    , start_(new uint8_t[space_size_])
    , survivor1_start_(Offset(start_, 0.2, space_size_))
    , survivor2_start_(Offset(start_, 0.3, space_size_))
    , old_start_(Offset(start_, 0.4, space_size_))
    , end_(start_ + space_size_) {
    from_ = survivor1_start_;
    to_ = survivor2_start_;
    to_free_ = to_;
    new_free_ = start_;
    old_free_ = old_start_;
}

GenerationGC::~GenerationGC() { delete[] start_; }

void GenerationGC::AllocationFail() {
    throw std::runtime_error("run out of memory.");
}

void GenerationGC::ProcessRootObjects(
    const RootObjectHolderInterface::Callback &cb) {
    for (auto it = Context::root_object_holder_begin(),
              end = Context::root_object_holder_end();
         it != end; ++it) {
        RootObjectHolderInterface *holder = *it;
        holder->ProcessRootObject(cb);
    }
}

void GenerationGC::WriteBarrier(
    HeapObject *obj, RawObject **field, HeapObject *new_obj) {
    if (reinterpret_cast<uint8_t *>(obj) >= old_start_ &&
        reinterpret_cast<uint8_t *>(new_obj) < old_start_ &&
        record_set_.count(obj) == 0) {
        record_set_.insert(obj);
    }
    *field = new_obj;
}

GenerationGC::HeapObject *GenerationGC::AllocateInNewSpace(size_t size) {
    if (new_free_ + size >= survivor1_start_) {
        MinorGC();
        if (new_free_ + size >= survivor1_start_) {
            // TODO: promote it into old space.
            AllocationFail();
        }
    }

    RawObject *raw = RawObject::From(new_free_);
    HeapObject *object = HeapObject::From(raw);
    object->set_age(0u);
    object->set_forwarded(false);
    object->set_size(static_cast<uint32_t>(size));

    new_free_ += size;

    return object;
}

void GenerationGC::MinorGC() {
    to_free_ = to_;

    auto cb = std::bind(
        &GenerationGC::CopyIntoAnotherSpace, this, std::placeholders::_1);

    ProcessRootObjects(cb);
    ProcessTransboundaryReference();

    std::swap(to_, from_);
}

GenerationGC::HeapObject *GenerationGC::CopyIntoAnotherSpace(HeapObject *obj) {
    // CopyIntoAnotherSpace is effective only for
    // objects within the Cenozoic region.
    if (reinterpret_cast<uint8_t *>(obj) >= old_start_) return obj;

    // TODO: run out of `to` memory.
    if (!obj->forwarded()) {
        const uint8_t age = obj->age();
        if (age < MAX_AGE) {
            obj->set_age(age + 1);
            memcpy(to_free_, obj, obj->size());
            obj->set_forwarded(true);
            obj->set_forwarding(reinterpret_cast<uintptr_t>(to_free_));

            to_free_ += obj->size();
            HeapObject::Children(
                obj,
                std::bind(
                    &GenerationGC::CopyIntoAnotherSpace, this,
                    std::placeholders::_1));
        } else {
            Promote(obj);
        }
    }
    return reinterpret_cast<HeapObject *>(obj->forwarding());
}

void GenerationGC::Promote(HeapObject *obj) {
    uint8_t *address = AllocateInOldSpace(obj->size());
    if (address == nullptr) {
        MajorGC();
        address = AllocateInOldSpace(obj->size());
        if (address == nullptr) AllocationFail();
    }

    RawObject *raw = RawObject::From(address);
    HeapObject *new_obj = HeapObject::From(raw);
    memcpy(new_obj, obj, obj->size());
    obj->set_forwarding(reinterpret_cast<uintptr_t>(new_obj));
    obj->set_forwarded(true);

    bool need_insert = false;
    HeapObject::Children(
        new_obj,
        [&need_insert, this, new_obj](HeapObject *child) -> HeapObject * {
            child = CopyIntoAnotherSpace(child);
            if (reinterpret_cast<uint8_t *>(child) < old_start_) {
                need_insert = true;
            }
            return child;
        });
    if (need_insert) record_set_.insert(new_obj);
}

void GenerationGC::ProcessTransboundaryReference() {
    for (auto it = record_set_.begin(); it != record_set_.end();) {
        HeapObject *obj = *it;
        bool result = false;
        HeapObject::Children(
            obj, [&result, this](HeapObject *child) -> HeapObject * {
                child = CopyIntoAnotherSpace(child);
                result =
                    result || old_start_ > reinterpret_cast<uint8_t *>(child);
                return child;
            });
        if (!result)
            it = record_set_.erase(it);
        else
            it++;
    }
}

uint8_t *GenerationGC::AllocateInOldSpace(size_t size) {
    if (old_free_ + size >= end_) { return nullptr; }
    uint8_t *result = old_free_;
    old_free_ += size;
    return result;
}

void GenerationGC::MajorGC() {
    ForwardingMap forwarding_map;

    // mark phase
    auto cb = std::bind(&GenerationGC::Mark, this, std::placeholders::_1);

    ProcessRootObjects(cb);
    RecordForwarding(forwarding_map);
    ResetReferences(forwarding_map);
    Compact(forwarding_map);
}

GenerationGC::HeapObject *GenerationGC::Mark(HeapObject *obj) {
    if (reinterpret_cast<uint8_t *>(obj) >= old_start_) {
        obj->set_forwarded(true);
    }
    HeapObject::Children(
        obj, std::bind(&GenerationGC::Mark, this, std::placeholders::_1));
    return obj;
}

void GenerationGC::RecordForwarding(ForwardingMap &map) {
    uint8_t *free = old_start_, *scan = old_start_;
    while (scan < end_) {
        HeapObject *obj = reinterpret_cast<HeapObject *>(scan);
        while (!obj->forwarded()) {
            scan += obj->size();
            obj = reinterpret_cast<HeapObject *>(scan);
        }

        HeapObject *new_obj = reinterpret_cast<HeapObject *>(free);
        map.insert(ForwardingMap::value_type(obj, new_obj));
        free += obj->size();
        scan += obj->size();
    }
}

void GenerationGC::ResetReferences(ForwardingMap &map) {
    RootObjectHolderInterface::Callback reseter =
        [this, &map, &reseter](HeapObject *obj) -> HeapObject * {
        if (map.count(obj)) { obj = map[obj]; }
        HeapObject::Children(obj, reseter);
        return obj;
    };
    ProcessRootObjects(reseter);
}

void GenerationGC::Compact(ForwardingMap &map) {
    uint8_t *scan = old_start_;
    while (scan < end_) {
        HeapObject *obj = reinterpret_cast<HeapObject *>(scan);
        while (!obj->forwarded()) {
            scan += obj->size();
            obj = reinterpret_cast<HeapObject *>(scan);
        }

        assert(map.count(obj));

        HeapObject *target = map[obj];
        memmove(target, obj, obj->size());
        scan += obj->size();
    }
}

void GenerationGC::FullGC() {
    MajorGC();
    MinorGC();
}

} // namespace gc
} // namespace nrk
