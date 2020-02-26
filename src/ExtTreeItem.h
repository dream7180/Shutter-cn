/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtTreeItem.h: interface for the ExtTreeItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTTREEITEM_H__A39029DB_3811_11D5_8E84_00B0D078DE24__INCLUDED_)
#define AFX_EXTTREEITEM_H__A39029DB_3811_11D5_8E84_00B0D078DE24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "intrusive_ptr.h"


class ExtTreeItem : public mik::counter_base
{
public:
	ExtTreeItem();
	virtual ~ExtTreeItem();

	// this fn is called by tree item when it's about to display text in given column
	virtual void GetDisplayText(int column, CString& rstrBuffer, bool& bold_font) const;

	// this fn is called by the tree item to determine it's font: normal or bold
	virtual bool IsBold() const;

	// this fn is called by the tree item to determine it's inactive
	virtual bool IsActive() const;

	// return an index of image to be displayed next to the item's label
	virtual int GetImageIndex() const;

	// helper fn: int -> string
	void IntToStr(int value, CString& rstrBuffer) const;

	HTREEITEM GetHandle() const			{ return item_; }

	void SetHandle(HTREEITEM item)		{ item_ = item; }

private:
	friend class ExtTreeCtrl;

	// tree ctrl item handle (to simplify finding tree items from ExtTreeItem pointers)
	HTREEITEM item_;
};


typedef mik::intrusive_ptr<ExtTreeItem> CExtTreeItemPtr;

#endif // !defined(AFX_EXTTREEITEM_H__A39029DB_3811_11D5_8E84_00B0D078DE24__INCLUDED_)
