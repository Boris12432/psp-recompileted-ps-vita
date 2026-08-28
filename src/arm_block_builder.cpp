#include "arm_block_builder.h"

#include <cstdio>
#include <iostream>

BasicBlock ARMBlockBuilder::build(
    uint32_t address
)
{

    BasicBlock block;

    block.startAddress = address;

    uint32_t pc = address;

    constexpr uint32_t MAX_INSTRUCTIONS = 64;

    for (uint32_t count = 0; count < MAX_INSTRUCTIONS; ++count)
    {
        uint32_t instruction =
            memory.read32(pc);

        if (instruction == 0)
        {
            block.exit =
                BlockExit::Fallthrough;

            block.endAddress =
                pc;

            return block;
        }

        IRInstruction ir =
            ARMDecoder::decode(instruction);

        ir.address = pc;

        block.instructions.push_back(ir);

        uint32_t nextPC =
            pc + 4;

        block.endAddress =
            nextPC;

        switch (ir.op)
        {
            case IROp::B:
            {
                block.branchTarget =
                    pc + 8 + ir.branchOffset;

                block.fallthroughAddress =
                    nextPC;

                if (ir.condition == Condition::AL)
                    block.exit = BlockExit::Branch;
                else
                    block.exit = BlockExit::ConditionalBranch;

                return block;
            }

            case IROp::BL:
            {
                block.branchTarget =
                    pc + 8 + ir.branchOffset;

                block.fallthroughAddress =
                    nextPC;

                block.exit = BlockExit::Call;


                return block;
            }

            case IROp::BX:
            {
                block.fallthroughAddress =
                    nextPC;

                if (ir.rm == 14)
                    block.exit = BlockExit::Return;
                else
                    block.exit = BlockExit::IndirectBranch;


                return block;
            }

            default:
                break;
        }

        pc = nextPC;
    }

    // Мы дошли до лимита.
    block.exit = BlockExit::Unknown;

    return block;
}