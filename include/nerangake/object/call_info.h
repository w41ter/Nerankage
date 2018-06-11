#pragma once

#include <nerangake/object/closure.h>
#include <nerangake/object/user_closure.h>

namespace nrk {
namespace object {

/**
 * CallInfo's layout
 * - is_light_func (uint8_t)
 * - begin (uint8_t)
 * - end (uint8_t)
 * - params (uint8_t)
 * - saved_pc (uintptr_t)
 * - callee (Closure)
 * - parent (CallInfo)
 * - Register (RawObject[16])
 * - captured (RawObject[])
 **/
class CallInfo : public HeapObject {
public:
    enum CallInfoLayout {
        kIsLightFunc = kFieldStart,
        kBegin = kIsLightFunc + sizeof(uint8_t),
        kEnd = kBegin + sizeof(uint8_t),
        kNumOfParams = kEnd + sizeof(uint8_t),
        kSavedPC = kNumOfParams + sizeof(uint8_t),
        kCallee = kSavedPC + sizeof(uintptr_t),
        kParent = kCallee + sizeof(uintptr_t),
        kRegister = kCallee + sizeof(uintptr_t),
        kCaptured = kRegister + sizeof(uintptr_t),
    };

    IMPLICIT_CONSTRUCTORS(CallInfo);

    static size_t Size();
    static CallInfo *Create(
        const Closure *closure, uint8_t begin, uint8_t end,
        uint8_t num_of_params);
    static CallInfo *Create(
        const UserClosure *closure, uint8_t begin, uint8_t end,
        uint8_t num_of_params);

    void SetNextPC(int32_t offset);
    void Reset();

    const uint8_t *saved_pc();
    void set_saved_pc(const uint8_t *pc);

    Element reg(uint8_t idx);
    void set_reg(uint8_t idx, Element e);
    Element captured(uint8_t idx);
    void set_captured(uint8_t idx, Element e);

    bool is_light_func() const;
    const UserClosure *user_callee() const;
    const Closure *callee() const;

    CallInfo *parent();
    void set_parent(CallInfo *);

    uint8_t begin() const;
    uint8_t end() const;
    uint8_t num_of_params() const;

    void Children(const ForwardingCallback &cb);

private:
    void set_is_light_func(bool);
    void set_callee(const UserClosure *);
    void set_callee(const Closure *);
    void set_begin(uint8_t);
    void set_end(uint8_t);
    void set_num_of_params(uint8_t);
};

static_assert(
    std::is_trivially_copyable<CallInfo>::value, 
	"class `CallInfo` must be trivially copyable type.");

} // namespace object
} // namespace nrk
