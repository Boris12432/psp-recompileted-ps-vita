#include "mips_backend.h"

#include <cstddef>


// ============================================================
// Constructor
// ============================================================

MIPSBackend::MIPSBackend()
{
    clear();
}


// ============================================================
// ARM register -> MIPS register
// ============================================================
//
// Пока используем прямое отображение:
//
// ARM R0  -> MIPS $s0
// ARM R1  -> MIPS $s1
// ...
// ARM R7  -> MIPS $s7
//
// Для R8-R15 пока используем временный диапазон.
//
// MIPS:
// $s0-$s7 = 16-23
// $t0-$t7 = 8-15
//
// ============================================================

int MIPSBackend::armReg(
    int arm
)
{
    if (arm < 0 || arm > 15)
        return -1;

    if (arm < 8)
        return 16 + arm;

    return 8 + (arm - 8);
}


// ============================================================
// Clear
// ============================================================

void MIPSBackend::clear()
{
    mips.code.clear();
    code.clear();
}


// ============================================================
// Translate one IR instruction
// ============================================================

bool MIPSBackend::translate(
    const IRInstruction& ir
)
{
    switch (ir.op)
    {
        case IROp::MOV:
        case IROp::AND:
        case IROp::EOR:
        case IROp::ORR:
        case IROp::BIC:
        case IROp::MVN:
        case IROp::ADD:
        case IROp::ADC:
        case IROp::SUB:
        case IROp::RSB:
        case IROp::SBC:
        case IROp::MUL:
            return translateDataProcessing(ir);

        case IROp::LDR:
        case IROp::STR:
            return translateMemory(ir);

        case IROp::B:
        case IROp::BL:
        case IROp::BX:
            return translateBranch(ir);

        default:
            return false;
    }
}


// ============================================================
// Translate vector
// ============================================================

bool MIPSBackend::translate(
    const std::vector<IRInstruction>& instructions
)
{
    clear();

    for (const auto& ir : instructions)
    {
        if (!translate(ir))
            return false;
    }

    code = mips.code;

    return true;
}


// ============================================================
// Data processing
// ============================================================

bool MIPSBackend::translateDataProcessing(
    const IRInstruction& ir
)
{
    int rd = armReg(ir.rd);

    if (rd < 0)
        return false;


    // --------------------------------------------------------
    // MOV
    // --------------------------------------------------------

    if (ir.op == IROp::MOV)
    {
        if (ir.operand2.immediate)
        {
            uint32_t imm =
                ir.operand2.imm;

            if (imm <= 0xFFFF)
            {
                mips.ori(
                    rd,
                    0,
                    static_cast<uint16_t>(imm)
                );
            }
            else
            {
                uint16_t hi =
                    static_cast<uint16_t>(
                        imm >> 16
                    );

                uint16_t lo =
                    static_cast<uint16_t>(
                        imm & 0xFFFF
                    );

                mips.lui(
                    rd,
                    hi
                );

                if (lo != 0)
                {
                    mips.ori(
                        rd,
                        rd,
                        lo
                    );
                }
            }

            return true;
        }

        int rm =
            armReg(
                ir.operand2.rm
            );

        if (rm < 0)
            return false;

        mips.addu(
            rd,
            rm,
            0
        );

        return true;
    }


    // --------------------------------------------------------
    // Register operand
    // --------------------------------------------------------

    if (!ir.operand2.immediate)
    {
        int rn =
            armReg(ir.rn);

        int rm =
            armReg(
                ir.operand2.rm
            );

        if (rn < 0 || rm < 0)
            return false;


        switch (ir.op)
        {
            case IROp::ADD:
                mips.addu(
                    rd,
                    rn,
                    rm
                );
                return true;

            case IROp::SUB:
                mips.subu(
                    rd,
                    rn,
                    rm
                );
                return true;

            case IROp::AND:
                mips.and_(
                    rd,
                    rn,
                    rm
                );
                return true;

            case IROp::ORR:
                mips.or_(
                    rd,
                    rn,
                    rm
                );
                return true;

            case IROp::EOR:
                mips.xor_(
                    rd,
                    rn,
                    rm
                );
                return true;

            case IROp::RSB:
                mips.subu(
                    rd,
                    rm,
                    rn
                );
                return true;

            case IROp::BIC:
                mips.and_(
                    rd,
                    rn,
                    rm
                );

                return true;

            default:
                return false;
        }
    }


    // --------------------------------------------------------
    // Immediate arithmetic
    // --------------------------------------------------------

    int rn =
        armReg(ir.rn);

    if (rn < 0)
        return false;

    uint32_t imm =
        ir.operand2.imm;


    if (imm > 0xFFFF)
        return false;


    switch (ir.op)
    {
        case IROp::ADD:

            mips.addiu(
                rd,
                rn,
                static_cast<int16_t>(imm)
            );

            return true;


        case IROp::SUB:

            mips.addiu(
                rd,
                rn,
                static_cast<int16_t>(
                    -static_cast<int32_t>(imm)
                )
            );

            return true;


        case IROp::AND:

            mips.andi(
                rd,
                rn,
                static_cast<uint16_t>(imm)
            );

            return true;


        case IROp::ORR:

            mips.ori(
                rd,
                rn,
                static_cast<uint16_t>(imm)
            );

            return true;


        case IROp::EOR:

            mips.xori(
                rd,
                rn,
                static_cast<uint16_t>(imm)
            );

            return true;


        default:
            return false;
    }
}


// ============================================================
// Memory
// ============================================================

bool MIPSBackend::translateMemory(
    const IRInstruction& ir
)
{
    int rt =
        armReg(ir.rd);

    int base =
        armReg(ir.rn);

    if (rt < 0 || base < 0)
        return false;


    int32_t offset = 0;


    if (ir.operand2.immediate)
    {
        offset =
            static_cast<int32_t>(
                ir.operand2.imm
            );

        if (!ir.up)
            offset = -offset;
    }


    if (offset < -32768 ||
        offset > 32767)
    {
        return false;
    }


    if (ir.op == IROp::LDR)
    {
        mips.lw(
            rt,
            base,
            static_cast<int16_t>(offset)
        );

        return true;
    }


    if (ir.op == IROp::STR)
    {
        mips.sw(
            rt,
            base,
            static_cast<int16_t>(offset)
        );

        return true;
    }


    return false;
}


// ============================================================
// Branch
// ============================================================

bool MIPSBackend::translateBranch(
    const IRInstruction& ir
)
{
    if (ir.op == IROp::BX)
    {
        int rs =
            armReg(ir.rm);

        if (rs < 0)
            return false;

        mips.jr(rs);

        return true;
    }


    // Пока branchOffset оставляем как
    // относительный байтовый offset.
    //
    // Полноценный branch linker появится позже.

    if (ir.op == IROp::B)
    {
        mips.branch(
            ir.branchOffset
        );

        return true;
    }


    if (ir.op == IROp::BL)
    {
        mips.branchAndLink(
            ir.branchOffset
        );

        return true;
    }


    return false;
}


// ============================================================
// Shift
// ============================================================

bool MIPSBackend::translateShift(
    const IRInstruction& ir
)
{
    int rd =
        armReg(ir.rd);

    int rm =
        armReg(ir.operand2.rm);

    if (rd < 0 || rm < 0)
        return false;


    if (ir.operand2.shiftImmediate)
    {
        uint8_t amount =
            ir.operand2.shiftAmount;


        switch (ir.operand2.shift)
        {
            case ShiftType::LSL:
                mips.sll(
                    rd,
                    rm,
                    amount
                );
                return true;

            case ShiftType::LSR:
                mips.srl(
                    rd,
                    rm,
                    amount
                );
                return true;

            case ShiftType::ASR:
                mips.sra(
                    rd,
                    rm,
                    amount
                );
                return true;

            default:
                return false;
        }
    }


    int rs =
        armReg(
            ir.operand2.shiftReg
        );

    if (rs < 0)
        return false;


    switch (ir.operand2.shift)
    {
        case ShiftType::LSL:
            mips.sllv(
                rd,
                rm,
                rs
            );
            return true;

        case ShiftType::LSR:
            mips.srlv(
                rd,
                rm,
                rs
            );
            return true;

        case ShiftType::ASR:
            mips.srav(
                rd,
                rm,
                rs
            );
            return true;

        default:
            return false;
    }
}