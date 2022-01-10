/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// EditCombo.cpp : implementation file
//

#include "stdafx.h"
#include "EditCombo.h"
#include "ItemIdList.h"
#include "resource.h"
#include "CtrlDraw.h"
#include "GetDefaultLineHeight.h"
#include "MemoryDC.h"
#include "UIElements.h"
#include "GetDefaultGuiFont.h"
#include "AppColors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern std::auto_ptr<Gdiplus::Bitmap> LoadPng(int rsrc_id, const wchar_t* rsrc_type, HMODULE instance);

/////////////////////////////////////////////////////////////////////////////
// EditCombo

static const TCHAR* const WND_CLASS= _T("EditComboCtrlMiK");
static void RegisterWndClass(const TCHAR* class_name);


EditCombo::EditCombo()
{
	RegisterWndClass(WND_CLASS);
	focus_ = false;
	scan_in_progress_ = false;
	state_ = 0;
	margin_ = 0;
	image_list_ = nullptr;
	image_to_draw_ = -1;
	tb_bitmap_id_ = 0;
	create_controls_ = true;
	button_commands_[0] = button_commands_[1] = 0;
	tb_bitmap_id_ = 0;
	margins_.SetRectEmpty();
	create_flags_ = 0;
	auto& colors = ::GetAppColors();
	SetColors(colors[AppColors::Background], colors[AppColors::EditBox], colors[AppColors::Text]);
	border_color_ = colors[AppColors::Separator];
}

EditCombo::~EditCombo()
{}

const int EDIT_ID= 100;
const int ID_TOOLBAR= 101;
const int CMD_RECENT= 102;
const int CMD_BUTTON= 103;
const int CMD_BUTTON_2= 104;

BEGIN_MESSAGE_MAP(EditCombo, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_COMMAND_RANGE(CMD_BUTTON, CMD_BUTTON_2, OnTbButton)
	ON_EN_SETFOCUS(EDIT_ID, OnSetFocus)
	ON_EN_KILLFOCUS(EDIT_ID, OnKillFocus)
	ON_NOTIFY(TBN_DROPDOWN, ID_TOOLBAR, OnTbDropDown)
	ON_NOTIFY(TBN_GETDISPINFO, ID_TOOLBAR, OnTbGetDispInfo)
	ON_NOTIFY(TBN_GETINFOTIP, ID_TOOLBAR, OnGetInfoTip)
	ON_EN_CHANGE(EDIT_ID, OnTextChanged)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE instance= AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, class_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = CS_VREDRAW | CS_HREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}

/////////////////////////////////////////////////////////////////////////////
// EditCombo message handlers


bool EditCombo::Create(CWnd* parent, int toolbar_images, int state_1_cmd, int state_2_cmd, UINT flags)
{
	SetParams(toolbar_images, state_1_cmd, state_2_cmd, flags);

	create_controls_ = false;	// pre create wnd cannot create any windows due to the open hook proc

	if (!CWnd::CreateEx(0, WND_CLASS, nullptr, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), parent, 0-1))
		return false;

	CreateControls();

	return true;
}


void EditCombo::SetParams(int toolbar_images, int state_1_cmd, int state_2_cmd, UINT flags)
{
	create_flags_ = flags;
	tb_bitmap_id_ = toolbar_images;
	button_commands_[0] = state_1_cmd;
	button_commands_[1] = state_2_cmd;
}


void EditCombo::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	if (create_controls_)
		CreateControls();
}


void EditCombo::CreateControls()
{
	ASSERT(button_commands_[0] != 0 || button_commands_[1] != 0 || tb_bitmap_id_ != 0);

	edit_box_.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, CRect(0,0,0,0), this, EDIT_ID);

	edit_box_.ConnectEndEditKeyPress(boost::bind(&EditCombo::EndEdit, this, _1));

	SetFont(&GetDefaultGuiFont());
	edit_box_.SetFont(&GetDefaultGuiFont());

	int commands[]= { CMD_RECENT, CMD_BUTTON, CMD_BUTTON_2 };
	char pattern[]= "vi.";
	bool create_auto_complete= !!(create_flags_ & AUTO_COMPLETE);
	if (!create_auto_complete)
		pattern[0] = '.';
	if (create_flags_ & TWO_BUTTONS)
		pattern[2] = 'i';
	toolbar_.Create(pattern, commands, tb_bitmap_id_, 0, this, ID_TOOLBAR);
	toolbar_.HandleTooltips(false);
	toolbar_.SetOnIdleUpdateState(false);
	toolbar_.CreateDisabledImageList(tb_bitmap_id_, 0.0f, 0.0f, 0.5f);

	if (create_auto_complete)
	{
		TBBUTTONINFO bi;
		memset(&bi, 0, sizeof(bi));
		bi.cbSize = sizeof (bi);
		bi.dwMask = TBIF_SIZE | TBIF_IMAGE;
bi.cx = Pixels(16);
		bi.iImage = I_IMAGENONE;
		toolbar_.SetButtonInfo(CMD_RECENT, &bi);
		toolbar_.AutoResize();
	}

	int height= GetDefaultLineHeight();

	CRect rect(0,0,0,0);
	toolbar_.GetWindowRect(rect);
	margin_ = (rect.Height() - height) / 2;

	height = rect.Height()+2;

	GetWindowRect(rect);
	SetWindowPos(nullptr, 0, 0, rect.Width(), height + margins_.top + margins_.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	if (create_auto_complete)
	{
		auto_complete_.Create();
		auto_complete_.AlignToParent(true);
		auto_complete_.AutoPopup(false);
		auto_complete_.AutoUnhook(false);
	}

	Resize();
}


void EditCombo::OnSize(UINT type, int cx, int cy)
{
	Resize();
}


void EditCombo::Resize()
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	rect.DeflateRect(margins_);

	if (toolbar_.m_hWnd && edit_box_.m_hWnd && rect.Width() > 0 && rect.Height() > 0)
	{
		CRect tb(0,0,0,0);
		toolbar_.GetWindowRect(tb);
		rect.DeflateRect(1,1,1,1);//这3行被迫缩小图标，以免hover时的3d边框遮住了box的边框
		rect.left +=1;//
		rect.right +=1;//
		toolbar_.SetWindowPos(nullptr, rect.right - tb.Width() - 1, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		rect.DeflateRect(margin_ / 2, margin_, tb.Width() + 1, margin_);

		if (image_list_)
		{
			IMAGEINFO ii;
			if (::ImageList_GetImageInfo(image_list_, 0, &ii))
				rect.left += (ii.rcImage.right - ii.rcImage.left) * 6 / 5;
		}
		else if (image_.get())
		{
			rect.left += image_->GetWidth() * 6 / 5;
		}

		edit_box_.SetWindowPos(nullptr, rect.left, rect.top, std::max(0, rect.Width()), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


CEdit& EditCombo::GetEditCtrl()
{
	return edit_box_;
}


BOOL EditCombo::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	MemoryDC mem_dc(*dc, this, ::GetSysColor(COLOR_3DFACE));//outside_color_);

	rect.DeflateRect(margins_);
	mem_dc.FillSolidRect(rect, border_color_);
	rect.DeflateRect(1,1,1,1);
	mem_dc.FillSolidRect(rect, backgnd_color_);
	//CtrlDraw::DrawComboBorder(mem_dc, rect, false);//CalcNewColor(::GetSysColor(COLOR_3DSHADOW), 10.0));//此处颜色定义无用啊？？？

	if (image_list_ && image_to_draw_ >= 0)
	{
		IMAGEINFO ii;
		int x= rect.left;
		int y= rect.top;
		if (::ImageList_GetImageInfo(image_list_, image_to_draw_, &ii))
		{
			x += (ii.rcImage.right - ii.rcImage.left) / 5;
			y += (rect.Height() - (ii.rcImage.bottom - ii.rcImage.top)) / 2;
		}
		::ImageList_Draw(image_list_, image_to_draw_, mem_dc, x, y, ILD_NORMAL | ILD_TRANSPARENT);
	}
	else if (image_.get())
	{
		int x= rect.left;
		int y= rect.top;

		Gdiplus::Graphics g(mem_dc);
		UINT w= image_->GetWidth();
		UINT h= image_->GetHeight();
		Gdiplus::Rect r(x + w / 5, y + (rect.Height() - h) / 2, w, h);
		g.DrawImage(image_.get(), r);
	}

	mem_dc.BitBlt();

	return true;
}


void EditCombo::Run(int cmd)
{
	if (!run_cmd_event_.empty())
		run_cmd_event_(cmd);
	else if (CWnd* parent = GetParent())
		parent->SendMessage(WM_COMMAND, cmd, 0);

}


void EditCombo::OnTbButton(UINT cmd)
{
	Run(GetButtonCmd(cmd));
}


void EditCombo::OnSetFocus()
{
	focus_ = true;
	Invalidate();
}


void EditCombo::OnKillFocus()
{
	focus_ = false;
	auto_complete_.ShowWindow(SW_HIDE);
	Invalidate();
}


void EditCombo::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info_tip->iItem)
	{
	case CMD_RECENT:
		OnAutoCompleteDropDown();
		break;

	default:
		break;
	}
}


void EditCombo::OnTbGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	NMTBDISPINFO* disp= reinterpret_cast<NMTBDISPINFO*>(nmhdr);
	*result = 0;

	if (disp->dwMask && TBIF_IMAGE)
	{
		switch (disp->idCommand)
		{
		case CMD_BUTTON:
			disp->iImage = state_ == 0 ? 1 : 2;
			break;
		case CMD_BUTTON_2:
			disp->iImage = state_ == 0 ? 2 : 1;
			break;
		}
	}
}


void EditCombo::OnGetInfoTip(NMHDR* nmhdr, LRESULT* result)
{
	NMTBGETINFOTIP* info_tip= reinterpret_cast<NMTBGETINFOTIP*>(nmhdr);
	*result = 1;

	if (info_tip->iItem == CMD_BUTTON || info_tip->iItem == CMD_BUTTON_2)
	{
		CString tip;
		tip.LoadString(GetButtonCmd(info_tip->iItem));
		_tcsncpy(info_tip->pszText, tip, INFOTIPSIZE);
	}
}


void EditCombo::OnAutoCompleteDropDown()
{
	if (auto_complete_.IsWindowVisible())
		auto_complete_.ShowWindow(SW_HIDE);
	else
	{
		auto_complete_.ShowList();
		edit_box_.SetFocus();
	}
}


void EditCombo::SetHistory(const std::vector<String>& history)
{
	auto_complete_.ControlEditBox(&edit_box_, &history);
	EnableDropDown(!history.empty());
}


void EditCombo::SetState(int state)
{
	state_ = state;
	if (toolbar_.m_hWnd)
		toolbar_.Invalidate();
}


slot_connection EditCombo::ConnectRunCommand(RunCommand::slot_function_type fn)
{
	return run_cmd_event_.connect(fn);
}


slot_connection EditCombo::ConnectFinishCommand(FinishCommand::slot_function_type fn)
{
	return finish_cmd_event_.connect(fn);
}


void EditCombo::EndEdit(int key)
{
	if (!finish_cmd_event_.empty())
	{
		if (key == VK_ESCAPE)
			finish_cmd_event_(false);
		else if (key == VK_RETURN)
			finish_cmd_event_(true);
	}
}


int EditCombo::GetButtonCmd(UINT btn) const
{
	switch (btn)
	{
	case CMD_BUTTON:
		return button_commands_[state_ == 0 ? 0 : 1];
	case CMD_BUTTON_2:
		return button_commands_[1];
	default:
		ASSERT(false);
	}
	return 0;
}


void EditCombo::EnableDropDown(bool enable)
{
	toolbar_.EnableButton(CMD_RECENT, enable);
}


void EditCombo::EnableButton(bool enable, int index)
{
	toolbar_.EnableButton(index == 0 ? CMD_BUTTON : CMD_BUTTON_2, enable);
}


void EditCombo::SetImageList(HIMAGELIST image_list)
{
	image_list_ = image_list;
}


void EditCombo::SetImageIndex(int image)
{
	image_to_draw_ = image;
	if (m_hWnd)
		Invalidate();
}


void EditCombo::SetImage(int image_rsrc_id)
{
	image_ = LoadPng(image_rsrc_id, L"PNG", AfxGetResourceHandle());
	if (m_hWnd)
		Invalidate();
}


AutoCompletePopup& EditCombo::GetAutoComplete()
{
	return auto_complete_;
}


void EditCombo::OnTextChanged()
{
	// forward notification to the parent window

	const MSG* msg= CWnd::GetCurrentMessage();

	if (CWnd* parent= GetParent())
		parent->SendMessage(msg->message, MAKELONG(GetDlgCtrlID(), HIWORD(msg->wParam)), msg->lParam);
}


void EditCombo::SetMargins(const CRect& margins)
{
	margins_ = margins;
	if (m_hWnd)
		Invalidate();
}


void EditCombo::SetColors(COLORREF background, COLORREF editbox, COLORREF text)
{
	backgnd_color_ = editbox;
	backgnd_color_brush_.DeleteObject();
	backgnd_color_brush_.CreateSolidBrush(editbox);

	text_color_ = text;
	outside_color_ = background;

	if (m_hWnd)
		Invalidate();
}


BOOL EditCombo::PreTranslateMessage(MSG* msg)
{
	if (msg && msg->message == WM_KEYDOWN && msg->wParam == VK_ESCAPE && ::GetFocus() == edit_box_.m_hWnd && !finish_cmd_event_.empty())
	{
		EndEdit(int(msg->wParam));
		return true;
	}

	return CWnd::PreTranslateMessage(msg);
}


HBRUSH EditCombo::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr = CWnd::OnCtlColor(dc, wnd, ctl_color);

	dc->SetTextColor(text_color_);
	dc->SetBkColor(backgnd_color_);

	return backgnd_color_brush_;
}
