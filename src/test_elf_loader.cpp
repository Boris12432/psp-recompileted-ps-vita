#include <iostream>
#include <cassert>
#include <cstdint>

#include "elf_loader.h"
#include "arm_cpu.h"
#include "arm_engine.h"
#include "arm_memory.h"


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr
            << "Usage: test_elf_loader.exe <elf_file>\n";

        return 1;
    }


    const char* filename = argv[1];


    // ========================================================
    // ELF
    // ========================================================

    ELFLoader loader;

    bool result =
        loader.load(filename);

    assert(result);

    std::cout
        << "[OK] ELF loaded\n";


    std::cout
        << "Entry point: 0x"
        << std::hex
        << loader.entryPoint()
        << std::dec
        << "\n";


    std::cout
        << "Segments: "
        << loader.segments().size()
        << "\n";


    // ========================================================
    // Memory
    // ========================================================

    SimpleMemory memory(
        16 * 1024 * 1024
    );


    // ========================================================
    // Load segments
    // ========================================================

    for (const auto& segment :
         loader.segments())
    {
        if (segment.data.empty())
            continue;


        memory.load(
            segment.vaddr,
            segment.data.data(),
            static_cast<uint32_t>(
                segment.data.size()
            )
        );


        std::cout
            << "Loaded segment @ 0x"
            << std::hex
            << segment.vaddr
            << " size=0x"
            << segment.data.size()
            << std::dec
            << "\n";
    }


    std::cout
        << "[OK] Segments loaded into memory\n";


    // ========================================================
    // Check entry instruction
    // ========================================================

    uint32_t entry =
        loader.entryPoint();


    uint32_t instruction =
        memory.read32(entry);


    std::cout
        << "Instruction @ entry: 0x"
        << std::hex
        << instruction
        << std::dec
        << "\n";


    // ========================================================
    // ARM CPU
    // ========================================================

    ARMCPU cpu;


    cpu.r[15] =
        entry;


    // ARM state
    cpu.T =
        false;


    std::cout
        << "Initial PC: 0x"
        << std::hex
        << cpu.r[15]
        << std::dec
        << "\n";


    // ========================================================
    // Engine
    // ========================================================

    ARMEngine engine(
        cpu,
        memory
    );


    // ========================================================
    // Execute one instruction
    // ========================================================

    bool stepResult =
        engine.step();


    std::cout
        << "step() returned: "
        << stepResult
        << "\n";


    std::cout
        << "PC after step: 0x"
        << std::hex
        << cpu.r[15]
        << std::dec
        << "\n";


    std::cout
        << "R0 = 0x"
        << std::hex
        << cpu.r[0]
        << "\n";


    std::cout
        << "R1 = 0x"
        << cpu.r[1]
        << "\n";


    std::cout
        << "R2 = 0x"
        << cpu.r[2]
        << "\n";


    std::cout
        << "R3 = 0x"
        << cpu.r[3]
        << std::dec
        << "\n";


    std::cout
        << "[OK] ARMEngine executed entry instruction\n";


    return 0;
}