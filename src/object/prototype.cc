#include <nerangake/object/prototype.h>

#include <assert.h>

namespace nrk {
namespace object {

size_t Prototype::Size(uint16_t num_of_captured) {
    return sizeof(int32_t) + sizeof(uint32_t) + sizeof(uintptr_t) +
        sizeof(Captured) * num_of_captured;
}

Prototype *Prototype::Create(
    const uint8_t *code, uint32_t size_of_code, bool is_vararg,
    uint8_t num_of_params, std::vector<Captured> &captureds) {
    assert(code && "nullptr exception");

    size_t size = Size(captureds.size());
    Prototype *proto = Static<Prototype>(size);
    proto->set_type(kPrototype);
    proto->set_code(code);
    proto->set_size_of_code(size_of_code);
    proto->set_num_of_params(num_of_params);
    proto->set_is_vararg(is_vararg);
    proto->set_num_of_captured(captureds.size());

    for (size_t i = 0; i < captureds.size(); ++i) {
        proto->set_captured(i, captureds[i]);
    }

    return proto;
}

const uint8_t *Prototype::code() const {
    return GetFieldAs<const uint8_t *, kCode>();
}

uint32_t Prototype::size_of_code() const {
    return GetFieldAs<uint32_t, kSizeOfCode>();
}

uint8_t Prototype::num_of_params() const {
    return GetFieldAs<uint8_t, kNumOfParams>();
}

uint16_t Prototype::num_of_captureds() const {
    return GetFieldAs<uint16_t, kNumOfCaptureds>();
}

bool Prototype::is_vararg() const { return GetFieldAs<uint8_t, kIsVarArg>(); }

const Captured &Prototype::captured(unsigned idx) const {
    assert(idx < num_of_captureds() && "out of range");

    const Captured *captureds = GetArrayFieldAs<Captured, kCaptureds>();
    return captureds[idx];
}

void Prototype::set_code(const uint8_t *code) { SetField<kCode>(code); }

void Prototype::set_size_of_code(uint32_t codesize) {
    SetField<kSizeOfCode>(codesize);
}

void Prototype::set_num_of_params(uint8_t num_of_params) {
    SetField<kNumOfParams>(num_of_params);
}

void Prototype::set_is_vararg(bool is_vararg) {
    SetField<kIsVarArg, uint8_t>(is_vararg);
}

void Prototype::set_num_of_captured(uint16_t num_of_captured) {
    SetField<kNumOfCaptureds>(num_of_captured);
}

void Prototype::set_captured(unsigned idx, const Captured &cap) {
    SetArrayField<kCaptureds>(idx, cap);
}

} // namespace object
} // namespace nrk
