/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PathEdit.cpp : implementation file
//

#include "stdafx.h"
#include "PathEdit.h"
#include "AutoComplete.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CPathEdit

CPathEdit::CPathEdit()
{
	autoComplete_ = true;
	FileNameEditing(false);
}

CPathEdit::~CPathEdit()
{
}

void CPathEdit::InitAutoComplete(bool on)
{
	autoComplete_ = on;
}


BEGIN_MESSAGE_MAP(CPathEdit, CEdit)
	ON_WM_CHAR()
	ON_MESSAGE(WM_PASTE, OnPaste)
END_MESSAGE_MAP()


const static TCHAR* ILLEGAL_PATH_CHARS= _T("*?|><\"");
const static TCHAR* ILLEGAL_FILE_CHARS= _T("/\\:*?|><\"");

// CPathEdit message handlers


void CPathEdit::FileNameEditing(bool file)
{
	file_name_ = file;
	SetIllegalChars(file_name_ ? ILLEGAL_FILE_CHARS : ILLEGAL_PATH_CHARS);
}


void CPathEdit::SetIllegalChars(const TCHAR* illegal)
{
	illegalChars_ = illegal;
}


void CPathEdit::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	const TCHAR* illegal= illegalChars_;

	if (_tcschr(illegal, chr))
		return;

	CEdit::OnChar(chr, rep_cnt, flags);
}


LRESULT CPathEdit::OnPaste(WPARAM, LPARAM)
{
	LRESULT ret= Default();

	CString str;
	GetWindowText(str);

	const TCHAR* illegal= illegalChars_;

	int idx= str.FindOneOf(illegal);
	if (idx < 0)
		return ret;	// everything's fine, return

	// fix the string
	do
	{
		str.Delete(idx);

		idx = str.FindOneOf(illegal);

	} while (idx >= 0);

	SetWindowText(str);

	return ret;
}


void CPathEdit::PreSubclassWindow()
{
	LimitText(MAX_PATH);

	if (autoComplete_)
	{
		// add SHAutoComplete fn call if it's available
		AutoComplete::TurnOn(m_hWnd);
	}
}
