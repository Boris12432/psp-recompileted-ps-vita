#pragma once

#include <cstdint>

#include "arm_cpu.h"
#include "arm_memory.h"
#include "ir.h"

struct MemoryAccess
{
    uint32_t baseAddress = 0;
    uint32_t offset = 0;
    uint32_t address = 0;

    bool add = true;
    bool preIndex = true;
    bool writeBack = false;
};


class ARMMemoryAccess
{
public:

    static uint32_t calculateOffset(
        const ARMCPU& cpu,
        const Operand2& operand
    );

    static MemoryAccess calculateAddress(
        const ARMCPU& cpu,
        const IRInstruction& instruction
    );

    static uint32_t read(
        ARMMemory& memory,
        uint32_t address,
        MemorySize size,
        bool signExtend
    );

    static void write(
        ARMMemory& memory,
        uint32_t address,
        MemorySize size,
        uint32_t value
    );
};