#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <fstream>

#include "arm_cpu.h"
#include "arm_decoder.h"
#include "arm_interpreter.h"
#include "hle.h"
#include "arm_engine.h"

static void printCPU(const ARMCPU& cpu)
{
    for (int i = 0; i < 16; ++i) {

        printf(
            "R%-2d = %08X\n",
            i,
            cpu.r[i]
        );
    }

    std::printf(
        "N=%d Z=%d C=%d V=%d T=%d\n",
        cpu.N,
        cpu.Z,
        cpu.C,
        cpu.V,
        cpu.T
    );

    std::printf(
        "CPSR = %08X\n",
        cpu.cpsr()
    );
}

// ============================================================
// Load file
// ============================================================

static bool loadFile(
    const char* path,
    std::vector<uint8_t>& data
)
{
    std::ifstream file(
        path,
        std::ios::binary |
        std::ios::ate
    );

    if (!file)
    {
        std::printf(
            "ERROR: cannot open file: %s\n",
            path
        );

        return false;
    }

    std::streamsize size =
        file.tellg();

    if (size <= 0)
    {
        std::printf(
            "ERROR: empty file\n"
        );

        return false;
    }

    file.seekg(
        0,
        std::ios::beg
    );

    data.resize(
        static_cast<size_t>(size)
    );

    if (!file.read(
        reinterpret_cast<char*>(data.data()),
        size
    ))
    {
        std::printf(
            "ERROR: cannot read file\n"
        );

        return false;
    }

    std::printf(
        "Loaded: %s (%lld bytes)\n",
        path,
        static_cast<long long>(size)
    );

    return true;
}

int main(int argc, char* argv[])
{
    std::printf(
        "=== Vita ARM -> PSP recompiler ===\n"
    );

    HLE::initialize();

    std::printf("HLE initialized\n");

      // --------------------------------------------------------
    // Check arguments
    // --------------------------------------------------------

    if (argc < 2)
    {
        std::printf(
            "\nUsage:\n"
            "  arm2mips.exe <binary>\n\n"
            "Example:\n"
            "  arm2mips.exe eboot.bin\n"
        );

        return 1;
    }


    // --------------------------------------------------------
    // Load file
    // --------------------------------------------------------

    std::vector<uint8_t> fileData;

    if (!loadFile(
        argv[1],
        fileData
    ))
    {
        return 1;
    }


    // --------------------------------------------------------
    // Memory
    //
    // Пока временно грузим файл
    // с адреса 0.
    //
    // Позже ELF loader будет
    // использовать реальные Vita
    // виртуальные адреса.
    // --------------------------------------------------------

    SimpleMemory memory;

    memory.map(
        0x00000000,
        static_cast<uint32_t>(
            fileData.size()
        ),
        true,
        true,
        true
    );

    memory.load(
        0x00000000,
        fileData.data(),
        static_cast<uint32_t>(
            fileData.size()
        )
    );


    // --------------------------------------------------------
    // CPU
    // --------------------------------------------------------

    ARMCPU cpu;


    // --------------------------------------------------------
    // Пока временно:
    // entry point = 0
    //
    // ELF loader потом заменит это
    // на настоящий entry point.
    // --------------------------------------------------------

    cpu.r[15] =
        0x00000000;


    // --------------------------------------------------------
    // ARM state
    // --------------------------------------------------------

    cpu.T =
        false;


    // --------------------------------------------------------
    // Engine
    // --------------------------------------------------------

    ARMEngine engine(
        cpu,
        memory
    );


    // --------------------------------------------------------
    // Execute
    //
    // Пока ограничиваем количество
    // инструкций, чтобы случайный бинарник
    // не улетел в бесконечный цикл.
    // --------------------------------------------------------

    std::printf(
        "\nStarting execution...\n"
    );

    engine.run(
        1000
    );

    // --------------------------------------------------------
    // CPU state
    // --------------------------------------------------------

    std::printf(
        "\n=== CPU STATE ===\n"
    );

    printCPU(
        cpu
    );

    getchar();

    return 0;
}