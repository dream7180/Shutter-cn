/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2011 Michael Kowalski
____________________________________________________________________________*/

// Config.cpp: implementation of the Config class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "Config.h"
#include "ProfileVector.h"
//#include "BoldFont.h"
#include <shlwapi.h>
#include "DefaultColors.h"
#include "ImgDb.h"
#include "Color.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using std::make_pair;

// Global settings object
Config g_Settings;
extern bool g_first_time_up;
extern int GetLogicalProcessorInfo();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Config::Config()
{
	dct_method_ = JDEC_INTEGER_HIQ;			// jpeglib decoder param: DCL type
	gamma_ = 1.6;							// gamma value
//	use_gamma_ = false;						// use image color correction in preview pane & printing
	//rgb_cur_selection_ = RGB(192,192,192);	// preview pane current photo selection color
	keep_sel_photo_centered_ = false;
	preload_photos_ = true;
	language_ = 1;
	viewer_ui_gamma_correction_ = 1.0;
	horz_resolution_ = 90.0f;				// monitor resolution
	vert_resolution_ = 90.0f;
	correct_CRT_aspect_ratio_ = true;
	display_method_ = DIB_SMOOTH_DRAW;		// default hi quality
	image_cache_size_ = 20;					// 20% RAM
	save_tags_to_photo_ = true;
	warn_about_broken_thumbnail_img_ = false;
#ifdef _WIN64
	db_file_length_limit_mb_ = 5 * 1024;	// 5 GB
#else
	db_file_length_limit_mb_ = 3 * 1024;	// 3 GB
#endif
	image_blending_ = 1;					// 0-always, 1-in slide show only
	read_thumbs_from_db_ = false;			// by default keep jpeg thumbs in memory
	allow_magnifying_above100_ = false;
	allow_zoom_to_fill_ = false;
	percent_of_image_to_hide_ = 30;
	close_app_when_viewer_exits_ = true;
	smooth_scrolling_speed_ = 15;

	int cores= GetLogicalProcessorInfo();

	// if more than 2 cores are available, sharpen thumbnails
	thumbnail_sharpening_ = cores > 1 ? 30 : 0;

	file_types_.reserve(FT_LAST + 1);
	file_types_.push_back(FileTypeConfig(_T("Jpeg 图像"), _T("jpeg; jpg; jpe"), _T("JPEG"), false));
	file_types_.push_back(FileTypeConfig(_T("Photoshop 文档"), _T("psd"), _T("PSD")));
	file_types_.push_back(FileTypeConfig(_T("Tiff 图像"), _T("tiff; tif"), _T("TIFF")));
	file_types_.push_back(FileTypeConfig(_T("便携式网络图形"), _T("png"), _T("PNG")));
	file_types_.push_back(FileTypeConfig(_T("佳能 RAW 文件"), _T("crw+thm; cr2"), _T("CRW")));
	file_types_.push_back(FileTypeConfig(_T("尼康 RAW 文件"), _T("nef"), _T("NEF")));
	file_types_.push_back(FileTypeConfig(_T("奥林巴斯 RAW 文件"), _T("orf"), _T("ORF")));
	file_types_.push_back(FileTypeConfig(_T("数字负片文件"), _T("dng"), _T("DNG")));
	file_types_.push_back(FileTypeConfig(_T("富士 RAW 文件"), _T("raf"), _T("RAF")));
	file_types_.push_back(FileTypeConfig(_T("图形交换格式"), _T("gif"), _T("GIF")));
	file_types_.push_back(FileTypeConfig(_T("Windows 位图文件"), _T("bmp"), _T("BMP")));
	file_types_.push_back(FileTypeConfig(_T("宾得 RAW 文件"), _T("pef"), _T("PEF")));
	file_types_.push_back(FileTypeConfig(_T("索尼 DSLR RAW 文件"), _T("arw"), _T("ARW")));
	file_types_.push_back(FileTypeConfig(_T("松下 RAW 文件"), _T("rw2"), _T("RW2")));
	file_types_.push_back(FileTypeConfig(_T("适马 RAW 文件"), _T("x3f"), _T("X3F")));
	file_types_.push_back(FileTypeConfig(_T("三星 RAW 文件"), _T("srw"), _T("SRW")));
	file_types_.push_back(FileTypeConfig(_T("Shutter 分类文件"), _T("catalog"), _T("catalog"), false));

	//open_photo_app_
	main_wnd_ = new ICMProfile(_T("浏览器主窗口"));
	viewer_ = new ICMProfile(_T("图像查看器"));
	default_photo_ = new ICMProfile(_T("默认图像"));
	default_printer_ = new ICMProfile(_T("默认打印机"));

	main_wnd_->AssignDefault_sRGB();
	viewer_->AssignDefault_sRGB();
	default_photo_->AssignDefault_sRGB();
	default_printer_->AssignDefault_sRGB();

	// default: no ICM
	main_wnd_->enabled_ = false;
	viewer_->enabled_ = false;
	default_printer_->enabled_ = false;
	default_photo_->enabled_ = true;

	// default: hi quality
	regenerate_thumbnails_ = GEN_THUMB_EXCEPT_REMOVABLE_DRV;

	img_cache_db_path_ = GetDefaultDbFolder();

	UpdateAppColors();
}


Config::~Config()
{}


namespace {
	const TCHAR* const REGISTRY_ENTRY_CONFIG=	_T("Config");
	const TCHAR* const REG_DCT_METHOD=			_T("DCLMethod");
	const TCHAR* const REG_GAMMA_VAL=			_T("GammaVal");
	const TCHAR* const REG_USE_GAMMA=			_T("UseGamma");
	const TCHAR* const REG_SEL_COLOR=			_T("SelColor");
	//const TCHAR* const REG_SCROLLING=_T("Scrolling");
	const TCHAR* const REG_FLEN_CONV=			_T("FocalLengthConv");
	const TCHAR* const REG_FLEN_MULT=			_T("FocalLengthMult");
	const TCHAR* const REG_CENTER_SEL=			_T("KeepSelCentered");
	const TCHAR* const REG_PRELOAD=				_T("PreloadPhotos");
	const TCHAR* const REG_DESC_FONT=			_T("DescriptionFont");
	const TCHAR* const REG_DESC_COLOR=			_T("DescriptionColor");
	const TCHAR* const REG_LANGUAGE=			_T("AppLanguage");
	const TCHAR* const REG_INFO_BAR_PREVIEW=	_T("InfoBarPreview");
	const TCHAR* const REG_INFO_BAR_RAW_INFO=	_T("InfoBarRawInfo");
	const TCHAR* const REG_LIST_CTRL_COLORS=	_T("PhotoCtrlColors");
	const TCHAR* const REG_USE_LIST_CTRL_COLORS=_T("PhotoCtrlColorsCust");
	const TCHAR* const REG_VIEWER_COLORS=		_T("ViewerColors");
	const TCHAR* const REG_USE_VIEWER_COLORS=	_T("ViewerColorsCust");
	const TCHAR* const REG_LIST_CTRL_SYS_CLR=	_T("PhotoCtrlUseSysColors");
	const TCHAR* const REG_HORZ_RESOLUTION=		_T("MonitorHorzResolution");
	const TCHAR* const REG_VERT_RESOLUTION=		_T("MonitorVertResolution");
	const TCHAR* const REG_CORRECT_ASPECT_RATIO=_T("CorrectCRTAspectRatio");
	const TCHAR* const REG_VIEWER_DEFAULT_CLR=	_T("ViewerUseSysColors");
	const TCHAR* const REG_OPEN_PHOTO_APP=		_T("OpenPhotoApp");
	const TCHAR* const REG_OPEN_RAW_PHOTO_APP=	_T("OpenRawPhotoApp");
	const TCHAR* const REG_DISPLAY_METHOD=		_T("DisplayMethod");
	const TCHAR* const REG_IMAGE_CACHE_SIZE=	_T("ImageCacheSize");
	const TCHAR* const REG_FILE_TYPES=			_T("Config\\FileTypes");
	const TCHAR* const REG_SHOW_THUMB_WARNING=	_T("ShowBrokenThumbnailWarning");
	const TCHAR* const REG_SAVE_TAGS=			_T("SaveTagsInPhotos");
	const TCHAR* const REG_BALLOON_FIELDS=		_T("BalloonFields");
	const TCHAR* const REG_DB_SIZE_LIMIT_MB=	_T("DbSizeLimitMB");
	const TCHAR* const REGISTRY_ENTRY_ICC=		_T("Config\\ICC");
	const TCHAR* const REG_MAIN_WND=			_T("MainWndProfile");
	const TCHAR* const REG_VIEWER=				_T("ViewerProfile");
	const TCHAR* const REG_DEFAULT_IMG=			_T("DefaultImgProfile");
	const TCHAR* const REG_DEFAULT_PRN=			_T("DefaultPrinterProfile");
	const TCHAR* const REG_IMG_BLENDING=		_T("ImageBlending");
	const TCHAR* const REG_FOV_MODELS=			_T("FoVModels");
	const TCHAR* const REG_FOV_CROPS=			_T("FoVCrops");
	const TCHAR* const REG_GEN_THUMB=			_T("GenerateThumbnailsMethod");
	const TCHAR* const REG_THUMB_ACCESS=		_T("ThumbnailAccess");
	const TCHAR* const REG_MAGNIFY_100=			_T("Magnify100");
	const TCHAR* const REG_MAGNIFY_FILL=		_T("ZoomFill");
	const TCHAR* const REG_IMG_HIDE=			_T("AmountOfImgToHide");
	const TCHAR* const REG_VIEWER_UI_GAMMA=		_T("ViewerUIBrightness");
	const TCHAR* const REG_CLOSE_APP=			_T("CloseAppWhenExitingViewer");
	const TCHAR* const REG_SMOOTH_SPEED_VAL=	_T("SmoothSpeedValue");
	const TCHAR* const REG_TAG_FONT=			_T("ImgTagFont");
	//const TCHAR* const REG_PANE_CAPTION_COLORS=	_T("PaneCaptionColors");
	//const TCHAR* const REG_USE_PANE_CAPTION_COLORS=	_T("PaneCaptionColorsCustom");
	const TCHAR REG_DB_PATH[]=				_T("ImageCacheDbPath");
	const TCHAR REG_THUMB_SHARPENING[]=		_T("ThumbnailSharpening");
}


void Config::Store()
{
	CWinApp* app= AfxGetApp();
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_DCT_METHOD, dct_method_);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_GAMMA_VAL, int(gamma_ * 100));
	//app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_USE_GAMMA, use_gamma_ ? 1 : 0);
	//app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_SEL_COLOR, rgb_cur_selection_);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_CENTER_SEL, keep_sel_photo_centered_ ? 1 : 0);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_PRELOAD, preload_photos_ ? 1 : 0);
	app->WriteProfileBinary(REGISTRY_ENTRY_CONFIG, REG_DESC_FONT, reinterpret_cast<BYTE*>(&description_font_), sizeof description_font_);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_LANGUAGE, language_);

	WriteProfileVector(REGISTRY_ENTRY_CONFIG, REG_LIST_CTRL_COLORS, main_wnd_colors_.CustomColors());
	WriteProfileVector(REGISTRY_ENTRY_CONFIG, REG_USE_LIST_CTRL_COLORS, main_wnd_colors_.UseCustomFlags());

	WriteProfileVector(REGISTRY_ENTRY_CONFIG, REG_VIEWER_COLORS, viewer_wnd_colors_.CustomColors());
	WriteProfileVector(REGISTRY_ENTRY_CONFIG, REG_USE_VIEWER_COLORS, viewer_wnd_colors_.UseCustomFlags());
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_VIEWER_UI_GAMMA, static_cast<int>(viewer_ui_gamma_correction_ * 100));

	//WriteProfileVector(REGISTRY_ENTRY_CONFIG, REG_PANE_CAPTION_COLORS, pane_caption_colors_.CustomColors());
	//WriteProfileVector(REGISTRY_ENTRY_CONFIG, REG_USE_PANE_CAPTION_COLORS, pane_caption_colors_.UseCustomFlags());

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_HORZ_RESOLUTION, static_cast<int>(horz_resolution_ * 100));
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_VERT_RESOLUTION, static_cast<int>(vert_resolution_ * 100));
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_CORRECT_ASPECT_RATIO, correct_CRT_aspect_ratio_ ? 1 : 0);

	app->WriteProfileString(REGISTRY_ENTRY_CONFIG, REG_OPEN_PHOTO_APP, open_photo_app_);
	app->WriteProfileString(REGISTRY_ENTRY_CONFIG, REG_OPEN_RAW_PHOTO_APP, open_raw_photo_app_);
	//::WritePrivateProfileString(REGISTRY_ENTRY_CONFIG,REG_OPEN_RAW_PHOTO_APP,open_raw_photo_app_,_T(".\\Shutter.ini"));//////////////////////////////////////ini file//////////////

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_DISPLAY_METHOD, display_method_);

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_IMAGE_CACHE_SIZE, image_cache_size_);

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_SHOW_THUMB_WARNING, warn_about_broken_thumbnail_img_ ? 1 : 0);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_SAVE_TAGS, save_tags_to_photo_ ? 1 : 0);

	const size_t count= file_types_.size();
	for (size_t i= 0; i < count; ++i)
		file_types_[i].Store(REG_FILE_TYPES);

	WriteProfileVector(REGISTRY_ENTRY_CONFIG, REG_BALLOON_FIELDS, balloon_fields_);

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_DB_SIZE_LIMIT_MB, db_file_length_limit_mb_);

	main_wnd_->Store(REGISTRY_ENTRY_ICC, REG_MAIN_WND);
	viewer_->Store(REGISTRY_ENTRY_ICC, REG_VIEWER);
	default_photo_->Store(REGISTRY_ENTRY_ICC, REG_DEFAULT_IMG);
	default_printer_->Store(REGISTRY_ENTRY_ICC, REG_DEFAULT_PRN);

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_IMG_BLENDING, image_blending_);

	SaveFoVCrop();

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_GEN_THUMB, regenerate_thumbnails_);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_THUMB_ACCESS, read_thumbs_from_db_ ? 1 : 0);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_MAGNIFY_100, allow_magnifying_above100_ ? 1 : 0);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_MAGNIFY_FILL, allow_zoom_to_fill_ ? 1 : 0);
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_IMG_HIDE, percent_of_image_to_hide_);

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_CLOSE_APP, close_app_when_viewer_exits_ ? 1 : 0);

	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_SMOOTH_SPEED_VAL, smooth_scrolling_speed_);
	app->WriteProfileBinary(REGISTRY_ENTRY_CONFIG, REG_TAG_FONT, reinterpret_cast<BYTE*>(&img_tag_font_), sizeof img_tag_font_);
	app->WriteProfileString(REGISTRY_ENTRY_CONFIG, REG_DB_PATH, img_cache_db_path_.c_str());
	app->WriteProfileInt(REGISTRY_ENTRY_CONFIG, REG_THUMB_SHARPENING, thumbnail_sharpening_);
}


void Config::SaveFoVCrop()
{
	size_t count= field_of_view_crop_.size();
	CString models, crops;
	for (size_t i= 0; i < count; ++i)
	{
		models += field_of_view_crop_[i].first.c_str();
		models += _T('\n');
		crops += field_of_view_crop_[i].second.c_str();
		crops += _T('\n');
	}

	CWinApp* app= AfxGetApp();
	app->WriteProfileString(REGISTRY_ENTRY_CONFIG, REG_FOV_MODELS, models);
	app->WriteProfileString(REGISTRY_ENTRY_CONFIG, REG_FOV_CROPS, crops);
}


struct cmp_fov
{
	bool operator () (const std::pair<String, String>& a, const std::pair<String, String>& b)
	{
		return a.first < b.first;	// cmp model names
	}
};


void Config::Restore()
{
	CWinApp* app= AfxGetApp();
	//todo: remove DCT method
	dct_method_ = static_cast<JpegDecoderMethod>(app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_DCT_METHOD, dct_method_));
	gamma_				= app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_GAMMA_VAL, 160);
	gamma_ /= 100.0;
//	use_gamma_			= !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_USE_GAMMA, use_gamma_);
	//rgb_cur_selection_	= app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_SEL_COLOR, rgb_cur_selection_);
//	scrolling_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_SCROLLING, scrolling_);
	keep_sel_photo_centered_	= !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_CENTER_SEL, keep_sel_photo_centered_);
	preload_photos_		= !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_PRELOAD, preload_photos_);

	// description font
	HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(default_description_font_), &default_description_font_);
	default_description_font_.lfHeight -= 14;
	default_description_font_.lfWeight = FW_BOLD;
	//default_description_font_.lfQuality = ANTIALIASED_QUALITY;
	_tcscpy(default_description_font_.lfFaceName, _T("Microsoft Yahei"));
	/*default_description_font_.lfItalic = 0;
	default_description_font_.lfUnderline = 0;
	default_description_font_.lfStrikeOut = 0;
	default_description_font_.lfCharSet = DEFAULT_CHARSET;
	default_description_font_.lfPitchAndFamily = DEFAULT_PITCH;
	default_description_font_.lfEscapement = 0;
	default_description_font_.lfOrientation = 0;
	default_description_font_.lfOutPrecision = OUT_DEFAULT_PRECIS;
	default_description_font_.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	default_description_font_.lfWidth = 0;*/

	BYTE* data= 0;
	UINT len;
	app->GetProfileBinary(REGISTRY_ENTRY_CONFIG, REG_DESC_FONT, &data, &len);
	if (data != 0)
	{
		memcpy(&description_font_, data, sizeof description_font_);
		delete [] data;
	}
	else
	{
		description_font_ = default_description_font_;
	}
	/*description_font_.lfEscapement = 0;
	description_font_.lfOrientation = 0;
	description_font_.lfOutPrecision = OUT_DEFAULT_PRECIS;
	description_font_.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	description_font_.lfQuality = ANTIALIASED_QUALITY;
	description_font_.lfWidth = 0;*/

//	rgb_description_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_DESC_COLOR, rgb_description_);

	language_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_LANGUAGE, -1);

	if (language_ == -1)	// language not yet chosen?
	{
		// try to select language
		LANGID lang_id= ::GetUserDefaultLangID();
		switch (PRIMARYLANGID(lang_id))
		{
		case LANG_ENGLISH:
			language_ = 0;
			break;
		case LANG_CHINESE:
			language_ = 1;
			break;
		case LANG_DUTCH:
			language_ = 2;
			break;
		case LANG_GERMAN:
			language_ = 3;
			break;
		case LANG_FRENCH:
			language_ = 4;
			break;
		case LANG_SPANISH:
			language_ = 5;
			break;
		case LANG_ITALIAN:
			language_ = 6;
			break;
		default:
			language_ = 0;
			break;
		}
	}

	//if (!GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_LIST_CTRL_COLORS, list_ctrl_colors_))
	//{
	//	// default colors
	//	PhotoCtrl ctrl;
	//	ctrl.GetColors(list_ctrl_colors_);
	//}
	//list_ctrl_sys_colors_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_LIST_CTRL_SYS_CLR, list_ctrl_sys_colors_ ? 1 : 0);

	//if (!GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_VIEWER_COLORS, viewer_colors_))
	//{
	//	//TODO: default colors

	//	viewer_colors_.resize(10);

	//	viewer_colors_[0] = RGB(0,0,0);
	//	viewer_colors_[1] = RGB(255,255,0);
	//	viewer_colors_[2] = RGB(0,0,0);
	//	viewer_colors_[3] = RGB(255,0,0);
	//	viewer_colors_[4] = RGB(0,200,0);
	//	viewer_colors_[5] = RGB(255,255,255);
	//}
	//viewer_default_colors_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_VIEWER_DEFAULT_CLR, viewer_default_colors_ ? 1 : 0);
	viewer_ui_gamma_correction_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_VIEWER_UI_GAMMA, static_cast<int>(viewer_ui_gamma_correction_ * 100)) / 100.0;

	horz_resolution_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_HORZ_RESOLUTION, static_cast<int>(horz_resolution_ * 100)) / 100.0f;
	vert_resolution_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_VERT_RESOLUTION, static_cast<int>(vert_resolution_ * 100)) / 100.0f;
	correct_CRT_aspect_ratio_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_CORRECT_ASPECT_RATIO, correct_CRT_aspect_ratio_ ? 1 : 0);

	display_method_ = static_cast<DibDispMethod>(app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_DISPLAY_METHOD, display_method_));

	open_photo_app_ = app->GetProfileString(REGISTRY_ENTRY_CONFIG, REG_OPEN_PHOTO_APP);
	open_raw_photo_app_ = app->GetProfileString(REGISTRY_ENTRY_CONFIG, REG_OPEN_RAW_PHOTO_APP);

	if (g_first_time_up)
	{
		TCHAR app[MAX_PATH * 2]= { 0 };
		DWORD len= MAX_PATH * 2;
		if (::AssocQueryString(0, ASSOCSTR_EXECUTABLE, _T(".psd"), _T("edit"), app, &len) == S_OK)	// photoshop
		{
			open_photo_app_ = app;
			open_raw_photo_app_ = app;
		}
		else if (::AssocQueryString(0, ASSOCSTR_EXECUTABLE, _T(".xcf"), _T("open"), app, &len) == S_OK)	// gimp
		{
			open_photo_app_ = app;
			open_raw_photo_app_ = app;
		}
		else if (::AssocQueryString(0, ASSOCSTR_EXECUTABLE, _T(".jpg"), _T("edit"), app, &len) == S_OK)
			open_photo_app_ = app;

		//TCHAR prog_path[_MAX_PATH];
		//VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), prog_path, _MAX_PATH));
		//CPath exif_dir= CPath(prog_path).GetDir();

		//TCHAR app[_MAX_PATH]= { 0 };
		//DWORD len= _MAX_PATH;
		//HINSTANCE err= HINSTANCE(32);
		//if (::FindExecutable(_T("file.psd"), exif_dir.c_str(), app) > err)
		//	open_photo_app_ = app;
		//else if (::FindExecutable(_T("file.jpg"), exif_dir.c_str(), app) > err)
		//	open_photo_app_ = app;
	}

	image_cache_size_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_IMAGE_CACHE_SIZE, image_cache_size_);
	image_blending_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_IMG_BLENDING, image_blending_);

	//===================================================================================

	const size_t count= file_types_.size();
	for (size_t i= 0; i < count; ++i)
		file_types_[i].Restore(REG_FILE_TYPES);

	warn_about_broken_thumbnail_img_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_SHOW_THUMB_WARNING, warn_about_broken_thumbnail_img_ ? 1 : 0);
	save_tags_to_photo_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_SAVE_TAGS, save_tags_to_photo_ ? 1 : 0);

	if (!GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_BALLOON_FIELDS, balloon_fields_))
	{
		int n= 16;
		balloon_fields_.resize(n);
		for (int i= 0; i < n; ++i)
			balloon_fields_[i] = i;
	}

	db_file_length_limit_mb_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_DB_SIZE_LIMIT_MB, db_file_length_limit_mb_);

	main_wnd_->Restore(REGISTRY_ENTRY_ICC, REG_MAIN_WND);
	viewer_->Restore(REGISTRY_ENTRY_ICC, REG_VIEWER);
	default_photo_->Restore(REGISTRY_ENTRY_ICC, REG_DEFAULT_IMG);
	default_printer_->Restore(REGISTRY_ENTRY_ICC, REG_DEFAULT_PRN);

	field_of_view_crop_.reserve(30);

	if (g_first_time_up)
	{
		field_of_view_crop_.push_back(make_pair(_T("CYBERSHOT"), _T("35 / 7.1")));
		field_of_view_crop_.push_back(make_pair(_T("NIKON D80"), _T("1.5")));
		field_of_view_crop_.push_back(make_pair(_T("NIKON D70"), _T("1.5")));
		field_of_view_crop_.push_back(make_pair(_T("NIKON D50"), _T("1.5")));
		field_of_view_crop_.push_back(make_pair(_T("NIKON D200"), _T("1.5")));
		field_of_view_crop_.push_back(make_pair(_T("Canon 10D"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon 300D"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon EOS 300D DIGITAL"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon EOS 10D"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon EOS 20D"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon EOS 30D"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon EOS D60"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon EOS D30"), _T("1.6")));
		field_of_view_crop_.push_back(make_pair(_T("Canon EOS 1D"), _T("1.3")));
		field_of_view_crop_.push_back(make_pair(_T("DSC-F828"), _T("28 / 5.6")));
		field_of_view_crop_.push_back(make_pair(_T("C8080WZ"), _T("28 / 7.1")));
		field_of_view_crop_.push_back(make_pair(_T("E5400"), _T("28 / 5.8")));
		field_of_view_crop_.push_back(make_pair(_T("E5700"), _T("35 / 8.9")));
		field_of_view_crop_.push_back(make_pair(_T("E8700"), _T("35 / 8.9")));	// Nikon Coolpix
		field_of_view_crop_.push_back(make_pair(_T("E-1"), _T("2")));
		field_of_view_crop_.push_back(make_pair(_T("Canon PowerShot A70"), _T("35 / 5.4")));
		field_of_view_crop_.push_back(make_pair(_T("Canon PowerShot A75"), _T("35 / 5.4")));
		field_of_view_crop_.push_back(make_pair(_T("Canon PowerShot G2"), _T("34 / 7")));
		field_of_view_crop_.push_back(make_pair(_T("Canon PowerShot G1"), _T("34 / 7")));
		field_of_view_crop_.push_back(make_pair(_T("Canon PowerShot S1 IS"), _T("38 / 5.8")));
		field_of_view_crop_.push_back(make_pair(_T("Canon PowerShot G5"), _T("35 / 7.2")));
		field_of_view_crop_.push_back(make_pair(_T("Canon PowerShot Pro1"), _T("35 / 7.2")));

		sort(field_of_view_crop_.begin(), field_of_view_crop_.end(), cmp_fov());

		SaveFoVCrop();
	}
	else
	{
		CString models= app->GetProfileString(REGISTRY_ENTRY_CONFIG, REG_FOV_MODELS);
		CString crops= app->GetProfileString(REGISTRY_ENTRY_CONFIG, REG_FOV_CROPS);

		for (int i= 0; ; ++i)
		{
			CString model;
			if (!AfxExtractSubString(model, models, i))
				break;

			CString crop;
			if (!AfxExtractSubString(crop, crops, i))
				break;

			if (!model.IsEmpty() || !crop.IsEmpty())
			{
				const TCHAR* m= model;
				const TCHAR* c= crop;
				field_of_view_crop_.push_back(make_pair(m, c));
			}
		}

		sort(field_of_view_crop_.begin(), field_of_view_crop_.end(), cmp_fov());
	}

	BuildFoVCropMap();

	regenerate_thumbnails_ = static_cast<GenThumbMode>(app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_GEN_THUMB, regenerate_thumbnails_));

	read_thumbs_from_db_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_THUMB_ACCESS, read_thumbs_from_db_ ? 1 : 0);

	allow_magnifying_above100_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_MAGNIFY_100, allow_magnifying_above100_ ? 1 : 0);
	allow_zoom_to_fill_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_MAGNIFY_FILL, allow_zoom_to_fill_ ? 1 : 0);
	percent_of_image_to_hide_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_IMG_HIDE, percent_of_image_to_hide_);

	close_app_when_viewer_exits_ = !!app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_CLOSE_APP, close_app_when_viewer_exits_ ? 1 : 0);

	smooth_scrolling_speed_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_SMOOTH_SPEED_VAL, smooth_scrolling_speed_);

	//HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(img_default_tag_font_), &img_default_tag_font_);
	//img_default_tag_font_.lfHeight -= 2;
	_tcscpy(img_default_tag_font_.lfFaceName, _T("Microsoft Yahei"));
	//img_default_tag_font_.lfWeight = FW_BOLD;
	img_tag_font_ = img_default_tag_font_;

	{
		BYTE* data= 0;
		UINT len= 0;
		app->GetProfileBinary(REGISTRY_ENTRY_CONFIG, REG_TAG_FONT, &data, &len);
		if (data != 0)
		{
			if (len == sizeof img_tag_font_)
				memcpy(&img_tag_font_, data, sizeof img_tag_font_);
			delete [] data;
		}
	}

	// photo ctrl colors
	{
		std::vector<COLORREF> def_colors= ::GetPhotoCtrlDefaultColors(app_colors_);
		std::vector<COLORREF> colors;
		std::vector<char> use_custom;

		GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_LIST_CTRL_COLORS, colors);
		GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_USE_LIST_CTRL_COLORS, use_custom);

		if (use_custom.empty() && !app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_LIST_CTRL_SYS_CLR, 0))
			use_custom.resize(def_colors.size(), 1);

		main_wnd_colors_.Create(colors, use_custom, def_colors);
	}

	// viewer window colors
	{
		std::vector<COLORREF> def_colors= ::GetViewerWndDefaultColors();
		std::vector<COLORREF> colors;
		std::vector<char> use_custom;

		GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_VIEWER_COLORS, colors);
		GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_USE_VIEWER_COLORS, use_custom);

		if (use_custom.empty() && !app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_VIEWER_DEFAULT_CLR, 0))
			use_custom.resize(def_colors.size(), 1);

		viewer_wnd_colors_.Create(colors, use_custom, def_colors);
	}

	// pane caption colors (defaults)
	{
		COLORREF base = app_colors_[AppColors::Background];
		COLORREF text = app_colors_[AppColors::DimText];
		COLORREF active = app_colors_[AppColors::ActiveText];
		std::vector<COLORREF> def_colors;
		def_colors.reserve(7);
		def_colors.push_back(base);
		def_colors.push_back(base); //::GetSysColor(COLOR_ACTIVECAPTION));
		def_colors.push_back(base); //::GetSysColor(COLOR_INACTIVECAPTION));
		def_colors.push_back(active);//::GetSysColor(COLOR_CAPTIONTEXT));
		def_colors.push_back(text);//::GetSysColor(COLOR_INACTIVECAPTIONTEXT));
		COLORREF sep = app_colors_[AppColors::Separator];
/*		// hack: replace white by something else
		if (sep == RGB(255,255,255))
			sep = ::GetSysColor(COLOR_3DFACE); */
		def_colors.push_back(sep);	// separator (between panes)
		def_colors.push_back(base);

		std::vector<COLORREF> colors;
		std::vector<char> use_custom;

		//GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_PANE_CAPTION_COLORS, colors);
		//GetProfileVector(REGISTRY_ENTRY_CONFIG, REG_USE_PANE_CAPTION_COLORS, use_custom);

		pane_caption_colors_.Create(colors, use_custom, def_colors);
	}

	img_cache_db_path_ = app->GetProfileString(REGISTRY_ENTRY_CONFIG, REG_DB_PATH, img_cache_db_path_.c_str());
	thumbnail_sharpening_ = app->GetProfileInt(REGISTRY_ENTRY_CONFIG, REG_THUMB_SHARPENING, thumbnail_sharpening_);
}


WORD Config::GetLanguageId() const
{
	switch (language_)
	{
	case 0:
		return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	case 1:
		return MAKELANGID(LANG_POLISH, SUBLANG_CHINESE_SIMPLIFIED);
	case 2:
		return MAKELANGID(LANG_DUTCH, SUBLANG_DUTCH);
	case 3:
		return MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN);
	case 4:
		return MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH);
	case 5:
		return MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN);
	case 6:
		return MAKELANGID(LANG_ITALIAN, SUBLANG_ITALIAN);
	}

	return MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
}


bool Config::ScanFileType(int index) const
{
	if (index >= 0 && index <= FT_LAST)
		return file_types_[index].scan;

	ASSERT(false);
	return false;
}


bool Config::MarkFileType(int index) const
{
	if (index >= 0 && index <= FT_LAST)
		return file_types_[index].show_marker;

	ASSERT(false);
	return false;
}


bool Config::NoExifIndicator(int index) const
{
	if (index >= 0 && index <= FT_LAST)
		return file_types_[index].show_no_exif;

	ASSERT(false);
	return false;
}


void Config::ToggleScanFileType(int index)
{
	if (index >= 0 && index <= FT_LAST)
		file_types_[index].scan = !file_types_[index].scan;
	else
	{ ASSERT(false); }
}


void Config::SetScanFileType(int index, bool scan)
{
	if (index >= 0 && index <= FT_LAST)
		file_types_[index].scan = scan;
	else
	{ ASSERT(false); }
}


void Config::SetAllFileTypes(bool scan)
{
	const size_t count= file_types_.size();
	for (size_t i= 0; i < count; ++i)
		file_types_[i].scan = scan;
}


void FileTypeConfig::Store(const TCHAR* reg_key) const
{
	TCHAR buf[4]= { scan ? '1' : '0', show_marker ? '1' : '0', show_no_exif ? '1' : '0', 0 };
	AfxGetApp()->WriteProfileString(reg_key, internal_name.c_str(), buf);
}

void FileTypeConfig::Restore(const TCHAR* reg_key)
{
	CString str= AfxGetApp()->GetProfileString(reg_key, internal_name.c_str());
	if (str.GetLength() == 3)
	{
		scan = str[0] == '1';
		show_marker = str[1] == '1';
		show_no_exif = str[2] == '1';
	}
}


void Config::NotifyGammaChanged()
{
	std::list<OnGammaChange>::iterator it= gamma_callbacks_.begin();

	bool use_gamma= true;
	while (it != gamma_callbacks_.end())
	{
		(*it)(gamma_, use_gamma);
		++it;
	}
}


double CalcFoVCrop(const TCHAR* factor)
{
	TCHAR* end= 0;
	double val= _tcstod(factor, &end);
	while (isspace(*end))
		++end;

	if (*end == '/')
	{
		double denom= _tcstod(end + 1, &end);
		if (denom > 0.0)
			return val / denom;
	}

	return val;
}


void Config::BuildFoVCropMap()
{
	foV_crop_map_.clear();

	size_t count= field_of_view_crop_.size();
	for (size_t i= 0; i < count; ++i)
		foV_crop_map_[field_of_view_crop_[i].first] = CalcFoVCrop(field_of_view_crop_[i].second.c_str());
}


double Config::GetFocalLengthMultiplier(const String& model) const
{
	std::unordered_map<String, double>::const_iterator it= foV_crop_map_.find(model);

	if (it != foV_crop_map_.end())
		return it->second;

	return 0.0;
}


double Config::GetScreenAspectRatio() const
{
	if (correct_CRT_aspect_ratio_ && vert_resolution_ > 0.0f && horz_resolution_ > 0.0f)
		return static_cast<double>(horz_resolution_) / vert_resolution_;
	else
		return 1.0;
}


float Config::GetThumbnailSharpening() const
{
	return thumbnail_sharpening_ / 100.0f;
}


ApplicationColors& Config::AppColors()
{
	return app_colors_;
}


void Config::UpdateAppColors()
{
	std::vector<std::pair<::AppColors, ColorCfg>> colors;
	colors.reserve(20);

	auto bkgnd = ::GetSysColor(COLOR_3DFACE);//RGB(235, 235, 235);// gray background
	auto text = ::GetSysColor(COLOR_WINDOWTEXT);	// light text

	colors.push_back(std::make_pair(AppColors::Background, bkgnd));
	colors.push_back(std::make_pair(AppColors::Text, text));
	colors.push_back(std::make_pair(AppColors::DimText, CalcNewColor(bkgnd, text, 0.60f)));
	colors.push_back(std::make_pair(AppColors::DisabledText, CalcNewColor(bkgnd, text, 0.40f)));
	colors.push_back(std::make_pair(AppColors::ActiveText, text));
	colors.push_back(std::make_pair(AppColors::SelectedText, text));//RGB(255, 255, 255)));//::GetSysColor(COLOR_HIGHLIGHTTEXT)));
	auto sep = CalcShade(bkgnd, -30.0f);
	colors.push_back(std::make_pair(AppColors::Separator, sep));
	colors.push_back(std::make_pair(AppColors::SecondarySeparator,  CalcShade(bkgnd, -20.0f)));//CalcNewColor(sep, bkgnd, -2.50f)));
	colors.push_back(std::make_pair(AppColors::EditBox, RGB(255,255,255)));//CalcShade(bkgnd, -20.0f)));
	auto sel = RGB(145, 201, 247);//auto sel = RGB(247, 123, 0);
	colors.push_back(std::make_pair(AppColors::Selection, sel));//::GetSysColor(COLOR_HIGHLIGHT)));
	colors.push_back(std::make_pair(AppColors::AccentBackground, CalcShade(bkgnd, +12.0f)));
	colors.push_back(std::make_pair(AppColors::Activebg, CalcNewColor(sel, bkgnd, 0.6f)));//RGB(250, 210, 170)));

	app_colors_.SetColors(colors);
}

CGdiObject* Config::SelectDefaultFont(CDC& dc, bool bold)
{
	return dc.SelectObject(bold ? &GetDefaultGuiBoldFont() : &GetDefaultGuiFont());
}


HFONT Config::GetDefaultFont(bool bold)
{
	return bold ? GetDefaultGuiBoldFont() : GetDefaultGuiFont();
}
