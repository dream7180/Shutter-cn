/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CatalogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CatalogDlg.h"
#include "CatchAll.h"
#include "CatalogImages.h"
#include "Database/Database.h"
#include "FileTypeIndex.h"
#include "Config.h"
#include "FormatFileSize.h"
#include "CatalogOptionsDlg.h"
#include "RString.h"
#include "PathEdit.h"
#include "Profile.h"
#include "ItemIdList.h"
#include "BoldFont.h"
#include <boost/shared_ptr.hpp>
#include "FolderSelect.h"
#include "BalloonMsg.h"
#include "CatalogHelpDlg.h"
#include <boost/bind.hpp>
#include "FolderPathHelpers.h"
#include "BrowserFrame.h"	// dependency on FolderSelected()
#include "PhotoInfoPtr.h"

namespace {
//	const TCHAR* CATALOG_OPTIONS_DLG= _T("CatalogOptionsDlg");
	const TCHAR* SCAN_TYPES= _T("ScanTypes");
	const TCHAR* SECTION= _T("CatalogDlg");
}


struct CatalogDlg::Impl
{
	std::auto_ptr<CatalogImages> catalog_;
	HANDLE thread_;
	CButton ok_;
	bool processing_now_;
	bool list_of_types_empty_;
	String initial_src_path_;		// if given use this path to scan for images
	CProgressCtrl progress_;
	CEdit title_;
	CEdit description_;
	CSliderCtrl img_size_;
	CStatic action_;
	CStatic file_name_;
	CStatic file_counter_;
	CStatic est_size_;
	CStatic est_time_;
	CButton abort_;
	CButton drives_btn_;
	CButton folder_btn_;
	CButton browse_folder_btn_;
	CButton save_as_btn_;
	CWnd* wnd_;
	bool catalog_built_;
	size_t total_;
	FILETIME start_time_;
	bool break_;
	CatalogOptionsDlg options_;
	CStatic catalog_path_;			// path to save catalog
	CPathEdit catalog_fname_;		// filename to save catalog
	CPathEdit folder_to_scan_;
	Profile<int> scan_drive_;
	Profile<int> drive_index_;
	Profile<CString> scan_folder_;
	Profile<CString> save_as_path_;
	//Profile<CString> save_as_fname_;
	Profile<int> preview_slider_;
	Profile<int> compr_level_;
	Profile<bool> open_catalog_;	// open created catalog in ExifPro
	Profile<CString> title_str_;
	Profile<CString> descr_str_;
	Profile<bool> auto_close_;		// close window after catalog creation
	CFont small_font_;
	CListCtrl drives_;
	typedef boost::shared_ptr<ItemIdList> ItemIdListPtr;
	std::vector<ItemIdListPtr> drive_paths_;
	CImageList sys_img_list_;
	CatalogHelpDlg help_dlg_;
	bool ready_;

	// params used by worker thread:-------
	Path scan_path_;
	Path save_path_;
	int img_size_limit_;
	bool folder_scan_;
	String ctlg_title_;
	String ctlg_descr_;
	int compression_level_;
	//-------------------------------------

	Impl()
	{
		wnd_ = 0;
		thread_ = 0;
		catalog_built_ = false;
		total_ = 0;
		break_ = false;
		img_size_limit_ = 0;
		folder_scan_ = false;
		compression_level_ = 0;
		processing_now_ = false;
		list_of_types_empty_ = false;
		ready_ = false;

		SHFILEINFO info;
		if (DWORD_PTR image_list= ::SHGetFileInfo(_T(""), NULL, &info, sizeof info, SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SYSICONINDEX))
			sys_img_list_.Attach(reinterpret_cast<HIMAGELIST>(image_list));

		scan_drive_.Register(SECTION, _T("ScanObj"), 0);
		drive_index_.Register(SECTION, _T("DriveIndex"), 1);
		scan_folder_.Register(SECTION, _T("ScanPath"), _T(""));
		preview_slider_.Register(SECTION, _T("PreviewSize"), 5);
		compr_level_.Register(SECTION, _T("ComprLevel"), 92);
		ItemIdList my_docs(CSIDL_PERSONAL);
		save_as_path_.Register(SECTION, _T("SaveAsPath"), my_docs.GetPath());
//		save_as_fname_.Register(SECTION, _T("SaveAsName"), _T("My Pictures.catalog"));
		open_catalog_.Register(SECTION, _T("OpenCatalog"), true);
		auto_close_.Register(SECTION, _T("AutoClose"), true);
	}

	~Impl()
	{
		sys_img_list_.Detach();

		Break(false);
	}

	void InitDlg(CWnd* wnd);
	void StartBuild(const TCHAR* path, const TCHAR* save_here, int img_size);
	void Progress(int step, size_t count);
	static UINT AFX_CDECL WorkerProc(LPVOID param);
	void CreateCatalog();
	void Break(bool del_catalog);
	void ResetControls();
	void SaveSettings();
	void OnSaveAs();
	void OnBrowse();
	void SetFont(int id, CFont& font);
	void SystemDrives(CListCtrl& drives, int selected);
	void Build();
	void SetFitInBoxLabel();
	void TypeToScanChanged();
	void EnableOk();
	void EnableControls();
	void ShowControls();
	void DriveChanged();
	void AfterBrowse(const CString& str);
};


void CatalogDlg::Impl::SetFont(int id, CFont& font)
{
	if (CWnd* ctrl= wnd_->GetDlgItem(id))
		ctrl->SetFont(&font);
}


static const int IMG_SIZES[]=
{
	160, 200, 240, 280,
	320, 400, 480, 560,
	640, 720, 800, 880,
	1024, 1152, 1280
};

//static const IMG_SIZES[]= { 160, 320, 640, 800, 1024 };

void CatalogDlg::Impl::SetFitInBoxLabel()
{
	if (wnd_ == 0 || img_size_.m_hWnd == 0)
		return;

	unsigned int pos= img_size_.GetPos();
	if (pos < array_count(IMG_SIZES))
	{
		int n= IMG_SIZES[pos];
		oStringstream ost;
		ost << _T("(fit in a ") << n << _T(" by ") << n << _T(" box)");

		wnd_->SetDlgItemText(IDC_FIT_BOX, ost.str().c_str());
	}
}


void CatalogDlg::Impl::InitDlg(CWnd* wnd)
{
	CWaitCursor wait;

	wnd_ = wnd;

	VERIFY(ok_.SubclassDlgItem(IDOK, wnd_));
	VERIFY(drives_btn_.SubclassDlgItem(IDC_DRIVE, wnd_));
	VERIFY(folder_btn_.SubclassDlgItem(IDC_FOLDER, wnd_));
	VERIFY(browse_folder_btn_.SubclassDlgItem(IDC_BROWSE_FOLDER, wnd_));
	VERIFY(save_as_btn_.SubclassDlgItem(IDC_SAVE_AS, wnd_));
	VERIFY(img_size_.SubclassDlgItem(IDC_THUMB_SIZE, wnd_));
	VERIFY(progress_.SubclassDlgItem(IDC_PROGRESS, wnd_));
	VERIFY(title_.SubclassDlgItem(IDC_TITLE, wnd_));
	VERIFY(description_.SubclassDlgItem(IDC_DESCRIPTION, wnd_));
	VERIFY(action_.SubclassDlgItem(IDC_ACTION, wnd_));
	VERIFY(file_name_.SubclassDlgItem(IDC_FILE, wnd_));
	VERIFY(file_counter_.SubclassDlgItem(IDC_COUNTER, wnd_));
	VERIFY(est_size_.SubclassDlgItem(IDC_EST_SIZE, wnd_));
	VERIFY(est_time_.SubclassDlgItem(IDC_TIME_LEFT, wnd_));
	VERIFY(abort_.SubclassDlgItem(IDC_ABORT, wnd_));
	VERIFY(catalog_path_.SubclassDlgItem(IDC_PATH, wnd_));
	catalog_path_.SetWindowText(save_as_path_.Value());
	catalog_path_.ModifyStyle(0, SS_PATHELLIPSIS);
	VERIFY(catalog_fname_.SubclassDlgItem(IDC_FILENAME, wnd_));
	catalog_fname_.FileNameEditing(true);
	//TODO: generate unique file name

	VERIFY(folder_to_scan_.SubclassDlgItem(IDC_SCAN_FOLDER, wnd_));

	title_.SetWindowText(title_str_.Value());
	description_.SetWindowText(descr_str_.Value());

	wnd_->CheckRadioButton(IDC_DRIVE, IDC_FOLDER, IDC_DRIVE + scan_drive_);
	if (initial_src_path_.empty())
		folder_to_scan_.SetWindowText(scan_folder_.Value());
	else
	{
//		folder_to_scan_.SetWindowText(initial_src_path_.c_str());
//		wnd_->CheckRadioButton(IDC_DRIVE, IDC_FOLDER, IDC_FOLDER);
		AfterBrowse(initial_src_path_.c_str());
	}

	img_size_.SetRange(0, array_count(IMG_SIZES) - 1);
//	img_size_.SetTicFreq(2);
	img_size_.SetPos(preview_slider_);
	SetFitInBoxLabel();

	options_.SetDlgItemInt(IDC_COMPR, compr_level_);
	options_.CheckDlgButton(IDC_OPEN_CATALOG, open_catalog_);
	options_.CheckDlgButton(IDC_AUTO_CLOSE, auto_close_);
	options_.LoadSettings(SECTION, SCAN_TYPES);
	options_.SetTypeChangeCallback(boost::bind(&Impl::TypeToScanChanged, this));

	::CreateSmallFont(wnd_, small_font_);

	SetFont(IDC_SMALL_1, small_font_);
	SetFont(IDC_SMALL_2, small_font_);
	SetFont(IDC_SMALL_3, small_font_);
	SetFont(IDC_SMALL_4, small_font_);
	SetFont(IDC_SMALL_5, small_font_);
	SetFont(IDC_FIT_BOX, small_font_);

	VERIFY(drives_.SubclassDlgItem(IDC_SOURCE, wnd_));
	drives_.SetImageList(&sys_img_list_, LVSIL_SMALL);
	SystemDrives(drives_, drive_index_);

	ready_ = true;
}


void CatalogDlg::Impl::SystemDrives(CListCtrl& drives, int selected)
{
	drives.DeleteAllItems();
	drive_paths_.clear();

	int item= 0;
	if (DWORD len= ::GetLogicalDriveStrings(0, 0))
	{
		std::vector<TCHAR> drv(len + 1, 0);
		::GetLogicalDriveStrings(len, &drv[0]);

		TCHAR* p= &drv[0];
		while (*p)
		{
			ItemIdListPtr ptr(new ItemIdList(p));
			drive_paths_.push_back(ptr);
			UINT state= selected == item ? LVIS_SELECTED : 0;
			drives.InsertItem(LVIF_TEXT | LVIF_STATE | LVIF_IMAGE, item++, drive_paths_.back()->GetName(),
				state, LVIS_SELECTED, drive_paths_.back()->GetIconIndex(), 0);

/*			switch (::GetDriveType(p))
			{
			case DRIVE_CDROM:
			case DRIVE_NO_ROOT_DIR:
				break;
			default:
				InsertUniquePath(impl_.current_paths_, ItemIdList(p), true);
				break;
			} */

			p += _tcslen(p) + 1;
		}
	}
}


void CatalogDlg::Impl::Break(bool del_catalog)
{
	break_ = true;

	HANDLE t= thread_;
	if (t)
	{
		//FIX: potential crash
		if (catalog_.get())
			catalog_->Break();

		CWaitCursor wait;
		// boost thread priority so it can reach the break flag
		::SetThreadPriority(t, THREAD_PRIORITY_NORMAL);
		::WaitForSingleObject(t, 5000);

		t = thread_;
		if (t)
		{
			ASSERT(false);
			::TerminateThread(t, -1);
		}

		thread_ = 0;

		if (del_catalog && !save_path_.empty())
			::DeleteFile(save_path_.c_str());
	}

	if (wnd_ && wnd_->m_hWnd && abort_.m_hWnd)
	{
		abort_.ShowWindow(SW_HIDE);
		abort_.EnableWindow(false);

		processing_now_ = false;
		EnableOk();
		EnableControls();
		ShowControls();
	}
}


void CatalogDlg::Impl::StartBuild(const TCHAR* path, const TCHAR* save_here, int img_size)
{
	if (thread_ != 0)
		return;

	scan_path_ = path;
	save_path_ = save_here;
	img_size_limit_ = img_size;

	break_ = false;

	if (CWinThread* thread= ::AfxBeginThread(WorkerProc, this,
		THREAD_PRIORITY_LOWEST/*THREAD_PRIORITY_NORMAL*/, 0, CREATE_SUSPENDED))
	{
		thread_ = *thread;
		processing_now_ = true;
		EnableOk();
		EnableControls();
		thread->ResumeThread();

		abort_.EnableWindow(true);
		abort_.ShowWindow(SW_SHOWNA);
	}
	else
		wnd_->MessageBox(_T("创建线程错误"));
}


// CatalogDlg dialog

CatalogDlg::CatalogDlg(CWnd* parent, const TCHAR* src_path)
	: DialogChild(CatalogDlg::IDD, parent), pImpl_(new Impl)
{
	if (src_path && *src_path)
		pImpl_->initial_src_path_ = src_path;
}

CatalogDlg::~CatalogDlg()
{
}

void CatalogDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(CatalogDlg, DialogChild)
	ON_MESSAGE(CatalogImages::MESSAGE, OnProgress)
	ON_MESSAGE(WM_USER + 100, OnBeginEnd)
	ON_MESSAGE(WM_USER + 101, OnThreadAborted)
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	ON_BN_CLICKED(IDC_SAVE_AS, OnSaveAs)
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, OnBrowse)
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SOURCE, OnDriveChanged)
END_MESSAGE_MAP()


// CatalogDlg message handlers


BOOL CatalogDlg::OnInitDialog()
{
	try
	{
		DialogChild::OnInitDialog();

		if (!pImpl_->options_.Create(GetParent()))
		{
			EndDialog(IDCANCEL);
			return true;
		}

		if (!pImpl_->help_dlg_.Create(this))
		{
			EndDialog(IDCANCEL);
			return true;
		}

		const int ID= 100;
		if (CWnd* frm= GetDlgItem(IDC_FRAME_3))
		{
			WINDOWPLACEMENT wp;
			frm->GetWindowPlacement(&wp);
			CRect r= wp.rcNormalPosition;
			pImpl_->help_dlg_.SetWindowPos(0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
			pImpl_->help_dlg_.SetDlgCtrlID(ID);
		}

		pImpl_->InitDlg(this);

		SetFooterDlg(&pImpl_->options_);
		SetDlgItemText(IDC_OPTIONS, RString(IDS_SAVING_OPTIONS));

		SubclassHelpBtn(_T("ToolCatalog.htm"));

		BuildResizingMap();
		SetWndResizing(IDC_SOURCE, DlgAutoResize::RESIZE);
		SetWndResizing(IDC_FRAME_1, DlgAutoResize::RESIZE);
		SetWndResizing(IDC_FOLDER, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_SCAN_FOLDER, DlgAutoResize::MOVE_V_RESIZE_H);
//		SetWndResizing(IDC_LABEL_0, DlgAutoResize::MOVE_V);
//		SetWndResizing(IDC_IMG_TYPES, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_BROWSE_FOLDER, DlgAutoResize::MOVE);
		SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_TITLE, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_DESCRIPTION, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_LABEL_3, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_THUMB_SIZE, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_SMALL_1, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_SMALL_2, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_SMALL_3, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_SMALL_4, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_SMALL_5, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_FIT_BOX, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_FRAME_2, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_LABEL_4, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_LABEL_5, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_PATH, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_SAVE_AS, DlgAutoResize::MOVE);
		SetWndResizing(IDC_FILENAME, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_FRAME_3, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_ACTION, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_EST_SIZE3, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_TIME_LEFT, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_EST_SIZE2, DlgAutoResize::MOVE);
		SetWndResizing(IDC_EST_SIZE, DlgAutoResize::MOVE);
		SetWndResizing(IDC_FILE, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_COUNTER, DlgAutoResize::MOVE);
		SetWndResizing(IDC_PROGRESS, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_OPTIONS, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_ABORT, DlgAutoResize::MOVE);
		SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
		SetWndResizing(IDOK, DlgAutoResize::MOVE);
		SetWndResizing(ID, DlgAutoResize::MOVE, DlgAutoResize::HALF_MOVE_H);	// help dlg

		return true;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


void CatalogDlg::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (scroll_bar && scroll_bar->m_hWnd == pImpl_->img_size_.m_hWnd)
		pImpl_->SetFitInBoxLabel();
}


void CatalogDlg::Impl::Build()
{
	Path save;
	{
		CString str;
		catalog_path_.GetWindowText(str);
		save = static_cast<const TCHAR*>(str);
	}
	{
		CString name;
		catalog_fname_.GetWindowText(name);
		if (name.IsEmpty())
		{
			new BalloonMsg(&catalog_fname_, _T("需要分类文件名"),
				_T("请输入分类的文件名."), BalloonMsg::IERROR);
			return;
		}

		Path file(name);
		if (file.GetExtension() != _T("catalog"))
			name += _T(".catalog");
		save.AppendDir(name, false);

		if (save.FileExists())
		{
#ifdef _DEBUG
			::DeleteFile(save.c_str());
#else
			new BalloonMsg(&catalog_fname_, _T("文件已经存在"),
				_T("请输入新的分类文件名."), BalloonMsg::IERROR);
			return;
#endif
		}
	}

	int img_size= 0;
	unsigned int size_idx= img_size_.GetPos();
	if (size_idx < array_count(IMG_SIZES))
		img_size = IMG_SIZES[size_idx];
	else
	{
		ASSERT(false);	// img size
		return;
	}

	{
		CString str;
		title_.GetWindowText(str);
		ctlg_title_ = str;
	}
	{
		CString str;
		description_.GetWindowText(str);
		ctlg_descr_ = str;
	}

	compression_level_ = options_.GetDlgItemInt(IDC_COMPR);
	if (compression_level_ < 1)
		compression_level_ = 1;
	else if (compression_level_ > 100)
		compression_level_ = 100;

	if (wnd_->IsDlgButtonChecked(IDC_FOLDER))	// scan folder
	{
		CString str;
		folder_to_scan_.GetWindowText(str);

		Path path(str);
		if (path.IsFolder())
		{
			folder_scan_ = true;
			StartBuild(path.c_str(), save.c_str(), img_size);
		}
		else
		{
			new BalloonMsg(&folder_to_scan_, _T("需要文件夹路径"),
				_T("请输入文件夹路径来扫描图像."), BalloonMsg::IERROR);
		}
	}
	else	// drive selected?
	{
		POSITION pos= drives_.GetFirstSelectedItemPosition();
		int item= drives_.GetNextSelectedItem(pos);
		if (static_cast<size_t>(item) < drive_paths_.size())
		{
			CString drv= drive_paths_[item]->GetPath();
			folder_scan_ = false;
			StartBuild(drv, save.c_str(), img_size);
		}
		else
		{
			new BalloonMsg(&drives_, _T("需要选择驱动器"),
				_T("请选择驱动器来扫描图像."), BalloonMsg::IERROR);
		}
	}
}


void CatalogDlg::OnOK()
{
	try
	{
		pImpl_->SaveSettings();

		if (pImpl_->catalog_built_)
			DialogChild::OnOK();
		else
			pImpl_->Build();
	}
	CATCH_ALL
}


void CatalogDlg::OnAbort()
{
	pImpl_->Break(true);
}


void CatalogDlg::OnCancel()
{
	pImpl_->Break(true);

	DialogChild::OnCancel();
}


UINT AFX_CDECL CatalogDlg::Impl::WorkerProc(LPVOID param)
{
	CatalogDlg::Impl* dlg= reinterpret_cast<CatalogDlg::Impl*>(param);

	try
	{
		dlg->CreateCatalog();

		return 0;
	}
	CATCH_ALL_W(dlg->wnd_)

	dlg->wnd_->PostMessage(WM_USER + 101);

//	dlg->catalog_.reset();

	return 1;
}


void CatalogDlg::Impl::CreateCatalog()
{
	if (break_)
		return;

	wnd_->PostMessage(WM_USER + 100, 0, false);

	catalog_built_ = false;

	Database db(false);	// catalog is a db without index file

	::DeleteFile(save_path_.c_str());

	if (!db.Open(save_path_, 1, false))
		throw UserException(L"Catalog Builder", L"Catalog cration failed." , (L"Cannot create catalog at " + save_path_).c_str());

	if (break_)
		return;

	catalog_.reset(new CatalogImages(wnd_->m_hWnd, false, db));

	std::vector<bool> scan_types;
	options_.GetSelectedTypes(scan_types);
	if (scan_types.empty())
	{
		ASSERT(false);
		return;
	}

	if (break_)
		return;

	catalog_->ProcessFiles(scan_path_, true, scan_types, ctlg_title_, ctlg_descr_, img_size_limit_, folder_scan_,
		compression_level_);

	if (!break_)
		catalog_built_ = true;

	wnd_->PostMessage(WM_USER + 100, catalog_->Count(), true);

	thread_ = 0;
}


LRESULT CatalogDlg::OnProgress(WPARAM count, LPARAM step)
{
	pImpl_->Progress(static_cast<int>(step), count);
	return 0;
}


void CatalogDlg::Impl::Progress(int step, size_t count)
{
	switch (step)
	{
	case CatalogImages::DIR_SCANNING:
		{
			// count of files found so far
			oStringstream ost;
			ost << count;
			file_counter_.SetWindowText(ost.str().c_str());
		}
		break;

	case CatalogImages::DIR_SCAN_DONE:
		{
			progress_.SetRange32(0, static_cast<int>(count));
			progress_.SetPos(0);
			total_ = count;

			file_counter_.SetWindowText(_T(""));
			file_counter_.ShowWindow(SW_SHOWNA);
			action_.SetWindowText(_T("正在处理图像:"));

			::GetSystemTimeAsFileTime(&start_time_);
		}
		break;

	case CatalogImages::IMG_PROCESSING:
		{
			progress_.SetPos(static_cast<int>(count));

			if (catalog_.get())
			{
				String path= catalog_->GetFile(count);
				file_name_.SetWindowText(path.c_str());
			}

			// update counter
			{
				oStringstream ost;
				ost << count << _T('/') << total_;
				file_counter_.SetWindowText(ost.str().c_str());
			}

			// estimate catalog size
			if (count > 0)
			{
				uint64 len= save_path_.GetFileLength();
				uint64 est= len * total_ / count;
				est_size_.SetWindowText(FormatFileSize(est).c_str());
			}

			// estimate time left
			if (count > 0)
			{
				FILETIME ft;
				::GetSystemTimeAsFileTime(&ft);
				uint64 lapse= (ft.dwLowDateTime + (uint64(ft.dwHighDateTime) << 32)) -
					(start_time_.dwLowDateTime + (uint64(start_time_.dwHighDateTime) << 32));
				const uint64 TO_SEC= 10000000;
				uint64 est= (lapse * total_ / count - lapse) / TO_SEC;
				CString time= CTimeSpan(est).Format(_T("%H:%M:%S"));
				est_time_.SetWindowText(time);
			}
		}
		break;

	case CatalogImages::IMG_PROC_DONE:
		progress_.SetPos(static_cast<int>(total_));	// go to the end of progress bar ctrl
		file_counter_.ShowWindow(SW_HIDE);
		action_.ShowWindow(SW_HIDE);
		file_name_.ShowWindow(SW_HIDE);
		est_size_.SetWindowText(FormatFileSize(save_path_.GetFileLength()).c_str());
		break;

	default:
		ASSERT(false);
		break;
	}
}


void CatalogDlg::Impl::ResetControls()
{
	progress_.SetPos(0);
	est_time_.SetWindowText(_T("-"));
	file_counter_.SetWindowText(_T(""));
	file_name_.SetWindowText(_T(""));

	action_.ShowWindow(SW_HIDE);
	file_name_.ShowWindow(SW_HIDE);
	file_counter_.ShowWindow(SW_HIDE);
}


LRESULT CatalogDlg::OnThreadAborted(WPARAM, LPARAM)
{
	OnAbort();
	return 0;
}


LRESULT CatalogDlg::OnBeginEnd(WPARAM count, LPARAM finished)
{
	if (finished)	// done
	{
		pImpl_->ResetControls();
		pImpl_->Break(false);
		pImpl_->processing_now_ = false;
		pImpl_->EnableOk();

		if (pImpl_->catalog_built_)
		{
			if (pImpl_->options_.IsDlgButtonChecked(IDC_AUTO_CLOSE))
			{
				FolderPathPtr path;
				// open newly created catalog file
				if (pImpl_->options_.IsDlgButtonChecked(IDC_OPEN_CATALOG))
					path = ::CreateFolderPath(pImpl_->save_path_.c_str());

				DialogChild::OnOK();	// exit

				if (path != 0)
					if (BrowserFrame* frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd()))
						frame->FolderSelected(path);

				return 0;
			}
		}

		pImpl_->ShowControls();

		// start over...
		pImpl_->catalog_built_ = false;
	}
	else	// just starting
	{
		pImpl_->ResetControls();
		pImpl_->action_.SetWindowText(_T("正在搜索图像:"));
		pImpl_->file_counter_.SetWindowText(_T("-"));
		pImpl_->est_size_.SetWindowText(_T("-"));
		pImpl_->ShowControls();
	}

	return 0;
}


void CatalogDlg::OnOptions()
{
	if (pImpl_->options_.m_hWnd == 0)
		return;

	bool show= pImpl_->options_.GetStyle() & WS_VISIBLE ? false : true;

	if (GetGripWnd().m_hWnd)
		GetGripWnd().ShowWindow(show ? SW_HIDE : SW_SHOWNA);

	ShowFooterDlg(show);

	SetDlgItemText(IDC_OPTIONS, RString(show ? IDS_CLOSE_SAVING_OPTIONS : IDS_SAVING_OPTIONS));
}


void CatalogDlg::OnDestroy()
{
}


void CatalogDlg::Impl::SaveSettings()
{
	scan_drive_ = wnd_->IsDlgButtonChecked(IDC_DRIVE) ? 0 : 1;
	if (scan_drive_ == 0)
	{
		POSITION pos= drives_.GetFirstSelectedItemPosition();
		int item= drives_.GetNextSelectedItem(pos);
		if (static_cast<size_t>(item) < drive_paths_.size())
			drive_index_ = item;
	}
	{
		CString path;
		folder_to_scan_.GetWindowText(path);
		scan_folder_ = path;
	}
	{
		CString path;
		catalog_path_.GetWindowText(path);
		save_as_path_ = path;
	}
	preview_slider_ = img_size_.GetPos();

	int compr_level= options_.GetDlgItemInt(IDC_COMPR);
	if (compr_level > 0 && compr_level <= 100)
		compr_level_ = compr_level;

	open_catalog_ = options_.IsDlgButtonChecked(IDC_OPEN_CATALOG) != 0;
	auto_close_ = options_.IsDlgButtonChecked(IDC_AUTO_CLOSE) != 0;

	{
		CString title;
		title_.GetWindowText(title);
		title_str_ = title;
	}
	{
		CString desc;
		description_.GetWindowText(desc);
		descr_str_ = desc;
	}

	options_.SaveSettings(SECTION, SCAN_TYPES);
}


void CatalogDlg::OnSaveAs()
{
	pImpl_->OnSaveAs();
}


void CatalogDlg::Impl::OnSaveAs()
{
	CString name;
	catalog_fname_.GetWindowText(name);

	CFileDialog dlg(false, _T(".catalog"), name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("分类文件 (*.catalog)|*.catalog|全部文件 (*.*)|*.*||"), wnd_, sizeof(OPENFILENAME));

	dlg.m_pOFN->lpstrInitialDir = save_as_path_.Value();
	//dlg.m_pOFN->Flags |= ;

	if (dlg.DoModal() != IDOK)
		return;

	Path file= dlg.GetPathName();

	if (file.FileExists())	// user has been warned already!
		::DeleteFile(file.c_str());

	catalog_path_.SetWindowText(file.GetDir().c_str());
	catalog_fname_.SetWindowText(file.GetFileNameAndExt().c_str());
}


void CatalogDlg::OnBrowse()
{
	pImpl_->OnBrowse();
}


void CatalogDlg::Impl::OnBrowse()
{
	CFolderSelect browse(wnd_);

	CString path;
	folder_to_scan_.GetWindowText(path);
	CString str= browse.DoSelectPath(_T("选择文件夹进行扫描"), path);

	if (str.IsEmpty())
		return;

	scan_folder_ = str;

	AfterBrowse(str);
}

void CatalogDlg::Impl::AfterBrowse(const CString& str)
{
	folder_to_scan_.SetWindowText(str);

	wnd_->CheckRadioButton(IDC_DRIVE, IDC_FOLDER, IDC_FOLDER);

	String name= Path(str).GetFileNameAndExt();
	if (!name.empty())
	{
		title_.SetWindowText(name.c_str());
		catalog_fname_.SetWindowText(name.c_str());
	}
}


void CatalogDlg::OnDriveChanged(NMHDR* nmhdr, LRESULT* result)
{
	pImpl_->DriveChanged();
}

void CatalogDlg::Impl::DriveChanged()
{
	if (!ready_)
		return;

	// when new drive selected use its volume name as a suggested catalog name

	POSITION pos= drives_.GetFirstSelectedItemPosition();
	int item= drives_.GetNextSelectedItem(pos);
	if (static_cast<size_t>(item) < drive_paths_.size())
	{
		Path path= drive_paths_[item]->GetPath();

		TCHAR volume[MAX_PATH]= { 0 };
		DWORD serial_no= 0;
		DWORD dummy= 0;
		DWORD vol_flags= 0;
		if (::GetVolumeInformation(path.c_str(), volume, MAX_PATH, &serial_no, &dummy, &vol_flags, 0, 0) == 0)
		{
			path += _T("\\");
			if (::GetVolumeInformation(path.c_str(), volume, MAX_PATH, &serial_no, &dummy, &vol_flags, 0, 0) == 0)
				::GetVolumeInformation(path.GetRoot().c_str(), volume, MAX_PATH, &serial_no, &dummy, &vol_flags, 0, 0);
		}

		if (volume[0])
		{
			title_.SetWindowText(volume);
			catalog_fname_.SetWindowText(volume);
		}
	}

	if (wnd_)
		wnd_->CheckRadioButton(IDC_DRIVE, IDC_FOLDER, IDC_DRIVE);
}


void CatalogDlg::Impl::TypeToScanChanged()
{
	std::vector<bool> scan_types;
	options_.GetSelectedTypes(scan_types);

	const size_t count= scan_types.size();
	bool no_selection= true;
	for (size_t i= 0; i < count; ++i)
		if (scan_types[i])
		{
			no_selection = false;
			break;
		}

	if (no_selection != list_of_types_empty_)
	{
		list_of_types_empty_ = no_selection;
		EnableOk();
	}
}


void CatalogDlg::Impl::EnableOk()
{
	ok_.EnableWindow(!processing_now_ && !list_of_types_empty_);
}


void CatalogDlg::Impl::EnableControls()
{
	bool enable= !processing_now_;

	title_.EnableWindow(enable);
	description_.EnableWindow(enable);
	img_size_.EnableWindow(enable);
	catalog_fname_.EnableWindow(enable);
	drives_.EnableWindow(enable);
	folder_to_scan_.EnableWindow(enable);
	drives_btn_.EnableWindow(enable);
	folder_btn_.EnableWindow(enable);
	browse_folder_btn_.EnableWindow(enable);
	save_as_btn_.EnableWindow(enable);

	options_.EnableControls(enable);
}


void CatalogDlg::Impl::ShowControls()
{
	int show= processing_now_ ? SW_SHOWNA : SW_HIDE;

//	action_.SetWindowText(_T("Searching for images:"));
	action_.ShowWindow(show);
	file_name_.ShowWindow(show);
	//file_counter_.SetWindowText(_T("-"));
	file_counter_.ShowWindow(show);
	//est_size_.SetWindowText(_T("-"));
	est_size_.ShowWindow(show);

	help_dlg_.ShowWindow(!processing_now_ ? SW_SHOWNA : SW_HIDE);

	if (wnd_ == 0)
		return;

	// show/hide progress frame and controls
	static const int ctrls[]=
	{ IDC_FRAME_3, IDC_ACTION, IDC_FILE, IDC_COUNTER, IDC_PROGRESS,
	  IDC_EST_SIZE3, IDC_TIME_LEFT, IDC_EST_SIZE2, IDC_EST_SIZE };

	for (int i= 0; i < array_count(ctrls); ++i)
		if (HWND ctrl= ::GetDlgItem(wnd_->m_hWnd, ctrls[i]))
			::ShowWindow(ctrl, show);
}
