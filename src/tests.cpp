#include <cstdio>
#include <cstdint>
#include <cstdlib>

#include "arm_engine.h"
#include "arm_cpu.h"
#include "arm_decoder.h"
#include "arm_interpreter.h"

static int tests = 0;
static int passed = 0;

#define TEST(name, condition)                                      \
    do {                                                           \
        tests++;                                                   \
        if (condition) {                                           \
            passed++;                                              \
            printf("[PASS] %s\n", name);                           \
        } else {                                                   \
            printf("[FAIL] %s\n", name);                           \
            printf("        at %s:%d\n", __FILE__, __LINE__);      \
        }                                                          \
    } while (0)


static void execute(
    ARMInterpreter& interpreter,
    uint32_t instruction)
{
    IRInstruction ir =
        ARMDecoder::decode(instruction);

    interpreter.execute(ir);
}


int main()
{
    printf("========================================\n");
    printf("        ARM CORE TEST SUITE\n");
    printf("========================================\n\n");

    SimpleMemory memory(1024 * 1024);

    ARMCPU cpu;
    ARMEngine engine(
        cpu,
        memory
    );

    ARMInterpreter interpreter(
        cpu,
        memory
    );

    memory.write32(
    0x100,
    0xE3A0000A
    );

    memory.write32(
        0x104,
        0xE3A01014
    );

    memory.write32(
        0x108,
        0xE0802001
    );

    cpu.r[15] = 0x100;

    engine.step();

    TEST(
        "Engine executes instruction at PC",
        cpu.r[0] == 10
    );

    TEST(
        "PC advances by 4",
        cpu.r[15] == 0x104
    );

    engine.step();

    TEST(
        "Second instruction executes",
        cpu.r[1] == 20
    );

    TEST(
        "PC advances to third instruction",
        cpu.r[15] == 0x108
    );

    engine.step();

    TEST(
        "Third instruction executes",
        cpu.r[2] == 30
    );

    TEST(
        "PC advances after third instruction",
        cpu.r[15] == 0x10C
    );

    // ============================================================
    // MOV
    // ============================================================

    execute(
        interpreter,
        0xE3A0000A
    );

    TEST(
        "MOV R0,#10",
        cpu.r[0] == 10
    );


    execute(
        interpreter,
        0xE3A01014
    );

    TEST(
        "MOV R1,#20",
        cpu.r[1] == 20
    );


    // ============================================================
    // ADD
    // ============================================================

    execute(
        interpreter,
        0xE0802001
    );

    TEST(
        "ADD R2,R0,R1",
        cpu.r[2] == 30
    );


    // ============================================================
    // SUB
    // ============================================================

    execute(
        interpreter,
        0xE2423005
    );

    TEST(
        "SUB R3,R2,#5",
        cpu.r[3] == 25
    );


    // ============================================================
    // CMP
    // ============================================================

    execute(
        interpreter,
        0xE352001E
    );

    TEST(
        "CMP R2,#30 sets Z",
        cpu.Z == true
    );

    TEST(
        "CMP R2,#30 N=0",
        cpu.N == false
    );


    // ============================================================
    // Conditional ADD
    //
    // ADDEQ R4,R0,R1
    // ============================================================

    execute(
        interpreter,
        0x00804001
    );

    TEST(
        "ADDEQ executes when Z=1",
        cpu.r[4] == 30
    );


    // ============================================================
    // CMP with unequal value
    // ============================================================

    execute(
        interpreter,
        0xE352001F
    );

    TEST(
        "CMP R2,#31 clears Z",
        cpu.Z == false
    );


    // ADDEQ should NOT execute

    cpu.r[5] = 0;

    execute(
        interpreter,
        0x00805001
    );

    TEST(
        "ADDEQ does not execute when Z=0",
        cpu.r[5] == 0
    );


    // ============================================================
    // AND
    // ============================================================

    cpu.r[0] = 0xFF00FF00;
    cpu.r[1] = 0x0F0F0F0F;

    execute(
        interpreter,
        0xE0002001
    );

    TEST(
        "AND R2,R0,R1",
        cpu.r[2] == 0x0F000F00
    );


    // ============================================================
    // ORR
    // ============================================================

    execute(
        interpreter,
        0xE1803001
    );

    TEST(
        "ORR R3,R0,R1",
        cpu.r[3] == 0xFF0FFF0F
    );


    // ============================================================
    // EOR
    // ============================================================

    printf(
    "EOR test: R0=%08X R1=%08X\n",
    cpu.r[0],
    cpu.r[1]
);

    IRInstruction eorIR =
        ARMDecoder::decode(0xE0204001);

    printf(
        "EOR decoded: op=%d rn=%d rd=%d rm=%d\n",
        static_cast<int>(eorIR.op),
        eorIR.rn,
        eorIR.rd,
        eorIR.operand2.rm
    );

    execute(
        interpreter,
        0xE0204001
    );

    printf(
        "EOR result: R4=%08X expected=%08X\n",
        cpu.r[4],
        0xF20FF20F
    );

    execute(
        interpreter,
        0xE0204001
    );

    TEST(
        "EOR R4,R0,R1",
        cpu.r[4] == 0xF00FF00F
    );


    // ============================================================
    // TST
    // ============================================================

    cpu.r[0] = 0x00000010;
    cpu.r[1] = 0x00000010;

    execute(
        interpreter,
        0xE1100001
    );

    TEST(
        "TST sets Z=0 when bits match",
        cpu.Z == false
    );


    // ============================================================
    // TST zero
    // ============================================================

    cpu.r[1] = 0x00000001;

    execute(
        interpreter,
        0xE1100001
    );

    TEST(
        "TST sets Z=1 when result is zero",
        cpu.Z == true
    );


    // ============================================================
    // MEMORY ADDRESSING TESTS
    // ============================================================

    printf("\n");
    printf("========================================\n");
    printf("        MEMORY ADDRESSING TESTS\n");
    printf("========================================\n");


    // ------------------------------------------------------------
    // Word
    // ------------------------------------------------------------

    memory.write32(
        0x200,
        0x12345678
    );

    cpu.r[1] = 0x200;

    IRInstruction ldrWord;

    ldrWord.op = IROp::LDR;
    ldrWord.condition = Condition::AL;
    ldrWord.rd = 0;
    ldrWord.rn = 1;
    ldrWord.load = true;
    ldrWord.memorySize = MemorySize::Word;
    ldrWord.preIndex = true;
    ldrWord.up = true;
    ldrWord.writeBack = false;
    ldrWord.operand2.immediate = true;
    ldrWord.operand2.imm = 0;

    interpreter.execute(ldrWord);

    TEST(
        "LDR word",
        cpu.r[0] == 0x12345678
    );


    // ------------------------------------------------------------
    // Byte
    // ------------------------------------------------------------

    memory.write8(
        0x210,
        0xAB
    );

    cpu.r[1] = 0x210;

    IRInstruction ldrByte;

    ldrByte.op = IROp::LDR;
    ldrByte.rd = 0;
    ldrByte.rn = 1;
    ldrByte.load = true;
    ldrByte.memorySize = MemorySize::Byte;
    ldrByte.operand2.immediate = true;
    ldrByte.operand2.imm = 0;

    interpreter.execute(ldrByte);

    TEST(
        "LDRB",
        cpu.r[0] == 0x000000AB
    );


    // ------------------------------------------------------------
    // Signed byte
    // ------------------------------------------------------------

    memory.write8(
        0x211,
        0x80
    );

    cpu.r[1] = 0x211;

    IRInstruction ldrSignedByte;

    ldrSignedByte.op = IROp::LDR;
    ldrSignedByte.rd = 0;
    ldrSignedByte.rn = 1;
    ldrSignedByte.load = true;
    ldrSignedByte.memorySize = MemorySize::Byte;
    ldrSignedByte.signExtend = true;
    ldrSignedByte.operand2.immediate = true;
    ldrSignedByte.operand2.imm = 0;

    interpreter.execute(ldrSignedByte);

    TEST(
        "LDRSB sign extension",
        cpu.r[0] == 0xFFFFFF80
    );


    // ------------------------------------------------------------
    // Halfword
    // ------------------------------------------------------------

    memory.write16(
        0x220,
        0x1234
    );

    cpu.r[1] = 0x220;

    IRInstruction ldrHalf;

    ldrHalf.op = IROp::LDR;
    ldrHalf.rd = 0;
    ldrHalf.rn = 1;
    ldrHalf.load = true;
    ldrHalf.memorySize = MemorySize::Halfword;
    ldrHalf.operand2.immediate = true;
    ldrHalf.operand2.imm = 0;

    interpreter.execute(ldrHalf);

    TEST(
        "LDRH",
        cpu.r[0] == 0x00001234
    );


    // ------------------------------------------------------------
    // Signed halfword
    // ------------------------------------------------------------

    memory.write16(
        0x222,
        0x8000
    );

    cpu.r[1] = 0x222;

    IRInstruction ldrSignedHalf;

    ldrSignedHalf.op = IROp::LDR;
    ldrSignedHalf.rd = 0;
    ldrSignedHalf.rn = 1;
    ldrSignedHalf.load = true;
    ldrSignedHalf.memorySize = MemorySize::Halfword;
    ldrSignedHalf.signExtend = true;
    ldrSignedHalf.operand2.immediate = true;
    ldrSignedHalf.operand2.imm = 0;

    interpreter.execute(ldrSignedHalf);

    TEST(
        "LDRSH sign extension",
        cpu.r[0] == 0xFFFF8000
    );


    // ------------------------------------------------------------
    // Negative offset
    // ------------------------------------------------------------

    memory.write32(
        0x300,
        0xCAFEBABE
    );

    cpu.r[1] = 0x304;

    IRInstruction ldrNegative;

    ldrNegative.op = IROp::LDR;
    ldrNegative.rd = 0;
    ldrNegative.rn = 1;
    ldrNegative.load = true;
    ldrNegative.memorySize = MemorySize::Word;
    ldrNegative.up = false;
    ldrNegative.operand2.immediate = true;
    ldrNegative.operand2.imm = 4;

    interpreter.execute(ldrNegative);

    TEST(
        "LDR negative offset",
        cpu.r[0] == 0xCAFEBABE
    );


    // ------------------------------------------------------------
    // Write-back
    // ------------------------------------------------------------

    memory.write32(
        0x404,
        0xDEADBEEF
    );

    cpu.r[1] = 0x400;

    IRInstruction ldrWriteBack;

    ldrWriteBack.op = IROp::LDR;
    ldrWriteBack.rd = 0;
    ldrWriteBack.rn = 1;
    ldrWriteBack.load = true;
    ldrWriteBack.memorySize = MemorySize::Word;
    ldrWriteBack.preIndex = true;
    ldrWriteBack.up = true;
    ldrWriteBack.writeBack = true;
    ldrWriteBack.operand2.immediate = true;
    ldrWriteBack.operand2.imm = 4;

    interpreter.execute(ldrWriteBack);

    TEST(
        "LDR write-back value",
        cpu.r[0] == 0xDEADBEEF
    );

    TEST(
        "LDR write-back address",
        cpu.r[1] == 0x404
    );


    // ------------------------------------------------------------
    // Post-index
    // ------------------------------------------------------------

    memory.write32(
        0x500,
        0xAABBCCDD
    );

    cpu.r[1] = 0x500;

    IRInstruction ldrPost;

    ldrPost.op = IROp::LDR;
    ldrPost.rd = 0;
    ldrPost.rn = 1;
    ldrPost.load = true;
    ldrPost.memorySize = MemorySize::Word;
    ldrPost.preIndex = false;
    ldrPost.up = true;
    ldrPost.writeBack = true;
    ldrPost.operand2.immediate = true;
    ldrPost.operand2.imm = 4;

    interpreter.execute(ldrPost);

    TEST(
        "LDR post-index value",
        cpu.r[0] == 0xAABBCCDD
    );

    TEST(
        "LDR post-index write-back",
        cpu.r[1] == 0x504
    );


    // ============================================================
    // Final
    // ============================================================

    printf("\n========================================\n");

    printf(
        "RESULT: %d/%d tests passed\n",
        passed,
        tests
    );

    printf("========================================\n");
    getchar();
    return passed == tests ? 0 : 1;
}