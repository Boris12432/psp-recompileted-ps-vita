#include "thumb_decoder.h"


// ============================================================
// Проверка: является ли первый halfword префиксом Thumb-2
// ============================================================

bool ThumbDecoder::is32BitPrefix(uint16_t instruction)
{
    uint16_t top5 = instruction >> 11;

    return
        top5 == 0b11101 ||
        top5 == 0b11110 ||
        top5 == 0b11111;
}


// ============================================================
// MOVS Rd, #imm8
// ============================================================

IRInstruction ThumbDecoder::decodeMoveImmediate(
    uint16_t instruction
)
{
    IRInstruction ir{};

    ir.op = IROp::MOV;
    ir.width = 2;

    ir.rd =
        (instruction >> 8) & 0x7;

    ir.operand2.immediate = true;
    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags = true;

    return ir;
}


// ============================================================
// CMP Rn, #imm8
// ============================================================

IRInstruction ThumbDecoder::decodeCompareImmediate(
    uint16_t instruction
)
{
    IRInstruction ir{};

    ir.op = IROp::CMP;
    ir.width = 2;

    ir.rn =
        (instruction >> 8) & 0x7;

    ir.operand2.immediate = true;
    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags = true;

    return ir;
}


// ============================================================
// ADDS Rd, #imm8
// ============================================================

IRInstruction ThumbDecoder::decodeAddImmediate(
    uint16_t instruction
)
{
    IRInstruction ir{};

    ir.op = IROp::ADD;
    ir.width = 2;

    ir.rd =
        (instruction >> 8) & 0x7;

    ir.rn = ir.rd;

    ir.operand2.immediate = true;
    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags = true;

    return ir;
}


// ============================================================
// SUBS Rd, #imm8
// ============================================================

IRInstruction ThumbDecoder::decodeSubImmediate(
    uint16_t instruction
)
{
    IRInstruction ir{};

    ir.op = IROp::SUB;
    ir.width = 2;

    ir.rd =
        (instruction >> 8) & 0x7;

    ir.rn = ir.rd;

    ir.operand2.immediate = true;
    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags = true;

    return ir;
}


// ============================================================
// Thumb ALU
//
// 010000 Op Rm Rdn
// ============================================================

IRInstruction ThumbDecoder::decodeALU(
    uint16_t instruction
)
{
    IRInstruction ir{};

    ir.width = 2;

    uint32_t opcode =
        (instruction >> 6) & 0xF;

    uint32_t rm =
        (instruction >> 3) & 0x7;

    uint32_t rd =
        instruction & 0x7;

    ir.rn = rd;
    ir.rd = rd;

    ir.operand2.immediate = false;
    ir.operand2.rm = rm;

    ir.setFlags = true;

    switch (opcode)
    {
        case 0x0:
            ir.op = IROp::AND;
            break;

        case 0x1:
            ir.op = IROp::EOR;
            break;

        // LSL Rd, Rs
        case 0x2:
            ir.op = IROp::MOV;
            ir.operand2.rm = rd;
            ir.operand2.shift = ShiftType::LSL;
            ir.operand2.shiftImmediate = false;
            ir.operand2.shiftReg = rm;
            break;

        // LSR Rd, Rs
        case 0x3:
            ir.op = IROp::MOV;
            ir.operand2.rm = rd;
            ir.operand2.shift = ShiftType::LSR;
            ir.operand2.shiftImmediate = false;
            ir.operand2.shiftReg = rm;
            break;

        // ASR Rd, Rs
        case 0x4:
            ir.op = IROp::MOV;
            ir.operand2.rm = rd;
            ir.operand2.shift = ShiftType::ASR;
            ir.operand2.shiftImmediate = false;
            ir.operand2.shiftReg = rm;
            break;

        case 0x5:
            ir.op = IROp::ADC;
            break;

        case 0x6:
            ir.op = IROp::SBC;
            break;

        // ROR Rd, Rs
        case 0x7:
            ir.op = IROp::MOV;
            ir.operand2.rm = rd;
            ir.operand2.shift = ShiftType::ROR;
            ir.operand2.shiftImmediate = false;
            ir.operand2.shiftReg = rm;
            break;

        case 0x8:
            ir.op = IROp::TST;
            ir.rd = -1;
            break;

        case 0x9:
            ir.op = IROp::RSB;
            break;

        case 0xA:
            ir.op = IROp::CMP;
            ir.rd = -1;
            break;

        case 0xB:
            ir.op = IROp::CMN;
            ir.rd = -1;
            break;

        case 0xC:
            ir.op = IROp::ORR;
            break;

        case 0xD:
            ir.op = IROp::MUL;
            break;

        case 0xE:
            ir.op = IROp::BIC;
            break;

        case 0xF:
            ir.op = IROp::MVN;
            break;

        default:
            ir.op = IROp::NOP;
            break;
    }

    return ir;
}


// ============================================================
// High register operations / BX
// ============================================================

IRInstruction ThumbDecoder::decodeHighRegister(
    uint16_t instruction
)
{
    IRInstruction ir{};

    ir.width = 2;

    uint32_t opcode =
        (instruction >> 8) & 0x3;

    uint32_t h1 =
        (instruction >> 7) & 1;

    uint32_t h2 =
        (instruction >> 6) & 1;

    uint32_t rs =
        (instruction >> 3) & 0x7;

    uint32_t rd =
        instruction & 0x7;

    rd |= h1 << 3;
    rs |= h2 << 3;

    ir.rd = rd;
    ir.rn = rd;

    ir.operand2.immediate = false;
    ir.operand2.rm = rs;

    switch (opcode)
    {
        case 0:
            ir.op = IROp::ADD;
            ir.setFlags = false;
            break;

        case 1:
            ir.op = IROp::CMP;
            ir.rd = -1;
            ir.setFlags = true;
            break;

        case 2:
            ir.op = IROp::MOV;
            ir.setFlags = false;
            break;

        case 3:
            ir.op = IROp::BX;
            ir.rm = rs;
            ir.rd = -1;
            ir.rn = -1;
            ir.setFlags = false;
            break;

        default:
            ir.op = IROp::NOP;
            break;
    }

    return ir;
}


// ============================================================
// Thumb B
// ============================================================

IRInstruction ThumbDecoder::decodeBranch(
    uint16_t instruction
)
{
    IRInstruction ir{};

    ir.op = IROp::B;
    ir.width = 2;

    uint32_t imm11 =
        instruction & 0x7FF;

    int32_t offset =
        static_cast<int32_t>(imm11 << 1);

    // sign extend 12-bit offset
    if (offset & 0x800)
    {
        offset |= ~0xFFF;
    }

    ir.branchOffset = offset;

    return ir;
}


// ============================================================
// Thumb-2 decoder
// ============================================================

IRInstruction ThumbDecoder::decode(
    uint16_t instruction
)
{
    // --------------------------------------------------------
    // MOVS Rd,#imm8
    // --------------------------------------------------------

    if ((instruction & 0xF800) == 0x2000)
    {
        return decodeMoveImmediate(instruction);
    }


    // --------------------------------------------------------
    // CMP Rn,#imm8
    // --------------------------------------------------------

    if ((instruction & 0xF800) == 0x2800)
    {
        return decodeCompareImmediate(instruction);
    }


    // --------------------------------------------------------
    // ADDS Rd,#imm8
    // --------------------------------------------------------

    if ((instruction & 0xF800) == 0x3000)
    {
        return decodeAddImmediate(instruction);
    }


    // --------------------------------------------------------
    // SUBS Rd,#imm8
    // --------------------------------------------------------

    if ((instruction & 0xF800) == 0x3800)
    {
        return decodeSubImmediate(instruction);
    }


    // --------------------------------------------------------
    // ALU
    // --------------------------------------------------------

    if ((instruction & 0xFC00) == 0x4000)
    {
        return decodeALU(instruction);
    }


    // --------------------------------------------------------
    // High registers / BX
    // --------------------------------------------------------

    if ((instruction & 0xFC00) == 0x4400)
    {
        return decodeHighRegister(instruction);
    }


    // --------------------------------------------------------
    // Unconditional B
    // --------------------------------------------------------

    if ((instruction & 0xF800) == 0xE000)
    {
        return decodeBranch(instruction);
    }


    // --------------------------------------------------------
    // Unknown
    // --------------------------------------------------------

    IRInstruction ir{};

    ir.op = IROp::NOP;
    ir.width = 2;

    return ir;
}


// ============================================================
// Thumb-2 32-bit instruction
// ============================================================

IRInstruction ThumbDecoder::decode32(
    uint16_t first,
    uint16_t second
)
{
    IRInstruction ir{};

    ir.width = 4;

    /*
     * Thumb-2 instruction:
     *
     * first  = first halfword
     * second = second halfword
     *
     * В памяти Thumb инструкции идут:
     *
     *   low halfword
     *   high halfword
     *
     * ARM decoder получает их уже как first/second.
     */


    // --------------------------------------------------------
    // Thumb-2 B.W
    // --------------------------------------------------------

    if ((first & 0xF800) == 0xF000 ||
        (first & 0xF800) == 0xF800)
    {
        return decode32Branch(
            first,
            second
        );
    }


    // --------------------------------------------------------
    // Thumb-2 data processing
    // --------------------------------------------------------

    if ((first & 0xFE00) == 0xEA00)
    {
        return decode32DataProcessing(
            first,
            second
        );
    }


    // --------------------------------------------------------
    // Unknown Thumb-2
    // --------------------------------------------------------

    ir.op = IROp::NOP;

    return ir;
}


// ============================================================
// Thumb-2 data processing register
// ============================================================

IRInstruction ThumbDecoder::decode32DataProcessing(
    uint16_t first,
    uint16_t second
)
{
    IRInstruction ir{};

    ir.width = 4;

    uint32_t op =
        (first >> 5) & 0xF;

    uint32_t rn =
        first & 0xF;

    uint32_t rd =
        (second >> 8) & 0xF;

    uint32_t rm =
        second & 0xF;

    ir.rn = rn;
    ir.rd = rd;

    ir.operand2.immediate = false;
    ir.operand2.rm = rm;

    ir.setFlags = false;

    switch (op)
    {
        case 0x0:
            ir.op = IROp::AND;
            break;

        case 0x1:
            ir.op = IROp::BIC;
            break;

        case 0x2:
            ir.op = IROp::ORR;
            break;

        case 0x4:
            ir.op = IROp::EOR;
            break;

        case 0x8:
            ir.op = IROp::ADD;
            break;

        case 0xD:
            ir.op = IROp::SUB;
            break;

        default:
            ir.op = IROp::NOP;
            break;
    }

    return ir;
}


// ============================================================
// Thumb-2 B.W
// ============================================================

IRInstruction ThumbDecoder::decode32Branch(
    uint16_t first,
    uint16_t second
)
{
    IRInstruction ir{};

    ir.width = 4;
    ir.op = IROp::B;

    /*
     * Thumb-2 B.W:
     *
     * S:J1:J2:imm10:imm11
     */

    uint32_t S =
        (first >> 10) & 1;

    uint32_t imm10 =
        first & 0x03FF;

    uint32_t J1 =
        (second >> 13) & 1;

    uint32_t J2 =
        (second >> 11) & 1;

    uint32_t imm11 =
        second & 0x07FF;

    uint32_t I1 =
        ~(J1 ^ S) & 1;

    uint32_t I2 =
        ~(J2 ^ S) & 1;

    uint32_t value =
        (S << 24) |
        (I1 << 23) |
        (I2 << 22) |
        (imm10 << 12) |
        (imm11 << 1);

    int32_t offset =
        static_cast<int32_t>(value);

    if (S)
    {
        offset |=
            static_cast<int32_t>(
                0xFE000000u
            );
    }

    ir.branchOffset = offset;

    return ir;
}