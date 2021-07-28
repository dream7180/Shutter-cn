/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PrintDlg.h"
#include "GlobalLock.h"
#include "BalloonMsg.h"
#include "ParseRange.h"
#include "PrintThumbnails.h"
#include "PrintPhotos.h"
#include "ColorMngDlg.h"
#include "CatchAll.h"
#include "Config.h"
#include "LoadImageList.h"
#include "WhistlerLook.h"
#include "GetDefaultGuiFont.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//#define USE_PRINTER_LIST	1

/////////////////////////////////////////////////////////////////////////////
// PrintDlg dialog

CPrintDialog PrintDlg::dlg_print_(true);


PrintDlg::PrintDlg(VectPhotoInfo& selected, bool thumbnails, const TCHAR* folder_path)
 : DialogChild(PrintDlg::IDD, 0),
	print_photos_(new PrintPhotos()), print_thumbnails_(new PrintThumbnails()),
	selected_(selected), preview_wnd_(selected, 0), folder_path_(folder_path)
{
	layout_type_ = FULL_PAGE;
	print_ = thumbnails ? print_thumbnails_.get() : print_photos_.get();
	preview_wnd_.SetEngine(print_);

	TCHAR units[2]= { 0 };
	::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, units, array_count(units));
	use_metric_units_for_info_ = metric_ = units[0] == _T('0');

	thumbnails_ = thumbnails;
	//{{AFX_DATA_INIT(PrintDlg)
	//}}AFX_DATA_INIT
	// margins in tenths of millimeter
	if (metric_)
		margins_rect_ = CRect(200, 200, 200, 200);
	else
		margins_rect_ = CRect(254, 254, 254, 254);
	selected_margin_ = ALL_MARGINS;

	const TCHAR* REGISTRY_PRNT_DLG= _T("PrintDlg");
	profile_show_prinatable_area_.Register(REGISTRY_PRNT_DLG, _T("ShowPrinatableArea"), false);
	profile_show_margins_.Register(REGISTRY_PRNT_DLG, _T("ShowMargins"), true);
	profile_show_img_space_.Register(REGISTRY_PRNT_DLG, _T("ShowImgSpace"), false);
	profile_layout_type_.Register(REGISTRY_PRNT_DLG, _T("LayoutType"), 0);
	//profile_layout_thumb_type_.Register(REGISTRY_PRNT_DLG, _T("LayoutThumbType"), 0);
	profile_cur_page_.Register(REGISTRY_PRNT_DLG, _T("CurrentPage"), 0);
	profile_cur_page_thumb_.Register(REGISTRY_PRNT_DLG, _T("CurrentPageThumb"), 0);
	profile_page_range_.Register(REGISTRY_PRNT_DLG, _T("PageRange"), 0);
	profile_print_footer_.Register(REGISTRY_PRNT_DLG, _T("PrintFooter"), true);
	profile_print_footer_text_.Register(REGISTRY_PRNT_DLG, _T("PrintFooterText"), true);
	profile_selected_pages_.Register(REGISTRY_PRNT_DLG, _T("PageSelection"), _T("1-999"));
	profile_margin_type_.Register(REGISTRY_PRNT_DLG, _T("MarginType"), 0);
	profile_margins_.Register(REGISTRY_PRNT_DLG, _T("Margins"), margins_rect_);
	profile_margins_thumbs_.Register(REGISTRY_PRNT_DLG, _T("MarginsThumbs"), margins_rect_);
	profile_thumbs_items_across_.Register(REGISTRY_PRNT_DLG, _T("ThumbsItemsAcross"), 6);
	profile_photo_copies_.Register(REGISTRY_PRNT_DLG, _T("PhotoCopies"), 1);
	profile_info_units_.Register(REGISTRY_PRNT_DLG, _T("InfoUnits"), use_metric_units_for_info_);
	profile_footer_text_.Register(REGISTRY_PRNT_DLG, _T("FooterText"), _T("Thumbnail images"));
	LOGFONT lf;
	print_thumbnails_->GetDefaultFont(lf);
	profile_font_.Register(REGISTRY_PRNT_DLG, _T("Font"), lf);
	profile_print_option_.Register(REGISTRY_PRNT_DLG, _T("PrintOption"), 0);
	profile_zoom_.Register(REGISTRY_PRNT_DLG, L"Zoom", 100);

	dlg_thumbs_options_.font_ = profile_font_;
	dlg_thumbs_options_.print_option_ = profile_print_option_;

	profiles_changed_ = false;
}


PrintDlg::~PrintDlg()
{
//	delete &print_;
}


static const TCHAR* const REG_PRINTER_CFG= _T("PrinterConfig");

/////////////////////////////////////////////////////////////////////////////////

class Handle
{
public:
	Handle(HANDLE h= 0) : h(h)
	{}

	~Handle()
	{
		Close();
	}

	HANDLE get() const			{ return h; }

	operator HANDLE () const	{ return h; }

	HANDLE* get_addr()
	{
		Close();
		return &h;
	}

	void Close()
	{
		if (h)
			::ClosePrinter(h);
		h = 0;
	}

private:
	HANDLE h;

	Handle(const Handle&);
	Handle& operator = (const Handle&);
};


class GlobalMemory
{
public:
	GlobalMemory(size_t size, DWORD flags)
	{
		mem = ::GlobalAlloc(flags, size);
	}

	~GlobalMemory()
	{
		free();
	}

	GlobalMemory(HGLOBAL attach)
	{
		mem = attach;
	}

	GlobalMemory()
	{
		mem = 0;
	}

	HGLOBAL get() const		{ return mem; }

	HGLOBAL release()
	{
		HGLOBAL m= mem;
		mem = 0;
		return m;
	}

	void Attach(HGLOBAL attach)
	{
		free();
		mem = attach;
	}

private:
	HGLOBAL mem;

	void free()
	{
		if (mem)
		{
			::GlobalFree(mem);
			mem = 0;
		}
	}

	GlobalMemory(const GlobalMemory&);
	GlobalMemory& operator = (const GlobalMemory&);
};


HGLOBAL GetPrinterDevMode(HWND wnd, const TCHAR* printer_name_arg, bool portrait, int paper_size, int print_quality, int y_resolution)
{
	// OpenPrinter and DocumentProperties should really have const input params
	TCHAR* printer_name= const_cast<TCHAR*>(printer_name_arg);

	Handle printer_handle;
	// Start by opening the printer
	if (!::OpenPrinter(printer_name, printer_handle.get_addr(), NULL))
		return 0;

	// Step 1:
	// Allocate a buffer of the correct size.
	DWORD size= ::DocumentProperties(wnd,
		printer_handle,		// Handle to our printer
		printer_name,		// Name of the printer
		NULL,				// Asking for size, so
		NULL,				// these are not used
		0);					// Zero returns buffer size

	if (size == 0)
		return 0;

	GlobalMemory dev_mode_mem(size, GMEM_MOVEABLE | GMEM_ZEROINIT);

	if (dev_mode_mem.get() == 0)
		return 0;

	GlobalMemLock<DEVMODE> dev_mode(dev_mode_mem.get());

	// Step 2:
	// Get the default DevMode for the printer and modify it for your needs
	if (::DocumentProperties(wnd,
		printer_handle,
		printer_name,
		dev_mode,		// The address of the buffer to fill
		NULL,			// Not using the input buffer
		DM_OUT_BUFFER	// Have the output buffer filled
		) != IDOK)
		return 0;

	// Make changes to the DevMode which are supported.
	if (dev_mode->dmFields & DM_ORIENTATION)
	{
		// If the printer supports paper orientation, set it
		dev_mode->dmOrientation = portrait ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
	}

	if (dev_mode->dmFields & DM_PAPERSIZE)
		dev_mode->dmPaperSize = paper_size;

	if (dev_mode->dmFields & DM_PRINTQUALITY)
		dev_mode->dmPrintQuality = print_quality;

	if (dev_mode->dmFields & DM_YRESOLUTION)
		dev_mode->dmYResolution = y_resolution;

	//if (dev_mode->dmFields & DM_DUPLEX)
	//{
	//	/* If it supports duplex printing, use it. */ 
	//	dev_mode->dmDuplex = DMDUP_HORIZONTAL;
	//}

	// Step 3:
	// Merge the new settings with the old.
	// This gives the driver an opportunity to update any private portions of the DevMode structure

	if (::DocumentProperties(wnd,
		printer_handle,
		printer_name,
		dev_mode,       // Reuse our buffer for output
		dev_mode,       // Pass the driver our changes
		DM_IN_BUFFER |  // Commands to Merge our changes and
		DM_OUT_BUFFER	// write the result
		) != IDOK)
		return 0;

	// return the modified DevMode structure
	return dev_mode_mem.release();
}


HGLOBAL GetPrinterDevNames(const TCHAR* printer_name, const TCHAR* driver_name, const TCHAR* output_name)
{
	if (*printer_name == 0 || *driver_name == 0 || *output_name == 0)
		return 0;

	size_t size= (_tcslen(printer_name) + 1 + _tcslen(driver_name) + 1 + _tcslen(output_name) + 1) * sizeof(TCHAR) + sizeof(DEVNAMES);

	GlobalMemory dev_names_mem(size, GMEM_MOVEABLE | GMEM_ZEROINIT);

	if (dev_names_mem.get() == 0)
		return 0;

	GlobalMemLock<DEVNAMES> dev_names(dev_names_mem.get());

	const TCHAR* start= dev_names.AsTCharPtr();
	TCHAR* p= reinterpret_cast<TCHAR*>(dev_names.AsBytePtr() + sizeof DEVNAMES);

	_tcscpy(p, driver_name);
	dev_names->wDriverOffset = static_cast<WORD>(p - start);
	p += _tcslen(driver_name) + 1;

	_tcscpy(p, printer_name);
	dev_names->wDeviceOffset = static_cast<WORD>(p - start);
	p += _tcslen(printer_name) + 1;

	_tcscpy(p, output_name);
	dev_names->wOutputOffset = static_cast<WORD>(p - start);
	p += _tcslen(output_name) + 1;

	dev_names->wDefault = 0;

	return dev_names_mem.release();
}

// store/restore printer selection

bool SavePrinterSelection(const TCHAR* key, HANDLE dev_mode_handle, HANDLE dev_names_handle)
{
	// save the current printer name, spooler and port to the registry

	GlobalMemLock<DEVNAMES> dev_names(dev_names_handle);

	if (dev_names == 0)
		return false;

	CString printer = dev_names.AsTCharPtr() + dev_names->wDeviceOffset;
	CString driver = dev_names.AsTCharPtr() + dev_names->wDriverOffset;
	CString port = dev_names.AsTCharPtr() + dev_names->wOutputOffset;

	GlobalMemLock<DEVMODE> dev_mode(dev_mode_handle);

	// get the landscape/portrait mode of the printer
	int portrait= true;
	int paper_size= DMPAPER_LETTER;
	int print_quality= 0;
	int y_resolution= 0;

	if (dev_mode)
	{
		// get orientation
		portrait = dev_mode->dmOrientation == DMORIENT_PORTRAIT;
		// selected papaer size
		paper_size = dev_mode->dmPaperSize;
        //short dmPaperLength;
        //short dmPaperWidth;
        //short dmScale;
        //short dmCopies;
        //short dmDefaultSource;
        //short dmPrintQuality;
		print_quality = dev_mode->dmPrintQuality;
		y_resolution = dev_mode->dmYResolution;
	}

	CWinApp* app= AfxGetApp();
	ASSERT(app);
	VERIFY(app->WriteProfileString(key, _T("PrinterName"), printer));
	VERIFY(app->WriteProfileString(key, _T("Driver"), driver));
	VERIFY(app->WriteProfileString(key, _T("Port"), port));
	VERIFY(app->WriteProfileInt(key, _T("Portrait"), portrait));
	VERIFY(app->WriteProfileInt(key, _T("PaperSize"), paper_size));
	VERIFY(app->WriteProfileInt(key, _T("PrintQuality"), print_quality));
	VERIFY(app->WriteProfileInt(key, _T("YResolution"), y_resolution));

	return true;
}


bool RestorePrinterSelection(const TCHAR* key, GlobalMemory& dev_mode_handle, GlobalMemory& dev_names_handle)
{
	// read the settings back from the registry, abort if not present
	CWinApp* app= AfxGetApp();
	ASSERT(app);

	CString printer= app->GetProfileString(key, _T("PrinterName"));
	CString driver= app->GetProfileString(key, _T("Driver"));
	CString port= app->GetProfileString(key, _T("Port"));
	int portrait= app->GetProfileInt(key, _T("Portrait"), true);
	int paper_size= app->GetProfileInt(key, _T("PaperSize"), DMPAPER_LETTER);
	int print_quality= app->GetProfileInt(key, _T("PrintQuality"), 0);
	int y_resolution= app->GetProfileInt(key, _T("YResolution"), 0);

	if (printer.IsEmpty() || driver.IsEmpty() || port.IsEmpty())
		return false;			// not setup

	GlobalMemory dev_mode(GetPrinterDevMode(0, printer, portrait != 0, paper_size, print_quality, y_resolution));

	if (dev_mode.get() == 0)
		return false;

	GlobalMemory dev_names(GetPrinterDevNames(printer, driver, port));

	if (dev_names.get() == 0)
		return false;

	dev_mode_handle.Attach(dev_mode.release());
	dev_names_handle.Attach(dev_names.release());

	return true;
}


/////////////////////////////////////////////////////////////////////////////////


void PrintDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(PrintDlg)
	DDX_Control(DX, IDC_LABEL_PAGE, page_label_wnd_);
	DDX_Control(DX, IDC_LABEL_SIZE, size_label_wnd_);
	DDX_Control(DX, IDC_LAYOUT_INFO, layout_info_wnd_);
	DDX_Control(DX, IDC_MARGIN, edit_margin_);
	DDX_Control(DX, IDC_MARGIN_SLIDER, margin_slider_wnd_);
	DDX_Control(DX, IDC_MARGIN_SPIN, margin_spin_wnd_);
	DDX_Control(DX, IDC_TYPE, type_wnd_);
	DDX_Control(DX, IDC_PRINTERS, printers_wnd_);
	DDX_Control(DX, IDC_PAGES, pages_wnd_);
	DDX_Control(DX, IDC_SWITCH, switch_wnd_);
	DDX_Control(DX, IDC_UNITS, units_wnd_);
	DDX_Control(DX, IDC_MARGIN_SELECT, margin_select_wnd_);
	//}}AFX_DATA_MAP

	if (dlg_range_.m_hWnd)
		dlg_range_.UpdateData(DX->m_bSaveAndValidate);
	if (dlg_thumbs_options_.m_hWnd)
		dlg_thumbs_options_.UpdateData(DX->m_bSaveAndValidate);
	if (dlg_photo_options_.m_hWnd)
		dlg_photo_options_.UpdateData(DX->m_bSaveAndValidate);
}


BEGIN_MESSAGE_MAP(PrintDlg, DialogChild)
	//{{AFX_MSG_MAP(PrintDlg)
	ON_BN_CLICKED(IDC_PRN_SETTINGS, OnPrinterSettings)
	ON_BN_CLICKED(IDC_PAGE_SETUP, OnPageSetup)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TYPE, OnTypeSelected)
	ON_COMMAND(ID_SHOW_MARGINS, OnShowMargins)
	ON_UPDATE_COMMAND_UI(ID_SHOW_MARGINS, OnUpdateShowMargins)
	ON_COMMAND(ID_SHOW_PRINT_AREA, OnShowPrintArea)
	ON_UPDATE_COMMAND_UI(ID_SHOW_PRINT_AREA, OnUpdateShowPrintArea)
	ON_BN_CLICKED(IDC_FOOTER, OnFooter)
	ON_BN_CLICKED(IDC_FOOTER_T, OnFooter)
	ON_EN_CHANGE(IDC_FOOTER_TEXT, OnChangeFooterText)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_PAGES_BOX, OnChangePagesBox)
	ON_WM_HSCROLL()
	ON_NOTIFY(UDN_DELTAPOS, IDC_MARGIN_SPIN, OnDeltaPosMarginSpin)
	ON_EN_CHANGE(IDC_MARGIN, OnChangeMargin)
	ON_BN_CLICKED(IDC_ICM, OnColorSetup)
	//}}AFX_MSG_MAP
#if USE_PRINTER_LIST
	ON_CBN_SELCHANGE(IDC_PRINTERS, OnSelChangePrinters)
#endif
	ON_COMMAND_RANGE(ID_SELECT_PAGE_0, ID_SELECT_PAGE_0 + 9999, OnSelectPage)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SELECT_PAGE_0, ID_SELECT_PAGE_0 + 9999, OnUpdateSelectPage)
	ON_NOTIFY(TBN_DROPDOWN, IDC_MARGIN_SELECT, OnTbDropDown)
	ON_NOTIFY(TBN_DROPDOWN, IDC_UNITS, OnTbUnitsDown)
	ON_MESSAGE(WM_USER, OnItemsNumberChanged)
	ON_MESSAGE(WM_USER + 1, OnNumberOfCopiesChanged)
	ON_MESSAGE(WM_USER + 2, OnFontChanged)
	ON_MESSAGE(WM_USER + 3, OnPrintOptionChanged)
	ON_MESSAGE(WM_USER + 4, OnZoomChanged)
	ON_COMMAND(ID_SHOW_IMAGE_SPACE, OnShowImgSpace)
	ON_UPDATE_COMMAND_UI(ID_SHOW_IMAGE_SPACE, OnUpdateShowImgSpace)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PrintDlg message handlers

static PrintEngine::PrintingOptions RadioBtnToPrintOption(int btn)
{
	PrintEngine::PrintingOptions o= PrintEngine::None;

	switch (btn)
	{
	case 0:
		o = PrintEngine::PrintFileNames;
		break;
	case 1:
		o = PrintEngine::PrintDateTime;
		break;
	case 2:
		o = PrintEngine::None;
		break;

	default:
		break;
	};

	return o;
}


BOOL PrintDlg::OnInitDialog()
{
	try
	{
		return InitDialog();
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


BOOL PrintDlg::InitDialog()
{
	if (dlg_print_.m_pd.hDevMode == 0 && dlg_print_.m_pd.hDevNames == 0)
	{
		GlobalMemory dev_mode, dev_names;
		if (RestorePrinterSelection(REG_PRINTER_CFG, dev_mode, dev_names))
		{
			dlg_print_.m_pd.hDevMode = dev_mode.release();
			dlg_print_.m_pd.hDevNames = dev_names.release();

			AfxGetApp()->SelectPrinter(dlg_print_.m_pd.hDevNames, dlg_print_.m_pd.hDevMode, true);
		}
	}

	if (dlg_print_.m_pd.hDevMode == 0)
	{
		if (!AfxGetApp()->GetPrinterDeviceDefaults(&dlg_print_.m_pd) || dlg_print_.m_pd.hDevNames == 0)
		{
			AfxMessageBox(_T("There's no default printer installed in the system."), MB_ICONWARNING | MB_OK);
			EndDialog(IDCANCEL);
			return false;
		}
	}

	dlg_range_.page_range_ = profile_page_range_;
	dlg_range_.page_range_str_ = profile_selected_pages_;
	dlg_thumbs_options_.print_footer_ = profile_print_footer_;
	dlg_thumbs_options_.print_footer_text_ = profile_print_footer_text_;
	dlg_thumbs_options_.items_across_ = profile_thumbs_items_across_;
	dlg_thumbs_options_.footer_text_ = profile_footer_text_;
	dlg_photo_options_.copies_ = profile_photo_copies_;
	dlg_photo_options_.zoom_ = profile_zoom_;
	use_metric_units_for_info_ = profile_info_units_;

	margins_rect_ = layout_type_ == THUMBNAILS ? profile_margins_thumbs_ : profile_margins_;

	CStatic	options_placeholder_wnd;
	options_placeholder_wnd.SubclassDlgItem(IDC_OPTIONS_PLACEHOLDER, this);

	DialogChild::OnInitDialog();

	//SubclassHelpBtn(_T("ToolPrint.htm"));

	if (!dlg_range_.Create(this, IDC_PAGE_RANGE_PLACE) ||
		!dlg_thumbs_options_.Create(this, options_placeholder_wnd) ||
		!dlg_photo_options_.Create(this, options_placeholder_wnd))
	{
		ASSERT(false);
		EndDialog(IDCANCEL);
		return true;
	}

	options_placeholder_wnd.DestroyWindow();

	margin_slider_wnd_.SetRange(0, RANGE);
	margin_slider_wnd_.SetTicFreq(RANGE / 10);
	margin_slider_wnd_.SetPageSize(RANGE / 10);

	margin_spin_wnd_.SetRange32(0, 60000);	// CmnCtrl 4.71 or later

	margin_select_wnd_.SetPadding(0, 10);
	int cmds[]= { ID_ALL, ID_LEFT, ID_RIGHT, ID_TOP, ID_BOTTOM }; //ID_SELECT_MARGIN;
	margin_select_wnd_.AddButtons("VVVVV", cmds, IDB_EMPTY2, IDS_MARGINS);

	if (!WhistlerLook::IsAvailable())
	{
		CDC dc;
		dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
		/*LOGFONT lf;
		HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		::GetObject(hfont, sizeof(lf), &lf);
		//lf.lfHeight += 1;
		//lf.lfQuality = ANTIALIASED_QUALITY;
		_tcscpy(lf.lfFaceName, _T("Tahoma"));
		CFont _font;
		_font.CreateFontIndirect(&lf);*/
		dc.SelectObject(&GetDefaultGuiFont());//&_font);
		//dc.SelectStockObject(DEFAULT_GUI_FONT);
		int extra_width= dc.GetTextExtent(_T("XXX"), 3).cx;

		CString btn;
		btn.LoadString(IDS_MARGINS);

		// btn sizing is broken
		for (int i= 0; i < array_count(cmds); ++i)
		{
			CString str;
			::AfxExtractSubString(str, btn, i);

			int width= dc.GetTextExtent(str).cx;

			TBBUTTONINFO tbi;
			memset(&tbi, 0, sizeof tbi);
			tbi.cbSize = sizeof tbi;
			tbi.dwMask = TBIF_SIZE;
			tbi.cx = width + extra_width;
			margin_select_wnd_.SetButtonInfo(cmds[i], &tbi);
		}

		WINDOWPLACEMENT wp;
		if (margin_select_wnd_.GetWindowPlacement(&wp))
		{
			CRect rect= wp.rcNormalPosition;
			rect.OffsetRect(0, 2);
			margin_select_wnd_.MoveWindow(rect, false);
		}
	}

	int margin_lo_metric= GetSelectedMargin(profile_margin_type_);
	SelectMarginButton(selected_margin_);
	UpdateSliderPos(margin_lo_metric);

	SetDlgItemText(IDC_LABEL_8, metric_ ? _T("厘米") : _T("英寸"));

	if (!pager_wnd_.Create(this, PAGER_ID))
	{
		EndDialog(IDCANCEL);
		return false;
	}
	else
	{
		WINDOWPLACEMENT wp;
		pages_wnd_.GetWindowPlacement(&wp);
		CRect rect= wp.rcNormalPosition;

		pages_wnd_.DestroyWindow();
		pages_wnd_.SetPadding(5, 10);
		pages_wnd_.Create("", 0, 0, 1, &pager_wnd_, IDC_PAGES);
		pager_wnd_.SetChild(pages_wnd_);
		pager_wnd_.SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

#if USE_PRINTER_LIST
	int count= printers_.GetCount();
	for (int i= 0; i < count; ++i)
		printers_wnd_.AddString(printers_.GetPrinterName(i));
#endif

	SelectPrinterInCombo(dlg_print_.m_pd.hDevNames);

	if (CWnd* wnd= GetDlgItem(IDC_PREVIEW))
	{
		WINDOWPLACEMENT wp;
		wnd->GetWindowPlacement(&wp);
		wnd->DestroyWindow();

		preview_wnd_.Create(this, wp.rcNormalPosition);
		preview_wnd_.SetDlgCtrlID(IDC_PREVIEW);
	}
	auto dpi = GetResolutionDpi();
	const int img_width= static_cast<int>(48/dpi.Width*96);
	VERIFY(::LoadImageList(img_list_layouts_, IDB_LAYOUTS, img_width, RGB(255,255,255), false));
	type_wnd_.SetIconSpacing(img_width + 30, 126 + 26);
	type_wnd_.SetImageList(&img_list_layouts_, LVSIL_NORMAL);

	static const TCHAR* labels[]=
	{
		_T("整页"),
		_T("双图像"),
		_T("三图像"),
		_T("四图像"),
		_T("钱夹打印"),
		_T("钱夹打印"),
		_T("缩略图"),
		0
	};
	int default_layout= profile_layout_type_;
	if (thumbnails_)
		default_layout = 6;
	for (int item= 0; labels[item]; ++item)
	{
		LVITEM li;
		memset(&li, 0, sizeof li);

		li.mask			= LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
		li.iItem		= item;
		li.iSubItem		= 0;
		li.state		= item == default_layout ? LVIS_SELECTED | LVIS_FOCUSED : 0;
		li.stateMask	= LVIS_SELECTED | LVIS_FOCUSED;
		li.pszText		= const_cast<TCHAR*>(labels[item]);
		li.cchTextMax	= 0;
		li.iImage		= item;
		li.lParam		= 0;
		li.iIndent		= 0;

		type_wnd_.InsertItem(&li);
	}

	// zoom gets reset by inserting items above
	dlg_photo_options_.SetZoom(profile_zoom_);

	SetTypeLayoutInfo();

	static const int commands[]= { ID_SHOW_MARGINS, ID_SHOW_PRINT_AREA, ID_SHOW_IMAGE_SPACE };
	switch_wnd_.AddButtons("xxx", commands, IDB_PRINT_DLG_TB);

	//units_wnd_.SubclassDlgItem(IDC_SYMBOLS, this);
	int cmd_id[]= { IDC_UNITS };
	units_wnd_.SetPadding(2, 6);
	units_wnd_.AddButtons("v", cmd_id, 0);

	print_thumbnails_->show_margins_ = print_photos_->show_margins_ = profile_show_margins_;
	print_thumbnails_->show_prinatable_area_ = print_photos_->show_prinatable_area_ = profile_show_prinatable_area_;
	print_thumbnails_->show_image_space_ = print_photos_->show_image_space_ = profile_show_img_space_;
	print_thumbnails_->SetPrintingOptions(RadioBtnToPrintOption(dlg_thumbs_options_.print_option_));

	print_photos_->SetNoOfPhotoCopies(dlg_photo_options_.copies_);

	VERIFY(UpdatePreviewParams());

	ResetNumberOfPages();

	OnFooter();

	//if (IsWhistlerLookAvailable())
	//	CreateGripWnd();	// create grip ctrl

	BuildResizingMap();

	SetWndResizing(IDC_PREVIEW, DlgAutoResize::RESIZE);
	SetWndResizing(IDC_LABEL_PAGE, DlgAutoResize::MOVE_V);
	SetWndResizing(IDC_UNITS, DlgAutoResize::MOVE);
	SetWndResizing(PAGER_ID, DlgAutoResize::MOVE_V_RESIZE_H);
	SetWndResizing(IDC_LABEL_4, DlgAutoResize::MOVE);
	SetWndResizing(IDC_SWITCH, DlgAutoResize::MOVE);
	SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_PRINTERS, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_PRN_SETTINGS, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_LAYOUT_INFO, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_TYPE, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_PAGE_SETUP, DlgAutoResize::MOVE_H);
	//SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE);
	SetWndResizing(IDC_ICM, DlgAutoResize::MOVE);
	SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
	SetWndResizing(IDOK, DlgAutoResize::MOVE);

	SetWndResizing(IDD_PRN_PAGE_RANGE, DlgAutoResize::MOVE_H);

//	SetWndResizing(IDC_FOOTER, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_LABEL_5, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_LABEL_6, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_LABEL_7, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_LABEL_8, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_DIVIDER_1, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_MARGIN_SELECT, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_MARGIN_SLIDER, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_MARGIN, DlgAutoResize::MOVE_H);
	SetWndResizing(IDC_MARGIN_SPIN, DlgAutoResize::MOVE_H);

	SetWndResizing(IDD_PRN_THUMBS_OPTIONS, DlgAutoResize::MOVE_H);
	SetWndResizing(IDD_PRN_PHOTO_OPTIONS, DlgAutoResize::MOVE_H);

	SetWndResizing(IDC_LABEL_SIZE, DlgAutoResize::MOVE_V_RESIZE_H);

	//if (IsWhistlerLookAvailable())
	//	dlg_resize_map_.SetWndResizing(IDC_GRIP, DlgAutoResize::MOVE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void PrintDlg::SelectMarginButton(int index)
{
	for (int i= 0; i < 5; ++i)
		margin_select_wnd_.HideButtonIdx(i, i != index);

	margin_select_wnd_.AutoResize();
}


void PrintDlg::SelectPrinterInCombo(HGLOBAL dev_names_handle)
{
	GlobalMemLock<DEVNAMES> dev_names(dev_names_handle);
	const TCHAR* printer_name= dev_names.AsTCharPtr() + dev_names->wDeviceOffset;
#if USE_PRINTER_LIST
	printers_wnd_.SelectString(0, printer_name);
#else
	printers_wnd_.SetWindowText(printer_name);
#endif
}


void PrintDlg::OnPrinterSettings()
{
	if (AfxGetApp()->DoPrintDialog(&dlg_print_) == IDOK)
	{
		SelectPrinterInCombo(dlg_print_.m_pd.hDevNames);
		UpdatePreviewParams();
		ResetNumberOfPages();
	}
}


bool PrintDlg::UpdatePreviewParams()
{
	CDC print_dc;
	if (!AfxGetApp()->CreatePrinterDC(print_dc))
//		if (!print_dc.Attach(dlg_print_.CreatePrinterDC()))
		return false;

	CSize page_size= GetPageSize(dlg_print_.m_pd.hDevMode);
	print_photos_->SetPageSize(page_size, margins_rect_);
	print_thumbnails_->SetPageSize(page_size, margins_rect_);

	CRect printable_rect= GetPrintableArea(print_dc);
	print_photos_->SetPrintableArea(printable_rect);
	print_thumbnails_->SetPrintableArea(printable_rect);

	if (preview_wnd_.m_hWnd)
		preview_wnd_.Invalidate();
//	preview_wnd_.SetPrinableArea(page_size);
//	preview_wnd_.SetPageSize(GetPageSize(dlg_print_.m_pd.hDevMode), margins_rect_);

	UpdatePageAndImageSizes();

	return true;
}


void PrintDlg::UpdatePageAndImageSizes()
{
	CSize page_size= GetPageSize(dlg_print_.m_pd.hDevMode);

	const double INCH= 254.0;
	const double CM= 100.0;
	oStringstream ost;

	if (page_size.cx > 0 && page_size.cy > 0 && print_ != 0)
	{
		ost.precision(3);
		ost << _T("页面尺寸: ");
		if (use_metric_units_for_info_)
			ost << page_size.cx / CM << _T(" x ") << page_size.cy / CM << _T(" [cm]");
		else
			ost << page_size.cx / INCH << _T(" x ") << page_size.cy / INCH << _T(" [in]");

		if (1)
		{
			CSize size= print_->GetImageSize();
			if (size.cx > 0 && size.cy > 0)
			{
				ost << _T("   图像尺寸: ");
				if (use_metric_units_for_info_)
					ost << size.cx / CM << _T(" x ") << size.cy / CM << _T(" [cm]");
				else
					ost << size.cx / INCH << _T(" x ") << size.cy / INCH << _T(" [in]");
			}
		}
	}

	size_label_wnd_.SetWindowText(ost.str().c_str());
}


void PrintDlg::OnPageSetup()
{
	dlg_page_setup_.DoModal();

}


void PrintDlg::OnColorSetup()
{
	try
	{
		ColorMngDlg dlg;

		dlg.monitor_viewer_ = new ICMProfile(g_Settings.viewer_.get());
		dlg.monitor_main_wnd_ = new ICMProfile(g_Settings.main_wnd_.get());
		if (g_Settings.default_printer_)
			dlg.default_printer_ = new ICMProfile(g_Settings.default_printer_.get());
		dlg.default_image_ = new ICMProfile(g_Settings.default_photo_.get());

		if (dlg.DoModal() == IDOK && dlg.changed_)
		{
			g_Settings.viewer_			= dlg.monitor_viewer_;
			g_Settings.main_wnd_		= dlg.monitor_main_wnd_;
			g_Settings.default_printer_	= dlg.default_printer_;
			g_Settings.default_photo_	= dlg.default_image_;

			profiles_changed_ = true;

			print_->SetPrinterProfile(g_Settings.default_printer_);
			print_->SetPhotoProfile(g_Settings.default_photo_);
		}
	}
	CATCH_ALL
}


CSize PrintDlg::GetPageSize(HGLOBAL dev_mode_handle)
{
	CSize size(0, 0);

	GlobalMemLock<DEVMODE> dev_mode(dev_mode_handle);

	if (dev_mode)
	{
		if (dev_mode->dmFields & DM_PAPERLENGTH && dev_mode->dmFields & DM_PAPERWIDTH)
		{
			size = CSize(dev_mode->dmPaperWidth, dev_mode->dmPaperLength);
		}
		else if (dev_mode->dmFields & DM_PAPERSIZE)
		{
#ifdef _UNICODE
  #define TOCHAR(a) a
#else
  #define TOCHAR(a) reinterpret_cast<CHAR*>(a)
#endif
// in ANSI build dmDeviceName is defined as BYTE instead of CHAR

			int size_len= ::DeviceCapabilities(TOCHAR(dev_mode->dmDeviceName), _T(""), DC_PAPERS, 0, dev_mode);
			if (size_len > 0)
			{
				std::vector<WORD> papers(size_len);
				std::vector<POINT> sizes(size_len);
				::DeviceCapabilities(TOCHAR(dev_mode->dmDeviceName), _T(""), DC_PAPERS, reinterpret_cast<TCHAR*>(&papers.front()), dev_mode);
				::DeviceCapabilities(TOCHAR(dev_mode->dmDeviceName), _T(""), DC_PAPERSIZE, reinterpret_cast<TCHAR*>(&sizes.front()), dev_mode);
#undef TOCHAR
				for (int i= 0; i < size_len; ++i)
					if (dev_mode->dmPaperSize == papers[i])
					{
						size = sizes[i];
						break;
					}
			}
		}

		if ((dev_mode->dmFields & DM_ORIENTATION) && dev_mode->dmOrientation == DMORIENT_LANDSCAPE)
		{
			if (size.cx < size.cy)	// swap w & h for the landscape mode
				size = CSize(size.cy, size.cx);
		}
	}

	return size;
}


bool AnalyzeRanges(const TCHAR* page_range, int page_count, std::vector<std::pair<int, int>>& ranges, String& msg)
{
	if (page_range == 0 || *page_range == 0)
	{
		msg = _T("Please enter comma separated pages or page ranges.");
		return false;
	}

	ParseRange parser(page_range);

	ParseRange::Status status= parser.Parse(ranges);

	if (status == ParseRange::OK)
		return true;

	msg = parser.GetErrMessage(status);

	return false;
}


void PrintDlg::OnOK()
{
	if (!UpdateData())
		return;

	if (dlg_print_.m_pd.hDevMode == 0 && dlg_print_.m_pd.hDevNames == 0)
		if (!dlg_print_.GetDefaults())
			return;

	CDC print_dc;
	if (!print_dc.Attach(dlg_print_.CreatePrinterDC()))
		return;

	CWaitCursor wait;

	print_dc.m_bPrinting = true;

	CSize page_size= GetPageSize(dlg_print_.m_pd.hDevMode);

	CRect dev_page_rect(CPoint(0, 0), page_size);

	// maybe...
//	print_dc.SetAbortProc(_AfxAbortProc);

	DOCINFO docInfo;
	memset(&docInfo, 0, sizeof docInfo);
	docInfo.cbSize = sizeof docInfo;
	docInfo.lpszDocName = _T("ExifPro--Print");
	CString port_name= dlg_print_.GetPortName();
	docInfo.lpszOutput = port_name;

	int page_count= print_->GetPageCount(static_cast<int>(selected_.size()));
	if (page_count == 0)
		return;

	// ranges of selected pages (one based: 1..n)
	std::vector<std::pair<int, int>> ranges;

	if (dlg_range_.page_range_ == 0)		// all pages?
	{
		ranges.push_back(std::make_pair(1, page_count));
	}
	else if (dlg_range_.page_range_ == 1)	// current page?
	{
		int cur_page= print_->GetCurPage() + 1;
		ranges.push_back(std::make_pair(cur_page, cur_page));
	}
	else	// selected pages
	{
		String msg;
		if (!AnalyzeRanges(dlg_range_.page_range_str_, page_count, ranges, msg))
		{
			new BalloonMsg(&dlg_range_.edit_page_range_, _T("Invalid Page Selection"), msg.c_str(), BalloonMsg::IERROR);
			return;
		}
	}

	// start document printing process
	if (print_dc.StartDoc(&docInfo) == SP_ERROR)
	{
		// error message
		AfxMessageBox(AFX_IDP_FAILED_TO_START_PRINT);
		return;
	}

	bool ok= true;

	int ranges_len= static_cast<int>(ranges.size());
	for (int range= 0; range < ranges_len; ++range)
	{
		int start_page= std::max(ranges[range].first - 1, 0);
		int last_page= std::min(ranges[range].second - 1, page_count - 1);

		for (int page= start_page; page <= last_page; ++page)
		{
			ASSERT(page >= 0 && page < page_count);

			print_dc.StartPage();

			print_->Print(print_dc, dev_page_rect, selected_, page);

			print_dc.EndPage();

			//TODO: break & progress
		}
	}

	if (ok)
		print_dc.EndDoc();
	else
		print_dc.AbortDoc();
/*
	profile_show_margins_ = print_.show_margins_;
	profile_show_prinatable_area_ = print_.show_prinatable_area_;
	int sel_type= GetSelectedType();
	if (sel_type >= 0)
		profile_layout_type_ = sel_type;
	profile_cur_page_ = print_.GetCurPage();
	profile_print_footer_ = !!print_footer_; */
	profile_page_range_ = dlg_range_.page_range_;
	profile_selected_pages_ = dlg_range_.page_range_str_;
//	if (margin_slider_wnd_.m_hWnd)
//		profile_margin_index_ = margin_slider_wnd_.GetPos();

	EndDialog(IDOK);

	if (profiles_changed_)
	{
		//TODO: refresh?

		// save changes
		g_Settings.Store();
	}
}

/*
bool PrintDlg::PrintPhotos(CDC& print_dc)
{
	if (selected_.empty())
		return false;

	CSize printable_size_size(print_dc.GetDeviceCaps(HORZRES), print_dc.GetDeviceCaps(VERTRES));
	CPoint phys_offset(print_dc.GetDeviceCaps(PHYSICALOFFSETX), print_dc.GetDeviceCaps(PHYSICALOFFSETY));
	CRect rect(phys_offset, printable_size_size);

	print_dc.SetMapMode(MM_LOMETRIC);
	CSize we= print_dc.GetWindowExt();
	CSize ve= print_dc.GetViewportExt();

	print_dc.SetMapMode(MM_ISOTROPIC);
	print_dc.SetWindowExt(we);
	ve.cy = -ve.cy;
	print_dc.SetViewportExt(ve);

	print_dc.DPtoLP(rect);

	int col= 10;
	int row= 10;
	int w= rect.Width() / (row + 1);
	int offset= w + w / (row - 1);
	CRect item_rect(CPoint(rect.left, rect.top), CSize(w, w));

	VectPhotoInfo::iterator it= selected_.begin();
	VectPhotoInfo::iterator end= selected_.end();

	for (int y= 0; y < col; ++y)
	{
		for (int x= 0; x < row; ++x)
		{
			(*it)->Draw(&print_dc, item_rect, 0, PhotoInfo::DRAW_HALFTONE);

			if (++it == end)
				break;

			item_rect.OffsetRect(offset, 0);
		}

		if (it == end)
			break;

		item_rect.OffsetRect(-row * offset, offset);
	}

	return true;
} */


int PrintDlg::GetSelectedType()
{
	if (type_wnd_.m_hWnd)
		return type_wnd_.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);

	return -1;
}


void PrintDlg::SelectNewType(Type layout)
{
	bool margin_changed= false;

	if (layout_type_ == THUMBNAILS && layout != THUMBNAILS)
	{
		profile_margins_thumbs_ = margins_rect_;
		margins_rect_ = profile_margins_;
		margin_changed = true;
	}
	else if (layout_type_ != THUMBNAILS && layout == THUMBNAILS)
	{
		profile_margins_ = margins_rect_;
		margins_rect_ = profile_margins_thumbs_;
		margin_changed = true;
	}

	layout_type_ = layout;

	print_ = layout_type_ == THUMBNAILS ? print_thumbnails_.get() : print_photos_.get();

	preview_wnd_.SetEngine(print_);

	if (margin_changed)
	{
		preview_wnd_.SetMargins(margins_rect_);
		UpdateSliderPos(GetSelectedMargin(selected_margin_));
	}

	switch (layout_type_)
	{
	case THUMBNAILS:
		preview_wnd_.SetItemsAcross(dlg_thumbs_options_.items_across_);
		profile_margins_thumbs_ = margins_rect_;
		OnFooter();
		break;
	case FULL_PAGE:
		preview_wnd_.SetItemsAcross(1);
		break;
	case TWO_PER_PAGE:
		preview_wnd_.SetItemsAcross(2);
		break;
	case THREE_PER_PAGE:
		preview_wnd_.SetItemsAcross(3);
		break;
	case FOUR_PER_PAGE:
		preview_wnd_.SetItemsAcross(4);
		break;
	case WALLET_PRINTS_LARGE:
		preview_wnd_.SetItemsAcross(9);
		break;
	case WALLET_PRINTS:
		preview_wnd_.SetItemsAcross(16); // ?
		break;
	default:
		ASSERT(false);
		break;
	}

	ResetNumberOfPages();
	SetTypeLayoutInfo();

	if (layout_type_ == THUMBNAILS)
	{
		dlg_thumbs_options_.EnableWindow();
		dlg_thumbs_options_.ShowWindow(SW_SHOWNA);

		dlg_photo_options_.ShowWindow(SW_HIDE);
		dlg_photo_options_.EnableWindow(false);
	}
	else
	{
		dlg_photo_options_.EnableWindow();
		dlg_photo_options_.ShowWindow(SW_SHOWNA);

		dlg_thumbs_options_.ShowWindow(SW_HIDE);
		dlg_thumbs_options_.EnableWindow(false);
	}

	UpdatePageAndImageSizes();
}


void PrintDlg::OnTypeSelected(NMHDR* nmhdr, LRESULT* result)
{
	NM_LISTVIEW* NM_list_view = reinterpret_cast<NM_LISTVIEW*>(nmhdr);
	*result = 0;
	bool reset_zoom= true;

	if (type_wnd_.m_hWnd && preview_wnd_.m_hWnd)
	{
		switch (GetSelectedType())
		{
		case -1:	// no selection
			return;

		case 0:
			SelectNewType(FULL_PAGE);
			break;
		case 1:
			SelectNewType(TWO_PER_PAGE);
			break;
		case 2:
			SelectNewType(THREE_PER_PAGE);
			break;
		case 3:
			SelectNewType(FOUR_PER_PAGE);
			break;
		case 4:
			SelectNewType(WALLET_PRINTS_LARGE);
			break;
		case 5:
			SelectNewType(WALLET_PRINTS);
			break;
		case 6:
			SelectNewType(THUMBNAILS);
			reset_zoom = false;
			break;
		}
//		preview_wnd_.SetItemsAcross(items);
	}

	// reset zoom?
	if (reset_zoom)
		dlg_photo_options_.SetZoom(100);
}


void PrintDlg::ResetNumberOfPages()
{
	if (int page_count= print_->GetPageCount(static_cast<int>(selected_.size())))
	{
		if (print_->GetCurPage() >= page_count)
			print_->SetCurPage(page_count - 1);
		SetPageToolBar(page_count);

		int show_cmd= page_count > 1 ? SW_SHOWNA : SW_HIDE;

		if (dlg_range_.m_hWnd)
			dlg_range_.ShowWindow(show_cmd);

		if (page_label_wnd_.m_hWnd)
			page_label_wnd_.ShowWindow(show_cmd);

		if (pages_wnd_.m_hWnd)
			pages_wnd_.ShowWindow(show_cmd);
	}
}


CRect PrintDlg::GetPrintableArea(CDC& print_dc)
{
	CSize printable_size_size(print_dc.GetDeviceCaps(HORZRES), print_dc.GetDeviceCaps(VERTRES));
	CPoint phys_offset(print_dc.GetDeviceCaps(PHYSICALOFFSETX), print_dc.GetDeviceCaps(PHYSICALOFFSETY));
	CRect rect(phys_offset, printable_size_size);

	print_dc.SetMapMode(MM_LOMETRIC);
	CSize we= print_dc.GetWindowExt();
	CSize ve= print_dc.GetViewportExt();

	print_dc.SetMapMode(MM_ISOTROPIC);
	print_dc.SetWindowExt(we);
	ve.cy = -ve.cy;
	print_dc.SetViewportExt(ve);

	print_dc.DPtoLP(rect);

	return rect;
}


void PrintDlg::OnShowMargins()
{
	print_photos_->show_margins_ = !print_photos_->show_margins_;
	print_thumbnails_->show_margins_ = print_photos_->show_margins_;
	preview_wnd_.Invalidate();
}

void PrintDlg::OnUpdateShowMargins(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(print_photos_->show_margins_ ? 1 : 0);
}


void PrintDlg::OnShowPrintArea()
{
	print_photos_->show_prinatable_area_ = !print_photos_->show_prinatable_area_;
	print_thumbnails_->show_prinatable_area_ = print_photos_->show_prinatable_area_;
	preview_wnd_.Invalidate();
}

void PrintDlg::OnUpdateShowPrintArea(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(print_photos_->show_prinatable_area_ ? 1 : 0);
}


void PrintDlg::OnShowImgSpace()
{
	print_photos_->show_image_space_ = !print_photos_->show_image_space_;
	print_thumbnails_->show_image_space_ = print_photos_->show_image_space_;
	preview_wnd_.Invalidate();
}

void PrintDlg::OnUpdateShowImgSpace(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(print_photos_->show_image_space_ ? 1 : 0);
}


BOOL PrintDlg::ContinueModal()
{
//	SendMessageToDescendants(WM_IDLEUPDATECMDUI, TRUE, 0, TRUE, TRUE);
	pages_wnd_.SendMessage(WM_IDLEUPDATECMDUI, true);
	switch_wnd_.SendMessage(WM_IDLEUPDATECMDUI, true);
	return DialogChild::ContinueModal();
}


BOOL PrintDlg::IsFrameWnd() const
{
	return true;
}


void PrintDlg::SetPageToolBar(int page_count)
{
	int count= pages_wnd_.GetButtonCount();

	if (count == page_count)
		return;

	if (count > page_count)
	{
		while (count > page_count)
			pages_wnd_.CToolBarCtrl::DeleteButton(--count);

		pages_wnd_.AutoResize();
	}
	else
	{
		int add= page_count - count;
		std::vector<TCHAR> text_buf(add * 5);
		TCHAR* text= &text_buf.front();

		for (int i= count; i < page_count; ++i)
		{
			text = _itot(i + 1, text, 10);
			text += _tcslen(text) + 1;
		}

		pages_wnd_.AddButtons(add, ID_SELECT_PAGE_0 + count, &text_buf.front(), BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT | BTNS_CHECK);
	}

	CRect tb_rect;
	pages_wnd_.GetWindowRect(tb_rect);
	pager_wnd_.SetSize(tb_rect.Size());
}


void PrintDlg::OnSelectPage(UINT cmd)
{
	print_->SetCurPage(cmd - ID_SELECT_PAGE_0);
	preview_wnd_.Invalidate();
}

void PrintDlg::OnUpdateSelectPage(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(cmd_ui->m_nID - ID_SELECT_PAGE_0 == print_->GetCurPage() ? 1 : 0);
}


void PrintDlg::OnChangeFooterText()
{
	if (dlg_thumbs_options_.edit_footer_text_.m_hWnd != 0)
	{
		dlg_thumbs_options_.edit_footer_text_.GetWindowText(dlg_thumbs_options_.footer_text_);

		if (dlg_thumbs_options_.IsDlgButtonChecked(IDC_FOOTER_T))
			OnFooter();
	}
}

void PrintDlg::OnFooter()
{
	bool print_page_no= !!dlg_thumbs_options_.IsDlgButtonChecked(IDC_FOOTER);
	bool print_footer_text= !!dlg_thumbs_options_.IsDlgButtonChecked(IDC_FOOTER_T);

	dlg_thumbs_options_.print_footer_ = print_page_no;
	dlg_thumbs_options_.print_footer_text_ = print_footer_text;

	print_->print_page_number_ = print_page_no;
	if (print_footer_text)
	{
		CString str;
		dlg_thumbs_options_.edit_footer_text_.GetWindowText(str);
		print_->footer_ = str;
	}
	else
		print_->footer_ = folder_path_;

/*	if (dlg_thumbs_options_.IsDlgButtonChecked(IDC_FOOTER))
	{
		dlg_thumbs_options_.print_footer_ = true;
		print_->print_page_number_ = true;
		print_->footer_ = folder_path_;
	}
	else
	{
		dlg_thumbs_options_.print_footer_ = false;
		print_->print_page_number_ = false;
		print_->footer_.erase();
	} */

	if (preview_wnd_.m_hWnd)
		preview_wnd_.Invalidate();
}


void PrintDlg::OnSelChangePrinters()
{
#if USE_PRINTER_LIST
	if (printers_wnd_.m_hWnd)
		SelectPrinter(printers_wnd_.GetCurSel());
#endif
}


void PrintDlg::SelectPrinter(int printer_index)
{
	const TCHAR* name= printers_.GetPrinterName(printer_index);
	const TCHAR* port= _T(""); //printers_.GetPortName(printer_index);
#if 1
	if (name && port)
	{
		int name_size= static_cast<int>(_tcslen(name));
		int port_size= static_cast<int>(_tcslen(port));
		//port
		//DEVNAMES
		int size= sizeof(DEVNAMES) + sizeof(TCHAR) * (name_size + port_size + 4);
		HGLOBAL mem= ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size);
		if (mem == 0)
		{
			ASSERT(false);
			return;
		}
		else
		{
			GlobalMemLock<DEVNAMES> dev_names(mem);
			DEVNAMES* names= dev_names;
			TCHAR* strings= reinterpret_cast<TCHAR*>(names + 1);

			names->wDriverOffset = static_cast<WORD>(strings - dev_names.AsTCharPtr());
			strings++;

			names->wDeviceOffset = static_cast<WORD>(strings - dev_names.AsTCharPtr());
			_tcscpy(strings, name);
			strings += name_size + 1;

			names->wOutputOffset = static_cast<WORD>(strings - dev_names.AsTCharPtr());
			_tcscpy(strings, port);
			strings += port_size + 1;

			names->wDefault = static_cast<WORD>(strings - dev_names.AsTCharPtr());
		}

//		::GlobalFree(dlg_print_.m_pd.dev_names);
		dlg_print_.m_pd.hDevNames = mem;
		AfxGetApp()->SelectPrinter(mem, dlg_print_.m_pd.hDevMode);
/*
		PRINTDLG pdlg= dlg_print_.m_pd;
		pdlg.hDevNames = mem;

		AfxGetApp()->SelectPrinter(mem, 0);

		if (!AfxGetApp()->GetPrinterDeviceDefaults(&pdlg) || pdlg.hDevNames == 0)
		{
			::GlobalFree(mem);
			return;
		}

		::GlobalFree(dlg_print_.m_pd.hDevNames);
		::GlobalFree(dlg_print_.m_pd.hDevMode);

		dlg_print_.m_pd = pdlg;
*/
		UpdatePreviewParams();
	}
#endif
}


/*
HRESULT DisplayPrintPropertySheet()
{
HRESULT result;
LPPRINTDLGEX PDX = NULL;
LPPRINTPAGERANGE page_ranges = NULL;

// Allocate the PRINTDLGEX structure.

PDX = (LPPRINTDLGEX)GlobalAlloc(GPTR, sizeof(PRINTDLGEX));
if (!PDX)
    return E_OUTOFMEMORY;

// Allocate an array of PRINTPAGERANGE structures.

page_ranges = (LPPRINTPAGERANGE) GlobalAlloc(GPTR, 
                   10 * sizeof(PRINTPAGERANGE));
if (!page_ranges)
    return E_OUTOFMEMORY;

//  Initialize the PRINTDLGEX structure.

PDX->struct_size = sizeof(PRINTDLGEX);
PDX->owner = m_hWnd;
PDX->hDevMode = NULL;
PDX->hDevNames = NULL;
PDX->dc = NULL;
PDX->Flags = PD_RETURNDC | PD_COLLATE;
PDX->Flags2 = 0;
PDX->ExclusionFlags = 0;
PDX->page_ranges = 0;
PDX->max_page_ranges = 10;
PDX->page_ranges = page_ranges;
PDX->min_page = 1;
PDX->max_page = 1000;
PDX->copies = 1;
PDX->instance = 0;
PDX->print_template_name = NULL;
PDX->callback = NULL;
PDX->property_pages = 0;
PDX->lphPropertyPages = NULL;
PDX->start_page = START_PAGE_GENERAL;
PDX->result_action = 0;

//  Invoke the Print property sheet.

result = PrintDlgEx(PDX);

if ( (result == S_OK) &&
           PDX->result_action == PD_RESULT_PRINT) {

    // User clicked the Print button, so
    // use the DC and other information returned in the 
    // PRINTDLGEX structure to print the document

}

if (PDX->dc != NULL)
    DeleteDC(PDX->dc);
if (PDX->hDevMode != NULL)
    GlobalFree(PDX->hDevMode);
if (PDX->hDevNames != NULL)
    GlobalFree(PDX->hDevNames);

return result;
}
*/


void PrintDlg::OnDestroy()
{
	// store only one engine's settings: they should be in sync
	profile_show_margins_ = print_photos_->show_margins_;
	profile_show_prinatable_area_ = print_photos_->show_prinatable_area_;
	profile_show_img_space_ = print_photos_->show_image_space_;

	int sel_type= GetSelectedType();
	if (sel_type >= 0)
		profile_layout_type_ = sel_type;

	profile_cur_page_			= print_photos_->GetCurPage();
	profile_cur_page_thumb_		= print_thumbnails_->GetCurPage();
	profile_print_footer_		= !!dlg_thumbs_options_.print_footer_;
	profile_print_footer_text_	= !!dlg_thumbs_options_.print_footer_text_;
	profile_print_option_		= dlg_thumbs_options_.print_option_;
	profile_thumbs_items_across_= dlg_thumbs_options_.items_across_;
	profile_footer_text_		= dlg_thumbs_options_.footer_text_;
	profile_photo_copies_		= dlg_photo_options_.copies_;
	profile_zoom_				= dlg_photo_options_.zoom_;
	profile_margin_type_		= selected_margin_;
	profile_info_units_			= use_metric_units_for_info_;
	profile_font_				= dlg_thumbs_options_.font_;

	if (layout_type_ == THUMBNAILS)
		profile_margins_thumbs_ = margins_rect_;
	else
		profile_margins_ = margins_rect_;

	if (dlg_print_.m_pd.hDevMode && dlg_print_.m_pd.hDevNames)
		SavePrinterSelection(REG_PRINTER_CFG, dlg_print_.m_pd.hDevMode, dlg_print_.m_pd.hDevNames);

	DialogChild::OnDestroy();
}


void PrintDlg::OnChangePagesBox()
{
	// if user is typing here, change range radio btn to point to the selection
	if (!IsDlgButtonChecked(IDC_SELECTED_PAGES))
		CheckRadioButton(IDC_ALL_PAGES, IDC_SELECTED_PAGES, IDC_SELECTED_PAGES);
}


void PrintDlg::OnTbUnitsDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	CMenu menu;
	if (!menu.LoadMenu(IDR_UNITS))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != NULL);
	if (popup == 0)
		return;

	CRect rect;
	units_wnd_.GetWindowRect(rect);
	CPoint pos(rect.right, rect.bottom);

	popup->CheckMenuRadioItem(ID_CM, ID_INCH, use_metric_units_for_info_ ? ID_CM : ID_INCH, MF_BYCOMMAND);

	int cmd= popup->TrackPopupMenu(TPM_RIGHTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this);

	if (cmd == ID_CM)
		use_metric_units_for_info_ = true;
	else if (cmd == ID_INCH)
		use_metric_units_for_info_ = false;
	else
		return;

	UpdatePageAndImageSizes();
}


void PrintDlg::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

//	switch (info_tip->iItem)
//	{
//	case ID_VIEW_MODE:		// view mode
//		if (view_)
//	}

	CMenu menu;
	if (!menu.LoadMenu(IDR_MARGINS))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != NULL);
	if (popup == 0)
		return;

	CRect rect;
	margin_select_wnd_.GetWindowRect(rect);
	CPoint pos(rect.left, rect.bottom);

	int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this);
	int margin= -1;

	switch (cmd)
	{
	case ID_ALL:
		SelectMarginButton(selected_margin_ = ALL_MARGINS);
		MarginChanged();
		break;
	case ID_LEFT:
		SelectMarginButton(selected_margin_ = LEFT_MARGIN);
		margin = margins_rect_.left;
		break;
	case ID_RIGHT:
		SelectMarginButton(selected_margin_ = RIGHT_MARGIN);
		margin = margins_rect_.right;
		break;
	case ID_TOP:
		SelectMarginButton(selected_margin_ = TOP_MARGIN);
		margin = margins_rect_.top;
		break;
	case ID_BOTTOM:
		SelectMarginButton(selected_margin_ = BOTTOM_MARGIN);
		margin = margins_rect_.bottom;
		break;

	default:
		return;
	}

	if (margin != -1)
		UpdateSliderPos(margin);
}


void PrintDlg::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (scroll_bar == 0)
		return;

	if (margin_slider_wnd_.m_hWnd == scroll_bar->m_hWnd)
		SetMargin(margin_slider_wnd_.GetPos());
//	else if (dlg_thumbs_options_.slider_wnd_.m_hWnd == scroll_bar->m_hWnd)
//		preview_wnd_.SetItemsAcross(dlg_thumbs_options_.items_across_);
}


void PrintDlg::SetMargin(int index)
{
	SetMargin(MarginIndexToAbsValue(index));
}


const double MAX_MARGIN= 10.0;

double PrintDlg::SetMargin(double val)
{
	if (val < 0.0)
		val = 0.0;
	if (val > MAX_MARGIN)
		val = MAX_MARGIN;
	oStringstream ost;
	ost.precision(3);
	ost << val;
	edit_margin_.SetWindowText(ost.str().c_str());
	int end= static_cast<int>(ost.str().size());
	edit_margin_.SetSel(end, end);
	return val;
}


double PrintDlg::GetMargin()
{
	CString val;
	edit_margin_.GetWindowText(val);
	TCHAR* end= 0;
	return _tcstod(val, &end);
}


int PrintDlg::GetSelectedMargin(int selected)
{
	int margin_lo_metric= 0;

	switch (selected)
	{
	case 1:
		selected_margin_ = LEFT_MARGIN;
		margin_lo_metric = margins_rect_.left;
		break;
	case 2:
		selected_margin_ = RIGHT_MARGIN;
		margin_lo_metric = margins_rect_.right;
		break;
	case 3:
		selected_margin_ = TOP_MARGIN;
		margin_lo_metric = margins_rect_.top;
		break;
	case 4:
		selected_margin_ = BOTTOM_MARGIN;
		margin_lo_metric = margins_rect_.bottom;
		break;
	case 0:
	default:
		selected_margin_ = ALL_MARGINS;
		margin_lo_metric = margins_rect_.left;
		break;
	}

	return margin_lo_metric;
}


void PrintDlg::UpdateSliderPos(int lo_metric)
{
	UpdateSliderPos(SetMargin(lo_metric / (metric_ ? 100.0 : 254.0)));
}

void PrintDlg::UpdateSliderPos(double val)
{
	int pos= static_cast<int>(val * RANGE / MAX_MARGIN);
	margin_slider_wnd_.SetPos(pos);
}


double PrintDlg::MarginIndexToAbsValue(int index)
{
	if (index <= 0)
		return 0.0;

	if (index > RANGE)
		index = RANGE;

	return MAX_MARGIN * index / RANGE;
}


void PrintDlg::OnDeltaPosMarginSpin(NMHDR* nmhdr, LRESULT* result)
{
	NM_UPDOWN* up_down= reinterpret_cast<NM_UPDOWN*>(nmhdr);
	*result = 0;

	up_down->iPos = 3000;
	if (up_down->iDelta)
	{
		double val= SetMargin(GetMargin() + (up_down->iDelta > 0 ? 0.1 : -0.1));
		UpdateSliderPos(val);
	}
}


void PrintDlg::OnChangeMargin()
{
	MarginChanged();
}


void PrintDlg::MarginChanged()
{
	if (edit_margin_.m_hWnd == 0)
		return;

	double margin= GetMargin();
	int margin_val= 0;

	if (metric_)
		margin_val = static_cast<int>(margin * 100.0);
	else
		margin_val = static_cast<int>(margin * 254.0);

	CRect margins_rect= margins_rect_;

	switch (selected_margin_)
	{
	case ALL_MARGINS:
		margins_rect.SetRect(margin_val, margin_val, margin_val, margin_val);
		break;
	case LEFT_MARGIN:
		margins_rect.left = margin_val;
		break;
	case RIGHT_MARGIN:
		margins_rect.right = margin_val;
		break;
	case TOP_MARGIN:
		margins_rect.top = margin_val;
		break;
	case BOTTOM_MARGIN:
		margins_rect.bottom = margin_val;
		break;
	}

	if (margins_rect != margins_rect_)
	{
		margins_rect_ = margins_rect;
		preview_wnd_.SetMargins(margins_rect_);
		UpdatePageAndImageSizes();
	}
}


void PrintDlg::SetTypeLayoutInfo()
{
	const TCHAR* info= 0;

	switch (GetSelectedType())
	{
	case 0:
		info = _T("图像缩放至适合整页.");
		break;
	case 1:
		info = _T("每页两张图像.");
		break;
	case 2:
		info = _T("每页三张图像.");
		break;
	case 3:
		info = _T("每页四张图像.");
		break;
	case 4:
		info = _T("钱夹打印 (大).");
		break;
	case 5:
		info = _T("钱夹打印.");
		break;
	case 6:
		info = _T("缩略图.");
		break;
	default:
		ASSERT(false);
		break;
	}

	if (info && layout_info_wnd_.m_hWnd)
		layout_info_wnd_.SetWindowText(info);
}


LRESULT PrintDlg::OnFontChanged(WPARAM, LPARAM)
{
//	preview_wnd_.SetItemsAcross(items);
	print_->SetDefaultFont(dlg_thumbs_options_.font_);
	preview_wnd_.Invalidate();
	return 0;
}


LRESULT PrintDlg::OnItemsNumberChanged(WPARAM items, LPARAM)
{
	preview_wnd_.SetItemsAcross(static_cast<int>(items));
	ResetNumberOfPages();
	UpdatePageAndImageSizes();
	return 0;
}


LRESULT PrintDlg::OnNumberOfCopiesChanged(WPARAM copies, LPARAM)
{
	ASSERT(print_photos_.get() == print_);

	print_photos_->SetNoOfPhotoCopies(static_cast<int>(copies));

	if (preview_wnd_.m_hWnd)
		preview_wnd_.Invalidate();

	ResetNumberOfPages();

	return 0;
}


LRESULT PrintDlg::OnZoomChanged(WPARAM zoom, LPARAM)
{
	ASSERT(print_photos_.get() == print_);
	if (print_photos_.get() != print_)
		return 0;

	print_photos_->SetZoom(zoom / 100.0);

	if (preview_wnd_.m_hWnd)
		preview_wnd_.Invalidate();

	return 0;
}


void PrintDlg::Resize()
{
	DialogChild::Resize();
	if (size_label_wnd_.m_hWnd)
		size_label_wnd_.Invalidate();
}


LRESULT PrintDlg::OnPrintOptionChanged(WPARAM opt, LPARAM)
{
	if (print_)
		print_->SetPrintingOptions(RadioBtnToPrintOption(static_cast<int>(opt)));

	if (preview_wnd_.m_hWnd)
		preview_wnd_.Invalidate();

	return 0;
}
