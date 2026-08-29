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
    //
    // IMPORTANT: this arithmetic must happen in 64-bit to avoid
    // signed/unsigned 32-bit overflow on a hostile or corrupted
    // header (sectionHeaderCount/EntrySize are attacker-controlled
    // uint16_t values whose product can exceed INT_MAX).
    // --------------------------------------------------------

    if (
        header->sectionHeaderEntrySize <
        sizeof(ELF32SectionHeader)
    )
    {
        std::printf(
            "[ELF] Invalid section header entry size\n"
        );

        return false;
    }


    uint64_t sectionTableEnd =
        static_cast<uint64_t>(
            header->sectionHeaderOffset
        ) +
        static_cast<uint64_t>(
            header->sectionHeaderCount
        ) *
        static_cast<uint64_t>(
            header->sectionHeaderEntrySize
        );


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


    // NOTE: section headers are addressed by byte offset using
    // the *file-declared* entry size (sectionHeaderEntrySize),
    // not sizeof(ELF32SectionHeader). Plain array indexing
    // (sections[i]) is only correct when the two happen to
    // match; since a corrupted/hostile file can legally declare
    // a larger entry size, we must stride manually.

    const uint8_t* sectionBase =
        data.data() +
        header->sectionHeaderOffset;

    auto sectionAt =
        [&](uint16_t index) -> const ELF32SectionHeader&
    {
        return *reinterpret_cast<const ELF32SectionHeader*>(
            sectionBase +
            static_cast<uint64_t>(index) *
                header->sectionHeaderEntrySize
        );
    };


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
        sectionAt(
            header->stringTableIndex
        );


    if (
        static_cast<uint64_t>(stringSection.offset) +
        static_cast<uint64_t>(stringSection.size) >
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
            sectionAt(i);


        ELFSection section;


        if (
            src.name <
            stringSection.size
        )
        {
            // Do not trust the string table to be
            // NUL-terminated within its declared bounds -
            // constructing a std::string directly from a
            // possibly-unterminated char* can read past the
            // buffer. Bound the search explicitly.

            const char* nameStart =
                strings + src.name;

            size_t maxLen =
                stringSection.size - src.name;

            size_t len =
                strnlen(nameStart, maxLen);

            section.name =
                std::string(nameStart, len);
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