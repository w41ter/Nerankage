#pragma once

#include <stdint.h>

#include <nerangake/opcode.h>

namespace nrk {

class Instruction {
public:
    static OPCode OP(const uint8_t *code);
    static uint8_t A(const uint8_t *code);
    static uint8_t B(const uint8_t *code);
    static uint8_t C(const uint8_t *code);
    static uint16_t Bx(const uint8_t *code);
    static uint32_t Ax(const uint8_t *code);

    static void OPABC(
        uint8_t *code, OPCode op, uint8_t a, uint8_t b, uint8_t c);
    static void OPABx(uint8_t *code, OPCode op, uint8_t a, uint16_t bx);
    static void OPAx(uint8_t *code, OPCode op, uint32_t ax);

    static const uint8_t *Next(const uint8_t *pc, int32_t offset);

private:
    static uint16_t LittleEndianToLocal(uint16_t value);
    static uint32_t LittleEndianToLocal(uint32_t value);

    static uint16_t ToLittleEndian(uint16_t value);
    static uint32_t ToLittleEndian(uint32_t value);
};

} // namespace nrk
