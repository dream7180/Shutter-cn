/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "StarCtrl.h"
#include "MemoryDC.h"
#include "LoadImageList.h"
#include "GlobalAlloc.h"

std::auto_ptr<Gdiplus::Bitmap> LoadPng(int rsrc_id, const wchar_t* rsrc_type, HMODULE instance);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(StarCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_TIMER()
END_MESSAGE_MAP()


CSize StarCtrl::star_size_= 0;
boost::scoped_ptr<Gdiplus::Bitmap> StarCtrl::stars_bmp_;
int StarCtrl::counter_= 0;

static const int HORZ_SPACE= 4;
static const int ANGLE= 7;
static const TCHAR* const STAR_CTRL_CLASS= _T("StarCtrlMiK");
enum Images { LIT, HOT, FAINT, INDETERMINATE };

static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE inst = AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(inst, class_name, &wndcls))
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
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}


StarCtrl::StarCtrl()
{
	RegisterWndClass(STAR_CTRL_CLASS);

	if (counter_ == 0)
	{
		stars_bmp_.reset(LoadPng(IDB_STARS, L"PNG", AfxGetResourceHandle()).release());
		if (stars_bmp_.get() == 0)
			throw "StarCtrl: cannot load PNG resources";
		const int single_image_width= stars_bmp_->GetWidth() / 4;
		UINT h= stars_bmp_->GetHeight();
		star_size_.cx = single_image_width;
		star_size_.cy = static_cast<long>(h);
	}

	counter_++;

/*	if (star_img_.m_hImageList == 0)
	{
		const int single_image_width= 23;
		::LoadImageList(star_img_, IDB_STARS, single_image_width, ::GetSysColor(COLOR_3DFACE));

		IMAGEINFO ii;
		if (star_img_.GetImageInfo(0, &ii))
		{
			star_size_.cx = ii.rcImage.right - ii.rcImage.left;
			star_size_.cy = ii.rcImage.bottom;
		}
		else
		{ ASSERT(false); }
	}
*/
	stars_ = 5;
	angles_.resize(stars_, 0);
	rating_ = 0;
	hot_stars_ = -1;
	background_ = ::GetSysColor(COLOR_3DFACE);
}


StarCtrl::~StarCtrl()
{
	if (--counter_ == 0)
		stars_bmp_.reset();
}


void StarCtrl::OnTimer(UINT_PTR event_id)
{
	CWnd::OnTimer(event_id);

	if (event_id == 1234)
	{
		for (int i= 0; i < angles_.size(); ++i)
			if (angles_[i])
			{
				Invalidate();
				break;
			}
	}
}


bool StarCtrl::Create(CWnd* parent, int count, int rating, const boost::function<void (int)>& on_clicked, int id)
{
	stars_ = count;
	rating_ = rating;
	on_clicked_ = on_clicked;

	CRect rect(0,0,0,0);
	rect.bottom = star_size_.cy;
	rect.right = (star_size_.cx + HORZ_SPACE) * count - HORZ_SPACE;

	if (!CWnd::Create(STAR_CTRL_CLASS, 0, WS_CHILD | WS_VISIBLE, rect, parent, id))
		return false;

	return true;
}


void StarCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	timer_id_ = SetTimer(1234, 33, 0);
}


void DrawStar(CDC& dc, Gdiplus::Bitmap* stars, CPoint pos, CSize star_size, int image, int max_angle)
{
	Gdiplus::Graphics g(dc);

	DWORD tick= ::GetTickCount();

	double angle= max_angle * sin(tick / 100.0);

	g.TranslateTransform(static_cast<float>(pos.x), static_cast<float>(pos.y));
	if (angle != 0.0)
	{
		float dx= static_cast<float>(star_size.cx) / 2.0f;
		float dy= static_cast<float>(star_size.cy) / 2.0f;
		g.TranslateTransform(dx, dy);
		g.RotateTransform(static_cast<float>(angle));
		g.TranslateTransform(-dx, -dy);
	}
	g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	g.DrawImage(stars, 0, 0, image * star_size.cx, 0, star_size.cx, star_size.cy, Gdiplus::UnitPixel);
}


BOOL StarCtrl::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	COLORREF back= background_;
	bool enabled= (GetStyle() & WS_DISABLED) == 0;

	if (stars_ > 0 && enabled)
	{
		MemoryDC dc(*dc, rect, back);

		//star_img_.SetBkColor(back);

		CPoint pos(0, 0);

		for (int i= 0; i < stars_; ++i)
		{
			int image= i < rating_ ? LIT : FAINT;
			if (rating_ < 0)
				image = i < -rating_ ? INDETERMINATE : FAINT;
			else if (i >= rating_ && i < hot_stars_)
				image = HOT;

			//star_img_.Draw(&dc, image, pos, ILD_NORMAL | ILD_TRANSPARENT);

			bool rocking= i + 1 == hot_stars_;
			if (rocking)
				angles_[i] = ANGLE * 2;
			else if (angles_[i] > 0)
				--angles_[i];

			DrawStar(dc, stars_bmp_.get(), pos, star_size_, image, angles_[i] / 2);

			pos.x += star_size_.cx + HORZ_SPACE;
		}

		dc.BitBlt();
	}
	else
		dc->FillSolidRect(rect, back);

	return true;
}


void StarCtrl::OnMouseMove(UINT flags, CPoint point)
{
	CWnd::OnMouseMove(flags, point);

	if ((flags & MK_LBUTTON) == 0)
	{
		TRACKMOUSEEVENT et;
		et.cbSize = sizeof et;
		et.dwFlags = TME_LEAVE;
		et.hwndTrack = m_hWnd;
		et.dwHoverTime = 0;

		_TrackMouseEvent(&et);

		int star= FindStar(point);
		if (star != hot_stars_)
		{
			hot_stars_ = star;
			InvalidateRect(nullptr);
		}
	}
}


LRESULT StarCtrl::OnMouseLeave(WPARAM, LPARAM)
{
	if (hot_stars_ != -1)
	{
		hot_stars_ = -1;
		InvalidateRect(nullptr);
	}

	return 0;
}


int StarCtrl::FindStar(CPoint point) const
{
	if (star_size_.cx > 0)
	{
		int w= star_size_.cx + HORZ_SPACE;
		int star= point.x / w + 1;
		if (point.y <= star_size_.cy && star >= 0 && star <= stars_)
			return star;
	}

	return -1;
}


void StarCtrl::OnLButtonDown(UINT flags, CPoint point)
{
	SetCursor(point);

	int star= FindStar(point);

	if (star >= 0 && on_clicked_ != 0)
		on_clicked_(star);
}


void StarCtrl::SetRating(int rating)
{
	rating_ = rating;
	Invalidate();
}


void StarCtrl::SetClickCallback(const boost::function<void (int)>& on_clicked)
{
	on_clicked_ = on_clicked;
}


int StarCtrl::GetStarCount() const
{
	return stars_;
}


int StarCtrl::GetRating() const
{
	return rating_;
}


void StarCtrl::SetCursor(CPoint point)
{
	HCURSOR cursor= 0;

	if (IsWindowEnabled() && FindStar(point) >= 0 && on_clicked_ != 0)
	{
		cursor = AfxGetApp()->LoadStandardCursor(IDC_HAND);
		if (cursor == 0)
			cursor = AfxGetApp()->LoadCursor(IDC_POINTING);
	}
	else
		cursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	::SetCursor(cursor);
}


BOOL StarCtrl::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		CPoint pos(0, 0);
		GetCursorPos(&pos);
		ScreenToClient(&pos);
		SetCursor(pos);
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}

void StarCtrl::SetBackgndColor(COLORREF c)
{
	background_ = c;
	if (m_hWnd)
		Invalidate();
}