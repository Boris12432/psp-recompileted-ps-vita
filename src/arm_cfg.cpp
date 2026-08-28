#include "arm_cfg.h"


void ARMControlFlowGraph::build(
    uint32_t entryPoint
)
{
    graph.clear();

    processBlock(entryPoint);
}


void ARMControlFlowGraph::processBlock(
    uint32_t address
)
{
    // ========================================================
    // Already processed
    // ========================================================

    if (graph.find(address) != graph.end())
        return;


    // ========================================================
    // Build block
    // ========================================================

    BasicBlock block =
        builder.build(address);


    CFGNode node;

    node.address =
        address;

    node.block =
        block;


    // ========================================================
    // Determine successors
    // ========================================================

    switch (block.exit)
    {
        // ----------------------------------------------------
        // Unconditional B
        // ----------------------------------------------------

        case BlockExit::Branch:
        {
            node.successors.push_back(
                block.branchTarget
            );

            break;
        }


        // ----------------------------------------------------
        // Conditional B
        // ----------------------------------------------------

        case BlockExit::ConditionalBranch:
        {
            node.successors.push_back(
                block.branchTarget
            );

            node.successors.push_back(
                block.fallthroughAddress
            );

            break;
        }


        // ----------------------------------------------------
        // BL
        // ----------------------------------------------------

        case BlockExit::Call:
        {
            /*
             * BL:
             *
             * branch target
             * +
             * instruction after BL
             */

            node.successors.push_back(
                block.branchTarget
            );

            node.successors.push_back(
                block.fallthroughAddress
            );

            break;
        }


        // ----------------------------------------------------
        // BX
        // ----------------------------------------------------

        case BlockExit::IndirectBranch:
        {
            /*
             * BX Rm
             *
             * Static target unknown.
             */

            break;
        }


        // ----------------------------------------------------
        // Return
        // ----------------------------------------------------

        case BlockExit::Return:
        {
            break;
        }


        // ----------------------------------------------------
        // Unknown
        // ----------------------------------------------------

        case BlockExit::Unknown:
        {
            break;
        }


        // ----------------------------------------------------
        // Fallthrough
        // ----------------------------------------------------

        case BlockExit::Fallthrough:
        {
            if (block.fallthroughAddress != 0)
            {
                node.successors.push_back(
                    block.fallthroughAddress
                );
            }

            break;
        }
    }


    // ========================================================
    // Insert BEFORE recursion
    // ========================================================

    graph.emplace(
        address,
        std::move(node)
    );


    // ========================================================
    // Process successors
    // ========================================================

    const CFGNode& inserted =
        graph.at(address);


    for (uint32_t successor :
         inserted.successors)
    {
        processBlock(successor);
    }
}


const CFGNode*
ARMControlFlowGraph::getNode(
    uint32_t address
) const
{
    auto it =
        graph.find(address);

    if (it == graph.end())
        return nullptr;

    return &it->second;
}