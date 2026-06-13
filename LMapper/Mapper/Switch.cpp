#include "Switch.h"
#include "SerializationImpl.h"

namespace Mapping
{
    Switcher::Switcher(SwitchCondition c, X360::IEventPtr ev, std::vector<IMapperPtr> m)
        : condition_(c)
        , event_(std::move(ev))
        , mappers_(std::move(m))
    { }

    std::optional<IMapper::StringDescription> Switcher::ToString() const
    {
        return std::nullopt;
    }

    void Switcher::Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to)
    {
        auto on = event_->Happened(from, keyboard);
        switch (condition_)
        {
        case Mapping::SwitchCondition::PressNext:
            if (on && !prevOn_)
                curMapper_++;
            
            break;
        case Mapping::SwitchCondition::HoldEnable:
            curMapper_ = on ? 1 : 0;

            break;
        case Mapping::SwitchCondition::HoldNext:
            if (on)
                curMapper_++;

            break;
        }

        prevOn_ = on;
        mappers_[curMapper_ % mappers_.size()]->Map(from, keyboard, to);
    }

    YAML::Node Switcher::Serialize() const
    {
        YAML::Node ret;
        for (auto& mapper : mappers_)
        {
            ret.push_back(mapper->Serialize());
        }

        return ret;
    }
}

namespace YAML
{
    const std::map<std::string, Mapping::SwitchCondition> convert<Mapping::SwitchCondition>::names
    {
#define ENUMSTR(name) { #name, Mapping::SwitchCondition::name } 
        ENUMSTR(PressNext),
        ENUMSTR(HoldEnable),
        ENUMSTR(HoldNext),
#undef ENUMSTR
    };

    Node convert<Mapping::SwitchCondition>::encode(const enum Mapping::SwitchCondition& s)
    {
        return Serializer::Encode(names, s);
    }

    bool convert<Mapping::SwitchCondition>::decode(const Node& node, enum Mapping::SwitchCondition& s)
    {
        return Serializer::Decode(names, node, s);
    }

    bool convert<Mapping::SwitchCondition>::is(std::string& name)
    {
        return names.find(name) != names.end();
    }

    bool convert<Mapping::SwitcherPtr>::decode(const Node& node, Mapping::SwitcherPtr& s)
    {
        auto conditionNode = node["condition"];
        auto switchesNode = node["switches"];
        auto eventNode = node["event"]; 
        if (!conditionNode || !switchesNode || !eventNode)
            return false;

        auto cond = conditionNode.as<Mapping::SwitchCondition>();
        auto switches = switchesNode.as<std::vector<Mapping::IMapperPtr>>();
        auto event = eventNode.as<X360::IEventPtr>();

        s = std::make_shared<Mapping::Switcher>(cond, event, switches);
        return true;
    }
}