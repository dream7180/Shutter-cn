/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2002 Michael Kowalski

____________________________________________________________________________*/

// DlgAutoResize.h: interface for the DlgAutoResize class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGAUTORESIZE_H__34FFAD6F_D93B_42BD_BF95_A754BE42CB19__INCLUDED_)
#define AFX_DLGAUTORESIZE_H__34FFAD6F_D93B_42BD_BF95_A754BE42CB19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DlgAutoResize
{
public:
	DlgAutoResize();
	virtual ~DlgAutoResize();

	// prepare dialog controls for resizing
	void BuildMap(CWnd* dialog);

	enum ResizeFlag { NONE= 0, MOVE_H= 1, MOVE_V, MOVE, MOVE_H_RESIZE_V, MOVE_V_RESIZE_H, RESIZE_H, RESIZE_V, RESIZE,
		MOVE_H_RESIZE, MOVE_V_RESIZE, MOVE_H_RESIZE_H };

	enum HalfResizeFlag { HALF_MOVE_H= 1, HALF_MOVE_V= 2, HALF_RESIZE_V= 4, HALF_RESIZE_H= 8,
		SHIFT= 0x30, SHIFT_LEFT= 0x10, SHIFT_RIGHT= 0x20, SHIFT_RESIZES= 0xc0 };

	// specify how controls should be moved/resized
	void SetWndResizing(int id, ResizeFlag flag);
	void SetWndResizing(int id, ResizeFlag flag, UINT half_flags);

	// resize controls (call after receiving WM_SIZE)
	void Resize();
	void Resize(CRect rect);

	// if controls are being dynamically created/destroyed this update is necessary
	void UpdateControl(int id, HWND new_ctrl);
	void AddControl(int id, HWND ctrl);

	// delete all
	void Clear();

	// extra offset for all items the either move or are resizable
	void SetOffset(CSize shift);

private:
	static BOOL CALLBACK EnumChildProcStatic(HWND wnd, LPARAM lParam);
	BOOL EnumChildProc(HWND wnd);

	struct Ctrl
	{
		Ctrl(HWND wnd) : wnd(wnd)
		{
			id = ::GetWindowLong(wnd, GWL_ID);
			WINDOWPLACEMENT wp;
			wp.length = sizeof wp;
			::GetWindowPlacement(wnd, &wp);
			rect = wp.rcNormalPosition;
			resize_flags = 0;
			half_flags = 0;
		}

		int id;
		int resize_flags;
		UINT half_flags;
		HWND wnd;
		CRect rect;
	};

	std::vector<Ctrl> controls_;
	CRect dlg_rect_;
	CWnd* dialog_;
	CSize shift_;
};

#endif // !defined(AFX_DLGAUTORESIZE_H__34FFAD6F_D93B_42BD_BF95_A754BE42CB19__INCLUDED_)
