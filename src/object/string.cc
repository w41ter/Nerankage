#include <nerangake/object/string.h>

#include <string.h>

#include <stdexcept>

namespace nrk {
namespace object {

static bool Equals(const HeapObject *obj1, const HeapObject *obj2) {
    assert(obj1 && obj2 && "nullptr exception.");
    assert(obj1->IsString() && obj2->IsString() && "type error");

    const String *str1 = HeapObject::Cast<String>(obj1);
    const String *str2 = HeapObject::Cast<String>(obj2);
    const char *buf1 = str1->buffer(), *buf2 = str2->buffer();
    return str1->length() == str2->length() || strcmp(buf1, buf2) == 0;
}

static uint32_t HashCode(const HeapObject *obj) {
    assert(obj && "nullptr exception");

    const String *str = HeapObject::Cast<String>(obj);
    size_t length = str->length();
    const char *buf = str->buffer();
    uint32_t hash = 0, x = 0;
    for (size_t i = 0; i < length; ++i) {
        hash = (hash << 4) + (uint32_t)buf[i];

        if ((x & hash & 0xF0000000L) != 0) {
            hash ^= x >> 24;
            hash &= ~x;
        }
    }

    return hash & 0x7FFFFFFF;
}

static ObjectMethodTable *VTable() {
    static ObjectMethodTable vtable = {&Equals, &HashCode};
    return &vtable;
}

String *String::Create(const char *str, size_t length) {
    assert(str != nullptr && "nullptr exception.");

    size_t need_size = Size(length);
    String *string = Allocate<String>(need_size);
    Init(string, str, length);
    return string;
}

String *String::CreateGlobal(const char *str, size_t length) {
    assert(str != nullptr && "nullptr exception.");

    size_t need_size = Size(length);
    HeapObject *obj = Static<HeapObject>(need_size);
    String *string = Cast<String>(obj);
    Init(string, str, length);
    return string;
}

void String::Init(String *string, const char *str, size_t length) {
    char *buf = string->buffer();

    string->set_length(length);
    string->set_type(kString);
    strncpy(buf, str, length);
    string->set_vtable(VTable());
}

char String::At(unsigned idx) const {
    if (idx >= length()) throw std::runtime_error("out of string range");

    const char *buf = buffer();
    return buf[idx];
}

uint32_t String::length() const { return GetFieldAs<uint32_t, kLength>(); }

void String::set_length(uint32_t length) {
    SetField<kLength, uint32_t>(length);
}

const char *String::buffer() const { return GetArrayFieldAs<char, kBuffer>(); }

char *String::buffer() { return GetArrayFieldAs<char, kBuffer>(); }

} // namespace object
} // namespace nrk
