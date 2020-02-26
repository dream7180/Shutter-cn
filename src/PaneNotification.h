/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _Pane_Notification_h_
#define _Pane_Notification_h_
#include "SnapFrame/SnapFrame.h"
class PaneWnd;


template <class MemPtr, class Arg, class Arg2> struct PaneNotification
{
	PaneNotification(SnapFrame& frame) : frame_(frame)
	{}

	void SendParam(SnapView* sender, MemPtr memFn, Arg& arg)
	{
		// count of snap views
		UINT count= frame_.GetSnapViewCount();

		// send initial update
		for (UINT i= 0; i < count; ++i)
		{
			if (SnapView* view= frame_.GetSnapView(i))
				if (sender != view)
					if (PaneWnd* pane= dynamic_cast<PaneWnd*>(view->GetChildView()))
						(pane->*memFn)(arg);
		}
	}

	void SendParam2(SnapView* sender, MemPtr memFn, Arg& arg, Arg2& arg2)
	{
		// count of snap views
		UINT count= frame_.GetSnapViewCount();

		// send initial update
		for (UINT i= 0; i < count; ++i)
		{
			if (SnapView* view= frame_.GetSnapView(i))
				if (sender != view)
					if (PaneWnd* pane= dynamic_cast<PaneWnd*>(view->GetChildView()))
						(pane->*memFn)(arg, arg2);
		}
	}

	void Send(SnapView* sender, MemPtr memFn)
	{
		// count of snap views
		UINT count= frame_.GetSnapViewCount();

		// send initial update
		for (UINT i= 0; i < count; ++i)
		{
			if (SnapView* view= frame_.GetSnapView(i))
				if (sender != view)
					if (PaneWnd* pane= dynamic_cast<PaneWnd*>(view->GetChildView()))
						(pane->*memFn)();
		}
	}

private:
	SnapFrame& frame_;
};

///////////////////////////////////////////////////////////////////////////////

template <class MemPtr, class Arg, class Arg2> void SendPaneNotification(SnapFrame& frame, MemPtr memFn, Arg& arg, Arg2& arg2)
{
	PaneNotification<MemPtr, Arg, Arg2> notif(frame);
	notif.SendParam2(0, memFn, arg, arg2);
}


template <class MemPtr, class Arg> void SendPaneNotification(SnapFrame& frame, MemPtr memFn, Arg& arg)
{
	PaneNotification<MemPtr, Arg, /*void*/int> notif(frame);
	notif.SendParam(0, memFn, arg);
}


template <class MemPtr> void SendPaneNotification(SnapFrame& frame, MemPtr memFn)
{
	PaneNotification<MemPtr, /*void*/int, /*void*/int> notif(frame);
	notif.Send(0, memFn);
}

///////////////////////////////////////////////////////////////////////////////

template <class MemPtr, class Arg, class Arg2> void SendPaneNotification(SnapView* view, MemPtr memFn, Arg& arg, Arg2& arg2)
{
	if (view)
		if (SnapFrame* frame= view->GetFrame())
		{
			PaneNotification<MemPtr, Arg, Arg2> notif(*frame);
			notif.SendParam2(view, memFn, arg, arg2);
		}
}


template <class MemPtr, class Arg> void SendPaneNotification(SnapView* view, MemPtr memFn, Arg& arg)
{
	if (view)
		if (SnapFrame* frame= view->GetFrame())
		{
			PaneNotification<MemPtr, Arg, /*void*/int> notif(*frame);
			notif.SendParam(view, memFn, arg);
		}
}


template <class MemPtr> void SendPaneNotification(SnapView* view, MemPtr memFn)
{
	if (view)
		if (SnapFrame* frame= view->GetFrame())
		{
			PaneNotification<MemPtr, /*void*/int, /*void*/int> notif(*frame);
			notif.Send(view, memFn);
		}
}

#endif
