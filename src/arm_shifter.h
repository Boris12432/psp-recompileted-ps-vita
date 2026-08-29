#pragma once

#include <cstdint>
#include "arm_cpu.h"
#include "ir.h"

struct ShiftResult {
    uint32_t value;
    bool carry;
};

/*
 * registerForm:
 *
 *     false (immediate shift, e.g. "MOV R0,R1,LSR #imm"):
 *         amount == 0 has special architectural meaning -
 *         LSR/ASR #0 mean "shift by 32", ROR #0 means RRX.
 *         (In practice the decoder already converts immediate
 *         ROR #0 into ShiftType::RRX before we ever get here,
 *         but LSR/ASR #0 -> 32 is still handled below.)
 *
 *     true (register-controlled shift, e.g. "MOV R0,R1,LSR R2"):
 *         amount comes from a register (masked to 8 bits) and
 *         is a literal shift count. If it happens to be 0, the
 *         ARM architecture says NO shift occurs at all and the
 *         carry-out is simply the old C flag - regardless of
 *         shift type. Reusing the immediate-form "0 means 32"
 *         / "0 means RRX" rules here would be wrong.
 */
inline ShiftResult shiftValue(
    uint32_t value,
    ShiftType type,
    uint32_t amount,
    bool oldCarry,
    bool registerForm = false)
{
    ShiftResult result{value, oldCarry};

    if (registerForm && amount == 0) {
        // No shift at all: value unchanged, carry unchanged.
        return result;
    }

    switch (type) {

        case ShiftType::LSL:

            if (amount == 0) {
                result.value = value;
            }
            else if (amount < 32) {
                result.carry =
                    (value >> (32 - amount)) & 1;

                result.value =
                    value << amount;
            }
            else if (amount == 32) {
                result.carry = value & 1;
                result.value = 0;
            }
            else {
                result.carry = false;
                result.value = 0;
            }

            break;

        case ShiftType::LSR:

            if (amount == 0) {
                amount = 32;
            }

            if (amount < 32) {
                result.carry =
                    (value >> (amount - 1)) & 1;

                result.value =
                    value >> amount;
            }
            else if (amount == 32) {
                result.carry =
                    (value >> 31) & 1;

                result.value = 0;
            }
            else {
                result.carry = false;
                result.value = 0;
            }

            break;

        case ShiftType::ASR:

            if (amount == 0) {
                amount = 32;
            }

            if (amount < 32) {

                result.carry =
                    (value >> (amount - 1)) & 1;

                result.value =
                    static_cast<uint32_t>(
                        static_cast<int32_t>(value)
                        >> amount
                    );

            } else {

                result.carry =
                    (value >> 31) & 1;

                result.value =
                    (value & 0x80000000u)
                        ? 0xFFFFFFFFu
                        : 0;
            }

            break;

        case ShiftType::ROR:

            if (amount == 0) {
                // Only reachable for the immediate-encoded form
                // (register-form amount==0 already returned above).
                // ROR #0 is RRX.
                result.carry = value & 1;

                result.value =
                    (oldCarry ? 0x80000000u : 0) |
                    (value >> 1);
            }
            else if ((amount & 31) == 0) {
                // ROR by a multiple of 32 (e.g. amount == 32):
                // per the ARM architecture reference, the value
                // is unchanged and carry = bit 31 of the value.
                //
                // Reducing "amount &= 31" first would turn this
                // into a bogus ROR-by-0 path below, and computing
                // (amount - 1) after that reduction shifts by
                // 0xFFFFFFFF - undefined behaviour.
                result.carry =
                    (value >> 31) & 1;

                result.value =
                    value;
            }
            else {

                amount &= 31;

                result.carry =
                    (value >> (amount - 1)) & 1;

                result.value =
                    (value >> amount) |
                    (value << (32 - amount));
            }

            break;

        case ShiftType::RRX:

            result.carry = value & 1;

            result.value =
                (oldCarry ? 0x80000000u : 0) |
                (value >> 1);

            break;
    }

    return result;
}
