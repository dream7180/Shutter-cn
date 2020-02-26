/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "CtrlDraw.h"
#include <UxTheme.h>
//#include <tmschema.h>
//#include <VsStyle.h>
//#include <AeroStyle.xml>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct _ThemeHelper
{
	typedef BOOL(__stdcall *PFNISAPPTHEMED)();
	typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR class_list);
	typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME theme);
	typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME theme, HDC hdc, int part_id, int iStateId, const RECT *rect, OPTIONAL const RECT *clip_rect);
	typedef HRESULT(__stdcall *PFNGETTHEMEPARTSIZE)(HTHEME theme, HDC hdc, int part_id, int iStateId, RECT * rect, enum THEMESIZE size, OUT SIZE *psz);

	static void* GetProc(LPCSTR proc, void* pfnFail)
	{
		static HMODULE theme_dll = LoadLibrary(_T("UxTheme.dll"));

		void* ret = pfnFail;
		if (theme_dll != NULL)
		{
			ret = GetProcAddress(theme_dll, proc);
		}
		return ret;
	}

	static BOOL IsAppThemedFail()
	{
		return FALSE;
	}
	static BOOL IsAppThemed()
	{
		static PFNISAPPTHEMED pfnIsAppThemed = (PFNISAPPTHEMED)GetProc("IsAppThemed", IsAppThemedFail);
		return (*pfnIsAppThemed)();
	}

	static HTHEME OpenThemeDataFail(HWND , LPCWSTR )
	{
		return NULL;
	}
	static HTHEME OpenThemeData(HWND hwnd, LPCWSTR class_list)
	{
		static PFNOPENTHEMEDATA pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProc("OpenThemeData", OpenThemeData);
		return (*pfnOpenThemeData)(hwnd, class_list);
	}
	static HRESULT CloseThemeDataFail(HTHEME)
	{
		return E_FAIL;
	}
	static HRESULT CloseThemeData(HTHEME theme)
	{
		static PFNCLOSETHEMEDATA pfnCloseThemeData = (PFNCLOSETHEMEDATA)GetProc("CloseThemeData", CloseThemeDataFail);
		return (*pfnCloseThemeData)(theme);
	}

	static HRESULT DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT *, const RECT *)
	{
		return E_FAIL;
	}

	static HRESULT DrawThemeBackground(HTHEME theme, HDC hdc, int part_id, int iStateId, const RECT *rect, const RECT *clip_rect)
	{
		static PFNDRAWTHEMEBACKGROUND pfnDrawThemeBackground = 
			(PFNDRAWTHEMEBACKGROUND)GetProc("DrawThemeBackground", DrawThemeBackgroundFail);
		return (*pfnDrawThemeBackground)(theme, hdc, part_id, iStateId, rect, clip_rect);
	}

	static HRESULT GetThemePartSizeFail(HTHEME, HDC, int, int, RECT *, enum THEMESIZE, SIZE *)
	{
		return E_FAIL;
	}

	static HRESULT GetThemePartSize(HTHEME theme, HDC hdc, int part_id, int iStateId, RECT * rect, enum THEMESIZE size, SIZE *psz)
	{
		static PFNGETTHEMEPARTSIZE pfnGetThemePartSize = 
			(PFNGETTHEMEPARTSIZE)GetProc("GetThemePartSize", GetThemePartSizeFail);
		return (*pfnGetThemePartSize)(theme, hdc, part_id, iStateId, rect, size, psz);
	}
};


///////////////////////////////////////////////////////////////////////////////

namespace CtrlDraw
{

//	static CImageList image_list_;
//void DrawCtrl(CDC& dc, CRect rect, int state, bool check_box);
//	CSize GetCtrlSize(CWnd* wnd, int part, int state) const;


static CImageList g_image_list_;


//CCtrlDraw::CCtrlDraw()
//{
//	bool LoadImageList(CImageList& il, UINT id, int img_width);
//
//	if (g_image_list_.m_hImageList == 0)
//	{
//		LoadImageList(g_image_list_, IDB_CTRL_DRAW_IMAGES, 13);
//		//HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDB_CTRL_DRAW_IMAGES), RT_BITMAP);
//		//image_list_.Attach(ImageList_LoadImage(inst, MAKEINTRESOURCE(IDB_CTRL_DRAW_IMAGES),
//		//	13, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION));
//	}
//}

//CCtrlDraw::~CCtrlDraw()
//{}


void DrawCtrl(CDC& dc, CRect rect, int state, bool check_box)
{
	bool done= false;

	int part= check_box ? BP_CHECKBOX : BP_RADIOBUTTON;

	int cyItem= check_box ? GetCheckBoxSize(dc.GetWindow()).cy : GetRadioBtnSize(dc.GetWindow()).cy;

	// Draw the check boxes using the theme API's only if the app is themed.
	if (_ThemeHelper::IsAppThemed())
	{
		if (HTHEME theme= _ThemeHelper::OpenThemeData(*dc.GetWindow(), L"Button"))
		{
			CSize size(0, 0);
			if (SUCCEEDED(_ThemeHelper::GetThemePartSize(theme, dc.m_hDC, part, state, NULL, TS_TRUE, &size)))
			{
				CRect check_rect = rect; //drawItem.rcItem;
				check_rect.left += 1;
				check_rect.top += 1 + std::max<int>(0, (cyItem - size.cy) / 2);
				check_rect.right = check_rect.left + size.cx;
				check_rect.bottom = check_rect.top + size.cy;

				CRect item_rect = rect; //drawItem.rcItem;
				item_rect.right = item_rect.left + size.cx + 2;

				//CRect check_box_rect = OnGetCheckPosition(item_rect, check_rect);
				if (SUCCEEDED(_ThemeHelper::DrawThemeBackground(theme, dc.m_hDC, part, state, &check_rect, NULL)))
				{
					done = true;
					//drawItem.rcItem.left = drawItem.rcItem.left + size.cx + 3;
				}
			}

			_ThemeHelper::CloseThemeData(theme);
		}
	}

	if (!done)
	{
		// draw the bitmap

		// TODO: center
		int img= 0;
		if (check_box)
			switch (state)
			{
			case CBS_UNCHECKEDHOT:
			case CBS_UNCHECKEDNORMAL:
			case CBS_UNCHECKEDDISABLED:
				img = 0;
				break;
			case CBS_CHECKEDHOT:
			case CBS_CHECKEDNORMAL:
			case CBS_CHECKEDDISABLED:
				img = 1;
				break;
			case CBS_MIXEDHOT:
			case CBS_MIXEDNORMAL:
			case CBS_MIXEDDISABLED:
				img = 2;
				break;
			default:
				ASSERT(false);
				break;
			}
		else
			switch (state)
			{
			case RBS_UNCHECKEDNORMAL:
			case RBS_UNCHECKEDHOT:
			case RBS_UNCHECKEDPRESSED:
			case RBS_UNCHECKEDDISABLED:
				img = 3;
				break;
			case RBS_CHECKEDNORMAL:
			case RBS_CHECKEDHOT:
			case RBS_CHECKEDPRESSED:
			case RBS_CHECKEDDISABLED:
				img = 4;
				break;
			default:
				ASSERT(false);
				break;
			}

		bool LoadImageList(CImageList& il, UINT id, int img_width);

		if (g_image_list_.m_hImageList == 0)
			LoadImageList(g_image_list_, IDB_CTRL_DRAW_IMAGES, 13);

		g_image_list_.Draw(&dc, img, rect.TopLeft(), ILD_TRANSPARENT);
	}
}


void DrawCheckBox(CDC& dc, CRect rect, int state)
{
	DrawCtrl(dc, rect, state, true);
}


void DrawRadioBox(CDC& dc, CRect rect, int state)
{
	DrawCtrl(dc, rect, state, false);
}


void DrawExpandBox(CDC& dc, const CRect& rect, bool collapsed, bool hot)
{
	if (hot)
	{
		int a= 0;
	}
	// hot glyph doesn't work
	int part= TVP_GLYPH; //hot ? TVP_HOTGLYPH : TVP_GLYPH;
	int state = collapsed ? GLPS_CLOSED : GLPS_OPENED; //hot ? (collapsed ? HGLPS_CLOSED : HGLPS_OPENED) : (collapsed ? GLPS_CLOSED : GLPS_OPENED);
	bool done= false;

	if (_ThemeHelper::IsAppThemed())
	{
		if (HTHEME theme= _ThemeHelper::OpenThemeData(*dc.GetWindow(), L"TREEVIEW"))
		{
			CSize size(0, 0);
			if (SUCCEEDED(_ThemeHelper::GetThemePartSize(theme, dc.m_hDC, part, state, NULL, TS_TRUE, &size)))
			{
				CRect box_rect = rect;
				box_rect .left += 1;
				box_rect .top += 1 + std::max<int>(0, (rect.Height() - size.cy) / 2);
				box_rect .right = box_rect .left + size.cx;
				box_rect .bottom = box_rect .top + size.cy;

				if (SUCCEEDED(_ThemeHelper::DrawThemeBackground(theme, dc.m_hDC, part, state, &box_rect , NULL)))
					done = true;
			}

			_ThemeHelper::CloseThemeData(theme);
		}
	}

	if (!done)
	{
		//
	}
}


CSize GetCtrlSize(CWnd* wnd, int part, int state)
{
	static CSize size(0, 0);

	if (size.cx == 0 && size.cy == 0)
	{
		if (_ThemeHelper::IsAppThemed())
		{
			if (HTHEME theme= _ThemeHelper::OpenThemeData(*wnd, L"Button"))
			{
				CSize btn_size(0, 0);
				CClientDC dc(wnd);
				if (SUCCEEDED(_ThemeHelper::GetThemePartSize(theme, dc.m_hDC, part, state, NULL, TS_TRUE, &btn_size)))
					size = btn_size;

				_ThemeHelper::CloseThemeData(theme);
			}
		}

		if (size.cx == 0 || size.cy == 0)
			size = CSize(13, 13);
	}

	return size;
}


CSize GetCheckBoxSize(CWnd* wnd)
{
	return GetCtrlSize(wnd, BP_CHECKBOX, CBS_CHECKEDNORMAL);
}


CSize GetRadioBtnSize(CWnd* wnd)
{
	return GetCtrlSize(wnd, BP_RADIOBUTTON, RBS_CHECKEDNORMAL);
}


//#define GP_BORDER   1
//#define BSS_FLAT   1


void DrawWndElement(CDC& dc, CRect rect, const wchar_t* wnd_class, int part, int state, bool exclude_interior)
{
	bool done= false;

	// Draw the check boxes using the theme API's only if the app is themed.
	if (_ThemeHelper::IsAppThemed())
	{
		if (HTHEME theme= _ThemeHelper::OpenThemeData(*dc.GetWindow(), wnd_class))
		{
			if (exclude_interior)
			{
				// exclude interior to eliminate flickering
				CRgn rgn;
				rgn.CreateRectRgnIndirect(rect);
				dc.SelectClipRgn(&rgn);
				CRect clip_rect= rect;
				clip_rect.DeflateRect(1, 1);
				dc.ExcludeClipRect(clip_rect);
			}

			if (SUCCEEDED(_ThemeHelper::DrawThemeBackground(theme, dc.m_hDC, part, state, rect, 0)))
				done = true;

			if (exclude_interior)
				dc.SelectClipRgn(0);

			_ThemeHelper::CloseThemeData(theme);
		}
	}

	if (!done)
	{
		COLORREF rgb_gray= RGB(176,176,176);
		dc.Draw3dRect(rect, rgb_gray, rgb_gray);
	}
}


void DrawBorder(CDC& dc, CRect rect)
{
	DrawWndElement(dc, rect, L"ListView", LVP_LISTITEM, LISS_NORMAL, true);
}


void DrawComboBorder(CDC& dc, CRect rect, bool hot)
{
	DrawWndElement(dc, rect, L"ComboBox", CP_BORDER, hot ? CBXS_HOT : CBXS_NORMAL, false);
}


//bool DrawListItem(CDC& dc, CRect rect, int state)
//{
//	if (!_ThemeHelper::IsAppThemed())
//		return false;
//
//	int part= /*LVP_LISTDETAIL;*/ LVP_LISTITEM;
////	state=LBPSI_SELECTED;
//	bool done= false;
//	CWnd* wnd= dc.GetWindow();
//
//	if (HTHEME theme= _ThemeHelper::OpenThemeData(*wnd, VSCLASS_LISTVIEW))
//	{
//		if (IsThemeBackgroundPartiallyTransparent(theme, part, state))
//			::DrawThemeParentBackground(*wnd, dc.m_hDC, rect);
//
//		if (SUCCEEDED(_ThemeHelper::DrawThemeBackground(theme, dc.m_hDC, part, state, &rect, &rect)))
//			done = true;
//
//		_ThemeHelper::CloseThemeData(theme);
//	}
//
//	return done;
//}


bool DrawThemedElement(HDC hdc, const CRect& rect, const wchar_t* wnd_class, int part, int state, int exclude_interior)
{
	bool done= false;

	// Draw the check boxes using the theme API's only if the app is themed.
	if (::IsAppThemed())
	{
		HWND hwnd= ::WindowFromDC(hdc);

		if (HTHEME theme= ::OpenThemeData(hwnd, wnd_class))
		{
			HRGN hrgn= 0;
			if (exclude_interior > 0)
			{
				// exclude interior
				hrgn = ::CreateRectRgnIndirect(rect);
				::SelectClipRgn(hdc, hrgn);
				RECT clip_rect= rect;
				::InflateRect(&clip_rect, -exclude_interior, -exclude_interior);
				if (clip_rect.right - clip_rect.left > 0 && clip_rect.bottom - clip_rect.top > 0)
					::ExcludeClipRect(hdc, clip_rect.left, clip_rect.top, clip_rect.right, clip_rect.bottom);
			}

			if (::IsThemeBackgroundPartiallyTransparent(theme, part, state))
				::DrawThemeParentBackground(hwnd, hdc, rect);

			if (SUCCEEDED(::DrawThemeBackground(theme, hdc, part, state, rect, 0)))
				done = true;

			if (exclude_interior > 0)
				::SelectClipRgn(hdc, 0);
			if (hrgn)
				::DeleteObject(hrgn);

			::CloseThemeData(theme);
		}
	}

	return done;
}


} // namespace
