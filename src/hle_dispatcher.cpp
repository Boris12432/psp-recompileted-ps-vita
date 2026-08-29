#include "hle_dispatcher.h"
#include "arm_cpu.h"

#include <cstdio>

HLEDispatcher& HLEDispatcher::instance()
{
    static HLEDispatcher dispatcher;

    return dispatcher;
}


void HLEDispatcher::registerFunction(
    uint32_t id,
    HLEFunction function
)
{
    functions[id] = function;
}


bool HLEDispatcher::call(
    uint32_t id,
    ARMCPU& cpu
)
{
    auto it =
        functions.find(id);

    if (it == functions.end())
    {
        std::printf(
            "[HLE] Unknown function: 0x%08X\n",
            id
        );

        cpu.r[0] =
            0xFFFFFFFFu;

        return false;
    }


    std::printf(
        "[HLE] Call: 0x%08X\n",
        id
    );

    it->second(cpu);

    return true;
}