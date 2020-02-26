////////////////////////////////////////////////////////////////
// Copyright 1998 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CMenuBar implements menu bar for MFC. See MenuBar.h for how
// to use, and also the MBTest sample application.
//
// MiK: many changes and fixes

#include "StdAfx.h"
#include "MenuBar.h"
#include "WhistlerLook.h"
#include "AppColors.h"

const UINT MB_SET_MENU_NULL= WM_USER + 1100;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// if you want to see extra TRACE diagnostics, set CMenuBar::TRACE_ = TRUE
BOOL CMenuBar::TRACE_ = FALSE;

#ifdef _DEBUG
#define MBTRACEFN			\
	if (CMenuBar::TRACE_)\
		TRACE
#define MBTRACE			\
	if (CMenuBar::TRACE_)\
		TRACE
#else
#define MBTRACEFN TRACE
#define MBTRACE   TRACE
#endif


BEGIN_MESSAGE_MAP(CMenuBar, ToolBarWnd)
	//{{AFX_MSG_MAP(CMenuBar)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_MESSAGE(MB_SET_MENU_NULL, OnSetMenuNull)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()


CMenuBar::CMenuBar()
{
	tracking_state_ = TRACK_NONE;		 // initial state: not tracking 
	popup_tracking_ = new_popup_ = -1; // invalid
	hmenu_ = NULL;
	auto_remove_frame_menu_ = TRUE;		 // set frame's menu to NULL
	menu_tracking_ = 0;
	parent_ = 0;
}

CMenuBar::~CMenuBar()
{}


bool CMenuBar::Create(CWnd* parent, UINT menu_id)
{
	DWORD tb_style= WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_LIST | CCS_TOP | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER;

	SetDefaultFontFlag(false);

	if (!ToolBarWnd::Create(tb_style, CRect(0,0,0,0), parent, -1))
		return false;

	// turn off visual styles or else toolbar will ignore color requests from OnCustomDraw
	::SetWindowTheme(m_hWnd, L"", L"");
/*	if (WhistlerLook::IsAvailable())
	{
		if (HMODULE lib= ::LoadLibrary(_T("UxTheme.dll")))
		{
			typedef HRESULT (STDAPICALLTYPE *fnSetWindowTheme)(HWND hwnd, LPCWSTR sub_app_name, LPCWSTR sub_id_list);

			fnSetWindowTheme pfnSetWindowTheme= reinterpret_cast<fnSetWindowTheme>(::GetProcAddress(lib, "SetWindowTheme"));

			if (pfnSetWindowTheme)
				pfnSetWindowTheme(m_hWnd, L"", L"");
		}
	}*/

	parent_ = parent;

	SetOnIdleUpdateState(false);

	LoadMenu(menu_id);

	return true;
}


//////////////////
// Menu bar was created: install hook into owner window
//
int CMenuBar::OnCreate(LPCREATESTRUCT create_struct)
{
	if (ToolBarWnd::OnCreate(create_struct)==-1)
		return -1;
	UpdateFont();
	CWnd* frame = GetOwner();
	ASSERT_VALID(frame);
	frame_hook_.Install(this, *frame);
	return 0; // OK
}

//////////////////
// Set menu bar font from current system menu font
//
void CMenuBar::UpdateFont()
{
	static CFont font;
	NONCLIENTMETRICS info;
	info.cbSize = sizeof info;
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
	if (font.m_hObject)
		font.DeleteObject();
	VERIFY(font.CreateFontIndirect(&info.lfMenuFont));
	SetFont(&font);
}

//////////////////
// Recompute layout of menu bar
//
void CMenuBar::RecomputeMenuLayout()
{
//	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE |
//		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

//////////////////
// Make frame recalculate control bar sizes after menu change
//
void CMenuBar::RecomputeToolbarSize()
{
	// Force toolbar to recompute size
/*	CFrameWnd* frame = (CFrameWnd*)GetOwner();
	ASSERT_VALID(frame);
	ASSERT(frame->IsFrameWnd());
	frame->RecalcLayout();

	// floating frame
	frame = GetParentFrame();
	if (frame->IsKindOf(RUNTIME_CLASS(CMiniFrameWnd)))
		frame->RecalcLayout();	 */
}

//////////////////
// Set tracking state: none, button, or popup
//
void CMenuBar::SetTrackingState(TRACKINGSTATE iState, int btn)
{
	ASSERT_VALID(this);

	if (iState != tracking_state_)
	{
		if (iState == TRACK_NONE)
			btn = -1;

#ifdef _DEBUG
		static LPCTSTR StateName[] = { _T("NONE"), _T("BUTTON"), _T("POPUP") };
		MBTRACE(_T("CMenuBar::SetTrackingState to %s, button=%d\n"), StateName[iState], btn);
#endif

		SetHotItem(btn);					 // could be none (-1)

		if (iState == TRACK_POPUP)
		{
			// set related state stuff
			escape_was_pressed_ = FALSE;	 // assume Esc key not pressed
			process_right_arrow_ =			 // assume left/right arrow..
				process_left_arrow_ = TRUE; // ..will move to prev/next popup
			popup_tracking_ = btn;	 // which popup I'm tracking
		}

		tracking_state_ = iState;
	}
}

//////////////////
// Toggle state from home state to button-tracking and back
//
void CMenuBar::ToggleTrackButtonMode()
{
	ASSERT_VALID(this);
	if (tracking_state_ == TRACK_NONE || tracking_state_ == TRACK_BUTTON)
	{
		SetTrackingState(tracking_state_ == TRACK_NONE ? TRACK_BUTTON : TRACK_NONE, 0);
	}
}

//////////////////
// Get button index before/after a given button
//
int CMenuBar::GetNextOrPrevButton(int btn, BOOL prev)
{
	ASSERT_VALID(this);
	if (prev) {
		btn--;
		if (btn < 0)
			btn = GetButtonCount() - 1;
	} else {
		btn++;
		if (btn >= GetButtonCount())
			btn = 0;
	}
	return btn;
}

/////////////////
// This is to correct a bug in the system toolbar control: TB_HITTEST only
// looks at the buttons, not the size of the window. So it returns a button
// hit even if that button is totally outside the size of the window!
//
int CMenuBar::HitTest(CPoint p) const
{
	int hit= ToolBarWnd::HitTest(&p);
	if (hit > 0)
	{
		CRect rc;
		GetClientRect(&rc);
		if (!rc.PtInRect(p)) // if point is outside window
			hit = -1;			// can't be a hit!
	}
	return hit;
}

//////////////////
// Load a different menu. The HMENU must not belong to any CMenu,
// and you must free it when you're done. Returns old menu.
//
HMENU CMenuBar::LoadMenu(HMENU hmenu)
{
	MBTRACEFN(_T("CMenuBar::LoadMenu\n"));
//	UINT prev_id=(UINT)-1;
	ASSERT(::IsMenu(hmenu));
	ASSERT_VALID(this);

	if (auto_remove_frame_menu_)
	{
		CFrameWnd* frame = GetParentFrame();
		if (::GetMenu(*frame)!=NULL)
		{
			// I would like to set the frame's menu to NULL now, but if I do, MFC
			// gets all upset: it calls GetMenu and expects to have a real menu.
			// So Instead, I post a message to myself. Because the message is
			// posted, not sent, I won't process it until MFC is done with all its
			// initialization stuff. (MFC needs to set CFrameWnd::m_hMenuDefault
			// to the menu, which it gets by calling GetMenu.)
			//
			frame->SetMenu(NULL); //MK: ExifPro is not upset
//			PostMessage(MB_SET_MENU_NULL, (WPARAM)frame->GetSafeHwnd());
		}
	}
	HMENU old_menu = hmenu_;
	hmenu_ = hmenu;

	// delete existing buttons
	int count = GetButtonCount();
	while (count--)
	{
		VERIFY(CToolBarCtrl::DeleteButton(0));
	}

	// add text buttons
	UINT menu_items= hmenu ? ::GetMenuItemCount(hmenu) : 0;

	AddButtons(menu_items, 0, 0);

	for (UINT i= 0; i < menu_items; i++)
	{
		TCHAR name[256];
		memset(name, 0, sizeof(name));	// guarantees double-0 at end
		if (::GetMenuString(hmenu, i, name, array_count(name)-1, MF_BYPOSITION))
		{
			SetButtonText(i, name);
		}
	}

	AutoSize();						// size buttons

/*
	SetImageList(NULL);
//	SetButtonSize(CSize(0,0)); // This barfs in VC 6.0

	DWORD dwStyle = GetStyle();
	BOOL modify_style = ModifyStyle(0, TBSTYLE_FLAT|TBSTYLE_TRANSPARENT);

	// add text buttons
	UINT menu_items = hmenu ? ::GetMenuItemCount(hmenu) : 0;

	for (UINT i=0; i < menu_items; i++) {
		TCHAR name[64];
		memset(name, 0, sizeof(name)); // guarantees double-0 at end
		if (::GetMenuString(hmenu, i, name, array_count(name)-1, MF_BYPOSITION)) {
			TBBUTTON tbb;
			memset(&tbb, 0, sizeof(tbb));
			tbb.idCommand = ::GetMenuItemID(hmenu, i);

			// Because the toolbar is too brain-damaged to know if it already has
			// a string, and is also too brain-dead to even let you delete strings,
			// I have to determine if each string has been added already. Otherwise
			// in a MDI app, as the menus are repeatedly switched between doc and
			// no-doc menus, I will keep adding strings until somebody runs out of
			// memory. Sheesh!
			// 
			int iString = -1;
			for (int j=0; j<ar_strings_.GetSize(); j++) {
				if (ar_strings_[j] == name) {
					iString = j; // found it
					break;
				}
			}
			if (iString <0) {
				// string not found: add it
				iString = AddStrings(name);
				ar_strings_.SetAtGrow(iString, name);
			}

			tbb.iString = iString;
			tbb.fsState = TBSTATE_ENABLED;
			tbb.fsStyle = TBSTYLE_AUTOSIZE;
			tbb.iBitmap = -1;
			tbb.idCommand = i;
			VERIFY(AddButtons(1, &tbb));
		}
	}

	if (modify_style)
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	
	if (hmenu) {
		AutoSize();								 // size buttons
		RecomputeToolbarSize();				 // and menubar itself
	}
*/

	return old_menu;
}

//////////////////
// Load menu from resource
//
HMENU CMenuBar::LoadMenu(LPTSTR menu_name)
{
	return LoadMenu(::LoadMenu(AfxGetResourceHandle(), menu_name));
}

//////////////////
// Set the frame's menu to NULL. WPARAM is HWND of frame.
//
LRESULT CMenuBar::OnSetMenuNull(WPARAM wp, LPARAM lp)
{
	HWND hwnd = (HWND)wp;
	ASSERT(::IsWindow(hwnd));
	::SetMenu(hwnd, NULL);
	return 0;
}

//////////////////
// Handle mouse click: if clicked on button, press it
// and go into main menu loop.
//
void CMenuBar::OnLButtonDown(UINT flags, CPoint pt)
{
	MBTRACEFN(_T("CMenuBar::OnLButtonDown\n"));
	ASSERT_VALID(this);
	int btn = HitTest(pt);
	if (btn >= 0 && btn < GetButtonCount()) // if mouse is over a button:
		TrackPopup(btn);								 //   track it
	else														 // otherwise:
		ToolBarWnd::OnLButtonDown(flags, pt);	 //   pass it on...
}

//////////////////
// Handle mouse movement
//
void CMenuBar::OnMouseMove(UINT flags, CPoint pt)
{
	ASSERT_VALID(this);

	if (tracking_state_==TRACK_BUTTON)
	{
		// In button-tracking state, ignore mouse-over to non-button area.
		// Normally, the toolbar would de-select the hot item in this case.
		// 
		// Only change the hot item if the mouse has actually moved.
		// This is necessary to avoid a bug where the user moves to a different
		// button from the one the mouse is over, and presses arrow-down to get
		// the menu, then Esc to cancel it. Without this code, the button will
		// jump to wherever the mouse is--not right.

		int hot= HitTest(pt);
		if (IsValidButton(hot) && pt != mouse_)
			SetHotItem(hot);
		return;			 // don't let toolbar get it
	}
	mouse_ = pt; // remember point
	ToolBarWnd::OnMouseMove(flags, pt);
}

//////////////////
// Window was resized: need to recompute layout
//
void CMenuBar::OnSize(UINT type, int cx, int cy)
{
	ToolBarWnd::OnSize(type, cx, cy);
	RecomputeMenuLayout();
}

//////////////////
// Bar style changed: eg, moved from left to right dock or floating
/*
void CMenuBar::OnBarStyleChange(DWORD old_style, DWORD new_style)
{
	ToolBarWnd::OnBarStyleChange(old_style, new_style);
	RecomputeMenuLayout();2
} */

/////////////////
// When user selects a new menu item, note whether it has a submenu
// and/or parent menu, so I know whether right/left arrow should
// move to the next popup.
//
void CMenuBar::OnMenuSelect(HMENU hmenu, UINT item)
{
	if (tracking_state_ > 0)
	{
		// process right-arrow if item is NOT a submenu
		process_right_arrow_ = (::GetSubMenu(hmenu, item) == NULL);

		// process left-arrow if curent menu is one I'm tracking
		process_left_arrow_ = hmenu==menu_tracking_;

//TRACE(L"proc %x\n", process_right_arrow_);
	}
}

// globals--yuk! But no other way using windows hooks.
//
static CMenuBar*	g_menu_bar = NULL;
static HHOOK		g_msg_hook = NULL;

////////////////
// Menu filter hook just passes to virtual CMenuBar function
//
LRESULT CALLBACK
CMenuBar::MenuInputFilter(int code, WPARAM wp, LPARAM lp)
{
	return (code == MSGF_MENU && g_menu_bar && g_menu_bar->OnMenuInput(*((MSG*)lp))) ?
		TRUE : CallNextHookEx(g_msg_hook, code, wp, lp);
}

//////////////////
// Handle menu input event: Look for left/right to change popup menu,
// mouse movement over over a different menu button for "hot" popup effect.
// Returns TRUE if message handled (to eat it).
//
BOOL CMenuBar::OnMenuInput(MSG& m)
{
	ASSERT_VALID(this);
	ASSERT(tracking_state_ == TRACK_POPUP); // sanity check
	int msg = m.message;
//TRACE("msg %x\n", msg);
	if (msg == WM_PAINT)
	{
//lousy workaround
SetHotItem(popup_tracking_);
	}

	if (msg == WM_KEYDOWN)
	{
//SetHotItem(-1);
//SetHotItem(popup_tracking_);
//Invalidate();
//UpdateWindow();

//TRACE(L"key %x\n", process_right_arrow_);
		// handle left/right-arow.
		TCHAR vkey = m.wParam;
		if ((vkey == VK_LEFT  && process_left_arrow_) ||
			(vkey == VK_RIGHT && process_right_arrow_))
		{
			MBTRACE(_T("CMenuBar::OnMenuInput: handle VK_LEFT/RIGHT\n"));
			CancelMenuAndTrackNewOne(GetNextOrPrevButton(popup_tracking_, vkey == VK_LEFT));
			return TRUE; // eat it

		}
		else if (vkey == VK_ESCAPE)
		{
			escape_was_pressed_ = TRUE;	 // (menu will abort itself)
		}
	}
	else if (msg == WM_MOUSEMOVE || msg == WM_LBUTTONDOWN)
	{
		// handle mouse move or click
		CPoint pt = m.lParam;
		ScreenToClient(&pt);

		if (msg == WM_MOUSEMOVE)
		{
			if (pt != mouse_)
			{
				int btn = HitTest(pt);
				if (IsValidButton(btn) && btn != popup_tracking_)
				{
					// user moved mouse over a different button: track its popup
					CancelMenuAndTrackNewOne(btn);
				}
				mouse_ = pt;
			}

		}
		else if (msg == WM_LBUTTONDOWN)
		{
			if (HitTest(pt) == popup_tracking_)
			{
				// user clicked on same button I am tracking: cancel menu
				MBTRACE(_T("CMenuBar:OnMenuInput: handle mouse click to exit popup\n"));
				CancelMenuAndTrackNewOne(-1);
				return TRUE; // eat it
			}
		}
	}
	return FALSE; // not handled
}

//////////////////
// Cancel the current popup menu by posting WM_CANCELMODE, and track a new
// menu. iNewPopup is which new popup to track (-1 to quit).
//
void CMenuBar::CancelMenuAndTrackNewOne(int new_popup)
{
	MBTRACE(_T("CMenuBar::CancelMenuAndTrackNewOne: %d\n"), new_popup);
	ASSERT_VALID(this);
	if (new_popup != popup_tracking_)
	{
		parent_->PostMessage(WM_CANCELMODE);		// quit menu loop
		new_popup_ = new_popup;					// go to this popup (-1 = quit)
	}
}

//////////////////
// Track the popup submenu associated with the i'th button in the menu bar.
// This fn actually goes into a loop, tracking different menus until the user
// selects a command or exits the menu.
//
void CMenuBar::TrackPopup(int btn)
{
	MBTRACE(_T("CMenuBar::TrackPopup %d\n"), btn);
	ASSERT_VALID(this);
	ASSERT(hmenu_);

	CMenu menu;
	menu.Attach(hmenu_);
	int menu_items = menu.GetMenuItemCount();

	while (btn >= 0)		// while user selects another menu
	{
		new_popup_ = -1;					// assume quit after this
		//PressButton(btn, TRUE);		// press the button
//		Invalidate();

		// post a simulated arrow-down into the message stream
		// so TrackPopupMenu will read it and move to the first item
		parent_->PostMessage(WM_KEYDOWN, VK_DOWN, 1);
		parent_->PostMessage(WM_KEYUP, VK_DOWN, 1);

		SetTrackingState(TRACK_POPUP, btn); // enter tracking state

//		UpdateWindow();						// and force repaint now

		// Need to install a hook to trap menu input in order to make
		// left/right-arrow keys and "hot" mouse tracking work.
		//
		ASSERT(g_menu_bar == NULL);
		g_menu_bar = this;
		ASSERT(g_msg_hook == NULL);
		g_msg_hook = SetWindowsHookEx(WH_MSGFILTER, MenuInputFilter, NULL, ::GetCurrentThreadId());

		// get submenu and display it beneath button
		TPMPARAMS tpm;
		CRect rcButton;
		GetRect(btn, rcButton);
		ClientToScreen(&rcButton);
		CPoint pt = ComputeMenuTrackPoint(rcButton, tpm);
		HMENU menu_popup = ::GetSubMenu(hmenu_, btn);
		menu_tracking_ = menu_popup;
		ASSERT(menu_popup);
		BOOL ret = TrackPopupMenuEx(menu_popup, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, pt.x, pt.y, parent_->GetSafeHwnd(), &tpm);

		// uninstall hook.
		::UnhookWindowsHookEx(g_msg_hook);
		g_msg_hook = NULL;
		g_menu_bar = NULL;
		menu_tracking_ = 0;

//		PressButton(btn, FALSE);	 // un-press button
//		UpdateWindow();					 // and force repaint now

		// If the user exited the menu loop by pressing Escape,
		// return to track-button state; otherwise normal non-tracking state.
		SetTrackingState(escape_was_pressed_ ? TRACK_BUTTON : TRACK_NONE, btn);

		// If the user moved mouse to a new top-level popup (eg from File to
		// Edit button), I will have posted a WM_CANCELMODE to quit
		// the first popup, and set new_popup_ to the new menu to show.
		// Otherwise, new_popup_ will be -1 as set above.
		// So just set btn to the next popup menu and keep looping...
		btn = new_popup_;
	}
	menu.Detach();

	popup_tracking_ = -1;
}

//////////////////
// Given button rectangle, compute point and "exclude rect" for
// TrackPopupMenu, based on current docking style, so that the menu will
// appear always inside the window.
//
CPoint CMenuBar::ComputeMenuTrackPoint(const CRect& rcButn, TPMPARAMS& tpm)
{
	tpm.cbSize = sizeof(tpm);
	DWORD style = 0; //style_;
	CPoint pt;
	CRect& rcExclude = (CRect&)tpm.rcExclude;
	rcExclude = rcButn;
	::GetWindowRect(::GetDesktopWindow(), &rcExclude);

	switch (style & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_BOTTOM:
		pt = CPoint(rcButn.left, rcButn.top);
		rcExclude.top = rcButn.top;
		break;

	case CBRS_ALIGN_LEFT:
		pt = CPoint(rcButn.right, rcButn.top);
		rcExclude.right = rcButn.right;
		break;

	case CBRS_ALIGN_RIGHT:
		pt = CPoint(rcButn.left, rcButn.top);
		rcExclude.left = rcButn.left;
		break;

	default: //	case CBRS_ALIGN_TOP:
		pt = CPoint(rcButn.left, rcButn.bottom);
		break;
	}
	return pt;
}

//////////////////
// This function translates special menu keys and mouse actions.
// You must call it from your frame's PreTranslateMessage.
//
BOOL CMenuBar::TranslateFrameMessage(MSG* msg)
{
	ASSERT_VALID(this);
	ASSERT(msg);
	UINT message = msg->message;
	if (WM_LBUTTONDOWN <= message && message <= WM_MOUSELAST)
	{
		if (msg->hwnd != m_hWnd && tracking_state_ > 0)
		{
			// user clicked outside menu bar: exit tracking mode
			MBTRACE(_T("CMenuBar::TranslateFrameMessage: user clicked outside menu bar: end tracking\n"));
			SetTrackingState(TRACK_NONE);
		}
	}
	else if (message == WM_SYSKEYDOWN || message == WM_SYSKEYUP || message == WM_KEYDOWN)
	{
		BOOL alt = HIWORD(msg->lParam) & KF_ALTDOWN; // Alt key down
		TCHAR vkey = msg->wParam;							  // get virt key
		if (vkey == VK_MENU ||
			(vkey == VK_F10 && !((GetKeyState(VK_SHIFT) & 0x80000000) ||
			                   (GetKeyState(VK_CONTROL) & 0x80000000) || alt)))
		{
			// key is VK_MENU or F10 with no alt/ctrl/shift: toggle menu mode
			if (message==WM_SYSKEYUP)
			{
				MBTRACE(_T("CMenuBar::TranslateFrameMessage: handle menu key\n"));
				ToggleTrackButtonMode();
			}
			return TRUE;

		}
		else if (message == WM_SYSKEYDOWN || message == WM_KEYDOWN)
		{
			if (tracking_state_ == TRACK_BUTTON)
			{
				// I am tracking: handle left/right/up/down/space/Esc
				switch (vkey)
				{
				case VK_LEFT:
				case VK_RIGHT:
					// left or right-arrow: change hot button if tracking buttons
					MBTRACE(_T("CMenuBar::TranslateFrameMessage: VK_LEFT/RIGHT\n"));
					SetHotItem(GetNextOrPrevButton(GetHotItem(), vkey == VK_LEFT));
					return TRUE;

				case VK_SPACE:  // (personally, I like SPACE to enter menu too)
				case VK_UP:
				case VK_DOWN:
					// up or down-arrow: move into current menu, if any
					MBTRACE(_T("CMenuBar::TranslateFrameMessage: VK_UP/DOWN/SPACE\n"));
					TrackPopup(GetHotItem());
					return TRUE;

				case VK_ESCAPE:
					// escape key: exit tracking mode
					MBTRACE(_T("CMenuBar::TranslateFrameMessage: VK_ESCAPE\n"));
					SetTrackingState(TRACK_NONE);
					return TRUE;
				}
			}

			// Handle alphanumeric key: invoke menu. Note that Alt-X
			// chars come through as WM_SYSKEYDOWN, plain X as WM_KEYDOWN.
			if ((alt || tracking_state_ == TRACK_BUTTON) && isalnum(vkey) && IsTopParentActive())
			{
				// Alt-X, or else X while in tracking mode
				UINT id;
				if (MapAccelerator(vkey, &id))
				{
					MBTRACE(_T("CMenuBar::TranslateFrameMessage: map acclerator\n"));
					TrackPopup(id);	 // found menu mnemonic: track it
					return TRUE;	 // handled
				}
				else if (tracking_state_ == TRACK_BUTTON && !alt)
				{
					MessageBeep(0);
					return TRUE;
				}
			}

			// Default for any key not handled so far: return to no-menu state
			if (tracking_state_ > 0)
			{
				MBTRACE(_T("CMenuBar::TranslateFrameMessage: unknown key, stop tracking\n"));
				SetTrackingState(TRACK_NONE);
			}
		}
	}
	return FALSE; // not handled, pass along
}

#ifdef _DEBUG
void CMenuBar::AssertValid() const
{
	ToolBarWnd::AssertValid();
	ASSERT(hmenu_==NULL || ::IsMenu(hmenu_));
	ASSERT(TRACK_NONE<=tracking_state_ && tracking_state_<=TRACK_POPUP);
	frame_hook_.AssertValid();
}

void CMenuBar::Dump(CDumpContext& dc) const
{
	ToolBarWnd::Dump(dc);
}
#endif


void CMenuBar::OnCustomDraw(NMHDR* nmhdr, LRESULT* result)
{
	NMTBCUSTOMDRAW* custom_draw= reinterpret_cast<NMTBCUSTOMDRAW*>(nmhdr);

//TRACE("drw %x\n", custom_draw->nmcd.dwDrawStage);

	switch (custom_draw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*result = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		custom_draw->clrText = GetAppColors()[AppColors::Text];
		custom_draw->clrBtnHighlight = custom_draw->clrHighlightHotTrack = GetAppColors()[AppColors::Selection]; //::GetSysColor(COLOR_HIGHLIGHT);
		custom_draw->clrTextHighlight = GetAppColors()[AppColors::SelectedText];// ::GetSysColor(COLOR_HIGHLIGHTTEXT);

//TRACE("hot: %d\n", GetToolBarCtrl().GetHotItem());
		{
			bool is_hot= !!(custom_draw->nmcd.uItemState & CDIS_HOT);
			int hot= GetHotItem();
			if (hot == static_cast<int>(custom_draw->nmcd.dwItemSpec) || is_hot ||
				(hot == -1 && popup_tracking_ == static_cast<int>(custom_draw->nmcd.dwItemSpec)))
			{
				custom_draw->clrText = custom_draw->clrTextHighlight;
				custom_draw->clrBtnFace = custom_draw->clrBtnHighlight;
			}
		}
		*result = TBCDRF_NOOFFSET | TBCDRF_NOEDGES | TBCDRF_NOETCHEDEFFECT | TBCDRF_HILITEHOTTRACK | TBCDRF_NOMARK;
		break;

	default:
		*result = CDRF_DODEFAULT;
		break;
	}
}


//////////////////////////////////////////////////////////////////
// CMenuBarFrameHook is used to trap menu-related messages sent to the owning
// frame. The same class is also used to trap messages sent to the MDI client
// window in an MDI app. I should really use two classes for this,
// but it uses less code to chare the same class. Note however: there
// are two different INSTANCES of CMenuBarFrameHook in CMenuBar: one for
// the frame and one for the MDI client window.
//
CMenuBarFrameHook::CMenuBarFrameHook()
{
}

CMenuBarFrameHook::~CMenuBarFrameHook()
{
	HookWindow((HWND)NULL); // (unhook)
}

//////////////////
// Install hook to trap window messages sent to frame or MDI client.
// 
BOOL CMenuBarFrameHook::Install(CMenuBar* menu_bar, HWND wnd_to_hook)
{
	ASSERT_VALID(menu_bar);
	menu_bar_ = menu_bar;
	return HookWindow(wnd_to_hook);
}

//////////////////////////////////////////////////////////////////
// Trap frame/MDI client messages specific to menubar. 
//
LRESULT CMenuBarFrameHook::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	CMenuBar& mb= *menu_bar_;

	switch (msg)
	{
	// The following messages are trapped for the frame window
	case WM_SYSCOLORCHANGE:
		mb.UpdateFont();
		break;

	case WM_MENUSELECT:
		mb.OnMenuSelect((HMENU)lp, (UINT)LOWORD(wp));
		break;
	}

	return CSubclassWnd::WindowProc(msg, wp, lp);
}
