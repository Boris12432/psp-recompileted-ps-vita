#include <cstdio>
#include <cstdint>

#include "arm_cpu.h"
#include "arm_decoder.h"
#include "arm_interpreter.h"

static void printCPU(const ARMCPU& cpu)
{
    for (int i = 0; i < 16; ++i) {

        printf(
            "R%-2d = %08X\n",
            i,
            cpu.r[i]
        );
    }

    printf(
        "N=%d Z=%d C=%d V=%d T=%d\n",
        cpu.N,
        cpu.Z,
        cpu.C,
        cpu.V,
        cpu.T
    );

    printf(
        "CPSR = %08X\n",
        cpu.cpsr()
    );
}

int main()
{
    SimpleMemory memory(
        1024 * 1024
    );

    ARMCPU cpu;

    ARMInterpreter interpreter(
        cpu,
        memory
    );

    // --------------------------------------------------------
    // MOV R0, #10
    // E3A0000A
    // --------------------------------------------------------

    uint32_t mov =
        0xE3A0000A;

    auto ir1 =
        ARMDecoder::decode(mov);

    interpreter.execute(ir1);

    // --------------------------------------------------------
    // MOV R1, #20
    // E3A01014
    // --------------------------------------------------------

    uint32_t mov2 =
        0xE3A01014;

    auto ir2 =
        ARMDecoder::decode(mov2);

    interpreter.execute(ir2);

    // --------------------------------------------------------
    // ADD R2, R0, R1
    // E0802001
    // --------------------------------------------------------

    uint32_t add =
        0xE0802001;

    auto ir3 =
        ARMDecoder::decode(add);

    interpreter.execute(ir3);

    printf(
        "=== ARM CPU ===\n"
    );

    printCPU(cpu);

    getchar();
    
    return 0;
}