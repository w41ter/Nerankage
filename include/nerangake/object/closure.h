#pragma once

#include <nerangake/object/prototype.h>

namespace nrk {
namespace object {

/**
 * Object's layout:
 * - num_of_captureds
 * - callee
 * - captureds
 */
class Closure : public HeapObject {
public:
    enum ClosureLayout {
        kNumOfCaptures = kFieldStart,
        kReserved = kNumOfCaptures + sizeof(uint16_t),
        kCallee = kNumOfCaptures + sizeof(uint16_t),
        kCaptureds = kCallee + sizeof(uintptr_t)
    };

    IMPLICIT_CONSTRUCTORS(Closure);

    static size_t Size(uint16_t num_of_captureds);
    static Closure *Create(const Prototype *type, uint16_t num_of_captureds);
    static Closure *CreateGlobal(
        const Prototype *type, uint16_t num_of_captureds);

    const Prototype *callee() const;
    uint16_t num_of_captureds() const;

    Element captured(unsigned idx);
    const Element captured(unsigned idx) const;
    void set_captured(unsigned idx, Element e);

    void Children(const ForwardingCallback &cb);

private:
    static void Init(
        Closure *closure, const Prototype *type, uint16_t num_of_captured);

    void set_num_of_captureds(uint16_t);
    void set_callee(const Prototype *type);
};

static_assert(
    std::is_trivially_copyable<Closure>::value,
    "Class `Closure` must be trivially copyable type.");

} // namespace object
} // namespace nrk
