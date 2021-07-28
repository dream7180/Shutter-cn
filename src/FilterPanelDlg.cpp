/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FilterPanelDlg.cpp : implementation file

#include "stdafx.h"
#include "resource.h"
#include "FilterPanelDlg.h"
#include "DlgAutoResize.h"
#include "DlgListCtrl.h"
#include "FilterDialogs.h"
#include "TagsCommonCode.h"
#include "LoadImageList.h"
#include "ToolbarWnd.h"
#include <boost/function.hpp>
#include "Block.h"
#include "BalloonMsg.h"


struct FilterPanelDlg::Impl
{
	Impl()
	{
		operations_ = 0;
		self_ = 0;
		block_notifications_ = false;

		dialogs_[0] = &dlg_1_;		// tags
		dialogs_[1] = &dlg_4_;		// rating (stars)
		dialogs_[2] = &dlg_2_;		// text (anywhere in the metadata)
		dialogs_[3] = &dlg_3_;		// advanced filter expression (script)
		dialogs_[4] = 0;
	}

	void InitDlg(DialogBase* self);
	void SubFilterChanged(FilterDialog* dlg);
	//void UpdateStoreButton();

	DialogBase* self_;
	DlgAutoResize resize_map_;
	DlgListCtrl list_;
	//CEdit filter_name_;
	CFilterDialog_1 dlg_1_;
	CFilterDialog_2 dlg_2_;
	CFilterDialog_4 dlg_4_;
	CFilterDialog_3 dlg_3_;
	FilterDialog* dialogs_[5];
	FilterOperations* operations_;
	CImageList funnel_;
	//ToolBarWnd toolbar_;
	bool block_notifications_;
};


FilterPanelDlg::FilterPanelDlg() : impl_(new Impl), DialogBase(FilterPanelDlg::IDD, 0)
{
	::LoadPngImageList(impl_->funnel_, IDB_FUNNEL, ::GetSysColor(COLOR_3DFACE), true, 1);
}


FilterPanelDlg::~FilterPanelDlg()
{
}


void FilterPanelDlg::DoDataExchange(CDataExchange* DX)
{
	DialogBase::DoDataExchange(DX);
}

enum { ADD_FILTER= 1111, DELETE_FILTER, UPDATE_FILTER };


BEGIN_MESSAGE_MAP(FilterPanelDlg, DialogBase)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//ON_COMMAND_RANGE(ADD_FILTER, UPDATE_FILTER, OnFilterCommand)
	//ON_EN_CHANGE(IDC_FILTER_NAME, OnFilterNameChanged)
END_MESSAGE_MAP()


// FilterPanelDlg message handlers


bool FilterPanelDlg::Create(CWnd* parent, PhotoTagsCollection& tags, FilterOperations* operations)
{
	if (!DialogBase::Create(IDD, parent))
		return false;

	impl_->dlg_1_.SetTags(tags);

	for (int i= 0; impl_->dialogs_[i]; ++i)
	{
		FilterDialog* dlg= impl_->dialogs_[i];
		dlg->FilterChanged = boost::bind(&Impl::SubFilterChanged, impl_.get(), _1);
	}

	impl_->operations_ = operations;

	return true;
}


BOOL FilterPanelDlg::OnInitDialog()
{
	DialogBase::OnInitDialog();

	impl_->InitDlg(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void FilterPanelDlg::Impl::InitDlg(DialogBase* self)
{
	self_ = self;

	list_.SubclassDlgItem(IDC_DLG_LIST, self);
	const int MARGIN= 4;
	list_.SetLeftMargin(MARGIN);
	list_.SetRightMargin(MARGIN);
	list_.SetTopMargin(MARGIN);
	list_.SetBottomMargin(MARGIN);
	list_.SetImageList(&funnel_);

	//filter_name_.SubclassDlgItem(IDC_FILTER_NAME, self);

	// limit name length to make tabs widths reasonable
	//filter_name_.LimitText(50);

	resize_map_.BuildMap(self);
	resize_map_.SetWndResizing(IDC_DLG_LIST, DlgAutoResize::RESIZE);
	resize_map_.SetWndResizing(IDC_SEPARATOR, DlgAutoResize::MOVE_V_RESIZE_H);
	//resize_map_.SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_V);
	//resize_map_.SetWndResizing(IDC_FILTER_NAME, DlgAutoResize::MOVE_V_RESIZE_H);
	//resize_map_.SetWndResizing(IDC_TOOLBAR, DlgAutoResize::MOVE_V);

	//resize_map_.SetWndResizing(IDC_DEL_FILTER, DlgAutoResize::MOVE_V, DlgAutoResize::HALF_MOVE_H);
	//resize_map_.SetWndResizing(IDC_UPDATE_FILTER, DlgAutoResize::MOVE_V, DlgAutoResize::HALF_MOVE_H);
	//resize_map_.SetWndResizing(IDC_ADD_FILTER, DlgAutoResize::MOVE_V, DlgAutoResize::HALF_MOVE_H);

	const int no_image= -1;
	for (int i= 0; dialogs_[i]; ++i)
		list_.AddSubDialog(dialogs_[i], no_image, 0, true);

	/*toolbar_.SubclassDlgItem(IDC_TOOLBAR, self);
	int cmd[]= { ADD_FILTER, DELETE_FILTER, UPDATE_FILTER };
	toolbar_.SetPadding(DEFAULT_TB_PAD_DX, DEFAULT_TB_PAD_DY);
	toolbar_.AddButtons("PPP", cmd, IDB_CUSTOM_FILTERS, IDS_CUSTOM_FILTERS);
	toolbar_.SetOnIdleUpdateState(false);
	toolbar_.HideButton(DELETE_FILTER);
	toolbar_.HideButton(UPDATE_FILTER);*/
}

/*
void FilterPanelDlg::OnFilterNameChanged()
{
	impl_->UpdateStoreButton();

	if (impl_->operations_)
		impl_->operations_->NameChanged();
}


void FilterPanelDlg::Impl::UpdateStoreButton()
{
	if (toolbar_.m_hWnd && filter_name_.m_hWnd)
	{
		toolbar_.EnableButton(ADD_FILTER, filter_name_.GetWindowTextLength() > 0);
		toolbar_.EnableButton(UPDATE_FILTER, filter_name_.GetWindowTextLength() > 0);
	}
}


void FilterPanelDlg::OnFilterCommand(UINT cmd)
{
	if (impl_->operations_ == 0)
		return;

	switch (cmd)
	{
	case ADD_FILTER:
		impl_->operations_->StoreFilter();
		break;
	case DELETE_FILTER:
		impl_->operations_->DeleteFilter();
		break;
	case UPDATE_FILTER:
		impl_->operations_->UpdateFilter();
		break;
	}
}
*/

void FilterPanelDlg::Impl::SubFilterChanged(FilterDialog* dlg)
{
	if (block_notifications_)
		return;

	list_.SetSubdialogImage(dlg, dlg->IsFilterActive() ? 0 : 1);

	if (operations_)
		operations_->FilterParamsChanged();
}


void FilterPanelDlg::GetCurrentFilter(FilterData& filter) const
{
	impl_->dlg_1_.GetCurrentSelection(Tags::GetTagCollection(), filter.selected_tags_);

	impl_->dlg_2_.GetText(filter.text_.include, filter.text_.exclude);

	filter.expression_.rule = impl_->dlg_3_.GetRule();

	filter.stars_.stars = impl_->dlg_4_.GetStars();

	//filter.name_ = GetFilterName();
}


void FilterPanelDlg::SetFilter(const FilterData& filter, bool custom)
{
	Block in_update(impl_->block_notifications_);

	impl_->dlg_1_.SetSelection(Tags::GetTagCollection(), filter.selected_tags_);

	impl_->dlg_2_.SetText(filter.text_.include.c_str(), filter.text_.exclude.c_str());

	impl_->dlg_3_.SetRule(filter.expression_.rule);

	impl_->dlg_4_.SetStars(filter.stars_.stars);

	//impl_->filter_name_.SetWindowText(filter.name_.c_str());
	//impl_->UpdateStoreButton();

	// refresh funnel icons and 'clear' buttons
	for (int i= 0; impl_->dialogs_[i]; ++i)
	{
		FilterDialog* dlg= impl_->dialogs_[i];
		bool active= dlg->IsFilterActive();

		impl_->list_.SetSubdialogImage(dlg, active ? 0 : 1);

		dlg->ShowClearAllBtn(active);
	}

	//impl_->toolbar_.HideButton(ADD_FILTER, custom);
	//impl_->toolbar_.HideButton(DELETE_FILTER, !custom);
	//impl_->toolbar_.HideButton(UPDATE_FILTER, !custom);
}


void FilterPanelDlg::OnOK()	{}
void FilterPanelDlg::OnCancel()	{}


void FilterPanelDlg::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	impl_->resize_map_.Resize();
}


BOOL FilterPanelDlg::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	if (rect.IsRectEmpty())
		return true;

	DialogBase::OnEraseBkgnd(dc);
	//dc->FillSolidRect(rect.left, rect.top, rect.Width() - 1, rect.Height(), ::GetSysColor(COLOR_3DFACE));
	//dc->FillSolidRect(rect.right - 1, rect.top, 1, rect.Height(), ::GetSysColor(COLOR_3DFACE));
	dc->FillSolidRect(rect.left, rect.top, rect.Width(), rect.Height(), ::GetSysColor(COLOR_3DFACE));

	return true;
}

/*
String FilterPanelDlg::GetFilterName() const
{
	CString name;
	impl_->filter_name_.GetWindowText(name);
	return String(name);
}


void FilterPanelDlg::ShowNameInUseErr()
{
	new BalloonMsg(&impl_->filter_name_, _T("过滤器名称使用中"),
		_T("请提供唯一的过滤器名称."), BalloonMsg::IERROR);
}
*/

void FilterPanelDlg::GetPanelUISettings(size_t panel, int& height, bool& expanded, int& flags)
{
	if (panel < array_count(impl_->dialogs_) - 1)
	{
		expanded = impl_->list_.IsSubDialogExpanded(panel);
		height = impl_->list_.GetSubdialogHeight(panel);
		flags = impl_->dialogs_[panel]->GetFlags();
	}
}


void FilterPanelDlg::SetPanelUISettings(size_t panel, int height, bool expanded, int flags)
{
	if (panel < array_count(impl_->dialogs_) - 1)
	{
		impl_->list_.ExpandSubDialog(panel, expanded);

		if (impl_->dialogs_[panel]->IsResizable() && height > 0)
		{
			impl_->dialogs_[panel]->ResizePane(height);
		}

		impl_->dialogs_[panel]->SetFlags(flags);
	}
}


size_t FilterPanelDlg::GetPanelCount() const
{
	return array_count(impl_->dialogs_) - 1;
}
