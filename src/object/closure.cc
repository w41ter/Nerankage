#include <nerangake/object/closure.h>

#include <assert.h>

namespace nrk {
namespace object {

static void Children(HeapObject *obj, const ForwardingCallback &cb) {
    assert(obj && "nullptr exception");

    Closure *closure = HeapObject::Cast<Closure>(obj);
    closure->Children(cb);
}

static const ObjectMethodTable *VTable() {
    static ObjectMethodTable table = {nullptr, nullptr, &Children};
    return &table;
}

size_t Closure::Size(uint16_t num_of_captured) {
    return sizeof(uintptr_t) * (num_of_captured + 1);
}

Closure *Closure::Create(const Prototype *type, uint16_t num_of_captureds) {
    assert(type && "nullptr exception");

    size_t size = Size(num_of_captureds);
    Closure *closure = Allocate<Closure>(size);
    Init(closure, type, num_of_captureds);
    return closure;
}

Closure *Closure::CreateGlobal(
    const Prototype *type, uint16_t num_of_captureds) {
    assert(type && "nullptr exception");

    size_t size = Size(num_of_captureds);
    Closure *closure = Static<Closure>(size);
    Init(closure, type, num_of_captureds);
    return closure;
}

void Closure::Init(
    Closure *closure, const Prototype *type, uint16_t num_of_captured) {
    closure->set_num_of_captureds(num_of_captured);
    closure->set_callee(type);
    closure->set_type(kClosure);
    closure->set_vtable(VTable());
}

const Prototype *Closure::callee() const {
    return GetFieldAs<Prototype *, kCallee>();
}

uint16_t Closure::num_of_captureds() const {
    return GetFieldAs<uint16_t, kNumOfCaptures>();
}

void Closure::set_num_of_captureds(uint16_t num_of_captureds) {
    SetField<kNumOfCaptures>(num_of_captureds);
}

void Closure::set_callee(const Prototype *type) { SetField<kCallee>(type); }

Closure::Element Closure::captured(unsigned idx) {
    Element *elements = GetArrayFieldAs<Element, kCaptureds>();
    return elements[idx];
}

const Closure::Element Closure::captured(unsigned idx) const {
    const Element *elements = GetArrayFieldAs<Element, kCaptureds>();
    return elements[idx];
}

void Closure::set_captured(unsigned idx, Element e) {
    SetArrayField<kCaptureds>(idx, e);
}

void Closure::Children(const ForwardingCallback &cb) {
    const Prototype *proto = callee();
    proto = ForwardingObject<Prototype>(cb, proto);
    set_callee(proto);

    uint16_t num_of_captureds = this->num_of_captureds();
    for (uint16_t i = 0; i < num_of_captureds; ++i) {
        RawObject *raw = captured(i);
        if (!raw->IsObject()) continue;
        HeapObject *obj = HeapObject::From(raw);
        obj = ForwardingObject<HeapObject>(cb, obj);
        set_captured(i, obj);
    }
}

} // namespace object
} // namespace nrk
