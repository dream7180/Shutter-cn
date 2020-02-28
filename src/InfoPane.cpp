/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// InfoPane.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "InfoPane.h"
#include "Color.h"
#include "PhotoInfo.h"
#include "MemoryDC.h"
#include "SnapFrame/SnapView.h"
#include "ImgDb.h"
#include <boost/bind.hpp>
#include "CatchAll.h"
#include "PhotoCtrl.h"
#include "Config.h"
#include "Color.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// InfoPane

InfoPane::InfoPane()
{
	photo_ = 0;

	const TCHAR* REGISTRY_INFO_PANE= _T("InfoPane");
	profile_show_preview_.Register(REGISTRY_INFO_PANE, _T("ShowPreview"), false);
	profile_raw_info_.Register(REGISTRY_INFO_PANE, _T("RawInfo"), false);
	profile_hide_unknown_.Register(REGISTRY_INFO_PANE, _T("HideUnknown"), true);

	img_preview_ = profile_show_preview_;
	raw_data_ = profile_raw_info_;
	hide_unknown_ = profile_hide_unknown_;
}

InfoPane::~InfoPane()
{
}

CString InfoPane::wnd_class_;


BEGIN_MESSAGE_MAP(InfoPane, PaneWnd)
	//{{AFX_MSG_MAP(InfoPane)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_COMMAND(ID_RAW_INFO, OnRawInfo)
	ON_UPDATE_COMMAND_UI(ID_RAW_INFO, OnUpdateRawInfo)
	ON_COMMAND(ID_COPY_INFO, OnCopyInfo)
	ON_UPDATE_COMMAND_UI(ID_COPY_INFO, OnUpdateCopyInfo)
	ON_COMMAND(ID_TASK_EXPORT, OnExportExif)
	ON_UPDATE_COMMAND_UI(ID_TASK_EXPORT, OnUpdateExportExif)
	ON_COMMAND(ID_TOGGLE_PREVIEW, OnTogglePreview)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_PREVIEW, OnUpdateTogglePreview)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
//	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, OnGetDispInfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// InfoPane message handlers

namespace {
	float g_backgnd_brightness= 7.0f;
	float g_dark_backgnd_brightness= -4.0f;
	enum { LINE_NAME, LINE_PATH, LINE_DIM, LINE_FILESIZE, LINE_LAST };
}

bool InfoPane::Create(CWnd* parent)
{
	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	if (!PaneWnd::CreateEx(0, wnd_class_, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, AFX_IDW_PANE_FIRST + 5))
		return false;

	COLORREF rgb_gray= ::GetSysColor(COLOR_3DFACE);
	COLORREF rgb_background= CalcNewColor(rgb_gray, g_backgnd_brightness);
	COLORREF rgb_background_dark= CalcShade(rgb_background, g_dark_backgnd_brightness);

	if (!disp_wnd_.Create(this, boost::bind(&InfoPane::GetText, this, _1, _2, _3, _4), rgb_background, rgb_background_dark))
		return false;

	if (!bar_.Create(this, boost::bind(&InfoPane::Filter, this, _1, _2), hide_unknown_))
		return false;

	static const int commands[]=
	{
		ID_COPY_INFO, ID_RAW_INFO, ID_TASK_EXPORT, ID_TOGGLE_PREVIEW
	};

	tool_bar_wnd_.SetOwnerDraw(true);
	tool_bar_wnd_.SetPadding(8, 10);

	if (!tool_bar_wnd_.Create("Pxpx", commands, /*IsCaptionBig() ? IDB_INFO_TOOLBAR_BIG : */IDB_INFO_TOOLBAR, IDS_INFO_TOOLBAR, this, -1))
		return false;

	tool_bar_wnd_.CWnd::SetOwner(this);

	SetColors();

	AddBand(&tool_bar_wnd_, this);

	return true;
}


static const int g_PREVIEW_HEIGHT= 130;

CRect InfoPane::GetPreviewRect()
{
	CRect rect;
	GetClientRect(rect);

	rect.top = rect.bottom - g_PREVIEW_HEIGHT;
	if (rect.Height() <= 0)
		return CRect(0,0,0,0);		// no space

	return rect;
}


BOOL InfoPane::OnEraseBkgnd(CDC* dc)
{
	COLORREF rgb_background = g_Settings.AppColors()[AppColors::Background];// CalcNewColor(::GetSysColor(COLOR_3DFACE), g_backgnd_brightness);
	MemoryDC mem_dc(*dc, this, rgb_background);

	if (img_preview_ && photo_)
	{
		// draw preview
		CRect rect= GetPreviewRect();
		//mem_dc.FillSolidRect(rect.left, rect.top, rect.Width(), 1, ::GetSysColor(COLOR_3DSHADOW));
		rect.DeflateRect(0, 5);
		photo_->Draw(&mem_dc, rect, rgb_background);
	}

	mem_dc.BitBlt();

	return true;
}


void InfoPane::OnSize(UINT type, int cx, int cy)
{
	PaneWnd::OnSize(type, cx, cy);

	Resize();
}


void InfoPane::Resize()
{
	if (disp_wnd_.m_hWnd && bar_.m_hWnd)
	{
		CRect client(0,0,0,0);
		GetClientRect(client);

		CRect r(0,0,0,0);
		bar_.GetClientRect(r);

		int h= r.Height();
		bar_.SetWindowPos(0, client.left, client.top, client.Width(), h, SWP_NOZORDER | SWP_NOACTIVATE);

		int wnd_height= client.Height() - h;
		if (img_preview_)
			wnd_height -= g_PREVIEW_HEIGHT;
		if (wnd_height < 0)
			wnd_height = 0;

		CSize wnd_size(client.Width(), std::max(0, wnd_height));
		disp_wnd_.SetWindowPos(0, 0, h, wnd_size.cx, wnd_size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


// display photo info
//
void InfoPane::UpdateInfo(PhotoInfoPtr photo, bool force)
{
	if (photo_ == photo && !force)
		return;

	photo_ = photo;

	// Note: ideally ResetText() should be invoked for hidden pane (when photo is null),
	// but that leads to scrolling problems with list ctrl;
	//TODO: replace list ctrl by custom wnd
	if (IsPaneVisible())
		ResetText();
}


void InfoPane::ResetText()
{
	try
	{
		file_info_.Clear();

		if (photo_)
		{
			try
			{
				photo_->CompleteInfo(GetImageDataBase(true, true), file_info_);
			}
			catch (...)
			{
				ASSERT(false);
			}
		}

		Filter(bar_.FilterText(), bar_.IsUnknownHidden());

		if (img_preview_)
			InvalidateRect(GetPreviewRect());
	}
	CATCH_ALL
}


void InfoPane::Filter(const CString& text, bool hide_unknown)
{
	hide_unknown_ = hide_unknown;

	selected_lines_.clear();

	if (photo_ != 0)
	{
		int count = file_info_.Count() + LINE_LAST;

		if (photo_->HasEmbeddedColorProfile())
			++count;

		CString pattern= text;
		pattern.MakeLower();

		selected_lines_.reserve(count);
		for (int i= 0; i < count; ++i)
		{
			int line= i - LINE_LAST;

			if (hide_unknown || !text.IsEmpty())
			{
				const int MAX= 2000;
				TCHAR buf[MAX + 1];
				GetLineText(line, 1, buf, MAX);		// get tag name
				buf[MAX] = 0;

				if (hide_unknown && _tcsicmp(buf, _T("unknown")) == 0)
					continue;

				if (text.IsEmpty())
				{
					selected_lines_.push_back(line);
					continue;
				}

				CString s= buf;
				s.MakeLower();
				if (s.Find(pattern) >= 0)
				{
					selected_lines_.push_back(line);
					continue;
				}

				GetLineText(line, 2, buf, MAX);		// get tag value
				buf[MAX] = 0;
				s = buf;
				s.MakeLower();
				if (s.Find(pattern) >= 0)
				{
					selected_lines_.push_back(line);
					continue;
				}
			}
			else
				selected_lines_.push_back(line);
		}
	}

	disp_wnd_.SetInfo(static_cast<int>(selected_lines_.size()));
}


String InfoPane::GetText() const
{
	if (photo_)
	{
		String str= _T("File: ") + photo_->GetDisplayPath() + _T("\r\n");
		str += file_info_.GetInfo(raw_data_);
		return str;
	}

	return _T("");
}


BOOL InfoPane::IsFrameWnd() const
{
	return true;	// true so InfoPane can handle UI messages sent by ToolBarWnd
}


void InfoPane::OnRawInfo()
{
	raw_data_ = !raw_data_;
	disp_wnd_.Invalidate();
}

void InfoPane::OnUpdateRawInfo(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(raw_data_ ? 1 : 0);
}


void InfoPane::OnCopyInfo()
{
	if (!OpenClipboard())
		return;

	EmptyClipboard();

	String str= GetText();

	// Allocate a global memory object for the text

	if (HGLOBAL copy= ::GlobalAlloc(GMEM_MOVEABLE, (str.length() + 1) * sizeof TCHAR))
	{
		// Lock the handle and copy the text to the buffer

		if (void* copy_str= ::GlobalLock(copy))
		{
			memcpy(copy_str, str.c_str(), (str.length() + 1) * sizeof TCHAR);

			::GlobalUnlock(copy);

			// Place the handle on the clipboard

			SetClipboardData(sizeof TCHAR == 2 ? CF_UNICODETEXT : CF_TEXT, copy);
		}
	}

	// Close the clipboard

	CloseClipboard();
}

void InfoPane::OnUpdateCopyInfo(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(photo_ != 0);
}


void InfoPane::OnExportExif()
{
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_TASK_EXPORT, 0);
}

void InfoPane::OnUpdateExportExif(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(photo_ != 0);
}


void InfoPane::OnTogglePreview()
{
	img_preview_ = !img_preview_;
	Resize();
	ResetText();
}

void InfoPane::OnUpdateTogglePreview(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(img_preview_ ? 1 : 0);
}


void InfoPane::OnDestroy()
{
	profile_show_preview_ = img_preview_;
	profile_raw_info_ = raw_data_;
	profile_hide_unknown_ = hide_unknown_;

	PaneWnd::OnDestroy();
}


void InfoPane::CurrentModified(PhotoInfoPtr photo)
{
	UpdateInfo(photo, true);
}


void InfoPane::CurrentChanged(PhotoInfoPtr photo)
{
	UpdateInfo(photo);
}


void InfoPane::GetNameText(int line, TCHAR* text, int max_len)
{
	if (photo_ == 0)
		return;

	switch (line)
	{
	case LINE_NAME:
		_tcsncpy(text, _T("File Name"), max_len);
		break;
	case LINE_PATH:
		_tcsncpy(text, _T("File Path"), max_len);
		break;
	case LINE_DIM:
		_tcsncpy(text, _T("Dimensions"), max_len);
		break;
	case LINE_FILESIZE:
		_tcsncpy(text, _T("File Size"), max_len);
		break;
	default:
		ASSERT(false);
		break;
	}
}


void InfoPane::GetValueText(int line, TCHAR* text, int max_len)
{
	if (photo_ == 0)
		return;

	switch (line)
	{
	case LINE_NAME:
		_tcsncpy(text, photo_->GetNameAndExt().c_str(), max_len);
		break;
	case LINE_PATH:
		_tcsncpy(text, photo_->GetDisplayPath().c_str(), max_len);
		break;
	case LINE_DIM:
		_tcsncpy(text, photo_->Size().c_str(), max_len);
		break;
	case LINE_FILESIZE:
		_tcsncpy(text, photo_->FileSize().c_str(), max_len);
		break;
	default:
		ASSERT(false);
		break;
	}
}


void InfoPane::GetText(int index, int col, TCHAR* text, int max_len)
{
	if (photo_ == 0)
		return;

	if (index < 0 || index >= selected_lines_.size())
	{
		ASSERT(false);
		return;
	}

	int line= selected_lines_[index];

	GetLineText(line, col, text, max_len);
}


void InfoPane::GetLineText(int line, int col, TCHAR* text, int max_len)
{
	if (line < 0)
	{
		line += LINE_LAST;

		switch (col)
		{
		case 0:	// no tag
			text[0] = _T('-');
			text[1] = _T('\0');
			break;

		case 1:	// name
			GetNameText(line, text, max_len);
			break;

		case 2:	// value
			GetValueText(line, text, max_len);
			break;
		}
	}
	else if (photo_ != 0 && line < file_info_.Count())
	{
		switch (col)
		{
		case 0:	// tag
			file_info_.GetTagText(line, text, max_len);
			break;

		case 1:	// name
			file_info_.GetNameText(line, text, max_len);
			break;

		case 2:	// value
			if (raw_data_)
				file_info_.GetValueText(line, text, max_len);
			else
				file_info_.GetInterpretedText(line, text, max_len);
			break;
		}
	}
	else if (line == file_info_.Count() && photo_->GetColorProfile() != 0)
	{
		switch (col)
		{
		case 0:	// tag
			*text = 0;
			break;

		case 1:	// name
			_tcsncpy(text, _T("Embedded ICC Profile"), max_len);
			break;

		case 2:	// value
			_tcsncpy(text, photo_->GetColorProfile()->GetProductInfo().c_str(), max_len);
			break;
		}
	}
}

/*
void InfoPane::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	if ((disp_info->item.mask & LVIF_TEXT) == 0 || photo_ == 0)
		return;

	if (photo_->icc_profile_.get() == 0)
		return;

	switch (disp_info->item.iSubItem)
	{
	case 0:	// tag
		*disp_info->item.pszText = 0;
		break;

	case 1:	// name
		_tcsncpy(disp_info->item.pszText, _T("Embedded ICC Profile"), disp_info->item.cchTextMax);
		break;

	case 2:	// value
		_tcsncpy(disp_info->item.pszText, photo_->icc_profile_->GetProductInfo().c_str(), disp_info->item.cchTextMax);
		break;
	}
}
*/
/*
void InfoPane::CaptionHeightChanged(bool big)
{
	tool_bar_wnd_.ReplaceImageList(big ? IDB_INFO_TOOLBAR_BIG : IDB_INFO_TOOLBAR);
	ResetBandsWidth();
}
*/

void InfoPane::SetColors()
{
	auto dim_text = g_Settings.AppColors()[AppColors::DimText];
	auto text = g_Settings.AppColors()[AppColors::Text];
	auto backgnd = g_Settings.AppColors()[AppColors::Background];

	float brightness = CalcColorBrightness(backgnd);

	COLORREF dark_backgnd = brightness > 128 ? CalcShade(backgnd, g_dark_backgnd_brightness) : CalcShade(backgnd, g_backgnd_brightness);

	disp_wnd_.SetColors(backgnd, dark_backgnd, text, dim_text);

	tool_bar_wnd_.Invalidate();
}


void InfoPane::OptionsChanged(OptionsDlg& dlg)
{
	SetColors();
}
