#include "Keyboard.h"
#include "ControllerInterfaceImpl.h"
#include "SerializationImpl.h"

namespace YAML
{
    const std::map<std::string, Keyboard::Buttons> convert<Keyboard::Buttons>::names
    {
#define ENUMSTR(name) { #name, Keyboard::Buttons::name },
#include "KeyboardXMacro.h"
#undef ENUMSTR
    };

    Node convert<Keyboard::Buttons>::encode(const enum Keyboard::Buttons& thumb)
    {
        return Serializer::Encode(names, thumb);
    }

    bool convert<Keyboard::Buttons>::decode(const Node& node, enum Keyboard::Buttons& thumb)
    {
        return Serializer::Decode(names, node, thumb);
    }
}

namespace Keyboard
{
    Button::Button(Buttons button) : IButton(button) { }

    bool Button::Happened(const Controller& c, const std::atomic_bool* keyboard) const
    {
        return Applied(keyboard);
    }

    std::optional<Simple::FromButton> Button::ToSimpleButton() const
    {
        switch (button_)
        {
#define ENUMSTR(name) case Keyboard::Buttons::name: return Simple::FromButton::name;
#include "KeyboardXMacro.h"
#undef ENUMSTR
        }

        return std::nullopt;
    }
}
