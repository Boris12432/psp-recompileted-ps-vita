#include "arm_engine.h"
#include "thumb_decoder.h"
#include "arm_decoder.h"

// ============================================================
// Fetch
// ============================================================

uint32_t ARMEngine::fetch32(uint32_t address) const
{
    return memory.read32(address);
}

uint16_t ARMEngine::fetch16(uint32_t address) const
{
    return memory.read16(address);
}

// ============================================================
// Step
// ============================================================

bool ARMEngine::step()
{
    // PC текущей инструкции.
    // Сохраняем отдельно, потому что сама инструкция
    // может изменить R15.
    const uint32_t realPC = cpu.r[15];

    IRInstruction ir;

    // ========================================================
    // THUMB / THUMB-2
    // ========================================================

    if (cpu.T)
    {
        const uint16_t first = fetch16(realPC);

        // ----------------------------------------------------
        // Thumb-2 32-bit instruction
        // ----------------------------------------------------

        if (ThumbDecoder::is32BitPrefix(first))
        {
            const uint16_t second =
                fetch16(realPC + 2);

            ir = ThumbDecoder::decode32(
                first,
                second
            );

            ir.address = realPC;

            const bool pcChanged =
                interpreter.execute(ir);

            if (!pcChanged)
            {
                cpu.r[15] = realPC + 4;
            }

            return true;
        }

        // ----------------------------------------------------
        // Thumb 16-bit instruction
        // ----------------------------------------------------

        ir = ThumbDecoder::decode(first);

        ir.address = realPC;

        const bool pcChanged =
            interpreter.execute(ir);

        if (!pcChanged)
        {
            cpu.r[15] = realPC + 2;
        }

        return true;
    }

    // ========================================================
    // ARM 32-bit
    // ========================================================

    const uint32_t instruction =
        fetch32(realPC);

    ir = ARMDecoder::decode(
        instruction
    );

    ir.address = realPC;

    const bool pcChanged =
        interpreter.execute(ir);

    if (!pcChanged)
    {
        cpu.r[15] = realPC + 4;
    }

    return true;
}

// ============================================================
// Run
// ============================================================

void ARMEngine::run(uint32_t maxInstructions)
{
    for (uint32_t i = 0; i < maxInstructions; ++i)
    {
        if (!step())
        {
            break;
        }
    }
}