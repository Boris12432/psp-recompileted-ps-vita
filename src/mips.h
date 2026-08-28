#pragma once

#include <cstdint>
#include <vector>

class MIPS32 {
public:

    std::vector<uint32_t> code;

    void emit(uint32_t instruction);

    void nop();

    void addu(int rd, int rs, int rt);
    void subu(int rd, int rs, int rt);

    void and_(int rd, int rs, int rt);
    void or_(int rd, int rs, int rt);
    void xor_(int rd, int rs, int rt);

    void addiu(int rt, int rs, int16_t imm);
    void andi(int rt, int rs, uint16_t imm);
    void ori(int rt, int rs, uint16_t imm);
    void xori(int rt, int rs, uint16_t imm);

    void lw(int rt, int base, int16_t offset);
    void sw(int rt, int base, int16_t offset);

    void jr(int rs);

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
};