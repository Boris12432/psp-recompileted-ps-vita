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

        case 0x2:
            ir.op = IROp::LSL;
            break;

        case 0x3:
            ir.op = IROp::LSR;
            break;

        case 0x4:
            ir.op = IROp::ASR;
            break;

        case 0x5:
            ir.op = IROp::ADC;
            break;

        case 0x6:
            ir.op = IROp::SBC;
            break;

        case 0x7:
            ir.op = IROp::ROR;
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