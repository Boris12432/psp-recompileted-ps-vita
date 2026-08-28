#include "vita_elf.h"

#include <fstream>
#include <cstring>
#include <cstdio>


// ============================================================
// ELF structures
// ============================================================

struct ELF32Header
{
    uint8_t  ident[16];

    uint16_t type;
    uint16_t machine;

    uint32_t version;

    uint32_t entry;

    uint32_t programHeaderOffset;
    uint32_t sectionHeaderOffset;

    uint32_t flags;

    uint16_t headerSize;

    uint16_t programHeaderEntrySize;
    uint16_t programHeaderCount;

    uint16_t sectionHeaderEntrySize;
    uint16_t sectionHeaderCount;

    uint16_t stringTableIndex;
};


struct ELF32SectionHeader
{
    uint32_t name;

    uint32_t type;

    uint32_t flags;

    uint32_t address;

    uint32_t offset;

    uint32_t size;

    uint32_t link;

    uint32_t info;

    uint32_t alignment;

    uint32_t entrySize;
};


// ============================================================
// Load
// ============================================================

bool VitaELF::load(
    const std::string& filename
)
{
    std::ifstream file(
        filename,
        std::ios::binary
    );

    if (!file)
    {
        std::printf(
            "[ELF] Cannot open: %s\n",
            filename.c_str()
        );

        return false;
    }


    file.seekg(
        0,
        std::ios::end
    );

    std::streamsize size =
        file.tellg();

    file.seekg(
        0,
        std::ios::beg
    );


    if (size <= 0)
        return false;


    data.resize(
        static_cast<size_t>(size)
    );


    if (!file.read(
            reinterpret_cast<char*>(data.data()),
            size))
    {
        return false;
    }


    return parse();
}


// ============================================================
// Parse
// ============================================================

bool VitaELF::parse()
{
    if (data.size() <
        sizeof(ELF32Header))
    {
        std::printf(
            "[ELF] File too small\n"
        );

        return false;
    }


    const auto* header =
        reinterpret_cast<
            const ELF32Header*
        >(data.data());


    // --------------------------------------------------------
    // ELF magic
    // --------------------------------------------------------

    if (
        header->ident[0] != 0x7F ||
        header->ident[1] != 'E' ||
        header->ident[2] != 'L' ||
        header->ident[3] != 'F'
    )
    {
        std::printf(
            "[ELF] Invalid magic\n"
        );

        return false;
    }


    // --------------------------------------------------------
    // ELF class
    // --------------------------------------------------------

    if (header->ident[4] != 1)
    {
        std::printf(
            "[ELF] Not ELF32\n"
        );

        return false;
    }


    // --------------------------------------------------------
    // ARM
    // --------------------------------------------------------

    if (header->machine != 40)
    {
        std::printf(
            "[ELF] Not ARM ELF\n"
        );

        return false;
    }


    entry =
        header->entry;


    std::printf(
        "[ELF] Entry point: 0x%08X\n",
        entry
    );


    // --------------------------------------------------------
    // Section table validation
    // --------------------------------------------------------

    uint32_t sectionTableEnd =
        header->sectionHeaderOffset +
        header->sectionHeaderCount *
        header->sectionHeaderEntrySize;


    if (
        header->sectionHeaderOffset >= data.size() ||
        sectionTableEnd > data.size()
    )
    {
        std::printf(
            "[ELF] Invalid section table\n"
        );

        return false;
    }


    const auto* sections =
        reinterpret_cast<
            const ELF32SectionHeader*
        >(
            data.data() +
            header->sectionHeaderOffset
        );


    // --------------------------------------------------------
    // Section string table
    // --------------------------------------------------------

    if (
        header->stringTableIndex >=
        header->sectionHeaderCount
    )
    {
        return false;
    }


    const auto& stringSection =
        sections[
            header->stringTableIndex
        ];


    if (
        stringSection.offset +
        stringSection.size >
        data.size()
    )
    {
        return false;
    }


    const char* strings =
        reinterpret_cast<
            const char*
        >(
            data.data() +
            stringSection.offset
        );


    // --------------------------------------------------------
    // Read sections
    // --------------------------------------------------------

    sectionList.clear();


    for (
        uint16_t i = 0;
        i < header->sectionHeaderCount;
        ++i
    )
    {
        const auto& src =
            sections[i];


        ELFSection section;


        if (
            src.name <
            stringSection.size
        )
        {
            section.name =
                strings + src.name;
        }


        section.type =
            src.type;

        section.flags =
            src.flags;

        section.address =
            src.address;

        section.offset =
            src.offset;

        section.size =
            src.size;

        section.link =
            src.link;

        section.info =
            src.info;

        section.alignment =
            src.alignment;

        section.entrySize =
            src.entrySize;


        sectionList.push_back(
            section
        );


        std::printf(
            "[ELF] Section %-20s "
            "addr=0x%08X "
            "size=0x%08X\n",

            section.name.c_str(),

            section.address,

            section.size
        );
    }


    return true;
}


// ============================================================
// Getters
// ============================================================

uint32_t VitaELF::entryPoint() const
{
    return entry;
}


const std::vector<ELFSection>&
VitaELF::sections() const
{
    return sectionList;
}


const std::vector<uint8_t>&
VitaELF::image() const
{
    return data;
}