/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RenameToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CatchAll.h"
#include "RenameToolDlg.h"
#include "RenamePanelBasic.h"
#include "RenamePanelAdvanced.h"
#include "PhotoInfo.h"
#include "Profile.h"
#include "SamplePhoto.h"
#include "LoadImageList.h"
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/trim.hpp"
#include <set>
#include "ResizeWnd.h"
#include "SeparatorWnd.h"
#include "clamp.h"
#include "RenameFileThread.h"
#include "ImgDb.h"
#include "ErrorDlg.h"
#include "BalloonMsg.h"


static const TCHAR REGISTRY_ENTRY_RENAME_TOOL[]= _T("RenameTool");
static const TCHAR REG_RENAME_RULES[]= _T("RenameRules");
static const TCHAR RENAME_RULE_NAME[]= _T("RuleName");
static const TCHAR RENAME_RULE_EXPR[]= _T("RuleScript");

static const int TIMER_ID= 12340;
static const int REFRESH_DELAY= 500;


void SaveRules(const std::vector<RenameRule>& rules)
{
	// save rename rules
	CString section= REGISTRY_ENTRY_RENAME_TOOL;
	section += '\\';
	section += REG_RENAME_RULES;

	CWinApp* app= AfxGetApp();

	for (size_t i= 0; i < rules.size(); ++i)
	{
		const RenameRule& rule= rules[i];

		{
			oStringstream key;
			key << RENAME_RULE_NAME << '-' << i;
			app->WriteProfileString(section, key.str().c_str(), rule.name_.c_str());
		}
		{
			oStringstream key;
			key << RENAME_RULE_EXPR << '-' << i;
			app->WriteProfileString(section, key.str().c_str(), rule.expression_.c_str());
		}
	}
}


void LoadRules(std::vector<RenameRule>& rules)
{
	// load rename rules
	CString section= REGISTRY_ENTRY_RENAME_TOOL;
	section += '\\';
	section += REG_RENAME_RULES;

	CWinApp* app= AfxGetApp();

	for (size_t i= 0; i < rules.size(); ++i)
	{
		RenameRule& rule= rules[i];

		{
			oStringstream key;
			key << RENAME_RULE_NAME << '-' << i;
			rule.name_ = app->GetProfileString(section, key.str().c_str(), rule.name_.c_str());
		}
		{
			oStringstream key;
			key << RENAME_RULE_EXPR << '-' << i;
			rule.expression_ = app->GetProfileString(section, key.str().c_str(), rule.expression_.c_str());
		}
	}
}


std::vector<RenameRule>& GetFileRenamingRules()
{
	static std::vector<RenameRule> rules;

	if (rules.empty())
	{
		const size_t count= 20;
		rules.resize(count);

		for (size_t i= 0; i < count; ++i)
		{
			oStringstream ost;
			ost << i + 1 << _T(". Renaming Rule");
			rules[i].name_ = ost.str();
			//TODO: examples

			if (i == 1)
			{
				rules[i].expression_ =
L"-- Rename jpg & raw keeping the same index\r\n"
L"\r\n"
L"-- keep track of names\r\n"
L"files = files or {};\r\n"
L"\r\n"
L"-- new index\r\n"
L"local n = files[file.dir..file.name];\r\n"
L"\r\n"
L"-- if file.name has not been seen, use new number\r\n"
L"if not n then\r\n"
L"\tn = number;\r\n"
L"\tnumber = number + 1;\r\n"
L"end\r\n"
L"\r\n"
L"-- remember index used by file.name\r\n"
L"files[file.dir..file.name] = n;\r\n"
L"\r\n"
L"return img.date..'-'..text..'-'..n..'.'..file.ext;\r\n";
			}
			else
			{
				oStringstream ost;
				ost << _T("-- File rename rule ") << i + 1 <<
					_T(".\r\n\r\n")
					_T("local name= file.name..'-'..text..'-'..number;\r\n")
					_T("\r\n")
					_T("number = number + 1;\r\n")
					_T("\r\n")
					_T("return name..'.'..file.ext;");
				rules[i].expression_ = ost.str();
			}

		}
	}

	LoadRules(rules);

	return rules;
}


struct NewName
{
	NewName() : status_(OK), renamed_(false), error_(RenameFileThread::OK)
	{}

	String fname_;		// new file name (with or without the path)
	enum Status
	{
		OK,
		InvalidName,	// file name is not valid
		NotAbsPath,		// absolute path expected
		NoFileName,		// missing file name
		InvalidPath,	// path or file name is not valid
		NameConflict,	// conflicting file names (duplicates)
		NoFileNameExt,	// missing file name extension
		NoPath,			// absolute path expected
	};
	Status status_;		// path/file name status
	bool renamed_;		// processed already?
	RenameFileThread::Errors error_;	// processing or verification error
};


enum Images { IMG_OK, IMG_WARNING, IMG_ERROR, IMG_RENAMED };


struct RenameToolDlg::Impl : ImgPageNotifications, ResizeWnd
{
	Impl(const VectPhotoInfo& photos, SmartPtr example)
		: photos_(photos), new_names_(photos.size()), advanced_panel_(example, GetFileRenamingRules()), resize_(false), ctrl_shift_(0)
	{
		current_dlg_ = 0;
		self_ = 0;
		open_page_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("OpenPage"), 0);
		selected_rule_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("SelectedRule"), 0);
		starting_number_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("StartingNumber"), 1);
		input_text_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("InputText"), _T(""));
		pattern_text_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("Pattern"), _T("%f"));
		starting_number_2_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("StartingNumber2"), 1);
		store_ctrl_shift_.Register(L"CopyMoveDlg", L"CtrlShift", ctrl_shift_);
		ctrl_shift_ = store_ctrl_shift_;
		results_list_initial_width_ = 0;
		current_image_ = 0;
		processing_phase_ = 0;
		errors_encountered_ = false;

		advanced_panel_.current_rule_ = selected_rule_;
		advanced_panel_.starting_number_ = starting_number_;
		advanced_panel_.input_text_ = input_text_;
		basic_panel_.pattern_ = pattern_text_;
		basic_panel_.starting_number_ = starting_number_2_;
	}
	~Impl()
	{
		store_ctrl_shift_ = ctrl_shift_;
	}

	CTabCtrl tab_;
	RenamePanelBasic basic_panel_;
	RenamePanelAdvanced advanced_panel_;
	CListCtrl results_;
	CImageList image_list_;
	RenamePage* current_dlg_;
	RenameToolDlg* self_;
	const VectPhotoInfo& photos_;
	std::vector<NewName> new_names_;
	std::vector<String> renamed_;
	String text_buffer_;
	Profile<int> open_page_;
	Profile<int> selected_rule_;
	Profile<int> starting_number_;
	Profile<CString> input_text_;
	Profile<int> col_1_width_;
	Profile<int> col_2_width_;
	Profile<CString> pattern_text_;
	Profile<int> starting_number_2_;
	SeparatorWnd resize_;
	int ctrl_shift_;	// when splitter is used it resizes left and right part of the window; this is shift from original location
	Profile<int> store_ctrl_shift_;
	int results_list_initial_width_;
	CButton abort_btn_;
	CButton ok_btn_;
	CButton cancel_btn_;
	CProgressCtrl progress_bar_;
	CStatic error_label_;
	boost::scoped_ptr<RenameFileThread> rename_thread_;
	int current_image_;
	int processing_phase_;
	bool errors_encountered_;

	bool InitDialog(CWnd* self);
	RenamePage* ShowDlg(RenamePage* dlg);
	void ShowDlg();
	void PositionTabDlg(RenamePage* dlg);
	void RefreshList();
	void SymbolsPopupMenu();
	void SaveSettings();
	bool Finish(CDialog* self);
	void StopProcessing(CDialog* self);
	void OnNextPhoto(CDialog* self, int photo_index);
	void StartProcessing(CWnd* self, int phase);

	virtual void ParamChanged(ImgPage* wnd, bool reset);

	// ResizeWnd notifications:
	virtual int GetPaneHeight();
	virtual void ResizePane(int width);

	void ResizePaneBy(int shift);
	int GetResultsCtrlWidth();
};


// RenameToolDlg dialog

RenameToolDlg::RenameToolDlg(CWnd* parent, VectPhotoInfo& photos)
  : DialogChild(RenameToolDlg::IDD, parent), impl_(new Impl(photos, photos.empty() ? CreateSamplePhotoInfo() : photos.front()))
{
	impl_->self_ = this;
}


RenameToolDlg::~RenameToolDlg()
{}


void RenameToolDlg::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);
	DDX_Control(dx, IDC_ABORT, impl_->abort_btn_);
	DDX_Control(dx, IDOK, impl_->ok_btn_);
	DDX_Control(dx, IDCANCEL, impl_->cancel_btn_);
	DDX_Control(dx, IDC_PROGRESS, impl_->progress_bar_);
	DDX_Control(dx, IDC_ERROR, impl_->error_label_);
}


BEGIN_MESSAGE_MAP(RenameToolDlg, DialogChild)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &RenameToolDlg::OnSelChangeTab)
	ON_NOTIFY_EX(LVN_GETDISPINFO, IDC_LIST, &RenameToolDlg::OnGetDispInfo)
	ON_NOTIFY_EX(LVN_GETINFOTIP, IDC_LIST, &RenameToolDlg::OnGetToolTipInfo)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ABORT, &RenameToolDlg::OnAbort)
	ON_MESSAGE(WM_APP, OnNextPhase)
	ON_MESSAGE(IPM_NEXT_PHOTO, OnNextPhoto)
	ON_MESSAGE(IPM_EXCEPTION, OnRenameError)
	ON_MESSAGE(IPM_PHOTO_STATUS, OnPhotoStatus)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


int RenameToolDlg::Impl::GetPaneHeight()
{
	return GetResultsCtrlWidth();
}


void RenameToolDlg::Impl::ResizePane(int width)
{
	if (results_.m_hWnd && self_ != 0)
	{
		int MIN_WIDTH= 100;
		width = clamp(width, MIN_WIDTH, results_list_initial_width_ * 3 / 2);
		ctrl_shift_ = results_list_initial_width_ - width;
TRACE(L"w: %d, init: %d  shift: %d\n", width, results_list_initial_width_, ctrl_shift_);
		ResizePaneBy(ctrl_shift_);
	}
}


void RenameToolDlg::Impl::ResizePaneBy(int shift)
{
	if (results_.m_hWnd && self_ != 0)
	{
		self_->SetControlsShift(CSize(shift, 0));
		self_->Invalidate();
	}
}


int RenameToolDlg::Impl::GetResultsCtrlWidth()
{
	WINDOWPLACEMENT wp;
	if (results_.m_hWnd != 0 && results_.GetWindowPlacement(&wp))
		return wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	return 0;
}


// RenameToolDlg message handlers

BOOL RenameToolDlg::OnInitDialog()
{
	try
	{
		DialogChild::OnInitDialog();

		VERIFY(impl_->resize_.SubclassDlgItem(IDC_RESIZE, this));
		impl_->resize_.SetCallbacks(impl_.get());
		impl_->resize_.DrawBar(false);
		impl_->resize_.GrowWhenMovingLeft(false);

		SubclassHelpBtn(_T("ToolRenamePhotos.htm"));

		bool ret= impl_->InitDialog(this);

		BuildResizingMap();

		SetWndResizing(IDC_TAB, DlgAutoResize::RESIZE, DlgAutoResize::SHIFT_RIGHT);
		SetWndResizing(IDC_LIST, DlgAutoResize::MOVE_H_RESIZE_V, /*DlgAutoResize::HALF_MOVE_H | DlgAutoResize::HALF_RESIZE_H |*/ DlgAutoResize::SHIFT_LEFT);
		SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_H, DlgAutoResize::SHIFT);
		SetWndResizing(IDOK, DlgAutoResize::MOVE);
		SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
		SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_RESIZE, DlgAutoResize::MOVE_H_RESIZE_V, DlgAutoResize::SHIFT);
		SetWndResizing(IDC_ABORT, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);
		SetWndResizing(IDC_PROGRESS, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);
		SetWndResizing(IDC_ERROR, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);

		CRect rect(0,0,0,0);
		GetWindowRect(rect);
		SetMinimalDlgSize(rect.Size());

		impl_->ResizePaneBy(impl_->ctrl_shift_);

		impl_->RefreshList();

		return ret;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


bool RenameToolDlg::Impl::InitDialog(CWnd* self)
{
	VERIFY(tab_.SubclassDlgItem(IDC_TAB, self));

	tab_.InsertItem(0, _T("Pattern-based Rename"), 0);
	tab_.InsertItem(1, _T("Advanced Rename"), 1);

	tab_.ModifyStyle(0, WS_CLIPCHILDREN);
	tab_.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	if (!basic_panel_.Create(&tab_))
		throw "Pattern panel creation failed";
	basic_panel_.SetHost(this);
	if (!advanced_panel_.Create(&tab_))
		throw "Advanced panel creation failed";
	advanced_panel_.SetHost(this);

	VERIFY(results_.SubclassDlgItem(IDC_LIST, self));
	results_.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_INFOTIP | LVS_EX_DOUBLEBUFFER);
	LoadPngImageList(image_list_, IDB_FILE_ICONS, ::GetSysColor(COLOR_WINDOW), true, 4);
	results_.SetImageList(&image_list_, LVSIL_SMALL);

	results_list_initial_width_ = GetResultsCtrlWidth();

	CRect client(0,0,0,0);
	results_.GetClientRect(client);
	int width= (client.Width() - ::GetSystemMetrics(SM_CXVSCROLL)) / 2;
	int min_width= 10;
	if (width < min_width)
		width = 10;

	col_1_width_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("Col1Width"), width);
	col_2_width_.Register(REGISTRY_ENTRY_RENAME_TOOL, _T("Col2Width"), width);

	int w1= col_1_width_;
	int w2= col_2_width_;

	if (w1 < min_width)
		w1= min_width;
	if (w2 < min_width)
		w2= min_width;
	//TODO: calc width based on actual string
	results_.InsertColumn(0, _T("Existing Name"), LVCFMT_LEFT, w1);
	results_.InsertColumn(1, _T("New Name"), LVCFMT_LEFT, w2);

	//TODO:
	//SetFooterDlg(&options_dlg_);

	tab_.SetCurSel(open_page_);

	ShowDlg();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void RenameToolDlg::OnSelChangeTab(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;
	impl_->ShowDlg();
}


void RenameToolDlg::Impl::ShowDlg()
{
	RenamePage* dlg= 0;

	switch (tab_.GetCurSel())
	{
	case 0:
		dlg = &basic_panel_;
		break;
	case 1:
		dlg = &advanced_panel_;
		break;
	}

	if (dlg == 0 || dlg == current_dlg_)
		return;

	if (current_dlg_)
	{
		current_dlg_->ShowWindow(SW_HIDE);
		current_dlg_->EnableWindow(false);
	}

	dlg->EnableWindow();
	current_dlg_ = ShowDlg(dlg);

	RefreshList();
}


RenamePage* RenameToolDlg::Impl::ShowDlg(RenamePage* dlg)
{
	if (dlg == 0 || dlg->m_hWnd == 0)
	{
		ASSERT(false);
		return 0;
	}

	PositionTabDlg(dlg);

	dlg->ShowWindow(SW_SHOWNA);

	if (self_)
		if (CWnd* ctrl= self_->GetNextDlgTabItem(0))
			self_->GotoDlgCtrl(ctrl);

	return dlg;
}


void RenameToolDlg::Impl::PositionTabDlg(RenamePage* dlg)
{
	if (dlg && dlg->m_hWnd)
	{
		CRect rect(0,0,0,0);
		tab_.GetClientRect(rect);
		tab_.AdjustRect(false, rect);
		dlg->MoveWindow(rect);
	}
}


void RenameToolDlg::Resize()
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return;

	DialogChild::Resize(rect);

	if (impl_->current_dlg_)
		impl_->PositionTabDlg(impl_->current_dlg_);
}


BOOL RenameToolDlg::OnGetDispInfo(UINT id, NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	size_t line= disp_info->item.iItem;

	if (disp_info->item.mask & LVIF_IMAGE)
	{
		if (line >= impl_->new_names_.size())
			return false;

		switch (impl_->new_names_[line].status_)
		{
		case NewName::OK:
			if (impl_->new_names_[line].error_)
				disp_info->item.iImage = IMG_ERROR;
			else
				disp_info->item.iImage = impl_->new_names_[line].renamed_ ? IMG_RENAMED : IMG_OK;
			break;
		case NewName::NameConflict:
			disp_info->item.iImage = IMG_WARNING;
			break;
		case NewName::InvalidName:
		case NewName::InvalidPath:
		case NewName::NoFileName:
		case NewName::NotAbsPath:
		case NewName::NoFileNameExt:
		case NewName::NoPath:
			disp_info->item.iImage = IMG_ERROR;
			break;
		default:
			ASSERT(false);
			break;
		}
	}

	if ((disp_info->item.mask & LVIF_TEXT) == 0)
		return false;

	disp_info->item.pszText[0] = _T('\0');

	try
	{
		switch (disp_info->item.iSubItem)
		{
		case 0:	// existing name
			if (line >= impl_->photos_.size())
				return false;

			impl_->text_buffer_ = impl_->photos_[line]->GetPhysicalPath().GetFileNameAndExt();
			disp_info->item.pszText = const_cast<TCHAR*>(impl_->text_buffer_.c_str());
			break;

		case 1:	// new name
			if (line >= impl_->new_names_.size())
				return false;
	
			impl_->text_buffer_ = impl_->new_names_[line].fname_;
			if (impl_->text_buffer_.empty())
				_tcsncpy(disp_info->item.pszText, _T("-"), disp_info->item.cchTextMax);
			else
				disp_info->item.pszText = const_cast<TCHAR*>(impl_->text_buffer_.c_str());
			break;
		}
	}
	catch (...)
	{
		ASSERT(false);
	}

	return true;
}


BOOL RenameToolDlg::OnGetToolTipInfo(UINT id, NMHDR* nmhdr, LRESULT* result)
{
	NMLVGETINFOTIP* info= reinterpret_cast<NMLVGETINFOTIP*>(nmhdr);

	if (!info || info->pszText == 0 || info->cchTextMax < 1)
		return false;

	size_t line= info->iItem;
	if (line >= impl_->new_names_.size())
		return false;

	const wchar_t* tip= 0;

	switch (impl_->new_names_[line].status_)
	{
	case NewName::OK:
		switch (impl_->new_names_[line].error_)
		{
		case RenameFileThread::NoFile:
			tip = L"Photograph is not available";
			break;
		case RenameFileThread::ReadOnlyFile:
			tip = L"Photograph file is read-only";
			break;
		case RenameFileThread::FileNameConflict:
			tip = L"File with new name already exists";
			break;
		case RenameFileThread::DirNameConflict:
			tip = L"Cannot use new name because there is a folder with such name already";
			break;
		case RenameFileThread::InvalidPath:
			tip = L"New path is too long";
			break;
		case RenameFileThread::OK:
			break;	// no tool tip
		default:
			ASSERT(false);
			break;	// no tool tip
		}
		break;
	case NewName::NameConflict:
		tip = L"New name has already been used";
		break;
	case NewName::InvalidName:
		tip = L"Invalid file name";
		break;
	case NewName::InvalidPath:
		tip = L"Invalid file path";
		break;
	case NewName::NoFileName:
		tip = L"Missing file name";
		break;
	case NewName::NoFileNameExt:
		tip = L"Missing file name extension";
		break;
	case NewName::NotAbsPath:
		tip = L"Absolute path expected";
		break;
	case NewName::NoPath:
		tip = L"Expected absolute path and a file name";
		break;
	default:
		ASSERT(false);
		break;
	}

	if (tip)
	{
		_tcsncpy(info->pszText, tip, info->cchTextMax);
		info->pszText[info->cchTextMax - 1] = 0;
	}
	else
		*info->pszText = 0;	// no tool tip

	return true;
}


//void GeneratePatternNames(const VectPhotoInfo& photos, int serial, const TCHAR* rename_pattern, std::vector<String>& new_names)
//{
//	const size_t count= photos.size();
//	new_names.resize(count);
//
//	for (size_t i= 0; i < count; ++i)
//	{
//		ConstPhotoInfoPtr photo= photos[i];
//		Path src= photo->GetPhysicalPath(); //.GetFileNameAndExt();
//
//		String dest= src;
//
//		if (rename_pattern != 0 && *rename_pattern != 0)	// rename file?
//		{
//			COleDateTime date(photo->GetDateTime().GetTime());
//
//			dest = ParsePattern(src.GetFileName().c_str(), date, serial++, rename_pattern);
//			String ext= src.GetExtension();
//			if (!ext.empty())
//			{
//				dest += _T('.');
//				dest += ext;
//			}
//		}
//		else	// same file name
//		{
//			dest = src.GetFileNameAndExt();
//		}
//
//		new_names[i] = dest;
//	}
//}

bool IsPathAbsolute(const String& path)
{
	if (path.length() < 2)
		return false;

	wchar_t c= path[0];
	wchar_t c2= path[1];

	if (c == '\\' && c2 == '\\' || c == '/' && c2 == '/')
		return true;

	if (path.length() > 1 && path[1] == ':' && (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'))
		return true;

	return false;
}


void RenameToolDlg::Impl::RefreshList()
{
	self_->KillTimer(TIMER_ID);

	if (current_dlg_ == 0 || photos_.empty())
		return;

	//new_names_.clear();

	bool show_full_path= current_dlg_->ShowFullPath();
	std::set<String> file_names;

	if (current_dlg_->GenerateFileName(RenamePage::Start, 0, 0))
	{
		const size_t count= photos_.size();
		//new_names_.resize(count);
		ASSERT(new_names_.size() == count);

		for (size_t i= 0; i < count; ++i)
		{
			Path output;
			if (current_dlg_->GenerateFileName(RenamePage::Next, photos_[i].get(), &output))
			{
				new_names_[i].fname_ = show_full_path ? output : output.GetFileNameAndExt();

				boost::algorithm::trim(new_names_[i].fname_);

				Path fname= new_names_[i].fname_;
				NewName::Status status= NewName::OK;

				if (show_full_path)
				{
					if (!IsPathAbsolute(fname))
						status = NewName::NotAbsPath;		// expected absolute path
					Path file= fname.GetFileNameAndExt();
					if (file == fname)
						status = NewName::NoPath;			// mising path part
				}
				else
				{
					if (fname.empty())
						status = NewName::NoFileName;
					else
					{
						size_t pos= fname.rfind(L'.');
						if (pos == String::npos || pos == fname.size() - 1)
							status = NewName::NoFileNameExt;
					}
				}

				if (status != NewName::OK)
					new_names_[i].status_ = status;
				else if (fname.empty())		// TODO validate name
					new_names_[i].status_ = NewName::InvalidName;
				else
				{
					boost::algorithm::to_lower(fname);
					new_names_[i].status_ = file_names.insert(fname).second ? NewName::OK : NewName::NameConflict;
				}
			}

	//int serial= basic_panel_.GetStartNumber();
	//CString pattern= basic_panel_.GetPattern();

//	bool ok= GeneratePatternNames(photos_, serial, pattern, new_names_);
		}

		// end
		current_dlg_->GenerateFileName(RenamePage::End, 0, 0);
	}

	results_.SetItemCount(static_cast<int>(photos_.size()));
	results_.Invalidate();
}


void RenameToolDlg::Impl::ParamChanged(ImgPage* wnd, bool reset)
{
	//TODO: delayed refresh
	self_->SetTimer(TIMER_ID, REFRESH_DELAY, 0);
//	RefreshList();
}


void RenameToolDlg::OnDestroy()
{
	if (impl_->results_.m_hWnd)
	{
		impl_->col_1_width_ = impl_->results_.GetColumnWidth(0);
		impl_->col_2_width_ = impl_->results_.GetColumnWidth(1);
	}
}


void RenameToolDlg::Impl::SaveSettings()
{
	if (tab_.m_hWnd)
		open_page_ = tab_.GetCurSel();

	selected_rule_ = advanced_panel_.current_rule_;
	starting_number_ = advanced_panel_.starting_number_;
	input_text_ = advanced_panel_.input_text_;

	pattern_text_ = basic_panel_.pattern_;
	starting_number_2_ = basic_panel_.starting_number_;

	SaveRules(advanced_panel_.Rules());
}


void RenameToolDlg::OnTimer(UINT_PTR timer_id)
{
	DialogChild::OnTimer(timer_id);

	if (timer_id == TIMER_ID)
		impl_->RefreshList();
}


void RenameToolDlg::OnOK()
{
	try
	{
		if (!Finish())
			return;

//		EndDialog(IDOK);
	}
	CATCH_ALL
}


bool RenameToolDlg::Finish()
{
	try
	{
		if (!UpdateData()) // || !dlg_options_.UpdateData())
			return false;

		return impl_->Finish(this);
	}
	CATCH_ALL_W(this)
	return false;
}


bool RenameToolDlg::Impl::Finish(CDialog* self)
{
	if (current_dlg_ == 0 || !current_dlg_->UpdateData())
		return false;

	//progress_bar_.SetMarquee(false, 40);

	// save rules

	// validate new file names, check for conflicts

	//if (new_names_.size() != photos_.size())
	// regenerate names, necessary for adv. panel
	RefreshList();

	const size_t count= new_names_.size();
	renamed_.clear();
	renamed_.reserve(count);
	for (size_t i= 0; i < count; ++i)
	{
		if (new_names_[i].status_ != NewName::OK)
		{
			// highlight error
			new BalloonMsg(&results_, _T("Please Correct Problems"), L"Some files cannot be renamed.\nMouse over warning icon to see the details.", BalloonMsg::IERROR);
			return false;
		}

		new_names_[i].error_ = RenameFileThread::OK;
		new_names_[i].renamed_ = false;
		renamed_.push_back(new_names_[i].fname_);
	}
	results_.Invalidate();

	error_label_.ShowWindow(SW_HIDE);

//	progress_bar_.SetMarquee(false, 0);
	progress_bar_.SetRange32(0, 2 * static_cast<int>(count));
	progress_bar_.SetPos(0);
	progress_bar_.ShowWindow(SW_SHOWNA);

	errors_encountered_ = false;

	// start worker thread in name verification phase
	StartProcessing(self, 0);

	abort_btn_.ShowWindow(SW_SHOWNA);

	// TODO

	SaveSettings();

	self->GotoDlgCtrl(&abort_btn_);

	tab_.EnableWindow(false);
	ok_btn_.EnableWindow(false);
	cancel_btn_.EnableWindow(false);

	return true;
}


void RenameToolDlg::Impl::StartProcessing(CWnd* self, int phase)
{
	// start worker thread

	// there are two renaming phases (or runs):
	// first phase (dry run) verifies destination names/folders and checks for conflicts with existing files
	// second phase renames/moves files
	processing_phase_ = phase;

	ImageDatabase& db= GetImageDataBase(false, true);
	rename_thread_.reset(new RenameFileThread(photos_, renamed_, db, processing_phase_ == 0));
	rename_thread_->SetStatusWnd(self);
	rename_thread_->Start();
}


void RenameToolDlg::Impl::StopProcessing(CDialog* self)
{
	rename_thread_.reset();

	tab_.EnableWindow(true);
	ok_btn_.EnableWindow(true);
	cancel_btn_.EnableWindow(true);

	self->GotoDlgCtrl(&ok_btn_);
	progress_bar_.ShowWindow(SW_HIDE);
	abort_btn_.ShowWindow(SW_HIDE);
}


void RenameToolDlg::OnAbort()
{
	impl_->errors_encountered_ = true;	// set this flag just in case WM_APP is in a message queue to prevent it from starting next processing phase
	impl_->StopProcessing(this);
}


LRESULT RenameToolDlg::OnNextPhase(WPARAM phase, LPARAM)
{
	if (impl_->errors_encountered_)
		return 0;

	// start renaming thread
	if (phase == 0)	// first phase completed?
	{
		const size_t count= impl_->new_names_.size();
		for (size_t i= 0; i < count; ++i)
		{
			if (impl_->new_names_[i].error_ != 0)
			{
				//TODO: error message

				// highlight error...
				impl_->error_label_.ShowWindow(SW_SHOWNA);

				// scroll to error
				impl_->results_.EnsureVisible(static_cast<int>(i), false);

				impl_->StopProcessing(this);

				return 0;	// stop now
			}
		}

		impl_->StartProcessing(this, 1);
	}
	else
	{
		impl_->StopProcessing(this);

		if (!impl_->errors_encountered_)
			EndDialog(IDOK);
	}

	return 0;
}


void RenameToolDlg::Impl::OnNextPhoto(CDialog* self, int photo_index)
{
	current_image_ = photo_index;
	if (current_image_ < 0)
	{
		// photo_index == IPM_COMPLETION_STATUS_OK -> processing finished normally;
		// photo_index == IPM_COMPLETION_STATUS_ERR -> processing aborted
		if (processing_phase_ == 0 && !errors_encountered_ && photo_index == IPM_COMPLETION_STATUS_OK)
			self->PostMessage(WM_APP, 0);	// start next phase; post it to itself and return to let worker thread finish
		else
			self->PostMessage(WM_APP, 1);	// stop processing
	}
	else
	{
		progress_bar_.SetPos(current_image_ + processing_phase_ * static_cast<int>(photos_.size()));

		if (processing_phase_ > 0 && static_cast<size_t>(current_image_) < new_names_.size())
		{
			new_names_[current_image_].renamed_ = true;
			// TODO: repaint one item...
			results_.Invalidate();
		}
	}
}


LRESULT RenameToolDlg::OnNextPhoto(WPARAM photo_index, LPARAM after)
{
	impl_->OnNextPhoto(this, static_cast<int>(photo_index));
	return 0;
}


LRESULT RenameToolDlg::OnRenameError(WPARAM indexFile, LPARAM message)
{
	impl_->errors_encountered_ = true;

	if (message)
	{
		std::auto_ptr<String> s(reinterpret_cast<String*>(message));
		CString msg= s->c_str();
		if (indexFile < impl_->photos_.size())
		{
			msg += L'\n';
			msg += impl_->photos_[indexFile]->GetPhysicalPath().c_str();
		}

		::DisplayErrorDialog(this, L"Changing file name failed.", msg);
	}

	OnAbort();
	return 0;
}


void RenameToolDlg::OnCancel()
{
	impl_->StopProcessing(this);
	return DialogChild::OnCancel();
}


LRESULT RenameToolDlg::OnPhotoStatus(WPARAM file_index, LPARAM status)
{
	if (file_index < impl_->new_names_.size())
	{
		impl_->new_names_[file_index].error_ = static_cast<RenameFileThread::Errors>(status);
		impl_->results_.Invalidate();
	}
	return 0;
}
