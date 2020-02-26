/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class WndTimer
{
public:
	WndTimer();
	WndTimer(HWND hwnd);

	~WndTimer();

	bool Start(int event_id, UINT elapse);

	void Stop();

	bool IsRunning() const;
	bool IsStopped() const;

	UINT_PTR Id() const;

	void ResetWnd(HWND hwnd);

	// amount of time in ms that passed since timer was started
	double Elapsed() const;

private:
	UINT_PTR timer_id_;
	HWND hwnd_;
	LARGE_INTEGER start_;
};
