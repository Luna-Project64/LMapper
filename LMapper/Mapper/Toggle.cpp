#include "Toggle.h"
#include "SerializationImpl.h"

namespace Mapping
{
    Toggler::Toggler(X360::IEventPtr event, N64::IModifierPtr on, N64::IModifierPtr off)
        : event_(std::move(event))
        , on_(std::move(on))
        , off_(std::move(off))
    {
    }

    std::optional<Simple::Config> Toggler::ToSimpleConfig() const
    {
        auto button = event_->ToSimpleButton();
        if (!on_)
            return std::nullopt;

        auto onButton = on_->ToSimpleLunaRawButton();
        if (!onButton)
            return std::nullopt;

        if (off_)
        {
            auto offButton = off_->ToSimpleLunaRawButton();
            if (!offButton)
                return std::nullopt;

            if (offButton == Simple::LunaRawButton::LockFPS && onButton == Simple::LunaRawButton::UnlockFPS)
            {
                return Simple::ButtonMapping{ *button, Simple::ToButton::ToggleFPS };
            }

            return std::nullopt;
        }
        else
        {
            if (onButton == Simple::LunaRawButton::LoadState)
            {
                return Simple::ButtonMapping{ *button, Simple::ToButton::LoadState };
            }
            else if (onButton == Simple::LunaRawButton::SaveState)
            {
                return Simple::ButtonMapping{ *button, Simple::ToButton::SaveState };
            }

            return std::nullopt;
        }
    }

    void Toggler::Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to)
    {
        auto on = event_->Happened(from, keyboard);
        if (prevOn_ ^ on)
        {
            if (on)
            {
                if (on_)
                {
                    on_->Alter(to);
                }
            }
            else
            {
                if (off_)
                {
                    off_->Alter(to);
                }
            }
        }

        prevOn_ = on;
    }

    YAML::Node Toggler::Serialize() const
    {
        YAML::Node ret;
        ret["type"] = "toggle";
        ret["event"] = event_->Serialize();
        if (on_)
            ret["on"] = on_->Serialize();
        if (off_)
            ret["off"] = off_->Serialize();

        return ret;
    }
}

namespace YAML
{
    bool convert<Mapping::TogglerPtr>::decode(const Node& node, Mapping::TogglerPtr& t)
    {
        auto eventNode = node["event"];
        auto onNode = node["on"];
        auto offNode = node["off"];
        if (!eventNode)
            return false;

        auto on = onNode ? onNode.as<N64::IModifierPtr>() : nullptr;
        auto off = offNode ? offNode.as<N64::IModifierPtr>() : nullptr;
        auto event = eventNode.as<X360::IEventPtr>();

        t = std::make_unique<Mapping::Toggler>(std::move(event), std::move(on), std::move(off));
        return true;
    }
}