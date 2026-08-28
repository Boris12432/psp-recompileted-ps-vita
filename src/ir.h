#pragma once

#include <cstdint>


enum class IROp
{
    INVALID,

    AND,
    EOR,
    SUB,
    RSB,
    ADD,
    ADC,
    SBC,
    RSC,
    
    MUL,
    MLA,
    UMULL,
    SMULL,

    TST,
    TEQ,
    CMP,
    CMN,

    ORR,
    MOV,
    BIC,
    MVN,

    LDR,
    STR,
    LDM,
    STM,

    B,
    BL,
    BX,

    NOP
};


enum class ShiftType
{
    LSL,
    LSR,
    ASR,
    ROR,
    RRX
};


enum class Condition
{
    EQ = 0,
    NE = 1,

    CS = 2,
    HS = 2,

    CC = 3,
    LO = 3,

    MI = 4,
    PL = 5,

    VS = 6,
    VC = 7,

    HI = 8,
    LS = 9,

    GE = 10,
    LT = 11,

    GT = 12,
    LE = 13,

    AL = 14,
    NV = 15
};


enum class MemorySize
{
    Byte,
    Halfword,
    Word
};


struct Operand2
{
    // Immediate operand
    bool immediate = false;

    uint32_t imm = 0;


    // Register operand
    int rm = -1;


    // Register shift
    ShiftType shift = ShiftType::LSL;

    bool shiftImmediate = true;

    uint8_t shiftAmount = 0;

    int shiftReg = -1;
};


struct IRInstruction
{
    IROp op = IROp::NOP;

    Condition condition = Condition::AL;


    // Registers
    int rd = -1;
    int rs = -1;
    int rn = -1;
    int rm = -1;


    // ARM Operand2
    Operand2 operand2{};


    int32_t branchOffset = 0;

    uint16_t registerList = 0;


    // CPSR
    bool setFlags = false;


    // Memory addressing

    // true:
    //     [Rn,#offset]
    //
    // false:
    //     [Rn],#offset
    bool preIndex = true;


    // true:
    //     +offset
    //
    // false:
    //     -offset
    bool up = true;


    // Write-back:
    //
    // [Rn,#offset]!
    bool writeBack = false;

    // Register list for LDM / STM
   

    // true  = load
    // false = store
    bool load = false;


    // Memory width
    MemorySize memorySize =
        MemorySize::Word;


    // Signed load
    //
    // LDR   -> false
    // LDRB  -> false
    // LDRH  -> false
    // LDRSB -> true
    // LDRSH -> true
    bool signExtend = false;
    
    uint32_t address = 0;

    uint8_t width = 4;
};