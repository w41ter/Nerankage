#pragma once

#include <assert.h>

#include <functional>

#include <nerangake/object/raw_object.h>

namespace nrk {
namespace object {

class HeapObject;

typedef std::function<HeapObject *(HeapObject *)> ForwardingCallback;

struct ObjectMethodTable {
    bool (*equals)(const HeapObject *, const HeapObject *);
    uint32_t (*hash_code)(const HeapObject *);
    void (*process_children)(HeapObject *, const ForwardingCallback &);
};

/**
 * For GC:
 * +-----+-----------+-------------+------------+--------+
 * | age | forwarded | object size | forwarding | others |
 * +-----+-----------+-------------+------------+--------+
 * | 7b  | 1b        | 24b         | 4 or 8 b   |        |
 * +-----+-----------+-------------+------------+--------+
 * For Object:
 * +-----+-----------+-------------+------------+--------+
 * | age | forwarded | object size | meta-info  | others |
 * +-----+-----------+-------------+------------+--------+
 *
 * MetaInfo:
 * +------+---------------+
 * | type |   reserved    |
 * +------+---------------+
 */
class HeapObject : public RawObject {
public:
    typedef RawObject *Element;

    enum ObjectLayout {
        kAge = 0,
        kForwarded = 0,
        kObjectSize = 1,
        kForwarding = 4,
        kMetaInfo = 4,
    };

    enum MetaInfoLayout {
        kType = kMetaInfo,
        kVTable = kType + sizeof(uint32_t),
        kFieldStart = kVTable + sizeof(uintptr_t)
    };

    enum Type {
        kArray,
        kCallInfo,
        kClosure,
        kFloat,
        kHashMap,
        kHashNode,
        kPrototype,
        kStack,
        kString,
        kUserClosure,
        kVector
    };

    static HeapObject *From(RawObject *obj) { return obj->As<HeapObject>(); }

    static const HeapObject *From(const RawObject *obj) {
        return obj->As<HeapObject>();
    }

    template <
        typename Class,
        typename = typename std::enable_if<
            std::is_base_of<HeapObject, Class>::value, Class>::type>
    static Class *Cast(HeapObject *obj) {
        return reinterpret_cast<Class *>(obj);
    }

    template <
        typename Class,
        typename = typename std::enable_if<
            std::is_base_of<HeapObject, Class>::value, Class>::type>
    static const Class *Cast(const HeapObject *obj) {
        return reinterpret_cast<const Class *>(obj);
    }

    // static constexpr uint32_t Align(uint32_t value) {
    //    return (value + 0x3) & ~0x3;
    //}

    static uint32_t HashCode(const RawObject *obj);
    static bool Equals(const RawObject *key1, const RawObject *key2);
    static void Children(HeapObject *obj, const ForwardingCallback &cb);

    IMPLICIT_CONSTRUCTORS(HeapObject);

    // FIXME: gc meta-info access control.
    void set_age(uint8_t age) {
        assert(age < 128 && "age out of range[0, 128)");
        uint8_t &field = At<uint8_t, kAge>();
        field &= 0b0001;
        field |= age << 1;
    }

    uint8_t age() const { return At<uint8_t, kAge>() >> 1; }

    void set_forwarded(bool status) {
        if (status)
            At<uint8_t, kForwarded>() |= 0b0001;
        else
            At<uint8_t, kForwarded>() &= ~0b0001;
    }

    bool forwarded() { return At<uint8_t, kForwarded>() & 0b0001; }

    uint32_t size() {
        uint8_t high = At<uint8_t, kObjectSize>();
        uint16_t low = At<uint16_t, kObjectSize + 1>();
        return ((uint32_t)high << 16) | low;
    }

    void set_size(uint32_t size) {
        assert(size < (1 << 24) && "size out of range[0, 2^24)");
        uint16_t low = -1;
        uint8_t high = -1;
        low &= size;
        size >>= 16;
        high &= size;
        At<uint8_t, kObjectSize>() = high;
        At<uint16_t, kObjectSize + 1>() = low;
    }

    uintptr_t forwarding() { return At<uintptr_t, kForwarding>(); }

    void set_forwarding(uintptr_t ptr) { At<uintptr_t, kForwarding>() = ptr; }

    const ObjectMethodTable *vtable() const {
        return At<ObjectMethodTable *, kVTable>();
    }

    void set_vtable(const ObjectMethodTable *tb) {
        At<const ObjectMethodTable *, kVTable>() = tb;
    }

    uint8_t type() const { return At<uint8_t, kType>(); }

#define IS_CHILD(name) \
    bool Is##name() const { return At<uint8_t, kType>() == k##name; }

#define CHILDREN_LIST(V) \
    V(Array)             \
    V(CallInfo)          \
    V(Closure)           \
    V(Float)             \
    V(HashMap)           \
    V(Prototype)         \
    V(Stack)             \
    V(String)            \
    V(UserClosure)       \
    V(Vector)            \
    V(HashNode)

    // is_xxx
    CHILDREN_LIST(IS_CHILD)

#undef CHILDREN_LIST
#undef IS_CHILD

protected:
    static size_t header_size() { return kFieldStart; }

    /**
     * allocate - allocate memroy from allocator.
     *
     * @param size  the size child object need(without gc's and meta info.
     */
    template <
        typename Type,
        typename = typename std::enable_if<
            std::is_base_of<HeapObject, Type>::value, Type>::type>
    static Type *Allocate(size_t size) {
        HeapObject *obj = AllocateDef(size);
        return Cast<Type>(obj);
    }

    template <
        typename Type,
        typename = typename std::enable_if<
            std::is_base_of<HeapObject, Type>::value, Type>::type>
    static Type *Static(size_t size) {
        HeapObject *obj = StaticDef(size);
        return Cast<Type>(obj);
    }

    // Proxy
    template <typename Class, unsigned offset>
    Class GetFieldAs() {
        return At<Class, offset>();
    }

    template <typename Class, unsigned offset>
    Class GetFieldAs() const {
        return At<Class, offset>();
    }

    template <typename Class, unsigned offset>
    Class *GetArrayFieldAs() {
        return Index<Class, offset>();
    }

    template <typename Class, unsigned offset>
    const Class *GetArrayFieldAs() const {
        return Index<Class, offset>();
    }

    template <unsigned offset, typename Class>
    void SetField(Class val) {
        At<Class, offset>() = val;
    }

    template <unsigned offset>
    void SetField(RawObject *obj) {
        if (obj->IsObject())
            // Write barrier are used.
            SetFieldInteranl(
                Index<RawObject *, offset>(), HeapObject::From(obj));
        else
            At<RawObject *, offset>() = obj;
    }

    template <unsigned offset, typename Class>
    void SetArrayField(unsigned idx, Class val) {
        Index<Class, offset>()[idx] = val;
    }

    template <unsigned offset>
    void SetArrayField(unsigned idx, RawObject *obj) {
        if (obj->IsObject())
			// Write barrier are used.
            SetFieldInteranl(
                &Index<RawObject *, offset>()[idx], HeapObject::From(obj));
        else
            Index<RawObject *, offset>()[idx] = obj;
    }

    template <
        typename Type,
        typename = typename std::enable_if<
            std::is_base_of<HeapObject, Type>::value, Type>::type>
    Type *ForwardingObject(const ForwardingCallback &cb, HeapObject *obj) {
        return HeapObject::Cast<Type>(cb(obj));
    }

    template <
        typename Type,
        typename = typename std::enable_if<
            std::is_base_of<HeapObject, Type>::value, Type>::type>
    const Type *ForwardingObject(
        const ForwardingCallback &cb, const HeapObject *obj) {
        HeapObject *nobj = const_cast<HeapObject *>(obj);
        return ForwardingObject<Type>(cb, nobj);
    }

    void set_type(uint8_t type) { SetField<kType>(type); }

private:
    HeapObject *AllocateDef(size_t size);
    HeapObject *StaticDef(size_t size);

    void SetFieldInternal(RawObject **field, HeapObject *obj);

    /**
     * At like `this->filed`.
     */
    template <typename T, unsigned offset>
    auto At() const -> const typename std::remove_reference<T>::type & {
        typedef typename std::remove_reference<T>::type Type;
        return *reinterpret_cast<Type *>(This() + offset);
    }

    template <typename T, unsigned offset>
    auto At() -> typename std::remove_reference<T>::type & {
        typedef typename std::remove_reference<T>::type Type;
        const HeapObject *thiz = this;
        return const_cast<Type &>(thiz->At<T, offset>());
    }

    template <typename T>
    auto At(uintptr_t offset) const -> const
        typename std::remove_reference<T>::type & {
        typedef typename std::remove_reference<T>::type Type;
        return *reinterpret_cast<Type *>(This() + offset);
    }

    template <typename T>
    auto At(uintptr_t offset) -> typename std::remove_reference<T>::type & {
        typedef typename std::remove_reference<T>::type Type;
        const HeapObject *thiz = this;
        return const_cast<Type &>(thiz->At<T>(offset));
    }

    /**
     * Index like `&this->filed`.
     */
    template <typename T, unsigned offset>
    auto Index() const -> const typename std::remove_reference<T>::type * {
        typedef typename std::remove_reference<T>::type Type;
        return reinterpret_cast<Type *>(This() + offset);
    }

    template <typename T, unsigned offset>
    auto Index() -> typename std::remove_reference<T>::type * {
        typedef typename std::remove_reference<T>::type Type;
        const HeapObject *thiz = this;
        return const_cast<Type *>(thiz->Index<T, offset>());
    }

    template <typename T>
    auto Index(unsigned offset) const -> const
        typename std::remove_reference<T>::type * {
        typedef typename std::remove_reference<T>::type Type;
        return reinterpret_cast<Type *>(This() + offset);
    }

    template <typename T>
    auto Index(unsigned offset) -> typename std::remove_reference<T>::type * {
        typedef typename std::remove_reference<T>::type Type;
        HeapObject *thiz = this;
        return const_cast<Type *>(thiz->Index<T>(offset));
    }
};

static_assert(
    std::is_trivially_copyable<HeapObject>::value,
    "class `HeapObject` must be trivially copyable type.");

} // namespace object
} // namespace nrk
