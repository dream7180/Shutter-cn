/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// EditFld.h: interface for the CEditFld class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITFLD_H__AB501280_A95F_41ED_8532_376F7D99803E__INCLUDED_)
#define AFX_EDITFLD_H__AB501280_A95F_41ED_8532_376F7D99803E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CPropField;


class CEditFld : public CWnd
{
public:
	CEditFld(CPropField* field);
	virtual ~CEditFld();

	bool Create(DWORD styles, const CRect& rect, CWnd* parent, int id);

	void EnterEdit();

private:
	//{{AFX_MSG(CEditFld)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditKillFocus();
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	AutoPtr<CEdit> edit_;
	DWORD edit_styles_;

	static String class_;
	static HFONT font_;
	CPropField* field_;
	String original_val_;
};

#endif // !defined(AFX_EDITFLD_H__AB501280_A95F_41ED_8532_376F7D99803E__INCLUDED_)
