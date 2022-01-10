/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ToolBarWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ToolBarWnd.h"
#include "PNGImage.h"
#include "WhistlerLook.h"
#include "GetDefaultGuiFont.h"
#include "AppColors.h"
#include "Color.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ToolBarWnd

ToolBarWnd::ToolBarWnd()
{
	shift_image_for_checked_btn_ = 0;
	on_idle_update_state_ = true;
	pad_size_ = CSize(5, 7);
	bmp_btn_size_ = CSize(0, 0);
	owner_draw_ = false;
	pass_scroll_info_ = 0;
	set_default_font_ = true;
	handle_tooltips_ = true;
	image_count_ = 0;
}


ToolBarWnd::~ToolBarWnd()
{}


BEGIN_MESSAGE_MAP(ToolBarWnd, CToolBarCtrl)
	//{{AFX_MSG_MAP(ToolBarWnd)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_NOTIFY_REFLECT_EX(TBN_GETINFOTIP, OnGetInfoTip)
	ON_NOTIFY_REFLECT(TBN_QUERYINSERT, OnQueryInsert)
	ON_NOTIFY_REFLECT(TBN_QUERYDELETE, OnQueryDelete)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
//	ON_NOTIFY_REFLECT(TBN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnGetButtonInfo)
	ON_NOTIFY_REFLECT(TBN_BEGINADJUST, OnBeginAdjust)
	ON_NOTIFY_REFLECT(TBN_RESET, OnReset)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnClick)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnClick)
	ON_NOTIFY_REFLECT(NM_RDBLCLK, OnClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ToolBarWnd message handlers


LRESULT ToolBarWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	if (!on_idle_update_state_)
		return 0L;

	// the toolbar must be visible
	if ((GetStyle() & WS_VISIBLE))
	{
		CFrameWnd* target = static_cast<CFrameWnd*>(GetOwner());
		if (target == NULL || !target->IsFrameWnd())
			target = GetParentFrame();
		if (target != NULL)
			OnUpdateCmdUI(target, !!wParam);
	}
	return 0L;
}


/////////////////////////////////////////////////////////////////////////////
// CToolBar idle update through xCToolCmdUI class

class xCToolCmdUI : public CCmdUI        // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL on);
	virtual void SetCheck(int check);
	virtual void SetText(LPCTSTR lpszText);
};

void xCToolCmdUI::Enable(BOOL on)
{
	m_bEnableChanged = TRUE;
	CToolBarCtrl* tool_bar = static_cast<CToolBarCtrl*>(m_pOther);
	ASSERT(tool_bar != NULL);
	ASSERT_KINDOF(CToolBarCtrl, tool_bar);
	ASSERT(m_nIndex < m_nIndexMax);
	if (!on)
		tool_bar->PressButton(m_nID, false);	// unpress btn when disabled
	tool_bar->EnableButton(m_nID, on);
}

void xCToolCmdUI::SetCheck(int check)
{
	ASSERT(check >= 0 && check <= 2); // 0=>off, 1=>on, 2=>indeterminate
	ToolBarWnd* tool_bar = dynamic_cast<ToolBarWnd*>(m_pOther);
	ASSERT(tool_bar != NULL);
//	dynamiASSERT_KINDOF(ToolBarWnd, tool_bar);
	ASSERT(m_nIndex < m_nIndexMax);

	if (tool_bar->shift_image_for_checked_btn_ != 0 && tool_bar->shift_image_for_checked_btn_ == m_nID)
	{
		TBBUTTONINFO tbbi;
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_IMAGE;
		tbbi.iImage = check;
		tool_bar->SetButtonInfo(m_nID, &tbbi);
	}
	else
	{
		TBBUTTONINFO tbbi;
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_STYLE;
		// do not set checked state for normal button (non-check buttons)
		tool_bar->GetButtonInfo(m_nID, &tbbi);
		if ((tbbi.fsStyle & BTNS_CHECK) == 0)
			return;
		tool_bar->CheckButton(m_nID, check);
	}
}

void xCToolCmdUI::SetText(LPCTSTR)
{
	// ignore it
}


void ToolBarWnd::OnUpdateCmdUI(CFrameWnd* target, bool disable_if_no_hndler)
{
	xCToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = GetButtonCount(); // (UINT)DefWindowProc(TB_BUTTONCOUNT, 0, 0);
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
	{
		// get buttons state
		TBBUTTON button;
		GetButton(state.m_nIndex, &button);
//		_GetButton(state.m_nIndex, &button);
		state.m_nID = button.idCommand;

		// ignore separators
		if (!(button.fsStyle & TBSTYLE_SEP))
		{
			// allow reflections
//			if (CWnd::OnCmdMsg(0,
//				MAKELONG((int)CN_UPDATE_COMMAND_UI, WM_COMMAND+WM_REFLECT_BASE),
//				&state, NULL))
//				continue;

			// allow the toolbar itself to have update handlers
//			if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
//				continue;

			// allow the owner to process the update
			state.DoUpdate(target, disable_if_no_hndler);
		}
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(target, disable_if_no_hndler);
}


///////////////////////////////////////////////////////////////////////////////


BOOL ToolBarWnd::OnGetInfoTip(NMHDR* nmhdr, LRESULT* result)
{
	if (!handle_tooltips_)
		return false;

	NMTBGETINFOTIP* info_tip= reinterpret_cast<NMTBGETINFOTIP*>(nmhdr);
	*result = 0;

	CString tip= GetToolTip(info_tip->iItem);

	_tcsncpy(info_tip->pszText, tip, INFOTIPSIZE);

	return true;
}


CString ToolBarWnd::GetToolTip(int cmd_id)
{
	CString tip;
	tip.LoadString(cmd_id);

	int sep= tip.Find(_T('\n'));
	if (sep >= 0)
		return tip.Right(tip.GetLength() - sep - 1);

	if (tip == L" ")
		return CString();

	return tip;
}


void ToolBarWnd::OnQueryInsert(NMHDR* notify_struct, LRESULT* result)
{
	*result = 1;
}


void ToolBarWnd::OnQueryDelete(NMHDR* notify_struct, LRESULT* result)
{
	*result = 1;
}


///////////////////////////////////////////////////////////////////////////////

struct CToolBarData
{
	WORD version;
	WORD width;
	WORD height;
	WORD item_count;
	WORD items[1];
};


bool ToolBarWnd::Create(const char* a_template, int rsrc_id, int str_id, CWnd* parent, int id/*= -1*/)
{
	ASSERT(m_hWnd == 0);
	ASSERT(a_template != 0);

	const TCHAR* resource_name= MAKEINTRESOURCE(rsrc_id);
	HINSTANCE inst= AfxFindResourceHandle(resource_name, RT_TOOLBAR);
	HRSRC rsrc= ::FindResource(inst, resource_name, RT_TOOLBAR);
	if (rsrc == NULL)
		return false;

	HGLOBAL global= ::LoadResource(inst, rsrc);
	if (global == NULL)
		return false;

	CToolBarData* data= reinterpret_cast<CToolBarData*>(::LockResource(global));
	if (data == NULL)
		return false;
	ASSERT(data->version == 1);

	if (data->item_count != strlen(a_template))
	{
		ASSERT(false);
		return false;
	}

	std::vector<int> commands;
	commands.reserve(data->item_count);

	for (int i= 0; i < data->item_count; ++i)
		commands.push_back(data->items[i]);

//	CSize bmp_size(data->width, data->height);

	::UnlockResource(global);
	::FreeResource(global);

	return Create(a_template, &commands.front(), rsrc_id, str_id, parent, id);
}


bool ToolBarWnd::Create(const char* a_template, const int commands[], int bmp_id, int str_id, CWnd* parent, int id/*= -1*/, bool vertical/*= false*/)
{
	DWORD tb_style= WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT |
		CCS_TOP/*CCS_NOMOVEY*/ | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER; // | TBSTYLE_ALTDRAG | CCS_ADJUSTABLE;

	if (vertical)
		tb_style |= CCS_VERT;

	if (str_id)
		tb_style |= TBSTYLE_LIST;

	if (!CToolBarCtrl::Create(tb_style, CRect(0,0,0,0), parent, id))
		return false;

	::SetWindowTheme(m_hWnd, L"", L"");

	return AddButtons(a_template, commands, bmp_id, str_id, vertical);
}


bool ToolBarWnd::AddButtons(const char* a_template, const int commands[], int bmp_id, int str_id/*= 0*/, bool vertical/*= false*/)
{
	if (set_default_font_)
		SetFont(&GetDefaultGuiFont());

	// calculate no of buttons
	int btn_count= 0;
	for (int i= 0; a_template[i]; ++i)
		if (a_template[i] != '|')
			++btn_count;

	SetButtonStructSize(sizeof(TBBUTTON));

	CBitmap mask_bmp;
	CBitmap image_bmp;
	// mask is only necessary on non-XP windows without transparency support
	bool create_mask_image= !WhistlerLook::IsAvailable();

	{ // bitmap
		if (bmp_id != 0)
		{
			PNGImage ping;

			if (create_mask_image)
			{
				ping.AlphaToColor(::GetSysColor(COLOR_3DFACE));
				PNGImage().LoadMask(bmp_id, mask_bmp);
			}

			// try to find PNG first
			ping.Load(bmp_id, image_bmp);
		}
	}

	// determine single button bitmap size
	CSize bmp_size(0, 0);
	if (bmp_id != 0)
	{
		BITMAP bmp;
		memset(&bmp, 0, sizeof bmp);

		if (image_bmp.m_hObject != 0)
			image_bmp.GetBitmap(&bmp);
		else
		{
			CBitmap Bmp;
			if (!Bmp.LoadBitmap(bmp_id))
			{
				ASSERT(false);
				return false;
			}
			Bmp.GetBitmap(&bmp);
		}
		bmp_size = CSize(bmp.bmWidth / btn_count, bmp.bmHeight);
		// this is a case of 'empty' bitmaps (one pixel wide)
		if (bmp_size.cx == 0 && bmp.bmWidth == 1)
			bmp_size.cx = 1;
		bmp_btn_size_ = bmp_size;
		image_count_ = btn_count;
	}
	else
	{
		CDC dc;
		dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
		dc.SelectObject(&GetDefaultGuiFont());
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		bmp_btn_size_ = bmp_size = CSize(1, tm.tmHeight);
		image_count_ = 0;
	}

	SetBitmapSize(bmp_size);
	SetButtonSize(bmp_size + CSize(8, 7));

	CSize padding_size= pad_size_; //CSize(5, 7); //GetApp()->WhistlerLook::IsAvailable() ? CSize(5, 9) : CSize(3, 7);
	if ((GetStyle() & TBSTYLE_LIST) == 0)
		padding_size += CSize(3, 3);
	SendMessage(TB_SETPADDING, 0, MAKELONG(padding_size.cx, padding_size.cy));

	if (bmp_id != 0)
	{
		if (image_bmp.m_hObject != 0 && (mask_bmp.m_hObject != 0 || !create_mask_image))
		{
			UINT flags= ILC_COLOR32;
			if (create_mask_image)
				flags |= ILC_MASK;

			HIMAGELIST img_list= ImageList_Create(bmp_size.cx, bmp_size.cy, flags, btn_count, 0);
			ASSERT(img_list != 0);
			if (img_list)
			{
				image_list_.DeleteImageList();
				image_list_.Attach(img_list);
				image_list_.Add(&image_bmp, create_mask_image ? &mask_bmp : static_cast<CBitmap*>(0));
				SetImageList(&image_list_);
			}
		}
		else
		{
			if (HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(bmp_id), RT_BITMAP))
			{
				HIMAGELIST img_list= ImageList_LoadImage(inst, MAKEINTRESOURCE(bmp_id),
					bmp_size.cx, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
				ASSERT(img_list != 0);
				if (img_list)
				{
					image_list_.Attach(img_list);
					SetImageList(&image_list_);
				}
			}
		}
	}

	int bmp= 0; //AddBitmap(btn_count, bmp_id);
	int string= 0;

	if (str_id > 0)	 // add string to the toolbar?
	{
		CString tb;
		tb.LoadString(str_id);
		tb += "\n";
		tb.Replace('\n', '\0');
		string = AddStrings(tb);
	}

//SendMessage(TB_SETDRAWTEXTFLAGS, -1, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS);
//SendMessage(TB_SETANCHORHIGHLIGHT, 0, 0);
//SendMessage(TB_SETMAXTEXTROWS, 1, 0);

	buttons_.clear();
	buttons_.reserve(strlen(a_template));

	bool drop_down_btn= false;

	// auto size buttons when toolbar has a TBSTYLE_LIST style or else they look ugly under 4.71 comctl
	bool auto_size= (GetStyle() & TBSTYLE_LIST) != 0;

	// add all the buttons

	for (int index= 0, cmd_index= 0; *a_template; ++index, ++a_template)
	{
		TBBUTTON btn;
		memset(&btn, 0, sizeof btn);
		btn.iBitmap = bmp;
		if (bmp_size.cx > 1)
			++bmp;
		btn.idCommand = commands[cmd_index++];
		btn.fsState = TBSTATE_ENABLED;
		btn.fsStyle = BTNS_BUTTON;
		if (!vertical && auto_size)
			btn.fsStyle |= BTNS_AUTOSIZE;
		if (vertical)
			btn.fsState |= TBSTATE_WRAP;
		btn.dwData  = 0;
		btn.iString = -1;
		bool add= true;

		switch (tolower(*a_template))
		{
		case 'p':		// push button (normal button)
			break;

		case 'v':		// button with down arrow
			btn.fsStyle |= BTNS_WHOLEDROPDOWN;
			drop_down_btn = true;
			break;

		case 'm':		// button with separate section for down arrow (for popup menu)
			btn.fsStyle |= BTNS_DROPDOWN;
			drop_down_btn = true;
			break;

		case 'x':		// check button
			btn.fsStyle |= BTNS_CHECK;
			break;

		case 'g':		// group check button
			btn.fsStyle |= BTNS_CHECKGROUP;
			break;

		case 'i':		// push button with image callback
			btn.iBitmap = I_IMAGECALLBACK;
			--bmp;
			break;

		case '|':		// separator
			if (btn.idCommand != 0)		// if there is no 0 in command id vector for a separator decrease index
				--cmd_index;
			btn.iBitmap = 8;
			--bmp;
			btn.idCommand = -1;
			btn.fsState = TBSTATE_ENABLED;
			//if (vertical)
			//	btn.fsState |= TBSTATE_WRAP;
			btn.fsStyle = TBSTYLE_SEP;
			btn.dwData  = 0;
			btn.iString = 0;
			break;

		case '.':		// skip button
			add = false;
			break;

		case ':':		// skip button and text label
			add = false;
			string++;
			break;

		case 't':		// push button with text callback (doesn't work)
			break;

		default:
			ASSERT(false);
			break;
		}

		if (isupper(*a_template))
		{
			btn.iString = string++;
			btn.fsStyle |= BTNS_SHOWTEXT;
		}

		if (add)
			buttons_.push_back(btn);
	}

	if (buttons_.size() > 0)
		CToolBarCtrl::AddButtons(static_cast<int>(buttons_.size()), &buttons_.front());

	SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);

	// resize
	AutoResize();
/*
	// get tb size
	CSize tb_size;
	GetMaxSize(&tb_size);

	if (drop_down_btn && !GetApp()->WhistlerLook::IsAvailable())
		tb_size.cx += 13;	// bug in CommonCtrls--wrong size of tb reported

	// resize tb window to fit all the buttons
	SetWindowPos(0, 0, 0, tb_size.cx, tb_size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
*/
	return true;
}


void ToolBarWnd::AutoResize()
{
	// resize
	AutoSize();

	// get tb size
	//CSize tb_size;
	GetMaxSize(&tb_size);

	if (WhistlerLook::IsAvailable())	// work aroud bug in CmnCtrl prior to v.6
	{
		CRect tb_rect(0,0,0,0);

		// go through all buttons in case they are reordered
		int count= GetButtonCount();
		for (int i= 0; i < count; ++i)
		{
			CRect rect(0,0,0,0);
			GetItemRect(i, rect);
			tb_rect |= rect;
		}

		tb_size = tb_rect.Size();

/*
		bool drop_down_btn= false;

		int size= buttons_.size();
		for (int i= 0; i < size; ++i)
			if (buttons_[i].fsStyle & BTNS_WHOLEDROPDOWN)
			{
				drop_down_btn = true;
				break;
			}

		if (drop_down_btn)
			tb_size.cx += 13;	// bug in CommonCtrls--wrong size of tb reported
*/
	}

	// resize tb window to fit all the buttons
	SetWindowPos(0, 0, 0, tb_size.cx, tb_size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void ToolBarWnd::SetOnIdleUpdateState(bool enabled)
{
	on_idle_update_state_ = enabled;
}


void ToolBarWnd::SetPadding(CSize pad)
{
	pad_size_ = pad;
}

void ToolBarWnd::SetPadding(int cx, int cy)	// set button pad (before creation)
{
	pad_size_ = CSize(cx, cy);
}


void ToolBarWnd::OnCustomDraw(NMHDR* nm_hdr, LRESULT* result)
{
	NMTBCUSTOMDRAW* NM_custom_draw= reinterpret_cast<NMTBCUSTOMDRAW*>(nm_hdr);
	*result = CDRF_DODEFAULT;

	const ApplicationColors& colors = GetAppColors();
	static HBRUSH backgnd = 0;

	if (!owner_draw_)
		return;
	NM_custom_draw->clrText = colors[AppColors::Text];;
//	NM_custom_draw->clrTextHighlight = RGB(240, 255, 0);
//	NM_custom_draw->hbrMonoDither = (HBRUSH)::GetStockObject(WHITE_BRUSH);
	NM_custom_draw->hbrMonoDither = backgnd;
	//NM_custom_draw->hbrLines = (HBRUSH)::GetStockObject(GRAY_BRUSH);
	//NM_custom_draw->hpenLines = (HPEN)::GetStockObject(WHITE_PEN);
	//NM_custom_draw->clrBtnFace = RGB(0, 0, 255);
	NM_custom_draw->clrBtnHighlight = colors[AppColors::Activebg];//RGB(80, 80, 80);colors[AppColors::Selection];	// checked button
	//NM_custom_draw->clrMark = RGB(0, 255, 0);
	NM_custom_draw->clrHighlightHotTrack =colors[AppColors::Activebg];//CalcNewColor(colors[AppColors::Background], colors[AppColors::Selection], 0.3f);

	switch (NM_custom_draw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*result = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;// | TBCDRF_NOEDGES | TBCDRF_HILITEHOTTRACK | TBCDRF_NOETCHEDEFFECT;
		backgnd = ::CreateSolidBrush(colors[AppColors::Selection]);
		NM_custom_draw->hbrMonoDither = backgnd;
		break;
	case CDDS_ITEMPREPAINT:
//		*result = CDRF_SKIPDEFAULT; //TBCDRF_NOETCHEDEFFECT | TBCDRF_NOOFFSET | TBCDRF_NOEDGES | TBCDRF_NOMARK; //| TBCDRF_NOBACKGROUND;
//		HIMAGELIST image_list= NM_custom_draw->nmcd.uItemState & CDIS_HOT ? image_list_hot_ : image_list_;
//		CPoint pos(NM_custom_draw->nmcd.rc.left, NM_custom_draw->nmcd.rc.top);
////		if (NM_custom_draw->nmcd.uItemState & CDIS_SELECTED)
////			++pos.x;
//		ImageList_Draw(image_list, CommandToIndex(NM_custom_draw->nmcd.dwItemSpec), NM_custom_draw->nmcd.hdc,
//			pos.x + 1, pos.y + 1, ILD_TRANSPARENT);
////TRACE(L"spec  %x\n", NM_custom_draw->nmcd.dwItemSpec);
////TRACE(L"state %x\n", NM_custom_draw->nmcd.uItemState);
		*result = TBCDRF_NOEDGES | TBCDRF_HILITEHOTTRACK | TBCDRF_NOETCHEDEFFECT;
		break;

	case CDDS_POSTPAINT:
		::DeleteObject(backgnd);
		backgnd = nullptr;
		break;
	}
} 


void ToolBarWnd::SetHotImageList(int hot_bmp_id)
{
	ASSERT(image_list_hot_.m_hImageList == 0);

	if (!PNGImage().LoadImageList(hot_bmp_id, -image_count_ /*bmp_btn_size_.cx*/, ::GetSysColor(COLOR_3DFACE), image_list_hot_))
	{
		HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(hot_bmp_id), RT_BITMAP);
		image_list_hot_.Attach(ImageList_LoadImage(inst, MAKEINTRESOURCE(hot_bmp_id),
			bmp_btn_size_.cx, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION));
	}

	ASSERT(image_list_hot_.m_hImageList != 0);

	CToolBarCtrl::SetHotImageList(&image_list_hot_);
}


void ToolBarWnd::SetDisabledImageList(int disabled_bmp_id)
{
	CreateDisabledImageList(disabled_bmp_id, 0.0f, 0.0f, 1.0f);
}


bool ToolBarWnd::CreateDisabledImageList(int bmp_id, float saturation, float lightness, float alpha)
{
	image_list_disabled_.DeleteImageList();

	if (!PNGImage().LoadImageList(bmp_id, -image_count_ /* bmp_btn_size_.cx*/, ::GetSysColor(COLOR_3DFACE), image_list_disabled_, saturation, lightness, alpha, true))
	{
		// note: lightness & saturation adjustment works only with PNG images
		if (HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(bmp_id), RT_BITMAP))
			image_list_disabled_.Attach(ImageList_LoadImage(inst, MAKEINTRESOURCE(bmp_id),
				bmp_btn_size_.cx, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION));
	}

	if (image_list_disabled_.m_hImageList == 0)
		return false;

	CToolBarCtrl::SetDisabledImageList(&image_list_disabled_);
	return true;
}

/*
void ToolBarWnd::OnGetDispInfo(NMHDR* notify_struct, LRESULT* result)
{
	NMTBDISPINFO* nm_tb= reinterpret_cast<NMTBDISPINFO*>(notify_struct);
	*result = 0;
}
*/


void ToolBarWnd::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (pass_scroll_info_)
	{
		const MSG* msg= GetCurrentMessage();
		pass_scroll_info_->SendMessage(msg->message, msg->wParam, msg->lParam);
	}
//	CToolBarCtrl::OnHScroll(sb_code, pos, scroll_bar);
}


void ToolBarWnd::OnBeginAdjust(NMHDR* notify_struct, LRESULT* result)
{
	*result = 0;
}


void ToolBarWnd::OnReset(NMHDR* notify_struct, LRESULT* result)
{
	*result = 0;
	ResetToolBar(true);
}


void ToolBarWnd::OnGetButtonInfo(NMHDR* notify_struct, LRESULT* result)
{
	TBNOTIFY* tbn= reinterpret_cast<TBNOTIFY*>(notify_struct);
	*result = false;

	int count= buttons_.size();
	if (tbn->iItem >= count || tbn->iItem < 0)
		return;

	int index= tbn->iItem;
	// skip separators
	for (int i= 0; i <= index; ++i)
	{
		if (buttons_[i].fsStyle & TBSTYLE_SEP)
		{
			index++;
			if (index >= count)
				return;
		}
	}

	tbn->tbButton = buttons_[index];

	CString button;
	if (button.LoadString(tbn->tbButton.idCommand))
	{
		CString ttl;
		AfxExtractSubString(ttl, button, 0);
		int index= ttl.Find(_T('('));
		if (index > 0)
			ttl = ttl.Left(index);
		_tcsncpy(tbn->pszText, ttl, tbn->cchText - 1);
		tbn->pszText[tbn->cchText - 1] = _T('\0');
	}

	*result = true;
}


bool ToolBarWnd::DeleteButton(int command_id)
{
	return !!CToolBarCtrl::DeleteButton(CommandToIndex(command_id));
}


void ToolBarWnd::SaveState(const TCHAR* sub_key, const TCHAR* value_name)
{
	HKEY app_key= AfxGetApp()->GetAppRegistryKey();
	CToolBarCtrl::SaveState(app_key, sub_key, value_name);
	RegCloseKey(app_key);
}


void ToolBarWnd::RestoreState(const TCHAR* sub_key, const TCHAR* value_name)
{
	HKEY app_key= AfxGetApp()->GetAppRegistryKey();
	CToolBarCtrl::RestoreState(app_key, sub_key, value_name);
	RegCloseKey(app_key);
}


bool ToolBarWnd::AddButtons(int btn_count, int first_command, const TCHAR* text, BYTE btn_style/*= BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT*/)
{
	if (set_default_font_)
		SetFont(&GetDefaultGuiFont());

	SetButtonStructSize(sizeof(TBBUTTON));

	// determine single button bitmap size
	CSize bmp_size(0, 0);

	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	dc.SelectObject(&GetDefaultGuiFont());
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	bmp_btn_size_ = bmp_size = CSize(1, tm.tmHeight);

	SetBitmapSize(bmp_size);
	SetButtonSize(bmp_size + CSize(8, 7));

	CSize padding_size= pad_size_;
	if ((GetStyle() & TBSTYLE_LIST) == 0)
		padding_size += CSize(6, 5);
	SendMessage(TB_SETPADDING, 0, MAKELONG(padding_size.cx, padding_size.cy));

	int bmp= AddBitmap(1, IDB_EMPTY);

	buttons_.clear();
	buttons_.reserve(btn_count);

	int string= AddStrings(text ? text : _T("\0\0"));
//	bool drop_down_btn= false;

	// add all the buttons

	for (int index= 0, cmd_index= first_command; index < btn_count; ++index)
	{
		TBBUTTON btn;
		memset(&btn, 0, sizeof btn);
		btn.iBitmap = 0; //I_IMAGENONE;
		btn.idCommand = cmd_index++;
		btn.fsState = TBSTATE_ENABLED;
		btn.fsStyle = btn_style; //BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT;
		btn.dwData  = 0;
		btn.iString = string;
		buttons_.push_back(btn);

		if (text)
			++string;
	}

	if (buttons_.size() > 0)
		CToolBarCtrl::AddButtons(buttons_.size(), &buttons_.front());

//	SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);

//	SetDrawTextFlags(DT_CENTER | DT_RIGHT, DT_CENTER);

	// resize
	AutoResize();
/*
	// get tb size
	CSize tb_size;
	GetMaxSize(&tb_size);

//	if (drop_down_btn && !WhistlerLook::IsAvailable())
//		tb_size.cx += 13;	// bug in CommonCtrls--wrong size of tb reported

	// resize tb window to fit all the buttons
	SetWindowPos(0, 0, 0, tb_size.cx, tb_size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
*/
	return true;
}


void ToolBarWnd::SetButtonText(int id, const TCHAR* text)
{
	TBBUTTONINFO tbbi;
	tbbi.cbSize = sizeof tbbi;
	tbbi.dwMask = TBIF_TEXT;
	tbbi.pszText = const_cast<TCHAR*>(text);

	SetButtonInfo(id, &tbbi);

	//if (!WhistlerLook::IsAvailable())
	{
		//TODO:
	/*
		CDC dc;
		dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
		dc.SelectStockObject(DEFAULT_GUI_FONT); //TODO: proper font
		int extra_width= dc.GetTextExtent(_T("XXX"), 3).cx;

		CString btn;
		btn.LoadString(IDS_MARGINS);

		// btn sizing is broken
		for (int i= 0; i < buttons_.size(); ++i)
		{
			CString str;
			::AfxExtractSubString(str, btn, i);

			int width= dc.GetTextExtent(str).cx;

			TBBUTTONINFO tbi;
			memset(&tbi, 0, sizeof tbi);
			tbi.cbSize = sizeof tbi;
			tbi.mask = TBIF_SIZE;
			tbi.cx = width + extra_width;
			margin_select_wnd_.SetButtonInfo(cmds[i], &tbi);
		}

		WINDOWPLACEMENT wp;
		if (margin_select_wnd_.GetWindowPlacement(&wp))
		{
			CRect rect= wp.rcNormalPosition;
			rect.OffsetRect(0, 2);
			margin_select_wnd_.MoveWindow(rect, false);
		}
	*/
	}
}


void ToolBarWnd::SetDefaultFontFlag(bool enabled)
{
	set_default_font_ = enabled;
}


void ToolBarWnd::ResetToolBar(bool resize_to_fit)
{
	int count= GetButtonCount();
	for (int i= count - 1; i >= 0; --i)
		CToolBarCtrl::DeleteButton(i);

	if (buttons_.size() > 0)
		CToolBarCtrl::AddButtons(buttons_.size(), &buttons_.front());

	if (resize_to_fit)
		AutoResize();
}


void ToolBarWnd::HideButtonIdx(int index, bool hide)
{
	if (index < buttons_.size() && index >= 0)
		HideButton(buttons_[index].idCommand, hide);
}


void ToolBarWnd::Destroy()
{
	DestroyWindow();
	image_list_.DeleteImageList();
	image_list_hot_.DeleteImageList();
	image_list_disabled_.DeleteImageList();
	buttons_.clear(); // swap perhaps?
}


void ToolBarWnd::OnClick(NMHDR* notify_struct, LRESULT* result)
{
	*result = 0;

	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER + 100, WPARAM(notify_struct->hwndFrom), notify_struct->code);
}


bool ToolBarWnd::ReplaceImageList(int new_bmp_id, int btn_count/*= 0*/, ImgListFlag list/*= NORMAL*/)
{
	if (btn_count == 0)
	{
		size_t count= buttons_.size();
		for (size_t i= 0; i < count; ++i)
			if ((buttons_[i].fsStyle & TBSTYLE_SEP) == 0)
				++btn_count;
	}

	CImageList il;
	CSize bmp_size= LoadImageList(new_bmp_id, btn_count, il);

	if (il.m_hImageList == 0)
		return false;

	bmp_btn_size_ = bmp_size;

	switch (list)
	{
	case NORMAL:
		image_list_.DeleteImageList();
		image_list_.Attach(il.Detach());
		SetImageList(&image_list_);
		break;
	case HOT:
		image_list_hot_.DeleteImageList();
		image_list_hot_.Attach(il.Detach());
		CToolBarCtrl::SetHotImageList(&image_list_hot_);
		break;
	case DISABLED:
		image_list_disabled_.DeleteImageList();
		image_list_disabled_.Attach(il.Detach());
		CToolBarCtrl::SetDisabledImageList(&image_list_disabled_);
		break;
	}

	if (list != NORMAL)
		return true;

	SetBitmapSize(bmp_size);
	SetButtonSize(bmp_size + CSize(8, 7));

	CSize padding_size= pad_size_;
	if ((GetStyle() & TBSTYLE_LIST) == 0)
		padding_size += CSize(3, 3);
//	SendMessage(TB_SETPADDING, 0, MAKELONG(padding_size.cx, padding_size.cy));
//SendMessage(TB_SETDRAWTEXTFLAGS, -1, DT_LEFT | DT_TOP | DT_SINGLELINE);

	size_t count= GetButtonCount();
	for (size_t i= 0; i < count; ++i)
	{
		//TBBUTTONINFO tbbi;
		//memset(&tbbi, 0, sizeof tbbi);
		//tbbi.cbSize = sizeof tbbi;
		//tbbi.mask = TBIF_TEXT;
		//TCHAR buf[500];
		//buf[0] = 0;
		//tbbi.cchText = 500;
		//tbbi.pszText = buf;
		//GetButtonInfo(buttons_[i].idCommand, &tbbi);
		//if (*tbbi.pszText)
		//{
		//	tbbi.cchText = _tcslen(buf);
		//	SetButtonInfo(buttons_[i].idCommand, &tbbi);
		//}

		// only this brutal method works with older toolbar (5.80)
		TBBUTTON tb;
		GetButton(i, &tb);
		CToolBarCtrl::DeleteButton(i);
//	SendMessage(TB_SETPADDING, 0, MAKELONG(padding_size.cx, padding_size.cy));
		InsertButton(i, &tb);
	}

	Invalidate();

	AutoResize();
//SendMessage(TB_SETDRAWTEXTFLAGS, -1, DT_LEFT | DT_TOP | DT_SINGLELINE);

	return true;
}


CSize ToolBarWnd::LoadImageList(int bmp_id, int btn_count, CImageList& img_list)
{
	CBitmap mask_bmp;
	CBitmap image_bmp;
	// mask is only necessary on non-XP windows without transparency support
	bool create_mask_image= !WhistlerLook::IsAvailable();

	{ // bitmap
		if (bmp_id != 0)
		{
			PNGImage ping;

			if (create_mask_image)
			{
				ping.AlphaToColor(::GetSysColor(COLOR_3DFACE));
				PNGImage().LoadMask(bmp_id, mask_bmp);
			}

			// try to find PNG first
			ping.Load(bmp_id, image_bmp);
		}
	}

	// determine single button bitmap size
	CSize bmp_size(0, 0);
	if (bmp_id != 0)
	{
		BITMAP bmp;
		memset(&bmp, 0, sizeof bmp);

		if (image_bmp.m_hObject != 0)
			image_bmp.GetBitmap(&bmp);
		else
		{
			CBitmap Bmp;
			if (!Bmp.LoadBitmap(bmp_id))
			{
				ASSERT(false);
				return false;
			}
			Bmp.GetBitmap(&bmp);
		}
		bmp_size = CSize(bmp.bmWidth / btn_count, bmp.bmHeight);
		// this is a case of 'empty' bitmaps (one pixel wide)
		if (bmp_size.cx == 0 && bmp.bmWidth == 1)
			bmp_size.cx = 1;
		//bmp_btn_size_ = bmp_size;
	}
	else
	{
		CDC dc;
		dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
		dc.SelectObject(&GetDefaultGuiFont());
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		//bmp_btn_size_ =
		bmp_size = CSize(1, tm.tmHeight);
	}

//	SetBitmapSize(bmp_size);
//	SetButtonSize(bmp_size + CSize(8, 7));

	CSize padding_size= pad_size_; //CSize(5, 7); //WhistlerLook::IsAvailable() ? CSize(5, 9) : CSize(3, 7);
	if ((GetStyle() & TBSTYLE_LIST) == 0)
		padding_size += CSize(3, 3);
//	SendMessage(TB_SETPADDING, 0, MAKELONG(padding_size.cx, padding_size.cy));

	if (bmp_id != 0)
	{
		if (image_bmp.m_hObject != 0 && (mask_bmp.m_hObject != 0 || !create_mask_image))
		{
			UINT flags= ILC_COLOR32;
			if (create_mask_image)
				flags |= ILC_MASK;

			HIMAGELIST himg_list= ImageList_Create(bmp_size.cx, bmp_size.cy, flags, btn_count, 0);
			ASSERT(himg_list != 0);
			if (himg_list)
			{
				img_list.Attach(himg_list);
				img_list.Add(&image_bmp, create_mask_image ? &mask_bmp : static_cast<CBitmap*>(0));
//				SetImageList(&img_list);
			}
		}
		else
		{
			if (HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(bmp_id), RT_BITMAP))
			{
				HIMAGELIST himg_list= ImageList_LoadImage(inst, MAKEINTRESOURCE(bmp_id),
					bmp_size.cx, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
				ASSERT(himg_list != 0);
				if (himg_list)
				{
					img_list.Attach(himg_list);
//					SetImageList(&img_list);
				}
			}
		}
	}

	return bmp_size;
}


void ToolBarWnd::HandleTooltips(bool enable)
{
	handle_tooltips_ = enable;
}
