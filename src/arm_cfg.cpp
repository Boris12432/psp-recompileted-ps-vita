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
    /*
     * Уже построен?
     */
    if (graph.find(address) != graph.end())
        return;


    BasicBlock block =
        builder.build(address);


    CFGNode node;

    node.address =
        address;

    node.block =
        block;


    /*
     * ----------------------------------------------------------
     * Determine successors
     * ----------------------------------------------------------
     */

    switch (block.exit)
    {
        // ------------------------------------------------------
        // Unconditional branch
        // ------------------------------------------------------

        case BlockExit::Branch:
        {
            node.successors.push_back(
                block.branchTarget
            );

            break;
        }


        // ------------------------------------------------------
        // Conditional branch
        // ------------------------------------------------------

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


        // ------------------------------------------------------
        // Function call
        // ------------------------------------------------------

        case BlockExit::Call:
        {
            /*
             * BL имеет два логических выхода:
             *
             * 1. вызываемая функция
             * 2. инструкция после BL
             *
             * Пока считаем оба.
             */

            node.successors.push_back(
                block.branchTarget
            );

            node.successors.push_back(
                block.fallthroughAddress
            );

            break;
        }


        // ------------------------------------------------------
        // Indirect branch
        // ------------------------------------------------------

        case BlockExit::IndirectBranch:
        {
            /*
             * BX Rm невозможно разрешить
             * статически без анализа значения регистра.
             */

            break;
        }


        // ------------------------------------------------------
        // Normal instruction sequence
        // ------------------------------------------------------

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


        case BlockExit::Return:
        case BlockExit::Unknown:
            break;
    }


    /*
     * Сначала помещаем блок в граф.
     *
     * Это важно для циклов:
     *
     * A → B → A
     *
     * иначе рекурсия никогда не закончится.
     */

    graph.emplace(
        address,
        std::move(node)
    );


    /*
     * Теперь рекурсивно исследуем successors.
     */

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