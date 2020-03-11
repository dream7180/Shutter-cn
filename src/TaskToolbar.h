/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ToolBarWnd.h"

// This is toolbar with big icons representing different tools
// It's docked at the bottom/right of the main window

class TaskToolbar :	public ToolBarWnd
{
public:
	TaskToolbar();

	bool Create(CWnd* parent, UINT id, UINT rebar_band_id, bool vertical);

	CSize GetSize();

	bool IsHorizontal() const;

	void SetOrientation(bool horizontal);

	virtual void ResetToolBar(bool resize_to_fit);

	virtual ~TaskToolbar();

private:
	DECLARE_MESSAGE_MAP()

private:
	//bool small_icons_;
	UINT rebar_band_id_;
	bool horizontal_;

	bool Create(CWnd* parent, UINT id, bool vertical);

	LRESULT OnSizeParent(WPARAM, LPARAM lParam);
	afx_msg void OnToolbarCustomize();
	afx_msg void OnDestroy();
	void OnGetInfoTip(NMHDR*, LRESULT*);
	//void OnRightClick(NMHDR* notify_struct, LRESULT* result);
	void OnContextMenu(CWnd* wnd, CPoint point);
	//void OnUpdateSmallIcons(CCmdUI* cmd_ui);
	//void OnSmallIcons();
	//void OnUpdateLargeIcons(CCmdUI* cmd_ui);
	//void OnLargeIcons();
	void OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu);
	void AdjustReBar();
	void DeleteButtons();
	void OnGetDispInfo(NMHDR* notify_struct, LRESULT* result);
	void OnButtonRestore(NMHDR* notify_struct, LRESULT* result);
	//void OnToolbarHorizontal();
	//void OnToolbarVertical();
	//void OnUpdateVerticalLayout(CCmdUI* cmd_ui);
	//void OnUpdateHorizontalLayout(CCmdUI* cmd_ui);
};
