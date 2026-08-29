#pragma once

#include <cstdint>

#include "arm_cpu.h"
#include "arm_memory.h"
#include "arm_decoder.h"
#include "ir.h"


class ARMInterpreter
{
public:

    ARMInterpreter(
        ARMCPU& cpu,
        ARMMemory& memory
    )
        : cpu(cpu),
          memory(memory)
    {
    }


    /*
     * Выполнить одну IR-инструкцию.
     *
     * return true:
     *     инструкция изменила PC
     *     (B / BL / BX)
     *
     * return false:
     *     PC должен перейти
     *     на следующую инструкцию.
     */
    bool execute(
        const IRInstruction& ir
    );


private:

    ARMCPU& cpu;

    ARMMemory& memory;


    /*
     * Read a general-purpose register as a *source* operand.
     *
     * On real ARM hardware, reading R15 (PC) as a source operand
     * (data-processing Rn/Rm, LDR/STR base register, BX target, etc.)
     * yields (address of current instruction + 8) due to the
     * classic 3-stage fetch/decode/execute pipeline - NOT the raw
     * value stored in r[15].
     *
     * This must never be used for the *destination* register (Rd),
     * since writing PC is a real branch, not a pipeline artifact.
     */
    uint32_t readReg(
        int reg,
        uint32_t currentAddress
    ) const
    {
        if (reg == 15)
        {
            return currentAddress + 8;
        }

        return cpu.r[reg];
    }


    uint32_t operand2(
        const Operand2& op,
        uint32_t currentAddress,
        bool* carryOut
    );


    void setArithmeticFlags(
        uint32_t result,
        bool carry,
        bool overflow
    );
};