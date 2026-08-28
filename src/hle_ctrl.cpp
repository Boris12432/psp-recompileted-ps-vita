#include "hle_ctrl.h"
#include "hle_registry.h"
#include "arm_cpu.h"

#include <cstdio>


static void sceCtrlPeekBufferPositive(
    ARMCPU& cpu
)
{
    uint32_t port =
        cpu.r[0];

    uint32_t buffer =
        cpu.r[1];

    uint32_t count =
        cpu.r[2];

    std::printf(
        "[HLE][SceCtrl] "
        "sceCtrlPeekBufferPositive "
        "port=%u "
        "buffer=0x%08X "
        "count=%u\n",
        port,
        buffer,
        count
    );

    cpu.r[0] = 0;
}


void HLECtrl::initialize()
{
    HLERegistry::instance().registerFunction(
        "SceCtrl",
        0x00000000,
        sceCtrlPeekBufferPositive
    );
}