/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// LightTable.cpp: implementation of the LightTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "LightTable.h"
#include "CatchAll.h"
#include "Dib.h"
#include "viewer/FancyToolBar.h"
#include "SetWindowSize.h"
#include "ImageLoader.h"
#include "viewer/PreviewBandWnd.h"
#include "Config.h"
#include "ImageDraw.h"
#include "PhotoCache.h"
#include "TaggingPopup.h"
#include "BrowserFrame.h"
#include <boost/bind.hpp>
#include "BmpFunc.h"
#include "Color.h"
#include "CreateDecoderJob.h"
#include "DragAndDrop.h"
#include "TagsCommonCode.h"
#include "Tasks.h"
#include "PhotoCollection.h"
#include "GetDefaultGuiFont.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern bool LoadPingFromRsrc(LPCTSTR resource_id, Dib& bmp);


struct LightTable::Impl
{
	Impl(PhotoInfoStorage& storage, PhotoCache* cache) : cache_(cache), photo_storage_(storage),
		img_loader_(boost::bind(&PhotoCache::CacheDecodedImage, cache, _1, _2), &CreateImageDecoderJob, &CanPhotoBeDecoded), color_transf_(0)
	{
		vertRes_ = horzRes_ = 0.0f;
		ui_gamma_correction_ = 1.0;

		tags_changed_ =		PhotoCollection::Instance().ConnectOnTagsChanged(boost::bind(&Impl::RefreshAfterChange, this, _1));
		rating_changed_ =	PhotoCollection::Instance().ConnectOnRatingChanged(boost::bind(&Impl::RefreshAfterChange, this, _1));
		metadata_changed_ =	PhotoCollection::Instance().ConnectOnMetadataChanged(boost::bind(&Impl::RefreshAfterChange, this, _1));
	}

	PhotoInfoStorage& photo_storage_;
	//Dib header_;
	Dib separator_;
	FancyToolBar toolbar_;
	PreviewBandWnd ctrl_;
	FancyToolBar close_btn_;
	ImageLoader img_loader_;
	VectPhotoInfo photos_;
	boost::function<void (PhotoInfoPtr)> selected_callback_;
	boost::function<PhotoInfoPtr (const TCHAR* path)> path_to_photo_;
	const ICMTransform* color_transf_; //TODO: use it
	float vertRes_;
	float horzRes_;
	double ui_gamma_correction_;
	PhotoCache* cache_;
	std::auto_ptr<COleDropTarget> drop_target_;
	auto_connection tags_changed_;
	auto_connection rating_changed_;
	auto_connection metadata_changed_;

	void PaintSeparator(CDC& dc, const CRect& rect);
	void ItemClicked(size_t item, AnyPointer key, PreviewBandWnd::ClickAction action);
	void DrawItem(CDC& dc, CRect rect, size_t item, AnyPointer key);
	String ItemToolTipText(size_t item, AnyPointer key);
	void AddPhotoItem(PhotoInfoPtr p);
	void PhotoModified(PhotoInfoPtr photo);
	Dib* RequestImage(PhotoInfoPtr photo, CSize size);
	void PhotoLoaded(PhotoInfoPtr photo);
	CSize CalcItemSize(PhotoInfoPtr p) const;
	void TagsPopup(CWnd* parent);
	void LoadBitmaps(double gamma);
	DROPEFFECT PhotoDragAndDrop(PhotoDrop::DropAction action, const TCHAR* files);
	bool HasPhoto(ConstPhotoInfoPtr photo) const;
	void AddPhoto(PhotoInfoPtr photo);
	void RefreshAfterChange(const VectPhotoInfo& photos);
};


void LightTable::Impl::PhotoLoaded(PhotoInfoPtr photo)
{
	ctrl_.RedrawItem(photo);
}


Dib* LightTable::Impl::RequestImage(PhotoInfoPtr photo, CSize size)
{
	if (cache_ == 0)
		return 0;

	// if photo cannot be opened then give up now
	if (!::CanPhotoBeDecoded(photo))
	{
		img_loader_.RemovePhoto(photo, true);// queue_.Erase(photo);
		return 0;
	}

	Dib* dib= 0;

	if (CacheImg* img= cache_->FindEntry(photo))
	{
		if (img->Dib() && img->Dib()->IsValid())
		{
			if (::IsCachedImageAvailable(img, size))
				return img->Dib().get();

			// existing (decoded) img is too small, decode it again (below)

			// return existing bmp; it's better than thumbnail; bigger version will be delivered later
			dib = img->Dib().get();
		}
	}

	//if (return_if_not_available)
	//	return 0;

	// add photo to decoder's queue

//TRACE(L"queue: %s\n", photo->name_.c_str());

	if (!img_loader_.AddPhoto(photo, size))
		img_loader_.MarkPhotoAsRequested(photo);

	//	queue_.Add(photo, image_size);
	//else
	//	queue_.RequestedPhoto(photo);

	// get rid of invisible items (TODO: except those for speculative load)
	for (size_t i= 0; i < img_loader_.ItemCount(); )
	{
		PhotoInfoPtr p= img_loader_.GetItem(i);
		if (/*!img_loader_.IsSpeculativeLoadingItem(i) &&*/ !ctrl_.IsItemVisible(p))
			img_loader_.RemovePhoto(p, false);
		else
			++i;
	}

	// start decoder if decoding not pending already
	img_loader_.StartDecoding();

	return dib;
}


CSize LightTable::Impl::CalcItemSize(PhotoInfoPtr p) const
{
	CSize size= p->GetSize();
	//CSize size(p->width_, p->height_);
	//if (size.cx > size.cy && !p->HorizontalOrientation())
	//	swap(size.cx, size.cy);

	if (size.cx == 0 || size.cy == 0)
	{
		ASSERT(false); // decode image size!
		size.cx = 160; size.cy = 120;
	}

	if (vertRes_ > 0.0f && horzRes_ > 0.0f)
	{
		double scrnRatio= horzRes_;
		scrnRatio /= vertRes_;
//			scrnRatio = g_Settings.horz_resolution_ / double(g_Settings.vert_resolution_);

		if (scrnRatio >= 1.0)
			size.cx = static_cast<int>(size.cx * scrnRatio + 0.5);
		else
			size.cy = static_cast<int>(size.cy / scrnRatio + 0.5);
	}
//TRACE(L"size: %d, %d\n", size.cx, size.cy);

	return size;
}


void LightTable::Impl::PhotoModified(PhotoInfoPtr photo)
{
	ctrl_.ModifyItem(CalcItemSize(photo), photo);
}


void LightTable::Impl::AddPhotoItem(PhotoInfoPtr photo)
{
	ctrl_.AddItem(CalcItemSize(photo), photo);
}


void LightTable::Impl::PaintSeparator(CDC& dc, const CRect& rect)
{
	separator_.Draw(&dc, rect);
}


void LightTable::Impl::ItemClicked(size_t item, AnyPointer key, PreviewBandWnd::ClickAction action)
{
	if (item >= photos_.size() || selected_callback_ == 0)
		return;

	if (action == PreviewBandWnd::MouseBtnReleased)
	{
		selected_callback_(photos_[item]);
	}
	else if (action == PreviewBandWnd::StartDragDrop)
	{
		// drag image from the light table into the view pane
		//
		if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd()))
		{
			VectPhotoInfo selected;
			selected.push_back(photos_[item]);
			DragAndDrop(selected, frame);
		}
	}
}


void LightTable::Impl::DrawItem(CDC& dc, CRect rect, size_t item, AnyPointer key)
{
	if (item >= photos_.size())
		return;

	if (Dib* dib= RequestImage(photos_[item], rect.Size()))
	{
		ImageDraw::Draw(dib, &dc, rect, 0, 0, 0, 0, 0, ImageDraw::DRAW_DIBDRAW | ImageDraw::NO_RECT_RESCALING | ImageDraw::DRAW_HALFTONE, 0);
	}
	else
	{
		photos_[item]->Draw(&dc, rect, 0, 0, 0, 0, 0, ImageDraw::DRAW_DIBDRAW | ImageDraw::NO_RECT_RESCALING | ImageDraw::DRAW_HALFTONE);
	}
}


String LightTable::Impl::ItemToolTipText(size_t item, AnyPointer key)
{
	if (item < photos_.size())
	{
		PhotoInfo& inf= *photos_[item];
		return inf.ToolTipInfo(g_Settings.balloon_fields_);
	}

	return _T("");
}


//////////////////////////////////////////////////////////////////////

LightTable::LightTable(PhotoInfoStorage& storage, PhotoCache* cache)
 : DockedPane(DockedPane::PANEL_ALIGN_RIGHT, 200),
	pImpl_(new Impl(storage, cache)), PhotoInfoStorageObserver(storage)
{
}

LightTable::~LightTable()
{
}


BEGIN_MESSAGE_MAP(LightTable, DockedPane)
	ON_WM_SIZE()
//	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnTbDropDown)
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

const static COLORREF TEXT_COLOR= RGB(200,200,200);

void LightTable::Impl::LoadBitmaps(double gamma)
{
	//VERIFY(::LoadPingFromRsrc(MAKEINTRESOURCE(IDB_LIGHT_TABLE_HEADER), header_));
////header_.Load(IDB_LIGHT_TABLE_HEADER);
	separator_.Load(IDB_LIGHT_TABLE_SEP);

	if (gamma != 1.0)
	{
		//::ApplyGammaInPlace(&header_, gamma, -1, -1);
		::ApplyGammaInPlace(&separator_, gamma, -1, -1);
	}
}


bool LightTable::Create(CWnd* parent, UINT id, int width, const boost::function<void (PhotoInfoPtr)>& selected_callback,
						 const boost::function<PhotoInfoPtr (const TCHAR* path)>& path_to_photo)
{
	if (!DockedPane::Create(parent, id, width))
		return false;

	pImpl_->selected_callback_ = selected_callback;
	pImpl_->path_to_photo_ = path_to_photo;

	pImpl_->LoadBitmaps(pImpl_->ui_gamma_correction_);

	// --------- toolbar -----------
	pImpl_->toolbar_.SetPadding(CRect(4,3,4,3));
	{
		const int cmd[]=
		{ ID_ADD_TO_LIGHT_TABLE, ID_REMOVE_FROM_LIGHT_TABLE, ID_LIGHT_TABLE_TAGS, ID_LIGHT_TABLE_OPTIONS };
		FancyToolBar::Params p;
		//p.string_rsrc_id = IDS_LIGHT_TABLE_TOOLBAR;
		p.text_color = TEXT_COLOR;
		p.hot_text_color = RGB(255,255,255);
		p.dis_text_color = RGB(111,111,130);
		if (!pImpl_->toolbar_.Create(this, "PPVv", cmd, IDB_LIGHT_TABLE_TB, &p))
			return false;
	}

	pImpl_->toolbar_.SetOwner(parent);
	pImpl_->toolbar_.SetDlgCtrlID(AFX_IDW_TOOLBAR);
	pImpl_->toolbar_.SetOnIdleUpdateState(true);

	// --------- preview of images -----------

	if (!pImpl_->ctrl_.Create(this))
		return false;

	pImpl_->ctrl_.SetOrientation(false);
	pImpl_->ctrl_.EnableSelectionDisp(false);

	pImpl_->ctrl_.SetCallBacks(
		boost::bind(&Impl::ItemClicked, pImpl_.get(), _1, _2, _3),
		boost::bind(&Impl::DrawItem, pImpl_.get(), _1, _2, _3, _4),
		boost::bind(&Impl::ItemToolTipText, pImpl_.get(), _1, _2));

	// --------- image loader -----------

	pImpl_->img_loader_.SetPhotoLoadedCallback(boost::bind(&Impl::PhotoLoaded, pImpl_.get(), _1));

	// --------- close btn -----------
	{
		const int close_cmd[]= { 1, 1, ID_HIDE_LIGHT_TABLE };
		FancyToolBar::Params p;
		p.shade = -0.28f;
		if (!pImpl_->close_btn_.Create(this, "..p", close_cmd, IDR_CLOSEBAR_PNG, &p))
			return false;
	}

	pImpl_->close_btn_.SetOption(FancyToolBar::HOT_OVERLAY, false);
	pImpl_->close_btn_.SetOwner(parent);

	// --------- separator painting -----------

	SetEraseCallback(boost::bind(&Impl::PaintSeparator, pImpl_.get(), _1, _2));

	// --------- drag & drop (drop actually) support ------------

	pImpl_->drop_target_.reset(
		PhotoDrop::CreatePhotoDropTarget(pImpl_->ctrl_, true, boost::bind(&LightTable::Impl::PhotoDragAndDrop, pImpl_.get(), _1, _2)));

	pImpl_->drop_target_->Register(&pImpl_->ctrl_);

	// ---------

	Resize();

	return true;
}

// this function handles d&d requests to drop photos into the light table
//
DROPEFFECT LightTable::Impl::PhotoDragAndDrop(PhotoDrop::DropAction action, const TCHAR* files)
{
	if (action == PhotoDrop::Enter)
	{
		// check if dropped file is loaded
		if (PhotoInfoPtr photo= path_to_photo_(files))
		{
			if (HasPhoto(photo))
				return DROPEFFECT_NONE;
			else
				return DROPEFFECT_COPY;
		}
		else
			return DROPEFFECT_NONE;
	}
	else if (action == PhotoDrop::Drop)
	{
		if (PhotoInfoPtr photo= path_to_photo_(files))
			if (!HasPhoto(photo))
				AddPhoto(photo);
	}
	else if (action == PhotoDrop::DragOver)
	{
		return DROPEFFECT_COPY;
	}

	return DROPEFFECT_NONE;
}


LRESULT LightTable::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		OnEraseBkgnd(dc);

	return 1;
}


void LightTable::Resize()
{
	if (pImpl_->ctrl_.m_hWnd == 0 || pImpl_->toolbar_.m_hWnd == 0 || pImpl_->close_btn_.m_hWnd == 0)
		return;

	CRect rect(0,0,0,0);
	GetClientRect(rect);

	int height= LIGHTTABLE_H;//pImpl_->header_.GetHeight();

	CSize s= pImpl_->toolbar_.Size();
	SetWindowSize(pImpl_->toolbar_, rect.left, rect.top + height - s.cy, s.cx, s.cy);

	SetWindowSize(pImpl_->ctrl_, 0, height, rect.Width(), rect.Height() - height);

	CSize c= pImpl_->close_btn_.Size();
	SetWindowSize(pImpl_->close_btn_, rect.right - c.cx, 0, c.cx, c.cy);
}


void LightTable::OnSize(UINT type, int cx, int cy)
{
	DockedPane::OnSize(type, cx, cy);

	Resize();
}


// photo delete notification
//
void LightTable::Deleting(PhotoInfoStorage& storage, const VectPhotoInfo& selected)
{
	for (VectPhotoInfo::const_iterator it= selected.begin(); it != selected.end(); ++it)
		RemovePhoto(*it);
}

void LightTable::Deleting(PhotoInfoStorage& storage, PhotoInfoPtr photo)
{
	RemovePhoto(photo);
}

/*
void LightTable::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	switch (info_tip->iItem)
	{
	case ID_LIGHT_TABLE_OPTIONS:
		OperationsPopup();
		break;
	}
	*result = TBDDRET_DEFAULT;
} */


void LightTable::OperationsPopup()
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_LIGHT_TABLE_OPERATIONS))
		return;

	CMenu* popup= menu.GetSubMenu(0);

	CRect rect= pImpl_->toolbar_.GetCmdButtonRect(ID_LIGHT_TABLE_OPTIONS);
	pImpl_->toolbar_.ClientToScreen(rect);

	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.bottom, GetParent());
}


void LightTable::TagsPopup()
{
	pImpl_->TagsPopup(this);
}


void LightTable::Impl::TagsPopup(CWnd* parent)
{
	if (photos_.empty())
		return;

	if (TaggingPopup::IsPopupActive())
		return;

	PhotoTagsCollection& tags= Tags::GetTagCollection();

	CRect rect= toolbar_.GetCmdButtonRect(ID_LIGHT_TABLE_TAGS);
	CPoint pt(rect.left, rect.bottom);
	toolbar_.ClientToScreen(&pt);

	new TaggingPopup(tags, photos_, &::ApplyTagToPhotos, &::ApplyRatingToPhotos, pt, parent);
}


// store light table files in a text file (paths)
void LightTable::Store(const TCHAR* storage_file)
{
	try
	{
		// binary type is needed to preserve 16 bits Unicode
		CStdioFile file(storage_file, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary);

		size_t count= pImpl_->photos_.size();
		for (size_t i= 0; i < count; ++i)
		{
			// real images only, not virtual
			String str= pImpl_->photos_[i]->GetPhysicalPath();
			if (!str.empty())
				file.WriteString((str + _T("\r\n")).c_str());
		}

		file.Close();
	}
	CATCH_ALL
}


namespace {
	struct PhotosByPath
	{
		bool operator () (ConstPhotoInfoPtr p1, ConstPhotoInfoPtr p2) const
		{
			return p1->GetPhysicalPath() < p2->GetPhysicalPath();
		}

		bool operator () (ConstPhotoInfoPtr p1, const CString& path) const
		{
			return p1->GetPhysicalPath() < static_cast<const TCHAR*>(path);
		}
	};
}

// restore light table contents from a text file
void LightTable::Restore(const VectPhotoInfo& photos, const TCHAR* storage_file)
{
	try
	{
		CStdioFile file;
		CFileException ex;
		// reading as binary (16 bit Unicode)
		if (!file.Open(storage_file, CFile::modeRead | CFile::typeBinary, &ex))
			return;

		VectPhotoInfo sorted_photos= photos;
		sort(sorted_photos.begin(), sorted_photos.end(), PhotosByPath());

		CString path;
		while (file.ReadString(path))
		{
			path.TrimRight('\r');
			VectPhotoInfo::iterator it= lower_bound(sorted_photos.begin(), sorted_photos.end(), path, PhotosByPath());
			if (it != sorted_photos.end() && (*it)->GetPhysicalPath() == static_cast<const TCHAR*>(path))
				AddPhoto(*it);
		}

		file.Close();
	}
	CATCH_ALL
}


BOOL LightTable::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
//dc->FillSolidRect(rect, RGB(255,0,0));
//return 1;
	try
	{
		int bottom= rect.bottom;
		rect.bottom = rect.top + LIGHTTABLE_H;//pImpl_->header_.GetHeight();
		dc->FillSolidRect(rect, RGB(25,25,25));
		//pImpl_->header_.Draw(dc, rect);
		CFont* old= dc->SelectObject(&GetDefaultGuiBoldFont());//&font);
		dc->SetTextColor(TEXT_COLOR);
		dc->SetBkMode(TRANSPARENT);
		CString str;
		str.LoadString(IDS_LIGHT_TABLE);
		dc->TextOut(rect.left + 5, rect.top + 4, str);
		dc->SelectObject(old);

		//COLORREF light= CalcNewColor(RGB(78,81,96), pImpl_->ui_gamma_correction_);
		//COLORREF dark= CalcNewColor(RGB(57,59,74), pImpl_->ui_gamma_correction_);

		//TODO: alpha transp. needed here
		//dc->FillSolidRect(rect.left + 9, rect.top + 21, rect.Width() - 18, 1, light);
		//dc->FillSolidRect(rect.left + 9, rect.top + 22, rect.Width() - 18, 1, dark);
		//DrawHorzSeparatorBar(Dib& dest, int x, int y, int width, float black_opacity, float white_opacity)

		rect.top = rect.bottom;
		rect.bottom = bottom;
	}
	catch(...)
	{}

	return true;
}


void LightTable::SetBackgndColor(COLORREF rgb_backgnd)
{
	//no-op
}


void LightTable::SetUIBrightness(double gamma)
{
	pImpl_->ctrl_.SetUIBrightness(gamma);
	pImpl_->ui_gamma_correction_ = gamma;
	pImpl_->LoadBitmaps(gamma);
	Invalidate();
}


void LightTable::SetGamma(const ICMTransform& trans)
{
	pImpl_->color_transf_ = &trans;
	pImpl_->ctrl_.Invalidate();
}


void LightTable::GetPhotos(VectPhotoInfo& photos)
{
	photos = pImpl_->photos_;
}


bool LightTable::HasPhotos() const
{
	return !pImpl_->photos_.empty();
}


void LightTable::Invalidate()
{
	pImpl_->ctrl_.Invalidate();
}


bool LightTable::Impl::HasPhoto(ConstPhotoInfoPtr photo) const
{
	return find(photos_.begin(), photos_.end(), photo) != photos_.end();
}


bool LightTable::HasPhoto(ConstPhotoInfoPtr photo) const
{
	return pImpl_->HasPhoto(photo);
}


void LightTable::EnableToolTips(bool enable)
{
	pImpl_->ctrl_.EnableToolTips(enable);
}


void LightTable::SelectionVisible(ConstPhotoInfoPtr current, bool center_current)
{
//	pImpl_->ctrl_.SelectionVisible(current, center_current);
}


void LightTable::ScrollLeft(int speed_up_factor)
{
	pImpl_->ctrl_.ScrollLeft(speed_up_factor);
}

void LightTable::ScrollRight(int speed_up_factor)
{
	pImpl_->ctrl_.ScrollRight(speed_up_factor);
}


void LightTable::AddPhoto(PhotoInfoPtr photo)
{
	try
	{
		if (HasPhoto(photo))
			return; // photo already there

		pImpl_->AddPhoto(photo);
	}
	CATCH_ALL
}


void LightTable::Impl::AddPhoto(PhotoInfoPtr photo)
{
	ASSERT(!HasPhoto(photo));
	ASSERT(photo);

	photos_.push_back(photo);

	AddPhotoItem(photo);

	ctrl_.Invalidate();

	ctrl_.ResetScrollBar(false);
}


void LightTable::PhotoModified(PhotoInfoPtr photo)
{
	try
	{
		if (!HasPhoto(photo))
			return; // photo not here

		ASSERT(photo);

		pImpl_->PhotoModified(photo);
	}
	CATCH_ALL
}


void LightTable::RemovePhoto(PhotoInfoPtr photo)
{
	try
	{
		VectPhotoInfo::iterator it= find(pImpl_->photos_.begin(), pImpl_->photos_.end(), photo);

		if (it == pImpl_->photos_.end())
			return; // photo not here

		pImpl_->photos_.erase(it);

		//TODO: remove single item

		pImpl_->ctrl_.RemoveAllItems();
		pImpl_->ctrl_.ReserveItems(pImpl_->photos_.size());

		for (size_t i= 0; i < pImpl_->photos_.size(); ++i)
		{
			PhotoInfoPtr p= pImpl_->photos_[i];
			pImpl_->AddPhotoItem(p);
		}

		pImpl_->ctrl_.Invalidate();
		pImpl_->ctrl_.ResetScrollBar(false);
	}
	CATCH_ALL
}


void LightTable::SetScreenAspectRatio(float horzRes, float vertRes)
{
	if (pImpl_->vertRes_ == vertRes && pImpl_->horzRes_ == horzRes)
		return;

	pImpl_->vertRes_ = vertRes;
	pImpl_->horzRes_ = horzRes;

	pImpl_->ctrl_.RemoveAllItems();
	pImpl_->ctrl_.ReserveItems(pImpl_->photos_.size());

	for (size_t i= 0; i < pImpl_->photos_.size(); ++i)
	{
		PhotoInfoPtr p= pImpl_->photos_[i];
		pImpl_->AddPhotoItem(p);
	}

	pImpl_->ctrl_.Invalidate();
	pImpl_->ctrl_.ResetScrollBar(false);
}


void LightTable::RemoveAll()
{
	pImpl_->ctrl_.RemoveAllItems();
	pImpl_->photos_.clear();
	pImpl_->ctrl_.Invalidate();
	pImpl_->ctrl_.ResetScrollBar(false);
}


void LightTable::Impl::RefreshAfterChange(const VectPhotoInfo& photos)
{
	ctrl_.Invalidate();

	TaggingPopup::Refresh(photos_);
}
