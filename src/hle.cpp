#include "hle.h"
#include "hle_dispatcher.h"
#include "arm_cpu.h"
#include "hle_ctrl.h"

#include <cstdio>


// ============================================================
// Test HLE function
// ============================================================

static void hleTest(
    ARMCPU& cpu
)
{

    uint32_t value =
        cpu.r[0];

    cpu.r[0] =
        value + 1;
}


// ============================================================
// Initialize
// ============================================================

void HLE::initialize()
{
    auto& dispatcher =
        HLEDispatcher::instance();


    /*
     * Temporary test function.
     *
     * Later these IDs will become
     * Vita module/function mappings.
     */

    dispatcher.registerFunction(
        0x00000001,
        hleTest
    );
    HLECtrl::initialize();
}


// ============================================================
// SWI
// ============================================================

void HLE::handleSWI(
    ARMCPU& cpu,
    uint32_t swiNumber
)
{
    HLEDispatcher::instance().call(
        swiNumber,
        cpu
    );
}