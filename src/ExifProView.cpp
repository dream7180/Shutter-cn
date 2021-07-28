/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifProView.cpp : implementation of the ExifView class
//

#include "stdafx.h"
#include "resource.h"
#include "ExifProView.h"
#include "BrowserFrame.h"
#include "PhotoVector.h"
#include "ItemIdList.h"

#include "MainFrm.h"
#include "MemoryDC.h"
#include "Color.h"
#include "ProfileVector.h"
#include "GetDefaultLineHeight.h"
#include "OptionsDlg.h"
#include "CopyMoveDlg.h"

#include "HistogramDlg.h"
#include "UniqueLetter.h"
#include "Tasks.h"
#include "Config.h"
#include "HeaderDialog.h"
#include "DibCache.h"

#include "CatchAll.h"
#include "BalloonMsg.h"
#include "DrawFields.h"
#include "clamp.h"

#include "DibColorTransform.h"
#include "RMenu.h"

#include "ImgDb.h"
#include "Block.h"

#include "ImageScanner.h"

#include "FileRenameDlg.h"
#include "ImageLabelDlg.h"
#include "EditFileInfo.h"
#include "TagsBarCommon.h"
#include "TagsCommonCode.h"

#include "boost/bind.hpp"

#include "LoadErrorsDlg.h"
#include "CreateDecoderJob.h"

#include "WhistlerLook.h"
#include "PhotoFactory.h"
#include "ImgScanner.h"

#include "ExprCalculator.h"
#include "CustomFilter.h"

#include "PhotoCollection.h"
#include "DeleteConfirmationDlg.h"

#include "DateTimeUtils.h"

#include "UIElements.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern const TCHAR* x_stristr(const TCHAR* source, const TCHAR* search);
extern void BuildCatalog(const TCHAR* path);

extern AutoPtr<DibCache> global_dib_cache;
extern AutoPtr<PhotoCache> global_photo_cache;

#define MULTI_GROUPS

static const int FIXED_TABS= 2;
static const size_t MAX_TABS= 1000;

/////////////////////////////////////////////////////////////////////////////
// ExifView

BEGIN_MESSAGE_MAP(ExifView, PaneWnd)
	ON_COMMAND(ID_VIEW, OnView)
	ON_UPDATE_COMMAND_UI(ID_VIEW, OnUpdateView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SORT, OnUpdateViewSort)
	ON_COMMAND(ID_VIEW_MODE, OnViewMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODE, OnUpdateViewMode)
	ON_COMMAND(ID_VIEW_THUMBNAILS, OnViewThumbnails)
	ON_COMMAND(ID_VIEW_DETAILS, OnViewDetails)
	ON_COMMAND(ID_VIEW_TILES, OnViewTiles)
	ON_COMMAND(ID_VIEW_PICTURES, OnViewPictures)
	ON_UPDATE_COMMAND_UI(ID_VIEW_THUMBNAILS, OnUpdateViewThumbnails)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DETAILS, OnUpdateViewDetails)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TILES, OnUpdateViewTiles)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PICTURES, OnUpdateViewPictures)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_RECURSIVE, OnRecursive)
	ON_UPDATE_COMMAND_UI(ID_RECURSIVE, OnUpdateRecursive)
	ON_COMMAND(ID_READ_ONLY_EXIF, OnReadOnlyExif)
	ON_UPDATE_COMMAND_UI(ID_READ_ONLY_EXIF, OnUpdateReadOnlyExif)
	ON_COMMAND(ID_REFRESH, OnRefresh)
	ON_UPDATE_COMMAND_UI(ID_REFRESH, OnUpdateRefresh)
	ON_COMMAND(ID_STOP_SCANNING, OnStopScanning)
	ON_COMMAND(ID_MARK_IMAGE, OnMarkImage)
	ON_UPDATE_COMMAND_UI(ID_MARK_IMAGE, OnUpdateMarkImage)
	ON_COMMAND(ID_MARK_ALL, OnMarkAll)
	ON_UPDATE_COMMAND_UI(ID_MARK_ALL, OnUpdateMarkAll)
	ON_COMMAND(ID_MARK_NONE, OnMarkNone)
	ON_UPDATE_COMMAND_UI(ID_MARK_NONE, OnUpdateMarkNone)
	ON_COMMAND(ID_MARK_INVERT, OnMarkInvert)
	ON_UPDATE_COMMAND_UI(ID_MARK_INVERT, OnUpdateMarkInvert)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_TASK_DELETE, OnUpdateDelete)
	ON_COMMAND(ID_TASK_DELETE, OnTaskDelete)
	ON_COMMAND(ID_TASK_COPY, OnTaskCopy)
	ON_COMMAND(ID_TASK_MOVE, OnTaskMove)
	ON_COMMAND(ID_TASK_RESIZE, OnTaskResize)
	ON_COMMAND(ID_TASK_GEN_SLIDE_SHOW, OnTaskGenSlideShow)
	ON_UPDATE_COMMAND_UI(ID_TASK_GEN_SLIDE_SHOW, OnUpdateSelectionRequired)
	ON_COMMAND(ID_TASK_HISTOGRAM, OnTaskHistogram)
	ON_UPDATE_COMMAND_UI(ID_NEXT_PANE, OnUpdateNextPane)
	ON_COMMAND(ID_TASK_EDIT_DESC, OnTaskEditDesc)
	ON_UPDATE_COMMAND_UI(ID_TASK_EDIT_DESC, OnUpdateTaskEditDesc)
	ON_UPDATE_COMMAND_UI(ID_ESCAPE, OnUpdateEscape)
	ON_COMMAND(ID_SORT_BY_SIMILARITY, OnSortBySimilarity)
	ON_UPDATE_COMMAND_UI(ID_SORT_BY_SIMILARITY, OnUpdateSortBySimilarity)
	ON_COMMAND(ID_VIEW_THUMB_SMALLER, OnViewThumbSmaller)
	ON_UPDATE_COMMAND_UI(ID_VIEW_THUMB_SMALLER, OnUpdateViewThumbSmaller)
	ON_COMMAND(ID_VIEW_THUMB_BIGGER, OnViewThumbBigger)
	ON_UPDATE_COMMAND_UI(ID_VIEW_THUMB_BIGGER, OnUpdateViewThumbBigger)
	ON_WM_HSCROLL()
	ON_COMMAND(ID_GROUP_FOLDER, OnGroupByFolders)
	ON_UPDATE_COMMAND_UI(ID_GROUP_FOLDER, OnUpdateGroupByFolders)
	ON_COMMAND(ID_GROUP_TAGS, OnGroupByTags)
	ON_UPDATE_COMMAND_UI(ID_GROUP_TAGS, OnUpdateGroupByTags)
	ON_COMMAND(ID_GROUP_STARS, OnGroupByStars)
	ON_UPDATE_COMMAND_UI(ID_GROUP_STARS, OnUpdateGroupByStars)
	ON_COMMAND(ID_GROUP_DATE, OnGroupByDate)
	ON_UPDATE_COMMAND_UI(ID_GROUP_DATE, OnUpdateGroupByDate)
	ON_COMMAND(ID_GROUP_FLAT, OnNoGrouping)
	ON_UPDATE_COMMAND_UI(ID_GROUP_FLAT, OnUpdateNoGrouping)
	ON_COMMAND(ID_TASK_GEN_HTML_ALBUM, OnTaskGenHTMLAlbum)
	ON_UPDATE_COMMAND_UI(ID_TASK_GEN_HTML_ALBUM, OnUpdateSelectionRequired)
	ON_COMMAND(ID_TASK_EXPORT, OnTaskExport)
	ON_UPDATE_COMMAND_UI(ID_TASK_EXPORT, OnUpdateTaskExport)
	ON_COMMAND(ID_TASK_ROTATE, OnTaskRotate)
	ON_UPDATE_COMMAND_UI(ID_TASK_ROTATE, OnUpdateTaskRotate)
	ON_COMMAND(ID_STICKY_SELECTION, OnStickySelection)
	ON_UPDATE_COMMAND_UI(ID_STICKY_SELECTION, OnUpdateStickySelection)
	ON_COMMAND(ID_TASK_EDIT_IPTC, OnEditIPTC)
	ON_UPDATE_COMMAND_UI(ID_TASK_EDIT_IPTC, OnUpdateEditIPTC)
	ON_UPDATE_COMMAND_UI(ID_STOP_SCANNING, OnUpdateStopScanning)
	ON_COMMAND(ID_TASK_PRINT, OnTaskPrint)
	ON_COMMAND(ID_TASK_PRINT_THUMBNAILS, OnTaskPrintThumbnails)
	ON_UPDATE_COMMAND_UI(ID_TASK_PRINT, OnUpdateTaskPrint)
	ON_UPDATE_COMMAND_UI(ID_TASK_PRINT_THUMBNAILS, OnUpdateTaskPrintThumbnails)
	ON_COMMAND(ID_TASK_COPY_TAGGED, OnTaskCopyTagged)
	ON_UPDATE_COMMAND_UI(ID_TASK_COPY_TAGGED, OnUpdateTaskCopyTagged)
	ON_COMMAND(ID_VIEW_ALL, OnViewAll)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ALL, OnUpdateViewAll)
	ON_COMMAND(ID_VIEW_SELECTED, OnViewSelected)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECTED, OnUpdateViewSelected)
	ON_COMMAND(ID_VIEW_TAGGED, OnViewTagged)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TAGGED, OnUpdateViewTagged)
	ON_COMMAND(ID_OPEN_IN_EXPLORER, OnOpenInExplorer)
	ON_UPDATE_COMMAND_UI(ID_OPEN_IN_EXPLORER, OnUpdateOpenInExplorer)
	ON_COMMAND(ID_OPEN_PHOTO, OnOpenPhoto)
	ON_UPDATE_COMMAND_UI(ID_OPEN_PHOTO, OnUpdateOpenPhoto)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_TASK_COPY, OnUpdateTaskCopy)
	ON_UPDATE_COMMAND_UI(ID_TASK_MOVE, OnUpdateTaskMove)
	ON_UPDATE_COMMAND_UI(ID_TASK_RESIZE, OnUpdateTaskResize)
	ON_UPDATE_COMMAND_UI(ID_TASK_HISTOGRAM, OnUpdateTaskHistogram)
	ON_COMMAND(ID_ESCAPE, OnEscape)
	ON_COMMAND(ID_VIEW_SHOW_BALLOONS, OnShowBalloons)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_BALLOONS, OnUpdateShowBalloons)
	ON_UPDATE_COMMAND_UI(ID_SHOW_OPTIONS, OnUpdateShowOptions)
	ON_COMMAND(ID_VIEW_SHOW_TAGS, OnShowTags)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_TAGS, OnUpdateShowTags)
	ON_COMMAND(ID_VIEW_SHOW_MARKER, OnShowMarker)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_MARKER, OnUpdateShowMarker)
	ON_COMMAND(ID_VIEW_SHOW_TIMELINE, OnViewShowTimeLine)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_TIMELINE, OnUpdateViewShowTimeLine)

	ON_COMMAND(ID_VIEW_SHOW_LABELS, OnViewShowLabels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_LABELS, OnUpdateViewShowLabels)
	ON_COMMAND(ID_VIEW_SHOW_DATETIME, OnViewShowDateTimeLabels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_DATETIME, OnUpdateViewShowDateTimeLabels)
	ON_COMMAND(ID_VIEW_SHOW_NO_TEXT_LABEL, OnViewShowNoTextLabels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_NO_TEXT_LABEL, OnUpdateViewShowNoTextLabels)
	ON_COMMAND(ID_SHOW_CUST_INFO, OnViewCustLabels)
	ON_UPDATE_COMMAND_UI(ID_SHOW_CUST_INFO, OnUpdateViewCustLabels)
	ON_COMMAND(ID_CUSTOMIZE_LABELS, OnEditCustomLabels)
	ON_UPDATE_COMMAND_UI(ID_CUSTOMIZE_LABELS, OnUpdateEditCustomLabels)
	ON_COMMAND(ID_VIEW_SHOW_FILE_NAME_EXT, OnViewShowFileNameExt)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_FILE_NAME_EXT, OnUpdateViewShowFileNameExt)

	ON_COMMAND(ID_TASK_TOUCH_UP, OnTaskImgManip)
	ON_UPDATE_COMMAND_UI(ID_TASK_TOUCH_UP, OnUpdateTaskImgManip)

	ON_COMMAND(ID_CANCEL_SORT_BY_SIMILARITY, OnCancelSortBySimilarity)
	ON_UPDATE_COMMAND_UI(ID_CANCEL_SORT_BY_SIMILARITY, OnUpdateCancelSortBySimilarity)

	ON_COMMAND(ID_SEND_EMAIL, OnTaskSendEMail)
	ON_UPDATE_COMMAND_UI(ID_SEND_EMAIL, OnUpdateSelectionRequired)

	ON_COMMAND(ID_BUILD_CATALOG_2, OnBuildCatalogHere)

//	ON_COMMAND(ID_CUSTOMIZE_IMG_LABEL, OnCustomizeImageLabel)

	ON_COMMAND(ID_DATE_TIME_ADJ, OnDateTimeAdjust)
	ON_UPDATE_COMMAND_UI(ID_DATE_TIME_ADJ, OnUpdateDateTimeAdjust)

	ON_COMMAND(ID_TASK_EXTRACT_JPEG, OnExtractJpegFromRaws)
	ON_UPDATE_COMMAND_UI(ID_TASK_EXTRACT_JPEG, OnUpdateExtractJpegFromRaws)
#ifdef _DEBUG
	ON_COMMAND(ID_PROCESSOR, OnProcessImages)
	ON_UPDATE_COMMAND_UI(ID_PROCESSOR, OnUpdateProcessImages)
#endif
	//ON_COMMAND(ID_DEFINE_CUSTOM_COLUMNS, OnDefineCustomColumns)
	//ON_UPDATE_COMMAND_UI(ID_DEFINE_CUSTOM_COLUMNS, OnUpdateDefineCustomColumns)


	// Standard printing commands
//	ON_COMMAND(ID_FILE_PRINT_DIRECT, PaneWnd::OnFilePrint)
//	ON_COMMAND(ID_FILE_PRINT_PREVIEW, PaneWnd::OnFilePrintPreview)

	ON_MESSAGE(ReloadJob::RELOADING_THREAD_MSG, OnPhotoLoaded)
	ON_NOTIFY_EX_RANGE(TBN_DROPDOWN, AFX_IDW_CONTROLBAR_LAST - 3, AFX_IDW_CONTROLBAR_LAST, OnTbDropDown)

	ON_UPDATE_COMMAND_UI_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + MAX_TAGS - 1, OnUpdateTags)
	ON_COMMAND_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + MAX_TAGS - 1, OnTagSelected)

	ON_COMMAND_RANGE(ID_RATE_NONE, ID_RATE_5_STARS, OnRateImage)
	ON_UPDATE_COMMAND_UI_RANGE(ID_RATE_NONE, ID_RATE_5_STARS, OnUpdateRates)

	ON_COMMAND(ID_SET_WALLPAPER, OnSetAsWallpaper)
	ON_UPDATE_COMMAND_UI(ID_SET_WALLPAPER, OnUpdateSetAsWallpaper)

	ON_NOTIFY(CTCN_SELCHANGE, IDC_TAB, OnSelChangeTab)

	ON_COMMAND(ID_FILTER_SWITCH, OnFilterPaneSwitch)
	ON_UPDATE_COMMAND_UI(ID_FILTER_SWITCH, OnUpdateFilterPaneSwitch)

	ON_COMMAND(ID_RENAME_FILE, OnRenameFile)		// rename single file (simple edit box)
	ON_UPDATE_COMMAND_UI(ID_RENAME_FILE, OnUpdateRenameFile)

	ON_COMMAND(ID_TASK_RENAME, OnRenameImages)		// batch file rename (rule-based)
	ON_UPDATE_COMMAND_UI(ID_TASK_RENAME, OnUpdateRenameImages)
END_MESSAGE_MAP()


static const int thumb_size_levels[]=
{
	// it is silly to spell that out; it can be generated: this is simple geometrical progression (%4)
	32, 33, 34, 35, 37, 38, 40, 42, 43, 45, 47, 49, 51, 53, 55, 58, 60, 63, 65, 68, 71, 74,
	77, 80, 83, 87, 90, 94, 98, 102, 106, 111, 115, 120, 125, 130, 136, 142, 147, 154, 160,
//#ifdef _WIN64
	166, 173, 180, 187, 195, 202, 211, 219, 228, 237
//#endif
};

static const int images_across_levels[]= { 4, 3, 2, 1 };


namespace {
//	const int g_LIST_CTRL_ID= 1000;
	const int g_LIST_CTRL_ID2= 1001;

//	const CSize g_BIG_THUMB_size(160, 160);
//	const CSize g_SMALL_THUMB_size(80, 70);
//	const CSize g_TILE_THUMB_size(80, 80);
//	const CSize g_TILE_IMG_size(212, 84);

	const TCHAR REGISTRY_ENTRY_EXIF[]	= _T("Exif");
	const TCHAR REGISTRY_ENTRY_EXIF_FLTR[]	= _T("Exif\\FilterPanel");

	const TCHAR REG_VIEW_SETTINGS[]		= _T("Settings");
	const TCHAR REG_VIEW_COL_ORDER[]	= _T("ColumnOrder");
	const TCHAR REG_VIEW_COL_WIDTH[]	= _T("ColumnWidth");
	const TCHAR REG_COLUMN_SELECTED[]	= _T("SelectedColumns");
	const TCHAR REG_CUSTOM_FILTERS[]	= _T("CustomFilters");


struct BlockNotifications
{
	BlockNotifications(ExifView& view) : view_(view)
	{
		view_.EnableNotification(false);
		call_enable_ = true;
	}

	void Unblock()
	{
		if (call_enable_)
		{
			view_.EnableNotification(true);
			call_enable_ = false;
		}
	}

	virtual ~BlockNotifications()
	{
		Unblock();
	}

private:
	ExifView& view_;
	bool call_enable_;
};


struct SaveCurrentImage : protected BlockNotifications
{
	SaveCurrentImage(ExifView& view) : BlockNotifications(view), view_(view)
	{
		PhotoCtrl& ctrl= view_.GetListCtrl();
		current_ = ctrl.GetCurrentItem();
		ctrl.EnableSettingCurrentItem(false);
//		view_.EnableNotification(false);
	}

	virtual ~SaveCurrentImage()
	{
		try
		{
			Unblock();	// unblock notifications

			PhotoCtrl& ctrl= view_.GetListCtrl();

			ctrl.EnableSettingCurrentItem(true);
			// restore current photo & notify clients
			PhotoInfoPtr current= current_;
			if (!ctrl.HasItem(current))
				current = ctrl.GetItem(PhotoCtrl::FIRST_VISIBLE);
			ctrl.SetCurrentItem(current, false, true, true);
		}
		CATCH_ALL_W(&view_)
	}

private:
	ExifView& view_;
	PhotoInfoPtr current_;
};


struct SaveSelection : SaveCurrentImage
{
	SaveSelection(ExifView& view) : SaveCurrentImage(view), view_(view)
	{
		// current image already captured by save_current_

		PhotoCtrl& ctrl= view_.GetListCtrl();

		sel_all_ = false;

		if (ctrl.GetSelectedCount() == ctrl.GetItemCount())
			sel_all_ = true;
		else
			ctrl.GetSelectedItems(selected_);
	}

	~SaveSelection()
	{
		try
		{
			Unblock();	// unblock notifications

			PhotoCtrl& ctrl= view_.GetListCtrl();

			if (sel_all_)
				ctrl.SelectItems(PhotoCtrl::ALL, true);
			else
				ctrl.SelectItems(selected_, true);

			// note that save_current_'s destructor will execute next
		}
		CATCH_ALL_W(&view_)
	}

private:
	ExifView& view_;
	bool sel_all_;
	VectPhotoInfo selected_;
};

} // namespace


/////////////////////////////////////////////////////////////////////////////
// ExifView construction/destruction

static void CacheThumbnail(DecoderJobInterface* decoder, ImageStat status)
{
	// thumbnail has been decoded; this fn call comes in the main thread

	if (decoder == 0)
		return;

	//PhotoInfoPtr photo= const_cast<PhotoInfoPtr>(decoder->GetPhoto());
	PhotoInfoPtr photo= ConstCast(decoder->GetPhoto());
	if (photo == 0)
		return;

	photo->thumbnail_stat_ = status;

	DibPtr dib= decoder->GetBitmap();
	if (dib == 0)
		return;

	// cache decoded thumbnail
	photo->PutThumbnail(dib.get(), decoder->GetRequestedSize());
}


ExifView::ExifView(Columns& columns)
 : cols_(columns), image_loader_(boost::bind(&PhotoCache::CacheDecodedImage, global_photo_cache.get(), _1, _2), &CreateImageDecoderJob, &CanPhotoBeDecoded), thumbnail_loader_(&CacheThumbnail, &CreateThumbnailDecoderJob, &CanThumbnailBeDecoded)
{
	read_only_photos_withEXIF_ = false;
	recursive_scan_ = true;
	view_mode_ = VIEW_MODE_THUMBNAILS;
	old_grouping_ = grouping_mode_ = GROUP_BY_FOLDERS;
	frame_ = 0;
	sort_by_ = 1;	// defaults to the date & time
	secondary_sorting_column_ = 0;	// no secondary key
	last_sync_time_ = 0;
	img_size_index_ = array_count(thumb_size_levels) * 2 / 3;
	images_across_index_ = 2;
	tagged_photos_in_groups_ = true;
	rated_photos_in_groups_ = true;
	date_grouping_ = BY_DAY;	// crrently this is fixed and doesn't change
	//show_labels_ = SHOW_DATE_TIME;
	show_labels_ = SHOW_FILE_NAME;
	notifications_enabled_counter_ = 0;

	sorted_photos_.reserve(100);

	synched_item_count_ = 0;

	arrow_down_.LoadBitmap(IDB_ARROW_DOWN);
	arrow_up_.LoadBitmap(IDB_ARROW_UP);

	color_vs_shape_weight_ = 0.5;
	current_viewer_type_ = ALL_PHOTOS;

	window_scrolled_down_ = true;

	// custom label for thumbnail view
	customInfoFields_[0].reserve(2);
	customInfoFields_[0].push_back(COL_PHOTO_NAME);
	customInfoFields_[0].push_back(COL_EXP_TIME);
	// custom label for preview view
	customInfoFields_[1].reserve(3);
	customInfoFields_[1].push_back(COL_PHOTO_NAME);
	customInfoFields_[1].push_back(COL_EXP_TIME);
	customInfoFields_[1].push_back(COL_FNUMBER);

	// names used for first two tabs
	main_page_.name_ = _T("全部图像");
	filter_page_.name_ = _T("过滤图像");

	profile_thumb_size_.Register(REGISTRY_ENTRY_EXIF, _T("ThumbSize"), img_size_index_);
	profile_sticky_selection_.Register(REGISTRY_ENTRY_EXIF, _T("StickySelection"), false);
	profile_group_by_tags_.Register(REGISTRY_ENTRY_EXIF, _T("GroupByTags"), false);
	profile_group_by_stars_.Register(REGISTRY_ENTRY_EXIF, _T("GroupByStars"), false);
	profile_color_vs_shape_weight_.Register(REGISTRY_ENTRY_EXIF, _T("ColorVsShape"), color_vs_shape_weight_);
	profile_show_labels_.Register(REGISTRY_ENTRY_EXIF, _T("ShowLabels"), show_labels_);
	profile_show_balloons_.Register(REGISTRY_ENTRY_EXIF, _T("ShowBalloons"), true);
	profile_show_tags_.Register(REGISTRY_ENTRY_EXIF, _T("ShowTagText"), true);
	profile_show_marker_.Register(REGISTRY_ENTRY_EXIF, _T("ShowMarker"), true);
	profile_show_time_line_.Register(REGISTRY_ENTRY_EXIF, _T("ShowTimeLine"), true);
	profile_images_across_.Register(REGISTRY_ENTRY_EXIF, _T("ImagesAcross"), images_across_index_);
	fieldSelectionProfile1_.Register(REGISTRY_ENTRY_EXIF, _T("FieldSelThumbs"), customInfoFields_[0]);
	fieldSelectionProfile2_.Register(REGISTRY_ENTRY_EXIF, _T("FieldSelPreview"), customInfoFields_[1]);
	filter_panel_expanded_.Register(REGISTRY_ENTRY_EXIF_FLTR, _T("expanded"), true);
	filter_panel_height_.Register(REGISTRY_ENTRY_EXIF_FLTR, _T("height"), 0);
	filter_panel_flags_.Register(REGISTRY_ENTRY_EXIF_FLTR, _T("flags"), 0);

	icc_ = false;

	image_loader_.SetPhotoLoadedCallback(boost::bind(&ExifView::ImgReloaded, this, _1));
	image_loader_.SetQueuePopulateCallback(boost::bind(&ExifView::PopulateQueue, this, boost::ref(window_scrolled_down_)));

	thumbnail_loader_.SetPhotoLoadedCallback(boost::bind(&ExifView::ImgReloaded, this, _1));
	thumbnail_loader_.SetQueuePopulateCallback(boost::bind(&ExifView::PopulateThumbnailQueue, this, boost::ref(window_scrolled_down_)));

	dir_watcher_.SetCallBacks(boost::bind(&ExifView::OnFileChangeDetected, this, _1, _2, _3),
		boost::bind(&ExifView::OnDirChangeDetected, this, _1, _2, _3));

	filter_dlg_width_ = 0;

	PhotoCollection::Instance().ConnectOnTagsChanged(boost::bind(&ExifView::OnTagsApplied, this, _1));
	PhotoCollection::Instance().ConnectOnMetadataChanged(boost::bind(&ExifView::OnMetadataChanged, this, _1));
	PhotoCollection::Instance().ConnectOnRatingChanged(boost::bind(&ExifView::OnRatingApplied, this, _1));
}


ExifView::~ExifView()
{
	dir_watcher_.Stop();
}


BOOL ExifView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!PaneWnd::PreCreateWindow(cs))
		return false;

	if (WhistlerLook::IsAvailable())
		cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ExifView drawing


void ExifView::OnDraw(CDC* dc)
{}


void ExifView::InitialUpdate()
{
	PaneWnd::InitialUpdate();
	LOGFONT lf;
	/*HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	//lf.lfQuality = ANTIALIASED_QUALITY;
	//lf.lfHeight += 1;
	lf.lfQuality = CLEARTYPE_QUALITY;
	_tcscpy(lf.lfFaceName, _T("Microsoft Yahei"));
	hfont = CreateFontIndirectW(&lf);*/
	::GetDefaultGuiFont(lf);
	HFONT hfont = CreateFontIndirectW(&lf);
	SendMessage(WM_SETFONT, WPARAM(hfont));
	//SendMessage(WM_SETFONT, WPARAM(::GetStockObject(DEFAULT_GUI_FONT)));

	frame_->GetStatusBar().SetRecipient(this);

	dib_marker_.Load(IDB_MARKED);

	CWinApp* app= AfxGetApp();

	CViewSettings vs;
	if (vs.Restore(frame_->GetRegSection(), REG_VIEW_SETTINGS))
	{
		int cols= vs.GetColCount();
		if (cols > cols_.MaxCount())
			cols = cols_.MaxCount();
		columns_.resize(cols);
		for (int i= 0; i < cols; ++i)
		{
			int index= vs.GetColIndex(i); // MIN(vs.GetColIndex(i), cols_.MaxCount() - 1);
			GetListCtrl().InsertColumn(i, cols_.ShortName(index), cols_.Alignment(index), cols_.DefaultWidth(index));
			columns_[i] = index;
		}

		// restore column order and widths

		if (CHeaderCtrl* ctrl= GetListCtrl().GetHeaderCtrl())
		{
			int count= ctrl->GetItemCount();

			std::vector<INT> col_order;
			if (GetProfileVector(frame_->GetRegSection(), REG_VIEW_COL_ORDER, col_order) &&
				count == col_order.size())
			{
				GetListCtrl().SetColumnOrderArray(col_order);
			}

			std::vector<int16> width_array;
			if (GetProfileVector(frame_->GetRegSection(), REG_VIEW_COL_WIDTH, width_array) &&
				count == col_order.size() && width_array.size() == count)
			{
				for (int i= 0; i < count; ++i)
					GetListCtrl().SetColumnWidth(i, width_array[i]);
			}
		}

		view_mode_ = vs.GetViewMode();
		grouping_mode_ = vs.GetGrouping();
		if (grouping_mode_ == GROUP_BY_SIMILARITY)
			grouping_mode_ = GROUP_BY_FOLDERS;
		sort_by_ = vs.GetSortOrder();
		GetListCtrl().SetSortColumn(sort_by_);
		recursive_scan_ = vs.GetRecursiveScan();
		read_only_photos_withEXIF_ = vs.GetReadPhotosWithEXIFOnly();

		// restore custom filters
		CString section= REGISTRY_ENTRY_EXIF;
		section += '\\';
		section += REG_CUSTOM_FILTERS;

		const size_t count= std::min<size_t>(MAX_TABS, app->GetProfileInt(section, _T("count"), 0));

		std::vector<String> filters;
		filters.reserve(count);
		for (size_t i= 0; i < count; ++i)
		{
			TCHAR key[200];
			wsprintf(key, _T("f%d-%d"), static_cast<int>(sizeof(TCHAR)), static_cast<int>(i));
			BYTE* data= 0;
			UINT len= 0;
			String s;
			if (app->GetProfileBinary(section, key, &data, &len))
			{
				//TODO: fix possible 'data' leak
				s.assign(reinterpret_cast<TCHAR*>(data), len / sizeof(TCHAR));
				delete [] data;
				filters.push_back(s);
			}
		}

		try
		{
			int version= 2;
			filters_.SerializeFrom(filters, version);
		}
		CATCH_ALL

		// add tabs for custom filters
		for (size_t i= 0; i < count; ++i)
			if (FilterData* filter= filters_.GetFilter(i))
				tab_ctrl_.InsertItem(-1, filter->name_.c_str());
	}
	else	// there is no registry info stored yet; apply default settings
	{
		const int COUNT= 12;//默认列的数量！！！！！！！！！
		columns_.resize(COUNT);
		for (int i= 0; i < COUNT; ++i)
		{
			GetListCtrl().InsertColumn(i, cols_.ShortName(i), cols_.Alignment(i), cols_.DefaultWidth(i));
			columns_[i] = i;
		}
	}

	// restore filtering panels
	{
		const size_t count= filter_dlg_.GetPanelCount();
		for (size_t panel= 0; panel < count; ++panel)
			filter_dlg_.SetPanelUISettings(panel, filter_panel_height_.Value(panel), filter_panel_expanded_.Value(panel), filter_panel_flags_.Value(panel));
	}

	RemoveItems();

	ResetICC();

	ModeSwitch(view_mode_);
}

/////////////////////////////////////////////////////////////////////////////
// ExifView printing
#if 0
BOOL ExifView::OnPreparePrinting(CPrintInfo* info)
{
	// default preparation
	return DoPreparePrinting(info);
}

void ExifView::OnBeginPrinting(CDC* /*dc*/, CPrintInfo* /*info*/)
{
	// TODO: add extra initialization before printing
}

void ExifView::OnEndPrinting(CDC* /*dc*/, CPrintInfo* /*info*/)
{
	// TODO: add cleanup after printing
}
#endif
/////////////////////////////////////////////////////////////////////////////
// ExifView diagnostics

#ifdef _DEBUG
void ExifView::AssertValid() const			{ PaneWnd::AssertValid(); }
void ExifView::Dump(CDumpContext& dc) const	{ PaneWnd::Dump(dc); }
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////

bool ExifView::Browse(FolderPathPtr path)	// scan 'path' for photos
{
	ASSERT(frame_ != 0);

	current_folder_ = path;

	return ReloadPhotos(path);
}


bool ExifView::ReloadPhotos()
{
	return ReloadPhotos(current_folder_); //GetDocument()->GetFolder());
}


bool ExifView::ReloadPhotos(FolderPathPtr path)
{
	if (path == 0)
		return false;

	cur_path_ = path->GetDisplayPath();

//	if (cur_path_.empty())	// there is no path for 'My Computer' and other non-FS objects
//		return false;

	if (reload_thread_.get())	// if there is working reload thread stop it
		reload_thread_->Quit();

	StopDirectoryWatching();

	image_loader_.RemoveAll();
	thumbnail_loader_.RemoveAll();
	//if (decoder_.get())
	//{
	//	decoder_->Quit();
	//	decoder_ = 0;
	//}

	if (viewer_link_.IsValid())
	{
		if (current_viewer_type_ == ALL_PHOTOS)
			viewer_link_->Clean();
		else
			viewer_link_->DestroyWindow();
	}

	RemoveItems();

	//TODO: today filter has to be cleared before reloading images to avoid problems when populating photo ctrl;
	// this is a bug essentially and should be fixed
	ClearFilter();

	distribution_bar_wnd_.ClearHistory();
	distribution_bar_wnd_.BuildHistogram(sorted_photos_, sorted_photos_);
	distribution_bar_wnd_.SetTick(DateTime());
	distribution_bar_wnd_.UpdateWindow();

	all_photos_.clear();
	all_photos_.reserve(1000);

	window_scrolled_down_ = true;	// so speculative load reloads photos properly

	// sorting by similarity is dubious when reloading is in progress...
	OnCancelSortBySimilarity();

	try
	{
		std::auto_ptr<ImageScanner> scanner= path->GetScanner();
		if (scanner.get() == 0)
			return false;	// no scanner for a given path

		// if browsing was started with an image file selected it will be passed here
		selected_file_ = path->GetSelectedFile();

		reload_thread_ = new ReloadJob(all_photos_, scanner, read_only_photos_withEXIF_,
			recursive_scan_, *this, g_Settings.db_file_length_limit_mb_, GetImageDataBase(false, false));
	}
	catch (CException* ex)
	{
		ex->ReportError();
		ex->Delete();
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

// sorting functor: sorting by attributes
//
class SortPhotosByAttrib
{
public:
	SortPhotosByAttrib(ExifView* view) : view_(view)
	{
		col_index_ = 0;
		secondary_index_ = 0;
		fn_ = 0;

		if (view_->sort_by_ == 0)
			fn_ = &SortPhotosByAttrib::SortByTime;
		else
		{
			col_index_ = view_->columns_[abs(view_->sort_by_) - 1];

			if (view_->secondary_sorting_column_ != 0)
			{
				// sorting using secondary key
				secondary_index_ = view_->columns_[abs(view_->secondary_sorting_column_) - 1];

				if (view_->secondary_sorting_column_ > 0)
				{
					if (view_->sort_by_ > 0)
						fn_ = &SortPhotosByAttrib::SortAscendingSndKeyAsc;
					else
						fn_ = &SortPhotosByAttrib::SortDescendingSndKeyAsc;
				}
				else
				{
					if (view_->sort_by_ > 0)
						fn_ = &SortPhotosByAttrib::SortAscendingSndKeyDesc;
					else
						fn_ = &SortPhotosByAttrib::SortDescendingSndKeyDesc;
				}
			}
			else if (view_->sort_by_ > 0)
				fn_ = &SortPhotosByAttrib::SortAscending;
			else
				fn_ = &SortPhotosByAttrib::SortDescending;
		}
	}

	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return (this->*fn_)(p1, p2);
	}

private:
	ExifView* view_;
	int col_index_;
	int secondary_index_;
	bool (SortPhotosByAttrib::*fn_)(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const;

	bool SortByTime(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		// default sort order
//		return p1->GetPhotoExactTime() < p2->GetPhotoExactTime();
		return p1->GetDateTime() < p2->GetDateTime();
	}

	bool SortAscending(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return view_->cols_.Less(col_index_, *p1, *p2);
	}

	bool SortDescending(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return view_->cols_.Less(col_index_, *p2, *p1);
	}

	bool SortAscendingSndKeyAsc(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return view_->cols_.Less(col_index_, secondary_index_, *p1, *p2, *p1, *p2);
	}

	bool SortDescendingSndKeyAsc(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return view_->cols_.Less(col_index_, secondary_index_, *p2, *p1, *p1, *p2);
	}

	bool SortAscendingSndKeyDesc(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return view_->cols_.Less(col_index_, secondary_index_, *p1, *p2, *p2, *p1);
	}

	bool SortDescendingSndKeyDesc(ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return view_->cols_.Less(col_index_, secondary_index_, *p2, *p1, *p2, *p1);
	}
};


// sorting functor: sorting by directories and then by attributes
//
struct SortPhotosByDirAndAttrib
{
	SortPhotosByDirAndAttrib(ExifView* view) : attrib_sort(view)
	{}

	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		if (p1->GetVisitedDirId() == p2->GetVisitedDirId())
			return attrib_sort(p1, p2);
		else
			return p1->GetVisitedDirId() < p2->GetVisitedDirId();
	}

private:
	SortPhotosByAttrib attrib_sort;
};


// sort only by year/month/day (as specified), and not any further, so different attributes
// can be used to sort images within the same year/month/day
struct SortByDate : std::binary_function<ConstPhotoInfoPtr, ConstPhotoInfoPtr, bool>
{
	SortByDate(ExifView::GroupByDate group) : group_(group)
	{}

	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		const uint32 ds1= p1->GetDateStamp();
		const uint32 ds2= p2->GetDateStamp();

		//tm t1, t2;
		//if (p1->GetDateTime().GetLocalTm(&t1) == 0 || p2->GetDateTime().GetLocalTm(&t2) == 0)
		//	return false;

		switch (group_)
		{
		case ExifView::DONT_GROUP:
			return false;

		case ExifView::BY_YEAR:
			return ds1 / (12 * 31) < ds2 / (12 * 31);
//			return t1.tm_year < t2.tm_year;

		case ExifView::BY_MONTH:
			return ds1 / 31 < ds2 / 31;
			//if (t1.tm_year == t2.tm_year)
			//	return t1.tm_mon < t2.tm_mon;
			//else
			//	return t1.tm_year < t2.tm_year;

		case ExifView::BY_DAY:
			return ds1 < ds2;
			//if (t1.tm_year == t2.tm_year)
			//	return t1.tm_yday < t2.tm_yday;
			//else
			//	return t1.tm_year < t2.tm_year;

		default:
			ASSERT(false);
			return false;
		}
	}

	/*
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		if (group_ == ExifView::DONT_GROUP)
			return false;

		auto t1= p1->GetDateTime();
		auto t2= p2->GetDateTime();
		if (t1.is_not_a_date_time() || t2.is_not_a_date_time())
			return false;	// todo: verify

		//const uint32 ds1= p1->GetDateStamp();
		//const uint32 ds2= p2->GetDateStamp();

		auto ymd1= t1.date().year_month_day();
		auto ymd2= t2.date().year_month_day();

		//tm t1, t2;
		//if (p1->GetDateTime().GetLocalTm(&t1) == 0 || p2->GetDateTime().GetLocalTm(&t2) == 0)
		//	return false;

		switch (group_)
		{
		case ExifView::DONT_GROUP:
			return false;

		case ExifView::BY_YEAR:
			return ymd1.year < ymd2.year;
//			return ds1 / (12 * 31) < ds2 / (12 * 31);
//			return t1.tm_year < t2.tm_year;

		case ExifView::BY_MONTH:
			return ymd1.year == ymd2.year && ymd1.month < ymd2.month;
//			return ds1 / 31 < ds2 / 31;
			//if (t1.tm_year == t2.tm_year)
			//	return t1.tm_mon < t2.tm_mon;
			//else
			//	return t1.tm_year < t2.tm_year;

		case ExifView::BY_DAY:
			return ymd1.year == ymd2.year && t1.date().day_of_year() < t2.date().day_of_year();
//			return ds1 < ds2;
			//if (t1.tm_year == t2.tm_year)
			//	return t1.tm_yday < t2.tm_yday;
			//else
			//	return t1.tm_year < t2.tm_year;

		default:
			ASSERT(false);
			return false;
		}
	}
	*/
private:
	ExifView::GroupByDate group_;
};


struct SortPhotosByDateAndAttrib
{
	SortPhotosByDateAndAttrib(ExifView::GroupByDate group, ExifView* view) : by_date_(group), attrib_sort(view)
	{}

	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		if (by_date_(p1, p2))
			return true;
		else if (by_date_(p2, p1))
			return false;
		else
			return attrib_sort(p1, p2);
	}

private:
	SortByDate by_date_;
	SortPhotosByAttrib attrib_sort;
};


struct SortByDirs : std::binary_function<ConstPhotoInfoPtr, ConstPhotoInfoPtr, bool>
{
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return p1->GetVisitedDirId() < p2->GetVisitedDirId();
	}
};


struct SortByRating : std::binary_function<ConstPhotoInfoPtr, ConstPhotoInfoPtr, bool>
{
	// 'less than' operator
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		// photo with higher rating to be on top (so it's considered being 'less')
		return p1->GetRating() > p2->GetRating();
	}
};


struct SortByTags : std::binary_function<ConstPhotoInfoPtr, ConstPhotoInfoPtr, bool>
{
	// 'less than' operator
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		const size_t size1= p1->GetTags().size();
		const size_t size2= p2->GetTags().size();
#ifdef MULTI_GROUPS
		if (size1 > 0 && size2 > 0)	// both have tags?
		{
			const size_t size= std::min(size1, size2);

			for (size_t i= 0; i < size; ++i)
				if (p1->GetTags()[i] != p2->GetTags()[i])
					return p1->GetTags()[i] < p2->GetTags()[i];

			if (size1 < size2)
				return true;
			if (size2 < size1)
				return false;

			// same amount of identical tags
			return false;
		}

		if (size1 > 0)	// first has tags, snd hasn't
			return true;
		if (size2 > 0)	// first hasn't, snd has
			return false;

		// none has tags
		return false;
#else
		if (size1 == size2 || size1 > 1 && size2 > 1)	// same amount of tags or both have multiple?
		{
			if (size1 == 1 && size2 == 1)	// each photo has one tag: compare tags
				if (p1->GetTags()[0] != p2->GetTags()[0])
					return p1->GetTags()[0] < p2->GetTags()[0];

			return attrib_sort(p1, p2);
		}

		// no. of tags differ

		if (size1 == 1)		// if first photo has only one tag then it's 'lower' in sorting order
			return true;	// than photo with no tags or multiple tags

		if (size1 > 1 && size2 == 0)	// first photo has multiple tags and snd photo has no tags?
			return true;
#endif
		return false;		// snd photo is lower
	}
};


template<class TCmp>
struct SortPhotosByRating
{
	SortPhotosByRating(ExifView* view) : attrib_sort_(view)
	{}

	// 'less than' operator
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		int r1= p1->GetRating();
		int r2= p2->GetRating();
		if (r1 > r2)
			return true;	// photo with higher rating to be on top (so it's considered being 'less')
		if (r1 < r2)
			return false;	// likewise, smaller rating means 'bigger'

		// none has rating; compare attributes instead
		return attrib_sort_(p1, p2);
	}

	TCmp attrib_sort_;
};


// sorting functor: sorting by tags & ratings and then by attributes
//
template<class TCmp>
struct SortPhotosByTags
{
	SortPhotosByTags(ExifView* view) : attrib_sort(view)
	{}

	// 'less than' operator
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		const size_t size1= p1->GetTags().size();
		const size_t size2= p2->GetTags().size();
#ifdef MULTI_GROUPS
		if (size1 > 0 && size2 > 0)	// both have tags?
		{
			const size_t size= std::min(size1, size2);

			for (size_t i= 0; i < size; ++i)
				if (p1->GetTags()[i] != p2->GetTags()[i])
					return p1->GetTags()[i] < p2->GetTags()[i];

			if (size1 < size2)
				return true;
			if (size2 < size1)
				return false;

			// same amount of identical tags; sort by attrib
			return attrib_sort(p1, p2);
		}

		if (size1 > 0)	// first has tags, snd hasn't
			return true;
		if (size2 > 0)	// first hasn't, snd has
			return false;

		// none has tags; compare attributes instead
		return attrib_sort(p1, p2);
#else
		if (size1 == size2 || size1 > 1 && size2 > 1)	// same amount of tags or both have multiple?
		{
			if (size1 == 1 && size2 == 1)	// each photo has one tag: compare tags
				if (p1->GetTags()[0] != p2->GetTags()[0])
					return p1->GetTags()[0] < p2->GetTags()[0];

			return attrib_sort(p1, p2);
		}

		// no. of tags differ

		if (size1 == 1)		// if first photo has only one tag then it's 'lower' in sorting order
			return true;	// than photo with no tags or multiple tags

		if (size1 > 1 && size2 == 0)	// first photo has multiple tags and snd photo has no tags?
			return true;
#endif
		return false;		// snd photo is lower
	}

	TCmp attrib_sort;
};


struct EqualPhotosByDir : std::binary_function<ConstPhotoInfoPtr, uint32, bool>
{
	bool operator () (ConstPhotoInfoPtr p, uint32 dir) const
	{
		return p->GetVisitedDirId() == dir;
	}
};


struct EqualPhotosByDate : std::binary_function<ConstPhotoInfoPtr, uint32, bool>
{
	EqualPhotosByDate(ExifView::GroupByDate grouping) : grouping_(grouping)
	{}

	bool operator () (ConstPhotoInfoPtr p, uint32 date_stamp) const
	{
		const uint32 ds= p->GetDateStamp();

		//tm t1, t2;
		//if (p->GetDateTime().GetLocalTm(&t1) == 0 || date.GetLocalTm(&t2) == 0)
		//	return false;

		switch (grouping_)
		{
		case ExifView::DONT_GROUP:
			return false;

		case ExifView::BY_YEAR:
			return ds / (12 * 31) == date_stamp / (12 * 31);
//			return t1.tm_year == t2.tm_year;

		case ExifView::BY_MONTH:
			return ds / 31 == date_stamp / 31;
			//if (t1.tm_year == t2.tm_year)
			//	return t1.tm_mon == t2.tm_mon;
			//else
			//	return false;

		case ExifView::BY_DAY:
			return ds == date_stamp;
			//if (t1.tm_year == t2.tm_year)
			//	return t1.tm_yday == t2.tm_yday;
			//else
			//	return false;

		default:
			ASSERT(false);
			return false;
		}
	}

private:
	ExifView::GroupByDate grouping_;
};


// "has tags" functor
//
struct PhotoHasTags : std::unary_function<ConstPhotoInfoPtr, bool>
{
	// 'has tags' operator
	bool operator () (ConstPhotoInfoPtr p1) const
	{
		return !p1->GetTags().empty();
	}
};

// "photo has rating" functor
//
struct PhotoHasRating : std::unary_function<ConstPhotoInfoPtr, bool>
{
	bool operator () (ConstPhotoInfoPtr photo) const
	{
		return photo->GetRating() != 0;
	}
};

// "has tags or rating" functor
//
struct PhotoHasTagsOrRating : std::unary_function<ConstPhotoInfoPtr, bool>
{
	bool operator () (ConstPhotoInfoPtr photo) const
	{
		return !photo->GetTags().empty() || photo->GetRating() != 0;
	}
};

// "equal tags" functor (it's not strict; multiple tags are treated as same unless MULTI_GROUPS is defined)
//
struct EqualPhotoByTags : std::binary_function<ConstPhotoInfoPtr, ConstPhotoInfoPtr, bool>
{
	// 'the same' operator
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		const size_t size1= p1->GetTags().size();
		const size_t size2= p2->GetTags().size();
#ifdef MULTI_GROUPS
		if (size1 != size2)
			return false;

		if (size1 > 0 && size2 > 0)	// both have tags?
		{
			ASSERT(size1 == size2);

			for (size_t i= 0; i < size1; ++i)
				if (p1->GetTags()[i] != p2->GetTags()[i])
					return false;

			return true;
		}

		// none has tags
		return true;
#else
		if (size1 > 1 && size2 > 1)
			return true;	// multiple tags: such photos belong to the same group

		if (size1 != size2)
			return false;

		if (size1 == 1)
			return p1->GetTags()[0] == p2->GetTags()[0];
#endif
		return true;
	}
};


struct EqualPhotosByRating : std::binary_function<ConstPhotoInfoPtr, ConstPhotoInfoPtr, bool>
{
	// 'the same' operator
	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		return p1->GetRating() == p2->GetRating();
	}
};


#define MATCH_XMP_FIELD(FLD)	if (x_stristr(xmp->FLD.c_str(), keyword_.c_str()) != 0) return true;


// filtering support functor
struct IsPhotoFilteredIn : std::unary_function<ConstPhotoInfoPtr, bool>
{
	IsPhotoFilteredIn(const String& keyword, DateTime from, DateTime to) : keyword_(keyword), from_(from), to_(to)
	{}

	// filter in?
	bool operator () (ConstPhotoInfoPtr photo) const
	{
		if (!keyword_.empty())
		{
			// first check list of keywords
			if (photo->tags_.FindTagNoCase(keyword_, true))
				return true;

			// then check description
			String desc;
			photo->Description(desc);

			// partial match is ok
			if (x_stristr(desc.c_str(), keyword_.c_str()) != 0)
				return true;

			// finally try to match file name too
			if (x_stristr(photo->GetDisplayPath().c_str(), keyword_.c_str()) != 0)
				return true;

			// search in metadata too if present
			if (const XmpData* xmp= photo->GetMetadata())
			{
				MATCH_XMP_FIELD(DocumentTitle)
				MATCH_XMP_FIELD(Author)
				MATCH_XMP_FIELD(Description)
				MATCH_XMP_FIELD(ImageRating)
				MATCH_XMP_FIELD(CopyrightNotice)
				MATCH_XMP_FIELD(Keywords)
				MATCH_XMP_FIELD(DescriptionWriter)
				MATCH_XMP_FIELD(Headline)
				MATCH_XMP_FIELD(CreatorsJob)
				MATCH_XMP_FIELD(Address)
				MATCH_XMP_FIELD(City)
				MATCH_XMP_FIELD(State)
				MATCH_XMP_FIELD(PostalCode)
				MATCH_XMP_FIELD(Country)
				MATCH_XMP_FIELD(Phones)
				MATCH_XMP_FIELD(EMails)
				MATCH_XMP_FIELD(WebSites)
				MATCH_XMP_FIELD(JobIdentifier)
				MATCH_XMP_FIELD(Instructions)
				MATCH_XMP_FIELD(Provider)
				MATCH_XMP_FIELD(Source)
				MATCH_XMP_FIELD(RightsUsageTerms)
				MATCH_XMP_FIELD(CopyrightInfoURL)
				MATCH_XMP_FIELD(CreationDate)
				MATCH_XMP_FIELD(IntellectualGenre)
				MATCH_XMP_FIELD(Location)
				MATCH_XMP_FIELD(City2)
				MATCH_XMP_FIELD(StateProvince)
				MATCH_XMP_FIELD(Country2)
				MATCH_XMP_FIELD(ISOCountryCode)
				MATCH_XMP_FIELD(IPTCScene)
				MATCH_XMP_FIELD(IPTCSubjectCode)
				MATCH_XMP_FIELD(CreatorTool)
			}
		}

		if (!from_.is_not_a_date_time() && !to_.is_not_a_date_time())
		{
			if (photo->GetDateTime() >= from_ &&
				photo->GetDateTime() <= to_)
				return true;
		}

		return false;
	}

private:
	const String& keyword_;
	DateTime from_, to_;
};

#undef MATCH_XMP_FIELD


#define MATCH_XMP_FIELD(FLD)	if (x_stristr(xmp->FLD.c_str(), text.c_str()) != 0) return true;


struct FilterByText : std::unary_function<ConstPhotoInfoPtr, bool>
{
	FilterByText(const String& include, const String& exclude) : include_(include), exclude_(exclude)
	{}

	// filter in?
	bool operator () (ConstPhotoInfoPtr photo) const
	{
		if (!match_text(include_, photo))
			return false;

		if (!exclude_.empty() && match_text(exclude_, photo))
			return false;

		return true;
	}

	// partial text match
	static bool match_text(const String& text, ConstPhotoInfoPtr photo)
	{
		if (text.empty())
			return true;

		// first check list of keywords
		if (photo->tags_.FindTagNoCase(text, true))
			return true;

		// then check description
		String desc;
		photo->Description(desc);

		// partial match is ok
		if (x_stristr(desc.c_str(), text.c_str()) != 0)
			return true;

		// finally try to match file name too
		if (x_stristr(photo->GetDisplayPath().c_str(), text.c_str()) != 0)
			return true;

		// search in metadata too if present
		if (const XmpData* xmp= photo->GetMetadata())
		{
			MATCH_XMP_FIELD(DocumentTitle)
			MATCH_XMP_FIELD(Author)
			MATCH_XMP_FIELD(Description)
			MATCH_XMP_FIELD(ImageRating)
			MATCH_XMP_FIELD(CopyrightNotice)
			MATCH_XMP_FIELD(Keywords)
			MATCH_XMP_FIELD(DescriptionWriter)
			MATCH_XMP_FIELD(Headline)
			MATCH_XMP_FIELD(CreatorsJob)
			MATCH_XMP_FIELD(Address)
			MATCH_XMP_FIELD(City)
			MATCH_XMP_FIELD(State)
			MATCH_XMP_FIELD(PostalCode)
			MATCH_XMP_FIELD(Country)
			MATCH_XMP_FIELD(Phones)
			MATCH_XMP_FIELD(EMails)
			MATCH_XMP_FIELD(WebSites)
			MATCH_XMP_FIELD(JobIdentifier)
			MATCH_XMP_FIELD(Instructions)
			MATCH_XMP_FIELD(Provider)
			MATCH_XMP_FIELD(Source)
			MATCH_XMP_FIELD(RightsUsageTerms)
			MATCH_XMP_FIELD(CopyrightInfoURL)
			MATCH_XMP_FIELD(CreationDate)
			MATCH_XMP_FIELD(IntellectualGenre)
			MATCH_XMP_FIELD(Location)
			MATCH_XMP_FIELD(City2)
			MATCH_XMP_FIELD(StateProvince)
			MATCH_XMP_FIELD(Country2)
			MATCH_XMP_FIELD(ISOCountryCode)
			MATCH_XMP_FIELD(IPTCScene)
			MATCH_XMP_FIELD(IPTCSubjectCode)
			MATCH_XMP_FIELD(CreatorTool)
		}

		return false;
	}

private:
	String include_;
	String exclude_;
};

#undef MATCH_XMP_FIELD


struct FilterByTime : std::unary_function<ConstPhotoInfoPtr, bool>
{
	FilterByTime(DateTime from, DateTime to) : from_(from), to_(to)
	{}

	// filter in?
	bool operator () (ConstPhotoInfoPtr photo) const
	{
		if (from_.is_not_a_date_time() || to_.is_not_a_date_time())
			return true;
		else
			return photo->GetDateTime() >= from_ && photo->GetDateTime() <= to_;
	}

private:
	DateTime from_, to_;
};


struct FilterByTags : std::unary_function<ConstPhotoInfoPtr, bool>
{
	FilterByTags(const FilterTags& filter) : filter_(filter)
	{}

	// filter in?
	bool operator () (ConstPhotoInfoPtr photo) const
	{
		const size_t icount= filter_.include.size();

		if (filter_.match_all)
		{
			// match ALL selected tags

			for (size_t i= 0; i < icount; ++i)
				if (!photo->tags_.FindTag(filter_.include[i]))
					return false;	// one 'include' tag not present -> filter out

			const size_t ecount= filter_.exclude.size();
			for (size_t j= 0; j < ecount; ++j)
				if (!photo->tags_.FindTag(filter_.exclude[j]))
					return true;	// one 'exclude' tag not present -> fine, include photo

			// if no excluded tags selected, photo passed;
			// if excluded tags selected photo contains them all, so filter it out
			return ecount == 0 ? true : false;
		}
		else
		{
			// match ANY selected tag

			bool found= icount == 0; // default to 'found/true' for empty tag selection
			if (!photo->tags_.empty())
				for (size_t i= 0; i < icount; ++i)
					if (photo->tags_.FindTag(filter_.include[i]))
					{
						// found matching tag; check excluded tags now
						found = true;
						break;
					}

			if (!found)
				return false;

			const size_t ecount= filter_.exclude.size();
			if (!photo->tags_.empty())
				for (size_t j= 0; j < ecount; ++j)
					if (photo->tags_.FindTag(filter_.exclude[j]))
						return false;	// at least one excluded tag present, filter photo out

			return true;
		}
	}

private:
	const FilterTags& filter_;
};


struct FilterByRating : std::unary_function<ConstPhotoInfoPtr, bool>
{
	FilterByRating(const FilterStars& filter) : filter_(filter)
	{}

	bool operator () (ConstPhotoInfoPtr photo) const
	{
		if (filter_.stars == 0)
			return true;
		else
			return photo->GetRating() >= filter_.stars;
	}

private:
	const FilterStars& filter_;
};


struct FilterByExpression : std::unary_function<ConstPhotoInfoPtr, bool>
{
	FilterByExpression(const String& rule) : filter_(rule), rule_(rule)
	{}

	bool operator () (ConstPhotoInfoPtr photo) const
	{
		if (rule_.empty())
			return true;
		else
			return filter_.CalcResult(*photo);
	}

private:
	String rule_;
	CustomFilter filter_;
};



// this function awaits 'selected_file_' to show up in the list of decoded images; when it does this fn
// will open it in a viewer; if it doesn't though, app will be closed (unless selected_file_ is empty)
//
bool ExifView::CheckViewerStart(bool loading_done)
{
	if (selected_file_.empty())
		return false;

	bool close_app= false;

	if (!all_photos_.empty())
	{
		PhotoInfoPtr photo= all_photos_.GetNthItem(0);
		CString s= selected_file_.c_str();
		if (s.CompareNoCase(photo->GetPhysicalPath().c_str()) == 0)
		{
			// if 'selected_file_' matches first loaded image, then show it in the viewer window
			BlockPhotoCtrlUpdates block(GetListCtrl());
			SyncListCtrl(1);
			distribution_bar_wnd_.BuildHistogram(sorted_photos_, sorted_photos_);
			ViewPhoto(photo, ALL_PHOTOS);
			selected_file_.clear();

			return false;	// image found; don't quit
		}
		else
			close_app = true;
	}

	if (loading_done || close_app)
	{
		// selected photo should come first, if it doesn't it wasn't decoded properly, so we can quit now
		if (CWnd* wnd= AfxGetMainWnd())
			if ((wnd->GetStyle() & WS_VISIBLE) == 0)
				wnd->DestroyWindow();
	}

	return true;
}


//#define LOAD_TIME
#ifdef LOAD_TIME
DWORD timer= 0;
#endif

LRESULT ExifView::OnPhotoLoaded(WPARAM count, LPARAM progress_param)
{
	const DWORD SYNCHRO_DELAY= 200;	// 200 ms
	ReloadJob::Progress progress= static_cast<ReloadJob::Progress>(progress_param);

	try
	{
		switch (progress)
		{
		case ReloadJob::RELOADING_START:
#ifdef LOAD_TIME
timer = ::GetTickCount();
#endif
//		RemoveItems();
			//if (MainFrame* frame= dynamic_cast<MainFrame*>(AfxGetMainWnd()))
			//	frame->UpdateStatusBar();

			last_sync_time_ = ::GetTickCount(); // first update with no delay + SYNCHRO_DELAY / 2;

			break;

		case ReloadJob::RELOADING_PENDING:

			if (labs(static_cast<long>(::GetTickCount() - last_sync_time_)) > SYNCHRO_DELAY)
			{
				bool ctrl= ::GetAsyncKeyState(VK_CONTROL) < 0;
				if (!ctrl)
				{
					BlockPhotoCtrlUpdates block(GetListCtrl());
					SyncListCtrl(all_photos_.size());
					distribution_bar_wnd_.BuildHistogram(sorted_photos_, sorted_photos_);
				}
				last_sync_time_ = ::GetTickCount();
				UpdateDialogControls(AfxGetMainWnd(), true);

				if (viewer_link_.IsValid() && current_viewer_type_ == ALL_PHOTOS)
				{
					viewer_photos_ = sorted_photos_;
					viewer_link_->Synch();
				}
			}

			CheckViewerStart(false);

			break;

		case ReloadJob::RELOADING_CANCELLED:
		case ReloadJob::RELOADING_FINISHED:

			{
				BlockPhotoCtrlUpdates block(GetListCtrl());
				SyncListCtrl(all_photos_.size());
			}

			//if (MainFrame* frame= dynamic_cast<MainFrame*>(AfxGetMainWnd()))
			//	frame->UpdateStatusBar();

			if (viewer_link_.IsValid() && current_viewer_type_ == ALL_PHOTOS)
			{
				viewer_photos_ = sorted_photos_;
				viewer_link_->Synch();
			}

			distribution_bar_wnd_.BuildHistogram(sorted_photos_, sorted_photos_);

#ifdef LOAD_TIME
timer = ::GetTickCount() - timer;
{CString s;
s.Format(L"%d", timer);
AfxMessageBox(s, MB_OK);}
#endif
			if (!CheckViewerStart(true))
				StartDirectoryWatching();

/*{
	ofstream ofs("c:\\features.txt", ios_base::out | ios_base::trunc);

	int n= sorted_photos_.size();
	for (int i= 0; i < n; ++i)
	{
		for (int j= 0; j < 256; ++j)
		{
			ofs.precision(6);
			ofs << sorted_photos_[i]->index_.GetSimilarityHistogram()[j] << '\t';
		}
		ofs << endl;
	}
}*/

//		ResortItems();
/*
//GetListCtrl().AddItems(all_photos_);
GetListCtrl().RemoveAll();

VectPhotoInfo photos;
int count= all_photos_.size();
photos.reserve(count);
PhotoInfoStorage::iterator it= all_photos_.begin();
for (int i= 0; i < count; ++i, ++it)
	photos.push_back(it->get());
sort(photos.begin(), photos.end(), SortPhotosByDirAndAttrib(this));

if (!photos.empty())
{
	for (VectPhotoInfo::iterator it= photos.begin(); it != photos.end(); )
	{
		// find all photos from same directory (identical GetVisitedDirId())
		// they are sorted by GetVisitedDirId(), so it's a range of adjacent values

		VectPhotoInfo::iterator last= find_if(it, photos.end(), not1(bind2nd(EqualPhotosByDir(), (*it)->GetVisitedDirId())));

		GetListCtrl().AddItems(it, last, Path((*it)->path_).GetDir(), PhotoCtrl::FILM_ROLL, (*it)->GetVisitedDirId());

		it = last;
	}
}
*/
			break;

		case ReloadJob::RELOADING_QUIT:
			CheckViewerStart(true);
			break;
		}

		if (!reload_progress_event_.empty())
			reload_progress_event_(progress, all_photos_.size());

	}
	CATCH_ALL

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////


void ExifView::StartDirectoryWatching()
{
	if (cur_path_.IsFolder())
	{
		String ext= GetPhotoFactory().GetRegisteredExtensions();
		dir_watcher_.WatchFolder(cur_path_.c_str(), recursive_scan_, ext.c_str());
	}
}


void ExifView::StopDirectoryWatching()
{
	dir_watcher_.Stop();

	// empty lists...
	add_new_items_.clear();
	remove_old_items_.clear();
	rename_items_.clear();
}


void ExifView::OnFileChangeDetected(const TCHAR* path, DirectoryChangeWatcher::Change change, const TCHAR* new_path)
{
#ifdef DIR_WATCH

TRACE(L"ExifView::OnFileChangeDetected\n");
	PhotoInfoPtr photo= all_photos_.FindItem(path);

	if (change == DirectoryChangeWatcher::NewFile)
	{
TRACE(L"ExifView::OnFileChangeDetected add %p\n", photo);
		if (photo)
			return;

		if (AutoPtr<PhotoInfo> new_item= ScanPhoto(path))
		{
			add_new_items_.Append(new_item);
TRACE(L"new photo added at %d\n", ::GetTickCount());
		}
	}
	else if (change == DirectoryChangeWatcher::FileModified)
	{
		if (photo)
			return;

		if (AutoPtr<PhotoInfo> new_item= ScanPhoto(path))
		{
			add_new_items_.Append(new_item);
TRACE(L"added changed file %d\n", ::GetTickCount());
		}
	}
	else if (change == DirectoryChangeWatcher::FileRemoved)
	{
TRACE(L"ExifView::OnFileChangeDetected del %p\n", photo);
		if (photo == 0)
			// check those being added
			return;

		remove_old_items_.push_back(photo);
	}
	else if (change == DirectoryChangeWatcher::FileRenamed)
	{
		if (photo == 0)
			return;

		rename_items_.push_back(photo);
	}

	// dir/file change notifications arrive in a main thread already

	//
#endif
}


SmartPhotoPtr ExifView::ScanPhoto(const TCHAR* path)
{
	FileStream ifs;

	if (!ifs.Open(path))
		return SmartPhotoPtr();			// error opening file

	ImgScanner scan(_T(""));

	int dir_visited= 0;

	return scan.SingleFileScan(Path(path), dir_visited);
}


void ExifView::OnDirChangeDetected(const TCHAR* path, DirectoryChangeWatcher::Change change, const TCHAR* new_path)
{

}

////////////////////////////////////////////////////////////////////////////////////


//void ExifView::TagApplied(PhotoInfoPtr photo)
//{
//	MovePhoto(photo);
//
//	//if (viewer_link_.IsValid())
//	//	viewer_link_->TagApplied(photo);
//}


//void ExifView::TagApplied(const VectPhotoInfo& photos)
//{
//	MovePhotos(photos);
//
//	//if (viewer_link_.IsValid() && !photos.empty())
//	//	viewer_link_->TagApplied(photos[0]);	//HACK: viewer only invalidates preview, so call it once with just first image
//}


void ExifView::MovePhotos(const VectPhotoInfo& photos)
{
	try
	{
		const size_t count= photos.size();

		if (count == 0)
			return;

		SaveSelection selection(*this);
//		VectPhotoInfo selected;
//		GetSelectedPhotos(selected);

		// first remove all: this is crucial step; when tags or rating is applied to multiple images
		// their order could have changed completely, removing just one and adding it back may fail
		// due to the new sorting order (sorting will reposition those changed images that are to be moved)

		for (size_t i= 0; i < count; ++i)
			RemoveItem(photos[i], true, false);

		// now it's safe to add them back to list ctrl

		for (size_t i= 0; i < count; ++i)
			AddItem(photos[i]);

		// restore selection now

		//if (selected.empty())
		//	GetListCtrl().SetCurrentItem(photos.front(), true);
		//else
		//{
		//	GetListCtrl().SelectItems(selected, true);
		//	GetListCtrl().SetCurrentItem(selected.front(), false);
		//}
	}
	CATCH_ALL
}


void ExifView::MovePhoto(PhotoInfoPtr photo)
{
	ASSERT(photo != 0);

	//TODO: for now removing from photo ctrl; later it should be avoided
	//TODO: replace by some form of MoveItem
	try
	{
		RemoveItem(photo, true, false);
		AddItem(photo);
		GetListCtrl().SetCurrentItem(photo, true);
	}
	CATCH_ALL
/*
	if (photo->GetTags().empty())	// no tags?
	{
		// move it into folder group
//		GetListCtrl().MoveItem(photo, photo->path_.GetDir(), PhotoCtrl::FILM_ROLL, photo->GetVisitedDirId());
		RemoveItem(photo);
		AddItem(photo);
	}
	else if (photo->GetTags().size() == 1)	// single tag?
	{
		// move it into tag group
		String group_name= _T("Tag \x201D") + photo->GetTags()[0] + _T("\x201D");
		int id= INT_MIN + frame_->GetTags().GetTagId(photo->GetTags()[0]);
		GetListCtrl().MoveItem(photo, group_name, PhotoCtrl::LABEL, id);
	}
	else // many tags?
	{
		// move it into multiple tags group
		String group_name= _T("Multiple Tags");
		GetListCtrl().MoveItem(photo, group_name, PhotoCtrl::LABEL, 0);
	} */
}


void ExifView::SyncListCtrl(const size_t count)
{
	static bool in_update= false;

	if (in_update)
		return;

	Block update(in_update);

	if (synched_item_count_ < count)
	{
		GetListCtrl().UpdateWindow();
		BlockPhotoCtrlUpdates block(GetListCtrl());

		sorted_photos_.reserve(count);
		visible_photos_.reserve(count);

		const FilterData* filter= GetCurrentFilterData();

		VectPhotoInfo output;
		VectPhotoInfo input;
		// copy range of source images
		all_photos_.Copy(synched_item_count_, count, input);

		if (filter)		// filter images (if filter is on)
			FilterSet(input, *filter, output);
		else
			output = input;

		const size_t new_images= output.size();
		for (size_t index= 0; index < new_images; ++index)
		{
			PhotoInfoPtr photo= output[index];
			ASSERT(photo);
			if (photo == 0)
				break;

			AddItem(photo);
			//visible_photos_.push_back(photo);
		}

		synched_item_count_ = count;
	}
}


int ExifView::GetImageCount() const
{
	if (IsFilterActive())
		return GetListCtrl().GetItemCount();
	else
		return static_cast<int>(all_photos_.size());
}


// get status text (no of images or scanning progress)
//
CString ExifView::GetImagesStat() const
{
	int count= GetImageCount();
	CString status;

	bool scanning= reload_thread_->IsScanning();	// it's safe to call IsScanning with null 'this' ptr

	if (scanning)
	{
		// thanks to this simple trick with rounding to full tens status bar refreshes itself only
		// once every ten times SetText is invoked (due to identical text being provided)
		status.Format(_T("扫描中... (%d)"), count > 10 ? count - count % 10 : count);
	}
	else if (count == 0)
		status = _T("无图像");
	//else if (count == 1)
	//	status = _T("1 图像");
	else
		status.Format(_T("%d 图像"), count);

	if (!scanning)
	{
		status += _T(", ");

		int selected= GetListCtrl().GetSelectedCount();
		if (selected == 0)
			status += _T("无选定");
		else if (selected == all_photos_.size())
			status += _T("全部选定");
		else
		{
			CString s;
			s.Format(_T("%d 选定"), selected);
			status += s;
		}

	}

	if (reload_thread_)
	{
		const ImgLogger& logger= reload_thread_->GetImageErrorLogger();

		if (logger.GetCount() > 0)
			status.Insert(0, '!');	// show warning icon in the status bar
	}

	if (IsFilterActive() && GetListCtrl().GetItemCount() < all_photos_.size())
	{
		// show funnel icon to indicate filtering is on

		status.Insert(0, '@');	// show funnel the status bar
	}

	return status;
}


///////////////////////////////////////////////////////////////////////////////

void ExifView::RemovePhoto(PhotoInfoPtr photo)
{
//	SaveCurrentImage current(*this);

	RemoveItem(photo, true, true);

	VectPhotoInfo::iterator in_viewer= find(viewer_photos_.begin(), viewer_photos_.end(), photo);
	if (in_viewer != viewer_photos_.end())
	{
		viewer_photos_.erase(in_viewer);

		// refresh image viewer not necessary (this is notification sent from viewer)
//		if (viewer_link_.IsValid())
//			viewer_link_->PhotosListChange();
	}

	if (global_dib_cache.get())
//		global_dib_cache->Remove(photo);
		RemoveAllPhotoDibs(photo, *global_dib_cache);

	if (global_photo_cache.get())
		global_photo_cache->Remove(photo);

	image_loader_.RemovePhoto(photo, true);
	thumbnail_loader_.RemovePhoto(photo, true);

	//queue_.Erase(photo);

	//if (decoder_ != 0 && decoder_->GetPhoto() == photo)
	//{
	//	decoder_->Quit();
	//	decoder_ = 0;
	//	StartDecodingNextPhoto();
	//}

	all_photos_.Remove(photo);

	synched_item_count_--;
}


// return collection of photos that satisfy filtering criteria (time range and text match)
//
//void FilterPhotos(const PhotoInfoStorage& photos, VectPhotoInfo& out, const String& filter_text, CTime from, CTime to)
//{
//	VectPhotoInfo copy;
//	photos.Copy(copy);
//
//	const size_t count= copy.size();
//
//	out.clear();
//	out.reserve(count / 2);
//
//	for (size_t i= 0; i < count; ++i)
//	{
//		PhotoInfoPtr photo= copy[i];
//
//		if (photo == 0)
//			continue;
//
//		if (IsPhotoFilteredIn(filter_text, from, to)(photo))
//			out.push_back(photo);
//	}
//}


// return collection of photos that satisfy filtering criteria (time range and text match)
//
void GetHistogramPhotos(const VectPhotoInfo& visible_photos, VectPhotoInfo& out, DateTime from, DateTime to)
{
	const size_t count= visible_photos.size();

	out.clear();
	out.reserve(count / 2);

	FilterByTime filter(from, to);

	for (size_t i= 0; i < count; ++i)
	{
		PhotoInfoPtr photo= visible_photos[i];

		if (photo == 0)
			continue;

		if (filter(photo))
			out.push_back(photo);
	}
}


void ExifView::RemoveItemsWorker(const VectPhotoInfo& remove, bool removed_are_selected)
{
#ifdef _DEBUG
	if (removed_are_selected)
	{
		// verify selection
		VectPhotoInfo selected2;
		GetSelectedPhotos(selected2);
		ASSERT(selected2 == remove);
	}
#endif

	//TODO: stop current item notification processing for multiple items removal...

//	bool decoder_stopped= false;

	image_loader_.RemovePhotos(remove);
	thumbnail_loader_.RemovePhotos(remove);

	// remove selected photos from our 'remove' vector
	for (VectPhotoInfo::const_iterator it= remove.begin(); it != remove.end(); ++it)
	{
		VectPhotoInfo::iterator erase= FindInsertPos(*it);

		if (erase != sorted_photos_.end() && &*erase == &*it)
			sorted_photos_.erase(erase);
		else
		{
			// sorting by similarity?

			VectPhotoInfo::iterator er= find(sorted_photos_.begin(), sorted_photos_.end(), *it);
			if (er != sorted_photos_.end())
				sorted_photos_.erase(er);
			else
			{ ASSERT(false); }
		}

		VectPhotoInfo::iterator er= find(visible_photos_.begin(), visible_photos_.end(), *it);
		if (er != visible_photos_.end())
			visible_photos_.erase(er);

		VectPhotoInfo::iterator in_viewer= find(viewer_photos_.begin(), viewer_photos_.end(), *it);
		if (in_viewer != viewer_photos_.end())
			viewer_photos_.erase(in_viewer);

		if (global_dib_cache.get())
			RemoveAllPhotoDibs(*it, *global_dib_cache);

		if (global_photo_cache.get())
			global_photo_cache->Remove(*it);

		//queue_.Erase(*it);

		//if (decoder_ != 0 && decoder_->GetPhoto() == *it)
		//{
		//	decoder_->Quit();
		//	decoder_ = 0;
		//	decoder_stopped = true;
		//}
	}

	if (removed_are_selected)
	{
		GetListCtrl().RemoveSelected();

		if (GetListCtrl().GetCurrentItem() == 0)
			GetListCtrl().SetCurrentItem(GetListCtrl().GetItem(PhotoCtrl::FIRST_VISIBLE), true, true, true);
	}
	else
	{
		for (VectPhotoInfo::const_iterator it= remove.begin(); it != remove.end(); ++it)
			GetListCtrl().RemoveItem(*it, false);

		if (GetListCtrl().GetCurrentItem() == 0)
			GetListCtrl().SetCurrentItem(GetListCtrl().GetItem(PhotoCtrl::FIRST_VISIBLE), true, true, true);
		else
			CurrentItemChanged(GetListCtrl().GetCurrentItem());
	}

	//if (decoder_stopped)
	//	StartDecodingNextPhoto();

	//TODO: resort to make sure there's no empty groups (maybe)

	// refresh image viewer
	if (viewer_link_.IsValid())
		viewer_link_->PhotosListChange();

	all_photos_.Remove(remove);
	synched_item_count_ -= remove.size();

	//TODO: is there no selection change or some other notification?
	RebuildHistogram();

#ifdef _DEBUG
	for (VectPhotoInfo::const_iterator it= remove.begin(); it != remove.end(); ++it)
		(*it)->SetDeleted();
#endif
}


void ExifView::RebuildHistogram(const FilterData* filter)
{
	if (!hist_date_from_.is_not_a_date_time() && !hist_date_to_.is_not_a_date_time())
	{
		if (filter == 0)
			filter = GetCurrentFilterData();

		VectPhotoInfo background;
		VectPhotoInfo input;
		all_photos_.Copy(input);	// lock once, and copy pointers to 'input'

		if (filter != 0)
		{
			FilterData copy= *filter;
			copy.time_.from = hist_date_from_;
			copy.time_.to = hist_date_to_;
			FilterSet(input, copy, background);
		}
		else
			background.swap(input);

		VectPhotoInfo histo;
		::GetHistogramPhotos(background, histo, sel_hist_date_from_, sel_hist_date_to_);

		distribution_bar_wnd_.BuildHistogram(histo, background);
	}
	else
	{
		distribution_bar_wnd_.BuildHistogram(visible_photos_, visible_photos_);
	}
}


void ExifView::RemoveItems(const VectPhotoInfo& remove)
{
	RemoveItemsWorker(remove, false);
}


void ExifView::EmptyList(bool erase_viewer_list)
{
	if (notifications_enabled_counter_ >= 0)
	{
		frame_->SelectionChanged(0);	// clear tags pane
		frame_->CurrentChanged(0, false);	// clear status bar

		PhotoInfoPtr Null= 0;
		SendNotification(&PaneWnd::CurrentPhotoChanged, Null);
	}
	GetListCtrl().RemoveAll();
	sorted_photos_.clear();
	visible_photos_.clear();
	if (erase_viewer_list)
		viewer_photos_.clear();
	synched_item_count_ = 0;
	RedrawWindow();
	//PreviewPhoto(0);
}

// remove all photos from list ctrl
void ExifView::RemoveItems()
{
	EmptyList();

	// clear the caches
	if (global_dib_cache.get())
		global_dib_cache->RemoveAll();

	if (global_photo_cache.get())
		global_photo_cache->RemoveAll();

	//queue_.Clear();
	image_loader_.RemoveAll();
	thumbnail_loader_.RemoveAll();
}


// remove selected photos from list ctrl and list of photos
void ExifView::RemoveSelected(const VectPhotoInfo& selected)
{
	RemoveItemsWorker(selected, true);
}

#if 0
void ExifView::SetColors(CDC* dc, const CRect& img_rect, UINT state, bool frame/*= true*/)
{
	dc->SetBkMode(OPAQUE);

	COLORREF rgb_fill= ::GetSysColor(COLOR_WINDOW);
	COLORREF rgb_frame= frame ? CalcNewColor(rgb_fill, -15.0) : rgb_fill; // 15% darker
	COLORREF rgb_text= ::GetSysColor(COLOR_WINDOWTEXT);

	if (state & LVIS_SELECTED)
	{
		rgb_fill = ::GetFocus() == GetListCtrl().m_hWnd ? /*::GetSysColor(COLOR_HIGHLIGHT)*/ g_Settings.AppColors()[AppColors::Selection] : ::GetSysColor(COLOR_3DFACE);
		rgb_frame = rgb_fill;
		rgb_text = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	}

	if (frame && (state & LVIS_FOCUSED))
		rgb_frame =  g_Settings.AppColors()[AppColors::Selection];//::GetSysColor(COLOR_HIGHLIGHT);

	// fill backgnd
	CRect rect= img_rect;
	rect.DeflateRect(3, 1);
	dc->FillSolidRect(rect, rgb_frame);
	rect.DeflateRect(1, 1);
	dc->FillSolidRect(rect, rgb_fill);

	dc->SetTextColor(rgb_text);
	dc->SetBkColor(rgb_fill);
}
#endif


// return item with focus
//
PhotoInfoPtr ExifView::CurrentItem() const
{
	return GetListCtrl().GetCurrentItem();
}


// get short text info about currently selected photo
//
CString ExifView::GetCurImageInfo() const
{
	PhotoInfoPtr photo= CurrentItem();
	CString info;

	if (photo == 0)
		return info;

	return cols_.GetStatusText(frame_->GetStatusBar().fields_, *photo, true).c_str();
/*
	if (photo->IsExifDataPresent())
//	if (photo->ExposureTimeValue() > 0 && photo->FNumberValue() > 0)
	{
		return cols_.GetStatusText(frame_->GetStatusBar().fields_, *photo, true).c_str();

/ *		oStringstream ost;

		//ost << _T("File: ") << photo->name_ 
		ost << DrawFields::LABEL << _T("Date: ") << CDrawFields::VALUE << photo->DateTime();
		ost << DrawFields::LABEL << _T("   Dimensions: ") << CDrawFields::VALUE << photo->Size();
		ost << DrawFields::LABEL << _T("   Aperture: ") << CDrawFields::VALUE << photo->FNumber();
		ost << DrawFields::LABEL << _T("   Shutter: ") << CDrawFields::VALUE << photo->ExposureTime() << _T(" s");
		ost << DrawFields::LABEL << _T("   Focal length: ") << CDrawFields::VALUE << photo->FocalLength();
		ost << DrawFields::LABEL << _T("   ISO: ") << CDrawFields::VALUE << photo->ISOSpeed();
		ost << DrawFields::LABEL << _T("   Exposure Bias: ") << DrawFields::VALUE << photo->ExposureBias();
		ost << DrawFields::LABEL << _T("   Exposure Prg: ") << DrawFields::VALUE << photo->ExposureProgram();
		info = ost.str().c_str(); * /
		// status pane text is limited to 127 chars (WinXP), but I'm using owner draw one
	}
	else
	{
		vector<uint16> fields(3);
		fields[0] = COL_FILE_NAME;
		fields[1] = COL_DATE_TIME;
		fields[2] = COL_DIMENSIONS;
		return cols_.GetStatusText(fields, *photo, true).c_str();
//		oStringstream ost;
//		ost << DrawFields::LABEL << _T("Date: ") << DrawFields::VALUE << photo->DateTime();
//		ost << DrawFields::LABEL << _T("  Dimensions: ") << DrawFields::VALUE << photo->Size();
	}

	return info; */
}


// set view mode
//
void ExifView::SetViewMode(ViewMode view_mode)
{
	if (view_mode == view_mode_)
		return;

	ModeSwitch(view_mode);
}

void ExifView::SetNextViewMode()
{
	ModeSwitch(++view_mode_);
}


void ExifView::ModeSwitch(ViewMode view_mode)
{
	tool_bar_wnd_.ImgSizeLabel(view_mode != VIEW_MODE_PICTURES);

	if (view_mode != VIEW_MODE_PICTURES)
	{
		tool_bar_wnd_.ResetThumbRange(array_count(thumb_size_levels));
		tool_bar_wnd_.SetThumbPos(img_size_index_);
	}
	else
	{
		tool_bar_wnd_.SetThumbPos(0);	// set it to zero before changing range or else it may ignore SetPos()
		tool_bar_wnd_.ResetThumbRange(array_count(images_across_levels));
		tool_bar_wnd_.SetThumbPos(images_across_index_);
	}

	view_mode_ = view_mode;

	switch (view_mode)
	{
	case VIEW_MODE_THUMBNAILS:
		GetListCtrl().ChangeMode(PhotoCtrl::THUMBNAILS);
		break;

	case VIEW_MODE_DETAILS:
		GetListCtrl().ChangeMode(PhotoCtrl::DETAILS);
		break;

	case VIEW_MODE_TILES:
		GetListCtrl().ChangeMode(PhotoCtrl::TILES);
		break;

	case VIEW_MODE_PICTURES:
		GetListCtrl().ChangeMode(PhotoCtrl::PREVIEWS);
		break;

	default:
		ASSERT(false);
		break;
	}
}


// new path selected; start reloading
//
bool ExifView::PathSelected(FolderPathPtr path)
{
	return Browse(path);
}


void ExifView::ItemDblClicked(PhotoInfoPtr photo)
{
	if (photo)
		ViewPhoto(photo, ALL_PHOTOS);
}


void ExifView::ViewPhoto(PhotoInfoPtr photo, ViewerType type)
{
	if (photo == 0)	// if nothing's selected then pick up first one
		photo = CurrentItem();

	if (photo == 0)
		return;

	switch (type)
	{
	case ALL_PHOTOS:
		viewer_photos_ = sorted_photos_;
		break;
	case SELECTED_PHOTOS:
		GetSelectedPhotos(viewer_photos_);
		break;
	case TAGGED_PHOTOS:
		GetTaggedPhotos(viewer_photos_);
		break;
	default:
		ASSERT(false);
		break;
	}

	if (viewer_photos_.empty())
	{
		ASSERT(false);
		return;
	}

	current_viewer_type_ = type;

	if (find(viewer_photos_.begin(), viewer_photos_.end(), photo) == viewer_photos_.end())
		photo = viewer_photos_.front();

	if (!viewer_link_.IsValid())
	{
		ViewerDlg* viewer= new ViewerDlg(&viewer_link_, viewer_photos_, all_photos_, this, cols_);
		viewer_link_.Connect(viewer);
		viewer->Create(_T("Shutter图像查看器"));
	}
	else
	{
		viewer_link_->BringWindowToTop();
		viewer_link_->ShowWindow(SW_RESTORE);
		viewer_link_->Synch();
	}

	if (viewer_link_.IsValid())
		viewer_link_->LoadPhoto(photo);

//	if (frame_)
//		frame_->ReparentTagBar(viewer_link_.get());
}


// view selected photo (open viewer window)
//
void ExifView::OnView()
{
	ViewPhoto(0, ALL_PHOTOS);
}

void ExifView::OnUpdateView(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(all_photos_.size() > 0);
}


//=======================================================================================

bool ExifView::PhotosPresent() const
{
	return !sorted_photos_.empty();
}

// returns true if there is at least one item selected
//
bool ExifView::Selection()
{
	return GetListCtrl().GetSelectedCount() > 0;
}

// returns true if there is exactly one item selected
//
bool ExifView::SingleSelection()
{
	return GetListCtrl().GetSelectedCount() == 1;
}


// return vector of pointers to all selected photos
//
void ExifView::GetSelectedPhotos(VectPhotoInfo& selected)
{
	selected.clear();
	GetListCtrl().GetSelectedItems(selected);
}


void ExifView::GetTaggedPhotos(VectPhotoInfo& tagged)
{
	tagged.clear();
	tagged.reserve(100);
	// copy tagged photos (this is missing copy_if)
	remove_copy_if(sorted_photos_.begin(), sorted_photos_.end(), back_inserter(tagged), std::not1(PhotoHasTags()));
}

//=======================================================================================


void ExifView::OnUpdateViewSort(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}

void ExifView::OnViewMode()
{
	SetNextViewMode();
}

void ExifView::OnUpdateViewMode(CCmdUI* cmd_ui)
{
}

void ExifView::OnViewThumbnails()						{ SetViewMode(VIEW_MODE_THUMBNAILS); }
void ExifView::OnViewDetails()							{ SetViewMode(VIEW_MODE_DETAILS); }
void ExifView::OnViewTiles()							{ SetViewMode(VIEW_MODE_TILES); }
void ExifView::OnViewPictures()							{ SetViewMode(VIEW_MODE_PICTURES); }

void ExifView::OnUpdateViewThumbnails(CCmdUI* cmd_ui)	{ cmd_ui->SetRadio(GetViewMode() == VIEW_MODE_THUMBNAILS); }
void ExifView::OnUpdateViewDetails(CCmdUI* cmd_ui)		{ cmd_ui->SetRadio(GetViewMode() == VIEW_MODE_DETAILS); }
void ExifView::OnUpdateViewTiles(CCmdUI* cmd_ui)		{ cmd_ui->SetRadio(GetViewMode() == VIEW_MODE_TILES); }
void ExifView::OnUpdateViewPictures(CCmdUI* cmd_ui)		{ cmd_ui->SetRadio(GetViewMode() == VIEW_MODE_PICTURES); }

//=======================================================================================

BOOL ExifView::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);
	dc->FillSolidRect(rect, g_Settings.AppColors()[AppColors::SecondarySeparator]);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void ExifView::OnRecursive()
{
	recursive_scan_ = !recursive_scan_;
	ReloadPhotos();
}

void ExifView::OnUpdateRecursive(CCmdUI* cmd_ui)		{ cmd_ui->SetCheck(recursive_scan_ ? 1 : 0); }

void ExifView::OnReadOnlyExif()
{
	read_only_photos_withEXIF_ = !read_only_photos_withEXIF_;
	ReloadPhotos();
}

void ExifView::OnUpdateReadOnlyExif(CCmdUI* cmd_ui)	{ cmd_ui->SetCheck(read_only_photos_withEXIF_ ? 1 : 0); }



void ExifView::OnRefresh()
{
	// TODO: refresh folder view

	ReloadPhotos();
}

void ExifView::OnUpdateRefresh(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


///////////////////////////////////////////////////////////////////////////////

// notification from the photo control
void ExifView::SortByColumn(int column, bool secondary_key)
{
	if (secondary_key)
	{
		if (sort_by_ != 0 && grouping_mode_ != GROUP_BY_SIMILARITY)
		{
			if (column >= 0 && column < columns_.size())
			{
				column++;
				// reverse or set secondary sorting key
				secondary_sorting_column_ = column == abs(secondary_sorting_column_) ? -secondary_sorting_column_ : column;

				//TODO: secondary sorting key indicator in PhotoCtrl...
				// GetListCtrl().SetSortColumn(secondary_sorting_column_, true <- secondary);

				ResortItems();
			}
		}
	}
	else
		SortPhotosByColumn(column);
}


void ExifView::SortPhotosByColumn(int column)
{
	if (column < 0 || column >= columns_.size())
	{
		ASSERT(false);
		return;
	}

	if (grouping_mode_ == GROUP_BY_SIMILARITY)
		OnCancelSortBySimilarity();

	column++;

	// if the same column selected for sorting then reverse the order
	if (column == abs(sort_by_))
	{
		sort_by_ = -sort_by_;
		// do not reset secondary key in this case
	}
	else
	{
		sort_by_ = column;
		// reset secondary key when primary key has changed
		secondary_sorting_column_ = 0;
	}
	GetListCtrl().SetSortColumn(sort_by_);
	ResortItems();
}


void ExifView::AddTaggedPhotos(VectPhotoInfo::iterator begin, VectPhotoInfo::iterator end,
								PhotoInfoPtr add_photo, size_t index)
{
	if (begin == end)
		return;

	PhotoInfo& photo= **begin;
//	ASSERT(!photo.GetTags().empty());

#ifdef MULTI_GROUPS
	String tags, label;
	PhotoCtrl::Icons icon= PhotoCtrl::LABEL;

	if (rated_photos_in_groups_ && photo.GetRating())		// rating takes precedence over tags (rated images show up first, then tagged ones)
	{
		// wsprintf used over stringstream for speed...
		TCHAR buf[128];
		int rating= photo.GetRating();
		wsprintf(buf, rating > 1 ? _T("%d 星") : _T("%d 星"), rating);
		label = buf;
		icon = PhotoCtrl::STAR;
	}
	else
	{
		for (PhotoTags::const_iterator it= photo.GetTags().begin(); it != photo.GetTags().end(); ++it)
		{
			if (!tags.empty())
			{
				tags += _T("\n");
				label += _T(", ");
			}
			tags += *it;
			label += *it;
		}
	}
#else

	ASSERT(!photo.GetTags().empty());
	const String& tags= photo.GetTags()[0];
#endif

	int groupId= INT_MIN + 10;	// space for rating (goes below that number)

	//TODO: fix it! if photos arrive at random order group id will vary and tag groups won't show up sorted;
	// to fix it groups should be rearranged (their ids reset!)

#ifdef MULTI_GROUPS
	if (rated_photos_in_groups_ && photo.GetRating())
		groupId -= photo.GetRating();	// rating is only 0 to 5
	else if (!photo.GetTags().empty() && !tags.empty())
#else
	if (photo.GetTags().size() == 1)
#endif
	{
		// single tag only
		if (!tags_.HasTag(tags))
			tags_.AddTag(tags);

		groupId += tags_.GetTagId(tags) + 1;
	}

#ifdef MULTI_GROUPS
#else
	String label= photo.GetTags().size() > 1 ? _T("多个标签") : _T("标签 \"") + tags + _T("\"");
#endif

	if (add_photo != 0 && end - begin == 1)
		GetListCtrl().InsertItem(add_photo, index, label, icon, groupId);
	else
		GetListCtrl().AddItems(begin, end, label, icon, groupId);
}


// this is fragile: catalog images have dir ids greater eq than 0x40000000
static PhotoCtrl::Icons GroupIcon(PhotoInfoPtr photo)
{
	PhotoCtrl::Icons icon= PhotoCtrl::FILM_ROLL;
	if (photo && photo->GetVisitedDirId() >= 0x40000000)
		icon = PhotoCtrl::BOOK;
	return icon;
}


//ExifView::GroupByDate CalcDateGroupingType(const VectPhotoInfo& photos)
////{ DONT_GROUP= 0, BY_YEAR, BY_MONTH, BY_DAY };
//{
//	return ExifView::BY_DAY;
//}

String GetPeriodName(ExifView::GroupByDate grouping, DateTime date)
{
	oStringstream ost;

	switch (grouping)
	{
	case ExifView::DONT_GROUP:
		return _T("-");

	case ExifView::BY_YEAR:
		return DateFmt(date, DateFmt_Year);

	case ExifView::BY_MONTH:
		return DateFmt(date, DateFmt_YearMonth);

	case ExifView::BY_DAY:
		return DateFmt(date, DateFmt_Medium);

	default:
		ASSERT(false);
		return false;
	}

	return ost.str();
}


//int GetPeriodId(const CTime& date)
//{
//	tm t;
//	if (date.GetLocalTm(&t))
//		return t.tm_year * 366 + t.tm_yday;
//	else
//		return 1;
//}


namespace {

struct Range
{
	Range(VectPhotoInfo& v) : begin(v.begin()), end(v.begin())	// empty range by default, starting from beggining
	{}

	Range(VectPhotoInfo::iterator b, VectPhotoInfo::iterator e) : begin(b), end(e)
	{}

	bool empty() const		{ return begin == end; }

	VectPhotoInfo::iterator begin;
	VectPhotoInfo::iterator end;
};

} // namespace


String ExifView::FormatFolderName(PhotoInfoPtr photo)
{
	return FolderName(cur_path_, Path(photo->GetDisplayPath()).GetDir());
}


void ExifView::ResortItems(bool reset_groups/*= false*/)
{
	SaveCurrentImage current(*this);

	if (grouping_mode_ == GROUP_BY_SIMILARITY)
	{
		// sorting in 'similarity' mode is impossible--switch back to folder group view
		SetGroupMode(GROUP_BY_FOLDERS);
		return;
	}

	CWaitCursor wait;

	BlockPhotoCtrlUpdates block(GetListCtrl());

	//TODO: if tagged_photos_in_groups_ remove only tag groups (or make sure their ids are repeatable)
	if (reset_groups || tagged_photos_in_groups_ || rated_photos_in_groups_)
		GetListCtrl().RemoveGroups();

	VectPhotoInfo photos= visible_photos_;

	if (photos.empty())
		return;

	Range rated(photos);
	Range tagged(photos);
	Range rest(photos.begin(), photos.end());

	if (rated_photos_in_groups_)
	{
		// partition photos: those with ratings to come first
		rest.begin = rated.end = partition(rest.begin, rest.end, PhotoHasRating());
	}

	if (tagged_photos_in_groups_)	// move tagged photos to the separate groups?
	{
		// partition photos: those with tags first
		tagged.begin = rest.begin;
		rest.begin = tagged.end = partition(rest.begin, rest.end, PhotoHasTags());
	}

	// essentially either group photos by folder or not; this includes sorting by photos' attribs
	switch (grouping_mode_)
	{
	case GROUP_BY_FOLDERS:
		sort(rest.begin, rest.end, SortPhotosByDirAndAttrib(this));
		break;

	case GROUP_BY_DATE:
		//date_grouping_ = CalcDateGroupingType(sorted_photos_);
		sort(rest.begin, rest.end, SortPhotosByDateAndAttrib(date_grouping_, this));
		break;

	case NO_GROUPING:
		sort(rest.begin, rest.end, SortPhotosByAttrib(this));
		break;

	default:
		ASSERT(false);
		break;
	}

	ASSERT(!photos.empty());

	// rated photos first; populate groups with rated photos

	if (!rated.empty())
	{
		// sort photos with rating (stars)
		sort(rated.begin, rated.end, SortPhotosByRating<SortPhotosByAttrib>(this));

		for (VectPhotoInfo::iterator it= rated.begin; it != rated.end; )
		{
			// find all photos with the same rating;
			// they are sorted by rating, so it's a range of adjacent values

			VectPhotoInfo::iterator last= find_if(it, rated.end, std::not1(std::bind2nd(EqualPhotosByRating(), *it)));

			AddTaggedPhotos(it, last, 0, 0);

			it = last;
		}
	}

	// tagged photos follow; populate groups with tagged photos

	if (!tagged.empty())
	{
		// sort tagged photos
		sort(tagged.begin, tagged.end, SortPhotosByTags<SortPhotosByAttrib>(this));

		tags_.Clear();

		for (VectPhotoInfo::iterator it= tagged.begin; it != tagged.end; )
		{
			// find all photos with the same tag;
			// they are sorted by tags, so it's a range of adjacent values

			VectPhotoInfo::iterator last= find_if(it, tagged.end, std::not1(std::bind2nd(EqualPhotoByTags(), *it)));

			AddTaggedPhotos(it, last, 0, 0);

			it = last;
		}
	}

	// finally remaining images

	switch (grouping_mode_)
	{
	case GROUP_BY_FOLDERS:
		for (VectPhotoInfo::iterator it= rest.begin; it != rest.end; )
		{
			// find all photos from the same directory (identical GetVisitedDirId())
			// they are sorted by GetVisitedDirId(), so it's a range of adjacent values

			VectPhotoInfo::iterator last= find_if(it, rest.end, std::not1(std::bind2nd(EqualPhotosByDir(), (*it)->GetVisitedDirId())));

			// add them all, this is one group
			PhotoCtrl::Icons icon= GroupIcon(*it);
			int group_id= (*it)->GetVisitedDirId();
			String name= FormatFolderName(*it);
			GetListCtrl().AddItems(it, last, name, icon, group_id);

			it = last;
		}
		break;

	case GROUP_BY_DATE:
		for (VectPhotoInfo::iterator it= rest.begin; it != rest.end; )
		{
			// find all photos from the same period (either year/month/day)
			// they are sorted by y/m/d respectively, so it's a range of adjacent values
			uint32 date= (*it)->GetDateStamp();

			VectPhotoInfo::iterator last= find_if(it, rest.end, std::not1(std::bind2nd(EqualPhotosByDate(date_grouping_), date)));

			// add them all, this is one group
			PhotoCtrl::Icons icon= PhotoCtrl::CALENDAR;
			int group_id= (*it)->GetDateStamp();// GetPeriodId(date);
			String name= GetPeriodName(date_grouping_, (*it)->GetDateTime());
			GetListCtrl().AddItems(it, last, name, icon, group_id);

			it = last;
		}
		break;

	default:
		GetListCtrl().AddItems(rest.begin, rest.end, _T(""), PhotoCtrl::NO_ICON, 0);
		break;
	}

	// store pointers to all photos in current sorting order for reference
	sorted_photos_.swap(photos);

	block.Release();

	// refresh the image viewer
	if (viewer_link_.IsValid())
		viewer_link_->PhotosListChange();
}


// inefficient but convenient composition of available sorting methods
struct Sort
{
	Sort(bool by_rates, bool by_tags, ExifView::GroupByDate by_date, bool by_dirs, ExifView* view)
		: sort_by_date_(by_date), by_attrib_(view)
	{
		by_rates_ = by_rates;
		by_tags_ = by_tags;
		by_dirs_ = by_dirs;
		by_date_ = by_date;
	}

	bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
	{
		if (by_rates_)
		{
			if (SortByRating()(p1, p2))
				return true;
			else if (SortByRating()(p2, p1))
				return false;
		}

		if (by_tags_)
		{
			if (SortByTags()(p1, p2))
				return true;
			else if (SortByTags()(p2, p1))
				return false;
		}

		if (by_date_)
		{
			if (sort_by_date_(p1, p2))
				return true;
			if (sort_by_date_(p2, p1))
				return false;
		}

		if (by_dirs_)
		{
			if (SortByDirs()(p1, p2))
				return true;
			if (SortByDirs()(p2, p1))
				return false;
		}

		return by_attrib_(p1, p2);
	}

private:
	bool by_rates_;
	bool by_tags_;
	ExifView::GroupByDate by_date_;
	SortByDate sort_by_date_;
	bool by_dirs_;
	SortPhotosByAttrib by_attrib_;
};


VectPhotoInfo::iterator ExifView::FindInsertPos(PhotoInfoPtr photo)
{
	ASSERT(grouping_mode_ != GROUP_BY_SIMILARITY);

	if (grouping_mode_ == GROUP_BY_SIMILARITY)
		return sorted_photos_.end();

	Sort s(rated_photos_in_groups_, tagged_photos_in_groups_, grouping_mode_ == GROUP_BY_DATE ? date_grouping_ : DONT_GROUP, grouping_mode_ == GROUP_BY_FOLDERS, this);

	// using sorting predicate find insertion point in a vector of sorted photos
	return lower_bound(sorted_photos_.begin(), sorted_photos_.end(), photo, s);
}


// add a new photo to the list ctrl preserving current order
void ExifView::AddItem(PhotoInfoPtr photo)
{
	ASSERT(photo);

	// reserve space before adding something in the middle
	if (sorted_photos_.capacity() == sorted_photos_.size())
		sorted_photos_.reserve(sorted_photos_.size() * 4 / 3);	// increase space by 33%

	bool sortByTags= tagged_photos_in_groups_ && PhotoHasTags()(photo);
	bool sortByStars= rated_photos_in_groups_ && PhotoHasRating()(photo);

	// find place in the sorted sequence to insert a new photo (preserving sort order)
	VectPhotoInfo::iterator insert= FindInsertPos(photo);

	// insert item if it's not yet there (new item)
	if (insert == sorted_photos_.end() || *insert != photo)
		insert = sorted_photos_.insert(insert, photo);
	else
	{
		ASSERT(false);	// item already in the sorted_photos_... deep troubles
		return;
	}

	// keep images in the list of visible items too
	visible_photos_.push_back(photo);

	VectPhotoInfo::iterator begin= sorted_photos_.begin();

	if (!sortByStars && rated_photos_in_groups_)
	{
		// skip all photos with ratings
//TODO: binary search
		begin = find_if(begin, sorted_photos_.end(), std::not1(PhotoHasRating()));
	}

	if (!sortByTags && tagged_photos_in_groups_)
	{
		// skip all photos with tags: given photo has no tags so it belongs to the groups after the tags
//TODO: binary search
		begin = find_if(begin, sorted_photos_.end(), std::not1(PhotoHasTags()));
	}

	typedef VectPhotoInfo::reverse_iterator rev;

	if (sortByStars)
	{
		// find first and last item in the same group as a newly inserted item
//TODO: binary search
		VectPhotoInfo::iterator first= find_if(rev(insert), sorted_photos_.rend(), not1(bind2nd(EqualPhotosByRating(), *insert))).base();

#if _DEBUG
		VectPhotoInfo::iterator last= find_if(insert, sorted_photos_.end(), not1(bind2nd(EqualPhotosByRating(), *insert)));
		ASSERT(insert >= first && insert < last);
#endif
		AddTaggedPhotos(insert, insert + 1, photo, insert - first);
	}
	else if (sortByTags)
	{
		// find first and last item in the same group as a newly inserted item
//TODO: binary search
		VectPhotoInfo::iterator first= find_if(rev(insert), rev(begin), not1(bind2nd(EqualPhotoByTags(), *insert))).base();

		AddTaggedPhotos(insert, insert + 1, photo, insert - first);
	}
	else if (grouping_mode_ == GROUP_BY_DATE)
	{
		// find first photo from the same year (or month, or day) as 'insert'
		// (images are sorted, so it's a range of adjacent values)

		// first photo in 'insert's directory
		VectPhotoInfo::iterator first= find_if(rev(insert), rev(begin), not1(bind2nd(EqualPhotosByDate(date_grouping_), (*insert)->GetDateStamp()))).base();

		int group_id= (*insert)->GetDateStamp(); //GetPeriodId((*insert)->GetDateTime());
		String name;
		if (!GetListCtrl().HasGroup(group_id))	// don't bother preparing name of existing group, it won't be used
			name = GetPeriodName(date_grouping_, (*insert)->GetDateTime());
		PhotoCtrl::Icons icon= PhotoCtrl::CALENDAR;

		// update group containing 'insert'
		GetListCtrl().InsertItem(photo, insert - first, name, icon, group_id);
	}
	else if (grouping_mode_ == GROUP_BY_FOLDERS)
	{
		// find first photo from same directory (identical GetVisitedDirId()) as 'insert'
		// they are sorted by GetVisitedDirId(), so it's a range of adjacent values

		// first photo in 'insert's directory
		VectPhotoInfo::iterator first= find_if(rev(insert), rev(begin), not1(bind2nd(EqualPhotosByDir(), (*insert)->GetVisitedDirId()))).base();

		int group_id= (*first)->GetVisitedDirId();
		String name;
		if (!GetListCtrl().HasGroup(group_id))	// don't bother preparing name of existing group, it won't be used
			name = FormatFolderName(*first);
		PhotoCtrl::Icons icon= GroupIcon(photo);

		// update group containing 'insert'
		GetListCtrl().InsertItem(photo, insert - first, name, icon, group_id);
	}
	else
	{
		String name;
		GetListCtrl().InsertItem(photo, insert - begin, name, PhotoCtrl::NO_ICON, 0);
	}
}


void ExifView::RemoveItem(PhotoInfoPtr photo, bool remove_from_ctrl, bool notify)
{
	try
	{
		VectPhotoInfo::iterator it= find(sorted_photos_.begin(), sorted_photos_.end(), photo);
		if (it != sorted_photos_.end())
			sorted_photos_.erase(it);
		else
		{ ASSERT(false); }	// photo not in the list?

		VectPhotoInfo::iterator er= find(visible_photos_.begin(), visible_photos_.end(), photo);
		if (er != visible_photos_.end())
			visible_photos_.erase(er);

		if (remove_from_ctrl)
			GetListCtrl().RemoveItem(photo, notify);
	}
	CATCH_ALL
}


void ExifView::SortBySimilarity()
{
	try
	{
		if (PhotoInfoPtr photo= CurrentItem())
		{
			GetListCtrl().RemoveAll();
			GetListCtrl().UpdateWindow();

			CWaitCursor wait;

			VectPhotoInfo sorted;
			::SortBySimilarity(visible_photos_, *photo, sorted, color_vs_shape_weight_);

			if (grouping_mode_ != GROUP_BY_SIMILARITY)
				old_grouping_ = grouping_mode_;
			grouping_mode_ = GROUP_BY_SIMILARITY;

			GetListCtrl().AddItems(sorted, _T("按相似度排序 \"") + photo->GetName() + _T("\""), PhotoCtrl::SIMILARITY, 0);

			tool_bar_wnd_.ShowCancelSortBtn(true);
		}
	}
	CATCH_ALL
}


static const int ID_SELECT_COLUMNS= 9998;
static const int ID_CUSTOM_RULES= 9999;

void ExifView::OnViewSortPopupMenu(CPoint pos)
{
	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	PopulateSortPopupMenu(menu, 1);

	// add custom rules item to bring up custom columns dlg
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_SELECT_COLUMNS, _T("选择列进行排序..."));
	menu.AppendMenu(MF_STRING, ID_CUSTOM_RULES, _T("自定义排序规则...\tShift+Ctrl+F"));

	int column= menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pos.x, pos.y, this);

	if (column == ID_SELECT_COLUMNS)
	{
		frame_->OptionsDlg(1);	// columns
	}
	else if (column == ID_CUSTOM_RULES)
	{
		frame_->SendMessage(WM_COMMAND, ID_DEFINE_CUSTOM_COLUMNS);
		// todo: add custom column
	}
	else if (column > 0)
		SortPhotosByColumn(column - 1);
}


void ExifView::PopulateSortPopupMenu(CMenu& popup, UINT first_cmd_it)
{
	UniqueLetter shortcuts;

	for (int col_index= 0; col_index < columns_.size(); ++col_index)
	{
		String item= cols_.Name(columns_[col_index]);
		//item.Replace(_T("&"), _T("&&"));
		shortcuts.SelectUniqueLetter(item);
		popup.InsertMenu(col_index, MF_BYPOSITION | MF_STRING, col_index + first_cmd_it, item.c_str());
	}

	if (sort_by_ != 0)
	{
		CBitmap* arrow= sort_by_ < 0 ? &arrow_down_ : &arrow_up_;
		popup.SetMenuItemBitmaps(abs(sort_by_) - 1, MF_BYPOSITION, arrow, arrow);
	}
}


// stop scanning folders
//
void ExifView::OnStopScanning()
{
	if (reload_thread_)
		reload_thread_->Quit();
}

void ExifView::OnUpdateStopScanning(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(); //reload_thread_ != 0);
}

///////////////////////////////////////////////////////////////////////////////

ExifView::CViewSettings::CViewSettings()
{
	pv_columns_ = 0;
	data_ = 0;
	byte_data_ = 0;
}

ExifView::CViewSettings::~CViewSettings()
{
	if (byte_data_)
		delete [] byte_data_;
}


size_t ExifView::CViewSettings::Data::Size(size_t count)
{
	ASSERT(count > 0);
	return sizeof Data + (count - 1) * sizeof uint16;
}

ExifView::CViewSettings::CViewSettings(ExifView::ViewMode view_mode, ExifView::GroupMode grouping,
	int sorting_order, bool recursive_scan, bool photos_with_exif_only, const std::vector<uint16>& columns)
{
	byte_data_ = 0;
	pv_columns_ = &columns;
	buffer_.resize(Data::Size(columns.size()));
	data_ = reinterpret_cast<Data*>(&buffer_.front());

	data_->version_ = 0x0002;
	data_->view_mode_ = static_cast<uint16>(view_mode);
	data_->grouping_ = static_cast<uint16>(grouping);
	data_->sorting_order_ = static_cast<int16>(sorting_order);
	data_->recursive_scan_ = recursive_scan;
	data_->read_photos_withEXIF_only_ = photos_with_exif_only;
	data_->count_ = static_cast<uint16>(columns.size());
	for (int i= 0; i < columns.size(); ++i)
		data_->columns_[i] = columns[i];
}


bool ExifView::CViewSettings::Store(const TCHAR* section, const TCHAR* entry)
{
	if (data_ == 0)
	{
		ASSERT(false);
		return false;
	}
	return !!AfxGetApp()->WriteProfileBinary(section, entry, &buffer_.front(), static_cast<UINT>(buffer_.size()));
}


bool ExifView::CViewSettings::Restore(const TCHAR* section, const TCHAR* entry)
{
	ASSERT(data_ == 0);

	CWinApp* app= AfxGetApp();

	byte_data_ = 0;
	UINT bytes= 0;
	app->GetProfileBinary(section, entry, &byte_data_, &bytes);
	if (byte_data_ && bytes)
	{
		data_ = reinterpret_cast<Data*>(byte_data_);
		if (data_->version_ != 0x0002 || Data::Size(data_->count_) != bytes)
			return false;

		return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////


// store current settings
//
void ExifView::SaveSettings()
{
	if (frame_)
	{
		CViewSettings vs(view_mode_, grouping_mode_, sort_by_, recursive_scan_, read_only_photos_withEXIF_, columns_);

		vs.Store(frame_->GetRegSection(), REG_VIEW_SETTINGS);

//		GetListCtrl().ModifyStyle(LVS_TYPEMASK, LVS_REPORT);

		if (CHeaderCtrl* ctrl= GetListCtrl().GetHeaderCtrl())
		{
			int col_count= ctrl->GetItemCount();

			std::vector<int> col_order;
			GetListCtrl().GetColumnOrderArray(col_order);
			WriteProfileVector(frame_->GetRegSection(), REG_VIEW_COL_ORDER, col_order);

			std::vector<int16> width_array;
			width_array.resize(col_count);
			for (int i= 0; i < col_count; ++i)
			{
				width_array[i] = GetListCtrl().GetColumnWidth(i);
				if (width_array[i] == 0)
				{ ASSERT(false); }
			}
			WriteProfileVector(frame_->GetRegSection(), REG_VIEW_COL_WIDTH, width_array);
		}
	}

//	if (tool_bar_wnd_.IsSliderValid())
//		profile_thumb_size_ = tool_bar_wnd_.GetThumbPos();
	profile_show_labels_ = show_labels_;
	profile_show_balloons_ = GetListCtrl().IsBalloonInfoEnabled();
	profile_show_tags_ = GetListCtrl().ShowingTagText();
	profile_show_marker_ = GetListCtrl().ShowingMarker();
	fieldSelectionProfile1_ = customInfoFields_[0];
	fieldSelectionProfile2_ = customInfoFields_[1];

	profile_sticky_selection_ = GetListCtrl().StickySelectionFlag();
	profile_group_by_tags_ = tagged_photos_in_groups_;
	profile_group_by_stars_ = rated_photos_in_groups_;

	profile_color_vs_shape_weight_ = color_vs_shape_weight_;

	profile_show_time_line_ = distribution_bar_wnd_.m_hWnd && (distribution_bar_wnd_.GetStyle() & WS_VISIBLE) != 0;

	CWinApp* app= AfxGetApp();

	// save custom filters
	CString section= REGISTRY_ENTRY_EXIF;
	section += '\\';
	section += REG_CUSTOM_FILTERS;

	int version= 2;
	std::vector<String> filters;
	filters_.SerializeTo(filters, version);
	const UINT count= static_cast<UINT>(filters.size());

	for (UINT i= 0; i < count; ++i)
	{
		TCHAR key[200];
		wsprintf(key, _T("f%d-%d"), static_cast<int>(sizeof(TCHAR)), static_cast<int>(i));
		app->WriteProfileBinary(section, key, (BYTE*)(filters[i].data()), static_cast<UINT>(filters[i].length() * sizeof(TCHAR)));
	}
	app->WriteProfileInt(section, _T("count"), count);

	{
		const size_t count= filter_dlg_.GetPanelCount();
		for (size_t panel= 0; panel < count; ++panel)
		{
			bool expanded= true;
			int height= 0, flags= 0;
			filter_dlg_.GetPanelUISettings(panel, height, expanded, flags);

			filter_panel_height_.Store(panel, height);
			filter_panel_expanded_.Store(panel, expanded);
			filter_panel_flags_.Store(panel, flags);
		}
	}
}


void ExifView::ResetICC()
{
	transform_.Reset(g_Settings.default_photo_->profile_, g_Settings.main_wnd_->profile_, g_Settings.main_wnd_->rendering_);
	icc_ = g_Settings.main_wnd_->enabled_ && g_Settings.default_photo_->enabled_;
}


void ExifView::OptionsChanged(OptionsDlg& dlg)
{
	if (dlg.ColumnsChanged())	// columns selected?
	{
		std::vector<uint16>& sel= dlg.SelectedColumns();
		SelectColumns(sel);
	}

	SetPhotoCtrlColors();
	SetViewerColors();

	ResetICC();

/*
	if (g_Settings.viewer_default_colors_)
		preview_wnd_.ResetColors();
	else
		preview_wnd_.SetColors(g_Settings.viewer_colors_);
*/

	// halftone drawing effects photo ctrl too
	GetListCtrl().SetHalftoneDrawing(g_Settings.display_method_ == 2);

	GetListCtrl().Invalidate();

	if (viewer_link_.IsValid())
		viewer_link_->ResetSettings();
}


// set photo ctrl colors based on profile settings
void ExifView::SetPhotoCtrlColors()
{
	GetListCtrl().SetColors(g_Settings.main_wnd_colors_.Colors());
	GetListCtrl().SetTagFont(g_Settings.img_tag_font_);

	tool_bar_wnd_.SetBackgroundColor(g_Settings.pane_caption_colors_[SnapFrame::C_BAR].SelectedColor());

	COLORREF backgnd = g_Settings.AppColors()[AppColors::Background];
	COLORREF text = g_Settings.AppColors()[AppColors::Text];
	tab_ctrl_.SetTabColor(backgnd, backgnd, text);
/*
	if (g_Settings.list_ctrl_sys_colors_)
	{
		GetListCtrl().ResetColors();
//		preview_wnd_.ResetColors();
//		preview_wnd_.SetBkColor(::GetSysColor(COLOR_WINDOW));
	}
	else
	{
		GetListCtrl().SetColors(g_Settings.list_ctrl_colors_);
//		preview_wnd_.SetBkColor(g_Settings.list_ctrl_colors_[0]);	// same backgnd as main wnd
//		preview_wnd_.SetTextColor(g_Settings.viewer_colors_[1]);	// description text color
	}
*/
}

// set viewer colors based on profile settings
void ExifView::SetViewerColors()
{
	if (!viewer_link_.IsValid())
		return;

	//if (g_Settings.viewer_default_colors_)
	//	viewer_link_->ResetColors();
	//else
	{
		viewer_link_->SetColors(g_Settings.viewer_wnd_colors_.Colors());
		viewer_link_->SetUIBrightness(g_Settings.viewer_ui_gamma_correction_);
	}
}


void ExifView::SelectColumns(std::vector<uint16>& sel)
{
	if (sel.size() == 0)
		sel.push_back(0);

	typedef std::map<uint16, uint8> Map;
	Map present;
	//present.resize(cols_.MaxCount());
	//fill(present.begin(), present.end(), 0);

	// mark existing columns
	for (int i= 0; i < columns_.size(); ++i)
		present[columns_[i]] = 1;

	// mark new columns
	for (int j= 0; j < sel.size(); ++j)
		present[sel[j]] |= 2;

	int col_index= GetListCtrl().GetHeaderCtrl()->GetItemCount() - 1;	// last column index

	if (!present.empty())
	{
		Map::iterator begin= present.begin();
		Map::iterator it= present.end();
		for (--it; ; --it)
		{
			ASSERT(col_index >= 0);
			int k= it->first;

			switch (it->second)
			{
			case 0:		// no column
				break;
			case 1:		// column to delete
				GetListCtrl().DeleteColumn(col_index);
				if (abs(sort_by_) == col_index + 1)
				{
					sort_by_ = 0;
					GetListCtrl().SetSortColumn(0);
				}
				--col_index;
				break;
			case 2:		// column to insert
				GetListCtrl().InsertColumn(col_index + 1, cols_.ShortName(k), cols_.Alignment(k), cols_.DefaultWidth(k));
				break;
			case 3:		// no change
				--col_index;
				break;
			}

			if (it == begin)
				break;
		}
	}
	ASSERT(col_index == -1);

	columns_.swap(sel);
	sel.clear();
}


///////////////////////////////////////////////////////////////////////////////

void ExifView::OnMarkImage()						{ GetListCtrl().SelectItems(PhotoCtrl::TOGGLE_CURRENT); }
void ExifView::OnUpdateMarkImage(CCmdUI* cmd_ui)	{ cmd_ui->Enable(!sorted_photos_.empty()); }

void ExifView::OnMarkAll()							{ GetListCtrl().SelectItems(PhotoCtrl::ALL); }
void ExifView::OnUpdateMarkAll(CCmdUI* cmd_ui)		{ cmd_ui->Enable(); }

void ExifView::OnMarkNone()							{ GetListCtrl().SelectItems(PhotoCtrl::NONE); }
void ExifView::OnUpdateMarkNone(CCmdUI* cmd_ui)		{ cmd_ui->Enable(); }

void ExifView::OnMarkInvert()						{ GetListCtrl().SelectItems(PhotoCtrl::INVERT); }
void ExifView::OnUpdateMarkInvert(CCmdUI* cmd_ui)	{ cmd_ui->Enable(); }


// create window
bool ExifView::Create(CWnd* parent)
{
	if (!PaneWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;

	return true;
}


int ExifView::OnCreate(LPCREATESTRUCT create_struct)
{
	if (PaneWnd::OnCreate(create_struct) == -1)
		return -1;

	GetListCtrl().Create(this, this, g_LIST_CTRL_ID2);

	SetPhotoCtrlColors();

	// halftone drawing effects photo ctrl too
	GetListCtrl().SetHalftoneDrawing(g_Settings.display_method_ == 2);

	if (!tool_bar_wnd_.Create(this, array_count(thumb_size_levels), -1))//, IsCaptionBig()))
		return -1;

	img_size_index_ = clamp<int>(profile_thumb_size_, 0, array_count(thumb_size_levels) - 1);

	tool_bar_wnd_.SetThumbPos(img_size_index_);

	images_across_index_ = clamp<int>(profile_images_across_, 0, array_count(images_across_levels) - 1);

	switch (profile_show_labels_)
	{
	case 0:
		show_labels_ = NO_LABELS;	break;
	case 1:
		show_labels_ = SHOW_DATE_TIME;	break;
	case 2:
		show_labels_ = SHOW_FILE_NAME;	break;
	case 3:
		show_labels_ = CUSTOM_INFO;	break;
	case 4:
		show_labels_ = SHOW_FILE_NAME_EXT;	break;
	default:
		ASSERT(false);
	}

	customInfoFields_[0] = fieldSelectionProfile1_;
	customInfoFields_[1] = fieldSelectionProfile2_;

	GetListCtrl().ShowItemLabel(show_labels_ != NO_LABELS);
	GetListCtrl().EnableBalloonInfo(profile_show_balloons_);
	GetListCtrl().ShowTagText(profile_show_tags_);
	GetListCtrl().ShowMarker(profile_show_marker_);
	GetListCtrl().SetImageSize(Pixels(thumb_size_levels[img_size_index_]));
	GetListCtrl().SetItemsAcross(images_across_levels[images_across_index_]);

	tagged_photos_in_groups_ = profile_group_by_tags_;
	rated_photos_in_groups_ = profile_group_by_stars_;

	GetListCtrl().StickySelection(profile_sticky_selection_);
	GetListCtrl().ClickAndSelect(true);

	color_vs_shape_weight_ = profile_color_vs_shape_weight_;

	AddBand(&tool_bar_wnd_, this, tool_bar_wnd_.GetMinMaxWidth());

	if (!distribution_bar_wnd_.Create(this))
		return -1;

	if (!tab_ctrl_.Create(WS_CHILD | /*WS_VISIBLE |*/ CTCS_AUTOHIDEBUTTONS | CTCS_TOOLTIPS | CTCS_DRAGMOVE | CTCS_DRAGCOPY | CTCS_BUTTONSAFTER /*| CTCS_TOP*/, CRect(0,0,0,0), this, IDC_TAB))
		return -1;

	tab_ctrl_.SetFont(GetFont());

	// add tabs (main page and filters)
	for (UINT i= 0; i < FIXED_TABS; ++i)
	{
		FilterData* filter= GetFilterData(i);
		if (filter == 0)
			break;

		tab_ctrl_.InsertItem(i, filter->name_.c_str());
	}
	SetCurFilterTab(0);
	tab_ctrl_.SetItemTooltipText(0, _T("浏览器主界面 (Ctrl+F)"));
	tab_ctrl_.SetItemTooltipText(1, _T("图像过滤面板 (Ctrl+F)"));
	tab_ctrl_.SetIdealHeight(GetDefaultLineHeight() * 20 / 10);

	filter_page_.name_ = _T("过滤");

	if (!filter_dlg_.Create(this, Tags::GetTagCollection(), this))
		return -1;

	{
		CRect rect(0,0,0,0);
		filter_dlg_.GetWindowRect(rect);
		filter_dlg_width_ = rect.Width() + 40;
	}

	bool show_time_line= profile_show_time_line_;
	distribution_bar_wnd_.ShowWindow(show_time_line ? SW_SHOWNA : SW_HIDE);
	SetFaintCaptionEdge(show_time_line);

	distribution_bar_wnd_.SetFilterCallback(boost::bind(&ExifView::FilterPhotos, this, _1, _2, _3, _4));
	distribution_bar_wnd_.SetCancelCallback(boost::bind(&ExifView::CancelDateFiltering, this),
		boost::bind(&ExifView::IsDateTimeFilterActive, this));

	return 0;
}

/*
void ExifView::NameChanged()		// filter name changed
{
	if (FilterData* current= GetCurrentFilterData())
		current->name_ = filter_dlg_.GetFilterName();
}
*/

void ExifView::GetCurrentFilter(FilterData& filter)
{
	filter_dlg_.GetCurrentFilter(filter);
	filter.time_.from = sel_hist_date_from_;
	filter.time_.to = sel_hist_date_to_;
}

/*
void ExifView::StoreFilter()		// store current filter
{
	FilterData filter;
	GetCurrentFilter(filter);

	if (filters_.FindFilter(filter.name_))
	{
		// this name is already in use
		filter_dlg_.ShowNameInUseErr();
	}
	else
	{
		filters_.AddFilter(filter);
		int index= tab_ctrl_.InsertItem(-1, filter.name_.c_str());
		SetCurFilterTab(index);
		FilterPaneSwitch();
		tab_ctrl_.BlinkSelectedTab(2);
	}
}


void ExifView::DeleteFilter()		// delete current filter
{
	int tab= GetCurFilterTab();

	bool custom= tab >= FIXED_TABS;

	if (custom)
	{
		filters_.DeleteFilter(tab - FIXED_TABS);
		tab_ctrl_.DeleteItem(tab);
		SetCurFilterTab(tab - 1);
		FilterPaneSwitch();
	}
}


void ExifView::UpdateFilter()		// update current filter
{
	FilterData filter;
	GetCurrentFilter(filter);

	int tab= GetCurFilterTab();
	FilterData* cur= GetFilterData(tab);
	if (cur == 0)
		return;

	if (filters_.FindFilter(filter.name_) != cur)
	{
		// this name is already in use
		filter_dlg_.ShowNameInUseErr();
	}
	else
	{
		*cur = filter;
		tab_ctrl_.SetItemText(tab, cur->name_.c_str());
	}
}
*/

int ExifView::GetCurFilterTab() const
{
	return filter_tab_;
}

void ExifView::SetCurFilterTab(int tab)
{
	filter_tab_ = tab;
}


void ExifView::FilterParamsChanged()		// notification: filter params have been modified
{
	try
	{
		FilterData filter;
		GetCurrentFilter(filter);

		if (FilterData* current= GetCurrentFilterData())
			*current = filter;

		// apply filter
		ApplyFilter(filter);
	}
	CATCH_ALL
}


FilterData* ExifView::GetFilterData(size_t index)
{
	if (index == 0)
		return &main_page_;
	else if (index == 1)
		return &filter_page_;
	else
		return filters_.GetFilter(index - 2);
}


const FilterData* ExifView::GetCurrentFilterData() const
{
	return const_cast<ExifView*>(this)->GetCurrentFilterData();
}


FilterData* ExifView::GetCurrentFilterData()
{
	return GetFilterData(GetCurFilterTab());
}


void ExifView::FilterSet(const VectPhotoInfo& input, const FilterData& filter, VectPhotoInfo& output)
{
	output.clear();

	size_t total_count= input.size();

	if (filter.IsActive())
	{
		output.reserve(100);

		FilterByTags by_tags(filter.selected_tags_);
		FilterByRating by_stars(filter.stars_);
		FilterByText by_text(filter.text_.include, filter.text_.exclude);
		FilterByExpression by_expr(filter.expression_.rule);
		FilterByTime by_time(filter.time_.from, filter.time_.to);

		for (size_t index= 0; index < total_count; ++index)
		{
			PhotoInfoPtr photo= input[index];
			ASSERT(photo);
			if (photo == 0)
				break;

			if (!by_tags(photo))
				continue;

			if (!by_stars(photo))
				continue;

			if (!by_text(photo))
				continue;

			if (!by_time(photo))
				continue;

			if (!by_expr(photo))
				continue;

			output.push_back(photo);
		}
	}
	else
		output = input;
}


void ExifView::ApplyFilter(const FilterData& filter)
{
	CWaitCursor wait;

	// what this really does is that it resets current item if there is no one set after filtering
	SaveCurrentImage current(*this);

	VectPhotoInfo output;
	VectPhotoInfo input;
	all_photos_.Copy(input);	// lock once, and copy pointers to 'input'

	FilterSet(input, filter, output);

	distribution_bar_wnd_.ClearSelection();
	sel_hist_date_from_ = filter.time_.from;
	sel_hist_date_to_ = filter.time_.to;

	// this check doesn't always work, all_photos may have different order than items in visible_photos_
	if (visible_photos_ == output)
		return;		// no changes

	// checking if the two vectors contain different elements:
	if (visible_photos_.size() == output.size())
	{
		// this sorting is still going to be cheaper than emptying and populating PhotoCtrl

		VectPhotoInfo current= output;
		std::sort(current.begin(), current.end());

		VectPhotoInfo visible= visible_photos_;
		std::sort(visible.begin(), visible.end());

		if (current == visible)
			return;
	}

	BlockPhotoCtrlUpdates block(GetListCtrl());

	EmptyList(false);

	GetListCtrl().UpdateWindow();

	visible_photos_ = output;
	synched_item_count_ = input.size();

	if (grouping_mode_ == GROUP_BY_SIMILARITY)
		OnCancelSortBySimilarity();
	else
		ResortItems(true);

	RebuildHistogram(&filter);
}


BOOL ExifView::OnTbDropDown(UINT, NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info_tip->iItem)
	{
	case ID_VIEW_SORT:		// sorting
		{
			CRect rect= tool_bar_wnd_.GetSortBtnRect(info_tip->iItem);
			CPoint pos(rect.left, rect.bottom);
			OnViewSortPopupMenu(pos);
		}
		break;

	case ID_SORT_BY_SIMILARITY:	// color vs shape slider
		{
			CRect rect= tool_bar_wnd_.GetSortBtnRect(info_tip->iItem);
			CPoint pos(rect.left, rect.bottom);
			OnSortBySimilarityOptions(pos);
		}
		break;

	case ID_VIEW:			// open viewer popup
		{
			CRect rect= tool_bar_wnd_.GetMainBtnRect(info_tip->iItem);
			CPoint pos(rect.left, rect.bottom);
			OnViewPhotosPopupMenu(pos);
		}
		break;

	case ID_SHOW_OPTIONS:
		{
			CRect rect= tool_bar_wnd_.GetViewBtnRect(info_tip->iItem);
			CPoint pos(rect.left, rect.bottom);
			OnShowOptionsPopupMenu(pos);
		}
		break;

	default:
		return false;	// notification not handled
	}

	return true;
}


void ExifView::Resize()
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (tab_ctrl_.m_hWnd && tab_ctrl_.IsWindowVisible())
	{
		int h= tab_ctrl_.GetIdealHeight();
		tab_ctrl_.MoveWindow(rect.left, rect.bottom - h, rect.Width(), h);
		rect.bottom -= h;
		if (rect.Height() < 0)
			rect.bottom = rect.top;
	}

	if (distribution_bar_wnd_.m_hWnd && (distribution_bar_wnd_.GetStyle() & WS_VISIBLE))
	{
		distribution_bar_wnd_.MoveWindow(rect.left, rect.top, rect.Width(), distribution_bar_wnd_.GetHeight());
		rect.top += distribution_bar_wnd_.GetHeight();
		if (rect.Height() < 0)
			rect.bottom = rect.top;
	}
	rect.top += 1;
	if (filter_dlg_.m_hWnd && (filter_dlg_.GetStyle() & WS_VISIBLE))
	{
		filter_dlg_.SetWindowPos(0, rect.left, rect.top, filter_dlg_width_, rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		rect.left += filter_dlg_width_;
		if (rect.Width() < 0)
			rect.right = rect.left;
	}

	if (GetListCtrl().m_hWnd)
		GetListCtrl().MoveWindow(rect);
}


void ExifView::OnSize(UINT type, int cx, int cy)
{
	PaneWnd::OnSize(type, cx, cy);

	Resize();
}


void ExifView::ActiveMainWnd()
{
	// make sure preview contains up to date image; it gets out of sync when browsing photos in img viewer
	if (PhotoInfoPtr photo= CurrentItem())
	{
		// send notification to peer pane windows
		SendNotification(&PaneWnd::CurrentPhotoChanged, photo);
	}
}


void ExifView::ActivatePane(bool active)
{
	if (active)
		if (IsTopParentActive())	// take the focus if this frame/view/pane is now active
			GetListCtrl().SetFocus();
}

/*
void ExifView::OnActivateView(BOOL activate, CView* activate_view, CView* deactive_view)
{
	UNUSED(activate_view);   // unused in release builds

	if (activate)
	{
		ASSERT(activate_view == this);

		// take the focus if this frame/view/pane is now active
		if (IsTopParentActive())
			GetListCtrl().SetFocus();
	}
} */


/////////////////////////////////////////////////////////////////////////////////////////////
//
// File Operations
//
/////////////////////////////////////////////////////////////////////////////////////////////

// concatenate path of given photos using '\0' separator
extern void ConcatenatedPath(const VectPhotoInfo& photos, String& paths)
{
	paths.resize(0);

	if (!photos.empty())
	{
		// collect null separated selected photos paths
		paths.reserve((photos.front()->GetOriginalPath().length() + 2) * photos.size());

		// using original photo path; this is important for catalog images
		for (VectPhotoInfo::const_iterator it= photos.begin(); it != photos.end(); ++it)
		{
			String str= (*it)->GetOriginalPath();
			if (!str.empty())
				paths += str + _T('\0');
			for (size_t assoc_file_idx= 0; ; ++assoc_file_idx)
			{
				Path path= (*it)->GetAssociatedFilePath(assoc_file_idx);
				if (path.empty())
					break;

				if (path.FileExists())	// this is potentially *very* expensive! (for multiple files)
					paths += path + _T('\0');
			}
		}
	}

	if (!paths.empty())
		paths += _T('\0');
}


void ExifView::OnTaskDelete()
{
	DeletePhotos(true);
}


void ExifView::DeletePhotos(bool warn)
{
	try
	{
		// files to delete
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		if (selected.empty())
		{
			if (warn)
				new BalloonMsg(&GetListCtrl(), _T("没有选定的照片"),
					_T("请选择要删除的照片."), BalloonMsg::IERROR);
			return;
		}

		const size_t count= selected.size();
		for (size_t i= 0; i < count; ++i)
			if (!selected[i]->CanDelete())
			{
				if (warn)
					new BalloonMsg(&GetListCtrl(), _T("未能删除照片"),
					_T("选定的照片不能删除."), BalloonMsg::IERROR);
				return;
			}

		if (warn)
		{
			CString msg;
			if (selected.size() == 1)
			{
				msg = _T("有一张选定的照片,\n\n");
				msg += selected.front()->GetOriginalPath().GetFileNameAndExt().c_str();
				msg += _T("\n\n确认删除?");
			}
			else
				msg.Format(_T("有 %d 张选定的照片.\n\n确认删除?"), static_cast<int>(selected.size()));

			DeleteConfirmationDlg dlg(this, msg);
			if (dlg.DoModal() != IDOK)
				return;
//			if (AfxMessageBox(msg, MB_YESNO | MB_ICONQUESTION, 0) != IDYES)
//				return;
		}

		CWaitCursor wait;

		// collect null separated selected photos paths
		String src;
		ConcatenatedPath(selected, src);
		if (src.empty())
		{
			ASSERT(false);
			return;
		}

		// perform delete operation
		SHFILEOPSTRUCT op;
		op.hwnd = *this;
		op.wFunc = FO_DELETE;
		op.pFrom = src.data();
		op.pTo = 0;
		op.fFlags = FOF_NOCONFIRMATION | FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
		op.fAnyOperationsAborted = false;
		op.hNameMappings = 0;
		op.lpszProgressTitle = 0;
//TRACE(_T("Deleting photo: %s\n"), src.c_str());
		if (::SHFileOperation(&op) == 0 && !op.fAnyOperationsAborted)
		{
			// if photos were deleted remove them from list ctrl window
			RemoveSelected(selected);
		}
	}
	CATCH_ALL
}


void ExifView::OnUpdateDelete(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(Selection());
}

// key down notifications from photo ctrl
//
void ExifView::KeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	bool extend_sel_key= ::GetKeyState(VK_SHIFT) < 0;
	bool toggle_sel_key= ::GetKeyState(VK_CONTROL) < 0;
	bool alt_key=		::GetKeyState(VK_MENU) < 0;

	switch (chr)
	{
	case VK_DELETE:
		if (!extend_sel_key && !toggle_sel_key && !alt_key)
			OnTaskDelete();
		break;

	case VK_ESCAPE:
		if (!extend_sel_key && !toggle_sel_key && !alt_key)
			OnEscape();
		break;

	case VK_RETURN:
		if (!extend_sel_key && !toggle_sel_key && !alt_key)
			OnView();
		break;
	}
}


void ExifView::FileOperation(bool copy)
{
	try
	{
		// files to copy/move
		VectPhotoInfo selected;

		GetSelectedPhotos(selected);

		if (selected.empty())
		{
			new BalloonMsg(&GetListCtrl(), _T("没有选定的照片"),
				_T("请先选择要复制/移动的照片."), BalloonMsg::IERROR);
			return;
		}

		FileOperation(selected, copy, this);
	}
	CATCH_ALL
}


void ExifView::FileOperation(const VectPhotoInfo& photos, bool copy, CWnd* parent)
{
	if (photos.empty())
		return;

	try
	{
		String src;
		{
			CWaitCursor wait;
			ConcatenatedPath(photos, src);
			if (src.empty())
			{
				if (!photos.empty())
					AfxMessageBox(_T("选定的图像不能复制."), MB_OK | MB_ICONWARNING);
				return;
			}
		}

		Profile<String> profileDestPath(REGISTRY_ENTRY_EXIF, copy ? _T("DestPath") : _T("DestPathMove"), _T("c:\\"));
		String dest_path= profileDestPath;
//		FileOperDlg dlg(copy, dest_path.c_str(), this);
		CopyMoveDlg dlg(copy, dest_path.c_str(), parent, current_folder_);
		HeaderDialog dlgHdr(dlg, copy ? _T("复制") : _T("移动"), copy ? HeaderDialog::IMG_COPY : HeaderDialog::IMG_MOVE, parent);
		if (dlgHdr.DoModal() != IDOK)
			return;

		dest_path = dlg.DestPath();
		profileDestPath = dest_path;
		dest_path += _T('\0');

/*
		src.reserve(photos.front()->path_.length() * photos.size());

		for (VectPhotoInfo::const_iterator it= photos.begin(); it != photos.end(); ++it)
			src += (*it)->path_ + _T('\0');

		src += _T('\0');
*/
		SHFILEOPSTRUCT op;
		memset(&op, 0, sizeof op);
		op.hwnd   = parent ? parent->m_hWnd : m_hWnd;
		op.wFunc  = copy ? FO_COPY : FO_MOVE;
		op.pFrom  = src.data();
		op.pTo    = dest_path.data();
		op.fFlags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
		op.fAnyOperationsAborted = false;
		op.hNameMappings = 0;
		op.lpszProgressTitle = 0;
		if (::SHFileOperation(&op) == 0)
		{
			if (!copy)
			{
				VectPhotoInfo selected;
				GetSelectedPhotos(selected);
				if (photos != selected)
					SelectPhotos(photos);
				RemoveSelected(photos);
			}
		}
	}
	CATCH_ALL
}


void ExifView::OnTaskCopy()
{
	FileOperation(true);
}

void ExifView::OnTaskMove()
{
	FileOperation(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////


void ExifView::OnTaskResize()
{
	try
	{
		// photos to resize
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		if (selected.empty())
		{
			new BalloonMsg(&GetListCtrl(), _T("没有选定的照片"),
				_T("请先选择要改变尺寸的照片."), BalloonMsg::IERROR);
			return;
		}

		CTaskResize resize(selected, this);
		resize.Go();
	}
	CATCH_ALL
}


void ExifView::OnTaskGenSlideShow()
{
	try
	{
		// photos to generate slide show
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		if (selected.empty())
		{
			new BalloonMsg(&GetListCtrl(), _T("没有选定的照片"),
				_T("请先选择要制作幻灯片的照片."), BalloonMsg::IERROR);
			return;
		}

		CTaskGenSlideShow generator(selected);
		generator.Go();
	}
	CATCH_ALL
}


void ExifView::OnTaskGenHTMLAlbum()
{
	try
	{
		// photos to generate HTML album
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		if (selected.empty())
		{
			new BalloonMsg(&GetListCtrl(), _T("没有选定的照片"),
				_T("请先选择要制作 HTML 相册的照片."), BalloonMsg::IERROR);
			return;
		}

		CTaskGenHTMLAlbum generator(selected, this);
		generator.Go();
	}
	CATCH_ALL
}


void ExifView::OnUpdateSelectionRequired(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(Selection());
}


void ExifView::OnTaskHistogram()
{
	try
	{
		if (PhotoInfoPtr photo= CurrentItem())
		{
			HistogramDlg dlgHist(*photo, global_photo_cache.get());
			HeaderDialog dlg(dlgHist, _T("直方图"), HeaderDialog::IMG_HISTOGRAM);

			if (dlgHist.PhotoLoaded())
				dlg.DoModal();
		}
	}
	CATCH_ALL
}


/////////////////////////////////////////////////////////////////////////////////////////////


//void ExifView::DoFiltering()
//{
//	BlockPhotoCtrlUpdates block(GetListCtrl());
//
//	RemoveItems();
//
//	// resync will use filter info
//	SyncListCtrl(all_photos_.size());
//
//	distribution_bar_wnd_.ClearSelection();
//
//	RebuildHistogram();
//}

// display only photos with given text in description (pass null to cancel filtering)
//
void ExifView::FilterPhotos(const TCHAR* text)
{
	try
	{
		if (text)
			main_page_.text_.include = text;
		else
			main_page_.text_.ClearAll();

		if (GetCurFilterTab() == 0)
		{
			// reapply filter
			ApplyFilter(main_page_);
		}
		else
		{
			// switch back to the main page; filter will be applied
			SetCurFilterTab(0);
			FilterPaneSwitch();
		}

		if (!filter_changed_event_.empty())
			filter_changed_event_(IsMainPageFilterActive());
/*
		{
			if (filter_.empty())
				return;

			filter_.clear();
		}

		DoFiltering(); */
	}
	CATCH_ALL
}


void ExifView::FilterPhotos(DateTime from, DateTime to, DateTime hist_from, DateTime hist_to)
{
	try
	{
		hist_date_from_ = hist_from;
		hist_date_to_ = hist_to;
		sel_hist_date_from_ = from;
		sel_hist_date_to_ = to;

		if (FilterData* filter= GetCurrentFilterData())
		{
			filter->time_.from = from;
			filter->time_.to = to;

			ApplyFilter(*filter);
		}

		if (GetCurFilterTab() == 0 && !filter_changed_event_.empty())
			filter_changed_event_(IsMainPageFilterActive());
	}
	CATCH_ALL
}


void ExifView::ClearFilter()
{
	main_page_.ClearAll();

	hist_date_from_ = DateTime();
	hist_date_to_ = DateTime();
	sel_hist_date_from_ = DateTime();
	sel_hist_date_to_ = DateTime();
}


void ExifView::CancelDateFiltering()
{
	try
	{
		ClearFilter();

		distribution_bar_wnd_.ClearHistory();

		if (FilterData* filter= GetCurrentFilterData())
		{
			// date/time is not part of defined filter, so clear it here
			filter->time_.ClearAll();
			ApplyFilter(*filter);
		}

		if (GetCurFilterTab() == 0 && !filter_changed_event_.empty())
			filter_changed_event_(IsMainPageFilterActive());
	}
	CATCH_ALL
}


void ExifView::CancelFiltering()
{
	try
	{
		if (GetCurFilterTab() == 0)
		{
			ClearFilter();

			distribution_bar_wnd_.ClearHistory();

			if (FilterData* filter= GetCurrentFilterData())
				ApplyFilter(*filter);

			if (!filter_changed_event_.empty())
				filter_changed_event_(IsMainPageFilterActive());
		}
	}
	CATCH_ALL
}


bool ExifView::IsDateTimeFilterActive() const
{
	if (const FilterData* current= GetCurrentFilterData())
		if (current->time_.IsActive())
			return true;

	return false;
}


bool ExifView::IsMainPageFilterActive() const
{
	return main_page_.IsActive();
}


bool ExifView::IsFilterActive() const
{
	if (const FilterData* current= GetCurrentFilterData())
		if (current->IsActive())
			return true;

	return false;
}


void ExifView::OnUpdateNextPane(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void ExifView::OnTaskEditDesc()
{
	OnEditIPTC();
}

void ExifView::OnUpdateTaskEditDesc(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


void ExifView::PhotoSelected(PhotoInfo& photo)
{
	GetListCtrl().SetCurrentItem(&photo, true);
	CurrentItemChanged(&photo);
}


void ExifView::OnEscape()
{
	if (IsMainPageFilterActive())
	{
		// cancel filter
		CancelFiltering();
	}
	else
	{
		// no filter; then Esc means stop reloading
		OnStopScanning();
	}
}

void ExifView::OnUpdateEscape(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void ExifView::OnSortBySimilarity()
{
	SortBySimilarity();
}

void ExifView::OnUpdateSortBySimilarity(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();	//TODO: block while loading?
}


void ExifView::CreatePaneImage(CImageList& img_list_dragged_photos, CSize photo_size, PhotoInfoPtr photo)
{
	if (img_list_dragged_photos.GetSafeHandle())
		img_list_dragged_photos.DeleteImageList();

	if (photo_size.cx == 0 && photo_size.cy == 0)
	{
		img_list_dragged_photos.Create(1, 1, ILC_MASK, 1, 0);
		return;
	}

	COLORREF rgb_mask= RGB(212,196,240);
	img_list_dragged_photos.Create(photo_size.cx, photo_size.cy, ILC_MASK | ILC_COLOR24, 1, 0);
	ASSERT(img_list_dragged_photos.GetSafeHandle() != 0);

	// create a bitmap and draw photos in it
	CBitmap bmp;
	CDC dc;
	dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
	bmp.CreateCompatibleBitmap(&dc, photo_size.cx, photo_size.cy);
	{
		CDC bmp_dc;
		bmp_dc.CreateCompatibleDC(&dc);
		bmp_dc.SelectObject(&bmp);
		CRect rect(CPoint(0, 0), photo_size);
		photo->Draw(&bmp_dc, rect, rgb_mask);
/*		bmp_dc.FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));
		for (int i= 0; i < 3; ++i)
		{
			bmp_dc.Draw3dRect(rect, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
			rect.DeflateRect(1, 1);
		}
		rect.bottom = rect.top + 16;
		bmp_dc.FillSolidRect(rect, ::GetSysColor(COLOR_ACTIVECAPTION)); */
	}

	img_list_dragged_photos.Add(&bmp, rgb_mask);
	img_list_dragged_photos.SetBkColor(CLR_NONE);
}


int DragAndDrop(VectPhotoInfo& selected, CWnd* frame);

// drag and drop selected photos
//
void ExifView::DoDragDrop(VectPhotoInfo& selected)
{
	if (DragAndDrop(selected, frame_) == DROPEFFECT_MOVE)
	{
		// files moved

		// if photos were moved remove them from the photo ctrl window
		VectPhotoInfo remove;
		remove.reserve(selected.size());

		for (VectPhotoInfo::const_iterator it= selected.begin(); it != selected.end(); ++it)
		{
			Path path= (*it)->GetOriginalPath();
			if (!path.FileExists())
				remove.push_back(*it);
		}

		RemoveItems(remove);
	}
}


void ExifView::SelectionChanged(VectPhotoInfo& selectedPhotos)
{
	if (notifications_enabled_counter_ >= 0)
	{
		if (selectedPhotos.empty())
			frame_->SelectionChanged(0);
		else if (selectedPhotos.size() == 1)
			frame_->SelectionChanged(selectedPhotos.front());
		else
			frame_->SelectionChanged(selectedPhotos);
	}
}


void ExifView::OnViewThumbSmaller()
{
	int index= GetCurrSliderIndex();
	if (index > 0)
	{
		--index;
		tool_bar_wnd_.SetThumbPos(index);
		ChangeImageSize(index);
	}
}

void ExifView::OnUpdateViewThumbSmaller(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(GetCurrSliderIndex() > 0);
}


void ExifView::OnViewThumbBigger()
{
	int index= GetCurrSliderIndex();
	if (index < GetCurrSliderMax() - 1)
	{
		++index;
		tool_bar_wnd_.SetThumbPos(index);
		ChangeImageSize(index);
	}
}

void ExifView::OnUpdateViewThumbBigger(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(GetCurrSliderIndex() < GetCurrSliderMax() - 1);
}


void ExifView::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (!tool_bar_wnd_.IsSliderValid())
		return;

	switch (sb_code)
	{
	case TB_LINEUP:
	case TB_LINEDOWN:
	case TB_PAGEUP:
	case TB_PAGEDOWN:
	case TB_BOTTOM:
	case TB_TOP:
		{
			int pos= tool_bar_wnd_.GetThumbPos();
			if (pos >= 0 && pos < GetCurrSliderMax())
				ChangeImageSize(pos);
			else
			{ ASSERT(false); }
		}
		break;

	case TB_THUMBPOSITION:
	case TB_THUMBTRACK:
		if (pos >= 0 && pos < array_count(thumb_size_levels))
			ChangeImageSize(pos);
		else
		{ ASSERT(false); }
		break;
	}
}


int ExifView::GetCurrSliderIndex() const
{
	return view_mode_ == VIEW_MODE_PICTURES ? images_across_index_ : img_size_index_;
}


int ExifView::GetCurrSliderMax() const
{
	return view_mode_ == VIEW_MODE_PICTURES ? array_count(images_across_levels) : array_count(thumb_size_levels);
}

//extern float unsharp_amount= 1.0f;

void ExifView::ChangeImageSize(int index)
{
//unsharp_amount = index / 30.0f;
//global_dib_cache->RemoveAll();
//GetListCtrl().Invalidate();
//return;

	if (view_mode_ == VIEW_MODE_PICTURES)
	{
		if (images_across_index_ != index)
		{
			images_across_index_ = index;
			profile_images_across_ = index;
			GetListCtrl().SetItemsAcross(images_across_levels[images_across_index_]);

			thumbnail_loader_.RemoveAll();
		}
	}
	else
	{
		if (img_size_index_ != index)
		{
			img_size_index_ = index;
			profile_thumb_size_ = index;
			GetListCtrl().SetImageSize(Pixels(thumb_size_levels[img_size_index_]));
			SetViewMode(VIEW_MODE_THUMBNAILS);

			if (global_dib_cache)
				global_dib_cache->RemoveAll();

			thumbnail_loader_.RemoveAll();
		}
	}
}


void ExifView::GetItemText(PhotoInfoPtr photo, int column, String& buffer)
{
	if (photo == 0)
	{
		ASSERT(false);
		return;
	}
	if (column >= columns_.size())
	{
		ASSERT(false);
		return;
	}

	int col_index= columns_[column];
	cols_.GetInfo(buffer, col_index, *photo);
}


///////////////////////////////////////////////////////////////////////////////


void ExifView::OnGroupByFolders()
{
	// toggle grouping by folders
	SetGroupMode(grouping_mode_ != GROUP_BY_FOLDERS ? GROUP_BY_FOLDERS : NO_GROUPING);
	tool_bar_wnd_.ShowCancelSortBtn(false);
}

void ExifView::OnUpdateGroupByFolders(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(grouping_mode_ == GROUP_BY_FOLDERS ? 1 : 0);
}


void ExifView::OnGroupByDate()
{
	// toggle grouping by date
	SetGroupMode(grouping_mode_ != GROUP_BY_DATE ? GROUP_BY_DATE : NO_GROUPING);
	tool_bar_wnd_.ShowCancelSortBtn(false);
}

void ExifView::OnUpdateGroupByDate(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(grouping_mode_ == GROUP_BY_DATE ? 1 : 0);
}


void ExifView::OnGroupByTags()
{
	if (grouping_mode_ != GROUP_BY_SIMILARITY)
		tagged_photos_in_groups_ = !tagged_photos_in_groups_;
	else
	{
		// in similarity mode tagging button is raised so pressing it should always turn on 'group by tags'
		tagged_photos_in_groups_ = true;
	}
	ResortItems(true);
	tool_bar_wnd_.ShowCancelSortBtn(false);
}

void ExifView::OnUpdateGroupByTags(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	// in similarity mode grouping by tagging is not supported
	cmd_ui->SetCheck(tagged_photos_in_groups_ && grouping_mode_ != GROUP_BY_SIMILARITY ? 1 : 0);
}


void ExifView::OnGroupByStars()
{
	if (grouping_mode_ != GROUP_BY_SIMILARITY)
		rated_photos_in_groups_ = !rated_photos_in_groups_;
	else
	{
		// in similarity mode stars button is raised so pressing it should always turn on 'group by stars'
		rated_photos_in_groups_ = true;
	}
	ResortItems(true);
	tool_bar_wnd_.ShowCancelSortBtn(false);
}

void ExifView::OnUpdateGroupByStars(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	// in similarity mode grouping by stars is not supported
	cmd_ui->SetCheck(rated_photos_in_groups_ && grouping_mode_ != GROUP_BY_SIMILARITY ? 1 : 0);
}


void ExifView::OnNoGrouping()
{
	SetGroupMode(NO_GROUPING);
}

void ExifView::OnUpdateNoGrouping(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(grouping_mode_ == NO_GROUPING ? 1 : 0);
}


void ExifView::SetGroupMode(GroupMode mode)
{
	if (mode == grouping_mode_)
		return;	// no change

	grouping_mode_ = mode;

	ResortItems(true);
}


void ExifView::CurrentItemChanged(PhotoInfoPtr photo)
{
	if (notifications_enabled_counter_ >= 0)
	{
		// if time line is visible, adjust time tick
		distribution_bar_wnd_.SetTick(photo != 0 ? photo->GetDateTime() : DateTime());

		// send notification to peer pane windows
		SendNotification(&PaneWnd::CurrentPhotoChanged, photo);

		// let frame know too
		frame_->CurrentChanged(photo, GetListCtrl().GetSelectedCount() > 0);
	}
}


// header right-click (detailed mode): show available columns
//
void ExifView::ColumnRClick(int col_index)
{
	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	cols_.GetPopupMenu(menu, columns_);

	const int ID_RESET_COLUMNS= 99998;
	const int ID_RESET_COLUMN_ORDER= 99997;
	const int DEFINE_CUSTOM_COLUMNS= 99996;

	menu.AppendMenu(MF_SEPARATOR);
	if (CurrentItem())
		menu.AppendMenu(MF_STRING, DEFINE_CUSTOM_COLUMNS, _T("自定义列\tShift+Ctrl+F"));
	menu.AppendMenu(MF_STRING, ID_RESET_COLUMNS, _T("重置列"));
	menu.AppendMenu(MF_STRING, ID_RESET_COLUMN_ORDER, _T("重置列顺序"));

	CPoint cursor;
	GetCursorPos(&cursor);
	int column= menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, cursor.x, cursor.y, this);

	if (column > 0)
	{
		std::vector<uint16> sel= columns_;

		if (column == ID_RESET_COLUMNS)
		{
			const int MAX= 16;
			sel.resize(MAX);
			for (int i= 0; i < MAX; ++i)
				sel[i] = i;
		}
		else if (column == ID_RESET_COLUMN_ORDER)
		{
			std::vector<int> col_order(columns_.size());
			for (int i= 0; i < col_order.size(); ++i)
				col_order[i] = i;

			GetListCtrl().SetColumnOrderArray(col_order);

			return;
		}
		else if (column == DEFINE_CUSTOM_COLUMNS)
		{
			frame_->SendMessage(WM_COMMAND, ID_DEFINE_CUSTOM_COLUMNS);
			return;
		}
		else
		{
			--column;	// 0..n-1 range now

			std::vector<uint16>::iterator it= remove(sel.begin(), sel.end(), column);
			if (it != sel.end())
			{
				sel.erase(it);

				if (column == abs(sort_by_) - 1)	// sorting column removed?
					sort_by_ = 0;
			}
			else
				//TODO: revise
				sel.insert(lower_bound(sel.begin(), sel.end(), column), column);
				//sel.insert(upper_bound(sel.begin(), sel.end(), column), column);
		}

		SelectColumns(sel);
	}
}


void ExifView::OnBuildCatalogHere()
{
	String path= Path(build_catalog_path_).GetDir();

	if (!path.empty())
		BuildCatalog(path.c_str());

	build_catalog_path_.clear();
}


void ExifView::ContextMenu(CPoint pos, const String& path)
{
	RMenu menu;
	if (!menu.LoadMenu(IDR_EXIF_CONTEXT))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != 0);

	CRect rect(0,0,0,0);
	GetListCtrl().GetWindowRect(rect);

	if (pos == CPoint(-1, -1))	// kbd invoked?
	{
		pos = rect.CenterPoint();
	}
	else
	{
		// restrict this contextual menu to the list ctrl area; ignore outside clicks
		if (!rect.PtInRect(pos))
			return;
	}

	if (!path.empty())
	{
		popup->AppendMenu(MF_SEPARATOR);
		popup->AppendMenu(MF_STRING, ID_BUILD_CATALOG_2, _T("构建图像分类(&B)..."));
		build_catalog_path_ = path;
	}

	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, GetParent());
}

void ExifView::ContextMenu(CPoint pos, bool mouse_click, int group_id, int group_elem)
{
	String path;
	if (group_elem == 1)	// group label clicked?
		if (PhotoInfoPtr photo= GetListCtrl().GetFirstItem(group_id))
			path = photo->GetPhysicalPath();

	GetListCtrl().ClientToScreen(&pos);
	ContextMenu(pos, path);
}

void ExifView::OnContextMenu(CWnd* wnd, CPoint pos)
{
	ContextMenu(pos, String());
}


/*
void ExifView::OnClickAndSelect()
{
	GetListCtrl().ClickAndSelect(true);
}

void ExifView::OnUpdateClickAndSelect(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(GetListCtrl().ClickAndSelectFlag() ? 1 : 0);
}


void ExifView::OnClickNoSelect()
{
	GetListCtrl().ClickAndSelect(false);
}

void ExifView::OnUpdateClickNoSelect(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(GetListCtrl().ClickAndSelectFlag() ? 0 : 1);
}

void ExifView::OnClickToggleSelecting()
{
	GetListCtrl().ClickAndSelect(!GetListCtrl().ClickAndSelectFlag());
}

void ExifView::OnUpdateClickToggleSelecting(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}
*/

void ExifView::OnStickySelection()
{
	GetListCtrl().StickySelection(!GetListCtrl().StickySelectionFlag());
}

void ExifView::OnUpdateStickySelection(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(GetListCtrl().StickySelectionFlag() ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// export EXIF data from selected photos to text file
//
void ExifView::ExportExifData()
{
	VectPhotoInfo selected;
	GetSelectedPhotos(selected);

	VectPhotoInfo* photos= &selected;

	// if there's no selected photos use all of them
	if (photos->empty())
		photos = &sorted_photos_;

	if (photos->empty())
		return;

	std::vector<int> col_order;
	GetListCtrl().GetColumnOrderArray(col_order);

	CTaskExportEXIF exprt(*photos, columns_, col_order, photos->size() == sorted_photos_.size(), cols_);

	exprt.Go(this);
}


void ExifView::OnTaskExport()
{
	try
	{
		ExportExifData();
	}
	CATCH_ALL
}

void ExifView::OnUpdateTaskExport(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// rotate photos (lossless JPEG rotation)
//
void ExifView::RotatePhotos()
{
	try
	{
		if (!PhotosPresent())
			return;

		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		if (selected.empty())
			selected.push_back(CurrentItem());

		VectPhotoInfo rotate_images;

		const size_t count= selected.size();

		rotate_images.reserve(count);

		for (size_t i= 0; i < count; ++i)
			if (selected[i]->IsRotationFeasible())
				rotate_images.push_back(selected[i]);

		if (rotate_images.empty())
		{
			// none of selected images can be rotated
			new BalloonMsg(&GetListCtrl(), _T("选定的图像不能旋转"),
				_T("选定的图像不支持旋转."), BalloonMsg::IERROR);
			return;
		}

		bool all_loaded_photos_selected= rotate_images.size() == sorted_photos_.size();

		CTaskRotatePhotos rotate(rotate_images, all_loaded_photos_selected, this);

		if (rotate.Go())
		{
			//TODO: optimize cache invalidation
			if (rotate_images.size() == 1)
			{
				if (global_dib_cache)
					RemoveAllPhotoDibs(rotate_images.front(), *global_dib_cache);
				if (global_photo_cache)
					global_photo_cache->Remove(rotate_images.front());
			}
			else
			{
				if (global_dib_cache)
					global_dib_cache->RemoveAll();
				if (global_photo_cache)
					global_photo_cache->RemoveAll();
			}

			// invalidate to redraw thumbnails (they might have been rotated accordingly)
			GetListCtrl().ResetItemsLocation();

			//TODO: rework notification system to use signals; viewer should repaint itself
			// repaint viewer wnd as well
			if (viewer_link_.IsValid())
				viewer_link_->Synch();		//TODO: this may not be enough (for current photo)

			//TODO: optimize
			// refresh preview if necessary
			if (PhotoInfoPtr photo= CurrentItem())
				SendNotification(&PaneWnd::CurrentPhotoModified, photo);
		}
	}
	CATCH_ALL
}


void ExifView::OnTaskRotate()
{
	RotatePhotos();
}

void ExifView::OnUpdateTaskRotate(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnEditIPTC()
{
	try
	{
		if (PhotoInfoPtr photo= CurrentItem())
		{
			VectPhotoInfo selected;
			GetSelectedPhotos(selected);

			VectPhotoInfo photos= selected;

			if (photos.empty())
				photos.push_back(photo);

			EditFileInfo file_info(sorted_photos_, photos, this, boost::bind(&ExifView::PhotoSelected, this, _1));

			if (file_info.DoModal())
			{
				// refresh (tags could have changed...)

//				TagApplied(selected);

//				const size_t count= selected.size();
//				for (size_t i= 0; i < count; ++i)
//					TagApplied(selected[i]);
			}

			//???
			//SendNotification(&PaneWnd::PhotoDescriptionChanged, photo->photo_desc_);

			SelectionChanged(selected);
		}
	}
	CATCH_ALL
}

void ExifView::OnUpdateEditIPTC(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnTaskPrint()
{
	try
	{
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		if (selected.empty())
			if (PhotoInfoPtr photo= CurrentItem())
				selected.push_back(photo);

		if (selected.empty())
		{
//			new BalloonMsg(&GetListCtrl(), _T("There Are No Selected Photographs"),
//				_T("Please select photographs to print first."), BalloonMsg::IERROR);
			return;
		}

		CTaskPrint print(selected, cur_path_.c_str());
		print.Go();
	}
	CATCH_ALL
}

void ExifView::OnUpdateTaskPrint(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


void ExifView::OnTaskPrintThumbnails()
{
	try
	{
		CTaskPrintThumbnails print(sorted_photos_, cur_path_.c_str());
		print.Go();
	}
	CATCH_ALL
}

void ExifView::OnUpdateTaskPrintThumbnails(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnTaskCopyTagged()
{
	try
	{
		VectPhotoInfo photos;
		all_photos_.Copy(photos);

		if (photos.empty())
			return;

		VectPhotoInfo::iterator tagged= photos.begin();
		VectPhotoInfo::iterator remaining= photos.begin();
		VectPhotoInfo::iterator end= photos.end();

		VectPhotoRanges tagged_photos;

		// partition photos: those with tags first
		remaining = partition(tagged, end, PhotoHasTags());

		for (VectPhotoInfo::iterator it= tagged; it != remaining; )
		{
			// find all photos with same tag;
			// they are sorted by tags, so it's a range of adjacent values

			VectPhotoInfo::iterator last= find_if(it, remaining, not1(bind2nd(EqualPhotoByTags(), *it)));
			tagged_photos.push_back(make_pair(it, last));
			it = last;
		}

		if (tagged_photos.empty())
		{
			new BalloonMsg(&GetListCtrl(), _T("没有标签的照片"),
				_T("请先给图片添加标签."), BalloonMsg::IERROR);
			return;
		}

		CTaskCopyTagged copy(tagged_photos, this);
		copy.Go();
	}
	CATCH_ALL
}


void ExifView::OnUpdateTaskCopyTagged(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnShowOptionsPopupMenu(CPoint pos)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_SHOW_OPTIONS))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
//		popup->SetDefaultItem(ID_VIEW_ALL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON, pos.x, pos.y, GetParent());
	}
	else
	{
		ASSERT(popup != NULL);
	}
}


void ExifView::OnViewPhotosPopupMenu(CPoint pos)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_VIEWER_POPUP))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		popup->SetDefaultItem(ID_VIEW_ALL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON, pos.x, pos.y, GetParent());
	}
	else
	{
		ASSERT(popup != NULL);
	}
}


void ExifView::OnViewAll()
{
	ViewPhoto(0, ALL_PHOTOS);
}

void ExifView::OnUpdateViewAll(CCmdUI* cmd_ui)
{
	OnUpdateView(cmd_ui);
}


void ExifView::OnViewSelected()
{
	ViewPhoto(0, SELECTED_PHOTOS);
}

void ExifView::OnUpdateViewSelected(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(Selection());
}


void ExifView::OnViewTagged()
{
	ViewPhoto(0, TAGGED_PHOTOS);
}

void ExifView::OnUpdateViewTagged(CCmdUI* cmd_ui)
{
	// potentially expensive to be invoked in UI update
	bool tagged= find_if(sorted_photos_.begin(), sorted_photos_.end(), PhotoHasTags()) != sorted_photos_.end();
	cmd_ui->Enable(tagged);
}


extern void OpenPhotographInExplorer(const TCHAR* file_path, CWnd* wnd)
{
	CString cmd= _T("/select,");
	cmd += file_path;
	::ShellExecute(wnd->GetSafeHwnd(), _T("open"), _T("Explorer"), cmd, 0, SW_SHOWDEFAULT);
}


extern void OpenPhotograph(const TCHAR* photo_path, CWnd* wnd, bool raw_photo)
{
	const CString& app= raw_photo ? g_Settings.open_raw_photo_app_ : g_Settings.open_photo_app_;

	if (!app.IsEmpty())
	{
		Path path= photo_path;
		if (!path.FileExists())
			return;
		Path dir2= path.GetDir();
		Path file= _T("\"") + path + _T("\"");
		Path dir= _T("\"") + dir2 + _T("\"");
		::ShellExecute(*wnd, _T("open"), app, file.c_str(), dir.c_str(), SW_SHOW);
	}
	else
		wnd->MessageBox(_T("请在选项对话框里选择用于打开图像的应用程序."), 0, MB_OK);
}


void ExifView::OnUpdateOpenInExplorer(CCmdUI* cmd_ui)
{
	// to avoid confusion: enable "open in explorer" cmd when there's only single item selected,
	// or when there's no selection (current item is used), but disable for multiple selection
	cmd_ui->Enable(CurrentItem() != 0 && GetListCtrl().GetSelectedCount() <= 1);
}

void ExifView::OnOpenInExplorer()
{
	if (PhotoInfoPtr photo= CurrentItem())
		::OpenPhotographInExplorer(photo->GetOriginalPath().c_str(), this);
}


void ExifView::OnOpenPhoto()
{
	if (PhotoInfoPtr photo= CurrentItem())
		::OpenPhotograph(photo->GetOriginalPath().c_str(), this, photo->IsRaw());
}

void ExifView::OnUpdateOpenPhoto(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(CurrentItem() != 0 && !g_Settings.open_photo_app_.IsEmpty());
}


void ExifView::OnUpdateTaskCopy(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(Selection());
}

void ExifView::OnUpdateTaskMove(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(Selection());
}

void ExifView::OnUpdateTaskResize(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(Selection());
}

void ExifView::OnUpdateTaskHistogram(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}


void ExifView::StartTagging()
{
	OnViewAll();
	if (viewer_link_.IsValid())
		viewer_link_->TurnTagsBarOn();
}


bool ExifView::ShowPhotoMarker(int file_type)
{
	return g_Settings.MarkFileType(file_type);
}


bool ExifView::ShowNoExifIndicator(int file_type)
{
	return g_Settings.NoExifIndicator(file_type);
}


// color vs shape options
void ExifView::OnSortBySimilarityOptions(CPoint pos)
{
	if (!CSortingOptionsPopup::IsPopupActive())
		new CSortingOptionsPopup(pos, this, this, color_vs_shape_weight_);
}

void ExifView::SetColorVsShapeWeight(float weight)
{
	color_vs_shape_weight_ = weight;
	SortBySimilarity();
}


// remove common part of two paths
String ExifView::FolderName(const Path& base_path, const Path& folder)
{
	String dir= base_path;//.GetDir();
	if (dir.empty() || /*dir.length() >= base_path.length() ||*/ dir.length() > folder.length())
		return folder;

	std::pair<String::const_iterator, String::const_iterator> its= mismatch(dir.begin(), dir.end(), folder.begin());

	if (its.second > folder.begin() && its.second < folder.end())
		return *its.second == _T('\\') ? String(its.second + 1, folder.end()) : String(its.second, folder.end());

	return folder;
}


// select photographs in list ctrl
void ExifView::SelectPhotos(const VectPhotoInfo& photos)
{
	GetListCtrl().SelectItems(photos);
}


void ExifView::FieldSelectionChanged()
{
	frame_->RefreshStatusBar();
}

void ExifView::PopupMenu(bool displayed)
{}


void ExifView::PostNcDestroy()
{} // do not delete


BOOL ExifView::IsFrameWnd() const
{
	return true;	// true so ExifView can handle UI messages sent by ToolBarWnd
}


void ExifView::OpenCurrentPhoto()
{
	ViewPhoto(0, ALL_PHOTOS);
}


void ExifView::OnViewShowNoTextLabels()
{
	if (show_labels_ != NO_LABELS)
	{
		show_labels_ = NO_LABELS;
		GetListCtrl().ShowItemLabel(false);
	}
}

void ExifView::OnUpdateViewShowNoTextLabels(CCmdUI* cmd_ui)
{
	bool enabled= view_mode_ == VIEW_MODE_THUMBNAILS || view_mode_ == VIEW_MODE_PICTURES;

	cmd_ui->Enable(enabled);
	cmd_ui->SetRadio(enabled && show_labels_ == NO_LABELS ? 1 : 0);
}


void ExifView::OnViewShowDateTimeLabels()
{
	if (show_labels_ != SHOW_DATE_TIME)
	{
		show_labels_ = SHOW_DATE_TIME;
		GetListCtrl().ShowItemLabel(true);
		GetListCtrl().Invalidate();
	}
}

void ExifView::OnUpdateViewShowDateTimeLabels(CCmdUI* cmd_ui)
{
	bool enabled= view_mode_ == VIEW_MODE_THUMBNAILS || view_mode_ == VIEW_MODE_PICTURES;

	cmd_ui->Enable(enabled);
	cmd_ui->SetRadio(enabled && show_labels_ == SHOW_DATE_TIME ? 1 : 0);
}


void ExifView::OnViewShowLabels()
{
	if (show_labels_ != SHOW_FILE_NAME)
	{
		show_labels_ = SHOW_FILE_NAME;
		GetListCtrl().ShowItemLabel(true);
		GetListCtrl().Invalidate();
	}
}

void ExifView::OnUpdateViewShowLabels(CCmdUI* cmd_ui)
{
	bool enabled= view_mode_ == VIEW_MODE_THUMBNAILS || view_mode_ == VIEW_MODE_PICTURES;

	cmd_ui->Enable(enabled);
	cmd_ui->SetRadio(enabled && show_labels_ == SHOW_FILE_NAME ? 1 : 0);
}


void ExifView::OnViewShowFileNameExt()
{
	if (show_labels_ != SHOW_FILE_NAME_EXT)
	{
		show_labels_ = SHOW_FILE_NAME_EXT;
		GetListCtrl().ShowItemLabel(true);
		GetListCtrl().Invalidate();
	}
}

void ExifView::OnUpdateViewShowFileNameExt(CCmdUI* cmd_ui)
{
	bool enabled= view_mode_ == VIEW_MODE_THUMBNAILS || view_mode_ == VIEW_MODE_PICTURES;

	cmd_ui->Enable(enabled);
	cmd_ui->SetRadio(enabled && show_labels_ == SHOW_FILE_NAME_EXT ? 1 : 0);
}


void ExifView::OnViewCustLabels()
{
	if (show_labels_ != CUSTOM_INFO)
	{
		show_labels_ = CUSTOM_INFO;
		GetListCtrl().ShowItemLabel(true);
		GetListCtrl().Invalidate();
	}
}

void ExifView::OnUpdateViewCustLabels(CCmdUI* cmd_ui)
{
	bool enabled= view_mode_ == VIEW_MODE_THUMBNAILS || view_mode_ == VIEW_MODE_PICTURES;

	cmd_ui->Enable(enabled);
	cmd_ui->SetRadio(enabled && show_labels_ == CUSTOM_INFO ? 1 : 0);
}


void ExifView::OnEditCustomLabels()
{
	ImageLabelDlg dlg(customInfoFields_[0], customInfoFields_[1], this, cols_);
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetSelection(0, customInfoFields_[0]);
		dlg.GetSelection(1, customInfoFields_[1]);
		// show them now
		show_labels_ = CUSTOM_INFO;
		GetListCtrl().ShowItemLabel(true);
	}
}

void ExifView::OnUpdateEditCustomLabels(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


String ExifView::GetToolTipText(PhotoInfoPtr photo)
{
	String str;
	if (photo)
		str = photo->ToolTipInfo(g_Settings.balloon_fields_);

	return str;
}


int ExifView::GetLabelLines(PhotoCtrl::Mode mode) const
{
	if (show_labels_ == CUSTOM_INFO)
	{
		// how many lines for an image label: as many as fields selected
		const std::vector<uint16>& vect= customInfoFields_[mode == PhotoCtrl::THUMBNAILS ? 0 : 1];
		return static_cast<int>(vect.size());
	}
	else
		return 1;
}


String ExifView::GetItemLabel(PhotoInfoPtr photo, CDC& dc, int label_space)
{
	if (!photo)
		return String();

	if (show_labels_ == CUSTOM_INFO)// && photo->IsExifDataPresent())
	{
		const std::vector<uint16>& vect= customInfoFields_[view_mode_ == VIEW_MODE_THUMBNAILS ? 0 : 1];
		oStringstream ost;
		const size_t fields= vect.size();
		for (size_t i= 0; i < fields; ++i)
		{
			if (i > 0)
				ost << std::endl;

			int index= vect[i];
			const TCHAR* name= cols_.Abbreviation(index);
			if (name && *name)
				ost << name << _T(": ");
//			ost << DrawFields::LABEL << cols_.ShortName(index) << _T(": ");
			String value;
			cols_.GetInfo(value, index, *photo);
			ost << value;
		}
		return ost.str();
	}
	else if (show_labels_ == SHOW_DATE_TIME && !photo->GetDateTime().is_not_a_date_time())//.GetTime() != ~0)
	{
		return ::GetFormattedDateTime(photo->GetDateTime(), dc, label_space);
	}
	else if (show_labels_ == SHOW_FILE_NAME_EXT)
	{
		return photo->GetNameAndExt();
	}
	else
	{
		return photo->GetName();
	}
}


void ExifView::OnShowBalloons()
{
	GetListCtrl().EnableBalloonInfo(!GetListCtrl().IsBalloonInfoEnabled());
}

void ExifView::OnUpdateShowBalloons(CCmdUI* cmd_ui)
{
	bool enabled= true;// details mode should be excluded view_mode_ == VIEW_MODE_THUMBNAILS;
	cmd_ui->Enable(enabled);
	cmd_ui->SetCheck(enabled && GetListCtrl().IsBalloonInfoEnabled() ? 1 : 0);
}


void ExifView::OnShowTags()
{
	GetListCtrl().ShowTagText(!GetListCtrl().ShowingTagText());
}

void ExifView::OnUpdateShowTags(CCmdUI* cmd_ui)
{
	bool enabled= true; //view_mode_ != VIEW_MODE_DETAILS;
	cmd_ui->Enable(enabled);
	cmd_ui->SetCheck(enabled && GetListCtrl().ShowingTagText() ? 1 : 0);
}

void ExifView::OnShowMarker()
{
	GetListCtrl().ShowMarker(!GetListCtrl().ShowingMarker());
}

void ExifView::OnUpdateShowMarker(CCmdUI* cmd_ui)
{
	bool enabled= true; //view_mode_ != VIEW_MODE_DETAILS;
	cmd_ui->Enable(enabled);
	cmd_ui->SetCheck(enabled && GetListCtrl().ShowingMarker() ? 1 : 0);
}


void ExifView::OnUpdateShowOptions(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


// notifications from ViewerDlg --------------------------------

void ExifView::CurrentChanged(PhotoInfoPtr photo)
{
	// user has selected new photo in an image viewer window: change current item
	// but refrain from refreshing preview; modify selection as well (under certain conditions),
	// so tools invoked from viewer operate on a right image 

	if (photo)
	{
		bool select= true;

		// if sticky selection mode is on avoid modifying selection; it's to be preserved
		if (GetListCtrl().StickySelectionFlag())
			select = false;

		// if viewer window was invoked with selected images only, this selection
		// is not to be messed up!
		if (current_viewer_type_ == SELECTED_PHOTOS)
			select = false;

		// avoid scrolling in a preview mode--it requires decoding big images
		bool scroll_to_item= view_mode_ != VIEW_MODE_PICTURES;
		GetListCtrl().SetCurrentItem(photo, select, scroll_to_item);

		// no notification being sent to the pane windows

		frame_->CurrentChanged(photo, GetListCtrl().GetSelectedCount() > 0);
	}
}


void ExifView::PhotoDeleted(PhotoInfoPtr photo)
{
	RemoveItem(photo, true, true);

	VectPhotoInfo::iterator in_viewer= find(viewer_photos_.begin(), viewer_photos_.end(), photo);
	if (in_viewer != viewer_photos_.end())
		viewer_photos_.erase(in_viewer);

	all_photos_.Remove(photo);
}


void ExifView::PhotoOrientationChanged(PhotoInfoPtr photo)	// notification form viewer window
{
	// potentially rotated thumbnail: redraw everything

	//TODO: optimize
	GetListCtrl().ResetItemsLocation();

	// refresh preview if necessary
	if (photo == CurrentItem())
	{
		//current modified
		SendNotification(&PaneWnd::CurrentPhotoModified, photo);
	}
}


void ExifView::SelectionRequest(const VectPhotoInfo& selected)
{
	SelectPhotos(selected);
}


void ExifView::SelectAndDelete(const VectPhotoInfo& photos)
{
	SelectPhotos(photos);
	DeletePhotos(false);
}


void ExifView::FileOperationRequest(const VectPhotoInfo& photos, ViewerDlgNotifications::FileOper oper, CWnd* parent)
{
	switch (oper)
	{
	case ViewerDlgNotifications::Copy:
		FileOperation(photos, true, parent);
		break;

	case ViewerDlgNotifications::Move:
		FileOperation(photos, false, parent);
		break;

	case ViewerDlgNotifications::Rename:
		ASSERT(photos.size() == 1);
		if (photos.size() == 1)
			RenameFile(photos.front(), parent);
		break;

	default:
		ASSERT(false);
		break;
	}
}


void ExifView::AnimationStarts()
{
	SendNotification(&PaneWnd::UpdatePane);
}


void ExifView::UpdateSortOrderPopup(CMenu& popup)
{
	UINT count= popup.GetMenuItemCount();
	while (count-- > 0)
		popup.DeleteMenu(0, MF_BYPOSITION);

	PopulateSortPopupMenu(popup, ID_SORTING_ORDER);
}


void ExifView::ChangeSortOrder(UINT cmd_id)
{
	cmd_id -= ID_SORTING_ORDER;
	SortPhotosByColumn(cmd_id);
}


void ExifView::OnViewShowTimeLine()
{
	if (distribution_bar_wnd_.m_hWnd)
	{
		bool show= !distribution_bar_wnd_.IsWindowVisible();
		if (show)
			distribution_bar_wnd_.ShowWindow(SW_SHOWNA);
		else
			distribution_bar_wnd_.ShowWindow(SW_HIDE);

		SetFaintCaptionEdge(show);
		tool_bar_wnd_.Invalidate();

		Resize();
	}
}

void ExifView::OnUpdateViewShowTimeLine(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(distribution_bar_wnd_.m_hWnd && (distribution_bar_wnd_.GetStyle() & WS_VISIBLE) != 0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnDateTimeAdjust()
{
	try
	{
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		PhotoInfoPtr cur_photo= CurrentItem();

		if (selected.empty() && cur_photo)
			selected.push_back(cur_photo);

		if (selected.empty())
			return;

		CTaskDateTimeAdjustment adj(this, selected);
		if (!adj.Go())
			return;

		// refresh views
		CurrentItemChanged(cur_photo);

		GetListCtrl().Invalidate();

		if (grouping_mode_ != GROUP_BY_SIMILARITY)
			ResortItems(true);

		// refresh image viewer
		if (viewer_link_.IsValid())
			viewer_link_->PhotosListChange();

		RebuildHistogram();
	}
	CATCH_ALL
}

void ExifView::OnUpdateDateTimeAdjust(CCmdUI* cmd_ui)	{ cmd_ui->Enable(PhotosPresent()); }

///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnExtractJpegFromRaws()
{
	try
	{
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		PhotoInfoPtr cur_photo= CurrentItem();

		if (selected.empty() && cur_photo)
			selected.push_back(cur_photo);

		if (selected.empty())
			return;

		CTaskExtractJpegs extract(selected, this);
		if (!extract.Go())
			return;

		//// refresh views
		//CurrentItemChanged(cur_photo);

		//GetListCtrl().Invalidate();

		//if (grouping_mode_ != GROUP_BY_SIMILARITY)
		//	ResortItems();

		//// refresh image viewer
		//if (viewer_link_.IsValid())
		//	viewer_link_->PhotosListChange();
	}
	CATCH_ALL
}


void ExifView::OnUpdateExtractJpegFromRaws(CCmdUI* cmd_ui)
{
	VectPhotoInfo selected;
	GetSelectedPhotos(selected);

	for (VectPhotoInfo::iterator it= selected.begin(); it != selected.end(); ++it)
	{
		PhotoInfo& photo= **it;

		uint32 jpeg_data_offset_= 0;
		uint32 jpeg_data_size_= 0;

		if (photo.GetEmbeddedJpegImage(jpeg_data_offset_, jpeg_data_size_))
		{
			cmd_ui->Enable(true);
			return;
		}
	}

	cmd_ui->Enable(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnProcessImages()
{
	try
	{
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		PhotoInfoPtr cur_photo= CurrentItem();

		if (selected.empty() && cur_photo)
			selected.push_back(cur_photo);

		if (selected.empty())
			return;

		CTaskImageProcessor processor(selected, this);
		if (!processor.Go())
			return;
	}
	CATCH_ALL
}

void ExifView::OnUpdateProcessImages(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(PhotosPresent());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ExifView::OnTaskImgManip()
{
	try
	{
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		if (selected.empty())
			if (PhotoInfoPtr photo= CurrentItem())
				selected.push_back(photo);

		if (selected.empty())
			return;

		CTaskImgManipulation manip(this, selected);
		manip.Go();
	}
	CATCH_ALL
}

void ExifView::OnUpdateTaskImgManip(CCmdUI* cmd_ui)	{ cmd_ui->Enable(PhotosPresent()); }


void ExifView::OnCancelSortBySimilarity()
{
	if (grouping_mode_ == GROUP_BY_SIMILARITY)
	{
		SetGroupMode(old_grouping_);
		tool_bar_wnd_.ShowCancelSortBtn(false);
	}
}

void ExifView::OnUpdateCancelSortBySimilarity(CCmdUI* cmd_ui)	{ cmd_ui->Enable(); }


void ExifView::CaptionHeightChanged(bool big)
{
	tool_bar_wnd_.SetBitmapSize(big);
	ResetBandsWidth(tool_bar_wnd_.GetMinMaxWidth());
}


Dib* ExifView::RequestThumbnail(PhotoInfoPtr photo, CSize size_image, bool return_if_not_available)
{
//	return photo->GetThumbnail(size_image);

	// if (big enough) thumbnail already exists in the cache return it
	if (Dib* dib= photo->IsThumbnailAvailable(size_image))
		return dib;

	if (global_dib_cache.empty())
		return 0;

	if (photo->thumbnail_stat_ != IS_NO_IMAGE && photo->thumbnail_stat_ != IS_OK)
		return 0;

//	pair<DibImg*, bool> dib= global_dib_cache->GetEntry(DibImgKey(photo, size_image));

	if (!thumbnail_loader_.AddPhoto(photo, size_image))
		thumbnail_loader_.MarkPhotoAsRequested(photo);

	// get rid of invisible items (including speculative load items)
	for (size_t i= 0; i < thumbnail_loader_.ItemCount(); )
		if (!GetListCtrl().IsItemVisible(thumbnail_loader_.GetItem(i)))
			thumbnail_loader_.RemovePhoto(thumbnail_loader_.GetItem(i), false);
		else
			++i;

	// start decoder if decoding not pending already
	thumbnail_loader_.StartDecoding();

	return 0;
}


Dib* ExifView::RequestImage(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available)
{
	if (global_photo_cache.get() == 0)
		return 0;

	// if photo cannot be opened then give up now
	if (!::CanPhotoBeDecoded(photo))
	{
		image_loader_.RemovePhoto(photo, true);// queue_.Erase(photo);
		return 0;
	}

	Dib* dib= 0;

	if (CacheImg* img= global_photo_cache->FindEntry(photo))
	{
		if (img->Dib() && img->Dib()->IsValid())
		{
			if (::IsCachedImageAvailable(img, image_size))
				return img->Dib().get();

			// existing (decoded) img is too small, decode it again (below)

			// return existing bmp; it's better than thumbnail; bigger version will be delivered later
			dib = img->Dib().get();
		}
	}

	if (return_if_not_available)
		return dib;

	// add photo to decoder's queue
//TRACE(L"queue: %s\n", photo->name_.c_str());

	if (!image_loader_.AddPhoto(photo, image_size))
		image_loader_.MarkPhotoAsRequested(photo);

	// get rid of invisible items (except those for speculative load)
	for (size_t i= 0; i < image_loader_.ItemCount(); )
		if (!image_loader_.IsSpeculativeLoadingItem(i) &&
			!GetListCtrl().IsItemVisible(image_loader_.GetItem(i)))
				image_loader_.RemovePhoto(image_loader_.GetItem(i), false);
		else
			++i;

	// start decoder if decoding not pending already
	image_loader_.StartDecoding();

	return dib;
}


AutoPtr<Dib> ExifView::CorrectImageColors(PhotoInfoPtr photo, Dib* bmp)
{
	AutoPtr<Dib> trans_bmp;

	if (photo && bmp && bmp->IsValid() && icc_ && transform_.IsValid())
	{
		try
		{
			trans_bmp = DibColorTransform(*bmp, photo->HasColorProfile() ? photo->GetColorProfile() : transform_.in_,
				transform_.out_, transform_.rendering_intent_, 0);
		}
		catch (ColorException& ex)
		{
			ex;
#ifdef _DEBUG
			AfxMessageBox(ex.GetMessage(), MB_OK | MB_ICONERROR);
#endif
		}
	}

	return trans_bmp;
}


void ExifView::ContentScrolled(Shift dir)
{
	window_scrolled_down_ = dir == TOP || dir == DOWN;

	if (image_loader_.IsQueueEmpty())
	{
		// queue is empty; attempt speculative loading
		StartDecodingNextPhoto();
	}

	if (thumbnail_loader_.IsQueueEmpty())
	{
		// queue is empty; attempt speculative loading
		thumbnail_loader_.StartDecoding();
	}
}


void ExifView::PopulateThumbnailQueue(bool scrolledDown)
{
	thumbnail_loader_.RemoveSpeculativeLoadingItems();

	if (global_dib_cache == 0)
		return;

	// last or first depending on the last scroll operation
	PhotoInfoPtr photo= GetListCtrl().GetItem(scrolledDown ? PhotoCtrl::LAST_VISIBLE : PhotoCtrl::FIRST_VISIBLE);
	if (photo == 0)
		return;

	int vert_amount= 10;	//TODO: read from photo ctrl
	int images= vert_amount * 10; //TODO: items horizontally

	const int limit= static_cast<int>(global_dib_cache->Size() / 4);
	if (images > limit)
		images = limit;

	std::vector<std::pair<PhotoInfoPtr, CSize>> photos;
	photos.reserve(images);

	for (int i= 0; i < images; ++i)
	{
		// next or prev
		photo = GetListCtrl().GetItem(scrolledDown ? PhotoCtrl::NEXT_ITEM : PhotoCtrl::PREV_ITEM, photo);
		if (photo == 0)
			break;

		{
			//TODO: determine *exact* size of image in the photo ctrl, or else cache cannot be prepopulated...

			CSize thumb_size= GetListCtrl().GetImageSize(photo);
			ASSERT(thumb_size.cx > 0 && thumb_size.cy > 0);

			if (DibImg* img= global_dib_cache->FindEntry(DibImgKey(photo, thumb_size), false))
				if (img->dib.IsValid())
					continue;

			if (::CanPhotoBeDecoded(photo))
				photos.push_back(std::make_pair(photo, thumb_size));
		}
	}
//TRACE(L"------adding %d photos for preload\n", static_cast<int>(photos.size()));
	if (!photos.empty())
	{
		// add photos to decode (speculative loading)
		for (size_t i= 0; i < photos.size(); ++i)
			thumbnail_loader_.AddPhoto(photos[i].first, photos[i].second, true);
	}
}


void ExifView::PopulateQueue(bool scrolled_down)
{
	CSize image_size(0, 0);
	PhotoInfoPtr photo= 0;

	image_loader_.RemoveSpeculativeLoadingItems();

	if (global_photo_cache == 0)
		return;

	// last or first depending on the last scroll operation
	photo = GetListCtrl().GetItem(scrolled_down ? PhotoCtrl::LAST_VISIBLE : PhotoCtrl::FIRST_VISIBLE);
	if (photo == 0)
		return;

	int vert_amount= 2;	//TODO: read from photo ctrl
	int images= vert_amount * images_across_levels[images_across_index_];

	image_size = GetListCtrl().GetImageSize();
	if (image_size.cx <= 0 || image_size.cy <= 0)
	{
		ASSERT(false);
		return;
	}

	// hardcoded estimated image size; very inacurate
	size_t img_mem= image_size.cx * 3/*RGB*/ * image_size.cy;

	size_t free_mem= global_photo_cache->GetMemoryLimit();
	// continue only if cache is big enough
	if (free_mem / img_mem < 5 * images)
		return;

	if (free_mem / img_mem > 20 * images)	// if plenty of space then read more photos
		images *= 2;

	VectPhotoInfo photos;
	photos.reserve(images);

	for (int i= 0; i < images; ++i)
	{
		// next or prev
		photo = GetListCtrl().GetItem(scrolled_down ? PhotoCtrl::NEXT_ITEM : PhotoCtrl::PREV_ITEM, photo);
		if (photo)
		{
			if (CacheImg* img= global_photo_cache->FindEntry(photo, false))
				if (img->Dib() && img->Dib()->IsValid())	// no size checking here since image_size is approximated only
					continue;

			if (::CanPhotoBeDecoded(photo))
				photos.push_back(photo);
		}
	}

	if (!photos.empty())
	{
		// add photos to decode (speculative loading)
		image_loader_.AddPhotos(photos, image_size, true);
	}
}


void ExifView::StartDecodingNextPhoto()
{
	image_loader_.StartDecoding();
}


void ExifView::ImgReloaded(PhotoInfoPtr photo)
{
	if (GetListCtrl().IsItemVisible(photo))
		GetListCtrl().RedrawItem(photo);
}


bool ExifView::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr >= '0' && chr <= '9') // TODO: no alt nor ctrl - && flags == ?)
	{
		int index= chr - '0' - 1;
		if (index < 0)
			index += 10;
		SendNotification(&PaneWnd::AssignTag, index);
		return true;
	}
	else
	{
		return false;
		//CWnd::OnChar(chr, rep_cnt, flags);
	}
}


void ExifView::OnTaskSendEMail()
{
	VectPhotoInfo photos;
	GetSelectedPhotos(photos);

	if (photos.empty())
		return;

	CTaskSendEMail send(photos, this);

	send.Go();
}


FolderPathPtr ExifView::GetCurrentPath() const
{
	return current_folder_;
}


void ExifView::RenameFile(PhotoInfoPtr photo, CWnd* parent)
{
	extern void RenamePhoto(PhotoInfoPtr photo, const String& fileName, bool replace_name_and_ext, ImageDatabase& db);

	if (photo != 0 && photo->CanRename())
	{
		try
		{
			Dib* img= photo->GetThumbnail();
			String path= photo->GetPhysicalPath();
			ImageDatabase& db= GetImageDataBase(false, false);

			FileRenameDlg dlg(photo->GetName(), path, img, boost::bind(&RenamePhoto, photo, _1, false, boost::ref(db)), parent);

			if (dlg.DoModal() == IDOK)
			{
				GetListCtrl().Invalidate();

				if (grouping_mode_ != GROUP_BY_SIMILARITY)
					ResortItems();

				// refresh
				PhotoSelected(*photo);

				// refresh image viewer
				if (viewer_link_.IsValid())
					viewer_link_->PhotosListChange();
			}
		}
		CATCH_ALL_W(parent)
	}
}


void ExifView::OnRenameFile()
{
	PhotoInfoPtr photo= CurrentItem();

	RenameFile(photo, this);
}


void ExifView::OnUpdateRenameFile(CCmdUI* cmd_ui)
{
	PhotoInfoPtr photo= CurrentItem();
	cmd_ui->Enable(photo != 0 && photo->CanRename() && (reload_thread_.get() == 0 || !reload_thread_->IsScanning()));
}


void ExifView::OnUpdateTags(CCmdUI* cmd_ui)
{
	try
	{
		const PhotoTagsCollection& tags= Tags::GetTagCollection();

		// the goal here is to update all submenu items in one go rather than one by one as those messages arrive for each tag

		if (cmd_ui->m_pMenu != 0 && cmd_ui->m_pSubMenu == 0 && cmd_ui->m_nID == ID_TAG_SELECTED)	// submenu hit?
		{
			// user tries to open popup with tags; update list of tags
			::ResetPopupMenuTags(*cmd_ui->m_pMenu, ID_TAG_SELECTED, tags);

			VectPhotoInfo selected;
			GetSelectedPhotos(selected);

			const UINT items= std::min<UINT>(cmd_ui->m_pMenu->GetMenuItemCount(), static_cast<UINT>(tags.GetCount()));
			for (UINT item= 0; item < items; ++item)
			{
				if (tags.Empty() || selected.empty())
					cmd_ui->m_pMenu->EnableMenuItem(item, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
				else
				{
					cmd_ui->m_pMenu->EnableMenuItem(item, MF_BYPOSITION | MF_ENABLED);

					bool has_tag= find_if(selected.begin(), selected.end(), boost::bind(&PhotoInfo::FindTag, _1, boost::ref(tags.Get(item)))) != selected.end();
					cmd_ui->m_pMenu->CheckMenuItem(item, MF_BYPOSITION | (has_tag ? MF_CHECKED : MF_UNCHECKED));
				}
			}
		}
	}
	catch (...)
	{
		ASSERT(false);
	}
}


void ExifView::OnTagSelected(UINT cmd)
{
	try
	{
		const PhotoTagsCollection& tags= Tags::GetTagCollection();
		int index= cmd - ID_TAG_SELECTED;
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);
		if (!tags.Empty() && index < tags.GetCount() && index >= 0 && !selected.empty())
		{
			bool has_tag= find_if(selected.begin(), selected.end(), boost::bind(&PhotoInfo::FindTag, _1, boost::ref(tags.Get(index)))) != selected.end();
			//frame_->ApplyTagToPhotos(selected, tags.Get(index), !has_tag, this);
			::ApplyTagToPhotos(selected, tags.Get(index), !has_tag, this);
		}
	}
	CATCH_ALL
}


void ExifView::OnRateImage(UINT cmd)
{
	try
	{
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);

		int rating= cmd - ID_RATE_NONE;
		ASSERT(rating >= 0 && rating <= 5);

		// rate images
//		frame_->ApplyRatingToPhotos(selected, rating, this);
		::ApplyRatingToPhotos(selected, rating, this);
	}
	CATCH_ALL
}


void ExifView::OnUpdateRates(CCmdUI* cmd_ui)
{
	try
	{
		// the goal here is to update all submenu items in one go rather than one by one as those messages arrive

		if (cmd_ui->m_pMenu != 0 && cmd_ui->m_pSubMenu == 0 && cmd_ui->m_nID == ID_RATE_NONE)	// submenu hit?
		{
			VectPhotoInfo selected;
			GetSelectedPhotos(selected);

			const UINT items= cmd_ui->m_pMenu->GetMenuItemCount();
			for (UINT item= 0; item < items; ++item)
			{
				if (selected.empty())
					cmd_ui->m_pMenu->EnableMenuItem(item, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
				else
				{
					cmd_ui->m_pMenu->EnableMenuItem(item, MF_BYPOSITION | MF_ENABLED);

					int rating= selected.front()->GetRating();

					const size_t count= selected.size();
					for (size_t i= 1; i < count; ++i)
						if (selected[i]->GetRating() != rating)
						{
							rating = -1;
							break;
						}

					if (rating >= 0)
						cmd_ui->m_pMenu->CheckMenuRadioItem(ID_RATE_NONE, ID_RATE_5_STARS, ID_RATE_NONE + rating, MF_BYCOMMAND);
				}
			}
		}
	}
	catch (...)
	{
		ASSERT(false);
	}
}


void ExifView::StatusPaneClick(const CRect& rect)
{
	// show error messages (if any)
	if (reload_thread_)
	{
		const ImgLogger& logger= reload_thread_->GetImageErrorLogger();

		if (logger.GetCount() > 0)
		{
			CLoadErrorsDlg dlg(this, logger);
			dlg.DoModal();
		}
	}
}


void ExifView::SetCustomColumns(const CustomColumns& col)
{
	cols_.SetCustomColumns(col);

	if (GetViewMode() == VIEW_MODE_DETAILS)
	{
		GetListCtrl().Invalidate();

		// update custom column captions
		int first= cols_.GetStart(Columns::CUSTOM);
		int last= first + cols_.GetCount(Columns::CUSTOM);

		const size_t count= columns_.size();
		for (size_t i= 0; i < count; ++i)
			if (columns_[i] >= first && columns_[i] < last)
			{
				int index= static_cast<int>(i);
				GetListCtrl().UpdateColumnHeading(index, cols_.ShortName(columns_[i]));
			}
	}

	// viewer status can use custom columns
	if (viewer_link_.IsValid())
		viewer_link_->CustomColumnsRefresh();

	// status bar can use custom columns
	frame_->RefreshStatusBar();
}


//const CustomColumns& ExifView::GetCustomColumns() const
//{
//	return cols_.GetCustomColumns();
//}


void ExifView::OnSetAsWallpaper()
{
	bool SetWallpaper(const PhotoInfo& photo);

	if (PhotoInfoPtr photo= CurrentItem())
		SetWallpaper(*photo);
}

void ExifView::OnUpdateSetAsWallpaper(CCmdUI* cmd)
{
	cmd->Enable(CurrentItem() != 0);
}


void ExifView::OnFilterPaneSwitch()
{
	int tab= GetCurFilterTab();
	if (tab == 0)
		SetCurFilterTab(1);
	else
		SetCurFilterTab(0);

	FilterPaneSwitch();
}

void ExifView::OnUpdateFilterPaneSwitch(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	int tab= GetCurFilterTab();
	cmd_ui->SetCheck(tab);
}


void ExifView::FilterPaneSwitch()
{
	int tab= GetCurFilterTab();

	if (tab == 0)
	{
		filter_dlg_.ShowWindow(SW_HIDE);
		filter_dlg_.EnableWindow(false);
	}
	else
	{
		filter_dlg_.EnableWindow(true);
		filter_dlg_.ShowWindow(SW_SHOWNA);
	}

	Resize();

	distribution_bar_wnd_.ClearHistory();

	if (FilterData* filter= GetFilterData(tab))
	{
		bool custom= tab >= FIXED_TABS;
		if (tab > 0)
			filter_dlg_.SetFilter(*filter, custom);

		ApplyFilter(*filter);
	}
}


void ExifView::OnSelChangeTab(NMHDR* nmhdr, LRESULT* result)
{
	FilterPaneSwitch();
	*result = 0;
}


void ExifView::OnTagsApplied(const VectPhotoInfo& photos)
{
	// tags are metadata, so same refresh:
	OnMetadataChanged(photos);
}


void ExifView::OnMetadataChanged(const VectPhotoInfo& photos)
{
	// metadata contains all tags, refresh photos, groups, and filter

	SaveSelection sel(*this);

	MovePhotos(photos);

	if (FilterData* filter= GetCurrentFilterData())
		ApplyFilter(*filter);
}


void ExifView::OnRatingApplied(const VectPhotoInfo& photos)
{
	// rating is also basis of image grouping, so reuse tagging notification handler
	OnMetadataChanged(photos);
}


void ExifView::OnRenameImages()
{
	try
	{
		VectPhotoInfo selected;
		GetSelectedPhotos(selected);
		CTaskImageRename rename(selected, this);
		rename.Go();
		// notify about changes;
		// in case of renaming errors, user can still cancel out above tool, but some images may be changed

		GetListCtrl().Invalidate();

		if (grouping_mode_ != GROUP_BY_SIMILARITY)
			ResortItems();

		// refresh
		if (PhotoInfoPtr photo= CurrentItem())
		{
			SendNotification(&PaneWnd::CurrentPhotoModified, photo);
		}
//			PhotoSelected(*photo);

		// refresh image viewer
		if (viewer_link_.IsValid())
			viewer_link_->PhotosListChange();
	}
	CATCH_ALL_W(this)
}

void ExifView::OnUpdateRenameImages(CCmdUI* cmd_ui)
{
	//TODO: restrict it for catalog files
	cmd_ui->Enable(Selection());
}


void ExifView::CurrentModified(PhotoInfoPtr photo)	// this notification may come from peer PaneWnd (like preview)
{
	GetListCtrl().ResetItemsLocation();

	//TODO: rework notification system to use signals; viewer should repaint itself
	// repaint viewer wnd as well
	if (viewer_link_.IsValid())
		viewer_link_->Synch();		//TODO: this may not be enough (for current photo)

	//TODO: status bar should listen for events too
	frame_->RefreshStatusBar();
}


void ExifView::EnableNotification(bool enable)
{
	if (enable)
		++notifications_enabled_counter_;
	else
		--notifications_enabled_counter_;
}


slot_connection ExifView::ConnectReloadProgress(ReloadProgress::slot_function_type fn)
{
	return reload_progress_event_.connect(fn);
}


slot_connection ExifView::ConnectFilterChanged(FilterChanged::slot_function_type fn)
{
	return filter_changed_event_.connect(fn);
}
