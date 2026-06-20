#pragma once

#include <variant>

namespace Simple
{
#define SIMPLE_FROM_BUTTONS_X360 \
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

#define SIMPLE_TO_BUTTONS_N64 \
    ENUMSTR(DpadUp) \
    ENUMSTR(DpadDown) \
    ENUMSTR(DpadLeft) \
    ENUMSTR(DpadRight) \
    ENUMSTR(Start) \
    ENUMSTR(L) \
    ENUMSTR(R) \
    ENUMSTR(A) \
    ENUMSTR(B) \
    ENUMSTR(Z) \
    ENUMSTR(CUp) \
    ENUMSTR(CDown) \
    ENUMSTR(CLeft) \
    ENUMSTR(CRight)

    // TODO: FromButton -> Source
    enum class FromButton
    {
#define ENUMSTR(name) name,
        SIMPLE_FROM_BUTTONS_X360
#undef ENUMSTR

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
#define ENUMSTR(name) name,
        SIMPLE_TO_BUTTONS_N64
#undef ENUMSTR

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

        Count,
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
    
    constexpr int X360TriggerToButtonRange = 200;
    constexpr int X360ThumbToButtonRange = 16000;
    constexpr int N64StickToButtonRange = 80;
    constexpr float ToStretch = 70.f / 80.f;

    static bool similar(float a, float b)
    {
        const float EPSILON = 0.0001f;
        return fabs(a - b) < EPSILON;
    }
}
