/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2002 Michael Kowalski

____________________________________________________________________________*/

// DlgAutoResize.cpp: implementation of the DlgAutoResize class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgAutoResize.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DlgAutoResize::DlgAutoResize()
{
	dialog_ = 0;
	shift_ = 0;
}

DlgAutoResize::~DlgAutoResize()
{
}


void DlgAutoResize::SetOffset(CSize shift)
{
	shift_ = shift;
}


BOOL DlgAutoResize::EnumChildProc(HWND wnd)
{
	if (wnd && ::GetParent(wnd) == *dialog_)
		controls_.push_back(Ctrl(wnd));
	return true;
}


BOOL CALLBACK DlgAutoResize::EnumChildProcStatic(HWND wnd, LPARAM lParam)
{
	return reinterpret_cast<DlgAutoResize*>(lParam)->EnumChildProc(wnd);
}


void DlgAutoResize::BuildMap(CWnd* dialog)
{
	if (dialog->GetSafeHwnd() == 0)
		return;

	dialog_ = dialog;

	controls_.clear();
	controls_.reserve(32);

	::EnumChildWindows(dialog->m_hWnd, EnumChildProcStatic, reinterpret_cast<LPARAM>(this));

	dialog->GetClientRect(dlg_rect_);
}


void DlgAutoResize::SetWndResizing(int id, ResizeFlag flags)
{
	SetWndResizing(id, flags, 0);
}


void DlgAutoResize::SetWndResizing(int id, ResizeFlag flags, UINT half_flags)
{
	size_t count= controls_.size();

	for (size_t i= 0; i < count; ++i)
	{
		if (controls_[i].id == id)
		{
			controls_[i].resize_flags = flags;
			controls_[i].half_flags = half_flags;
			return;
		}
	}

	ASSERT(false);	// wrong window Id
}


void DlgAutoResize::Resize()
{
	if (dialog_ == 0)
		return;

	CRect rect;
	dialog_->GetClientRect(rect);

	Resize(rect);
}


void DlgAutoResize::Resize(CRect rect)
{
	if (dialog_ == 0)
		return;

	size_t count= controls_.size();
	//bool shift= shift_.cx != 0 || shift_.cy != 0;

	int resize_count= 0;
	{
		// calc wnd to resize
		for (size_t i= 0; i < count; ++i)
			if (controls_[i].resize_flags != 0 || controls_[i].half_flags != 0)
				++resize_count;
	}

//	CRect rect;
//	dialog_->GetClientRect(rect);

	CSize delta_size= rect.Size() - dlg_rect_.Size();

	HDWP wnd_pos_info= ::BeginDeferWindowPos(resize_count);

	for (size_t i= 0; i < count; ++i)
	{
		if ((controls_[i].resize_flags == 0 && controls_[i].half_flags == 0) || controls_[i].wnd == 0)
			continue;

		CRect rect= controls_[i].rect;
		UINT flags= SWP_NOZORDER | SWP_NOACTIVATE;
		CSize move_size= delta_size;
		CSize resize_size= delta_size;
		if (UINT half_flags= controls_[i].half_flags)
		{
			if (half_flags & HALF_MOVE_H)
				move_size.cx /= 2;
			if (half_flags & HALF_MOVE_V)
				move_size.cy /= 2;

			if (half_flags & HALF_RESIZE_H)
				resize_size.cx /= 2;
			if (half_flags & HALF_RESIZE_V)
				resize_size.cy /= 2;
		}

		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof wp);
		wp.length = sizeof wp;
		if (::GetWindowPlacement(controls_[i].wnd, &wp) == 0)
		{
//			DWORD d= ::GetLastError();
			ASSERT(false);
			continue;
		}

		CSize shift(0, 0);
		if (controls_[i].half_flags & (SHIFT | SHIFT_RESIZES))
			shift = shift_;

		switch (controls_[i].resize_flags)
		{
		case MOVE_H:
			rect.OffsetRect(shift.cx + move_size.cx, shift.cy);
			flags |= SWP_NOSIZE;
			break;
		case MOVE_V:
			rect.OffsetRect(shift.cx, shift.cy + move_size.cy);
			flags |= SWP_NOSIZE;
			break;
		case MOVE:
			rect.OffsetRect(move_size + shift);
			flags |= SWP_NOSIZE;
			break;

		case MOVE_H_RESIZE_V:
			rect.OffsetRect(move_size.cx, 0);
			rect.bottom += resize_size.cy;
			if (controls_[i].half_flags & SHIFT_LEFT)
				rect.left += shift.cx;
			if (controls_[i].half_flags & SHIFT_RIGHT)
				rect.right += shift.cx;
			break;
		case MOVE_V_RESIZE_H:
			rect.OffsetRect(0, move_size.cy);
			rect.right += resize_size.cx;
			break;

		case MOVE_H_RESIZE_H:
			rect.OffsetRect(move_size.cx, 0);
			rect.right += resize_size.cx;
			break;
		case MOVE_H_RESIZE:
			rect.OffsetRect(move_size.cx, 0);
			rect.right += resize_size.cx;
			rect.bottom += resize_size.cy;
			break;
		case MOVE_V_RESIZE:
			rect.OffsetRect(0, move_size.cy);
			rect.right += resize_size.cx;
			rect.bottom += resize_size.cy;
			break;

		case RESIZE_H:
			rect.right += resize_size.cx;
			if (controls_[i].half_flags & SHIFT)
			{
				rect.left += shift.cx;
				rect.top += shift.cy;
				rect.bottom += shift.cy;
			}
			else if (controls_[i].half_flags & SHIFT_RESIZES)
			{
				rect.bottom += shift.cy;
			}
			else
			{
				rect.bottom = wp.rcNormalPosition.bottom;
				flags |= SWP_NOMOVE;
			}
			break;

		case RESIZE_V:
			rect.bottom += resize_size.cy;
			if (controls_[i].half_flags & SHIFT)
			{
				rect.left += shift.cx;
				rect.right += shift.cx;
				rect.top += shift.cy;
			}
			else if (controls_[i].half_flags & SHIFT_RESIZES)
			{
				rect.right += shift.cx;
			}
			else
			{
				rect.right = wp.rcNormalPosition.right;
				flags |= SWP_NOMOVE;
			}
			break;

		case RESIZE:
			rect.BottomRight() += resize_size;
			if (controls_[i].half_flags & SHIFT_LEFT)
				rect.left += shift.cx;
			if (controls_[i].half_flags & SHIFT_RIGHT)
				rect.right += shift.cx;
//			rect.TopLeft() += shift;
			rect.top += shift.cy;
			if (shift == CSize(0, 0))
				flags |= SWP_NOMOVE;
			break;

		case NONE:
			if (shift != CSize(0, 0))
			{
				rect.OffsetRect(shift);
				flags &= ~SWP_NOMOVE;
			}
			break;
		}

		if (flags & SWP_NOMOVE)
			rect.TopLeft() = CPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top);

		if (rect != wp.rcNormalPosition)		// placement changed?
		{
			if (wnd_pos_info)
				wnd_pos_info = ::DeferWindowPos(wnd_pos_info, controls_[i].wnd, 0, rect.left, rect.top, rect.Width(), rect.Height(), flags);
			else
				::SetWindowPos(controls_[i].wnd, 0, rect.left, rect.top, rect.Width(), rect.Height(), flags);

			::InvalidateRect(controls_[i].wnd, 0, true);
		}
	}

	if (wnd_pos_info)
		::EndDeferWindowPos(wnd_pos_info);
}


// if controls are being dynamically created/destroyed this update is necessary
void DlgAutoResize::UpdateControl(int id, HWND new_ctrl)
{
	const size_t count= controls_.size();

	for (size_t i= 0; i < count; ++i)
		if (controls_[i].id == id)
		{
			controls_[i].wnd = new_ctrl;
			return;
		}

	ASSERT(false);	// wrong window Id
}


void DlgAutoResize::AddControl(int id, HWND ctrl)
{
	const size_t count= controls_.size();

	for (size_t i= 0; i < count; ++i)
		if (controls_[i].id == id)
		{
			ASSERT(false);	// wrong window Id
			return;
		}

	controls_.push_back(Ctrl(ctrl));
	controls_.back().id = id;
}


void DlgAutoResize::Clear()
{
	controls_.clear();
}
