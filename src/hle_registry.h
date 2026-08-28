#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

struct ARMCPU;

using HLEFunction =
    void (*)(ARMCPU&);


class HLERegistry
{
public:

    static HLERegistry& instance();

    void registerFunction(
        const std::string& module,
        uint32_t nid,
        HLEFunction function
    );

    HLEFunction find(
        const std::string& module,
        uint32_t nid
    ) const;


private:

    struct Key
    {
        std::string module;
        uint32_t nid;

        bool operator==(
            const Key& other
        ) const
        {
            return
                module == other.module &&
                nid == other.nid;
        }
    };


    struct KeyHash
    {
        size_t operator()(
            const Key& key
        ) const
        {
            return
                std::hash<std::string>{}(
                    key.module
                )
                ^
                (
                    std::hash<uint32_t>{}(
                        key.nid
                    )
                    << 1
                );
        }
    };


    std::unordered_map<
        Key,
        HLEFunction,
        KeyHash
    > functions;
};