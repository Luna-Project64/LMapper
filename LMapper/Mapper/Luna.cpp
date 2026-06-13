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

	std::string Cmd::ToString()
	{
		return sNames[cmd_];
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

		for (size_t i = 0; i < sizeof(LunaExCommand) / sizeof(LunaExCommand); i++)
		{
			if (cmdStr == Luna::sNames[i])
			{
				cmd = static_cast<LunaExCommand>(i);
				break;
			}
		}

		if (cmd < 0)
			return false;

		ptr = std::make_shared<Luna::Cmd>((LunaExCommand) cmd);
		return true;
	}
}
