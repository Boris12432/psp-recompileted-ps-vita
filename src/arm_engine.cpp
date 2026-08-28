#include "arm_engine.h"


uint32_t ARMEngine::fetch32(
    uint32_t address
) const
{
    return memory.read32(address);
}


bool ARMEngine::step()
{
    /*
     * PC до выполнения инструкции.
     *
     * В ARM state CPU при исполнении инструкции
     * видит PC как address + 8.
     *
     * В Thumb state пока используем текущий PC
     * и размер инструкции 2 байта.
     */

    uint32_t realPC =
        cpu.r[15];


    // --------------------------------------------------------
    // Fetch
    // --------------------------------------------------------

    uint32_t instruction =
        fetch32(realPC);


    // --------------------------------------------------------
    // Decode
    // --------------------------------------------------------

    IRInstruction ir =
        ARMDecoder::decode(instruction);


    // --------------------------------------------------------
    // Architectural PC
    // --------------------------------------------------------

    if (!cpu.T)
    {
        /*
         * ARM:
         *
         * instruction address = realPC
         * visible PC          = realPC + 8
         */

        cpu.r[15] =
            realPC + 8;
    }


    // --------------------------------------------------------
    // Execute
    // --------------------------------------------------------

    bool pcChanged =
        interpreter.execute(ir);


    // --------------------------------------------------------
    // Sequential execution
    // --------------------------------------------------------

    if (!pcChanged)
    {
        if (cpu.T)
        {
            /*
             * Thumb instruction:
             * 16-bit = 2 bytes
             */

            cpu.r[15] =
                realPC + 2;
        }
        else
        {
            /*
             * ARM instruction:
             * 32-bit = 4 bytes
             */

            cpu.r[15] =
                realPC + 4;
        }
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