#include "Luna.h"

namespace Luna
{
    HWND gMainWindow = NULL;
    LunaExCommandFn gExCommandHandler = nullptr;

    Cmd::Cmd(LunaExCommand cmd) : cmd_(cmd) {}

    static std::string sNames[] = {
        /* LUNA_EXCMD_LOAD_STATE */ "load_state",
        /* LUNA_EXCMD_SAVE_STATE */ "save_state",
        /* LUNA_EXCMD_UNLOCK_FPS */ "unlock_fps",
        /* LUNA_EXCMD_LOCK_FPS   */ "lock_fps"
    };

    std::optional<Simple::LunaRawButton> Cmd::ToSimpleLunaRawButton() const
    {
        switch (cmd_)
        {
        case LUNA_EXCMD_LOAD_STATE:
            return Simple::LunaRawButton::LoadState;
        case LUNA_EXCMD_SAVE_STATE:
            return Simple::LunaRawButton::SaveState;
        case LUNA_EXCMD_UNLOCK_FPS:
            return Simple::LunaRawButton::UnlockFPS;
        case LUNA_EXCMD_LOCK_FPS:
            return Simple::LunaRawButton::LockFPS;
        }

        return std::nullopt;
    }

    void Cmd::Alter(N64::Controller& /*controller*/) const
    {
        if (gExCommandHandler)
        {
            gExCommandHandler(gMainWindow, cmd_);
        }
    }

    YAML::Node Cmd::Serialize() const
    {
        YAML::Node node;
        node["cmd"] = sNames[cmd_];
        return node;
    }
}

namespace YAML
{
    bool convert<Luna::CmdPtr>::decode(const Node& node, Luna::CmdPtr& ptr)
    {
        if (!node.IsScalar())
            return false;

        int cmd = -1;
        std::string cmdStr = node.as<std::string>();

        for (size_t i = 0; i < sizeof(Luna::sNames) / sizeof(Luna::sNames[0]); i++)
        {
            if (cmdStr == Luna::sNames[i])
            {
                cmd = static_cast<LunaExCommand>(i);
                break;
            }
        }

        if (cmd < 0)
            return false;

        ptr = std::make_unique<Luna::Cmd>((LunaExCommand)cmd);
        return true;
    }
}
