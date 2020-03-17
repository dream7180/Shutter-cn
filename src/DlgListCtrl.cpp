/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DlgListCtrl.cpp : implementation file
//
/////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DlgListCtrl.h"
#include "FindTabCtrl.h"
#include "LoadImageList.h"
#include "DrawBmpEffects.h"
#include "Dib.h"
#include "CtrlDraw.h"
#include "Color.h"

CImageList DlgListCtrl::img_list_triangles_;


BEGIN_MESSAGE_MAP(DlgListCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()


static const TCHAR* cls_name_= _T("DialogListCtrl");
static const int LAST_TRIANGLE= 6;
static const int TRIANGLE_LEFT_SPACE= 5;
static const int TRIANGLE_RIGHT_SPACE= 5;
static const int SHADOW_SPACE= 3;
static const int TOP_SPACE= 1 + SHADOW_SPACE;
static const int IDEAL_HEIGHT= 22;


DlgListCtrl::DlgListCtrl()
{
	bottom_margin_ = top_margin_ = left_margin_ = right_margin_ = 0;
	draw_checkboxes_ = false;

	RegisterWndClass();

	if (img_list_triangles_.m_hImageList == 0){
		CDC dc;
		CDC* pdc = nullptr;
		//if (pdc == nullptr)
			//{
			dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
			pdc = &dc;
		//}
		int log_inch_x = pdc->GetDeviceCaps(LOGPIXELSX);
		LoadImageList(img_list_triangles_, IDB_TRIANGLES, 14/static_cast<Gdiplus::REAL>(log_inch_x)*96, ::GetSysColor(COLOR_3DFACE));
	}
	image_list_ = 0;
	//LOGFONT lf;
	//::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof lf, &lf);
	//_tcscpy(lf.lfFaceName, _T("Arial"));
	//lf.lfHeight = -10;
	//small_fnt_.CreateFontIndirect(&lf);

	extend_last_dlg_to_whole_wnd_ = false;
	range_size_ = CSize(0, 0);
	header_height_ = IDEAL_HEIGHT;	// default height
	scroll_bars_size_ = CSize(::GetSystemMetrics(SM_CXVSCROLL), ::GetSystemMetrics(SM_CXVSCROLL));
	single_expand_ = false;
//	single_expand_ =1;
}


DlgListCtrl::~DlgListCtrl()
{
}


void DlgListCtrl::SetRightMargin(int margin)
{
	right_margin_ = margin;
}

void DlgListCtrl::SetLeftMargin(int margin)
{
	left_margin_ = margin;
}

void DlgListCtrl::SetTopMargin(int margin)
{
	top_margin_ = margin;

	Invalidate();
	ResizeSubDialogs();
}

void DlgListCtrl::SetBottomMargin(int margin)
{
	bottom_margin_ = margin;

	Invalidate();
	ResizeSubDialogs();
}


bool DlgListCtrl::Create(CWnd* parent, int dlg_id)
{
	if (!CWnd::CreateEx(WS_EX_CONTROLPARENT, cls_name_, 0, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, dlg_id))
		return false;

	return true;
}


bool DlgListCtrl::Create(CWnd* parent)
{
	return Create(parent, -1);
}


void DlgListCtrl::RegisterWndClass()
{
	HINSTANCE inst = AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(inst, cls_name_, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = 0;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = inst;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = cls_name_;

		AfxRegisterClass(&wndcls);
	}
}


void DlgListCtrl::PreSubclassWindow()
{
	if (CFont* font= GetParent()->GetFont())
		SetFont(font);

	CalcHeaderHeight();

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	//CreateSubDlg();

	CWnd::PreSubclassWindow();
}


void DlgListCtrl::SelectFont(CDC& dc)
{
	if (CFont* font= GetFont())
		dc.SelectObject(font);
	else{
		LOGFONT lf;
		HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		::GetObject(hfont, sizeof(lf), &lf);
		lf.lfWeight = FW_NORMAL;
		lf.lfHeight += 1;
		_tcscpy(lf.lfFaceName, _T("Tahoma"));
		//lf.lfQuality = ANTIALIASED_QUALITY;
		_font.CreateFontIndirect(&lf);
		dc.SelectObject(&_font);
		//dc.SelectStockObject(DEFAULT_GUI_FONT);
	}
}


void DlgListCtrl::CalcHeaderHeight()
{
	CClientDC dc(this);
	SelectFont(dc);
	TEXTMETRIC tm;
	if (dc.GetTextMetrics(&tm))
		header_height_ = (tm.tmHeight + tm.tmExternalLeading) * IDEAL_HEIGHT / 13;
	//dc.SelectStockObject(DEFAULT_GUI_FONT);
	dc.SelectObject(&_font);
}


//static COLORREF CalcShade(COLORREF rgb_color, float shade)
//{
//	shade /= 100.0f;
//
//	int red= GetRValue(rgb_color);
//	int green= GetGValue(rgb_color);
//	int blue= GetBValue(rgb_color);
//
//	if (shade > 0.0f)	// lighter
//	{
//		return RGB(	min(255, int(red + shade * (0xff - red))),
//					min(255, int(green + shade * (0xff - green))),
//					min(255, int(blue + shade * (0xff - blue))));
//	}
//	else if (shade < 0.0f)	// darker
//	{
//		shade = 1.0f + shade;
//
//		return RGB(	min(255, int(red * shade)),
//					min(255, int(green * shade)),
//					min(255, int(blue * shade)));
//	}
//	else
//		return rgb_color;
//}


BOOL DlgListCtrl::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	SelectFont(*dc);

	dc->SetViewportOrg(-GetOffset());

	int width_wnd= std::max<int>(rect.Width(), range_size_.cx);

	COLORREF gray= ::GetSysColor(COLOR_3DFACE);

	if (top_margin_ > 0)
		dc->FillSolidRect(rect.left, 0, rect.Width(), top_margin_, gray);

	CPoint dlg_pos(0, top_margin_);
	const size_t count= dialogs_.size();
	for (size_t i= 0; i < count; ++i)
	{
		CRect header_rect(dlg_pos, CSize(rect.Width(), header_height_));
		dialogs_[i].DrawHeader(*dc, header_rect, width_wnd, left_margin_, right_margin_, image_list_, draw_checkboxes_);

		dlg_pos.y += header_height_;

		if (dialogs_[i].IsExpanded())
			dlg_pos.y += dialogs_[i].Height();
	}

	int extra= 1;
	if (!dialogs_.empty() && dialogs_.back().IsExpanded())
		extra = 0;

	dc->FillSolidRect(rect.left, dlg_pos.y + extra, rect.Width(), rect.Height(), gray);

	dc->SetViewportOrg(0, 0);

	//dc->SelectStockObject(DEFAULT_GUI_FONT);
	dc->SelectObject(&_font);
	return true;
}


bool DlgListCtrl::SubDlg::SetLocation(CPoint dlg_pos, int available_width, int height)
{
	if (dlg_ != 0 && dlg_->m_hWnd != 0)
	{
		int top= dlg_pos.y + TOP_SPACE;

		WINDOWPLACEMENT wp;
		dlg_->GetWindowPlacement(&wp);
		if (height == 0)
			height = Height();

		dlg_->SetWindowPos(0, dlg_pos.x, top, available_width, height - TOP_SPACE, SWP_NOZORDER | SWP_NOACTIVATE);

		return wp.rcNormalPosition.left != dlg_pos.x || wp.rcNormalPosition.top != top;
	}

	return false;
}


static void DrawShade(CDC& dc, CRect rect)
{
	static const short shades[]=
		{ 253, 252, 251, 250, 249, 248, 246, 245, 244, 242, 241, 236, 231, 228, 227, 228, 230, 231, 233, 235, 244 };

	int height= rect.Height();
	int width= rect.Width();

	if (height > 0 && width > 0)
	{
		for (int y= 0; y < height; ++y)
		{
			int index= (array_count(shades) - 1) * y / (height - 1);
			ASSERT(index >= 0 && index < array_count(shades));

			int gray= shades[index];
			dc.FillSolidRect(rect.left, rect.top + y, width, 1, RGB(gray, gray, gray));
		}
	}
}


void DlgListCtrl::SubDlg::DrawArrowAnimation(CDC& dc, CRect rect, bool closing_anim)
{
	int start= 0;
	int end= LAST_TRIANGLE;
	int step= 1;

	if (closing_anim)
	{
		std::swap(start, end);
		step = -1;
	}

	// limit background redrawing to the area beneath the triangle
	IMAGEINFO ii;
	if (DlgListCtrl::img_list_triangles_.GetImageInfo(0, &ii))
		rect.right = rect.left + TRIANGLE_LEFT_SPACE + ii.rcImage.right - ii.rcImage.left;

	for (int i= start; i != end; i += step)
	{
		DrawShade(dc, rect);
		DrawArrow(dc, rect, i, false);
		::Sleep(15);
	}
}


void DlgListCtrl::SubDlg::DrawArrow(CDC& dc, CRect& header_rect, int image, bool adjust_rect)
{
	CRect rect= header_rect;

	IMAGEINFO ii;
	if (DlgListCtrl::img_list_triangles_.GetImageInfo(0, &ii))
	{
		rect.left += TRIANGLE_LEFT_SPACE;
		// v-center image
		CPoint pos(rect.left, (rect.top + rect.bottom + ii.rcImage.top - ii.rcImage.bottom) / 2);

		DlgListCtrl::img_list_triangles_.Draw(&dc, image, pos, ILD_NORMAL);

		rect.left += ii.rcImage.right - ii.rcImage.left + TRIANGLE_RIGHT_SPACE;
	}

	if (adjust_rect)
		header_rect = rect;
}


void DlgListCtrl::SubDlg::DrawHeader(CDC& dc, CRect rect, int width_wnd, int left_margin, int right_margin, CImageList* image_list, bool draw_checkboxes)
{
	if (rect.Width() <= 0)
		return;

	COLORREF frame= RGB(156, 166, 186);

	if (left_margin > 0)
		dc.FillSolidRect(rect.left, rect.top, left_margin, rect.Height() + 1 + SHADOW_SPACE,
			::GetSysColor(COLOR_3DFACE));

	if (right_margin > 0)
		dc.FillSolidRect(rect.left + width_wnd - right_margin, rect.top, right_margin, rect.Height() + 1 + SHADOW_SPACE,
			::GetSysColor(COLOR_3DFACE));

	rect.DeflateRect(left_margin, 0, right_margin, 0);

	if (rect.Width() <= 0)
		return;

	CRect header= CRect(rect.TopLeft(), CSize(width_wnd - right_margin - left_margin, rect.Height() + 1));
	dc.Draw3dRect(header, frame, frame);

	CRect shadow= header;

	rect.top++;

	CRect back= header;
	back.DeflateRect(1, 1);	// take out space used by outline
	DrawShade(dc, back);

	DrawArrow(dc, header, expanded_ ? LAST_TRIANGLE : 0, true);

	rect.left = header.left;

	if (rect.Width() > 0)
	{
		if (draw_checkboxes)
		{
			CRect r(rect.left, rect.top, rect.left + rect.Height(), rect.bottom);
			r.DeflateRect(2, 3, 2, 2);
			CtrlDraw::DrawCheckBox(dc, r, expanded_ ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL);
			rect.left = r.right + r.Width() / 6;
		}

		if (rect.Width() > 0)
		{
			dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
			dc.SetBkColor(RGB(0xff,0xff,0xff));
			dc.SetBkMode(TRANSPARENT);
			dc.DrawText(item_.data(), static_cast<int>(item_.size()), rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_NOPREFIX);
		}

		if (rect.Width() > 0 && image_index_ >= 0 && image_list != 0)
		{
			IMAGEINFO ii;
			if (image_list->GetImageInfo(image_index_, &ii))
			{
				CRect r= ii.rcImage;
				int x= rect.right - (r.Width() * 13 / 10);
				// v-center image
				CPoint pos(x, (rect.top + rect.bottom + ii.rcImage.top - ii.rcImage.bottom) / 2);

				image_list->Draw(&dc, image_index_, pos, ILD_NORMAL | ILD_TRANSPARENT);

				rect.left += ii.rcImage.right + 8;
			}
		}
	}

	// draw shadow
	try
	{
		shadow.top = rect.bottom + 1;
		shadow.bottom = shadow.top + SHADOW_SPACE;

		COLORREF back= ::GetSysColor(COLOR_3DFACE);

		extern void DrawShadow(Dib& dib, CRect rect, const Dib& shadow, const int OPQ);

		Dib bmp(shadow.Width(), shadow.Height(), 32);
		CDC mem;
		mem.CreateCompatibleDC(&dc);

		CBitmap* old= mem.SelectObject(bmp.GetBmp());

		CRect r(CPoint(0, 0), shadow.Size());

		mem.FillSolidRect(r, back);

		const float opacity= 0.20f;

		CRect bar= r;
		bar.DeflateRect(SHADOW_SPACE, 0);
		::DrawBlackGradient(bmp, bar, opacity, 2);

		::DrawBlackCorner(bmp, CRect(CPoint(r.left, r.top), CSize(SHADOW_SPACE, SHADOW_SPACE)), opacity, -2);
		::DrawBlackCorner(bmp, CRect(CPoint(r.right - SHADOW_SPACE, r.top), CSize(SHADOW_SPACE, SHADOW_SPACE)), opacity, 2);

		dc.BitBlt(shadow.left, shadow.top, shadow.Width(), shadow.Height(), &mem, 0, 0, SRCCOPY);

		mem.SelectObject(old);
	}
	catch (...)
	{}
}


/////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

typedef struct
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX;

typedef struct
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;
} DLGITEMTEMPLATEEX;

#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////

DlgListCtrl::SubDlg::SubDlg(CWnd* dlg, const TCHAR* title, bool expanded, int image_index, CWnd* parent)
  : dlg_(dlg), expanded_(expanded)
{
	is_resizable_ = false;
	image_index_ = image_index;
	dlg_rect_.SetRectEmpty();

	if (dlg->m_hWnd)
	{
		ASSERT(dlg->GetParent() == parent);
		// dlg already created
	}
	else
		if (!Create(dynamic_cast<CDialog*>(dlg), parent))
			throw std::exception("Subdialog creation failed in dlg list control");

	if (dlg->m_hWnd)
	{
		dlg->GetClientRect(dlg_rect_);

		if (expanded_)
			dlg->ShowWindow(SW_SHOWNA);
	}

	if (title)
		item_ = title;
}


bool DlgListCtrl::SubDlg::Create(CDialog* dlg, CWnd* parent)
{
	if (dlg == 0 || dlg->m_hWnd != 0)
	{
		ASSERT(false);
		return false;
	}

	struct xCDialog : public CDialog
	{
		LPCTSTR Id() const	{ return m_lpszTemplateName; }
	};

	LPCTSTR template_name= static_cast<xCDialog*>(dlg)->Id();
	ASSERT(template_name != NULL);

	HINSTANCE inst= AfxFindResourceHandle(template_name, RT_DIALOG);
	HRSRC resource= ::FindResource(inst, template_name, RT_DIALOG);
	HGLOBAL dialog_template_handle= LoadResource(inst, resource);

	if (dialog_template_handle == NULL)
	{
		ASSERT(false);
		return false;
	}

	DWORD size= ::SizeofResource(inst, resource);
	void* dialog_template= ::LockResource(dialog_template_handle);

	std::vector<BYTE> a_template;
	a_template.resize(size);
	DLGTEMPLATEEX* dlg_template= reinterpret_cast<DLGTEMPLATEEX*>(&a_template.front());
	memcpy(dlg_template, dialog_template, size);

	// DLGTEMPLATEEX expected (DIALOGEX resource)
	ASSERT(dlg_template->dlgVer == 1 && dlg_template->signature == 0xffff);

	dlg_template->style &= ~(WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | WS_THICKFRAME | WS_VISIBLE);
	dlg_template->style |= WS_CHILD;
	is_resizable_ = (dlg_template->style & WS_THICKFRAME) != 0;
	dlg_template->exStyle |= WS_EX_CONTROLPARENT;

	if (!dlg->CreateIndirect(dlg_template, parent))
	{
		ASSERT(false);
		return false;
	}

	CString title;
	dlg->GetWindowText(title);
	item_ = title;

	return true;
}


void DlgListCtrl::AddSubDialog(CDialog* dlg, int image_index, const TCHAR* title, bool expanded)
{
	AddSubDialog(static_cast<CWnd*>(dlg), image_index, title, expanded, CSize(0, 0));
}


void DlgListCtrl::AddSubDialog(CWnd* dlg, int image_index, const TCHAR* title, bool expanded, CSize min_dim_size)
{
	ASSERT(m_hWnd != 0);

	if (dlg == 0)
	{
		ASSERT(false);
		return;
	}

	dialogs_.push_back(SubDlg(dlg, title, expanded, image_index, this));

	CRect& rect= dialogs_.back().dlg_rect_;

	if (min_dim_size.cx > 0)
		rect.right = rect.left + min_dim_size.cx;
	if (min_dim_size.cy > 0)
		rect.bottom = rect.top + min_dim_size.cy;

	ResizeSubDialogs();
}


void DlgListCtrl::AddSubDialog(CDialog* dlg, const TCHAR* title/*= 0*/, bool expanded/*= true*/)
{
	AddSubDialog(dlg, -1, title, expanded);
}


CSize DlgListCtrl::GetTotalSize() const
{
	CSize size(0, top_margin_);

	for (std::vector<SubDlg>::const_iterator it= dialogs_.begin(); it != dialogs_.end(); ++it)
	{
		size.cy += header_height_;

		if (it->IsExpanded())
		{
			int width= it->Width();
			if (width > size.cx)
				size.cx = width;

			size.cy += it->Height();
		}

		size.cx; // TODO: modify depending on width of the subdlg title string and toolbar
	}

	if (!dialogs_.empty() && !dialogs_.back().IsExpanded())
		size.cy++;

	size.cy += bottom_margin_;

	return size;
}


void DlgListCtrl::ResizeSubDialogs(bool shifting_down/*= false*/)
{
	range_size_ = GetTotalSize();

	SetScrollBar();

	if (dialogs_.empty())
		return;

	CRect rect;
	GetClientRect(rect);
	ASSERT(rect.top == 0);

	rect.OffsetRect(-GetOffset());

	CPoint dlg_pos= rect.TopLeft();
	dlg_pos.y += top_margin_;
	const size_t count= dialogs_.size();

	if (!shifting_down)
	{
		for (size_t i= 0; i < count; ++i)
		{
			SubDlg& dlg= dialogs_[i];

			dlg_pos.y += header_height_;

			if (dlg.IsExpanded())
			{
				int height= dlg.Height();

				if (i == count - 1 && extend_last_dlg_to_whole_wnd_)
					height = std::max(TOP_SPACE + dlg.Height(), rect.Height() - dlg_pos.y);

				if (dlg.SetLocation(dlg_pos, std::max<int>(range_size_.cx, rect.Width()), height) && dlg.dlg_)
					dlg.dlg_->Invalidate();

				dlg_pos.y += height;
			}
		}
	}
	else
	{
		std::vector<std::pair<int, int>> heights(count);

		for (size_t i= 0; i < count; ++i)
		{
			SubDlg& dlg= dialogs_[i];

			dlg_pos.y += header_height_;

			heights[i].first = dlg_pos.y;
			heights[i].second = 0;

			if (dlg.IsExpanded())
			{
				int height= dlg.Height();

				if (i == count - 1 && extend_last_dlg_to_whole_wnd_)
					height = std::max(dlg.Height(), rect.Height() - dlg_pos.y);

				dlg_pos.y += height;
				heights[i].second = height;
			}
		}

		dlg_pos.y = rect.top;

		ASSERT(count > 0);
		for (size_t i= count - 1; ; --i)
		{
			SubDlg& dlg= dialogs_[i];

			dlg_pos.y = heights[i].first;

			if (dlg.IsExpanded())
			{
				if (dlg.SetLocation(dlg_pos, std::max<int>(range_size_.cx, rect.Width()), heights[i].second) && dlg.dlg_)
					dlg.dlg_->Invalidate();
			}

			if (i == 0)
				break;
		}
	}
}


void DlgListCtrl::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	OnScroll(MAKEWORD(sb_code, -1), pos);
}


void DlgListCtrl::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	OnScroll(MAKEWORD(-1, sb_code), pos);
}


bool DlgListCtrl::OnScroll(UINT scroll_code, UINT pos, bool do_scroll/*= true*/)
{
	CRect rect;
	GetClientRect(rect);
	CSize line_size(30, 30);

	// calc new x position
	int x= GetScrollPos(SB_HORZ);
	int xOrig= x;

	switch (LOBYTE(scroll_code))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= line_size.cx;
		break;
	case SB_LINEDOWN:
		x += line_size.cx;
		break;
	case SB_PAGEUP:
		x -= rect.Width();
		break;
	case SB_PAGEDOWN:
		x += rect.Height();
		break;
	case SB_THUMBTRACK:
		x = pos;
		break;
	}

	// calc new y position
	int y= GetScrollPos(SB_VERT);
	int yOrig= y;

	switch (HIBYTE(scroll_code))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= line_size.cy;
		break;
	case SB_LINEDOWN:
		y += line_size.cy;
		break;
	case SB_PAGEUP:
		y -= rect.Height();
		break;
	case SB_PAGEDOWN:
		y += rect.Height();
		break;
	case SB_THUMBTRACK:
		y = pos;
		break;
	}

	bool result= OnScrollBy(CSize(x - xOrig, y - yOrig), do_scroll);
	if (result && do_scroll)
		UpdateWindow();

	return result;
}


bool DlgListCtrl::OnScrollBy(CSize scroll_size, bool do_scroll)
{
	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	DWORD dwStyle= GetStyle();
	CScrollBar* bar= GetScrollBarCtrl(SB_VERT);
	if ((bar != NULL && !bar->IsWindowEnabled()) || (bar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		scroll_size.cy = 0;
	}
	bar = GetScrollBarCtrl(SB_HORZ);
	if ((bar != NULL && !bar->IsWindowEnabled()) || (bar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		scroll_size.cx = 0;
	}

	// adjust current x position
	int xOrig= GetScrollPos(SB_HORZ);
	int x= xOrig;
	int xMax= GetScrollLimit(SB_HORZ);
	x += scroll_size.cx;
	if (x < 0)
		x = 0;
	else if (x > xMax)
		x = xMax;

	// adjust current y position
	int yOrig= GetScrollPos(SB_VERT);
	int y= yOrig;
	int yMax= GetScrollLimit(SB_VERT);
	y += scroll_size.cy;
	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig)
		return false;

	if (do_scroll)
	{
		// do scroll and update scroll positions
		ScrollWindow(-(x-xOrig), -(y-yOrig));
		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);
		if (y != yOrig)
			SetScrollPos(SB_VERT, y);
	}

	return true;
}


void DlgListCtrl::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	ResizeSubDialogs();
}


void DlgListCtrl::SetScrollBar()
{
	CRect rect;
	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return;

	DWORD dwStyle= GetStyle();
	if (dwStyle & WS_VSCROLL)
		rect.right += scroll_bars_size_.cx;
	if (dwStyle & WS_HSCROLL)
		rect.bottom += scroll_bars_size_.cy;

	if (rect.Width() >= range_size_.cx && rect.Height() >= range_size_.cy)
	{
		// enough space to hide both scrollbars

		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 1;
		si.nPage = 2;
		SetScrollInfo(SB_HORZ, &si);
		SetScrollInfo(SB_VERT, &si);
		// scrollbars are hidden now
	}
	else
	{
		// at least one or both scrollbars have to be displayed

		if (rect.Width() < range_size_.cx)
			rect.bottom -= scroll_bars_size_.cy;
		if (rect.Height() < range_size_.cy)
			rect.right -= scroll_bars_size_.cx;

		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = range_size_.cx - 1;
		si.nPage = rect.Width();
		SetScrollInfo(SB_HORZ, &si);

		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = range_size_.cy - 1;
		si.nPage = rect.Height();
		SetScrollInfo(SB_VERT, &si);
	}
}


void DlgListCtrl::OnLButtonDown(UINT flags, CPoint pos)
{
	pos += GetOffset();
	int header_index= HitText(pos);

	if (header_index >= 0)
		ToggleSubDialog(header_index);
}

void DlgListCtrl::OnLButtonUp(UINT flags, CPoint pos)
{}


CRect DlgListCtrl::GetHeaderRect(int index)
{
	CRect cl_rect;
	GetClientRect(cl_rect);

	CRect rect(CPoint(0, top_margin_), CSize(std::max<long>(cl_rect.Width(), range_size_.cx), header_height_));

	size_t limit= static_cast<size_t>(index);

	if (limit <= dialogs_.size())
	{
		for (size_t i= 0; i < limit; ++i)
		{
			int offset= header_height_;

			if (dialogs_[i].IsExpanded())
				offset += dialogs_[i].Height();

			rect.OffsetRect(0, offset);
		}
	}

	rect.DeflateRect(left_margin_, 0, right_margin_, 0);

	return rect;
}


int DlgListCtrl::HitText(CPoint pos)
{
	CRect cl_rect;
	GetClientRect(cl_rect);

	CRect rect(CPoint(0, top_margin_), CSize(std::max<long>(cl_rect.Width(), range_size_.cx), header_height_));
	rect.DeflateRect(left_margin_, 0, right_margin_, 0);

	const size_t count= dialogs_.size();

	for (size_t i= 0; i < count; ++i)
	{
		if (rect.PtInRect(pos))
			return static_cast<int>(i);

		int offset= header_height_;

		if (dialogs_[i].IsExpanded())
			offset += dialogs_[i].Height();

		rect.OffsetRect(0, offset);
	}

	return -1;
}


COLORREF DlgListCtrl::GetBackgndColor()
{
	return CalcShade(::GetSysColor(COLOR_3DFACE), 80.0f);
}


static bool IsCtrlWindowVisible(HWND wnd)
{
	if (!::IsWindowVisible(wnd))
		return false;

	wnd = ::GetParent(wnd);
	if (wnd == 0)
		return true;

	return IsCtrlWindowVisible(wnd);
}


void DlgListCtrl::ToggleSubDialog(int index)
{
	if (static_cast<size_t>(index) < dialogs_.size())
	{
		CWnd* focus= GetFocus();

		SubDlg& dlg= dialogs_[index];

		CClientDC dc(this);
		dc.SetViewportOrg(-GetOffset());

		//img_list_triangles_.SetBkColor(GetBackgndColor());

		CRect area= GetHeaderRect(index);
		area.DeflateRect(1, 1, 1, 0);	// take out frame
		dlg.DrawArrowAnimation(dc, area, dlg.IsExpanded());

		dc.SetViewportOrg(0, 0);

		bool visible= dlg.Toggle();

		if (single_expand_)
		{
			// hide all dialogs
			const size_t count= dialogs_.size();
			for (size_t i= 0; i < count; ++i)
				if (i != index)
					dialogs_[i].Hide();
		}

		Invalidate();

		ResizeSubDialogs(visible);

		if (visible)
			dlg.ShowDlg();

		// check focus
//		if (focus && !IsCtrlWindowVisible(*focus))
		MoveFocusToVisibleWindow(visible ? dlg.dlg_ : this);

		// anti-locking measure against MFC's buggy _AfxNextControl
		{
			size_t limit= dialogs_.size();
			bool hidden= true; // whole DlgListCtrl
			for (size_t i= 0; i < limit; ++i)
				if (dialogs_[i].IsExpanded())
				{
					hidden = false;
					break;
				}

			if (hidden)
				ModifyStyleEx(WS_EX_CONTROLPARENT, 0);
			else
				ModifyStyleEx(0, WS_EX_CONTROLPARENT);
		}
	}
}


void DlgListCtrl::SubDlg::Hide()
{
	if (expanded_)
	{
		if (dlg_)
			dlg_->ShowWindow(SW_HIDE);
		expanded_ = false;
	}
}


void DlgListCtrl::SubDlg::ShowDlg()
{
	if (dlg_)
		dlg_->ShowWindow(SW_SHOWNA);
}


bool DlgListCtrl::SubDlg::Expand(bool expand)
{
	if (expanded_ == expand)
		return false;

	if (expanded_)
	{
		expanded_ = false;

		if (dlg_)
			dlg_->ShowWindow(SW_HIDE);
	}
	else
	{
		expanded_ = true;

		if (dlg_)
			dlg_->ShowWindow(SW_SHOWNA);
	}

	return true;
}


bool DlgListCtrl::SubDlg::Toggle()
{
	if (expanded_)
	{
		if (dlg_)
			dlg_->ShowWindow(SW_HIDE);
		expanded_ = false;
	}
	else
	{
		expanded_ = true;
	}

	return expanded_;
}


BOOL DlgListCtrl::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
	UINT code= delta > 0 ? SB_LINEUP : SB_LINEDOWN;

	DWORD dwStyle= GetStyle();

	if (dwStyle & WS_VSCROLL)
		OnVScroll(code, 0, 0);
	else if (dwStyle & WS_HSCROLL)
		OnHScroll(code, 0, 0);

	return true;
}


void DlgListCtrl::MoveFocusToVisibleWindow(CWnd* start)
{
	CWnd* focus= 0;

	CWnd* wnd= start ? start : this;

	for (int n= 0; n < 5 && wnd != 0; ++n)	// 5--max depth
	{
		focus = FindFirstTabCtrl(wnd);
		if (focus != 0)
			break;

		wnd = wnd->GetParent();
	}

	if (focus)
	{
		//PostMessage(WM_NEXTDLGCTL, WPARAM(focus->m_hWnd), 1L);
		SendMessage(WM_NEXTDLGCTL, WPARAM(focus->m_hWnd), 1L);
		focus->SetFocus();
	}
	else
		ASSERT(false);	// dire straits--no widow to set focus found; dlg can hang the app when next key down msg arrives
}


bool DlgListCtrl::IsSubDialogExpanded(size_t index)
{
	if (index < dialogs_.size())
		return dialogs_[index].IsExpanded();

	ASSERT(false);
	return false;
}


void DlgListCtrl::ExpandSubDialog(size_t index, bool expand)
{
	if (index < dialogs_.size())
	{
		SubDlg& dlg= dialogs_[index];

		if (dlg.Expand(expand))
		{
			Invalidate();
			ResizeSubDialogs(expand);
		}

		return;
	}

	ASSERT(false);
}


void DlgListCtrl::OnSetFocus(CWnd* focus)
{
	if (focus == this)
		MoveFocusToVisibleWindow();
}

UINT DlgListCtrl::OnGetDlgCode()
{
	return DLGC_STATIC;
}


void DlgListCtrl::SetSingleExpand(bool enable)
{
	single_expand_ = enable;
}


void DlgListCtrl::EnsureVisible(CWnd* dialog, CRect rect)
{
	CRect view(0,0,0,0);
	GetClientRect(view);

	dialog->MapWindowPoints(this, rect);

	int scroll= 0;

	if (rect.top < view.top)
	{
		scroll = rect.top - view.top;
		// scroll down
	}
	else if (rect.bottom > view.bottom)
	{
		scroll = rect.bottom - view.bottom + 4;	// plus extra space to have some additional distance from the bottom
		// scroll up
	}

	if (scroll)
	{
//TRACE(L"scroll: %d  is: %d\n", scroll, GetScrollPos(SB_VERT));
		if (OnScrollBy(CSize(0, scroll), true))
			UpdateWindow();
	}
}


size_t DlgListCtrl::GetCount() const
{
	return dialogs_.size();
}


void DlgListCtrl::DrawCheckboxes(bool enable)
{
	draw_checkboxes_ = enable;

	Invalidate();
}


void DlgListCtrl::SetSubdialogHeight(CWnd* dlg, int height)
{
	for (std::vector<SubDlg>::iterator it= dialogs_.begin(); it != dialogs_.end(); ++it)
	{
		if (it->dlg_ == dlg)
		{
			// resize it

			bool grow= height > it->Height();
			it->dlg_rect_.bottom = it->dlg_rect_.top + height;
			Invalidate();
			ResizeSubDialogs(grow);

			break;
		}
	}
}


int DlgListCtrl::GetSubdialogHeight(size_t dlg_index) const
{
	if (dlg_index < GetCount())
		return dialogs_[dlg_index].Height();

	return 0;
}


void DlgListCtrl::SetSubdialogImage(CWnd* dlg, int image_index)
{
	for (std::vector<SubDlg>::iterator it= dialogs_.begin(); it != dialogs_.end(); ++it)
	{
		if (it->dlg_ == dlg)
		{
			// update image
			it->image_index_ = image_index;
			Invalidate();
			break;
		}
	}
}
