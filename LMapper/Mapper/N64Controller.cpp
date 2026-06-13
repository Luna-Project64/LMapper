#include "N64Controller.h"

#include "Luna.h"
#include "ControllerInterfaceImpl.h"
#include "SerializationImpl.h"

#define AXIS \
    ENUMSTR(X) \
    ENUMSTR(Y)

#define BUTTONS \
    ENUMSTR(DpadUp) \
    ENUMSTR(DpadDown) \
    ENUMSTR(DpadLeft) \
    ENUMSTR(DpadRight) \
    ENUMSTR(Start) \
    ENUMSTR(Z) \
    ENUMSTR(A) \
    ENUMSTR(B) \
    ENUMSTR(CRight) \
    ENUMSTR(CLeft) \
    ENUMSTR(CDown) \
    ENUMSTR(CUp) \
    ENUMSTR(R) \
    ENUMSTR(L)

namespace ControllerInterface
{
    template<>
    std::optional<std::string> toOffsetString<N64::Axises>(size_t offset)
    {
        switch (offset)
        {
#define ENUMSTR(name) case N64::Axises::name: return #name;
            AXIS
#undef ENUMSTR
        }

        return std::nullopt;
    }
}

namespace YAML
{
    const std::map<std::string, N64::Axises> convert<N64::Axises>::names
    {
#define ENUMSTR(name) { #name, N64::Axises::name },
        AXIS
#undef ENUMSTR
    };

    const std::map<std::string, N64::Buttons> convert<N64::Buttons>::names
    {
#define ENUMSTR(name) { #name, N64::Buttons::name },
        BUTTONS
#undef ENUMSTR
    };

    // TODO: Avoid repetition?
    Node convert<N64::Axises>::encode(const enum N64::Axises& thumb)
    {
        return Serializer::Encode(names, thumb);
    }

    bool convert<N64::Axises>::decode(const Node& node, enum N64::Axises& thumb)
    {
        return Serializer::Decode(names, node, thumb);
    }

    bool convert<N64::Axises>::is(std::string& name)
    {
        return names.find(name) != names.end();
    }

    Node convert<N64::Buttons>::encode(const enum N64::Buttons& thumb)
    {
        return Serializer::EncodeBitWise(names, thumb);
    }

    bool convert<N64::Buttons>::decode(const Node& node, enum N64::Buttons& thumb)
    {
        return Serializer::DecodeBitWise(names, node, thumb);
    }

    Node convert<N64::IModifierPtr>::encode(const N64::IModifierPtr& ptr)
    {
        return ptr->Serialize();
    }

    bool convert<N64::IModifierPtr>::decode(const Node& node, N64::IModifierPtr& ptr)
    {
        if (node.IsScalar() || node.IsSequence())
        {
            // Button case
            auto buttons = node.as<N64::Buttons>();
            ptr = std::make_shared<N64::Button>(buttons);
            return true;
        }

        if (node.IsMap())
        {
            auto cmdNode = node["cmd"];
            if (cmdNode)
            {
                ptr = cmdNode.as<Luna::CmdPtr>();
                return true;
            }

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
                if (convert<N64::Axises>::is(offsetStr))
                {
                    ptr = node.as<N64::AxisPtr>();
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

    bool convert<N64::AxisPtr>::decode(const Node& node, N64::AxisPtr& ptr)
    {
        if (!node.IsMap())
            return false;

        auto offsetNode = node["offset"];
        auto valueNode = node["axis"];

        if (!offsetNode || !valueNode)
            return false;

        auto offset = offsetNode.as<N64::Axises>();
        auto value = (SHORT)valueNode.as<int>();

        ptr = std::make_shared<N64::Axis>(offset, value);
        return true;
    }
}

namespace N64
{
    Button::Button(Buttons button) : IButton(button) { }

    void Button::Alter(Controller& c) const
    {
        Apply(c.Value);
    }

    std::optional<std::string> Button::ToString() const
    {
        switch (button_)
        {
#define ENUMSTR(name) case N64::Buttons::name: return #name;
            BUTTONS
#undef ENUMSTR
        }
        return std::nullopt;
    }

    Axis::Axis(Axises thumb, signed value) : IAxis(value, thumb) { }

    void Axis::Alter(Controller& c) const
    {
        return Apply(&c);
    }

    std::optional<std::string> Axis::ToString() const
    {
		auto offsetStr = ControllerInterface::toOffsetString<Axises>(offset_);
        if (!offsetStr)
			return std::nullopt;

        return *offsetStr + " " + std::to_string(axis_);
    }
}
