#include "mips.h"
#include <cstdio>

uint32_t MIPS32::R(
    int rs,
    int rt,
    int rd,
    int shamt,
    int funct)
{
    return
        ((uint32_t)rs << 21) |
        ((uint32_t)rt << 16) |
        ((uint32_t)rd << 11) |
        ((uint32_t)shamt << 6) |
        funct;
}

uint32_t MIPS32::I(
    int opcode,
    int rs,
    int rt,
    int16_t imm)
{
    return
        ((uint32_t)opcode << 26) |
        ((uint32_t)rs << 21) |
        ((uint32_t)rt << 16) |
        (uint16_t)imm;
}

void MIPS32::emit(uint32_t instruction)
{
    code.push_back(instruction);
}

void MIPS32::nop()
{
    emit(0);
}

void MIPS32::addu(int rd, int rs, int rt)
{
    emit(R(rs, rt, rd, 0, 0x21));
}

void MIPS32::subu(int rd, int rs, int rt)
{
    emit(R(rs, rt, rd, 0, 0x23));
}

void MIPS32::and_(int rd, int rs, int rt)
{
    emit(R(rs, rt, rd, 0, 0x24));
}

void MIPS32::or_(int rd, int rs, int rt)
{
    emit(R(rs, rt, rd, 0, 0x25));
}

void MIPS32::xor_(int rd, int rs, int rt)
{
    emit(R(rs, rt, rd, 0, 0x26));
}

void MIPS32::addiu(
    int rt,
    int rs,
    int16_t imm)
{
    emit(I(0x09, rs, rt, imm));
}

void MIPS32::andi(
    int rt,
    int rs,
    uint16_t imm)
{
    emit(I(0x0C, rs, rt, imm));
}

void MIPS32::ori(
    int rt,
    int rs,
    uint16_t imm)
{
    emit(I(0x0D, rs, rt, imm));
}

void MIPS32::xori(
    int rt,
    int rs,
    uint16_t imm)
{
    emit(I(0x0E, rs, rt, imm));
}

void MIPS32::lw(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x23, base, rt, offset));
}

void MIPS32::sw(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x2B, base, rt, offset));
}

void MIPS32::jr(int rs)
{
    emit(R(rs, 0, 0, 0, 0x08));
}

void MIPS32::dump() const
{
    for (size_t i = 0; i < code.size(); ++i) {

        printf(
            "%04zu: %08X\n",
            i,
            code[i]
        );
    }
}