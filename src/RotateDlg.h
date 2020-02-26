/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_ROTATEDLG_H__D4F7CA74_DCBE_4750_A2DB_91633C3BA4B0__INCLUDED_)
#define AFX_ROTATEDLG_H__D4F7CA74_DCBE_4750_A2DB_91633C3BA4B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RotateDlg.h : header file
//
#include "DialogChild.h"
#include "ImageCtrl.h"
#include "LineSeparatorCtrl.h"
#include "PrintPreviewWnd.h"
#include "Transform.h"
#include "Profile.h"

class PhotoPreview : public PrintEngine
{
public:
	PhotoPreview();
	virtual ~PhotoPreview();

	virtual int GetPageCount(int items_count) const;

	virtual CSize GetImageSize() const;

	void SetTransformation(RotationTransformation rotation, bool horz_flip, bool vert_flip);

protected:
	virtual bool PrintFn(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page);

private:
	Dib image_;
	RotationTransformation rotation_;
	bool mirror_;
};


/////////////////////////////////////////////////////////////////////////////
// CRotateDlg dialog

class CRotateDlg : public DialogChild
{
// Construction
public:
	CRotateDlg(VectPhotoInfo& selected, bool all, CWnd* parent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_ROTATE };
	bool all_;
	BOOL rotate_cw_;
	BOOL rotate_ccw_;
	BOOL rotate_180_;
	BOOL rotate_auto_;
	BOOL mirror_;

	RotationTransformation GetOperation() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRotateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRotateDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	virtual BOOL ContinueModal();
	DECLARE_MESSAGE_MAP()

private:
	CString original_ok_btn_text_;
	bool fix_rotated_photos_;
	CButton ok_button_;
	ImageCtrl images_[5];
	LineSeparatorCtrl separator_;
	PrintPreviewWnd preview_;
	PhotoPreview print_;
	bool update_;
	Profile<int> operation_;
	Profile<bool> horz_flip_;

	void UpdateCheckBoxes(UINT id);
	void UpdateExample();
	int GetOperationEx() const;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROTATEDLG_H__D4F7CA74_DCBE_4750_A2DB_91633C3BA4B0__INCLUDED_)
