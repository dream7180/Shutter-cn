/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// GenHTMLAlbumDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "GenHTMLAlbumDlg.h"
#include "ProfileVector.h"
#include "BalloonMsg.h"
#include "RString.h"
#include "Path.h"
#include "CatchAll.h"
#include "FolderSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


namespace {
	const TCHAR* REGISTRY_ENTRY_HTML	= _T("HTMLAlbumGen");
	const TCHAR* REG_ALBUM_TYPE			= _T("Type");
	const TCHAR* REG_COLOR_PAGE			= _T("PageColor");
	const TCHAR* REG_COLOR_TEXT			= _T("TextColor");
	const TCHAR* REG_COLOR_PREVIEW		= _T("PreviewColor");
	const TCHAR* REG_THUMB_WIDTH		= _T("ThumbWidth");
	const TCHAR* REG_THUMB_HEIGHT		= _T("ThumbHeight");
	const TCHAR* REG_ROOT_DIR			= _T("RootDir");
	const TCHAR* REG_PHOTOS_DIR			= _T("PhotosDir");
	const TCHAR* REG_THUMBS_DIR			= _T("ThumbsDir");
	const TCHAR* REG_CUSTOM_COLORS		= _T("CustomColors");
	const TCHAR* REG_QUALITY			= _T("Quality");
	const TCHAR* REG_JPEG_QUALITY		= _T("Compression");
	const TCHAR* REG_FILE_NAME			= _T("PageFileName");
	const TCHAR* REG_PAGE_TITLE			= _T("PageTitle");
	const TCHAR* REG_ITEM_TEXT_TYPE		= _T("ItemTextType");
	const TCHAR* REG_GRID_COLUMNS		= _T("GridColumns");
	const TCHAR* REG_IMG_TEXT_1			= _T("ImgTextItem1");
	const TCHAR* REG_IMG_TEXT_2			= _T("ImgTextItem2");
	const TCHAR* REG_IMG_TEXT_3			= _T("ImgTextItem3");
	const TCHAR* REG_IMG_TEXT_4			= _T("ImgTextItem4");
	const TCHAR* REG_OPEN_BROWSER		= _T("OpenBrowser");
}

/////////////////////////////////////////////////////////////////////////////
// GenHTMLAlbumDlg dialog


GenHTMLAlbumDlg::GenHTMLAlbumDlg(CWnd* parent, double ratio, OrientationOfImages orientation)
	: ResizeDlg(parent, ratio, orientation, GenHTMLAlbumDlg::IDD)
{
	//{{AFX_DATA_INIT(GenHTMLAlbumDlg)
	type_ = 1;
	grid_columns_ = 6;
	grid_rows_ = 1;
	root_dir_ = _T("c:\\HTML Album");
	thumb_height_ = 100;
	thumb_width_ = 100;
	photos_dir_ = _T("");
	thumbs_dir_ = _T("");
	//}}AFX_DATA_INIT
	page_title_ = _T("Page Title");
	page_file_name_ = _T("page.html");
	item_text_type_ = 0;
	text_1_ = text_2_ = text_3_ = text_4_ = false;
	open_in_browser_ = true;

	rgb_page_backgnd_ = RGB(0,0,0);
	rgb_page_text_ = RGB(255,255,255);
	rgb_preview_backgnd_ = RGB(0,0,0);

	GetProfileVector(REGISTRY_ENTRY_HTML, REG_CUSTOM_COLORS, custom_colors_);
	custom_colors_.resize(CUSTOM_COLORS_SIZE);

	registry_ = REGISTRY_ENTRY_HTML;
	options_dlg_id_ = IDD_GEN_HTML_OPTIONS;

	dlg_options_.baseline_jpeg_		= true;
	dlg_options_.progressive_jpeg_		= true;
	dlg_options_.preserve_exif_block_	= false;
}


void GenHTMLAlbumDlg::DoDataExchange(CDataExchange* DX)
{
	ResizeDlg::DoDataExchange(DX);
	//{{AFX_DATA_MAP(GenHTMLAlbumDlg)
	DDX_Control(DX, IDC_ROOT_DIR, edit_root_);
	DDX_Control(DX, IDC_PHOTOS_DIR, edit_photos_);
	DDX_Control(DX, IDC_THUMBS_DIR, edit_thumbs_);

	DDX_Control(DX, IDC_THUMB_SPIN_2, thumb_spin2_wnd_);
	DDX_Control(DX, IDC_THUMB_SPIN_1, thumb_spin1_wnd_);
	DDX_Control(DX, IDC_GRID_SPIN_2, grid_spin2_wnd_);
	DDX_Control(DX, IDC_GRID_SPIN_1, grid_spin1_wnd_);
	DDX_Control(DX, IDC_COLOR_TEXT, btn_color_text_);
	DDX_Control(DX, IDC_COLOR_PREVIEW, btn_color_preview_);
	DDX_Control(DX, IDC_COLOR_PAGE, btn_color_page_);
	DDX_Control(DX, IDC_IMG_FRAMES, frames_wnd_);
	DDX_Control(DX, IDC_IMG_GRID, grid_wnd_);
	DDX_Radio(DX, IDC_TYPE_GRID, type_);
	DDX_Text(DX, IDC_GRID_COLS, grid_columns_);
	DDV_MinMaxInt(DX, grid_columns_, 1, 999);
	DDX_Text(DX, IDC_GRID_ROWS, grid_rows_);
	DDV_MinMaxInt(DX, grid_rows_, 1, 999);
	DDX_Text(DX, IDC_ROOT_DIR, root_dir_);
	DDV_MaxChars(DX, root_dir_, 250);
	DDX_Text(DX, IDC_THUMBNAIL_HEIGHT, thumb_height_);
	DDV_MinMaxInt(DX, thumb_height_, 1, 999);
	DDX_Text(DX, IDC_THUMBNAIL_WIDTH, thumb_width_);
	DDV_MinMaxInt(DX, thumb_width_, 1, 999);
	DDX_Text(DX, IDC_PHOTOS_DIR, photos_dir_);
	DDV_MaxChars(DX, photos_dir_, 250);
	DDX_Text(DX, IDC_THUMBS_DIR, thumbs_dir_);
	DDV_MaxChars(DX, thumbs_dir_, 250);
	DDX_Text(DX, IDC_TITLE, page_title_);
	DDX_Text(DX, IDC_FILENAME, page_file_name_);
	DDX_CBIndex(DX, IDC_ITEM_TEXT, item_text_type_);
	//}}AFX_DATA_MAP
	DDX_Control(DX, IDC_GRID_COLS, edit_grid_size_);
	DDX_Check(DX, IDC_TEXT_1, text_1_);
	DDX_Check(DX, IDC_TEXT_2, text_2_);
	DDX_Check(DX, IDC_TEXT_3, text_3_);
	DDX_Check(DX, IDC_TEXT_4, text_4_);

	if (dlg_options_.m_hWnd && DX->m_bSaveAndValidate)
		open_in_browser_ = dlg_options_.IsDlgButtonChecked(IDC_START_BROWSER) != 0;
}


BEGIN_MESSAGE_MAP(GenHTMLAlbumDlg, ResizeDlg)
	//{{AFX_MSG_MAP(GenHTMLAlbumDlg)
	ON_BN_CLICKED(IDC_COLOR_PAGE, OnColorPage)
	ON_BN_CLICKED(IDC_COLOR_PREVIEW, OnColorPreview)
	ON_BN_CLICKED(IDC_COLOR_TEXT, OnColorText)
	ON_BN_CLICKED(IDC_IMG_GRID, OnGridClicked)
	ON_BN_CLICKED(IDC_IMG_FRAMES, OnFramesClicked)
	ON_BN_CLICKED(IDC_TYPE_FRAMES, OnTypeChange)
	ON_BN_CLICKED(IDC_TYPE_GRID, OnTypeChange)
	ON_BN_CLICKED(IDC_BROWSE_2, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// GenHTMLAlbumDlg message handlers

BOOL GenHTMLAlbumDlg::OnInitDialog()
{
	CWinApp* app= AfxGetApp();
	type_				= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_ALBUM_TYPE, type_);
	rgb_page_backgnd_	= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_COLOR_PAGE, rgb_page_backgnd_);
	rgb_page_text_		= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_COLOR_TEXT, rgb_page_text_);
	rgb_preview_backgnd_	= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_COLOR_PREVIEW, rgb_preview_backgnd_);
	thumb_height_		= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_THUMB_HEIGHT, thumb_height_);
	thumb_width_		= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_THUMB_WIDTH, thumb_width_);
	root_dir_		= app->GetProfileString(REGISTRY_ENTRY_HTML, REG_ROOT_DIR, root_dir_);
	photos_dir_		= app->GetProfileString(REGISTRY_ENTRY_HTML, REG_PHOTOS_DIR, photos_dir_);
	thumbs_dir_		= app->GetProfileString(REGISTRY_ENTRY_HTML, REG_THUMBS_DIR, thumbs_dir_);
	page_file_name_	= app->GetProfileString(REGISTRY_ENTRY_HTML, REG_FILE_NAME, page_file_name_);
	page_title_		= app->GetProfileString(REGISTRY_ENTRY_HTML, REG_PAGE_TITLE, page_title_);
	item_text_type_		= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_ITEM_TEXT_TYPE, item_text_type_);
	grid_columns_		= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_GRID_COLUMNS, grid_columns_);
	text_1_			= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_1, text_1_);
	text_2_			= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_2, text_2_);
	text_3_			= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_3, text_3_);
	text_4_			= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_4, text_4_);
	open_in_browser_	= app->GetProfileInt(REGISTRY_ENTRY_HTML, REG_OPEN_BROWSER, open_in_browser_);

	ResizeDlg::OnInitDialog();

	SubclassHelpBtn(_T("ToolHTML.htm"));

	thumb_spin1_wnd_.SetRange(1, 999);
	thumb_spin2_wnd_.SetRange(1, 999);
	grid_spin1_wnd_.SetRange(1, 999);
	grid_spin2_wnd_.SetRange(1, 999);

	edit_photos_.FileNameEditing(true);
	edit_thumbs_.FileNameEditing(true);

	btn_color_text_.SetColor(rgb_page_text_);
	btn_color_page_.SetColor(rgb_page_backgnd_);
	btn_color_preview_.SetColor(rgb_preview_backgnd_);

	MapColors();

	OnTypeChange();

	dlg_options_.CheckDlgButton(IDC_START_BROWSER, open_in_browser_);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void GenHTMLAlbumDlg::MapColors()
{
	MapColor(IDB_TEMPLATE_GRID, grid_bmp_, grid_wnd_);
	MapColor(IDB_TEMPLATE_FRAMES, frames_bmp_, frames_wnd_);
}


void GenHTMLAlbumDlg::MapColor(int id, CBitmap& bmp, CStatic& wnd)
{
	COLORMAP colors[]=
	{
		{ RGB(192,192,192), ::GetSysColor(COLOR_3DFACE) },
		{ RGB(0,0,255), ::GetSysColor(COLOR_ACTIVECAPTION) },	// window frame
		{ RGB(255,0,0), RGB(160,160,160) },					// thumbnails
		{ RGB(0,255,0), rgb_preview_backgnd_ },				// preview background
		{ RGB(0,255,255), rgb_page_backgnd_ },				// page background
		{ RGB(0,0,0), rgb_page_text_ },						// page text
	};

	if (bmp.m_hObject)
		bmp.DeleteObject();

	if (bmp.LoadMappedBitmap(id, 0, colors, array_count(colors)))
		wnd.SetBitmap(bmp);
}


void GenHTMLAlbumDlg::OnColorPage()
{
	OnSelColor(rgb_page_backgnd_, btn_color_page_);
	MapColors();
}

void GenHTMLAlbumDlg::OnColorPreview()
{
	OnSelColor(rgb_preview_backgnd_, btn_color_preview_);
	MapColors();
}

void GenHTMLAlbumDlg::OnColorText()
{
	OnSelColor(rgb_page_text_, btn_color_text_);
	MapColors();
}


void GenHTMLAlbumDlg::OnGridClicked()
{
	CheckRadioButton(IDC_TYPE_FRAMES, IDC_TYPE_GRID, IDC_TYPE_GRID);
	OnTypeChange();
}

void GenHTMLAlbumDlg::OnFramesClicked()
{
	CheckRadioButton(IDC_TYPE_FRAMES, IDC_TYPE_GRID, IDC_TYPE_FRAMES);
	OnTypeChange();
}


void GenHTMLAlbumDlg::OnTypeChange()
{
	if (edit_grid_size_.m_hWnd)
	{
		bool read_only= GetCheckedRadioButton(IDC_TYPE_FRAMES, IDC_TYPE_GRID) == IDC_TYPE_FRAMES;
		edit_grid_size_.SetReadOnly(read_only);
		grid_spin1_wnd_.EnableWindow(!read_only);
	}
}


void GenHTMLAlbumDlg::OnSelColor(COLORREF& rgb_color, ColorButton& btn)
{
	custom_colors_.resize(CUSTOM_COLORS_SIZE);

	// restore custom colors
	memcpy(CColorDialog::GetSavedCustomColors(), &custom_colors_.front(), CUSTOM_COLORS_SIZE * sizeof COLORREF);

	CColorDialog dlg(rgb_color, CC_FULLOPEN, this);

	if (dlg.DoModal() == IDOK && rgb_color != dlg.GetColor())
	{
		rgb_color = dlg.GetColor();
		btn.SetColor(rgb_color);
	}

	// store custom colors
	memcpy(&custom_colors_.front(), CColorDialog::GetSavedCustomColors(), CUSTOM_COLORS_SIZE * sizeof COLORREF);
}


void GenHTMLAlbumDlg::OnOK()
{
	if (!Finish())
		return;

//	if (!UpdateData(TRUE))
//		return;

	try
	{
		Path root= root_dir_;

		if (!root.CreateIfDoesntExist(GetDlgItem(IDC_ROOT_DIR)))
			return;

		DWORD attrib= ::GetFileAttributes(root_dir_);
		if (attrib == INVALID_FILE_ATTRIBUTES || (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			new BalloonMsg(GetDlgItem(IDC_ROOT_DIR), _T("Valid Directory Reguired"),
				_T("Please select an existing directory."), BalloonMsg::IERROR);
			return;
		}

		if (page_file_name_.IsEmpty())
		{
			new BalloonMsg(GetDlgItem(IDC_FILENAME), _T("Page File Name Reguired"),
				_T("Please enter the name of an HTML destination file."), BalloonMsg::IERROR);
			return;
		}
	}
	CATCH_ALL

	CWinApp* app= AfxGetApp();
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_ALBUM_TYPE, type_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_COLOR_PAGE, rgb_page_backgnd_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_COLOR_TEXT, rgb_page_text_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_COLOR_PREVIEW, rgb_preview_backgnd_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_THUMB_HEIGHT, thumb_height_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_THUMB_WIDTH, thumb_width_);
	app->WriteProfileString(REGISTRY_ENTRY_HTML, REG_ROOT_DIR, root_dir_);
	app->WriteProfileString(REGISTRY_ENTRY_HTML, REG_PHOTOS_DIR, photos_dir_);
	app->WriteProfileString(REGISTRY_ENTRY_HTML, REG_THUMBS_DIR, thumbs_dir_);
	WriteProfileVector(REGISTRY_ENTRY_HTML, REG_CUSTOM_COLORS, custom_colors_);

	app->WriteProfileString(REGISTRY_ENTRY_HTML, REG_FILE_NAME, page_file_name_);
	app->WriteProfileString(REGISTRY_ENTRY_HTML, REG_PAGE_TITLE, page_title_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_ITEM_TEXT_TYPE, item_text_type_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_GRID_COLUMNS, grid_columns_);

	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_1, text_1_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_2, text_2_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_3, text_3_);
	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_IMG_TEXT_4, text_4_);

	app->WriteProfileInt(REGISTRY_ENTRY_HTML, REG_OPEN_BROWSER, open_in_browser_);

	EndDialog(IDOK);
}


int GenHTMLAlbumDlg::TranslateSizeSlider(int pos)
{
	return pos - 1;
}


void GenHTMLAlbumDlg::OnBrowse()
{
	CString path;
	edit_root_.GetWindowText(path);

	if (path.IsEmpty())
		path = _T("c:\\");

	CFolderSelect fs(this);
	CString selected= fs.DoSelectPath(RString(IDS_SELECT_OUTPUT_FOLDER), path);

	if (selected.IsEmpty())
		return;

	edit_root_.SetWindowText(selected);
}
