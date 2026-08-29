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
    std::printf(
        "[HLE] Test function\n"
    );

    /*
     * ARM ABI:
     *
     * R0-R3 = arguments
     * R0    = return value
     */

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
    /*
     * HLE::initialize() previously had to be called manually by
     * whichever program used the interpreter, and nothing in the
     * codebase actually called it - meaning the HLE function table
     * was always empty at runtime and every SWI silently fell
     * through to "unknown function".
     *
     * Guarantee initialization happens exactly once, on first use,
     * regardless of which entry point (main, tests, future driver)
     * is running.
     */

    static bool initialized = false;

    if (!initialized)
    {
        initialized = true;
        HLE::initialize();
    }

    HLEDispatcher::instance().call(
        swiNumber,
        cpu
    );
}