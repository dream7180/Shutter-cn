/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

/////////////////////////////////////////////////////////////////////////////
// AboutDlg dialog used for App About

#include "stdafx.h"
#include "resource.h"
#include "AboutDlg.h"
#include "CatchAll.h"
//#include "PNGImage.h"
#include "UIElements.h"
#include "GetDefaultGuiFont.h"

//#define	WEB_SITE	L"https://github.com/dream7180/ExifPro-mod"
#define	GPL_SITE	L"https://www.gnu.org/licenses/gpl-3.0-standalone.html"
#define	WEB_SITE	L"https://gitee.com/dream7180/Shutter"
//#define	BLOG_SITE	L"https://www.cnblogs.com/foobox"
#define	CODE_SITE	L"https://github.com/mikekov/ExifPro"
//#define	OFF_SITE	L"http://www.exifpro.com/"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


AboutDlg::AboutDlg() : CDialog(AboutDlg::IDD)
{
	//{{AFX_DATA_INIT(AboutDlg)
	//version_ = _T("");
	//}}AFX_DATA_INIT
	//scroll_pos_ = 0;
	//stop_delay_ = 0;
	//text_lines_ = 0;
}


AboutDlg::~AboutDlg()
{
	//scroll_dc_.DeleteDC();	// delete dc before the bmp selected into it
	//backgnd_dc_.DeleteDC();
}


void AboutDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_IMAGE, image_);
	//{{AFX_DATA_MAP(AboutDlg)
//	DDX_Text(DX, IDC_VERSION, version_);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(AboutDlg, CDialog)
	//{{AFX_MSG_MAP(AboutDlg)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_NOTIFY(NM_CLICK, IDC_LINK_2, &AboutDlg::OnNMClickSyslink1)
	ON_NOTIFY(NM_CLICK, IDC_LINK, &AboutDlg::OnNMClickSyslink1)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*/ an area in a bitmap to print extra text info
namespace {
	const CPoint LEFTTOP(60, 120);
	const CSize AREA_SIZE(650, 80);
	const CPoint LINK_TOP(250, 200);
	const COLORREF TEXT_COLOR= RGB(255,255,255);
	const COLORREF LINK_COLOR= RGB(70,170,255);
	const CPoint LEFTTOP2(30, 228);
}


extern AutoPtr<Dib> LoadJpeg(UINT img_id);

*/
extern CString ReadAppVersion(bool including_build)
{
	CString version;

	HRSRC rsrc= ::FindResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	HGLOBAL global;
	if (rsrc && (global = ::LoadResource(AfxGetInstanceHandle(),rsrc)) != NULL)
	{
		VS_FIXEDFILEINFO* ver= (VS_FIXEDFILEINFO*)((BYTE*)::LockResource(global) + 0x28);
		if (ver->dwSignature == 0xfeef04bd)
		{
			//if (including_build)
				//version.Format(_T("Version %d.%u.%u Build %u %s"),
				version.Format(_T("Version %d.%u %s Build"),
					(int)HIWORD(ver->dwProductVersionMS), (int)LOWORD(ver->dwProductVersionMS),
					//(int)HIWORD(ver->dwProductVersionLS), (int)LOWORD(ver->dwProductVersionLS),
					sizeof(TCHAR) == 1 ? _T("ASCII") : _T("Unicode"));
			//else
				//version.Format(_T("Version %d.%u.%u\n%s Build"),
					//(int)HIWORD(ver->dwProductVersionMS), (int)LOWORD(ver->dwProductVersionMS),
					//(int)HIWORD(ver->dwProductVersionLS),
					//sizeof(TCHAR) == 1 ? _T("ASCII") : _T("Unicode"));
		}
		::FreeResource(global);
	}

	return version;
}

BOOL AboutDlg::OnInitDialog()
{
	try
	{
		version_ = ReadAppVersion(true);
/*		about_ = version_ + _T("\n")
#ifdef _WIN64
			_T("x64")
#else
			_T("x86")
#endif
			_T(" Release\n");

		about_ += _T("Free software released under the terms of the GNU Public License.");
*/
		CDialog::OnInitDialog();
		
		LOGFONT lf;
		::GetDefaultGuiBoldFont(lf);
		lf.lfHeight -= 3;
		small_fnt_.CreateFontIndirect(&lf);
		
		GetDlgItem(IDC_LABEL)->SetFont(&small_fnt_);
		//GetDlgItem(IDC_LABEL_2)->SetFont(&small_fnt_);

		//if (!PNGImage().Load(IDR_ABOUT, dib_about_) || !dib_about_.IsValid())
		//	EndDialog(IDCANCEL);

		//CRect rect(CPoint(0, 0), dib_about_.GetSize());
		//::AdjustWindowRectEx(rect, GetStyle(), false, GetExStyle());
		CRect rect(0,0,0,0);
		GetClientRect(rect);
		SetWindowPos(0, 0, 0, rect.Width(), rect.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		GetDlgItem(IDC_LINK_2)->SetWindowText(L"Free software released under the terms of the <a href=\"" GPL_SITE L"\">GNU Public License</a>");
		//GetDlgItem(IDC_LINK)->SetWindowText(L"项目主页: <a href=\"" WEB_SITE L"\">https://gitee.com/dream7180/ExifPro-mod-cn</a>\n\n其他链接: <a href=\"" BLOG_SITE L"\">我的博客</a>, 本软件基于已停止开发的 <a href=\"" CODE_SITE L"\">ExifPro</a>");//, <a href=\"" OFF_SITE L"\">www.exifpro.com</a>");
		GetDlgItem(IDC_LINK)->SetWindowText(L"项目主页: <a href=\"" WEB_SITE L"\">https://gitee.com/dream7180/ExifPro-mod-cn</a>\n\n本软件基于已停止开发的 <a href=\"" CODE_SITE L"\">ExifPro</a>");//, <a href=\"" OFF_SITE L"\">www.exifpro.com</a>");
		GetDlgItem(IDC_VERSION)->SetWindowText(version_);
		image_.SetImage(IDR_ABOUT);
		
		//CPoint pos= CPoint(Pixels(LEFTTOP.x), );
		//pos.y += Pixels(AREA_SIZE.cy);
	/*	link_wnd_.Create(this, CPoint(Pixels(LINK_TOP.x), Pixels(LINK_TOP.y)), _T("https://github.com/dream7180/ExifPro-mod"), _T("https://github.com/dream7180/ExifPro-mod"), &small_fnt_);
		link_wnd_.rgb_text_color_ = LINK_COLOR;
		link2_wnd_.Create(this, CPoint(Pixels(LINK_TOP.x), Pixels(LINK_TOP.y) + 25), _T("https://www.cnblogs.com/foobox/"), _T("https://www.cnblogs.com/foobox/"), &small_fnt_);
		link2_wnd_.rgb_text_color_ = LINK_COLOR;
		link4_wnd_.Create(this, CPoint(Pixels(LINK_TOP.x), Pixels(LINK_TOP.y) + 50), _T("https://github.com/mikekov/ExifPro"), _T("https://github.com/mikekov/ExifPro"), &small_fnt_);
		link4_wnd_.rgb_text_color_ = LINK_COLOR;
		link3_wnd_.Create(this, CPoint(Pixels(LINK_TOP.x), Pixels(LINK_TOP.y) + 75), _T("http://www.exifpro.com/"), _T("http://www.exifpro.com/"), &small_fnt_);
		link3_wnd_.rgb_text_color_ = LINK_COLOR;
		
		about_2 = _T("***Image decoder libraries***\n");
		about_2 += _T("Libjpeg-turbo: 2.0.4\n");
		about_2 += _T("LibTIFF: 4.1.0\n");
		about_2 += _T("LibPNG: 1.6.37\n");
		about_2 += _T("GIFlib: 4.2.3\n");
		about_2 += _T("Little CMS: 1.19");
		
		link_txt_1 = _T("Mod by dreamawake:");
		link_txt_2 = _T("dreamawake 的博客:");
		link_txt_3 = _T("Mod from ExifPro V2.3:");
		link_txt_4 = _T("ExifPro V2.1 网站:");

		libs_ =
			_T("\n")
			_T("ExifPro is based in part on the work of the Independent JPEG Group\nCopyright (c) 1991-1996, Thomas G. Lane\n")
			_T("TIFF library is copyright as follows:\n")
			_T("Copyright (c) 1988-1997 Sam Leffler\nCopyright (c) 1991-1997 Silicon Graphics, Inc.\n")
			_T("PNG library is copyright as follows:\n")
			_T("Copyright (c) 1998-2002 Glenn Randers-Pehrson\nCopyright (c) 1996-1997 Andreas Dilger\nCopyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.\n")
			_T("ZLib library is copyright as follows:\n")
			_T("(c) 1995-2002 Jean-loup Gailly and Mark Adler\n")
			_T("Boost libraries: www.boost.org\n")
			_T("CoolSB library: (c) J. Brown 2001\n")
			_T("LCMS library: www.littlecms.com\n")
			_T("Lua interpreter: www.lua.org\n")
			_T("XMP library: Copyright (c) 1999-2012, Adobe Systems Incorporated\n")
			_T("Scintilla: Copyright 1998-2003 by Neil Hodgson\n")
			_T("LibJPEG-Turbo: www.libjpeg-turbo.org\n")
			_T("\n");

		const TCHAR* b= libs_;
		const TCHAR* e= b + libs_.GetLength();
		text_lines_ = static_cast<int>(std::count(b, e, L'\n'));	// lines of text to scroll

		SetTimer(1, 25, 0);
*/
		return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
	}
	CATCH_ALL

	EndDialog(IDCANCEL);

	return true;
}


BOOL AboutDlg::OnEraseBkgnd(CDC* dc)
{
	//if (!dib_about_.IsValid())
	//	return CDialog::OnEraseBkgnd(dc);

	//dib_about_.DibDraw(dc, CPoint(0, 0));
/*
	CFont* old= dc->SelectObject(&small_fnt_);

	dc->SetTextColor(TEXT_COLOR);
	dc->SetBkMode(TRANSPARENT);
	dc->DrawText(about_, CRect(CPoint(Pixels(LEFTTOP.x), Pixels(LEFTTOP.y)), CSize(Pixels(AREA_SIZE.cx), Pixels(AREA_SIZE.cy))), DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_END_ELLIPSIS);
	dc->DrawText(about_2, CRect(CPoint(Pixels(LEFTTOP.x), Pixels(LINK_TOP.y) + 120), CSize(Pixels(AREA_SIZE.cx), Pixels(AREA_SIZE.cy) + 40)), DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_END_ELLIPSIS);
	dc->DrawText(link_txt_1, CRect(CPoint(Pixels(LEFTTOP.x), Pixels(LINK_TOP.y)), CSize(180, 20)), DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_END_ELLIPSIS);
	dc->DrawText(link_txt_2, CRect(CPoint(Pixels(LEFTTOP.x), Pixels(LINK_TOP.y) + 25), CSize(180, 20)), DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_END_ELLIPSIS);
	dc->DrawText(link_txt_3, CRect(CPoint(Pixels(LEFTTOP.x), Pixels(LINK_TOP.y) + 50), CSize(180, 20)), DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_END_ELLIPSIS);
	dc->DrawText(link_txt_4, CRect(CPoint(Pixels(LEFTTOP.x), Pixels(LINK_TOP.y) + 75), CSize(180, 20)), DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_END_ELLIPSIS);
	dc->SelectObject(old);
*/
	//COLORREF background = RGB(60,60,60);
//	MemoryDC mem_dc(*dc, this, rgb_background);
//	mem_dc.BitBlt();
	CRect rect;
	GetClientRect(rect);
	dc->FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));
	return true;
}


void AboutDlg::OnLButtonUp(UINT flags, CPoint point)
{
	EndDialog(IDCANCEL);
}
/*

void AboutDlg::OnTimer(UINT_PTR event_id)
{
	CDialog::OnTimer(event_id);

	if (event_id != 1)
		return;

	if (stop_delay_ > 0)
	{
		--stop_delay_;
		return;
	}

	// scroll text

	CClientDC dc(this);

	CRect rect;
	GetClientRect(rect);
	rect.DeflateRect(Pixels(20), Pixels(1), Pixels(2), Pixels(1));
	rect.top = rect.bottom - Pixels(19);

	// prepare bmp to erase background behind text
	if (scroll_bmp_.m_hObject == 0)
	{
		scroll_bmp_.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
		scroll_dc_.CreateCompatibleDC(&dc);
		scroll_dc_.SelectObject(&scroll_bmp_);
		backgnd_dc_.CreateCompatibleDC(&dc);
		//backgnd_dc_.SelectObject(dib_about_.GetBmp());
	}

	CFont* old= scroll_dc_.SelectObject(&small_fnt_);

	scroll_dc_.SetTextColor(TEXT_COLOR);
	scroll_dc_.SetBkColor(0);
	scroll_dc_.SetBkMode(TRANSPARENT);
	scroll_dc_.BitBlt(0, 0, rect.Width(), rect.Height(), &backgnd_dc_, rect.left, rect.top, SRCCOPY);
	scroll_dc_.SetViewportOrg(0, -scroll_pos_);
	CRect bounds(0, 0, rect.Width(), rect.Height());
	scroll_dc_.DrawText(libs_, bounds, DT_LEFT | DT_BOTTOM | DT_NOPREFIX | DT_WORDBREAK | DT_NOCLIP);
	scroll_dc_.DrawText(libs_, bounds, DT_LEFT | DT_BOTTOM | DT_NOPREFIX | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);
	scroll_dc_.SetViewportOrg(0, 0);

	scroll_dc_.SelectObject(old);

	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &scroll_dc_, 0, 0, SRCCOPY);

	int line_height = bounds.Height() / (text_lines_ + 1);
	if (line_height > 0 && scroll_pos_ % line_height == 0)		// pause after scrolling whole line
	{
		stop_delay_ = 90;
		if (scroll_pos_ / line_height == text_lines_)
			scroll_pos_ = 0;
	}

	++scroll_pos_;
}
*/
void AboutDlg::OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	 PNMLINK pNMLink = (PNMLINK) pNMHDR; 
	 if (wcscmp(pNMLink->item.szUrl, WEB_SITE) == 0)
     {
		ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);  //主要执行语句
     //} else if (wcscmp(pNMLink->item.szUrl, BLOG_SITE) == 0){
	//	ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
	 } else if (wcscmp(pNMLink->item.szUrl, CODE_SITE) == 0){
		ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
	 } else if (wcscmp(pNMLink->item.szUrl, GPL_SITE) == 0){
		ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
	 }else{
		ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
	 }
	*pResult = 0;
}

HBRUSH AboutDlg::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr = CDialog::OnCtlColor(dc, wnd, ctl_color);
	//m_brush.CreateSolidBrush(RGB(255,255,255));
	//if (wnd->GetDlgCtrlID() == IDC_STATIC){
		//dc->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));  //字体颜色
		//dc->SetBkColor(RGB(60, 60, 60));
		dc->SetBkMode(TRANSPARENT);
	//	return m_brush;
	//} 
	return hbr;
}