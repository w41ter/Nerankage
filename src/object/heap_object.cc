#include <nerangake/object/heap_object.h>

#include <nerangake/context.h>

namespace nrk {
namespace object {

static const uint32_t HASH_SEED = 0xdeadbeef;

static uint32_t HashValue(const RawObject *key) {
    // hash key to uint32_t value one-to-one
    return (static_cast<uint32_t>(key->This()) ^ HASH_SEED);
}

static bool DefaultEquals(const HeapObject *o1, const HeapObject *o2) {
    return false;
}

static uint32_t DefaultHashCode(const HeapObject *key) {
    return HashValue(key);
}

static ObjectMethodTable *DefaultObjectMethodTable() {
    static ObjectMethodTable table = {
        &DefaultEquals,
        &DefaultHashCode,
        nullptr,
    };
    return &table;
}

// FIXME: vtable bind.
HeapObject *HeapObject::AllocateDef(size_t size) {
    using memory::AllocateType;
    using memory::AllocatorInterface;

    size_t target_size = header_size() + size;
    AllocatorInterface *allocator = Context::allocator();
    void *whitespace = allocator->Allocate(AllocateType::Heap, target_size);
    HeapObject *object = reinterpret_cast<HeapObject *>(whitespace);
    object->set_vtable(DefaultObjectMethodTable());
    return object;
}

HeapObject *HeapObject::StaticDef(size_t size) {
    using memory::AllocateType;
    using memory::AllocatorInterface;

    size_t target_size = header_size() + size;
    AllocatorInterface *allocator = Context::allocator();
    void *whitespace = allocator->Allocate(AllocateType::Static, target_size);
    HeapObject *object = reinterpret_cast<HeapObject *>(whitespace);
    object->set_vtable(DefaultObjectMethodTable());
    return object;
}

void HeapObject::SetFieldInternal(RawObject **field, HeapObject *obj) {
    using gc::GCInterface;

    GCInterface *gc = Context::gc();
    gc->WriteBarrier(this, field, obj);
}

bool HeapObject::Equals(const RawObject *key1, const RawObject *key2) {
    if (key1 == key2) return true;

    if (key1->IsObject() && key2->IsObject()) {
        // must be heap object.
        const HeapObject *o1 = HeapObject::From(key1);
        const HeapObject *o2 = HeapObject::From(key2);
        if (o1->type() != o2->type()) return false;

        const ObjectMethodTable *table = o1->vtable();
        if (table->equals == NULL) {
            DefaultObjectMethodTable()->equals(o1, o2);
        } else {
            return table->equals(o1, o2);
        }
    }
    return false;
}

uint32_t HeapObject::HashCode(const RawObject *key) {
    if (key->IsObject()) {
        const HeapObject *object = HeapObject::From(key);

        const ObjectMethodTable *table = object->vtable();
        if (table->hash_code == NULL) {
            return DefaultObjectMethodTable()->hash_code(object);
        } else {
            return table->hash_code(object);
        }
    } else {
        return HashValue(key);
    }
}

void HeapObject::Children(HeapObject *obj, const ForwardingCallback &cb) {
    const ObjectMethodTable *vtable = obj->vtable();
    // some HeapObjects haven't any child, so `process_children` will be
    // nullptr.
    if (vtable->process_children == nullptr)
        DefaultObjectMethodTable()->process_children(obj, cb);
}

} // namespace object
} // namespace nrk
