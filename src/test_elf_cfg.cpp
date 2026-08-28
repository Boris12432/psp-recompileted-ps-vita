#include <cassert>
#include <cstdint>
#include <iostream>

#include "elf_loader.h"
#include "arm_memory.h"
#include "arm_control_flow_graph.h"

int main()
{
    const char* filename = "nano";

    // --------------------------------------------------------
    // ELF
    // --------------------------------------------------------

    ELFLoader loader;

    assert(loader.load(filename));

    std::cout
        << "[OK] ELF loaded\n";

    std::cout
        << "Entry: 0x"
        << std::hex
        << loader.entryPoint()
        << std::dec
        << "\n";


    // --------------------------------------------------------
    // Memory
    // --------------------------------------------------------

    SimpleMemory memory;

    for (const auto& segment : loader.segments())
    {
        memory.map(
            segment.vaddr,
            segment.size,
            (segment.flags & 4) != 0,
            (segment.flags & 2) != 0,
            (segment.flags & 1) != 0
        );

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
            << segment.size
            << " flags=0x"
            << segment.flags
            << std::dec
            << "\n";
    }


    std::cout
        << "[OK] Segments loaded\n";


    // --------------------------------------------------------
    // CFG
    // --------------------------------------------------------

    ARMControlFlowGraph cfg(memory);

    cfg.build(
        loader.entryPoint()
    );


    std::cout
        << "[OK] CFG built\n";


    // --------------------------------------------------------
    // Entry block
    // --------------------------------------------------------

    const BasicBlock* entry =
        cfg.getNode(
            loader.entryPoint()
        );


    assert(entry != nullptr);


    std::cout
        << "Entry block:\n";

    std::cout
        << "  start = 0x"
        << std::hex
        << entry->startAddress
        << "\n";

    std::cout
        << "  end   = 0x"
        << entry->endAddress
        << "\n";

    std::cout
        << "  instructions = "
        << std::dec
        << entry->instructions.size()
        << "\n";


    for (const auto& ir : entry->instructions)
    {
        std::cout
            << "    IR @ 0x"
            << std::hex
            << ir.address
            << "\n";
    }


    std::cout
        << "[OK] Entry block exists\n";


    return 0;
}