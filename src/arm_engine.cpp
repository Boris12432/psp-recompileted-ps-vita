#include "arm_engine.h"


uint32_t ARMEngine::fetch32(
    uint32_t address
) const
{
    return memory.read32(address);
}


bool ARMEngine::step()
{
    // --------------------------------------------------------
    // Real address of current instruction
    // --------------------------------------------------------

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
    // Save real instruction address
    // --------------------------------------------------------

    ir.address =
        realPC;


    // --------------------------------------------------------
    // ARM architectural PC
    // --------------------------------------------------------
    //
    // In ARM state an instruction sees PC as:
    //
    //     current_address + 8
    //
    // This is important for B/BL.
    //

    if (!cpu.T)
    {
        cpu.r[15] =
            realPC + 8;
    }


    // --------------------------------------------------------
    // Execute
    // --------------------------------------------------------

    bool pcChanged =
        interpreter.execute(ir);


    // --------------------------------------------------------
    // Sequential PC update
    // --------------------------------------------------------
    //
    // execute() returns:
    //
    // false = instruction did NOT modify PC
    // true  = instruction modified PC
    //

    if (!pcChanged)
    {
        if (cpu.T)
        {
            // Thumb instruction = 16 bit

            cpu.r[15] =
                realPC + 2;
        }
        else
        {
            // ARM instruction = 32 bit

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