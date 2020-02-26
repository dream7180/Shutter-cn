/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageManipulationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImageManipulationDlg.h"
#include "PhotoInfo.h"
#include "CatchAll.h"
#include "JPEGEncoder.h"
#include "FileDataDestination.h"
#include "RString.h"
#include "ImgLevelsPage.h"
#include "ImgBrightnessPage.h"
#include "ImgColorBalancePage.h"
#include "ImgCropPage.h"
#include "Transform.h"
#include "ExifBlock.h"
#include "TransformationThread.h"
#include "ProcessingProgressDlg.h"
#include "BalloonMsg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


bool LoadImageList(CImageList& il, UINT id, int img_width);
extern String ReplaceIllegalChars(const String& text);
static const TCHAR* section= _T("ImageAdjustmentBatchMode");


// ImageManipulationDlg dialog

ImageManipulationDlg::ImageManipulationDlg(CWnd* parent, VectPhotoInfo& photos, PhotoCache* cache)
	: DialogChild(ImageManipulationDlg::IDD, parent), photos_(photos)
{
	cur_dlg_ = 0;
	cache_ = cache;
	show_batch_mode_params_ = false;

	profile_dest_folder_sel_.Register(section, _T("DestFolderSel"), dlg_batch_mode_.dest_folder_);
	profile_dest_folder_.Register(section, _T("DestFolder"), dlg_batch_mode_.dest_folder_str_);
	profile_file_suffix_.Register(section, _T("FileSuffix"), dlg_batch_mode_.suffix_);
	last_selected_tab_.Register(section, _T("LastPage"), 0);

	dlg_batch_mode_.dest_folder_ = profile_dest_folder_sel_;
	dlg_batch_mode_.dest_folder_str_ = profile_dest_folder_;
	dlg_batch_mode_.suffix_ = profile_file_suffix_;
}


ImageManipulationDlg::~ImageManipulationDlg()
{}


void ImageManipulationDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	DDX_Control(DX, IDC_TAB, tab_wnd_);
	DDX_Control(DX, IDC_PREVIEW, original_wnd_);
	DDX_Control(DX, IDC_MODIFIED, modified_wnd_);
	DDX_Control(DX, IDC_CROP, crop_wnd_);
	DDX_Control(DX, IDC_LABEL_1, label_wnd_);
}


BEGIN_MESSAGE_MAP(ImageManipulationDlg, DialogChild)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelChangeTab)
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// ImageManipulationDlg message handlers


BOOL ImageManipulationDlg::OnInitDialog()
{
	try
	{
		return InitDialog();
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


void MoveCtrl(CWnd* wnd, int dx, int dy)
{
	if (wnd == 0)
	{
		ASSERT(false);
		return;
	}

	WINDOWPLACEMENT wp;
	if (wnd->GetWindowPlacement(&wp))
	{
		int x= wp.rcNormalPosition.left + dx;
		int y= wp.rcNormalPosition.top + dy;
		wnd->SetWindowPos(0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


bool ImageManipulationDlg::InitDialog()
{
	ASSERT(!photos_.empty());

	CWaitCursor wait;

	DialogChild::OnInitDialog();

	SubclassHelpBtn(_T("ToolImgAdjust.htm"));

	CacheImg* img= 0;

	CSize img_size(500, 500);	// limit size of image for preview

#if 0
	// this branch works, but has no reliable rotation information that cropping needs
	if (cache_ && (img = cache_->FindEntry(photos_.front())) != 0 && img->dib.get())
	{
		if (img->dib->GetWidth() / 2 > img_size.cx || img->dib->GetHeight() / 2 > img_size.cy)
		{
			dib_original_.Clone(*img->dib);
			dib_original_.ResizeToFit(img_size, Dib::RESIZE_HALFTONE);
			dib_current_.Clone(dib_original_);
		}
		else
		{
			dib_original_.Clone(*img->dib);
			dib_current_.Clone(*img->dib);
		}

		crop_wnd_.SetOriginalBmpSize(img->original_size);
	}
	else
#endif
	{
		CImageDecoderPtr decoder= photos_.front()->GetDecoder();
		photos_.front()->photo_stat_ = decoder->DecodeImg(dib_original_, img_size, false);
		if (photos_.front()->photo_stat_ == IS_OK)
			dib_current_.Clone(dib_original_);
		else
		{
			// message and bail
			//
			MessageBox(_T("Cannot load selected photograph."));

			EndDialog(IDCANCEL);
			return true;
		}

		crop_wnd_.SetOriginalBmpSize(decoder->GetOriginalSize());
	}

	original_wnd_.SetDib(dib_original_);
	modified_wnd_.SetDib(dib_current_);
	crop_wnd_.SetDib(dib_current_);

	original_wnd_.SetHost(this);
	modified_wnd_.SetHost(this);

	crop_wnd_.EnableSelection(true);

	LoadImageList(image_list_, IDB_IMG_MANIP, 18);

	tab_dlg_.reserve(4);
	tab_dlg_.push_back(new ImgLevelsPage());
	tab_dlg_.push_back(new ImgBrightnessPage());
	tab_dlg_.push_back(new ImgColorBalancePage());
	tab_dlg_.push_back(new ImgCropPage(crop_wnd_, photos_.front()));

	for (size_t i= 0; i < tab_dlg_.size(); ++i)
	{
		tab_dlg_[i].RestoreSettings(section);
		tab_dlg_[i].Create(&tab_wnd_);
		tab_dlg_[i].Initialize(dib_original_);
	}

	tab_wnd_.SetImageList(&image_list_);
	tab_wnd_.InsertItem(0, _T("Image Levels"), 0);
	tab_wnd_.InsertItem(1, _T("Brightness/Contrast"), 1);
	tab_wnd_.InsertItem(2, _T("Color Balance"), 2);
	tab_wnd_.InsertItem(3, _T("Image Crop"), 3);

	tab_wnd_.ModifyStyle(0, WS_CLIPCHILDREN);
	tab_wnd_.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	if (!dlg_info_.Create(this) || !dlg_options_.Create(GetParent()) || !dlg_batch_mode_.Create(this))
	{
		EndDialog(IDCANCEL);
		return true;
	}

	show_batch_mode_params_ = photos_.size() > 1;

	// position 'batch mode params' sub dialog
	if (show_batch_mode_params_)
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);

		CRect dlg_rect(0,0,0,0);
		dlg_batch_mode_.GetClientRect(dlg_rect);

		CRect wnd_rect(0,0,0,0);
		GetWindowRect(wnd_rect);

		CWnd* btn= GetDlgItem(IDC_OPTIONS);
		if (btn == 0)
		{
			ASSERT(false);
			EndDialog(IDCANCEL);
			return true;
		}

		WINDOWPLACEMENT wp;
		VERIFY(btn->GetWindowPlacement(&wp));

		int height= dlg_rect.Height();
		int yPos= wp.rcNormalPosition.top - 3;

		SetWindowPos(0, 0, 0, wnd_rect.Width(), wnd_rect.Height() + height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		dlg_batch_mode_.SetWindowPos(0, 0, yPos, rect.Width(), height, SWP_NOZORDER | SWP_NOACTIVATE);

		MoveCtrl(GetDlgItem(IDC_OPTIONS), 0, height);
		MoveCtrl(GetDlgItem(IDCANCEL), 0, height);
		MoveCtrl(GetDlgItem(IDOK), 0, height);
		MoveCtrl(GetDlgItem(IDC_HELP_BTN), 0, height);

		dlg_batch_mode_.SetDlgCtrlID(IDD_IMG_BATCH_MODE);

		SetDlgItemText(IDOK, RString(IDS_PROCESS_PHOTOS));

		dlg_batch_mode_.ShowWindow(SW_SHOWNA);

		// hide suffix edit box in options dlg
		dlg_options_.label2_wnd_.ShowWindow(SW_HIDE);
		dlg_options_.suffix_wnd_.ShowWindow(SW_HIDE);
		dlg_options_.suffix_wnd_.EnableWindow(false);
	}

	dlg_options_.histogram_wnd_.Build(dib_original_);

	SetFooterDlg(&dlg_options_);
	SetDlgItemText(IDC_OPTIONS, RString(IDS_SAVING_OPTIONS));

	if (CWnd* frame= GetDlgItem(IDC_INFO))
	{
		WINDOWPLACEMENT wp;
		frame->GetWindowPlacement(&wp);
		dlg_info_.MoveWindow(&wp.rcNormalPosition);
		frame->DestroyWindow();
		dlg_info_.SetDlgCtrlID(IDC_INFO);
		// init RGB readings
		MouseMoved(0, 0, 0, true);
	}
	else
	{
		ASSERT(false);
		EndDialog(IDCANCEL);
		return true;
	}

	BuildResizingMap();

	SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	SetWndResizing(IDC_PREVIEW, DlgAutoResize::RESIZE, DlgAutoResize::HALF_RESIZE_H);
	SetWndResizing(IDC_MODIFIED, DlgAutoResize::MOVE_H_RESIZE, DlgAutoResize::HALF_MOVE_H | DlgAutoResize::HALF_RESIZE_H);
	SetWndResizing(IDC_CROP, DlgAutoResize::RESIZE);
	SetWndResizing(IDC_TAB, DlgAutoResize::MOVE_V_RESIZE_H);
	SetWndResizing(IDC_NAVIGATION, DlgAutoResize::MOVE_V);
	SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
	SetWndResizing(IDOK, DlgAutoResize::MOVE);
	SetWndResizing(IDC_OPTIONS, DlgAutoResize::MOVE_V);
	SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);
	SetWndResizing(IDC_INFO_FRAME, DlgAutoResize::MOVE);
	SetWndResizing(IDC_INFO, DlgAutoResize::MOVE);
	if (show_batch_mode_params_)
		SetWndResizing(IDD_IMG_BATCH_MODE, DlgAutoResize::MOVE_V_RESIZE_H);

	int tab= last_selected_tab_;
	if (tab < 0 && tab > tab_dlg_.size())
		tab = 0;
	tab_wnd_.SetCurSel(tab);
	ShowDlg();

	for (size_t i= 0; i < tab_dlg_.size(); ++i)
		tab_dlg_[i].SetHost(this);

	effect_stack_.reserve(tab_dlg_.size());

	return true;
}


ImgPage* ImageManipulationDlg::ShowDlg(ImgPage* dlg)
{
	if (dlg == 0 || dlg->m_hWnd == 0)
	{
		ASSERT(false);
		return 0;
	}

	PositionTabDlg(dlg);

	dlg->ShowWindow(SW_SHOWNA);

	if (CWnd* ctrl= GetNextDlgTabItem(0))
		GotoDlgCtrl(ctrl);

	return dlg;
}


void ImageManipulationDlg::PositionTabDlg(ImgPage* dlg)
{
	if (dlg && dlg->m_hWnd)
	{
		CRect rect(0,0,0,0);
		tab_wnd_.GetClientRect(rect);
		tab_wnd_.AdjustRect(false, rect);
		dlg->MoveWindow(rect);
	}
}


void ImageManipulationDlg::Resize()
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return;

	//dlg_resize_map_.Resize(rect);
	DialogChild::Resize(rect);

	PositionTabDlg(cur_dlg_);
}


void ImageManipulationDlg::OnSelChangeTab(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;

	ShowDlg();
}


void ImageManipulationDlg::ShowDlg()
{
	int tab= tab_wnd_.GetCurSel();

	if (tab < 0 && tab > tab_dlg_.size())
		return;

	ImgPage* dlg= &tab_dlg_[tab];
	if (dlg == cur_dlg_)
		return;

	if (cur_dlg_)
	{
		cur_dlg_->ShowWindow(SW_HIDE);
		cur_dlg_->EnableWindow(false);
	}

	if (dlg)
	{
		dlg->EnableWindow();
		cur_dlg_ = ShowDlg(dlg);

		if (cur_dlg_)
			if (cur_dlg_->NeedPreview())
			{
				crop_wnd_.ShowWindow(SW_HIDE);
				label_wnd_.ShowWindow(SW_SHOWNA);
				original_wnd_.ShowWindow(SW_SHOWNA);
				modified_wnd_.ShowWindow(SW_SHOWNA);
			}
			else
			{
				label_wnd_.ShowWindow(SW_HIDE);
				original_wnd_.ShowWindow(SW_HIDE);
				modified_wnd_.ShowWindow(SW_HIDE);
				crop_wnd_.ShowWindow(SW_SHOWNA);
			}
	}
}


void ImageManipulationDlg::ParamChanged(ImgPage* wnd, bool reset)
{
	size_t index= ~0;

	for (size_t i= 0; i < tab_dlg_.size(); ++i)
		if (&tab_dlg_[i] == wnd)
		{
			index = i;
			break;
		}

	if (index >= tab_dlg_.size())
	{
		ASSERT(false);
		return;
	}

	ApplyEffect(index, reset);

	if (dlg_options_.GetStyle() & WS_VISIBLE)
	{
		dlg_options_.histogram_wnd_.Build(dib_current_);
		dlg_options_.histogram_wnd_.UpdateWindow();
	}
}


void ImageManipulationDlg::ApplyEffect(size_t index, bool reset)
{
	// 'index' is an index to the tab_dlg_ element (selected effect)

	bool effects_changed= false;

	size_t position= ~0;

	for (size_t i= 0; i < effect_stack_.size(); ++i)
		if (effect_stack_[i] == index)
		{
			position = i;
			break;
		}

	if (reset)
	{
		if (position < effect_stack_.size())
		{
			effect_stack_.erase(effect_stack_.begin() + position);

			effects_changed = true;
		}
	}
	else
	{
		if (position >= effect_stack_.size())
		{
			position = effect_stack_.size();
			effect_stack_.push_back(static_cast<int>(index));

			effects_changed = true;
		}
	}

	// apply all effects

	dib_current_.Clone(dib_original_);

	for (size_t i= 0; i < effect_stack_.size(); ++i)
		tab_dlg_[effect_stack_[i]].Transform(dib_current_, true);

	modified_wnd_.RedrawWindow();
	crop_wnd_.RedrawWindow();

	if (effects_changed)
	{
		dlg_info_.ctrl_image_.SelectImages(effect_stack_);
		dlg_info_.ctrl_image_.RedrawWindow();
	}
}


void ImageManipulationDlg::MouseMoved(CWnd* preview, int x, int y, bool outside)
{
	RGBQUAD original= { 0, 0, 0, 0 };
	RGBQUAD new_color= { 0, 0, 0, 0 };

	if (!outside)
	{
		original = dib_original_.GetPixel(x, y);
		new_color = dib_current_.GetPixel(x, y);
	}

	dlg_info_.ShowColors(original, new_color, outside);
}


bool ImageManipulationDlg::SelectOutputFileName(const Path& pathPhoto, Path& file_name)
{
	CFileDialog dlgFile(false, _T(".jpg"), pathPhoto.GetFileName().c_str(),
		OFN_EXPLORER | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		RString(IDS_JPEG_FILTER), 0);

	CString dir= pathPhoto.GetDir().c_str();
	dlgFile.m_ofn.lpstrInitialDir = dir;

//	CString name= pathPhoto.GetFileName().c_str();
//	dlgFile.ofn_.lpstrFile = const_cast<TCHAR*>(static_cast<const TCHAR*>(name));

	// get output file name
	if (dlgFile.DoModal() != IDOK)
		return false;

	file_name.assign(dlgFile.GetPathName());

	return true;
}


class HideWnd
{
public:
	HideWnd(CWnd& wnd) : wnd_(wnd)
	{
		wnd_.ShowWindow(SW_HIDE);
		wnd_.EnableWindow(false);
		show_ = true;
	}

	~HideWnd()
	{
		if (show_)
		{
			wnd_.EnableWindow();
			wnd_.ShowWindow(SW_SHOW);
		}
	}

	void DontShow()		{ show_ = false; }

private:
	CWnd& wnd_;
	bool show_;
};


void ImageManipulationDlg::OnOK()
{
	if (effect_stack_.empty())
	{
		// there's nothing to do
		new BalloonMsg(&tab_wnd_, _T("No Transformations Selected"),
			_T("Please select desired image transformation before proceeding."), BalloonMsg::IWARNING);
		return;
	}

	try
	{
		if (!dlg_options_.UpdateData() || !dlg_batch_mode_.Finish())
			return;

		// prepare params (src, dest, encoding options...)
		TransformationParams params;
		if (show_batch_mode_params_)
		{
			params.use_src_path_ = dlg_batch_mode_.dest_folder_ == 0 ?
										TransformationParams::USE_SRC_PATH : TransformationParams::USE_DEST_PATH;
			params.suffix_ = dlg_batch_mode_.suffix_;
			params.dest_path_ = static_cast<const TCHAR*>(dlg_batch_mode_.dest_folder_str_);
			params.dest_file_extension_ = _T("jpg");
		}
		else
		{
			params.use_src_path_ = TransformationParams::USE_GIVEN_FILENAME;
			params.suffix_.clear();
			params.dest_path_.clear();

			Path pathFile= photos_.front()->GetPhysicalPath();	//TODO: use original path?
			pathFile.AppendToFileName(ReplaceIllegalChars(static_cast<const TCHAR*>(dlg_options_.suffix_)).c_str());
			if (!SelectOutputFileName(pathFile, params.dest_file_))
				return;

			//TODO: lift this limitation... (use temp copy)
			if (_tcsicmp(params.dest_file_.c_str(), photos_.front()->GetPhysicalPath().c_str()) == 0)
			{
				new BalloonMsg(GetDlgItem(IDOK), _T("Cannot Overwrite Original Image"),
					_T("Overwriting original image file is not supported."), BalloonMsg::IWARNING);
				return;
			}
		}

		params.copy_exif_		= !!dlg_options_.preserve_exif_;	// preserve EXIF
		params.baseline_		= !!dlg_options_.base_line_;		// JPEG params...
		params.progressive_		= !!dlg_options_.progressive_;
		params.jpeg_quality_	= dlg_options_.quality_;
		params.dest_file_extension_ = _T("jpg");

		// prepare transformations
		std::vector<Transformation> transforms;
		transforms.reserve(effect_stack_.size());

		//TODO: verify line below
		if (effect_stack_.size() == 1 && tab_dlg_[effect_stack_.front()].IsExclusive(photos_.front()))
		{
			Transformation::FnTransformFile fn= boost::bind(&ImgPage::TransformFile, &tab_dlg_[effect_stack_.front()], _1, _2);

			// exclusive transformation: currently it is loss-less JPEG cropping
			transforms.push_back(fn);
		}
		else
		{
			for (size_t i= 0; i < effect_stack_.size(); ++i)
			{
				Transformation::FnTransformBmp fn= boost::bind(&ImgPage::Transform, &tab_dlg_[effect_stack_[i]], _1, false);
				transforms.push_back(fn);
			}
		}

		ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new CTransformationThread(photos_, transforms, params)));

		ProcessingProgressDlg dlg(AfxGetMainWnd(), proc, _T("Transformation in Progress"), 0,
			ProcessingProgressDlg::AUTO_CLOSE | ProcessingProgressDlg::INPUT_OUTPUT);

		HideWnd temp(*GetParent());

		if (dlg.DoModal() != IDOK)
			return;

		temp.DontShow();
#if 0
		CWaitCursor wait;

		if (effect_stack_.size() == 1 && tab_dlg_[effect_stack_.front()].IsExclusive(&photo))
		{
			// exclusive transformation: currently it is loss-less JPEG cropping

			if (tab_dlg_[effect_stack_.front()].TransformFile(photo.path_, output))
				;
			else
			{
				// prepare error msg
				String str= _T("Cannot process file:\n") + photo.path_;
				MessageBox(str.c_str());
			}
		}
		else
		{
			// any transformations

			CSize img_size(0, 0);
			CImageDecoderPtr decoder= photo.GetDecoder();
			Dib dib;
			photo.photo_stat_ = decoder->DecodeImg(dib, img_size, false);
			if (photo.photo_stat_ == IS_OK)
			{
				// apply transformations
				for (size_t i= 0; i < effect_stack_.size(); ++i)
					tab_dlg_[effect_stack_[i]].Transform(dib, false);

				// EXIF block
				vector<uint8> exif;
				if (dlg_options_.preserve_exif_ && photo.IsExifDataPresent())
				{
					ExifBlock exifBlock;
					if (photo.ReadExifBlock(exifBlock))
					{
						exifBlock.ModifySizeFields(dib.GetSize());
						exifBlock.GetExifMarkerBlock(exif);
					}
				}

				// destination file
				CFileDataDestination fdest(output.c_str());

				// write resized photo
				JPEGEncoder encoder(dlg_options_.quality_, !!dlg_options_.progressive_, !!dlg_options_.base_line_);

				if (!encoder.Encode(fdest, &dib, &exif))
				{
					MessageBox(_T("Error encoding JPEG file."));
					return;
				}
			}
			else
			{
				// message and bail
				//
				MessageBox(_T("Cannot load selected photograph."));
				return;
			}
		}
#endif

		if (dlg_options_.m_hWnd)
			dlg_options_.SaveOptions();

		profile_dest_folder_sel_ = dlg_batch_mode_.dest_folder_;
		profile_dest_folder_ = dlg_batch_mode_.dest_folder_str_;
		profile_file_suffix_ = dlg_batch_mode_.suffix_;

		for (size_t i= 0; i < tab_dlg_.size(); ++i)
			tab_dlg_[i].StoreSettings(section);

		CDialog::OnOK();
	}
	CATCH_ALL
}


void ImageManipulationDlg::OnOptions()
{
	if (dlg_options_.m_hWnd == 0)
		return;

	bool show= dlg_options_.GetStyle() & WS_VISIBLE ? false : true;

	if (show)
		dlg_options_.histogram_wnd_.Build(dib_current_);

	if (GetGripWnd().m_hWnd)
		GetGripWnd().ShowWindow(show ? SW_HIDE : SW_SHOWNA);
	ShowFooterDlg(show);

	SetDlgItemText(IDC_OPTIONS, RString(show ? IDS_CLOSE_SAVING_OPTIONS : IDS_SAVING_OPTIONS));
}


void ImageManipulationDlg::OnDestroy()
{
	if (tab_wnd_.m_hWnd)
		last_selected_tab_ = tab_wnd_.GetCurSel();

	DialogChild::OnDestroy();
}
