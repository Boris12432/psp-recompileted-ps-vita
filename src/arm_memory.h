#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

class ARMMemory
{
public:
    virtual ~ARMMemory() = default;

    virtual uint8_t read8(uint32_t address) = 0;
    virtual uint16_t read16(uint32_t address) = 0;
    virtual uint32_t read32(uint32_t address) = 0;

    virtual void write8(uint32_t address, uint8_t value) = 0;
    virtual void write16(uint32_t address, uint16_t value) = 0;
    virtual void write32(uint32_t address, uint32_t value) = 0;
};

struct MemoryRegion
{
    uint32_t base = 0;
    uint32_t size = 0;

    bool readable = true;
    bool writable = true;
    bool executable = true;

    std::vector<uint8_t> data;
};

class SimpleMemory : public ARMMemory
{
public:
    SimpleMemory() = default;

    explicit SimpleMemory(size_t size);

    void map(
        uint32_t base,
        uint32_t size,
        bool readable = true,
        bool writable = true,
        bool executable = true
    );

    void load(
        uint32_t address,
        const uint8_t* data,
        uint32_t size
    );

    uint8_t read8(uint32_t address) override;
    uint16_t read16(uint32_t address) override;
    uint32_t read32(uint32_t address) override;

    void write8(uint32_t address, uint8_t value) override;
    void write16(uint32_t address, uint16_t value) override;
    void write32(uint32_t address, uint32_t value) override;

private:
    MemoryRegion* findRegion(uint32_t address);
    const MemoryRegion* findRegion(uint32_t address) const;

    bool contains(uint32_t address) const;

    std::vector<MemoryRegion> regions;
};