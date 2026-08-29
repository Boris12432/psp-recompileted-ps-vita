#include "thumb_decoder.h"


// ============================================================
// MOVS Rd, #imm8
//
// 00100 Rd Imm8
// ============================================================

IRInstruction ThumbDecoder::decodeMoveImmediate(
    uint16_t instruction
)
{
    IRInstruction ir;

    ir.op =
        IROp::MOV;

    ir.width =
        2;

    ir.rd =
        (instruction >> 8) & 0x7;

    ir.operand2.immediate =
        true;

    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags =
        true;

    return ir;
}


// ============================================================
// CMP Rn, #imm8
//
// 00101 Rn Imm8
// ============================================================

IRInstruction ThumbDecoder::decodeCompareImmediate(
    uint16_t instruction
)
{
    IRInstruction ir;

    ir.op =
        IROp::CMP;

    ir.width =
        2;

    ir.rn =
        (instruction >> 8) & 0x7;

    ir.operand2.immediate =
        true;

    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags =
        true;

    return ir;
}


// ============================================================
// ADDS Rd, #imm8
//
// 00110 Rd Imm8
// ============================================================

IRInstruction ThumbDecoder::decodeAddImmediate(
    uint16_t instruction
)
{
    IRInstruction ir;

    ir.op =
        IROp::ADD;

    ir.width =
        2;

    ir.rd =
        (instruction >> 8) & 0x7;

    ir.rn =
        ir.rd;

    ir.operand2.immediate =
        true;

    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags =
        true;

    return ir;
}


// ============================================================
// SUBS Rd, #imm8
//
// 00111 Rd Imm8
// ============================================================

IRInstruction ThumbDecoder::decodeSubImmediate(
    uint16_t instruction
)
{
    IRInstruction ir;

    ir.op =
        IROp::SUB;

    ir.width =
        2;

    ir.rd =
        (instruction >> 8) & 0x7;

    ir.rn =
        ir.rd;

    ir.operand2.immediate =
        true;

    ir.operand2.imm =
        instruction & 0xFF;

    ir.setFlags =
        true;

    return ir;
}


// ============================================================
// ALU operations
//
// 010000 Op Rm Rdn
// ============================================================

IRInstruction ThumbDecoder::decodeALU(
    uint16_t instruction
)
{
    IRInstruction ir;

    ir.width =
        2;

    uint32_t opcode =
        (instruction >> 6) & 0xF;

    uint32_t rm =
        (instruction >> 3) & 0x7;

    uint32_t rd =
        instruction & 0x7;

    ir.rn =
        rd;

    ir.rd =
        rd;

    ir.operand2.immediate =
        false;

    ir.operand2.rm =
        rm;

    ir.setFlags =
        true;


    switch (opcode)
    {
        case 0x0:
            ir.op = IROp::AND;
            break;

        case 0x1:
            ir.op = IROp::EOR;
            break;

        // ------------------------------------------------
        // LSL/LSR/ASR/ROR Rd,Rd,Rs
        //
        // IROp has no dedicated shift opcodes; on real ARM
        // (and in this decoder's own ARM-side convention)
        // a register-controlled shift is expressed as a
        // MOV whose Operand2 is "Rd shifted by Rs".
        // ------------------------------------------------

        case 0x2:
            ir.op = IROp::MOV;
            ir.operand2.rm = rd;
            ir.operand2.shift = ShiftType::LSL;
            ir.operand2.shiftImmediate = false;
            ir.operand2.shiftReg = rm;
            break;

        case 0x3:
            ir.op = IROp::MOV;
            ir.operand2.rm = rd;
            ir.operand2.shift = ShiftType::LSR;
            ir.operand2.shiftImmediate = false;
            ir.operand2.shiftReg = rm;
            break;

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

        case 0x7:
            ir.op = IROp::MOV;
            ir.operand2.rm = rd;
            ir.operand2.shift = ShiftType::ROR;
            ir.operand2.shiftImmediate = false;
            ir.operand2.shiftReg = rm;
            break;

        case 0x8:
            ir.op = IROp::TST;
            break;

        case 0x9:
            ir.op = IROp::RSB;
            break;

        case 0xA:
            ir.op = IROp::CMP;
            break;

        case 0xB:
            ir.op = IROp::CMN;
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
IRInstruction ThumbDecoder::decodeHighRegister(
    uint16_t instruction
)
{
    IRInstruction ir;

    ir.width =
        2;

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

    ir.rd =
        rd;

    ir.rn =
        rd;

    ir.operand2.immediate =
        false;

    ir.operand2.rm =
        rs;


    switch (opcode)
    {
        case 0:
            ir.op = IROp::ADD;
            break;

        case 1:
            ir.op = IROp::CMP;
            ir.rd = -1;
            ir.setFlags = true;
            break;

        case 2:
            ir.op = IROp::MOV;
            break;

        case 3:

            // BX
            ir.op =
                IROp::BX;

            ir.rm =
                rs;

            break;
    }

    return ir;
}
IRInstruction ThumbDecoder::decodeBranch(
    uint16_t instruction
)
{
    IRInstruction ir;

    ir.op =
        IROp::B;

    ir.width =
        2;

    uint32_t imm11 =
        instruction & 0x7FF;

    int32_t offset =
        static_cast<int32_t>(
            imm11 << 1
        );

    if (offset & 0x1000)
    {
        offset |=
            ~0x1FFF;
    }

    ir.branchOffset =
        offset;

    return ir;
}
IRInstruction ThumbDecoder::decode(
    uint16_t instruction
)
{
    // --------------------------------------------------------
    // MOVS Rd,#imm8
    // --------------------------------------------------------

    if (
        (instruction & 0xF800) ==
        0x2000
    )
    {
        return decodeMoveImmediate(
            instruction
        );
    }


    // --------------------------------------------------------
    // CMP Rn,#imm8
    // --------------------------------------------------------

    if (
        (instruction & 0xF800) ==
        0x2800
    )
    {
        return decodeCompareImmediate(
            instruction
        );
    }


    // --------------------------------------------------------
    // ADDS Rd,#imm8
    // --------------------------------------------------------

    if (
        (instruction & 0xF800) ==
        0x3000
    )
    {
        return decodeAddImmediate(
            instruction
        );
    }


    // --------------------------------------------------------
    // SUBS Rd,#imm8
    // --------------------------------------------------------

    if (
        (instruction & 0xF800) ==
        0x3800
    )
    {
        return decodeSubImmediate(
            instruction
        );
    }


    // --------------------------------------------------------
    // ALU
    // --------------------------------------------------------

    if (
        (instruction & 0xFC00) ==
        0x4000
    )
    {
        return decodeALU(
            instruction
        );
    }


    // --------------------------------------------------------
    // High register operations / BX
    // --------------------------------------------------------

    if (
        (instruction & 0xFC00) ==
        0x4400
    )
    {
        return decodeHighRegister(
            instruction
        );
    }


    // --------------------------------------------------------
    // Unconditional B
    // --------------------------------------------------------

    if (
        (instruction & 0xF800) ==
        0xE000
    )
    {
        return decodeBranch(
            instruction
        );
    }


    // --------------------------------------------------------
    // Unknown
    // --------------------------------------------------------

    IRInstruction ir;

    ir.op =
        IROp::NOP;

    ir.width =
        2;

    return ir;
}