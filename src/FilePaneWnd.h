/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>
#include "FileFilterCallback.h"
#include "FileInfo.h"


// FilePaneWnd dialog

class FilePaneWnd : public CDialog
{
public:
	FilePaneWnd(CWnd* parent = NULL);   // standard constructor
	virtual ~FilePaneWnd();

	bool Create(CWnd* parent, const TCHAR* label, bool showNew, bool showMask, bool includeFiles);

	void SetPathsRegistrySection(const TCHAR* section);

	void SetPath(const TCHAR* path);

	// files to show (set the filter callback)
	void SetFileMask(const FileFilterCallback& includeFileItem, const String& btnMaskText);

	// register callback to select types of files shown
	void SetFileTypeSelect(const boost::function<bool (CPoint pos)>& selectFileTypes);

	// register callback to create a new folder
	void SetNewFolderCmd(const boost::function<void ()>& createNewFolderDlg);

	// minimal width for this control window
	int GetMinWidth() const;

	// current path (if validation is on, balloon msg may be displayed)
	CString GetPath(bool validate);

	// return all file names (paths) currently in view
	void GetFiles(std::vector<FileInfo>& files);

	// add recent path (does not have to be unique)
	void AddRecentPath(const TCHAR* path);

	// store recent paths in registry
	void StoreRecentPaths();

	// refresh
	void Refresh();

protected:
	DECLARE_MESSAGE_MAP()

private:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	void OnSize(UINT, int cx, int cy);
	BOOL OnEraseBkgnd(CDC* dc);
	int OnCreate(CREATESTRUCT* cs);
	HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnAddressKillFocus();
	void OnAddressSetFocus();
	void OnGoLevelUp();
	void OnListView();
	void OnIconView();
	void OnNewFolder();
	LRESULT OnUpdateFolders(WPARAM, LPARAM);

	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};
