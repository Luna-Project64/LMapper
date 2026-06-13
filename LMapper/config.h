#pragma once

#include "Mapper/Mapping.h"

class Config
{
public:
    static constexpr char AllowedEnabledControllerMask = 0xf;

    Mapping::Mappers mappers;
    char enabledControllersMask = 0;
};

namespace YAML
{
    template<>
    struct convert<Config>
    {
        static Node encode(const Config&);
        static bool decode(const Node& node, Config&);
    };
}