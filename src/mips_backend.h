#pragma once

#include <cstdint>
#include <vector>

#include "ir.h"
#include "mips.h"

class MIPSBackend
{
public:

    MIPSBackend();

    // Перевод одной IR-инструкции
    bool translate(
        const IRInstruction& ir
    );

    // Перевод блока IR
    bool translate(
        const std::vector<IRInstruction>& instructions
    );

    // Сгенерированный MIPS-код
    std::vector<uint32_t> code;

    // Очистить backend
    void clear();

private:

    MIPS32 mips;

    // ARM R0-R15 -> MIPS registers
    static int armReg(
        int arm
    );

    bool translateDataProcessing(
        const IRInstruction& ir
    );

    bool translateMemory(
        const IRInstruction& ir
    );

    bool translateBranch(
        const IRInstruction& ir
    );

    bool translateShift(
        const IRInstruction& ir
    );
};