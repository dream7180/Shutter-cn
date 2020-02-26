/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2011 Michael Kowalski
____________________________________________________________________________*/

// Config.h: interface for the Config class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_H__BACE0CA4_2A03_11D2_809C_DFD6FB55D137__INCLUDED_)
#define AFX_CONFIG_H__BACE0CA4_2A03_11D2_809C_DFD6FB55D137__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include <boost/function.hpp>
#include "ICMProfile.h"
#include "DibDispMethod.h"
#include "JPEGDecoderMethod.h"
#include "GenThumbMode.h"
#include "FileTypeIndex.h"
#include "ColorConfiguration.h"
#include "AppColors.h"

struct FileTypeConfig
{
	String name;
	String extensions;
	String internal_name;
	bool scan;
	bool show_marker;
	bool show_no_exif;

	FileTypeConfig(const TCHAR* name, const TCHAR* ext, const TCHAR* intern_name, bool show_marker= true)
		: name(name), extensions(ext), internal_name(intern_name),
		scan(true), show_marker(show_marker), show_no_exif(!show_marker)
	{}
	FileTypeConfig() : scan(true), show_marker(true)
	{}

	void Store(const TCHAR* reg_key) const;
	void Restore(const TCHAR* reg_key);
};

class ConfigRegistrar;


class Config
{
public:
	Config();
	~Config();

	JpegDecoderMethod	dct_method_;				// jpeglib decoder param: DCT type
	double				gamma_;						// gamma value
	COLORREF			rgb_cur_selection_;			// preview pane current photo selection color
	bool				keep_sel_photo_centered_;	// keep selected photo in preview pane centered
	bool				preload_photos_;			// preload photographs to speed up browsing
	LOGFONT				description_font_;			// font used for photo description (user config.)
	LOGFONT				default_description_font_;	// default font used for photo description
	int					language_;					// language used in program
	double				viewer_ui_gamma_correction_;// brightness control for viewer bitmap elements
	float				horz_resolution_;			// monitor resolution
	float				vert_resolution_;
	bool				correct_CRT_aspect_ratio_;
	CString				open_photo_app_;			// path to application opening photo images
	CString				open_raw_photo_app_;		// path to application opening raw photos (DNG, NEF, CRW, etc.)
	DibDispMethod		display_method_;			// display method: DibDraw, bit stretch, bit stretch + halftone
	int					image_cache_size_;			// image cache size [% of RAM]
	std::vector<FileTypeConfig> file_types_;				// files types ExifPro loads
	bool				save_tags_to_photo_;		// save applied tags in a photo's file?
	bool				warn_about_broken_thumbnail_img_;	// show msg box complaining about broken thumbnail in EXIF block
	std::vector<uint16>	balloon_fields_;			// fields displayed inside info balloon
	uint32				db_file_length_limit_mb_;	// growth limit for image database in megabytes
	int					image_blending_;			// image alpha blending in a viewer window (0 - always, 1 - slide show, 2 - never)
	ICMProfilePtr		main_wnd_;
	ICMProfilePtr		viewer_;
	ICMProfilePtr		default_photo_;
	ICMProfilePtr		default_printer_;
	std::vector<std::pair<String, String> > field_of_view_crop_;	// camera model name & FoV crop (as a text)
	GenThumbMode		regenerate_thumbnails_;		// generate thumbnails while scanning photos?
	bool				read_thumbs_from_db_;		// if true thumbnails will be read from cache file freeing up memory
	bool				allow_magnifying_above100_;	// if true 'size to fit' may magnify small images above 100%
	bool				allow_zoom_to_fill_;		// if true 'size to fit' will try to fill entire viewer window (leaving no bars)
	int					percent_of_image_to_hide_;	// how much of an image area can be hidden by 'zoom to fill'
	bool				close_app_when_viewer_exits_;	// when ExifPro was started with a path to the single image, exiting
													// viewer will normally close the app, unless this flag is false
	int					smooth_scrolling_speed_;	// smooth scrolling speed divider use in a viewer preview bar
	LOGFONT				img_tag_font_;				// font used for overlaid tag didplay
	LOGFONT				img_default_tag_font_;		// default font for above use
	ColorConfiguration	main_wnd_colors_;			// colors to customize photo list control
	ColorConfiguration	viewer_wnd_colors_;			// colors to customize viewer window
	ColorConfiguration	pane_caption_colors_;		// pane caption colors: active & inactive
	// active tab/inactive tab, active tab text/inactive tab text
	ApplicationColors&	AppColors();
	void UpdateAppColors();

	String				img_cache_db_path_;			// path to the location for storing image cache file
	int					thumbnail_sharpening_;		// amount of sharpening for thumbnails (from 0-100, translated to 0.0-1.0 for UnsharpMask)

	bool ScanFileType(FileTypeIndex type) const	{ return file_types_[type].scan; }
	bool ScanFileType(int index) const;
	bool MarkFileType(FileTypeIndex type) const	{ return file_types_[type].show_marker; }
	bool MarkFileType(int index) const;
	void ToggleScanFileType(int index);
	void SetScanFileType(int index, bool scan= true);
	void SetAllFileTypes(bool scan);
	bool NoExifIndicator(int index) const;

	void NotifyGammaChanged();

	void Store();
	void Restore();

	WORD GetLanguageId() const;

	// find field of view crop factor for given camera (returns 0.0 if not found)
	double GetFocalLengthMultiplier(const String& model) const;

	void BuildFoVCropMap();

	double GetScreenAspectRatio() const;

	float GetThumbnailSharpening() const;

	CGdiObject* SelectDefaultFont(CDC& dc, bool bold = false);
	HFONT GetDefaultFont(bool bold = false);

private:
	friend class ConfigRegistrar;

	typedef boost::function2<void, double, bool> OnGammaChange;
	typedef boost::function3<void, float, float, bool> OnCorrectRatio;

	// callbacks invoked when gamma settings are changed
	std::list<OnGammaChange> gamma_callbacks_;
	// callbacks invoked when ratio correction settings are changed
	std::list<OnCorrectRatio> ratio_callbacks_;

	// camera model to field of view crop
	std::unordered_map<String, double> foV_crop_map_;

	void SaveFoVCrop();

	ApplicationColors app_colors_;
};


class ConfigRegistrar
{
public:
	ConfigRegistrar(Config& settings) : settings_(settings),
		gamma_(settings_.gamma_callbacks_.end()),
		ratio_(settings_.ratio_callbacks_.end())
	{}

	void RegisterForGammaChange(Config::OnGammaChange& func)
	{
		settings_.gamma_callbacks_.push_back(func);
		gamma_ = --settings_.gamma_callbacks_.end();
	}

	void RegisterForCorrectRatio(Config::OnCorrectRatio& func)
	{
		settings_.ratio_callbacks_.push_back(func);
		ratio_ = --settings_.ratio_callbacks_.end();
	}

	~ConfigRegistrar()
	{
		if (gamma_ != settings_.gamma_callbacks_.end())
			settings_.gamma_callbacks_.erase(gamma_);
		if (ratio_ != settings_.ratio_callbacks_.end())
			settings_.ratio_callbacks_.erase(ratio_);
	}

private:
	Config& settings_;
	std::list<Config::OnGammaChange>::iterator gamma_;
	std::list<Config::OnCorrectRatio>::iterator ratio_;
};



extern Config g_Settings;

#endif // !defined(AFX_CONFIG_H__BACE0CA4_2A03_11D2_809C_DFD6FB55D137__INCLUDED_)
