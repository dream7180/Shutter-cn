/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FilterDialogs.cpp : implementation file
//

#include "stdafx.h"
#include "FilterDialogs.h"
#include "PhotoTags.h"
#include "TagsCommonCode.h"
#include "DlgListCtrl.h"
#include "Block.h"
#include "EditFilterDlg.h"
#include "HeaderDialog.h"
#include "PhotoInfoJPEG.h"
#include "CatchAll.h"
#include "SamplePhoto.h"
#include <boost/algorithm/string/replace.hpp>
#include "FilterRules.h"
extern void ManageTagsCollection(CWnd* parent, PhotoTagsCollection& tag_collection);

using namespace boost::algorithm;

FilterDialog::FilterDialog(int id, CWnd* parent) : DialogBase(id, parent)
{
	min_height_ = 0;
}

FilterDialog::~FilterDialog()
{}


BEGIN_MESSAGE_MAP(FilterDialog, DialogBase)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CANCEL, &FilterDialog::OnClear)
END_MESSAGE_MAP()


void FilterDialog::OnSize(UINT type, int cx, int cy)
{
	DialogBase::OnSize(type, cx, cy);

	resize_map_.Resize();
}


BOOL FilterDialog::OnInitDialog()
{
	DialogBase::OnInitDialog();

	resize_map_.BuildMap(this);

	CRect rect(0,0,0,0);
	GetClientRect(rect);
	min_height_ = rect.Height();

	HWND h= 0;
	GetDlgItem(IDC_RESIZE, &h);
	if (h)
	{
		resize_.SubclassDlgItem(IDC_RESIZE, this);
		resize_map_.SetWndResizing(IDC_RESIZE, DlgAutoResize::MOVE_V_RESIZE_H);
		resize_.SetCallbacks(this);
	}

	h = 0;
	GetDlgItem(IDC_CANCEL, &h);
	if (h)
	{
		cancel_.SubclassDlgItem(IDC_CANCEL, this);
		cancel_.SetOnIdleUpdateState(false);
		int cmd= IDC_CANCEL;
		int strings= 0;
		if (cancel_.GetStyle() & TBSTYLE_LIST)
		{
			cancel_.SetPadding(DEFAULT_TB_PAD_DX, DEFAULT_TB_PAD_DY);
			strings = IDS_CANCEL;
		}
		cancel_.AddButtons(strings ? "P" : "p", &cmd, IDB_CANCEL_FILTER, strings);
		cancel_.HideButton(IDC_CANCEL);

		resize_map_.SetWndResizing(IDC_CANCEL, DlgAutoResize::MOVE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void FilterDialog::OnOK() {}

void FilterDialog::OnCancel() {}


int FilterDialog::GetPaneHeight()
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	return rect.Height();
}


void FilterDialog::ResizePane(int height)
{
	if (height < min_height_)
		height = min_height_;

	int max_height= 10000;
	if (height > max_height)
		height = max_height;

	if (DlgListCtrl* parent= dynamic_cast<DlgListCtrl*>(GetParent()))
		parent->SetSubdialogHeight(this, height);
}


void FilterDialog::OnClear()
{
	ClearFilter();
	cancel_.HideButton(IDC_CANCEL);
}


void FilterDialog::ShowClearAllBtn(bool show)
{
	cancel_.HideButton(IDC_CANCEL, !show);
}


void FilterDialog::NotifyFilterChanged()
{
	ShowClearAllBtn(IsFilterActive());

	if (FilterChanged)
		FilterChanged(this);
}


bool FilterDialog::IsResizable() const
{
	return resize_.m_hWnd != 0;		// if dlg has resizing bar, it must be resizable
}

int FilterDialog::GetFlags() const
{
	return 0;
}

void FilterDialog::SetFlags(int flags)
{}


// CFilterDialog_1 dialog =============================================================================================
// filter by tags

CFilterDialog_1::CFilterDialog_1(CWnd* parent) : FilterDialog(IDD, parent)
{
	include_ = exclude_ = 0;
	collection_ = 0;
	handle_clicks_ = false;
}


void CFilterDialog_1::DoDataExchange(CDataExchange* DX)
{
	FilterDialog::DoDataExchange(DX);
}

const int MSG_CHANGE= WM_APP + 10;

BEGIN_MESSAGE_MAP(CFilterDialog_1, FilterDialog)
	ON_NOTIFY(TVN_ITEMCHANGED, IDC_TREE, &CFilterDialog_1::OnItemChanged)	// this notification works only in Vista and above
	ON_NOTIFY(NM_CLICK, IDC_TREE, &CFilterDialog_1::OnItemClicked)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE, &CFilterDialog_1::OnItemKeyDown)
	ON_BN_CLICKED(IDC_ANY, &CFilterDialog_1::OnAny)
	ON_BN_CLICKED(IDC_ALL, &CFilterDialog_1::OnAll)
	ON_COMMAND(IDC_EDIT_TAGS, &CFilterDialog_1::OnEditTags)
	ON_MESSAGE(MSG_CHANGE, OnChange)
END_MESSAGE_MAP()


void RemoveCheckBox(CTreeCtrl& tree, HTREEITEM item)
{
	tree.SetItemState(item, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
}


BOOL CFilterDialog_1::OnInitDialog()
{
	FilterDialog::OnInitDialog();

	resize_map_.SetWndResizing(IDC_TREE, DlgAutoResize::RESIZE);
	resize_map_.SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_V);
	resize_map_.SetWndResizing(IDC_ANY, DlgAutoResize::MOVE_V);
	resize_map_.SetWndResizing(IDC_ALL, DlgAutoResize::MOVE_V);
	resize_map_.SetWndResizing(IDC_TAGS_IN_USE, DlgAutoResize::MOVE_V);
	resize_map_.SetWndResizing(IDC_EDIT_TAGS, DlgAutoResize::MOVE_V);

	min_height_ /= 2;

	edit_tags_.SubclassDlgItem(IDC_EDIT_TAGS, this);
	int cmd= IDC_EDIT_TAGS;
	edit_tags_.SetPadding(DEFAULT_TB_PAD_DX, DEFAULT_TB_PAD_DY);
	edit_tags_.AddButtons("P", &cmd, IDB_EDIT, IDS_EDIT_TAGS);
	edit_tags_.SetOnIdleUpdateState(false);

	tags_.SubclassDlgItem(IDC_TREE, this);

	// turn check boxes on programmatically to trigger internal init function
	tags_.ModifyStyle(0, TVS_CHECKBOXES);

	tags_.SetIndent(10);

	CheckRadioButton(IDC_ALL, IDC_ANY, IDC_ALL);

	DWORD ver= ::GetVersion();
	int major_version = LOBYTE(LOWORD(ver));
	//int minor_version = HIBYTE(LOWORD(ver));
	if ((ver & DWORD(0x80000000)) != 0 || major_version < 6)
	{
		// no item change notifications available below Vista...
		// use mouse clicks instead
		handle_clicks_ = true;
	}

	ready_ = true;

	return true;
}


void CFilterDialog_1::OnEditTags()
{
	if (collection_)
		ManageTagsCollection(this, *collection_);
}


void CFilterDialog_1::SetTags(PhotoTagsCollection& tags)
{
	collection_ = &tags;
	on_change_ = tags.ConnectOnChange(boost::bind(&CFilterDialog_1::TagsCollectionChanged, this));
	PopulateTree(tags);
}


void CFilterDialog_1::TagsCollectionChanged()
{
	if (collection_)
		PopulateTree(*collection_);
}


void UpdateNames(CTreeCtrl& tree, HTREEITEM node, const PhotoTagsCollection& tags, bool any)
{
	if (node == 0)
		return;

	bool changed= false;
	HTREEITEM leaf= tree.GetNextItem(node, TVGN_CHILD);

	CString prefix= !any ? _T("和 ") : _T("或 ");
	size_t index= 0;

	while (leaf && index < tags.GetCount())
	{
		CString tag= tags.Get(index).c_str();
		if (index > 0)
			tag = prefix + tag;

		tree.SetItemText(leaf, tag);

		leaf = tree.GetNextItem(leaf, TVGN_NEXT);
		index++;
	}
}


void CFilterDialog_1::UpdateTree(const PhotoTagsCollection& tags)
{
	if (include_ == 0 || exclude_ == 0)
	{
		PopulateTree(tags);
		return;
	}

	bool any= !IsDlgButtonChecked(IDC_ALL);
	UpdateNames(tags_, include_, tags, any);
	UpdateNames(tags_, exclude_, tags, any);
}


void CFilterDialog_1::PopulateTree(const PhotoTagsCollection& tags)
{
	bool incl_expanded= include_ ? (tags_.GetItemState(include_, TVIS_EXPANDED) & TVIS_EXPANDED) != 0 : true;
	bool excl_expanded= exclude_ ? (tags_.GetItemState(exclude_, TVIS_EXPANDED) & TVIS_EXPANDED) != 0 : false;

	tags_.DeleteAllItems();
	tree_map_.clear();

	exclude_ = include_ = 0;

	include_ = tags_.InsertItem(_T("查找标签"), -1, -1);
	exclude_ = tags_.InsertItem(_T("排除标签"), -1, -1);

	RemoveCheckBox(tags_, include_);
	RemoveCheckBox(tags_, exclude_);

	CString prefix= IsDlgButtonChecked(IDC_ALL) ? _T("和 ") : _T("或 ");

	const size_t count= tags.GetCount();
	for (size_t i= 0; i < count; ++i)
	{
		CString tag= tags.Get(i).c_str();
		if (i > 0)
			tag = prefix + tag;
		HTREEITEM inc= tags_.InsertItem(TVIF_TEXT | TVIF_PARAM, tag, 0, 0, 0, 0, i, include_, TVI_LAST);
		HTREEITEM exc= tags_.InsertItem(TVIF_TEXT | TVIF_PARAM, tag, 0, 0, 0, 0, i, exclude_, TVI_LAST);

		tree_map_[tags.Get(i)] = TreeItems(inc, exc);
	}

	if (incl_expanded)
		tags_.Expand(include_, TVE_EXPAND);
	if (excl_expanded)
		tags_.Expand(exclude_, TVE_EXPAND);
}


bool ClearCheckBoxes(CTreeCtrl& tree, HTREEITEM item)
{
	if (item == 0)
		return false;

	bool changed= false;
	HTREEITEM leaf= tree.GetNextItem(item, TVGN_CHILD);

	while (leaf)
	{
		if (tree.GetCheck(leaf))
			changed = true;

		tree.SetCheck(leaf, false);
		leaf = tree.GetNextItem(leaf, TVGN_NEXT);
	}

	return changed;
}


void CFilterDialog_1::OnAny()
{
	if (ready_ && collection_)
	{
		UpdateTree(*collection_);

		NotifyFilterChanged();
	}
}


void CFilterDialog_1::OnAll()
{
	if (ready_ && collection_)
	{
		UpdateTree(*collection_);

		NotifyFilterChanged();
	}
}


LRESULT CFilterDialog_1::OnChange(WPARAM, LPARAM)
{
	NotifyFilterChanged();

	return 0;
}


void CFilterDialog_1::ClearFilter()
{
	if (ClearCheckBoxes(tags_, include_) || ClearCheckBoxes(tags_, exclude_))
		NotifyFilterChanged();
}


void CFilterDialog_1::OnItemKeyDown(NMHDR* nmhdr, LRESULT* result)
{
	if (handle_clicks_)
		PostMessage(MSG_CHANGE);	// maybe checkbox has changed

	*result = 0;
}

void CFilterDialog_1::OnItemClicked(NMHDR* nmhdr, LRESULT* result)
{
	if (handle_clicks_)
		PostMessage(MSG_CHANGE);	// maybe checkbox has changed

	*result = 0;
}


void CFilterDialog_1::OnItemChanged(NMHDR* nmhdr, LRESULT* result)
{
	NMTVITEMCHANGE* change= reinterpret_cast<NMTVITEMCHANGE*>(nmhdr);

	bool checkbox= false;
	bool is_checked= false;

	if ((change->uStateOld & TVIS_STATEIMAGEMASK) != (change->uStateNew & TVIS_STATEIMAGEMASK))
	{
		// check box state modified

		if (ready_)
		{
			HTREEITEM parent= tags_.GetParentItem(change->hItem);

			if (parent == 0)	// root?
			{
				RemoveCheckBox(tags_, change->hItem);
			}
			else if (include_)
			{
//				checkbox = true;

				//bool include= tags_.GetParentItem(change->item) == include_;
				//size_t index= tags_.GetItemData(change->item);

				NotifyFilterChanged();
			}
		}
	}
/*
	if ((change->state_old & TVIS_SELECTED) != (change->state_new & TVIS_SELECTED))
	{
		// toggle checkbox when selection changes
		if (!checkbox && tags_.m_hWnd)
			tags_.SetCheck(change->item, !tags_.GetCheck(change->item));
	}
*/
	*result = 0;
}


void CollectCheckedTags(CTreeCtrl& tree, HTREEITEM node, const PhotoTagsCollection& collection, std::vector<String>& tags)
{
	if (node == 0)
		return;

	HTREEITEM leaf= tree.GetNextItem(node, TVGN_CHILD);

	size_t index= 0;

	while (leaf)
	{
		if (tree.GetCheck(leaf) && index < collection.GetCount())
			tags.push_back(collection.Get(index));

		leaf = tree.GetNextItem(leaf, TVGN_NEXT);
		index++;
	}

	ASSERT(collection.GetCount() == index);
}


void CFilterDialog_1::GetCurrentSelection(const PhotoTagsCollection& tags, FilterTags& selection)
{
	selection.include.clear();
	selection.exclude.clear();
	selection.match_all = true;

	if (!ready_)
		return;

	CollectCheckedTags(tags_, include_, tags, selection.include);
	CollectCheckedTags(tags_, exclude_, tags, selection.exclude);
	selection.match_all = !!IsDlgButtonChecked(IDC_ALL);
}


void CFilterDialog_1::CheckTags(CTreeCtrl& tree, const FilterTags& selection)
{
	{
		const size_t count= selection.include.size();
		for (size_t i= 0; i < count; ++i)
		{
			TreeMap::iterator it= tree_map_.find(selection.include[i]);
			if (it != tree_map_.end())
				tree.SetCheck(it->second.include);
		}
	}
	{
		const size_t count= selection.exclude.size();
		for (size_t i= 0; i < count; ++i)
		{
			TreeMap::iterator it= tree_map_.find(selection.exclude[i]);
			if (it != tree_map_.end())
				tree.SetCheck(it->second.exclude);
		}
	}
}


void CFilterDialog_1::SetSelection(const PhotoTagsCollection& tags, const FilterTags& selection)
{
	if (!ready_)
		return;

	ClearCheckBoxes(tags_, include_);
	ClearCheckBoxes(tags_, exclude_);
	CheckTags(tags_, selection);

	CheckRadioButton(IDC_ALL, IDC_ANY, selection.match_all ? IDC_ALL : IDC_ANY);
}


bool IsAnyChecked(const CTreeCtrl& tree, HTREEITEM node)
{
	if (node == 0)
		return false;

	HTREEITEM leaf= tree.GetNextItem(node, TVGN_CHILD);

	while (leaf)
	{
		if (tree.GetCheck(leaf))
			return true;

		leaf = tree.GetNextItem(leaf, TVGN_NEXT);
	}

	return false;
}


bool CFilterDialog_1::IsFilterActive() const
{
	if (!ready_)
		return false;

	return IsAnyChecked(tags_, include_) || IsAnyChecked(tags_, exclude_);
}


const int VALID_FLAGS= 0x100;


int CFilterDialog_1::GetFlags() const
{
	if (!ready_)
		return 0;

	bool incl_expanded= include_ ? (tags_.GetItemState(include_, TVIS_EXPANDED) & TVIS_EXPANDED) != 0 : true;
	bool excl_expanded= exclude_ ? (tags_.GetItemState(exclude_, TVIS_EXPANDED) & TVIS_EXPANDED) != 0 : false;

	return (incl_expanded ? 1 : 0) | (excl_expanded ? 2 : 0) | VALID_FLAGS;
}


void CFilterDialog_1::SetFlags(int flags)
{
	if (!ready_ || (flags & VALID_FLAGS) == 0)
		return;

	if (flags & 1)
		tags_.Expand(include_, TVE_EXPAND);
	if (flags & 2)
		tags_.Expand(exclude_, TVE_EXPAND);
}



// CFilterDialog_2 dialog =============================================================================================
// filter by text

CFilterDialog_2::CFilterDialog_2(CWnd* parent) : FilterDialog(IDD, parent)
{
	update_blocked_ = true;
}


void CFilterDialog_2::DoDataExchange(CDataExchange* DX)
{
	FilterDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(CFilterDialog_2, FilterDialog)
	ON_EN_CHANGE(IDC_INCLUDE_TEXT, OnTextChanged)
	ON_EN_CHANGE(IDC_EXCLUDE_TEXT, OnTextChanged)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CFilterDialog_2::OnInitDialog()
{
	FilterDialog::OnInitDialog();

	resize_map_.SetWndResizing(IDC_INCLUDE_TEXT, DlgAutoResize::RESIZE_H);
	resize_map_.SetWndResizing(IDC_EXCLUDE_TEXT, DlgAutoResize::RESIZE_H);

	include_.SubclassDlgItem(IDC_INCLUDE_TEXT, this);
	exclude_.SubclassDlgItem(IDC_EXCLUDE_TEXT, this);

	const int limit= 1000;
	include_.LimitText(limit);
	exclude_.LimitText(limit);

	update_blocked_ = false;

	return true;
}


void CFilterDialog_2::SetText(const TCHAR* include, const TCHAR* exclude)
{
	Block update(update_blocked_);

	include_.SetWindowText(include);
	exclude_.SetWindowText(exclude);
}


void CFilterDialog_2::GetText(String& include, String& exclude) const
{
	{
		CString text;
		include_.GetWindowText(text);
		include = text;
	}
	{
		CString text;
		exclude_.GetWindowText(text);
		exclude = text;
	}
}


void CFilterDialog_2::OnTextChanged()
{
	if (update_blocked_)
		return;

	SetTimer(TIMER_ID, 500, 0);	// 0.5 s
}


bool CFilterDialog_2::IsFilterActive() const
{
	return include_.GetWindowTextLength() > 0 || exclude_.GetWindowTextLength() > 0;
}


void CFilterDialog_2::OnTimer(UINT_PTR event)
{
	DialogBase::OnTimer(event);

	if (event == TIMER_ID)
	{
		KillTimer(event);

		NotifyFilterChanged();
	}
}


void CFilterDialog_2::ClearFilter()
{
	if (IsFilterActive())
	{
		SetText(_T(""), _T(""));

		NotifyFilterChanged();
	}
}


// CFilterDialog_3 dialog =============================================================================================
// filter by advanced script expression

CFilterDialog_3::CFilterDialog_3(CWnd* parent) : FilterDialog(IDD, parent)
{}


void CFilterDialog_3::DoDataExchange(CDataExchange* DX)
{
	FilterDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(CFilterDialog_3, FilterDialog)
	ON_NOTIFY(NM_CLICK, IDC_FILTER, &CFilterDialog_3::OnItemClicked)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILTER, &CFilterDialog_3::OnItemChanged)
	ON_BN_CLICKED(IDC_DEFINE, &CFilterDialog_3::OnEditRule)
END_MESSAGE_MAP()


BOOL CFilterDialog_3::OnInitDialog()
{
	FilterDialog::OnInitDialog();

	edit_rule_.SubclassDlgItem(IDC_DEFINE, this);
	int cmd= IDC_DEFINE;
	edit_rule_.SetPadding(DEFAULT_TB_PAD_DX, DEFAULT_TB_PAD_DY);
	edit_rule_.AddButtons("P", &cmd, IDB_EDIT, IDS_EDIT_RULE);
	edit_rule_.SetOnIdleUpdateState(false);

	resize_map_.SetWndResizing(IDC_FILTER, DlgAutoResize::RESIZE);
	resize_map_.SetWndResizing(IDC_DEFINE, DlgAutoResize::MOVE_V);

	min_height_ = min_height_ * 3 / 4;
	filterrule_.SubclassDlgItem(IDC_FILTER, this);
	filterrule_.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	CRect rect;
	GetDlgItem(IDC_FILTER)->GetClientRect(&rect);
	filterrule_.InsertColumn(0, _T("Filter"), LVCFMT_LEFT, rect.Width());
	/*LVITEM li;
		memset(&li, 0, sizeof li);

		li.mask			= LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
		li.iItem		= 0;
		li.iSubItem		= 0;
		li.state		= (0 == 0 ? LVIS_SELECTED | LVIS_FOCUSED : 0); // doesn't work -> | INDEXTOSTATEIMAGEMASK(scan_[i] ? 2 : 1);
		li.stateMask	= LVIS_SELECTED | LVIS_FOCUSED; // | LVIS_STATEIMAGEMASK;
		li.pszText		= LPSTR_TEXTCALLBACK;
		li.cchTextMax	= 0;
		li.iImage		= 0; //I_IMAGECALLBACK;
		li.lParam		= 0;
		li.iIndent		= 0;
	filterrule_.InsertItem(&li);//LoadFilterRules(AdvFilter::GetFiltersPathName().c_str()));
*/
	LoadFilterRules(AdvFilter::GetFiltersPathName().c_str());
	for (int i= 0; i < filter_rules.GetSize(); ++i){
		filterrule_.InsertItem(i,filter_rules[i]);
	}
	//filterrule_.InsertItem(0, LoadFilterRules(AdvFilter::GetFiltersPathName().c_str()).c_str());
	return true;
}


bool CFilterDialog_3::IsFilterActive() const
{
	return !rule_.empty()&&(rule_!=_T("x = 1 if x > 0\r\nthen return true\r\nend"));
}


String CFilterDialog_3::GetRule()// const
{
	rule_.clear();
	for (int i= 0; i < filter_rules.GetSize(); ++i){
		if(filterrule_.GetCheck(i))
		{
			rule_ += _T("and ");
			rule_ += String(filter_rules[i]);
			rule_ += _T("\n");
		}
	}
	return _T("x = 1 if x > 0\r\n") + rule_ + _T("then return true\r\nend");
}


void CFilterDialog_3::SetRule(const String& rule)
{
	UpdateRule(rule);
}


void CFilterDialog_3::ClearFilter()
{
	if (IsFilterActive())
	{
		for (int i= 0; i < filter_rules.GetSize(); ++i){
			filterrule_.SetCheck(i, false);
		}
		UpdateRule(String());
		NotifyFilterChanged();
	}
}


void CFilterDialog_3::UpdateRule(const String& rule)
{
	rule_ = rule;
	//SetDlgItemText(IDC_FILTER, rule.c_str());
}

void CFilterDialog_3::LoadFilterRules(const TCHAR* filename)
{
	//Path path(GetFiltersPathName().c_str());
	Path path(filename);
	if (!path.FileExists())
		return;

	CFile file(filename, CFile::modeRead);

	ULONGLONG len= file.GetLength();
	//if (len > 0x500000)
	//{
	//	if (parent)
	//		new BalloonMsg(parent, _T("标签文件太大"), _T("尝试载入的文件过大.\n文件应不大于 5 MB."), BalloonMsg::IERROR);
//
	//	return false;
	//}

	DWORD length= static_cast<DWORD>(len);

	std::vector<TCHAR> buf(1 + length / sizeof TCHAR);

	//TODO: handle non-Unicode

	file.Read(&buf.front(), length);

	// skip Unicode signature
	if (length > 2 && buf.front() == 0xfeff)		// Unicode marker?
		buf.erase(buf.begin(), buf.begin() + 1);

	replace_all(buf, _T("\xd\xa"), _T("\xa"));
	//return String(&buf.front());
	String _rules = String(&buf.front());
	iStringstream si(_rules);
	String filter_rule;
	filter_rules.RemoveAll();
	for (;;)
	{
		if (!getline(si, filter_rule, _T('\n')))
			break;
		trim_if(filter_rule, is_from_range(_T('\0'), _T('\x20')));
		if(filter_rule.length() != 0)
			filter_rules.Add(filter_rule.c_str());
	}
	//return arr;
}


void CFilterDialog_3::OnEditRule()
{
//	PhotoInfoPtr photo= exif_view_wnd_.CurrentItem();
//	if (photo == 0)
//		return;
	mik::intrusive_ptr<PhotoInfo> photo= CreateSamplePhotoInfo();

	try
	{
		EditFilterDlg dlg(this, *photo, Tags::GetTagCollection());
		HeaderDialog frame(dlg, _T("TTL"), GetParent());

		//if (!rule_.empty())
		//	dlg.filter_rule_ = rule_;
		String dlg_rules;
		for (int i= 0; i < filter_rules.GetSize(); ++i){
			dlg_rules += filter_rules[i];
			dlg_rules += _T("\n");
		}
		//SetDlgItemText(IDC_FILTER, dlg_rules);
		dlg.filter_rule_ = dlg_rules;
		if (frame.DoModal() != IDOK)
			return;
		filter_rules.RemoveAll();
		filterrule_.DeleteAllItems();
		LoadFilterRules(AdvFilter::GetFiltersPathName().c_str());
		for (int i= 0; i < filter_rules.GetSize(); ++i){
			filterrule_.InsertItem(i,filter_rules[i]);
		}
		UpdateRule(GetRule());
		NotifyFilterChanged();
	}
	CATCH_ALL_W(this);
}

void CFilterDialog_3::OnItemClicked(NMHDR* nmhdr, LRESULT* result)
{
	DWORD dwPos = GetMessagePos();  
	CPoint point( LOWORD(dwPos), HIWORD(dwPos) );
	filterrule_.ScreenToClient(&point);     
	LVHITTESTINFO lvinfo;  
	lvinfo.pt = point;
	int nItem = filterrule_.HitTest(&lvinfo);
	if(nItem != -1)  
	//m_itemSel = lvinfo.iItem;
	if(lvinfo.flags == LVHT_ONITEMSTATEICON)  
		m_bHit = true;  
	*result = 0;
}

void CFilterDialog_3::OnItemChanged(NMHDR* nmhdr, LRESULT* result)
{
	//NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)nmhdr;
	LPNMLISTVIEW pNMListView = reinterpret_cast<LPNMLISTVIEW>(nmhdr);
	if(m_bHit){
		m_bHit = false;
		//if(filterrule_.GetCheck(m_itemSel)){
		UpdateRule(GetRule());
		NotifyFilterChanged();
		//}
	}

/*	if(pNMListView->uChanged==LVIF_STATE || m_bHit)
	{
		if(pNMListView->uNewState & LVIS_SELECTED)
		{
			//GetRule();
			UpdateRule(GetRule());
			NotifyFilterChanged();
		}
	}*/
	*result = 0;
}

// CFilterDialog_4 dialog (filter by rating/stars) ====================================================================

CFilterDialog_4::CFilterDialog_4(CWnd* parent) : FilterDialog(IDD, parent)
{}


void CFilterDialog_4::DoDataExchange(CDataExchange* DX)
{
	FilterDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(CFilterDialog_4, FilterDialog)
END_MESSAGE_MAP()


BOOL CFilterDialog_4::OnInitDialog()
{
	FilterDialog::OnInitDialog();

	resize_map_.SetWndResizing(IDC_CANCEL, DlgAutoResize::MOVE_H);

	stars_.SubclassDlgItem(IDC_STARS, this);
	label_.SubclassDlgItem(IDC_LABEL, this);

	stars_.SetClickCallback(boost::bind(&CFilterDialog_4::OnStarsClicked, this, _1));

	SetStars(0);

	return true;
}


void CFilterDialog_4::SetStars(int stars)
{
	stars_.SetRating(stars);

	CString label;
	//if (stars == 1)
	//	label = _T("显示至少 1 星的图像");
	//else if (stars > 0)
	if (stars > 0)
		label.Format(_T("显示至少 %d 星的图像"), stars);
	else
		label = _T("显示全部图像");

	label_.SetWindowText(label);
}


int CFilterDialog_4::GetStars() const
{
	return stars_.GetRating();
}


bool CFilterDialog_4::IsFilterActive() const
{
	return GetStars() > 0;
}


void CFilterDialog_4::ClearFilter()
{
	if (IsFilterActive())
	{
		SetStars(0);

		NotifyFilterChanged();
	}
}


void CFilterDialog_4::OnStarsClicked(int stars)
{
	static bool in_update= false;

	if (in_update)
		return;

	Block update(in_update);

	if (GetStars() == stars)
	{
		stars--;
		if (stars < 0)
			stars = 0;
	}
	SetStars(stars);

	NotifyFilterChanged();
}