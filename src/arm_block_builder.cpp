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

    while (true)
    {
        uint32_t instruction =
            memory.read32(pc);

        IRInstruction ir =
            ARMDecoder::decode(instruction);

        ir.address = pc;

        block.instructions.push_back(ir);

        uint32_t nextPC =
            pc + 4;

        block.endAddress =
            nextPC;

        if (ir.op == IROp::INVALID)
        {
            block.exit = BlockExit::Unknown;
            block.fallthroughAddress = 0;

            return block;
        }

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
}