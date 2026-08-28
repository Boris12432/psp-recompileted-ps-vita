#pragma once

#include <cstdint>

#include "arm_memory.h"
#include "basic_block.h"

class ARMBlockBuilder
{
public:

    ARMBlockBuilder(
        ARMMemory& memory
    )
        : memory(memory)
    {
    }

    BasicBlock build(
        uint32_t startAddress
    );

private:

    ARMMemory& memory;
};