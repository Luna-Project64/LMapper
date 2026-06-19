#pragma once

#include <variant>

namespace Simple
{
    // TODO: FromButton -> Source
    enum class FromButton
    {
        DpadUp,
        DpadDown,
        DpadLeft,
        DpadRight,
        Start,
        Back,
        LeftThumb,
        RightThumb,
        L,
        R,
        Guide,
        A,
        B,
        X,
        Y,

        LeftStickUp,
        LeftStickDown,
        LeftStickLeft,
        LeftStickRight,
        RightStickUp,
        RightStickDown,
        RightStickLeft,
        RightStickRight,

        LeftTrigger,
        RightTrigger,

#define ENUMSTR(name) name,
#include "KeyboardXMacro.h"
#undef ENUMSTR

        Count,
    };

    // TODO: ToButton -> Destination
    enum class ToButton
    {
        DpadUp,
        DpadDown,
        DpadLeft,
        DpadRight,
        Start,
        L,
        R,
        A,
        B,
        Z,
        CUp,
        CDown,
        CLeft,
        CRight,

        StickUp,
        StickDown,
        StickLeft,
        StickRight,

        LoadState,
        SaveState,
        UnlockFPS,
        LockFPS,

        Count,
    };

    struct ButtonMapping
    {
        FromButton from;
        ToButton to;
    };

    enum class FromStick
    {
        Left,
        Right,
    };

    struct StickMapping
    {
        FromStick from;
        uint8_t range;

        float deadzone;

        bool angleDeadzoneWithDiagonals;
        float angleDeadzone;

        float stretcher;
    };

    using Config = std::variant<ButtonMapping, StickMapping>;
}
