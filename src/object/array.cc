#include <nerangake/object/array.h>

#include <stdexcept>

namespace nrk {
namespace object {

static void ProcessChildren(HeapObject *obj, const ForwardingCallback &cb) {
    assert(obj && "nullptr exception.");

    Array *array = HeapObject::Cast<Array>(obj);
    array->Children(cb);
}

static const ObjectMethodTable *VTable() {
    static ObjectMethodTable table = {nullptr, nullptr, &ProcessChildren};
    return &table;
}

Array *Array::Create(size_t size) {
    size_t need_size = Size(size);
    Array *array = Allocate<Array>(need_size);
    Init(array, size);
    return array;
}

void Array::Init(Array *array, size_t size) {
    Element *buf = array->buffer();
    array->set_length(size);
    array->set_type(kArray);
    array->set_vtable(VTable());
    for (size_t i = 0; i < size; ++i) buf[i] = Nil::Create();
}

Array::Element *Array::buffer() { return GetArrayFieldAs<Element, kBuffer>(); }

const Array::Element *Array::buffer() const {
    return GetArrayFieldAs<Element, kBuffer>();
}

uint32_t Array::length() const { return GetFieldAs<uint32_t, kLength>(); }

Array::Element Array::Get(size_t idx) {
    const Array *thiz = this;
    return const_cast<Element>(thiz->Get(idx));
}

const Array::Element Array::Get(size_t idx) const {
    if (idx >= length()) throw std::runtime_error("out of array range");

    const Element *buf = buffer();
    return buf[idx];
}

void Array::Set(size_t idx, Element obj) {
    if (idx >= length()) throw std::runtime_error("out of array range");

    Element *buf = buffer();
    buf[idx] = obj;
}

void Array::set_length(uint32_t size) { SetField<kLength, uint32_t>(size); }

void Array::Children(const ForwardingCallback &cb) {
    Element *buf = buffer();
    uint32_t len = length();
    for (uint32_t i = 0; i < len; ++i) {
        if (!buf[i]->IsObject()) continue;
        HeapObject *obj = HeapObject::From(buf[i]);
        buf[i] = ForwardingObject<HeapObject>(cb, obj);
    }
}

} // namespace object
} // namespace nrk
