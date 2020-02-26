// SlideShowApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "Photos.h"
#include "Rect.h"


//#pragma data_seg ("shared")
//
//static LONG shared_instance_counter= 0;
//
//#pragma data_seg ()
// linker: /SECTION:shared,RWS

//#pragma data_seg ("SlidShow")
//
//static LONG test[0x10000]= {0xab};
//
//#pragma data_seg ()


static HINSTANCE g_hInstance= 0;
static CPhotos* g_pPhotos= 0;

static int ErrMsg()
{
	::MessageBox(HWND_DESKTOP, "Please regenerate slide show.\nThis file is corrupt", "Slide Show", MB_OK | MB_ICONERROR);
	return 0;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	//INT_PTR CALLBACK WinProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
//nCmdShow=test[0];
	g_hInstance = hInstance;

	// app base addr
	const BYTE* pcBase= reinterpret_cast<const BYTE*>(0x400000);

	// read file's header
	const IMAGE_DOS_HEADER* dh= reinterpret_cast<const IMAGE_DOS_HEADER*>(pcBase);
	// PE header
	const IMAGE_NT_HEADERS* nth= reinterpret_cast<const IMAGE_NT_HEADERS*>(pcBase + dh->e_lfanew);
	// sections follow
	const IMAGE_SECTION_HEADER* pSections= reinterpret_cast<const IMAGE_SECTION_HEADER*>(nth + 1);
	// 'SlidShow' section (the last one)
	const IMAGE_SECTION_HEADER* pSlideShow= &pSections[nth->FileHeader.NumberOfSections - 1];

	if (strncmp(reinterpret_cast<const char*>(pSlideShow->Name), "SlidShow", 8) != 0)
		return ErrMsg();

	const BYTE* pcPhotos= pcBase + pSlideShow->VirtualAddress;
#ifdef _DEBUG
	// debug info at the end?
	if (pcPhotos[0] == 'N' && pcPhotos[1] == 'B')
	{
		pcPhotos += 0x10;
		pcPhotos += strlen((const char*)pcPhotos) + 1;
	}
#endif
	// check magic value
	const BYTE vMagic[]= { 0xfb, 0x05, 0xdc, 0x1d };
	if (memcmp(pcPhotos, vMagic, 4) != 0)
		return ErrMsg();

	g_pPhotos = new CPhotos(pcPhotos, pSlideShow->Misc.VirtualSize);

	if (g_pPhotos == 0 || g_pPhotos->Count() < 1)
		return ErrMsg();

	HACCEL hAccel= ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	const char* szWndClass= "SlideShowWnd";
	WNDCLASS win_class;
	win_class.style 		= CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc	= WinProc;
	win_class.cbClsExtra	= 0;
	win_class.cbWndExtra	= 0;
	win_class.hInstance 	= hInstance;
	win_class.hIcon 		= NULL;
	win_class.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = 0;
	win_class.lpszMenuName	= NULL;
	win_class.lpszClassName = szWndClass;
	if (!::RegisterClass(&win_class))
		return 0;

	if (HWND hWnd= ::CreateWindowEx(0, szWndClass, "Slide Show", WS_OVERLAPPEDWINDOW,
			 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			 NULL, NULL, hInstance, NULL))

//	if (HWND hWnd= ::CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SLIDE_SHOW), HWND_DESKTOP, DlgProc))
	{
		MSG msg;
		while (::GetMessage(&msg, 0, 0, 0))
		{
			if (!::TranslateAccelerator(hWnd, hAccel, &msg) && !::IsDialogMessage(hWnd, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}

	::DestroyAcceleratorTable(hAccel);

	delete g_pPhotos;
	g_pPhotos = 0;

	return 0;
}


DWORD GetStyle(HWND hWnd)
{
	return ::GetWindowLong(hWnd, GWL_STYLE);
}

DWORD GetStyleEx(HWND hWnd)
{
	return ::GetWindowLong(hWnd, GWL_EXSTYLE);
}


void ModifyStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd)
{
	DWORD dwStyle= GetStyle(hWnd);
	dwStyle &= ~dwRemove;
	dwStyle |= dwAdd;
	::SetWindowLong(hWnd, GWL_STYLE, dwStyle);
}

void ModifyStyleEx(HWND hWnd, DWORD dwRemove, DWORD dwAdd)
{
	DWORD dwStyleEx= GetStyleEx(hWnd);
	dwStyleEx &= ~dwRemove;
	dwStyleEx |= dwAdd;
	::SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
}


static DWORD g_dwDlgWndStyle= 0;
static DWORD g_dwDlgWndStyleEx= 0;
static RECT g_rectWndPos;


void FullScreen(HWND hWnd)
{
	::GetWindowRect(hWnd, &g_rectWndPos);

	MDC dcInfo;
	dcInfo.CreateIC("DISPLAY", NULL, NULL, NULL);
	int nWidth= ::GetDeviceCaps(dcInfo, HORZRES);
	int nHeight= ::GetDeviceCaps(dcInfo, VERTRES);

	ModifyStyle(hWnd, WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | DS_MODALFRAME, WS_MAXIMIZE);
	ModifyStyleEx(hWnd, WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME, 0);

	MRect rect(0, 0, nWidth, nHeight);
	::AdjustWindowRect(&rect, GetStyle(hWnd), false);

	::SetWindowPos(hWnd, 0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
}


void WindowMode(HWND hWnd)
{
	ModifyStyle(hWnd, WS_MAXIMIZE, g_dwDlgWndStyle);
	ModifyStyleEx(hWnd, 0, g_dwDlgWndStyleEx);

	::SetWindowPos(hWnd, 0, g_rectWndPos.left, g_rectWndPos.top,
		g_rectWndPos.right - g_rectWndPos.left, g_rectWndPos.bottom - g_rectWndPos.top, SWP_NOZORDER);
}


///////////////////////////////////////////////////////////////////////////////
static AutoPtr<CDib> s_dibPhoto;
static int s_nCurPhotoIndex= 0;
static bool g_bFullScreen= false;
static bool g_RepeatedLoop= true;

void DisplayPhoto(HWND hWnd)
{
	TCHAR szTitle[256];
	wsprintf(szTitle, _T("Slide Show (%d/%d)"), s_nCurPhotoIndex + 1, g_pPhotos->Count());
	::SetWindowText(hWnd, szTitle);
	s_dibPhoto = g_pPhotos->GetPhoto(s_nCurPhotoIndex);
	InvalidateRect(hWnd, 0, true);
}

void First(HWND hWnd)
{
	if (s_nCurPhotoIndex != 0)
	{
		s_nCurPhotoIndex = 0;
		DisplayPhoto(hWnd);
	}
}

void Next(HWND hWnd, bool repeat)
{
	if (s_nCurPhotoIndex < g_pPhotos->Count() - 1)
	{
		++s_nCurPhotoIndex;
		DisplayPhoto(hWnd);
	}
	else if (repeat)
		First(hWnd);
}

void Prev(HWND hWnd)
{
	if (s_nCurPhotoIndex > 0)
	{
		--s_nCurPhotoIndex;
		DisplayPhoto(hWnd);
	}
}

void Last(HWND hWnd)
{
	if (s_nCurPhotoIndex != g_pPhotos->Count() - 1)
	{
		s_nCurPhotoIndex = g_pPhotos->Count() - 1;
		DisplayPhoto(hWnd);
	}
}


void Paint(HWND hWnd, HDC hDC)
{
	MRect rect;
	::GetClientRect(hWnd, &rect);

	COLORREF rgbBack= RGB(0,0,0);

	if (s_dibPhoto.get())
	{
		s_dibPhoto->DrawImg(hDC, rect, rgbBack);
	}
	else
	{
		::SetBkColor(hDC, rgbBack);
		::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, 0, 0, 0);
	}
}


int EraseBknd(HWND hWnd, HDC hDC)
{
	return 1;
}


LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			g_dwDlgWndStyle = GetStyle(hWnd);
			g_dwDlgWndStyleEx = GetStyleEx(hWnd);

			HICON hIcon= ::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SLIDE_SHOW));
			::SendMessage(hWnd, WM_SETICON, true, (LPARAM)hIcon);
			::SendMessage(hWnd, WM_SETICON, false, (LPARAM)hIcon);

			g_RepeatedLoop = g_pPhotos->RepeatedLoop();

			if (g_pPhotos->StartFullScreen())
			{
				FullScreen(hWnd);
				g_bFullScreen = true;
			}

			DisplayPhoto(hWnd);

			g_rectWndPos.left = 100;
			g_rectWndPos.top = 100;
			g_rectWndPos.right = 700;
			g_rectWndPos.bottom = 500;

			if (s_dibPhoto.get())
			{
				g_rectWndPos.right = g_rectWndPos.left + s_dibPhoto->GetWidth();
				g_rectWndPos.bottom = g_rectWndPos.top + s_dibPhoto->GetHeight();
				::AdjustWindowRectEx(&g_rectWndPos, g_dwDlgWndStyle, false, g_dwDlgWndStyleEx);
			}

			if (DWORD dwDelay= g_pPhotos->Delay())
			{
				::SetTimer(hWnd, 1, dwDelay * 1000, 0);
			}

			::ShowWindow(hWnd, SW_SHOW);
		}
		return 0;

	case WM_ERASEBKGND:
		return EraseBknd(hWnd, reinterpret_cast<HDC>(wParam));

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			memset(&ps, 0, sizeof ps);
			if (HDC hDC= ::BeginPaint(hWnd, &ps))
			{
				Paint(hWnd, hDC);
				::EndPaint(hWnd, &ps);
			}
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			::DestroyWindow(hWnd);
			return 0;
		case ID_NEXT:
			Next(hWnd, false);
			return 0;
		case ID_PREV:
			Prev(hWnd);
			return 0;
		case ID_FIRST:
			First(hWnd);
			return 0;
		case ID_LAST:
			Last(hWnd);
			return 0;
		case ID_TOGGLE_FULL_SCRN:
			if (g_bFullScreen)
				WindowMode(hWnd);
			else
				FullScreen(hWnd);
			g_bFullScreen = !g_bFullScreen;
			return 0;
		}
		return FALSE;

	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_MAXIMIZE)
		{
			if (!g_bFullScreen)
			{
				FullScreen(hWnd);
				g_bFullScreen = true;
			}
			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		{
			int nDelta= GET_WHEEL_DELTA_WPARAM(wParam);
			if (nDelta < 0)
				Next(hWnd, false);
			else if (nDelta > 0)
				Prev(hWnd);
		}
		return 0;

	case WM_CONTEXTMENU:
		return 0;		//TODO

	case WM_TIMER:
		if (wParam == 1)
		{
			Next(hWnd, g_RepeatedLoop);
			return 0;
		}
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
