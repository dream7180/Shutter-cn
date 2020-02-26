/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtTreeNode.cpp: implementation of the ExtTreeNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ICMProfile.h"
#include "ExtTreeNode.h"
#include "ColorProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ExtTreeNode::ExtTreeNode(ExtTreeNode* parent, ICMProfilePtr icm)
 : ExtTreeRow(parent, icm)
{}


ExtTreeNode::~ExtTreeNode()
{}


// add child (leaf) to this node
//
void ExtTreeNode::AddLeaf(ExtTreeRow* child)
{
	ASSERT(child != 0);
	children_.push_back(child);
}

#if 0
bool ExtTreeNode::ChangeSet(ExtTreeCtrl* tree_wnd, ItemSet set, bool notify/*= true*/)
{
	if (!ExtTreeRow::ChangeSet(tree_wnd, set, false))
		return false;

	int count= children_.size();

	for (int index= 0; index < count; ++index)
		children_[index]->ChangeSet(tree_wnd, set, false);

	if (notify)
		ExtTreeRow::ChangeSet(tree_wnd, set, true);

	return true;
}


UINT ExtTreeNode::GetCumulatedSet()
{
	UINT state= 0; //ExtTreeRow::GetCumulatedSet();

	int count= children_.size();

	if (count == 0)
	{
		// if there are no children return item's state
		state = current_set_;
	}
	else
	{
		for (int index= 0; index < count; ++index)
			state |= children_[index]->GetCumulatedSet();
	}

	return state;
}
#endif


// remove child (leaf) from this node
//
void ExtTreeNode::RemoveLeaf(ExtTreeRow* child)
{
	ASSERT(child != 0);
	size_t count= children_.size();

	for (size_t index= 0; index < count; ++index)
		if (children_[index].get() == child)
		{
			children_.erase(children_.begin() + index);
			return;
		}

	ASSERT(false);
}

// delete all leaves from this node
//
void ExtTreeNode::RemoveLeaves()
{
	children_.clear();
}

void ExtTreeNode::RemoveLeaves(CTreeCtrl& tree)
{
	size_t count= children_.size();

	for (size_t index= 0; index < count; ++index)
		tree.DeleteItem(children_[index]->GetHandle());

	RemoveLeaves();
}


int ExtTreeNode::GetImageIndex() const
{
	return 0;//expr_->HasSubExpr() ? 5 : 6;
}


bool ExtTreeNode::IsPopupMenuColumn(int column) const
{
	return false;
}


///////////////////////////////////////////////////////////////////////////////

CExtTreeRootNode::CExtTreeRootNode(const TCHAR* name, int image_index)
 : ExtTreeNode(0, 0), name_(name), image_index_(image_index)
{}

CExtTreeRootNode::~CExtTreeRootNode()
{}


void CExtTreeRootNode::GetColumn(int column_index, CString& buff) const
{
	switch (column_index)
	{
	case COL_ITEM_NAME:
		buff = name_.c_str();
		break;

	default:
		buff = _T("");
		break;
	}
}


int CExtTreeRootNode::GetImageIndex() const
{
	return image_index_;
}
