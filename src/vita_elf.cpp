#include "vita_elf.h"

#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <limits>


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
    {
        std::printf(
            "[ELF] Empty file\n"
        );

        return false;
    }


    data.resize(
        static_cast<size_t>(size)
    );


    if (!file.read(
            reinterpret_cast<char*>(data.data()),
            size))
    {
        std::printf(
            "[ELF] Failed to read file\n"
        );

        return false;
    }


    return parse();
}


// ============================================================
// Parse
// ============================================================

bool VitaELF::parse()
{
    // --------------------------------------------------------
    // Minimum ELF header
    // --------------------------------------------------------

    if (
        data.size() <
        sizeof(ELF32Header)
    )
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

    if (
        header->ident[4] != 1
    )
    {
        std::printf(
            "[ELF] Not ELF32\n"
        );

        return false;
    }


    // --------------------------------------------------------
    // ELF header size
    // --------------------------------------------------------

    if (
        header->headerSize <
        sizeof(ELF32Header)
    )
    {
        std::printf(
            "[ELF] Invalid ELF header size\n"
        );

        return false;
    }


    if (
        header->headerSize >
        data.size()
    )
    {
        std::printf(
            "[ELF] ELF header exceeds file\n"
        );

        return false;
    }


    // --------------------------------------------------------
    // ARM
    // --------------------------------------------------------

    if (
        header->machine != 40
    )
    {
        std::printf(
            "[ELF] Not ARM ELF\n"
        );

        return false;
    }


    // --------------------------------------------------------
    // Entry point
    // --------------------------------------------------------

    entry =
        header->entry;


    std::printf(
        "[ELF] Entry point: 0x%08X\n",
        entry
    );


    // ========================================================
    // Section table validation
    // ========================================================

    // A section table with entries must have a valid entry size.
    if (
        header->sectionHeaderCount != 0 &&
        header->sectionHeaderEntrySize !=
        sizeof(ELF32SectionHeader)
    )
    {
        std::printf(
            "[ELF] Unsupported section header size: %u "
            "(expected %zu)\n",
            header->sectionHeaderEntrySize,
            sizeof(ELF32SectionHeader)
        );

        return false;
    }


    const uint64_t sectionTableOffset =
        static_cast<uint64_t>(
            header->sectionHeaderOffset
        );

    const uint64_t sectionTableCount =
        static_cast<uint64_t>(
            header->sectionHeaderCount
        );

    const uint64_t sectionEntrySize =
        static_cast<uint64_t>(
            header->sectionHeaderEntrySize
        );


    const uint64_t sectionTableSize =
        sectionTableCount *
        sectionEntrySize;


    const uint64_t sectionTableEnd =
        sectionTableOffset +
        sectionTableSize;


    // --------------------------------------------------------
    // Overflow-safe bounds check
    // --------------------------------------------------------

    if (
        sectionTableOffset >
        static_cast<uint64_t>(data.size())
    )
    {
        std::printf(
            "[ELF] Section table offset outside file\n"
        );

        return false;
    }


    if (
        sectionTableEnd >
        static_cast<uint64_t>(data.size())
    )
    {
        std::printf(
            "[ELF] Invalid section table\n"
        );

        return false;
    }


    // --------------------------------------------------------
    // No sections
    // --------------------------------------------------------

    if (
        header->sectionHeaderCount == 0
    )
    {
        sectionList.clear();

        return true;
    }


    // --------------------------------------------------------
    // Section table
    // --------------------------------------------------------

    const auto* sections =
        reinterpret_cast<
            const ELF32SectionHeader*
        >(
            data.data() +
            header->sectionHeaderOffset
        );


    // ========================================================
    // Section string table
    // ========================================================

    if (
        header->stringTableIndex >=
        header->sectionHeaderCount
    )
    {
        std::printf(
            "[ELF] Invalid string table index\n"
        );

        return false;
    }


    const auto& stringSection =
        sections[
            header->stringTableIndex
        ];


    const uint64_t stringTableOffset =
        static_cast<uint64_t>(
            stringSection.offset
        );

    const uint64_t stringTableSize =
        static_cast<uint64_t>(
            stringSection.size
        );

    const uint64_t stringTableEnd =
        stringTableOffset +
        stringTableSize;


    // --------------------------------------------------------
    // String table bounds
    // --------------------------------------------------------

    if (
        stringTableOffset >
        static_cast<uint64_t>(data.size())
    )
    {
        std::printf(
            "[ELF] Invalid string table offset\n"
        );

        return false;
    }


    if (
        stringTableEnd >
        static_cast<uint64_t>(data.size())
    )
    {
        std::printf(
            "[ELF] Invalid string table size\n"
        );

        return false;
    }


    const char* strings =
        reinterpret_cast<
            const char*
        >(
            data.data() +
            stringTableOffset
        );


    // ========================================================
    // Read sections
    // ========================================================

    sectionList.clear();

    sectionList.reserve(
        header->sectionHeaderCount
    );


    for (
        uint16_t i = 0;
        i < header->sectionHeaderCount;
        ++i
    )
    {
        const auto& src =
            sections[i];


        ELFSection section;


        // ----------------------------------------------------
        // Section name
        // ----------------------------------------------------

        if (
            static_cast<uint64_t>(src.name) <
            stringTableSize
        )
        {
            const char* name =
                strings + src.name;

            // ------------------------------------------------
            // Ensure the string is terminated inside table.
            // ------------------------------------------------

            const uint64_t remaining =
                stringTableSize -
                static_cast<uint64_t>(src.name);

            const void* terminator =
                std::memchr(
                    name,
                    '\0',
                    static_cast<size_t>(remaining)
                );

            if (terminator != nullptr)
            {
                section.name =
                    name;
            }
            else
            {
                std::printf(
                    "[ELF] Unterminated section name "
                    "at index %u\n",
                    i
                );

                return false;
            }
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


        // ----------------------------------------------------
        // Validate section data bounds
        // ----------------------------------------------------

        const uint64_t sectionOffset =
            static_cast<uint64_t>(
                src.offset
            );

        const uint64_t sectionSize =
            static_cast<uint64_t>(
                src.size
            );

        const uint64_t sectionEnd =
            sectionOffset +
            sectionSize;


        if (
            sectionOffset >
            static_cast<uint64_t>(data.size())
        )
        {
            std::printf(
                "[ELF] Section %u offset outside file\n",
                i
            );

            return false;
        }


        if (
            sectionEnd >
            static_cast<uint64_t>(data.size())
        )
        {
            std::printf(
                "[ELF] Section %u exceeds file\n",
                i
            );

            return false;
        }


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