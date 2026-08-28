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

    void execute(
        const IRInstruction& instruction
    );

private:

    ARMCPU& cpu;

    ARMMemory& memory;

    uint32_t operand2(
        const Operand2& op,
        bool* carryOut
    );

    void setArithmeticFlags(
        uint32_t result,
        bool carry,
        bool overflow
    );
};