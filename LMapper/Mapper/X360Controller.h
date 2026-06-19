#pragma once

#pragma warning( disable : 4250 )
#pragma warning( disable : 26812 )

#include <Windows.h>
#include <Xinput.h>

#include <yaml-cpp/yaml.h>
#include <memory>
#include "ControllerInterface.h"

namespace X360
{
    using Controller = XINPUT_GAMEPAD;

    using IEvent = ControllerInterface::IEvent<Controller>;
    using IEventPtr = ControllerInterface::IEventPtr<Controller>;

    // Must represent the offsets in buttons
    enum Buttons : USHORT
    {
        DpadUp = 0x0001,
        DpadDown = 0x0002,
        DpadLeft = 0x0004,
        DpadRight = 0x0008,
        Start = 0x0010,
        Back = 0x0020,
        LeftThumb = 0x0040,
        RightThumb = 0x0080,
        L = 0x0100,
        R = 0x0200,
        Guide = 0x0400,
        A = 0x1000,
        B = 0x2000,
        X = 0x4000,
        Y = 0x8000,
    };

    enum Thumbs : size_t
    {
        LeftX = offsetof(Controller, sThumbLX),
        LeftY = offsetof(Controller, sThumbLY),
        RightX = offsetof(Controller, sThumbRX),
        RightY = offsetof(Controller, sThumbRY),
    };

    enum Triggers : size_t
    {
        LeftTrigger = offsetof(Controller, bLeftTrigger),
        RightTrigger = offsetof(Controller, bRightTrigger),
    };

    using IButton = ControllerInterface::Button<Buttons>;
    class Button final : public IEvent
        , public IButton
    {

    public:
        Button(Buttons);

        virtual std::optional<Simple::FromButton> ToSimpleButton() const override;
        virtual bool Happened(const Controller&, const std::atomic_bool*) const;
    };

    template<typename AxisT, typename OffsetT>
    using IAxis = ControllerInterface::AxisEvent<AxisT, OffsetT>;

    template<typename AxisT, typename OffsetT>
    class Axis : public IEvent
        , public IAxis<AxisT, OffsetT>
    {
    public:
        Axis(IAxis<AxisT, OffsetT> me);

        virtual std::optional<Simple::FromButton> ToSimpleButton() const override;
        virtual bool Happened(const Controller&, const std::atomic_bool*) const;
    };

    class Thumb final : public Axis<SHORT, Thumbs>
    {
        using Intf = IAxis<SHORT, Thumbs>;

    public:
        Thumb(Thumbs, ControllerInterface::AxisComparerType type, SHORT value);
    };
    using ThumbPtr = std::shared_ptr<Thumb>;

    class Trigger final : public Axis<BYTE, Triggers>
    {
        using Intf = IAxis<BYTE, Triggers>;

    public:
        Trigger(Triggers, ControllerInterface::AxisComparerType type, BYTE value);
    };
    using TriggerPtr = std::shared_ptr<Trigger>;

    class ThumbsConverter final : public ControllerInterface::LinearConverter<X360::Thumbs, SHORT>
    {
    public:
        using ControllerInterface::LinearConverter<X360::Thumbs, SHORT>::LinearConverter;
        std::optional<Simple::StickAxis> ToSimpleStickFrom() const;
    };
    class TriggersConverter final : public ControllerInterface::LinearConverter<X360::Triggers, BYTE>
    {
    public:
        using ControllerInterface::LinearConverter<X360::Triggers, BYTE>::LinearConverter;
        std::optional<Simple::StickAxis> ToSimpleStickFrom() const;
    };
}

namespace YAML
{
    // TODO: Is it possible to avoid repeating code?
    template<>
    struct convert<X360::Thumbs>
    {
        using Serializer = Serialization::EnumSerializer<X360::Thumbs>;

        static const std::map<std::string, X360::Thumbs> names;

        static Node encode(const enum X360::Thumbs& thumb);
        static bool decode(const Node& node, enum X360::Thumbs& thumb);

        static bool is(std::string& name);
    };

    template<>
    struct convert<X360::Triggers>
    {
        using Serializer = Serialization::EnumSerializer<X360::Triggers>;

        static const std::map<std::string, X360::Triggers> names;

        static Node encode(const enum X360::Triggers& thumb);
        static bool decode(const Node& node, enum X360::Triggers& thumb);

        static bool is(std::string& name);
    };

    template<>
    struct convert<X360::Buttons>
    {
        using Serializer = Serialization::EnumSerializer<X360::Buttons>;

        static const std::map<std::string, X360::Buttons> names;

        static Node encode(const enum X360::Buttons& thumb);
        static bool decode(const Node& node, enum X360::Buttons& thumb);
    };

    template<>
    struct convert<X360::IEventPtr>
    {
        static Node encode(const X360::IEventPtr&);
        static bool decode(const Node& node, X360::IEventPtr&);
    };

    template<>
    struct convert<X360::ThumbPtr>
    {
        static bool decode(const Node& node, X360::ThumbPtr&);
    };

    template<>
    struct convert<X360::TriggerPtr>
    {
        static bool decode(const Node& node, X360::TriggerPtr&);
    };

    template<>
    struct convert<X360::ThumbsConverter>
    {
        static Node encode(const X360::ThumbsConverter&);
        static bool decode(const Node& node, X360::ThumbsConverter&);
    };

    template<>
    struct convert<X360::TriggersConverter>
    {
        static Node encode(const X360::TriggersConverter&);
        static bool decode(const Node& node, X360::TriggersConverter&);
    };

}