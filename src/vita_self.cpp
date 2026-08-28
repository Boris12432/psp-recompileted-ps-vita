#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>

static uint32_t u32le(
    const std::vector<uint8_t>& d,
    size_t o
)
{
    if (o + 4 > d.size())
        return 0;

    return
        uint32_t(d[o]) |
        (uint32_t(d[o + 1]) << 8) |
        (uint32_t(d[o + 2]) << 16) |
        (uint32_t(d[o + 3]) << 24);
}

static uint64_t u64le(
    const std::vector<uint8_t>& d,
    size_t o
)
{
    uint64_t lo = u32le(d, o);
    uint64_t hi = u32le(d, o + 4);

    return lo | (hi << 32);
}

static void dump32(
    const std::vector<uint8_t>& d,
    size_t offset,
    size_t count
)
{
    for (size_t i = 0; i < count; ++i)
    {
        size_t o = offset + i * 4;

        if (o + 4 > d.size())
            break;

        std::cout
            << "0x"
            << std::hex
            << std::setw(8)
            << std::setfill('0')
            << o
            << " : "
            << std::setw(8)
            << u32le(d, o)
            << std::dec
            << '\n';
    }
}

int main()
{
    const char* filename =
        "C:\\GAMES\\PCSE00120\\eboot.bin";

    std::ifstream f(
        filename,
        std::ios::binary
    );

    if (!f)
    {
        std::cerr
            << "Cannot open eboot.bin\n";

        return 1;
    }

    f.seekg(0, std::ios::end);

    size_t size =
        static_cast<size_t>(f.tellg());

    f.seekg(0);

    std::vector<uint8_t> data(size);

    f.read(
        reinterpret_cast<char*>(data.data()),
        data.size()
    );

    std::cout
        << "File size: 0x"
        << std::hex
        << size
        << std::dec
        << "\n\n";


    // ========================================================
    // SCE header
    // ========================================================

    std::cout
        << "=== SCE HEADER ===\n";

    uint32_t magic =
        u32le(data, 0);

    std::cout
        << "magic       = 0x"
        << std::hex
        << magic
        << std::dec
        << '\n';


    /*
     * Vita SELF begins with SCE magic.
     *
     * If this isn't recognized, don't attempt to interpret
     * the rest as a normal ELF.
     */

    std::cout
        << "First 0x100 bytes:\n";

    dump32(
        data,
        0,
        64
    );


    // ========================================================
    // Search possible SELF / ELF structures
    // ========================================================

    std::cout
        << "\n=== MAGIC SEARCH ===\n";


    for (size_t i = 0;
         i + 4 <= data.size();
         ++i)
    {
        uint32_t v =
            u32le(data, i);

        if (v == 0x464C457F)
        {
            std::cout
                << "ELF magic LE at 0x"
                << std::hex
                << i
                << std::dec
                << '\n';
        }

        if (v == 0x00454353)
        {
            std::cout
                << "ASCII SCE-like value at 0x"
                << std::hex
                << i
                << std::dec
                << '\n';
        }
    }


    return 0;
}