#include "hle_registry.h"


HLERegistry&
HLERegistry::instance()
{
    static HLERegistry registry;

    return registry;
}


void HLERegistry::registerFunction(
    const std::string& module,
    uint32_t nid,
    HLEFunction function
)
{
    Key key{
        module,
        nid
    };

    functions[key] =
        function;
}


HLEFunction HLERegistry::find(
    const std::string& module,
    uint32_t nid
) const
{
    Key key{
        module,
        nid
    };

    auto it =
        functions.find(key);

    if (it == functions.end())
        return nullptr;

    return it->second;
}