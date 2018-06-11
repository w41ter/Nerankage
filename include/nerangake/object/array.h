#pragma once

#include <assert.h> // assert

#include <nerangake/object/heap_object.h>

namespace nrk {
namespace object {

class Array : public HeapObject {
public:
    enum ArrayLayout {
        kLength = kFieldStart,
        kBuffer = kLength + 4,
    };

    IMPLICIT_CONSTRUCTORS(Array);

    static Array *Create(size_t length);

    uint32_t length() const;

    void Set(size_t idx, Element obj);
    Element Get(size_t idx);
    const Element Get(size_t idx) const;

    void Children(const ForwardingCallback &cb);

private:
    static void Init(Array *array, size_t length);

    Element *buffer();
    const Element *buffer() const;

    void set_length(uint32_t size);

    static size_t Size(size_t length) {
        uint32_t size = sizeof(uint32_t) +
            Align(static_cast<uint32_t>(length * sizeof(uintptr_t)));
        return static_cast<size_t>(size);
    }
};

static_assert(
    std::is_trivially_copyable<Array>::value,
    "class `Array` must be trivially copyable type.");

} // namespace object
} // namespace nrk
