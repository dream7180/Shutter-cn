/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ScriptEditCtrl.h"
#include "scintilla\include\SciLexer.h"
#include "ScintillaCtrl.h"
#include "UIElements.h"
#include "Attributes.h"
#include "Color.h"


extern COLORREF CalcColor(COLORREF rgb_color1, COLORREF rgb_color2, float bias);

struct ScriptEditCtrl::Impl : public CScintillaCtrl
{
	Impl() : self_(0), enable_input_attribs_(false)
	{}

	void virtual PreSubclassWindow();
	void OnNcCalcSize(BOOL calc_valid_rects, NCCALCSIZE_PARAMS* ncsp);
	void OnNcPaint();
	LRESULT OnNcHitTest(CPoint point);
	void OnSetFocus(CWnd* wnd);
	void OnKillFocus(CWnd* wnd);
	LRESULT ForceNCCalc(WPARAM, LPARAM);

	ScriptEditCtrl* self_;
	bool enable_input_attribs_;

	DECLARE_MESSAGE_MAP()
};

ScriptEditCtrl::ScriptEditCtrl() : impl_(new Impl())
{
	impl_->self_ = this;
}

ScriptEditCtrl::~ScriptEditCtrl()
{}

BEGIN_MESSAGE_MAP(ScriptEditCtrl::Impl, CScintillaCtrl)
	//{{AFX_MSG_MAP(ScriptEditCtrl)
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_MESSAGE(WM_APP+1234, ForceNCCalc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


static const int BORDER= 2;

LRESULT ScriptEditCtrl::Impl::ForceNCCalc(WPARAM, LPARAM)
{
	// force nonclient area recalc now, that our routines are hooked up, and override WM_NCCALCSIZE
	SetWindowPos(0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return 0;
}


void ScriptEditCtrl::Impl::OnNcCalcSize(BOOL calc_valid_rects, NCCALCSIZE_PARAMS* ncsp)
{
	if (calc_valid_rects)
		::InflateRect(ncsp->rgrc, -BORDER, -BORDER);
	Default();
}


static void FillSolidRect(HDC dc, int x, int y, int cx, int cy, COLORREF clr)
{
	::SetBkColor(dc, clr);
	CRect rect(x, y, x + cx, y + cy);
	::ExtTextOut(dc, 0, 0, ETO_OPAQUE, rect, 0, 0, 0);
}


void ScriptEditCtrl::Impl::OnSetFocus(CWnd* wnd)
{
	Default();
	// force nonclient area redraw after focus change to make frame reflect it
	RedrawWindow(0, 0, RDW_INVALIDATE | RDW_FRAME);
}


void ScriptEditCtrl::Impl::OnKillFocus(CWnd* wnd)
{
	Default();
	// force nonclient area redraw after focus change to make frame reflect it
	RedrawWindow(0, 0, RDW_INVALIDATE | RDW_FRAME);
}


void ScriptEditCtrl::Impl::OnNcPaint()
{
	if (HDC hdc= ::GetWindowDC(m_hWnd))
// below doesn't work in windows 7 anymore:
//	if (HDC hdc= ::GetDCEx(m_hWnd, reinterpret_cast<HRGN>(GetCurrentMessage()->wParam), DCX_WINDOW | DCX_INTERSECTRGN))
	{
		// draw scrollbar, if any
		Default();

		CRect rect;
		GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(-rect.left, -rect.top);

//		::FillSolidRect(hdc, rect.left, rect.top, rect.Width(), rect.Height(), ::GetSysColor(COLOR_WINDOW));
//		::FillSolidRect(hdc, rect.left, rect.top, rect.Width(), rect.Height(), RGB(255,0,0));

		//try
		//{
			DrawNCBorder(hdc, rect);
/*
			Gdiplus::Graphics g(hdc);

			COLORREF frame_clr= ::GetSysColor(COLOR_BTNSHADOW);
		//::FillSolidRect(hdc, rect.left, rect.top, 1, rect.Height(), frame);
		//::FillSolidRect(hdc, rect.left, rect.top, rect.Width(), 1, frame);
		//::FillSolidRect(hdc, rect.left, rect.bottom - 1, rect.Width(), 1, frame);
		//::FillSolidRect(hdc, rect.right - 1, rect.top, 1, rect.Height(), frame);

			COLORREF corner= CalcColor(frame_clr, ::GetSysColor(COLOR_3DFACE), 0.5);

//		::FillSolidRect(hdc, rect.left, rect.top, 1, 1, corner);
			g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

			Gdiplus::RectF area= CRectToRectF(rect);
			Gdiplus::RectF frame_rect= area;
			frame_rect.Width--;
			frame_rect.Height--;
			float radius= 1.1f;

			Gdiplus::GraphicsPath frame;
			RoundRect(frame, frame_rect, radius);

			Gdiplus::Color top= c2c(frame_clr);	// darker at the top
			Gdiplus::Color bottom= c2c(corner);
			Gdiplus::LinearGradientBrush brush(area, top, bottom, Gdiplus::LinearGradientModeVertical);
			Gdiplus::Color colors[]= { top, bottom, bottom };
			float positions[]= { 0.0f, 0.05f, 1.0f };
			brush.SetInterpolationColors(colors, positions, array_count(colors));
			Gdiplus::Pen pen(&brush);

			g.DrawPath(&pen, &frame); */
		//}
		//catch (...)
		//{
		//	ASSERT(false);
		//}

		::ReleaseDC(m_hWnd, hdc);
	}
}

LRESULT ScriptEditCtrl::Impl::OnNcHitTest(CPoint point)
{
	CRect rect(0,0,0,0);
	GetWindowRect(rect);

	rect.DeflateRect(BORDER, BORDER);

	if (!rect.PtInRect(point))
		return HTNOWHERE;	// frame

//	rect.left = rect.right - ::GetSystemMetrics(SM_CYVSCROLL);
//	rect.top = rect.bottom - ::GetSystemMetrics(SM_CXVSCROLL);

	return Default();
//	return rect.PtInRect(point) ? HTBOTTOMRIGHT : HTCLIENT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char LuaKeywords[]= "and break do else elseif end false for function if in local nil not or repeat return then true until while";

// Lua functions for syntax coloring; ExifPro doesn't use io/os libs (commented out in linit.c)
const char LuaFunctions[]=
"_G _VERSION assert collectgarbage dofile error getfenv getmetatable ipairs load loadfile loadstring module next pairs pcall "
"print rawequal rawget rawset require select setfenv setmetatable tonumber tostring type unpack xpcall gcinfo newproxy "

"coroutine.create coroutine.resume coroutine.running coroutine.status coroutine.wrap coroutine.yield "

"debug.debug debug.getfenv debug.gethook debug.getinfo debug.getlocal debug.getmetatable debug.getregistry "
"debug.getupvalue debug.setfenv debug.sethook debug.setlocal debug.setmetatable debug.setupvalue debug.traceback "

//"io.close io.flush io.input io.lines io.open io.output io.popen io.read io.stderr io.stdin io.stdout io.tmpfile io.type io.write "

"math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp math.floor math.fmod "
"math.frexp math.huge math.ldexp math.log math.log10 math.max math.min math.modf math.pi math.pow math.rad math.random "
"math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh math.mod "

//"os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale os.time os.tmpname "

"package.cpath package.loaded package.loaders package.loadlib package.path package.preload package.seeall package.config "

"string.byte string.char string.dump string.find string.format string.gmatch string.gsub string.len string.lower string.match "
"string.rep string.reverse string.sub string.upper string.gfind "

"table.concat table.insert table.maxn table.remove table.sort table.setn table.getn table.foreachi table.foreach "

"round";	// this is not a Lua fn, but it's defined within ExifPro cause it's useful

//"_LUA_VERSION"; doesn't seem to be available...

// user input fields
const char LuaUser[]= "text number";

//const char Attributes[]= "file.name";

std::string ConcatAttributes()
{
	std::ostringstream ost;
	USES_CONVERSION;

	for (size_t i= 0; i < ~0; ++i)
		if (const TCHAR* name= GetImageAttribName(i))
			ost << W2A(name) << ' ';
		else
			break;

	for (size_t i= 0; i < ~0; ++i)
		if (const TCHAR* name= GetMetaAttribName(i))
			ost << W2A(name) << ' ';
		else
			break;

	for (size_t i= 0; i < ~0; ++i)
		if (const TCHAR* name= GetFileAttribName(i))
			ost << W2A(name) << ' ';
		else
			break;

	return ost.str();
}


void ScriptEditCtrl::Impl::PreSubclassWindow()
{
	self_->m_hWnd = m_hWnd;	// expose HWND

	SetupDirectAccess();

	// If we are running as Unicode, then use the UTF8 codepage
#ifdef _UNICODE
	SetCodePage(SC_CP_UTF8);
#endif

	SetLexer(SCLEX_LUA);
	StyleSetFont(STYLE_DEFAULT, "Lucida Console");
	StyleSetSize(STYLE_DEFAULT, 10);
	SetKeyWords(0, LuaKeywords);
	SetKeyWords(1, LuaFunctions);
	if (enable_input_attribs_)
		SetKeyWords(2, LuaUser);
	SetKeyWords(3, ConcatAttributes().c_str());

	COLORREF comment= RGB(0,128,128);
	COLORREF string= RGB(128,128,0);

	StyleSetFore(SCE_LUA_COMMENT, comment);
	StyleSetFore(SCE_LUA_COMMENTLINE, comment);
	StyleSetFore(SCE_LUA_COMMENTDOC, comment);
	StyleSetFore(SCE_LUA_NUMBER, RGB(0,0,255));
	StyleSetFore(SCE_LUA_WORD, RGB(34,78,160));		// keywords
	StyleSetFore(SCE_LUA_STRING, string);
	StyleSetFore(SCE_LUA_CHARACTER, string);
	StyleSetFore(SCE_LUA_LITERALSTRING, string);
	StyleSetFore(SCE_LUA_WORD2, RGB(53,113,202));	// functions
	StyleSetFore(SCE_LUA_WORD3, RGB(124,37,203));	// test & number
	StyleSetBack(SCE_LUA_WORD3, CalcColor(::GetSysColor(COLOR_WINDOW), RGB(0,0,255), 0.95f));
	StyleSetFore(SCE_LUA_WORD4, RGB(164,97,49));	// todo: attributes

	StyleSetFont(SCE_LUA_WORD, "Lucida Console");
	StyleSetSize(SCE_LUA_WORD, 10);
	StyleSetBold(SCE_LUA_WORD, true);

	//#define SCE_LUA_PREPROCESSOR 9
	//#define SCE_LUA_OPERATOR 10
	//#define SCE_LUA_IDENTIFIER 11
	//#define SCE_LUA_STRINGEOL 12
	//#define SCE_LUA_WORD2 13
	//#define SCE_LUA_WORD3 14
	//#define SCE_LUA_WORD4 15
	//#define SCE_LUA_WORD5 16
	//#define SCE_LUA_WORD6 17
	//#define SCE_LUA_WORD7 18
	//#define SCE_LUA_WORD8 19

	Colorize(0, -1);

	//MarkerDefine(MARKER_POINTER, SC_MARK_ARROW);
	//MarkerSetBack(MARKER_POINTER, RGB(255,255,0));

	//MarkerDefine(MARKER_BREAKPOINT, SC_MARK_ROUNDRECT);
	//MarkerSetBack(MARKER_BREAKPOINT, RGB(0,0,255));

	//MarkerDefine(MARKER_ERROR, SC_MARK_ARROW);
	//MarkerSetBack(MARKER_ERROR, RGB(255,0,0));

	int width= 0;
	SetMarginWidthN(1, width);

	SetScrollWidthTracking(true);
	SetScrollWidth(1);

	SetWrapMode(SC_WRAP_WORD);
	SetWrapVisualFlags(SC_WRAPVISUALFLAG_END);

	SetSelBack(true, CalcShade(::GetSysColor(COLOR_HIGHLIGHT), 50.0f));

	SetTabWidth(4);
	SetIndent(4);

	DWORD pixels= 1;
	::SystemParametersInfo(SPI_GETCARETWIDTH, 0, &pixels, 0);
	SetCaretWidth(pixels);

	// hwnd is attached, but message processing is not working yet, we are not yet subclassed,
	// so postpone this WM_NCCALCSIZE call:
	PostMessage(WM_APP+1234);
}


CStringW ScriptEditCtrl::GetText()
{
	return impl_->GetText(impl_->GetTextLength() + 1);
}

int ScriptEditCtrl::GetWindowTextLength()
{
	return impl_->GetTextLength();
}

void ScriptEditCtrl::GetWindowText(CString& text)
{
	text = GetText();
}

void ScriptEditCtrl::SetWindowText(const TCHAR* text)
{
	impl_->SetText(text);
	impl_->EmptyUndoBuffer();
}

CWnd* ScriptEditCtrl::GetWnd()
{
	return impl_.get();
}

ScriptEditCtrl::operator CWnd& ()
{
	return *impl_;
}

void ScriptEditCtrl::SetFont(CFont* font)
{}

void ScriptEditCtrl::SetSel(int from, int end, bool no_scroll)
{
	impl_->SetSel(from, end);
}

void ScriptEditCtrl::ReplaceSel(const TCHAR* new_text, bool can_undo)
{
	impl_->ReplaceSel(new_text);
}

void ScriptEditCtrl::EnableInputAttribsColors(bool enable)
{
	impl_->enable_input_attribs_ = enable;
}
