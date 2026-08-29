#include <cstdio>
#include <cstdint>
#include <vector>

#include "arm_decoder.h"
#include "mips_backend.h"

int main()
{
    std::printf(
        "=== ARM -> IR -> MIPS backend test ===\n\n"
    );

    std::vector<uint32_t> armCode =
    {
        // MOV R0, #10
        0xE3A0000A,

        // MOV R1, #20
        0xE3A01014,

        // ADD R2, R0, R1
        0xE0802001,

        // SUB R3, R2, R0
        0xE0423000,

        // AND R4, R2, R3
        0xE0024003,

        // ORR R5, R2, R3
        0xE1825003,

        // EOR R6, R2, R3
        0xE0226003,

        // MUL R7, R0, R1
        0xE0000791
    };

    std::vector<IRInstruction> ir;

    // --------------------------------------------------------
    // ARM -> IR
    // --------------------------------------------------------

    for (uint32_t instruction : armCode)
    {
        IRInstruction decoded =
            ARMDecoder::decode(
                instruction
            );

        ir.push_back(decoded);
    }

    std::printf(
        "[OK] ARM -> IR: %zu instructions\n",
        ir.size()
    );

    // --------------------------------------------------------
    // IR -> MIPS
    // --------------------------------------------------------

    MIPSBackend backend;

    if (!backend.translateAll(ir))
    {
        std::printf(
            "[FAIL] IR -> MIPS\n"
        );

        return 1;
    }

    std::printf(
        "[OK] IR -> MIPS\n\n"
    );

    // --------------------------------------------------------
    // Dump
    // --------------------------------------------------------

    std::printf(
        "=== Generated MIPS ===\n"
    );

    backend.output().dump();

    std::printf(
        "\n[TEST] ARM -> IR -> MIPS PASSED\n"
    );
    getchar();
    return 0;
}