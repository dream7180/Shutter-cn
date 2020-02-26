/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PANE_H__71A20316_0BDC_40E7_9A99_A7AEDCB835BA__INCLUDED_)
#define AFX_PANE_H__71A20316_0BDC_40E7_9A99_A7AEDCB835BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Pane.h : header file
//
#include "PhotoInfo.h"
class OptionsDlg;
class SnapFrame;
#include "PaneNotification.h"
#include "VectPhotoInfo.h"

/////////////////////////////////////////////////////////////////////////////
// PaneWnd window

class PaneWnd : public CWnd
{
// Construction
public:
	PaneWnd();

// Attributes
	PhotoInfoPtr GetCurrentPhoto() const			{ return current_photo_; }

	bool IsPaneVisible() const					{ return visible_; }

	// big or small toolbar in a caption area?
	//bool IsCaptionBig() const;

// Operations
	void SyncVisibilityFlag();


// Notifications to be invoked by SendPaneNotification()

	// current item in main view has changed
	virtual void CurrentPhotoChanged(PhotoInfoPtr photo);

	// current photo selection has changed
	virtual void PhotoSelectionChanged(VectPhotoInfo& selected);

	// called after all panes have been created
	virtual void InitialUpdate();

	// called after options have changed
	virtual void OptionsChanged(OptionsDlg& dlg);

	// current photo has been modified (rotation or else)
	virtual void CurrentPhotoModified(PhotoInfoPtr photo);

	// photo's description has been changed
	virtual void PhotoDescriptionChanged(std::wstring& descr);

	// invoked by main frame when closing the app (on destroy)
	virtual void SaveSettings();

	// open viewer request
	virtual void OpenCurrentPhoto();

	// main window activated
	virtual void MainWndActivated();

	// sort order is about to be displayed and has to be updated
	virtual void UpdateSortOrderPopup(CMenu& popup);

	// requested new sort order (menu item selected)
	virtual void ChangeSortOrder(UINT cmd_id);

	// caption's height has changed
	//virtual void CaptionHeightChanged(bool big);

	// request to assign tag to selected photos
	virtual void AssignTag(int index);

	// update dirty windows; used before animation starts
	virtual void UpdatePane();

protected:
// Notifications received by pane windows

	// this is only invoked is pane is visible
	virtual void CurrentChanged(PhotoInfoPtr photo);

	// ditto
	virtual void SelectionHasChanged(VectPhotoInfo& selected);

	// called after pane is being closed
	virtual void PaneHidden();

	// this is only invoked is pane is visible
	virtual void CurrentModified(PhotoInfoPtr photo);

	// ditto
	virtual void DescriptionChanged(std::wstring& descr);

	// add window (usually toolbar) to the caption area
	void AddBand(CWnd* toolbar, CWnd* owner,
		std::pair<int, int> min_max_width= std::pair<int, int>(0, 0), bool resizable= false);

	void ResetBandsWidth(std::pair<int, int> min_max_width);
	void ResetBandsWidth();

	// notification from snap frame/view
	virtual void ActivatePane(bool active);

	// main wnd active
	virtual void ActiveMainWnd();

	void SetFaintCaptionEdge(bool faint);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PaneWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PaneWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(PaneWnd)
	//}}AFX_MSG
	void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	DECLARE_MESSAGE_MAP()

	template <class MemFn, class Arg, class Arg2> void SendNotification(MemFn fn, Arg& arg, Arg2& arg2)
	{
		if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
			SendPaneNotification(parent, fn, arg, arg2);
	}

	template <class MemFn, class Arg> void SendNotification(MemFn fn, Arg& arg)
	{
		if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
			SendPaneNotification(parent, fn, arg);
	}

	template <class MemFn> void SendNotification(MemFn fn)
	{
		if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
			SendPaneNotification(parent, fn);
	}

private:
	bool visible_;
	PhotoInfoPtr current_photo_;

	SnapFrame* GetSnapFrame() const;

	LRESULT OnShowPane(WPARAM visible, LPARAM notification);
	LRESULT OnActivatePane(WPARAM active, LPARAM);
	//LRESULT OnCaptionHeightChanged(WPARAM big, LPARAM);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PANE_H__71A20316_0BDC_40E7_9A99_A7AEDCB835BA__INCLUDED_)
