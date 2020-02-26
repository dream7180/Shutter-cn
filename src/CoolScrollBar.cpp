/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CoolScrollBar.h"
#include "coolsb/coolscroll.h"
#include "BmpFunc.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CoolScrollBar::CoolScrollBar() : wnd_(0), custom_draw_(false)
{
	size_ = 0;
}


CoolScrollBar::~CoolScrollBar()
{
	ASSERT(wnd_ == 0);
}


// initialize cool scrollbar for a given window
bool CoolScrollBar::Create(HWND wnd, bool enable)
{
	if (wnd_ != 0)
	{
		ASSERT(false);
		return false;
	}

	ASSERT(wnd);

	wnd_ = wnd;

	return enable ? !!::InitializeCoolSB(wnd_) : true;
}


// uninitialize cool scrollbars during WM_DESTROY
void CoolScrollBar::Delete()
{
	if (wnd_ == 0)
	{
		ASSERT(false);
		return;
	}

	if (IsCoolScrollEnabled())
		::UninitializeCoolSB(wnd_);

	wnd_ = 0;
}


void CoolScrollBar::Enable(bool enable)
{
	if (IsCoolScrollEnabled() != enable)
	{
		if (enable)
			::InitializeCoolSB(wnd_);
		else
			::UninitializeCoolSB(wnd_);
	}
}


bool CoolScrollBar::SetMinThumbSize(UINT bar, UINT size)
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_SetMinThumbSize(wnd_, bar, size);
}


bool CoolScrollBar::IsThumbTracking() const
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_IsThumbTracking(wnd_);
}


bool CoolScrollBar::IsCoolScrollEnabled() const
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_IsCoolScrollEnabled(wnd_);
}


bool CoolScrollBar::EnableScrollBar(int S_bflags, UINT arrows)
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_EnableScrollBar(wnd_, S_bflags, arrows);
}


bool CoolScrollBar::GetScrollInfo(int fnBar, const SCROLLINFO& si) const
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_GetScrollInfo(wnd_, fnBar, const_cast<SCROLLINFO*>(&si));
}


int	 CoolScrollBar::GetScrollPos(int nBar) const
{
	ASSERT(wnd_ != 0);
	return ::CoolSB_GetScrollPos(wnd_, nBar);
}


bool CoolScrollBar::GetScrollRange(int nBar, int& minPos, int& maxPos) const
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_GetScrollRange(wnd_, nBar, &minPos, &maxPos);
}


int	 CoolScrollBar::SetScrollInfo(int nBar, const SCROLLINFO& si, bool redraw)
{
	ASSERT(wnd_ != 0);
	return ::CoolSB_SetScrollInfo(wnd_, nBar, const_cast<SCROLLINFO*>(&si), redraw);
}


int  CoolScrollBar::SetScrollPos(int nBar, int pos, bool redraw/*= true*/)
{
	ASSERT(wnd_ != 0);
	return ::CoolSB_SetScrollPos(wnd_, nBar, pos, redraw);
}


int  CoolScrollBar::SetScrollRange(int nBar, int min_pos, int max_pos, bool redraw)
{
	ASSERT(wnd_ != 0);
	return ::CoolSB_SetScrollRange(wnd_, nBar, min_pos, max_pos, redraw);
}


bool CoolScrollBar::ShowScrollBar(int bar, bool show)
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_ShowScrollBar(wnd_, bar, show);
}


// Scrollbar dimension functions
bool CoolScrollBar::SetSize(int bar, int length, int width)
{
	ASSERT(wnd_ != 0);
	size_ = width;
	return !!::CoolSB_SetSize(wnd_, bar, length, width);
}


CSize CoolScrollBar::GetSize() const
{
	return IsCoolScrollEnabled() ? CSize(size_, size_) :
		CSize(::GetSystemMetrics(SM_CXVSCROLL), ::GetSystemMetrics(SM_CYHSCROLL));
}


// Set the visual nature of a scrollbar (flat, normal etc)
bool CoolScrollBar::SetStyle(int bar, UINT style)
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_SetStyle(wnd_, bar, style);
}


bool CoolScrollBar::SetThumbAlways(int bar, bool thumb_always)
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_SetThumbAlways(wnd_, bar, thumb_always);
}


bool CoolScrollBar::SetHotTrack(int bar, bool enable)
{
	ASSERT(wnd_ != 0);
	return !!::CoolSB_SetStyle(wnd_, bar, enable ? CSBS_HOTTRACKED : CSBS_NORMAL);
}


//UINT CoolScrollBar::CustomDrawCode()
//{
//	return NM_COOLSB_CUSTOMDRAW;
//}


LRESULT CoolScrollBar::HandleCustomDraw(NMHDR* nm_hdr)
{
	NMCSBCUSTOMDRAW* nm= reinterpret_cast<NMCSBCUSTOMDRAW*>(nm_hdr);
	CDC* dc= CDC::FromHandle(nm->hdc);

	// inserted buttons do not use PREPAINT etc..
	if (nm->nBar == SB_INSBUT)
	{
//		::CoolSB_DrawProc(nm->hdc, nm->uItem, nm->uState, &nm->rect);
		return CDRF_SKIPDEFAULT;
	}

	if (!custom_draw_)
		return CDRF_DODEFAULT;

	if (nm->dwDrawStage == CDDS_PREPAINT)
	{
		if (custom_draw_)
			return CDRF_SKIPDEFAULT;
		else
			return CDRF_DODEFAULT;
	}

	if (nm->dwDrawStage == CDDS_POSTPAINT)
	{}

	// the sizing gripper in the bottom-right corner
	if (nm->nBar == SB_BOTH)
	{
		RECT *rc = &nm->rect;

		//StretchBlt(nm->hdc, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top,
		//	hdcSkin, 100, 100, 18, 18, SRCCOPY);

		grip_.Draw(dc, &nm->rect);

		return CDRF_SKIPDEFAULT;
	}
	else if (nm->nBar == SB_HORZ)
	{
		Dib& dib= horz_;

		// bitmap order from top to bottom: normal, hot, pressed
		int height= dib.GetHeight() / 3;
		CRect src(0, 0, 0, height);
		if (nm->uState == CDIS_HOT)
			src.OffsetRect(0, height);
		else if (nm->uState == CDIS_SELECTED)
			src.OffsetRect(0, 2 * height);

		CRect dst= nm->rect;
		CRect tmp= dst;

		switch (nm->uItem)
		{
		case HTSCROLL_THUMB:
			{
				src.left = parts_[THUMB_L];
				src.right = parts_[THUMB_L + 1];
				CRect r= dst;
				r.right = r.left + src.Width();
				tmp.left = r.right;
				dib.Draw(dc, r, &src);
			}
			{
				src.left = parts_[THUMB_R];
				src.right = parts_[THUMB_R + 1];
				CRect r= dst;
				r.left = r.right - src.Width();
				tmp.right = r.left;
				dib.Draw(dc, r, &src);
			}
			if (tmp.Width() > 0)
			{
				src.left = parts_[THUMB_M];
				src.right = parts_[THUMB_M + 1];
				dib.Draw(dc, tmp, &src);
			}
			break;

		case HTSCROLL_LEFT:
			src.left = parts_[LEFT];
			src.right = parts_[LEFT + 1];
			dib.Draw(dc, dst, &src);
			break;

		case HTSCROLL_RIGHT:
			src.left = parts_[RIGHT];
			src.right = parts_[RIGHT + 1];
			dib.Draw(dc, dst, &src);
			break;

		case HTSCROLL_PAGELEFT:
		case HTSCROLL_PAGERIGHT:
			//{
			//	src.left = parts_[WELL_L];
			//	src.right = parts_[WELL_L + 1];
			//	CRect r= dst;
			//	r.right = r.left + src.Width();
			//	tmp.left = r.right;
			//	dib.Draw(dc, r, &src);
			//}
			//{
			//	src.left = parts_[WELL_R];
			//	src.right = parts_[WELL_R + 1];
			//	CRect r= dst;
			//	r.left = r.right - src.Width();
			//	tmp.right = r.left;
			//	dib.Draw(dc, r, &src);
			//}
			if (dst.Width() > 0)
			{
				src.left = parts_[WELL];
				src.right = parts_[WELL + 1];
				dib.Draw(dc, dst, &src);
			}
			break;
		}

		return CDRF_SKIPDEFAULT;
	}
	else if (nm->nBar == SB_VERT)
	{
		Dib& dib= vert_;

		// bitmap order from left to right: pressed, hot, normal
		int width= dib.GetWidth() / 3;
		CRect src(0, 0, width, 0);
		if (nm->uState == CDIS_HOT)
			src.OffsetRect(width, 0);
		else if (nm->uState != CDIS_SELECTED)
			src.OffsetRect(2 * width, 0);

		CRect dst= nm->rect;
		CRect tmp= dst;

		switch (nm->uItem)
		{
		case HTSCROLL_THUMB:
			{
				src.top = parts_[THUMB_L];
				src.bottom = parts_[THUMB_L + 1];
				CRect r= dst;
				r.bottom = r.top + src.Height();
				tmp.top = r.bottom;
				dib.Draw(dc, r, &src);
			}
			{
				src.top = parts_[THUMB_R];
				src.bottom = parts_[THUMB_R + 1];
				CRect r= dst;
				r.top = r.bottom - src.Height();
				tmp.bottom = r.top;
				dib.Draw(dc, r, &src);
			}
			if (tmp.Height() > 0)
			{
				src.top = parts_[THUMB_M];
				src.bottom = parts_[THUMB_M + 1];
				dib.Draw(dc, tmp, &src);
			}
			break;

		case HTSCROLL_LEFT:
			src.top = parts_[LEFT];
			src.bottom = parts_[LEFT + 1];
			dib.Draw(dc, dst, &src);
			break;

		case HTSCROLL_RIGHT:
			src.top = parts_[RIGHT];
			src.bottom = parts_[RIGHT + 1];
			dib.Draw(dc, dst, &src);
			break;

		case HTSCROLL_PAGEGDOWN:
		case HTSCROLL_PAGEGUP:
			//{
			//	src.top = parts_[WELL_L];
			//	src.bottom = parts_[WELL_L + 1];
			//	CRect r= dst;
			//	r.bottom = r.top + src.Height();
			//	tmp.top = r.bottom;
			//	dib.Draw(dc, r, &src);
			//}
			//{
			//	src.top = parts_[WELL_R];
			//	src.bottom = parts_[WELL_R + 1];
			//	CRect r= dst;
			//	r.top = r.bottom - src.Height();
			//	tmp.bottom = r.top;
			//	dib.Draw(dc, r, &src);
			//}
			if (dst.Height() > 0)
			{
				src.top = parts_[WELL];
				src.bottom = parts_[WELL + 1];
				dib.Draw(dc, dst, &src);
			}
			break;

		default:
			{
				int a= 0;
			}
			break;
		}

		return CDRF_SKIPDEFAULT;
	}
	//INSERTED BUTTONS are handled here...
	else if (nm->nBar == SB_INSBUT)
	{
//		::CoolSB_DrawProc(nm->hdc, nm->uItem, nm->uState, &nm->rect);
		return CDRF_SKIPDEFAULT;
	}
	else
	{
		return CDRF_DODEFAULT;
	}

	//normal bitmaps, use same code for HORZ and VERT
	//StretchBlt(nm->hdc, rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top,
	//	hdcSkin, cdt->x, cdt->y, cdt->width, cdt->height, SRCCOPY);

	return CDRF_SKIPDEFAULT;
}


void CoolScrollBar::LoadImages(int horz_id, int vert_id, int grip_bmp_id, const std::vector<int>& part_sizes, double gamma)
{
	custom_draw_ = false;

	if (part_sizes.size() != COUNT)
	{
		ASSERT(false);
		return;
	}

	parts_.clear();
	parts_.reserve(COUNT + 1);

	int sum= 0;
	for (int i= 0; i < COUNT; ++i)
	{
		parts_.push_back(sum);
		sum += part_sizes[i];
	}
	parts_.push_back(sum);

	SetImages(horz_id, vert_id, grip_bmp_id, gamma);

	custom_draw_ = true;
}


void CoolScrollBar::SetImages(int horz_id, int vert_id, int grip_bmp_id, double gamma)
{
	VERIFY(horz_.Load(horz_id));
	VERIFY(vert_.Load(vert_id));
	if (grip_bmp_id) VERIFY(grip_.Load(grip_bmp_id));

	if (gamma != 1.0)
	{
		::ApplyGammaInPlace(&horz_, gamma, -1, -1);
		::ApplyGammaInPlace(&vert_, gamma, -1, -1);
		if (grip_.IsValid())
			::ApplyGammaInPlace(&grip_, gamma, -1, -1);
	}
}


int CoolScrollBar::GetScrollLimit(int bar)
{
	int min= 0, max= 0;
	GetScrollRange(bar, min, max);

	SCROLLINFO info;
	memset(&info, 0, sizeof info);
	info.cbSize = sizeof info;
	info.fMask = SIF_PAGE;
	if (GetScrollInfo(bar, info))
		max -= std::max<int>(info.nPage - 1, 0);

	return max;
}


bool CoolScrollBar::SetColors(COLORREF background, COLORREF foreground, COLORREF hottracked)
{
	return !!::CoolSB_SetColors(wnd_, background, foreground, hottracked);
}