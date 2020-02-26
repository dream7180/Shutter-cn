/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Dib.h"


class CoolScrollBar
{
public:
	CoolScrollBar();
	~CoolScrollBar();

	// initialize cool scrollbar for a given window
	bool Create(HWND wnd, bool enable);

	// uninitialize cool scrollbars during WM_DESTROY
	void Delete();

	void Enable(bool enable);

	// simple wrappers for Cool_* functions

	bool SetMinThumbSize(UINT bar, UINT size);
	bool IsThumbTracking() const;
	bool IsCoolScrollEnabled() const;

	bool EnableScrollBar(int S_bflags, UINT arrows);
	bool GetScrollInfo(int fnBar, const SCROLLINFO& si) const;
	int	 GetScrollPos(int nBar) const;
	bool GetScrollRange(int nBar, int& minPos, int& maxPos) const;

	int	 SetScrollInfo(int fnBar, const SCROLLINFO& si, bool redraw);
	int  SetScrollPos(int nBar, int pos, bool redraw= true);
	int  SetScrollRange(int nBar, int min_pos, int max_pos, bool redraw);
	bool ShowScrollBar(int bar, bool show);

	bool SetColors(COLORREF background, COLORREF foreground, COLORREF hottracked);

	int GetScrollLimit(int bar);

	// Scrollbar dimension functions
	bool SetSize(int bar, int length, int width);

	// cy: height of horz sb, cx: width of vert one
	CSize GetSize() const;

	// Set the visual nature of a scrollbar (flat, normal etc)
	bool SetStyle(int bar, UINT style);
	bool SetThumbAlways(int bar, bool thumb_always);

	bool SetHotTrack(int bar, bool enable);

	enum { COOLSB_CUSTOMDRAW= (0-0xfffU), COOLSB_FLAT = 1 };

	static UINT CustomDrawCode();

	LRESULT HandleCustomDraw(NMHDR* nm_hdr);

	void LoadImages(int horz_bmp_id, int vert_bmp_id, int grip_bmp_id, const std::vector<int>& part_sizes, double gamma);

	void SetImages(int horz_bmp_id, int vert_bmp_id, int grip_bmp_id, double gamma);

private:
	HWND wnd_;
	bool custom_draw_;
	Dib horz_;
	Dib vert_;
	Dib grip_;
	std::vector<int> parts_;
	int size_;

	enum { LEFT= 0, WELL, RIGHT, THUMB_L, THUMB_M, THUMB_R, COUNT };
	enum { NORMAL= 0, HOT, PRESSED };
};
