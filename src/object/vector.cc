#include <nerangake/object/vector.h>

#include <assert.h>

#include <stdexcept>

#include <nerangake/context.h>

namespace nrk {
namespace object {

static void Children(HeapObject *obj, const ForwardingCallback &cb) {
    assert(obj && "nullptr exception");

    Vector *vector = HeapObject::Cast<Vector>(obj);
    vector->Children(cb);
}

static const ObjectMethodTable *VTable() {
    static ObjectMethodTable table = {nullptr, nullptr, &Children};
    return &table;
}

size_t Vector::Size() {
    return sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uintptr_t);
}

Vector *Vector::Create(size_t size) {
    HeapObject *obj = Allocate<HeapObject>(Size());

    GCInterface *gc = Context::gc();
    gc->Push(&obj);
    Array *array = Array::Create(size);
    gc->Pop();

    Vector *vector = Cast<Vector>(obj);
    vector->set_type(kVector);
    vector->set_length(0);
    vector->set_capacity(size);
    vector->set_buffer(array);
    vector->set_vtable(VTable());

    return vector;
}

uint32_t Vector::length() const { return GetFieldAs<uint32_t, kLength>(); }

uint32_t Vector::capacity() const { return GetFieldAs<uint32_t, kCapacity>(); }

void Vector::set_length(uint32_t val) { SetField<kLength, uint32_t>(val); }

void Vector::set_capacity(uint32_t cap) { SetField<kCapacity>(cap); }

void Vector::set_buffer(Array *array) { SetField<kArray>(array); }

const Array *Vector::buffer() const { return GetFieldAs<Array *, kArray>(); }

Array *Vector::buffer() {
    const Vector *thiz = this;
    return const_cast<Array *>(thiz->buffer());
}

bool Vector::Empty() const { return length() == 0; }

Vector::Element Vector::Get(unsigned idx) {
    const Vector *thiz = this;
    return const_cast<Element>(thiz->Get(idx));
}

const Vector::Element Vector::Get(unsigned idx) const {
    if (idx >= length()) { throw std::runtime_error("index out of range."); }
    return buffer()->Get(idx);
}

void Vector::Set(unsigned idx, Element e) {
    if (idx >= length()) { throw std::runtime_error("index out of range."); }
    return buffer()->Set(idx, e);
}

void Vector::Push(Element e) {
    uint32_t len = length();
    if (capacity() == len) { Extend(); }

    buffer()->Set(len, e);
    set_length(len + 1);
}

Vector::Element Vector::Pop() {
    uint32_t len = length() - 1;
    set_length(len);
    return buffer()->Get(len);
}

void Vector::Extend() {
    Array *array = Array::Create(capacity() * 2);
    Array *buf = buffer();
    uint32_t len = length();
    for (uint32_t i = 0; i < len; ++i) { array->Set(i, buf->Get(i)); }
    set_buffer(array);
}

void Vector::Children(const ForwardingCallback &cb) {
    Array *array = const_cast<Array *>(buffer());
    array = ForwardingObject<Array>(cb, array);
    set_buffer(array);
}

} // namespace object
} // namespace nrk
