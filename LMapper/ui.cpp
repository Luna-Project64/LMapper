#include "ui.h"

#pragma warning(push)
#pragma warning(disable : 4091) // warning C4091: 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
#pragma warning(disable : 4201) // warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4302) // warning C4302: 'type cast': truncation from 'LPCTSTR' to 'WORD'
#pragma warning(disable : 4458) // warning C4458: declaration of 'dwCommonButtons' hides class member
#pragma warning(disable : 4838) // warning C4838: conversion from 'int' to 'UINT' requires a narrowing conversion
#pragma warning(disable : 4996) // warning C4996: 'GetVersionExA': was declared deprecated
#define _ATL_DISABLE_NOTHROW_NEW
#include <atlbase.h>
#include "WTL/atlapp.h"
#include "WTL/atlcrack.h"
#include "WTL/atlctrls.h"
#include "WTL/atlctrlx.h"
#include "WTL/atlgdi.h"
#include "WTL/atlmisc.h"
#include "WTL/atlwinx.h"
#include <atlwin.h>
#pragma warning(pop)

#include "resource.h"

#include "config.h"
#include "Mapper/Keyboard.h"
#include "Mapper/Luna.h"
#include "Win.h"

#include <chrono>
#include <fstream>

extern const char* kDefaultConfig;

static std::optional<Simple::FromButton> keyboardToFromButton(unsigned vk)
{
    switch (vk)
    {
#define ENUMSTR(name) case Keyboard::Buttons::name: return Simple::FromButton::name;
#include "Mapper/KeyboardXMacro.h"
#undef ENUMSTR
    }

    return std::nullopt;
}

class CSliderCtrl : public CWindow
{
public:
    inline int GetPos() { return (int)::SendMessage(*this, TBM_GETPOS, 0, 0); }
    inline void SetPos(int pos, bool bRedraw = true) { ::SendMessage(*this, TBM_SETPOS, (WPARAM)bRedraw, (LPARAM)pos); }
};

class Dlg : public CDialogImpl<Dlg>
{
public:
    Dlg() = default;

    enum { IDD = IDD_DIALOG_MAIN };

    BEGIN_MSG_MAP_EX(Dlg)
        MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
        MESSAGE_HANDLER(WM_KEYUP, onKeyUp)
        MESSAGE_HANDLER(WM_KEYDOWN, onKeyDown)
        COMMAND_ID_HANDLER(IDOK, onSave)
        COMMAND_ID_HANDLER(IDCANCEL, onCancel)
        COMMAND_ID_HANDLER(ID_RESET, onReset)
        COMMAND_ID_HANDLER(ID_BUTTON_UP, onUp)
        COMMAND_ID_HANDLER(ID_BUTTON_DOWN, onDown)
        COMMAND_ID_HANDLER(ID_ADD, onAdd)
        COMMAND_ID_HANDLER(ID_REMOVE, onRemove)
        COMMAND_ID_HANDLER(IDC_KEY_CHOOSE, onCalibrate)
        NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_ITEMACTIVATE, onListItemActivate)
        NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_DELETEITEM, onListItemDeleted)
        NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_ITEMCHANGED, onListItemChanged)
        COMMAND_HANDLER(IDC_COMBO_XBOX, CBN_SELCHANGE, onDigitalXboxChanged)
        COMMAND_HANDLER(IDC_COMBO_N64, CBN_SELCHANGE, onDigitalN64Changed)
        COMMAND_HANDLER(IDC_COMBO_TYPE, CBN_SELCHANGE, onTypeChanged)
        COMMAND_HANDLER(IDC_DEADZONE, EN_CHANGE, onStickChangeDeadzone)
        COMMAND_HANDLER(IDC_ANGLE_DEADZONE, EN_CHANGE, onStickChangeAngleDeadzone)
        COMMAND_HANDLER(IDC_WANT_DIAGONAL_DZ, BN_CLICKED, onStickClickedWantDiagonalDz)
        COMMAND_HANDLER(IDC_STRETCH, EN_CHANGE, onStickChangeStretch)
        COMMAND_HANDLER(IDC_N64_RANGE, EN_CHANGE, onStickChangeN64Range)
        COMMAND_HANDLER(IDC_STRETCH_DIAGONALS, BN_CLICKED, onStickClickedStretchDiagonals)
        END_MSG_MAP()

    bool Saved(void) const { return saved_; }
    const Config& GetConfig(void) const { return config_; }

protected:
    bool saved_ = false;
    CListViewCtrl controls_;
    CComboBox types_;

    CComboBox digitalXboxOptions_;
    CButton digitalXboxKeyboard_;
    CComboBox digitalN64Options_;
    CButton digitalN64Active_;
    CWindow digitalKeyboard_;

    CWindow stickLabel1_;
    CWindow stickLabel2_;
    CWindow stickLabel3_;
    CWindow stickPicture_;
    CComboBox stickXboxOptions_;
    CEdit stickDeadzone_;
    CEdit stickAngleDeadzone_;
    CEdit stickStretching_;
    CEdit stickRange_;
    CButton stickAngleDeadzone8Dir_;
    CButton stickStretchingDiagonal_;
    CWindow stickDeadzoneSpin_;
    CWindow stickAngleDeadzoneSpin_;

    Config config_;
    int selectedIndex_;
    int curDrawnType_ = -1;
    float drawX_ = 0;
    float drawY_ = 0;

    std::atomic_bool activeKeys_[255];

    struct ChoosingContext
    {
        int64_t lastTime = 0;
        std::chrono::steady_clock::time_point deadline;
    };
    std::optional<ChoosingContext> choosing_;

    LRESULT onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT onKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT onKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT onSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onCalibrate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT onDigitalXboxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onDigitalN64Changed(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT onTypeChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    LRESULT onStickChangeDeadzone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT onStickChangeAngleDeadzone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT onStickClickedWantDiagonalDz(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT onStickChangeStretch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT onStickChangeN64Range(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT onStickClickedStretchDiagonals(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    LRESULT onListItemActivate(NMHDR* phdr);
    LRESULT onListItemDeleted(NMHDR* phdr);
    LRESULT onListItemChanged(NMHDR* phdr);

    void refresh();
    void refreshAt(size_t index);

    int selectedIndex();
    void setSelectedIndex(int index);

    std::optional<Simple::FromButton> selectedFromButton();
    std::optional<Simple::FromStick> selectedFromStick();
    std::optional<Simple::ToButton> selectedToButton();

    static void CALLBACK onTimer(HWND, UINT, UINT_PTR, DWORD);
    void onTimer();

    static LRESULT CALLBACK PictureSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    LRESULT pictureSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void choosingReset();
    void refreshTypeWindows(int type);
    void refreshDigital();
    void refreshStick();
};

static const char* toString(Simple::FromButton button)
{
    switch (button)
    {
    case Simple::FromButton::DpadUp:
        return "Dpad Up";
    case Simple::FromButton::DpadDown:
        return "Dpad Down";
    case Simple::FromButton::DpadLeft:
        return "Dpad Left";
    case Simple::FromButton::DpadRight:
        return "Dpad Right";
    case Simple::FromButton::Start:
        return "Start";
    case Simple::FromButton::Back:
        return "Back";
    case Simple::FromButton::LeftThumb:
        return "Left Thumb Click";
    case Simple::FromButton::RightThumb:
        return "Right Thumb Click";
    case Simple::FromButton::L:
        return "Left Shoulder";
    case Simple::FromButton::R:
        return "Right Shoulder";
    case Simple::FromButton::Guide:
        return "Guide";
    case Simple::FromButton::A:
        return "A";
    case Simple::FromButton::B:
        return "B";
    case Simple::FromButton::X:
        return "X";
    case Simple::FromButton::Y:
        return "Y";

    case Simple::FromButton::LeftStickUp:
        return "Left Stick Up";
    case Simple::FromButton::LeftStickDown:
        return "Left Stick Down";
    case Simple::FromButton::LeftStickLeft:
        return "Left Stick Left";
    case Simple::FromButton::LeftStickRight:
        return "Left Stick Right";
    case Simple::FromButton::RightStickUp:
        return "Right Stick Up";
    case Simple::FromButton::RightStickDown:
        return "Right Stick Down";
    case Simple::FromButton::RightStickLeft:
        return "Right Stick Left";
    case Simple::FromButton::RightStickRight:
        return "Right Stick Right";

    case Simple::FromButton::LeftTrigger:
        return "Left Trigger";
    case Simple::FromButton::RightTrigger:
        return "Right Trigger";

#define ENUMSTR(name) case Simple::FromButton::name: return #name;
#include "Mapper/KeyboardXMacro.h"
#undef ENUMSTR

    };

    return nullptr;
}

static const char* toString(Simple::ToButton button)
{
    switch (button)
    {
    case Simple::ToButton::DpadUp:
        return "Dpad Up";
    case Simple::ToButton::DpadDown:
        return "Dpad Down";
    case Simple::ToButton::DpadLeft:
        return "Dpad Left";
    case Simple::ToButton::DpadRight:
        return "Dpad Right";
    case Simple::ToButton::Start:
        return "Start";
    case Simple::ToButton::L:
        return "L";
    case Simple::ToButton::R:
        return "R";
    case Simple::ToButton::A:
        return "A";
    case Simple::ToButton::B:
        return "B";
    case Simple::ToButton::Z:
        return "Z";
    case Simple::ToButton::CUp:
        return "C Up";
    case Simple::ToButton::CDown:
        return "C Down";
    case Simple::ToButton::CLeft:
        return "C Left";
    case Simple::ToButton::CRight:
        return "C Right";
    case Simple::ToButton::StickUp:
        return "Stick Up";
    case Simple::ToButton::StickDown:
        return "Stick Down";
    case Simple::ToButton::StickLeft:
        return "Stick Left";
    case Simple::ToButton::StickRight:
        return "Stick Right";
    case Simple::ToButton::LoadState:
        return "Load State";
    case Simple::ToButton::SaveState:
        return "Save State";
    case Simple::ToButton::UnlockFPS:
        return "Unlock FPS";
    case Simple::ToButton::LockFPS:
        return "Lock FPS";
    }

    return nullptr;
}

static const char* toString(Simple::FromStick stick)
{
    switch (stick)
    {
    case Simple::FromStick::Left:
        return "Left";
    case Simple::FromStick::Right:
        return "Right";
    default:
        return "?";
    }
}

static X360::IEventPtr makeMapper(Simple::FromButton from)
{
    switch (from)
    {
#define ENUMSTR(name) case Simple::FromButton::name: return std::make_shared<X360::Button>(X360::Buttons::name);
        SIMPLE_FROM_BUTTONS_X360
#undef ENUMSTR

    case Simple::FromButton::LeftStickUp:
        return std::make_shared<X360::Thumb>(X360::Thumbs::LeftY, ControllerInterface::AxisComparerType::More, Simple::X360ThumbToButtonRange);
    case Simple::FromButton::LeftStickDown:
        return std::make_shared<X360::Thumb>(X360::Thumbs::LeftY, ControllerInterface::AxisComparerType::Less, -Simple::X360ThumbToButtonRange);
    case Simple::FromButton::LeftStickLeft:
        return std::make_shared<X360::Thumb>(X360::Thumbs::LeftX, ControllerInterface::AxisComparerType::More, Simple::X360ThumbToButtonRange);
    case Simple::FromButton::LeftStickRight:
        return std::make_shared<X360::Thumb>(X360::Thumbs::LeftX, ControllerInterface::AxisComparerType::Less, -Simple::X360ThumbToButtonRange);
    case Simple::FromButton::RightStickUp:
        return std::make_shared<X360::Thumb>(X360::Thumbs::RightY, ControllerInterface::AxisComparerType::More, Simple::X360ThumbToButtonRange);
    case Simple::FromButton::RightStickDown:
        return std::make_shared<X360::Thumb>(X360::Thumbs::RightY, ControllerInterface::AxisComparerType::Less, -Simple::X360ThumbToButtonRange);
    case Simple::FromButton::RightStickLeft:
        return std::make_shared<X360::Thumb>(X360::Thumbs::RightX, ControllerInterface::AxisComparerType::More, Simple::X360ThumbToButtonRange);
    case Simple::FromButton::RightStickRight:
        return std::make_shared<X360::Thumb>(X360::Thumbs::RightX, ControllerInterface::AxisComparerType::Less, -Simple::X360ThumbToButtonRange);

    case Simple::FromButton::LeftTrigger:
        return std::make_shared<X360::Trigger>(X360::Triggers::LeftTrigger, ControllerInterface::AxisComparerType::More, Simple::X360TriggerToButtonRange);
    case Simple::FromButton::RightTrigger:
        return std::make_shared<X360::Trigger>(X360::Triggers::RightTrigger, ControllerInterface::AxisComparerType::More, Simple::X360TriggerToButtonRange);

#define ENUMSTR(name) case Simple::FromButton::name: return std::make_shared<Keyboard::Button>(Keyboard::Buttons::name);
#include "Mapper/KeyboardXMacro.h"
#undef ENUMSTR
    }

    MessageBox(NULL, "Unknown FromButton", "Error", MB_ICONERROR);
    return {};
}

static N64::IModifierPtr makeMapper(Simple::ToButton to)
{
    switch (to)
    {
#define ENUMSTR(name) case Simple::ToButton::name: return std::make_shared<N64::Button>(N64::Buttons::name);
        SIMPLE_TO_BUTTONS_N64
#undef ENUMSTR

    case Simple::ToButton::StickUp:
        return std::make_shared<N64::Axis>(N64::Axises::Y, Simple::N64StickToButtonRange);
    case Simple::ToButton::StickDown:
        return std::make_shared<N64::Axis>(N64::Axises::Y, -Simple::N64StickToButtonRange);
    case Simple::ToButton::StickLeft:
        return std::make_shared<N64::Axis>(N64::Axises::X, -Simple::N64StickToButtonRange);
    case Simple::ToButton::StickRight:
        return std::make_shared<N64::Axis>(N64::Axises::X, Simple::N64StickToButtonRange);

    case Simple::ToButton::LoadState:
        return std::make_shared<Luna::Cmd>(LUNA_EXCMD_LOAD_STATE);
    case Simple::ToButton::SaveState:
        return std::make_shared<Luna::Cmd>(LUNA_EXCMD_SAVE_STATE);
    case Simple::ToButton::UnlockFPS:
        return std::make_shared<Luna::Cmd>(LUNA_EXCMD_UNLOCK_FPS);
    case Simple::ToButton::LockFPS:
        return std::make_shared<Luna::Cmd>(LUNA_EXCMD_LOCK_FPS);
    }

    MessageBox(NULL, "Unknown ToButton", "Error", MB_ICONERROR);
    return {};
}

static Mapping::IMapperPtr makeMapper(Simple::FromButton from, Simple::ToButton to)
{
    auto fromMapper = makeMapper(from);
    auto toMapper = makeMapper(to);

    if (!fromMapper || !toMapper)
    {
        return {};
    }

    return std::make_shared<Mapping::Digital::Mapper>(fromMapper, toMapper);
}

static std::optional<Simple::FromButton> fromX360ToButton(SHORT wButtons)
{
    if (wButtons & XINPUT_GAMEPAD_DPAD_UP)        return Simple::FromButton::DpadUp;
    if (wButtons & XINPUT_GAMEPAD_DPAD_DOWN)      return Simple::FromButton::DpadDown;
    if (wButtons & XINPUT_GAMEPAD_DPAD_LEFT)      return Simple::FromButton::DpadLeft;
    if (wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)     return Simple::FromButton::DpadRight;
    if (wButtons & XINPUT_GAMEPAD_START)          return Simple::FromButton::Start;
    if (wButtons & XINPUT_GAMEPAD_BACK)           return Simple::FromButton::Back;
    if (wButtons & XINPUT_GAMEPAD_LEFT_THUMB)     return Simple::FromButton::LeftThumb;
    if (wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)    return Simple::FromButton::RightThumb;
    if (wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)  return Simple::FromButton::L;
    if (wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) return Simple::FromButton::R;
    if (wButtons & XINPUT_GAMEPAD_A)              return Simple::FromButton::A;
    if (wButtons & XINPUT_GAMEPAD_B)              return Simple::FromButton::B;
    if (wButtons & XINPUT_GAMEPAD_X)              return Simple::FromButton::X;
    if (wButtons & XINPUT_GAMEPAD_Y)              return Simple::FromButton::Y;

    // Undoc?
    if (wButtons & 0x0400)                        return Simple::FromButton::Guide;

    return std::nullopt;
}

static inline int asScreenInt(int d, float val)
{
    return (int)(d * (1.f + val) / 2.f);
}

LRESULT CALLBACK Dlg::PictureSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    return ((Dlg*)dwRefData)->pictureSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Dlg::pictureSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hWindowDC = ::BeginPaint(hWnd, &ps);

        RECT rect;
        ::GetClientRect(hWnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        HDC hMemDC = CreateCompatibleDC(hWindowDC);
        HBITMAP hMemBmp = CreateCompatibleBitmap(hWindowDC, width, height);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);

        HBRUSH hBg = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hMemDC, &rect, hBg);
        DeleteObject(hBg);

        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
        HPEN hOldPen = (HPEN)SelectObject(hMemDC, hPen);

        MoveToEx(hMemDC, asScreenInt(width, -80.f / 128.f), asScreenInt(height, 0), NULL);
        LineTo(hMemDC, asScreenInt(width, -70.f / 128.f), asScreenInt(height, -70.f / 128.f));
        LineTo(hMemDC, asScreenInt(width, 0), asScreenInt(height, -80.f / 128.f));
        LineTo(hMemDC, asScreenInt(width, 70.f / 128.f), asScreenInt(height, -70.f / 128.f));
        LineTo(hMemDC, asScreenInt(width, 80.f / 128.f), asScreenInt(height, 0));
        LineTo(hMemDC, asScreenInt(width, 70.f / 128.f), asScreenInt(height, 70.f / 128.f));
        LineTo(hMemDC, asScreenInt(width, 0), asScreenInt(height, 80.f / 128.f));
        LineTo(hMemDC, asScreenInt(width, -70.f / 128.f), asScreenInt(height, 70.f / 128.f));
        LineTo(hMemDC, asScreenInt(width, -80.f / 128.f), asScreenInt(height, 0));

        MoveToEx(hMemDC, asScreenInt(width, drawY_) - 1, asScreenInt(height, -drawX_), NULL);
        LineTo(hMemDC, asScreenInt(width, drawY_) + 1, asScreenInt(height, -drawX_));
        MoveToEx(hMemDC, asScreenInt(width, drawY_), asScreenInt(height, -drawX_) - 1, NULL);
        LineTo(hMemDC, asScreenInt(width, drawY_), asScreenInt(height, -drawX_) + 1);

        SelectObject(hMemDC, hOldPen);
        DeleteObject(hPen);

        BitBlt(hWindowDC, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hOldBmp);
        DeleteObject(hMemBmp);
        DeleteDC(hMemDC);

        ::EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_NCDESTROY:
        ::RemoveWindowSubclass(hWnd, PictureSubclassProc, (uintptr_t)this);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Dlg::onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetTimer((UINT_PTR)this, USER_TIMER_MINIMUM, onTimer);

    controls_.Attach(GetDlgItem(IDC_LIST_MAPPINGS));
    types_.Attach(GetDlgItem(IDC_COMBO_TYPE));

    digitalXboxOptions_.Attach(GetDlgItem(IDC_COMBO_XBOX));
    digitalXboxKeyboard_.Attach(GetDlgItem(IDC_KEY_CHOOSE));
    digitalN64Options_.Attach(GetDlgItem(IDC_COMBO_N64));
    digitalN64Active_.Attach(GetDlgItem(IDC_N64_ACTIVE));
    digitalKeyboard_.Attach(GetDlgItem(IDC_KEY_CHOOSE));

    stickLabel1_.Attach(GetDlgItem(ID_LBL_DEADZONE2));
    stickLabel2_.Attach(GetDlgItem(ID_LBL_DEADZONE));
    stickLabel3_.Attach(GetDlgItem(IDC_LBL_RANGE));
    stickPicture_.Attach(GetDlgItem(IDC_STICK_DRAW));
    stickXboxOptions_.Attach(GetDlgItem(IDC_COMBO_XBOX_STICKS));
    stickDeadzone_.Attach(GetDlgItem(IDC_DEADZONE));
    stickAngleDeadzone_.Attach(GetDlgItem(IDC_ANGLE_DEADZONE));
    stickStretching_.Attach(GetDlgItem(IDC_STRETCH));
    stickRange_.Attach(GetDlgItem(IDC_N64_RANGE));
    stickAngleDeadzone8Dir_.Attach(GetDlgItem(IDC_WANT_DIAGONAL_DZ));
    stickStretchingDiagonal_.Attach(GetDlgItem(IDC_STRETCH_DIAGONALS));
    stickDeadzoneSpin_.Attach(GetDlgItem(IDC_SPIN_DEADZONE));
    stickAngleDeadzoneSpin_.Attach(GetDlgItem(IDC_SPIN_ANGLE));

    SetWindowSubclass(stickPicture_.m_hWnd, PictureSubclassProc, (UINT_PTR)this, (DWORD_PTR)this);

    digitalN64Active_.EnableWindow(FALSE);

    for (int i = 0; i < (int)Simple::FromButton::Count; i++)
    {
        digitalXboxOptions_.AddString(toString((Simple::FromButton)i));
    }
    for (int i = 0; i < (int)Simple::FromStick::Count; i++)
    {
        stickXboxOptions_.AddString(toString((Simple::FromStick)i));
    }

    types_.AddString("Digital");
    types_.AddString("Stick");

    for (int i = 0; i < (int)Simple::ToButton::Count; i++)
    {
        digitalN64Options_.AddString(toString((Simple::ToButton)i));
    }

    controls_.ModifyStyle(0, LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL);
    controls_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    controls_.InsertColumn(0, "Type", LVCFMT_LEFT, 50, 0);
    controls_.InsertColumn(1, "Xbox", LVCFMT_LEFT, 100, 0);
    controls_.InsertColumn(2, "N64", LVCFMT_LEFT, 100, 0);

    try
    {
        config_ = YAML::LoadFile(Win::ConfigPath()).as<Config>();
    }
    catch (...)
    {
        config_ = YAML::Load(kDefaultConfig).as<Config>();
    }

    refresh();
    setSelectedIndex(0);
    return 0;
}

void Dlg::refresh()
{
    controls_.DeleteAllItems();

    for (size_t i = 0; i < config_.mappers.size(); i++)
    {
        controls_.InsertItem(i, "");
        refreshAt(i);
    }
}

void Dlg::refreshAt(size_t index)
{
    const auto& mapper = config_.mappers[index];
    auto mapperDesc = mapper->ToSimpleConfig();
    if (!mapperDesc)
    {
        controls_.SetItemText(index, 0, "?");
        controls_.SetItemText(index, 1, "?");
        controls_.SetItemText(index, 2, "?");
        return;
    }

    const char* type = nullptr;
    const char* from = nullptr;
    std::string to;

    if (auto digital = std::get_if<Simple::ButtonMapping>(&(*mapperDesc)))
    {
        type = "Digital";
        from = toString(digital->from);
        to = toString(digital->to);
    }

    if (auto stick = std::get_if<Simple::StickMapping>(&(*mapperDesc)))
    {
        type = "Stick";
        from = toString(stick->from);
        to = "Stick";
    }

    controls_.SetItemText(index, 0, type);
    controls_.SetItemText(index, 1, from);
    controls_.SetItemText(index, 2, to.c_str());
}

LRESULT Dlg::onSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    saved_ = true;
    EndDialog(wID);
    return 0;
}

LRESULT Dlg::onCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    saved_ = false;
    EndDialog(wID);
    return 0;
}

LRESULT Dlg::onListItemActivate(NMHDR* phdr)
{
    return 0;
}

LRESULT Dlg::onListItemDeleted(NMHDR* phdr)
{
    return 0;
}

void Dlg::refreshTypeWindows(int type)
{
    if (type > 2)
        type = 2;
    if (type < 0)
        type = -1;

    if (type == curDrawnType_)
        return;

    static CWindow* digitals[] = { &digitalN64Options_, &digitalN64Active_, &digitalKeyboard_, &digitalXboxOptions_ };
    static CWindow* sticks[] = { &stickLabel1_,
        &stickLabel2_,
        &stickLabel3_,
        &stickPicture_,
        &stickDeadzone_,
        &stickAngleDeadzone_,
        &stickStretching_,
        &stickRange_,
        &stickAngleDeadzone8Dir_,
        &stickStretchingDiagonal_,
        &stickDeadzoneSpin_,
        &stickAngleDeadzoneSpin_,
        &stickXboxOptions_
    };

    for (auto* w : digitals) w->ShowWindow(FALSE);
    for (auto* w : sticks) w->ShowWindow(FALSE);

    types_.SetCurSel(type);
    switch (type)
    {
    case 0:
        for (auto* w : digitals) w->ShowWindow(TRUE);
        break;

    case 1:
        for (auto* w : sticks) w->ShowWindow(TRUE);
        break;
    }

    curDrawnType_ = type;
}

LRESULT Dlg::onListItemChanged(NMHDR* phdr)
{
    int wantType = 0;

    int index = selectedIndex();
    selectedIndex_ = index;
    if (index < 0 || index >= (int)config_.mappers.size())
    {
        refreshTypeWindows(-1);
        return 0;
    }

    const auto& mapper = config_.mappers[index];
    auto mapperDesc = mapper->ToSimpleConfig();
    if (!mapperDesc)
    {
        refreshTypeWindows(2);
        return 0;
    }

    if (auto digital = std::get_if<Simple::ButtonMapping>(&(*mapperDesc)))
    {
        refreshTypeWindows(0);
        digitalXboxOptions_.SetCurSel((int)digital->from);
        digitalN64Options_.SetCurSel((int)digital->to);
    }

    if (auto stick = std::get_if<Simple::StickMapping>(&(*mapperDesc)))
    {
        refreshTypeWindows(1);
        stickXboxOptions_.SetCurSel((int)stick->from);
        stickDeadzone_.SetWindowTextA(std::to_string((int)roundf(stick->deadzone * 100.f)).c_str());
        stickAngleDeadzone_.SetWindowTextA(std::to_string((int)roundf(stick->angleDeadzone * 100.f)).c_str());
        stickAngleDeadzone8Dir_.SetCheck(stick->angleDeadzoneWithDiagonals ? BST_CHECKED : BST_UNCHECKED);
        stickStretchingDiagonal_.SetCheck(stick->stretcher ? BST_CHECKED : BST_UNCHECKED);
        stickStretching_.SetWindowTextA(std::to_string((int)roundf(stick->stretcher * 100.f)).c_str());
        stickRange_.SetWindowTextA(std::to_string(stick->range).c_str());
    }

    return 0;
}

void Dlg::choosingReset()
{
    digitalKeyboard_.SetWindowText("Choose");
    choosing_.reset();
    controls_.EnableWindow(TRUE);
    types_.EnableWindow(TRUE);
}

LRESULT Dlg::onKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    unsigned k = (unsigned)wParam;
    if (k < sizeof(activeKeys_))
        activeKeys_[k] = true;

    return 0;
}

LRESULT Dlg::onKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    auto index = selectedIndex_;
    bool validIndex = index >= 0 && index < (int)config_.mappers.size();
    unsigned k = (unsigned)wParam;

    if (choosing_)
    {
        if (auto button = keyboardToFromButton(k))
        {
            digitalN64Options_.SetCurSel((int)*button);
            choosingReset();
        }
    }

    if (k < sizeof(activeKeys_))
        activeKeys_[k] = false;

    return 0;
}

void Dlg::onTimer(HWND, UINT, UINT_PTR ptr, DWORD)
{
    return ((Dlg*)ptr)->onTimer();
}

void Dlg::onTimer()
{
    XINPUT_STATE state;
    XInputGetState(0, &state);

    if (choosing_)
    {
        auto now = std::chrono::steady_clock::now();
        if (now >= choosing_->deadline)
        {
            choosingReset();
        }

        auto secondsLeft = std::chrono::duration_cast<std::chrono::seconds>(choosing_->deadline - now).count();
        if (secondsLeft != choosing_->lastTime)
        {
            choosing_->lastTime = secondsLeft;
            std::string text = "Waiting... (";
            text += std::to_string(secondsLeft);
            text += "s)";
            digitalKeyboard_.SetWindowText(text.c_str());
        }

        if (auto from = fromX360ToButton(state.Gamepad.wButtons))
        {
            digitalXboxOptions_.SetCurSel((int)*from);
            choosingReset();
        }
    }

    auto index = selectedIndex_;
    const auto& mapper = config_.mappers[index];

    X360::Controller from = state.Gamepad;
    N64::Controller to;
    mapper->Map(from, activeKeys_, to);

    drawX_ = to.X_AXIS / 128.f;
    drawY_ = to.Y_AXIS / 128.f;
    stickPicture_.Invalidate();
}

LRESULT Dlg::onReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    config_ = YAML::Load(kDefaultConfig).as<Config>();
    refresh();
    return 0;
}

LRESULT Dlg::onUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int idxA = selectedIndex();
    int idxB = idxA - 1;
    if (idxB < 0)
        return 0;

    auto w0 = std::move(config_.mappers[idxA]);
    auto w1 = std::move(config_.mappers[idxB]);

    config_.mappers[idxA] = std::move(w1);
    config_.mappers[idxB] = std::move(w0);

    refreshAt(idxA);
    refreshAt(idxB);
    setSelectedIndex(idxB);
    return 0;
}

LRESULT Dlg::onDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int idxA = selectedIndex();
    int idxB = idxA + 1;
    if (idxB >= (int)config_.mappers.size())
        return 0;

    auto w0 = std::move(config_.mappers[idxA]);
    auto w1 = std::move(config_.mappers[idxB]);

    config_.mappers[idxA] = std::move(w1);
    config_.mappers[idxB] = std::move(w0);

    refreshAt(idxA);
    refreshAt(idxB);
    setSelectedIndex(idxB);
    return 0;
}

LRESULT Dlg::onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int idx = selectedIndex();
    if (idx < 0 || idx >= (int)config_.mappers.size())
        idx = config_.mappers.size() - 1;

    Mapping::Mappers mappers;

    for (size_t i = 0; i < config_.mappers.size(); i++)
    {
        if (i == idx)
        {
            auto ev = std::make_shared<X360::Button>(X360::Buttons::A);
            auto mod = std::make_shared<N64::Button>(N64::Buttons::A);
            mappers.push_back(std::make_shared<Mapping::Digital::Mapper>(std::move(ev), std::move(mod)));
        }

        mappers.push_back(config_.mappers[i]);
    }

    config_.mappers = std::move(mappers);
    refresh();
    setSelectedIndex(idx - 1);
    return 0;
}

LRESULT Dlg::onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int idx = selectedIndex();
    if (idx < 0 || idx >= (int)config_.mappers.size())
        return 0;

    Mapping::Mappers mappers;

    for (size_t i = 0; i < config_.mappers.size(); i++)
    {
        if (i != idx)
            mappers.push_back(config_.mappers[i]);
    }

    config_.mappers = std::move(mappers);
    controls_.DeleteItem(idx);
    setSelectedIndex(idx);
    return 0;
}

LRESULT Dlg::onCalibrate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int time = 10;
    choosing_ = ChoosingContext{
        .lastTime = time,
        .deadline = std::chrono::steady_clock::now() + std::chrono::seconds(time)
    };
    controls_.EnableWindow(FALSE);
    types_.EnableWindow(FALSE);

    return 0;
}

int Dlg::selectedIndex()
{
    return controls_.GetSelectedIndex();
}

std::optional<Simple::FromButton> Dlg::selectedFromButton()
{
    int index = digitalXboxOptions_.GetCurSel();
    if (index < 0 || index >= digitalXboxOptions_.GetCount())
        return std::nullopt;

    return (Simple::FromButton)index;
}

std::optional<Simple::FromStick> Dlg::selectedFromStick()
{
    int index = stickXboxOptions_.GetCurSel();
    if (index < 0 || index >= stickXboxOptions_.GetCount())
        return std::nullopt;

    return (Simple::FromStick)index;
}

std::optional<Simple::ToButton> Dlg::selectedToButton()
{
    int index = digitalN64Options_.GetCurSel();
    if (index < 0 || index >= digitalN64Options_.GetCount())
        return std::nullopt;

    return (Simple::ToButton)index;
}

void Dlg::setSelectedIndex(int index)
{
    if (index >= controls_.GetItemCount())
        return;
    if (index < 0)
        return;

    controls_.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    controls_.EnsureVisible(index, FALSE);
}

void Dlg::refreshDigital()
{
    int idx = selectedIndex();
    if (idx < 0 || idx >= (int)config_.mappers.size())
        return;

    auto from = selectedFromButton();
    if (!from)
        return;

    auto to = selectedToButton();
    if (!to)
        return;

    if (auto simple = config_.mappers[idx]->ToSimpleConfig())
    {
        if (auto digital = std::get_if<Simple::ButtonMapping>(&(*simple)))
        {
            if (digital->to == *to && digital->from == *from)
            {
                return;
            }
        }
    }

    config_.mappers[idx] = makeMapper(*from, *to);
    refreshAt(idx);
}

static inline std::optional<std::string> extract(CEdit& edit)
{
    int nLen = edit.GetWindowTextLength();
    if (nLen > 0)
    {
        char* label = new char[nLen + 1];
        edit.GetWindowTextA(label, nLen + 1);
        std::string result(label);
        delete[] label;
        return result;
    }

    return {};
}

void Dlg::refreshStick()
{
    int idx = selectedIndex();
    if (idx < 0 || idx >= (int)config_.mappers.size())
        return;

    auto from = selectedFromStick();
    if (!from)
        return;

    float deadzone = 0.f;
    float angleDeadzone = 0.f;
    bool angleDeadzoneWithDiagonals = false;
    float stretch = 0.f;
    bool stretcherDiagonal = false;
    int range = 80;
    try
    {
        if (auto str = extract(stickDeadzone_))
            deadzone = std::stof(*str);
        if (auto str = extract(stickAngleDeadzone_))
            angleDeadzone = std::stof(*str);
        angleDeadzoneWithDiagonals = stickAngleDeadzone8Dir_.GetCheck() == BST_CHECKED;
        if (auto str = extract(stickStretching_))
            stretch = std::stof(*str);
        stretcherDiagonal = stickStretchingDiagonal_.GetCheck() == BST_CHECKED;
        if (auto str = extract(stickRange_))
            range = std::stoi(*str);
    }
    catch (...)
    {
        return;
    }

    if (auto simple = config_.mappers[idx]->ToSimpleConfig())
    {
        if (auto stick = std::get_if<Simple::StickMapping>(&(*simple)))
        {
            if (stick->from == *from &&
                Simple::similar(stick->deadzone, deadzone / 100.f) &&
                Simple::similar(stick->angleDeadzone, angleDeadzone / 100.f) &&
                (Simple::similar(angleDeadzone, 0.f) || stick->angleDeadzoneWithDiagonals == angleDeadzoneWithDiagonals) &&
                Simple::similar(stick->stretcher, stretch / 100.f) &&
                stick->range == range)
            {
                return;
            }
        }
    }

    deadzone = std::clamp(deadzone, 0.f, 100.f);
    angleDeadzone = std::clamp(angleDeadzone, 0.f, 100.f);
    stretch = std::clamp(stretch, 0.f, 50.f);
    range = std::clamp(range, 1, 127);

    Mapping::Analog::BilinearStickMapper::Stretcher stretcher;
    if (stretch)
        stretcher.emplace(Simple::ToStretch - stretch / 100.f, Simple::ToStretch, 1.f);

    Mapping::Analog::BilinearStickMapper::Deadzoner deadzoner;
    if (deadzone)
        deadzoner.emplace(deadzone / 100.f);

    Mapping::Analog::BilinearStickMapper::AngleLimiter angleDeadzoner;
    if (angleDeadzone)
        angleDeadzoner.emplace(angleDeadzoneWithDiagonals ? 8 : 4, angleDeadzone / 100.f);

    X360::ThumbsConverter fX(from == Simple::FromStick::Left ? X360::Thumbs::LeftX : X360::Thumbs::RightX, 0, 32000);
    X360::ThumbsConverter fY(from == Simple::FromStick::Left ? X360::Thumbs::LeftY : X360::Thumbs::RightY, 0, 32000);
    N64::AxisConverter tX(N64::Axises::X, 0, range);
    N64::AxisConverter tY(N64::Axises::Y, 0, range);

    config_.mappers[idx] = std::make_shared<Mapping::Analog::BilinearStickMapper>(fX, fY, tX, tY, stretcher, deadzoner, std::nullopt, angleDeadzoner);
    refreshAt(idx);
}

LRESULT Dlg::onDigitalXboxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    refreshDigital();
    return 0;
}

LRESULT Dlg::onDigitalN64Changed(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    refreshDigital();
    return 0;
}

LRESULT Dlg::onTypeChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int idx = selectedIndex();
    if (idx < 0 || idx >= (int)config_.mappers.size())
        return 0;

    refreshTypeWindows(types_.GetCurSel());
    return 0;
}

void DialogPresent(HWND)
{
    Dlg dlg;

    auto err = dlg.DoModal();
    auto errVal = GetLastError();
    if (dlg.Saved())
    {
        const auto& config = dlg.GetConfig();

        YAML::Node node = YAML::Node(config);
        try
        {
            std::ofstream fout(Win::ConfigPath(), std::ios::out | std::ios::trunc);
            fout << node;
        }
        catch (...) {}
    }
}

class WtlModule : public CAppModule
{
public:
    WtlModule(HINSTANCE hinst) {
        Init(NULL, hinst);
    }
    virtual ~WtlModule(void) {
        Term();
    }
};

static WtlModule* gWtlModule = NULL;

void ConfigInit(HMODULE hinst)
{
    gWtlModule = new WtlModule((HINSTANCE)hinst);
}

void ConfigCleanup(void)
{
    if (gWtlModule) {
        delete gWtlModule;
        gWtlModule = NULL;
    }
}

LRESULT Dlg::onStickChangeDeadzone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    refreshStick();
    return 0;
}

LRESULT Dlg::onStickChangeAngleDeadzone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    refreshStick();
    return 0;
}

LRESULT Dlg::onStickClickedWantDiagonalDz(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    refreshStick();
    return 0;
}

LRESULT Dlg::onStickChangeStretch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    refreshStick();
    return 0;
}

LRESULT Dlg::onStickChangeN64Range(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    refreshStick();
    return 0;
}

LRESULT Dlg::onStickClickedStretchDiagonals(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    refreshStick();
    return 0;
}
