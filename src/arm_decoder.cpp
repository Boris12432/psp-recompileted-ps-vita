#include "arm_decoder.h"

#include <cstdint>


// ============================================================
// Utility
// ============================================================

static uint32_t ror32(
    uint32_t value,
    unsigned amount
)
{
    amount &= 31;

    if (amount == 0)
        return value;

    return
        (value >> amount) |
        (value << (32 - amount));
}


// ============================================================
// Condition
// ============================================================

Condition ARMDecoder::condition(
    uint32_t instruction
)
{
    return static_cast<Condition>(
        (instruction >> 28) & 0xF
    );
}


// ============================================================
// Instruction classification
// ============================================================

bool ARMDecoder::isBX(
    uint32_t instruction
)
{
    /*
     * BX:
     *
     * cond 0001 0010 1111 1111 1111 0001 Rm
     *
     * Ignore condition field.
     */

    return
        (instruction & 0x0FFFFFF0u)
        == 0x012FFF10u;
}


bool ARMDecoder::isBranch(
    uint32_t instruction
)
{
    /*
     * ARM B / BL:
     *
     * bits 27:25 = 101
     */

    return
        ((instruction >> 25) & 0x7)
        == 0b101;
}


bool ARMDecoder::isDataProcessing(
    uint32_t instruction
)
{
    /*
     * Data processing:
     *
     * bits 27:26 = 00
     */

    return
        ((instruction >> 26) & 0x3)
        == 0b00;
}


bool ARMDecoder::isLoadStore(
    uint32_t instruction
)
{
    /*
     * Single data transfer:
     *
     * LDR / STR / LDRB / STRB
     *
     * bits 27:26 = 01
     */

    return
        ((instruction >> 26) & 0x3)
        == 0b01;
}

bool ARMDecoder::isBlockTransfer(
    uint32_t instruction
)
{
    /*
     * ARM LDM / STM:
     *
     * bits 27:25 = 100
     */

    return
        ((instruction >> 25) & 0x7) == 0b100;
}

bool ARMDecoder::isHalfwordTransfer(
    uint32_t instruction
)
{
    /*
     * ARM halfword / signed transfer:
     *
     * bits 27:25 = 000
     * bits 7:4   = 1001
     *
     * This covers:
     *
     * LDRH
     * STRH
     * LDRSB
     * LDRSH
     */

    uint32_t bits27_25 =
        (instruction >> 25) & 0x7;

    uint32_t bits7_4 =
        (instruction >> 4) & 0xF;


    if (bits27_25 != 0)
        return false;


    if (bits7_4 != 0x9)
        return false;


    return true;
}


// ============================================================
// Operand2
// ============================================================

Operand2 ARMDecoder::decodeOperand2(
    uint32_t instruction
)
{
    Operand2 op;


    /*
     * I bit:
     *
     * 1 = immediate
     * 0 = register
     */

    bool I =
        (instruction >> 25) & 1;


    // --------------------------------------------------------
    // Immediate
    // --------------------------------------------------------

    if (I)
    {
        op.immediate = true;


        uint32_t imm8 =
            instruction & 0xFF;


        uint32_t rotate =
            ((instruction >> 8) & 0xF) * 2;


        op.imm =
            ror32(
                imm8,
                rotate
            );


        return op;
    }


    // --------------------------------------------------------
    // Register
    // --------------------------------------------------------

    op.immediate = false;


    op.rm =
        instruction & 0xF;


    bool registerShift =
        (instruction >> 4) & 1;


    uint32_t shiftType =
        (instruction >> 5) & 0x3;


    switch (shiftType)
    {
        case 0:

            op.shift =
                ShiftType::LSL;

            break;


        case 1:

            op.shift =
                ShiftType::LSR;

            break;


        case 2:

            op.shift =
                ShiftType::ASR;

            break;


        case 3:

            /*
             * ROR #0 is RRX.
             *
             * For register shifts ROR Rm,
             * amount comes from Rs, so it remains ROR.
             */

            op.shift =
                ShiftType::ROR;

            break;
    }


    // --------------------------------------------------------
    // Register-specified shift
    // --------------------------------------------------------

    if (registerShift)
    {
        op.shiftImmediate = false;


        op.shiftReg =
            (instruction >> 8) & 0xF;


        return op;
    }


    // --------------------------------------------------------
    // Immediate shift
    // --------------------------------------------------------

    op.shiftImmediate = true;


    op.shiftAmount =
        (instruction >> 7) & 0x1F;


    /*
     * ROR #0 means RRX.
     */

    if (
        op.shift == ShiftType::ROR &&
        op.shiftAmount == 0
    )
    {
        op.shift =
            ShiftType::RRX;
    }


    return op;
}

// ============================================================
// Multiply
// ============================================================

IRInstruction ARMDecoder::decodeMultiply(
    uint32_t instruction
)
{
    IRInstruction ir;

    ir.condition =
        condition(instruction);

    bool A =
        (instruction >> 21) & 1;

    bool S =
        (instruction >> 20) & 1;

    bool U =
        (instruction >> 22) & 1;

    bool longMultiply =
        (instruction >> 23) & 1;

    ir.setFlags =
        S;

    /*
     * Register layout:
     *
     * Rd = bits 19:16
     * Rn = bits 15:12
     * Rs = bits 11:8
     * Rm = bits 3:0
     */

    ir.rd =
        (instruction >> 16) & 0xF;

    ir.rn =
        (instruction >> 12) & 0xF;

    ir.rs =
        (instruction >> 8) & 0xF;

    ir.rm =
        instruction & 0xF;

    // --------------------------------------------------------
    // MUL / MLA
    // --------------------------------------------------------

    if (!longMultiply)
    {
        if (A)
        {
            ir.op =
                IROp::MLA;
        }
        else
        {
            ir.op =
                IROp::MUL;
        }

        return ir;
    }

    // --------------------------------------------------------
    // UMULL / SMULL
    // --------------------------------------------------------

    if (U)
    {
        ir.op =
            IROp::SMULL;
    }
    else
    {
        ir.op =
            IROp::UMULL;
    }

    return ir;
}

// ============================================================
// BX
// ============================================================

bool ARMDecoder::isMultiply(
    uint32_t instruction
)
{
    /*
     * ARM multiply family:
     *
     * 000000xx xxxx xxxx xxxx 1001 xxxx
     *
     * Bits 7:4 = 1001
     * Bits 27:22 = 000000
     *
     * This covers:
     *
     * MUL
     * MLA
     * UMULL
     * SMULL
     */

    if (((instruction >> 22) & 0x3F) != 0)
        return false;

    if (((instruction >> 4) & 0xF) != 0x9)
        return false;

    return true;
}

IRInstruction ARMDecoder::decodeBX(
    uint32_t instruction
)
{
    IRInstruction ir;


    ir.op =
        IROp::BX;


    ir.condition =
        condition(instruction);


    ir.rm =
        instruction & 0xF;


    return ir;
}

// ============================================================
// LDM / STM
// ============================================================

IRInstruction ARMDecoder::decodeBlockTransfer(
    uint32_t instruction
)
{
    IRInstruction ir;

    ir.condition =
        condition(instruction);

    bool P =
        (instruction >> 24) & 1;

    bool U =
        (instruction >> 23) & 1;

    bool S =
        (instruction >> 22) & 1;

    bool W =
        (instruction >> 21) & 1;

    bool L =
        (instruction >> 20) & 1;


    ir.rn =
        (instruction >> 16) & 0xF;

    ir.preIndex =
        P;

    ir.up =
        U;

    ir.writeBack =
        W;

    ir.load =
        L;

    ir.setFlags =
        S;

    ir.registerList =
        static_cast<uint16_t>(
            instruction & 0xFFFF
        );


    ir.op =
        L
            ? IROp::LDM
            : IROp::STM;


    return ir;
}

// ============================================================
// Branch
// ============================================================

IRInstruction ARMDecoder::decodeBranch(
    uint32_t instruction
)
{
    IRInstruction ir;


    /*
     * B / BL
     */

    bool link =
        (instruction >> 24) & 1;


    ir.op =
        link
            ? IROp::BL
            : IROp::B;


    ir.condition =
        condition(instruction);


    /*
     * 24-bit signed offset.
     *
     * ARM branch:
     *
     * offset = sign_extend(imm24 << 2)
     */

    uint32_t imm24 =
        instruction & 0x00FFFFFF;


    int32_t offset =
        static_cast<int32_t>(
            imm24 << 2
        );


    /*
     * Sign extend from 26 bits.
     */

    if (offset & 0x02000000)
    {
        offset |=
            static_cast<int32_t>(
                0xFC000000
            );
    }


    ir.branchOffset =
        offset;


    return ir;
}


// ============================================================
// Data Processing
// ============================================================

IRInstruction ARMDecoder::decodeDataProcessing(
    uint32_t instruction
)
{
    IRInstruction ir;


    ir.condition =
        condition(instruction);


    uint32_t opcode =
        (instruction >> 21) & 0xF;


    bool S =
        (instruction >> 20) & 1;


    ir.rn =
        (instruction >> 16) & 0xF;


    ir.rd =
        (instruction >> 12) & 0xF;


    ir.setFlags =
        S;


    /*
     * Decode Operand2.
     */

    ir.operand2 =
        decodeOperand2(
            instruction
        );


    /*
     * Opcode mapping:
     *
     * 0000 AND
     * 0001 EOR
     * 0010 SUB
     * 0011 RSB
     * 0100 ADD
     * 0101 ADC
     * 0110 SBC
     * 0111 RSC
     * 1000 TST
     * 1001 TEQ
     * 1010 CMP
     * 1011 CMN
     * 1100 ORR
     * 1101 MOV
     * 1110 BIC
     * 1111 MVN
     */


    switch (opcode)
    {
        case 0x0:

            ir.op =
                IROp::AND;

            break;


        case 0x1:

            ir.op =
                IROp::EOR;

            break;


        case 0x2:

            ir.op =
                IROp::SUB;

            break;


        case 0x3:

            /*
             * RSB is not currently present
             * in our IR.
             *
             * Leave unsupported for now.
             */

            ir.op =
                IROp::RSB;

            break;


        case 0x4:

            ir.op =
                IROp::ADD;

            break;


        case 0x5:

            ir.op =
                IROp::ADC;

            break;


        case 0x6:

            ir.op =
                IROp::SBC;

            break;


        case 0x7:

            /*
             * RSC not currently present.
             */

            ir.op =
                IROp::RSC;

            break;


        case 0x8:

            ir.op =
                IROp::TST;

            ir.setFlags =
                true;

            break;


        case 0x9:

            ir.op =
                IROp::TEQ;

            ir.setFlags =
                true;

            break;


        case 0xA:

            ir.op =
                IROp::CMP;

            ir.setFlags =
                true;

            break;


        case 0xB:

            ir.op =
                IROp::CMN;

            ir.setFlags =
                true;

            break;


        case 0xC:

            ir.op =
                IROp::ORR;

            break;


        case 0xD:

            ir.op =
                IROp::MOV;

            break;


        case 0xE:

            ir.op =
                IROp::BIC;

            break;


        case 0xF:

            ir.op =
                IROp::MVN;

            break;


        default:

            ir.op =
                IROp::NOP;

            break;
    }


    return ir;
}


// ============================================================
// LDR / STR / LDRB / STRB
// ============================================================

IRInstruction ARMDecoder::decodeLoadStore(
    uint32_t instruction
)
{
    IRInstruction ir;


    ir.condition =
        condition(instruction);


    bool I =
        (instruction >> 25) & 1;


    bool P =
        (instruction >> 24) & 1;


    bool U =
        (instruction >> 23) & 1;


    bool B =
        (instruction >> 22) & 1;


    bool W =
        (instruction >> 21) & 1;


    bool L =
        (instruction >> 20) & 1;


    ir.rn =
        (instruction >> 16) & 0xF;


    ir.rd =
        (instruction >> 12) & 0xF;


    ir.preIndex =
        P;


    ir.up =
        U;


    ir.writeBack =
        W;


    ir.load =
        L;


    ir.op =
        L
            ? IROp::LDR
            : IROp::STR;


    /*
     * B = 1:
     *     byte
     *
     * B = 0:
     *     word
     */

    ir.memorySize =
        B
            ? MemorySize::Byte
            : MemorySize::Word;


    ir.signExtend =
        false;


    // --------------------------------------------------------
    // Immediate offset
    // --------------------------------------------------------

    if (!I)
    {
        ir.operand2.immediate =
            true;


        ir.operand2.imm =
            instruction & 0xFFF;


        return ir;
    }


    // --------------------------------------------------------
    // Register offset
    // --------------------------------------------------------

    ir.operand2.immediate =
        false;


    ir.operand2.rm =
        instruction & 0xF;


    uint32_t shiftType =
        (instruction >> 5) & 0x3;


    uint32_t shiftAmount =
        (instruction >> 7) & 0x1F;


    switch (shiftType)
    {
        case 0:

            ir.operand2.shift =
                ShiftType::LSL;

            break;


        case 1:

            ir.operand2.shift =
                ShiftType::LSR;

            break;


        case 2:

            ir.operand2.shift =
                ShiftType::ASR;

            break;


        case 3:

            ir.operand2.shift =
                ShiftType::ROR;

            break;
    }


    ir.operand2.shiftImmediate =
        true;


    ir.operand2.shiftAmount =
        static_cast<uint8_t>(
            shiftAmount
        );


    /*
     * For memory register offsets,
     * ROR #0 is also RRX architecturally.
     */

    if (
        ir.operand2.shift == ShiftType::ROR &&
        ir.operand2.shiftAmount == 0
    )
    {
        ir.operand2.shift =
            ShiftType::RRX;
    }


    return ir;
}


// ============================================================
// LDRH / STRH / LDRSB / LDRSH
// ============================================================

IRInstruction ARMDecoder::decodeHalfwordTransfer(
    uint32_t instruction
)
{
    IRInstruction ir;


    ir.condition =
        condition(instruction);


    bool P =
        (instruction >> 24) & 1;


    bool U =
        (instruction >> 23) & 1;


    bool I =
        (instruction >> 22) & 1;


    bool W =
        (instruction >> 21) & 1;


    bool L =
        (instruction >> 20) & 1;


    uint32_t type =
        (instruction >> 5) & 0x3;


    ir.rn =
        (instruction >> 16) & 0xF;


    ir.rd =
        (instruction >> 12) & 0xF;


    ir.preIndex =
        P;


    ir.up =
        U;


    ir.writeBack =
        W;


    ir.load =
        L;


    /*
     * type:
     *
     * 01 = Halfword
     * 10 = Signed byte
     * 11 = Signed halfword
     */

    switch (type)
    {
        // ----------------------------------------------------
        // H = 01
        // ----------------------------------------------------

        case 1:

            ir.op =
                L
                    ? IROp::LDR
                    : IROp::STR;


            ir.memorySize =
                MemorySize::Halfword;


            ir.signExtend =
                false;


            break;


        // ----------------------------------------------------
        // SB = 10
        // ----------------------------------------------------

        case 2:

            /*
             * LDRSB only.
             *
             * There is no STRSB.
             */

            if (!L)
            {
                ir.op =
                    IROp::NOP;

                return ir;
            }


            ir.op =
                IROp::LDR;


            ir.memorySize =
                MemorySize::Byte;


            ir.signExtend =
                true;


            break;


        // ----------------------------------------------------
        // SH = 11
        // ----------------------------------------------------

        case 3:

            /*
             * LDRSH only.
             */

            if (!L)
            {
                ir.op =
                    IROp::NOP;

                return ir;
            }


            ir.op =
                IROp::LDR;


            ir.memorySize =
                MemorySize::Halfword;


            ir.signExtend =
                true;


            break;


        default:

            ir.op =
                IROp::NOP;

            return ir;
    }


    // --------------------------------------------------------
    // Offset
    // --------------------------------------------------------

    if (I)
    {
        /*
         * Immediate 8-bit offset:
         *
         * imm4H : imm4L
         */

        uint32_t high =
            (instruction >> 8) & 0xF;


        uint32_t low =
            instruction & 0xF;


        ir.operand2.immediate =
            true;


        ir.operand2.imm =
            (high << 4) | low;
    }
    else
    {
        /*
         * Register offset
         */

        ir.operand2.immediate =
            false;


        ir.operand2.rm =
            instruction & 0xF;
    }


    return ir;
}


// ============================================================
// Main decoder
// ============================================================

IRInstruction ARMDecoder::decode(
    uint32_t instruction
)
{
    if (instruction == 0)
    {
        IRInstruction ir;

        ir.op = IROp::INVALID;

        return ir;
    }

    // --------------------------------------------------------
    // BX
    // --------------------------------------------------------

    if (isBX(instruction))
    {
        return decodeBX(
            instruction
        );
    }

    if (isMultiply(instruction))
    {
        return decodeMultiply(
            instruction
        );
    }

    // --------------------------------------------------------
    // B / BL
    // --------------------------------------------------------

    if (isBranch(instruction))
    {
        return decodeBranch(
            instruction
        );
    }

    // --------------------------------------------------------
    // LDM / STM
    // --------------------------------------------------------

    if (isBlockTransfer(instruction))
    {
        return decodeBlockTransfer(
            instruction
        );
    }

    // --------------------------------------------------------
    // Halfword / signed transfer
    // --------------------------------------------------------

    if (isHalfwordTransfer(instruction))
    {
        return decodeHalfwordTransfer(
            instruction
        );
    }


    // --------------------------------------------------------
    // LDR / STR / LDRB / STRB
    // --------------------------------------------------------

    if (isLoadStore(instruction))
    {
        return decodeLoadStore(
            instruction
        );
    }


    // --------------------------------------------------------
    // Data processing
    // --------------------------------------------------------

    if (isDataProcessing(instruction))
    {
        return decodeDataProcessing(
            instruction
        );
    }


    // --------------------------------------------------------
    // Unknown
    // --------------------------------------------------------

    IRInstruction ir;

    ir.op =
        IROp::NOP;


    return ir;
}