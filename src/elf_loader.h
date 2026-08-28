#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "arm_memory.h"

bool loadIntoMemory(
    const std::string& filename,
    ARMMemory& memory
);

struct LoadedSegment
{
    uint32_t vaddr = 0;

    uint32_t size = 0;

    uint32_t flags = 0;

    std::vector<uint8_t> data;
};


class ELFLoader
{
public:

    bool load(
        const std::string& filename
    );


    uint32_t entryPoint() const
    {
        return entry;
    }


    const std::vector<LoadedSegment>&
    segments() const
    {
        return loadedSegments;
    }


private:

    uint32_t entry = 0;

    std::vector<LoadedSegment>
        loadedSegments;
};