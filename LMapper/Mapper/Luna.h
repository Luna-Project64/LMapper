#pragma once

#include <Windows.h>
#include "../Controller #1.1.h"
#include "N64Controller.h"

namespace Luna
{
	extern HWND gMainWindow;
    extern LunaExCommandFn gExCommandHandler;

    class Cmd final : public N64::IModifier
    {
    public:
        Cmd(LunaExCommand);

        virtual std::string ToString() override;
        virtual void Alter(N64::Controller&) const override;
        virtual YAML::Node Serialize() const override;

    private:
		LunaExCommand cmd_;
    };

    using CmdPtr = std::shared_ptr<Cmd>;
}

namespace YAML
{
    template<>
    struct convert<Luna::CmdPtr>
    {
        static bool decode(const Node& node, Luna::CmdPtr&);
    };
}