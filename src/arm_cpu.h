#pragma once

#include <array>
#include <cstdint>

struct ARMCPU
{
    // ========================================================
    // General purpose registers
    // ========================================================

    std::array<uint32_t, 16> r{};


    // ========================================================
    // CPSR flags
    // ========================================================

    bool N = false;
    bool Z = false;
    bool C = false;
    bool V = false;

    // Thumb state
    bool T = false;


    // ========================================================
    // Processor mode
    // ========================================================

    enum class Mode : uint32_t
    {
        User       = 0x10,
        FIQ        = 0x11,
        IRQ        = 0x12,
        Supervisor = 0x13,
        Abort      = 0x17,
        Undefined  = 0x1B,
        System     = 0x1F
    };

    Mode mode = Mode::User;


    // ========================================================
    // Saved Program Status Register
    //
    // Пока один SPSR.
    //
    // Позже, если понадобится полноценная ARM-архитектура,
    // можно сделать отдельные SPSR_fiq/spsr_irq/spsr_svc...
    // ========================================================

    uint32_t spsr = 0;


    // ========================================================
    // PC
    // ========================================================

    uint32_t pc() const
    {
        return r[15];
    }


    // ========================================================
    // Flags
    // ========================================================

    void setNZ(uint32_t value)
    {
        N =
            (value & 0x80000000u) != 0;

        Z =
            value == 0;
    }


    // ========================================================
    // CPSR
    // ========================================================

    uint32_t cpsr() const
    {
        uint32_t value = 0;

        // N
        if (N)
            value |= 1u << 31;

        // Z
        if (Z)
            value |= 1u << 30;

        // C
        if (C)
            value |= 1u << 29;

        // V
        if (V)
            value |= 1u << 28;

        // T
        if (T)
            value |= 1u << 5;

        // Mode
        value |=
            static_cast<uint32_t>(mode);

        return value;
    }


    // ========================================================
    // Set CPSR
    // ========================================================

    void setCPSR(uint32_t value)
    {
        // N
        N =
            (value & (1u << 31)) != 0;

        // Z
        Z =
            (value & (1u << 30)) != 0;

        // C
        C =
            (value & (1u << 29)) != 0;

        // V
        V =
            (value & (1u << 28)) != 0;

        // T
        T =
            (value & (1u << 5)) != 0;


        // ----------------------------------------------------
        // Processor mode
        // ----------------------------------------------------

        switch (value & 0x1Fu)
        {
            case 0x10:
                mode = Mode::User;
                break;

            case 0x11:
                mode = Mode::FIQ;
                break;

            case 0x12:
                mode = Mode::IRQ;
                break;

            case 0x13:
                mode = Mode::Supervisor;
                break;

            case 0x17:
                mode = Mode::Abort;
                break;

            case 0x1B:
                mode = Mode::Undefined;
                break;

            case 0x1F:
                mode = Mode::System;
                break;

            default:
                // Неизвестный mode не применяем.
                break;
        }
    }


    // ========================================================
    // Privilege
    // ========================================================

    bool privileged() const
    {
        return mode != Mode::User;
    }


    // ========================================================
    // SPSR availability
    // ========================================================

    bool hasSPSR() const
    {
        switch (mode)
        {
            case Mode::FIQ:
            case Mode::IRQ:
            case Mode::Supervisor:
            case Mode::Abort:
            case Mode::Undefined:
                return true;

            case Mode::User:
            case Mode::System:
                return false;
        }

        return false;
    }
};