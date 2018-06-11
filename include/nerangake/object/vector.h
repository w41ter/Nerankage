#pragma once

#include <nerangake/object/array.h>

namespace nrk {
namespace object {

/**
 * Object's layout
 * - capacity (uint32_t)
 * - length (uint32_t)
 * - Array array
 **/
class Vector : public HeapObject {
    typedef RawObject *Element;

public:
    enum VectorLayout {
        kCapacity = kFieldStart,
        kLength = kCapacity + sizeof(uint32_t),
        kArray = kLength + sizeof(uint32_t)
    };

    IMPLICIT_CONSTRUCTORS(Vector);

    static size_t Size();
    static Vector *Create(size_t size = 16);

    uint32_t length() const;
    uint32_t capacity() const;

    bool Empty() const;
    Element Get(unsigned idx);
    const Element Get(unsigned idx) const;
    void Set(unsigned idx, Element e);
    void Push(Element e);
    Element Pop();

    void Children(const ForwardingCallback &cb);

private:
    void Extend();

    void set_length(uint32_t);
    void set_capacity(uint32_t);
    void set_buffer(Array *array);
    const Array *buffer() const;
    Array *buffer();
};

static_assert(
    std::is_trivially_copyable<Vector>::value,
    "class `Vector` must be trivially copyable type.");

} // namespace object
} // namespace nrk
