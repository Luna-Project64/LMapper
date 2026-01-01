#include <windows.h>
#include "Controller #1.1.h"
#include "plugin.h"
#include "savestate.h"
#include "Win.h"

static Plugin gPlugin;

EXPORT void CALL OpenDLL(void);
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        OpenDLL();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        CloseDLL();
        break;
    }
    return TRUE;
}

EXPORT void CALL OpenDLL(void)
{
}

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/
EXPORT void CALL CloseDLL(void)
{
    return;
}

/******************************************************************
  Function: ControllerCommand
  Purpose:  To process the raw data that has just been sent to a
            specific controller.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none

  note:     This function is only needed if the DLL is allowing raw
            data, or the plugin is set to raw

            the data that is being processed looks like this:
            initilize controller: 01 03 00 FF FF FF
            read controller:      01 04 01 FF FF FF FF
*******************************************************************/
EXPORT void CALL ControllerCommand(int Control, BYTE* Command)
{
    return;
}

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
EXPORT void CALL DllAbout(HWND hParent)
{
    return;
}

/******************************************************************
  Function: DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
EXPORT void CALL DllConfig(HWND hParent)
{
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = "runas";
    ShExecInfo.lpFile = "notepad.exe";
    ShExecInfo.lpParameters = Win::ConfigPath().c_str();
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    if (ShExecInfo.hProcess)
    {
        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
        CloseHandle(ShExecInfo.hProcess);
    }

    gPlugin.ReadConfig();
}

/******************************************************************
  Function: DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/
EXPORT void CALL DllTest(HWND hParent)
{
    return;
}

/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/
EXPORT void CALL GetDllInfo(PLUGIN_INFO* PluginInfo)
{
    PluginInfo->Version = 0x0100;
    PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
    static const char PluginName[] = "LINK's Mapper 1.1.2";
    strncpy_s(PluginInfo->Name, PluginName, sizeof(PluginName));
}

/******************************************************************
  Function: GetKeys
  Purpose:  To get the current state of the controllers buttons.
  input:    - Controller Number (0 to 3)
            - A pointer to a BUTTONS structure to be filled with
            the controller state.
  output:   none
*******************************************************************/
EXPORT void CALL GetKeys(int Control, BUTTONS* Keys)
{
    if (!Keys)
        return;

    gPlugin.Get(Control, Keys);
}

/******************************************************************
  Function: InitiateControllers
  Purpose:  This function initialises how each of the controllers
            should be handled.
  input:    - The handle to the main window.
            - A controller structure that needs to be filled for
              the emulator to know how to handle each controller.
  output:   none
*******************************************************************/
EXPORT void CALL InitiateControllers(HWND hMainWindow, CONTROL Controls[4])
{
    XINPUT_STATE state;
    for (int i = 0; i <= 3; i++)
    {
        bool connected = gPlugin.enabledControllersMask() & (1 << i);
        if (!connected)
        {
            auto err = XInputGetState(i, &state);
            auto connected = ERROR_SUCCESS == err;
        }

        Controls[i].Plugin  = connected;
        Controls[i].Present = connected;
        Controls[i].RawData = false;
    }
}

/******************************************************************
  Function: ReadController
  Purpose:  To process the raw data in the pif ram that is about to
            be read.
  input:    - Controller Number (0 to 3) and -1 signalling end of
              processing the pif ram.
            - Pointer of data to be processed.
  output:   none
  note:     This function is only needed if the DLL is allowing raw
            data.
*******************************************************************/
EXPORT void CALL ReadController(int Control, BYTE* Command)
{
    return;
}

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/
EXPORT void CALL RomClosed(void)
{
    return;
}

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the
            emulation thread)
  input:    none
  output:   none
*******************************************************************/
EXPORT void CALL RomOpen(void)
{
    gPlugin.ReadConfig();
    gPlugin.clearKeys();
}

/******************************************************************
  Function: WM_KeyDown
  Purpose:  To pass the WM_KeyDown message from the emulator to the
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/
EXPORT void CALL WM_KeyDown(WPARAM wParam, LPARAM lParam) 
{
    if (gPlugin.savestateHack())
    {
        Savestate::handleKey(wParam, lParam);
    }

    gPlugin.setActiveKey(wParam);
}

/******************************************************************
  Function: WM_KeyUp
  Purpose:  To pass the WM_KEYUP message from the emulator to the
            plugin.
  input:    wParam and lParam of the WM_KEYDOWN message.
  output:   none
*******************************************************************/
EXPORT void CALL WM_KeyUp(WPARAM wParam, LPARAM lParam) 
{
    gPlugin.setInactiveKey(wParam);
}

char gPluginConfigDir[MAX_PATH]{};
namespace Zilmar
{
    short Set_PluginConfigDir = 0;
    enum SettingLocation
    {
        SettingType_ConstString = 0,
        SettingType_ConstValue = 1,
        SettingType_CfgFile = 2,
        SettingType_Registry = 3,
        SettingType_RelativePath = 4,
        TemporarySetting = 5,
        SettingType_RomDatabase = 6,
        SettingType_CheatSetting = 7,
        SettingType_GameSetting = 8,
        SettingType_BoolVariable = 9,
        SettingType_NumberVariable = 10,
        SettingType_StringVariable = 11,
        SettingType_SelectedDirectory = 12,
        SettingType_RdbSetting = 13,
    };

    enum SettingDataType
    {
        Data_DWORD = 0, Data_String = 1, Data_CPUTYPE = 2, Data_SelfMod = 3, Data_OnOff = 4, Data_YesNo = 5, Data_SaveChip = 6
    };

    typedef struct
    {
        uint32_t dwSize;
        int DefaultStartRange;
        int SettingStartRange;
        int MaximumSettings;
        int NoDefault;
        int DefaultLocation;
        void* handle;

        unsigned int(CALL* GetSetting)      (void* handle, int ID);
        const char* (CALL* GetSettingSz)    (void* handle, int ID, char* Buffer, int BufferLen);
        void(CALL* SetSetting)      (void* handle, int ID, unsigned int Value);
        void(CALL* SetSettingSz)    (void* handle, int ID, const char* Value);
        void(CALL* RegisterSetting) (void* handle, int ID, int DefaultID, SettingDataType Type,
            SettingLocation Location, const char* Category, const char* DefaultStr, uint32_t Value);
        void(CALL* UseUnregisteredSetting) (int ID);
    } PLUGIN_SETTINGS;

    typedef struct
    {
        unsigned int(CALL* FindSystemSettingId) (void* handle, const char* Name);
    } PLUGIN_SETTINGS2;

    static PLUGIN_SETTINGS  g_PluginSettings;
    static PLUGIN_SETTINGS2 g_PluginSettings2;
    static inline unsigned int GetSystemSetting(short SettingID)
    {
        return g_PluginSettings.GetSetting(g_PluginSettings.handle, SettingID);
    }

    static inline short FindSystemSettingId(const char* Name)
    {
        if (g_PluginSettings2.FindSystemSettingId && g_PluginSettings.handle)
        {
            return (short)g_PluginSettings2.FindSystemSettingId(g_PluginSettings.handle, Name);
        }
        return 0;
    }
}

extern "C" EXPORT void CALL SetSettingInfo(Zilmar::PLUGIN_SETTINGS* info)
{
    Zilmar::g_PluginSettings = *info;
}

extern "C" EXPORT void CALL SetSettingInfo2(Zilmar::PLUGIN_SETTINGS2* info)
{
    Zilmar::g_PluginSettings2 = *info;
}

extern "C" EXPORT void CALL PluginLoaded(void)
{
    int pluginConfigDir = Zilmar::FindSystemSettingId("Config Base Dir");
    if (pluginConfigDir)
    {
        const char* cfg = Zilmar::g_PluginSettings.GetSettingSz(Zilmar::g_PluginSettings.handle, pluginConfigDir, gPluginConfigDir, sizeof(gPluginConfigDir));
        if (!cfg)
            *gPluginConfigDir = '\0';
    }

    gPlugin.ReadConfig();
}