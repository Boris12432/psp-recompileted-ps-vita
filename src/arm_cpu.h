#pragma once

#include <array>
#include <cstdint>

struct ARMCPU
{
    std::array<uint32_t, 16> r{};

    bool N = false;
    bool Z = false;
    bool C = false;
    bool V = false;

    bool T = false;

    uint32_t pc() const
    {
        return r[15];
    }

    void setNZ(uint32_t value)
    {
        N = (value & 0x80000000u) != 0;
        Z = value == 0;
    }

    uint32_t cpsr() const
    {
        uint32_t value = 0;

        if (N) value |= 1u << 31;
        if (Z) value |= 1u << 30;
        if (C) value |= 1u << 29;
        if (V) value |= 1u << 28;
        if (T) value |= 1u << 5;

        return value;
    }

    void setCPSR(uint32_t value)
    {
        N = (value & (1u << 31)) != 0;
        Z = (value & (1u << 30)) != 0;
        C = (value & (1u << 29)) != 0;
        V = (value & (1u << 28)) != 0;
        T = (value & (1u << 5)) != 0;
    }
};