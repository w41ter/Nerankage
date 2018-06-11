#include <nerangake/object/call_info.h>

#include <nerangake/instruction.h>

namespace nrk {
namespace object {

static const int NUM_OF_REGISTERS = 32;

static void ProcessChildren(HeapObject *obj, const ForwardingCallback &cb) {
    assert(obj && "nullptr exception.");

    CallInfo *ci = HeapObject::Cast<CallInfo>(obj);
    ci->Children(cb);
}

static const ObjectMethodTable *VTable() {
    static ObjectMethodTable table = {nullptr, nullptr, &ProcessChildren};
    return &table;
}

size_t CallInfo::Size() {
    return sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uintptr_t) +
        sizeof(uintptr_t) + sizeof(uintptr_t) * 32;
}

CallInfo *CallInfo::Create(
    const Closure *closure, uint8_t begin, uint8_t end, uint8_t num_of_params) {
    size_t size = Size();
    CallInfo *ci = Static<CallInfo>(size);

    const Prototype *proto = closure->callee();
    ci->set_type(kCallInfo);
    ci->set_is_light_func(false);
    ci->set_callee(closure);
    ci->set_vtable(VTable());
    ci->set_saved_pc(proto->code());
    ci->set_begin(begin);
    ci->set_end(end);
    ci->set_num_of_params(num_of_params);
    for (uint8_t i = 0; i < NUM_OF_REGISTERS; ++i) {
        ci->set_reg(i, Nil::Create());
    }
    for (uint8_t i = 0; i < closure->num_of_captureds(); ++i) {
        ci->set_captured(i, closure->captured(i));
    }
    return ci;
}

CallInfo *CallInfo::Create(
    const UserClosure *closure, uint8_t begin, uint8_t end,
    uint8_t num_of_params) {
    size_t size = Size();
    CallInfo *ci = Static<CallInfo>(size);

    ci->set_type(kCallInfo);
    ci->set_is_light_func(true);
    ci->set_callee(closure);
    ci->set_vtable(VTable());
    ci->set_begin(begin);
    ci->set_end(end);
    ci->set_num_of_params(num_of_params);
    return ci;
}

void CallInfo::SetNextPC(int32_t offset) {
    set_saved_pc(Instruction::Next(saved_pc(), offset));
}

void CallInfo::Reset() {
    assert(is_light_func() && "only supported for closure");

    const Prototype *proto = callee()->callee();
    set_saved_pc(proto->code());
}

const uint8_t *CallInfo::saved_pc() {
    return GetFieldAs<const uint8_t *, kSavedPC>();
}

void CallInfo::set_saved_pc(const uint8_t *pc) { SetField<kSavedPC>(pc); }

CallInfo::Element CallInfo::reg(uint8_t idx) {
    Element *elements = GetArrayFieldAs<Element, kRegister>();
    return elements[idx];
}

void CallInfo::set_reg(uint8_t idx, Element e) {
    SetArrayField<kRegister>(idx, e);
}

bool CallInfo::is_light_func() const {
    return GetFieldAs<uint8_t, kIsLightFunc>();
}

void CallInfo::set_is_light_func(bool t) { SetField<kIsLightFunc, uint8_t>(t); }

const UserClosure *CallInfo::user_callee() const {
    return GetFieldAs<UserClosure *, kCallee>();
}

const Closure *CallInfo::callee() const {
    return GetFieldAs<Closure *, kCallee>();
}

void CallInfo::set_callee(const UserClosure *c) { SetField<kCallee>(c); }

void CallInfo::set_callee(const Closure *c) { SetField<kCallee>(c); }

CallInfo::Element CallInfo::captured(uint8_t idx) {
    assert(is_light_func() && "only support closure");

    const Closure *closure = callee();
    assert(idx < closure->num_of_captureds() && "out of range");

    Element *elements = GetArrayFieldAs<Element, kCaptured>();
    return elements[idx];
}

void CallInfo::set_captured(uint8_t idx, Element e) {
    assert(is_light_func() && "only support closure");

    const Closure *closure = callee();
    assert(idx < closure->num_of_captureds() && "out of range");

    SetArrayField<kCaptured>(idx, e);
}

CallInfo *CallInfo::parent() { return GetFieldAs<CallInfo *, kParent>(); }

void CallInfo::set_parent(CallInfo *ci) { SetField<kParent>(ci); }

void CallInfo::set_begin(uint8_t beg) { SetField<kBegin>(beg); }

void CallInfo::set_end(uint8_t end) { SetField<kEnd>(end); }

void CallInfo::set_num_of_params(uint8_t num_of_params) {
    SetField<kNumOfParams>(num_of_params);
}

uint8_t CallInfo::begin() const { return GetFieldAs<uint8_t, kBegin>(); }

uint8_t CallInfo::end() const { return GetFieldAs<uint8_t, kEnd>(); }

uint8_t CallInfo::num_of_params() const {
    return GetFieldAs<uint8_t, kNumOfParams>();
}

void CallInfo::Children(const ForwardingCallback &cb) {
    if (is_light_func()) {
        const UserClosure *closure = user_callee();
        closure = ForwardingObject<UserClosure>(cb, closure);
        set_callee(closure);
    } else {
        const Closure *closure = callee();
        closure = ForwardingObject<Closure>(cb, closure);
        set_callee(closure);
    }
}

} // namespace object
} // namespace nrk
