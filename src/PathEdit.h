/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// CPathEdit

class CPathEdit : public CEdit
{
public:
	CPathEdit();
	virtual ~CPathEdit();

	void FileNameEditing(bool file);

	void InitAutoComplete(bool on);

	void SetIllegalChars(const TCHAR* illegal);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnChar(UINT chr, UINT rep_cnt, UINT flags);
	LRESULT OnPaste(WPARAM, LPARAM);
	virtual void PreSubclassWindow();

	bool file_name_;
	bool autoComplete_;
	CString illegalChars_;
};
