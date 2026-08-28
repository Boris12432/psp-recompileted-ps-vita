#include "arm_control_flow_graph.h"

#include <set>
#include <vector>


void ARMControlFlowGraph::build(
    uint32_t entryAddress
)
{
    nodes.clear();

    std::set<uint32_t> visited;
    std::vector<uint32_t> worklist;

    worklist.push_back(entryAddress);


    while (!worklist.empty())
    {
        uint32_t address =
            worklist.back();

        worklist.pop_back();


        if (visited.count(address))
            continue;

        visited.insert(address);


        BasicBlock block =
            blockBuilder.build(address);


        uint32_t blockAddress =
            block.startAddress;


        nodes.emplace(
            blockAddress,
            std::move(block)
        );


        BasicBlock& node =
            nodes.at(blockAddress);


        // ====================================================
        // Unconditional branch
        // ====================================================

        if (node.exit == BlockExit::Branch)
        {
            worklist.push_back(
                node.branchTarget
            );

            continue;
        }


        // ====================================================
        // Conditional branch
        // ====================================================

        if (
            node.exit ==
            BlockExit::ConditionalBranch
        )
        {
            worklist.push_back(
                node.branchTarget
            );

            worklist.push_back(
                node.fallthroughAddress
            );

            continue;
        }


        // ====================================================
        // Call
        // ====================================================

        if (
            node.exit ==
            BlockExit::Call
        )
        {
            worklist.push_back(
                node.branchTarget
            );

            worklist.push_back(
                node.fallthroughAddress
            );

            continue;
        }


        // ====================================================
        // Indirect branch
        // ====================================================

        if (
            node.exit ==
            BlockExit::IndirectBranch
        )
        {
            // Статическую цель определить нельзя.

            continue;
        }


        // ====================================================
        // Return
        // ====================================================

        if (
            node.exit ==
            BlockExit::Return
        )
        {
            continue;
        }


        // ====================================================
        // Unknown
        // ====================================================

        if (
            node.exit ==
            BlockExit::Unknown
        )
        {
            /*
             * Очень важно:
             *
             * неизвестная инструкция НЕ означает,
             * что следующий адрес тоже является кодом.
             *
             * Поэтому здесь CFG останавливается.
             */

            continue;
        }


        // ====================================================
        // Fallthrough
        // ====================================================

        if (
            node.exit ==
            BlockExit::Fallthrough
        )
        {
            if (
                node.fallthroughAddress != 0
            )
            {
                worklist.push_back(
                    node.fallthroughAddress
                );
            }

            continue;
        }
    }
}


BasicBlock* ARMControlFlowGraph::getNode(
    uint32_t address
)
{
    auto it =
        nodes.find(address);

    if (it == nodes.end())
        return nullptr;

    return &it->second;
}


const BasicBlock*
ARMControlFlowGraph::getNode(
    uint32_t address
) const
{
    auto it =
        nodes.find(address);

    if (it == nodes.end())
        return nullptr;

    return &it->second;
}