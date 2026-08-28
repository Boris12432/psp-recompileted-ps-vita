#include <stdexcept>

#include "arm_memory.h"

SimpleMemory::SimpleMemory(size_t size)
{
    map(
        0,
        static_cast<uint32_t>(size)
    );
}

MemoryRegion* SimpleMemory::findRegion(
    uint32_t address
)
{
    for (auto& region : regions)
    {
        if (address >= region.base &&
            address < region.base + region.size)
        {
            return &region;
        }
    }

    return nullptr;
}


const MemoryRegion* SimpleMemory::findRegion(
    uint32_t address
) const
{
    for (const auto& region : regions)
    {
        if (address >= region.base &&
            address < region.base + region.size)
        {
            return &region;
        }
    }

    return nullptr;
}


bool SimpleMemory::contains(
    uint32_t address
) const
{
    return findRegion(address) != nullptr;
}


void SimpleMemory::map(
    uint32_t base,
    uint32_t size,
    bool readable,
    bool writable,
    bool executable
)
{
    MemoryRegion region;

    region.base = base;
    region.size = size;

    region.readable = readable;
    region.writable = writable;
    region.executable = executable;

    region.data.resize(
        size,
        0
    );

    regions.push_back(
        std::move(region)
    );
}


void SimpleMemory::load(
    uint32_t address,
    const uint8_t* data,
    uint32_t size
)
{
    if (data == nullptr || size == 0)
        return;


    MemoryRegion* region =
        findRegion(address);


    if (!region)
    {
        throw std::runtime_error(
            "SimpleMemory::load: unmapped address"
        );
    }


    uint32_t offset =
        address - region->base;


    if (offset + size > region->size)
    {
        throw std::runtime_error(
            "SimpleMemory::load: outside region"
        );
    }


    for (uint32_t i = 0; i < size; ++i)
    {
        region->data[offset + i] =
            data[i];
    }
}


uint8_t SimpleMemory::read8(
    uint32_t address
)
{
    MemoryRegion* region =
        findRegion(address);


    if (!region || !region->readable)
    {
        throw std::runtime_error(
            "SimpleMemory: invalid read8"
        );
    }


    return region->data[
        address - region->base
    ];
}


uint16_t SimpleMemory::read16(
    uint32_t address
)
{
    uint16_t b0 =
        read8(address);

    uint16_t b1 =
        read8(address + 1);


    return
        b0 |
        (b1 << 8);
}


uint32_t SimpleMemory::read32(
    uint32_t address
)
{
    uint32_t b0 =
        read8(address);

    uint32_t b1 =
        read8(address + 1);

    uint32_t b2 =
        read8(address + 2);

    uint32_t b3 =
        read8(address + 3);


    return
        b0 |
        (b1 << 8) |
        (b2 << 16) |
        (b3 << 24);
}


void SimpleMemory::write8(
    uint32_t address,
    uint8_t value
)
{
    MemoryRegion* region =
        findRegion(address);


    if (!region || !region->writable)
    {
        throw std::runtime_error(
            "SimpleMemory: invalid write8"
        );
    }


    region->data[
        address - region->base
    ] = value;
}


void SimpleMemory::write16(
    uint32_t address,
    uint16_t value
)
{
    write8(
        address,
        static_cast<uint8_t>(
            value & 0xFF
        )
    );


    write8(
        address + 1,
        static_cast<uint8_t>(
            (value >> 8) & 0xFF
        )
    );
}


void SimpleMemory::write32(
    uint32_t address,
    uint32_t value
)
{
    write8(
        address,
        static_cast<uint8_t>(
            value & 0xFF
        )
    );


    write8(
        address + 1,
        static_cast<uint8_t>(
            (value >> 8) & 0xFF
        )
    );


    write8(
        address + 2,
        static_cast<uint8_t>(
            (value >> 16) & 0xFF
        )
    );


    write8(
        address + 3,
        static_cast<uint8_t>(
            (value >> 24) & 0xFF
        )
    );
}