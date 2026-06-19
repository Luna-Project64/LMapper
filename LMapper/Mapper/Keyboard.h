#pragma once
#include <Windows.h>
#include <Xinput.h>

#include <yaml-cpp/yaml.h>
#include <memory>
#include "ControllerInterface.h"
#include "X360Controller.h"

namespace Keyboard
{
    using Controller = X360::Controller;
    using IEvent = ControllerInterface::IEvent<Controller>;
    using IEventPtr = ControllerInterface::IEventPtr<Controller>;

    enum Buttons
    {
        KeyLButton = 0x01,
        KeyRButton = 0x02,
        KeyCancel = 0x03,
        KeyMButton = 0x04,
        KeyXButton1 = 0x05,
        KeyXButton2 = 0x06,
        KeyBack = 0x08,
        KeyTab = 0x09,
        KeyClear = 0x0C,
        KeyReturn = 0x0D,
        KeyShift = 0x10,
        KeyControl = 0x11,
        KeyMenu = 0x12,
        KeyPause = 0x13,
        KeyCapital = 0x14,
        KeyKana = 0x15,
        KeyHanguel = 0x15,
        KeyHangul = 0x15,
        KeyImeOn = 0x16,
        KeyJunja = 0x17,
        KeyFinal = 0x18,
        KeyHanja = 0x19,
        KeyKanji = 0x19,
        KeyImeOff = 0x1A,
        KeyEscape = 0x1B,
        KeyConvert = 0x1C,
        KeyNonConvert = 0x1D,
        KeyAccept = 0x1E,
        KeyModeChange = 0x1F,
        KeySpace = 0x20,
        KeyPrior = 0x21,
        KeyNext = 0x22,
        KeyEnd = 0x23,
        KeyHome = 0x24,
        KeyLeft = 0x25,
        KeyUp = 0x26,
        KeyRight = 0x27,
        KeyDown = 0x28,
        KeySelect = 0x29,
        KeyPrint = 0x2A,
        KeyExecute = 0x2B,
        KeySnapshot = 0x2C,
        KeyInsert = 0x2D,
        KeyDelete = 0x2E,
        KeyHelp = 0x2F,
        Key0 = 0x30,
        Key1 = 0x31,
        Key2 = 0x32,
        Key3 = 0x33,
        Key4 = 0x34,
        Key5 = 0x35,
        Key6 = 0x36,
        Key7 = 0x37,
        Key8 = 0x38,
        Key9 = 0x39,

        KeyA = 0x41,
        KeyB = 0x42,
        KeyC = 0x43,
        KeyD = 0x44,
        KeyE = 0x45,
        KeyF = 0x46,
        KeyG = 0x47,
        KeyH = 0x48,
        KeyI = 0x49,
        KeyJ = 0x4A,
        KeyK = 0x4B,
        KeyL = 0x4C,
        KeyM = 0x4D,
        KeyN = 0x4E,
        KeyO = 0x4F,
        KeyP = 0x50,
        KeyQ = 0x51,
        KeyR = 0x52,
        KeyS = 0x53,
        KeyT = 0x54,
        KeyU = 0x55,
        KeyV = 0x56,
        KeyW = 0x57,
        KeyX = 0x58,
        KeyY = 0x59,
        KeyZ = 0x5A,
        KeyLWin = 0x5B,
        KeyRWin = 0x5C,
        KeyApps = 0x5D,
        KeySleep = 0x5F,
        KeyNumPad0 = 0x60,
        KeyNumPad1 = 0x61,
        KeyNumPad2 = 0x62,
        KeyNumPad3 = 0x63,
        KeyNumPad4 = 0x64,
        KeyNumPad5 = 0x65,
        KeyNumPad6 = 0x66,
        KeyNumPad7 = 0x67,
        KeyNumPad8 = 0x68,
        KeyNumPad9 = 0x69,
        KeyMultiply = 0x6A,
        KeyAdd = 0x6B,
        KeySeparator = 0x6C,
        KeySubstract = 0x6D,
        KeyDecimal = 0x6E,
        KeyDivide = 0x6F,
        KeyF1 = 0x70,
        KeyF2 = 0x71,
        KeyF3 = 0x72,
        KeyF4 = 0x73,
        KeyF5 = 0x74,
        KeyF6 = 0x75,
        KeyF7 = 0x76,
        KeyF8 = 0x77,
        KeyF9 = 0x78,
        KeyF10 = 0x79,
        KeyF11 = 0x7A,
        KeyF12 = 0x7B,
        KeyF13 = 0x7C,
        KeyF14 = 0x7D,
        KeyF15 = 0x7E,
        KeyF16 = 0x7F,
        KeyF17 = 0x80,
        KeyF18 = 0x81,
        KeyF19 = 0x82,
        KeyF20 = 0x83,
        KeyF21 = 0x84,
        KeyF22 = 0x85,
        KeyF23 = 0x86,
        KeyF24 = 0x87,
        KeyNumlock = 0x90,
        KeyScroll = 0x91,
        KeyLShift = 0xA0,
        KeyRShift = 0xA1,
        KeyLControl = 0xA2,
        KeyRControl = 0xA3,
        KeyLMenu = 0xA4,
        KeyRMenu = 0xA5,
        KeyBrowserBack = 0xA6,
        KeyBrowserForward = 0xA7,
        KeyBrowserRefresh = 0xA8,
        KeyBrowserStop = 0xA9,
        KeyBrowserSearch = 0xAA,
        KeyBrowserFavorites = 0xAB,
        KeyBrowserHome = 0xAC,
        KeyVolumeMute = 0xAD,
        KeyVolumeDown = 0xAE,
        KeyVolumeUp = 0xAF,
        KeyMediaNextTrack = 0xB0,
        KeyMediaPrevTrack = 0xB1,
        KeyMediaStop = 0xB2,
        KeyMediaPlayPause = 0xB3,
        KeyLaunchMail = 0xB4,
        KeyLaunchMediaSelect = 0xB5,
        KeyLaunchApp1 = 0xB6,
        KeyLaunchApp2 = 0xB7,

        KeyColon = 0xBA,
        KeyPlus = 0xBB,
        KeyComma = 0xBC,
        KeyMinus = 0xBD,
        KeyPeriod = 0xBE,
        KeyQuestion = 0xBF,
        KeySlash = 0xBF,
        KeyBackTick = 0xC0,
        KeyTilde = 0xC0,

        KeyBracketOpen = 0xdb,
        KeyBracketBackSlash = 0xdc,
        KeyBracketPipe = 0xdc,
        KeyBracketClose = 0xdd,
        KeyQuote = 0xde,
        KeyPipeUS = 0xe1,

        KeyProcessKey = 0xe5,

        KeyPacket = 0xe7,

        KeyAttn = 0xf6,
        KeyCrsel = 0xF7,
        KeyExsel = 0xF8,
        KeyEreof = 0xF9,
        KeyPlay = 0xFA,
        KeyZoom = 0xFB,
        KeyNoName = 0xFC,
        KeyPA1 = 0xFD,
        KeyOEMClear = 0xFE,
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
}

namespace YAML
{
    // TODO: Is it possible to avoid repeating code?
    template<>
    struct convert<Keyboard::Buttons>
    {
        using Serializer = Serialization::EnumSerializer<Keyboard::Buttons>;

        static const std::map<std::string, Keyboard::Buttons> names;

        static Node encode(const enum Keyboard::Buttons& thumb);
        static bool decode(const Node& node, enum Keyboard::Buttons& thumb);
    };

#if 0
    // Encoded in X360Controller as button can be either controller or keyboard
    template<>
    struct convert<Keyboard::IEventPtr>
    {
        static Node encode(const Keyboard::IEventPtr&);
        static bool decode(const Node& node, Keyboard::IEventPtr&);
    };
#endif
}
