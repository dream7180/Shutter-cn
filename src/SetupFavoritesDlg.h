/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_SETUPFAVORITESDLG_H__4AC46B18_9805_455D_BE78_D5AD3037E0C1__INCLUDED_)
#define AFX_SETUPFAVORITESDLG_H__4AC46B18_9805_455D_BE78_D5AD3037E0C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetupFavoritesDlg.h : header file
//
class FavoriteFolders;
class FavoriteFolder;
#include "MultiMonitor.h"
#include "DlgAutoResize.h"


/////////////////////////////////////////////////////////////////////////////
// SetupFavoritesDlg dialog

class SetupFavoritesDlg : public CDialog
{
// Construction
public:
	SetupFavoritesDlg(FavoriteFolders& folders, CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SetupFavoritesDlg)
	enum { IDD = IDD_FAVORITES };
	CListCtrl	list_wnd_;
	//}}AFX_DATA

	static const int NAME_MAX_LENGTH= 200;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SetupFavoritesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SetupFavoritesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnSetFolder();
	afx_msg void OnDblClkList(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnDelete();
	afx_msg void OnClickList(NMHDR* nmhdr, LRESULT* result);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnEndLabelEdit(NMHDR* nmhdr, LRESULT* result);
	void OnBeginLabelEdit(NMHDR* nmhdr, LRESULT* result);
	void OnDestroy();
	void OnGetMinMaxInfo(MINMAXINFO* MMI);
	void OnSize(UINT type, int cx, int cy);
	void OnEndTrack(UINT, NMHDR* nmhdr, LRESULT* result);
	void OnRename();

	FavoriteFolders& folders_;
	FavoriteFolder* GetSelectedItem();
	int GetSelectedItemIndex();
	void SelectFolder(FavoriteFolder& folder);

	WindowPosition wnd_pos_;
	CSize min_size_;
	DlgAutoResize dlg_resize_map_;

private:
	FavoriteFolder* GetFolder(LPARAM lParam)
	{
		return reinterpret_cast<FavoriteFolder*>(lParam);
	}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETUPFAVORITESDLG_H__4AC46B18_9805_455D_BE78_D5AD3037E0C1__INCLUDED_)
