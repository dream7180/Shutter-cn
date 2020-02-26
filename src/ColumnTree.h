/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ColumnTree.h: interface for the ColumnTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLUMNTREE_H__D12D6677_5757_4BA4_AC1D_3E312A181D71__INCLUDED_)
#define AFX_COLUMNTREE_H__D12D6677_5757_4BA4_AC1D_3E312A181D71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class Columns;


class ColumnTree
{
public:
	ColumnTree(Columns& columns);
//	virtual ~ColumnTree();

	const std::vector<uint16>* columns_;
	std::vector<uint16> selected_;
	bool changed_;

	CTreeCtrl tree_wnd_;

	void InitTree();
	void Reset(HTREEITEM skip_this);
	void SetCheck(int index, bool check);
	void RemoveCheckBox(HTREEITEM item);
	bool ColumnsChanged() const;
	void TreeDoDataExchange(CDataExchange* DX, UINT id);

protected:
//	vector<HTREEITEM> leaves_;
	typedef std::map<int, HTREEITEM> Map;
	Map leaves_;	// map column indices to tree items
	Columns& column_defs_;

	HTREEITEM item_common_;
	HTREEITEM item_canon_;
	HTREEITEM item_nikon_;
	HTREEITEM item_fuji_;
	HTREEITEM item_olympus_;
	HTREEITEM metadata_columns_;
	HTREEITEM custom_columns_;
};


#endif // !defined(AFX_COLUMNTREE_H__D12D6677_5757_4BA4_AC1D_3E312A181D71__INCLUDED_)
