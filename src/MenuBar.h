////////////////////////////////////////////////////////////////
// Copyright 1998 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#ifndef __MENUBAR_H
#define __MENUBAR_H

#include "subclass.h"

#include "ToolBarWnd.h"

//////////////////
// CMenuBar uses this private class to intercept messages on behalf
// of its owning frame, as well as the MDI client window. Conceptually,
// these should be two different hooks, but I want to save code.
//
class CMenuBarFrameHook : public CSubclassWnd
{
protected:
	friend class CMenuBar;
	CMenuBar* menu_bar_;
	CMenuBarFrameHook();
	virtual ~CMenuBarFrameHook();
	BOOL Install(CMenuBar* menu_bar, HWND wnd_to_hook);
	virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);
};

//////////////////
// CMenuBar implements an Office 97-style menu bar. Use it the way you would
// a CToolBar, only you need not call LoadToolbar. All you have to do is
//
// * Create the CMenuBar from your OnCreate or OnCreateBands handler.
//
// * Call LoadMenu to load a menu. This will set your frame's menu to NULL.
//
// * Implemenent your frame's PreTranslateMessage function, to call
//   CMenuBar::TranslateFrameMessage. 
//
class CMenuBar : public ToolBarWnd
{
public:
	BOOL auto_remove_frame_menu_;		// set frame's menu to NULL

	CMenuBar();
	virtual ~CMenuBar();

	bool Create(CWnd* parent, UINT menu_id);

	// You must call this from your frame's PreTranslateMessage fn
	virtual BOOL TranslateFrameMessage(MSG* msg);

	HMENU LoadMenu(HMENU hmenu);			// load menu
	HMENU LoadMenu(LPTSTR menu_name);		// ...from resource file
	HMENU LoadMenu(UINT id) { return LoadMenu(MAKEINTRESOURCE(id)); }

	HMENU GetMenu() { return hmenu_; }		// get current menu

	enum TRACKINGSTATE { // menubar has three states:
		TRACK_NONE = 0,   // * normal, not tracking anything
		TRACK_BUTTON,     // * tracking buttons (F10/Alt mode)
		TRACK_POPUP       // * tracking popups
	};

	TRACKINGSTATE GetTrackingState(int& popup) {
		popup = popup_tracking_; return tracking_state_;
	}
	static BOOL TRACE_;					// set TRUE to see TRACE msgs

protected:
	friend class CMenuBarFrameHook;

	CMenuBarFrameHook frame_hook_;		// hooks frame window messages
	HMENU hmenu_;						// the menu

	// menu tracking stuff:
	int popup_tracking_;				// which popup I'm tracking if any
	int new_popup_;						// next menu to track
	BOOL process_right_arrow_;			// process l/r arrow keys?
	BOOL process_left_arrow_;			// ...
	BOOL escape_was_pressed_;			// user pressed escape to exit menu
	CPoint mouse_;						// mouse location when tracking popup
	HMENU menu_tracking_;				// current popup I'm tracking
	CWnd* parent_;

	TRACKINGSTATE tracking_state_;		// current tracking state

	// helpers
	void RecomputeToolbarSize();
	void RecomputeMenuLayout();
	void UpdateFont();
	int GetNextOrPrevButton(int btn, BOOL prev);
	void SetTrackingState(TRACKINGSTATE iState, int btn=-1);
	void TrackPopup(int btn);
	void ToggleTrackButtonMode();
	void CancelMenuAndTrackNewOne(int btn);
	void OnMenuSelect(HMENU hmenu, UINT item_id);
	CPoint ComputeMenuTrackPoint(const CRect& btn, TPMPARAMS& tpm);

	BOOL IsValidButton(int btn) const
	{ return 0 <= btn && btn < GetButtonCount(); }

	virtual BOOL OnMenuInput(MSG& m);	// handle popup menu input

	// overrides
//	virtual void OnBarStyleChange(DWORD old_style, DWORD new_style);
	int HitTest(CPoint p) const;

	// command/message handlers
	afx_msg int  OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnUpdateMenuButton(CCmdUI* cmd_ui);
	afx_msg LRESULT OnSetMenuNull(WPARAM wp, LPARAM lp);
	afx_msg void OnCustomDraw(NMHDR* nmhdr, LRESULT* result);
	void OnMenuButton(UINT cmd);

	static LRESULT CALLBACK MenuInputFilter(int code, WPARAM wp, LPARAM lp);
	
	DECLARE_MESSAGE_MAP()
	 //{{AFX_MSG(CMenuBar)
	//}}AFX_MSG

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif
