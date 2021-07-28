/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifStatusBar.cpp: implementation of the ExifStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ExifStatusBar.h"
#include "DrawFields.h"
#include "Columns.h"
#include "CatchAll.h"
#include "LoadImageList.h"
#include "UIElements.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ExifStatusBar::ExifStatusBar(const Columns& columns) : columns_(columns)
{
	recipient_ = 0;
	DefaultFields();

	::LoadPngImageList(funnel_icon_, IDB_FUNNEL, ::GetSysColor(COLOR_3DFACE), true, 1);

	background_ = RGB(255, 0, 0);
	text_color_ = 0;
	dim_text_ = 0;
}


ExifStatusBar::~ExifStatusBar()
{
}


BEGIN_MESSAGE_MAP(ExifStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(ExifStatusBar)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void ExifStatusBar::DrawItem(DRAWITEMSTRUCT* draw)
{
	if (const TCHAR* text= reinterpret_cast<const TCHAR*>(draw->itemData))
	{
		int mode= ::SetBkMode(draw->hDC, TRANSPARENT);

		if (CDC* dc= CDC::FromHandle(draw->hDC))
		{
			CRect rect= draw->rcItem;
			rect.left += Pixels(2);

			if (draw->itemID == 1 && *text == '@')
			{
				if (funnel_icon_.m_hImageList)
					funnel_icon_.Draw(dc, 0, CPoint(rect.left, rect.top + rect.Height()/5), ILD_TRANSPARENT);
				rect.left += Pixels(20);
				++text;
			}

			if (draw->itemID == 1 && *text == '!')
			{
				dc->DrawIcon(rect.left, rect.top + rect.Height()/6, ::AfxGetApp()->LoadIcon(IDI_WARNING_));
				rect.left += Pixels(20);
				++text;
			}

			DrawFields::Draw(*dc, text, rect, text_color_, dim_text_, text_color_, false);
		}

		::SetBkMode(draw->hDC, mode);
	}
}


void ExifStatusBar::OnContextMenu(CWnd*, CPoint pos)
{
	OptionsPopup(pos);
}


void ExifStatusBar::OptionsPopup(CPoint pos)
{
	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	try
	{
		columns_.GetPopupMenu(menu, fields_);

		const int ID_RESET_COLUMNS= 99998;

		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_RESET_COLUMNS, _T("重置为默认值"));

		if (pos.x == -1 && pos.y == -1)
			GetCursorPos(&pos);

		if (recipient_)
			recipient_->PopupMenu(true);

		int field= menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pos.x, pos.y, this);

		if (recipient_)
			recipient_->PopupMenu(false);

		if (field <= 0)
			return;

		std::vector<uint16> sel= fields_;

		if (field == ID_RESET_COLUMNS)
		{
			DefaultFields();
		}
		else
		{
			--field;	// 0..n-1 range now (field numbers are not consecutive though)

			std::vector<uint16>::iterator it= remove(sel.begin(), sel.end(), field);
			if (it != sel.end())
				sel.erase(it);
			else
				//TODO: revise
				sel.insert(lower_bound(sel.begin(), sel.end(), field), field);

			fields_.swap(sel);
		}

		// notify parent
		if (recipient_)
			recipient_->FieldSelectionChanged();
	}
	CATCH_ALL
}


void ExifStatusBar::DefaultFields()
{
	fields_.clear();
	fields_.reserve(8);
	fields_.push_back(COL_FILE_NAME);
	fields_.push_back(COL_DATE_TIME);
	fields_.push_back(COL_EXP_TIME);
	fields_.push_back(COL_FNUMBER);
	fields_.push_back(COL_EXP_BIAS);
	fields_.push_back(COL_FOCAL_LENGTH);
	fields_.push_back(COL_ISO);
	fields_.push_back(COL_DIMENSIONS);
}


void ExifStatusBar::OnLButtonDown(UINT flags, CPoint pos)
{
	CRect rect(0, 0, 0, 0);
	GetItemRect(1, rect);

	if (rect.PtInRect(pos))
		if (recipient_)
			recipient_->StatusPaneClick(rect);
}


void ExifStatusBar::SetColors(COLORREF background, COLORREF text, COLORREF dim_text)
{
	background_ = background;
	text_color_ = text;
	dim_text_ = dim_text;

	if (m_hWnd)
	{
		GetStatusBarCtrl().SetBkColor(background);
		Invalidate();
	}
	else
	{
		ASSERT(false);
	}
}


int ExifStatusBar::OnCreate(LPCREATESTRUCT create_struct)
{
	CClientDC dc(nullptr);
	auto* old = dc.SelectObject(GetDefaultGuiFont());
	TEXTMETRIC tm;
	VERIFY(dc.GetTextMetrics(&tm));
	dc.SelectObject(old);
	// extra height
	m_nMinHeight = tm.tmHeight * 1.5 + tm.tmInternalLeading;
	m_cyBottomBorder = tm.tmInternalLeading / 2;
	
	//CWnd* pParentWnd;
	//pParentWnd->ModifyStyle(WS_THICKFRAME, 0, 0);
	if (CStatusBar::OnCreate(create_struct) == -1)
		return -1;

	// to customize colors and look and feel
	::SetWindowTheme(m_hWnd, L"", L"");

	return 0;
}


BOOL ExifStatusBar::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);
	rect.bottom += m_cyBottomBorder;
	dc->FillSolidRect(rect, background_);

	return true;
}