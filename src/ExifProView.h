/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifProView.h : interface of the ExifView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Columns.h"
#include "PhotoInfo.h"
#include "PhotoInfoStorage.h"
#include "ConnectionPoint.h"
#include "ViewerDlg.h"
#include "PreviewPane.h"
#include "SeparatorWnd.h"
#include "ResizeWnd.h"
#include "PhotoCtrl.h"
#include "ItemIdList.h"
class BrowserFrame;
#include "ReloadJob.h"
#include "SliderCtrlEx.h"
#include "Profile.h"
#include "Tasks.h"
#include "ExifViewReBar.h"
#include "SortingOptions.h"
#include "ExifStatusBar.h"
#include "Pane.h"
#include "ViewerDlgNotifications.h"
#include "DistributionBar.h"
#include <deque>
#include "FolderPath.h"
#include "ImageLoader.h"
#include "DirectoryChangeWatcher.h"
#include "CustomTabCtrl.h"
#include "FilterPanelDlg.h"
#include "CustomFilters.h"
#include "PhotoTagsCollection.h"
class CustomColumns;


class ExifView : public PaneWnd, /*public ResizeWnd,*/ PhotoCtrlNotification, CSortingOptionsNotification,
	CExifStatusBarNotifications, ViewerDlgNotifications, FilterOperations
{
public:
	ExifView(Columns& columns);

// Attributes
	enum ViewMode { VIEW_MODE_THUMBNAILS, VIEW_MODE_DETAILS, VIEW_MODE_TILES, VIEW_MODE_PICTURES };
	enum GroupMode { GROUP_BY_FOLDERS, GROUP_BY_SIMILARITY, NO_GROUPING, GROUP_BY_DATE };
	enum GroupByDate { DONT_GROUP= 0, BY_YEAR, BY_MONTH, BY_DAY };

	// get current view mode
	ViewMode GetViewMode() const			{ return view_mode_; }

	// set grouping mode
	void SetGroupMode(GroupMode mode);

	// get status text (no of images or scanning progress)
	CString GetImagesStat() const;

	// get short text info about currently selected photo
	CString GetCurImageInfo() const;

	// returns true if photos are filtered
	bool IsFilterActive() const;
	bool IsMainPageFilterActive() const;
	bool IsDateTimeFilterActive() const;

	// returns tru if scanning thread is running
	bool IsScanning() const					{ return reload_thread_->IsScanning(); }

	// list ctrl
	PhotoCtrl& GetListCtrl()				{ return photo_ctrl_; }
	const PhotoCtrl& GetListCtrl() const	{ return photo_ctrl_; }

	// change recursive scan flag
	void SetRecursiveScan(bool recursive)	{ recursive_scan_ = recursive; }

	// change 'read only EXIF' flag
	void SetReadOnlyExif(bool exif_only)	{ read_only_photos_withEXIF_ = exif_only; }

	void SetFrame(BrowserFrame* frame)		{ frame_ = frame; }

	std::vector<uint16>& SelectedColumns()	{ return columns_; }

	// current folder
//	const ItemIdList& GetCurrentPath() const	{ return idl_folder_; }
	FolderPathPtr GetCurrentPath() const;

	// return item with focus (current one)
	PhotoInfoPtr CurrentItem() const;

	// get custom columns definition
//	const CustomColumns& GetCustomColumns() const;

// Operations
public:
	// create window
	bool Create(CWnd* parent);

	// scan 'idlPath' for photos
	bool Browse(FolderPathPtr path);

	// set view mode
	void SetViewMode(ViewMode view_mode);

	// set next view mode
	void SetNextViewMode();

	// new path selected; start reloading
	bool PathSelected(FolderPathPtr path);

	// view sorting popup menu
	void OnViewSortPopupMenu(CPoint pos);

	// toggle preview pane visibility
//	void TogglePreviewPane();

	// display only photos with given text in description (pass null to cancel filtering)
	void FilterPhotos(const TCHAR* text);
	// diaplay only photos from given date range
	void FilterPhotos(DateTime from, DateTime to, DateTime hist_from, DateTime hist_to);
	void CancelFiltering();
	void CancelDateFiltering();
	void ClearFilter();

	// populate time line
	void RebuildHistogram(const FilterData* filter= 0);

	// move photo after applying tags
	void MovePhoto(PhotoInfoPtr photo);
	void MovePhotos(const VectPhotoInfo& photos);

	// export EXIF data
	void ExportExifData();

	// rotate photos (lossless JPEG rotation)
	void RotatePhotos();

	// remove photo (after deleting in viewer)
	void RemovePhoto(PhotoInfoPtr photo);

	// notification
	//void TagApplied(PhotoInfoPtr photo);
	//void TagApplied(const VectPhotoInfo& photos);

	// open viewer window and turn tags bar on
	void StartTagging();

	// load photos from folder stored in document
	bool ReloadPhotos();

	// select photographs in list ctrl
	void SelectPhotos(const VectPhotoInfo& photos);

	// copy/move operation (selected photos)
	void FileOperation(const VectPhotoInfo& photos, bool copy, CWnd* parent);

	// change custom columns definition
	void SetCustomColumns(const CustomColumns& col);

	// enable/disable sending current image change notification
	void EnableNotification(bool enable);

	// events
	typedef boost::signals2::signal<void (ReloadJob::Progress state, size_t images)> ReloadProgress;
	typedef boost::signals2::signal<void (bool on)> FilterChanged;

	// connect handler to the event
	slot_connection ConnectReloadProgress(ReloadProgress::slot_function_type fn);
	slot_connection ConnectFilterChanged(FilterChanged::slot_function_type fn);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ExifView)
public:
	virtual void OnDraw(CDC* dc);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
	virtual BOOL IsFrameWnd() const;

	virtual void InitialUpdate();

// Implementation
public:
	virtual ~ExifView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(ExifView)
	afx_msg void OnView();
	afx_msg void OnUpdateView(CCmdUI* cmd_ui);
	afx_msg void OnUpdateViewSort(CCmdUI* cmd_ui);
	afx_msg void OnViewMode();
	afx_msg void OnUpdateViewMode(CCmdUI* cmd_ui);
	afx_msg void OnViewThumbnails();
	afx_msg void OnViewDetails();
	afx_msg void OnViewTiles();
	afx_msg void OnViewPictures();
	afx_msg void OnUpdateViewThumbnails(CCmdUI* cmd_ui);
	afx_msg void OnUpdateViewDetails(CCmdUI* cmd_ui);
	afx_msg void OnUpdateViewTiles(CCmdUI* cmd_ui);
	afx_msg void OnUpdateViewPictures(CCmdUI* cmd_ui);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnRecursive();
	afx_msg void OnUpdateRecursive(CCmdUI* cmd_ui);
	afx_msg void OnReadOnlyExif();
	afx_msg void OnUpdateReadOnlyExif(CCmdUI* cmd_ui);
	afx_msg void OnRefresh();
	afx_msg void OnUpdateRefresh(CCmdUI* cmd_ui);
	afx_msg void OnStopScanning();
	afx_msg void OnMarkImage();
	afx_msg void OnUpdateMarkImage(CCmdUI* cmd_ui);
	afx_msg void OnMarkAll();
	afx_msg void OnUpdateMarkAll(CCmdUI* cmd_ui);
	afx_msg void OnMarkNone();
	afx_msg void OnUpdateMarkNone(CCmdUI* cmd_ui);
	afx_msg void OnMarkInvert();
	afx_msg void OnUpdateMarkInvert(CCmdUI* cmd_ui);
	afx_msg int  OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnUpdateDelete(CCmdUI* cmd_ui);
	afx_msg void OnTaskDelete();
	afx_msg void OnTaskCopy();
	afx_msg void OnTaskMove();
	afx_msg void OnTaskResize();
	afx_msg void OnTaskGenSlideShow();
	afx_msg void OnTaskHistogram();
	afx_msg void OnUpdateNextPane(CCmdUI* cmd_ui);
	afx_msg void OnTaskEditDesc();
	afx_msg void OnUpdateTaskEditDesc(CCmdUI* cmd_ui);
	afx_msg void OnUpdateEscape(CCmdUI* cmd_ui);
	afx_msg void OnSortBySimilarity();
	afx_msg void OnUpdateSortBySimilarity(CCmdUI* cmd_ui);
	afx_msg void OnViewThumbSmaller();
	afx_msg void OnUpdateViewThumbSmaller(CCmdUI* cmd_ui);
	afx_msg void OnViewThumbBigger();
	afx_msg void OnUpdateViewThumbBigger(CCmdUI* cmd_ui);
	afx_msg void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnGroupByFolders();
	afx_msg void OnUpdateGroupByFolders(CCmdUI* cmd_ui);
	afx_msg void OnGroupByTags();
	afx_msg void OnUpdateGroupByTags(CCmdUI* cmd_ui);
	afx_msg void OnGroupByStars();
	afx_msg void OnUpdateGroupByStars(CCmdUI* cmd_ui);
	afx_msg void OnGroupByDate();
	afx_msg void OnUpdateGroupByDate(CCmdUI* cmd_ui);
	afx_msg void OnNoGrouping();
	afx_msg void OnUpdateNoGrouping(CCmdUI* cmd_ui);
	afx_msg void OnTaskGenHTMLAlbum();
	afx_msg void OnTaskExport();
	afx_msg void OnUpdateTaskExport(CCmdUI* cmd_ui);
	afx_msg void OnTaskRotate();
	afx_msg void OnUpdateTaskRotate(CCmdUI* cmd_ui);
	afx_msg void OnStickySelection();
	afx_msg void OnUpdateStickySelection(CCmdUI* cmd_ui);
	afx_msg void OnEditIPTC();
	afx_msg void OnUpdateEditIPTC(CCmdUI* cmd_ui);
	afx_msg void OnUpdateStopScanning(CCmdUI* cmd_ui);
	afx_msg void OnTaskPrint();
	afx_msg void OnTaskPrintThumbnails();
	afx_msg void OnUpdateTaskPrint(CCmdUI* cmd_ui);
	afx_msg void OnUpdateTaskPrintThumbnails(CCmdUI* cmd_ui);
	afx_msg void OnTaskCopyTagged();
	afx_msg void OnUpdateTaskCopyTagged(CCmdUI* cmd_ui);
	afx_msg void OnViewAll();
	afx_msg void OnUpdateViewAll(CCmdUI* cmd_ui);
	afx_msg void OnViewSelected();
	afx_msg void OnUpdateViewSelected(CCmdUI* cmd_ui);
	afx_msg void OnViewTagged();
	afx_msg void OnUpdateViewTagged(CCmdUI* cmd_ui);
	afx_msg void OnOpenPhoto();
	afx_msg void OnOpenInExplorer();
	afx_msg void OnUpdateOpenInExplorer(CCmdUI* cmd_ui);
	afx_msg void OnUpdateOpenPhoto(CCmdUI* cmd_ui);
	afx_msg void OnContextMenu(CWnd* wnd, CPoint point);
	afx_msg void OnUpdateTaskCopy(CCmdUI* cmd_ui);
	afx_msg void OnUpdateTaskMove(CCmdUI* cmd_ui);
	afx_msg void OnUpdateTaskResize(CCmdUI* cmd_ui);
	afx_msg void OnUpdateTaskHistogram(CCmdUI* cmd_ui);
	afx_msg void OnEscape();
	afx_msg void OnViewShowLabels();
	afx_msg void OnUpdateViewShowLabels(CCmdUI* cmd_ui);
	afx_msg void OnShowBalloons();
	afx_msg void OnUpdateShowBalloons(CCmdUI* cmd_ui);
	afx_msg void OnUpdateShowOptions(CCmdUI* cmd_ui);
	afx_msg void OnShowTags();
	afx_msg void OnUpdateShowTags(CCmdUI* cmd_ui);
	afx_msg void OnShowMarker();
	afx_msg void OnUpdateShowMarker(CCmdUI* cmd_ui);
	afx_msg void OnViewShowDateTimeLabels();
	afx_msg void OnUpdateViewShowDateTimeLabels(CCmdUI* cmd_ui);
	afx_msg void OnViewShowNoTextLabels();
	afx_msg void OnUpdateViewShowNoTextLabels(CCmdUI* cmd_ui);
	afx_msg BOOL OnTbDropDown(UINT, NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnViewShowTimeLine();
	afx_msg void OnUpdateViewShowTimeLine(CCmdUI* cmd_ui);
	afx_msg void OnTaskImgManip();
	afx_msg void OnUpdateTaskImgManip(CCmdUI* cmd_ui);
	afx_msg void OnCancelSortBySimilarity();
	afx_msg void OnUpdateCancelSortBySimilarity(CCmdUI* cmd_ui);
	afx_msg void OnDateTimeAdjust();
	afx_msg void OnUpdateDateTimeAdjust(CCmdUI* cmd_ui);
	afx_msg void OnViewCustLabels();
	afx_msg void OnUpdateViewCustLabels(CCmdUI* cmd_ui);
	afx_msg void OnEditCustomLabels();
	afx_msg void OnUpdateEditCustomLabels(CCmdUI* cmd_ui);
	afx_msg void OnExtractJpegFromRaws();
	afx_msg void OnUpdateExtractJpegFromRaws(CCmdUI* cmd_ui);
	afx_msg void OnProcessImages();
	afx_msg void OnUpdateProcessImages(CCmdUI* cmd_ui);
	void OnViewShowFileNameExt();
	void OnUpdateViewShowFileNameExt(CCmdUI* cmd_ui);
	//}}AFX_MSG
	void OnTaskSendEMail();
	void OnUpdateSelectionRequired(CCmdUI* cmd_ui);
	void OnUpdateTags(CCmdUI* cmd_ui);
	void OnTagSelected(UINT cmd);
	void OnRateImage(UINT cmd);
	void OnUpdateRates(CCmdUI* cmd_ui);
	void OnSetAsWallpaper();
	void OnUpdateSetAsWallpaper(CCmdUI* cmd);
	void OnFilterPaneSwitch();
	void OnUpdateFilterPaneSwitch(CCmdUI* cmd_ui);
	void FilterPaneSwitch();
	//void OnDefineCustomColumns();
	//void OnUpdateDefineCustomColumns(CCmdUI* cmd_ui);
	void OnRenameImages();
	void OnUpdateRenameImages(CCmdUI* cmd_ui);

	virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()

	CImageList img_list_thumb_;
	CImageList img_list_tile_;
	CImageList img_list_small_thumb_;
	CImageList img_list_tiny_;
	bool read_only_photos_withEXIF_;
	bool recursive_scan_;
	Dib dib_marker_;

	// remove all photos from list ctrl
	void RemoveItems();
	void EmptyList(bool erase_viewer_list= true);

	// remove specified photos
	void RemoveItems(const VectPhotoInfo& remove);

	// draw photo in thumbnail mode
	void DrawThumbnailImage(CDC* dc, CRect img_rect, CRect label_rect, const PhotoInfo& photo, UINT state);

	// draw photo in tile mode
	void DrawTileImage(CDC* dc, CRect img_rect, const PhotoInfo& photo, UINT state);

	// draw photo in small thumbnail mode
	void DrawSmallThumbnailImage(CDC* dc, CRect img_rect, CRect label_rect, const PhotoInfo& photo, UINT state);

private:
	// current path
	FolderPathPtr current_folder_;
	// photos (storage)
	PhotoInfoStorage all_photos_;	// all images open so far; scanning thread keeps on adding new ones here
	VectPhotoInfo visible_photos_;	// images in physical order; only those currently visible (after filtering if any)
	VectPhotoInfo sorted_photos_;	// visible images in sorted order (sorted by img attribute like path, name, time, etc.)
	VectPhotoInfo viewer_photos_;	// images in a viewer
	// parent frame window
	BrowserFrame* frame_;
	// view mode
	ViewMode view_mode_;
	// grouping mode
	GroupMode grouping_mode_;
	// grouping mode prior to entering 'sorted by similarity' mode
	GroupMode old_grouping_;
	// vector of selected columns for detailed view
	std::vector<uint16> columns_;
	// photo info in details view mode
	Columns& cols_;
	// pointer to photo reloading thread
	AutoPtr<ReloadJob> reload_thread_;
	// last synchro time, to avoid synchronizing list ctrl for every single photo
	DWORD last_sync_time_;
	// list ctrl used to display photos
	PhotoCtrl photo_ctrl_;
	// list of tags; used only as a source of unique identifiers for PhotoCtrl tag groups
	PhotoTagsCollection tags_;
	// toolbar -------------------------------
	ExifViewReBar tool_bar_wnd_;
	SliderCtrlEx thumb_size_wnd_;

	int img_size_index_;		// thumbnail size--index to the static vector of sizes
	int images_across_index_;	// no of images horizontally in preview mode--index to the static vector

	// distribution of currently selected attribute (like date or shutter speed, etc.)
	DistributionBar distribution_bar_wnd_;

	// tab controls to switch between: "all images", "filter images", and individual custom filters
	CustomTabCtrl tab_ctrl_;
	FilterPanelDlg filter_dlg_;
	int filter_dlg_width_;

	// number of photos synchronized so far; scanning thread keeps on adding images to all_photos_,
	// main thread has seen synched_item_count_ of them
	size_t synched_item_count_;

	// range of dates for histogram (time line bar)
	DateTime hist_date_from_, hist_date_to_;
	DateTime sel_hist_date_from_, sel_hist_date_to_;

	// change display mode (details, thumbnails, ...)
	void ModeSwitch(ViewMode view_mode);

	enum ViewerType { ALL_PHOTOS, SELECTED_PHOTOS, TAGGED_PHOTOS };
	ViewerType current_viewer_type_;
	WndConnection<ViewerDlg> viewer_link_;

	// open separate viewer window and display photo
	void ViewPhoto(PhotoInfoPtr photo, ViewerType type);

	// display photo in preview pane
	//void PreviewPhoto(PhotoInfoPtr photo, bool force_reload= false);

	// returns true if there is at least one item selected
	bool Selection();

	// returns true if there is exactly one item selected
	bool SingleSelection();

	// return vector of all selected photos
	void GetSelectedPhotos(VectPhotoInfo& selected);

	// return true if there's any photos available in photo ctrl
	bool PhotosPresent() const;

	// return vector of all tagged photos
	void GetTaggedPhotos(VectPhotoInfo& tagged);

	// new photo loaded by reload threat
	LRESULT OnPhotoLoaded(WPARAM count, LPARAM progress);

	// synchronize list ctrl with available images
	void SyncListCtrl(const size_t count);

	// load photos from given folder
	bool ReloadPhotos(FolderPathPtr path);

	friend class SortPhotosByAttrib;

	// if true photos with tags are displayed in separate groups
	bool tagged_photos_in_groups_;
	bool rated_photos_in_groups_;
	GroupByDate date_grouping_;	// granularity of grouping by date

	// sorting column (1..N for ascending order, -N..-1 for descending)
	int sort_by_;
	// secondary key for sorting (if not 0)
	int secondary_sorting_column_;

	// sort by column
	void SortPhotosByColumn(int column);

	// sort by image similarity
	void SortBySimilarity();

	// force sorting (by sort_by_ column)
	void ResortItems(bool reset_groups= false);

	// helper fn: refresh one group of items with tags
	void AddTaggedPhotos(VectPhotoInfo::iterator begin, VectPhotoInfo::iterator end,
		PhotoInfoPtr photo, size_t index);

	// helper fn: remove one item
	void RemoveItem(PhotoInfoPtr photo, bool remove_from_ctrl, bool notify);

	// populate popup menu with visible column names (so the sorting order can be changed)
	void PopulateSortPopupMenu(CMenu& popup, UINT first_cmd_it);

	static CDC info_dc_;

	CBitmap arrow_down_;	// two bitmaps displayed in a popup
	CBitmap arrow_up_;		// manu with sorting options
	Path cur_path_;		// current path
	String selected_file_;	// when user drops single file into ExifPro, viewer window will be opened showing this image

	DirectoryChangeWatcher dir_watcher_;	// detect changes in the scanned folder to update list of photos even after scanning's done
	void OnFileChangeDetected(const TCHAR* path, DirectoryChangeWatcher::Change change, const TCHAR* new_path);
	void OnDirChangeDetected(const TCHAR* path, DirectoryChangeWatcher::Change change, const TCHAR* new_path);

	//-------------------------------------------------------------------------

	// prepare colors for list view; fill background and frame
//	void SetColors(CDC* dc, const CRect& img_rect, UINT state, bool frame= true);

	// copy/move operation
	void FileOperation(bool copy);

	friend class CViewSettings;

	class CViewSettings
	{
	public:
		CViewSettings(ViewMode view_mode, GroupMode grouping, int sorting_order, bool recursive_scan,
			bool photos_with_exif_only, const std::vector<uint16>& columns);
		CViewSettings();
		~CViewSettings();

		bool Store(const TCHAR* section, const TCHAR* entry);
		bool Restore(const TCHAR* section, const TCHAR* entry);

		ViewMode GetViewMode() const	{ return data_ ? static_cast<ExifView::ViewMode>(data_->view_mode_) : VIEW_MODE_THUMBNAILS; }
		GroupMode GetGrouping() const	{ return data_ ? static_cast<ExifView::GroupMode>(data_->grouping_) : GROUP_BY_FOLDERS; }
		int GetSortOrder() const		{ return data_ ? data_->sorting_order_ : 1; }
		bool GetRecursiveScan() const	{ return data_ ? !!data_->recursive_scan_ : false; }
		bool GetReadPhotosWithEXIFOnly() const	{ return data_ ? !!data_->read_photos_withEXIF_only_ : false; }
		int GetColCount() const			{ return data_ ? data_->count_ : 0; }
		int GetColIndex(int index) const { return data_ ? data_->columns_[index] : 0; }

	private:
		struct Data
		{
			Data();
			static size_t Size(size_t count);

			uint16 version_;
			uint16 view_mode_;
			uint16 grouping_;
			int16  sorting_order_;
			uint8  recursive_scan_;
			uint8  read_photos_withEXIF_only_;
			uint16 count_;
			uint16 columns_[1];
		};
		Data* data_;
		const std::vector<uint16>* pv_columns_;
		std::vector<BYTE> buffer_;
		BYTE* byte_data_;
	};

	Profile<int> profile_thumb_size_;
	Profile<bool> profile_sticky_selection_;
	Profile<bool> profile_group_by_tags_;
	Profile<bool> profile_group_by_stars_;
	Profile<bool> profile_show_balloons_;
	Profile<bool> profile_show_tags_;
	Profile<bool> profile_show_marker_;
	Profile<bool> profile_show_time_line_;
	Profile<int> profile_show_labels_;
	// type of text labels for photos
	enum { NO_LABELS, SHOW_DATE_TIME, SHOW_FILE_NAME, CUSTOM_INFO, SHOW_FILE_NAME_EXT } show_labels_;
	Profile<int> profile_images_across_;	// images accross the window in 'preview' mode (index)
	// column indices to use when composing a label (for two display modes)
	std::vector<uint16> customInfoFields_[2];
	Profile<std::vector<uint16> > fieldSelectionProfile1_;
	Profile<std::vector<uint16> > fieldSelectionProfile2_;
//	Profile<std::vector<CustomColumnDef> > custom_columns_settings_;
	Profile<bool> filter_panel_expanded_;
	Profile<int> filter_panel_height_;
	Profile<int> filter_panel_flags_;

	// change thumbnail size or preview image size
	void ChangeImageSize(int index);

	int GetCurrSliderIndex() const;
	int GetCurrSliderMax() const;

	void Resize();

	// concatenate path of given photos using '\0' separator
	//void ConcatenatedPath(const VectPhotoInfo& photos, String& paths);

	// remove selected photos from list ctrl and list of photos
	void RemoveSelected(const VectPhotoInfo& selected);

	// add new photo to list ctrl preserving current order
	void AddItem(PhotoInfoPtr photo);

	// helper fn: remove common part of two paths
	String FolderName(const Path& base_path, const Path& folder);

	// select columns for detailed view
	void SelectColumns(std::vector<uint16>& sel);

	// helper fn: find position of given photo in sort_order_
	VectPhotoInfo::iterator FindInsertPos(PhotoInfoPtr photo);

	//
	void CreatePaneImage(CImageList& img_list_dragged_photos, CSize photo_size, PhotoInfoPtr photo);

	// set photo ctrl & preview ctrl colors based on profile settings
	void SetPhotoCtrlColors();

	// set viewer colors based on profile settings
	void SetViewerColors();

	// notifications from PhotoCtrl ---------------------------------
	virtual void DoDragDrop(VectPhotoInfo& selected_photos);
	virtual void SelectionChanged(VectPhotoInfo& selected_photos);
	virtual void GetItemText(PhotoInfoPtr photo, int column, String& buffer);
	virtual void ItemDblClicked(PhotoInfoPtr photo);
	virtual void CurrentItemChanged(PhotoInfoPtr photo);
	virtual void ColumnRClick(int col_index);
	virtual void ContextMenu(CPoint mouse, bool mouse_click, int group_id, int group_elem);
	virtual void KeyDown(UINT chr, UINT rep_cnt, UINT flags);
	virtual bool ShowPhotoMarker(int file_type);
	virtual bool ShowNoExifIndicator(int file_type);
	virtual String GetToolTipText(PhotoInfoPtr photo);
	virtual String GetItemLabel(PhotoInfoPtr photo, CDC& dc, int label_space);
	virtual int GetLabelLines(PhotoCtrl::Mode mode) const;
	virtual Dib* RequestImage(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available);
	virtual Dib* RequestThumbnail(PhotoInfoPtr photo, CSize size_image, bool return_if_not_available);
	virtual AutoPtr<Dib> CorrectImageColors(PhotoInfoPtr photo, Dib* bmp);
	virtual void ContentScrolled(Shift dir);
	virtual bool OnChar(UINT chr, UINT rep_cnt, UINT flags);
	virtual void SortByColumn(int column, bool secondary_key);
	bool window_scrolled_down_;

	// contextual menu
	void ContextMenu(CPoint pos, const String& path);

	//
	void CopyTaggedPhotos(VectPhotoRanges& tagged_photos, Path dest_folder, bool create_separate_folders);
	String TagNameToFolderName(const String& tag);

	void OnViewPhotosPopupMenu(CPoint pos);
	void OnShowOptionsPopupMenu(CPoint pos);

	// color vs shape options
	void OnSortBySimilarityOptions(CPoint pos);
	virtual void SetColorVsShapeWeight(float weight);
	float color_vs_shape_weight_;
	Profile<float> profile_color_vs_shape_weight_;

	// notifications from status bar --------------------------------
	virtual void FieldSelectionChanged();
	virtual void PopupMenu(bool displayed);
	virtual void StatusPaneClick(const CRect& rect);

	// notifications from BrowserFrame via PaneWnd interface ---------
	virtual void OptionsChanged(OptionsDlg& dlg);
	virtual void SaveSettings();
	virtual void OpenCurrentPhoto();
	virtual void ActivatePane(bool active);
	virtual void ActiveMainWnd();
	virtual void UpdateSortOrderPopup(CMenu& popup);
	virtual void ChangeSortOrder(UINT cmd_id);
	virtual void CaptionHeightChanged(bool big);

	// notifications from ViewerDlg --------------------------------
	virtual void CurrentChanged(PhotoInfoPtr photo);
	virtual void PhotoDeleted(PhotoInfoPtr photo);
	virtual void PhotoOrientationChanged(PhotoInfoPtr photo);
	virtual void SelectionRequest(const VectPhotoInfo& selected);
	virtual void FileOperationRequest(const VectPhotoInfo& photos, ViewerDlgNotifications::FileOper oper, CWnd* parent);
	virtual void SelectAndDelete(const VectPhotoInfo& photos);
	virtual void AnimationStarts();

	// notification from CPanes
	virtual void CurrentModified(PhotoInfoPtr photo);

	// queue of photos waiting for a decoder thread to process and reloading service
	ImageLoader image_loader_;
	ImageLoader thumbnail_loader_;

	void ImgReloaded(PhotoInfoPtr photo);
	void StartDecodingNextPhoto();
	void PopulateQueue(bool scrolled_down);
	void PopulateThumbnailQueue(bool scrolledDown);

	//void DoFiltering();

	// color correction
	void ResetICC();
	ICMTransform transform_;
	bool icc_;

	// change current photo (callback for an image description tool)
	void PhotoSelected(PhotoInfo& photo);

	int GetImageCount() const;

	void DeletePhotos(bool warn);

	void OnBuildCatalogHere();
	String build_catalog_path_;

	void OnRenameFile();
	void OnUpdateRenameFile(CCmdUI* cmd_ui);
	void RenameFile(PhotoInfoPtr photo, CWnd* parent);

	// open viewer if single photo was selected?
	bool CheckViewerStart(bool loading_done);

	// low level item remove
	void RemoveItemsWorker(const VectPhotoInfo& remove, bool removed_are_selected);

	//
	void OnSelChangeTab(NMHDR* nmhdr, LRESULT* result);

	// helper to create name of the folder group (film roll)
	String FormatFolderName(PhotoInfoPtr photo);

	// notification handlers
	void OnTagsApplied(const VectPhotoInfo& photos);
	void OnMetadataChanged(const VectPhotoInfo& photos);
	void OnRatingApplied(const VectPhotoInfo& photos);

	//--------------------------------- directory change notifications -------------------------------
	void StartDirectoryWatching();
	void StopDirectoryWatching();
	PhotoInfoStorage add_new_items_;	// newly discovered photos (thru folder change notification)
	VectPhotoInfo remove_old_items_;	// items that have been removed (discovered as above)
	VectPhotoInfo rename_items_;		// items that have been renamed (discovered as above)
	SmartPhotoPtr ScanPhoto(const TCHAR* path);

	//--------------------------------- image filtering ----------------------------------------------
	void OnFilterChanged();
	void ApplyFilter(const FilterData& filter);
	void FilterSet(const VectPhotoInfo& input, const FilterData& filter, VectPhotoInfo& output);
	FilterData* GetFilterData(size_t index);
	const FilterData* GetCurrentFilterData() const;
	FilterData* GetCurrentFilterData();
	void GetCurrentFilter(FilterData& filter);

	FilterData main_page_;		// main tab (all images; filtering works here too)
	FilterData filter_page_;	// filtering pane where new filter is applied/defined
	CustomFilters filters_;		// user-defined filters

	// filter related notifications
	virtual void FilterParamsChanged();	// notification: filter params have been modified
	//virtual void StoreFilter();			// store current filter
	//virtual void DeleteFilter();		// delete current filter
	//virtual void UpdateFilter();		// update current filter
	//virtual void NameChanged();			// filter name changed

	// controls sending current image and selection change notifications
	int notifications_enabled_counter_;

	ReloadProgress reload_progress_event_;
	FilterChanged filter_changed_event_;

	int GetCurFilterTab() const;
	void SetCurFilterTab(int tab);
	int filter_tab_ = 0;
};


inline ExifView::ViewMode operator++ (ExifView::ViewMode& v)
{
	switch (v)
	{
	case ExifView::VIEW_MODE_THUMBNAILS:	return v = ExifView::VIEW_MODE_DETAILS;
	case ExifView::VIEW_MODE_DETAILS:		return v = ExifView::VIEW_MODE_TILES;
	case ExifView::VIEW_MODE_TILES:			return v = ExifView::VIEW_MODE_PICTURES;
	case ExifView::VIEW_MODE_PICTURES:		return v = ExifView::VIEW_MODE_THUMBNAILS;

	default:
		ASSERT(false);
	}
	return ExifView::VIEW_MODE_THUMBNAILS;
}
