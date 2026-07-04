#include "Mapping.h"

#include <unordered_set>

#include "ControllerInterfaceImpl.h"
#include "MappingImpl.h"
#include "Switch.h"
#include "Group.h"
#include "Toggle.h"

namespace Mapping
{
    namespace Digital
    {
        Mapper::Mapper(X360::IEventPtr from, N64::IModifierPtr to) : event_(std::move(from)), modifier_(std::move(to)) {}

        std::optional<Simple::Config> Mapper::ToSimpleConfig() const
        {
            auto eventDesc = event_->ToSimpleButton();
            if (!eventDesc)
                return std::nullopt;

            auto modifierDesc = modifier_->ToSimpleButton();
            if (!modifierDesc)
                return std::nullopt;

            return Simple::ButtonMapping{ .from = *eventDesc, .to = *modifierDesc };
        }

        void Mapper::Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to)
        {
            if (event_->Happened(from, keyboard))
                modifier_->Alter(to);
        }

        YAML::Node Mapper::Serialize() const
        {
            YAML::Node node;
            node["from"] = event_;
            node["to"] = modifier_;
            return node;
        }
    }

    void Map(const Mappers& mappers, const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to)
    {
        for (auto& mapper : mappers)
            mapper->Map(from, keyboard, to);
    }
}

namespace YAML
{
    Node convert<Mapping::IMapperPtr>::encode(const Mapping::IMapperPtr& mapper)
    {
        return mapper->Serialize();
    }

    bool convert<Mapping::IMapperPtr>::decode(const Node& node, Mapping::IMapperPtr& mapper)
    {
        if (!node.IsMap())
            return false;

        auto typeNode = node["type"];
        if (!typeNode)
        {
            // This is button mapping that does not have the type
            mapper = node.as<Mapping::Digital::MapperPtr>();
            return true;
        }
        else
        {
            auto type = typeNode.as<std::string>();
            if (type == "bilinear")
            {
                mapper = node.as<Mapping::Analog::BilinearStickMapperPtr>();
                return true;
            }
            else if (type == "linear")
            {
                auto fromNode = node["from"];
                if (!fromNode || !fromNode.IsMap())
                    return false;

                auto fromOffsetNode = fromNode["offset"];
                if (!fromOffsetNode || !fromOffsetNode.IsScalar())
                    return false;

                auto fromOffset = fromOffsetNode.as<std::string>();
                if (convert<X360::Thumbs>::is(fromOffset))
                {
                    mapper = node.as<Mapping::Analog::LinearStickMapperPtr>();
                    return true;
                }
                else if (convert<X360::Triggers>::is(fromOffset))
                {
                    mapper = node.as<Mapping::Analog::LinearTriggerMapperPtr>();
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if (type == "switch")
            {
                mapper = node.as<Mapping::SwitcherPtr>();
                return true;
            }
            else if (type == "group")
            {
                mapper = node.as<Mapping::GroupPtr>();
                return true;
            }
            else if (type == "toggle")
            {
                mapper = node.as<Mapping::TogglerPtr>();
                return true;
            }
        }
        return false;
    }

    bool convert<Mapping::Digital::MapperPtr>::decode(const Node& node, Mapping::Digital::MapperPtr& mapper)
    {
        // TODO: Impl direct simple button mapping
        if (!node.IsMap())
            return false;

        auto fromNode = node["from"];
        auto toNode = node["to"];

        if (!fromNode || !toNode)
            return false;

        auto event = fromNode.as<X360::IEventPtr>();
        auto modifier = toNode.as<N64::IModifierPtr>();

        mapper = std::make_unique<Mapping::Digital::Mapper>(std::move(event), std::move(modifier));
        return true;
    }
}
