#include "arm_engine.h"
#include "thumb_decoder.h"


// ============================================================
// Fetch
// ============================================================

uint32_t ARMEngine::fetch32(
    uint32_t address
) const
{
    return memory.read32(address);
}


uint16_t ARMEngine::fetch16(
    uint32_t address
) const
{
    return memory.read16(address);
}


// ============================================================
// Step
// ============================================================

bool ARMEngine::step()
{
    // --------------------------------------------------------
    // Real address of current instruction
    // --------------------------------------------------------

    uint32_t realPC =
        cpu.r[15];


    // --------------------------------------------------------
    // ARM state
    // --------------------------------------------------------

        // ----------------------------------------------------
        // Fetch ARM instruction
        // ----------------------------------------------------

        uint32_t instruction =
            fetch32(realPC);


        // ----------------------------------------------------
        // Decode ARM instruction
        // ----------------------------------------------------

        IRInstruction ir =
            ARMDecoder::decode(instruction);


        // ----------------------------------------------------
        // Save real instruction address
        // ----------------------------------------------------

        ir.address =
            realPC;

        bool pcChanged =
            interpreter.execute(ir);


        // ----------------------------------------------------
        // Sequential PC update
        // ----------------------------------------------------

        if (!pcChanged)
        {
            if (cpu.T)
            {
                cpu.r[15] =
                    realPC + 2;
            }
        
            else
            {
            cpu.r[15] =
                realPC + 4;
            }
           
        }
        return true;

}

// ============================================================
// Run
// ============================================================

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