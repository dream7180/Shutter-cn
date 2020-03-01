/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TagsBarCommon.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TagsBarCommon.h"
#include "TagsCommonCode.h"
#include "TagsManageDlg.h"
#include "BalloonMsg.h"
#include "Config.h"
#include "Color.h"
#include "CatchAll.h"
#include "block.h"
#include <boost/algorithm/string/replace.hpp>
#include "PhotoInfo.h"
#include "PhotoCtrl.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/find.hpp>
#include <set>
#include "SetWindowSize.h"
#include "UIElements.h"

extern void DrawParentBkgnd(CWnd& wnd, CDC& dc);

using namespace boost::algorithm;
static const int CLEAR_RATING= 11111;
static wchar_t REG_TAG_BAR[]= L"TagBar";

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDialogEX, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CDialogEX::CDialogEX()
{
	background_ = ::GetSysColor(COLOR_3DFACE);
}


void CDialogEX::SetColors(COLORREF text, COLORREF backgnd)
{
	text_ = text;
	background_ = backgnd;
	background_brush_.DeleteObject();
	background_brush_.CreateSolidBrush(backgnd);
	if (m_hWnd)
		Invalidate();
}


BOOL CDialogEX::OnEraseBkgnd(CDC* dc)
{
	CRect cl_rect(0,0,0,0);
	GetClientRect(cl_rect);
	dc->FillSolidRect(cl_rect, background_);
	return true;
}


HBRUSH CDialogEX::OnCtlColor(CDC* dc, CWnd* ctrl, UINT code)
{
	HBRUSH hbr= CWnd::OnCtlColor(dc, ctrl, code);

	if (ctrl != 0 && ctrl->GetDlgCtrlID() == IDC_LABEL)
	{
		dc->SetTextColor(text_);
	}

	dc->SetBkColor(background_);

	return background_brush_;
}

/////////////////////////////////////////////////////////////////////////////
// TagsBarCommon

TagsBarCommon::TagsBarCommon(PhotoTagsCollection& tag_collection, const wchar_t* registry_key)
  : tag_collection_(tag_collection)
{
	top_ = 0;
	tags_enabled_ = false;
	left_indent_ = 2;
	top_indent_ = 0;
	in_update_ = false;
	//tags_changed_ = false;		// if changed, save them on exit
	tool_bar_at_top_ = true;
	//create_big_toolbar_ = false;
	create_toolbar_ = true;
	create_status_bar_ = true;
	create_rating_wnd_ = true;
	create_filter_bar_ = true;
	rgb_background_ = ::GetSysColor(COLOR_WINDOW);
	edit_background_ = rgb_background_;
	extra_tags_begin_ = -1;
	use_parent_background_ = false;
	registry_key_ = registry_key;
}


TagsBarCommon::~TagsBarCommon()
{}


BEGIN_MESSAGE_MAP(TagsBarCommon, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
//	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_TAGS_MANAGE, OnTagsManage)
	ON_UPDATE_COMMAND_UI(ID_TAGS_MANAGE, OnUpdateTagsManage)
	ON_COMMAND(ID_TAGS_SAVE, OnTagsSave)
	ON_UPDATE_COMMAND_UI(ID_TAGS_SAVE, OnUpdateTagsSave)
	ON_WM_DESTROY()
	ON_COMMAND(ID_TAGS_LOAD, OnTagsLoad)
	ON_UPDATE_COMMAND_UI(ID_TAGS_LOAD, OnUpdateTagsLoad)
	ON_UPDATE_COMMAND_UI(ID_TAGS_SAVE_IN_PHOTO, OnUpdateTagsSaveInPhoto)
	ON_COMMAND(ID_TAGS_SAVE_IN_PHOTO, OnTagsSaveInPhoto)
	ON_UPDATE_COMMAND_UI(ID_TAGS_OPTIONS, OnUpdateTagsOptions)
	ON_NOTIFY(TBN_DROPDOWN, ID_TOOLBAR, OnTbDropDown)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + 9999, OnUpdateTags)
	ON_COMMAND_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + 9999, OnTagSelected)
	ON_COMMAND(CLEAR_RATING, OnClearRating)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_EN_CHANGE(ID_FILTER, OnChangeFilter)
	ON_COMMAND(ID_APPLY_TAGS, OnApplyTags)
	ON_COMMAND(ID_TAG_FILTER, OnTagFilter)
	ON_COMMAND(ID_CANCEL, OnCancelFilter)
END_MESSAGE_MAP()


static const float COLOR_SHADE= 7.0f;


/////////////////////////////////////////////////////////////////////////////
// TagsBarCommon message handlers


bool TagsBarCommon::Create(CWnd* parent)
{
	create_toolbar_ = false;
	create_status_bar_ = false;
	create_rating_wnd_ = false;
	create_filter_bar_ = false;

	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW),
		reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1)), 0, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;
/*
	// reload tags
	bool loaded= false;
	try
	{
		loaded = LoadTags(GetTagsPathName().c_str());
	}
	catch (...)
	{
	}

	if (!loaded)
	{
		// no tags loaded--add some so tag bar doesn't show up empty
		tag_collection_.FromString(_T("Good\nSharp\nCandidate\nOrder Print\n"));
		SynchronizeTags(tag_collection_);
		tags_changed_ = true;
	}
*/
	on_change_ = tag_collection_.ConnectOnChange(boost::bind(&TagsBarCommon::TagCollectionChanged, this));

	SynchronizeTags(tag_collection_);

	return true;
}


void TagsBarCommon::TagCollectionChanged()
{
	SynchronizeTags(tag_collection_);
}


bool TagsBarCommon::Create(CWnd* parent, bool big_toolbar)
{
	//create_big_toolbar_ = big_toolbar;

	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW),
		reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1)), 0, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;
/*
	// reload tags
	bool loaded= false;
	try
	{
		loaded = LoadTags(GetTagsPathName().c_str());
	}
	catch (...)
	{
	}

	if (!loaded)
	{
		// no tags loaded--add some so tag bar doesn't show up empty
		tag_collection_.FromString(_T("Good\nSharp\nCandidate\nOrder Print\n"));
		SynchronizeTags(tag_collection_);
		tags_changed_ = true;
	}
*/
	on_change_ = tag_collection_.ConnectOnChange(boost::bind(&TagsBarCommon::TagCollectionChanged, this));

	SynchronizeTags(tag_collection_);

	PhotoSelected(0, false, false);

	return true;
}


COLORREF TagsBarCommon::GetBackgndColor() const
{
	return rgb_background_;
//	return CalcNewColor(::GetSysColor(COLOR_3DFACE), COLOR_SHADE);
}


BOOL TagsBarCommon::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (use_parent_background_)
		DrawParentBkgnd(*this, *dc);
	else
		dc->FillSolidRect(rect, rgb_background_);

	return true;
}


CSize TagsBarCommon::GetIdealSize()
{
	// this only works when there's no toolbar & status bar
	CSize s= checks_wnd_.GetIdealSize();
	s.cx += left_indent_ + 1;
	s.cy += top_indent_;
	return s;
}


void TagsBarCommon::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	CSize bar_size(::GetSystemMetrics(SM_CXVSCROLL), ::GetSystemMetrics(SM_CYHSCROLL));

	CRect rect(0, 0, cx, cy);
	const int w= rect.Width();

	if (filter_bar_.m_hWnd)
	{
		CRect r(0,0,0,0);
		filter_bar_.GetWindowRect(r);
		int margin = Pixels(3);
		r.left = margin;
		r.right = cx - margin;
		if (r.Width() < 0)
			r.right = rect.left;
		SetWindowSize(filter_bar_, margin, 0, r.Width(), r.Height());
	}

	rect.top = top_indent_ + top_;

	if (status_wnd_.m_hWnd)
	{
		WINDOWPLACEMENT wp;
		if (status_wnd_.GetWindowPlacement(&wp))
		{
			int h= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			UINT flags= SWP_SHOWWINDOW;
			if (h > rect.Height())
				flags = SWP_HIDEWINDOW;
			else
				rect.bottom -= h;

			status_wnd_.SetWindowPos(0, 0, rect.bottom, cx, h, SWP_NOACTIVATE | SWP_NOZORDER | flags);

			if (status_label_.m_hWnd && status_label_.GetWindowPlacement(&wp))
			{
				int height= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
				status_label_.SetWindowPos(0, 0, 0, w, height, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | flags);
			}
		}
	}

	if (rating_wnd_.m_hWnd)
	{
		CRect w(0,0,0,0);
		rating_wnd_.GetClientRect(w);
		UINT flags= SWP_SHOWWINDOW;
		int h= w.Height();
		int SEP_H = Pixels(2);
		int margin = Pixels(5);
		if (h > rect.Height())
			flags = SWP_HIDEWINDOW;
		else
			rect.bottom -= h;

		rating_wnd_.SetWindowPos(0, 0, rect.bottom, rect.Width(), h, SWP_NOACTIVATE | SWP_NOZORDER | flags);
		if (separator_.m_hWnd)
			separator_.SetWindowPos(0, margin, 0, std::max(0, rect.Width() - 2 * margin), SEP_H, SWP_NOACTIVATE | SWP_NOZORDER | flags);
	}

	int tags_area_height= MAX(0, rect.Height());

	if (checks_wnd_.m_hWnd)
	{
		int scroll_bar_w= 1;//scroll_bar_wnd_.m_hWnd && scroll_bar_wnd_.GetStyle() & WS_VISIBLE ? bar_size.cx : 1;
		int dx= MAX(0, cx - left_indent_ - scroll_bar_w);
		checks_wnd_.SetWindowPos(0, left_indent_, rect.top /* top_ + top_indent_*/, dx, tags_area_height, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	//if (scroll_bar_wnd_.m_hWnd)
	//{
	//	scroll_bar_wnd_.SetWindowPos(0, cx - bar_size.cx, top_ + top_indent_, bar_size.cx, tags_area_height, SWP_NOACTIVATE | SWP_NOZORDER);

	//	SCROLLINFO si;
	//	si.cbSize = sizeof si;
	//	si.nPage = tags_area_height;
	//	si.nMin = 0;
	//	si.nMax = 300;
	//	si.fMask = SIF_RANGE | SIF_PAGE;
	//	scroll_bar_wnd_.SetScrollInfo(&si);
	//}
}


int TagsBarCommon::OnCreate(LPCREATESTRUCT create_struct)
{
	if (CWnd::OnCreate(create_struct) == -1)
		return -1;

	if (create_status_bar_)
	{
		VERIFY(status_wnd_.Create(IDD_TAG_STATUS, this));
		VERIFY(status_label_.SubclassDlgItem(IDC_LABEL, &status_wnd_));
	}

	if (create_rating_wnd_)
	{
		VERIFY(rating_wnd_.Create(IDD_RATING_FORM, this));
		VERIFY(stars_.SubclassDlgItem(IDC_STARS, &rating_wnd_));
		//VERIFY(rate_label_.SubclassDlgItem(IDC_RATE, &rating_wnd_));
		VERIFY(separator_.SubclassDlgItem(IDC_SEPARATOR, &rating_wnd_));
		stars_.SetClickCallback(boost::bind(&TagsBarCommon::StarClicked, this, _1));

		clear_rating_.SetOwnerDraw(true);
		VERIFY(clear_rating_.SubclassDlgItem(IDC_CANCEL, &rating_wnd_));
		int cmd[]= { CLEAR_RATING };
		enum { DEFAULT_TB_PAD_DX= 8, DEFAULT_TB_PAD_DY= 10 };
		clear_rating_.SetPadding(DEFAULT_TB_PAD_DX, DEFAULT_TB_PAD_DY);
		clear_rating_.AddButtons("P", cmd, IDB_CANCEL, 0);
		clear_rating_.SetOnIdleUpdateState(false);
		clear_rating_.SetOwner(this);
	}

	if (create_toolbar_)
	{
		tool_bar_wnd_.SetOwnerDraw(true);
		static int cmds[]= { ID_TAGS_MANAGE, ID_TAGS_OPTIONS };
		tool_bar_wnd_.SetOnIdleUpdateState(false);
		int bmp_id= IDB_TAGS_TOOLBAR;//create_big_toolbar_ ? IDB_TAGS_TOOLBAR_BIG : IDB_TAGS_TOOLBAR;
		VERIFY(tool_bar_wnd_.Create("pv", cmds, bmp_id, 0, this, ID_TOOLBAR));

		WINDOWPLACEMENT wp;
		wp.length = sizeof wp;
		tool_bar_wnd_.GetWindowPlacement(&wp);

		tool_bar_wnd_.SetOwner(this);

		//tool_bar_wnd_.CreateDisabledImageList(bmp_id);

		top_ = tool_bar_at_top_ ? wp.rcNormalPosition.bottom + 1 : 0;
	}
	else
		top_ = 0;

	if (create_filter_bar_)
	{
		filter_bar_.SetMargins(CRect(3, 3, 3, 0));
		filter_bar_.Create(this, IDB_ENTER_TOOLBAR, ID_APPLY_TAGS, ID_CANCEL, EditCombo::TWO_BUTTONS);
		filter_bar_.SetDlgCtrlID(ID_FILTER);
		filter_bar_.SetImage(IDB_FUNNEL_SMALL);
		filter_bar_.SetColors(rgb_background_, 0, 0);

		CRect rect(0,0,0,0);
		filter_bar_.GetWindowRect(rect);
		top_ += rect.Height();

		OnChangeFilter();

		filter_bar_.ConnectFinishCommand(boost::bind(&TagsBarCommon::OnFilterCmd, this, _1));
	}

	CreateTagsButtons();

	return 0;
}

/*
void TagsBarCommon::OnGetMinMaxInfo(MINMAXINFO FAR* MMI)
{
	CWnd::OnGetMinMaxInfo(MMI);
	MMI->ptMinTrackSize.x = 100;
	MMI->ptMinTrackSize.y = top_ + top_indent_ + 50;
	if (tool_bar_wnd_.m_hWnd)
	{
		CRect rect;
		tool_bar_wnd_.GetWindowRect(rect);
		MMI->ptMinTrackSize.x = rect.Width() + 8;	// min window width
	}
}
*/

void TagsBarCommon::OnUpdateTags(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(apply_tag_to_photos_ != 0 && tags_enabled_);
}


void TagsBarCommon::OnTagSelected(UINT cmd)
{
	int index= cmd - ID_TAG_SELECTED;
	AssignTag(index);
}


void TagsBarCommon::StarClicked(int stars)
{
	if (apply_rating_to_photos_ != 0 && !selected_photos_.empty() && stars >= 0)
		apply_rating_to_photos_(selected_photos_, stars, GetParent());
}


void TagsBarCommon::SetTagCallback(const ApplyTagsFn& fn)
{
	apply_tag_to_photos_ = fn;
}


void TagsBarCommon::SetRatingCallback(const ApplyRatingFn& fn)
{
	apply_rating_to_photos_ = fn;
}


void TagsBarCommon::RedrawItem(int index)	// redraw one button
{
	//TODO: optimize
	checks_wnd_.Invalidate();
}


void TagsBarCommon::CreateTagsButtons()
{
	checks_wnd_.Create(this, CRect(0,0,0,0), 0-1, this);
	checks_wnd_.SetCheckBoxes(true);
	checks_wnd_.SetBackgndColor(rgb_background_);
	checks_wnd_.SetItemSpace(1.8f);
}


void TagsBarCommon::EnableStars(bool enable)
{
TRACE(L"stars enabled: %d\n", enable ? 1 : 0);
	if (stars_.m_hWnd)
	{
		stars_.EnableWindow(enable);
		stars_.Invalidate();
		if (!enable)
			clear_rating_.ShowWindow(SW_HIDE);
	}
}


void TagsBarCommon::SyncButtons(ConstPhotoInfoPtr photo)
{
	bool change= false;

	if (photo == 0)	// nothing's selected?
	{
		tags_enabled_ = false;

		if (tags_.AllTagsClear())
			change = true;
	}
	else
	{
		tags_enabled_ = true;

		AddPhotoSpecificTags(photo);

		if (SyncButtons(*photo, true) == CHANGED)
			change = true;

		PhotoInfoPtr p= ConstCast(photo);
		SyncStars(&p, 1);
	}

	EnableStars(tags_enabled_);

//	checks_wnd_.EnableWindow(true);
	checks_wnd_.Invalidate();
}


void TagsBarCommon::SyncButtons(const VectPhotoInfo& photos)
{
	bool change= false;

	if (photos.empty())
	{
		if (tags_enabled_)
		{
			AddPhotoSpecificTags(photos);
			change = true;
		}

		tags_enabled_ = false;

		if (tags_.AllTagsClear())
			change = true;
	}
	else
	{
		if (!tags_enabled_)
			change = true;

		tags_enabled_ = true;

		AddPhotoSpecificTags(photos);

		if (SyncButtons(*photos.front(), true) == CHANGED)
			change = true;

		if (photos.size() > 1)
		{
			for (VectPhotoInfo::const_iterator it= photos.begin() + 1; it != photos.end(); ++it)
			{
				SyncState flag= SyncButtons(**it, false);

				if (flag == STOP)
					break;

				if (flag == CHANGED)
					change = true;
			}
		}

		SyncStars(&photos.front(), photos.size());
	}

	EnableStars(tags_enabled_);

	if (change)
	{
		//checks_wnd_.EnableWindow(tags_enabled_);
		checks_wnd_.Invalidate();
	}
}


void TagsBarCommon::SyncStars(const PhotoInfoPtr photos[], size_t count)
{
	if (stars_.m_hWnd == 0)
		return;

	ASSERT(count > 0);

	int rating= photos[0]->GetRating();
	bool different= false;

	for (size_t i= 1; i < count; ++i)
	{
		int stars= photos[i]->GetRating();

		if (stars != rating)
			different = true;	// different ratings

		if (stars > rating)
			rating = stars;
	}

	stars_.SetRating(different ? -rating : rating);
	clear_rating_.ShowWindow(rating != 0 ? SW_SHOWNA : SW_HIDE);
}


TagsBarCommon::SyncState TagsBarCommon::SyncButtons(const PhotoInfo& photo, bool ignore_existing_state)
{
	const PhotoTags& tags= photo.GetTags();
	SyncState continue_flag= STOP;
	bool has_tags= !tags.empty();
	TagBtn::TagState state= TagBtn::UNSELECTED;

	if (ignore_existing_state)
	{
		for (Tags::iterator it= tags_.begin(); it != tags_.end(); ++it)
		{
			if (has_tags)
				state = tags.FindTag(it->name_) ? TagBtn::SELECTED : TagBtn::UNSELECTED;

			if (it->state_ != state)
			{
				it->state_ = state;
				continue_flag = CHANGED;
			}
		}
	}
	else
	{
		for (Tags::iterator it= tags_.begin(); it != tags_.end(); ++it)
		{
			if (it->state_ == TagBtn::UNDETERMINED)
				continue;

			if (continue_flag == STOP)
				continue_flag = CONT;

			if (has_tags)
				state = tags.FindTag(it->name_) ? TagBtn::SELECTED : TagBtn::UNSELECTED;

			if (it->state_ != state)
			{
				it->state_ = TagBtn::UNDETERMINED;
				continue_flag = CHANGED;
			}
		}
	}

	return continue_flag;
}


void TagsBarCommon::RememberCollapsedGroups()
{
	if (checks_wnd_.m_hWnd && filter_text_.empty())
	{
		String collapsed_groups;

		int group= checks_wnd_.GetFirstGroup();
		while (group >= 0)
		{
			const String& label= checks_wnd_.GetItemText(group);
			bool collapsed= checks_wnd_.IsGroupCollapsed(group);

			if (collapsed)
				collapsed_groups += label + L'\n';

			group = checks_wnd_.GetNextGroup(group);
		}

		AfxGetApp()->WriteProfileString(REG_TAG_BAR, registry_key_, collapsed_groups.c_str());
	}
}


void TagsBarCommon::OnDestroy()
{
	if (tag_collection_.IsModified())
	{
		try
		{
			SaveTags(::Tags::GetTagsPathName().c_str(), tag_collection_);
		}
		catch (...)
		{
			ASSERT(false);	// saving failed
		}
	}

	RememberCollapsedGroups();

	CWnd::OnDestroy();
}


void TagsBarCommon::PhotoSelected(PhotoInfoPtr photo, bool selection, bool still_loading)
{
	if (in_update_)
		return;

	try
	{
		SyncButtons(photo);

		selected_photos_.clear();

		if (photo != 0)
			selected_photos_.push_back(photo);

		UpdateStatusBar(selection, still_loading);

		UpdateFilterButton();
	}
	CATCH_ALL_W(this)
}


void TagsBarCommon::PhotosSelected(const VectPhotoInfo& photos)
{
	if (in_update_)
		return;

	try
	{
		SyncButtons(photos);

		selected_photos_ = photos;

		UpdateStatusBar(true, false);

		UpdateFilterButton();
	}
	CATCH_ALL_W(this)
}


String TagsBarCommon::GetTagItemName(int index, const String& tag) const
{
	oStringstream ost;
	ost << (index + 1) << _T(". ") << tag;
	return ost.str();
}


void TagsBarCommon::SynchronizeTags(const PhotoTagsCollection& tags)
{
	checks_wnd_.DeleteFromItem(0);

	const size_t tag_count= tags.GetCount();
	const size_t group_count= tags.GroupCount();

	tags_.clear();
	tags_.reserve(tag_count);

	checks_wnd_.ReserveItems(group_count + tag_count);

	std::set<String> groups;
	{
		String collapsed_groups= AfxGetApp()->GetProfileString(REG_TAG_BAR, registry_key_);

		if (!collapsed_groups.empty())
			split(groups, collapsed_groups, is_any_of(L"\n"));
	}

	for (size_t g= 0; g < group_count; ++g)
	{
		const String& name= tags.GroupName(g);
		std::pair<size_t, size_t> span= tags.GroupSpan(g);

		if (!name.empty())
		{
			int item= checks_wnd_.OpenGroup(name.c_str());
			bool collapsed= groups.count(name) > 0;
			if (collapsed)
				checks_wnd_.CollapseGroup(item, collapsed);
		}

		for (size_t i= span.first; i < span.second; ++i)
		{
			const String& tag= tag_collection_.Get(i);
			tags_.push_back(tag);
			checks_wnd_.AddItem(tag.c_str(), 0, 0, tags_.size() - 1);
		}

		checks_wnd_.CloseGroup();
	}

	extra_tags_begin_ = checks_wnd_.GetItemCount();

/*
	tags_.clear();

	int count= checks_wnd_.GetItemCount();
	int index= 0;

	tags_.reserve(count);
	checks_wnd_.ReserveItems(count);

	for (; index < tag_collection_.GetCount(); ++index)
	{
		if (index >= count)
			checks_wnd_.AddItem(_T(""));

		const String& tag= tag_collection_.Get(index);
		String item= GetTagItemName(index, tag);
		checks_wnd_.SetItemText(index, item.c_str());
		tags_.push_back(tag);
	}

	// excessive buttons
	checks_wnd_.DeleteFromItem(index);
*/
}


void TagsBarCommon::AddPhotoSpecificTags(ConstPhotoInfoPtr photo)
{
	{
		if (tags_.size() > tag_collection_.GetCount())
		{
			// remove excessive buttons
			checks_wnd_.DeleteFromItem(extra_tags_begin_);
			tags_.erase(tags_.begin() + tag_collection_.GetCount(), tags_.end());
		}

		if (photo == 0 || photo->tags_.empty())
			return;
	}

	const size_t count= photo->tags_.size();

	checks_wnd_.ReserveItems(count + count);
	tags_.reserve(count + count);

	for (size_t i= 0; i < count; ++i)
	{
		if (!tag_collection_.HasTag(photo->tags_[i]))
		{
			const String& tag= photo->tags_[i];
			checks_wnd_.AddItem(tag.c_str(), 0, 0, tags_.size());
			tags_.push_back(tag);
		}
	}
}


void TagsBarCommon::AddPhotoSpecificTags(const VectPhotoInfo& photos)
{
	if (tags_.size() > tag_collection_.GetCount())
	{
		// remove excessive buttons
		checks_wnd_.DeleteFromItem(extra_tags_begin_);
		tags_.erase(tags_.begin() + tag_collection_.GetCount(), tags_.end());
	}

	if (photos.empty())
		return;

	PhotoTagsCollection collection= tag_collection_;

	for (VectPhotoInfo::const_iterator it= photos.begin(); it != photos.end(); ++it)
	{
		ConstPhotoInfoPtr photo= *it;

		if (photo == 0 || photo->tags_.empty())
			continue;

		const size_t tag_count= photo->tags_.size();
		int count= checks_wnd_.GetItemCount();

		checks_wnd_.ReserveItems(tag_count + count);
		tags_.reserve(tag_count + count);

		for (size_t i= 0; i < tag_count; ++i)
		{
			if (!collection.HasTag(photo->tags_[i]))
			{
				const String& new_tag= photo->tags_[i];

				collection.AddTag(new_tag);
				checks_wnd_.AddItem(new_tag.c_str(), 0, 0, tags_.size());
				tags_.push_back(new_tag);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////


extern void ManageTagsCollection(CWnd* parent, PhotoTagsCollection& tag_collection)
{
	try
	{
		CTagsManageDlg dlg(parent);

		dlg.tags_ = tag_collection.AsString().c_str();

		if (dlg.DoModal() == IDOK)
			tag_collection.FromString(String(dlg.tags_));
	}
	CATCH_ALL_W(parent)
}


void TagsBarCommon::OnTagsManage()
{
	RememberCollapsedGroups();
	ManageTagsCollection(this, tag_collection_);
}


void TagsBarCommon::OnUpdateTagsManage(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


///////////////////////////////////////////////////////////////////////////////

static TCHAR* g_ext= _T(".txt");
static TCHAR* g_name= _T("Photo Tags.txt");
static TCHAR* g_filter= _T("标记文本文件 (*.txt)|*.txt|所有文件 (*.*)|*.*||");

void TagsBarCommon::OnTagsLoad()
{
	try
	{
		CFileDialog dlg(true, g_ext, g_name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, g_filter, this);

		if (dlg.DoModal() == IDOK)
		{
			LoadTags(dlg.GetPathName());

			//		AfxGetApp()->WriteProfileString(REGISTRY_SECTION_TAGS_WND, REG_LAST_FILE, dlg.GetPathName());

			//tags_changed_ = true;
		}
	}
	CATCH_ALL_W(this)
}


bool TagsBarCommon::LoadTags(const TCHAR* filename)
{
	CFile file(filename, CFile::modeRead);

	ULONGLONG long_len= file.GetLength();
	if (long_len > 0x100000)
	{
		new BalloonMsg(this, _T("标记文件太大"), _T("尝试载入的文件过大.\n文件应不大于 1 MB."), BalloonMsg::IERROR);
		return false;
	}

	DWORD len= static_cast<DWORD>(long_len);

	std::vector<TCHAR> buf(1 + len / sizeof TCHAR);

	//TODO: handle non-Unicode

	file.Read(&buf.front(), len);

	// skip Unicode signature
	if (len > 2 && buf.front() == 0xfeff)		// Unicode marker?
		buf.erase(buf.begin(), buf.begin() + 1);

	replace_all(buf, _T("\xd\xa"), _T("\xa"));

	tag_collection_.FromString(String(&buf.front()));

	return true;
}


void TagsBarCommon::OnUpdateTagsLoad(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void TagsBarCommon::OnTagsSave()
{
	try
	{
		if (tag_collection_.Empty())
			return;

		CFileDialog dlg(false, g_ext, g_name,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN, g_filter, this);

		if (dlg.DoModal() == IDOK)
			SaveTags(dlg.GetPathName(), tag_collection_);
	}
	CATCH_ALL_W(this)
}


void TagsBarCommon::OnUpdateTagsSave(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(!tag_collection_.Empty());
}


void TagsBarCommon::SaveTags(const TCHAR* filename, const PhotoTagsCollection& collection)
{
	// save tags
	CFile file(filename, CFile::modeWrite | CFile::modeCreate);

	String tags= collection.AsString();

	if (tags.empty())
	{
		ASSERT(false);
		return;
	}

#ifdef _UNICODE
	// insert unicode signature, so Notepad will handle this file properly
	tags.insert(0, 1, wchar_t(0xfeff));
#endif

	// DOS end of lines
	replace_all(tags, _T("\xa"), _T("\xd\xa"));

	file.Write(&tags[0], static_cast<UINT>(tags.length() * sizeof TCHAR));
}


void TagsBarCommon::UpdateStatusBar(bool selection, bool still_loading)
{
	if (status_label_.m_hWnd == 0)// || rate_label_.m_hWnd == 0)
		return;

	int pane_type= SBT_NOBORDERS;
	int pane_index= 1;

	//rate_label_.EnableWindow(!selected_photos_.empty());

	if (selected_photos_.empty())
	{
		status_label_.SetWindowText(L"");
	}
	else if (selected_photos_.size() == 1)
	{
//		status_label_.SetWindowText(selected_photos_.front()->GetName().c_str());//, pane_index, pane_type);

		int err= 0;
		if (still_loading)
			err = -1;
		else
			selected_photos_.front()->CanEditIPTC(err);

		//HICON icon= AfxGetApp()->LoadIcon(err == 0 ? IDI_TAGS_EDIT : IDI_TAGS_NO_EDIT);
		//status_wnd_.SetIcon(0,  icon);

		oStringstream ost;
		if (err)
			ost << _T("只读 ");
		else
			ost << _T("选定的 ");
		// name in double quotation marks
		ost << L'\x201c' << selected_photos_.front()->GetName() << L'\x201d';

		status_label_.SetWindowText(ost.str().c_str());
	}
	else
	{
		oStringstream ost;
		ost << selected_photos_.size() << _T(" 选定的图像");
		status_label_.SetWindowText(ost.str().c_str());//, pane_index, pane_type);
		//status_label_.SetIcon(0, 0);
	}
}


void TagsBarCommon::OnUpdateTagsSaveInPhoto(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(g_Settings.save_tags_to_photo_ ? 1 : 0);
}


void TagsBarCommon::OnTagsSaveInPhoto()
{
	g_Settings.save_tags_to_photo_ = !g_Settings.save_tags_to_photo_;
}


void TagsBarCommon::ItemClicked(ListViewCtrl& ctrl, int index, size_t param)
{
	if (!tags_enabled_)
		return;

	if (param >= tags_.size())
	{
		ASSERT(false);
		return;
	}

	if (apply_tag_to_photos_ != 0 && !selected_photos_.empty())
	{
		bool apply= tags_[param].state_ == TagBtn::UNSELECTED;

		if (apply_tag_to_photos_(selected_photos_, tags_[param].name_, apply, GetParent()))
		{
			// no redraw is necessary, applying tags triggers refresh notifications (hopefully)
			//tags_[param].state_ = apply ? TagBtn::SELECTED : TagBtn::UNSELECTED;
			//RedrawItem(param);
		}
	}
}


int TagsBarCommon::GetItemCheckState(ListViewCtrl& ctrl, int index, size_t param)
{
	if (param < tags_.size())
	{
		switch (tags_[param].state_)
		{
		case TagBtn::UNSELECTED:
			return 0;

		case TagBtn::SELECTED:
			return 1;

		case TagBtn::UNDETERMINED:
			return 2;

		default:
			ASSERT(false);
			break;
		}
	}
	else
	{ ASSERT(false); }

	return 0;
}


bool TagsBarCommon::IsItemEnabled(ListViewCtrl& ctrl, int index, size_t param)
{
	if (ctrl.IsGroup(index))
		return true;

	return tags_enabled_;
}


bool TagsBarCommon::FilterItem(ListViewCtrl& ctrl, int index, size_t param, bool group, const String& label, bool& filter_in)
{
	if (filter_text_.empty())	// no filter?
		return false;	// no active filter

	filter_in = ifind_first(label, filter_text_);

	return true;
}


void TagsBarCommon::ItemColors(ListViewCtrl& ctrl, int index, size_t param, COLORREF& rgb_text, COLORREF& rgb_backgnd)
{
	//if (param < tags_.size())
	//	if (tags_[param].state_ == TagBtn::SELECTED)
	//		rgb_backgnd = CalcNewColorDelta(rgb_backgnd, 0.5f, -0.04f, 3.7f);
}


void TagsBarCommon::DrawItemBackground(ListViewCtrl& ctrl, int index, size_t param, CDC& dc, CRect rect)
{
	if (param < tags_.size())
		if (tags_[param].state_ == TagBtn::SELECTED)
		{
			auto margin = Pixels(1);
			rect.DeflateRect(margin, margin, 0, margin);
			auto color = GetAppColors()[AppColors::AccentBackground];
			::FillRoundRect(dc, rect, 2.0f, color);
		}
}


void TagsBarCommon::PhotoReloadingDone()
{
	UpdateStatusBar(true, false);
}


BOOL TagsBarCommon::IsFrameWnd() const
{
	return true;	// true so TagsBarCommon can handle UI messages sent by ToolBarWnd
}


void TagsBarCommon::AssignTag(int index)
{
	if (index < 0 || index >= tags_.size())
		return;

	if (apply_tag_to_photos_ != 0 && !selected_photos_.empty())
	{
		bool apply= tags_[index].state_ == TagBtn::UNSELECTED;

		if (apply_tag_to_photos_(selected_photos_, tags_[index].name_, apply, GetParent()))
		{
			tags_[index].state_ = apply ? TagBtn::SELECTED : TagBtn::UNSELECTED;
			RedrawItem(index);
		}
	}
}


void TagsBarCommon::OnViewPopupMenu(CWnd* toolbar, CRect btn)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_TAG_BAR_POPUP_MENU))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	if (popup == 0)
	{
		ASSERT(false);
		return;
	}

//	CRect rect= btn;
//	tool_bar_wnd_.GetRect(ID_TAGS_OPTIONS, rect);
	CPoint pos(btn.left, btn.bottom);
	toolbar->ClientToScreen(&pos);

	popup->TrackPopupMenu(TPM_LEFTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON, pos.x, pos.y, this);
}


void TagsBarCommon::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info->iItem)
	{
	case ID_TAGS_OPTIONS:
		OnViewPopupMenu(CWnd::FromHandle(info->hdr.hwndFrom), info->rcButton);
		break;

	default:
		break;
	}
}


void TagsBarCommon::OnUpdateTagsOptions(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


extern void ResetPopupMenuTags(CMenu& menu, int first_id, const PhotoTagsCollection& tags)
{
	// delete all items first
	{
		const UINT count= menu.GetMenuItemCount();
		for (UINT i= count; i > 0; --i)
			menu.DeleteMenu(i - 1, MF_BYPOSITION);
	}

	// limit amount of tags to show
	const int count= std::min<int>(int(tags.GetCount()), MAX_TAGS);

	if (count == 0)	// this menu cannot
		menu.AppendMenu(MF_STRING, first_id, _T("<无定义的标记>"));
	else
	{
		for (int i= 0; i < count; ++i)
		{
			int id= first_id + i;

			if (i < 9)
			{
				oStringstream ost;
				ost << _T('&') << i + 1 << _T(". ") << tags.Get(i).c_str();

				menu.AppendMenu(MF_STRING, id, ost.str().c_str());
			}
			else
				menu.AppendMenu(MF_STRING, id, tags.Get(i).c_str());
		}
	}
}


void TagsBarCommon::OnClearRating()
{
	StarClicked(0);
}


void TagsBarCommon::SetColors(const ApplicationColors& colors)
{
	rgb_background_ = colors[AppColors::Background];
	edit_background_ = colors[AppColors::EditBox];
	checks_wnd_.SetBackgndColors(colors[AppColors::Background], RGB(255, 0, 0));// colors[AppColors::Selection]);
	checks_wnd_.SetTextColors(colors[AppColors::Text], colors[AppColors::SelectedText], colors[AppColors::DisabledText]);

	if (rating_wnd_)
		rating_wnd_.SetColors(colors[AppColors::DimText], rgb_background_);

	if (status_wnd_)
		status_wnd_.SetColors(colors[AppColors::DimText], rgb_background_);

	if (stars_)
		stars_.SetBackgndColor(rgb_background_);

	if (m_hWnd)
		Invalidate();

	if (tool_bar_wnd_.m_hWnd)
		tool_bar_wnd_.Invalidate();

	if (filter_bar_.m_hWnd)
		filter_bar_.SetColors(edit_background_, edit_background_, colors[AppColors::Text]);
}


void TagsBarCommon::UseParentBackground(bool enable)
{
	use_parent_background_ = enable;
	checks_wnd_.UseParentBackground(enable);
}


LRESULT TagsBarCommon::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		DrawParentBkgnd(*this, *dc);

	return 0;
}


void TagsBarCommon::OnChangeFilter()
{
	try
	{
		filter_text_ = FilterText();

		checks_wnd_.FilterItems();

		UpdateFilterButton();
	}
	CATCH_ALL
}


bool TagsBarCommon::IsApplyBtnEnabled()
{
	return tags_enabled_ && !filter_text_.empty() && checks_wnd_.HasVisibleItems();
}


void TagsBarCommon::UpdateFilterButton()
{
	if (filter_bar_.m_hWnd)
	{
		filter_bar_.EnableButton(IsApplyBtnEnabled(), 0);
		filter_bar_.EnableButton(!filter_text_.empty(), 1);
	}
}


String TagsBarCommon::FilterText()
{
	CString str;
	filter_bar_.GetEditCtrl().GetWindowText(str);
	return String(str);
}


void TagsBarCommon::OnApplyTags()
{
	// apply all visible tags

	if (!tags_enabled_ || apply_tag_to_photos_.empty() || !IsApplyBtnEnabled())
		return;

	String tags;
	bool apply= false;

	const int count= checks_wnd_.GetItemCount();
	for (int i= 0; i < count; ++i)
		if (checks_wnd_.IsVisible(i) && !checks_wnd_.IsGroup(i))
		{
			const TagBtn& btn= tags_.at(checks_wnd_.GetUserParam(i));
			tags += btn.name_ + L'\n';

			if (btn.state_ != TagBtn::SELECTED)
				apply = true;
		}

	if (!tags.empty())
		apply_tag_to_photos_(selected_photos_, tags, apply, GetParent());
}


void TagsBarCommon::OnFilterCmd(bool enter)
{
	if (enter)
		OnApplyTags();
	else
		OnCancelFilter();
}


void TagsBarCommon::OnTagFilter()
{
	if (filter_bar_.m_hWnd)
		filter_bar_.GetEditCtrl().SetFocus();
}

void TagsBarCommon::OnCancelFilter()
{
	if (filter_bar_.m_hWnd)
		filter_bar_.GetEditCtrl().SetWindowText(L"");
}


void TagsBarCommon::SetFocus()
{
	if (filter_bar_.GetEditCtrl().m_hWnd)
		filter_bar_.GetEditCtrl().SetFocus();
	else
		SetFocus();
}


CWnd& TagsBarCommon::GetFilterEditBox()
{
	return filter_bar_.GetEditCtrl();
}
