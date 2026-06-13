#pragma once

#include <Windows.h>
#include "Controller #1.1.h"
#include "config.h"

class Plugin
{
public:
    void ReadConfig();
    void Get(int controller, BUTTONS*);

    const Mapping::Mappers& Mappers() const { return cfg.mappers; }
    const std::string& ConfigPath() const { return configPath; }
    char enabledControllersMask() const { return cfg.enabledControllersMask; }

    void clearKeys()
    {
        memset(activeKeys, sizeof(activeKeys), 1);
    }

    void setActiveKey(unsigned int k)
    {
        if (k < sizeof(activeKeys))
            activeKeys[k] = true;
    }

    void setInactiveKey(unsigned int k)
    {
        if (k < sizeof(activeKeys))
            activeKeys[k] = false;
    }

    const std::atomic_bool* keyboard()
    {
        return activeKeys;
    }

private:
    std::string configPath;
    Config cfg;
    std::atomic_bool activeKeys[255];
};
