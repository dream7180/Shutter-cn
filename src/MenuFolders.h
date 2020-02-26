/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MenuFolders.h: interface for the CMenuFolders class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUFOLDERS_H__5657F6D1_1DE4_4001_BBD9_B4C960475F17__INCLUDED_)
#define AFX_MENUFOLDERS_H__5657F6D1_1DE4_4001_BBD9_B4C960475F17__INCLUDED_
//#include "RMenu.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class ItemIdList;


class CMenuFolders : public CMenu
{
public:
	CMenuFolders();
	CMenuFolders(HMENU popup);
	virtual ~CMenuFolders();

	virtual void DrawItem(LPDRAWITEMSTRUCT draw_item_struct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT measure_item_struct);

	bool InsertItem(const ItemIdList& idlFolder, UINT id, const TCHAR* text);
	bool InsertItem(UINT id, const TCHAR* text);

private:
	HIMAGELIST images_;
	CSize label_size_;
	CSize shcut_size_;
	std::vector<const ItemIdList*> items_;
	CSize img_size_;
	int tab_stop_;
	void CalcStringLengths(CString text);
	bool own_menu_;
	void Init(HMENU popup);
};

#endif // !defined(AFX_MENUFOLDERS_H__5657F6D1_1DE4_4001_BBD9_B4C960475F17__INCLUDED_)
