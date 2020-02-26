/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FilterBar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "FilterBar.h"
#include "BalloonMsg.h"
#include "BrowserFrame.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FilterBar

FilterBar::FilterBar()
{}

FilterBar::~FilterBar()
{}

static const TCHAR* REGISTRY_SECTION_FILTER= _T("Filter");
static const TCHAR* REG_FILTER_TEXT= _T("Text%02d");
static const int MAX_STORED_ENTRIES= 10;

/////////////////////////////////////////////////////////////////////////////
// FilterBar message handlers

bool FilterBar::Create(CWnd* parent)
{
	if (!EditCombo::Create(parent, IDB_FIND_TOOLBAR, ID_FILTER_PHOTOS, ID_CANCEL_FILTER, EditCombo::AUTO_COMPLETE))
		return false;

	SetMargins(CRect(0, 0, Pixels(4), 0));

	// read texts into history_

	CWinApp* app= AfxGetApp();

	for (int i= 0; i < MAX_STORED_ENTRIES; ++i)
	{
		CString key;
		key.Format(REG_FILTER_TEXT, i);

		CString text= app->GetProfileString(REGISTRY_SECTION_FILTER, key, _T(""));

		if (!text.IsEmpty())
			history_.push_back(String(text));
	}

	SetHistory(history_);

	ConnectFinishCommand(boost::bind(&FilterBar::EndEdit, this, _1));

	return true;
}


void FilterBar::OnDestroy()
{
	if (!history_.empty())
	{
		CWinApp* app= AfxGetApp();

		int count= static_cast<int>(history_.size());

		for (int i= 0; i < MAX_STORED_ENTRIES; ++i)
		{
			CString key;
			key.Format(REG_FILTER_TEXT, i);

			CString text;

			if (i < count)
				text = history_[i].c_str();
			else
				text.Empty();

			app->WriteProfileString(REGISTRY_SECTION_FILTER, key, text);
		}
	}

	CWnd::OnDestroy();
}


void FilterBar::Activate()
{
	// set focus
	GetEditCtrl().SetFocus();
}


void FilterBar::EndEdit(bool ok)
{
	Run(ok ? ID_FILTER_PHOTOS : ID_CANCEL_FILTER);
}


void FilterBar::FilterOn(bool on)
{
	SetState(on ? 1 : 0);
}
