/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Frame.h: interface for the CFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRAME_H__4AED3E91_DEA9_49CB_95A0_2C08CAAEE70C__INCLUDED_)
#define AFX_FRAME_H__4AED3E91_DEA9_49CB_95A0_2C08CAAEE70C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCompositionView;
class CPrnUserParams;
class CFramePrepareDC;


class CFrame
{
public:
	CFrame();
	CFrame(const CRect& location_rect, int prec);
	virtual ~CFrame();

// Attributes

	// no of object handles
	virtual int GetHandleCount() const;

	// handle coordinates
	CPoint GetHandle(int handle) const;

	// handle location
	CRect GetHandleRect(int handle_id, CCompositionView* view) const;

	// return mouse cursor for a given handle
	HCURSOR GetHandleCursor(int handle) const;

	// check if frame intersects with rect
	bool Intersects(const CRect& rect) const	{ return !(rect & location_rect_).IsRectEmpty(); }

	const CRect& GetLocation() const			{ return location_rect_; }
	CRect& GetLocation()						{ return location_rect_; }

	// get frame bounding rectangle
	CRect GetBoundingRect() const;

	//
	bool IsLocked() const						{ return locked_; }


// Operations
	virtual int HitTest(CPoint pos, CCompositionView* view, bool selected);

	// move frame
	void MoveRel(const CSize& delta, CCompositionView* view);
	void MoveTo(const CRect& position, CCompositionView* view);
	void MoveHandleTo(int handle, CPoint point, CCompositionView* view);

	// drawing
	virtual void Draw(CDC* dc, const CPrnUserParams& Params, CDC* dc_real= NULL)= 0;

	// draw frame handles
	enum TrackerState { NORMAL, SELECTED, ACTIVE };
	virtual void DrawTracker(CDC* dc, TrackerState state);

	// get handle size
	static CSize GetHandleSize()  { return CSize(tracker_size_.cx * 2, tracker_size_.cy * 2); }

private:
	CRect location_rect_;
	bool locked_;

	static CSize tracker_size_;	// handle size (half the size)
};

#endif // !defined(AFX_FRAME_H__4AED3E91_DEA9_49CB_95A0_2C08CAAEE70C__INCLUDED_)
