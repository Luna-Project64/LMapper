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
#include "Win.h"

extern const char* kDefaultConfig;

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
        COMMAND_ID_HANDLER(IDOK, onSave)
        COMMAND_ID_HANDLER(IDCANCEL, onCancel)
        COMMAND_ID_HANDLER(ID_RESET, onReset)
        COMMAND_ID_HANDLER(ID_BUTTON_UP, onUp)
        COMMAND_ID_HANDLER(ID_BUTTON_DOWN, onDown)
        COMMAND_ID_HANDLER(ID_ADD, onAdd)
        COMMAND_ID_HANDLER(ID_REMOVE, onRemove)
        NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_ITEMACTIVATE, onListItemActivate)
        NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_DELETEITEM, onListItemDeleted)
        NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_ITEMCHANGED, onListItemChanged)
        END_MSG_MAP()

    bool Saved(void) const { return saved_; }

protected:
    bool saved_ = false;
    CListViewCtrl controls_;
    CComboBox types_;

    CComboBox digitalXboxOptions_;
    CButton digitalXboxKeyboard_;
    CComboBox digitalN64Options_;
    CButton digitalN64Active_;

    CWindow stickLabel1_;
    CWindow stickLabel2_;
    CWindow stickPicture_;
    CEdit stickDeadzone_;
    CEdit stickAngleDeadzone_;
    CEdit stickStretching_;
    CButton stickAngleDeadzone8Dir_;
    CButton stickStretchingDiagonal_;

    Config config_;

    LRESULT onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT onSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT onCalibrate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT onListItemActivate(NMHDR* phdr);
    LRESULT onListItemDeleted(NMHDR* phdr);
    LRESULT onListItemChanged(NMHDR* phdr);

    void refresh();
    void refreshAt(size_t index);

    int selectedIndex();
    void setSelectedIndex(int index);
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


LRESULT Dlg::onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    controls_.Attach(GetDlgItem(IDC_LIST_MAPPINGS));
    types_.Attach(GetDlgItem(IDC_COMBO_TYPE));

    digitalXboxOptions_.Attach(GetDlgItem(IDC_COMBO_XBOX));
    digitalXboxKeyboard_.Attach(GetDlgItem(IDC_KEY_CHOOSE));
    digitalN64Options_.Attach(GetDlgItem(IDC_COMBO_N64));
    digitalN64Active_.Attach(GetDlgItem(IDC_N64_ACTIVE));
    digitalKeyboard_.Attach(GetDlgItem(IDC_KEY_CHOOSE));

    stickLabel1_.Attach(GetDlgItem(ID_LBL_DEADZONE2));
    stickLabel2_.Attach(GetDlgItem(ID_LBL_DEADZONE));
    stickPicture_.Attach(GetDlgItem(IDC_STICK_DRAW));
    stickDeadzone_.Attach(GetDlgItem(IDC_DEADZONE));
    stickAngleDeadzone_.Attach(GetDlgItem(IDC_ANGLE_DEADZONE));
    stickStretching_.Attach(GetDlgItem(IDC_STRETCH));
    stickAngleDeadzone8Dir_.Attach(GetDlgItem(IDC_WANT_DIAGONAL_DZ));
    stickStretchingDiagonal_.Attach(GetDlgItem(IDC_STRETCH_DIAGONALS));

    types_.AddString("Digital");
    types_.AddString("Stick");

    for (int i = 0; i < (int)Simple::FromButton::Count; i++)
    {
        digitalXboxOptions_.AddString(toString((Simple::FromButton)i));
    }

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

LRESULT Dlg::onListItemChanged(NMHDR* phdr)
{
    static CWindow* digitals[] = { &digitalXboxOptions_, &digitalN64Options_, &digitalN64Active_ };
    digitalXboxKeyboard_.ShowWindow(FALSE);
    static CWindow* sticks[] = { &stickLabel1_, &stickLabel2_, &stickPicture_, &stickDeadzone_, &stickAngleDeadzone_, &stickStretching_, &stickAngleDeadzone8Dir_, &stickStretchingDiagonal_ };

    for (auto* w : digitals) w->ShowWindow(FALSE);
    for (auto* w : sticks) w->ShowWindow(FALSE);

    auto index = selectedIndex();
    if (index < 0 || index >= config_.mappers.size())
        return 0;

    const auto& mapper = config_.mappers[index];
    auto mapperDesc = mapper->ToSimpleConfig();
    if (!mapperDesc)
        return 0;

    if (auto digital = std::get_if<Simple::ButtonMapping>(&(*mapperDesc)))
    {
        for (auto* w : digitals) w->ShowWindow(TRUE);

        types_.SetCurSel(0);
        digitalXboxOptions_.SetCurSel((int)digital->from);
        digitalN64Options_.SetCurSel((int)digital->to);
    }

    if (auto stick = std::get_if<Simple::StickMapping>(&(*mapperDesc)))
    {
        for (auto* w : sticks) w->ShowWindow(TRUE);

        types_.SetCurSel(1);
    }

    return 0;
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
    if (idxB >= config_.mappers.size())
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
    if (idx < 0 || idx >= config_.mappers.size())
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
    if (idx < 0 || idx >= config_.mappers.size())
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
    return 0;
}

int Dlg::selectedIndex()
{
    return controls_.GetSelectedIndex();
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

bool DialogPresent(HWND)
{
    Dlg dlg;

    auto err = dlg.DoModal();
    auto errVal = GetLastError();
    bool accepted = dlg.Saved();

    return accepted;
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
