#include "arm_engine.h"

uint32_t ARMEngine::fetch32(
    uint32_t address
) const
{
    return memory.read32(address);
}


void ARMEngine::advancePC()
{
    if (cpu.T)
    {
        // Thumb state:
        // инструкции пока будем реализовывать отдельно.
        cpu.r[15] += 2;
    }
    else
    {
        // ARM state
        cpu.r[15] += 4;
    }
}


bool ARMEngine::step()
{
    uint32_t address = cpu.r[15];

    uint32_t instruction =
        fetch32(address);

    IRInstruction ir =
        ARMDecoder::decode(instruction);

    /*
     * ARM instructions normally see PC as:
     *
     *     current_address + 8
     *
     * while executing in ARM state.
     *
     * Поэтому временно устанавливаем
     * архитектурное значение PC.
     */

    uint32_t realPC = cpu.r[15];

    if (!cpu.T)
    {
        cpu.r[15] =
            realPC + 8;
    }

    interpreter.execute(ir);

    /*
     * Если инструкция сама не изменила PC,
     * переходим к следующей.
     */

    if (cpu.r[15] == realPC + 8)
    {
        cpu.r[15] =
            realPC + 4;
    }

    return true;
}


void ARMEngine::run(
    uint32_t maxInstructions
)
{
    for (
        uint32_t i = 0;
        i < maxInstructions;
        ++i
    )
    {
        if (!step())
            break;
    }
}