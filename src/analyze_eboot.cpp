#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>

static uint32_t read32le(
    const std::vector<uint8_t>& data,
    size_t offset
)
{
    if (offset + 4 > data.size())
        return 0;

    return
        static_cast<uint32_t>(data[offset]) |
        (static_cast<uint32_t>(data[offset + 1]) << 8) |
        (static_cast<uint32_t>(data[offset + 2]) << 16) |
        (static_cast<uint32_t>(data[offset + 3]) << 24);
}


static uint32_t read32be(
    const std::vector<uint8_t>& data,
    size_t offset
)
{
    if (offset + 4 > data.size())
        return 0;

    return
        (static_cast<uint32_t>(data[offset]) << 24) |
        (static_cast<uint32_t>(data[offset + 1]) << 16) |
        (static_cast<uint32_t>(data[offset + 2]) << 8) |
        static_cast<uint32_t>(data[offset + 3]);
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr
            << "Usage: "
            << (argc > 0 ? argv[0] : "analyze_eboot")
            << " <path-to-eboot.bin>\n";

        return 1;
    }

    const char* filename =
        argv[1];

    std::ifstream file(
        filename,
        std::ios::binary
    );

    if (!file)
    {
        std::cerr
            << "Cannot open "
            << filename
            << "\n";

        return 1;
    }


    file.seekg(
        0,
        std::ios::end
    );

    size_t size =
        static_cast<size_t>(
            file.tellg()
        );

    file.seekg(0);


    std::vector<uint8_t> data(size);

    file.read(
        reinterpret_cast<char*>(data.data()),
        size
    );


    std::cout
        << "Size: "
        << size
        << " bytes\n";

    std::cout
        << "Size: 0x"
        << std::hex
        << size
        << std::dec
        << "\n\n";


    // --------------------------------------------------------
    // Search for ELF magic
    // --------------------------------------------------------

    std::cout
        << "Searching for ELF...\n";


    size_t elfCount = 0;

    for (size_t i = 0;
         i + 4 <= data.size();
         ++i)
    {
        if (data[i]     == 0x7F &&
            data[i + 1] == 'E'  &&
            data[i + 2] == 'L'  &&
            data[i + 3] == 'F')
        {
            std::cout
                << "ELF found at 0x"
                << std::hex
                << i
                << std::dec
                << "\n";

            ++elfCount;
        }
    }


    if (elfCount == 0)
    {
        std::cout
            << "No ELF signature found.\n";
    }


    // --------------------------------------------------------
    // Search for ASCII strings
    // --------------------------------------------------------

    std::cout
        << "\nSearching for interesting strings...\n";


    const char* strings[] =
    {
        "SCE",
        "sce",
        "SELF",
        "ELF",
        "ARM",
        "VITA",
        "PSV",
        "PCSE00120"
    };


    for (const char* target : strings)
    {
        size_t length =
            std::strlen(target);

        for (size_t i = 0;
             i + length <= data.size();
             ++i)
        {
            if (std::memcmp(
                    &data[i],
                    target,
                    length) == 0)
            {
                std::cout
                    << target
                    << " found at 0x"
                    << std::hex
                    << i
                    << std::dec
                    << "\n";
            }
        }
    }


    // --------------------------------------------------------
    // Print 32-bit values at beginning
    // --------------------------------------------------------

    std::cout
        << "\nFirst 32-bit values:\n";


    for (size_t i = 0;
         i < 64 && i + 4 <= data.size();
         i += 4)
    {
        uint32_t le =
            read32le(data, i);

        uint32_t be =
            read32be(data, i);


        std::cout
            << "0x"
            << std::hex
            << std::setw(8)
            << std::setfill('0')
            << i
            << "  LE="
            << std::setw(8)
            << le
            << "  BE="
            << std::setw(8)
            << be
            << std::dec
            << "\n";
    }


    return 0;
}