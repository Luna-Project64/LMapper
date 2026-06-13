#pragma once

#pragma warning( disable : 4250 )
#pragma warning( disable : 26812 )

#include <Windows.h>

#include <yaml-cpp/yaml.h>
#include <memory>
#include "ControllerInterface.h"
#include "../Controller #1.1.h"

namespace N64
{
    using Controller = BUTTONS;

    using IModifier = ControllerInterface::IModifier<Controller>;
    using IModifierPtr = ControllerInterface::IModifierPtr<Controller>;

    // Must represent the offsets in buttons
    enum Buttons : DWORD
    {
        DpadRight = 0x0001,
        DpadLeft  = 0x0002,
        DpadDown  = 0x0004,
        DpadUp    = 0x0008,
        Start     = 0x0010,
        Z         = 0x0020,
        B         = 0x0040,
        A         = 0x0080,

        CRight    = 0x0100,
        CLeft     = 0x0200,
        CDown     = 0x0400,
        CUp       = 0x0800,

        R         = 0x1000,
        L         = 0x2000,
    };

    enum Axises : size_t
    {
        X = 2, // offsetof but cant because spec is dumb
        Y = 3,
    };

    using IButton = ControllerInterface::Button<Buttons>;
    class Button final : public IModifier
                       , public IButton
    {

    public:
        Button(Buttons);

        virtual std::optional<std::string> ToString() const override;
        virtual void Alter(Controller&) const override;
    };

    using IAxis = ControllerInterface::Axis<char, Axises>;
    class Axis final : public IModifier
                     , public IAxis
    {
    public:
        Axis(Axises, signed value);

        virtual std::optional<std::string> ToString() const override;
        virtual void Alter(Controller&) const override;
    };

    using AxisPtr = std::shared_ptr<Axis>;
    using AxisConverter = ControllerInterface::LinearConverter<Axises, char>;
}

namespace YAML
{
    // TODO: Is it possible to avoid repeating code?
    template<>
    struct convert<N64::Axises>
    {
        using Serializer = Serialization::EnumSerializer<N64::Axises>;

        static const std::map<std::string, N64::Axises> names;

        static Node encode(const enum N64::Axises& thumb);
        static bool decode(const Node& node, enum N64::Axises& thumb);

        static bool is(std::string& name);
    };

    template<>
    struct convert<N64::Buttons>
    {
        using Serializer = Serialization::EnumSerializer<N64::Buttons>;

        static const std::map<std::string, N64::Buttons> names;

        static Node encode(const enum N64::Buttons& thumb);
        static bool decode(const Node& node, enum N64::Buttons& thumb);
    };

    template<>
    struct convert<N64::IModifierPtr>
    {
        static Node encode(const N64::IModifierPtr&);
        static bool decode(const Node& node, N64::IModifierPtr&);
    };

    template<>
    struct convert<N64::AxisPtr>
    {
        static bool decode(const Node& node, N64::AxisPtr&);
    };
}