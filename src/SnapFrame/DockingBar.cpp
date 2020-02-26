/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DockingBar.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "DockingBar.h"
#include "../MemoryDC.h"
#include "MDIFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern COLORREF GetLighterColor(COLORREF rgb_color, float percent);

/////////////////////////////////////////////////////////////////////////////
// CDockingBar

CDockingBar::CDockingBar()
{
}

CDockingBar::~CDockingBar()
{
}

CString CDockingBar::wnd_class_;


BEGIN_MESSAGE_MAP(CDockingBar, CWnd)
	//{{AFX_MSG_MAP(CDockingBar)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDockingBar message handlers


//COLORREF g_rgb_background= RGB(255,255,223);


bool CDockingBar::Create(CWnd* parent, CFramePages* pages, bool show)
{
	ASSERT(parent);
	ASSERT(pages);

	pages_ = pages;

	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	LOGFONT lf;
	HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	lf.lfHeight += 1;
	//lf.lfQuality = ANTIALIASED_QUALITY;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));
	_font.CreateFontIndirect(&lf);
	dc.SelectObject(&_font);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int line_height= tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;
	int bar_height= ((line_height + 6) & ~1) + 1;	// ensure height is odd

	// create docking bar; width will be reset by frame wnd
	if (!CWnd::CreateEx(0, wnd_class_, _T(""), WS_CHILD | (show ? WS_VISIBLE : 0) | WS_CLIPCHILDREN,
		CRect(0, 0, 200, bar_height), parent, -1))
		return false;

	HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDB_DOCK_BAR), RT_BITMAP);
	image_list_.Attach(ImageList_LoadImage(inst, MAKEINTRESOURCE(IDB_DOCK_BAR),
		16, 0, RGB(192,192,192), IMAGE_BITMAP, LR_CREATEDIBSECTION));

	image_list_.SetBkColor(CLR_NONE);

	Refresh(true);

	return true;
}



void CDockingBar::ColectTabs(std::vector<CTab>& tabs)
{
/*
	tabs.clear();
	tabs.push_back(CTab(_T("My Pictures"), CTab::ICN_FOLDER));
	tabs.push_back(CTab(_T("My Documents"), CTab::ICN_FOLDER, true));
	tabs.push_back(CTab(_T("100DCF"), CTab::ICN_FOLDER));
	tabs.push_back(CTab(_T("Camera"), CTab::ICN_CAMERA));
	tabs.push_back(CTab(_T("Compo"), CTab::ICN_PHOTO));
*/
//	Refresh();
}


void CDockingBar::CalcWidth(std::vector<CTab>& tabs, CDC& dc, CRect wnd_rect)
{
	int GAP= wnd_rect.Height() / 2;
	int ICON= 20; // icon width plus space
	CPoint start= wnd_rect.TopLeft();
	start.x += GAP;

	for (int i= 0; i < tabs.size(); ++i)
	{
		CTab& tab= tabs[i];
		CSize text_size= dc.GetTextExtent(tab.name_);
		int width= ICON + text_size.cx;
		tab.location_rect_.SetRect(start.x, start.y, start.x + width, wnd_rect.bottom);

		start.x += width + GAP;
	}
}


void CDockingBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	COLORREF rgb_background= ::GetSysColor(COLOR_APPWORKSPACE);

	CMemoryDC mem_dc(dc, this, rgb_background);
	dc.SelectObject(&_font);
	//mem_dc.SelectStockObject(DEFAULT_GUI_FONT);

	CRect rect;
	GetClientRect(rect);

	mem_dc.Draw3dRect(rect, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DHILIGHT));
	rect.DeflateRect(1, 1);
	mem_dc.Draw3dRect(rect, ::GetSysColor(COLOR_3DDKSHADOW), rgb_background);
	rect.DeflateRect(1, 0);
	rect.top++;

	const int GAP= rect.Height() / 2;
	const int ICON= 18;
	COLORREF rgb_black= ::GetSysColor(COLOR_BTNTEXT);
	COLORREF rgb_dark= ::GetSysColor(COLOR_3DDKSHADOW);
	COLORREF rgb_light= ::GetSysColor(COLOR_3DHILIGHT);
	COLORREF rgb_gray= GetLighterColor(rgb_background, 13.0); // 13.0% brighter

	CPen penBlack(PS_SOLID, 0, rgb_black);
	CPen penDark(PS_SOLID, 0, rgb_dark);
	CPen penLight(PS_SOLID, 0, rgb_light);
	CPen penGray(PS_SOLID, 0, rgb_gray);
	CBrush brGray(::GetSysColor(COLOR_3DFACE));
	CBrush* brush= mem_dc.SelectObject(&brGray);
	mem_dc.SetBkMode(OPAQUE);

	CalcWidth(tabs_, mem_dc, rect);

	for (int index= 0; index < tabs_.size(); ++index)
	{
		CTab& tab= tabs_[index];
		CRect text_rect= tab.location_rect_;
		text_rect.left += ICON;

		COLORREF rgb_tab= tab.active_ ? ::GetSysColor(COLOR_3DFACE) : rgb_background;

		if (tab.active_)		// active tab--taller, gray backgnd
		{
			CRect rect= tab.location_rect_;
			rect.top++;
			rect.right++;
			rect.bottom = rect.top + 1;
			// draw two lines on the top
			mem_dc.FillSolidRect(rect, rgb_dark);
			rect.OffsetRect(0, 1);
			mem_dc.FillSolidRect(rect, rgb_light);
			rect.top++;
			rect.bottom = tab.location_rect_.bottom + 1;
			mem_dc.FillSolidRect(rect, rgb_tab);

			text_rect.top = rect.top;

			rect.top--;
			int bottom= tab.location_rect_.bottom - 1;
			int gap= (bottom - rect.top) / 2;

			POINT vptLeftTriangle[]=
			{
				{ rect.left, rect.top }, { rect.left, bottom + 2 },
				{ rect.left - gap - 1, bottom + 2 }, { rect.left, rect.top }
			};
			mem_dc.SelectStockObject(NULL_PEN);
			mem_dc.Polygon(vptLeftTriangle, array_count(vptLeftTriangle));

			POINT vptRightTriangle[]=
			{
				{ rect.right, rect.top }, { rect.right, bottom + 2 },
				{ rect.right + gap, bottom + 2 },  { rect.right + gap, bottom },
				{ rect.right, rect.top }
			};
			mem_dc.Polygon(vptRightTriangle, array_count(vptRightTriangle));

			// draw slanted line on the right side
			CPen* pen= mem_dc.SelectObject(&penBlack);
			mem_dc.MoveTo(rect.right, rect.top);
			mem_dc.LineTo(rect.right + gap, bottom);
			mem_dc.LineTo(rect.right + gap + 1, bottom);

			// draw slanted line on the left side
			mem_dc.SelectObject(&penDark);
			int left= rect.left - 1;
			mem_dc.MoveTo(left, rect.top);
			mem_dc.LineTo(left - gap, bottom);
			mem_dc.LineTo(left - gap + 1, bottom);

			// draw light slanted line on the left side
			mem_dc.SelectObject(&penLight);
			left++;
			mem_dc.MoveTo(left, rect.top);
			mem_dc.LineTo(left - gap, bottom);
			mem_dc.LineTo(left - gap, bottom + 2);
		}
		else		// inactive tab (smaller, dark background)
		{
			CRect rect= tab.location_rect_;
			rect.left++;
			rect.right++;
			rect.top += 2;
			rect.bottom = rect.top + 1;
			// draw two lines on the top
			mem_dc.FillSolidRect(rect, rgb_dark);
			rect.OffsetRect(0, 1);
			mem_dc.FillSolidRect(rect, rgb_gray);

			// draw slanted line on the right side
			CPen* pen= mem_dc.SelectObject(&penBlack);
			int right= rect.right - 0;
			mem_dc.MoveTo(right, rect.top);
			int bottom= tab.location_rect_.bottom - 1;
			int gap= (bottom - rect.top) / 2;
			mem_dc.LineTo(right + gap, bottom + 1);

			// draw short slanted line on the left side
			mem_dc.SelectObject(&penDark);
			int left= rect.left - 1;
			mem_dc.MoveTo(left, rect.top);
			if (index == 0)	// first tab?
			{
				mem_dc.LineTo(left - gap, bottom);	// in the first tab whole edge is visible
				mem_dc.LineTo(left - gap, bottom + 1);
				// draw light slanted line on the left side
				mem_dc.SelectObject(&penGray);
				mem_dc.MoveTo(left + 1, rect.top);
				mem_dc.LineTo(left - gap + 1, bottom);
				mem_dc.LineTo(left - gap + 1, bottom + 1);
			}
			else
			{
				int left_half= left - gap / 2;
				int bottom_half= (rect.top + bottom) / 2;
				mem_dc.LineTo(left_half, bottom_half);
				mem_dc.LineTo(left_half, bottom_half + 1);
				// draw light slanted line on the left side
				mem_dc.SelectObject(&penGray);
				mem_dc.MoveTo(left + 1, rect.top);
				mem_dc.LineTo(left_half, bottom_half + 2);
				// previous tab is not active?
//				if (index > 0 && !tabs_[index - 1].active_)
//					mem_dc.LineTo(left_half, bottom_half + 3);
			}


			mem_dc.SelectObject(pen);

			text_rect.top = rect.top + 1;
		}

		mem_dc.SetBkColor(rgb_tab);
		mem_dc.DrawText(tab.name_, text_rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_EXPANDTABS | DT_END_ELLIPSIS);

		int offset= tab.active_ ? 0 : 12;
		CPoint icon(tab.location_rect_.left + 1, text_rect.top);

		if (tab.icon_ >= 0)
			image_list_.Draw(&mem_dc, offset + tab.icon_, icon, ILD_NORMAL);
	}

	mem_dc.SelectObject(brush);

	mem_dc.BitBlt();
}


BOOL CDockingBar::OnEraseBkgnd(CDC* dc)
{
	return true;
}



void CDockingBar::Refresh(bool recreate/*= false*/)
{
	if (!pages_)
		return;

	int count= pages_->GetCount();

	if (tabs_.size() != count || recreate)
	{
		tabs_.clear();
		tabs_.reserve(count);

		for (int index= 0; index < count; ++index)
		{
			CSnapFrame* page= pages_->GetPage(index);
			tabs_.push_back(CTab(page->GetTabTitle(), page->GetIconIndex(), index, pages_->GetCurrent() == page));
		}
	}
	else
	{
		for (int index= 0; index < count; ++index)
		{
			CSnapFrame* page= pages_->GetPage(index);
			tabs_[index].active_ = pages_->GetCurrent() == page;
			//.push_back(CTab(page->GetTabTitle(), page->GetIconIndex(), index, ));
		}
	}

	Invalidate();
}


void CDockingBar::OnLButtonDown(UINT flags, CPoint pos)
{
	for (int index= 0; index < tabs_.size(); ++index)
	{
		CTab& tab= tabs_[index];

		if (tab.location_rect_.PtInRect(pos))
		{
			if (!tab.active_ && pages_ && pages_->SelectPage(index, true))
			{
				Refresh();
				Invalidate();
			}
			break;
		}
	}
}


void rgb_to_hsl(float r, float g, float b, float* h, float* s, float* l)
{
	float Min= r < g ? r : g;
	Min = b < Min ? b : Min;

	float Max= r > g ? r : g;
	Max = b > Max ? b : Max;

	*l = (Min + Max) / 2.0f;

	float Dist= Max - Min;
	if (Dist > 0)
		*s = *l <= 0.5f ? Dist / (Min + Max) : Dist / (2.0f - Min - Max);
	else
	{
		*s = 0.0f;
		*h = 0.0f;
		return;
	}

	if (r == Max)
		*h = g == Min ? 5.0f + (Max - b) / Dist : 1.0f - (Max - g) / Dist;
	else if (g == Max)
		*h = b == Min ? 1.0f + (Max - r) / Dist : 3.0f - (Max - b) / Dist;
	else
		*h = r == Min ? 3.0f + (Max - g) / Dist : 5.0f - (Max - r) / Dist;
}


void hsl_to_rgb(float h, float s, float l, float* r, float* g, float* b)
{
	float v= l <= 0.5f ? l * (1.0f + s) : l + s - l * s;
	if (v <= 0.0f)
	{
		*r = *g = *b = 0.0f;
	}
	else
	{
		float m= l + l - v;
		float sv= (v - m) / v;
//		h *= 6.0;
		int ns= int(h);
		float fract= h - ns;
		float vsf= v * sv * fract;
		float mid1= m + vsf;
		float mid2= v - vsf;
		switch (ns)
		{
		case 6:
		case 0: *r = v; *g = mid1; *b = m; break;
		case 1: *r = mid2; *g = v; *b = m; break;
		case 2: *r = m; *g = v; *b = mid1; break;
		case 3: *r = m; *g = mid2; *b = v; break;
		case 4: *r = mid1; *g = m; *b = v; break;
		case 5: *r = v; *g = m; *b = mid2; break;
		}
	}
}



// calculate new brighter color using rgb_color
//
COLORREF GetLighterColor(COLORREF rgb_color, float percent)
{
	float h, s, l;
	rgb_to_hsl(GetRValue(rgb_color) / 255.0f, GetGValue(rgb_color) / 255.0f, GetBValue(rgb_color) / 255.0f, &h, &s, &l);

//	l += (l + 0.01f) * percent / 100.0f;
	l += percent / 100.0f;
	if (l < 0.0f)
		l = 0.0f;
	else if (l > 1.0f)
		l = 1.0f;

	float r, g, b;
	hsl_to_rgb(h, s, l, &r, &g, &b);

	return RGB(uint8(r * 255), uint8(g * 255), uint8(b * 255));
}
