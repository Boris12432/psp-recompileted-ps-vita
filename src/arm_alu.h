#pragma once

#include <cstdint>
#include "arm_cpu.h"

struct AddResult {
    uint32_t value;
    bool carry;
    bool overflow;
};

inline AddResult addWithCarry(
    uint32_t a,
    uint32_t b,
    bool carryIn)
{
    uint64_t result =
        uint64_t(a) +
        uint64_t(b) +
        uint64_t(carryIn);

    uint32_t value =
        static_cast<uint32_t>(result);

    bool carry =
        (result >> 32) != 0;

    bool overflow =
        ((~(a ^ b) & (a ^ value))
         & 0x80000000u) != 0;

    return {
        value,
        carry,
        overflow
    };
}