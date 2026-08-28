#include "arm_interpreter.h"

#include "arm_condition.h"
#include "arm_shifter.h"
#include "arm_alu.h"

uint32_t ARMInterpreter::operand2(
    const Operand2& op,
    bool* carryOut)
{
    if (op.immediate) {

        *carryOut = cpu.C;

        return op.imm;
    }

    uint32_t value =
        cpu.r[op.rm];

    uint32_t amount;

    if (op.shiftImmediate) {

        amount =
            op.shiftAmount;

    } else {

        amount =
            cpu.r[op.shiftReg] & 0xFF;
    }

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

void ARMInterpreter::setArithmeticFlags(
    uint32_t result,
    bool carry,
    bool overflow)
{
    cpu.N = result & 0x80000000u;
    cpu.Z = result == 0;
    cpu.C = carry;
    cpu.V = overflow;
}

bool ARMInterpreter::execute(
    const IRInstruction& ir)
{
    if (!conditionPassed(
            ir.condition,
            cpu))
    {
        return false;
    }

    bool shifterCarry = cpu.C;

    uint32_t op2 =
        operand2(
            ir.operand2,
            &shifterCarry
        );

    switch (ir.op) {

        case IROp::MOV: {

            uint32_t result = op2;

            cpu.r[ir.rd] = result;

            if (ir.setFlags) {
                cpu.setNZ(result);
                cpu.C = shifterCarry;
            }

            break;
        }

        case IROp::MVN: {

            uint32_t result =
                ~op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {
                cpu.setNZ(result);
                cpu.C = shifterCarry;
            }

            break;
        }

        case IROp::ADD: {

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

            break;
        }

        case IROp::ADC: {

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

            break;
        }

        case IROp::SUB: {

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

            break;
        }

        case IROp::SBC: {

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

            break;
        }

        case IROp::AND: {

            uint32_t result =
                cpu.r[ir.rn] & op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {
                cpu.setNZ(result);
                cpu.C = shifterCarry;
            }

            break;
        }

        case IROp::ORR: {

            uint32_t result =
                cpu.r[ir.rn] | op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {
                cpu.setNZ(result);
                cpu.C = shifterCarry;
            }

            break;
        }

        case IROp::EOR: {

            uint32_t result =
                cpu.r[ir.rn] ^ op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {
                cpu.setNZ(result);
                cpu.C = shifterCarry;
            }

            break;
        }

        case IROp::BIC: {

            uint32_t result =
                cpu.r[ir.rn] & ~op2;

            cpu.r[ir.rd] =
                result;

            if (ir.setFlags) {
                cpu.setNZ(result);
                cpu.C = shifterCarry;
            }

            break;
        }

        case IROp::CMP: {

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

            break;
        }

        case IROp::CMN: {

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

            break;
        }

        case IROp::TST: {

            uint32_t result =
                cpu.r[ir.rn] & op2;

            cpu.setNZ(result);
            cpu.C = shifterCarry;

            break;
        }

        case IROp::TEQ: {

            uint32_t result =
                cpu.r[ir.rn] ^ op2;

            cpu.setNZ(result);
            cpu.C = shifterCarry;

            break;
        }

        case IROp::LDR: {

            // --------------------------------------------------------
            // Base address
            // --------------------------------------------------------

            uint32_t base =
                cpu.r[ir.rn];


            // --------------------------------------------------------
            // Calculate offset
            // --------------------------------------------------------

            uint32_t offset = 0;

            if (ir.operand2.immediate) {

                offset =
                    ir.operand2.imm;

            }
            else {

                // Register offset with optional shift.
                bool ignoredCarry = cpu.C;

                offset =
                    operand2(
                        ir.operand2,
                        &ignoredCarry
                    );
            }


            // --------------------------------------------------------
            // Indexed address
            // --------------------------------------------------------

            uint32_t indexed;

            if (ir.up)
                indexed = base + offset;
            else
                indexed = base - offset;


            // --------------------------------------------------------
            // Transfer address
            //
            // Pre-index:
            //      LDR r0, [r1, #4]
            //      address = base + offset
            //
            // Post-index:
            //      LDR r0, [r1], #4
            //      address = base
            // --------------------------------------------------------

            uint32_t address =
                ir.preIndex
                    ? indexed
                    : base;


            // --------------------------------------------------------
            // Read according to memory width
            // --------------------------------------------------------

            uint32_t value = 0;

            switch (ir.memorySize) {

                case MemorySize::Byte: {

                    uint8_t data =
                        memory.read8(address);

                    if (ir.signExtend) {

                        // Explicit sign extension:
                        //
                        // 0x80 -> 0xFFFFFF80
                        // 0xFF -> 0xFFFFFFFF
                        //
                        value =
                            (data & 0x80)
                                ? (0xFFFFFF00u | data)
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

                        // Explicit sign extension:
                        //
                        // 0x8000 -> 0xFFFF8000
                        // 0xFFFF -> 0xFFFFFFFF

                        value =
                            (data & 0x8000)
                                ? (0xFFFF0000u | data)
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


            // --------------------------------------------------------
            // Write loaded value to Rd
            // --------------------------------------------------------

            cpu.r[ir.rd] =
                value;


            // --------------------------------------------------------
            // Write-back
            //
            // IMPORTANT:
            // post-index must write INDEXED, not ADDRESS.
            //
            // LDR r0, [r1], #4
            //
            // r1 = old r1 + 4
            // --------------------------------------------------------

            if (ir.writeBack) {

                cpu.r[ir.rn] =
                    indexed;
            }

            break;
        }

        case IROp::STR: {

            // --------------------------------------------------------
            // Base address
            // --------------------------------------------------------

            uint32_t base =
                cpu.r[ir.rn];


            // --------------------------------------------------------
            // Calculate offset
            // --------------------------------------------------------

            uint32_t offset = 0;

            if (ir.operand2.immediate) {

                offset =
                    ir.operand2.imm;

            }
            else {

                bool ignoredCarry = cpu.C;

                offset =
                    operand2(
                        ir.operand2,
                        &ignoredCarry
                    );
            }


            // --------------------------------------------------------
            // Indexed address
            // --------------------------------------------------------

            uint32_t indexed;

            if (ir.up)
                indexed = base + offset;
            else
                indexed = base - offset;


            // --------------------------------------------------------
            // Actual transfer address
            // --------------------------------------------------------

            uint32_t address =
                ir.preIndex
                    ? indexed
                    : base;


            // --------------------------------------------------------
            // Store according to memory width
            // --------------------------------------------------------

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


            // --------------------------------------------------------
            // Write-back
            // --------------------------------------------------------

            if (ir.writeBack) {

                cpu.r[ir.rn] =
                    indexed;
            }

            break;
        }

        case IROp::B: {

            cpu.r[15] +=
                ir.branchOffset;

            return true;
        }

        case IROp::BL: {

            cpu.r[14] =
                cpu.r[15] - 4;

            cpu.r[15] +=
                ir.branchOffset;

            return true;
        }

        case IROp::BX: {

            uint32_t target =
                cpu.r[ir.rm];

            cpu.T =
                target & 1;

            cpu.r[15] =
                target & ~1u;

            return true;
        }

        case IROp::NOP:
            break;
    }

    return false;
}