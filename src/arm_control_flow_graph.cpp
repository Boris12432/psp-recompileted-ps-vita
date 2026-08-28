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
        uint32_t address = worklist.back();
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

        /*
         * Безусловный B:
         *
         * следующий исполняемый блок —
         * branchTarget.
         */
        if (node.exit == BlockExit::Branch)
        {
            worklist.push_back(
                node.branchTarget
            );
        }

        /*
         * Условный B:
         *
         * существуют два пути:
         *
         *   1. branchTarget
         *   2. fallthroughAddress
         */
        else if (
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
        }

        /*
         * BL:
         *
         * Пока рассматриваем target
         * как следующий блок.
         *
         * Более полноценный CFG позже
         * сможет отдельно учитывать
         * возврат из функции.
         */
        else if (
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
        }

        /*
         * BX:
         *
         * Цель неизвестна статически,
         * поэтому новый адрес не добавляем.
         */
        else if (
            node.exit ==
            BlockExit::IndirectBranch
        )
        {
            // неизвестная статическая цель
        }

        /*
         * Return:
         *
         * блок заканчивается.
         */
        else if (
            node.exit ==
            BlockExit::Return
        )
        {
            // конец блока
        }

        /*
         * Если инструкция не является
         * branch/call/return, продолжаем
         * через fallthrough.
         *
         * Это понадобится, когда BlockBuilder
         * научится ограничивать блок другими
         * инструкциями.
         */
        else
        {
            if (node.endAddress != 0)
            {
                worklist.push_back(
                    node.endAddress
                );
            }
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


const BasicBlock* ARMControlFlowGraph::getNode(
    uint32_t address
) const
{
    auto it =
        nodes.find(address);

    if (it == nodes.end())
        return nullptr;

    return &it->second;
}