#pragma once

#include <cstdint>

#include "arm_cpu.h"
#include "arm_memory.h"
#include "arm_decoder.h"
#include "arm_interpreter.h"

class ARMEngine
{
public:

    ARMEngine(
        ARMCPU& cpu,
        ARMMemory& memory
    )
        : cpu(cpu),
          memory(memory),
          interpreter(cpu, memory)
    {
    }

    bool step();

    void run(
        uint32_t maxInstructions
    );

private:

    void advancePC();
    
    uint16_t fetch16(uint32_t address) const;

    uint32_t fetch32(
        uint32_t address
    ) const;

private:

    ARMCPU& cpu;

    ARMMemory& memory;

    ARMInterpreter interpreter;
};