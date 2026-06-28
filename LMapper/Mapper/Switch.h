#pragma once

#include "Serialization.h"
#include "Mapping.h"

namespace Mapping
{
    enum class SwitchCondition
    {
        PressNext,
        HoldEnable,
        HoldNext,
    };

    class Switcher : public IMapper
    {
    public:
        Switcher() = default;
        Switcher(SwitchCondition cond, X360::IEventPtr event_, std::vector<IMapperPtr> mappers);

        virtual std::optional<Simple::Config> ToSimpleConfig() const override;
        virtual void Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to) override;
        virtual YAML::Node Serialize() const override;

        SwitchCondition condition_;
        X360::IEventPtr event_;
        std::vector<IMapperPtr> mappers_;

        bool prevOn_ = false;
        int curMapper_ = 0;
    };

    using SwitcherPtr = std::unique_ptr<Switcher>;
}

namespace YAML
{
    template<>
    struct convert<Mapping::SwitchCondition>
    {
        using Serializer = Serialization::EnumSerializer<Mapping::SwitchCondition>;

        static const std::map<std::string, Mapping::SwitchCondition> names;

        static Node encode(const enum Mapping::SwitchCondition& thumb);
        static bool decode(const Node& node, enum Mapping::SwitchCondition& thumb);

        static bool is(std::string& name);
    };

    template<>
    struct convert<Mapping::SwitcherPtr>
    {
        static bool decode(const Node& node, Mapping::SwitcherPtr&);
    };
}
