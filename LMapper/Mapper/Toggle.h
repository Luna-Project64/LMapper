#pragma once

#include "Serialization.h"
#include "Mapping.h"

namespace Mapping
{
    class Toggler : public IMapper
    {
    public:
        Toggler() = default;
        Toggler(X360::IEventPtr event, N64::IModifierPtr on, N64::IModifierPtr off);
        virtual std::optional<Simple::Config> ToSimpleConfig() const override;
        virtual void Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to) override;
        virtual YAML::Node Serialize() const override;

        X360::IEventPtr event_;
        N64::IModifierPtr on_;
        N64::IModifierPtr off_;

        bool prevOn_ = false;
    };

    using TogglerPtr = std::unique_ptr<Toggler>;
}

namespace YAML
{

    template<>
    struct convert<Mapping::TogglerPtr>
    {
        static bool decode(const Node& node, Mapping::TogglerPtr&);
    };
}
