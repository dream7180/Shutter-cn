/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000-2011 Michael Kowalski
____________________________________________________________________________*/

// ViewPane.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ViewPane.h"
#include "Config.h"
#include "ColorProfile.h"
#include "IsMMXAvailable.h"
#include "DibColorTransform.h"
#include "Block.h"
#include "CatchAll.h"
#include "PhotoInfoPtr.h"
#include "WhistlerLook.h"
#include "DirectRender.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int MOUSEHIDINGTIME= 7;		// 1.4 sec.
const double ZOOM_TO_FIT= 0.0;
const double ANIM_BUDGET= 200.0;	// max animation duration in ms
const int REPAINT_TIMER_ID= 1234;
const int REPAINT_TIMER_FRQ= 15;	// 15 ms (about 60 Hz)

/////////////////////////////////////////////////////////////////////////////
// ViewPane

enum ImageChange { NoChange= 0, LoadNew, RotatingCW, RotatingCCW, ReloadingLarger };

struct ViewPane::Impl
{
	Impl()
	{
		image_change_ = LoadNew;
		animation_duration_ = 0.0f;
		start_scale_ = target_scale_ = floating_scale_ = 1.0;
		floating_position_ = target_position_ = starting_offset_ = target_offset_ = CPoint(0, 0);
		original_scale_ = floating_angle_ = target_angle_ = 0.0;
		self_ = 0;
		eliminate_bars_ = false;
		fraction_of_image_to_hide_ = 0.3;
	}

	void StartSmoothZoom();
	void StartSmoothRotation(bool clockwise);
	void StopAnimation();
	void AnimateVars();

	ViewPane* self_;
	std::auto_ptr<DirectRender> direct_render_;	// Direct2D rendering (GPU)
	// flag set when image is changing (to update bmp cached by GPU);
	// it tells what has changed, so we can sync GPU copy appropriately
	ImageChange image_change_;
	double animation_duration_;		// amount of time transition (like smooth zoom) may take; animation duration in ms
	WndTimer repaint_timer_;		// timer for smooth zooming
	double floating_scale_;			// image scale used for smooth zooming
	double start_scale_;
	double target_scale_;			// floating scale drifts towards target scale if different, till it reaches it
	CPoint floating_position_;
	CPoint target_position_;
	CPoint target_offset_;
	CPoint starting_offset_;
	double target_angle_;			// rotation angle (if != 0)
	double floating_angle_;			// current rotation angle for smooth rotation
	double original_scale_;			// keep scale of image before it was rotated
	bool eliminate_bars_;
	double fraction_of_image_to_hide_;	// how much of an image area can be hidden when eliminating bars
};


ViewPane::ViewPane(bool coolSB/*= false*/) : impl_(new Impl())
{
	impl_->self_ = this;
	logical_zoom_ = ZOOM_TO_FIT;
	scroll_ = false;
	scrolling_ = false;
	img_pos_ = CPoint(0, 0);
	mouse_cursor_counter_ = 0;
	cursor_stay_visible_ = false;
	gamma_correction_ = true;
	next_photo_ = 0;
	display_photo_description_ = true;
	force_reloading_ = false;
	recipient_ = 0;
	alpha_blending_steps_ = 14;
	cache_ = 0;
	cur_photo_ = 0;
	scroll_bars_enabled_ = true;
	use_cool_scroll_bars_ = coolSB;
//	padding_ = CRect(8, 8, 8, 8);
	padding_.SetRectEmpty();

	range_size_ = CSize(0, 0);
	start_ = start_img_pos_ = CPoint(0, 0);

	rgb_background_ = RGB(0,0,0);

	got_capture_ = false;

	show_description_if_no_photo_ = false;
	zoom_to_fit_may_magnify_ = g_Settings.allow_magnifying_above100_;
}


ViewPane::~ViewPane()
{}

static int VIEW_WND_ID= 11111;

BEGIN_MESSAGE_MAP(ViewPane, CWnd)
	//{{AFX_MSG_MAP(ViewPane)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
//	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER + 999, OnPartialLoad)
	ON_MESSAGE(WM_USER + 998, OnOrientationChanged)
	ON_MESSAGE(WM_USER + 1000, OnImgReloadingDone)
	ON_NOTIFY_RANGE(CoolScrollBar::COOLSB_CUSTOMDRAW, 0, 0xffff, OnCustDrawScrollBar)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MBUTTONDOWN()
END_MESSAGE_MAP()

CString ViewPane::wnd_class_;

/////////////////////////////////////////////////////////////////////////////
// ViewPane message handlers


bool ViewPane::Create(CWnd* parent)
{
	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW));

	if (!CWnd::CreateEx(0, wnd_class_, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, VIEW_WND_ID))
		return false;

	cursor_hide_timer_.ResetWnd(m_hWnd);
	impl_->repaint_timer_.ResetWnd(m_hWnd);

	yellow_text_.ResetFont();

//	transparent_bar_wnd_.Create(this, -1);

	if (!scroll_bar_.Create(*this, use_cool_scroll_bars_))
		return false;

	if (use_cool_scroll_bars_)
	{
		static const int parts[]= { 21, 1, 21, 9, 1, 9 };

		std::vector<int> p(parts, parts + array_count(parts));

		scroll_bar_.LoadImages(IDB_HORZ_SCROLLBARS, IDB_VERT_SCROLLBARS, IDB_GRIP, p, 1.0);

		scroll_bar_.SetHotTrack(SB_BOTH, true);

		// for custom draw scrollbars set sizes
		scroll_bar_.SetSize(SB_BOTH, parts[0], 16);
		scroll_bar_.SetMinThumbSize(SB_BOTH, 9 + 1 + 9);
	}

	if (g_Settings.display_method_ == DIB_DIRECT_2D && Direct2D::IsAvailable())
		impl_->direct_render_.reset(new DirectRender(m_hWnd));

	return true;
}


CacheImg* ViewPane::GetCurImg() const
{
	if (cur_photo_ == 0)
		return 0;

	if (cache_)
		return cache_->FindEntry(cur_photo_);

	return 0;
}


DibPtr ViewPane::GetCurBitmap() const
{
	if (display_image_.get() != 0 && display_image_->IsValid())
		return display_image_;

	if (cur_photo_ == 0)
		return DibPtr();

	DibPtr decoder_dib= GetDecoderBitmap(decoder_.get());

	// is it being decoded now?
	if (decoder_dib.get() != 0 && decoder_->FileName() == cur_photo_->GetDisplayPath())
		return decoder_dib;

	// if not decoded find current photo in the cache
	if (cache_)
		if (CacheImg* img= cache_->FindEntry(cur_photo_))
			return img->Dib();

	return DibPtr();
}


double ViewPane::CalcPhysicalZoom(double logical_zoom, bool screen_ratio_fix/*= false*/, Dib* dib/*= 0*/) const
{
	DibPtr temp;

	if (dib == 0)
	{
		temp = GetCurBitmap();
		dib = temp.get();
	}

	if (dib == 0)
		return 0.0;

	int bmp_width= dib->GetWidth();
	int bmp_height= dib->GetHeight();
	if (bmp_width == 0 || bmp_height == 0)
		return 0.0;

	double scrn_ratio= 1.0;
	if (screen_ratio_fix)
		scrn_ratio = g_Settings.GetScreenAspectRatio();

	if (scrn_ratio >= 1.0)
		bmp_width = static_cast<int>(bmp_width * scrn_ratio);
	else
		bmp_height = static_cast<int>(bmp_height / scrn_ratio);

	if (logical_zoom == ZOOM_TO_FIT)	// zoom to fit?
	{
		CRect rect= ImageRect();

		int width= rect.Width();
		int height= rect.Height();

		if (width != bmp_width || height != bmp_height)
		{
			double ratio_org= double(bmp_width) / double(bmp_height);

			if (double(width) / double(height) > ratio_org)
				width = static_cast<int>(height * ratio_org);
			else
				height = static_cast<int>(width / ratio_org);
		}
//test:
//double d= double(height) / bmp_height;

		double zoom= double(width) / bmp_width;

		if (impl_->eliminate_bars_ && width > 0 && height > 0)	// if true, try to enlarge image to hide bars
		{
			ASSERT(impl_->fraction_of_image_to_hide_ >= 0.0 && impl_->fraction_of_image_to_hide_ < 1.0);

			int width= rect.Width();
			int height= rect.Height();

			// find magnification at which image occupies entire view

			double width_zoom= width / double(bmp_width);
			double height_zoom= height / double(bmp_height);
			double zoom_to_fill= std::max(width_zoom, height_zoom);

			if (width_zoom > height_zoom)
			{
				double new_height= bmp_height * zoom_to_fill;
				// fraction of an image hidden/cut off
				double removed_part= (new_height - height) / new_height;

				// if too much of an image would be invisible, limit zoom
				if (removed_part > impl_->fraction_of_image_to_hide_)
				{
					new_height = height / (1.0 - impl_->fraction_of_image_to_hide_);
					// correct zoom to fill
					zoom_to_fill = new_height / bmp_height;
				}
			}
			else
			{
				double new_width= bmp_width * zoom_to_fill;
				// fraction of an image hidden/cut off
				double removed_part= (new_width - width) / new_width;

				// if too much of an image would be invisible, limit zoom
				if (removed_part > impl_->fraction_of_image_to_hide_)
				{
					new_width = width / (1.0 - impl_->fraction_of_image_to_hide_);
					// correct zoom to fill
					zoom_to_fill = new_width / bmp_width;
				}
			}

			zoom = zoom_to_fill;
		}

		return zoom;
	}
	else
	{
		int reduction= GetReductionFactor();

		// if bmp is reduced physical zoom used is higher than logical
		double zoom= logical_zoom * reduction;

		return zoom;
	}
}


#if 0
float ViewPane::CalcZoom(bool screen_ratio_fix, bool scale_up) const
{
	Dib* dib= GetCurBitmap();
//	if (decoder_.get() == 0 || decoder_->Bitmap() == 0)
//		return 0.0f;

	if (dib == 0)
		return 0.0f;

	int bmp_width= dib->GetWidth();
	int bmp_height= dib->GetHeight();
	if (bmp_width == 0 || bmp_height == 0)
		return 0.0f;

	double scrn_ratio= 1.0;
	if (screen_ratio_fix && g_Settings.correctCRT_aspect_ratio_ &&
		g_Settings.vert_resolution_ > 0.0f && g_Settings.horz_resolution_ > 0.0f)
		scrn_ratio = g_Settings.horz_resolution_ / g_Settings.vert_resolution_;

	if (scrn_ratio >= 1.0)
		bmp_width = static_cast<int>(bmp_width * scrn_ratio);
	else
		bmp_height = static_cast<int>(bmp_height / scrn_ratio);

	int width= bmp_width;
	int height= bmp_height;

	CRect rect;
	GetClientRect(rect);

	if (scale_up)
	{
		width = rect.Width();
		height = rect.Height();
	}
	else
	{
		if (width > rect.Width())
			width = rect.Width();
		if (height > rect.Height())
			height = rect.Height();
	}

	if (width != bmp_width || height != bmp_height)
	{
		double ratio_org= double(bmp_width) / double(bmp_height);

		if (double(width) / double(height) > ratio_org)
			width = static_cast<int>(height * ratio_org);
		else
			height = static_cast<int>(width / ratio_org);
	}

	return 100.0f * width / bmp_width;
}
#endif


void ViewPane::DrawPhoto(CDC* dc, int lines_from, int lines_to, bool erase_bknd)
{
	if (DibPtr dib= GetCurBitmap())
//	if (decoder_.get() != 0 && decoder_->Bitmap() != 0)
	{
		int dir= 0;
		if (cur_photo_)
		{
			switch (cur_photo_->rotation_flag_ & 3)
			{
			case PhotoInfo::RF_90CW:
				dir = 1;
				break;
			case PhotoInfo::RF_90CCW:
				dir = -1;
				break;
			case PhotoInfo::RF_UPDN:
				dir = 2;
				break;
			}
		}

//		int dir= decoder_->Rotated();
		if (dir == 1 && lines_from >= 0 && lines_to > 0)
		{
			int width= dib->GetWidth(); // decoder_->Bitmap()->GetWidth();
			int to= width - lines_from;
			lines_from = width - lines_to;
			lines_to = to;
		}

		ColorProfilePtr icc= cur_photo_ != 0 ? cur_photo_->GetColorProfile() : 0;
		DrawPhoto(dc, dib.get() /*decoder_->Bitmap()*/, dir, erase_bknd, lines_from, lines_to, icc);
	}
	else if (show_description_if_no_photo_)
	{
		yellow_text_.PrintText(*dc);
	}
//	else
//		DrawPhoto(dc, 0, false, erase_bknd, 0, -1);
}


double ViewPane::GetScreenAspectRatio() const
{
	return g_Settings.GetScreenAspectRatio();
}


double ViewPane::CalcRangeSizeAndZoom(const CRect& clientRect, Dib* bmp, int& bmp_width, int& bmp_height,
										int& width, int& height, CPoint& pos_img, CSize& range) const
{
	range.cx = range.cy = 0;

	bmp_width= bmp->GetWidth();
	bmp_height= bmp->GetHeight();

	if (bmp_width == 0 || bmp_height == 0)
	{
		width = height = 0;
		return 1.0;
	}

	double scrn_ratio= GetScreenAspectRatio();

	if (scrn_ratio >= 1.0)
		bmp_width = static_cast<int>(bmp_width * scrn_ratio);
	else
		bmp_height = static_cast<int>(bmp_height / scrn_ratio);

	width = bmp_width;
	height = bmp_height;
/*
	if (lines_to < 0)
		if (rotated)
			lines_to = bmp->GetWidth();
		else
			lines_to = bmp->GetHeight();
	else
		if (rotated) // this is a hack: LINES_PER_STEP lines... to detect last msg
			final_step = rotation > 0 ? lines_to < DecoderJob::LINES_PER_STEP : lines_to == bmp->GetWidth();
		else
			final_step = lines_to == bmp->GetHeight();
*/
	double zoom= CalcPhysicalZoom(logical_zoom_, true, bmp);

	// logical_zoom_ == 0.0 means 'zoom to fit'
	if (!zoom_to_fit_may_magnify_ && logical_zoom_ == ZOOM_TO_FIT && zoom > 1.0)
		zoom = 1.0;

	width = static_cast<int>(width * zoom + 0.5);
	height = static_cast<int>(height * zoom + 0.5);

	int x_range= width - clientRect.Width();
	int y_range= height - clientRect.Height();
	bool scroll= x_range > 0 || y_range > 0;
	if (scroll)
	{
		range = CSize(x_range > 0 ? x_range : 0, y_range > 0 ? y_range : 0);
		if (pos_img.x > range.cx)
			pos_img.x = range.cx;
		if (pos_img.y > range.cy)
			pos_img.y = range.cy;

		if (logical_zoom_ == ZOOM_TO_FIT)
		{
			pos_img.x = range.cx / 2;
			pos_img.y = range.cy / 2;
		}
	}
	else
		pos_img = CPoint(0, 0);

	return zoom;
}


CPoint ViewPane::GetScrollPos() const
{
	return CPoint(scroll_bar_.GetScrollPos(SB_HORZ), scroll_bar_.GetScrollPos(SB_VERT));
}


int ViewPane::GetScrollPos(int bar) const
{
	if (scroll_bars_enabled_)
		return scroll_bar_.GetScrollPos(bar);
	else
		return bar == SB_HORZ ? img_pos_.x : img_pos_.y;
}


bool ViewPane::GetScrollInfo(int bar, SCROLLINFO* si, int mask) const
{
	ASSERT(si);
	si->fMask = mask;
	return GetScrollInfo(bar, si);
}


bool ViewPane::GetScrollInfo(int bar, SCROLLINFO* si) const
{
	return scroll_bar_.GetScrollInfo(bar, *si);
}


bool ViewPane::GetScrollRange(int bar, int& minPos, int& maxPos) const
{
	bool ret= scroll_bar_.GetScrollRange(bar, minPos, maxPos);
	if (maxPos < minPos)
		maxPos = minPos = 0;	// this is needed when there's no scrollbar enabled
	return ret;
}


int ViewPane::GetScrollLimit(int bar) const
{
	int min= 0, max= 0;
	GetScrollRange(bar, min, max);
	SCROLLINFO info;
	if (GetScrollInfo(bar, &info, SIF_PAGE))
		max -= std::max<int>(static_cast<int>(info.nPage) - 1, 0);
	return max;
}


void ViewPane::SetScrollPos(CPoint pos)
{
	scroll_bar_.SetScrollPos(SB_HORZ, pos.x, true);
	scroll_bar_.SetScrollPos(SB_VERT, pos.y, true);
}


void ViewPane::SetScrollPos(int bar, int pos)
{
	scroll_bar_.SetScrollPos(bar, pos, true);
}


void ViewPane::SetScrollInfo(CSize bmp, CSize view, CPoint pos)
{
	SCROLLINFO hsi, vsi;
	memset(&hsi, 0, sizeof hsi);
	memset(&vsi, 0, sizeof vsi);

	hsi.cbSize = sizeof hsi;
	vsi.cbSize = sizeof vsi;

	hsi.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	hsi.nMin = 0;
	hsi.nMax = bmp.cx - 1;
	hsi.nPage = view.cx;
	hsi.nPos = pos.x;

	vsi.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	vsi.nMin = 0;
	vsi.nMax = bmp.cy - 1;
	vsi.nPage = view.cy;
	vsi.nPos = pos.y;

	scroll_bar_.SetScrollInfo(SB_HORZ, hsi, true);
	scroll_bar_.SetScrollInfo(SB_VERT, vsi, true);
}


void ViewPane::SetScrollSizes(DibPtr bmp)
{
	CRect rect= ClientRect();

	int bmp_width= bmp ? bmp->GetWidth() : 0;
	int bmp_height= bmp ? bmp->GetHeight() : 0;

	if (bmp == 0 || bmp_width <= 0 || bmp_height <= 0 || IsZoomToFit() || rect.IsRectEmpty())
	{
		// Note: do not use scroll_bar_.ShowScrollBar(SB_BOTH, false) instead or resizing will cause subtle timing problems
		SetScrollInfo(CSize(0, 0), CSize(1, 1), CPoint(0, 0));
		return;
	}

	CPoint pos= img_pos_;

	// scrollbar sizes
	CSize size= scroll_bar_.GetSize();
	DWORD style= GetStyle();

	if (style & WS_VSCROLL)
		rect.right += size.cx;
	if (style & WS_HSCROLL)
		rect.bottom += size.cy;

	int width= bmp_width;
	int height= bmp_height;

	CSize range(0, 0);
	if (logical_zoom_ == ZOOM_TO_FIT)
	{
		// zoom to fit may attempt to hide bars (zoom to fill), so let's lie about sizes and hide scrollbars anyway
		width = rect.Width();
		height = rect.Height();
	}
	else
		CalcRangeSizeAndZoom(rect, bmp.get(), bmp_width, bmp_height, width, height, pos, range);

	bool horz_fits= width <= rect.Width();
	bool vert_fits= height <= rect.Height();

	if (horz_fits && vert_fits)
	{
		// img fits just fine
		width = height = 0;
		pos.x = pos.y = 0;
		rect.SetRect(0, 0, 1, 1);
	}
	else if (!horz_fits && !vert_fits)
	{
		// img doesn't fit at all
		rect.right -= size.cx;
		rect.bottom -= size.cy;
	}
	else if (!horz_fits)
	{
		// img doesn't fit only horizontally
		// a horz scrollbar will be needed; take out the space
		rect.bottom -= size.cy;

		// verify this change didn't modify height requirements
		if (height > rect.Height())
			rect.right -= size.cx;
	}
	else if (!vert_fits)
	{
		// img doesn't fit only vertically
		// a vert scrollbar will be needed; take out the space
		rect.right -= size.cx;

		// verify this change didn't modify width requirements
		if (width > rect.Width())
			rect.bottom -= size.cy;
	}

	SetScrollInfo(CSize(width, height), rect.Size(), pos);
}


void ViewPane::DrawPhoto(CDC* dc, Dib* bmp, int rotation, bool erase_bknd, int lines_from, int lines_to, ColorProfilePtr icc_profile)
{
//LARGE_INTEGER time, tm[5], frq;
//::QueryPerformanceCounter(&time);

	CRect client= ClientRect();
	CRect rect= ImageRect();

	bool partial_refresh= lines_to > 0;	// entire bitmap not yet decoded?
	bool final_step= false;				// final step of partial redrawing
	bool rotated= abs(rotation) == 1;
	bool upside_down= rotation == 2;

	ASSERT(lines_from >= 0);

	COLORREF rgb_back= rgb_background_;
//TRACE(L"lines from: %d  to:%d\n", lines_from, lines_to);
	if (lines_to == 0 ||		// first step of partial displaying an image--init
		bmp == 0 ||				// if there is no bmp just erase backgnd
		!bmp->IsValid())		// same if bmp is not yet valid //TODO: revise this checking: bmp is created in different thread
	{
		dc->FillSolidRect(client, rgb_back);
		return;
	}

	int bmp_width= bmp->GetWidth();
	int bmp_height= bmp->GetHeight();
	if (bmp_width == 0 || bmp_height == 0)
	{
		dc->FillSolidRect(client, rgb_back);
		return;
	}

	int width= bmp_width;
	int height= bmp_height;

	double zoom= CalcRangeSizeAndZoom(rect, bmp, bmp_width, bmp_height, width, height, img_pos_, range_size_);

	if (lines_to < 0)
		if (rotated)
			lines_to = bmp->GetWidth();
		else
			lines_to = bmp->GetHeight();
	else
		if (rotated) // this is a hack: LINES_PER_STEP lines... to detect last msg
			final_step = rotation > 0 ? lines_to < DecoderJob::LINES_PER_STEP : lines_to == bmp->GetWidth();
		else
			final_step = lines_to == bmp->GetHeight();

	scroll_ = range_size_.cx > 0 || range_size_.cy > 0;

	// bitmap source rectangle ---------------------------------
	CRect src_rect;
	if (rotated)
		src_rect.SetRect(lines_from, 0, lines_to, bmp->GetHeight());
	else if (upside_down)
		src_rect.SetRect(0, bmp->GetHeight() - lines_to, bmp->GetWidth(), bmp->GetHeight() - lines_from);
	else
		src_rect.SetRect(0, lines_from, bmp->GetWidth(), lines_to);

	// bitmap destination rectangle ----------------------------
	CRect dest_rect= rect;

	// make a correction for non-square pixels on the output device (by stretching dest rect)
	double scrn_ratio= GetScreenAspectRatio();

	if (rotated)
	{
		if (scrn_ratio >= 1.0)
		{
			lines_from = static_cast<int>(lines_from * scrn_ratio);
			lines_to = static_cast<int>(lines_to * scrn_ratio);
		}
	}
	else
	{
		if (scrn_ratio < 1.0)
		{
			lines_from = static_cast<int>(lines_from / scrn_ratio);
			lines_to = static_cast<int>(lines_to / scrn_ratio);
		}
	}

	if (width < dest_rect.Width())
		dest_rect.left = (dest_rect.Width() - width) / 2;
	else
		dest_rect.left = 0;

	if (height < rect.Height())
		dest_rect.top = (dest_rect.Height() - height) / 2;
	else
		dest_rect.top = 0;

	CPoint left_top= dest_rect.TopLeft();

	// this magic value minimizes image shift when drawing partial images
	const double ROUNDING= 0.55;

	CRect dest_whole_bmp_rect(dest_rect.TopLeft(), CSize(width, height));

	if (partial_refresh)
	{
		// rounding errors make img 'floating'
		if (rotated)
			width = int(lines_to * zoom + ROUNDING);
		else
			height = int(lines_to * zoom + ROUNDING);
	}

	dest_rect.right = dest_rect.left + width;
	dest_rect.bottom = dest_rect.top + height;

	if (partial_refresh)
	{
		// rounding errors make img 'floating'
		if (upside_down)
		{
			int h= dest_whole_bmp_rect.Height();
			int from= int(lines_from * zoom + ROUNDING);
			dest_rect.bottom = left_top.y + h - from;
			dest_rect.top = dest_rect.bottom - (height - from);
			ASSERT(dest_rect.top >= 0);
		}
		else if (rotated)
			dest_rect.left = left_top.x + int(lines_from * zoom + ROUNDING);
		else
			dest_rect.top = left_top.y + int(lines_from * zoom + ROUNDING);
	}

	// prepare bitmap; apply color correction if needed

	AutoPtr<Dib> trans_bmp;
	if (gamma_correction_ && transform_.IsValid())
	{
		try
		{
			trans_bmp = DibColorTransform(*bmp, icc_profile != 0 ? icc_profile : transform_.in_,
				transform_.out_, transform_.rendering_intent_, partial_refresh ? &src_rect : 0);
		}
		catch (int)
		{}

		if (trans_bmp.get())
			bmp = trans_bmp.get();
	}

	// use GPU to paint image?
	bool rendered= false;
	bool rotating= false;
	// if dc is memory-based, Direct2D won't work, it renders directly into window
	if (impl_->direct_render_.get() && WindowFromDC(dc->GetSafeHdc()) == m_hWnd)
	{
		if (impl_->image_change_ == LoadNew || !impl_->direct_render_->IsBitmapReady())
		{
			// if new image is being loaded, create corresponding GPU bitmap;
			// if cached bitmap maintained by GPU is not ready, create one
			impl_->floating_scale_ = zoom;
			impl_->floating_position_ = img_pos_;

			bool copy_pixel_data= !partial_refresh;
TRACE(L"PrepareBitmap (%p)\n", this);
			if (!impl_->direct_render_->PrepareBitmap(*bmp, rgb_back, copy_pixel_data))
				impl_->direct_render_->DeleteBitmap();

			impl_->image_change_ = NoChange;	// reset flags regardless of the outcome of bmp preparation above
		}

		int rotate_dir= 0;
		// rotating image?
		if (impl_->direct_render_->IsBitmapReady() && (impl_->image_change_ == RotatingCW || impl_->image_change_ == RotatingCCW))
		{
			rotate_dir = impl_->image_change_ == RotatingCW ? 1 : -1;
			impl_->direct_render_->KeepCopy();		// keep copy of current bitmap
		}

		// size mismatch?
		if (impl_->direct_render_->IsBitmapReady() && impl_->direct_render_->GetBitmapSize() != bmp->GetSize())
		{
			bool ok= false;
TRACE(L"size mismatch (%p)\n", this);

			if (impl_->image_change_ == ReloadingLarger)
				ok = impl_->direct_render_->RescaleBitmap(bmp->GetSize());
			else if (impl_->image_change_ == RotatingCW || impl_->image_change_ == RotatingCCW)
				ok = impl_->direct_render_->RotateBitmap(impl_->image_change_ == RotatingCW);
			else
				ok = impl_->direct_render_->PrepareBitmap(*bmp, rgb_back, !partial_refresh);

			if (!ok)
				impl_->direct_render_->DeleteBitmap();

			impl_->image_change_ = NoChange;
		}

		// detect rotation of square bitmap (it won't be handled above)
		if (impl_->direct_render_->IsBitmapReady() && (impl_->image_change_ == RotatingCW || impl_->image_change_ == RotatingCCW))
		{
			//todo: rotate bitmap
			impl_->direct_render_->RotateBitmap(impl_->image_change_ == RotatingCW);
			impl_->image_change_ = NoChange;
		}

		impl_->target_scale_ = zoom;
		impl_->target_position_ = img_pos_;

		if (rotate_dir != 0)
		{
			if (recipient_)
				recipient_->AnimationStarts();
			// start rotation animation
			impl_->StartSmoothRotation(rotate_dir > 0);
		}

		if (impl_->repaint_timer_.IsRunning())
		{
			impl_->AnimateVars();

			if (!impl_->repaint_timer_.IsRunning())
				impl_->direct_render_->ReleaseCopy();	// copy bmp used for smooth rotation
		}
		else
		{
			impl_->floating_position_ = img_pos_;
			impl_->floating_scale_ = zoom;
		}

		if (impl_->direct_render_->IsBitmapReady())
		{
			if (partial_refresh)
			{
				// either new image is being loaded or current image at higher resolution is being decoded;
				// copy decoded lines into cached bitmap kept by GPU
				impl_->direct_render_->CopyIntoBitmap(*bmp, src_rect);

				if (final_step)
					impl_->image_change_ = NoChange;
			}

			float opacity= 1.0f;

			CPoint offset= -impl_->floating_position_;
			double w= bmp_width * impl_->floating_scale_;
			double h= bmp_height * impl_->floating_scale_;

			if (scrn_ratio >= 1.0)
				w *= scrn_ratio;
			else
				h /= scrn_ratio;

			// center image if it's smaller than available wnd area
			if (w < rect.Width())
				offset.x = static_cast<int>((rect.Width() - w) / 2);
			if (h < rect.Height())
				offset.y = static_cast<int>((rect.Height() - h) / 2);

			offset.x += rect.left;
			offset.y += rect.top;

	//TRACE("scale: %f, offset: %d,%d \n", floating_scale_, offset.x, offset.y);
			double scale_x= impl_->floating_scale_;
			double scale_y= impl_->floating_scale_;

			// if pixel aspect ratio != 1 then adjust scale; make image bigger, not smaller
			if (scrn_ratio > 1.0)
				scale_x *= scrn_ratio;
			else if (scrn_ratio < 1.0)
				scale_y /= scrn_ratio;

//floating_position_.x = max(0, static_cast<int>((target_offset_.x - starting_offset_.x) * floating_scale_ / start_scale_ - target_offset_.x));

//TRACE(L"offset (%d, %d) z %.2f\n", offset.x, offset.y, scale_x);

//TRACE(L"%02d. (%d, %d) z %.2f %d, %.2f, %.2f \n", repaint_step_, offset.x, offset.y, floating_scale_, target_offset_.x, floating_scale_ ,start_scale_);
	//TRACE(L"%02d. (%.1f, %.1f) z %.2f\n", repaint_step_, offset.x / floating_scale_, offset.y / floating_scale_, floating_scale_);
//::QueryPerformanceCounter(&tm[0]);
			HRESULT hr= S_FALSE;
			if (impl_->target_angle_ != 0.0 && logical_zoom_ == ZOOM_TO_FIT && impl_->direct_render_->IsBitmapCopyReady())
			{
				rotating = true;
				// if target angle != 0, then we are in the middle of rotation animation
				// currently rotation animation is only supported for "zoom to fit" images
//TRACE(L"angle %.1f\n", impl_->floating_angle_);
				float angle= static_cast<float>(impl_->floating_angle_);
				if (angle == 0)
					angle = 0.001f;		// HACK: if angle is not zero, 'offset' below is ignored

				hr = impl_->direct_render_->RenderCopy(offset, static_cast<float>(scale_x), static_cast<float>(scale_y), static_cast<float>(angle), rgb_back, opacity);
			}

			if (hr != S_OK)
				hr = impl_->direct_render_->Render(offset, static_cast<float>(scale_x), static_cast<float>(scale_y), rgb_back, opacity);

			if (hr == S_OK)
				rendered = true;
//::QueryPerformanceCounter(&tm[1]);
		}
	}

	// fallback if no Direct2D available
	if (!rendered)
	{
		// erase background (around the bitmap)
		if (!partial_refresh || erase_bknd)
		{
			CRect fill_rect= rect;
			fill_rect.right = dest_rect.left;
			if (fill_rect.Width() > 0)
			{
				dc->FillSolidRect(fill_rect, rgb_back);
				if (display_photo_description_)
					yellow_text_.GetDC()->FillSolidRect(fill_rect, rgb_back);;
			}
			fill_rect.left = dest_rect.right;
			fill_rect.right = rect.right;
			if (fill_rect.Width() > 0)
			{
				dc->FillSolidRect(fill_rect, rgb_back);
				if (display_photo_description_)
					yellow_text_.GetDC()->FillSolidRect(fill_rect, rgb_back);;
			}
			
			fill_rect = rect;
			fill_rect.bottom = dest_rect.top;
			if (fill_rect.Height() > 0)
			{
				dc->FillSolidRect(fill_rect, rgb_back);
				if (display_photo_description_)
					yellow_text_.GetDC()->FillSolidRect(fill_rect, rgb_back);;
			}
			fill_rect.top = dest_rect.bottom;
			fill_rect.bottom = rect.bottom;
			if (fill_rect.Height() > 0)
			{
				dc->FillSolidRect(fill_rect, rgb_back);
				if (display_photo_description_)
					yellow_text_.GetDC()->FillSolidRect(fill_rect, rgb_back);;
			}
		}

		CPoint org= dc->SetWindowOrg(CPoint(img_pos_ - rect.TopLeft()));

		bool bmp_drawn= false;

		// if zoom to fit is selected and source bmp is smaller than screen use a filter to
		// magnify it so it appears smooth rather than blocky
		if ((!partial_refresh || final_step) && IsZoomToFit() && zoom > 1.0)
		{
			// commented out, because Windows takes care of smoothing
	//		if (MagnifyBitmap(*bmp, zoom, zoom, dib_buffer_, false))
			{
				//dib_buffer_.DrawBitmap(dc, dest_whole_bmp_rect, 0, g_Settings.display_method_);
				bmp->DrawBitmap(dc, dest_whole_bmp_rect, 0, DIB_SMOOTH_DRAW);
				bmp_drawn = true;
			}
		}

		if (!bmp_drawn)
		{
			DibDispMethod display_method= g_Settings.display_method_;
			if (display_method == DIB_SMOOTH_DRAW && partial_refresh && zoom > 1.0)
				display_method = DIB_FAST_DRAW;	// when magnifying partial bmp do not use smooth draw or
												// bmp fragments will have sharp transitions
			bmp->DrawBitmap(dc, dest_rect, partial_refresh ? &src_rect : 0, display_method);
	//TRACE(L"dest rect: %d, %d  %d, %d\n", dest_rect.left, dest_rect.top, dest_rect.right, dest_rect.bottom);
		}

		dc->SetWindowOrg(org);
	}

	// picture description only when photo fits on the screen to prevent scrolling problems
	if (!IsScrollingNeeded() && !rotating && display_photo_description_)
	{
		bmp->DrawBitmap(yellow_text_.GetDC(), dest_rect, partial_refresh ? &src_rect : 0, g_Settings.display_method_);
		if (!partial_refresh || final_step)
			yellow_text_.PrintText(*dc);
	}

//::QueryPerformanceCounter(&tm[2]);
//::QueryPerformanceFrequency(&frq);
//
//tm[2].QuadPart -= tm[1].QuadPart;
//tm[1].QuadPart -= tm[0].QuadPart;
//tm[0].QuadPart -= time.QuadPart;
//
//wchar_t buf[50];
//::OutputDebugString(_itow(tm[0].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");
//
//::OutputDebugString(_itow(tm[1].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");
//
//::OutputDebugString(_itow(tm[2].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n\n");
}


/*
void CopyJpeg(Dib* src_bmp, CBitmap& dest_bmp)
{
	BITMAP bm;
	dest_bmp.GetObject(sizeof(bm), &bm);

	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = src_bmp->GetWidth();
	bi.bmiHeader.biHeight = src_bmp->GetHeight();
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = src_bmp->GetBitsPerPixel();
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	{
		CDC dest_dc;
		dest_dc.CreateCompatibleDC(0);
		dest_dc.SelectObject(&dest_bmp);
		dest_dc.SetStretchBltMode(HALFTONE);
		::StretchDIBits(dest_dc, 0, 0, bm.bmWidth, bm.bmHeight, 0, 0, src_bmp->GetWidth(), src_bmp->GetHeight(), src_bmp->GetBits(), &bi, DIB_RGB_COLORS, SRCCOPY);
	}
} */


LRESULT ViewPane::OnOrientationChanged(WPARAM job_id, LPARAM orientation)
{
	//this is no longer necessary
/*
	WPARAM main_job_id= reinterpret_cast<WPARAM>(decoder_.get());

	uint32 rotation_info= (orientation & 3) | PhotoInfo::RF_AUTO;

	if (main_job_id == job_id)
	{
		if (decoder_.get() != 0)
		{
			PhotoInfoPtr photo= const_cast<PhotoInfoPtr>(decoder_->GetPhoto());
			photo->rotation_flag_ = rotation_info;
		}
	}
	else
	{
		if (decoder_next_.get() != 0)
		{
			PhotoInfoPtr photo= const_cast<PhotoInfoPtr>(decoder_next_->GetPhoto());
			photo->rotation_flag_ = rotation_info;
		}
	}
*/
	return 0;
}


//#define TEST_DECODING_SPEED
#ifdef TEST_DECODING_SPEED
LARGE_INTEGER timer;
#endif

// This function is called by decoder (by posting a msg)
// Params provide info about newly available bitmap part: <lines_from..lines_to)
//
LRESULT ViewPane::OnPartialLoad(WPARAM job_id, LPARAM lines_from_to)
{
	// check if it's a msg from main decoder (otherwise it's msg from next/prev photo)
	UINT_PTR main_job_id= GetDecoderUniqueId(decoder_.get());
	if (main_job_id != job_id)
		return 0;

	bool erase_bknd= false;
	int lines_to= HIWORD(lines_from_to);
	int lines_from= LOWORD(lines_from_to);
	if (lines_from == 0xffff)
	{
		lines_from = -1;
		erase_bknd = true;
	}

	if (lines_to == 0)	// start?
	{
#ifdef TEST_DECODING_SPEED
//::QueryPerformanceFrequency(&timer);
::QueryPerformanceCounter(&timer);
#endif
		// decoding started: reduction factor is now established, so refresh status zoom info (if any)
		if (recipient_)
			recipient_->DecodingStarted(this);

		if (scroll_bars_enabled_)
			SetScrollSizes(GetCurBitmap());
	}
	else if (lines_to > 0 && lines_from != -1)
	{
	}
	else if (lines_from == -1)	// decoding finished?
	{
#ifdef TEST_DECODING_SPEED
//CClientDC dc(this);
//DrawPhoto(&dc, 0, -1, true);			// display whole photo
LARGE_INTEGER fin;
::QueryPerformanceCounter(&fin);
DWORD timer= fin.QuadPart - timer.QuadPart;
TCHAR buf[256];
swprintf(buf, L"time: %d", timer);
::MessageBox(m_hWnd, buf, _T("Speedo"), MB_OK);
#endif
		if (recipient_)
			recipient_->DecodingFinished(this);

		if (next_photo_ != 0)
		{
			CRect rect= ClientRect();

			bool auto_rotate= false;//!next_photo_->OrientationAltered();
			decoder_next_ = new DecoderJob(*next_photo_, CSize(next_photo_->GetWidth(), next_photo_->GetHeight()),
				auto_rotate, logical_zoom_, rect.Size(), this, next_photo_->rotation_flag_);
		}
		return 0;
	}

	CClientDC dc(this);
	if (lines_from < 0)
		lines_from = 0;

	// note: current lines_from and previous lines_to are same, so partial image to redraw overlap, but
	// that helps a bit: rounding errors are same

	// make sure current photo is the one decoder works on
	if (DecodingCurrentPhoto())
//		Invalidate();
		DrawPhoto(&dc, lines_from, lines_to, erase_bknd);

	return 0;
}


bool ViewPane::DecodingCurrentPhoto() const
{
	DibPtr dib= GetDecoderBitmap(decoder_.get());
	return cur_photo_ != 0 && dib.get() != 0 && decoder_->FileName() == cur_photo_->GetDisplayPath();
}


void ViewPane::DisplayBitmap(const Dib& bmp)
{
	display_image_.reset(new Dib());
	display_image_->Clone(bmp);
	Invalidate();
}


// LoadPhoto prepares some variables and starts decoding
//
void ViewPane::LoadPhoto(const PhotoInfo& inf, ConstPhotoInfoPtr next, UINT flags)
{
	display_image_.reset();

	CRect rect= ClientRect();

	//TODO: auto-rotate should be decided here since it is already done in ViewPane::OnPartialLoad()
	bool auto_rotate= false; //!!(flags & AUTO_ROTATE);
	bool force_reloading= !!(flags & FORCE_RELOADING);
	bool alpha_blending= !!(flags & ALPHA_BLENDING);

	if (cur_photo_ == &inf && !force_reloading)
		return;		// same photo requested--ignore

	DibPtr existing_bmp= GetCurBitmap();

//	if (scroll_bars_enabled_)
//		SetScrollSizes(GetCurBitmap());

	cur_photo_ = &inf;

	try
	{
		if (decoder_.get() != 0 && decoder_->FileName() == inf.GetDisplayPath() && !force_reloading && !force_reloading)
			return;		// same photo requested--ignore

		// image is about to change, refresh copy kept by GPU
		impl_->image_change_ = LoadNew;

		// prepare bmp for description text
		if (recipient_)
			yellow_text_.Prepare(this, recipient_->PreparePhotoDescriptionText(this, inf));
		else
			yellow_text_.Prepare(this, inf.photo_desc_);
//TODO: allow date and time by request
//		yellow_text_.Prepare(this, inf.photo_desc_.empty() ? inf.DateTime() : inf.photo_desc_ + L"\n" + inf.DateTime());

		CacheImg* img= 0;
TRACE(L"loading photo\n");
		if (decoder_next_.get() != 0 && decoder_next_->FileName() == inf.GetDisplayPath())
		{
TRACE(L"using next decoder\n");
			AutoPtr<DecoderJob> existing= decoder_;
			decoder_ = decoder_next_;
			//auto_rotation_info_ = auto_rotation_info_next_;
			//auto_rotation_info_next_ = 0;
			CClientDC dc(this);
			if (decoder_->Pending())
			{
				next_photo_ = next;
				DrawPhoto(&dc, 0, 0, true);				// erase bkgnd
				DrawPhoto(&dc, 0, decoder_->LinesReady(), true);	// display available part
			}
			else
			{
				if (alpha_blending)
				{
					// here we have both existing bitmap and the next one already loaded;
					// do alpha blending:
					DrawAlphaBlend(dc, existing.get(), decoder_.get());
				}

				next_photo_ = 0;

				DrawPhoto(&dc, 0, -1, true);			// display whole photo
				if (next)
				{
					decoder_next_ = new DecoderJob(*next, CSize(next->GetWidth(), next->GetHeight()),
						auto_rotate, logical_zoom_, rect.Size(), this, next->rotation_flag_);
				}
			}
		}
		else if (cache_ && (img = cache_->FindEntry(&inf)) != 0)
		{
TRACE(L"using cached img\n");
			//TODO: force_reloading_ is ignored for now

			//TODO: temporarily NOT checking if next photo is in cache (to do it reliably size has to be taken into account)
			if (next) // && cache_ && cache_->FindEntry(next) == 0)	// next photo not found in cache?
			{
TRACE(L"  start next decoder\n");
				// start reloading next photo
				decoder_next_ = new DecoderJob(*next, CSize(next->GetWidth(), next->GetHeight()),
					auto_rotate, logical_zoom_, rect.Size(), this, next->rotation_flag_);
			}

			bool reloading= false;

			if (img->ReductionFactor() > 1)		// found image is reduced in size?
			{
				//int zoom= zoom_ > 0 ? zoom_ : static_cast<int>(CalcZoom(false, true));
				double zoom= logical_zoom_; // == 0.0 ? 0.0 : CalcPhysicalZoom(logical_zoom_);
				reloading = ReloadPhotoIfNeeded(img->Dib(), img->ReductionFactor(), img->OriginalSize(), false, zoom);
			}

			if (!reloading)
			{
TRACE(L"  no reloading\n");
				CClientDC dc(this);

				if (alpha_blending && existing_bmp && img->Dib() && cur_photo_)
				{
					// here we have both existing bitmap and the next one already loaded;
					// do alpha blending:
					DrawAlphaBlend(dc, existing_bmp.get(), cur_photo_->GetColorProfile(), img->Dib().get(), img->ColorProfile());
				}
			}
			else
			{
				// image found in cache is too small and will be reloaded; but use it for the time being
				if (impl_->direct_render_.get() && img->Dib()->IsValid())
				{
					bool copy_pixel_data= true;
					if (impl_->direct_render_->PrepareBitmap(*img->Dib(), rgb_background_, copy_pixel_data))
						impl_->image_change_ = ReloadingLarger;
					else
					{
						impl_->direct_render_->DeleteBitmap();
						impl_->image_change_ = LoadNew;
					}
				}
			}

			Invalidate();
		}
		else
		{
TRACE(L"starting new decoder\n");
			//auto_rotation_info_ = auto_rotation_info_next_ = 0;
			next_photo_ = next;
			decoder_ = new DecoderJob(inf, CSize(inf.GetWidth(), inf.GetHeight()),
				auto_rotate, logical_zoom_, rect.Size(), this, inf.rotation_flag_);
			decoder_next_.free();
			force_reloading_ = false;
			Invalidate();
		}

		if (scroll_bars_enabled_)
			SetScrollSizes(GetCurBitmap());

	}
	CATCH_ALL
}


BOOL ViewPane::OnEraseBkgnd(CDC* dc)
{
	if (GetCurBitmap() == 0)
	{
		CRect rect= ClientRect();
		dc->FillSolidRect(rect, rgb_background_);
	}

	return true;
}


/*
 *	Image zoomed to fit; scale z1
 *
 *	   cx          ix
 *	|------+--------+---+------|
 *	|      |        .   |      |
 *	|      |        .   |      |
 *	|      |        .   |      |
 *	|      |        .   |      |
 *	|      |        .   |      |
 *	|      |        o   |      |
 *	|      |            |      |
 *	|------+------------+------|
 *
 *	(X coordinate only)
 *	point in a view: vx;
 *	point in an image: ix;
 *	offset to center image in a view: cx;
 *
 *	vx = cx + ix
 *	ix = vx - cx
 *
 *
 *	Big image, small view; scale z2
 *
 *	    ox
 *	|-------+------------------|
 *	|       .                  |
 *	|       .                  |
 *	|       .       vx         |
 *	|       +--------+---+     |
 *	|       |        .   |     |
 *	|       |        o   |     |
 *	|       +------------+     |
 *	|--------------------------|
 *
 *	(X coordinate only)
 *	point in a view: vx (view corner to point in an image);
 *	image offset: ox (distance from image corner to view corner);
 *
 *
 *	For both views to refer to the same point 'o' on the image:
 *
 *	ox + vx   ix   vx - cx
 *	------- = -- = -------
 *	  z2      z1     z1
 *
 *	___________________________
 *
 *	ox = z2/z1 * (vx - cx) - vx
 *	___________________________
 *
 *	This equation is used to calculate image offset and keep selected point fixed in view.
 *	That is clicked point (when dbl clicking) or point currently at the center of view if zooming in/out.
 */

void ViewPane::SetLogicalZoomAndOffset(double zoom, bool zoom_to_fit_can_magnify, CPoint offset)
{
	if (scrolling_)
		return;	// sorry, not now...

	double old_zoom= CalcPhysicalZoom(logical_zoom_);
	double new_zoom= CalcPhysicalZoom(zoom);
	CPoint old_pos= img_pos_;

	if (zoom == ZOOM_TO_FIT && new_zoom > 1.0 && !zoom_to_fit_can_magnify)
		new_zoom = 1.0;

	if (old_zoom > 0.0)
	{
		CRect client= ImageRect();

		if (offset.x < 0 || offset.y < 0)	// no offset given?
			offset = client.CenterPoint();

		if (offset.x >= 0 && offset.y >= 0)	// new offset given?
		{
			CRect rect= GetImageRect();

			// image smaller than window is centered, take centering into account
			// image bigger than window may be shifted, take offset into account
			int cx= 0;
			if (client.Width() > rect.Width())
				cx = (client.Width() - rect.Width()) / 2;
			else
				cx = -img_pos_.x;

			int cy= 0;
			if (client.Height() > rect.Height())
				cy = (client.Height() - rect.Height()) / 2;
			else
				cy = -img_pos_.y;

			img_pos_.x = std::max(0, static_cast<int>((offset.x - cx) * new_zoom / old_zoom - offset.x));
			img_pos_.y = std::max(0, static_cast<int>((offset.y - cy) * new_zoom / old_zoom - offset.y));

			impl_->target_offset_ = offset;
			impl_->starting_offset_ = CSize(cx, cy);
		}
	}

	logical_zoom_ = zoom;
	zoom_to_fit_may_magnify_ = zoom_to_fit_can_magnify;

	// if photo is reduced in size image may have to be reloaded at higher resolution
	bool reloaded= false;
	if (GetReductionFactor() > 1)
		reloaded = ReloadCurrentPhoto(zoom);	// pass in logical zoom

	impl_->target_scale_ = new_zoom;
	impl_->target_position_ = img_pos_;

	// smooth zooming
	if (impl_->image_change_ == LoadNew || old_zoom <= 0.0 || reloaded)
	{
		//TODO: handle reloaded img case gracefully

		// old zoom was invalid or image has been changed
		impl_->start_scale_ = impl_->floating_scale_ = new_zoom;
		impl_->StopAnimation();
	}
	else
	{
		impl_->start_scale_ = impl_->floating_scale_ = old_zoom;

		if (impl_->direct_render_.get() != 0 && impl_->direct_render_->IsBitmapReady())
		{
			impl_->StartSmoothZoom();
			impl_->AnimateVars();
		}
	}

	Invalidate();

	if (scroll_bars_enabled_)
		SetScrollSizes(GetCurBitmap());
	else
	{
		int a= 0;
		//Dib* bmp= GetCurBitmap();

		//int width= bmp ? bmp->GetWidth() : 0;
		//int height= bmp ? bmp->GetHeight() : 0;

		//CRect rect(0,0,0,0);
		//GetClientRect(rect);
		//CSize range(0, 0);
		//int w= 0, h= 0;
		//CPoint pos(0, 0);
		//CalcRangeSizeAndZoom(rect, bmp, w, h, width, height, pos, range_size_);
	}
}


void ViewPane::SetLogicalZoom(double zoom, bool zoom_to_fit_can_magnify)
{
	SetLogicalZoomAndOffset(zoom, zoom_to_fit_can_magnify, CPoint(-1, -1));
}


double ViewPane::GetLogicalZoom() const
{
	if (logical_zoom_ != ZOOM_TO_FIT)
		return logical_zoom_;

	// zoom to fit is selected

	double zoom= CalcPhysicalZoom(ZOOM_TO_FIT);
	int reduction= GetReductionFactor();

	if (zoom == 0.0 || reduction == 0)
		return 0.0;

	return zoom / reduction;
}

#if 0
void ViewPane::Zoom(int zoom, bool relative_change/*= true*/)
{
	if (scrolling_)
		return;	// sorry, not now...

	if (zoom != zoom_)
	{
		int old_zoom= abs(zoom_);
		zoom_ = zoom;
		int new_zoom= zoom > 0 ? zoom : static_cast<int>(CalcZoom(false, true));

		if (old_zoom)
		{
			img_pos_.x = img_pos_.x * new_zoom / old_zoom;
			img_pos_.y = img_pos_.y * new_zoom / old_zoom;
		}

		// if photo is reduced in size image may have to be reloaded at higher resolution
		if (GetReductionFactor() > 1)
			ReloadCurrentPhoto(new_zoom, relative_change);

		Invalidate();
	}
}
#endif


void ViewPane::OnLButtonDown(UINT flags, CPoint point)
{
	if (recipient_)
		recipient_->ViewClicked(this);

	if (IsScrollingNeeded())
	{
		::SetCursor(AfxGetApp()->LoadCursor(IDC_SCROLL));
		start_ = point;
		start_img_pos_ = img_pos_;
		SetCapture();
		scrolling_ = true;
		got_capture_ = true;
	}
}


void ViewPane::OnMouseMove(UINT flags, CPoint point)
{
	static CPoint mouse(-9999, -9999);

	if (mouse != point)
	{
		mouse = point;
		if (mouse_cursor_counter_ > MOUSEHIDINGTIME)
			mouse_cursor_counter_ = 0;
	}

	if (scrolling_)
	{
		CSize delta_size= point - start_;
		CPoint img_pos= start_img_pos_;
		img_pos -= delta_size;

		if (ScrollTo(img_pos) && recipient_)
			recipient_->ViewScrolled(this, img_pos_);
	}
}


bool ViewPane::ScrollTo(CPoint pos)
{
	if (!IsScrollingNeeded())
		return false;

	CPoint img_pos= pos;

	if (img_pos.x > range_size_.cx)
		img_pos.x = range_size_.cx;
	else if (img_pos.x < 0)
		img_pos.x = 0;

	if (img_pos.y > range_size_.cy)
		img_pos.y = range_size_.cy;
	else if (img_pos.y < 0)
		img_pos.y = 0;

	if (img_pos == img_pos_)
		return false;

	CSize scroll_size= img_pos_ - img_pos;
	img_pos_ = img_pos;
	impl_->floating_position_ = img_pos;
	CRect rect= ClientRect();
//	CRgn rgn;
//	rgn.CreateRectRgnIndirect(rect);
	ScrollWindowEx(scroll_size.cx, scroll_size.cy, 0, rect, 0/*&rgn*/, 0, SW_INVALIDATE);
//	Invalidate(0);
//	InvalidateRgn(&rgn, 0);
//	UpdateWindow();

	if (scroll_bars_enabled_)
		SetScrollPos(img_pos_);

	UpdateWindow();

	return true;
}


BOOL ViewPane::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (mouse_cursor_counter_ > MOUSEHIDINGTIME)
	{
		::SetCursor(NULL);		// hide mouse cursor
		return true;
	}

	return CWnd::OnSetCursor(wnd, hit_test, message);
}


void ViewPane::OnLButtonUp(UINT flags, CPoint point)
{
	if (IsScrollingNeeded())
	{
		if (got_capture_)
		{
			::SetCursor(::LoadCursor(0, IDC_ARROW));
			ReleaseCapture();
			got_capture_ = false;
		}
		scrolling_ = false;
	}
}


void ViewPane::Rotate(bool clockwise)
{
	try
	{
		double zoom= 0.0;
		if (DecodingCurrentPhoto())
		{
			//TODO:
			// this is totally doable, but painful, so I'm skipping it for now
			//
			// rotate bitmap, and make DecoderJob aware of it so it can handle that too

			return;
		}
		else
		{
			if (CacheImg* img= GetCurImg())
				if (img->Dib())
				{
					zoom = CalcPhysicalZoom(logical_zoom_, true, img->Dib().get());
					DibPtr copy(img->Dib()->RotateCopy(clockwise).release());
					if (copy.get())
						img->SetDib(copy, 0);
				}
		}

		if (impl_->image_change_ == NoChange && zoom > 0.0)
		{
			impl_->image_change_ = clockwise ? RotatingCW : RotatingCCW;
			impl_->original_scale_ = zoom;
		}

		if (scroll_bars_enabled_)
			SetScrollSizes(GetCurBitmap());
	}
	catch (...)
	{
		//TODO: log
	}
}


void ViewPane::Impl::StartSmoothZoom()
{
	int img_size= 1;
	CSize size= direct_render_->GetBitmapSize();
	if (size != CSize(0, 0))
		img_size = std::max(size.cx, size.cy);

	const int MAX_STEPS= 12;
	int STEPS= 0;

	// amount of pixels img will be resized due to the change in scale
	int travel= abs(static_cast<int>(img_size * (target_scale_ - start_scale_)));

	// limit amount of transition steps for small changes in scale;
	// that way small adjustments will be carried out faster
	STEPS = travel / 5;
	if (STEPS <= 0)
		STEPS = 1;
	else if (STEPS == 1)
		STEPS = std::min(2, MAX_STEPS);
	else if (STEPS > MAX_STEPS)
		STEPS = MAX_STEPS;

	animation_duration_ = ANIM_BUDGET * STEPS / MAX_STEPS;

	target_angle_ = 0.0;	// no rotation animation

	repaint_timer_.Start(REPAINT_TIMER_ID, REPAINT_TIMER_FRQ);
}


void ViewPane::Impl::StartSmoothRotation(bool clockwise)
{
	StopAnimation();

	start_scale_ = original_scale_;
//		target_scale_;	// no zoom animation
	animation_duration_ = ANIM_BUDGET;
	target_angle_ = clockwise ? 90.0 : -90.0;

	repaint_timer_.Start(REPAINT_TIMER_ID, REPAINT_TIMER_FRQ);
}


void ViewPane::Impl::StopAnimation()
{
	floating_scale_ = target_scale_;
	floating_position_ = target_position_;
	target_angle_ = floating_angle_ = 0.0;	// clear angle (including target) so proper bitmap can be used for final step

	repaint_timer_.Stop();
}


void ViewPane::Impl::AnimateVars()
{
	// current scale races destination scale; rate of change is important:
	// fixed increments by a certain percentage (multiplication by a factor) do not yield
	// satisfying results (they are 'linear', and feel artificial), what's needed is
	// faster change at first ('spring to action') with slowing down at the end (arrival);
	// to accomplish that sin function is used: linear attack at 0, and nice tapering at pi/2;

	// this is simple animation of zoom variable; zoom changes are exponential;
	// during animation I vary variable 't' from 0 to 1; this is our timeline
	// 't' is plugged into sin to obtain desired transition (as described above)

	double e= repaint_timer_.Elapsed();
	if (e <= animation_duration_)
	{
TRACE(L"ellapsed: %f\n", e);
		double t= e / animation_duration_;		// timeline (0..1)
		double transition_factor= sin(t * M_PI / 2.0);	// transition - sharp attack and slow down at the end

		if (start_scale_ != target_scale_)
		{
			// first calculate scale limits as exponents of 2
			double two= log(2.0);
			double a= log(start_scale_) / two;
			double b= log(target_scale_) / two;

			// animate our variables: scale
			floating_scale_ = pow(2.0, a + transition_factor * (b - a));

			// and image offset transition
			floating_position_.x = std::max(0, static_cast<int>((target_offset_.x - starting_offset_.x) * floating_scale_ / start_scale_ - target_offset_.x));
			floating_position_.y = std::max(0, static_cast<int>((target_offset_.y - starting_offset_.y) * floating_scale_ / start_scale_ - target_offset_.y));
		}
		else
			floating_scale_ = target_scale_;

		if (target_angle_)
			floating_angle_ = transition_factor * target_angle_;
		else
			floating_angle_ = 0.0;
	}
	else
	{
TRACE(L"ellapsed: %f - done\n", e);
		// animation is over; set animated variables to final values, and stop timer
		StopAnimation();
	}
/*
	// transition_factor grows from 0 to 1
	double transition_factor= sin(static_cast<double>(repaint_step_) / total_repaint_steps_ * M_PI / 2.0);

	// zoom transition
	double scale_factor= target_scale_ / start_scale_;
	floating_scale_ = start_scale_ * pow(scale_factor, transition_factor);
TRACE(L"  floating scale: %.3f\n", floating_scale_);

	// image offset transition

	floating_position_.x = max(0, static_cast<int>((target_offset_.x - starting_offset_.x) * floating_scale_ / start_scale_ - target_offset_.x));
	floating_position_.y = max(0, static_cast<int>((target_offset_.y - starting_offset_.y) * floating_scale_ / start_scale_ - target_offset_.y));

	repaint_step_++;
	repaint_steps_left_--;
	if (repaint_steps_left_ == 0)
		StopAnimation();
*/
}


void ViewPane::OnTimer(UINT_PTR event_id)
{
	if (impl_->repaint_timer_.Id() == event_id)
	{
		if (impl_->direct_render_.get() == 0)
			return;

		if (!impl_->direct_render_->IsBitmapReady())
			impl_->StopAnimation();

		Invalidate();
		return;
	}
	else if (cursor_hide_timer_.Id() == event_id)
	{
		if (scrolling_ || cursor_stay_visible_)
			return;
		CWnd* frame= GetParentFrame();
		HWND active= ::GetActiveWindow();
		CPoint pos;
		GetCursorPos(&pos);
		ScreenToClient(&pos);
		CRect rect= ClientRect();
		if (!rect.PtInRect(pos) ||	// mouse outside the window
			frame != 0 && frame->m_hWnd != active)	// our pane window is not active
		{
			mouse_cursor_counter_ = 0;
			return;
		}
		if (mouse_cursor_counter_ <= MOUSEHIDINGTIME)
			++mouse_cursor_counter_;
		if (mouse_cursor_counter_ == MOUSEHIDINGTIME)
			::SetCursor(NULL);      // hide mouse cursor
	}
	else
		CWnd::OnTimer(event_id);
}


void ViewPane::OnDestroy()
{
	cursor_hide_timer_.Stop();
	impl_->repaint_timer_.Stop();

	// kill decoders while there is still a window
	// (cause they post messages to the window)
	decoder_.free();
	decoder_next_.free();

	scroll_bar_.Delete();

	CWnd::OnDestroy();
}


void ViewPane::CursorStayVisible(bool cursor_stay_visible)
{
	cursor_stay_visible_ = cursor_stay_visible;
	mouse_cursor_counter_ = 0;
}


void ViewPane::SetGamma(const ICMTransform& trans)
{
	if (transform_ != trans)
	{
		transform_ = trans;
		Invalidate();
	}
}

void ViewPane::EnableGamma(bool enabled)
{
	gamma_correction_ = enabled;
	Invalidate();
}


/*
void ViewPane::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	ModifyStyle(0, WS_CLIPSIBLINGS);
}
*/

void ViewPane::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	if (type == SIZE_MINIMIZED)
		return;

	if (impl_->direct_render_.get())
		impl_->direct_render_->Resize(cx, cy);

	if (display_photo_description_)
		yellow_text_.Prepare(this);

	//if (transparent_bar_wnd_.m_hWnd)
	//{
	//	int width= 80;
	//	transparent_bar_wnd_.SetWindowPos(0, cx - width, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	//}

	// if photo is reduced in size image may have to be reloaded at higher resolution
//	if (GetReductionFactor() > 1)
	{
//		int zoom= zoom_ > 0 ? zoom_ : static_cast<int>(CalcZoom(false));
//		image_has_changed_ = ReloadCurrentPhoto(zoom, false);
//		Invalidate();
	}

	static bool in_update= false;

	if (scroll_bars_enabled_ && cx > 0 && cy > 0 && !in_update)
	{
		Block update(in_update);
		SetScrollSizes(GetCurBitmap());
	}
}


void ViewPane::EnablePhotoDesc(bool enable)
{
	display_photo_description_ = enable;
	Invalidate();
}


void ViewPane::ResetDescription(const std::wstring& desc)
{
	yellow_text_.Prepare(this, desc);
}


void ViewPane::OnPaint()
{
	try
	{
		CPaintDC dc(this); // device context for painting

		if (cursor_hide_timer_.IsStopped() && cursor_stay_visible_ == false)
			cursor_hide_timer_.Start(1113, 200);

		if (DecodingCurrentPhoto() && decoder_.get() && decoder_->Pending())	// decoding in progress?
			DrawPhoto(&dc, 0, decoder_->LinesReady(), true);	// display available part
		else
			DrawPhoto(&dc, 0, -1, true);	// display entire photo
	}
	catch (...)
	{}
}


void ViewPane::Reset()
{
	cur_photo_ = next_photo_ = 0;
	decoder_.free();
	decoder_next_.free();
	yellow_text_.Empty();

	Invalidate();
}


// zoom support
//
void ViewPane::ChangeZoom(const uint16* zooms, size_t count, bool zoom_in)
{
	int new_zoom= 100;
	int zoom= static_cast<int>(GetLogicalZoom() * 100.0 + 0.5);

	if (zoom_in)		// zoom in?
	{
		for (int i= 0; ; ++i)
		{
			if (i == count)		// the end?
			{
				new_zoom = zooms[i - 1];	// max zoom
				break;
			}
			if (zoom < zooms[i])
			{
				new_zoom = zooms[i];
				break;
			}
		}
	}
	else				// zoom out
	{
		for (int i= static_cast<int>(count - 1); ; --i)
		{
			if (i == -1)		// the end?
			{
				new_zoom = zooms[0];		// min zoom
				break;
			}
			if (zoom > zooms[i])
			{
				new_zoom = zooms[i];
				break;
			}
		}
	}

	SetLogicalZoom(new_zoom / 100.0, true);
}


void ViewPane::ZoomIn(const uint16* zoom, size_t count)
{
	ChangeZoom(zoom, count, true);
}

void ViewPane::ZoomOut(const uint16* zoom, size_t count)
{
	ChangeZoom(zoom, count, false);
}


void ViewPane::OnLButtonDblClk(UINT flags, CPoint pos)
{
	if (IsScrollingNeeded())
		OnLButtonUp(flags, pos);		// stop scrolling

	if (CWnd* parent= GetParent())
		parent->PostMessage(WM_USER);

	if (recipient_)
		recipient_->MouseDoubleClick(pos);
}


void ViewPane::SetDescriptionText(const std::wstring* text)
{
	show_description_if_no_photo_ = true;
	if (text)
		yellow_text_.Prepare(this, *text);
	else
		yellow_text_.Empty();
}


int ViewPane::GetReductionFactor() const
{
	if (DecodingCurrentPhoto())
		return decoder_->ReductionFactor();

	if (CacheImg* img= GetCurImg())
		return img->ReductionFactor();

	return 1;
}


bool ViewPane::ReloadCurrentPhoto(double zoom)
{
	if (cur_photo_ == 0)
		return false;

	bool reset_decoder= false;
	bool reload= false;
	DibPtr dib;
	int reduction= 0;
	CSize org_size(0, 0);

	if (CacheImg* img= GetCurImg())
	{
		reduction = img->ReductionFactor();
		org_size = img->OriginalSize();
		reload = img->ReductionFactor() > 1;
		dib = img->Dib();
	}
	else
	{
		dib = GetDecoderBitmap(decoder_.get());

		if (dib.get() != nullptr && decoder_->ReductionFactor() > 1)
		{
			reduction = decoder_->ReductionFactor();
			org_size = decoder_->GetOriginalSize();
			reset_decoder = reload = true;
			if (abs(decoder_->Rotated()) == 1)
				std::swap(org_size.cx, org_size.cy);
		}
	}

	if (!reload || dib == 0)
		return false;

	return ReloadPhotoIfNeeded(dib, reduction, org_size, reset_decoder, zoom);
}


bool ViewPane::ReloadPhotoIfNeeded(DibPtr dib, int reduction, CSize org_size, bool reset_decoder, double zoom)
{
	//bool reload= false;
//	bool reset_decoder= false;

	if (dib == nullptr)
		return true;
/*
	if (CacheImg* img= GetCurImg())
	{
		reduction = img->ReductionFactor();
		org_size = img->OriginalSize();
		reload = img->ReductionFactor() > 1;
		dib = img->Dib();
//		if (img->IsRotated())
//			swap(org_size.cx, org_size.cy);
	}
	else
	{
		dib = GetDecoderBitmap(decoder_.get());

		if (dib.get() != nullptr && decoder_->ReductionFactor() > 1)
		{
			reduction = decoder_->ReductionFactor();
			org_size = decoder_->GetOriginalSize();
			reset_decoder = reload = true;
			if (abs(decoder_->Rotated()) == 1)
				swap(org_size.cx, org_size.cy);
		}
	}

	if (!reload || dib == 0)
		return false;
*/
	if (reduction == 0)
		reduction = 1;

	// requested size
	CSize img_size= org_size;
	if (zoom == ZOOM_TO_FIT)	// zoom to fit?
		zoom = CalcPhysicalZoom(ZOOM_TO_FIT, false, dib.get()) / reduction;

	img_size.cx = static_cast<int>(img_size.cx * zoom);
	img_size.cy = static_cast<int>(img_size.cy * zoom);

	int bmp_width= dib->GetWidth();
	int bmp_height= dib->GetHeight();

	// if currently decoded bitmap is smaller than the one at requested zoom it has to be reloaded
	if (bmp_width < img_size.cx || bmp_height < img_size.cy)
	{
		int new_reduction= 0;

		if (img_size.cx > 0 && img_size.cy > 0 && org_size.cx > 0 && org_size.cy > 0)
		{
			double ratio= double(org_size.cx) / double(org_size.cy);
			if (double(img_size.cx) / double(img_size.cy) > ratio)
				img_size.cx = static_cast<long>(img_size.cy * ratio);
			else
				img_size.cy = static_cast<long>(img_size.cx / ratio);

			int times_x= org_size.cx / img_size.cx;
			int times_y= org_size.cy / img_size.cy;

			if (times_x >= 8 && times_y >= 8)
				new_reduction = 8;
			else if (times_x >= 4 && times_y >= 4)
				new_reduction = 4;
			else if (times_x >= 2 && times_y >= 2)
				new_reduction = 2;
			else
				new_reduction = 1;
		}

		if (new_reduction >= reduction)
			return false;	// no improvment needed

		// new image gets bigger
		if (impl_->image_change_ == NoChange && impl_->direct_render_.get())
		{
			//bool rescaled= false;
			//// reuse existing low res image, rescale it; high res will came later when decoder supplies it
			//if (reduction == 4 && new_reduction == 2 || reduction == 2 && new_reduction == 1)
			//	rescaled = direct_render_->RescaleBitmap(2);
			//else if (reduction == 4 && new_reduction == 1)
			//	rescaled = direct_render_->RescaleBitmap(4);

			//impl_->image_change_ = rescaled ? ReloadingLarger : LoadNew;

			// set flags to 'ReloadingLarger' to allow painting routine to reuse low res bitmap
			impl_->image_change_ = ReloadingLarger;
		}
		else
			impl_->image_change_ = LoadNew;	//TODO: overrides rotation info...

		// restart decoder job
		if (reset_decoder)
			decoder_->Restart(img_size);
		else
		{
//			if (cache_)
//				cache_->Remove(cur_photo_);	// remove from cache so drawing will use bmp from decoder again

			bool auto_rotate= false;//!cur_photo_->OrientationAltered();

			decoder_ = new DecoderJob(*cur_photo_, CSize(cur_photo_->GetWidth(), cur_photo_->GetHeight()),
				auto_rotate, logical_zoom_, img_size, this, cur_photo_->rotation_flag_);
		}
#if 0
		if (relative_change && new_reduction > 0 && zoom_ > 0)
		{
			while (new_reduction < reduction)
			{
				zoom_ /= 2;
				new_reduction *= 2;
			}
		}
#endif
		return true;	// true indicates reloading
	}

	return false;
}


bool ViewPane::ReloadWithoutReduction()
{
	if (GetReductionFactor() > 1)
	{
		ReloadCurrentPhoto(1.0);
		Invalidate();
		return true;	// reloading
	}

	return false;	// no reloading
}


void ViewPane::ResetColors()
{
	// description text
	SetBackgndColor(RGB(0,0,0));
	SetTextColor(RGB(255, 138, 22));
}


bool ViewPane::StillLoading() const
{
	if (decoder_.get() == 0)
		return false;

	return decoder_->Pending();
}


DibPtr ViewPane::GetDibPtr() const
{
	if (cur_photo_ == 0)
		return DibPtr();

	if (CacheImg* img= GetCurImg())
		return img->Dib();

	if (decoder_.get() != 0 && decoder_->GetBitmap() != 0 && !decoder_->Pending())
		return decoder_->GetBitmap();

	return DibPtr();
}

// where is currently displayed image; returned rect can be smaller or larger than view
CRect ViewPane::GetImageRect() const
{
	CRect rect= ImageRect();

	DibPtr bmp= GetCurBitmap();
	if (bmp == 0)
		return rect;

	//if (decoder_.get() == 0 || decoder_->Bitmap() == 0)
	//	return rect;

	//bool rotated= decoder_->Rotated() != 0;
	//Dib* bmp= decoder_->Bitmap();

	int bmp_width= bmp->GetWidth();
	int bmp_height= bmp->GetHeight();
	if (bmp_width == 0 || bmp_height == 0)
		return rect;

	double scrn_ratio= g_Settings.GetScreenAspectRatio();

	if (scrn_ratio >= 1.0)
		bmp_width = static_cast<int>(bmp_width * scrn_ratio);
	else
		bmp_height = static_cast<int>(bmp_height / scrn_ratio);

	int width= bmp_width;
	int height= bmp_height;

	if (IsZoomToFit())	// fit to screen size?
	{
		if (width > rect.Width())
			width = rect.Width();
		if (height > rect.Height())
			height = rect.Height();

		if (width != bmp_width || height != bmp_height)
		{
			double ratio_org= double(bmp_width) / double(bmp_height);

			if (double(width) / double(height) > ratio_org)
				width = static_cast<int>(height * ratio_org);
			else
				height = static_cast<int>(width / ratio_org);
		}
	}
	else
	{
		double zoom= CalcPhysicalZoom(logical_zoom_, true, bmp.get());
		width = static_cast<int>(width * zoom);
		height = static_cast<int>(height * zoom);
	}

	int x_range= width - rect.Width();
	int y_range= height - rect.Height();

	bool scroll = x_range > 0 || y_range > 0;
	CPoint img_pos= img_pos_;

	if (scroll)
	{
		CSize range_size = CSize(x_range > 0 ? x_range : 0, y_range > 0 ? y_range : 0);
		if (img_pos.x > range_size.cx)
			img_pos.x = range_size.cx;
		if (img_pos.y > range_size.cy)
			img_pos.y = range_size.cy;
	}
	else
		img_pos = CPoint(0, 0);

	// bitmap source rectangle ---------------------------------
//	CRect src_rect;
//	if (rotated)
//		src_rect.SetRect(lines_from, 0, lines_to, bmp->GetHeight());
//	else
//		src_rect.SetRect(0, lines_from, bmp->GetWidth(), lines_to);

	// bitmap destination rectangle ----------------------------
	CRect dest_rect= rect;

/*	if (rotated)
	{
		if (scrn_ratio >= 1.0)
		{
			lines_from = static_cast<int>(lines_from * scrn_ratio);
			lines_to = static_cast<int>(lines_to * scrn_ratio);
		}
	}
	else
	{
		if (scrn_ratio < 1.0)
		{
			lines_from = static_cast<int>(lines_from / scrn_ratio);
			lines_to = static_cast<int>(lines_to / scrn_ratio);
		}
	} */

	if (width < dest_rect.Width())
		dest_rect.left = (dest_rect.Width() - width) / 2;
	else
		dest_rect.left = 0;

	if (height < rect.Height())
		dest_rect.top = (dest_rect.Height() - height) / 2;
	else
		dest_rect.top = 0;

	CPoint left_top= dest_rect.TopLeft();

/*	if (partial_refresh)
	{
		if (rotated)
			width = int(lines_to * zoom);
		else
			height = int(lines_to * zoom);
	} */

	dest_rect.right = dest_rect.left + width;
	dest_rect.bottom = dest_rect.top + height;

/*	if (partial_refresh)
	{
		if (rotated)
			dest_rect.left = left_top.x + int(lines_from * zoom);
		else
			dest_rect.top = left_top.y + int(lines_from * zoom);
	} */

	dest_rect.OffsetRect(-img_pos);

	return dest_rect;
}


// grab bmp from decoder and store it in a cache
//
void ViewPane::CacheDecodedImage(AutoPtr<DecoderJob>& decoder)
{
	if (cache_)
		cache_->CacheDecodedImage(decoder.get(), decoder->GetDecodingStatus());

	// delete decoder
	decoder = 0;
}


LRESULT ViewPane::OnImgReloadingDone(WPARAM job_id, LPARAM)
{
	UINT_PTR main_job_id= GetDecoderUniqueId(decoder_.get());
	UINT_PTR next_job_id= GetDecoderUniqueId(decoder_next_.get());

	if (cache_)	// cache decoded image
	{
		if (main_job_id == job_id)
			CacheDecodedImage(decoder_);

		if (next_job_id == job_id)
			CacheDecodedImage(decoder_next_);
	}

	// decoding thread finished
	if (main_job_id == job_id && recipient_)
		recipient_->DecodingThreadFinished(this);

	return 0;
}


void ViewPane::DrawAlphaBlend(CDC& dc, DecoderJob* existing, DecoderJob* next)
{
	if (existing == 0 || next == 0)
		return;

	DrawAlphaBlend(dc, existing->GetBitmap().get(), existing->GetPhoto()->GetColorProfile(),
		next->GetBitmap().get(), next->GetPhoto()->GetColorProfile());
}


void ViewPane::DrawAlphaBlend(CDC& dc, Dib* existing, ColorProfilePtr icc_existing, Dib* next, ColorProfilePtr icc_next)
{
	if (existing == 0 || next == 0)
		return;

	try
	{
		// prepare original bmp copy
		Dib dibExisting;
		PrepareBmpCopy(*existing, icc_existing, dc, dibExisting);

		// prepare copy of the next photo
		Dib dibNext;
		PrepareBmpCopy(*next, icc_next, dc, dibNext);

		DrawAlphaBlendEx(dc, dibExisting, dibNext);
	}
	catch (...)
	{
	}
}


void ViewPane::PrepareBmpCopy(Dib& dibSrc, ColorProfilePtr icc, CDC& dc, Dib& dibDest)
{
	CRect rect;
	dc.GetWindow()->GetClientRect(rect);

	CDC mem_dc;
	mem_dc.CreateCompatibleDC(&dc);

	dibDest.Create(rect.Width(), rect.Height(), 24);

	mem_dc.SelectObject(dibDest.GetBmp());

	int dir= 0;//decoder->Rotated();
	DrawPhoto(&mem_dc, &dibSrc, dir, true, 0, -1, icc);
}


void ViewPane::PrepareBmpCopy(DecoderJob* decoder, CDC& dc, Dib& dib)
{
	CRect rect;
	dc.GetWindow()->GetClientRect(rect);

	CDC mem_dc;
	mem_dc.CreateCompatibleDC(&dc);

	dib.Create(rect.Width(), rect.Height(), 24);

	mem_dc.SelectObject(dib.GetBmp());

	int dir= decoder->Rotated();
	DibPtr decoder_dib= decoder->GetBitmap();
	DrawPhoto(&mem_dc, decoder_dib.get(), dir, true, 0, -1, decoder->GetPhoto()->GetColorProfile());
}


void ViewPane::DrawAlphaBlendEx(CDC& dc, Dib& dibExisting, Dib& dibNext)
{
	// working buffer
	CRect rect= ClientRect();

	Dib dibScreen;
	dibScreen.Create(rect.Width(), rect.Height(), 24);

	LARGE_INTEGER frq;
	::QueryPerformanceFrequency(&frq);
	if (frq.QuadPart == 0)	// need to have performance counter
		return;

#ifdef _DEBUG
//#define ALPHA_SPEED_TEST
#endif
	// calc transition steps and bitblt them to the screen
	int adjust= 2;
	int steps= alpha_blending_steps_;
//steps=10;frq.QuadPart=0;
	for (int i= 0; i < steps; ++i)
	{
		// note: alpha goes to 0, which means last step if fully opaque: it's intended behavior.
		// DrawPhoto() follows transition anyway, but it's noticably slower than quick bit blt below
		// which leads to nasty 'snap in' of the last frame in case of alpha remaining > 0
		float f= (steps - i - 1) / float(steps);
		// non-linear transition helps to reduce the effect of disappearing last image
		// by smoothing (smaller steps) alpha transition at the end of blending sequence
		f = pow(f, 1.35f);
		int alpha= static_cast<int>(256 * f);

		LARGE_INTEGER start;
		::QueryPerformanceCounter(&start);

#ifdef ALPHA_SPEED_TEST
LARGE_INTEGER tm[9];
::QueryPerformanceCounter(&tm[0]);
#endif

		AlphaTransition(dibExisting, dibNext, dibScreen, alpha);

#ifdef ALPHA_SPEED_TEST
::QueryPerformanceCounter(&tm[1]);
//TRACE(L"alpha: %d  steps: %d\n", alpha, steps);
#endif

		dibScreen.Draw(&dc, CPoint(0, 0));

		LARGE_INTEGER time;
		::QueryPerformanceCounter(&time);

		if (frq.QuadPart != 0)
		{
			// delay in ms
			DWORD delay= static_cast<DWORD>((time.QuadPart - start.QuadPart) * 1000 / frq.QuadPart);

			DWORD del50Hz= 20;	// 20 ms = 50 Hz
			if (delay < del50Hz)
			{
				::Sleep(del50Hz - delay);
			}
			else if (delay > del50Hz)
			{
				DWORD budget= 500;	// 0.5 sec

				if (delay > budget)
					break;

				// 'adjust' is here so the second adjustment is final for the loop to avoid fluctuations
				if (--adjust > 0)
					steps = budget / delay;
			}
		}

#ifdef ALPHA_SPEED_TEST
::QueryPerformanceCounter(&tm[2]);
LARGE_INTEGER tmr[9], frq;
::QueryPerformanceFrequency(&frq);
for (int ii= 0; ii < 2; ++ii)
{
	tmr[ii].QuadPart = tm[ii + 1].QuadPart - tm[ii].QuadPart;
	tmr[ii].QuadPart *= 1000000;
	tmr[ii].QuadPart /= frq.QuadPart;
}
int a[10]= { 0 };
a[0] = int(tmr[0].QuadPart);
a[1] = int(tmr[1].QuadPart);

TCHAR str[256];
wsprintf(str, _T("alpha: %d  draw: %d  transp: %d\n"), a[0], a[1], alpha);
::OutputDebugString(str);
//		::Sleep(1);
#endif

	}

	alpha_blending_steps_ = steps;
}


// those variables control alpha blend function path
static bool g_prefetch= true;	// TODO: store in registry?
static bool g_non_temporal_writing= true;
static int g_test_performance= 2;	// testing routine performance

void ViewPane::AlphaTransition(Dib& dibExisting, Dib& dibNext, Dib& dibResult, int alpha)
{
	if (dibExisting.GetSize() != dibNext.GetSize() || dibNext.GetSize() != dibResult.GetSize())
	{
		ASSERT(false);
		return;
	}

	size_t width= dibExisting.GetWidth();
	size_t height= dibExisting.GetHeight();
	size_t line_len= dibExisting.GetLineBytes();

#ifndef _WIN64
	if (IsMMXAvailable())
	{
		void MMX_AlphaBlendRGB(int alpha, BYTE* img1, BYTE* img2, BYTE* output, DWORD width, DWORD height, DWORD line_length, bool prefetch, bool non_temporal_writing);

		if (g_test_performance == 0)
		{
			// testing alpha blending performance to find faster routine (CPU dependent)
			LARGE_INTEGER start;
			::QueryPerformanceCounter(&start);

			MMX_AlphaBlendRGB(alpha, dibExisting.GetBuffer(), dibNext.GetBuffer(), dibResult.GetBuffer(), width, height, line_len, true, true);
			LARGE_INTEGER t1;
			::QueryPerformanceCounter(&t1);

			MMX_AlphaBlendRGB(alpha, dibExisting.GetBuffer(), dibNext.GetBuffer(), dibResult.GetBuffer(), width, height, line_len, false, false);
			LARGE_INTEGER t2;
			::QueryPerformanceCounter(&t2);

			t2.QuadPart -= t1.QuadPart;
			t1.QuadPart -= start.QuadPart;

			// select faster variant of alpha blend routine based on the test
			if (t2.QuadPart < t1.QuadPart)
				g_prefetch = g_non_temporal_writing = false;

			g_test_performance = -1;

			return;
		}
		if (g_test_performance > 0)
			--g_test_performance;

		MMX_AlphaBlendRGB(alpha, dibExisting.GetBuffer(), dibNext.GetBuffer(), dibResult.GetBuffer(), width, height, line_len, g_prefetch, g_non_temporal_writing);
	}
	else
#endif
	{
		// this simplistic loop is 7 times slower than MMX version

		for (int y= 0; y < height; ++y)
		{
			BYTE* img_a= dibExisting.LineBuffer(y);
			BYTE* img_b= dibNext.LineBuffer(y);
			BYTE* dest= dibResult.LineBuffer(y);

			for (size_t x= 0; x < width; ++x)
			{
				dest[0] = ((alpha * (img_a[0] - img_b[0])) >> 8) + img_b[0];
				dest[1] = ((alpha * (img_a[1] - img_b[1])) >> 8) + img_b[1];
				dest[2] = ((alpha * (img_a[2] - img_b[2])) >> 8) + img_b[2];

				dest += 3;
				img_a += 3;
				img_b += 3;
			}
		}
	}
}


void ViewPane::OnCustDrawScrollBar(UINT id, NMHDR* hdr, LRESULT* result)
{
	*result = scroll_bar_.HandleCustomDraw(hdr);
}


//-----------------------------------------------------------------------------


void ViewPane::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (sb_code == SB_THUMBPOSITION || sb_code == SB_THUMBTRACK)
	{
		SCROLLINFO si;
		memset(&si, 0, sizeof si);
		si.cbSize = sizeof si;
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &si);
		pos = si.nTrackPos;
	}
	OnScroll(MAKEWORD(sb_code, -1), pos);
}


void ViewPane::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (sb_code == SB_THUMBPOSITION || sb_code == SB_THUMBTRACK)
	{
		SCROLLINFO si;
		memset(&si, 0, sizeof si);
		si.cbSize = sizeof si;
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);
		pos = si.nTrackPos;
	}
	OnScroll(MAKEWORD(-1, sb_code), pos);
}


void ViewPane::VertScroll(UINT code)
{
	OnScroll(MAKEWORD(-1, code), 0);
}


void ViewPane::HorzScroll(UINT code)
{
	OnScroll(MAKEWORD(code, -1), 0);
}


void ViewPane::OnScroll(UINT scroll_code, UINT pos)
{
	CRect rect= ClientRect();

	CSize line_size(20, 20);

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
		x += rect.Width();
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
	case 98:
		y -= pos;
		break;
	case 99:
		y += pos;
		break;
	}

	bool do_scroll= true;
	BOOL result = OnScrollBy(CSize(x - xOrig, y - yOrig), do_scroll);
	if (result && do_scroll)
		UpdateWindow();
}


BOOL ViewPane::OnScrollBy(CSize scroll_size, BOOL do_scroll)
{
	CPoint pos_img= img_pos_;
	pos_img += scroll_size;

	bool scrolled= ScrollTo(pos_img);

	if (scrolled && recipient_)
		recipient_->ViewScrolled(this, img_pos_);

	return scrolled;

#if 0
	int xOrig, x;
	int yOrig, y;

	//This is bogus (comes from MFC); view pane allows scrolling without scrollbars too
/*
	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	DWORD dwStyle= GetStyle();
	if (!(dwStyle & WS_VSCROLL))
	{
		// vertical scroll bar not enabled
		scroll_size.cy = 0;
	}
	if (!(dwStyle & WS_HSCROLL))
	{
		// horizontal scroll bar not enabled
		scroll_size.cx = 0;
	}
*/
	// adjust current x position
	xOrig = x = GetScrollPos(SB_HORZ);
	int xMax = GetScrollLimit(SB_HORZ);
	x += scroll_size.cx;
	if (x < 0)
		x = 0;
	else if (x > xMax)
		x = xMax;

	// adjust current y position
	yOrig = y = GetScrollPos(SB_VERT);
	int yMax = GetScrollLimit(SB_VERT);
	y += scroll_size.cy;
	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig)
		return FALSE;

	if (do_scroll)
	{
		img_pos_ = CPoint(x, y);

		ScrollWindow(xOrig - x, yOrig - y);

		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);

		if (y != yOrig)
		{
			SetScrollPos(SB_VERT, y);
//			RemoveToolTips();
		}

		if (recipient_)
			recipient_->ViewScrolled(this, img_pos_);
	}
	return TRUE;
#endif
}


//CPoint ViewPane::GetScrollOffset() const
//{
//	return CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
//}


//BOOL ViewPane::OnMouseWheel(UINT flags, short delta, CPoint pt)
//{
//	if (delta)
//	{
//		int dy= item_size_.cy;
//		if (mode_ == DETAILS)
//			dy *= 3;
//		else if (mode_ == PREVIEWS)
//			dy /= 2;
//
//		OnScroll(MAKEWORD(-1, delta > 0 ? 98 : 99), dy);
//
//		// verify cursor (group label might have been scrolled from under the cursor)
//		SetCursor();
//	}
//
//	return true;
//}


void ViewPane::UseScrollBars(bool enable)
{
	if (scroll_bars_enabled_ == enable)
		return;

	scroll_bars_enabled_ = enable;

	if (scroll_bars_enabled_)
		SetScrollSizes(GetCurBitmap());
	else
		SetScrollInfo(CSize(0, 0), CSize(1, 1), CPoint(0, 0));
}


void ViewPane::ResetScrollBars()
{
	if (scroll_bars_enabled_)
		SetScrollSizes(GetCurBitmap());
}


void ViewPane::OnMButtonDown(UINT flags, CPoint point)
{
	CWnd::OnMButtonDown(flags, point);

	if (recipient_)
		recipient_->MiddleButtonDown(point);
}


CRect ViewPane::ClientRect() const
{
	CRect client(0,0,0,0);
	GetClientRect(client);
	return client;
}

// desired image area
CRect ViewPane::ImageRect() const
{
	CRect rect= ClientRect();

	if (!padding_.IsRectNull())
	{
		rect.DeflateRect(padding_);

		if (rect.left > rect.right)
			rect.left = rect.right = 0;
		if (rect.top > rect.bottom)
			rect.top = rect.bottom = 0;
	}

	return rect;
}


void ViewPane::EnableZoomToFill(bool enable, int percentage_img_to_hide)
{
	impl_->eliminate_bars_ = enable;
	ASSERT(percentage_img_to_hide >= 0 && percentage_img_to_hide < 100);
	impl_->fraction_of_image_to_hide_ = percentage_img_to_hide / 100.0;
}


void ViewPane::SetBackgndColor(COLORREF rgb_back)
{
	rgb_background_ = rgb_back;
	yellow_text_.SetBackgndColor(rgb_back);
}


bool ViewPane::IsScrollingNeeded() const
{
	if (!scroll_)
		return false;

	if (logical_zoom_ == ZOOM_TO_FIT)
		return false;

	return true;
}
