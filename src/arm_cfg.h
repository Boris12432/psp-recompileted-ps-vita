#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "basic_block.h"
#include "arm_block_builder.h"

struct CFGNode
{
    uint32_t address = 0;

    BasicBlock block;

    std::vector<uint32_t> successors;
};

class ARMControlFlowGraph
{
public:

    ARMControlFlowGraph(
        ARMBlockBuilder& builder
    )
        : builder(builder)
    {
    }

    void build(
        uint32_t entryPoint
    );

    const CFGNode* getNode(
        uint32_t address
    ) const;

    const std::unordered_map<
        uint32_t,
        CFGNode
    >& nodes() const
    {
        return graph;
    }

private:

    void processBlock(
        uint32_t address
    );

private:

    ARMBlockBuilder& builder;

    std::unordered_map<
        uint32_t,
        CFGNode
    > graph;
};