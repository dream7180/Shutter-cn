/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_VIEWPANE_H__05E2DAAF_382B_42BA_95C8_A3D18650F385__INCLUDED_)
#define AFX_VIEWPANE_H__05E2DAAF_382B_42BA_95C8_A3D18650F385__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewPane.h : header file
//
#include "Dib.h"
#include "DecoderJob.h"
#include "PhotoInfo.h"
#include "YellowText.h"
#include "TransparentBar.h"
#include "PhotoCache.h"
#include "CoolScrollBar.h"
#include "WndTimer.h"
#include <boost/scoped_ptr.hpp>
class ViewerWnd;
class ViewPane;


class ViewPaneNotifications
{
public:
	virtual void DecodingStarted(ViewPane* view) = 0;
	virtual void DecodingFinished(ViewPane* view) = 0;
	virtual void DecodingThreadFinished(ViewPane* view) = 0;
	virtual void ViewClicked(ViewPane* view) {}
	virtual void ViewScrolled(ViewPane* view, CPoint pos_img) {}
	virtual std::wstring PreparePhotoDescriptionText(ViewPane* view, const PhotoInfo& photo) { return photo.PhotoDescription(); }
	virtual void MiddleButtonDown(CPoint pos) {}
	virtual void MouseDoubleClick(CPoint pos) {}
	virtual void AnimationStarts() {}
};

/////////////////////////////////////////////////////////////////////////////
// ViewPane window

class ViewPane : public CWnd
{
// Construction
public:
	ViewPane(/*bool coolSB= false*/);

// Attributes
public:
	bool IsPhotoDescDisplayed() const			{ return display_photo_description_; }
	bool IsGammaEnabled() const					{ return gamma_correction_; }
	bool IsZoomToFit() const					{ return logical_zoom_ == 0.0; }
	bool IsUsingScrollBars() const				{ return scroll_bars_enabled_; }
	bool IsScrollingNeeded() const;

	//float GetZoom() const;
	int GetReductionFactor() const;

	void SetBackgndColor(COLORREF rgb_back);

	void SetTextColor(COLORREF rgb_text)		{ yellow_text_.SetTextColor(rgb_text); }

	void SetPhotoCache(PhotoCache* cache)		{ cache_ = cache; }

	void ResetColors();

	COLORREF GetBackgndColor() const			{ return rgb_background_; }

	void ResetDescriptionFont()					{ yellow_text_.ResetFont(); }

	void SetDescriptionFont(const LOGFONT& lf)	{ yellow_text_.SetFont(lf); }

	void SetDescriptionText(const std::wstring* text);

	bool StillLoading() const;

	void SetHost(ViewPaneNotifications* recipient)	{ recipient_ = recipient; }

	// potentially dangerous; this pointer may become invalid
	DibPtr GetDibPtr() const;

	// this is an area occupied by current image
	CRect GetImageRect() const;

	// turn on/off scrollbars
	void UseScrollBars(bool enable);

	// reset scrollbar ranges
	void ResetScrollBars();

	// get current photo
	ConstPhotoInfoPtr GetCurrentPhoto() const	{ return cur_photo_; }

// Operations

	// create display pane window
	bool Create(CWnd* parent);

	// LoadPhoto flags
	enum { AUTO_ROTATE= 1, FORCE_RELOADING= 2, ALPHA_BLENDING= 0x10 };

	void LoadPhoto(const PhotoInfo& inf, ConstPhotoInfoPtr next, UINT flags); // bool auto_rotate, bool force_reloading);

	// display provided bitmap
	void DisplayBitmap(const Dib& bmp);

	// set magnification to given value
	//void Zoom(int zoom, bool relative_change= true);
	// zoom in (out) to the next available magnification (from the given list)
	void ZoomIn(const uint16* zoom, size_t count);
	void ZoomOut(const uint16* zoom, size_t count);

	void CursorStayVisible(bool cursor_stay_visible);

	void Rotate(bool clockwise);
	void SetGamma(const ICMTransform& trans);
	void EnableGamma(bool enable);
	void EnablePhotoDesc(bool enable);

	void ResetDescription(const std::wstring& desc);

	void Reset();

	bool ReloadWithoutReduction();

	// set logical zoom
	void SetLogicalZoom(double zoom, bool zoom_to_fit_can_magnify);
	void SetLogicalZoomAndOffset(double zoom, bool zoom_to_fit_can_magnify, CPoint offset);
	double GetLogicalZoom() const;

	bool ScrollTo(CPoint pos);
	void VertScroll(UINT code);
	void HorzScroll(UINT code);

	// turns on/off zoom to fill mode, there 'best fit' can enlarge image to get rid of bars
	void EnableZoomToFill(bool enable, int percentage_img_to_hide);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewPane)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ViewPane();

	// Generated message map functions
protected:
	//{{AFX_MSG(ViewPane)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	afx_msg void OnTimer(UINT_PTR event_id);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnPartialLoad(WPARAM job_id, LPARAM lines_from_to);
	afx_msg LRESULT OnOrientationChanged(WPARAM job_id, LPARAM orientation);
	afx_msg LRESULT OnImgReloadingDone(WPARAM job_id, LPARAM);

	DECLARE_MESSAGE_MAP()

private:
	static CString wnd_class_;
	COLORREF rgb_background_;
	double logical_zoom_;
	bool zoom_to_fit_may_magnify_;
	bool scroll_;						// if true image is bigger than evailable client rect
	bool scrolling_;
	bool got_capture_;
	CSize range_size_;
	CPoint start_;
	CPoint img_pos_;
	CPoint start_img_pos_;
	unsigned int mouse_cursor_counter_;	// hide mouse cursor if it stands still for a moment
	WndTimer cursor_hide_timer_;
	bool cursor_stay_visible_;			// if true, do not hide mouse cursor
	bool gamma_correction_;
	ICMTransform transform_;
	AutoPtr<DecoderJob> decoder_;		// decoder for current photo; keeps current photo
	AutoPtr<DecoderJob> decoder_next_;	// decoder for next/previous photo
	ConstPhotoInfoPtr next_photo_;		// next photo to preload or null
	ConstPhotoInfoPtr cur_photo_;		// current photo
	bool display_photo_description_;	// enable/disable displaing photo description
	YellowText yellow_text_;			// photo description drawing engine
	//CTransparentBar transparent_bar_wnd_;
	bool force_reloading_;
	bool show_description_if_no_photo_;
	ViewPaneNotifications* recipient_;
	int alpha_blending_steps_;			// number of steps for alpha blending
	PhotoCache* cache_;					// external cache of decoded photos (if any)
	CoolScrollBar scroll_bar_;
	bool scroll_bars_enabled_;			// if true scrollbars will be used
	bool use_cool_scroll_bars_;			// if true custom scrollbars will be used; normal sb otherwise
	DibPtr display_image_;				// if valid, this image will be displayed
	CRect padding_;						// image padding - extra space reserved in a view

	struct Impl;
	boost::scoped_ptr<Impl> impl_;

	CRect ClientRect() const;
	CRect ImageRect() const;

	void ChangeZoom(const uint16* zoom, size_t count, bool zoom_in);

	DibPtr GetCurBitmap() const;
	CacheImg* GetCurImg() const;

	// grab bmp from decoder and store it in a cache
	void CacheDecodedImage(AutoPtr<DecoderJob>& decoder);

	//int Zoom() const;
	void DrawPhoto(CDC* dc, Dib* bmp, int rotation, bool erase_bknd, int lines_from, int lines_to, ColorProfilePtr icc_profile);
	void DrawPhoto(CDC* dc, int lines_from, int lines_to, bool erase_bknd);

	// reload if necessary (specify requested logical zoom as input)
	bool ReloadCurrentPhoto(double zoom);
	bool ReloadPhotoIfNeeded(DibPtr dib, int reduction, CSize org_size, bool reset_decoder, double zoom);

	// transition to the next photo
	void TransitionEffect(CDC& dc);

	// alpha blending
	void DrawAlphaBlend(CDC& dc, DecoderJob* existing, DecoderJob* next);
	void DrawAlphaBlend(CDC& dc, Dib* existing, ColorProfilePtr icc_existing, Dib* next, ColorProfilePtr icc_next);
	void DrawAlphaBlendEx(CDC& dc, Dib& dibExisting, Dib& dibNext);
	void PrepareBmpCopy(DecoderJob* decoder, CDC& dc, Dib& dib);
	void PrepareBmpCopy(Dib& dibSrc, ColorProfilePtr icc, CDC& dc, Dib& dibDest);
	void AlphaTransition(Dib& dibExisting, Dib& dibNext, Dib& dibResult, int alpha);

	bool DecodingCurrentPhoto() const;

	double CalcPhysicalZoom(double logical_zoom, bool screen_ratio_fix= false, Dib* dib= 0) const;

	///void OnCustDrawScrollBar(UINT id, NMHDR* hdr, LRESULT* result);

	double CalcRangeSizeAndZoom(const CRect& clientRect, Dib* bmp, int& bmp_width, int& bmp_height,
								 int& width, int& height, CPoint& pos_img, CSize& range) const;
	double GetScreenAspectRatio() const;

	// scrolling support

	CPoint GetScrollPos() const;
	int GetScrollPos(int bar) const;
	bool GetScrollRange(int bar, int& minPos, int& maxPos) const;
	bool GetScrollInfo(int bar, SCROLLINFO* si) const;
	bool GetScrollInfo(int bar, SCROLLINFO* si, int mask) const;
	int GetScrollLimit(int bar) const;

	void SetScrollPos(int bar, int pos);
	void SetScrollPos(CPoint pos);
	void SetScrollSizes(DibPtr bmp);
	void SetScrollInfo(CSize bmp, CSize view, CPoint pos);

	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnScroll(UINT scroll_code, UINT pos);
	BOOL OnScrollBy(CSize scroll_size, BOOL do_scroll);

	void OnMButtonDown(UINT flags, CPoint point);
};


class CursorStayVisible
{
public:
	CursorStayVisible(ViewPane& view_pane) : view_pane_(view_pane)
	{ view_pane_.CursorStayVisible(true); }

	~CursorStayVisible()
	{ view_pane_.CursorStayVisible(false); }

private:
	ViewPane& view_pane_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWPANE_H__05E2DAAF_382B_42BA_95C8_A3D18650F385__INCLUDED_)
