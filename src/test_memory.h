#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include "arm_memory.h"


class TestMemory : public ARMMemory
{
public:

    explicit TestMemory(
        size_t size = 1024 * 1024
    )
        : data(size, 0)
    {
    }


    // ========================================================
    // Mapping
    // ========================================================

    bool isMapped(
        uint32_t address
    ) const override
    {
        return address < data.size();
    }


    // ========================================================
    // Read
    // ========================================================

    uint8_t read8(
        uint32_t address
    ) override
    {
        return data.at(address);
    }


    uint16_t read16(
        uint32_t address
    ) override
    {
        return
            static_cast<uint16_t>(data.at(address)) |
            (static_cast<uint16_t>(data.at(address + 1)) << 8);
    }


    uint32_t read32(
        uint32_t address
    ) override
    {
        return
            static_cast<uint32_t>(data.at(address)) |
            (static_cast<uint32_t>(data.at(address + 1)) << 8) |
            (static_cast<uint32_t>(data.at(address + 2)) << 16) |
            (static_cast<uint32_t>(data.at(address + 3)) << 24);
    }


    // ========================================================
    // Write
    // ========================================================

    void write8(
        uint32_t address,
        uint8_t value
    ) override
    {
        data.at(address) = value;
    }


    void write16(
        uint32_t address,
        uint16_t value
    ) override
    {
        data.at(address) =
            static_cast<uint8_t>(value & 0xFF);

        data.at(address + 1) =
            static_cast<uint8_t>((value >> 8) & 0xFF);
    }


    void write32(
        uint32_t address,
        uint32_t value
    ) override
    {
        data.at(address) =
            static_cast<uint8_t>(value & 0xFF);

        data.at(address + 1) =
            static_cast<uint8_t>((value >> 8) & 0xFF);

        data.at(address + 2) =
            static_cast<uint8_t>((value >> 16) & 0xFF);

        data.at(address + 3) =
            static_cast<uint8_t>((value >> 24) & 0xFF);
    }


private:

    std::vector<uint8_t> data;
};