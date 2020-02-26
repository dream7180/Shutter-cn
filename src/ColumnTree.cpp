/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ColumnTree.cpp: implementation of the ColumnTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ColumnTree.h"
#include "Columns.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ColumnTree::ColumnTree(Columns& columns) : column_defs_(columns), columns_(0)
{
	changed_ = false;
}


void ColumnTree::InitTree()
{
	tree_wnd_.ModifyStyle(0, TVS_CHECKBOXES);

	item_common_		= tree_wnd_.InsertItem(_T("Generic"), -1, -1);
	item_canon_			= tree_wnd_.InsertItem(_T("Canon"), -1, -1);
	item_nikon_			= tree_wnd_.InsertItem(_T("Nikon"), -1, -1);
	item_fuji_			= tree_wnd_.InsertItem(_T("Fuji"), -1, -1);
	item_olympus_		= tree_wnd_.InsertItem(_T("Olympus"), -1, -1);
	metadata_columns_	= tree_wnd_.InsertItem(_T("File Info"), -1, -1);
	custom_columns_		= tree_wnd_.InsertItem(_T("Custom Columns"), -1, -1);

	RemoveCheckBox(item_common_);
	RemoveCheckBox(item_canon_);
	RemoveCheckBox(item_nikon_);
	RemoveCheckBox(item_fuji_);
	RemoveCheckBox(item_olympus_);
	RemoveCheckBox(metadata_columns_);
	RemoveCheckBox(custom_columns_);

	struct { Columns::Set set; HTREEITEM node; } groups[]=
	{
		{ Columns::COMMON, item_common_ },
		{ Columns::CANON, item_canon_ },
		{ Columns::NIKON, item_nikon_ },
		{ Columns::FUJI, item_fuji_ },
		{ Columns::OLYMPUS, item_olympus_ },
		{ Columns::METADATA, metadata_columns_ },
		{ Columns::CUSTOM, custom_columns_ }
	};

	for (int n= 0; n < array_count(groups); ++n)
	{
		int index= column_defs_.GetStart(groups[n].set);

		for (int i= 0; i < column_defs_.GetCount(groups[n].set); ++i, ++index)
			leaves_[index] = tree_wnd_.InsertItem(column_defs_.Name(index), -1, -1, groups[n].node);
	}

	if (columns_)
		for (int i= 0; i < columns_->size(); ++i)
			SetCheck((*columns_)[i], true);

	tree_wnd_.Expand(item_common_, TVE_EXPAND);
}


void ColumnTree::Reset(HTREEITEM skip_this)
{
	// place check next to the first 16 columns
	for (Map::const_iterator it= leaves_.begin(); it != leaves_.end(); ++it)
		tree_wnd_.SetCheck(it->second, it->first < 16);

	tree_wnd_.Expand(item_common_, TVE_EXPAND);

	if (skip_this != item_canon_)		tree_wnd_.Expand(item_canon_, TVE_COLLAPSE);
	if (skip_this != item_nikon_)		tree_wnd_.Expand(item_nikon_, TVE_COLLAPSE);
	if (skip_this != item_fuji_)		tree_wnd_.Expand(item_fuji_, TVE_COLLAPSE);
	if (skip_this != item_olympus_)		tree_wnd_.Expand(item_olympus_, TVE_COLLAPSE);
	if (skip_this != metadata_columns_)	tree_wnd_.Expand(metadata_columns_, TVE_COLLAPSE);
	if (skip_this != custom_columns_)	tree_wnd_.Expand(custom_columns_, TVE_COLLAPSE);

	if (skip_this != 0)
		tree_wnd_.Expand(skip_this, TVE_EXPAND);
}


void ColumnTree::SetCheck(int index, bool check)
{
	if (leaves_.count(index) == 0)
	{
		ASSERT(false);
		return;
	}
	tree_wnd_.SetCheck(leaves_[index], check);
}


void ColumnTree::RemoveCheckBox(HTREEITEM item)
{
	tree_wnd_.SetItemState(item, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
}


bool ColumnTree::ColumnsChanged() const
{
	if (columns_ == 0)
		return false;

	if (columns_->size() != selected_.size())
		return true;

	for (int i= 0; i < selected_.size(); ++i)
		if (selected_[i] != (*columns_)[i])
			return true;

	return false;
}


void ColumnTree::TreeDoDataExchange(CDataExchange* DX, UINT id)
{
	DDX_Control(DX, id, tree_wnd_);

	if (columns_ == 0)
		return;

	if (DX->m_bSaveAndValidate)
	{
		selected_.clear();
		selected_.reserve(10);

		// store selected columns

		for (Map::const_iterator it= leaves_.begin(); it != leaves_.end(); ++it)
			if (tree_wnd_.GetCheck(it->second) != 0)
				selected_.push_back(it->first);

		changed_ = ColumnsChanged();
	}
}
