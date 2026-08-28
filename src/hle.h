#pragma once

#include <cstdint>

struct ARMCPU;

class HLE
{
public:

    static void handleSWI(
        ARMCPU& cpu,
        uint32_t swiNumber
    );

    static void initialize();
};