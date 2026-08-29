#include "mips.h"

#include <cstdio>


// ============================================================
// Encoding helpers
// ============================================================

uint32_t MIPS32::R(
    int rs,
    int rt,
    int rd,
    int shamt,
    int funct)
{
    return
        ((uint32_t)(rs & 0x1F) << 21) |
        ((uint32_t)(rt & 0x1F) << 16) |
        ((uint32_t)(rd & 0x1F) << 11) |
        ((uint32_t)(shamt & 0x1F) << 6) |
        (uint32_t)(funct & 0x3F);
}


uint32_t MIPS32::I(
    int opcode,
    int rs,
    int rt,
    int16_t imm)
{
    return
        ((uint32_t)(opcode & 0x3F) << 26) |
        ((uint32_t)(rs & 0x1F) << 21) |
        ((uint32_t)(rt & 0x1F) << 16) |
        (uint16_t)imm;
}


uint32_t MIPS32::IUnsigned(
    int opcode,
    int rs,
    int rt,
    uint16_t imm)
{
    return
        ((uint32_t)(opcode & 0x3F) << 26) |
        ((uint32_t)(rs & 0x1F) << 21) |
        ((uint32_t)(rt & 0x1F) << 16) |
        imm;
}


// ============================================================
// Emit
// ============================================================

void MIPS32::emit(
    uint32_t instruction)
{
    code.push_back(instruction);
}


void MIPS32::nop()
{
    emit(0x00000000);
}


// ============================================================
// Arithmetic
// ============================================================

void MIPS32::addu(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x21));
}


void MIPS32::subu(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x23));
}


// ============================================================
// Logical
// ============================================================

void MIPS32::and_(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x24));
}


void MIPS32::or_(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x25));
}


void MIPS32::xor_(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x26));
}


void MIPS32::nor(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x27));
}


// ============================================================
// Shifts
// ============================================================

void MIPS32::sll(
    int rd,
    int rt,
    int shamt)
{
    emit(R(0, rt, rd, shamt, 0x00));
}


void MIPS32::srl(
    int rd,
    int rt,
    int shamt)
{
    emit(R(0, rt, rd, shamt, 0x02));
}


void MIPS32::sra(
    int rd,
    int rt,
    int shamt)
{
    emit(R(0, rt, rd, shamt, 0x03));
}


void MIPS32::sllv(
    int rd,
    int rt,
    int rs)
{
    emit(R(rs, rt, rd, 0, 0x04));
}


void MIPS32::srlv(
    int rd,
    int rt,
    int rs)
{
    emit(R(rs, rt, rd, 0, 0x06));
}


void MIPS32::srav(
    int rd,
    int rt,
    int rs)
{
    emit(R(rs, rt, rd, 0, 0x07));
}


// ============================================================
// Compare
// ============================================================

void MIPS32::slt(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x2A));
}


void MIPS32::sltu(
    int rd,
    int rs,
    int rt)
{
    emit(R(rs, rt, rd, 0, 0x2B));
}


// ============================================================
// Multiply
// ============================================================

void MIPS32::mult(
    int rs,
    int rt)
{
    emit(R(rs, rt, 0, 0, 0x18));
}


void MIPS32::multu(
    int rs,
    int rt)
{
    emit(R(rs, rt, 0, 0, 0x19));
}


void MIPS32::mflo(
    int rd)
{
    emit(R(0, 0, rd, 0, 0x12));
}


void MIPS32::mfhi(
    int rd)
{
    emit(R(0, 0, rd, 0, 0x10));
}


// ============================================================
// Immediate
// ============================================================

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
    emit(IUnsigned(0x0C, rs, rt, imm));
}


void MIPS32::ori(
    int rt,
    int rs,
    uint16_t imm)
{
    emit(IUnsigned(0x0D, rs, rt, imm));
}


void MIPS32::xori(
    int rt,
    int rs,
    uint16_t imm)
{
    emit(IUnsigned(0x0E, rs, rt, imm));
}


void MIPS32::slti(
    int rt,
    int rs,
    int16_t imm)
{
    emit(I(0x0A, rs, rt, imm));
}


void MIPS32::lui(
    int rt,
    uint16_t imm)
{
    emit(IUnsigned(0x0F, 0, rt, imm));
}


// ============================================================
// Memory
// ============================================================

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


void MIPS32::lb(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x20, base, rt, offset));
}


void MIPS32::lbu(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x24, base, rt, offset));
}


void MIPS32::lh(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x21, base, rt, offset));
}


void MIPS32::lhu(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x25, base, rt, offset));
}


void MIPS32::sb(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x28, base, rt, offset));
}


void MIPS32::sh(
    int rt,
    int base,
    int16_t offset)
{
    emit(I(0x29, base, rt, offset));
}


// ============================================================
// Branches
// ============================================================

void MIPS32::beq(
    int rs,
    int rt,
    int16_t offset)
{
    emit(I(0x04, rs, rt, offset));
}


void MIPS32::bne(
    int rs,
    int rt,
    int16_t offset)
{
    emit(I(0x05, rs, rt, offset));
}


void MIPS32::bltz(
    int rs,
    int16_t offset)
{
    emit(I(0x01, rs, 0x00, offset));
}


void MIPS32::bgez(
    int rs,
    int16_t offset)
{
    emit(I(0x01, rs, 0x01, offset));
}


// ============================================================
// Jump
// ============================================================

void MIPS32::jr(
    int rs)
{
    emit(R(rs, 0, 0, 0, 0x08));
}


void MIPS32::jalr(
    int rd,
    int rs)
{
    emit(R(rs, 0, rd, 0, 0x09));
}


// ============================================================
// Dump
// ============================================================

void MIPS32::dump() const
{
    for (size_t i = 0; i < code.size(); ++i)
    {
        std::printf(
            "%04zu: %08X\n",
            i,
            code[i]
        );
    }
}

void MIPS32::lui(
    int rt,
    uint16_t imm
)
{
    emit(
        I(
            0x0F,
            0,
            rt,
            static_cast<int16_t>(imm)
        )
    );
}


// ============================================================
// Fixed shifts
// ============================================================

void MIPS32::sll(
    int rd,
    int rt,
    int shamt
)
{
    emit(
        R(
            0,
            rt,
            rd,
            shamt & 31,
            0x00
        )
    );
}


void MIPS32::srl(
    int rd,
    int rt,
    int shamt
)
{
    emit(
        R(
            0,
            rt,
            rd,
            shamt & 31,
            0x02
        )
    );
}


void MIPS32::sra(
    int rd,
    int rt,
    int shamt
)
{
    emit(
        R(
            0,
            rt,
            rd,
            shamt & 31,
            0x03
        )
    );
}


// ============================================================
// Variable shifts
// ============================================================

void MIPS32::sllv(
    int rd,
    int rt,
    int rs
)
{
    emit(
        R(
            rs,
            rt,
            rd,
            0,
            0x04
        )
    );
}


void MIPS32::srlv(
    int rd,
    int rt,
    int rs
)
{
    emit(
        R(
            rs,
            rt,
            rd,
            0,
            0x06
        )
    );
}


void MIPS32::srav(
    int rd,
    int rt,
    int rs
)
{
    emit(
        R(
            rs,
            rt,
            rd,
            0,
            0x07
        )
    );
}


// ============================================================
// Branch
// ============================================================
//
// На данном этапе используем:
//
//   BEQ $zero,$zero,offset
//
// Это безусловный branch.
// Offset должен быть байтовым ARM offset.
// MIPS branch displacement = offset / 4.
//

void MIPS32::branch(
    int32_t offset
)
{
    int32_t displacement =
        offset / 4;

    emit(
        I(
            0x04,
            0,
            0,
            static_cast<int16_t>(
                displacement
            )
        )
    );
}


// ============================================================
// Branch and link
// ============================================================
//
// Пока используем JAL с абсолютным адресом невозможно,
// поскольку backend ещё не знает финальный адрес блока.
//
// Поэтому временно используем:
//
//   JAL + placeholder
//
// Это будет заменено linker/relocation системой.
//

void MIPS32::branchAndLink(
    int32_t offset
)
{
    int32_t displacement =
        offset / 4;

    // Пока сохраняем branch-like representation.
    emit(
        I(
            0x04,
            0,
            0,
            static_cast<int16_t>(
                displacement
            )
        )
    );
}