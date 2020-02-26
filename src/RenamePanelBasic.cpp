/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RenamePanelBasic.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "RenamePanelBasic.h"
#include "CatchAll.h"
#include "PhotoInfo.h"
#include "adaptCString.h"

extern void RenameSymbolsPopupMenu(CPoint pos, CWnd* parent, CEdit& pattern_edit_box);
extern CString ParsePattern(const TCHAR* filename, DateTime date, int serial, const TCHAR* pattern);

const TCHAR* g_symbol_info_text= _T("%Y\tYear (4 digit)\r\n%y\tYear (2 digit)\r\n%m\tMonth\r\n%d\tDay\r\n%H\tHours\r\n%M\tMinutes\r\n%S\tSeconds\r\n%f\tOriginal file name\r\n%f{from}\tPartial file name\r\n%f{from:length}\tPartial file name\r\n%n\tConsecutive numbers\r\n%n{digits}\tConsecutive numbers (1 to 9 digits)\r\n%n{digits:start}\tConsecutive numbers");

/*
template<>
struct boost::range_const_iterator<CString>
{
	typedef const TCHAR* type;
};

template<>
struct boost::range_iterator<CString>
{
	typedef TCHAR* type;
};

template<>
struct boost::range_size<CString>
{
public:
	typedef int type;
};

template<>
struct boost::range_size<CString const>
{
public:
	typedef int type;
};


inline boost::range_iterator<CString>::type begin(CString& str)
{
	return str.GetBuffer(0);
}

inline boost::range_const_iterator<CString>::type begin(CString const& str)
{
	return str;
}

inline boost::range_size<CString>::type size(CString const& str)
{
	return str.GetLength();
}

inline boost::range_iterator<CString>::type end(CString& str)
{
	return begin(str) + size(str);
}

inline boost::range_const_iterator<CString>::type end(CString const& str)
{
	return begin(str) + size(str);
}

inline boost::range_iterator<CString>::type range_begin(CString& str)
{
        return begin(str);
}

inline boost::range_iterator<CString const>::type range_begin(CString const& str)
{
        return begin(str);
}

inline boost::range_iterator<CString>::type range_end(CString& str)
{
        return end(str);
}

inline boost::range_iterator<CString const>::type range_end(CString const& str)
{
        return end(str);
}
*/

// RenamePanelBasic dialog

RenamePanelBasic::RenamePanelBasic() : RenamePage(RenamePanelBasic::IDD),
	trim_left_(0), trim_right_(0), letter_case_(0), replace_case_sensitive_(false)
{
	starting_number_ = 1;
	extension_case_ = 1;
}


RenamePanelBasic::~RenamePanelBasic()
{}


void RenamePanelBasic::DoDataExchange(CDataExchange* dx)
{
	ImgPage::DoDataExchange(dx);
	DDX_Control(dx, IDC_PATTERN, pattern_edit_);
	DDX_Control(dx, IDC_START, number_);
	DDX_Text(dx, IDC_PATTERN, pattern_);
	DDX_Text(dx, IDC_START, starting_number_);
	DDX_Control(dx, IDC_START, number_);
	DDX_Control(dx, IDC_TRIM_LEFT, trim_left_edit_);
	DDX_Control(dx, IDC_TRIM_RIGHT, trim_right_edit_);
	DDX_Text(dx, IDC_TRIM_LEFT, trim_left_);
	DDX_Text(dx, IDC_TRIM_RIGHT, trim_right_);
	DDX_Check(dx, IDC_CASE_SENS_REPLACE, replace_case_sensitive_);
	DDX_CBIndex(dx, IDC_CASE, letter_case_);
	DDX_Text(dx, IDC_SEARCH, search_text_);
	DDX_Text(dx, IDC_REPLACE, replace_text_);
	DDX_Control(dx, IDC_SEARCH, search_edit_);
	DDX_Control(dx, IDC_REPLACE, replace_edit_);
	DDX_Radio(dx, IDC_EXTENSION_U, extension_case_);
}


BEGIN_MESSAGE_MAP(RenamePanelBasic, ImgPage)
	ON_EN_CHANGE(IDC_PATTERN, &RenamePanelBasic::OnPatternChange)
	ON_EN_CHANGE(IDC_START, &RenamePanelBasic::OnParamValueChange)
	ON_EN_CHANGE(IDC_TRIM_LEFT, &RenamePanelBasic::OnParamValueChange)
	ON_EN_CHANGE(IDC_TRIM_RIGHT, &RenamePanelBasic::OnParamValueChange)
	ON_EN_CHANGE(IDC_SEARCH, &RenamePanelBasic::OnSearchTextChange)
	ON_EN_CHANGE(IDC_REPLACE, &RenamePanelBasic::OnSearchTextChange)
	ON_NOTIFY(TBN_DROPDOWN, IDC_SYMBOLS, &RenamePanelBasic::OnTbDropDown)
	ON_BN_CLICKED(IDC_CASE_SENS_REPLACE, &RenamePanelBasic::OnSearchTextChange)
	ON_CBN_SELCHANGE(IDC_CASE, &RenamePanelBasic::OnParamValueChange)
	ON_BN_CLICKED(IDC_EXTENSION_U, &RenamePanelBasic::OnParamValueChange)
	ON_BN_CLICKED(IDC_EXTENSION_L, &RenamePanelBasic::OnParamValueChange)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//IDC_PATTERN
// RenamePanelBasic message handlers

BOOL RenamePanelBasic::OnInitDialog()
{
	try
	{
		// search/replace boxes don't need auto files system auto-completion
		search_edit_.InitAutoComplete(false);
		replace_edit_.InitAutoComplete(false);
		pattern_edit_.InitAutoComplete(false);

		ImgPage::OnInitDialog();

		symbols_tb_.SubclassDlgItem(IDC_SYMBOLS, this);
		int cmdId[]= { IDC_SYMBOLS };
		symbols_tb_.SetPadding(2, 9);
		symbols_tb_.AddButtons("v", cmdId, 0);

		// disable all illegal file name chars, but ':' needed for pattern
		pattern_edit_.SetIllegalChars(_T("/\\*?|><\""));

		dlg_resize_map_.BuildMap(this);

		dlg_resize_map_.SetWndResizing(IDC_PATTERN, DlgAutoResize::RESIZE_H);
		dlg_resize_map_.SetWndResizing(IDC_SYMBOLS, DlgAutoResize::MOVE_H);
		dlg_resize_map_.SetWndResizing(IDC_INFO, DlgAutoResize::RESIZE);

		if (info_.SubclassDlgItem(IDC_INFO, this))
		{
			info_.SetTabStops(61);
			info_.SetWindowText(g_symbol_info_text);
		}

		if (CSpinButtonCtrl* spin= static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN_1)))	// number
			spin->SetRange32(0, 1000000);

		if (CSpinButtonCtrl* spin= static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN_2)))	// trim left
			spin->SetRange32(0, 100);
		if (CSpinButtonCtrl* spin= static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN_3)))	// trim right
			spin->SetRange32(0, 100);

		search_edit_.FileNameEditing(true);
		replace_edit_.FileNameEditing(true);

		return true;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


CString RenamePanelBasic::GetPattern()
{
	if (pattern_edit_.m_hWnd)
		pattern_edit_.GetWindowText(pattern_);

	return pattern_;
}


int RenamePanelBasic::GetStartNumber()
{
	if (number_.m_hWnd)
		starting_number_ = GetDlgItemInt(IDC_START);

	return starting_number_;
}


void RenamePanelBasic::OnPatternChange()
{
	ParamChanged();
}


void RenamePanelBasic::OnParamValueChange()
{
	ParamChanged();
}


void RenamePanelBasic::OnSearchTextChange()
{
	ParamChanged();
}


void RenamePanelBasic::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	switch (info_tip->iItem)
	{
	case IDC_SYMBOLS:
		SymbolsPopupMenu();
		break;
	}
}


void RenamePanelBasic::SymbolsPopupMenu()
{
	CRect rect;
	symbols_tb_.GetRect(IDC_SYMBOLS, rect);
	symbols_tb_.ClientToScreen(rect);

	RenameSymbolsPopupMenu(CPoint(rect.right, rect.bottom), this, pattern_edit_);
}


bool RenamePanelBasic::GenerateFileName(State state, const PhotoInfo* input, Path* output_name)
{
	if (state == Start)
	{
		return !!UpdateData();
//		GetPattern();
//		GetStartNumber();
	}
	else if (state == Next)
	{
		Path src= input->GetPhysicalPath();

		if (!pattern_.IsEmpty())
		{
			auto date(input->GetDateTime());

			String name= ParsePattern(src.GetFileName().c_str(), date, starting_number_++, pattern_);

			if (!search_text_.IsEmpty())
			{
				// search/replace
				if (replace_case_sensitive_)
					boost::algorithm::replace_all(name, search_text_, replace_text_);
				else
					boost::algorithm::ireplace_all(name, search_text_, replace_text_);
			}

			if (trim_left_ > 0)
			{
				if (static_cast<size_t>(trim_left_) >= name.size())
					name.clear();
				else
					name = name.substr(trim_left_);
			}

			if (trim_right_ > 0)
			{
				if (static_cast<size_t>(trim_right_) >= name.size())
					name.clear();
				else
					name = name.substr(0, name.size() - trim_right_);
			}

			if (letter_case_ > 0)
			{
				switch (letter_case_)
				{
				case 1:		// lowercase
					boost::algorithm::to_lower(name);
					break;
				case 2:		// uppercase
					boost::algorithm::to_upper(name);
					break;
				case 3:		// capitalize first letters
					boost::algorithm::to_lower(name);
					bool up= true;
					for (size_t i= 0; i < name.size(); ++i)
						if (up && std::isalpha(name[i]))
						{
							name[i] = std::toupper(name[i]);
							up = false;
						}
						else
							up = name[i] == ' ';
					break;
				}
			}

			String ext= src.GetExtension();
			if (!ext.empty())
			{
				if (extension_case_ == 0)
					boost::algorithm::to_upper(ext);
				else
					boost::algorithm::to_lower(ext);

				name += _T('.');
				name += ext;
			}

			*output_name = src;
			output_name->ReplaceFileNameExt(name.c_str());
		}
		else	// same file name
		{
			*output_name = src;
		}

	}

	return true;
}


bool RenamePanelBasic::ShowFullPath()
{
	return false;	// basic rename operates on file names only, so don't show full paths
}


HBRUSH RenamePanelBasic::OnCtlColor(CDC* dc, CWnd* wnd, UINT flags)
{
	if (wnd != 0 && wnd->GetDlgCtrlID() == IDC_INFO)
	{
		dc->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		dc->SetBkMode(TRANSPARENT);
	}
	return HBRUSH(RenamePage::OnCtlColor(dc, wnd, flags));
}


bool RenamePanelBasic::UpdateData()
{
	return !!ImgPage::UpdateData(true);
}
