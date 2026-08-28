#include "arm_block_builder.h"
#include <iostream>
#include "arm_decoder.h"


BasicBlock ARMBlockBuilder::build(
    uint32_t startAddress
)
{
    BasicBlock block;

    block.startAddress = startAddress;
    block.endAddress = startAddress;

    uint32_t pc = startAddress;

    while (true)
    {
        std::cout
            << "[BLOCK] read instruction @ 0x"
            << std::hex
            << pc
            << std::dec
            << "\n";

        if (!memory.isMapped(pc) ||
            !memory.isMapped(pc + 1) ||
            !memory.isMapped(pc + 2) ||
            !memory.isMapped(pc + 3))
        {
            std::cout
                << "[BLOCK] unmapped memory @ 0x"
                << std::hex
                << pc
                << std::dec
                << "\n";

            block.endAddress = pc;

            block.exit =
                BlockExit::Unknown;

            block.fallthroughAddress = 0;

            return block;
        }

        uint32_t instruction =
            memory.read32(pc);

        IRInstruction ir =
            ARMDecoder::decode(instruction);

        ir.address = pc;

        block.instructions.push_back(ir);

        uint32_t nextPC =
            pc + 4;


        // ====================================================
        // INVALID
        // ====================================================

        if (ir.op == IROp::INVALID)
        {
            block.endAddress = pc;

            block.exit =
                BlockExit::Unknown;

            block.fallthroughAddress = 0;

            return block;
        }


        // ====================================================
        // B
        // ====================================================

        if (ir.op == IROp::B)
        {
            /*
             * ARM PC during execution:
             *
             * PC = instruction address + 8
             *
             * Branch:
             *
             * target = PC + signed_offset
             */

            block.branchTarget =
                pc + 8 +
                static_cast<int32_t>(
                    ir.branchOffset
                );

            block.fallthroughAddress =
                nextPC;

            block.endAddress =
                nextPC;


            /*
             * B AL
             *
             * unconditional
             */

            if (ir.condition == Condition::AL)
            {
                block.exit =
                    BlockExit::Branch;
            }
            else
            {
                /*
                 * B<cond>
                 *
                 * Two possible paths:
                 *
                 * target
                 * fallthrough
                 */

                block.exit =
                    BlockExit::ConditionalBranch;
            }

            return block;
        }


        // ====================================================
        // BL
        // ====================================================

        if (ir.op == IROp::BL)
        {
            block.branchTarget =
                pc + 8 +
                static_cast<int32_t>(
                    ir.branchOffset
                );

            block.fallthroughAddress =
                nextPC;

            block.endAddress =
                nextPC;

            block.exit =
                BlockExit::Call;

            return block;
        }


        // ====================================================
        // BX
        // ====================================================

        if (ir.op == IROp::BX)
        {
            block.fallthroughAddress =
                nextPC;

            block.endAddress =
                nextPC;


            /*
             * BX LR
             *
             * R14 = LR
             *
             * Typical ARM function return.
             */

            if (ir.rm == 14)
            {
                block.exit =
                    BlockExit::Return;
            }
            else
            {
                block.exit =
                    BlockExit::IndirectBranch;
            }

            return block;
        }


        // ====================================================
        // Normal instruction
        // ====================================================

        block.endAddress =
            nextPC;

        block.fallthroughAddress =
            nextPC;

        block.exit =
            BlockExit::Fallthrough;

        pc =
            nextPC;
    }
}