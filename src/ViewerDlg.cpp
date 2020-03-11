/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ViewerDlg.h"
#include "PhotoVector.h"
#include "RString.h"
#include "Path.h"
#include "SmoothScroll.h"
#include "BrowserFrame.h"
#include "SlideShowOptDlg.h"
//#include "PhotoList.h"
#include "Logger.h"
#include "Tasks.h"
#include "CatchAll.h"
#include "Columns.h"
#include "ProfileVector.h"
#include "DrawFields.h"
#include "ViewerDlgNotifications.h"
#include "MagnifierWnd.h"
#include "clamp.h"
#include "ImageDraw.h"
#include "SetWindowSize.h"
#include "MultiMonitor.h"
#include "LightTable.h"
#include "Config.h"
#include "ICMProfile.h"
#include "viewer/SimpleReBar.h"
#include "viewer/PreviewBandWnd.h"
#include "viewer/SeparatorWnd2.h"
#include "viewer/DarkCloseBar.h"
#include "viewer/ViewCaption.h"
#include "ViewerToolBar.h"
#include "PhotoInfo.h"
#include "TagsBar.h"
#include "Profile.h"
#include "Transform.h"
#include "DrawFunctions.h"
#include "BmpFunc.h"
#include "InfoBand.h"
#include "ResizeWnd.h"
#include "ViewPane.h"
#include "Color.h"
#include "EditFileInfo.h"
#include <boost/function.hpp>
#include "DragAndDrop.h"
#include "TagsCommonCode.h"
#include "PhotoCollection.h"
#include "GetDefaultLineHeight.h"
#include "DateTimeUtils.h"
#include "ViewerTagPane.h"

extern AutoPtr<PhotoCache> global_photo_cache;		// one central photo cache
extern void OpenPhotograph(const TCHAR* photo_path, CWnd* wnd, bool raw);
extern int DragAndDrop(VectPhotoInfo& selected, CWnd* frame);


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


namespace {
	const TCHAR* REGISTRY_ENTRY_VIEWER= _T("Viewer");
	const TCHAR* REG_REBAR_LAYOUT= _T("ReBarLayout");
	const TCHAR* REG_INFO_BAND_FIELDS= _T("InfoBandFields");

	const COLORREF VIEWER_TEXT_COLOR= RGB(220,220,220);
	const COLORREF LABEL_COLOR= RGB(139,139,147);

	class TaskPending	// when any photo task is pending main window is disabled
	{
	public:
		TaskPending(CWnd& dlg) : dlg_(dlg)
		{
			//dlg_.ShowWindow(SW_HIDE);
			dlg_.EnableWindow(false);
		}

		~TaskPending()
		{
			//dlg_.ShowWindow(SW_SHOW);
			dlg_.EnableWindow(true);
		}

	private:
		CWnd& dlg_;
	};

	enum MoveDir { MoveUp, MoveDown, MoveLeft, MoveRight };
}


struct Pane
{
	Pane() : pane_(true)
	{
		caption_.text_color_ = VIEWER_TEXT_COLOR;
		caption_.label_color_ = LABEL_COLOR;
	}

	bool IsValid() const
	{
		return pane_.m_hWnd != 0 && caption_.m_hWnd != 0;
	}

	int CaptionHeight() const
	{
		CRect r;
		caption_.GetWindowRect(&r);
		return r.Height();
	}

	void Reset()
	{
		pane_.Reset();
		caption_.SetWindowText(_T(""));
	}

	ViewPane pane_;		// display pane
	Path last_loaded_photo_path_;
	ViewCaption caption_;	// and its caption
	std::auto_ptr<COleDropTarget> drop_target_;
};


class ViewPanes
{
public:
	enum Layout { Horizontal, Vertical, Grid };

	ViewPanes()
	{
		visible_count_ = 0;
		layout_ = Horizontal;
	}

	void ShowPanes(size_t count, Layout layout)
	{
		if (count < 1 || count > 4)
			count = 1;

		// hide some
		for (size_t i= COUNT - 1; i >= count; --i)
		{
			display_[i].pane_.ShowWindow(SW_HIDE);
			display_[i].caption_.ShowWindow(SW_HIDE);
		}

		if (count == 1)
			display_[0].caption_.ShowWindow(SW_HIDE);
		else
			for (size_t i= 0; i < count; ++i)
			{
				display_[i].pane_.ModifyStyle(0, WS_VISIBLE);
				display_[i].caption_.ShowWindow(SW_SHOWNA);
			}

		visible_count_ = count;

		//if (layout == Grid && count != 4)
		//	layout_ = Horizontal;
		//else
			layout_ = layout;
	}

	// cycle through the available layouts
	Layout GetNextLayout() const
	{
		if (visible_count_ < 2)
			return Horizontal;

		switch (layout_)
		{
		case Horizontal:
			return Vertical;

		case Vertical:
			return visible_count_ != 4 ? Horizontal : Grid;

		case Grid:
			return Horizontal;
		}

		return Horizontal;
	}

	Layout GetCurrentLayout() const
	{
		return layout_;
	}

	Layout GetEffectiveLayout() const
	{
		if (layout_ == Grid && visible_count_ != 4)
			return Horizontal;
		return layout_;
	}

	// after list of photos has changed make sure that visible views (apart from current 'view')
	// do not contain any stray photos
	void VerifyRemainingViews(ViewPane* view, VectPhotoInfo& photos)
	{
		for (size_t i= 0; i < COUNT; ++i)
			if (&display_[i].pane_ != view)
			{
				VectPhotoInfo::iterator it= find(photos.begin(), photos.end(), display_[i].pane_.GetCurrentPhoto());
				if (it == photos.end())
					display_[i].pane_.Reset();
			}
	}

	void ResetCaptions()
	{
		for (size_t i= 0; i < COUNT; ++i)
			display_[i].caption_.SetWindowText(_T(""));
	}

	size_t GetVisibleCount() const
	{
		return visible_count_;
	}

	ViewPane* GetNextView(ViewPane* view)
	{
		if (visible_count_ == 0)
			return 0;

		for (size_t i= 0; i < visible_count_; ++i)
			if (&display_[i].pane_ == view)
				return &display_[(i + 1) % visible_count_].pane_;

		ASSERT(false);
		return 0;
	}

	void ResetHiddenPanes()
	{
		for (size_t i= visible_count_; i < COUNT; ++i)
			display_[i].Reset();
	}

	bool IsValid() const
	{
		for (size_t i= 0; i < COUNT; ++i)
			if (!display_[i].IsValid())
				return false;
		return true;
	}

	ViewPane* GetPane(size_t index)
	{
		if (index < COUNT)
			return &display_[index].pane_;
		return 0;
	}

	ViewCaption* GetCaption(ViewPane* pane)
	{
		for (size_t i= 0; i < COUNT; ++i)
			if (&display_[i].pane_ == pane)
				return &display_[i].caption_;
		return 0;
	}

	Path* GetViewPhotoPath(ViewPane* pane)
	{
		for (size_t i= 0; i < COUNT; ++i)
			if (&display_[i].pane_ == pane)
				return &display_[i].last_loaded_photo_path_;
		return 0;
	}

	size_t GetDisplayIndex(ViewPane* pane) const
	{
		for (size_t i= 0; i < COUNT; ++i)
			if (&display_[i].pane_ == pane)
				return i;
		return ~0;
	}

	void SetActive(ViewPane* pane)
	{
		for (size_t i= 0; i < COUNT; ++i)
			display_[i].caption_.SetActive(&display_[i].pane_ == pane);
	}

	void InvalidatePanes()
	{
		for (size_t i= 0; i < visible_count_; ++i)
			display_[i].pane_.Invalidate();
	}

	void ResetRemainingWithPhoto(ViewPane* pane, PhotoInfoPtr photo)
	{
		// remove 'photo' from all views other than 'pane'
		for (size_t i= 0; i < COUNT; ++i)
			if (&display_[i].pane_ != pane && display_[i].pane_.GetCurrentPhoto() == photo)
				display_[i].pane_.Reset();
	}

	void ScrollOthers(ViewPane* pane, CPoint pos_img)
	{
		for (size_t i= 0; i < visible_count_; ++i)
			if (&display_[i].pane_ != pane)
				display_[i].pane_.ScrollTo(pos_img);
	}

	void Resize(int x_pos, int y_pos, CSize size)
	{
		if (visible_count_ == 0)
			return;

		for (size_t i= 0; i < visible_count_; ++i)
			display_[i].pane_.Invalidate();

		int h= std::max(size.cy, 0L);
		int w= std::max(size.cx, 0L);

		if (visible_count_ == 1)
		{
			// single pane is a special case - there is no caption visible
			SetWindowSize(display_[0].pane_, x_pos, y_pos, w, h);
			return;
		}

		const int CAPTION= display_[0].CaptionHeight();	// all captions have the same height
		const int VSEP= 4;	// separator between vertically oriented panes

		Pane* panes[]= { &display_[0], &display_[1], &display_[2], &display_[3] };

		switch (layout_)
		{
		case Horizontal:
			HorzLayout(x_pos, y_pos, w, h, panes, visible_count_, CAPTION);
			break;

		case Vertical:
			VertLayout(x_pos, y_pos, w, h, panes, visible_count_, CAPTION, VSEP);
			break;

		case Grid:
			if (visible_count_ == 4)
			{
				int height= h / 2;

				Pane* top[]= { &display_[0], &display_[1] };
				VertLayout(x_pos, y_pos, w, height, top, array_count(top), CAPTION, VSEP);

				Pane* bottom[]= { &display_[2], &display_[3] };
				VertLayout(x_pos, y_pos + (h - height), w, height, bottom, array_count(bottom), CAPTION, VSEP);
			}
			else
			{
				//??
				// not allowed
				//ASSERT(false);


				HorzLayout(x_pos, y_pos, w, h, panes, visible_count_, CAPTION);
			}
			break;

		default:
			ASSERT(false);
			break;
		}

	}

	static int get_pos(size_t step, size_t count, int span, int separator)
	{
		//    pane1  sep  pane2  sep  pane3
		// |---------|-|---------|-|---------|
		// x0          x1          x2

		if (count <= 1 || step >= count)
			return 0;

		return static_cast<int>((step * (span + separator) + count / 2) / count);
	}

	// panes stretched horizontally
	static void HorzLayout(int x_pos, int y_pos, int width, int height, Pane* panes[], size_t count, int caption_h)
	{
		if (count == 0)
			return;

		// = horizontal layout

		int min_height= caption_h * static_cast<int>(count);
		if (height < min_height)
			height = min_height;

		// height of a single pane (uniform across all of them!)
		int h= std::max<int>(0, height / static_cast<int>(count) - caption_h);

		for (size_t i= 0; i < count; ++i)
		{
			int y= y_pos + get_pos(i, count, height, 0);

			SetWindowSize(panes[i]->caption_, x_pos, y, width, caption_h);
			SetWindowSize(panes[i]->pane_, x_pos, y + caption_h, width, h);
		}
	}

	// panes stretched vertically
	static void VertLayout(int x_pos, int y_pos, int width, int height, Pane* panes[], size_t count, int caption_h, int vert_separator)
	{
		if (count == 0)
			return;

		// || vertical layout

		int h= height - caption_h;
		if (h < 0)
			h = 0;

		int min_width= vert_separator * static_cast<int>(count - 1);
		if (width < min_width)
			width = min_width;

		// width of a single pane (uniform across all of them!)
		int w= std::max<int>(0, (width + vert_separator) / static_cast<int>(count) - vert_separator);

		for (size_t i= 0; i < count; ++i)
		{
			int x= x_pos + get_pos(i, count, width, vert_separator);

			SetWindowSize(panes[i]->caption_, x, y_pos, w, caption_h);
			SetWindowSize(panes[i]->pane_, x, y_pos + caption_h, w, h);
		}
	}


	void CursorStayVisible(bool visible)
	{
		for (size_t i= 0; i < visible_count_; ++i)
			display_[i].pane_.CursorStayVisible(visible);
	}

	void UseScrollBars(bool enable)
	{
		for (size_t i= 0; i < COUNT; ++i)
			display_[i].pane_.UseScrollBars(enable);
	}

	void EnablePhotoDesc(bool enable)
	{
		for (size_t i= 0; i < COUNT; ++i)
			display_[i].pane_.EnablePhotoDesc(enable);
	}

	ViewPane* FindViewPane(HWND wnd)
	{
		for (size_t i= 0; i < visible_count_; ++i)
			if (display_[i].pane_.m_hWnd == wnd)
				return &display_[i].pane_;
		return 0;
	}

	void Create(CWnd* parent, const boost::function<void (ViewPane*)>& view_clicked, const boost::function<void (ViewPane*, PhotoInfoPtr path)>& photo_dropped, const boost::function<PhotoInfoPtr (const TCHAR* path)>& path_to_photo, ViewPaneNotifications* host);

	bool IsViewVisible(ViewPane* pane) const
	{
		for (size_t i= 0; i < visible_count_; ++i)
			if (&display_[i].pane_ == pane)
				return true;
		return false;
	}

	void for_all(void (ViewPane::* fn)(void))
	{
		for (size_t i= 0; i < COUNT; ++i)
			(display_[i].pane_.*fn)();
	}

	template <class A> void for_all(void (ViewPane::* fn)(A), A a)
	{
		for (size_t i= 0; i < COUNT; ++i)
			(display_[i].pane_.*fn)(a);
	}

	template <class A, class B> void for_all(void (ViewPane::* fn)(A, B), A a, B b)
	{
		for (size_t i= 0; i < COUNT; ++i)
			(display_[i].pane_.*fn)(a, b);
	}

	template <class A, class B, class C> void for_all(void (ViewPane::* fn)(A, B, C), A a, B b, C c)
	{
		for (size_t i= 0; i < COUNT; ++i)
			(display_[i].pane_.*fn)(a, b, c);
	}

private:
	void ViewClicked(ViewPane* view)
	{
		if (view_clicked_)
			view_clicked_(view);
	}

	//void Assign(const boost::function<void (ViewPane*)>& view_clicked);

	DROPEFFECT PhotoDragAndDrop(size_t pane_index, PhotoDrop::DropAction action, const TCHAR* files);

	boost::function<void (ViewPane*)> view_clicked_;
	boost::function<PhotoInfoPtr (const TCHAR* path)> path_to_photo_;
	boost::function<void (ViewPane* view, PhotoInfoPtr photo)> photo_dropped_;

	static const size_t COUNT= 4;
	Pane display_[COUNT];		// up to four display panes
	size_t visible_count_;
	Layout layout_;
};


class CursorVisible
{
public:
	CursorVisible(ViewPanes& viewPanes) : viewPanes_(viewPanes)
	{
		viewPanes_.CursorStayVisible(true);
		disabled_ = false;
	}

	~CursorVisible()
	{
		if (!disabled_)
			viewPanes_.CursorStayVisible(false);
	}

	void Disable()
	{
		disabled_ = true;
	}

private:
	bool disabled_;
	ViewPanes& viewPanes_;
};


struct ViewerDlg::Impl : InfoBandNotification, ResizeWnd, ViewPaneNotifications
{
	Impl(PhotoInfoStorage& storage, PhotoCache* cache, VectPhotoInfo& photos, ViewerDlgNotifications* recipient, const Columns&	columns);
	~Impl();

	void Create(CWnd* wnd, Logger& log);
	void Resize(CWnd* wnd);
	void SetInfo();
	void SetViewCaption(ViewPane* view, ConstPhotoInfoPtr photo);
	void DrawItem(CDC& dc, CRect rect, size_t item, AnyPointer key);
	void EraseBandBk(CDC& dc, int id, CRect band, bool in_line);
	String ItemToolTipText(size_t item, AnyPointer key);
	void LoadPhoto(int item_index, bool load_hint= true);
	void LoadPhoto(int item_index, bool load_hint, bool scroll_to_it);
	void LoadPhoto(ConstPhotoInfoPtr photo);
	void LoadPhoto(ConstPhotoInfoPtr photo, bool scroll_to_it);
	void LoadPhoto(const PhotoInfo& photo, ConstPhotoInfoPtr next_photo, bool force_reload);
	void LoadPhotoIntoView(ViewPane* view, ConstPhotoInfoPtr photo);
	void Clean();
	void Synch(bool incremental);
	void Destroy(CWnd* wnd);
	void SlideShowOptions(CWnd* wnd);
	void ViewerOptions(CWnd* wnd, bool press_btn);
	void Delete(CWnd* wnd);
	void ResetSettings();	// call after settings have changed
	void SetColors(const std::vector<COLORREF>& colors);
	void SetUIBrightness(double gamma);
	void ResetColors();
	void Rotate(unsigned int rotation);
	void RotatePhoto(CWnd* wnd, RotationTransformation transform);
	//void PhotoList(CWnd* wnd, bool press_btn);
	void OnTimer(CWnd* wnd, UINT_PTR id_event);
	void StopSlideShow(CWnd* wnd);
	void StartSlideShow(CWnd* wnd);
	void ShowCloseBar(bool show);
	void WndMode(CWnd* wnd);			// window mode
	void FullScreen(CWnd* wnd);			// enter full screen mode
	void PhotoSelected(PhotoInfo& photo);	// new photo edited in description dlg
	void ItemClicked(size_t item, AnyPointer key, PreviewBandWnd::ClickAction action);
	void SendRotateNotification();
	void ShowMultiView(bool show);
	void ReinitMultiViews();
	void SetActiveView();
	void MagnifierLens();
	void LoadBitmaps(double gamma);
	void PhotoPrev();
	void PhotoNext();
	void SwitchViews();
	size_t FindPhotoIndex(PhotoInfoPtr photo);
	ViewPane& FirstPane()		{ return *displays_.GetPane(0); }
	void MultiView(CWnd* wnd, bool pressBtn);
	void ShowViewerToolbar(bool showTb, bool showInfo);
	void ZoomToFit();
	void Zoom100(CPoint offset= CPoint(-1, -1));

	static CString wndClass_;

	CWnd* wnd_;	// viewer window (self)
	ViewerToolBar	toolbar_;			// toolbar band
	InfoBand		status_wnd_;		// info band
	DarkCloseBar	close_wnd_;			// close btn band
	LightTable		light_table_;		// light table pane
	SimpleReBar		rebar_;
	PreviewBandWnd	preview_;
	ViewerSeparatorWnd separator_;		// separator between preview list and main display (resizing support)
	ViewPane*		display_;			// current display pane
	ViewPanes		displays_;
	const Columns&	columns_;
	ViewerTagPane	tags_pane_;

	enum { INFOBAR= 111 };
	Dib infbar_;
	Dib rebar_bgnd_;
	int previewPaneHeight_;
	COLORREF tag_text_color_;
	COLORREF tag_backgnd_color_;
	bool show_tags_in_previewbar_;
	UINT_PTR timer_id_;					// delay timer
	String photo_name_;					// photo file name
	enum { BAND_TOOLBAR= 10, BAND_INFO }; //, BAND_CLOSE };
	bool balloons_enabled_;				// display balloon photo info
	bool toolbar_visible_;
	bool infobar_visible_;
	bool preview_bar_visible_;
	UINT_PTR slide_show_timer_;			// timer for delay between slide show photos
	int slideShowDelay_;
//	UINT slide_show_photo_delay_;		// delay in seconds
//	CConnection<CTagsBar> tags_wnd_;
	ConfigRegistrar config_;			// register to receive settings changes
	//bool horz_views_;					// horizontal layout of two view panes
	ViewPanes::Layout view_layout_;
	double ui_gamma_correction_;		// gamma correction for toolbar and preview background images

	enum { TIMER_SET_INFO= 10, TIMER_SLIDE_SHOW };

	bool full_screen_;
	DWORD wnd_style_;
	CRect wnd_rectangle_;				// store window pos while in full screen mode
	VectPhotoInfo& photos_;				// photo info
	int cur_image_index_;				// current photo index
	PhotoInfoPtr current_image_;		// current photo
	bool rotation_feasible_;
	bool originalOrientationPreserved_;
//	PhotoInfo::ImgOrientation orientation_;
	//int previewPaneHeight_;
	bool smooth_scroll_;
	bool keep_current_img_centered_;
	bool showMagnifierLens_;
	ICMTransform transform_;
	CWnd* timer_wnd_;
	auto_connection tags_changed_;
	auto_connection rating_changed_;
	auto_connection metadata_changed_;

	// preview bar labels
	enum LabelType { LABEL_NONE= 0, LABEL_NAME, LABEL_NAME_EXT, LABEL_DATE };
	LabelType preview_bar_labels_;

	Profile<int>  profilePreviewPaneHeight_;
	Profile<bool> profileShowBalloonInfo_;
	Profile<bool> profilePreviewBarVisible_;
	Profile<bool> profileRebarVisible_;
	Profile<int>  profileSlideShowDelay_;
	Profile<bool> profileSlideShowRepeat_;
	Profile<bool> profileSlideShowHideTb_;
	//Profile<bool> profileGammaCorrection_;
	Profile<bool> profileFullScreen_;
	Profile<int>  profileLightTableWidth_;
	Profile<bool> profileLightTableVisible_;
	Profile<int>  profile_tag_pane_width_;
	Profile<bool> profile_tag_pane_visible_;
	WindowPosition windowPosition_;
	Profile<bool> profileInfoBarZoomField_;
	Profile<bool> profileInfoBarNameField_;
	Profile<bool> profileShowPhotoDescription_;
	Profile<bool> profileUseScrollBars_;
	Profile<bool> profileHorzPreviewBar_;
	Profile<bool> profileTagsInPreviewBar_;
	Profile<bool> profileSmoothScroll_;
	Profile<bool> profileMultiViewEnabled_;
	Profile<int> profileMultiViewCount_;
	Profile<int> profileViewLayout_;
	Profile<bool> profileShowFieldNames_;
//	Profile<bool> profileHorzView_;
	Profile<bool> profile_keep_current_centered_;
	Profile<int>  profile_preview_bar_labels_;

	// notifications are being sent here
	ViewerDlgNotifications* host_;

	// info band notifications
	virtual void FieldSelectionChanged();
	virtual void PopupMenu(bool displayed);

	// preview resizing support
	virtual int GetPaneHeight();
	virtual void ResizePane(int height);
	virtual void Resizing(bool start);

	// view pane notifications
	virtual void DecodingStarted(ViewPane* view);
	virtual void DecodingFinished(ViewPane* view);
	virtual void DecodingThreadFinished(ViewPane* view);
	virtual void ViewClicked(ViewPane* view);
	virtual void ViewScrolled(ViewPane* view, CPoint pos_img);
	virtual void MiddleButtonDown(CPoint pos);
	virtual void MouseDoubleClick(CPoint pos);
	virtual void AnimationStarts();

	// move to the next/prev photo or scroll
	void Move(MoveDir move);

	// populate popup menu with tags
	void ResetMenuTags(CMenu& menu, int first_id);

	// photo dropped into the view
	void ViewPhotoDropped(ViewPane* view, PhotoInfoPtr photo);

	// try to locate photo based on the given path
	PhotoInfoPtr FindPhoto(const TCHAR* original_path) const;

	// photos have changed, refresh views
	void RefreshAfterChange(const VectPhotoInfo& photos);

	void SwitchPreviewBarLabels(LabelType labels);

	// refresh tags displayed in the tag window to sync them with current image
	void RefreshTags();
};


//=======================================================================================

namespace {
	static std::vector<uint16> g_zoom;	// zoom in percents
}

static void PrepareZoomTable(double zoom_factor)
{
	if (!g_zoom.empty())
		return;

	// arbitrarily selected limits (may be slightly exceeded b/c of exponent floor/ceil)
	double min_zoom= 0.05;
	double max_zoom= 16.0;

	double b= log(zoom_factor);
	int min_exp= static_cast<int>(floor(log(min_zoom) / b));
	int max_exp= static_cast<int>(ceil(log(max_zoom) / b));

	g_zoom.reserve(max_exp - min_exp + 1);

	for (int e= min_exp; e <= max_exp; ++e)
		g_zoom.push_back(static_cast<uint16>(100.0 * pow(zoom_factor, e) + 0.5));

//	{ 5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100, 125, 150, 175, 200, 250, 300, 400, 600, 1000, 1500 };
}


ViewerDlg::Impl::Impl(PhotoInfoStorage& storage, PhotoCache* cache, VectPhotoInfo& photos,
					   ViewerDlgNotifications* recipient, const Columns& columns)
 : light_table_(storage, cache), photos_(photos), config_(g_Settings),
	host_(recipient), windowPosition_(REGISTRY_ENTRY_VIEWER), columns_(columns), status_wnd_(columns),
	tags_pane_(Tags::GetTagCollection())
{
	display_ = &FirstPane();
	wnd_ = 0;
	ui_gamma_correction_ = 1.0;
	full_screen_ = false;
	wnd_style_ = 0;
	wnd_rectangle_.SetRectEmpty();
	cur_image_index_ = 0;
	current_image_ = 0;
	originalOrientationPreserved_ = false;
	//current_orientation_ = 0;
//	orientation_ = PhotoInfo::ORIENT_NO_INFO;
	timer_id_ = 0;
	infobar_visible_ = toolbar_visible_ = true;
	slide_show_timer_ = 0;
	slideShowDelay_ = 0;
	rotation_feasible_ = false;
	previewPaneHeight_ = 80;
	tag_text_color_ = tag_backgnd_color_ = 0;
	timer_wnd_ = 0;
	view_layout_ = ViewPanes::Vertical;

	PrepareZoomTable(pow(2.0, 1.0 / 3.0));

	profilePreviewPaneHeight_.Register(REGISTRY_ENTRY_VIEWER, _T("PreviewPaneHeight"), previewPaneHeight_);
	profilePreviewBarVisible_.Register(REGISTRY_ENTRY_VIEWER, _T("PreviewPaneVisible"), true);
	profileRebarVisible_.Register(REGISTRY_ENTRY_VIEWER, _T("RebarVisible"), true);
	profileShowBalloonInfo_.Register(REGISTRY_ENTRY_VIEWER, _T("ShowBalloonInfo"), true);
	profileSlideShowDelay_.Register(REGISTRY_ENTRY_VIEWER, _T("SlideShowDelay"), 5);
	profileSlideShowRepeat_.Register(REGISTRY_ENTRY_VIEWER, _T("SlideShowRepeat"), true);
	profileSlideShowHideTb_.Register(REGISTRY_ENTRY_VIEWER, _T("SlideShowHideTb"), true);
//	profileGammaCorrection_.Register(REGISTRY_ENTRY_VIEWER, _T("GammaCorrection"), true);
	profileFullScreen_.Register(REGISTRY_ENTRY_VIEWER, _T("FullScreen"), false);
	profileLightTableWidth_.Register(REGISTRY_ENTRY_VIEWER, _T("LightTableWidth"), 232); // initial width to fit whole toolbar
	profileLightTableVisible_.Register(REGISTRY_ENTRY_VIEWER, _T("LightTableVisible"), false);
	profile_tag_pane_width_.Register(REGISTRY_ENTRY_VIEWER, _T("TagsPaneWidth"), 140);
	profile_tag_pane_visible_.Register(REGISTRY_ENTRY_VIEWER, _T("TagsPaneVisible"), false);
	profileInfoBarZoomField_.Register(REGISTRY_ENTRY_VIEWER, _T("InfoBandZoomField"), true);
	profileInfoBarNameField_.Register(REGISTRY_ENTRY_VIEWER, _T("InfoBandNameField"), true);
	profileShowPhotoDescription_.Register(REGISTRY_ENTRY_VIEWER, _T("ShowPhotoDescription"), true);
	profileUseScrollBars_.Register(REGISTRY_ENTRY_VIEWER, _T("UseScrollBars"), true);
	profileHorzPreviewBar_.Register(REGISTRY_ENTRY_VIEWER, _T("HorzPreviewBar"), false);
	profileTagsInPreviewBar_.Register(REGISTRY_ENTRY_VIEWER, _T("TagsInPreviewBar"), true);
	profileSmoothScroll_.Register(REGISTRY_ENTRY_VIEWER, _T("SmoothScroll"), false);
	profileMultiViewEnabled_.Register(REGISTRY_ENTRY_VIEWER, _T("MultiView"), false);
	//profileHorzView_.Register(REGISTRY_ENTRY_VIEWER, _T("MultiViewHorz"), horz_views_);
	profileViewLayout_.Register(REGISTRY_ENTRY_VIEWER, _T("MultiViewLayout"), view_layout_);
	profileMultiViewCount_.Register(REGISTRY_ENTRY_VIEWER, _T("MultiViewCount"), 2);
	profileShowFieldNames_.Register(REGISTRY_ENTRY_VIEWER, _T("ShowFieldNames"), true);
	profile_keep_current_centered_.Register(REGISTRY_ENTRY_VIEWER, _T("KeepCurrentCentered"), false);
	profile_preview_bar_labels_.Register(REGISTRY_ENTRY_VIEWER, _T("PreviewBarLabels"), LABEL_NONE);

	preview_bar_visible_ = profilePreviewBarVisible_;
	toolbar_visible_ = profileRebarVisible_;
	previewPaneHeight_ = profilePreviewPaneHeight_;
	balloons_enabled_ = profileShowBalloonInfo_;
	show_tags_in_previewbar_ = profileTagsInPreviewBar_;
	smooth_scroll_ = profileSmoothScroll_;
	keep_current_img_centered_ = profile_keep_current_centered_;
	view_layout_ = static_cast<ViewPanes::Layout>(profileViewLayout_.Value());
	preview_bar_labels_ = static_cast<LabelType>(profile_preview_bar_labels_.Value());

	status_wnd_.show_field_names_ = profileShowFieldNames_;

	status_wnd_.show_zoom_ = profileInfoBarZoomField_;
	status_wnd_.show_name_ = profileInfoBarNameField_;
	GetProfileVector(REGISTRY_ENTRY_VIEWER, REG_INFO_BAND_FIELDS, status_wnd_.fields_);
	status_wnd_.text_color_ = VIEWER_TEXT_COLOR;
	status_wnd_.label_color_ = LABEL_COLOR;

	showMagnifierLens_ = false;

	tags_changed_ =		PhotoCollection::Instance().ConnectOnTagsChanged(boost::bind(&Impl::RefreshAfterChange, this, _1));
	rating_changed_ =	PhotoCollection::Instance().ConnectOnRatingChanged(boost::bind(&Impl::RefreshAfterChange, this, _1));
	metadata_changed_ =	PhotoCollection::Instance().ConnectOnMetadataChanged(boost::bind(&Impl::RefreshAfterChange, this, _1));
}


ViewerDlg::Impl::~Impl()
{
	try
	{
		profilePreviewPaneHeight_ = previewPaneHeight_;
		profileShowBalloonInfo_ = balloons_enabled_;
		profilePreviewBarVisible_ = preview_bar_visible_;
		profileRebarVisible_ = toolbar_visible_;
		profile_preview_bar_labels_ = preview_bar_labels_;

		profileInfoBarZoomField_ = status_wnd_.show_zoom_;
		profileInfoBarNameField_ = status_wnd_.show_name_;

		profileViewLayout_ = view_layout_;
		profileSmoothScroll_ = smooth_scroll_;
		profile_keep_current_centered_ = keep_current_img_centered_;
	}
	catch (...)
	{
		ASSERT(false);
	}
}


CString ViewerDlg::Impl::wndClass_;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ViewerDlg window

ViewerDlg::ViewerDlg(ConnectionPointer* link, VectPhotoInfo& photos, PhotoInfoStorage& storage,
						ViewerDlgNotifications* recipient, const Columns& columns)
	: pImpl_(new Impl(storage, global_photo_cache.get(), photos, recipient, columns)), ConnectionPointer(link)
{
	ASSERT(pImpl_->host_);

	pImpl_->timer_wnd_ = this;
//	config_.RegisterForGammaChange(std::bind1st(std::mem_fun(&ViewerDlg::OnGammaChanged), this));

//	transform_.Reset(g_Settings.default_photo_->profile_, g_Settings.viewer_->profile_, g_Settings.viewer_->rendering_);
}


ViewerDlg::~ViewerDlg()
{}


BEGIN_MESSAGE_MAP(ViewerDlg, CFrameWnd)
	//{{AFX_MSG_MAP(ViewerDlg)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEWER_BAR, OnViewerBar)
	ON_COMMAND(ID_VIEWER_FULLSCR, OnFullScreen)
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(ID_VIEWER_FULLSCR, OnUpdateFullScreen)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_BAR, OnUpdateViewerBar)
	ON_COMMAND(ID_PHOTO_FIRST, OnPhotoFirst)
	ON_COMMAND(ID_PHOTO_LAST, OnPhotoLast)
	ON_COMMAND(ID_PHOTO_NEXT, OnPhotoNext)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_NEXT, OnUpdatePhotoNext)
	ON_COMMAND(ID_PHOTO_PREV, OnPhotoPrev)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_PREV, OnUpdatePhotoPrev)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_PHOTO_LIST, OnPhotoList)
	ON_COMMAND(ID_ZOOM_IN, OnZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, OnUpdateZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, OnZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, OnUpdateZoomOut)
	ON_COMMAND(ID_ZOOM_FIT, OnZoomFit)
	ON_COMMAND(ID_ZOOM_100, OnZoom100)
	ON_COMMAND(ID_PHOTO_DESC, OnPhotoDesc)
	ON_WM_DESTROY()
	ON_WM_NCDESTROY()
	ON_WM_SYSCOMMAND()
	ON_COMMAND(SC_CLOSE, OnCmdClose)
	ON_COMMAND(SC_RESTORE, OnRestore)
	//ON_COMMAND(ID_GAMMA, OnGamma)
	//ON_UPDATE_COMMAND_UI(ID_GAMMA, OnUpdateGamma)
	ON_COMMAND(ID_VIEWER_BALLOONS, OnViewerBalloons)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_BALLOONS, OnUpdateViewerBalloons)
	ON_COMMAND(ID_VIEWER_INFOBAR, OnViewerInfobar)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_INFOBAR, OnUpdateViewerInfobar)
	ON_COMMAND(ID_VIEWER_TOOLBAR, OnViewerToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_TOOLBAR, OnUpdateViewerToolbar)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_UPDATE_COMMAND_UI(ID_DELETE, OnUpdateDelete)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_100, OnUpdateZoom100)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_FIT, OnUpdateZoomFit)
	ON_COMMAND(ID_START_SLIDE_SHOW, OnStartSlideShow)
	ON_UPDATE_COMMAND_UI(ID_START_SLIDE_SHOW, OnUpdateStartSlideShow)
	ON_COMMAND(ID_STOP_SLIDE_SHOW, OnStopSlideShow)
	ON_UPDATE_COMMAND_UI(ID_STOP_SLIDE_SHOW, OnUpdateStopSlideShow)
	ON_COMMAND(ID_TAGS_BAR, OnTagsBar)
	ON_UPDATE_COMMAND_UI(ID_TAGS_BAR, OnUpdateTagsBar)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_PHOTO_FIRST, OnUpdatePhotoFirst)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_LAST, OnUpdatePhotoLast)
	ON_UPDATE_COMMAND_UI(ID_PHOTO_LIST, OnUpdatePhotoList)
	ON_COMMAND(ID_SLIDE_SHOW_PARAMS, OnSlideShowParams)
	ON_UPDATE_COMMAND_UI(ID_SLIDE_SHOW_PARAMS, OnUpdateSlideShowParams)
	ON_COMMAND(ID_ROTATE_90_CW, OnRotate90CW)
	ON_UPDATE_COMMAND_UI(ID_ROTATE_90_CW, OnUpdateRotate90CW)
	ON_COMMAND(ID_ROTATE_90_CCW, OnRotate90CCW)
	ON_UPDATE_COMMAND_UI(ID_ROTATE_90_CCW, OnUpdateRotate90CCW)
	ON_COMMAND(ID_JPEG_AUTO_ROTATE, OnJpegAutoRotate)
	ON_UPDATE_COMMAND_UI(ID_JPEG_AUTO_ROTATE, OnUpdateJpegAutoRotate)
	ON_COMMAND(ID_JPEG_ROTATE_90_CCW, OnJpegRotate90CCW)
	ON_UPDATE_COMMAND_UI(ID_JPEG_ROTATE_90_CCW, OnUpdateJpegRotate90CCW)
	ON_COMMAND(ID_JPEG_ROTATE_90_CW, OnJpegRotate90CW)
	ON_UPDATE_COMMAND_UI(ID_JPEG_ROTATE_90_CW, OnUpdateJpegRotate90CW)
	ON_COMMAND(ID_TOGGLE_LIGHT_TABLE, OnToggleLightTable)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_LIGHT_TABLE, OnUpdateToggleLightTable)
	ON_COMMAND(ID_ADD_TO_LIGHT_TABLE, OnAddToLightTable)
	ON_UPDATE_COMMAND_UI(ID_ADD_TO_LIGHT_TABLE, OnUpdateAddToLightTable)
	ON_COMMAND(ID_REMOVE_FROM_LIGHT_TABLE, OnRemoveFromLightTable)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_FROM_LIGHT_TABLE, OnUpdateRemoveFromLightTable)
	ON_COMMAND(ID_VIEWER_OPTIONS, OnViewerOptions)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_OPTIONS, OnUpdateViewerOptions)
	ON_COMMAND(ID_MAGNIFIER_LENS, OnMagnifierLens)
	ON_UPDATE_COMMAND_UI(ID_MAGNIFIER_LENS, OnUpdateMagnifierLens)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_TABLE_TAGS, OnUpdateLightTableTags)
	//}}AFX_MSG_MAP
//	ON_COMMAND(IDCLOSE, OnCmdClose)		// accel Esc sends IDCLOSE command
//	ON_NOTIFY(RBN_HEIGHTCHANGE, HorzReBar::HREBAR_CTRL_ID, OnReBarHeightChange)
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnTbDropDown)
	ON_WM_TIMER()
	ON_COMMAND(ID_LIGHT_TABLE_COPY, OnLightTableCopy)
	ON_COMMAND(ID_LIGHT_TABLE_HTML, OnLightTableHTML)
	ON_COMMAND(ID_LIGHT_TABLE_SELECT, OnLightTableSelect)
	ON_COMMAND(ID_LIGHT_TABLE_RESIZE, OnLightTableResize)
	ON_COMMAND(ID_EMPTY_LIGHT_TABLE, OnLightTableEmpty)
	ON_COMMAND(ID_LIGHT_DELETE_ALL_OTHERS, OnLightTableDeleteAllOthers)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_DELETE_ALL_OTHERS, OnUpdateLightTableOperations)
	ON_UPDATE_COMMAND_UI(ID_EMPTY_LIGHT_TABLE, OnUpdateLightTableOperations)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_TABLE_OPTIONS, OnUpdateLightTableOperations)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_TABLE_COPY, OnUpdateLightTableOperations)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_TABLE_SELECT, OnUpdateLightTableOperations)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_TABLE_RESIZE, OnUpdateLightTableOperations)

	ON_COMMAND_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + MAX_TAGS - 1, OnTagSelected)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + MAX_TAGS - 1, OnUpdateTags)

	ON_COMMAND_RANGE(ID_RATE_NONE, ID_RATE_5_STARS, OnRateImage)
	ON_UPDATE_COMMAND_UI_RANGE(ID_RATE_NONE, ID_RATE_5_STARS, OnUpdateRates)

	ON_WM_KEYDOWN()
	ON_COMMAND(ID_TOGGLE_DESCRIPTION, OnToggleDispDescription)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_DESCRIPTION, OnUpdateToggleDispDescription)
	ON_COMMAND(ID_USE_SCROLLBARS, OnUseScrollBars)
	ON_UPDATE_COMMAND_UI(ID_USE_SCROLLBARS, OnUpdateUseScrollBars)
	ON_COMMAND(ID_VIEWER_BAR_HORZ, OnHorzPreviewBar)
	ON_COMMAND(ID_VIEWER_BAR_VERT, OnVertPreviewBar)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_BAR_HORZ, OnUpdateHorzPreviewBar)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_BAR_VERT, OnUpdateVertPreviewBar)

	ON_COMMAND_RANGE(ID_VIEWER_BAR_DATE_TIME, ID_VIEWER_BAR_NO_LABELS, OnChangePreviewLabels)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEWER_BAR_DATE_TIME, ID_VIEWER_BAR_NO_LABELS, OnUpdateEnableLabels)

	ON_COMMAND(ID_SMALL_ICONS, OnSmallIcons)
	ON_COMMAND(ID_LARGE_ICONS, OnLargeIcons)
	ON_UPDATE_COMMAND_UI(ID_SMALL_ICONS, OnUpdateSmallIcons)
	ON_UPDATE_COMMAND_UI(ID_LARGE_ICONS, OnUpdateLargeIcons)
	ON_COMMAND(ID_HIDE_LIGHT_TABLE, OnHideLightTable)
	ON_COMMAND(ID_HIDE_TAG_PANE, OnHideTagPane)
	ON_COMMAND(ID_VIEWER_BAR_OVERLAY_TAGS, OnToggleTagsInPreviewBar)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_BAR_OVERLAY_TAGS, OnUpdateToggleTagsInPreviewBar)
	ON_COMMAND(ID_VIEWER_BAR_SMOOTH_SCROLL, OnToggleSmoothScroll)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_BAR_SMOOTH_SCROLL, OnUpdateToggleSmoothScroll)
	ON_COMMAND(ID_VIEWER_BAR_CENTER, OnToggleCenterImage)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_BAR_CENTER, OnUpdateToggleCenterImage)
	ON_MESSAGE(WM_APPCOMMAND, OnAppCommand)
	ON_COMMAND(ID_OPEN_PHOTO, OnOpenPhoto)
	ON_COMMAND(ID_TOGGLE_MULTIPLE_PANES, OnToggleMultiView)
	ON_COMMAND(ID_COMPARE_MULTIPLE, OnToggleMultiView)
	ON_UPDATE_COMMAND_UI(ID_COMPARE_MULTIPLE, OnUpdateMultiView)
	ON_COMMAND(ID_TOGGLE_VIEW_LAYOUT, OnToggleMultiViewLayout)
	ON_COMMAND(ID_CLOSE_VIEW, OnToggleMultiView)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_USER+1234, OnCloseApp)
	ON_COMMAND(ID_MOVE_UP, OnMoveUp)
	ON_COMMAND(ID_MOVE_DOWN, OnMoveDown)
	ON_COMMAND(ID_MOVE_LEFT, OnMoveLeft)
	ON_COMMAND(ID_MOVE_RIGHT, OnMoveRight)
	ON_COMMAND(ID_SWITCH_VIEWS, OnSwitchViews)
	ON_COMMAND_RANGE(ID_SINGLE_PANE, ID_FOUR_PANES, OnTurnMultiPane)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SINGLE_PANE, ID_FOUR_PANES, OnUpdateMultiPane)

	ON_COMMAND_RANGE(ID_HORIZONTAL_LAYOUT, ID_SWITCH_LAYOUT, OnChangeMultiPaneLayout)
	ON_UPDATE_COMMAND_UI_RANGE(ID_HORIZONTAL_LAYOUT, ID_SWITCH_LAYOUT, OnUpdateMultiPaneLayout)

	ON_COMMAND_RANGE(ID_VIEW_PANE_1, ID_VIEW_PANE_4, OnActivateViewPane)

	ON_COMMAND(ID_RENAME_FILE, OnRenamePhoto)
	ON_COMMAND(ID_TASK_PRINT, OnPrint)
	ON_UPDATE_COMMAND_UI(ID_TASK_PRINT, OnUpdatePrint)
	ON_COMMAND(ID_SET_WALLPAPER, OnSetAsWallpaper)
	ON_UPDATE_COMMAND_UI(ID_SET_WALLPAPER, OnUpdateSetAsWallpaper)

	ON_WM_MBUTTONDOWN()

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ViewerDlg message handlers


void ViewerDlg::OnSize(UINT type, int cx, int cy)
{
	CFrameWnd::OnSize(type, cx, cy);

	if (type == SIZE_RESTORED || type == SIZE_MAXIMIZED)
		pImpl_->Resize(this);
}


// reposition windows
//
void ViewerDlg::Impl::Resize(CWnd* wnd)
{
	if (preview_.m_hWnd == 0 || rebar_.m_hWnd == 0 || separator_.m_hWnd == 0 ||
		light_table_.m_hWnd == 0 || close_wnd_.m_hWnd == 0 || wnd == 0 || !displays_.IsValid() || tags_pane_.m_hWnd == 0)
		return;

	CRect cl_rect(0,0,0,0);
	wnd->GetClientRect(cl_rect);
	CSize wnd_size= cl_rect.Size();

	if (cl_rect.IsRectEmpty())
		return;

//	displays_.for_all(&CWnd::Invalidate);

	int y_pos= 0;
	int x_pos= 0;

	if (rebar_.GetStyle() & WS_VISIBLE)
	{
		int width= wnd_size.cx;
		int h= rebar_.GetHeight();

		if (close_wnd_.CWnd::GetStyle() & WS_VISIBLE)
		{
			int w= DarkCloseBar::CLOSE_BAR_WIDTH;
			width -= SetWindowSize(close_wnd_, cl_rect.right - w, 0, w, h).cx;
		}

		y_pos += SetWindowSize(rebar_, 0, y_pos, width, h).cy;
	}

	if (preview_.GetStyle() & WS_VISIBLE)
	{
		if (preview_.IsHorizontalOrientation())
		{
			y_pos += SetWindowSize(preview_, 0, y_pos, wnd_size.cx, previewPaneHeight_).cy;
			y_pos += SetWindowSize(separator_, 0, y_pos, wnd_size.cx, separator_.GetHeight()).cy;
		}
		else
		{
			int height= wnd_size.cy - y_pos;
			int width= previewPaneHeight_;

			x_pos += SetWindowSize(preview_, 0, y_pos, width, height).cx;
			x_pos += SetWindowSize(separator_, x_pos, y_pos, separator_.GetHeight(), height).cx;
		}

		preview_.Invalidate();
		separator_.Invalidate();
	}

	int right= cl_rect.right;

	if (tags_pane_.IsVisible())
	{
		int width= std::min<int>(tags_pane_.GetWidth(), wnd_size.cx - x_pos);
		if (width < 0)
			width = 0;
		wnd_size.cx -= SetWindowSize(tags_pane_, cl_rect.right - width, y_pos, width, wnd_size.cy - y_pos).cx;
		right -= width;
	}

	if (light_table_.IsVisible())
	{
		int width= std::min<int>(light_table_.GetWidth(), wnd_size.cx - x_pos);
		if (width < 0)
			width = 0;
		wnd_size.cx -= SetWindowSize(light_table_, right - width, y_pos, width, wnd_size.cy - y_pos).cx;
	}

	displays_.Resize(x_pos, y_pos, CSize(wnd_size.cx - x_pos, wnd_size.cy - y_pos));

	SetInfo();
}


// toggle preview bar visibility
//
void ViewerDlg::OnViewerBar()
{
	if (pImpl_->preview_.m_hWnd)
	{
		if (pImpl_->preview_bar_visible_)
		{
			pImpl_->preview_.ModifyStyle(WS_VISIBLE, 0);
			pImpl_->separator_.ModifyStyle(WS_VISIBLE, WS_DISABLED);
		}
		else
		{
			pImpl_->preview_.ModifyStyle(0, WS_VISIBLE);
			pImpl_->separator_.ModifyStyle(WS_DISABLED, WS_VISIBLE);
		}

		pImpl_->preview_bar_visible_ = !pImpl_->preview_bar_visible_;

		pImpl_->Resize(this);
	}
}

void ViewerDlg::OnUpdateViewerBar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	if (cmd_ui->m_pMenu == 0)
		cmd_ui->SetCheck(pImpl_->preview_bar_visible_ ? 1 : 0);
	try
	{
		CString text;
		text.LoadString(pImpl_->preview_bar_visible_ ? IDS_HIDE_PRVIEW_BAR : IDS_SHOW_PRVIEW_BAR);
		cmd_ui->SetText(text);
	}
	catch (...)
	{}
}


// light table storage place
static String GetLightTableStorageFilePath()
{
	TCHAR prog_path[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), prog_path, _MAX_PATH));
	Path dir= Path(prog_path).GetDir();
	//Path dir= GetApplicationDataFolder(_T("c:\\"));
	//if (!dir.CreateFolders())
	//	return _T("");

#ifdef _UNICODE
	dir.AppendDir(_T("LightTableContents.txt"), false);
#else
	dir.AppendDir(_T("LightTableContentsA.txt"), false);
#endif

	return dir;
}


int ViewerDlg::OnCreate(LPCREATESTRUCT create_struct)
{
	try
	{
		// correct coordinates if needed
		WINDOWPLACEMENT wp;
		if (GetWindowPlacement(&wp))
		{
			wp.showCmd = SW_HIDE;
			SetWindowPlacement(&wp);
		}

		Logger log;
		LOG_FILENAME(log); log << "ViewerDlg::OnCreate start\n";

		if (CFrameWnd::OnCreate(create_struct) == -1)
			return -1;

		LoadAccelTable(MAKEINTRESOURCE(IDR_VIEWER));

/*	CString title= _T("View ");
	title += folder;
	SetWindowText(title); */
//	ShowWindow(SW_SHOW);

		SetIcon(AfxGetApp()->LoadIcon(IDI_VIEWER), false);
		SetIcon(AfxGetApp()->LoadIcon(IDI_VIEWER), true);
		LOG_FILENAME(log); log << "icons set\n";

		pImpl_->Create(this, log);

		return 0;
	}
	CATCH_ALL

	return -1;
}


void ViewerDlg::Impl::SwitchPreviewBarLabels(LabelType labels)
{
	preview_bar_labels_ = labels;
	preview_.ReserveVerticalSpace(labels != LABEL_NONE ? GetDefaultLineHeight() + 2 : 0);
	preview_.Invalidate(false);
}


void ViewerDlg::Impl::Create(CWnd* wnd, Logger& log)
{
	wnd_ = wnd;

	VERIFY(toolbar_.Create(wnd, AFX_IDW_TOOLBAR));
	LOG_FILENAME(log); log << "toolbar created\n";

	VERIFY(status_wnd_.Create(wnd, this));
	LOG_FILENAME(log); log << "status created\n";

	VERIFY(close_wnd_.Create(wnd));
	LOG_FILENAME(log); log << "close wnd created\n";
	ShowCloseBar(false);

	preview_.Create(wnd);
	preview_.EnableToolTips(balloons_enabled_);
	preview_.KeepCurrentItemCentered(keep_current_img_centered_);

	preview_.SetCallBacks(
		boost::bind(&Impl::ItemClicked, this, _1, _2, _3),
		boost::bind(&Impl::DrawItem, this, _1, _2, _3, _4),
		boost::bind(&Impl::ItemToolTipText, this, _1, _2));

	preview_.SetOrientation(profileHorzPreviewBar_);

	SwitchPreviewBarLabels(preview_bar_labels_);

	separator_.Create(wnd, this);
	separator_.SetOrientation(preview_.IsHorizontalOrientation());

	displays_.Create(wnd, boost::bind(&Impl::ViewClicked, this, _1), boost::bind(&Impl::ViewPhotoDropped, this, _1, _2),
		boost::bind(&Impl::FindPhoto, this, _1), this);
	displays_.ShowPanes(profileMultiViewEnabled_ ? profileMultiViewCount_ : 1, view_layout_);
	SetActiveView();
	LOG_FILENAME(log); log << "display created\n";

	displays_.UseScrollBars(profileUseScrollBars_);

	if (!preview_bar_visible_)
	{
		preview_.ModifyStyle(WS_VISIBLE, 0);
		separator_.ModifyStyle(WS_VISIBLE, WS_DISABLED);
	}

	if (!light_table_.Create(wnd, 0, profileLightTableWidth_, boost::bind(&Impl::LoadPhoto, this, _1), boost::bind(&Impl::FindPhoto, this, _1)))
		throw String(_T("Light table creation failed"));

	light_table_.EnableToolTips(balloons_enabled_);

	light_table_.Restore(photos_, GetLightTableStorageFilePath().c_str());

	if (!tags_pane_.Create(wnd, 0, profile_tag_pane_width_))
		throw String(_T("Tag pane creation failed"));

//	if (!g_Settings.viewer_default_colors_)
	{
		SetColors(g_Settings.viewer_wnd_colors_.Colors());
		SetUIBrightness(g_Settings.viewer_ui_gamma_correction_);
	}
	//else
	//	ResetColors();

	ResetSettings();

	light_table_.Show(profileLightTableVisible_);
	tags_pane_.Show(profile_tag_pane_visible_);

	displays_.EnablePhotoDesc(profileShowPhotoDescription_);

	LOG_FILENAME(log); log << "ViewerDlg::OnCreate finished\n";

	LoadBitmaps(ui_gamma_correction_);

	status_wnd_.SetDlgCtrlID(Impl::INFOBAR);

	if (!rebar_.Create(wnd, &toolbar_, &status_wnd_,
		boost::bind(&Impl::Resize, this, wnd),
		boost::bind(&Impl::EraseBandBk, this, _1, _2, _3, _4)))
		throw String(_T("Rebar creation failed"));

	Synch(true);

	// set smooth scrolling speed (0 turns it off)
	preview_.SetSmoothScrollingSpeed(smooth_scroll_ ? g_Settings.smooth_scrolling_speed_ : 0);

	rebar_.RestoreLayout(static_cast<String>(AfxGetApp()->GetProfileString(REGISTRY_ENTRY_VIEWER, REG_REBAR_LAYOUT, _T(""))));

	ShowViewerToolbar(toolbar_visible_, infobar_visible_);

//	tool_rebar_wnd_.RestoreLayout(REGISTRY_ENTRY_VIEWER, REG_REBAR_LAYOUT);
//	LOG_FILENAME(log); log << "layout restored\n";
//
//	//	tool_rebar_wnd_.ShowBand(BAND_CLOSE, false);
////	LOG_FILENAME(log); log << "band shown\n";

	// store wnd style
	wnd_style_ = wnd->GetStyle();

	if (profileFullScreen_)
		FullScreen(wnd);
	else
		Resize(wnd);
}


void ViewerDlg::Impl::LoadBitmaps(double gamma)
{
	VERIFY(rebar_bgnd_.Load(IDB_VIEWER_REBAR_BACK));
	//VERIFY(infbar_.Load(IDB_INFOBAR));

	if (gamma != 1.0)
	{
		ApplyGammaInPlace(&rebar_bgnd_, gamma, -1, -1);
		ApplyGammaInPlace(&infbar_, gamma, -1, -1);
	}
}


void ViewerDlg::Impl::ItemClicked(size_t item, AnyPointer key, PreviewBandWnd::ClickAction action)
{
	int index= static_cast<int>(item);

	if (action == PreviewBandWnd::MouseBtnReleased)
	{
		bool load_next_hint= true;
		if (index == cur_image_index_ - 1)
			load_next_hint = false;	// load preceding, not next

		LoadPhoto(index, load_next_hint);
	}
	else if (action == PreviewBandWnd::StartDragDrop)
	{
		// drag image from the preview bar into the view pane
		//
		if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd()))
		{
			if (index >= 0 && index < photos_.size())
			{
				VectPhotoInfo selected;
				selected.push_back(photos_[index]);
				DragAndDrop(selected, frame);
			}
		}
	}
}


void ViewerDlg::Impl::DrawItem(CDC& dc, CRect rect, size_t item, AnyPointer key)
{
	if (item >= photos_.size())
		return;

	PhotoInfoPtr photo= photos_[item];

	::DrawPhoto(dc, rect, photo);

	if (!photo->exif_data_present_		  // missing EXIF indicator
		&& g_Settings.NoExifIndicator(photo->GetFileTypeIndex()))
		::DrawNoExifIndicator(dc, rect);

	if (show_tags_in_previewbar_)
		::DrawPhotoTags(dc, rect, photo->GetTags(), photo->GetRating(), tag_text_color_, tag_backgnd_color_);

	// if this item's photo is currently in one of the multiple view panes, indicate that
	if (displays_.GetVisibleCount() > 1)
	{
		static const COLORREF back_color[]=
		{
			RGB(66,155,44),		// green
			RGB(46,135,187),	// blue
			RGB(154,158,23),	// yellowish
			RGB(163,79,197)		// purple
		};

		for (size_t i= 0; i < displays_.GetVisibleCount(); ++i)
			if (ViewPane* view= displays_.GetPane(i))
			{
				if (view->GetCurrentPhoto() == photo)
				{
					::DrawPaneIndicator(dc, rect, RGB(255,255,255), back_color[i], i + 1);
					break;
				}
			}
	}

	if (light_table_.IsVisible() && light_table_.HasPhoto(photo))
		::DrawLightTableIndicator(dc, rect, RGB(236,246,123));

	if (preview_bar_labels_ != LABEL_NONE)
	{
		String label;

		switch (preview_bar_labels_)
		{
		case LABEL_NAME:
			label = photo->GetName();
			break;
		case LABEL_NAME_EXT:
			label = photo->GetNameAndExt();
			break;
		case LABEL_DATE:
			label = ::GetFormattedDateTime(photo->GetDateTime(), dc, rect.Width());
			break;
		}
		LOGFONT lf;
		HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		::GetObject(hfont, sizeof(lf), &lf);
		//lf.lfQuality = ANTIALIASED_QUALITY;
		lf.lfHeight += 1;
		_tcscpy(lf.lfFaceName, _T("Tahoma"));
		CFont _font;
		_font.CreateFontIndirect(&lf);
		dc.SelectObject(&_font);
		//dc.SelectStockObject(DEFAULT_GUI_FONT);
		int fh= GetLineHeight(dc);
		int space= 3;
		CRect text(rect.left, rect.bottom + space, rect.right, rect.bottom + space + fh);
		dc.SetTextColor(VIEWER_TEXT_COLOR);
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(label.c_str(), static_cast<int>(label.length()), text, DT_TOP | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
	}
}


String ViewerDlg::Impl::ItemToolTipText(size_t item, AnyPointer key)
{
	if (item < photos_.size())
	{
		PhotoInfo& inf= *photos_[item];
		return inf.ToolTipInfo(g_Settings.balloon_fields_);
	}

	return _T("");
}


void ViewerDlg::Impl::EraseBandBk(CDC& dc, int id, CRect band, bool in_line)
{
	if (!in_line && id == Impl::INFOBAR)
		infbar_.Draw(&dc, band, 0);
	else
		rebar_bgnd_.Draw(&dc, band, 0);
}


BOOL ViewerDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return false;

	cs.style |= WS_CLIPCHILDREN;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	CRect rect= pImpl_->windowPosition_.GetLocation(false);

	cs.x = rect.left;
	cs.y = rect.top;
	cs.cx = rect.Width();
	cs.cy = rect.Height();

	return true;
}


void ViewerDlg::Impl::ShowCloseBar(bool show)
{
	if (show)
	{
		close_wnd_.EnableWindow();
		close_wnd_.ShowWindow(SW_SHOWNA);
	}
	else
	{
		close_wnd_.ShowWindow(SW_HIDE);
		close_wnd_.EnableWindow(false);
	}
}


// create modeless viewer dialog window
//
bool ViewerDlg::Create(const TCHAR* folder)
{
	Logger log;
	LOG_FILENAME(log); log << "ViewerDlg::Create start\n";

	if (pImpl_->wndClass_.IsEmpty())
		pImpl_->wndClass_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	if (!CFrameWnd::Create(pImpl_->wndClass_, _T("Image Viewer")))
		return false;

	ShowWindow(/*full_screen_ ? SW_SHOWMAXIMIZED :*/ SW_SHOW);

	LOG_FILENAME(log); log << "ViewerDlg::Create finished\n";

	return true;
}


void ViewerDlg::OnCmdClose()
{
	if (pImpl_->slide_show_timer_)	// slide show?
		OnStopSlideShow();
	else
		DestroyWindow();
}


void ViewerDlg::OnFullScreen()
{
	// when changing mode exit slide show
	OnStopSlideShow();

	if (pImpl_->full_screen_)
		pImpl_->WndMode(this);
	else
		pImpl_->FullScreen(this);
	pImpl_->SetInfo();
}


static void InformTaskBar(CWnd* wnd, bool fullScreen)	// help taskbar recognize full-screen window
{
	_COM_SMARTPTR_TYPEDEF(ITaskbarList2, __uuidof(ITaskbarList2));
	ITaskbarList2Ptr task_bar;

	if (SUCCEEDED(task_bar.CreateInstance(CLSID_TaskbarList)))
		task_bar->MarkFullscreenWindow(wnd->m_hWnd, fullScreen);
}


const int CAPTION_STYLES= WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;


void ViewerDlg::Impl::WndMode(CWnd* wnd)
{
	if (toolbar_visible_)
		ShowCloseBar(false);

	wnd->ModifyStyle(WS_MAXIMIZE, (wnd_style_ & ~WS_MAXIMIZE) | CAPTION_STYLES);

	const CRect& r= wnd_rectangle_;
	wnd->SetWindowPos(0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
	full_screen_ = false;

	InformTaskBar(wnd, full_screen_);
}


void ViewerDlg::Impl::FullScreen(CWnd* wnd)
{
	CRect window_rect(0,0,0,0);
	wnd->GetWindowRect(window_rect);

	// full screen rect (of the monitor this window is displayed in)
	CRect screen_rect= ::GetFullScreenRect(window_rect);

	wnd_style_ = wnd->GetStyle();
	wnd->GetWindowRect(wnd_rectangle_);

	// remove caption & frame //and maximize?
	wnd->ModifyStyle(CAPTION_STYLES, 0); //WS_MAXIMIZE);

	CRect rect= screen_rect;
	::AdjustWindowRectEx(rect, wnd->GetStyle(), false, wnd->GetExStyle());

	if (toolbar_visible_)
		ShowCloseBar(true);

	wnd->SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_FRAMECHANGED);
	full_screen_ = true;
	if (toolbar_visible_)
	{
//		tool_rebar_wnd_.ShowBand(BAND_CLOSE);
//		tool_rebar_wnd_.Resize();
	}

	InformTaskBar(wnd, full_screen_);

//CRect r(0,0,0,0);
//wnd->GetWindowRect(r);
//
//wchar_t buf[400]; wsprintf(buf, L"%d, %d, %d, %d\n%d, %d, %d, %d\n%d, %d, %d, %d\n%d, %d, %d, %d", window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, screen_rect.left, screen_rect.top, screen_rect.right, screen_rect.bottom, rect.left, rect.top, rect.right, rect.bottom, r.left, r.top, r.right, r.bottom); ::MessageBox(wnd->m_hWnd, buf, L"info", MB_OK);
}


void ViewerDlg::OnUpdateFullScreen(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->full_screen_ ? 1 : 0);
}


BOOL ViewerDlg::OnEraseBkgnd(CDC* dc)
{
	// this window shouldn't be visible, it's completely covered
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	dc->FillSolidRect(rect, RGB(0,0,0));

	return true;
}


void ViewerDlg::RecalcLayout(BOOL /*notify= TRUE*/)
{
	pImpl_->Resize(this);
}

///////////////////////////////////////////////////////////////////////////////
// Navigation
//

void ViewerDlg::OnPhotoFirst()
{
	if (!pImpl_->photos_.empty() && pImpl_->cur_image_index_ != 0)
		pImpl_->LoadPhoto(pImpl_->cur_image_index_ = 0);
}

void ViewerDlg::OnUpdatePhotoFirst(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(!pImpl_->photos_.empty() && pImpl_->cur_image_index_ > 0);
}

void ViewerDlg::OnPhotoLast()
{
	if (!pImpl_->photos_.empty() && pImpl_->cur_image_index_ != pImpl_->photos_.size() - 1)
		pImpl_->LoadPhoto(pImpl_->cur_image_index_ = static_cast<int>(pImpl_->photos_.size()) - 1, false);
}

void ViewerDlg::OnUpdatePhotoLast(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(!pImpl_->photos_.empty() && pImpl_->cur_image_index_ < pImpl_->photos_.size() - 1);
}

void ViewerDlg::OnPhotoNext()
{
	pImpl_->PhotoNext();
}

void ViewerDlg::Impl::PhotoNext()
{
	if (!photos_.empty() && cur_image_index_ < photos_.size() - 1)
		LoadPhoto(++cur_image_index_);
}

void ViewerDlg::OnUpdatePhotoNext(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(!pImpl_->photos_.empty() && pImpl_->cur_image_index_ < pImpl_->photos_.size() - 1);
}

void ViewerDlg::OnPhotoPrev()
{
	pImpl_->PhotoPrev();
}

void ViewerDlg::Impl::PhotoPrev()
{
	if (!photos_.empty() && cur_image_index_ > 0)
		LoadPhoto(--cur_image_index_, false);
}

void ViewerDlg::OnUpdatePhotoPrev(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(!pImpl_->photos_.empty() && pImpl_->cur_image_index_ > 0);
}

// list of available photos (popup menu)
//
void ViewerDlg::OnPhotoList()
{
	//pImpl_->PhotoList(this, true);
	g_Settings.close_app_when_viewer_exits_ = false;
	CFrameWnd::OnClose();
}


/*void ViewerDlg::Impl::PhotoList(CWnd* wnd, bool press_btn)
{
	if (!toolbar_visible_)
		return;

	if (press_btn)
		toolbar_.PressButton(ID_PHOTO_LIST);

	CPhotoList menu(photos_);

	CRect rect;
	toolbar_.GetRect(ID_PHOTO_LIST, rect);
	CPoint pos(rect.left, rect.bottom);
	toolbar_.ClientToScreen(&pos);

	CursorVisible stay(displays_);
	int cmd= menu.TrackPopupMenu(wnd, pos, cur_image_index_);

	if (press_btn)
		toolbar_.PressButton(ID_PHOTO_LIST, false);

	if (cmd >= 0)
		LoadPhoto(cmd);
}
*/

void ViewerDlg::OnUpdatePhotoList(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->toolbar_visible_);
}


void ViewerDlg::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	switch (info_tip->iItem)
	{
	//case ID_PHOTO_LIST:
	//	pImpl_->PhotoList(this, false);
	//	break;

	case ID_VIEWER_OPTIONS:
		pImpl_->ViewerOptions(this, false);
		break;

	case ID_START_SLIDE_SHOW:
		pImpl_->SlideShowOptions(this);
		break;

	case ID_LIGHT_TABLE_OPTIONS:
		pImpl_->light_table_.OperationsPopup();
		break;

	case ID_LIGHT_TABLE_TAGS:
		pImpl_->light_table_.TagsPopup();
		break;

	case ID_COMPARE_MULTIPLE:
		pImpl_->MultiView(this, true);
		break;
	}
	*result = TBDDRET_DEFAULT;
}


///////////////////////////////////////////////////////////////////////////////
// Loading photos
//

void ViewerDlg::LoadPhoto(ConstPhotoInfoPtr photo)
{
	// go to the requested photo; do not smooth scroll, but jump there
	pImpl_->LoadPhoto(photo, false);
}

void ViewerDlg::Impl::LoadPhoto(ConstPhotoInfoPtr photo)
{
	LoadPhoto(photo, true);
}

void ViewerDlg::Impl::LoadPhoto(ConstPhotoInfoPtr photo, bool scroll_to_it)
{
	VectPhotoInfo::iterator it= find(photos_.begin(), photos_.end(), photo);
	if (it != photos_.end())
		LoadPhoto(static_cast<int>(distance(photos_.begin(), it)), true, scroll_to_it);
	else
	{ ASSERT(false); }
}

// if load_hint is true, next photo will be loaded in the background;
// if it is false, preceding one will be loaded in the background
void ViewerDlg::Impl::LoadPhoto(int item_index, bool load_hint/*= true*/)
{
	LoadPhoto(item_index, load_hint, true);
}

void ViewerDlg::Impl::LoadPhoto(int item_index, bool load_hint, bool scroll_to_it)
{
	showMagnifierLens_ = false;
	MagnifierWnd::Close();

	if (slide_show_timer_ != 0)	// if slide show is active reload delay counter
		slideShowDelay_ = profileSlideShowDelay_ * 4;
TRACE(L"slide show delay reloaded ============-=-=-=-=-=-=-=-=-=-=-\n");

	CursorVisible stay(displays_);
	if (item_index < 0 || item_index >= photos_.size())
	{
		ASSERT(false);
		return;
	}

	cur_image_index_ = item_index;

	int next_photo_idx= load_hint ? item_index + 1 : item_index - 1;
	if (next_photo_idx == photos_.size())
		next_photo_idx = 0;	// wrap around

	ConstPhotoInfoPtr next_photo= 0;
	if (next_photo_idx >= 0 && next_photo_idx < photos_.size())
		next_photo = photos_[next_photo_idx];

	preview_.SelectionVisible(cur_image_index_, scroll_to_it);
	preview_.RedrawSelection();

	light_table_.SelectionVisible(photos_[cur_image_index_], false);

	current_image_ = photos_[item_index];
	rotation_feasible_ = current_image_->IsRotationFeasible() && current_image_->IsFileEditable();
	LoadPhoto(*current_image_, next_photo, false);

	if (displays_.GetVisibleCount() > 1)
		preview_.Invalidate();	// redraw pane indicators

//	if (tags_wnd_.IsValid())
		RefreshTags();

	host_->CurrentChanged(current_image_);
}


void ViewerDlg::Impl::LoadPhoto(const PhotoInfo& photo, ConstPhotoInfoPtr next_photo, bool force_reload)
{
	//current_orientation_ = 0;
	originalOrientationPreserved_ = false;
//	orientation_ = photo.GetOrientation();
	bool auto_rotate= !photo.OrientationAltered();

	UINT flags= 0;
	if (auto_rotate)
		flags |= ViewPane::AUTO_ROTATE;
	if (force_reload)
		flags |= ViewPane::FORCE_RELOADING;

	switch (g_Settings.image_blending_)
	{
	case 0:		// always
		flags |= ViewPane::ALPHA_BLENDING;
		break;
	case 1:		// slide show only
		if (slide_show_timer_)
			flags |= ViewPane::ALPHA_BLENDING;
		break;
	case 2:		// never
		break;
	default:
		ASSERT(false);
		break;
	}

	display_->LoadPhoto(photo, g_Settings.preload_photos_ ? next_photo : 0, flags);

	oStringstream ost;
	ost << Path(photo.GetDisplayPath()).GetFileName() << _T("  (") << cur_image_index_ + 1 << _T("/") << photos_.size() << _T(")");

	photo_name_ = ost.str();

//	preview_.SelectionVisible();
//	preview_.RedrawSelection();
	SetInfo();
}


void ViewerDlg::Impl::LoadPhotoIntoView(ViewPane* view, ConstPhotoInfoPtr photo)
{
	if (view == 0)
		return;

	if (photo == 0)
	{
		view->Reset();
	}
	else
	{
		bool autoRotate= !photo->OrientationAltered();

		UINT flags= 0;
		if (autoRotate)
			flags |= ViewPane::AUTO_ROTATE;

		view->LoadPhoto(*photo, 0, flags);
	}

	if (Path* path= displays_.GetViewPhotoPath(view))
	{
		if (photo)
			*path = photo->GetOriginalPath();
		else
			path->clear();
	}

	SetViewCaption(view, photo);
}


BOOL ViewerDlg::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
//	static DWORD last_time= 0;
//	DWORD tick_count= ::GetTickCount();
//	long delay= abs(static_cast<long>(tick_count - last_time));
//	last_time = tick_count;

	HWND wnd= ::WindowFromPoint(pt);
TRACE(L"wheel %d\n", int(delta));

	if (ViewPane* view= pImpl_->displays_.FindViewPane(wnd))
	{
//TODO as an option
		// Logitech driver sometimes sends two mouse wheel notifications instead of one
//		if (delay < 108)
//			return true;

		if (flags & MK_CONTROL)
		{
			MagnifierWnd::Close();
			double zoom= view->GetLogicalZoom() * 100.0;

			// zoom
			if (delta < 0)
				zoom *= 1.2f;
			else if (delta > 0)
				zoom *= 1.0f / 1.2f;

			int new_zoom= static_cast<int>(zoom);
			if (new_zoom < g_zoom.front())
				new_zoom = g_zoom.front();
			else if (new_zoom > g_zoom.back())
				new_zoom = g_zoom.back();

			pImpl_->displays_.for_all(&ViewPane::SetLogicalZoom, new_zoom / 100.0, true);
			pImpl_->SetInfo();//???view);
		}
		else
		{
			if (delta < 0)
				OnPhotoNext();
			else if (delta > 0)
				OnPhotoPrev();
		}
	}
	else if (wnd == pImpl_->preview_.m_hWnd)
	{
		if (delta < 0)
			pImpl_->preview_.ScrollRight(2);//-delta / 120);
		else if (delta > 0)
			pImpl_->preview_.ScrollLeft(2);//delta / 120);
	}
	else if (wnd == pImpl_->light_table_.m_hWnd || ::GetParent(wnd) == pImpl_->light_table_.m_hWnd)
	{
		if (delta < 0)
			pImpl_->light_table_.ScrollRight(0);
		else if (delta > 0)
			pImpl_->light_table_.ScrollLeft(0);
	}

	return true;
}


void ViewerDlg::OnZoomIn()
{
	MagnifierWnd::Close();

	pImpl_->displays_.for_all(&ViewPane::ZoomIn, const_cast<const uint16*>(&g_zoom.front()), g_zoom.size());

	pImpl_->SetInfo();
}

void ViewerDlg::OnUpdateZoomIn(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->FirstPane().GetLogicalZoom() < g_zoom.back() / 100.0);
}


void ViewerDlg::OnZoomOut()
{
	MagnifierWnd::Close();

	pImpl_->displays_.for_all(&ViewPane::ZoomOut, const_cast<const uint16*>(&g_zoom.front()), g_zoom.size());

	pImpl_->SetInfo();
}

void ViewerDlg::OnUpdateZoomOut(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->FirstPane().GetLogicalZoom() > g_zoom[0] / 100.0);
}


void ViewerDlg::Impl::ZoomToFit()
{
	MagnifierWnd::Close();

	displays_.for_all(&ViewPane::SetLogicalZoom, 0.0, g_Settings.allow_magnifying_above100_);

	SetInfo();
}


void ViewerDlg::OnZoomFit()
{
	pImpl_->ZoomToFit();
}


void ViewerDlg::Impl::Zoom100(CPoint offset)
{
	MagnifierWnd::Close();

	displays_.for_all(&ViewPane::SetLogicalZoomAndOffset, 1.0, true, offset);

	SetInfo();
}


void ViewerDlg::OnZoom100()
{
	pImpl_->Zoom100();
}


void ViewerDlg::Impl::SetInfo()
{
	oStringstream ost;
	const TCHAR* SEP= _T("\t");

	if (status_wnd_.show_zoom_)
	{
		if (status_wnd_.show_field_names_)
			ost << DrawFields::LABEL << RString(IDS_VIEWER_ZOOM).CStr() << _T(" ");
		ost << DrawFields::VALUE;

		double zoom= display_->GetLogicalZoom() * 100.0;

		int reduction= display_->GetReductionFactor();
		if (reduction == 0)
		{
			// zoom factor not yet ready
			if (timer_id_ == 0)
				timer_id_ = timer_wnd_->SetTimer(TIMER_SET_INFO, 100, NULL);
			return;
		}

		if (display_->IsZoomToFit())
		{
			if (zoom > 0.0)
			{
				if (zoom > 100.0 && !g_Settings.allow_magnifying_above100_)
					zoom = 100.0;

				ost << RString(IDS_VIEWER_ZOOM_FIT).CStr() << _T(" (") << std::fixed << std::setprecision(1) << zoom << _T("%)");
			}
			else
			{	// zoom factor not yet ready
				if (timer_id_ == 0)
					timer_id_ = timer_wnd_->SetTimer(TIMER_SET_INFO, 100, NULL);
				ost << RString(IDS_VIEWER_ZOOM_FIT).CStr() << _T(" (--%)");
			}
		}
		else
			ost << std::fixed << std::setprecision(1) << zoom << _T("%");

		ost << SEP;
	}

	if (status_wnd_.show_name_)
	{
		if (status_wnd_.show_field_names_)
			ost << DrawFields::LABEL << RString(IDS_VIEWER_PHOTO).CStr();

		ost << DrawFields::VALUE << photo_name_ << SEP;
	}

	if (current_image_)
		ost << columns_.GetStatusText(status_wnd_.fields_, *current_image_, status_wnd_.show_field_names_);

	status_wnd_.SetWindowText(ost.str().c_str());
	status_wnd_.Invalidate();

	SetViewCaption(display_, current_image_);

/*
	if (ViewCaption* caption= displays_.GetCaption(display_))	// current pane's caption
	{ // caption text (in multi view layout)
		vector<uint16> fields(1, COL_PHOTO_NAME);
		String str= current_image_ ? columns_.GetStatusText(fields, *current_image_, true) : _T("");
		oStringstream ost;
		ost << DrawFields::VALUE << displays_.GetDisplayIndex(display_) + 1 << "  " << str;
		caption->SetWindowText(ost.str().c_str());
		caption->Invalidate();
	} */
}


void ViewerDlg::CustomColumnsRefresh()
{
	pImpl_->SetInfo();
}


void ViewerDlg::Impl::SetViewCaption(ViewPane* view, ConstPhotoInfoPtr photo)
{
	if (ViewCaption* caption= displays_.GetCaption(view))	// pane's caption
	{ // caption text (visible in multi view layout)
		std::vector<uint16> fields(1, COL_PHOTO_NAME);
		String str= photo ? columns_.GetStatusText(fields, *photo, true) : _T("");
		oStringstream ost;
		ost << DrawFields::VALUE << displays_.GetDisplayIndex(view) + 1 << "  " << str;
		caption->SetWindowText(ost.str().c_str());
		caption->Invalidate();
	}
}


void ViewerDlg::OnPhotoDesc()
{
	if (pImpl_->current_image_ == 0)
		return;

	CursorVisible stay(pImpl_->displays_);

	try
	{
		VectPhotoInfo selected;
		selected.push_back(&*pImpl_->current_image_);

		EditFileInfo file_info(pImpl_->photos_, selected, this, boost::bind(&Impl::PhotoSelected, pImpl_.get(), _1));

//		CTaskEditDescription desc(this, *pImpl_->current_image_, pImpl_->photos_, selected,
//			boost::bind(&Impl::PhotoSelected, pImpl_.get(), _1));

		if (file_info.DoModal())
		{
			//if (!pImpl_->display_wnd_.IsPhotoDescDisplayed())
			//{ }//EnablePhotoDesc(true);
			if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd()))
				frame->PhotoDescriptionChanged(pImpl_->current_image_->photo_desc_);

			pImpl_->display_->ResetDescription(pImpl_->current_image_->photo_desc_);
			pImpl_->display_->Invalidate();
		}
	}
	CATCH_ALL
}


void ViewerDlg::Impl::PhotoSelected(PhotoInfo& photo)	// new photo edited in description dlg
{
	LoadPhoto(&photo);
}


void ViewerDlg::Clean()
{
	pImpl_->Clean();
}

void ViewerDlg::Impl::Clean()
{
	cur_image_index_ = 0;			// current photo index
	current_image_ = 0;				// current photo
	displays_.for_all(&ViewPane::Reset);
	displays_.ResetCaptions();
	preview_.RemoveAllItems();
	light_table_.RemoveAll();
	photo_name_.erase();
	SetInfo();
}


size_t ViewerDlg::Impl::FindPhotoIndex(PhotoInfoPtr photo)
{
	VectPhotoInfo::iterator it= find(photos_.begin(), photos_.end(), photo);
	return it != photos_.end() ? distance(photos_.begin(), it) : 0;
}


void ViewerDlg::Synch()
{
	pImpl_->Synch(false);

	if (pImpl_->current_image_ == 0 && !pImpl_->photos_.empty())
		pImpl_->LoadPhoto(pImpl_->cur_image_index_ = 0);
	else if (pImpl_->current_image_ != 0)
		pImpl_->cur_image_index_ = static_cast<int>(pImpl_->FindPhotoIndex(pImpl_->current_image_));
}


void ViewerDlg::Impl::Synch(bool incremental)		// inform viewer about change in the list of photos
{
	// synch preview list (this is an append operation if 'incremental' is true)

	if (!incremental)
		preview_.RemoveAllItems();

	preview_.ReserveItems(photos_.size());

	size_t start= preview_.GetItemCount();

	for (size_t i= 0; i < photos_.size(); ++i)
	{
		PhotoInfoPtr p= photos_[i];

		CSize size= p->GetSize();
		//CSize size(p->width_, p->height_);
		//if (size.cx > size.cy && !p->HorizontalOrientation())
		//	swap(size.cx, size.cy);

		if (size.cx == 0 || size.cy == 0)
		{
			ASSERT(false); // decode image size!
			size.cx = 160; size.cy = 120;
		}

		if (i < start)
			preview_.ModifyItem(i, size, p);
		else
			preview_.AddItem(size, p);
	}

//	preview_.Invalidate();
	preview_.SetCurrent(current_image_);
	preview_.ResetScrollBar(false);

	light_table_.Restore(photos_, GetLightTableStorageFilePath().c_str());
}


void ViewerDlg::OnDestroy()
{
	pImpl_->Destroy(this);
	CFrameWnd::OnDestroy();
}


void ViewerDlg::OnNcDestroy()
{
	CFrameWnd::OnNcDestroy();

	if (CWnd* frame= AfxGetMainWnd())
	{
		DWORD style= frame->GetStyle();

		if ((style & WS_VISIBLE) == 0)
		{
			// main wnd is hidden

			if (g_Settings.close_app_when_viewer_exits_)
				frame->DestroyWindow();
			else
			{
				frame->EnableWindow();
				frame->ShowWindow(SW_SHOW);

				// enable saving setting...
				if (BrowserFrame* browser= dynamic_cast<BrowserFrame*>(frame))
					browser->EnableSavingSettings();

				//TODO:
				// scan sub folders was disabled; restore it now?

				// ...

			}
		}
	}
}


void ViewerDlg::Impl::Destroy(CWnd* wnd)
{
	CWinApp* app= AfxGetApp();

	// in full screen mode store wnd_rectangle_ rather than current size
	if (full_screen_)
		windowPosition_.StoreState(*wnd, wnd_rectangle_);
	else
		windowPosition_.StoreState(*wnd);	// store current wnd position

	// store view mode
	profileFullScreen_ = full_screen_;

	if (light_table_.m_hWnd)
	{
		profileLightTableWidth_ = light_table_.GetWidth();
		profileLightTableVisible_ = light_table_.IsVisible();

		light_table_.Store(GetLightTableStorageFilePath().c_str());
	}

	if (tags_pane_)
	{
		profile_tag_pane_width_ = tags_pane_.GetWidth();
		profile_tag_pane_visible_ = tags_pane_.IsVisible();
	}

	// status bar fields
	WriteProfileVector(REGISTRY_ENTRY_VIEWER, REG_INFO_BAND_FIELDS, status_wnd_.fields_);

	// store aspect ratio correction switch
//	app->WriteProfileInt(REGISTRY_ENTRY_VIEWER, REG_CORRECT_RATIO, display_wnd_.IsAspectRatioCorrected() ? 1 : 0);

	// store gamma switch
	//profileGammaCorrection_ = display_wnd_.IsGammaEnabled();

	// store tb state only if it's visible or else it'll remember closed bands
	if (toolbar_visible_)
	{
		String layout= rebar_.StoreLayout();
		if (!layout.empty())
			app->WriteProfileString(REGISTRY_ENTRY_VIEWER, REG_REBAR_LAYOUT, layout.c_str());
	}
	//if (toolbar_visible_)
	//	tool_rebar_wnd_.StoreLayout(REGISTRY_ENTRY_VIEWER, REG_REBAR_LAYOUT);

	profileShowPhotoDescription_ = FirstPane().IsPhotoDescDisplayed();
	profileUseScrollBars_ = FirstPane().IsUsingScrollBars();
	profileHorzPreviewBar_ = preview_.IsHorizontalOrientation();
	profileTagsInPreviewBar_ = show_tags_in_previewbar_;
	//profileMultiViewCount_ = displays_.GetVisibleCount();
	profileMultiViewEnabled_ = displays_.GetVisibleCount() > 1;
	profileShowFieldNames_ = status_wnd_.show_field_names_;

	if (timer_id_)
		timer_wnd_->KillTimer(timer_id_);
	if (slide_show_timer_)
		wnd->KillTimer(slide_show_timer_);
}


void ViewerDlg::OnSysCommand(UINT id, LPARAM lParam)
{
	switch (id & 0xfff0)
	{
	case SC_MAXIMIZE:
		pImpl_->FullScreen(this);
		break;

	default:
		CFrameWnd::OnSysCommand(id, lParam);
	}
}


void ViewerDlg::OnRestore()
{
	if (pImpl_->full_screen_)
		pImpl_->WndMode(this);
}


void ViewerDlg::OnTimer(UINT_PTR event)
{
	CWnd::OnTimer(event);
	pImpl_->OnTimer(this, event);
}


void ViewerDlg::Impl::OnTimer(CWnd* wnd, UINT_PTR event)
{
	if (event == TIMER_SLIDE_SHOW)
	{
		if (--slideShowDelay_ > 0)
		{
TRACE(L"slide show delay: %d\n", slideShowDelay_);
			return;
		}

//		slideShowDelay_ = profileSlideShowDelay_;	// delay in seconds

		if (photos_.empty())
		{
			StopSlideShow(wnd);
			return;
		}

		// load next photo, start from beginning if we are at the end

		if (cur_image_index_ < photos_.size() - 1)
			++cur_image_index_;
		else
		{
			if (profileSlideShowRepeat_)
				cur_image_index_ = 0;
			else
			{
				StopSlideShow(wnd);
				return;
			}
		}

		if (!photos_.empty())
			LoadPhoto(cur_image_index_);
	}
	else if (event == timer_id_)
	{
		timer_wnd_->KillTimer(timer_id_);
		timer_id_ = 0;
		SetInfo();
	}
}


void ViewerDlg::Impl::SlideShowOptions(CWnd* wnd)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_VIEWER_SLIDE_SHOW))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		popup->SetDefaultItem(ID_START_SLIDE_SHOW);

		CRect rect;
		toolbar_.GetRect(ID_START_SLIDE_SHOW, rect);
		toolbar_.ClientToScreen(rect);

		CursorVisible stay(displays_);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.bottom, wnd);
	}
}


void ViewerDlg::OnViewerOptions()
{
	pImpl_->ViewerOptions(this, true);
}


void ViewerDlg::Impl::ViewerOptions(CWnd* wnd, bool press_btn)
{
	if (!toolbar_visible_)
		return;

	if (press_btn)
		toolbar_.PressButton(ID_VIEWER_OPTIONS);

	CMenu menu;
	if (!menu.LoadMenu(IDR_VIEWER_OPTIONS))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		CRect rect;
		toolbar_.GetRect(ID_VIEWER_OPTIONS, rect);
		toolbar_.ClientToScreen(rect);

		CursorVisible stay(displays_);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.bottom, wnd);
	}

	if (press_btn)
		toolbar_.PressButton(ID_VIEWER_OPTIONS, false);
}


void ViewerDlg::OnUpdateViewerOptions(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->toolbar_visible_);
}

/*
void ViewerDlg::OnGamma()
{
	transform_.Reset(g_Settings.default_photo_->profile_, g_Settings.viewer_->profile_, g_Settings.viewer_->rendering_);
	display_wnd_.EnableGamma(!display_wnd_.IsGammaEnabled());
	//LoadPhoto(cur_image_index_);	// reload photo
	light_table_.SetGamma(display_wnd_.IsGammaEnabled() ? transform_ : ICMTransform());
}

void ViewerDlg::OnUpdateGamma(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(display_wnd_.IsGammaEnabled() ? 1 : 0);
}*/


//void ViewerDlg::OnAspectRatio()
//{
//	display_wnd_.EnableAspectRatioCorrection(!display_wnd_.IsAspectRatioCorrected());
//}
/*
void ViewerDlg::OnUpdateAspectRatio(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(display_wnd_.IsAspectRatioCorrected() ? 1 : 0);
}*/


void ViewerDlg::OnViewerBalloons()
{
	pImpl_->balloons_enabled_ = !pImpl_->balloons_enabled_;
	pImpl_->preview_.EnableToolTips(pImpl_->balloons_enabled_);
	pImpl_->light_table_.EnableToolTips(pImpl_->balloons_enabled_);
}

void ViewerDlg::OnUpdateViewerBalloons(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->balloons_enabled_ ? 1 : 0);
}


void ViewerDlg::OnViewerToolbar()
{
	pImpl_->ShowViewerToolbar(!pImpl_->toolbar_visible_, pImpl_->infobar_visible_);
}

void ViewerDlg::OnUpdateViewerInfobar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->infobar_visible_ ? 1 : 0);
}

void ViewerDlg::OnViewerInfobar()
{
//TODO:	pImpl_->ShowViewerToolbar(pImpl_->toolbar_visible_, !pImpl_->infobar_visible_);
}

void ViewerDlg::Impl::ShowViewerToolbar(bool showTb, bool showInfo)
{
	toolbar_visible_ = showTb;
	infobar_visible_ = showInfo;

	rebar_.ShowBand(0, toolbar_visible_);
//TODO:	rebar_.ShowBand(1, infobar_visible_);
	rebar_.ShowBand(1, toolbar_visible_);

	ShowCloseBar(full_screen_ && toolbar_visible_);
	Resize(wnd_);
}


void ViewerDlg::OnUpdateViewerToolbar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->toolbar_visible_ ? 1 : 0);
}


static bool DeleteImage(const String& path, CWnd* parent_wnd)
{
	if (path.empty())
		return false;

	CWaitCursor wait;

	String src;
	src += path + _T('\0');
	src += _T('\0');

	SHFILEOPSTRUCT op;
	op.hwnd = *parent_wnd;
	op.wFunc = FO_DELETE;
	op.pFrom = src.data();
	op.pTo = 0;
	op.fFlags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
	op.fAnyOperationsAborted = false;
	op.hNameMappings = 0;
	op.lpszProgressTitle = 0;
	bool deleted= ::SHFileOperation(&op) == 0 && !op.fAnyOperationsAborted;
	return deleted;
}


void ViewerDlg::OnDelete()
{
	pImpl_->Delete(this);
}


void ViewerDlg::Impl::Delete(CWnd* wnd)
{
	size_t count= photos_.size();

	if (count == 0 || cur_image_index_ >= count)
		return;

	CursorVisible stay(displays_);

	PhotoInfoPtr photo= photos_[cur_image_index_];

	if (!photo->CanDelete())
		return;

	// try to delete file
	bool ok= DeleteImage(photo->GetPhysicalPath(), wnd);

	if (!ok)
		return;

	// file deleted: remove it from PhotoCtrl view
	host_->PhotoDeleted(photo);

	if (count == 1)
	{
		stay.Disable();
		wnd->DestroyWindow();		// no more photos left to display--close viewer
		return;
	}

	if (cur_image_index_ == count - 1)
		cur_image_index_--;

	//TODO: remove an item...
	Synch(false);

	// load next photo (same cur_image_index_ index now points to the next photo)
	LoadPhoto(cur_image_index_);

	displays_.ResetRemainingWithPhoto(display_, photo);
}


void ViewerDlg::OnUpdateDelete(CCmdUI* cmd_ui)
{
	bool enable= false;
	if (static_cast<size_t>(pImpl_->cur_image_index_) < pImpl_->photos_.size())
		enable = pImpl_->photos_[pImpl_->cur_image_index_]->CanDelete();

	cmd_ui->Enable(enable);
}


void ViewerDlg::OnUpdateZoom100(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	ViewPane& first= pImpl_->FirstPane();
	cmd_ui->SetCheck(!first.IsZoomToFit() && first.GetLogicalZoom() == 1.0 ? 1 : 0);
}

void ViewerDlg::OnUpdateZoomFit(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->FirstPane().IsZoomToFit() ? 1 : 0);
}


void ViewerDlg::OnStartSlideShow()
{
	pImpl_->StartSlideShow(this);
}

void ViewerDlg::Impl::StartSlideShow(CWnd* wnd)
{
	if (profileSlideShowHideTb_)
	{
		CRect rect(0, 0, 0, 0);

		if (preview_.GetStyle() & WS_VISIBLE)
		{
			WINDOWPLACEMENT wp;
			preview_.GetWindowPlacement(&wp);
			rect |= wp.rcNormalPosition;
			preview_.ModifyStyle(WS_VISIBLE, 0);
			separator_.ModifyStyle(WS_VISIBLE, WS_DISABLED);
		}

		if (rebar_.GetStyle() & WS_VISIBLE)
		{
			WINDOWPLACEMENT wp;
			rebar_.GetWindowPlacement(&wp);
			rect |= wp.rcNormalPosition;
			rebar_.ModifyStyle(WS_VISIBLE, 0);

			if (full_screen_)
				close_wnd_.ModifyStyle(WS_VISIBLE, 0);
		}

		if (!rect.IsRectEmpty())
		{
			//TODO: scrolling is failing sometimes...
			CSmoothScroll scroll(wnd);
			scroll.Hide(rect, FirstPane().GetBackgndColor());
		}

		Resize(wnd);
	}

	slideShowDelay_ = profileSlideShowDelay_ * 4;	// delay in seconds times 4

	if (slideShowDelay_ != 0)	// start a timer (once a 1/4 second)
		slide_show_timer_ = timer_wnd_->SetTimer(TIMER_SLIDE_SHOW, 250, 0);

	// timers granularity increased to quarter of a second: when user presses page up or down
	// delay gets reset;
	// with 1 s timer resolution this reset is not always effective, with 0.25 s it's fine
}

void ViewerDlg::OnUpdateStartSlideShow(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void ViewerDlg::OnStopSlideShow()
{
	pImpl_->StopSlideShow(this);
}

void ViewerDlg::Impl::StopSlideShow(CWnd* wnd)
{
	if (slide_show_timer_)
	{
		timer_wnd_->KillTimer(slide_show_timer_);
		slide_show_timer_ = 0;

		// restore toolbar and preview pane
		if (preview_bar_visible_)
		{
			preview_.ModifyStyle(0, WS_VISIBLE);
			separator_.ModifyStyle(WS_DISABLED, WS_VISIBLE);
		}
		if (toolbar_visible_)
		{
			rebar_.ModifyStyle(0, WS_VISIBLE);

			if (full_screen_)
				close_wnd_.ModifyStyle(0, WS_VISIBLE);
		}

		Resize(wnd);
	}
}

void ViewerDlg::OnUpdateStopSlideShow(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->slide_show_timer_ != 0);
}


// notification from main app: some photos were removed/added
void ViewerDlg::PhotosListChange()
{
	if (pImpl_->photos_.empty())
	{
		// no more photos left--exit
		DestroyWindow();
	}
	else
	{
		// verify current photo index
		if (pImpl_->cur_image_index_ >= pImpl_->photos_.size())
			pImpl_->cur_image_index_ = static_cast<int>(pImpl_->photos_.size() - 1);

		// reset locations
		pImpl_->Synch(false);

		pImpl_->LoadPhoto(pImpl_->cur_image_index_);

		// check if photo displayed in the second view is still available
		pImpl_->displays_.VerifyRemainingViews(pImpl_->display_, pImpl_->photos_);
	}
}


void ViewerDlg::OnTagsBar()
{
	pImpl_->tags_pane_.ToggleVisibility();
//	pImpl_->preview_.Invalidate();
	pImpl_->Resize(this);
/*
	if (pImpl_->tags_wnd_.IsValid())
	{
		pImpl_->tags_wnd_->DestroyWindow();
	}
	else
	{
		if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd()))
		{
			CTagsBar* tags_bar= new CTagsBar(Tags::GetTagCollection(), &pImpl_->tags_wnd_);
			pImpl_->tags_wnd_.Connect(tags_bar);
			pImpl_->tags_wnd_->Create(this, frame);

			if (pImpl_->current_image_)
				pImpl_->RefreshTags();
		}
	}*/
}


void ViewerDlg::OnUpdateTagsBar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
//	cmd_ui->SetCheck(pImpl_->tags_wnd_.IsValid());
	cmd_ui->SetCheck(pImpl_->tags_pane_.IsVisible());
}


void ViewerDlg::ResetSettings()	// call after settings have changed
{
	pImpl_->ResetSettings();
}


void ViewerDlg::Impl::ResetSettings()	// call after settings have changed
{
	displays_.for_all(&ViewPane::ResetDescriptionFont);

	transform_.Reset(g_Settings.default_photo_->profile_, g_Settings.viewer_->profile_, g_Settings.viewer_->rendering_);
	bool enabled= g_Settings.default_photo_->enabled_ && g_Settings.viewer_->enabled_;

	displays_.for_all<const ICMTransform&>(&ViewPane::SetGamma, transform_);
	displays_.for_all(&ViewPane::EnableGamma, enabled);

	light_table_.SetGamma(enabled ? transform_ : ICMTransform());

	if (g_Settings.correct_CRT_aspect_ratio_)
		light_table_.SetScreenAspectRatio(g_Settings.horz_resolution_, g_Settings.vert_resolution_);
	else
		light_table_.SetScreenAspectRatio(0.0f, 0.0f);

	// reset pane's scrollbar range (due to the possible screen aspect ratio)
	displays_.for_all(&ViewPane::ResetScrollBars);

	// Note: ExifProView sets colors after options have changed
}


int ViewerDlg::Impl::GetPaneHeight()
{
	CRect rect;
	preview_.GetWindowRect(rect);
	return preview_.IsHorizontalOrientation() ? rect.Height() : rect.Width();
}


void ViewerDlg::Impl::ResizePane(int height)
{
	const int SCROLL= 20;	// custom scrollbar width/height

	// preview bar size limits (min/max height for horz bar):
	height = clamp(height, 35 + SCROLL, 180 + SCROLL);

	previewPaneHeight_ = height;
	Resize(wnd_);

	//const int SCROLL= ::GetSystemMetrics(SM_CYHSCROLL);

	//height = clamp(height, 8 + SCROLL, 120 + SCROLL);

	//if (preview_bar_wnd_.Resize(height))
	//{
	//	previewPaneHeight_ = height;
	//	Resize();
	//}
}


void ViewerDlg::Impl::Resizing(bool start)
{
	displays_.CursorStayVisible(start);
}


void ViewerDlg::OnContextMenu(CWnd* wnd, CPoint pos)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_VIEWER_CONTEXT))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		if (pos.x == -1 && pos.y == -1)
		{
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);
			pos = rect.CenterPoint();
		} else if (pImpl_->toolbar_visible_){
			CRect rect;
			pImpl_->toolbar_.GetClientRect(rect);
			ClientToScreen(rect);
			if(pos.y < rect.bottom + 1) return;
			//if(pos.y - rect.top < 41) return;
		}
		CursorVisible stay(pImpl_->displays_);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, this);
	}
}


void ViewerDlg::SetColors(const std::vector<COLORREF>& colors)
{
	pImpl_->SetColors(colors);
}


void ViewerDlg::ResetColors()
{
	pImpl_->ResetColors();
}


void ViewerDlg::SetUIBrightness(double gamma)
{
	pImpl_->SetUIBrightness(gamma);
}


void ViewerDlg::Impl::SetUIBrightness(double gamma)
{
	ui_gamma_correction_ = gamma;
	light_table_.SetUIBrightness(gamma);
	tags_pane_.SetUIBrightness(gamma);
	preview_.SetUIBrightness(gamma);
	LoadBitmaps(gamma);

	COLORREF light= RGB(90,90,90);	// colors for shaded dots in bands
	COLORREF dark= RGB(50,50,50);
	rebar_.SetGripperColors(CalcNewColor(light, gamma), CalcNewColor(dark, gamma));
}


void ViewerDlg::Impl::SetColors(const std::vector<COLORREF>& colors)
{
	displays_.for_all(&ViewPane::SetBackgndColor, colors[0]);
	displays_.for_all(&ViewPane::SetTextColor, colors[1]);
	const std::wstring* nul= 0;
	displays_.for_all(&ViewPane::SetDescriptionText, nul);

	displays_.InvalidatePanes();

	tag_backgnd_color_ = colors[4];
	tag_text_color_ = colors[5];

	preview_.SetSelectionColor(colors[3]);

	//list_bar_wnd_.rgb_back_ = colors[2];
	//list_bar_wnd_.rgb_cur_selection_ = colors[3];
	//list_bar_wnd_.tag_backgnd_color_ = colors[4];
	//list_bar_wnd_.tag_text_color_ = colors[5];
	//list_bar_wnd_.Invalidate();

	light_table_.SetBackgndColor(colors[0]);
	//tags_pane_
}


void ViewerDlg::Impl::ResetColors()
{
	displays_.for_all(&ViewPane::ResetColors);
	const std::wstring* nul= 0;
	displays_.for_all(&ViewPane::SetDescriptionText, nul);

	displays_.InvalidatePanes();

	tag_text_color_ = RGB(255,255,255);
	tag_backgnd_color_ = RGB(247, 123, 0);

	preview_.SetSelectionColor(RGB(0x31,0x6a,0xc5));
//	preview_.Invalidate();

	//list_bar_wnd_.ResetColors();
	//list_bar_wnd_.Invalidate();

	light_table_.SetBackgndColor(FirstPane().GetBackgndColor());
	//tags_pane_

	const double NORMAL= 1.0;
	if (ui_gamma_correction_ != NORMAL)
		SetUIBrightness(NORMAL);
}


void ViewerDlg::OnSlideShowParams()
{
	CSlideShowOptDlg dlg;

	dlg.delay_ = pImpl_->profileSlideShowDelay_;
	dlg.hide_toolbar_ = pImpl_->profileSlideShowHideTb_;
	dlg.repeat_ = pImpl_->profileSlideShowRepeat_;

	CursorVisible stay(pImpl_->displays_);
	if (dlg.DoModal() == IDOK)
	{
		pImpl_->profileSlideShowDelay_ = dlg.delay_;
		pImpl_->profileSlideShowHideTb_ = !!dlg.hide_toolbar_;
		pImpl_->profileSlideShowRepeat_ = !!dlg.repeat_;
	}
}

void ViewerDlg::OnUpdateSlideShowParams(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


///////////////////////////////////////////////////////////////////////////////

void ViewerDlg::Impl::SendRotateNotification()
{
	preview_.ModifyItem(current_image_->GetSize(), current_image_);
	light_table_.PhotoModified(current_image_);
	host_->PhotoOrientationChanged(current_image_);
}


void ViewerDlg::Impl::Rotate(unsigned int rotation)
{
	if (current_image_ == 0)
		return;

	if (display_->StillLoading())
		return;

	CursorVisible stay(displays_);
	CWaitCursor wait;

	switch (rotation)
	{
	case 0:
		break;
	case 1:
		display_->Rotate(false);		// rotate 90 deg. counter clockwise
		current_image_->PhotoRotated(false);
		break;
	case 2:
//		display_wnd_.UpsideDown();		// rotate 180 deg.
		display_->Rotate(true);
		display_->Rotate(true);
		current_image_->PhotoRotated(true);
		current_image_->PhotoRotated(true);
		break;
	case 3:
		display_->Rotate(true);		// rotate 90 deg. clockwise
		current_image_->PhotoRotated(true);
		break;
	}

	SendRotateNotification();
//	preview_.ModifyItem(current_image_->GetSize(), current_image_);
//	light_table_.PhotoModified(current_image_);

	displays_.InvalidatePanes();

	SetInfo();
}


void ViewerDlg::OnRotate90CW()
{
	pImpl_->Rotate(3);		// rotate 90 deg. clockwise
}

void ViewerDlg::OnUpdateRotate90CW(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0 && !pImpl_->display_->StillLoading());
}


void ViewerDlg::OnRotate90CCW()
{
	pImpl_->Rotate(1);		// rotate 90 deg. counter clockwise
}

void ViewerDlg::OnUpdateRotate90CCW(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0 && !pImpl_->display_->StillLoading());
}


///////////////////////////////////////////////////////////////////////////////


void ViewerDlg::OnJpegAutoRotate()
{
	pImpl_->RotatePhoto(this, AUTO_ROTATE);
}

void ViewerDlg::OnUpdateJpegAutoRotate(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0 && pImpl_->rotation_feasible_ && !pImpl_->display_->StillLoading());
}


void ViewerDlg::OnJpegRotate90CCW()
{
	pImpl_->RotatePhoto(this, ROTATE_90_DEG_COUNTERCW);
}

void ViewerDlg::OnUpdateJpegRotate90CCW(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0 && pImpl_->rotation_feasible_ && !pImpl_->display_->StillLoading());
}


void ViewerDlg::OnJpegRotate90CW()
{
	pImpl_->RotatePhoto(this, ROTATE_90_DEG_CW);
}

void ViewerDlg::OnUpdateJpegRotate90CW(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0 && pImpl_->rotation_feasible_ && !pImpl_->display_->StillLoading());
}


void ViewerDlg::Impl::RotatePhoto(CWnd* wnd, RotationTransformation transform)
{
	if (current_image_ == 0 || !rotation_feasible_)
		return;

	CWaitCursor wait;

	if (::RotatePhoto(*current_image_, transform, false, wnd) > 0)
	{
		if (global_photo_cache)
			global_photo_cache->Remove(current_image_);
		LoadPhoto(*current_image_, 0, true);

		SendRotateNotification();
//		host_->PhotoOrientationChanged(current_image_);
//		preview_.ModifyItem(CSize(current_image_->width_, current_image_->height_), current_image_);
//		preview_.Invalidate();	// redraw (potentially) rotated thumbnail
	}
}


void ViewerDlg::TurnTagsBarOn()
{
	if (!pImpl_->tags_pane_.IsVisible())
//	if (!pImpl_->tags_wnd_.IsValid())
		OnTagsBar();
}

void ViewerDlg::OnHideTagPane()
{
	pImpl_->tags_pane_.Show(false);
	pImpl_->preview_.Invalidate();
	pImpl_->Resize(this);
}


void ViewerDlg::OnHideLightTable()
{
	pImpl_->light_table_.Show(false);
	pImpl_->preview_.Invalidate();
	pImpl_->Resize(this);
}

void ViewerDlg::OnToggleLightTable()
{
	pImpl_->light_table_.ToggleVisibility();
	pImpl_->preview_.Invalidate();
	pImpl_->Resize(this);
}

void ViewerDlg::OnUpdateToggleLightTable(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->light_table_.IsVisible());
}


void ViewerDlg::OnAddToLightTable()
{
	if (pImpl_->current_image_)
	{
		pImpl_->light_table_.AddPhoto(pImpl_->current_image_);
		pImpl_->preview_.Invalidate();
	}
}

void ViewerDlg::OnUpdateAddToLightTable(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0 && !pImpl_->light_table_.HasPhoto(pImpl_->current_image_));
}

void ViewerDlg::OnRemoveFromLightTable()
{
	if (pImpl_->current_image_)
	{
		pImpl_->light_table_.RemovePhoto(pImpl_->current_image_);
		pImpl_->preview_.Invalidate();
	}
}

void ViewerDlg::OnUpdateRemoveFromLightTable(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0 && pImpl_->light_table_.HasPhoto(pImpl_->current_image_));
}


void ViewerDlg::OnUpdateLightTableOperations(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->light_table_.HasPhotos());
}


void ViewerDlg::OnUpdateLightTableTags(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->light_table_.HasPhotos());
}


void ViewerDlg::OnLightTableHTML()
{
	if (CWnd* main= AfxGetMainWnd())
	{
		TaskPending pending(*main);

		CursorVisible stay(pImpl_->displays_);

		// photos to copy
		VectPhotoInfo photos;
		pImpl_->light_table_.GetPhotos(photos);

		if (!photos.empty())
		{
			CTaskGenHTMLAlbum dlg(photos, this);
			dlg.Go();
		}

		//BringWindowToTop();
	}
}


void ViewerDlg::OnLightTableCopy()
{
	if (CWnd* main= AfxGetMainWnd())
	{
		TaskPending pending(*main);

		CursorVisible stay(pImpl_->displays_);

		// photos to copy
		VectPhotoInfo photos;
		pImpl_->light_table_.GetPhotos(photos);

		if (!photos.empty())
			pImpl_->host_->FileOperationRequest(photos, ViewerDlgNotifications::Copy, this);

		BringWindowToTop();
	}
}


void ViewerDlg::OnLightTableSelect()
{
	VectPhotoInfo selected;
	pImpl_->light_table_.GetPhotos(selected);
	pImpl_->host_->SelectionRequest(selected);
}


void ViewerDlg::OnLightTableResize()
{
	if (CWnd* main= AfxGetMainWnd())
	{
		TaskPending pending(*main);

		// photos to resize
		VectPhotoInfo photos;
		pImpl_->light_table_.GetPhotos(photos);

		CTaskResize resize(photos, this);
		resize.Go();

		BringWindowToTop();
	}
}


void ViewerDlg::OnLightTableEmpty()
{
	pImpl_->light_table_.RemoveAll();
	pImpl_->preview_.Invalidate();
}


void ViewerDlg::Impl::FieldSelectionChanged()
{
	SetInfo();
}


void ViewerDlg::Impl::PopupMenu(bool displayed)
{
	displays_.CursorStayVisible(displayed);
}


void ViewerDlg::Impl::DecodingStarted(ViewPane* /*view*/)
{
	// reset zoom info: reduction factor could have been changed
	SetInfo();
}

void ViewerDlg::Impl::DecodingFinished(ViewPane* /*view*/)
{}


void ViewerDlg::OnMagnifierLens()
{
	pImpl_->MagnifierLens();
}


void ViewerDlg::Impl::MagnifierLens()
{
	if (DibPtr bmp= display_->GetDibPtr())
	{
		if (display_->ReloadWithoutReduction())
		{
			// wait till photo is reloaded in 1:1
			showMagnifierLens_ = true;
		}
		else
		{
			showMagnifierLens_ = false;
			CRect rect= display_->GetImageRect();
			display_->ClientToScreen(rect);
			MagnifierWnd::Close();
			new MagnifierWnd(bmp, rect, wnd_);
		}
	}
}


void ViewerDlg::OnUpdateMagnifierLens(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(/*display_->GetDibPtr() != 0 &&*/ !pImpl_->display_->StillLoading());
}


void ViewerDlg::Impl::DecodingThreadFinished(ViewPane* view)
{
	if (showMagnifierLens_)
	{
		showMagnifierLens_ = false;
		MagnifierLens();
	}
	//if (tags_wnd_.IsValid())
	//	tags_wnd_->PhotoReloadingDone();
}


void ViewerDlg::OnTagSelected(UINT cmd)
{
	pImpl_->tags_pane_.AssignTag(cmd - ID_TAG_SELECTED);
	//if (pImpl_->tags_wnd_.IsValid())
	//	pImpl_->tags_wnd_->SendMessage(WM_COMMAND, cmd);
}


void ViewerDlg::OnUpdateTags(CCmdUI* cmd_ui)
{
	if (cmd_ui->m_pMenu != 0 && cmd_ui->m_pSubMenu == 0 && cmd_ui->m_nID == ID_TAG_SELECTED)	// submenu hit?
	{
		// user tries to open popup with tags; update list of tags
		pImpl_->ResetMenuTags(*cmd_ui->m_pMenu, ID_TAG_SELECTED);
	}

	//if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd()))
	{
		int index= cmd_ui->m_nID - ID_TAG_SELECTED;

		const PhotoTagsCollection& tags= Tags::GetTagCollection();

		if (tags.Empty() || index >= tags.GetCount() || pImpl_->current_image_ == 0)
			cmd_ui->Enable(false);
		else
		{
			cmd_ui->Enable();
			bool has_tag= pImpl_->current_image_->GetTags().FindTag(tags.Get(index));
			cmd_ui->SetCheck(has_tag ? 1 : 0);
		}
	}
//	else
//		cmd_ui->Enable(false);
}


void ViewerDlg::OnRateImage(UINT cmd)
{
	if (pImpl_->current_image_ != 0)
	{
		try
		{
			int rating= cmd - ID_RATE_NONE;
			ASSERT(rating >= 0 && rating <= 5);

			VectPhotoInfo photos;
			photos.push_back(pImpl_->current_image_);

			// rate image
			::ApplyRatingToPhotos(photos, rating, this);
		}
		CATCH_ALL
	}
}

void ViewerDlg::OnUpdateRates(CCmdUI* cmd_ui)
{
	if (pImpl_->current_image_ != 0)
	{
		cmd_ui->Enable();
		int rating= pImpl_->current_image_->GetRating();
		bool mark= rating == cmd_ui->m_nID - ID_RATE_NONE;
		if (mark)
			cmd_ui->SetRadio();
	}
	else
		cmd_ui->Enable(false);
}


void ViewerDlg::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == VK_ESCAPE && ::GetKeyState(VK_SHIFT) >= 0 && ::GetKeyState(VK_CONTROL) >= 0 && ::GetKeyState(VK_MENU) >= 0)
	{
		OnCmdClose();
		return;
	}

	CFrameWnd::OnKeyDown(chr, rep_cnt, flags);
}


void ViewerDlg::OnToggleDispDescription()
{
	bool enable= !pImpl_->FirstPane().IsPhotoDescDisplayed();
	pImpl_->displays_.for_all(&ViewPane::EnablePhotoDesc, enable);
}

void ViewerDlg::OnUpdateToggleDispDescription(CCmdUI* cmd_ui)
{
	cmd_ui->SetCheck(pImpl_->FirstPane().IsPhotoDescDisplayed());
}


void ViewerDlg::OnUseScrollBars()
{
	bool enable= !pImpl_->FirstPane().IsUsingScrollBars();
	pImpl_->displays_.UseScrollBars(enable);
}

void ViewerDlg::OnUpdateUseScrollBars(CCmdUI* cmd_ui)
{
	cmd_ui->SetCheck(pImpl_->FirstPane().IsUsingScrollBars());
}


void ViewerDlg::OnHorzPreviewBar()
{
	pImpl_->preview_.SetOrientation(true);
	pImpl_->separator_.SetOrientation(pImpl_->preview_.IsHorizontalOrientation());
	pImpl_->Resize(this);
}

void ViewerDlg::OnVertPreviewBar()
{
	pImpl_->preview_.SetOrientation(false);
	pImpl_->separator_.SetOrientation(pImpl_->preview_.IsHorizontalOrientation());
	pImpl_->Resize(this);
}

void ViewerDlg::OnUpdateHorzPreviewBar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetRadio(pImpl_->preview_.IsHorizontalOrientation());
}

void ViewerDlg::OnUpdateVertPreviewBar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetRadio(!pImpl_->preview_.IsHorizontalOrientation());
}


void ViewerDlg::OnSmallIcons()
{
	if (pImpl_->toolbar_.SmallIcons())
	{
		pImpl_->rebar_.BandResized(pImpl_->toolbar_, pImpl_->toolbar_.Size());
		pImpl_->Resize(this);
	}
}

void ViewerDlg::OnLargeIcons()
{
	if (pImpl_->toolbar_.LargeIcons())
	{
		pImpl_->rebar_.BandResized(pImpl_->toolbar_, pImpl_->toolbar_.Size());
		pImpl_->Resize(this);
	}
}

void ViewerDlg::OnUpdateSmallIcons(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetRadio(pImpl_->toolbar_.IsSmallSet());
}

void ViewerDlg::OnUpdateLargeIcons(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetRadio(!pImpl_->toolbar_.IsSmallSet());
}


void ViewerDlg::OnToggleTagsInPreviewBar()
{
	pImpl_->show_tags_in_previewbar_ = !pImpl_->show_tags_in_previewbar_;
	pImpl_->preview_.Invalidate();
}

void ViewerDlg::OnUpdateToggleTagsInPreviewBar(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->show_tags_in_previewbar_);
}


int ViewerDlg::CurrentPhoto() const
{
	return pImpl_->cur_image_index_;
}

void ViewerDlg::SetTransform(const ICMTransform& trans)
{
	pImpl_->transform_ = trans;
}


LRESULT ViewerDlg::OnAppCommand(WPARAM w, LPARAM l)
{
	int cmd= GET_APPCOMMAND_LPARAM(l);

	switch (cmd)
	{
	case APPCOMMAND_BROWSER_BACKWARD:
		OnPhotoPrev();
		break;

	case APPCOMMAND_BROWSER_FORWARD:
		OnPhotoNext();
		break;

	default:
		return 0;	// not used
	}

	return 1;	// handled
}


void ViewerDlg::OnOpenPhoto()
{
	if (static_cast<size_t>(pImpl_->cur_image_index_) < pImpl_->photos_.size())
		::OpenPhotograph(pImpl_->photos_[pImpl_->cur_image_index_]->GetOriginalPath().c_str(), this, pImpl_->photos_[pImpl_->cur_image_index_]->IsRaw());
}


void ViewerDlg::OnToggleSmoothScroll()
{
	pImpl_->smooth_scroll_ = !pImpl_->smooth_scroll_;
	pImpl_->preview_.SetSmoothScrollingSpeed(pImpl_->smooth_scroll_ ? g_Settings.smooth_scrolling_speed_ : 0);
}

void ViewerDlg::OnUpdateToggleSmoothScroll(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(pImpl_->smooth_scroll_);
}


void ViewerDlg::Impl::MultiView(CWnd* wnd, bool pressBtn)
{
	if (!toolbar_visible_)
		return;

	const int cmd= ID_COMPARE_MULTIPLE;

	if (pressBtn)
		toolbar_.PressButton(cmd);

	CMenu menu;
	if (!menu.LoadMenu(IDR_COMPARE_MULTIPLE))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		CRect rect;
		toolbar_.GetRect(cmd, rect);
		toolbar_.ClientToScreen(rect);

		CursorVisible stay(displays_);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.bottom, wnd);
	}

	if (pressBtn)
		toolbar_.PressButton(cmd, false);
}

/*
void ViewerDlg::OnUpdatePhotoList(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->toolbar_visible_);
}*/


void ViewerDlg::OnTurnMultiPane(UINT cmd)
{
	size_t count= cmd - ID_SINGLE_PANE + 1;

	if (pImpl_->displays_.GetVisibleCount() != count)
	{
		pImpl_->displays_.ShowPanes(count, pImpl_->displays_.GetCurrentLayout());
		// no of view may have changed; reinit them now
		pImpl_->ReinitMultiViews();
		pImpl_->Resize(this);
		pImpl_->profileMultiViewCount_ = static_cast<int>(count);
	}
}


void ViewerDlg::OnUpdateMultiPane(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	size_t count= cmd_ui->m_nID - ID_SINGLE_PANE + 1;
	cmd_ui->SetRadio(pImpl_->displays_.GetVisibleCount() == count);
}


void ViewerDlg::OnToggleMultiView()	// multi panes on/off
{
	bool show= pImpl_->displays_.GetVisibleCount() == 1;
	pImpl_->ShowMultiView(show);
	pImpl_->Resize(this);
}


void ViewerDlg::OnUpdateMultiView(CCmdUI* cmd_ui)	// update for a toolbar btn
{
	cmd_ui->Enable(pImpl_->toolbar_visible_);
	cmd_ui->SetCheck(pImpl_->displays_.GetVisibleCount() > 1 ? 1 : 0);
}


void ViewerDlg::OnToggleMultiViewLayout()	// toggle layout -> horz, vert, grid, ...
{
	if (pImpl_->displays_.GetVisibleCount() > 1)
	{
		// layout change only; not need to reinit views
		pImpl_->displays_.ShowPanes(pImpl_->displays_.GetVisibleCount(), pImpl_->displays_.GetNextLayout());
		pImpl_->Resize(this);
		pImpl_->view_layout_ = pImpl_->displays_.GetCurrentLayout();
	}
}


void ViewerDlg::Impl::ShowMultiView(bool show)
{
	if (show)
	{
		size_t count= profileMultiViewCount_;
		if (count < 2)
			count = 2;
		displays_.ShowPanes(count, view_layout_);
	}
	else
		displays_.ShowPanes(1, ViewPanes::Horizontal);

	ReinitMultiViews();

	preview_.Invalidate();	// redraw pane indicators
}


void ViewerDlg::Impl::ReinitMultiViews()
{
	ConstPhotoInfoPtr photo= display_->GetCurrentPhoto();

	display_ = &FirstPane();

	SetActiveView();

	if (displays_.GetVisibleCount() > 1)
	{
		// make sure zoom in all views is in sync

		double zoom= 0.0;
		if (!display_->IsZoomToFit())
			zoom = display_->GetLogicalZoom();

		displays_.for_all(&ViewPane::SetLogicalZoom, zoom, g_Settings.allow_magnifying_above100_);

		for (size_t i= 0; i < displays_.GetVisibleCount(); ++i)
			if (ViewPane* view= displays_.GetPane(i))
				if (view != display_)
					if (Path* path= displays_.GetViewPhotoPath(view))
						if (PhotoInfoPtr photo= FindPhoto(path->c_str()))
							LoadPhotoIntoView(view, photo);
	}
	else
	{
		LoadPhoto(photo ? photo : current_image_);
		//snd_display_.Reset();
	}

	displays_.ResetHiddenPanes();
}


void ViewerDlg::Impl::SetActiveView()
{
	displays_.SetActive(display_);
}


void ViewerDlg::Impl::ViewPhotoDropped(ViewPane* view, PhotoInfoPtr photo)
{
	if (photo)
	{
		if (display_ == view)
			LoadPhoto(photo);	// load into active view; change current photo
		else
		{
			// load into inactive view, no notifications sent

			LoadPhotoIntoView(view, photo);

			//preview_.RedrawItem(photo);
			preview_.Invalidate();
		}
	}
}


PhotoInfoPtr ViewerDlg::Impl::FindPhoto(const TCHAR* original_path) const
{
	const size_t count= photos_.size();

	for (size_t i= 0; i < count; ++i)
		if (photos_[i]->GetOriginalPath() == original_path)
			return photos_[i];

	return 0;
}


void ViewerDlg::Impl::ViewClicked(ViewPane* view)
{
	if (display_ == view || !displays_.IsViewVisible(view) || view == 0)
		return;

	display_ = view;
	SetActiveView();

	//TODO:

	if (ConstPhotoInfoPtr photo= view->GetCurrentPhoto())
		LoadPhoto(photo);
	else
		LoadPhoto(cur_image_index_, true);

	SetInfo();
}


void ViewerDlg::Impl::ViewScrolled(ViewPane* view, CPoint pos_img)
{
	if (::GetKeyState(VK_CONTROL) >= 0)	// not pressed?
		displays_.ScrollOthers(view, pos_img);
}


void ViewerDlg::Impl::MiddleButtonDown(CPoint pos)
{
	MagnifierLens();
}


void ViewerDlg::OnClose()
{
	// postponed close due to the second msg run loop in mini frame windows; let them finish first
	// if they are active or else they will crash the app
	PostMessage(WM_USER + 1234);
}


LRESULT ViewerDlg::OnCloseApp(WPARAM, LPARAM)
{
	CFrameWnd::OnClose();
	return 0;
}


void ViewerDlg::OnLightTableDeleteAllOthers()
{
	try
	{
		VectPhotoInfo table;
		pImpl_->light_table_.GetPhotos(table);

		VectPhotoInfo photos= pImpl_->photos_;

		sort(photos.begin(), photos.end());

		for (VectPhotoInfo::const_iterator it= table.begin(); it != table.end(); ++it)
		{
			VectPhotoInfo::iterator p= lower_bound(photos.begin(), photos.end(), *it);
			if (*p != *it)
				throw String(_T("Image information is out of sync"));

			photos.erase(p);
		}

		if (photos.empty())
			return;

		if (MessageBox(_T("Warning! All images except those in a light table will be deleted.\nAre you sure?"),
				_T("Deleting Images"), MB_ICONQUESTION | MB_YESNO) != IDYES)
			return;

		pImpl_->host_->SelectAndDelete(photos);
	}
	CATCH_ALL
}


void ViewerDlg::Impl::Move(MoveDir move)
{
	if (display_->IsScrollingNeeded())
	{
		// scroll image

		switch (move)
		{
		case MoveLeft:
			display_->HorzScroll(SB_LINELEFT);
			break;

		case MoveRight:
			display_->HorzScroll(SB_LINERIGHT);
			break;

		case MoveUp:
			display_->VertScroll(SB_LINEUP);
			break;

		case MoveDown:
			display_->VertScroll(SB_LINEDOWN);
			break;
		}
	}
	else
	{
		// move to the next or prev image

		switch (move)
		{
		case MoveLeft:
		case MoveUp:
			PhotoPrev();
			break;

		case MoveRight:
		case MoveDown:
			PhotoNext();
			break;
		}
	}
}

void ViewerDlg::OnMoveLeft()
{
	pImpl_->Move(MoveLeft);
}

void ViewerDlg::OnMoveRight()
{
	pImpl_->Move(MoveRight);
}

void ViewerDlg::OnMoveUp()
{
	pImpl_->Move(MoveUp);
}

void ViewerDlg::OnMoveDown()
{
	pImpl_->Move(MoveDown);
}


void ViewerDlg::OnSwitchViews()
{
	pImpl_->SwitchViews();
}

void ViewerDlg::Impl::SwitchViews()
{
	ViewPane* next= displays_.GetNextView(display_);
	if (next != 0 && next != display_)
		ViewClicked(next);	// switch active view
}


void ViewerDlg::Impl::ResetMenuTags(CMenu& menu, int first_id)
{
	if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd()))
	{
		const PhotoTagsCollection& tags= Tags::GetTagCollection();
		::ResetPopupMenuTags(menu, first_id, tags);
	}
}


//void ViewPanes::Assign(const boost::function<void (ViewPane*)>& view_clicked)
//{
//	view_clicked_ = view_clicked;
//}


DROPEFFECT ViewPanes::PhotoDragAndDrop(size_t pane_index, PhotoDrop::DropAction action, const TCHAR* files)
{
	if (action == PhotoDrop::Enter)
	{
		// check if dropped file is loaded
		if (path_to_photo_(files) != 0)
			return DROPEFFECT_COPY;
		else
			return DROPEFFECT_NONE;
	}
	else if (action == PhotoDrop::Drop)
	{
		//
		ASSERT(pane_index < array_count(display_));
		photo_dropped_(&display_[pane_index].pane_, path_to_photo_(files));
	}
	else if (action == PhotoDrop::DragOver)
	{
		return DROPEFFECT_COPY;
	}

	return DROPEFFECT_NONE;
}


void ViewPanes::Create(CWnd* parent, const boost::function<void (ViewPane*)>& view_clicked, const boost::function<void (ViewPane*, PhotoInfoPtr path)>& photo_dropped, const boost::function<PhotoInfoPtr (const TCHAR* path)>& path_to_photo, ViewPaneNotifications* host)
{
	view_clicked_ = view_clicked;
	photo_dropped_ = photo_dropped;
	path_to_photo_ = path_to_photo;

	{
		int cmd1= ID_TOGGLE_VIEW_LAYOUT;
		VERIFY(display_[0].caption_.Create(parent, IDB_HORZ_VERT_TB, &cmd1, 1,
			boost::bind(&ViewPanes::ViewClicked, this, &display_[0].pane_)));

		for (size_t i= 1; i < COUNT; ++i)
		{
			int cmd2= ID_CLOSE_VIEW;
			VERIFY(display_[i].caption_.Create(parent, IDB_CLOSE_TB, &cmd2, 1,
				boost::bind(&ViewPanes::ViewClicked, this, &display_[i].pane_)));
		}
	}

	for (size_t i= 0; i < COUNT; ++i)
	{
		VERIFY(display_[i].pane_.Create(parent));
		if (i > 0)
			display_[i].pane_.ShowWindow(SW_HIDE);
		display_[i].pane_.SetHost(host);
		display_[i].pane_.SetPhotoCache(global_photo_cache.get());
		display_[i].drop_target_.reset(PhotoDrop::CreatePhotoDropTarget(display_[i].pane_, true, boost::bind(&ViewPanes::PhotoDragAndDrop, this, i, _1, _2)));
		display_[i].drop_target_->Register(&display_[i].pane_);
		display_[i].pane_.EnableZoomToFill(g_Settings.allow_zoom_to_fill_, g_Settings.percent_of_image_to_hide_);
	}

	visible_count_ = 1;
}


void ViewerDlg::OnChangeMultiPaneLayout(UINT cmd)
{
	ViewPanes::Layout layout= ViewPanes::Vertical;

	switch (cmd)
	{
	case ID_HORIZONTAL_LAYOUT:
		layout = ViewPanes::Horizontal;
		break;
	case ID_VERTICAL_LAYOUT:
		layout = ViewPanes::Vertical;
		break;
	case ID_GRID_LAYOUT:
		layout = ViewPanes::Grid;
		break;
	case ID_SWITCH_LAYOUT:
		layout = pImpl_->displays_.GetNextLayout();
		break;
	default:
		ASSERT(false);
		break;
	}

	const size_t count= pImpl_->displays_.GetVisibleCount();

	if (count <= 1)
		return;

	if (layout == ViewPanes::Grid && count != 4)
		return;

	pImpl_->displays_.ShowPanes(pImpl_->displays_.GetVisibleCount(), layout);
	pImpl_->ReinitMultiViews();
	pImpl_->Resize(this);

	pImpl_->view_layout_ = layout;
}


void ViewerDlg::OnUpdateMultiPaneLayout(CCmdUI* cmd_ui)
{
	const size_t count= pImpl_->displays_.GetVisibleCount();
	if (count <= 1)
	{
		cmd_ui->Enable(false);
		return;
	}

	bool enable= true;

	switch (cmd_ui->m_nID)
	{
	case ID_HORIZONTAL_LAYOUT:
		cmd_ui->SetRadio(pImpl_->displays_.GetEffectiveLayout() == ViewPanes::Horizontal);
		break;
	case ID_VERTICAL_LAYOUT:
		cmd_ui->SetRadio(pImpl_->displays_.GetEffectiveLayout() == ViewPanes::Vertical);
		break;
	case ID_GRID_LAYOUT:
		cmd_ui->SetRadio(pImpl_->displays_.GetEffectiveLayout() == ViewPanes::Grid);
		enable = count == 4;
		break;
	case ID_SWITCH_LAYOUT:
		break;
	default:
		ASSERT(false);
		break;
	}

	cmd_ui->Enable(enable);
}


void ViewerDlg::OnActivateViewPane(UINT cmd)
{
	size_t index= 0;

	switch (cmd)
	{
	case ID_VIEW_PANE_1:
		index = 0;
		break;
	case ID_VIEW_PANE_2:
		index = 1;
		break;
	case ID_VIEW_PANE_3:
		index = 2;
		break;
	case ID_VIEW_PANE_4:
		index = 3;
		break;
	}

	pImpl_->ViewClicked(pImpl_->displays_.GetPane(index));
}


void ViewerDlg::OnRenamePhoto()
{
	if (pImpl_->current_image_ == 0)
		return;

	CursorVisible stay(pImpl_->displays_);

	try
	{
		PhotoInfoPtr photo= pImpl_->current_image_;

		// rename

		VectPhotoInfo photos;
		photos.push_back(photo);

		pImpl_->host_->FileOperationRequest(photos, ViewerDlgNotifications::Rename, this);

		BringWindowToTop();
	}
	CATCH_ALL
}


void ViewerDlg::OnPrint()
{
	if (pImpl_->current_image_ == 0)
		return;

	CursorVisible stay(pImpl_->displays_);

	try
	{
		PhotoInfoPtr photo= pImpl_->current_image_;
		VectPhotoInfo photos;
		photos.push_back(photo);

		CTaskPrint print(photos, photo->GetDisplayPath().c_str());
		print.Go();
	}
	CATCH_ALL
}


void ViewerDlg::OnUpdatePrint(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(pImpl_->current_image_ != 0);
}


void ViewerDlg::OnSetAsWallpaper()
{
	bool SetWallpaper(const PhotoInfo& photo);

	if (PhotoInfoPtr photo= pImpl_->current_image_)
		SetWallpaper(*photo);
}

void ViewerDlg::OnUpdateSetAsWallpaper(CCmdUI* cmd)
{
	cmd->Enable(pImpl_->current_image_ != 0);
}


void ViewerDlg::OnToggleCenterImage()
{
	pImpl_->keep_current_img_centered_ = !pImpl_->keep_current_img_centered_;

	pImpl_->preview_.KeepCurrentItemCentered(pImpl_->keep_current_img_centered_);

	pImpl_->preview_.SelectionVisible(pImpl_->cur_image_index_, true);
}

void ViewerDlg::OnUpdateToggleCenterImage(CCmdUI* cmd)
{
	cmd->Enable();
	cmd->SetCheck(pImpl_->keep_current_img_centered_ ? 1 : 0);
}


void ViewerDlg::OnMButtonDown(UINT flags, CPoint point)
{
	CFrameWnd::OnMButtonDown(flags, point);
}


void ViewerDlg::Impl::MouseDoubleClick(CPoint pos)
{
	if (display_->IsZoomToFit())	// zoom to fit currently?
	{
		// the user dbl clicked at 'pos'; translate this position into an image pixel at 1:1 scale
		// and create an offset, such that after zooming to 100%, clicked place will remain underneath the mouse cursor

//		CPoint offset(-1, -1);
//		CRect rect= display_->GetImageRect();
//
//		if (1)//rect.PtInRect(pos) && current_image_ != 0)
//		{
//			// physical image size in pixels
//			CSize size= current_image_->GetSize();
//
//			// now rescale offset to pixels of image at 100%
//			offset = CPoint(::MulDiv(size.cx, pos.x - rect.left, rect.Width()), ::MulDiv(size.cy, pos.y - rect.top, rect.Height()));
//
//			// since image offset really points to the left/top image corner rather than point of interest (the clicked one)
//			// we need to subtract this distance in screen pixels
//			offset -= CSize(pos.x, pos.y);
//
//			if (offset.x < 0)
//				offset.x = 0;
//			if (offset.y < 0)
//				offset.y = 0;
////offset=CPoint(pos.x - rect.left, pos.y - rect.top);
//offset=CPoint(pos.x, pos.y);
//		}

		Zoom100(pos);
	}
	else
		ZoomToFit();
}


void ViewerDlg::Impl::AnimationStarts()
{
	host_->AnimationStarts();
	//for (size_t i= 0; i < displays_.GetVisibleCount(); ++i)
	//	if (ViewPane* view= displays_.GetPane(i))
	//		view->UpdateWindow();
}


void ViewerDlg::Impl::RefreshAfterChange(const VectPhotoInfo& photos)
{
	preview_.Invalidate();

	// tags might have changed too, so refresh them
	RefreshTags();
}


void ViewerDlg::Impl::RefreshTags()
{
//	if (tags_wnd_.IsValid())
//		tags_wnd_->PhotoSelected(current_image_, true, display_->StillLoading());

	tags_pane_.PhotoSelected(current_image_, true, display_->StillLoading());
}


void ViewerDlg::OnUpdateEnableLabels(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();

	bool check= false;

	switch (cmd_ui->m_nID)
	{
	case ID_VIEWER_BAR_DATE_TIME:
		check = pImpl_->preview_bar_labels_ == Impl::LABEL_DATE;
		break;
	case ID_VIEWER_BAR_FILE_NAME:
		check = pImpl_->preview_bar_labels_ == Impl::LABEL_NAME;
		break;
	case ID_VIEWER_BAR_FILE_NAME_EXT:
		check = pImpl_->preview_bar_labels_ == Impl::LABEL_NAME_EXT;
		break;
	case ID_VIEWER_BAR_NO_LABELS:
		check = pImpl_->preview_bar_labels_ == Impl::LABEL_NONE;
		break;
	default:
		ASSERT(false);
		break;
	}
	cmd_ui->SetRadio(check);
}


void ViewerDlg::OnChangePreviewLabels(UINT cmd_id)
{
	switch (cmd_id)
	{
	case ID_VIEWER_BAR_DATE_TIME:
		pImpl_->SwitchPreviewBarLabels(Impl::LABEL_DATE);
		break;
	case ID_VIEWER_BAR_FILE_NAME:
		pImpl_->SwitchPreviewBarLabels(Impl::LABEL_NAME);
		break;
	case ID_VIEWER_BAR_FILE_NAME_EXT:
		pImpl_->SwitchPreviewBarLabels(Impl::LABEL_NAME_EXT);
		break;
	case ID_VIEWER_BAR_NO_LABELS:
		pImpl_->SwitchPreviewBarLabels(Impl::LABEL_NONE);
		break;
	default:
		ASSERT(false);
		break;
	}
}
