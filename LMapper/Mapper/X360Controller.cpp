#include "X360Controller.h"
#include "ControllerInterfaceImpl.h"
#include "Keyboard.h"
#include "SerializationImpl.h"

#define THUMBS \
    ENUMSTR(LeftX) \
    ENUMSTR(LeftY) \
    ENUMSTR(RightX) \
    ENUMSTR(RightY)

#define TRIGGERS \
    ENUMSTR(LeftTrigger) \
    ENUMSTR(RightTrigger)

#define BUTTONS \
    ENUMSTR(DpadUp) \
    ENUMSTR(DpadDown) \
    ENUMSTR(DpadLeft) \
    ENUMSTR(DpadRight) \
    ENUMSTR(Start) \
    ENUMSTR(Back) \
    ENUMSTR(LeftThumb) \
    ENUMSTR(RightThumb) \
    ENUMSTR(L) \
    ENUMSTR(R) \
    ENUMSTR(Guide) \
    ENUMSTR(A) \
    ENUMSTR(B) \
    ENUMSTR(X) \
    ENUMSTR(Y)

namespace YAML
{
    const std::map<std::string, X360::Thumbs> convert<X360::Thumbs>::names
    {
#define ENUMSTR(name) { #name, X360::Thumbs::name },
        THUMBS
#undef ENUMSTR
    };

    const std::map<std::string, X360::Triggers> convert<X360::Triggers>::names
    {
#define ENUMSTR(name) { #name, X360::Triggers::name },
        TRIGGERS
#undef ENUMSTR
    };

    const std::map<std::string, X360::Buttons> convert<X360::Buttons>::names
    {
#define ENUMSTR(name) { #name, X360::Buttons::name },
        BUTTONS
#undef ENUMSTR
    };

    // TODO: Avoid repetition?
    Node convert<X360::Thumbs>::encode(const enum X360::Thumbs& thumb)
    {
        return Serializer::Encode(names, thumb);
    }

    bool convert<X360::Thumbs>::decode(const Node& node, enum X360::Thumbs& thumb)
    {
        return Serializer::Decode(names, node, thumb);
    }

    bool convert<X360::Thumbs>::is(std::string& name)
    {
        return names.find(name) != names.end();
    }

    Node convert<X360::Triggers>::encode(const enum X360::Triggers& thumb)
    {
        return Serializer::Encode(names, thumb);
    }

    bool convert<X360::Triggers>::decode(const Node& node, enum X360::Triggers& thumb)
    {
        return Serializer::Decode(names, node, thumb);
    }

    bool convert<X360::Triggers>::is(std::string& name)
    {
        return names.find(name) != names.end();
    }

    Node convert<X360::Buttons>::encode(const enum X360::Buttons& thumb)
    {
        return Serializer::EncodeBitWise(names, thumb);
    }

    bool convert<X360::Buttons>::decode(const Node& node, enum X360::Buttons& thumb)
    {
        return Serializer::DecodeBitWise(names, node, thumb);
    }

    Node convert<X360::IEventPtr>::encode(const X360::IEventPtr& ptr)
    {
        return ptr->Serialize();
    }

    bool convert<X360::IEventPtr>::decode(const Node& node, X360::IEventPtr& ptr)
    {
        if (node.IsScalar() || node.IsSequence())
        {
            // Button case
            bool isXboxButton = true;
            if (node.IsScalar())
            {
                auto name = node.as<std::string>();
                if (0 == name.compare(0, 3, "Key"))
                {
                    isXboxButton = false;
                }
            }

            if (isXboxButton)
            {
                auto buttons = node.as<X360::Buttons>();
                ptr = std::make_shared<X360::Button>(buttons);
            }
            else
            {
                auto buttons = node.as<Keyboard::Buttons>();
                ptr = std::make_shared<Keyboard::Button>(buttons);
            }
            return true;
        }

        if (node.IsMap())
        {
            auto typeNode = node["type"];
            if (!typeNode)
                return false;

            auto type = typeNode.as<std::string>();
            if (type == "axis")
            {
                auto offsetNode = node["offset"];
                if (!offsetNode)
                    return false;

                auto offsetStr = offsetNode.as<std::string>();
                if (convert<X360::Thumbs>::is(offsetStr))
                {
                    ptr = node.as<X360::ThumbPtr>();
                    return true;
                }
                else if (convert<X360::Triggers>::is(offsetStr))
                {
                    ptr = node.as<X360::TriggerPtr>();
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        return false;
    }

    bool convert<X360::ThumbPtr>::decode(const Node& node, X360::ThumbPtr& ptr)
    {
        if (!node.IsMap())
            return false;

        auto offsetNode = node["offset"];
        auto valueNode = node["axis"];
        auto comparNode = node["comparer"];

        if (!offsetNode || !valueNode || !comparNode)
            return false;

        auto offset = offsetNode.as<X360::Thumbs>();
        auto value = (SHORT)valueNode.as<int>();
        auto compar = comparNode.as<ControllerInterface::AxisComparerType>();

        ptr = std::make_shared<X360::Thumb>(offset, compar, value);
        return true;
    }

    bool convert<X360::TriggerPtr>::decode(const Node& node, X360::TriggerPtr& ptr)
    {
        if (!node.IsMap())
            return false;

        auto offsetNode = node["offset"];
        auto valueNode = node["axis"];
        auto comparNode = node["comparer"];

        if (!offsetNode || !valueNode || !comparNode)
            return false;

        auto offset = offsetNode.as<X360::Triggers>();
        auto value = (BYTE)valueNode.as<int>();
        auto compar = comparNode.as<ControllerInterface::AxisComparerType>();

        ptr = std::make_shared<X360::Trigger>(offset, compar, value);
        return true;
    }

    Node convert<X360::ThumbsConverter>::encode(const X360::ThumbsConverter& ptr)
    {
        return convert<ControllerInterface::LinearConverter<X360::Thumbs, SHORT>>{}.encode(ptr);
    }

    bool convert<X360::ThumbsConverter>::decode(const Node& node, X360::ThumbsConverter& ptr)
    {
        return convert<ControllerInterface::LinearConverter<X360::Thumbs, SHORT>>{}.decode(node, ptr);
    }

    Node convert<X360::TriggersConverter>::encode(const X360::TriggersConverter& ptr)
    {
        return convert<ControllerInterface::LinearConverter<X360::Triggers, BYTE>>{}.encode(ptr);
    }

    bool convert<X360::TriggersConverter>::decode(const Node& node, X360::TriggersConverter& ptr)
    {
        return convert<ControllerInterface::LinearConverter<X360::Triggers, BYTE>>{}.decode(node, ptr);
    }
}

namespace X360
{
    Button::Button(Buttons button) : IButton(button) {}

    bool Button::Happened(const Controller& c, const std::atomic_bool*) const
    {
        return Applied(c.wButtons);
    }

    std::optional<Simple::FromButton> Button::ToSimpleButton() const
    {
        switch (button_)
        {
#define ENUMSTR(name) case X360::Buttons::name: return Simple::FromButton::name;
            BUTTONS
#undef ENUMSTR
        }
        return std::nullopt;
    }

    template<typename AxisT, typename OffsetT>
    Axis<AxisT, OffsetT>::Axis(IAxis<AxisT, OffsetT> me) : IAxis<AxisT, OffsetT>(me) {}

    template<typename AxisT, typename OffsetT>
    bool Axis<AxisT, OffsetT>::Happened(const Controller& c, const std::atomic_bool*) const
    {
        return IAxis<AxisT, OffsetT>::Applied(&c);
    }

    static std::optional<Simple::FromButton> toDirection(const ControllerInterface::AxisComparerType& type, int value, Simple::FromButton hi, Simple::FromButton lo)
    {
        if (type == ControllerInterface::AxisComparerType::Less)
        {
            if (value == -16000)
                return lo;
            else
                return std::nullopt;
        }
        else
        {
            if (value == 16000)
                return hi;
            else
                return std::nullopt;
        }
    }

    template<>
    std::optional<Simple::FromButton> Axis<SHORT, Thumbs>::ToSimpleButton() const
    {
        ControllerInterface::AxisComparerType type = comparer_.type_;
        switch (offset_)
        {
        case Thumbs::LeftX:
            return toDirection(type, axis_, Simple::FromButton::LeftStickRight, Simple::FromButton::LeftStickLeft);
        case Thumbs::LeftY:
            return toDirection(type, axis_, Simple::FromButton::LeftStickUp, Simple::FromButton::LeftStickDown);
        case Thumbs::RightX:
            return toDirection(type, axis_, Simple::FromButton::RightStickRight, Simple::FromButton::RightStickLeft);
        case Thumbs::RightY:
            return toDirection(type, axis_, Simple::FromButton::RightStickUp, Simple::FromButton::RightStickDown);
        default:
            return std::nullopt;
        }
    }

    template<>
    std::optional<Simple::FromButton> Axis<BYTE, Triggers>::ToSimpleButton() const
    {
        if (axis_ != 200)
            return std::nullopt;
        if (comparer_.type_ != ControllerInterface::AxisComparerType::More)
            return std::nullopt;

        switch (offset_)
        {
        case Triggers::LeftTrigger:
            return Simple::FromButton::LeftTrigger;
        case Triggers::RightTrigger:
            return Simple::FromButton::RightTrigger;
        default:
            return std::nullopt;
        }
    }

    // TODO: Stupid!
    Thumb::Thumb(Thumbs thumb, ControllerInterface::AxisComparerType compar, SHORT value) : Axis(Intf(value, compar, thumb)) {}
    Trigger::Trigger(Triggers trigger, ControllerInterface::AxisComparerType compar, BYTE value) : Axis(Intf(value, compar, trigger)) {}

    std::optional<Simple::StickAxis> ThumbsConverter::ToSimpleStickFrom() const
    {
        if (maxval_ != 32000)
            return std::nullopt;

        switch (offset_)
        {
        case Thumbs::LeftX:
            return Simple::StickAxis::LeftX;
        case Thumbs::LeftY:
            return Simple::StickAxis::LeftY;
        case Thumbs::RightX:
            return Simple::StickAxis::RightX;
        case Thumbs::RightY:
            return Simple::StickAxis::RightY;
        }

        return std::nullopt;
    }

    std::optional<Simple::StickAxis> TriggersConverter::ToSimpleStickFrom() const
    {
        return std::nullopt;
    }
}
