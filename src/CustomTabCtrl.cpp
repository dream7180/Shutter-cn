/**********************************************************************
**
**	CustomTabCtrl.cpp : implementation file of CustomTabCtrl class
**
**	by Andrzej Markowski June 2004
**
**********************************************************************/

#include "stdafx.h"
#include <AFXPRIV.H>
#include "CustomTabCtrl.h"
#include <math.h>
#include "UIElements.h"

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif


#define TAB_SHAPE1		0		//  Invisible

#define TAB_SHAPE2		1		//	 __
								//	| /
								//	|/

#define TAB_SHAPE3		2		//	|\
								//	|/

#define TAB_SHAPE4		3		//	____________
								//	\          /
								//   \________/

#define TAB_SHAPE5		4		//	___________
								//	\          \
								//	  \________/

#define RECALC_PREV_PRESSED			0
#define RECALC_NEXT_PRESSED			1
#define RECALC_ITEM_SELECTED		2
#define RECALC_RESIZED				3
#define RECALC_FIRST_PRESSED		4
#define RECALC_LAST_PRESSED			5
#define RECALC_EDIT_RESIZED			6
#define RECALC_CLOSE_PRESSED		7


extern COLORREF CalcColor(COLORREF rgb_color1, COLORREF rgb_color2, float bias);

COLORREF GetLineColor()
{
	COLORREF gray= ::GetSysColor(COLOR_3DFACE);
//	COLORREF light= ::GetSysColor(COLOR_3DHIGHLIGHT);
	COLORREF dark= ::GetSysColor(COLOR_3DSHADOW);
	return CalcColor(dark, gray, 0.6f);
}

// CCustomTabCtrlItem

CCustomTabCtrlItem::CCustomTabCtrlItem(CString text, LPARAM lParam)
 : text_(text), param_(lParam), shape_(TAB_SHAPE1), selected_(false), highlighted_(false), highlight_changed_(false)
{
}


void CCustomTabCtrlItem::operator = (const CCustomTabCtrlItem& other)
{
	text_ = other.text_;
	param_ = other.param_;
}


void CCustomTabCtrlItem::ComputeRgn(bool on_top)
{
	rgn_.DeleteObject();

	CPoint pts[6];
	GetRegionPoints(rect_, pts, on_top);
	rgn_.CreatePolygonRgn(pts, 6, WINDING);
}


void CCustomTabCtrlItem::GetRegionPoints(const CRect& rc, CPoint* pts, bool on_top) const
{
	switch (shape_)
	{
	case TAB_SHAPE2:
		{
			if (on_top)
			{
				pts[0] = CPoint(rc.left, rc.bottom+1);
				pts[1] = CPoint(rc.left, rc.top);
				pts[2] = CPoint(rc.left + rc.Height()/2, rc.bottom+1);
				pts[3] = CPoint(rc.left + rc.Height()/2, rc.bottom+1);
				pts[4] = CPoint(rc.left + rc.Height()/2, rc.bottom+1);
				pts[5] = CPoint(rc.left + rc.Height()/2, rc.bottom+1);
			}
			else
			{
				pts[0] = rc.TopLeft();
				pts[1] = CPoint(rc.left, rc.bottom);
				pts[2] = CPoint(rc.left + rc.Height()/2, rc.top);
				pts[3] = CPoint(rc.left + rc.Height()/2, rc.top);
				pts[4] = CPoint(rc.left + rc.Height()/2, rc.top);
				pts[5] = CPoint(rc.left + rc.Height()/2, rc.top);
			}
		}
		break;
	case TAB_SHAPE3:
		{
			if (on_top)
			{
				pts[0] = CPoint(rc.left, rc.bottom+1);
				pts[1] = CPoint(rc.left + rc.Height()/4, rc.Height()/2);
				pts[2] = CPoint(rc.left, rc.top);
				pts[3] = CPoint(rc.left, rc.top);
				pts[4] = CPoint(rc.left, rc.top);
				pts[5] = CPoint(rc.left, rc.top);
			}
			else
			{
				pts[0] = rc.TopLeft();
				pts[1] = CPoint(rc.left + rc.Height()/4, rc.Height()/2);
				pts[2] = CPoint(rc.left, rc.bottom);
				pts[3] = CPoint(rc.left, rc.bottom);
				pts[4] = CPoint(rc.left, rc.bottom);
				pts[5] = CPoint(rc.left, rc.bottom);
			}
		}
		break;
	case TAB_SHAPE4:
		{
			if (on_top)
			{
				pts[0] = CPoint(rc.left, rc.bottom+1);
				pts[1] = CPoint(rc.left + rc.Height()/4, rc.Height()/2);
				pts[2] = CPoint(rc.left + rc.Height()/2, rc.top);
				pts[3] = CPoint(rc.right - rc.Height()/2, rc.top);
				pts[4] = CPoint(rc.right - rc.Height()/4, rc.Height()/2);
				pts[5] = CPoint(rc.right, rc.bottom+1);
			}
			else
			{
				pts[0] = rc.TopLeft();
				pts[1] = CPoint(rc.left + rc.Height()/4, rc.Height()/2);
				pts[2] = CPoint(rc.left + rc.Height()/2, rc.bottom);
				pts[3] = CPoint(rc.right - rc.Height()/2, rc.bottom);
				pts[4] = CPoint(rc.right - rc.Height()/4, rc.Height()/2);
				pts[5] = CPoint(rc.right, rc.top);
			}
		}
		break;
	case TAB_SHAPE5:
		{
			if (on_top)
			{
				pts[0] = CPoint(rc.left, rc.bottom+1);
				pts[1] = CPoint(rc.left + rc.Height()/4, rc.Height()/2);
				pts[2] = CPoint(rc.left + rc.Height()/2 , rc.top);
				pts[3] = CPoint(rc.right - rc.Height()/2, rc.top);
				pts[4] = CPoint(rc.right - rc.Height()/4, rc.Height()/2);
				pts[5] = CPoint(rc.right - rc.Height()/2, rc.bottom+1);
			}
			else
			{
				pts[0] = rc.TopLeft();
				pts[1] = CPoint(rc.left + rc.Height()/4, rc.Height()/2);
				pts[2] = CPoint(rc.left + rc.Height()/2 , rc.bottom);
				pts[3] = CPoint(rc.right - rc.Height()/2, rc.bottom);
				pts[4] = CPoint(rc.right - rc.Height()/4, rc.Height()/2);
				pts[5] = CPoint(rc.right - rc.Height()/2, rc.top);
			}
		}
		break;
	default:
		{
			pts[0] = CPoint(0, 0);
			pts[1] = CPoint(0, 0);
			pts[2] = CPoint(0, 0);
			pts[3] = CPoint(0, 0);
			pts[4] = CPoint(0, 0);
			pts[5] = CPoint(0, 0);
		}
		break;
	}
}

void CCustomTabCtrlItem::GetDrawPoints(const CRect& rc, CPoint* pts, bool on_top) const
{
	switch (shape_)
	{
	case TAB_SHAPE2:
	case TAB_SHAPE3:
		{
			if (on_top)
			{
				pts[0] = CPoint(rc.left, rc.top);
				pts[1] = CPoint(rc.left + rc.Height()/2, rc.bottom);
			}
			else
			{
				pts[0] = CPoint(rc.left, rc.bottom);
				pts[1] = CPoint(rc.left + rc.Height()/2, rc.top);
			}
		}
		break;
	case TAB_SHAPE4:
	case TAB_SHAPE5:
		{
			if (on_top)
			{
				pts[0] = CPoint(rc.left, rc.bottom);
				pts[1] = CPoint(rc.left + rc.Height()/2, rc.top);
				pts[2] = CPoint(rc.right - rc.Height()/2, rc.top);
				pts[3] = CPoint(rc.right, rc.bottom);
			}
			else
			{
				pts[0] = rc.TopLeft();
				pts[1] = CPoint(rc.left + rc.Height()/2, rc.bottom);
				pts[2] = CPoint(rc.right - rc.Height()/2, rc.bottom);
				pts[3] = CPoint(rc.right, rc.top);
			}
		}
		break;
	}

}


void CreateTabShape(Gdiplus::GraphicsPath& tab, float x, float y, float w, float h)
{
	tab.StartFigure();

	// tabs have sides at an angle: \___/
	float slope= h / 4.0f;
	// corners are rounded; diameter for AddArc methods
	float diameter= h / 1.8f;
	// side is angled; calc the angle to draw proper arcs
	float angle= atan2(slope, h - diameter / 2.0f) / M_PI * 180.0f;
	float sweep_angle= -(90.0f - angle);

	float y_arcs= y + h - diameter;

	tab.AddArc(x + slope, y_arcs, diameter, diameter, 180.0f - angle, sweep_angle);
	tab.AddArc(x + w - slope - diameter, y_arcs, diameter, diameter, 90.0f, sweep_angle);
	tab.AddLine(x + w, y, x, y);
	tab.CloseFigure();
}


void CCustomTabCtrlItem::Draw(CDC& dc, CFont& font, bool selected, bool on_top, bool RTL, int selection_tinge, COLORREF tab_color, COLORREF text_color)
{
	COLORREF bgColor= GetSysColor(selected_ || highlighted_ ? COLOR_WINDOW     : COLOR_3DFACE);
	//COLORREF fgColor= GetSysColor(selected_ || highlighted_ ? COLOR_WINDOWTEXT : COLOR_BTNTEXT);

	CBrush brush(bgColor);

	CPen blackPen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));

	COLORREF gray= ::GetSysColor(COLOR_3DFACE);
	COLORREF dark= ::GetSysColor(COLOR_3DSHADOW);
	CPen shadowPen(PS_SOLID, 1, CalcColor(dark, gray, 0.25f));

	dc.SetBkMode(OPAQUE);

	CPoint pts[4];
	CRect rc = rect_;

	// shift selected tab, so it stands out
	int offset= -1;
	if (!selected)
		rc.OffsetRect(0, offset);

	GetDrawPoints(rc, pts, on_top);

	CPen* old_pen = dc.SelectObject(&blackPen);

	// draw item
	switch (shape_)
	{
	case TAB_SHAPE2:
	case TAB_SHAPE3:
		{
			// Paint item background
			dc.FillRgn(&rgn_, &brush);

			dc.MoveTo(pts[0]);
			dc.LineTo(pts[1]);

			if (!selected_)
			{
				dc.SelectObject(&shadowPen);
				dc.MoveTo(pts[0].x - 1, pts[0].y);
				dc.LineTo(pts[1].x - 1, pts[1].y);
			}
		}
		break;
	case TAB_SHAPE4:
	case TAB_SHAPE5:
		{
			//int i= 0, h= pts[3].y - pts[2].y;
			//COLORREF gray= selected_ ? ::GetSysColor(COLOR_3DHIGHLIGHT) :
			//	CalcColor(::GetSysColor(COLOR_3DFACE), ::GetSysColor(COLOR_3DSHADOW), 0.95f);
			//COLORREF dark= CalcColor(::GetSysColor(selected_ ? COLOR_3DFACE : COLOR_3DSHADOW), gray, 0.3f);
			//if (selection_tinge > 0)
			//{
			//	float ratio= float(selection_tinge) / 100.0f;
			//	gray = CalcColor(::GetSysColor(COLOR_HIGHLIGHT), gray, ratio / 2.0f);
			//	dark = CalcColor(::GetSysColor(COLOR_HIGHLIGHT), dark, ratio);
			//}
			//int x= pts[1].x, w= pts[2].x - pts[1].x;
			//for (int y= pts[2].y + 1; y <= pts[3].y; ++y, ++i)
			//{
			//	float r= float(i) / h;
			//	COLORREF c= CalcColor(gray, dark, 1.0f - r * r);
			//	if (i == 0 && !selected_)	// highlight at the top
			//		c = CalcColor(::GetSysColor(COLOR_3DHIGHLIGHT), ::GetSysColor(COLOR_3DFACE), 0.4f);
			//	dc.FillSolidRect(x, y, w, 1, c);
			//	if (i & 1)
			//		x--, w += 2;
			//}

			{
				float x= pts[0].x, y= pts[0].y;
				float w= pts[3].x - x, h= pts[1].y - y;
				x += 2; w -= 4;
				Gdiplus::Graphics g(dc);
				g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
				g.TranslateTransform(-0.5f, -0.5f);

	//			COLORREF tab_color= tab_color_;
				if (selection_tinge > 0)
				{
					float ratio= float(selection_tinge) / 100.0f;
					tab_color = CalcColor(RGB(247, 123, 0)/*::GetSysColor(COLOR_HIGHLIGHT)*/, tab_color, ratio);
				}

				Gdiplus::SolidBrush tab_brush(c2c(tab_color));
				{
					if (selected)
					{
						Gdiplus::GraphicsPath tab;
						CreateTabShape(tab, x, y, w + 1, h);

						// draw shadow below selected tab
						Gdiplus::SolidBrush shadow(Gdiplus::Color(7, 0, 0, 0));
						int depth= 3;	// pixels
						g.TranslateTransform(0.0f, static_cast<float>(depth));
						for (int i= 0; i < depth; ++i)
						{
							g.FillPath(&shadow, &tab);
							g.TranslateTransform(0.0f, -1.0f);
						}

						// erase separator at the top, directly above selected tab to make it flush with area above
						g.FillRectangle(&tab_brush, x + 1, y + offset, w - 2, static_cast<float>(abs(offset)));
					}

					// this will become tab outline
					Gdiplus::GraphicsPath tab;
					CreateTabShape(tab, x, y, w, h);
					COLORREF dark= GetLineColor();
					Gdiplus::SolidBrush brush(c2c(dark));
					g.FillPath(&brush, &tab);
				}
				// fill the tab
				{
					Gdiplus::GraphicsPath tab;
					CreateTabShape(tab, x + 1, y, w - 2, h - 1);
					g.FillPath(&tab_brush, &tab);
				}
			}
//			if (!selected_)
//			{
//				dc.SelectObject(&shadowPen);
//				dc.MoveTo(pts[2].x - 1, pts[2].y);
//				dc.LineTo(pts[3].x - 1, pts[3].y);
//			}
//			dc.SelectObject(&blackPen);
//
//			dc.MoveTo(pts[0]);
//			dc.LineTo(pts[1]);
//			
////			dc.SelectObject(&shadowPen);
//			dc.LineTo(pts[2]);
//			
//			dc.MoveTo(pts[2]);
//			dc.LineTo(pts[3]);

			// draw item text

			COLORREF bgOldColor = dc.SetBkColor(bgColor);
			COLORREF fgOldColor = dc.SetTextColor(text_color);
			rc.DeflateRect(rc.Height()/2, 2, rc.Height()/2, 2);
			CFont* old_font = dc.SelectObject(&font);
			dc.SetBkMode(TRANSPARENT);
			if (RTL)
				dc.DrawText(text_, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_RTLREADING);
			else 
				dc.DrawText(text_, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
			dc.SelectObject(old_font);
			dc.SetTextColor(fgOldColor);
			dc.SetBkColor(bgOldColor);
		}
		break;
	}
	dc.SelectObject(old_pen);
}

// CustomTabCtrl

LOGFONT CustomTabCtrl::lf_default = {-Pixels(11), 0, 0, 0, FW_NORMAL, 0, 0, 0,
			DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Microsoft Sans Serif")};

BYTE CustomTabCtrl::bits_glyphs_[] = {
										0xBD, 0xFB, 0xDF, 0xBD, 0x3C, 0x00,
										0xB9, 0xF3, 0xCF, 0x9D, 0x99, 0x00,
										0xB1, 0xE3, 0xC7, 0x8D, 0xC3, 0x00,
										0xA3, 0xC7, 0xE3, 0xC5, 0xE7, 0x00,
										0xB1, 0xE3, 0xC7, 0x8D, 0xC3, 0x00,
										0xB9, 0xF3, 0xCF, 0x9D, 0x99, 0x00,
										0xBD, 0xFB, 0xDF, 0xBD, 0x3C, 0x00
										};


CustomTabCtrl::CustomTabCtrl() :
			button_id_down_(CTCID_NOBUTTON),
			prev_state_(BNST_INVISIBLE),
			next_state_(BNST_INVISIBLE),
			first_state_(BNST_INVISIBLE),
			last_state_(BNST_INVISIBLE),
			close_state_(BNST_INVISIBLE),
			item_selected_(-1),
			item_ndx_offset_(0),
			last_repeat_time_(0),
			bmp_bk_left_spin_(NULL),
			bmp_bk_right_spin_(NULL),
			cursor_move_(NULL),
			cursor_copy_(NULL),
			item_drag_dest_(0)
{
	RegisterWindowClass();
	SetControlFont(GetDefaultFont());
	glyphs_mono_bmp_.CreateBitmap(48, 7, 1, 1, bits_glyphs_);
	divider_height_ = 0;
	top_margin_ = 0;
	bottom_margin_ = 2;
	tab_color_ = ::GetSysColor(COLOR_WINDOW);
	backgnd_color_ = ::GetSysColor(COLOR_3DFACE);
	text_color_ = GetSysColor(COLOR_BTNTEXT);
}

// Register the window class if it has not already been registered.

bool CustomTabCtrl::RegisterWindowClass()
{
    WNDCLASS wndcls;
    HINSTANCE inst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(inst, CustomTabCtrl_CLASSNAME, &wndcls)))
    {
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = inst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = CustomTabCtrl_CLASSNAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return false;
        }
    }

    return true;
}

CustomTabCtrl::~CustomTabCtrl()
{
	for (int i=0; i< a_items_.GetSize(); i++)
		delete a_items_[i];
	a_items_.RemoveAll();

	::DeleteObject(bmp_bk_left_spin_);
	bmp_bk_left_spin_ = NULL;
	::DeleteObject(bmp_bk_right_spin_);
	bmp_bk_right_spin_ = NULL;
	::DestroyCursor(cursor_move_);
	cursor_move_ = NULL;
	::DestroyCursor(cursor_copy_);
	cursor_copy_ = NULL;
}

BEGIN_MESSAGE_MAP(CustomTabCtrl, CWnd)
	//{{AFX_MSG_MAP(CustomTabCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
//	ON_MESSAGE(THM_WM_THEMECHANGED, OnThemeChanged)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_EN_UPDATE(CTCID_EDITCTRL, OnUpdateEdit)
	ON_WM_RBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
END_MESSAGE_MAP()

// CustomTabCtrl message handlers

bool CustomTabCtrl::Create(UINT dwStyle, const CRect & rect, CWnd * parent_wnd, UINT id)
{
	return !!CWnd::Create(CustomTabCtrl_CLASSNAME, _T(""), dwStyle, rect, parent_wnd, id);
}

BOOL CustomTabCtrl::OnEraseBkgnd(CDC* /*dc*/)
{
	return true;
}

void CustomTabCtrl::OnPaint()
{
	CPaintDC dc(this);

	DrawCtrl(dc, 0);
}


void CustomTabCtrl::DrawCtrl(CDC& dc, int selection_tinge)
{
	if (!bmp_bk_left_spin_)
	{
		rgb_glyph_[0] = GetSysColor(COLOR_BTNTEXT);
		rgb_glyph_[1] = GetSysColor(COLOR_BTNTEXT);
		rgb_glyph_[2] = GetSysColor(COLOR_BTNTEXT);
		rgb_glyph_[3] = GetSysColor(COLOR_BTNTEXT);
	}

	CRect client(0, 0, 0, 0);
	GetClientRect(client);

	CRect cl= GetTabArea();
	if (IsVertical())
		cl.SetRect(0, 0, cl.Height(), cl.Width());

	COLORREF dark= GetLineColor();
	CPen blackPen(PS_SOLID, 1, dark);

	CDC mem_dc;
	CBitmap mem_bmp;
	CBitmap* old_bmp=NULL;

	if (mem_dc.CreateCompatibleDC(&dc))
	{
		if (mem_bmp.CreateCompatibleBitmap(&dc, client.Width(), client.Height()))
			old_bmp = mem_dc.SelectObject(&mem_bmp);
		else
			return;
	}
	else
		return;

	int btns = 0;
	if (close_state_)
		btns++;
	if (prev_state_)
		btns += 2;
	if (first_state_)
		btns += 2;

	// clear background
	mem_dc.FillSolidRect(client, backgnd_color_);

	bool RTL = (GetExStyle() & WS_EX_LAYOUTRTL) != 0;
	bool after = (GetStyle() & CTCS_BUTTONSAFTER) != 0;
	bool top = (GetStyle() & CTCS_TOP) != 0;
	int a = cl.Height()-4;
	CRect all;
	if (top)
		all.SetRect(0, 0, btns*a+3, cl.Height()-1);
	else
		all.SetRect(0, 1, btns*a+3, cl.Height());
	
	if (btns==0)
		all.SetRectEmpty();
	int close_offset = 0;
	
	if (after)
	{
		close_offset = cl.Width()-all.Width();
		all.OffsetRect(close_offset, 0);
	}

	// draw tab items visible and not selected
	const int count= a_items_.GetSize();
	for (int i= 0; i < count; ++i)
		if (a_items_[i]->shape_ && !a_items_[i]->selected_)
			a_items_[i]->Draw(mem_dc, font_, a_items_[i]->highlighted_, !!(GetStyle() & CTCS_TOP), RTL, 0, tab_color_, text_color_);

	// draw shadow
	{
		Gdiplus::Graphics g(mem_dc);
		Gdiplus::RectF shadow_area(client.left, client.top, client.Width(), 4);
		Gdiplus::Color line(50, 0, 0, 0);
		Gdiplus::Color top(25, 0, 0, 0);	// darker at the top
		Gdiplus::Color bottom(0, 0, 0, 0);
		Gdiplus::LinearGradientBrush brush(shadow_area, top, bottom, Gdiplus::LinearGradientModeVertical);
		Gdiplus::Color colors[]= { line, top, bottom };
		float positions[]= { 0.0f, 0.25f, 1.0f };
		brush.SetInterpolationColors(colors, positions, array_count(colors));

		g.FillRectangle(&brush, shadow_area);
	}

	// draw selected tab item	
	if (item_selected_ != -1 && a_items_[item_selected_]->shape_)
		a_items_[item_selected_]->Draw(mem_dc, font_selected_, true, !!(GetStyle() & CTCS_TOP), RTL, selection_tinge, tab_color_, text_color_);

	if (close_state_ || prev_state_)
	{
		CPen* old_pen = mem_dc.SelectObject(&blackPen);
		mem_dc.Rectangle(all);
		mem_dc.SelectObject(old_pen);
	}


	// draw buttons
/*	if (close_state_)
	{
		bool mirrored = true;
		if (RTL&&after || !RTL&&!after)
			mirrored = false;
		CRect close;
		if (after)
		{
			if (top)
				close.SetRect(cl.Width()-a-1, 1, cl.Width()-1, cl.Height()-2);
			else
				close.SetRect(cl.Width()-a-1, 2, cl.Width()-1, cl.Height()-1);
		}
		else
		{
			close_offset = a;
			if (top)
				close.SetRect(1, 1, a+1, cl.Height()-2);
			else
				close.SetRect(1, 2, a+1, cl.Height()-1);

		}
		CPoint close(close.left+(close.Width()-8)/2+close.Width()%2, close.top+(close.Height()-7)/2);
		if (mirrored && bmp_bk_right_spin_)
			DrawBk(mem_dc, close, bmp_bk_right_spin_, is_right_image_hor_layout_, mrgn_right_, close_state_-1);
		else if (bmp_bk_left_spin_)
			DrawBk(mem_dc, close, bmp_bk_left_spin_, is_left_image_hor_layout_, mrgn_left_, close_state_-1);
		else
		{
			if (close_state_==BNST_PRESSED)
				mem_dc.DrawFrameControl(close, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);
			else
				mem_dc.DrawFrameControl(close, DFC_BUTTON, DFCS_BUTTONPUSH);
		}
		if (RTL)
			DrawGlyph(mem_dc, close, 1, close_state_-1);
		else
			DrawGlyph(mem_dc, close, 4, close_state_-1);
	} */

	if (prev_state_)
	{
		CRect prev, next;

		if (top)
		{
//			if (btns<4)
			{
				prev.SetRect(close_offset, 2, close_offset+a+2, cl.Height()+2);
				next.SetRect(close_offset+a+2, 2, close_offset+2*a+3, cl.Height()+2);
			}
			//else
			//{
			//	first.SetRect(close_offset+1, 1, close_offset+a+1, cl.Height()-2);
			//	prev.SetRect(close_offset+a+1, 1, close_offset+2*a+1, cl.Height()-2);
			//	next.SetRect(close_offset+2*a+2, 1, close_offset+3*a+2, cl.Height()-2);
			//	last.SetRect(close_offset+3*a+2, 1, close_offset+4*a+2, cl.Height()-2);
			//}
		}
		else
		{
//			if (btns<4)
			{
				prev.SetRect(close_offset+1, 2, close_offset+a+1, cl.Height()-1);
				next.SetRect(close_offset+a+2, 2, close_offset+2*a+2, cl.Height()-1);
			}
			//else
			//{
			//	first.SetRect(close_offset+1, 2, close_offset+a+1, cl.Height()-1);
			//	prev.SetRect(close_offset+a+1, 2, close_offset+2*a+1, cl.Height()-1);
			//	next.SetRect(close_offset+2*a+2, 2, close_offset+3*a+2, cl.Height()-1);
			//	last.SetRect(close_offset+3*a+2, 2, close_offset+4*a+2, cl.Height()-1);
			//}
		}
/*
		if (btns>=4)
		{
			CPoint first(first.left+(first.Width()-8)/2, first.top+(first.Height()-7)/2);
			if (RTL && bmp_bk_right_spin_)
				DrawBk(mem_dc, first, bmp_bk_right_spin_, is_right_image_hor_layout_, mrgn_right_, first_state_-1);
			else if (bmp_bk_left_spin_)
				DrawBk(mem_dc, first, bmp_bk_left_spin_, is_left_image_hor_layout_, mrgn_left_, first_state_-1);
			else
			{
				if (first_state_==BNST_PRESSED)
					mem_dc.DrawFrameControl(first, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);
				else
					mem_dc.DrawFrameControl(first, DFC_BUTTON, DFCS_BUTTONPUSH);
			}
			if (RTL)
				DrawGlyph(mem_dc, first, 2, first_state_-1);
			else
				DrawGlyph(mem_dc, first, 0, first_state_-1);
				
			CPoint last(last.left+(last.Width()-8)/2, last.top+(last.Height()-7)/2);
			if (RTL && bmp_bk_left_spin_)
				DrawBk(mem_dc, last, bmp_bk_left_spin_, is_left_image_hor_layout_, mrgn_left_, last_state_-1);
			else if (bmp_bk_right_spin_)
				DrawBk(mem_dc, last, bmp_bk_right_spin_, is_right_image_hor_layout_, mrgn_right_, last_state_-1);
			else
			{
				if (last_state_==BNST_PRESSED)
					mem_dc.DrawFrameControl(last, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);
				else
					mem_dc.DrawFrameControl(last, DFC_BUTTON, DFCS_BUTTONPUSH);
			}
			if (RTL)
				DrawGlyph(mem_dc, last, 5, last_state_-1);
			else
				DrawGlyph(mem_dc, last, 3, last_state_-1);
		}
*/
		CPoint prev_pt(prev.left+(prev.Width()-8)/2, prev.top+(prev.Height()-7)/2);
		UINT state= DFCS_BUTTONPUSH | DFCS_FLAT;
		//if (RTL && bmp_bk_right_spin_)
		//	DrawBk(mem_dc, prev, bmp_bk_right_spin_, is_right_image_hor_layout_, mrgn_right_, prev_state_-1);
		//else if (bmp_bk_left_spin_)
		//	DrawBk(mem_dc, prev, bmp_bk_left_spin_, is_left_image_hor_layout_, mrgn_left_, prev_state_-1);
		//else
		{
			if (prev_state_==BNST_PRESSED)
				mem_dc.DrawFrameControl(prev, DFC_BUTTON, state | DFCS_PUSHED);
			else
				mem_dc.DrawFrameControl(prev, DFC_BUTTON, state);
		}
		//if (RTL)
		//	DrawGlyph(mem_dc, prev_pt, 3, prev_state_-1);
		//else
			DrawGlyph(mem_dc, prev_pt, 1, prev_state_-1);

		CPoint next_pt(next.left+(next.Width()-8)/2, next.top+(next.Height()-7)/2);
		//if (RTL && bmp_bk_left_spin_)
		//	DrawBk(mem_dc, next, bmp_bk_left_spin_, is_left_image_hor_layout_, mrgn_left_, next_state_-1);
		//else if (bmp_bk_right_spin_)
		//	DrawBk(mem_dc, next, bmp_bk_right_spin_, is_right_image_hor_layout_, mrgn_right_, next_state_-1);
		//else
		{
			next.left -= 1;
			if (next_state_==BNST_PRESSED)
				mem_dc.DrawFrameControl(next, DFC_BUTTON, state | DFCS_PUSHED);
			else
				mem_dc.DrawFrameControl(next, DFC_BUTTON, state);
		}
		//if (RTL)
		//	DrawGlyph(mem_dc, next_pt, 4, next_state_-1);
		//else
			DrawGlyph(mem_dc, next_pt, 2, next_state_-1);
	}

	// draw black lines
	int offset_x = all.Width();

	CPoint pts[4];
	int bottom= cl.bottom - 1;
	if (after)
	{
		if (GetStyle()&CTCS_TOP)
		{
			if (item_selected_==-1)
			{
				pts[0] = CPoint(0, bottom);
				pts[1] = CPoint(0, bottom);
				pts[2] = CPoint(0, bottom);
				pts[3] = CPoint(cl.right - offset_x, bottom);
			}
			else
			{
				if (a_items_[item_selected_]->shape_)
				{
					pts[0] = CPoint(0, bottom);
					pts[1] = CPoint(a_items_[item_selected_]->rect_.left, bottom);
					pts[2] = CPoint(a_items_[item_selected_]->rect_.right, bottom);
					pts[3] = CPoint(cl.right-offset_x, bottom);
				}
				else
				{
					pts[0] = CPoint(0, bottom);
					pts[1] = CPoint(0, bottom);
					pts[2] = CPoint(0, bottom);
					pts[3] = CPoint(cl.right-offset_x, bottom);
				}
			}
		}
		else
		{
			if (item_selected_ == -1)
			{
				pts[0] = CPoint(0, 1);
				pts[1] = CPoint(0, 1);
				pts[2] = CPoint(0, 1);
				pts[3] = CPoint(cl.right - offset_x, 1);
			}
			else
			{
				if (a_items_[item_selected_]->shape_)
				{
					pts[0] = CPoint(0, 1);
					pts[1] = CPoint(a_items_[item_selected_]->rect_.left, 1);
					pts[2] = CPoint(a_items_[item_selected_]->rect_.right, 1);
					pts[3] = CPoint(cl.right-offset_x, 1);
				}
				else
				{
					pts[0] = CPoint(0, 1);
					pts[1] = CPoint(0, 1);
					pts[2] = CPoint(0, 1);
					pts[3] = CPoint(cl.right-offset_x, 1);
				}
			}
		}
	}
	else
	{
		if (GetStyle() & CTCS_TOP)
		{
			if (item_selected_ == -1)
			{
				pts[0] = CPoint(offset_x, bottom); 
				pts[1] = CPoint(offset_x, bottom); 
				pts[2] = CPoint(offset_x, bottom); 
				pts[3] = CPoint(cl.right, bottom);
			}
			else
			{
				if (a_items_[item_selected_]->shape_)
				{
					pts[0] = CPoint(offset_x, bottom); 
					pts[1] = CPoint(a_items_[item_selected_]->rect_.left, bottom); 
					pts[2] = CPoint(a_items_[item_selected_]->rect_.right, bottom); 
					pts[3] = CPoint(cl.right, bottom);
				}
				else
				{
					pts[0] = CPoint(offset_x, bottom); 
					pts[1] = CPoint(offset_x, bottom); 
					pts[2] = CPoint(offset_x, bottom); 
					pts[3] = CPoint(cl.right, bottom);
				}
			}
		}
		else
		{
			if (item_selected_ == -1)
			{
				pts[0] = CPoint(offset_x, 1); 
				pts[1] = CPoint(offset_x, 1); 
				pts[2] = CPoint(offset_x, 1); 
				pts[3] = CPoint(cl.right, 1);
			}
			else
			{
				if (a_items_[item_selected_]->shape_)
				{
					pts[0] = CPoint(offset_x, 1); 
					pts[1] = CPoint(a_items_[item_selected_]->rect_.left, 1);
					pts[2] = CPoint(a_items_[item_selected_]->rect_.right, 1);
					pts[3] = CPoint(cl.right, 1);
				}
				else
				{
					pts[0] = CPoint(offset_x, 1);
					pts[1] = CPoint(offset_x, 1);
					pts[2] = CPoint(offset_x, 1);
					pts[3] = CPoint(cl.right, 1);
				}
			}
		}
	}

//	CPen p(PS_SOLID, 1, RGB(255, 0, 0));
	CPen* old_pen = mem_dc.SelectObject(&blackPen);
	mem_dc.MoveTo(pts[0]);
//	mem_dc.LineTo(pts[1]);
	mem_dc.MoveTo(pts[2]);
//	mem_dc.LineTo(pts[3]);
//mem_dc.SelectStockObject(WHITE_PEN);
//mem_dc.MoveTo(pts[0].x, pts[0].y+1);
//mem_dc.LineTo(pts[1].x, pts[1].y+1);
//mem_dc.MoveTo(pts[2].x, pts[2].y+1);
//mem_dc.LineTo(pts[3].x, pts[3].y+1);

	mem_dc.SelectObject(old_pen);
	
	if (button_id_down_>=0 && (GetCursor()==cursor_move_ || GetCursor()==cursor_copy_))
	{
		// Draw drag destination marker
		CPen* old_pen = mem_dc.SelectObject(&blackPen);
		int x;
		if (item_drag_dest_==a_items_.GetSize())
			x = a_items_[item_drag_dest_-1]->text_rect_.right + cl.Height()/4-3;
		else
			x = a_items_[item_drag_dest_]->text_rect_.left - cl.Height()/4-3;
		if (x>=cl.right-7)
			x = cl.right-7;
		mem_dc.MoveTo(x, 1);
		mem_dc.LineTo(x+7, 1);
		mem_dc.MoveTo(x+1, 2);
		mem_dc.LineTo(x+6, 2);
		mem_dc.MoveTo(x+2, 3);
		mem_dc.LineTo(x+5, 3);
		mem_dc.MoveTo(x+3, 4);
		mem_dc.LineTo(x+4, 4);
		mem_dc.SelectObject(old_pen);
	}

	// draw divider

	//COLORREF gray= ::GetSysColor(COLOR_3DFACE);
	//COLORREF light= ::GetSysColor(COLOR_3DHIGHLIGHT);
	//COLORREF dark= GetLineColor();
//	COLORREF dark= CalcColor(::GetSysColor(COLOR_3DSHADOW), gray, 0.5f);

//	mem_dc.FillSolidRect(client.left, client.bottom - 1, client.Width(), 1, gray);
	//for (int i= 0; i < divider_height_ - 1; ++i)
	//{
	//	int y= client.bottom - divider_height_ + i;

	//	double x= double(i) / divider_height_;

	//	COLORREF c= CalcColor(light, dark, float(1 - x * x));// 1.0f - (float(i) / divider_height_) / 2.0f);

	//	mem_dc.FillSolidRect(client.left, y, client.Width(), 1, c);
	//}

//	mem_dc.FillSolidRect(client.left, client.bottom - 1, client.Width(), 1, dark);

//	if (top_margin_ > 0)
//		mem_dc.FillSolidRect(client.left, client.top, client.Width(), top_margin_, gray);

	//if (top_margin_ > 0)
	//	mem_dc.FillSolidRect(client.left, client.top + top_margin_ - 1, client.Width(), 1, dark);

/*	if (IsVertical())
	{
		POINT pts[3];
		if (RTL)
		{
			pts[0].x = -1;
			pts[0].y = cl.Width();
			pts[1].x = -1;
			pts[1].y = 0;
			pts[2].x = cl.Height()-1;
			pts[2].y = cl.Width();
			::PlgBlt(dc.m_hDC, pts, mem_dc.m_hDC, -1, 0, cl.Width(), cl.Height(), NULL, 0, 0);
		}
		else
		{
			pts[0].x = 0;
			pts[0].y = cl.Width();
			pts[1].x = 0;
			pts[1].y = 0;
			pts[2].x = cl.Height();
			pts[2].y = cl.Width();
			::PlgBlt(dc.m_hDC, pts, mem_dc.m_hDC, 0, 0, cl.Width(), cl.Height(), NULL, 0, 0);
		}
	}
	else */
		dc.BitBlt(client.left, client.top, client.Width(), client.Height(), &mem_dc, client.left, client.top, SRCCOPY);

	mem_dc.SelectObject(old_bmp);
}

void CustomTabCtrl::OnSize(UINT type, int cx, int cy) 
{
	CWnd::OnSize(type, cx, cy);
	if (cx && cy)
		RecalcLayout(RECALC_RESIZED, item_selected_);
}

LRESULT CustomTabCtrl::OnSizeParent(WPARAM, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* params = reinterpret_cast<AFX_SIZEPARENTPARAMS*>(lParam);

	CRect r;
	GetWindowRect(r);


	if (IsVertical())
	{
		if (GetStyle()&CTCS_TOP) // left
		{
			params->rect.left += r.Width();
			MoveWindow(params->rect.left-r.Width(), params->rect.top, r.Width(), params->rect.bottom-params->rect.top, true);
		}
		else // right
		{
			params->rect.right -= r.Width();
			MoveWindow(params->rect.right, params->rect.top, r.Width(), params->rect.bottom-params->rect.top, true);
		}
	}
	else
	{
		if (GetStyle()&CTCS_TOP)
		{
			params->rect.top += r.Height();
			MoveWindow(params->rect.left, params->rect.top-r.Height(), params->rect.right-params->rect.left, r.Height(), true);
		}
		else
		{
			params->rect.bottom -= r.Height();
			MoveWindow(params->rect.left, params->rect.bottom, params->rect.right-params->rect.left, r.Height(), true);
		}
	}

	return 0;

}

void CustomTabCtrl::OnLButtonDown(UINT flags, CPoint point)
{
	int hit_test = HitTest(point);
	NotifyParent(CTCN_CLICK, hit_test, point);
	ProcessLButtonDown(hit_test, flags, point);
	CWnd::OnLButtonDown(flags, point);
}

void CustomTabCtrl::OnLButtonDblClk(UINT flags, CPoint point) 
{
	int HT_ret = ProcessLButtonDown(HitTest(point), flags, point);
	if (HT_ret>=0)
	{
		button_id_down_ = CTCID_NOBUTTON;
		if (HT_ret==HitTest(point))
			EditLabel(HT_ret, true);
	}
	NotifyParent(CTCN_DBLCLK, HitTest(point), point);
	CWnd::OnLButtonDblClk(flags, point);
}

int CustomTabCtrl::ProcessLButtonDown(int hit_test, UINT flags, CPoint point)
{
	SetCapture();
	switch (hit_test)
	{
	case CTCHT_NOWHERE:
		button_id_down_ = CTCID_NOBUTTON;
		break;
	case CTCHT_ONFIRSTBUTTON:
		{
			button_id_down_ = CTCID_FIRSTBUTTON;
			first_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_FIRST_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			SetTimer(1, 100, NULL);
		}
		break;
	case CTCHT_ONPREVBUTTON:
		{
			button_id_down_ = CTCID_PREVBUTTON;
			prev_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_PREV_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			SetTimer(1, 100, NULL);
		}
		break;
	case CTCHT_ONNEXTBUTTON:
		{
			button_id_down_ = CTCID_NEXTBUTTON;
			next_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_NEXT_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			SetTimer(1, 100, NULL);
		}
		break;
	case CTCHT_ONLASTBUTTON:
		{
			button_id_down_ = CTCID_LASTBUTTON;
			last_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_LAST_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			SetTimer(1, 100, NULL);
		}
		break;
	case CTCHT_ONCLOSEBUTTON:
		{
			button_id_down_ = CTCID_CLOSEBUTTON;
			close_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_CLOSE_PRESSED, item_selected_);
			Invalidate(false);
		}
		break;
	default:
		{
			DWORD dwStyle = GetStyle();
			if (((dwStyle&CTCS_DRAGMOVE) && !(flags&MK_CONTROL) && cursor_move_) || 
				((dwStyle&CTCS_DRAGCOPY) && (flags&MK_CONTROL) && cursor_copy_))
			{
				button_id_down_ = hit_test;
				item_drag_dest_ = CTCID_NOBUTTON;
				SetTimer(2, 300, NULL);
			}
			else
				button_id_down_ = CTCID_NOBUTTON;

			if ((GetStyle()&CTCS_MULTIHIGHLIGHT) && (flags&MK_CONTROL))
				HighlightItem(hit_test, true, !!(flags & MK_CONTROL));
			else
			{
				bool notify = hit_test!=item_selected_;
				SetCurSel(hit_test, true, !!(flags & MK_CONTROL));
				if (notify)
					NotifyParent(CTCN_SELCHANGE, item_selected_, point);
			}
			for (int i=0; i<a_items_.GetSize();i++)
			{
				if (a_items_[i]->highlight_changed_)
					NotifyParent(CTCN_HIGHLIGHTCHANGE, i, point);
			}
		}
		break;
	}
	return hit_test;
}


bool CustomTabCtrl::NotifyParent(UINT code, int item, CPoint pt)
{
	CTC_NMHDR nmh;
	memset(&nmh, 0, sizeof(CTC_NMHDR));
	nmh.hdr.hwndFrom = GetSafeHwnd();
	nmh.hdr.idFrom = GetDlgCtrlID();
	nmh.hdr.code = code;
	nmh.item = item;
	nmh.hit_test = pt;
	if (item >= 0)
	{
		_tcsncpy(nmh.pszText, a_items_[item]->text_, MAX_LABEL_TEXT);
		nmh.pszText[MAX_LABEL_TEXT] = 0;
		nmh.lParam = a_items_[item]->param_;
		nmh.item_rect = a_items_[item]->text_rect_;
		nmh.selected = a_items_[item]->selected_;
		nmh.highlighted = a_items_[item]->highlighted_;
	}
	return GetParent()->SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&nmh) != 0;
}


void CustomTabCtrl::OnLButtonUp(UINT /*flags*/, CPoint /*point*/)
{
	if (prev_state_ || next_state_ || first_state_ || last_state_ || close_state_)
	{
		if (close_state_)
			close_state_ = BNST_NORMAL;
		if (prev_state_)
		{
			prev_state_ = BNST_NORMAL;
			next_state_ = BNST_NORMAL;
		}
		if (first_state_)
		{
			first_state_ = BNST_NORMAL;
			last_state_ = BNST_NORMAL;
		}
		Invalidate(false);
		KillTimer(1);
	}
	if (button_id_down_>=0)
	{
		if ((GetCursor()==cursor_copy_) && (GetKeyState(VK_CONTROL)&0x8000))
			CopyItem(button_id_down_, item_drag_dest_, true);
		else if ((GetCursor()==cursor_move_) && !(GetKeyState(VK_CONTROL)&0x8000))
			MoveItem(button_id_down_, item_drag_dest_, true);
	}
	button_id_down_ = CTCID_NOBUTTON;
	item_drag_dest_ = CTCID_NOBUTTON;
	ReleaseCapture();
}


void CustomTabCtrl::OnMouseMove(UINT flags, CPoint point)
{
	TRACKMOUSEEVENT trackmouseevent;
	trackmouseevent.cbSize = sizeof(trackmouseevent);
	trackmouseevent.dwFlags = TME_LEAVE;
	trackmouseevent.hwndTrack = GetSafeHwnd();
	trackmouseevent.dwHoverTime = 0;
	_TrackMouseEvent(&trackmouseevent);

	int hit_test = HitTest(point);
	
	if (first_state_)
	{
		if (hit_test==CTCHT_ONFIRSTBUTTON)
		{
			if (button_id_down_==CTCID_FIRSTBUTTON)
				first_state_ = BNST_PRESSED;
			else if (button_id_down_==CTCID_NOBUTTON && !(flags&MK_LBUTTON))
				first_state_ = BNST_HOT;
			else
				first_state_ = BNST_NORMAL;
		}
		else
			first_state_ = BNST_NORMAL;
		Invalidate(false);
	}

	if (prev_state_)
	{
		if (hit_test==CTCHT_ONPREVBUTTON)
		{
			if (button_id_down_==CTCID_PREVBUTTON)
				prev_state_ = BNST_PRESSED;
			else if (button_id_down_==CTCID_NOBUTTON && !(flags&MK_LBUTTON))
				prev_state_ = BNST_HOT;
			else
				prev_state_ = BNST_NORMAL;
		}
		else
			prev_state_ = BNST_NORMAL;
		Invalidate(false);
	}

	if (next_state_)
	{
		if (hit_test==CTCHT_ONNEXTBUTTON)
		{
			if (button_id_down_==CTCID_NEXTBUTTON)
				next_state_ = BNST_PRESSED;
			else if (button_id_down_==CTCID_NOBUTTON && !(flags&MK_LBUTTON))
				next_state_ = BNST_HOT;
			else
				next_state_ = BNST_NORMAL;
		}
		else
			next_state_ = BNST_NORMAL;
		Invalidate(false);
	}

	if (last_state_)
	{
		if (hit_test==CTCHT_ONLASTBUTTON)
		{
			if (button_id_down_==CTCID_LASTBUTTON)
				last_state_ = BNST_PRESSED;
			else if (button_id_down_==CTCID_NOBUTTON && !(flags&MK_LBUTTON))
				last_state_ = BNST_HOT;
			else
				last_state_ = BNST_NORMAL;
		}
		else
			last_state_ = BNST_NORMAL;
		Invalidate(false);
	}

	if (close_state_)
	{
		if (hit_test==CTCHT_ONCLOSEBUTTON)
		{
			if (button_id_down_==CTCID_CLOSEBUTTON)
				close_state_ = BNST_PRESSED;
			else if (button_id_down_==CTCID_NOBUTTON && !(flags&MK_LBUTTON))
				close_state_ = BNST_HOT;
			else
				close_state_ = BNST_NORMAL;
		}
		else
			close_state_ = BNST_NORMAL;
		Invalidate(false);
	}

	if (button_id_down_>=0 && item_drag_dest_>=0)
	{
		CRect cl;
		GetClientRect(&cl);
		int x = point.x;
		if (IsVertical())
		{
			x = cl.Height()-point.y;
			cl.SetRect(0, 0, cl.Height(), cl.Width());
		}
		if (item_drag_dest_>=a_items_.GetSize())
			item_drag_dest_ = a_items_.GetSize()-1;
		
		int x1 = a_items_[item_drag_dest_]->text_rect_.left - cl.Height()/4;
		int x2 = a_items_[item_drag_dest_]->text_rect_.right + cl.Height()/4;
		if (x>=cl.right)
		{
			item_drag_dest_++;
			if (item_drag_dest_>=a_items_.GetSize())
				RecalcLayout(RECALC_NEXT_PRESSED, a_items_.GetSize()-1);
			else
				RecalcLayout(RECALC_NEXT_PRESSED, item_drag_dest_);
			Invalidate(false);
		}
		else if (x>=x2)
		{
			item_drag_dest_++;
			if (item_drag_dest_>=a_items_.GetSize())
				RecalcLayout(RECALC_ITEM_SELECTED, a_items_.GetSize()-1);
			else
				RecalcLayout(RECALC_ITEM_SELECTED, item_drag_dest_);
			Invalidate(false);
		}
		else if (x<x1)
		{
			if (item_drag_dest_>0)
				item_drag_dest_--;
			
			RecalcLayout(RECALC_ITEM_SELECTED, item_drag_dest_);
			Invalidate(false);
		}
	}
}


LRESULT CustomTabCtrl::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/) 
{
	if (first_state_ || prev_state_ || close_state_)
	{
		if (close_state_)
			close_state_ = BNST_NORMAL;
		if (prev_state_)
		{
			prev_state_ = BNST_NORMAL;
			next_state_ = BNST_NORMAL;
		}
		if (first_state_)
		{
			first_state_ = BNST_NORMAL;
			last_state_ = BNST_NORMAL;
		}
		Invalidate(false);
		KillTimer(1);
	}
	return 0;
}


void CustomTabCtrl::OnUpdateEdit() 
{
	if (ctrl_edit_.m_hWnd)
	{
		ctrl_edit_.GetWindowText(a_items_[item_selected_]->text_);
		RecalcLayout(RECALC_EDIT_RESIZED, item_selected_);
		Invalidate(false);
	}
}

/*
LONG CustomTabCtrl::OnThemeChanged(WPARAM *wParam*, LPARAM *lParam*) 
{
	::DeleteObject(bmp_bk_left_spin_);
	bmp_bk_left_spin_ = NULL;
	::DeleteObject(bmp_bk_right_spin_);
	bmp_bk_right_spin_ = NULL;

	HBITMAP bmp_glyph = NULL;
	CDC glyph_dc;
	glyph_dc.CreateCompatibleDC(NULL);
	CBitmap* old_bmp_glyph = NULL;

	try
	{
		CThemeUtil tm;
		if (!tm.OpenThemeData(m_hWnd, L"SPIN"))
			AfxThrowUserException();

		{
			// left spin background
			int bk_type;
			if (!tm.GetThemeEnumValue(SPNP_DOWNHORZ, 0, TMT_BGTYPE, &bk_type))
				AfxThrowUserException();
			if (bk_type!=BT_IMAGEFILE)
				AfxThrowUserException();

			int image_count;
			if (!tm.GetThemeInt(SPNP_DOWNHORZ, 0, TMT_IMAGECOUNT, &image_count))
				AfxThrowUserException();
			if (image_count!=4)
				AfxThrowUserException();

			WCHAR spin_bk_left_bitmap_filename[MAX_PATH];
			if (!tm.GetThemeFilename(SPNP_DOWNHORZ, 0, TMT_IMAGEFILE, spin_bk_left_bitmap_filename, MAX_PATH))
				AfxThrowUserException();
			bmp_bk_left_spin_ = tm.LoadBitmap(spin_bk_left_bitmap_filename);
			if (!bmp_bk_left_spin_)
				AfxThrowUserException();

			int left_image_layout;
			if (!tm.GetThemeEnumValue(SPNP_DOWNHORZ, 0, TMT_IMAGELAYOUT, &left_image_layout))
				AfxThrowUserException();
			if (left_image_layout==IL_VERTICAL)
				is_left_image_hor_layout_ = false;
			else
				is_left_image_hor_layout_ = true;
			
			if (!tm.GetThemeMargins(SPNP_DOWNHORZ, 0, TMT_SIZINGMARGINS, &mrgn_left_))
				AfxThrowUserException();
		}
		{
			// right spin background
			int bk_type;
			if (!tm.GetThemeEnumValue(SPNP_UPHORZ, 0, TMT_BGTYPE, &bk_type))
				AfxThrowUserException();
			if (bk_type!=BT_IMAGEFILE)
				AfxThrowUserException();

			int image_count;
			if (!tm.GetThemeInt(SPNP_UPHORZ, 0, TMT_IMAGECOUNT, &image_count))
				AfxThrowUserException();
			if (image_count!=4)
				AfxThrowUserException();

			WCHAR spin_bk_right_bitmap_filename[MAX_PATH];
			if (!tm.GetThemeFilename(SPNP_UPHORZ, 0, TMT_IMAGEFILE, spin_bk_right_bitmap_filename, MAX_PATH))
				AfxThrowUserException();
			
			bmp_bk_right_spin_ = tm.LoadBitmap(spin_bk_right_bitmap_filename);
			if (!bmp_bk_right_spin_)
				AfxThrowUserException();
	
			int right_image_layout;
			if (!tm.GetThemeEnumValue(SPNP_UPHORZ, 0, TMT_IMAGELAYOUT, &right_image_layout))
				AfxThrowUserException();
			if (right_image_layout==IL_VERTICAL)
				is_right_image_hor_layout_ = false;
			else
				is_right_image_hor_layout_ = true;

			if (!tm.GetThemeMargins(SPNP_UPHORZ, 0, TMT_SIZINGMARGINS, &mrgn_right_))
				AfxThrowUserException();
		}
		{
			// glyph color
			int glyph_type;
			if (!tm.GetThemeEnumValue(SPNP_DOWNHORZ, 0, TMT_GLYPHTYPE, &glyph_type))
				AfxThrowUserException();
			
			if (glyph_type==GT_IMAGEGLYPH)
			{
				COLORREF rgb_trans_glyph = RGB(255, 0, 255);
				if (!tm.GetThemeColor(SPNP_DOWNHORZ, 0, TMT_GLYPHTRANSPARENTCOLOR, &rgb_trans_glyph))
					AfxThrowUserException();
				WCHAR spin_glyph_icon_filename[MAX_PATH];
				if (!tm.GetThemeFilename(SPNP_DOWNHORZ, 0, TMT_GLYPHIMAGEFILE, spin_glyph_icon_filename, MAX_PATH))
					AfxThrowUserException();
				bmp_glyph = tm.LoadBitmap(spin_glyph_icon_filename);
				if (!bmp_glyph)
					AfxThrowUserException();

				CBitmap* bmp = CBitmap::FromHandle(bmp_glyph);
				if (bmp==NULL)
					AfxThrowUserException();
				old_bmp_glyph = glyph_dc.SelectObject(bmp);
				BITMAP bm;
				bmp->GetBitmap(&bm);
				rgb_glyph_[0] = rgb_trans_glyph;
				rgb_glyph_[1] = rgb_trans_glyph;
				rgb_glyph_[2] = rgb_trans_glyph;
				rgb_glyph_[3] = rgb_trans_glyph;
				if (is_left_image_hor_layout_)
				{
					for (int i=0;i<bm.bmWidth;i++)
					{
						if (i<bm.bmWidth/4 && rgb_glyph_[0]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmHeight;j++)
							{
								if ((rgb_glyph_[0]=glyph_dc.GetPixel(i, j))!=rgb_trans_glyph)
									break;
							}
							if (i==bm.bmWidth/4-1 && rgb_glyph_[0]==rgb_trans_glyph)
								AfxThrowUserException();
						}
						else if (i>=bm.bmWidth/4 && i<bm.bmWidth/2 && rgb_glyph_[1]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmHeight;j++)
							{
								if ((rgb_glyph_[1]=glyph_dc.GetPixel(i, j))!=rgb_trans_glyph)
									break;
							}
							if (i==bm.bmWidth/2-1 && rgb_glyph_[1]==rgb_trans_glyph)
								AfxThrowUserException();
						}
						else if (i>=bm.bmWidth/2 && i<3*bm.bmWidth/4 && rgb_glyph_[2]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmHeight;j++)
							{
								if ((rgb_glyph_[2]=glyph_dc.GetPixel(i, j))!=rgb_trans_glyph)
									break;
							}
							if (i==3*bm.bmWidth/4-1 && rgb_glyph_[2]==rgb_trans_glyph)
								AfxThrowUserException();
						}
						else if (i>=3*bm.bmWidth/4 && i<bm.bmWidth && rgb_glyph_[3]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmHeight;j++)
							{
								if ((rgb_glyph_[3]=glyph_dc.GetPixel(i, j))!=rgb_trans_glyph)
									break;
							}
							if (i==bm.bmWidth-1 && rgb_glyph_[3]==rgb_trans_glyph)
								AfxThrowUserException();
						}
					}
				}
				else
				{
					for (int i=0;i<bm.bmHeight;i++)
					{
						if (i<bm.bmHeight/4 && rgb_glyph_[0]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmWidth;j++)
							{
								if ((rgb_glyph_[0] = glyph_dc.GetPixel(j, i))!=rgb_trans_glyph)
									break;
							}
							if (i==bm.bmHeight/4-1 && rgb_glyph_[0]==rgb_trans_glyph)
								AfxThrowUserException();
						}
						else if (i>=bm.bmHeight/4 && i<bm.bmHeight/2 && rgb_glyph_[1]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmWidth;j++)
							{
								if ((rgb_glyph_[1]=glyph_dc.GetPixel(j, i))!=rgb_trans_glyph)
									break;
							}
							if (i==bm.bmHeight/2-1 && rgb_glyph_[1]==rgb_trans_glyph)
								AfxThrowUserException();
						}
						else if (i>=bm.bmHeight/2 && i<3*bm.bmHeight/4 && rgb_glyph_[2]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmWidth;j++)
							{
								if ((rgb_glyph_[2] = glyph_dc.GetPixel(j, i))!=rgb_trans_glyph)
									break;
							}
							if (i==3*bm.bmHeight/4-1 && rgb_glyph_[2]==rgb_trans_glyph)
								AfxThrowUserException();
						}
						else if (i>=3*bm.bmHeight/4 && i<bm.bmHeight && rgb_glyph_[3]==rgb_trans_glyph)
						{
							for (int j=0;j<bm.bmWidth;j++)
							{
								if ((rgb_glyph_[3]=glyph_dc.GetPixel(j, i))!=rgb_trans_glyph)
									break;
							}
							if (i==bm.bmHeight-1 && rgb_glyph_[3]==rgb_trans_glyph)
								AfxThrowUserException();
						}
					}
				}
				glyph_dc.SelectObject(old_bmp_glyph);
				old_bmp_glyph = NULL;
				::DeleteObject(bmp_glyph);
				bmp_glyph = NULL;
			}
			else if (glyph_type==GT_FONTGLYPH)
			{
				if (!tm.GetThemeColor(SPNP_UPHORZ, UPHZS_NORMAL, TMT_GLYPHTEXTCOLOR, &rgb_glyph_[0]))
					AfxThrowUserException();
				if (!tm.GetThemeColor(SPNP_UPHORZ, UPHZS_HOT, TMT_GLYPHTEXTCOLOR, &rgb_glyph_[1]))
					AfxThrowUserException();
				if (!tm.GetThemeColor(SPNP_UPHORZ, UPHZS_PRESSED, TMT_GLYPHTEXTCOLOR, &rgb_glyph_[2]))
					AfxThrowUserException();	
			}
			else
				AfxThrowUserException();
		}
		tm.CloseThemeData();
	}
	catch(CUserException* e)
	{
		e->Delete();
		::DeleteObject(bmp_bk_left_spin_);
		bmp_bk_left_spin_ = NULL;
		::DeleteObject(bmp_bk_right_spin_);
		bmp_bk_right_spin_ = NULL;
		if (old_bmp_glyph)
			glyph_dc.SelectObject(old_bmp_glyph);
		::DeleteObject(bmp_glyph);
		bmp_glyph = NULL;
	}
	return 0;
}
*/
void CustomTabCtrl::OnTimer(UINT_PTR id_event) 
{
	CWnd::OnTimer(id_event);

	if (id_event == 1)
	{
		if (first_state_==BNST_PRESSED && ::GetTickCount()-last_repeat_time_>=REPEAT_TIMEOUT)
		{
			first_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_FIRST_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			return;

		}
		if (prev_state_==BNST_PRESSED && ::GetTickCount()-last_repeat_time_>=REPEAT_TIMEOUT)
		{
			prev_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_PREV_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			return;

		}
		if (next_state_==BNST_PRESSED && ::GetTickCount()-last_repeat_time_>=REPEAT_TIMEOUT)
		{
			next_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_NEXT_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			return;
		}
		if (last_state_==BNST_PRESSED && ::GetTickCount()-last_repeat_time_>=REPEAT_TIMEOUT)
		{
			last_state_ = BNST_PRESSED;
			RecalcLayout(RECALC_LAST_PRESSED, item_selected_);
			Invalidate(false);
			last_repeat_time_ = ::GetTickCount();
			return;
		}
	}
	else if (id_event == 2)
	{
		KillTimer(2);

		if (button_id_down_ >= 0)
		{
			if (item_drag_dest_ == CTCID_NOBUTTON)
				item_drag_dest_ = button_id_down_;
			SetTimer(2, 10, NULL);
			DWORD dwStyle = GetStyle();
			if ((dwStyle&CTCS_DRAGCOPY) && (GetKeyState(VK_CONTROL)&0x8000))
				SetCursor(cursor_copy_);
			else if ((dwStyle&CTCS_DRAGMOVE) && !(GetKeyState(VK_CONTROL)&0x8000))
				SetCursor(cursor_move_);
			else
			{
				button_id_down_ = CTCID_NOBUTTON;
				ReleaseCapture();
			}
			Invalidate(false);
		}
	}
}

void CustomTabCtrl::SetControlFont(const LOGFONT& lf, bool redraw)
{
	if (font_.m_hObject)
	{
		DeleteObject(font_);
		font_.m_hObject = NULL;
	}

	if (font_selected_.m_hObject)
	{
		DeleteObject(font_selected_);
		font_selected_.m_hObject = NULL;
	}

	if (!font_.CreateFontIndirect(&lf))
		font_.CreateFontIndirect(&lf_default);


	LOGFONT lfSel;
	font_.GetLogFont(&lfSel);
//	lfSel.lfWeight = FW_BOLD;
	font_selected_.CreateFontIndirect(&lfSel);

	if (redraw)
	{
		RecalcLayout(RECALC_RESIZED, item_selected_);
		Invalidate();
	}
}

int CustomTabCtrl::InsertItem(int index, CString text, LPARAM lParam)
{
	if (index == -1)
		index = a_items_.GetSize();

	if (index < 0 || index > a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;

	if (text.GetLength()>MAX_LABEL_TEXT-1)
		return CTCERR_TEXTTOOLONG;

	CCustomTabCtrlItem* item = new CCustomTabCtrlItem(text, lParam);
	if (item==NULL)
		return CTCERR_OUTOFMEMORY;

	try
	{
		a_items_.InsertAt(index, item);
	}
	catch(CMemoryException* e)
	{
		e->Delete();
		delete item;
		return CTCERR_OUTOFMEMORY;
	}

	if (item_selected_ >= index)
		item_selected_++;

	if (ctrl_tool_tip_.m_hWnd)
	{
		for (int i=a_items_.GetSize()-1; i > index; i--)
		{
			CString s;
			ctrl_tool_tip_.GetText(s, this, i);
			ctrl_tool_tip_.DelTool(this, i);
			ctrl_tool_tip_.AddTool(this, s, CRect(0, 0, 0, 0), i+1);
		}
		ctrl_tool_tip_.DelTool(this, index + 1);
	}
	
	RecalcLayout(RECALC_RESIZED, item_selected_);
	Invalidate(false);

	return index;
}


int CustomTabCtrl::MoveItem(int item_src, int item_dst)
{
	return MoveItem(item_src, item_dst, false);
}

int CustomTabCtrl::MoveItem(int item_src, int item_dst, bool mouse_sel)
{
	if (item_src<0||item_src>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	if (item_dst<0||item_dst>a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;

	if (item_src==item_dst || item_src==item_dst-1)
		return item_src;

	CCustomTabCtrlItem *item = a_items_[item_src];
	
	// remove item from old place
	CString old_tooltip;
	if (ctrl_tool_tip_.m_hWnd)
	{
		ctrl_tool_tip_.GetText(old_tooltip, this, item_src+1);
		for (int i=item_src+1; i< a_items_.GetSize(); i++)
		{
			CString s;
			ctrl_tool_tip_.GetText(s, this, i+1);
			ctrl_tool_tip_.DelTool(this, i);
			ctrl_tool_tip_.AddTool(this, s, CRect(0, 0, 0, 0), i);
		}
	}

	a_items_.RemoveAt(item_src);

	// insert item in new place
	if (item_dst>item_src)
		item_dst--;

	try
	{
		a_items_.InsertAt(item_dst, item);
	}
	catch(CMemoryException* e)
	{
		e->Delete();
		delete item;
		if (mouse_sel)
			NotifyParent(CTCN_ITEMMOVE, item_src, CPoint(0, 0));
		return CTCERR_OUTOFMEMORY;
	}

	if (ctrl_tool_tip_.m_hWnd)
	{
		for (int i=a_items_.GetSize()-1; i>item_dst; i--)
		{
			CString s;
			ctrl_tool_tip_.GetText(s, this, i);
			ctrl_tool_tip_.DelTool(this, i+1);
			ctrl_tool_tip_.AddTool(this, s, CRect(0, 0, 0, 0), i+1);
		}
		ctrl_tool_tip_.DelTool(this, item_dst+1);
		ctrl_tool_tip_.AddTool(this, old_tooltip, CRect(0, 0, 0, 0), item_dst+1);
	}
	
	item_selected_ = item_dst;

	RecalcLayout(RECALC_ITEM_SELECTED, item_selected_);
	Invalidate(false);
	if (mouse_sel)
		NotifyParent(CTCN_ITEMMOVE, item_selected_, CPoint(0, 0));
	return item_dst;
}

int CustomTabCtrl::CopyItem(int item_src, int item_dst)
{
	return CopyItem(item_src, item_dst, false);
}

int CustomTabCtrl::CopyItem(int item_src, int item_dst, bool mouse_sel)
{
	if (item_src<0||item_src>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	if (item_dst<0||item_dst>a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;

	CString dst;
	try
	{
		bool append_flag=true;
		int n = a_items_[item_src]->text_.GetLength();
		if (n>=4)
		{
			if (a_items_[item_src]->text_[n-1]==_T(')') && 
				a_items_[item_src]->text_[n-2]>_T('1') &&
				a_items_[item_src]->text_[n-2]<=_T('9') &&
				a_items_[item_src]->text_[n-3]==_T('('))
			{
				n = a_items_[item_src]->text_.GetLength()-3;
				append_flag = false;
			}
			else if (a_items_[item_src]->text_[n-1]==_T(')') && 
					a_items_[item_src]->text_[n-2]>=_T('0') &&
					a_items_[item_src]->text_[n-2]<=_T('9') &&
					a_items_[item_src]->text_[n-3]>=_T('1') &&
					a_items_[item_src]->text_[n-3]<=_T('9') &&
					a_items_[item_src]->text_[n-4]==_T('('))
			{
				n = a_items_[item_src]->text_.GetLength()-4;
				append_flag = false;
			}
		}
		int ndx = 1;
		while(1)
		{
			ndx++;
			if (append_flag)
				dst.Format(_T("%s (%d)"), (LPCTSTR)a_items_[item_src]->text_, ndx);
			else
				dst.Format(_T("%s(%d)"), (LPCTSTR)a_items_[item_src]->text_.Left(n), ndx);
			int i= 0;
			for (i=0;i<a_items_.GetSize();i++)
			{
				if (a_items_[i]->text_==dst)
					break;
			}
			if (i==a_items_.GetSize())
				break;
		}
	}
	catch(CMemoryException* e)
	{
		e->Delete();
		if (mouse_sel)
			NotifyParent(CTCN_OUTOFMEMORY, item_src, CPoint(0, 0));
		return CTCERR_OUTOFMEMORY;
	}

	int ret_item = InsertItem(item_dst, dst, a_items_[item_src]->param_);
	if (ret_item>=0)
	{
		SetCurSel(ret_item);
		if (mouse_sel)
			NotifyParent(CTCN_ITEMCOPY, ret_item, CPoint(0, 0));
	}
	else if (mouse_sel && ret_item==CTCERR_OUTOFMEMORY)
		NotifyParent(CTCN_OUTOFMEMORY, ret_item, CPoint(0, 0));

	return ret_item;
}

int CustomTabCtrl::DeleteItem(int item)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;

	try
	{
		if (ctrl_tool_tip_.m_hWnd)
		{
			for (int i=item; i<a_items_.GetSize(); i++)
			{
				ctrl_tool_tip_.DelTool(this, i+1);
				if (i!=a_items_.GetSize()-1)
				{
					CString s;
					ctrl_tool_tip_.GetText(s, this, i+2);
					ctrl_tool_tip_.AddTool(this, s, CRect(0, 0, 0, 0), i+1);
				}
			}
		}
	}
	catch(CMemoryException* e)
	{
		e->Delete();
		return CTCERR_OUTOFMEMORY;
	}

	if (a_items_.GetSize()==1)
		item_selected_ = -1;
	else if (item_selected_==item)
	{
		if (item_selected_==a_items_.GetSize()-1) // last item
		{
			item_selected_--;
			a_items_[item_selected_]->selected_ = true;	
		}
		else
			a_items_[item_selected_+1]->selected_ = true;	
	}
	else if (item_selected_>item)
		item_selected_--;

	delete a_items_[item];
	a_items_.RemoveAt(item);

	RecalcLayout(RECALC_RESIZED, item_selected_);
	Invalidate(false);
	return CTCERR_NOERROR;
}

void CustomTabCtrl::DeleteAllItems()
{
	if (ctrl_tool_tip_.m_hWnd)
	{
		for (int i=0; i< a_items_.GetSize(); i++)
		{
			delete a_items_[i];
			ctrl_tool_tip_.DelTool(this, i+1);
		}
	}
	else
	{
		for (int i=0; i< a_items_.GetSize(); i++)
			delete a_items_[i];
	}

	a_items_.RemoveAll();

	item_selected_ = -1;
		
	RecalcLayout(RECALC_RESIZED, item_selected_);
	Invalidate(false);
}

int CustomTabCtrl::SetCurSel(int item)
{
	return SetCurSel(item, false, false);
}

int CustomTabCtrl::HighlightItem(int item, bool highlight)
{
	if (!(GetStyle()&CTCS_MULTIHIGHLIGHT))
		return CTCERR_NOMULTIHIGHLIGHTSTYLE;
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	if (item_selected_==-1 && !highlight)
		return CTCERR_NOERROR;
	if (item_selected_==-1)
	{
		SetCurSel(item);
		return CTCERR_NOERROR;
	}
	if (highlight==a_items_[item]->highlighted_ || item==item_selected_)
		return CTCERR_NOERROR;
	
	a_items_[item]->highlighted_ = highlight;
	return CTCERR_NOERROR;
}

int CustomTabCtrl::GetItemText(int item, CString& text)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	text = a_items_[item]->text_;
	return CTCERR_NOERROR;
}

int CustomTabCtrl::SetItemText(int item, CString text)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	a_items_[item]->text_ = text;
	RecalcLayout(RECALC_RESIZED, item_selected_);
	Invalidate(false);
	return CTCERR_NOERROR;
}

int CustomTabCtrl::SetItemTooltipText(int item, CString text)
{
	if (!(GetStyle()&CTCS_TOOLTIPS))
		return CTCERR_NOTOOLTIPSSTYLE;
	if (item>=CTCID_CLOSEBUTTON && item<a_items_.GetSize())
	{
		if (ctrl_tool_tip_.m_hWnd==NULL)
		{
			if (!ctrl_tool_tip_.Create(this))
				return CTCERR_CREATETOOLTIPFAILED;
			ctrl_tool_tip_.Activate(true);
		}
		if (item>=0)
			item++;
		ctrl_tool_tip_.DelTool(this, item);
		ctrl_tool_tip_.AddTool(this, text, CRect(0, 0, 0, 0), item);
		RecalcLayout(RECALC_RESIZED, item_selected_);
		Invalidate(false);
		return CTCERR_NOERROR;
	}
	return CTCERR_INDEXOUTOFRANGE;
}

int CustomTabCtrl::GetItemData(int item, DWORD& data)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	data = a_items_[item]->param_;
	return CTCERR_NOERROR;
}

int CustomTabCtrl::GetItemRect(int item, CRect& rect) const
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	rect = a_items_[item]->text_rect_;
	return CTCERR_NOERROR;
}

int CustomTabCtrl::SetItemData(int item, DWORD data)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	a_items_[item]->param_ = data;
	return CTCERR_NOERROR;
}

int CustomTabCtrl::IsItemHighlighted(int item)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	return (a_items_[item]->highlighted_)?1:0;
}

int	CustomTabCtrl::HitTest(CPoint pt)
{
	CRect cl= GetTabArea();
//	GetClientRect(&cl);
	if (IsVertical())
	{
		cl.SetRect(0, 0, cl.Height(), cl.Width());
		pt = CPoint(cl.Width()-pt.y, pt.x);
	}

	int btns = 0;
	if (close_state_)
		btns++;
	if (prev_state_)
		btns += 2;
	if (first_state_)
		btns += 2;
	int a = cl.Height()-3;

	int close_offset = 0;
	if (close_state_)
	{
		close_offset = a;
		CRect close(0, 0, a+1, cl.Height());
		if (GetStyle()&CTCS_BUTTONSAFTER)
			close.SetRect(cl.Width()-a-1, 0, cl.Width(), cl.Height());
		if (close.PtInRect(pt))
			return CTCHT_ONCLOSEBUTTON;
	}
	CRect first, prev, next, last;
	if (GetStyle()&CTCS_BUTTONSAFTER)
	{
		if (btns==2||btns==3)
		{
			next.SetRect(cl.Width()-close_offset-a-1, 0, cl.Width()-close_offset, cl.Height());
			prev.SetRect(cl.Width()-close_offset-2*a-3, 0, cl.Width()-close_offset-a-1, cl.Height());
		}
		else if (btns==4||btns==5)
		{
			last.SetRect(cl.Width()-close_offset-a-1, 0, cl.Width()-close_offset, cl.Height());
			next.SetRect(cl.Width()-close_offset-2*a-1, 0, cl.Width()-close_offset-a-1, cl.Height());
			prev.SetRect(cl.Width()-close_offset-3*a-2, 0, cl.Width()-close_offset-2*a-1, cl.Height());
			first.SetRect(cl.Width()-close_offset-4*a-3, 0, cl.Width()-close_offset-3*a-2, cl.Height());
		}
	}
	else
	{
		if (btns==2||btns==3)
		{
			prev.SetRect(close_offset, 0, close_offset+a+1, cl.Height());
			next.SetRect(close_offset+a+1, 0, close_offset+2*a+3, cl.Height());
		}
		else if (btns==4||btns==5)
		{
			first.SetRect(close_offset, 0, close_offset+a+1, cl.Height());
			prev.SetRect(close_offset+a+1, 0, close_offset+2*a+1, cl.Height());
			next.SetRect(close_offset+2*a+1, 0, close_offset+3*a+2, cl.Height());
			last.SetRect(close_offset+3*a+2, 0, close_offset+4*a+3, cl.Height());
		}
	}

	if (btns>=4 && first_state_ && first.PtInRect(pt))
		return CTCHT_ONFIRSTBUTTON;

	if (prev_state_ && prev.PtInRect(pt))
		return CTCHT_ONPREVBUTTON;

	if (next_state_ && next.PtInRect(pt))
		return CTCHT_ONNEXTBUTTON;

	if (btns>=4 && last_state_ && last.PtInRect(pt))
		return CTCHT_ONLASTBUTTON;

	for (int i=0; i<a_items_.GetSize(); i++)
	{
		if (a_items_[i]->HitTest(pt))
			return i;
	}
	return CTCHT_NOWHERE;
}

int CustomTabCtrl::HighlightItem(int item, bool mouse_sel, bool ctrl_pressed)
{
	if (!(GetStyle()&CTCS_MULTIHIGHLIGHT))
		return CTCERR_NOMULTIHIGHLIGHTSTYLE;

	for (int i=0; i<a_items_.GetSize();i++)
		a_items_[i]->highlight_changed_ = false;

	if (ctrl_pressed)
	{
		if (item!=item_selected_)
		{
			a_items_[item]->highlighted_ = !a_items_[item]->highlighted_;
			if (mouse_sel)
				a_items_[item]->highlight_changed_ = true;
		}
	}
	else if (!a_items_[item]->highlighted_)
	{
		a_items_[item]->highlighted_ = true;
		a_items_[item]->highlight_changed_ = true;
		for (int i=0;i<a_items_.GetSize();i++)
		{
			if (i!=item_selected_)
			{
				if (a_items_[i]->highlighted_)
				{
					a_items_[i]->highlighted_ = false;
					if (mouse_sel)
						a_items_[i]->highlight_changed_ = true;
				}
			}
		}
	}
	if (mouse_sel)
		RecalcLayout(RECALC_ITEM_SELECTED, item);
	Invalidate(false);
	return CTCERR_NOERROR;
}

int CustomTabCtrl::SetCurSel(int item, bool mouse_sel, bool ctrl_pressed)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;

	if (item_selected_!=-1)
		a_items_[item_selected_]->selected_ = false;

	item_selected_ = item;
	
	if (item_selected_!=-1)
		a_items_[item_selected_]->selected_ = true;

	if (mouse_sel)
		RecalcLayout(RECALC_ITEM_SELECTED, item_selected_);
	else
	{
		item_ndx_offset_ = item;
		RecalcLayout(RECALC_RESIZED, item_selected_);
	}
	Invalidate(false);
	HighlightItem(item, mouse_sel, ctrl_pressed);
	return CTCERR_NOERROR;
}

void CustomTabCtrl::RecalcLayout(int recalc_type, int item)
{
	CRect cl= GetTabArea();
//	GetClientRect(&cl);
	if (IsVertical())
		cl.SetRect(0, 0, cl.Height(), cl.Width());

	int close_offset = 0;
	int a = cl.Height()-3;
	int bn_width = 0;
	int btns = 0;
	if (GetStyle()&CTCS_CLOSEBUTTON)
	{
		btns++;
		close_offset = a;
		bn_width = a+3;
		if (close_state_==BNST_INVISIBLE)
			close_state_ = BNST_NORMAL;
	}
	else
		close_state_ = BNST_INVISIBLE;

	int width = RecalcRectangles();

	if ((GetStyle()&CTCS_AUTOHIDEBUTTONS) && (a_items_.GetSize()<2 || width <= cl.Width()-bn_width))
	{
		first_state_ = BNST_INVISIBLE;
		prev_state_ = BNST_INVISIBLE;
		next_state_ = BNST_INVISIBLE;
		last_state_ = BNST_INVISIBLE;
		item_ndx_offset_ = 0;
		RecalcOffset(bn_width);
		if (recalc_type==RECALC_EDIT_RESIZED)
			RecalcEditResized(0, item);
		
		if (ctrl_tool_tip_.m_hWnd)
		{
			ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(0, 0, 0, 0));
			ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(0, 0, 0, 0));
			ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(0, 0, 0, 0));
			ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(0, 0, 0, 0));
			if (GetStyle()&CTCS_CLOSEBUTTON)
			{
				if (IsVertical())
				{
					if (GetStyle()&CTCS_BUTTONSAFTER)
						ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(0, 0, cl.Height(), a+1));
					else
						ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(0, cl.Width()-a-1, cl.Height(), cl.Width()));
				}
				else
				{
					if (GetStyle()&CTCS_BUTTONSAFTER)
						ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(cl.Width()-a-1, 0, cl.Width(), cl.Height()));
					else
						ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(0, 0, a+1, cl.Height()));
				}
			}

		}
		return;
	}
	
	if (prev_state_==BNST_INVISIBLE)
	{
		prev_state_ = BNST_NORMAL;
		next_state_ = BNST_NORMAL;
	}
	if (GetStyle()&CTCS_FOURBUTTONS)
	{
		btns += 4;
		if (first_state_==BNST_INVISIBLE)
		{
			first_state_ = BNST_NORMAL;
			last_state_ = BNST_NORMAL;
		}
	}
	else
	{
		btns += 2;
		first_state_ = BNST_INVISIBLE;
		last_state_ = BNST_INVISIBLE;
	}
	
	if (ctrl_tool_tip_.m_hWnd)
	{
		if (GetStyle()&CTCS_BUTTONSAFTER)
		{
			if (IsVertical())
			{
				if (btns<4)
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(0, 0, 0, 0));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(0, close_offset+a+1, cl.Height(), close_offset+2*a+3));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(0, close_offset, cl.Height(), close_offset+a+1));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(0, 0, 0, 0));
				}
				else
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(0, close_offset+3*a+2, cl.Height(), close_offset+4*a+3));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(0, close_offset+2*a+1, cl.Height(), close_offset+3*a+2));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(0, close_offset+a+1, cl.Height(), close_offset+2*a+1));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(0, close_offset, cl.Height(), close_offset+a+1));
				}
				if (GetStyle()&CTCS_CLOSEBUTTON)
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(0, 0, cl.Height(), a+1));
			}
			else
			{
				if (btns<4)
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(0, 0, 0, 0));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(cl.Width()-close_offset-2*a-3, 0, cl.Width()-close_offset-a-1, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(cl.Width()-close_offset-a-1, 0, cl.Width()-close_offset, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(0, 0, 0, 0));
				}
				else
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(cl.Width()-close_offset-4*a-3, 0, cl.Width()-close_offset-3*a-2, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(cl.Width()-close_offset-3*a-2, 0, cl.Width()-close_offset-2*a-1, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(cl.Width()-close_offset-2*a-1, 0, cl.Width()-close_offset-a-1, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(cl.Width()-close_offset-a-1, 0, cl.Width()-close_offset, cl.Height()));
				}
				if (GetStyle()&CTCS_CLOSEBUTTON)
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(cl.Width()-a-1, 0, cl.Width(), cl.Height()));
			}
		}
		else
		{
			if (IsVertical())
			{
				if (btns<4)
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(0, 0, 0, 0));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(0, cl.Width()-close_offset-a-1, cl.Height(), cl.Width()-close_offset));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(0, cl.Width()-close_offset-2*a-3, cl.Height(), cl.Width()-close_offset-a-1));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(0, 0, 0, 0));
				}
				else
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(0, cl.Width()-close_offset-a-1, cl.Height(), cl.Width()-close_offset));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(0, cl.Width()-close_offset-2*a-3, cl.Height(), cl.Width()-close_offset-a-1));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(0, cl.Width()-close_offset-3*a-2, cl.Height(), cl.Width()-close_offset-2*a-1));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(0, cl.Width()-close_offset-4*a-3, cl.Height(), cl.Width()-close_offset-3*a-2));
				}
				if (GetStyle()&CTCS_CLOSEBUTTON)
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(0, cl.Width()-a-1, cl.Height(), cl.Width()));
			}
			else
			{
				if (btns<4)
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(0, 0, 0, 0));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(close_offset, 0, close_offset+a+1, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(close_offset+a+1, 0, close_offset+2*a+3, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(0, 0, 0, 0));
				}
				else
				{
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_FIRSTBUTTON, CRect(close_offset, 0, close_offset+a+1, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_PREVBUTTON, CRect(close_offset+a+1, 0, close_offset+2*a+3, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_NEXTBUTTON, CRect(close_offset+2*a+1, 0, close_offset+3*a+2, cl.Height()));
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_LASTBUTTON, CRect(close_offset+3*a+2, 0, close_offset+4*a+3, cl.Height()));
				}
				if (GetStyle()&CTCS_CLOSEBUTTON)
					ctrl_tool_tip_.SetToolRect(this, (UINT)CTCID_CLOSEBUTTON, CRect(0, 0, a+1, cl.Height()));
			}
		}
	}

	if (a_items_.GetSize()==0)
		return;

	bn_width = btns*a+3;
	if (GetStyle()&CTCS_BUTTONSAFTER)
		cl.right -= bn_width;
	switch (recalc_type)
	{
	case RECALC_CLOSE_PRESSED:
		RecalcRectangles();
		RecalcOffset(bn_width);
		break;
	case RECALC_FIRST_PRESSED:
		{
			item_ndx_offset_=0;
			RecalcRectangles();
			RecalcOffset(bn_width);
		}
		break;
	case RECALC_PREV_PRESSED:
		{	
			RecalcOffset(bn_width);
			if (item_ndx_offset_>0)
			{
				item_ndx_offset_--;
				RecalcRectangles();
				RecalcOffset(bn_width);
			}
		}
		break;
	case RECALC_NEXT_PRESSED:
		{	
			RecalcOffset(bn_width);
			if (a_items_[a_items_.GetSize()-1]->rect_.right>cl.Width() && item_ndx_offset_!=a_items_.GetSize()-1)
			{
				item_ndx_offset_++;
				RecalcRectangles();
				RecalcOffset(bn_width);
			}
		}
		break;
	case RECALC_ITEM_SELECTED:
		{
			RecalcOffset(bn_width);
			if (a_items_[item]->shape_==TAB_SHAPE2 || a_items_[item]->shape_==TAB_SHAPE3)
			{
				item_ndx_offset_--;
				RecalcRectangles();
				RecalcOffset(bn_width);
			}
			else
			{
				while(item_ndx_offset_<item && 
						a_items_[item]->shape_==TAB_SHAPE4 && 
						a_items_[item]->rect_.right>cl.Width() && 
						a_items_[item]->rect_.left>((GetStyle()&CTCS_BUTTONSAFTER)?0:bn_width))
				{
					item_ndx_offset_++;
					RecalcRectangles();
					RecalcOffset(bn_width);
				}
			}
		}
		break;
	case RECALC_EDIT_RESIZED:
		{
			RecalcOffset(bn_width);
			RecalcEditResized(bn_width, item);
		}
		break;
	case RECALC_LAST_PRESSED:
		{
			item_ndx_offset_=a_items_.GetSize()-1;
		}
	default:	// window resized
		{
			bool ndx_offset_changed = false;
			RecalcOffset(bn_width);
			while(item_ndx_offset_>=0 && a_items_[a_items_.GetSize()-1]->rect_.right<cl.Width())
			{
				item_ndx_offset_--;
				if (item_ndx_offset_>=0)
				{
					RecalcRectangles();
					RecalcOffset(bn_width);
				}
				ndx_offset_changed = true;
			}
			if (ndx_offset_changed)
			{
				item_ndx_offset_++;
				RecalcRectangles();
				RecalcOffset(bn_width);
			}
		}
		break;
	}
}

void CustomTabCtrl::RecalcEditResized(int offset, int item)
{
	CRect cl= GetTabArea();
//	GetClientRect(cl);
	if (GetStyle()&CTCS_BUTTONSAFTER)
		cl.right -= offset;
	do
	{
		CRect r(0, 0, 0, 0);
		int h= 0;
		{
			CClientDC dc(this);
//		CDC* dc = GetDC();
			CFont* old_font = dc.SelectObject(&font_selected_);
			h = dc.DrawText(a_items_[item]->text_ + _T("X"), r, DT_CALCRECT);
			dc.SelectObject(old_font);
		}
		//ReleaseDC(dc);
		r = a_items_[item]->text_rect_;
		if (r.Height()>h)
		{
			r.top += (r.Height()-h)/2;
			r.bottom -= (r.Height()-h)/2;
		}
		r.left += 2;
		if (r.right>cl.right && item_selected_>item_ndx_offset_)
		{
			item_ndx_offset_++;
			RecalcRectangles();
			RecalcOffset(offset);
		}
		else
		{
			if (r.right>cl.right)
				r.right = cl.right;
			ctrl_edit_.MoveWindow(r);
			int n = a_items_[item]->text_.GetLength();
			int start, end;
			ctrl_edit_.GetSel(start, end);
			if (start==end && start==n)
			{
				ctrl_edit_.SetSel(0, 0);
				ctrl_edit_.SetSel(n, n);
			}
			return;
		}
	} 
	while(1);
}


CRect CustomTabCtrl::GetTabArea() const
{
	CRect client(0, 0, 0, 0);
	GetClientRect(client);
	if (client.IsRectEmpty())
		return client;
	client.top += top_margin_;
	client.bottom -= divider_height_ + bottom_margin_;
	if (client.Height() < 0)
		client.bottom = client.top;
	return client;
}


void CustomTabCtrl::RecalcOffset(int offset)
{
	CRect cl= GetTabArea();

	if (IsVertical())
		cl.SetRect(0, 0, cl.Height(), cl.Width());
	
	int rightAdjusment = 0;

	if (GetStyle() & CTCS_BUTTONSAFTER)
	{
		rightAdjusment = offset;
		offset = 6;	// margin
	}

	for (int i = 0; i < a_items_.GetSize(); i++)
	{
		if (i < item_ndx_offset_ - 1)
		{
			a_items_[i]->shape_ = TAB_SHAPE1;
			offset -= a_items_[i]->rect_.Width() - cl.Height() / 2;
			a_items_[i]->text_rect_.SetRectEmpty();
		}
		else if (i == item_ndx_offset_ - 1)
		{
			int btns = 2;
			if (GetStyle() & CTCS_FOURBUTTONS)
				btns = 4;
			if (GetStyle() & CTCS_CLOSEBUTTON)
				btns++;
			int bn_width = btns * (cl.Height() - 3) + 3;
			if (i == item_selected_)
				a_items_[i]->shape_ = TAB_SHAPE2;
			else
				a_items_[i]->shape_ = TAB_SHAPE3;
			offset -= a_items_[i]->rect_.Width() - cl.Height() / 2;
			a_items_[i]->rect_.SetRect(0, cl.top + 1, cl.Height() / 2, cl.bottom - 1);
			if (!(GetStyle() & CTCS_BUTTONSAFTER))
				a_items_[i]->rect_.OffsetRect(bn_width, 0);
			a_items_[i]->text_rect_.SetRectEmpty();
		}
		else
		{
			if (i == item_selected_)
				a_items_[i]->shape_ = TAB_SHAPE4;
			else if (i == a_items_.GetSize() - 1)	// last item
				a_items_[i]->shape_ = TAB_SHAPE4;
			else
				a_items_[i]->shape_ = TAB_SHAPE5;

			a_items_[i]->rect_.OffsetRect(offset, 0);
			a_items_[i]->text_rect_.OffsetRect(offset, 0);
		}

		a_items_[i]->ComputeRgn(!!(GetStyle() & CTCS_TOP));

		if (ctrl_tool_tip_.m_hWnd)
		{
			CRect t = a_items_[i]->text_rect_;
			if (t.left >= cl.Width() - rightAdjusment)
				t.SetRect(0, 0, 0, 0);
			else if (t.right > cl.Width() - rightAdjusment)
				t.right = cl.right - rightAdjusment;
			if (IsVertical())
				t.SetRect(0, cl.Width() - t.right, cl.Height(), cl.Width() - t.left);
			ctrl_tool_tip_.SetToolRect(this, i + 1, t);
		}
	}
}

int CustomTabCtrl::RecalcRectangles()
{
	CRect cl= GetTabArea();
//	GetClientRect(&cl);
	if (IsVertical())
		cl.SetRect(0, 0, cl.Height(), cl.Width());
	bool top = !!(GetStyle() & CTCS_TOP);
	int width = 0;
	
	{
		// calculate width

		int offset = 0;
		CRect rcText;
		CDC* dc = GetDC();
		CFont* old_font = dc->SelectObject(&font_selected_);
		if (GetStyle() & CTCS_FIXEDWIDTH)
		{
			int max_width=0;
			for (int i=0; i<a_items_.GetSize(); i++)
			{
				int w=0;
				int h = dc->DrawText(a_items_[i]->text_ + _T("X"), rcText, DT_CALCRECT | DT_SINGLELINE);
				if (h>0)
					w = rcText.Width();
				if (w>max_width)
					max_width = w;
			}
			for (int i=0; i<a_items_.GetSize(); i++)
			{
				if (top)
				{
					a_items_[i]->rect_ = CRect(0, cl.top, max_width + cl.Height()+4, cl.bottom - 1);
					a_items_[i]->text_rect_ = CRect(cl.Height()/2, 0, max_width+cl.Height()/2+4, cl.Height()-1);
				}
				else
				{
					a_items_[i]->rect_ = CRect(0, 1, max_width+cl.Height()+4, cl.Height()-1);
					a_items_[i]->text_rect_ = CRect(cl.Height()/2, 1, max_width+cl.Height()/2+4, cl.Height()-1);
				}
				a_items_[i]->rect_ += CPoint(offset, 0);
				a_items_[i]->text_rect_ += CPoint(offset, 0);

				offset += a_items_[i]->rect_.Width()-cl.Height()/2;
				width = a_items_[i]->rect_.right;
			}
		}
		else
		{
			for (int i= 0; i < a_items_.GetSize(); i++)
			{
				int w=0;
				int h = dc->DrawText(a_items_[i]->text_ + _T("X"), rcText, DT_CALCRECT | DT_SINGLELINE);
				if (h>0)
					w = rcText.Width();
				if (top)
				{
					a_items_[i]->rect_ = CRect(0, cl.top, w + cl.Height() + 4, cl.bottom - 1);
					a_items_[i]->text_rect_ = CRect(cl.Height() / 2, cl.top, w + cl.Height() / 2 + 4, cl.bottom - 1);
				}
				else
				{
					a_items_[i]->rect_ = CRect(0, 1, w+cl.Height()+4, cl.Height()-1);
					a_items_[i]->text_rect_ = CRect(cl.Height()/2, 1, w+cl.Height()/2+4, cl.Height()-1);
				}
				
				a_items_[i]->rect_ += CPoint(offset, 0);
				a_items_[i]->text_rect_ += CPoint(offset, 0);

				offset += a_items_[i]->rect_.Width()-cl.Height()/2;
				width = a_items_[i]->rect_.right;

			}
		}
		dc->SelectObject(old_font);
		ReleaseDC(dc);
	}

	return width;
}

BOOL CustomTabCtrl::PreTranslateMessage(MSG* msg)
{
	if (GetStyle()&CTCS_TOOLTIPS && ctrl_tool_tip_.m_hWnd && 
		(msg->message==WM_LBUTTONDOWN || msg->message==WM_LBUTTONUP || msg->message==WM_MOUSEMOVE))
			ctrl_tool_tip_.RelayEvent(msg);

	return CWnd::PreTranslateMessage(msg);
}

void CustomTabCtrl::DrawBk(CDC& dc, CRect& r, HBITMAP bmp, bool is_image_hor_layout, MY_MARGINS& mrgn, int image_ndx)
{
	CDC mem_dc;
	mem_dc.CreateCompatibleDC(&dc);
	CBitmap* bitmap= CBitmap::FromHandle(bmp);
	BITMAP bm;
	bitmap->GetBitmap(&bm);
	CBitmap* old_bmp = mem_dc.SelectObject(bitmap);
	if (is_image_hor_layout)
	{
		// left-top
		dc.BitBlt(r.left,
				r.top,
				mrgn.cxLeftWidth,
				mrgn.cyTopHeight,
				&mem_dc,
				image_ndx*bm.bmWidth/4,
				0,
				SRCCOPY);
		
		// right-top
		dc.BitBlt(r.right-mrgn.cxRightWidth,
				r.top,
				mrgn.cxRightWidth,
				mrgn.cyTopHeight,
				&mem_dc,
				(image_ndx+1)*bm.bmWidth/4-mrgn.cxRightWidth,
				0,
				SRCCOPY);

		// left-bottom
		dc.BitBlt(r.left,
				r.bottom-mrgn.cyBottomHeight,
				mrgn.cxLeftWidth,
				mrgn.cyBottomHeight,
				&mem_dc,
				image_ndx*bm.bmWidth/4,
				bm.bmHeight-mrgn.cyBottomHeight,
				SRCCOPY);

		// right-bottom
		dc.BitBlt(r.right-mrgn.cxRightWidth,
				r.bottom-mrgn.cyBottomHeight,
				mrgn.cxRightWidth,
				mrgn.cyBottomHeight,
				&mem_dc,
				(image_ndx+1)*bm.bmWidth/4-mrgn.cxRightWidth,
				bm.bmHeight-mrgn.cyBottomHeight,
				SRCCOPY);

		// middle-top
		dc.StretchBlt(r.left+mrgn.cxLeftWidth,
			r.top,
			r.Width()-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyTopHeight,
			&mem_dc,
			image_ndx*bm.bmWidth/4+mrgn.cxLeftWidth,
			0,
			bm.bmWidth/4-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyTopHeight,
			SRCCOPY);

		// middle-bottom
		dc.StretchBlt(r.left+mrgn.cxLeftWidth,
			r.bottom-mrgn.cyBottomHeight,
			r.Width()-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyBottomHeight,
			&mem_dc,
			image_ndx*bm.bmWidth/4+mrgn.cxLeftWidth,
			bm.bmHeight-mrgn.cyBottomHeight,
			bm.bmWidth/4-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyBottomHeight,
			SRCCOPY);

		// middle-left
		dc.StretchBlt(r.left,
			r.top+mrgn.cyTopHeight,
			mrgn.cxLeftWidth,
			r.Height()-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			&mem_dc,
			image_ndx*bm.bmWidth/4,
			mrgn.cyTopHeight,
			mrgn.cxLeftWidth,
			bm.bmHeight-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			SRCCOPY);

		// middle-right
		dc.StretchBlt(r.right-mrgn.cxRightWidth,
			r.top+mrgn.cyTopHeight,
			mrgn.cxRightWidth,
			r.Height()-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			&mem_dc,
			(image_ndx+1)*bm.bmWidth/4-mrgn.cxRightWidth,
			mrgn.cyTopHeight,
			mrgn.cxRightWidth,
			bm.bmHeight-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			SRCCOPY);

		// middle
		dc.StretchBlt(
			r.left+mrgn.cxLeftWidth,
			r.top+mrgn.cyTopHeight,
			r.Width()-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			r.Height()-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			&mem_dc,
			image_ndx*bm.bmWidth/4 + mrgn.cxLeftWidth,
			mrgn.cyTopHeight,
			bm.bmWidth/4-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			bm.bmHeight-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			SRCCOPY);
	}
	else
	{
		// left-top
		dc.BitBlt(r.left,
				r.top,
				mrgn.cxLeftWidth,
				mrgn.cyTopHeight,
				&mem_dc,
				0,
				image_ndx*bm.bmHeight/4,
				SRCCOPY);
		
		// right-top
		dc.BitBlt(r.right-mrgn.cxRightWidth,
				r.top,
				mrgn.cxRightWidth,
				mrgn.cyTopHeight,
				&mem_dc,
				bm.bmWidth-mrgn.cxRightWidth,
				image_ndx*bm.bmHeight/4,
				SRCCOPY);
		
		// left-bottom
		dc.BitBlt(r.left,
				r.bottom-mrgn.cyBottomHeight,
				mrgn.cxLeftWidth,
				mrgn.cyBottomHeight,
				&mem_dc,
				0,
				(image_ndx+1)*bm.bmHeight/4-mrgn.cyBottomHeight,
				SRCCOPY);

		// right-bottom
		dc.BitBlt(r.right-mrgn.cxRightWidth,
				r.bottom-mrgn.cyBottomHeight,
				mrgn.cxRightWidth,
				mrgn.cyBottomHeight,
				&mem_dc,
				bm.bmWidth-mrgn.cxRightWidth,
				(image_ndx+1)*bm.bmHeight/4-mrgn.cyBottomHeight,
				SRCCOPY);

		// middle-top
		dc.StretchBlt(r.left+mrgn.cxLeftWidth,
			r.top,
			r.Width()-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyTopHeight,
			&mem_dc,
			mrgn.cxLeftWidth,
			image_ndx*bm.bmHeight/4,
			bm.bmWidth-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyTopHeight,
			SRCCOPY);

		// middle-bottom
		dc.StretchBlt(r.left+mrgn.cxLeftWidth,
			r.bottom-mrgn.cyBottomHeight,
			r.Width()-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyBottomHeight,
			&mem_dc,
			mrgn.cxLeftWidth,
			(image_ndx+1)*bm.bmHeight/4-mrgn.cyBottomHeight,
			bm.bmWidth-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			mrgn.cyBottomHeight,
			SRCCOPY);

		// middle-left
		dc.StretchBlt(r.left,
			r.top+mrgn.cyTopHeight,
			mrgn.cxLeftWidth,
			r.Height()-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			&mem_dc,
			0,
			image_ndx*bm.bmHeight/4+mrgn.cyTopHeight,
			mrgn.cxLeftWidth,
			bm.bmHeight/4-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			SRCCOPY);

		// middle-right
		dc.StretchBlt(r.right-mrgn.cxRightWidth,
			r.top+mrgn.cyTopHeight,
			mrgn.cxRightWidth,
			r.Height()-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			&mem_dc,
			bm.bmWidth-mrgn.cxRightWidth,
			image_ndx*bm.bmHeight/4+mrgn.cyTopHeight,
			mrgn.cxRightWidth,
			bm.bmHeight/4-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			SRCCOPY);

		// middle
		dc.StretchBlt(
			r.left+mrgn.cxLeftWidth,
			r.top+mrgn.cyTopHeight,
			r.Width()-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			r.Height()-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			&mem_dc,
			mrgn.cxLeftWidth,
			image_ndx*bm.bmHeight/4+mrgn.cyTopHeight,
			bm.bmWidth-mrgn.cxLeftWidth-mrgn.cxRightWidth,
			bm.bmHeight/4-mrgn.cyTopHeight-mrgn.cyBottomHeight,
			SRCCOPY);
	}
	mem_dc.SelectObject(old_bmp);
}


void CustomTabCtrl::DrawGlyph(CDC& dc, CPoint& pt, int image_ndx, int color_ndx)
{
	CDC mem_dc, mem_mono_dc;
	mem_dc.CreateCompatibleDC(&dc);
	mem_mono_dc.CreateCompatibleDC(&dc);

	CBitmap* old_bmp_glyph_mono = mem_mono_dc.SelectObject(&glyphs_mono_bmp_);

	CBitmap glyph_color_bmp;
	glyph_color_bmp.CreateCompatibleBitmap(&dc, 8, 7);
	
	CBitmap* old_bmp_glyph_color = mem_dc.SelectObject(&glyph_color_bmp);

	COLORREF rgb_old_text_glyph =  mem_dc.SetTextColor(rgb_glyph_[color_ndx]);
	mem_dc.BitBlt(0, 0, 8, 7, &mem_mono_dc, image_ndx*8, 0, SRCCOPY);
	mem_dc.SetTextColor(rgb_old_text_glyph);

	COLORREF rgb_old_bk = dc.SetBkColor(RGB(255, 255, 255));
	COLORREF rgb_old_text = dc.SetTextColor(RGB(0, 0, 0));
	dc.BitBlt(pt.x, pt.y, 8, 7, &mem_dc, 0, 0, SRCINVERT);
	dc.BitBlt(pt.x, pt.y, 8, 7, &mem_mono_dc, image_ndx*8, 0, SRCAND);
	dc.BitBlt(pt.x, pt.y, 8, 7, &mem_dc, 0, 0, SRCINVERT);

	mem_dc.SelectObject(old_bmp_glyph_color);
	mem_mono_dc.SelectObject(old_bmp_glyph_mono);
	dc.SetBkColor(rgb_old_bk);
	dc.SetTextColor(rgb_old_text);
}


bool CustomTabCtrl::ModifyStyle(DWORD remove, DWORD add, UINT flags)
{
	if (remove&CTCS_TOOLTIPS)
		ctrl_tool_tip_.DestroyWindow();
	if (remove&CTCS_MULTIHIGHLIGHT)
	{
		for (int i=0;i<a_items_.GetSize();i++)
			a_items_[i]->highlighted_ = false;
	}
	if (add&CTCS_MULTIHIGHLIGHT)
	{
		for (int i=0;i<a_items_.GetSize();i++)
		{
			if (i==item_selected_)
				a_items_[i]->highlighted_ = true;
		}
	}
	CWnd::ModifyStyle(remove, add, flags);
	RecalcLayout(RECALC_RESIZED, item_selected_);
	Invalidate(false);
	return true;
}


bool CustomTabCtrl::ModifyStyleEx(DWORD remove, DWORD add, UINT flags)
{
	CWnd::ModifyStyleEx(remove, add, flags);
	RecalcLayout(RECALC_RESIZED, item_selected_);
	Invalidate(false);
	return true;
}


void CustomTabCtrl::PreSubclassWindow() 
{
//	OnThemeChanged(0, 0);
	CWnd::ModifyStyle(0, WS_CLIPCHILDREN);
	RecalcLayout(RECALC_RESIZED, item_selected_);
	CWnd::PreSubclassWindow();
}


void CustomTabCtrl::SetDragCursors(HCURSOR cursor_move, HCURSOR cursor_copy)
{
	::DestroyCursor(cursor_move_);
	cursor_move_ = NULL;
	::DestroyCursor(cursor_copy_);
	cursor_copy_ = NULL;
	cursor_move_ = CopyCursor(cursor_move);
	cursor_copy_ = CopyCursor(cursor_copy);
}


void CustomTabCtrl::OnRButtonDown(UINT flags, CPoint point)
{
	NotifyParent(CTCN_RCLICK, HitTest(point), point);
	CWnd::OnRButtonDown(flags, point);
}


void CustomTabCtrl::OnRButtonDblClk(UINT flags, CPoint point) 
{
	NotifyParent(CTCN_RDBLCLK, HitTest(point), point);
	CWnd::OnRButtonDblClk(flags, point);
}


int CustomTabCtrl::EditLabel(int item)
{
	return EditLabel(item, false);
}


int CustomTabCtrl::EditLabel(int item, bool mouse_sel)
{
	if (item<0 || item>=a_items_.GetSize())
		return CTCERR_INDEXOUTOFRANGE;
	if (!(GetStyle()&CTCS_EDITLABELS))
		return CTCERR_NOEDITLABELSTYLE;
	if (item!=item_selected_)
		return CTCERR_ITEMNOTSELECTED;
	if (ctrl_edit_.m_hWnd)
		return CTCERR_ALREADYINEDITMODE;
	if (IsVertical())
		return CTCERR_EDITNOTSUPPORTED;
	try
	{
		CRect r(0, 0, 0, 0);
		int h= 0;
		{
			CClientDC dc(this); //* dc = GetDC();
			CFont* old_font = dc.SelectObject(&font_selected_);
			h = dc.DrawText(a_items_[item]->text_, r, DT_CALCRECT);
			dc.SelectObject(old_font);
		}
		//ReleaseDC(dc);
		r = a_items_[item]->text_rect_;
		if (r.Height()>h)
		{
			r.top += (r.Height()-h)/2;
			r.bottom -= (r.Height()-h)/2;
		}
		r.left += 2;
		if (ctrl_edit_.Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, r, this, CTCID_EDITCTRL))
		{
			CString old = a_items_[item]->text_;
			ctrl_edit_.SetFont(&font_selected_, false);
			ctrl_edit_.SetLimitText(MAX_LABEL_TEXT);
			ctrl_edit_.SetWindowText(a_items_[item]->text_);
			ctrl_edit_.SetFocus();
			ctrl_edit_.SetSel(0, -1);
			if (mouse_sel)
				ReleaseCapture();
			for (;;) 
			{
				MSG msg;
				::GetMessage(&msg, NULL, 0, 0);

				switch (msg.message) 
				{
				case WM_SYSKEYDOWN:
					{
						if (msg.wParam == VK_F4 && msg.lParam&29)
							break;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					break;
				case WM_KEYDOWN:
					{
						if (msg.wParam == VK_ESCAPE)
						{
							a_items_[item]->text_ = old;
							ctrl_edit_.DestroyWindow();
							RecalcLayout(RECALC_RESIZED, item_selected_);
							Invalidate(false);
							return CTCERR_NOERROR;
						}
						if (msg.wParam == VK_RETURN)
						{
							if (NotifyParent(CTCN_LABELUPDATE, item, CPoint(0, 0)))
								break;
							ctrl_edit_.GetWindowText(a_items_[item]->text_);
							ctrl_edit_.DestroyWindow();
							RecalcLayout(RECALC_RESIZED, item);
							Invalidate(false);
							return CTCERR_NOERROR;
						}
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					break;
				case WM_LBUTTONDOWN:
					{
						if (msg.hwnd==m_hWnd)
						{
							POINTS pt = MAKEPOINTS(msg.lParam);
							if (HitTest(CPoint(pt.x, pt.y))!=item_selected_)
							{
								if (NotifyParent(CTCN_LABELUPDATE, item, CPoint(0, 0)))
									break;
								ctrl_edit_.GetWindowText(a_items_[item_selected_]->text_);
								ctrl_edit_.DestroyWindow();
								TranslateMessage(&msg);
								DispatchMessage(&msg);
								return CTCERR_NOERROR;
							}
						}
						else if (msg.hwnd==ctrl_edit_.m_hWnd)
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
						else
						{
							if (NotifyParent(CTCN_LABELUPDATE, item, CPoint(0, 0)))
								break;
							ctrl_edit_.GetWindowText(a_items_[item_selected_]->text_);
							ctrl_edit_.DestroyWindow();
							return CTCERR_NOERROR;
						}
					}
					break;
				case WM_LBUTTONUP:
					{
						if (msg.hwnd==ctrl_edit_.m_hWnd)
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
					break;
				case WM_NCLBUTTONDOWN:
					{
						if (NotifyParent(CTCN_LABELUPDATE, item, CPoint(0, 0)))
							break;
						ctrl_edit_.GetWindowText(a_items_[item_selected_]->text_);
						ctrl_edit_.DestroyWindow();
						TranslateMessage(&msg);
						DispatchMessage(&msg);
						return CTCERR_NOERROR;
					}
					break;
				case WM_LBUTTONDBLCLK:
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
					break;
				default:
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					break;
				}
			}
		}
	}
	catch(CMemoryException* e)
	{
		e->Delete();
		if (mouse_sel)
			NotifyParent(CTCN_OUTOFMEMORY, item, CPoint(0, 0));
		return CTCERR_OUTOFMEMORY;
	}
	return CTCERR_NOERROR;
}


void CustomTabCtrl::SetIdealHeight(int height)
{
	ideal_height_ = height;
}

int CustomTabCtrl::GetIdealHeight() const
{
	return ideal_height_;
}


void CustomTabCtrl::BlinkSelectedTab(int blinks)
{
	UpdateWindow();

	CClientDC dc(this);

	for (int blink= 0; blink < blinks; ++blink)
	{
		for (int tinge= 5; tinge < 100; tinge += 18)
		{
			DrawCtrl(dc, tinge);
			::Sleep(30);
		}

		for (int tinge= 100; tinge > 0; tinge -= 12)
		{
			DrawCtrl(dc, tinge);
			::Sleep(30);
		}
	}

	DrawCtrl(dc, 0);
}


void CustomTabCtrl::SetTabColor(COLORREF tab, COLORREF backgnd, COLORREF text_color)
{
	tab_color_ = tab;
	backgnd_color_ = backgnd;
	text_color_ = text_color;

	if (m_hWnd)
		Invalidate();
}
