#pragma once

#include <cstdint>
#include <unordered_map>

struct ARMCPU;

using HLEFunction =
    void (*)(ARMCPU& cpu);

class HLEDispatcher
{
public:

    static HLEDispatcher& instance();

    void registerFunction(
        uint32_t id,
        HLEFunction function
    );

    bool call(
        uint32_t id,
        ARMCPU& cpu
    );

private:

    std::unordered_map<
        uint32_t,
        HLEFunction
    > functions;
};