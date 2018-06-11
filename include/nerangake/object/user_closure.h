#pragma once

#include <nerangake/object/heap_object.h>

namespace nrk {

class VMScene;

namespace object {

// UserDefFunc - Scene, retval[beg, end), parameters
typedef void (*UserDefFunc)(VMScene *, uint8_t, uint8_t, uint8_t);

class UserClosure : public HeapObject {
public:
    typedef UserDefFunc Func;

    enum UserClosureLayout {
        kPointer = kFieldStart,
    };

    IMPLICIT_CONSTRUCTORS(UserClosure);

    static UserClosure *Create(const Func func) {
        size_t size = sizeof(uintptr_t);
        HeapObject *obj = Static<HeapObject>(size);
        UserClosure *closure = Cast<UserClosure>(obj);
        closure->set_callable(func);
        closure->set_type(kUserClosure);
        return closure;
    }

    const Func callable() const { return GetFieldAs<const Func, kPointer>(); }

private:
    void set_callable(const Func fun) { SetField<kPointer>(fun); }
};

static_assert(
    std::is_trivially_copyable<UserClosure>::value,
    "class `UserClosure` must be trivially copyable type.");

} // namespace object
} // namespace nrk
