/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DragControl.h: interface for the DragControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAGCONTROL_H__C21205F5_698E_11D5_8E8F_00B0D078DE24__INCLUDED_)
#define AFX_DRAGCONTROL_H__C21205F5_698E_11D5_8E8F_00B0D078DE24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class DragControl
{
public:
	DragControl();
	/*virtual ~DragControl();*/

	void InitLoop();

	void CancelLoop();

	void DrawFocusRect(const CRect& rect, bool remove_rect= false);

	void StartDrag(CWnd* wnd);

	void MoveTo(const CRect& new_pos_rect);

	bool Track();

	void EndDrag();

	virtual void MouseMoved()= 0;

	void ScreenDraw(const CRect& rect, COLORREF rgb_fill);

private:
// Attributes
//	CPoint last_;            // last mouse position during drag
	CRect last1_rect_;
	CRect last2_rect_;
	CRect last3_rect_;
	CSize last_size_;
	BOOL dither_last_;

	// Rectangles used during dragging or resizing
//	CRect drag_horz_rect_;
//	CRect drag_vert_rect_;
//	CRect frame_drag_horz_rect_;
//	CRect frame_drag_vert_rect_;

	BOOL flip_;               // if shift key is down
	BOOL force_frame_;         // if ctrl key is down

	CDC* dc_;                 // where to draw during drag
	BOOL dragging_;
	int hit_test_;

	UINT MRU_dock_id_;
	CRect MRU_dock_pos_rect_;

	DWORD MRU_float_style_;
	CPoint MRU_float_pos_;

	CWnd* wnd_;
};

#endif // !defined(AFX_DRAGCONTROL_H__C21205F5_698E_11D5_8E8F_00B0D078DE24__INCLUDED_)
