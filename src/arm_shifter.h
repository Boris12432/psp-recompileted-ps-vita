#pragma once

#include <cstdint>
#include "arm_cpu.h"
#include "ir.h"

struct ShiftResult {
    uint32_t value;
    bool carry;
};

inline ShiftResult shiftValue(
    uint32_t value,
    ShiftType type,
    uint32_t amount,
    bool oldCarry)
{
    ShiftResult result{value, oldCarry};

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
                // RRX
                result.carry = value & 1;

                result.value =
                    (oldCarry ? 0x80000000u : 0) |
                    (value >> 1);
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