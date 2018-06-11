#pragma once

#include <vector>

#include <nerangake/object/heap_object.h>

namespace nrk {
namespace object {

struct Captured {
    int8_t instack;
    int8_t index;
};

/**
 * Prototype's layout
 * - is_vararg (int8_t)
 * - num_of_params (int8_t)
 * - num_of_captured (int16_t)
 * - size_of_code (uint32_t)
 * - code (uintptr_t)
 * - captureds (Captured[num_of_captured])
 **/
class Prototype : public HeapObject {
public:
    enum PrototypeLayout {
        kIsVarArg = kFieldStart,
        kNumOfParams = kIsVarArg + sizeof(int8_t),
        kNumOfCaptureds = kNumOfParams + sizeof(int8_t),
        kSizeOfCode = kNumOfCaptureds + sizeof(int16_t),
        kCode = kSizeOfCode + sizeof(uint32_t),
        kCaptureds = kCode + sizeof(uintptr_t)
    };

    IMPLICIT_CONSTRUCTORS(Prototype);

    static size_t Size(uint16_t);
    static Prototype *Create(
        const uint8_t *code, uint32_t size_of_code, bool is_vararg,
        uint8_t num_of_params, std::vector<Captured> &captureds);

    const uint8_t *code() const;
    uint32_t size_of_code() const;
    uint8_t num_of_params() const;
    uint16_t num_of_captureds() const;
    bool is_vararg() const;

    const Captured &captured(unsigned idx) const;

private:
    void set_code(const uint8_t *code);
    void set_size_of_code(uint32_t codesize);
    void set_num_of_params(uint8_t num_of_params);
    void set_is_vararg(bool is_vararg);
    void set_num_of_captured(uint16_t num_of_captured);
    void set_captured(unsigned idx, const Captured &);
};

static_assert(
    std::is_trivially_copyable<Prototype>::value,
    "class `Prototype` must be trivially copyable type.");

} // namespace object
} // namespace nrk
