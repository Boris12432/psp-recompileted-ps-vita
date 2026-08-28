#pragma once

#include <cstdint>
#include <map>

#include "arm_memory.h"
#include "arm_block_builder.h"
#include "basic_block.h"

class ARMControlFlowGraph
{
public:

    explicit ARMControlFlowGraph(
        ARMMemory& memory
    )
        : memory(memory),
          blockBuilder(memory)
    {
    }

    void build(
        uint32_t entryAddress
    );

    BasicBlock* getNode(
        uint32_t address
    );

    const BasicBlock* getNode(
        uint32_t address
    ) const;

private:

    ARMMemory& memory;

    ARMBlockBuilder blockBuilder;

    std::map<uint32_t, BasicBlock> nodes;
};