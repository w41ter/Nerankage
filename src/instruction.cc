#include <nerangake/instruction.h>

#include <assert.h>

namespace nrk {

uint8_t LEN_OF_INSTRUCTION = 4;

OPCode Instruction::OP(const uint8_t *code) {
    assert(code && "nullptr exception");

    return static_cast<OPCode>(*code);
}

uint8_t Instruction::A(const uint8_t *code) {
    assert(code && "nullptr exception");

    return *(code + 1);
}

uint8_t Instruction::B(const uint8_t *code) {
    assert(code && "nullptr exception");

    return *(code + 2);
}

uint8_t Instruction::C(const uint8_t *code) {
    assert(code && "nullptr exception");

    return *(code + 3);
}

uint16_t Instruction::Bx(const uint8_t *code) {
    assert(code && "nullptr exception");

    return LittleEndianToLocal(*reinterpret_cast<const uint16_t *>(code));
}

uint32_t Instruction::Ax(const uint8_t *code) {
    assert(code && "nullptr exception");

    uint8_t a = A(code);
    uint8_t b = B(code);
    uint8_t c = C(code);
    uint32_t little_endian_data = (c << 16) | (b << 8) | a;
    return LittleEndianToLocal(little_endian_data);
}

void Instruction::OPABC(
    uint8_t *code, OPCode op, uint8_t a, uint8_t b, uint8_t c) {
    assert(code && "nullptr exception");

    *(code) = static_cast<uint8_t>(op);
    *(code + 1) = a;
    *(code + 2) = b;
    *(code + 3) = c;
}

void Instruction::OPABx(uint8_t *code, OPCode op, uint8_t a, uint16_t bx) {
    assert(code && "nullptr exception");

    *(code) = static_cast<uint8_t>(op);
    *(code + 1) = a;
    *reinterpret_cast<uint16_t *>(code + 2) = ToLittleEndian(bx);
}

void Instruction::OPAx(uint8_t *code, OPCode op, uint32_t ax) {
    assert(code && "nullptr exception");

    uint32_t little_endian_data = ToLittleEndian(ax);
    uint8_t a = (little_endian_data >> 0) & 0xFF;
    uint8_t b = (little_endian_data >> 8) & 0xFF;
    uint8_t c = (little_endian_data >> 8) & 0xFF;
    return OPABC(code, op, a, b, c);
}

const uint8_t *Instruction::Next(const uint8_t *pc, int32_t offset) {
    return pc + offset * LEN_OF_INSTRUCTION;
}

union EndianTest {
    struct {
        int8_t a;
        int8_t b;
        int8_t c;
        int8_t d;
    };
    int32_t value;
};

constexpr EndianTest __TEST__ = {{(int32_t)1}};
constexpr bool PLATFORM_IS_LITTLE_ENDIAN = __TEST__.a;

template <bool val>
struct EndianHelper {
    static uint16_t ToLittle(uint16_t value) { return Swap(value); }

    static uint32_t ToLittle(uint32_t value) { return Swap(value); }

    static uint16_t LittleToLocal(uint16_t value) { return value; }

    static uint32_t LittleToLocal(uint32_t value) { return value; }

private:
    static uint16_t Swap(uint16_t value) {
        uint8_t *p = (uint8_t *)&value;
        p[0] = p[0] ^ p[1];
        p[1] = p[0] ^ p[1];
        p[0] = p[0] ^ p[1];
        return value;
    }

    static uint32_t Swap(uint32_t value) {
        uint8_t *p = (uint8_t *)&value;
        p[0] = p[0] ^ p[3];
        p[3] = p[0] ^ p[3];
        p[0] = p[0] ^ p[3];
        p[1] = p[1] ^ p[2];
        p[2] = p[1] ^ p[2];
        p[1] = p[1] ^ p[2];
        return value;
    }
};

template <>
struct EndianHelper<true> {
    static uint16_t ToLittle(uint16_t value) { return value; }

    static uint32_t ToLittle(uint32_t value) { return value; }

    static uint16_t LittleToLocal(uint16_t value) { return value; }

    static uint32_t LittleToLocal(uint32_t value) { return value; }
};

typedef EndianHelper<PLATFORM_IS_LITTLE_ENDIAN> Endian;

uint16_t Instruction::LittleEndianToLocal(uint16_t value) {
    return Endian::LittleToLocal(value);
}

uint32_t Instruction::LittleEndianToLocal(uint32_t value) {
    return Endian::LittleToLocal(value);
}

uint16_t Instruction::ToLittleEndian(uint16_t value) {
    return Endian::ToLittle(value);
}

uint32_t Instruction::ToLittleEndian(uint32_t value) {
    return Endian::ToLittle(value);
}

} // namespace nrk