#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ELFSection
{
    std::string name;

    uint32_t type = 0;
    uint32_t flags = 0;

    uint32_t address = 0;
    uint32_t offset = 0;
    uint32_t size = 0;

    uint32_t link = 0;
    uint32_t info = 0;

    uint32_t alignment = 0;
    uint32_t entrySize = 0;
};


class VitaELF
{
public:

    bool load(
        const std::string& filename
    );

    uint32_t entryPoint() const;

    const std::vector<ELFSection>& sections() const;

    const std::vector<uint8_t>& image() const;

private:

    bool parse();

    std::vector<uint8_t> data;

    std::vector<ELFSection> sectionList;

    uint32_t entry = 0;
};