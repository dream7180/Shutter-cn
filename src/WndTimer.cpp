/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "WndTimer.h"

//#define USE_PERFORMANCE_COUNTER 1

static LARGE_INTEGER g_frq;

WndTimer::WndTimer()
{
	hwnd_ = 0;
	timer_id_ = 0;
	::QueryPerformanceFrequency(&g_frq);
}

WndTimer::WndTimer(HWND hwnd)
{
	hwnd_ = hwnd;
	timer_id_ = 0;
}


WndTimer::~WndTimer()
{
	Stop();
}


bool WndTimer::Start(int event_id, UINT elapse)
{
	ASSERT(hwnd_ != 0);
	if (hwnd_ == 0)
		return false;
	Stop();
	timer_id_ = SetTimer(hwnd_, event_id, elapse, 0);
#if USE_PERFORMANCE_COUNTER
	::QueryPerformanceCounter(&start_);
#else
	start_.QuadPart = ::GetTickCount();
#endif
	return timer_id_ != 0;
}


void WndTimer::Stop()
{
	if (hwnd_ != nullptr && timer_id_ != 0)
	{
		::KillTimer(hwnd_, timer_id_);
		timer_id_ = 0;
	}
}


UINT_PTR WndTimer::Id() const
{
	return timer_id_;
}


bool WndTimer::IsRunning() const
{
	return timer_id_ != 0;
}


bool WndTimer::IsStopped() const
{
	return timer_id_ == 0;
}


void WndTimer::ResetWnd(HWND hwnd)
{
	Stop();
	hwnd_ = hwnd;
}


double WndTimer::Elapsed() const
{
#if USE_PERFORMANCE_COUNTER
	LARGE_INTEGER t;
	if (!::QueryPerformanceCounter(&t))
		return 0.0;

	double delta= static_cast<double>(t.QuadPart - start_.QuadPart);
	return delta * 1000.0 / g_frq.QuadPart;
#else
	DWORD t= ::GetTickCount();
	double delta= static_cast<double>(t - start_.QuadPart);
	return delta;
#endif;
}
