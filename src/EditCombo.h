/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// Combination of an edit box with a toolbar action buttons on the far right
// Toolbar buttons are down arrow (for recent items), and one two-state toggle button (on/off, or active/inactive)

#include "ToolBarWnd.h"
#include "ACListWnd.h"
#include "signals.h"
#include "EditEsc.h"


class EditCombo : public CWnd
{
// Construction
public:
	EditCombo();

// Attributes
public:
	CEdit& GetEditCtrl();

	AutoCompletePopup& GetAutoComplete();

	void SetParams(int toolbar_images, int state_1_cmd, int state_2_cmd, UINT flags);

	void SetMargins(const CRect& margins);

	void SetColors(COLORREF background, COLORREF editbox, COLORREF text);

// Operations
public:
	enum Flags { NONE= 0, AUTO_COMPLETE= 1, TWO_BUTTONS= 2 };

	bool Create(CWnd* parent, int toolbar_images, int state_1_cmd, int state_2_cmd, UINT flags);

	// populate auto complete history
	void SetHistory(const std::vector<String>& history);

	// select toolbar button
	void SetState(int state);

	// enable drop down button in a toolbar
	void EnableDropDown(bool enable);

	// enable button in a toolbar
	void EnableButton(bool enable, int index);

	// image/icon in fron of the text box
	void SetImageList(HIMAGELIST image_list);
	void SetImageIndex(int image);
	void SetImage(int image_rsrc_id);

	// events
	typedef boost::signals2::signal<void (int cmd)> RunCommand;
	typedef boost::signals2::signal<void (bool ok)> FinishCommand;

	// connect handler to the event
	slot_connection ConnectRunCommand(RunCommand::slot_function_type fn);
	slot_connection ConnectFinishCommand(FinishCommand::slot_function_type fn);

// Implementation
public:
	virtual ~EditCombo();

	virtual BOOL PreTranslateMessage(MSG* msg);

	// message map functions
protected:
	void Run(int cmd);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	void OnSize(UINT type, int cx, int cy);
	void OnTbButton(UINT);
	void OnStopScan();
	void OnSetFocus();
	void OnKillFocus();
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnTbGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	void OnAutoCompleteDropDown();
	void OnGetInfoTip(NMHDR* nmhdr, LRESULT* result);
	void EndEdit(int key);
	void OnTextChanged();
	HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);

	virtual void PreSubclassWindow();

	DECLARE_MESSAGE_MAP()

private:
	HIMAGELIST image_list_;
	EditEsc edit_box_;
	ToolBarWnd toolbar_;
	bool focus_;
	bool scan_in_progress_;
	bool create_controls_;
	AutoCompletePopup auto_complete_;
	RunCommand run_cmd_event_;
	FinishCommand finish_cmd_event_;
	int button_commands_[2];
	int tb_bitmap_id_;
	int state_;
	int margin_;
	int image_to_draw_;
	UINT create_flags_;
	std::auto_ptr<Gdiplus::Bitmap> image_;
	CRect margins_;
	COLORREF outside_color_;
	COLORREF backgnd_color_;
	CBrush  backgnd_color_brush_;
	COLORREF text_color_;


	int GetButtonCmd(UINT btn) const;
	void Resize();
	void CreateControls();
};
