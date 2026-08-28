#include "elf_loader.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <cstring>


namespace
{

// ============================================================
// ELF32 structures
// ============================================================

struct Elf32_Ehdr
{
    uint8_t  e_ident[16];

    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;

    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;

    uint32_t e_flags;

    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;

    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};


struct Elf32_Phdr
{
    uint32_t p_type;

    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;

    uint32_t p_filesz;
    uint32_t p_memsz;

    uint32_t p_flags;
    uint32_t p_align;
};


// ============================================================
// ELF constants
// ============================================================

constexpr uint8_t ELF_MAGIC_0 = 0x7F;
constexpr uint8_t ELF_MAGIC_1 = 'E';
constexpr uint8_t ELF_MAGIC_2 = 'L';
constexpr uint8_t ELF_MAGIC_3 = 'F';

constexpr uint8_t ELFCLASS32 = 1;
constexpr uint8_t ELFDATA2LSB = 1;

constexpr uint16_t EM_ARM = 40;

constexpr uint32_t PT_LOAD = 1;

constexpr uint32_t PF_X = 1;
constexpr uint32_t PF_W = 2;
constexpr uint32_t PF_R = 4;


// ============================================================
// Read complete file
// ============================================================

bool readFile(
    const std::string& filename,
    std::vector<uint8_t>& data
)
{
    std::ifstream file(
        filename,
        std::ios::binary
    );

    if (!file)
        return false;


    file.seekg(
        0,
        std::ios::end
    );

    std::streamoff size =
        file.tellg();

    if (size < 0)
        return false;


    file.seekg(
        0,
        std::ios::beg
    );


    data.resize(
        static_cast<size_t>(size)
    );


    if (size != 0)
    {
        file.read(
            reinterpret_cast<char*>(
                data.data()
            ),
            size
        );

        if (!file)
            return false;
    }


    return true;
}


// ============================================================
// Bounds check
// ============================================================

template<typename T>
bool canRead(
    const std::vector<uint8_t>& data,
    uint64_t offset
)
{
    return
        offset <= data.size() &&
        sizeof(T) <=
            data.size() - offset;
}


} // namespace


// ============================================================
// ELFLoader::load
// ============================================================

bool ELFLoader::load(
    const std::string& filename
)
{
    entry = 0;

    loadedSegments.clear();


    // --------------------------------------------------------
    // Read file
    // --------------------------------------------------------

    std::vector<uint8_t> file;

    if (!readFile(
            filename,
            file))
    {
        return false;
    }


    // --------------------------------------------------------
    // ELF header
    // --------------------------------------------------------

    if (!canRead<Elf32_Ehdr>(
            file,
            0))
    {
        return false;
    }


    Elf32_Ehdr ehdr{};

    std::memcpy(
        &ehdr,
        file.data(),
        sizeof(ehdr)
    );


    // --------------------------------------------------------
    // ELF magic
    // --------------------------------------------------------

    if (
        ehdr.e_ident[0] != ELF_MAGIC_0 ||
        ehdr.e_ident[1] != ELF_MAGIC_1 ||
        ehdr.e_ident[2] != ELF_MAGIC_2 ||
        ehdr.e_ident[3] != ELF_MAGIC_3
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // ELF32
    // --------------------------------------------------------

    if (
        ehdr.e_ident[4] != ELFCLASS32
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // Little endian
    // --------------------------------------------------------

    if (
        ehdr.e_ident[5] != ELFDATA2LSB
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // ARM
    // --------------------------------------------------------

    if (
        ehdr.e_machine != EM_ARM
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // Program header validation
    // --------------------------------------------------------

    if (ehdr.e_phnum == 0)
    {
        return false;
    }


    uint64_t phTableEnd =
        static_cast<uint64_t>(
            ehdr.e_phoff
        ) +
        static_cast<uint64_t>(
            ehdr.e_phentsize
        ) *
        static_cast<uint64_t>(
            ehdr.e_phnum
        );


    if (
        ehdr.e_phoff > file.size() ||
        phTableEnd > file.size()
    )
    {
        return false;
    }


    if (
        ehdr.e_phentsize <
        sizeof(Elf32_Phdr)
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // Entry point
    // --------------------------------------------------------

    entry =
        ehdr.e_entry;


    // --------------------------------------------------------
    // Load PT_LOAD segments
    // --------------------------------------------------------

    for (
        uint16_t i = 0;
        i < ehdr.e_phnum;
        ++i
    )
    {
        uint64_t offset =
            static_cast<uint64_t>(
                ehdr.e_phoff
            ) +
            static_cast<uint64_t>(
                i
            ) *
            ehdr.e_phentsize;


        Elf32_Phdr phdr{};

        std::memcpy(
            &phdr,
            file.data() + offset,
            sizeof(phdr)
        );


        // ----------------------------------------------------
        // Ignore non-loadable segments
        // ----------------------------------------------------

        if (
            phdr.p_type != PT_LOAD
        )
        {
            continue;
        }


        // ----------------------------------------------------
        // Validate sizes
        // ----------------------------------------------------

        if (
            phdr.p_memsz <
            phdr.p_filesz
        )
        {
            return false;
        }


        uint64_t fileEnd =
            static_cast<uint64_t>(
                phdr.p_offset
            ) +
            static_cast<uint64_t>(
                phdr.p_filesz
            );


        if (
            fileEnd >
            file.size()
        )
        {
            return false;
        }


        // ----------------------------------------------------
        // Create segment
        // ----------------------------------------------------

        LoadedSegment segment;

        segment.vaddr =
            phdr.p_vaddr;

        segment.size =
            phdr.p_memsz;

        segment.flags =
            phdr.p_flags;


        // ----------------------------------------------------
        // Allocate complete memory segment
        //
        // Important:
        //
        // p_filesz = data stored in ELF
        // p_memsz  = actual memory size
        //
        // The remaining bytes are .bss and stay zero.
        // ----------------------------------------------------

        segment.data.resize(
            phdr.p_memsz,
            0
        );


        if (
            phdr.p_filesz != 0
        )
        {
            std::memcpy(
                segment.data.data(),
                file.data() + phdr.p_offset,
                phdr.p_filesz
            );
        }


        loadedSegments.push_back(
            std::move(segment)
        );
    }


    // --------------------------------------------------------
    // Must have at least one PT_LOAD
    // --------------------------------------------------------

    if (
        loadedSegments.empty()
    )
    {
        return false;
    }


    return true;
}


// ============================================================
// loadIntoMemory
// ============================================================

bool loadIntoMemory(
    const std::string& filename,
    ARMMemory& memory
)
{
    ELFLoader loader;


    if (!loader.load(
            filename))
    {
        return false;
    }


    for (
        const LoadedSegment& segment :
        loader.segments()
    )
    {
        if (
            segment.data.empty()
        )
        {
            continue;
        }


        for (
            uint32_t i = 0;
            i < segment.data.size();
            ++i
        )
        {
            memory.write8(
                segment.vaddr + i,
                segment.data[i]
            );
        }
    }


    return true;
}