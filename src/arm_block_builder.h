#pragma once

#include <cstdint>

#include "arm_memory.h"
#include "arm_decoder.h"
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
        uint32_t address
    );

private:

    ARMMemory& memory;
};