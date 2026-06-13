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
		COMMAND_ID_HANDLER(ID_CALIBRATE, onCalibrate)
		NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_ITEMACTIVATE, onListItemActivate)
		NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_DELETEITEM, onListItemDeleted)
		NOTIFY_HANDLER_EX(IDC_LIST_MAPPINGS, LVN_ITEMCHANGED, onListItemChanged)
	END_MSG_MAP()

	bool Saved(void) const { return saved_; }

protected:
	bool saved_ = false;
	CListViewCtrl controls_;
	CComboBox types_;

	CSliderCtrl xboxSlider_;
	CComboBox xboxOptions_;
	CButton xboxActive_;

	CSliderCtrl n64Slider_;
	CComboBox n64Options_;
	CButton n64Active_;

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

LRESULT Dlg::onInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	controls_.Attach(GetDlgItem(IDC_LIST_MAPPINGS));
	types_.Attach(GetDlgItem(IDC_COMBO_TYPE));
	xboxSlider_.Attach(GetDlgItem(IDC_XBOX_RANGE));
	xboxOptions_.Attach(GetDlgItem(IDC_COMBO_XBOX));
	xboxActive_.Attach(GetDlgItem(IDC_XBOX_ACTIVE));

	n64Slider_.Attach(GetDlgItem(IDC_N64_RANGE));
	n64Options_.Attach(GetDlgItem(IDC_COMBO_N64));
	n64Active_.Attach(GetDlgItem(IDC_N64_ACTIVE));

	xboxSlider_.ShowWindow(FALSE);
	xboxActive_.ShowWindow(FALSE);
	n64Slider_.ShowWindow(FALSE);
	n64Active_.ShowWindow(FALSE);

	types_.AddString("Digital");
	types_.AddString("Linear");
	types_.AddString("Bilinear");

	xboxOptions_.AddString("A");
	xboxOptions_.AddString("B");
	xboxOptions_.AddString("X");
	xboxOptions_.AddString("Y");
	xboxOptions_.AddString("Start");
	xboxOptions_.AddString("Guide");
	xboxOptions_.AddString("Back");
	xboxOptions_.AddString("Dpad Up");
	xboxOptions_.AddString("Dpad Down");
	xboxOptions_.AddString("Dpad Left");
	xboxOptions_.AddString("Dpad Right");
	xboxOptions_.AddString("L Shoulder");
	xboxOptions_.AddString("R Shoulder");
	xboxOptions_.AddString("L Trigger");
	xboxOptions_.AddString("R Trigger");
	xboxOptions_.AddString("Left Stick X");
	xboxOptions_.AddString("Left Stick Y");
	xboxOptions_.AddString("Left Stick");
	xboxOptions_.AddString("Left Stick Click");
	xboxOptions_.AddString("Right Stick X");
	xboxOptions_.AddString("Right Stick Y");
	xboxOptions_.AddString("Right Stick");
	xboxOptions_.AddString("Right Stick Click");

	n64Options_.AddString("A");
	n64Options_.AddString("B");
	n64Options_.AddString("Start");
	n64Options_.AddString("Dpad Up");
	n64Options_.AddString("Dpad Down");
	n64Options_.AddString("Dpad Left");
	n64Options_.AddString("Dpad Right");
	n64Options_.AddString("C Up");
	n64Options_.AddString("C Down");
	n64Options_.AddString("C Left");
	n64Options_.AddString("C Right");
	n64Options_.AddString("L");
	n64Options_.AddString("R");
	n64Options_.AddString("Z");
	n64Options_.AddString("Stick X");
	n64Options_.AddString("Stick Y");
	n64Options_.AddString("Stick");

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
	auto mapper = config_.mappers[index];
	auto mapperDesc = mapper->ToString();
	if (!mapperDesc)
	{
		controls_.SetItemText(index, 0, "?");
		controls_.SetItemText(index, 1, "?");
		controls_.SetItemText(index, 2, "?");
		return;
	}

	controls_.SetItemText(index, 0, mapperDesc->type.c_str());
	controls_.SetItemText(index, 1, mapperDesc->from.c_str());
	controls_.SetItemText(index, 2, mapperDesc->to.c_str());
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
	auto index = selectedIndex();
	if (index < 0 || index >= config_.mappers.size())
		return 0;

	auto mapper = config_.mappers[index];

	xboxSlider_.ShowWindow(FALSE);
	xboxActive_.ShowWindow(FALSE);
	n64Slider_.ShowWindow(FALSE);
	n64Active_.ShowWindow(FALSE);

	return 0;
}

LRESULT Dlg::onListItemDeleted(NMHDR* phdr)
{
	return 0;
}

LRESULT Dlg::onListItemChanged(NMHDR* phdr)
{
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
