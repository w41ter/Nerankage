#pragma once

#include <assert.h>
#include <stdint.h>

#include <type_traits>

namespace nrk {
namespace object {

#define IMPLICIT_CONSTRUCTORS(Class)          \
    Class() = default;                        \
    ~Class() = default;                       \
    Class(const Class&) = default;            \
    Class& operator=(const Class&) = default; \
    Class(Class&&) = default;                 \
    Class& operator=(Class&&) = default

/**
 * RawObject is the base class of the internal data objects of virtual machines,
 * which are used to describe data objects inside a virtual machine.
 *
 * The data object design uses the Tagging approach. In the common architecture,
 * because memory aligned, such as 4 aligned, the last two bits of the data
 * object address must be 0, so we can add type information to the last two
 * bits.
 *
 * There are 4 basic types of virtual machines: Fixnum, Nil, Boolean, and
 * HeapObject.
 * Among them, Boolean is divided into two values, True and False. HeapObject
 * this means that the pointer points to the specific data structure, otherwise
 * the pointer itself is data.
 *
 * In this version of the virtual machine, the default two bits are the most
 * Tagging bits. 4 different data types can be represented. Here, 00 is used to
 * represent HeapObject, so you do not need to convert when using the object.
 * Use 01 to represent Fixnum, and the other two type values are fixed, that is
 * to say, other bits can record the data. Here, Special is used to represent
 * the remaining two types.
 */
class RawObject {
public:
    enum Tag {
        kObject = 0x00,
        kFixnum = 0x01,  // 0000_0001
        kSpecial = 0x03, // 0000_0011
    };

    enum Special {
        kNil = 0x03,   // 0000_0011
        kTrue = 0x06,  // 0000_0111
        kFalse = 0x0B, // 0000_1011
    };

    enum {
        kTagMask = 0x03,     // 0000_0011
        kSpecialMask = 0x0F, // 0000_1111
        kTagShift = 2,
        kSpecialShift = 4,
    };

    IMPLICIT_CONSTRUCTORS(RawObject);

    static RawObject* Not(const RawObject*);
    static RawObject* Add(const RawObject*, const RawObject*);
    static RawObject* Sub(const RawObject*, const RawObject*);
    static RawObject* Mul(const RawObject*, const RawObject*);
    static RawObject* Div(const RawObject*, const RawObject*);
    static RawObject* Mod(const RawObject*, const RawObject*);
    static RawObject* Pow(const RawObject*, const RawObject*);
    static RawObject* GT(const RawObject*, const RawObject*);
    static RawObject* GE(const RawObject*, const RawObject*);
    static RawObject* LT(const RawObject*, const RawObject*);
    static RawObject* LE(const RawObject*, const RawObject*);
    static RawObject* EQ(const RawObject*, const RawObject*);
    static RawObject* NE(const RawObject*, const RawObject*);
    static RawObject* Index(RawObject*, const RawObject*);
    static void SetIndex(RawObject*, const RawObject*, RawObject*);
    static bool True(const RawObject*);
    static bool NZ(const RawObject*);

    uintptr_t This() const { return reinterpret_cast<uintptr_t>(this); }

    bool IsNil() const { return tag() == RawObject::kNil; }

    bool IsFixnum() const { return tag() == RawObject::kFixnum; }

    bool IsBoolean() const {
        return This() == RawObject::kTrue || This() == RawObject::kFalse;
    }

    bool IsObject() const { return tag() == RawObject::kObject; }

    uintptr_t tag() const { return This() & RawObject::kTagMask; }

    /**
     * Converts values and other types of pointers to RawObject pointers.
     *
     * @param value     value needs to be converted
     *
     * @return  RawObject pointer
     */
    static RawObject* From(uintptr_t value) {
        return reinterpret_cast<RawObject*>(value);
    }

    /**
     * Converts values and other types of pointers to RawObject pointers.
     *
     * @param value     value needs to be converted
     *
     * @return  RawObject pointer
     */
    static RawObject* From(uint8_t* value) {
        return From(reinterpret_cast<uintptr_t>(value));
    }

    /**
     * Convert RawObject to its subclass, so the user needs to ensure that
     * the target type is a subclass of the current type (using IsXXX() to
     * judge).
     *
     * @return  target type reference.
     */
    template <
        typename Type,
        typename = typename std::enable_if<
            std::is_base_of<RawObject, Type>::value, Type>::type>
    const Type* As() const {
        return reinterpret_cast<const Type*>(this);
    }

    /**
     * Convert RawObject to its subclass, so the user needs to ensure that
     * the target type is a subclass of the current type (using IsXXX() to
     * judge).
     *
     * @return  target type reference.
     */
    template <
        typename Type,
        typename = typename std::enable_if<
            std::is_base_of<RawObject, Type>::value, Type>::type>
    Type* As() {
        const RawObject* thiz = this;
        return const_cast<Type*>(thiz->As<Type>());
    }
};

/**
 * Fixnum is the built-in numeric type of the virtual machine that
 * can express ranges [-2^29 - 1= -536870913, 2^29 = 536870912] values.
 */
class Fixnum : public RawObject {
public:
    IMPLICIT_CONSTRUCTORS(Fixnum);

    static Fixnum* Create(int32_t value) {
        assert(
            -536870913 <= value && 536870912 >= value &&
            "integer value out of range");

        int32_t data = (value << RawObject::kTagShift) & kFixnum;
        RawObject* object = RawObject::From(static_cast<uintptr_t>(data));
        return object->As<Fixnum>();
    }

    int32_t value() const { return This() >> kTagShift; }
};

class Boolean : public RawObject {
public:
    IMPLICIT_CONSTRUCTORS(Boolean);

    static Boolean* Create(bool value) {
        uintptr_t data = static_cast<uintptr_t>(value ? kTrue : kFalse);
        RawObject* object = RawObject::From(data);
        return object->As<Boolean>();
    }

    bool value() const { return This() == RawObject::kTrue; }
};

class Nil : public RawObject {
public:
    IMPLICIT_CONSTRUCTORS(Nil);

    static Nil* Create() {
        return RawObject::From(static_cast<uintptr_t>(kNil))->As<Nil>();
    }
};

static_assert(
    std::is_trivially_copyable<RawObject>::value,
    "class `RawObject` must be trivially copyable type.");
static_assert(
    std::is_trivially_copyable<Fixnum>::value,
    "class `Fixnum` must be trivially copyable type.");
static_assert(
    std::is_trivially_copyable<Boolean>::value,
    "class `Boolean` must be trivially copyable type.");
static_assert(
    std::is_trivially_copyable<Nil>::value,
    "class `Nil` must be trivially copyable type.");

} // namespace object
} // namespace nrk
