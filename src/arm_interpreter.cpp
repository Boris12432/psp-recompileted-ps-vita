#include "arm_interpreter.h"

#include "arm_condition.h"
#include "arm_shifter.h"
#include "arm_alu.h"


// ============================================================
// Operand2
// ============================================================

uint32_t ARMInterpreter::operand2(
    const Operand2& op,
    bool* carryOut)
{
    // --------------------------------------------------------
    // Immediate operand
    // --------------------------------------------------------

    if (op.immediate) {

        // For a plain immediate operand the shifter carry
        // remains unchanged.
        *carryOut = cpu.C;

        return op.imm;
    }


    // --------------------------------------------------------
    // Register operand
    // --------------------------------------------------------

    uint32_t value =
        cpu.r[op.rm];

    uint32_t amount;


    // --------------------------------------------------------
    // Shift amount
    // --------------------------------------------------------

    if (op.shiftImmediate) {

        amount =
            op.shiftAmount;

    }
    else {

        // ARM register-controlled shifts use
        // the lowest 8 bits of Rs.
        amount =
            cpu.r[op.shiftReg] & 0xFFu;
    }


    // --------------------------------------------------------
    // Perform shift
    // --------------------------------------------------------

    ShiftResult shifted =
        shiftValue(
            value,
            op.shift,
            amount,
            cpu.C
        );


    *carryOut =
        shifted.carry;

    return shifted.value;
}


// ============================================================
// Arithmetic flags
// ============================================================

void ARMInterpreter::setArithmeticFlags(
    uint32_t result,
    bool carry,
    bool overflow)
{
    cpu.N =
        (result & 0x80000000u) != 0;

    cpu.Z =
        (result == 0);

    cpu.C =
        carry;

    cpu.V =
        overflow;
}


// ============================================================
// Execute
// ============================================================

bool ARMInterpreter::execute(
    const IRInstruction& ir)
{
    // --------------------------------------------------------
    // Condition check
    // --------------------------------------------------------

    if (!conditionPassed(
            ir.condition,
            cpu))
    {
        return false;
    }


    // --------------------------------------------------------
    // IMPORTANT:
    //
    // Do NOT calculate operand2 here globally.
    //
    // Branches, BX, memory instructions, etc. do not
    // necessarily contain a valid Operand2.
    //
    // Calculating it globally can access cpu.r[garbage]
    // and cause an array bounds assertion.
    // --------------------------------------------------------

    bool shifterCarry =
        cpu.C;


    switch (ir.op) {


        // ====================================================
        // MOV
        // ====================================================
    

        case IROp::MOV: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result =
                op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {

                cpu.setNZ(result);

                cpu.C =
                    shifterCarry;
            }

            return false;
        }


        // ====================================================
        // MVN
        // ====================================================

        case IROp::MVN: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result = ~op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {

                cpu.setNZ(result);

                cpu.C =
                    shifterCarry;
            }

            return false;
        }


        // ====================================================
        // ADD
        // ====================================================

        case IROp::ADD: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    cpu.r[ir.rn],
                    op2,
                    false
                );

            cpu.r[ir.rd] =
                result.value;

            if (ir.setFlags) {

                setArithmeticFlags(
                    result.value,
                    result.carry,
                    result.overflow
                );
            }

            return false;
        }


        // ====================================================
        // ADC
        // ====================================================

        case IROp::ADC: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    cpu.r[ir.rn],
                    op2,
                    cpu.C
                );

            cpu.r[ir.rd] =
                result.value;

            if (ir.setFlags) {

                setArithmeticFlags(
                    result.value,
                    result.carry,
                    result.overflow
                );
            }

            return false;
        }


        // ====================================================
        // SUB
        // ====================================================

        case IROp::SUB: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    cpu.r[ir.rn],
                    ~op2,
                    true
                );

            cpu.r[ir.rd] =
                result.value;

            if (ir.setFlags) {

                setArithmeticFlags(
                    result.value,
                    result.carry,
                    result.overflow
                );
            }

            return false;
        }


        // ====================================================
        // RSB
        //
        // RSB Rd,Rn,Operand2
        //
        // Rd = Operand2 - Rn
        // ====================================================

        case IROp::RSB: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    op2,
                    ~cpu.r[ir.rn],
                    true
                );

            cpu.r[ir.rd] =
                result.value;

            if (ir.setFlags) {

                setArithmeticFlags(
                    result.value,
                    result.carry,
                    result.overflow
                );
            }

            return false;
        }


        // ====================================================
        // SBC
        // ====================================================

        case IROp::SBC: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    cpu.r[ir.rn],
                    ~op2,
                    cpu.C
                );

            cpu.r[ir.rd] =
                result.value;

            if (ir.setFlags) {

                setArithmeticFlags(
                    result.value,
                    result.carry,
                    result.overflow
                );
            }

            return false;
        }


        // ====================================================
        // RSC
        //
        // Rd = Operand2 - Rn - !C
        // ====================================================

        case IROp::RSC: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    op2,
                    ~cpu.r[ir.rn],
                    cpu.C
                );

            cpu.r[ir.rd] =
                result.value;

            if (ir.setFlags) {

                setArithmeticFlags(
                    result.value,
                    result.carry,
                    result.overflow
                );
            }

            return false;
        }


        // ====================================================
        // AND
        // ====================================================

        case IROp::AND: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result =
                cpu.r[ir.rn] & op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {

                cpu.setNZ(result);

                cpu.C =
                    shifterCarry;
            }

            return false;
        }


        // ====================================================
        // ORR
        // ====================================================

        case IROp::ORR: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result =
                cpu.r[ir.rn] | op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {

                cpu.setNZ(result);

                cpu.C =
                    shifterCarry;
            }

            return false;
        }


        // ====================================================
        // EOR
        // ====================================================

        case IROp::EOR: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result =
                cpu.r[ir.rn] ^ op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {

                cpu.setNZ(result);

                cpu.C =
                    shifterCarry;
            }

            return false;
        }


        // ====================================================
        // BIC
        // ====================================================

        case IROp::BIC: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result =
                cpu.r[ir.rn] & ~op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {

                cpu.setNZ(result);

                cpu.C =
                    shifterCarry;
            }

            return false;
        }


        // ====================================================
        // CMP
        // ====================================================

        case IROp::CMP: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    cpu.r[ir.rn],
                    ~op2,
                    true
                );

            setArithmeticFlags(
                result.value,
                result.carry,
                result.overflow
            );

            return false;
        }


        // ====================================================
        // CMN
        // ====================================================

        case IROp::CMN: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            auto result =
                addWithCarry(
                    cpu.r[ir.rn],
                    op2,
                    false
                );

            setArithmeticFlags(
                result.value,
                result.carry,
                result.overflow
            );

            return false;
        }


        // ====================================================
        // TST
        // ====================================================

        case IROp::TST: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result =
                cpu.r[ir.rn] & op2;

            cpu.setNZ(result);

            cpu.C =
                shifterCarry;

            return false;
        }


        // ====================================================
        // TEQ
        // ====================================================

        case IROp::TEQ: {

            uint32_t op2 =
                operand2(
                    ir.operand2,
                    &shifterCarry
                );

            uint32_t result =
                cpu.r[ir.rn] ^ op2;

            cpu.setNZ(result);

            cpu.C =
                shifterCarry;

            return false;
        }


        // ====================================================
        // LDR
        // ====================================================

        case IROp::LDR: {

            uint32_t base =
                cpu.r[ir.rn];


            // ------------------------------------------------
            // Calculate offset
            // ------------------------------------------------

            uint32_t offset =
                0;

            if (ir.operand2.immediate) {

                offset =
                    ir.operand2.imm;
            }
            else {

                bool ignoredCarry =
                    cpu.C;

                offset =
                    operand2(
                        ir.operand2,
                        &ignoredCarry
                    );
            }


            // ------------------------------------------------
            // Calculate indexed address
            // ------------------------------------------------

            uint32_t indexed;

            if (ir.up) {

                indexed =
                    base + offset;

            }
            else {

                indexed =
                    base - offset;
            }


            // ------------------------------------------------
            // Determine actual memory address
            // ------------------------------------------------

            uint32_t address =
                ir.preIndex
                    ? indexed
                    : base;


            // ------------------------------------------------
            // Read memory
            // ------------------------------------------------

            uint32_t value =
                0;

            switch (ir.memorySize) {

                case MemorySize::Byte: {

                    uint8_t data =
                        memory.read8(address);

                    if (ir.signExtend) {

                        value =
                            (data & 0x80u)
                                ? (0xFFFFFF00u |
                                   static_cast<uint32_t>(data))
                                : static_cast<uint32_t>(data);

                    }
                    else {

                        value =
                            static_cast<uint32_t>(data);
                    }

                    break;
                }


                case MemorySize::Halfword: {

                    uint16_t data =
                        memory.read16(address);

                    if (ir.signExtend) {

                        value =
                            (data & 0x8000u)
                                ? (0xFFFF0000u |
                                   static_cast<uint32_t>(data))
                                : static_cast<uint32_t>(data);

                    }
                    else {

                        value =
                            static_cast<uint32_t>(data);
                    }

                    break;
                }


                case MemorySize::Word: {

                    value =
                        memory.read32(address);

                    break;
                }
            }


            // ------------------------------------------------
            // Write to destination register
            // ------------------------------------------------

            cpu.r[ir.rd] =
                value;


            // ------------------------------------------------
            // Write-back
            // ------------------------------------------------

            if (ir.writeBack) {

                cpu.r[ir.rn] =
                    indexed;
            }

            return false;
        }


        // ====================================================
        // STR
        // ====================================================

        case IROp::STR: {

            uint32_t base =
                cpu.r[ir.rn];


            // ------------------------------------------------
            // Calculate offset
            // ------------------------------------------------

            uint32_t offset =
                0;

            if (ir.operand2.immediate) {

                offset =
                    ir.operand2.imm;
            }
            else {

                bool ignoredCarry =
                    cpu.C;

                offset =
                    operand2(
                        ir.operand2,
                        &ignoredCarry
                    );
            }


            // ------------------------------------------------
            // Calculate indexed address
            // ------------------------------------------------

            uint32_t indexed;

            if (ir.up) {

                indexed =
                    base + offset;

            }
            else {

                indexed =
                    base - offset;
            }


            // ------------------------------------------------
            // Actual memory address
            // ------------------------------------------------

            uint32_t address =
                ir.preIndex
                    ? indexed
                    : base;


            // ------------------------------------------------
            // Store
            // ------------------------------------------------

            switch (ir.memorySize) {

                case MemorySize::Byte:

                    memory.write8(
                        address,
                        static_cast<uint8_t>(
                            cpu.r[ir.rd] & 0xFFu
                        )
                    );

                    break;


                case MemorySize::Halfword:

                    memory.write16(
                        address,
                        static_cast<uint16_t>(
                            cpu.r[ir.rd] & 0xFFFFu
                        )
                    );

                    break;


                case MemorySize::Word:

                    memory.write32(
                        address,
                        cpu.r[ir.rd]
                    );

                    break;
            }


            // ------------------------------------------------
            // Write-back
            // ------------------------------------------------

            if (ir.writeBack) {

                cpu.r[ir.rn] =
                    indexed;
            }

            return false;
        }


        // ====================================================
        // B
        //
        // Branch
        // ====================================================

        case IROp::B: {

            cpu.r[15] +=
                ir.branchOffset;

            return true;
        }


        // ====================================================
        // BL
        // ====================================================

        case IROp::BL: {

            uint32_t oldPC =
                cpu.r[15];

            cpu.r[14] =
                oldPC - 4;

            cpu.r[15] =
                oldPC + ir.branchOffset;

            return true;
        }


        // ====================================================
        // BX
        // ====================================================

        case IROp::BX: {

            uint32_t target =
                cpu.r[ir.rm];

            cpu.T =
                (target & 1u) != 0;

            cpu.r[15] =
                target & ~1u;

            return true;
        }


        // ====================================================
        // NOP
        // ====================================================

        case IROp::INVALID: 
            return false;
        

        case IROp::NOP: 
            return true;
    }          

    return false;
}