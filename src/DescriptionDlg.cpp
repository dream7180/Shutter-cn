/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DescriptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DescriptionDlg.h"
#include "PhotoInfo.h"
#include "PhotoAttr.h"
#include "Config.h"
#include "CatchAll.h"
#include "EnableCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDescriptionDlg dialog


CDescriptionDlg::CDescriptionDlg(CWnd* parent, PhotoInfo& inf, VectPhotoInfo& photos,
								 VectPhotoInfo& selected, PhotoChangedFn fn/*= 0*/)
	: CDialogChild(CDescriptionDlg::IDD, parent), photos_(photos), selected_(selected), photo_changed_(fn)
{
	//{{AFX_DATA_INIT(CDescriptionDlg)
	//}}AFX_DATA_INIT
	unicode_ = ::GetVersion() < DWORD(0x80000000);
	edit_ = 0;
	modified_ = false;
	current_photo_index_ = -1;

	VectPhotoInfo::iterator it= find(photos_.begin(), photos_.end(), &inf);
	if (it == photos_.end())
	{
		ASSERT(false);
	}
	else
		current_photo_index_ = it - photos_.begin();

	multiple_ = selected_.size() > 1;
}


void CDescriptionDlg::DoDataExchange(CDataExchange* DX)
{
	CDialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CDescriptionDlg)
	DDX_Control(DX, IDC_TOOLBAR, tool_bar_wnd_);
	DDX_Control(DX, IDC_NAVIGATION, navigation_wnd_);
	DDX_Control(DX, IDC_INFO, infoText_);
	DDX_Control(DX, IDC_PROGRESS, progressCtrl_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDescriptionDlg, CDialogChild)
	//{{AFX_MSG_MAP(CDescriptionDlg)
	ON_COMMAND(ID_PREVIOUS, OnPrevious)
	ON_COMMAND(ID_NEXT, OnNext)
	ON_EN_CHANGE(IDC_DESCRIPTION, OnChangeDescription)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_INSERT_CHAR_00, ID_INSERT_CHAR_15, OnSymbol)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDescriptionDlg message handlers

BOOL CDescriptionDlg::OnInitDialog()
{
	try
	{
		return InitDlg();
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


BOOL CDescriptionDlg::InitDlg()
{
	if (current_photo_index_ == -1)
	{
		EndDialog(IDCANCEL);
		return true;
	}

	edit_ = ::GetDlgItem(*this, IDC_DESCRIPTION);
	if (edit_ == 0)
	{
		EndDialog(IDCANCEL);
		return true;
	}

	if (unicode_ && sizeof(TCHAR) == sizeof(char))
	{
		CRect rect;
		::GetWindowRect(edit_, rect);
		ScreenToClient(rect);
		DWORD dwStyle= ::GetWindowLong(edit_, GWL_STYLE);
		DWORD dwExStyle= ::GetWindowLong(edit_, GWL_EXSTYLE);
		::DestroyWindow(edit_);
		edit_ = ::CreateWindowExW(dwExStyle, L"EDIT", L"", dwStyle, rect.left, rect.top,
			rect.Width(), rect.Height(), *this, HMENU(IDC_DESCRIPTION), AfxGetInstanceHandle(), 0);

		if (edit_ == 0)
		{
			EndDialog(IDCANCEL);
			return true;
		}
	}

	LOGFONT lf= g_Settings.lf_description_;
	lf.lfHeight = -18;
	fnd_edit_.CreateFontIndirect(&lf);

	::SendMessageW(edit_, WM_SETFONT, reinterpret_cast<WPARAM>(fnd_edit_.GetSafeHandle()), 0);
	::SendMessageW(edit_, EM_SETLIMITTEXT, PhotoAttr::MAX_DESC_LEN, 0);

	GetWindowText(title_);

	CDialogChild::OnInitDialog();

	SubclassHelpBtn(_T("ToolDescription.htm"));

	BuildToolbar();

	static const int commands[]= { ID_PREVIOUS, ID_NEXT };
	navigation_wnd_.AddButtons("PP", commands, IDB_NAVIGATION, IDS_NAVIGATION);
	navigation_wnd_.SetDisabledImageList(IDB_NAVIGATION_DIS);
	UpdateButtons();

	if (multiple_)
	{
		EnableCtrl(&navigation_wnd_, false, true);
		oStringstream ost;
		ost << selected_.size() << _T(" 图像被选定.");
		infoText_.SetWindowText(ost.str().c_str());
		EnableCtrl(&infoText_, true, true);
	}

	if (!Load(current_photo_index_))
	{
		EndDialog(IDCANCEL);
		return true;
	}

	if (multiple_)
		modified_ = true;	// there is no checking if all images have the same description, so this is
							// required to make sure writing occures

	::SendMessage(m_hWnd, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(edit_), 1L);

	BuildResizingMap();

	SetWndResizing(IDC_DESCRIPTION, CDlgAutoResize::RESIZE);
	SetWndResizing(IDC_TOOLBAR, CDlgAutoResize::MOVE_H);
	SetWndResizing(IDC_HELP_BTN, CDlgAutoResize::MOVE_V);
	SetWndResizing(IDC_NAVIGATION, CDlgAutoResize::MOVE_V);
	SetWndResizing(IDC_INFO, CDlgAutoResize::MOVE_V);
	SetWndResizing(IDCANCEL, CDlgAutoResize::MOVE);
	SetWndResizing(IDOK, CDlgAutoResize::MOVE);

	return false;  // return TRUE unless you set the focus to a control
}


void CDescriptionDlg::OnOK()
{
	if (multiple_)
	{
		// write description to multiple images

		CWaitCursor wait;

		//TODO: worker thread

		infoText_.ShowWindow(SW_HIDE);
		progressCtrl_.ModifyStyle(0, PBS_SMOOTH);
		progressCtrl_.SetRange32(0, int(selected_.size() - 1));
		progressCtrl_.SetPos(0);
		progressCtrl_.ShowWindow(SW_SHOWNA);

		if (!Write(&selected_[0], selected_.size()))
		{
			progressCtrl_.ShowWindow(SW_HIDE);
			infoText_.ShowWindow(SW_SHOWNA);
			return;
		}
	}
	else if (!WriteDescription(current_photo_index_))
		return;

	EndDialog(IDOK);
}


void CDescriptionDlg::BuildToolbar()
{
	static const int commands[]=
	{
		ID_INSERT_CHAR_00, ID_INSERT_CHAR_01, ID_INSERT_CHAR_02, ID_INSERT_CHAR_03,
		ID_INSERT_CHAR_04, ID_INSERT_CHAR_05, ID_INSERT_CHAR_06, ID_INSERT_CHAR_07,
		ID_INSERT_CHAR_08, ID_INSERT_CHAR_09, ID_INSERT_CHAR_10, ID_INSERT_CHAR_11,
		ID_INSERT_CHAR_12, ID_INSERT_CHAR_13, ID_INSERT_CHAR_14, ID_INSERT_CHAR_15
	};

	tool_bar_wnd_.SetPadding(4, 5);
	tool_bar_wnd_.AddButtons(unicode_ ? "pppppppppppppppp" : "pppppppppp......", commands, IDB_CHARACTERS);
	tool_bar_wnd_.SetRows(array_count(commands) / 2, false, 0);
}


void CDescriptionDlg::OnSymbol(UINT cmd_id)
{
	cmd_id -= ID_INSERT_CHAR_00;

	ASSERT(cmd_id < 16);

	TCHAR pszText[2]= { 0, 0 };

#ifdef UNICODE
	static const TCHAR symbols[]= _T("\xBD\xBC\xA9\xAE\xB1\xB0\xB2\xB3\xBB\xAB\x2018\x2019\x201C\x201D\x201A\x201E");
#else
	static const TCHAR symbols[]= _T("\xBD\xBC\xA9\xAE\xB1\xB0\xB2\xB3\xBB\xAB\x2e\x2e\x2e\x2e\x2e\x2e");
#endif

	pszText[0] = symbols[cmd_id];

	::SendMessage(edit_, EM_REPLACESEL, true, LPARAM(pszText));
}


void CDescriptionDlg::OnPrevious()
{
	if (current_photo_index_ > 0)
	{
		if (!WriteDescription(current_photo_index_))
			return;

		if (Load(current_photo_index_ - 1))
		{
			--current_photo_index_;
			UpdateButtons();
		}
	}
}

void CDescriptionDlg::UpdateButtons()
{
	navigation_wnd_.EnableButton(ID_PREVIOUS, current_photo_index_ > 0);
	navigation_wnd_.EnableButton(ID_NEXT, current_photo_index_ < photos_.size() - 1);
}


void CDescriptionDlg::OnNext()
{
	if (current_photo_index_ < photos_.size() - 1)
	{
		if (!WriteDescription(current_photo_index_))
			return;

		if (Load(current_photo_index_ + 1))
		{
			++current_photo_index_;
			UpdateButtons();
		}
	}
}


void CDescriptionDlg::SetDescriptionText(PhotoInfo* photo)
{
	if (photo)
	{
		if (unicode_)
		{
			::SetWindowTextW(edit_, photo->photo_desc_.c_str());
		}
		else
		{
			string desc;
			photo->DescriptionAscii(desc);
			::SetWindowTextA(edit_, desc.c_str());
		}
	}

	modified_ = false;
}


bool CDescriptionDlg::WriteDescription(int index)
{
	if (!modified_)
		return true;

	if (index < 0 || index >= photos_.size())
	{
		ASSERT(false);
		return false;
	}

	bool ok= Write(&photos_[index], 1);

	modified_ = false;

	return ok;
}


void CDescriptionDlg::OnChangeDescription()
{
	modified_ = true;
}


bool CDescriptionDlg::Write(PhotoInfo* photo[], const size_t count)
{
	try
	{
		bool all_written= true;

		wstring desc;

		if (!unicode_)
		{
			vector<char> buff;
			buff.resize(::GetWindowTextLengthA(edit_) + 1);
			::GetWindowTextA(edit_, &buff.front(), buff.size());

			vector<wchar_t> wide;
			wide.resize(buff.size() + 32);
			int len= ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, &buff.front(), buff.size() - 1,
				&wide.front(), wide.size());
			desc.assign(&wide.front(), len);
		}
		else
		{
			vector<wchar_t> buff;
			buff.resize(::GetWindowTextLengthW(edit_) + 1);
			::GetWindowTextW(edit_, &buff.front(), buff.size());
			desc.assign(&buff.front(), buff.size() - 1);
		}

		for (size_t i= 0; i < count; ++i)
		{
			progressCtrl_.SetPos(int(i));
			progressCtrl_.UpdateWindow();

			if (photo[i] == 0 || !photo[i]->IsDescriptionEditable())
				all_written = false;
			else if (!photo[i]->WriteDescription(desc, this))
				all_written = false;
		}

		return all_written;
	}
	CATCH_ALL

	return false;
}


bool CDescriptionDlg::Load(int index)
{
	if (index < 0 || index >= photos_.size())
		return false;

	PhotoInfo* photo= photos_[index];

	if (photo == 0)
		return false;

	if (!photo->IsDescriptionEditable())
		return false;

	if (CWnd* wnd= GetParent())
	{
		CString title= title_ + _T(": ") + photo->name_.c_str();
		wnd->SetWindowText(title);
	}

	SetDescriptionText(photo);

	ShowImage(photo);

	if (!multiple_ && photo_changed_)
		photo_changed_(*photo);

	return true;
}
