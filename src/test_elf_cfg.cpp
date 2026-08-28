#include <iostream>
#include <iomanip>

#include "elf_loader.h"
#include "arm_memory.h"
#include "arm_control_flow_graph.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr
            << "Usage: test_elf_cfg.exe <elf_file>\n";

        return 1;
    }

    const char* filename = argv[1];

    // ========================================================
    // ELF
    // ========================================================

    ELFLoader loader;

    if (!loader.load(filename))
    {
        std::cerr
            << "[FAIL] ELF loading\n";

        return 1;
    }

    std::cout
        << "[OK] ELF loaded\n";

    std::cout
        << "Entry: 0x"
        << std::hex
        << loader.entryPoint()
        << std::dec
        << "\n";


    // ========================================================
    // Memory
    // ========================================================

    SimpleMemory memory(
        16 * 1024 * 1024
    );


    // ========================================================
    // Load ELF segments
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
            << "Loaded @ 0x"
            << std::hex
            << segment.vaddr
            << " size=0x"
            << segment.data.size()
            << std::dec
            << "\n";
    }


    // ========================================================
    // Check entry instruction
    // ========================================================

    uint32_t entry =
        loader.entryPoint();

    uint32_t instruction =
        memory.read32(entry);

    std::cout
        << "Entry instruction: 0x"
        << std::hex
        << std::setw(8)
        << std::setfill('0')
        << instruction
        << std::dec
        << "\n";


    // ========================================================
    // CFG
    // ========================================================

    ARMControlFlowGraph cfg(
        memory
    );


    cfg.build(entry);


    std::cout
        << "[OK] CFG built\n";


    // ========================================================
    // Entry node
    // ========================================================

    const BasicBlock* node =
        cfg.getNode(entry);


    if (!node)
    {
        std::cerr
            << "[FAIL] Entry block not found\n";

        return 1;
    }


    std::cout
        << "[OK] Entry block found\n";


    std::cout
        << "Block start: 0x"
        << std::hex
        << node->startAddress
        << "\n";


    std::cout
        << "Block end:   0x"
        << node->endAddress
        << std::dec
        << "\n";


    std::cout
        << "Instructions: "
        << node->instructions.size()
        << "\n";


    return 0;
}