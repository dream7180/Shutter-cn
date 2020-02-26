/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtTreeRow.h: interface for the ExtTreeRow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTTREEROW_H__0CE852F5_44B1_11D5_8E89_00B0D078DE24__INCLUDED_)
#define AFX_EXTTREEROW_H__0CE852F5_44B1_11D5_8E89_00B0D078DE24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ExtTreeItem.h"
#include "ICMProfile.h"
class ExtTreeNode;


class ExtTreeRow : public ExtTreeItem
{
public:
	enum Flags { NONE, OUTPUT= 1, RENDERING_INTENT= 2 };
	ExtTreeRow(ExtTreeNode* Parent, ICMProfilePtr icm, UINT flags= RENDERING_INTENT);
	virtual ~ExtTreeRow();

	// get text data for a given column
	CString GetColumn(int column_index) const;
	virtual void GetColumn(int column_index, CString& buff) const;

	// image index
	virtual int GetImageIndex() const;

	virtual bool IsBold() const;

	void SetBold(bool bold)	{ bold_ = bold; }

	enum Columns { COL_ITEM_NAME, COL_PROFILE, COL_RENDERING, COL_ENABLE_FLAG };

	virtual void GetDisplayText(int column, CString& rstrBuffer, bool& bold_font) const;

	ExtTreeNode* GetParent()	{ return parent_; }

	ICMProfilePtr icm_;

	virtual void Draw(CDC* dc, std::vector<std::pair<int, int>> column_info, COLORREF rgb_text, COLORREF rgb_back, CRect rect);

	virtual bool IsPopupMenuColumn(int column) const;

	bool IsOutputProfile() const;

	bool IsMatchingProfile(ColorProfilePtr profile) const;

protected:
	bool bold_;
	bool output_profile_;
	bool rendering_intent_;

	enum { CHECKBOX_SIZE= 12 };

private:
	ExtTreeNode* parent_;	// parent node
	static CFont bold_fnt_;
};


#endif // !defined(AFX_EXTTREEROW_H__0CE852F5_44B1_11D5_8E89_00B0D078DE24__INCLUDED_)
