/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtTreeNode.h: interface for the ExtTreeNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTTREENODE_H__0CE852F6_44B1_11D5_8E89_00B0D078DE24__INCLUDED_)
#define AFX_EXTTREENODE_H__0CE852F6_44B1_11D5_8E89_00B0D078DE24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExtTreeRow.h"
class ExtTreeCtrl;


class ExtTreeNode : public ExtTreeRow
{
public:
	ExtTreeNode(ExtTreeNode* parent, ICMProfilePtr icm);
	virtual ~ExtTreeNode();

	// add child (leaf) to this node
	void AddLeaf(ExtTreeRow* child);

	// remove child (leaf) from this node
	void RemoveLeaf(ExtTreeRow* child);

	// delete all leaves from this node
	void RemoveLeaves();
	void RemoveLeaves(CTreeCtrl& tree);

	// return an index of image to be displayed next to the item label
	virtual int GetImageIndex() const;

	typedef std::vector<mik::intrusive_ptr<ExtTreeRow> > TreeRowVector;
	typedef TreeRowVector::iterator iterator;

	iterator begin()	{ return children_.begin(); }
	iterator end()		{ return children_.end(); }

	virtual bool IsPopupMenuColumn(int column) const;

private:
	TreeRowVector children_;
};

typedef mik::intrusive_ptr<ExtTreeNode> CExtTreeNodePtr;



class CExtTreeRootNode : public ExtTreeNode
{
public:
	CExtTreeRootNode(const TCHAR* name, int image_index);
	virtual ~CExtTreeRootNode();

	virtual void GetColumn(int column_index, CString& buff) const;

	// return an index of image to be displayed next to the item label
	virtual int GetImageIndex() const;

	String name_;
	int image_index_;
};


#endif // !defined(AFX_EXTTREENODE_H__0CE852F6_44B1_11D5_8E89_00B0D078DE24__INCLUDED_)
