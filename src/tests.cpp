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


    // ============================================================
    // ENGINE BASIC EXECUTION
    // ============================================================

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
    // CMP unequal
    // ============================================================

    execute(
        interpreter,
        0xE352001F
    );

    TEST(
        "CMP R2,#31 clears Z",
        cpu.Z == false
    );


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
    // MEMORY ADDRESSING
    // ============================================================

    printf("\n");
    printf("========================================\n");
    printf("        MEMORY ADDRESSING TESTS\n");
    printf("========================================\n");


    // ------------------------------------------------------------
    // LDR word
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
    // LDRB
    // ------------------------------------------------------------

    memory.write8(
        0x210,
        0xAB
    );

    cpu.r[1] = 0x210;

    IRInstruction ldrByte;

    ldrByte.op = IROp::LDR;
    ldrByte.condition = Condition::AL;
    ldrByte.rd = 0;
    ldrByte.rn = 1;
    ldrByte.load = true;
    ldrByte.memorySize = MemorySize::Byte;
    ldrByte.preIndex = true;
    ldrByte.up = true;
    ldrByte.writeBack = false;
    ldrByte.operand2.immediate = true;
    ldrByte.operand2.imm = 0;

    interpreter.execute(ldrByte);

    TEST(
        "LDRB",
        cpu.r[0] == 0x000000AB
    );


    // ------------------------------------------------------------
    // LDRSB
    // ------------------------------------------------------------

    memory.write8(
        0x211,
        0x80
    );

    cpu.r[1] = 0x211;

    IRInstruction ldrSignedByte;

    ldrSignedByte.op = IROp::LDR;
    ldrSignedByte.condition = Condition::AL;
    ldrSignedByte.rd = 0;
    ldrSignedByte.rn = 1;
    ldrSignedByte.load = true;
    ldrSignedByte.memorySize = MemorySize::Byte;
    ldrSignedByte.signExtend = true;
    ldrSignedByte.preIndex = true;
    ldrSignedByte.up = true;
    ldrSignedByte.writeBack = false;
    ldrSignedByte.operand2.immediate = true;
    ldrSignedByte.operand2.imm = 0;

    interpreter.execute(ldrSignedByte);

    TEST(
        "LDRSB sign extension",
        cpu.r[0] == 0xFFFFFF80
    );


    // ------------------------------------------------------------
    // LDRH
    // ------------------------------------------------------------

    memory.write16(
        0x220,
        0x1234
    );

    cpu.r[1] = 0x220;

    IRInstruction ldrHalf;

    ldrHalf.op = IROp::LDR;
    ldrHalf.condition = Condition::AL;
    ldrHalf.rd = 0;
    ldrHalf.rn = 1;
    ldrHalf.load = true;
    ldrHalf.memorySize = MemorySize::Halfword;
    ldrHalf.preIndex = true;
    ldrHalf.up = true;
    ldrHalf.writeBack = false;
    ldrHalf.operand2.immediate = true;
    ldrHalf.operand2.imm = 0;

    interpreter.execute(ldrHalf);

    TEST(
        "LDRH",
        cpu.r[0] == 0x00001234
    );


    // ------------------------------------------------------------
    // LDRSH
    // ------------------------------------------------------------

    memory.write16(
        0x222,
        0x8000
    );

    cpu.r[1] = 0x222;

    IRInstruction ldrSignedHalf;

    ldrSignedHalf.op = IROp::LDR;
    ldrSignedHalf.condition = Condition::AL;
    ldrSignedHalf.rd = 0;
    ldrSignedHalf.rn = 1;
    ldrSignedHalf.load = true;
    ldrSignedHalf.memorySize = MemorySize::Halfword;
    ldrSignedHalf.signExtend = true;
    ldrSignedHalf.preIndex = true;
    ldrSignedHalf.up = true;
    ldrSignedHalf.writeBack = false;
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
    ldrNegative.condition = Condition::AL;
    ldrNegative.rd = 0;
    ldrNegative.rn = 1;
    ldrNegative.load = true;
    ldrNegative.memorySize = MemorySize::Word;
    ldrNegative.preIndex = true;
    ldrNegative.up = false;
    ldrNegative.writeBack = false;
    ldrNegative.operand2.immediate = true;
    ldrNegative.operand2.imm = 4;

    interpreter.execute(ldrNegative);

    TEST(
        "LDR negative offset",
        cpu.r[0] == 0xCAFEBABE
    );


    // ------------------------------------------------------------
    // LDR write-back
    // ------------------------------------------------------------

    memory.write32(
        0x404,
        0xDEADBEEF
    );

    cpu.r[1] = 0x400;

    IRInstruction ldrWriteBack;

    ldrWriteBack.op = IROp::LDR;
    ldrWriteBack.condition = Condition::AL;
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
    // LDR post-index
    // ------------------------------------------------------------

    memory.write32(
        0x500,
        0xAABBCCDD
    );

    cpu.r[1] = 0x500;

    IRInstruction ldrPost;

    ldrPost.op = IROp::LDR;
    ldrPost.condition = Condition::AL;
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
    // STORE TESTS
    // ============================================================

    printf("\n");
    printf("========================================\n");
    printf("             STORE TESTS\n");
    printf("========================================\n");


    // ------------------------------------------------------------
    // STR word
    // ------------------------------------------------------------

    cpu.r[0] = 0x11223344;
    cpu.r[1] = 0x600;

    IRInstruction strWord;

    strWord.op = IROp::STR;
    strWord.condition = Condition::AL;
    strWord.rd = 0;
    strWord.rn = 1;
    strWord.load = false;
    strWord.memorySize = MemorySize::Word;
    strWord.preIndex = true;
    strWord.up = true;
    strWord.writeBack = false;
    strWord.operand2.immediate = true;
    strWord.operand2.imm = 0;

    interpreter.execute(strWord);

    TEST(
        "STR word",
        memory.read32(0x600) == 0x11223344
    );


    // ------------------------------------------------------------
    // STRB
    // ------------------------------------------------------------

    cpu.r[0] = 0x123456AB;
    cpu.r[1] = 0x610;

    IRInstruction strByte;

    strByte.op = IROp::STR;
    strByte.condition = Condition::AL;
    strByte.rd = 0;
    strByte.rn = 1;
    strByte.load = false;
    strByte.memorySize = MemorySize::Byte;
    strByte.preIndex = true;
    strByte.up = true;
    strByte.writeBack = false;
    strByte.operand2.immediate = true;
    strByte.operand2.imm = 0;

    interpreter.execute(strByte);

    TEST(
        "STRB",
        memory.read8(0x610) == 0xAB
    );


    // ------------------------------------------------------------
    // STRH
    // ------------------------------------------------------------

    cpu.r[0] = 0x1234ABCD;
    cpu.r[1] = 0x620;

    IRInstruction strHalf;

    strHalf.op = IROp::STR;
    strHalf.condition = Condition::AL;
    strHalf.rd = 0;
    strHalf.rn = 1;
    strHalf.load = false;
    strHalf.memorySize = MemorySize::Halfword;
    strHalf.preIndex = true;
    strHalf.up = true;
    strHalf.writeBack = false;
    strHalf.operand2.immediate = true;
    strHalf.operand2.imm = 0;

    interpreter.execute(strHalf);

    TEST(
        "STRH",
        memory.read16(0x620) == 0xABCD
    );


    // ------------------------------------------------------------
    // STR negative offset
    // ------------------------------------------------------------

    cpu.r[0] = 0xCAFEBABE;
    cpu.r[1] = 0x704;

    IRInstruction strNegative;

    strNegative.op = IROp::STR;
    strNegative.condition = Condition::AL;
    strNegative.rd = 0;
    strNegative.rn = 1;
    strNegative.load = false;
    strNegative.memorySize = MemorySize::Word;
    strNegative.preIndex = true;
    strNegative.up = false;
    strNegative.writeBack = false;
    strNegative.operand2.immediate = true;
    strNegative.operand2.imm = 4;

    interpreter.execute(strNegative);

    TEST(
        "STR negative offset",
        memory.read32(0x700) == 0xCAFEBABE
    );


    // ------------------------------------------------------------
    // STR write-back
    // ------------------------------------------------------------

    cpu.r[0] = 0xDEADBEEF;
    cpu.r[1] = 0x800;

    IRInstruction strWriteBack;

    strWriteBack.op = IROp::STR;
    strWriteBack.condition = Condition::AL;
    strWriteBack.rd = 0;
    strWriteBack.rn = 1;
    strWriteBack.load = false;
    strWriteBack.memorySize = MemorySize::Word;
    strWriteBack.preIndex = true;
    strWriteBack.up = true;
    strWriteBack.writeBack = true;
    strWriteBack.operand2.immediate = true;
    strWriteBack.operand2.imm = 4;

    interpreter.execute(strWriteBack);

    TEST(
        "STR write-back value",
        memory.read32(0x804) == 0xDEADBEEF
    );

    TEST(
        "STR write-back address",
        cpu.r[1] == 0x804
    );


    // ------------------------------------------------------------
    // STR post-index
    // ------------------------------------------------------------

    cpu.r[0] = 0xAABBCCDD;
    cpu.r[1] = 0x900;

    IRInstruction strPost;

    strPost.op = IROp::STR;
    strPost.condition = Condition::AL;
    strPost.rd = 0;
    strPost.rn = 1;
    strPost.load = false;
    strPost.memorySize = MemorySize::Word;
    strPost.preIndex = false;
    strPost.up = true;
    strPost.writeBack = true;
    strPost.operand2.immediate = true;
    strPost.operand2.imm = 4;

    interpreter.execute(strPost);

    TEST(
        "STR post-index value",
        memory.read32(0x900) == 0xAABBCCDD
    );

    TEST(
        "STR post-index write-back",
        cpu.r[1] == 0x904
    );

        // ============================================================
    // SHIFTER TESTS
    // ============================================================

    printf("\n");
    printf("========================================\n");
    printf("             SHIFTER TESTS\n");
    printf("========================================\n");


    // ------------------------------------------------------------
    // LSL #4
    //
    // MOV R0, R1, LSL #4
    //
    // R1 = 0x00000001
    // R0 = 0x00000010
    // ------------------------------------------------------------

    cpu.r[1] = 0x00000001;
    cpu.C = false;

    execute(
        interpreter,
        0xE1A00201
    );

    TEST(
        "MOV R0,R1,LSL #4",
        cpu.r[0] == 0x00000010
    );


    // ------------------------------------------------------------
    // LSL carry
    //
    // 0x80000000 << 1
    // result = 0
    // carry = 1
    // ------------------------------------------------------------

    cpu.r[1] = 0x80000000u;
    cpu.C = false;

    execute(
        interpreter,
        0xE1B00081
    );

    TEST(
        "MOVS R0,R1,LSL #1 result",
        cpu.r[0] == 0x00000000
    );

    TEST(
        "MOVS LSL carry",
        cpu.C == true
    );


    // ------------------------------------------------------------
    // LSR #4
    // ------------------------------------------------------------

    cpu.r[1] = 0x00000010;
    cpu.C = false;

    execute(
        interpreter,
        0xE1A00221
    );

    TEST(
        "MOV R0,R1,LSR #4",
        cpu.r[0] == 0x00000001
    );


    // ------------------------------------------------------------
    // LSR carry
    //
    // 0x11 >> 1
    // result = 0x08
    // carry = bit 0 = 1
    // ------------------------------------------------------------

    cpu.r[1] = 0x00000011;
    cpu.C = false;

    execute(
        interpreter,
        0xE1B000A1
    );

    TEST(
        "MOVS R0,R1,LSR #1 result",
        cpu.r[0] == 0x00000008
    );

    TEST(
        "MOVS LSR carry",
        cpu.C == true
    );


    // ------------------------------------------------------------
    // ASR #4
    // ------------------------------------------------------------

    cpu.r[1] = 0xFFFFFFF0u;
    cpu.C = false;

    execute(
        interpreter,
        0xE1A00241
    );

    TEST(
        "MOV R0,R1,ASR #4",
        cpu.r[0] == 0xFFFFFFFFu
    );


    // ------------------------------------------------------------
    // ASR positive
    // ------------------------------------------------------------

    cpu.r[1] = 0x000000F0;
    cpu.C = false;

    execute(
        interpreter,
        0xE1A00241
    );

    TEST(
        "MOV R0,R1,ASR #4 positive",
        cpu.r[0] == 0x0000000F
    );


    // ------------------------------------------------------------
    // ROR #8
    // ------------------------------------------------------------

    cpu.r[1] = 0x12345678;

    execute(
        interpreter,
        0xE1A00461
    );

    TEST(
        "MOV R0,R1,ROR #8",
        cpu.r[0] == 0x78123456
    );


    // ------------------------------------------------------------
    // ROR carry
    //
    // ROR #8
    // carry = bit 7 of original value
    // ------------------------------------------------------------

    cpu.r[1] = 0x00000080;
    cpu.C = false;

    execute(
        interpreter,
        0xE1B00461
    );

    TEST(
        "MOVS ROR #8 result",
        cpu.r[0] == 0x80000000u
    );

    TEST(
        "MOVS ROR #8 carry",
        cpu.C == true
    );


    // ------------------------------------------------------------
    // RRX
    //
    // old C = 1
    //
    // value = 0x00000002
    //
    // result:
    //
    // 0x80000001
    //
    // carry = old bit 0 = 0
    // ------------------------------------------------------------

    cpu.r[1] = 0x00000002;
    cpu.C = true;

    execute(
        interpreter,
        0xE1B00061
    );

    TEST(
        "MOVS RRX result",
        cpu.r[0] == 0x80000001u
    );

    TEST(
        "MOVS RRX carry",
        cpu.C == false
    );


    // ------------------------------------------------------------
    // LSL #0
    //
    // Carry must remain unchanged.
    // ------------------------------------------------------------

    cpu.r[1] = 0x12345678;
    cpu.C = true;

    execute(
        interpreter,
        0xE1B00001
    );

    TEST(
        "MOVS LSL #0 value unchanged",
        cpu.r[0] == 0x12345678
    );

    TEST(
        "MOVS LSL #0 preserves carry",
        cpu.C == true
    );


    // ------------------------------------------------------------
    // LSR #0
    //
    // ARM semantics:
    //
    // LSR #0 == LSR #32
    // ------------------------------------------------------------

    cpu.r[1] = 0x80000001u;
    cpu.C = false;

    execute(
        interpreter,
        0xE1B00021
    );

    TEST(
        "MOVS LSR #0 result",
        cpu.r[0] == 0
    );

    TEST(
        "MOVS LSR #0 carry",
        cpu.C == true
    );


    // ------------------------------------------------------------
    // ASR #0
    //
    // ARM semantics:
    //
    // ASR #0 == ASR #32
    // ------------------------------------------------------------

    cpu.r[1] = 0x80000000u;
    cpu.C = false;

    execute(
        interpreter,
        0xE1B00041
    );

    TEST(
        "MOVS ASR #0 result",
        cpu.r[0] == 0xFFFFFFFFu
    );

    TEST(
        "MOVS ASR #0 carry",
        cpu.C == true
    );


    // ============================================================
    // REGISTER SHIFT TESTS
    // ============================================================

    printf("\n");
    printf("========================================\n");
    printf("         REGISTER SHIFT TESTS\n");
    printf("========================================\n");


    // ------------------------------------------------------------
    // LSL Rs
    //
    // MOV R0,R1,LSL R2
    // ------------------------------------------------------------

    cpu.r[1] = 0x00000001;
    cpu.r[2] = 4;

    execute(
        interpreter,
        0xE1A00211
    );

    TEST(
        "MOV R0,R1,LSL R2",
        cpu.r[0] == 0x00000010
    );


    // ------------------------------------------------------------
    // LSR Rs
    // ------------------------------------------------------------

    cpu.r[1] = 0x00000010;
    cpu.r[2] = 2;

    execute(
        interpreter,
        0xE1A00231
    );

    TEST(
        "MOV R0,R1,LSR R2",
        cpu.r[0] == 0x00000004
    );


    // ------------------------------------------------------------
    // ASR Rs
    // ------------------------------------------------------------

    cpu.r[1] = 0xFFFFFFF0u;
    cpu.r[2] = 2;

    execute(
        interpreter,
        0xE1A00251
    );

    TEST(
        "MOV R0,R1,ASR R2",
        cpu.r[0] == 0xFFFFFFFCu
    );


    // ------------------------------------------------------------
    // ROR Rs
    // ------------------------------------------------------------

    cpu.r[1] = 0x12345678;
    cpu.r[2] = 8;

    execute(
        interpreter,
        0xE1A00271
    );

    TEST(
        "MOV R0,R1,ROR R2",
        cpu.r[0] == 0x78123456
    );


    // ------------------------------------------------------------
    // Register shift amount = 0
    //
    // Result unchanged.
    // Carry unchanged.
    // ------------------------------------------------------------

    cpu.r[1] = 0x12345678;
    cpu.r[2] = 0;
    cpu.C = true;

    execute(
        interpreter,
        0xE1B00211
    );

    TEST(
        "MOVS register LSL #0 value",
        cpu.r[0] == 0x12345678
    );

    TEST(
        "MOVS register LSL #0 carry",
        cpu.C == true
    );

        // ============================================================
    // BRANCH TESTS
    // ============================================================

    printf("\n");
    printf("========================================\n");
    printf("             BRANCH TESTS\n");
    printf("========================================\n");


    // ------------------------------------------------------------
    // B
    //
    // B +8
    //
    // ARM branch offset is relative to architectural PC.
    // Architectural PC = current address + 8.
    // ------------------------------------------------------------

    cpu.r[15] = 0x1000;

    IRInstruction branch;

    branch.op =
        IROp::B;

    branch.condition =
        Condition::AL;

    branch.branchOffset =
        8;

    interpreter.execute(branch);

    TEST(
        "B forward +8",
        cpu.r[15] == 0x1008
    );


    // ------------------------------------------------------------
    // B negative
    // ------------------------------------------------------------

    cpu.r[15] = 0x2000;

    IRInstruction branchBack;

    branchBack.op =
        IROp::B;

    branchBack.condition =
        Condition::AL;

    branchBack.branchOffset =
        static_cast<int32_t>(-8);

    interpreter.execute(branchBack);

    TEST(
        "B backward -8",
        cpu.r[15] == 0x1FF8
    );


    // ------------------------------------------------------------
    // Conditional BEQ
    // ------------------------------------------------------------

    cpu.r[15] = 0x3000;

    cpu.Z = true;

    IRInstruction beq;

    beq.op =
        IROp::B;

    beq.condition =
        Condition::EQ;

    beq.branchOffset =
        0x20;

    interpreter.execute(beq);

    TEST(
        "BEQ executes when Z=1",
        cpu.r[15] == 0x3020
    );


    // ------------------------------------------------------------
    // Conditional BNE must NOT execute
    // ------------------------------------------------------------

    cpu.r[15] = 0x3000;

    cpu.Z = true;

    IRInstruction bne;

    bne.op =
        IROp::B;

    bne.condition =
        Condition::NE;

    bne.branchOffset =
        0x20;

    interpreter.execute(bne);

    TEST(
        "BNE does not execute when Z=1",
        cpu.r[15] == 0x3000
    );


    // ------------------------------------------------------------
    // BNE when Z=0
    // ------------------------------------------------------------

    cpu.r[15] = 0x3000;

    cpu.Z = false;

    interpreter.execute(bne);

    TEST(
        "BNE executes when Z=0",
        cpu.r[15] == 0x3020
    );


    // ------------------------------------------------------------
    // BL
    //
    // BL stores return address in LR.
    // ------------------------------------------------------------

    cpu.r[15] = 0x4000;

    IRInstruction bl;

    bl.op =
        IROp::BL;

    bl.condition =
        Condition::AL;

    bl.branchOffset =
        0x100;

    interpreter.execute(bl);

    TEST(
        "BL branches forward",
        cpu.r[15] == 0x4100
    );

    TEST(
        "BL stores return address in LR",
        cpu.r[14] == 0x3FFC
    );


    // ------------------------------------------------------------
    // BL backward
    // ------------------------------------------------------------

    cpu.r[15] = 0x5000;

    bl.branchOffset =
        static_cast<int32_t>(-0x100);

    interpreter.execute(bl);

    TEST(
        "BL branches backward",
        cpu.r[15] == 0x4F00
    );

    TEST(
        "BL updates LR",
        cpu.r[14] == 0x4FFC
    );


    // ------------------------------------------------------------
    // BX ARM
    // ------------------------------------------------------------

    cpu.r[15] = 0x6000;

    cpu.r[1] =
        0x7000;

    cpu.T = false;

    IRInstruction bx;

    bx.op =
        IROp::BX;

    bx.condition =
        Condition::AL;

    bx.rm =
        1;

    interpreter.execute(bx);

    TEST(
        "BX changes PC",
        cpu.r[15] == 0x7000
    );

    TEST(
        "BX stays in ARM state",
        cpu.T == false
    );


    // ------------------------------------------------------------
    // BX ARM -> Thumb
    // ------------------------------------------------------------

    cpu.r[1] =
        0x8001;

    cpu.T = false;

    interpreter.execute(bx);

    TEST(
        "BX switches to Thumb",
        cpu.T == true
    );

    TEST(
        "BX clears Thumb bit from PC",
        cpu.r[15] == 0x8000
    );


    // ------------------------------------------------------------
    // BX Thumb -> ARM
    // ------------------------------------------------------------

    cpu.r[1] =
        0x9000;

    cpu.T = true;

    interpreter.execute(bx);

    TEST(
        "BX switches to ARM",
        cpu.T == false
    );

    TEST(
        "BX loads aligned ARM address",
        cpu.r[15] == 0x9000
    );

    // ============================================================
    // FINAL
    // ============================================================

    printf("\n");
    printf("========================================\n");

    printf(
        "RESULT: %d/%d tests passed\n",
        passed,
        tests
    );

    printf("========================================\n");

    getchar();

    return passed == tests ? 0 : 1;
}