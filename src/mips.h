#pragma once

#include <cstdint>
#include <vector>

class MIPS32
{
public:

    std::vector<uint32_t> code;

    void emit(uint32_t instruction);

    void nop();

    // --------------------------------------------------------
    // R-type
    // --------------------------------------------------------

    void addu(int rd, int rs, int rt);
    void subu(int rd, int rs, int rt);

    void and_(int rd, int rs, int rt);
    void or_(int rd, int rs, int rt);
    void xor_(int rd, int rs, int rt);

    void nor(int rd, int rs, int rt);

    void sll(int rd, int rt, int shamt);
    void srl(int rd, int rt, int shamt);
    void sra(int rd, int rt, int shamt);

    void sllv(int rd, int rt, int rs);
    void srlv(int rd, int rt, int rs);
    void srav(int rd, int rt, int rs);

    void slt(int rd, int rs, int rt);
    void sltu(int rd, int rs, int rt);

    void mult(int rs, int rt);
    void multu(int rs, int rt);

    void mflo(int rd);
    void mfhi(int rd);

    void jr(int rs);
    void jalr(int rd, int rs);

    // --------------------------------------------------------
    // I-type
    // --------------------------------------------------------

    void addiu(int rt, int rs, int16_t imm);

    void andi(int rt, int rs, uint16_t imm);
    void ori(int rt, int rs, uint16_t imm);
    void xori(int rt, int rs, uint16_t imm);

    void slti(int rt, int rs, int16_t imm);

    void lw(int rt, int base, int16_t offset);
    void sw(int rt, int base, int16_t offset);

    void lb(int rt, int base, int16_t offset);
    void lbu(int rt, int base, int16_t offset);

    void lh(int rt, int base, int16_t offset);
    void lhu(int rt, int base, int16_t offset);

    void sb(int rt, int base, int16_t offset);
    void sh(int rt, int base, int16_t offset);

    // --------------------------------------------------------
    // Branch
    // --------------------------------------------------------

    void beq(int rs, int rt, int16_t offset);
    void bne(int rs, int rt, int16_t offset);

    void bltz(int rs, int16_t offset);
    void bgez(int rs, int16_t offset);

    // --------------------------------------------------------
    // Immediate
    // --------------------------------------------------------

    void lui(int rt, uint16_t imm);

    // --------------------------------------------------------
    // Debug
    // --------------------------------------------------------

    void dump() const;

private:

    static uint32_t R(
        int rs,
        int rt,
        int rd,
        int shamt,
        int funct
    );

    static uint32_t I(
        int opcode,
        int rs,
        int rt,
        int16_t imm
    );

    static uint32_t IUnsigned(
        int opcode,
        int rs,
        int rt,
        uint16_t imm
    );
};