/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_ADDRESSBOX_H__03E1DE9A_8813_4093_9B85_EC38DAAAA62F__INCLUDED_)
#define AFX_ADDRESSBOX_H__03E1DE9A_8813_4093_9B85_EC38DAAAA62F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddressBox.h : header file
//
#include "EditCombo.h"
#include "FolderPath.h"
class ItemIdList;


/////////////////////////////////////////////////////////////////////////////
// AddressBox window

class AddressBox : public EditCombo
{
// Construction
public:
	AddressBox();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent);

	// set path to display
	void SetPath(FolderPathPtr path);

	// populate auto complete history
	void SetHistory(const CRecentFileList& paths);

	void SetScan(bool scan_in_progress);

	// events
//	typedef boost::signals2::signal<void (int cmd)> RunCommand;

	// connect handler to the event
//	slot_connection ConnectRunCommand(RunCommand::slot_function_type fn);

// Implementation
public:
	virtual ~AddressBox();

	// message map functions
protected:
	//afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//void OnSize(UINT type, int cx, int cy);
	//void OnTbButton();
	//void OnStopScan();
	//void OnSetFocus();
	//void OnKillFocus();
	//void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	//void OnTbGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	//void OnAutoCompleteDropDown();
	//void OnGetInfoTip(NMHDR* nmhdr, LRESULT* result);
	//void EndEdit(int key);

	//DECLARE_MESSAGE_MAP()

private:
//	CImageList image_list_;
//	EditEsc edit_box_;
//	ToolBarWnd toolbar_;
//	bool focus_;
	bool scan_in_progress_;
//	AutoCompletePopup auto_complete_;
//	RunCommand run_cmd_event_;

//	void Resize();
//	void Run(int cmd);
	void EndEdit(bool ok);
	void SetPath(const ItemIdList& idlPath);
	void SetPath(const TCHAR* path);
	void Selected();
};


#endif // !defined(AFX_ADDRESSBOX_H__03E1DE9A_8813_4093_9B85_EC38DAAAA62F__INCLUDED_)
