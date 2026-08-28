#pragma once

#include <cstdint>
#include <vector>

#include "ir.h"

enum class BlockExit
{
    Fallthrough,
    Branch,
    ConditionalBranch,
    Call,
    Return,
    IndirectBranch,
    Unknown
};

struct BasicBlock
{
    uint32_t startAddress = 0;
    uint32_t endAddress = 0;

    std::vector<IRInstruction> instructions;

    BlockExit exit = BlockExit::Unknown;

    uint32_t branchTarget = 0;
    uint32_t fallthroughAddress = 0;
};