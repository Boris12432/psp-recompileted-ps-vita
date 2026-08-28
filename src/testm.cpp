#include <vector>
#include <cstdint>
#include <stdexcept>

#include "arm_memory.h"

class TestMemory : public ARMMemory
{
public:

    explicit TestMemory(size_t size)
        : data(size, 0)
    {
    }

    uint8_t read8(uint32_t address) override
    {
        check(address, 1);

        return data[address];
    }

    uint16_t read16(uint32_t address) override
    {
        check(address, 2);

        return
            static_cast<uint16_t>(data[address]) |
            (static_cast<uint16_t>(data[address + 1]) << 8);
    }

    uint32_t read32(uint32_t address) override
    {
        check(address, 4);

        return
            static_cast<uint32_t>(data[address]) |
            (static_cast<uint32_t>(data[address + 1]) << 8) |
            (static_cast<uint32_t>(data[address + 2]) << 16) |
            (static_cast<uint32_t>(data[address + 3]) << 24);
    }

    void write8(uint32_t address, uint8_t value) override
    {
        check(address, 1);

        data[address] = value;
    }

    void write16(uint32_t address, uint16_t value) override
    {
        check(address, 2);

        data[address] =
            static_cast<uint8_t>(value);

        data[address + 1] =
            static_cast<uint8_t>(value >> 8);
    }

    void write32(uint32_t address, uint32_t value) override
    {
        check(address, 4);

        data[address] =
            static_cast<uint8_t>(value);

        data[address + 1] =
            static_cast<uint8_t>(value >> 8);

        data[address + 2] =
            static_cast<uint8_t>(value >> 16);

        data[address + 3] =
            static_cast<uint8_t>(value >> 24);
    }

private:

    std::vector<uint8_t> data;

    void check(
        uint32_t address,
        uint32_t size
    )
    {
        if (
            static_cast<uint64_t>(address) + size >
            data.size()
        )
        {
            throw std::out_of_range(
                "TestMemory: address out of range"
            );
        }
    }
};