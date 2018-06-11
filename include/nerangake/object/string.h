#pragma once

#include <assert.h> // assert
#include <string.h> // memmove

#include <nerangake/object/heap_object.h>

namespace nrk {
namespace object {

class String : public HeapObject {
public:
    enum StringLayout {
        kLength = kFieldStart,
        kBuffer = kLength + 4,
    };

    IMPLICIT_CONSTRUCTORS(String);

    static size_t Size(size_t length) {
        uint32_t size = sizeof(uint32_t) + Align(static_cast<uint32_t>(length));
        return static_cast<size_t>(size);
    }

    static String *Create(const char *str, size_t length);
    static String *CreateGlobal(const char *str, size_t length);

    char At(unsigned idx) const;

    uint32_t length() const;

    const char *buffer() const;

private:
    static void Init(String *string, const char *str, size_t length);

    char *buffer();
    void set_length(uint32_t length);
};

static_assert(
    std::is_trivially_copyable<String>::value,
    "class `String` must be trivially copyable type.");

} // namespace object
} // namespace nrk
