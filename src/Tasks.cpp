/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "Tasks.h"
#include "HeaderDialog.h"
#include "RString.h"
#include "ToolDlg.h"
#include "CatchAll.h"
#include "PhotoCache.h"
#include "FormatFileSize.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


extern AutoPtr<PhotoCache> global_photo_cache;		// one central photo cache

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ResizingThread.h"
#include "ResizeDlg.h"
#include "ProcessingProgressDlg.h"

extern const TCHAR* PathIllegalChars()
{
	return _T("|<>/\\\":*?");
}

extern String ReplaceIllegalChars(const String& text, TCHAR replacement_char)
{
	String str= text;
	String::size_type pos= 0;
	// replace illegal chars
	for (;;)
	{
		pos = str.find_first_of(PathIllegalChars(), pos);
		if (pos == String::npos)
			break;
		str[pos] = replacement_char;
	}

	return str;
}


extern String ReplaceIllegalChars(const String& text)
{
	return ReplaceIllegalChars(text, _T('_'));
}


double FindCommonRatio(const VectPhotoInfo& selected, double limit= 1.0 / 1000.0)
{
	double ratio= 0.0;
	double epsilon= 0.001;
	const double invalid= -1.0;

	// find photos common ratio
	for (VectPhotoInfo::const_iterator it= selected.begin(); it != selected.end(); ++it)
	{
		const PhotoInfo& photo= **it;

		if (photo.GetWidth() == 0 || photo.GetHeight() == 0)
		{
			ratio = invalid;	// missing photo dimensions
			break;
		}

		double cur_ratio= photo.GetWidth() < photo.GetHeight() ?
			double(photo.GetWidth()) / double(photo.GetHeight()) : double(photo.GetHeight()) / double(photo.GetWidth());

		if (ratio == 0.0)
		{
			ratio = cur_ratio;
		}
		else if (cur_ratio < ratio - epsilon || cur_ratio > ratio + epsilon)
		{
			ratio = invalid;	// photos have different ratios
			break;
		}
	}

	if (ratio > 0.0 && limit > 0.0 && (ratio < limit || ratio > 1.0 / limit))
		ratio = invalid;		// ratio too small/big

	return ratio;
}


// find common orientation of images (all landscape, all portrait, or mixed)

OrientationOfImages FindCommonOrientation(const VectPhotoInfo& photos)
{
	if (photos.empty())
		return MIXED_ORIENTATION;

	bool horizontal= photos.front()->HorizontalOrientation();

	for (VectPhotoInfo::const_iterator it= photos.begin(); it != photos.end(); ++it)
	{
		const PhotoInfo& photo= **it;

		if (photo.HorizontalOrientation())
		{
			if (!horizontal)
				return MIXED_ORIENTATION;
		}
		else
		{
			if (horizontal)
				return MIXED_ORIENTATION;
		}
	}

	return horizontal ? LANDSCAPE_ORIENTATION : PORTRAIT_ORIENTATION;
}


bool CTaskResize::Go()
{
	std::vector<ResizePhotoInfo> photos;
	photos.resize(selected_.size());

	double ratio= FindCommonRatio(selected_);
	if (ratio == -1.0)
		ratio = 1.0;

	OrientationOfImages orientation= FindCommonOrientation(selected_);

	// enter resizing params
	ResizeDlg dlg(0, ratio, orientation);
	HeaderDialog dlgHdr(dlg, _T("Resize"), HeaderDialog::IMG_RESIZE, parent_);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	int index= 0;
	for (VectPhotoInfo::iterator it= selected_.begin(); it != selected_.end(); ++it)
	{
		PhotoInfo& photo= **it;
		ResizePhotoInfo& inf= photos[index++];

		inf.orientation_	= photo.OrientationField();
		inf.photo_size_.cx	= photo.GetWidth();
		inf.photo_size_.cy	= photo.GetHeight();
		inf.src_file_		= photo.GetOriginalPath();
		inf.decoder_		= photo.GetDecoder();
		inf.dest_file_		= dlg.same_dir_ == 0 ? inf.src_file_.GetDir() : dlg.GetDestPath();
		inf.dest_file_.AppendDirSeparator();
		inf.dest_file_		+= inf.src_file_.GetFileName();
		inf.dest_file_		+= ReplaceIllegalChars(static_cast<const TCHAR*>(dlg.suffix_));
		inf.dest_file_		+= dlg.output_format_ == 0 ? _T(".jpg") : _T(".bmp");
		inf.fit_this_size_	= false;
		inf.photo_			= &photo;	// used to extract EXIF block
	}

	// resize photos
	ResizeFormat fmt(dlg.image_size_, dlg.GetJPEGQuality(), !!dlg.dlg_options_.progressive_jpeg_,
		!!dlg.dlg_options_.baseline_jpeg_, dlg.output_format_, CSize(0, 0),
		static_cast<Dib::ResizeMethod>(dlg.dlg_options_.resizing_method_), !!dlg.dlg_options_.preserve_exif_block_,
		!!dlg.dlg_options_.copyTags_);

	ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ResizingThread(photos, fmt)));

	ProcessingProgressDlg progress(parent_, proc, _T("Resizing in Progress"), 0, ProcessingProgressDlg::AUTO_CLOSE);
	progress.DoModal();

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GenSlideShowDlg.h"
#include "SlideShowGenerator.h"


bool CTaskGenSlideShow::Go()
{
	GenSlideShowDlg dlg;
	HeaderDialog dlgHdr(dlg, _T("Slide Show"), HeaderDialog::IMG_SLIDE_SHOW);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	std::vector<ResizePhotoInfo> photos;
	photos.resize(selected_.size());

	int index= 0;
	for (VectPhotoInfo::iterator it= selected_.begin(); it != selected_.end(); ++it)
	{
		PhotoInfo& photo= **it;

		ResizePhotoInfo& inf= photos[index++];
		inf.orientation_	= photo.OrientationField();
		inf.photo_size_.cx	= photo.GetWidth();
		inf.photo_size_.cy	= photo.GetHeight();
		inf.src_file_		= photo.GetOriginalPath();
		inf.decoder_		= photo.GetDecoder();
		//inf.dest_file_; -- do not write file
		inf.fit_this_size_	= false;
	}

	// generate slide show
	ResizeFormat fmt(CSize(dlg.width_, dlg.height_), dlg.GetJPEGQuality(), false, false, 0, CSize(0, 0), Dib::RESIZE_CUBIC, false, false);

	CSlideShowGenerator gen;
	if (!gen.WriteSlideShow(dlg.GetDestPath(), dlg.delay_, !!dlg.full_screen_, !!dlg.loopRepeatedly_))
		throw String(_T("Cannot write slide show application in ")) + dlg.GetDestPath();

	// single thread processing for slide show;
	// currently slide show needs to append images one by one in the order specified
	ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ResizingThread(photos, fmt, &gen)), 1);

	ProcessingProgressDlg progress(nullptr, proc, _T("Slide Show Generation in Progress"), nullptr, ProcessingProgressDlg::INPUT_OUTPUT);
	progress.SetSlideShow(&gen);
	progress.DoModal();

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GenHTMLAlbumDlg.h"

void ReadTemplate(const String& path, std::string& content)
{
	CFile file(path.c_str(), CFile::modeRead); // | CFile::typeText);

	content.resize(static_cast<DWORD>(file.GetLength()));

	if (content.empty())
		throw String(_T("Template file is empty: ") + path);

	if (file.Read(&content[0], static_cast<UINT>(content.size())) != content.size())
		throw String(_T("Cannot read template file: ") + path);
}


std::string ColorToText(COLORREF rgb_color)
{
	std::ostringstream ost;
	ost << "#";
	ost.fill('0');
	ost << std::hex;
	ost.width(2);
	ost << int(GetRValue(rgb_color));
	ost.width(2);
	ost << int(GetGValue(rgb_color));
	ost.width(2);
	ost << int(GetBValue(rgb_color));
	return ost.str();
}


bool CTaskGenHTMLAlbum::Go()
{
	try
	{
		return Generate(parent_);
	}
	CATCH_ALL_W(parent_)

	return false;
}


bool CTaskGenHTMLAlbum::Generate(CWnd* parent)
{
	TCHAR prog_path[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), prog_path, _MAX_PATH));
	Path templates= Path(prog_path).GetDir();
//#ifdef _DEBUG
//	templates.AppendDir(_T("..\\ReleaseUnicode\\Templates"));
//#else
	templates.AppendDir(_T("Templates"));
//#endif

	// templates
	TCHAR* grid_file= _T("Grid.html");
	TCHAR* frames_file= _T("Frames.html");

	if (::GetFileAttributes((templates + grid_file).c_str()) == INVALID_FILE_ATTRIBUTES ||
		::GetFileAttributes((templates + frames_file).c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		parent->MessageBox((_T("Cannot open HTML template files.\n\nLocation: ") + templates).c_str(), 0,
			MB_ICONERROR | MB_OK);
		return false;
	}

	double ratio= FindCommonRatio(selected_);
	if (ratio == -1.0)
		ratio = 1.0;

	OrientationOfImages orientation= FindCommonOrientation(selected_);

	GenHTMLAlbumDlg dlg(0, ratio, orientation);
	HeaderDialog dlgHdr(dlg, _T("HTML Album"), HeaderDialog::IMG_HTML2, parent);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	std::vector<ResizePhotoInfo> photos;
	photos.resize(selected_.size());

	Path pathRoot= dlg.GetRootDir();
	{
		Path photos= pathRoot;
		photos.AppendDir(dlg.photos_dir_, false);

		Path thumbs= pathRoot;
		thumbs.AppendDir(dlg.thumbs_dir_, false);

		if (!photos.CreateFolders() || !thumbs.CreateFolders())
		{
			parent->MessageBox(_T("Error creating destination folders."), 0, MB_ICONERROR | MB_OK);
			return false;
		}
	}

	int index= 0;
	// prepare info for resizing photos -----------------------------
	for (VectPhotoInfo::iterator it= selected_.begin(); it != selected_.end(); ++it)
	{
		PhotoInfo& photo= **it;

		ResizePhotoInfo& inf= photos[index++];
		inf.orientation_	= photo.OrientationField();
		inf.photo_size_.cx	= photo.GetWidth();
		inf.photo_size_.cy	= photo.GetHeight();
		inf.src_file_		= photo.GetOriginalPath();
		inf.decoder_		= photo.GetDecoder();
		inf.date_time_		= photo.DateTimeStr();
		inf.dest_file_		= dlg.GetRootDir();
		inf.dest_file_.AppendDir(dlg.photos_dir_);
		inf.dest_file_		+= inf.src_file_.GetFileName();
		inf.dest_file_		+= _T(".jpg"); // HTML album does not have BMP option: dlg.output_format_ == 0 ? _T(".jpg") : _T(".bmp");
		inf.fit_this_size_	= true;
		inf.thumbnail_size_.cx	= dlg.thumb_width_;
		inf.thumbnail_size_.cy	= dlg.thumb_height_;
		inf.dest_thumbnail_	= dlg.GetRootDir();
		inf.dest_thumbnail_.AppendDir(dlg.thumbs_dir_);
		inf.dest_thumbnail_	+= _T("th_");
		inf.dest_thumbnail_	+= inf.src_file_.GetFileName();
		inf.dest_thumbnail_	+= _T(".jpg");
		inf.description_	= !photo.PhotoDescription().empty() ? photo.PhotoDescription() : photo.GetExifDescription();
		inf.photo_			= &photo;	// used to extract EXIF block
	}

	// resize photos ------------------------------------------------
	bool progressive_jpeg= true;
	bool baseline_jpeg= false;
	CSize thumb_size(dlg.thumb_width_, dlg.thumb_height_);
	CSize image_size(dlg.width_, dlg.height_);
	if (dlg.size_ == 0)
		image_size.cx = image_size.cy = 0;
	ResizeFormat fmt(image_size, dlg.GetJPEGQuality(), progressive_jpeg, baseline_jpeg, 0, thumb_size,
		static_cast<Dib::ResizeMethod>(dlg.dlg_options_.resizing_method_), !!dlg.dlg_options_.preserve_exif_block_, !!dlg.dlg_options_.copyTags_);

	ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ResizingThread(photos, fmt)));

	ProcessingProgressDlg progress(parent, proc, _T("HTML Album Generation in Progress"), 0,
		ProcessingProgressDlg::INPUT_OUTPUT | ProcessingProgressDlg::AUTO_CLOSE);

	if (progress.DoModal() != IDOK)
		return false;

	// generate HTML pages ------------------------------------------

	bool use_frame_set= dlg.type_ == 1;

	std::string a_template;
	ReadTemplate(templates + (use_frame_set ? frames_file : grid_file), a_template);

	std::string img_template;
	ReadTemplate(templates + (use_frame_set ? _T("FrameImage.html") : _T("GridImage.html")), img_template);

	String title= dlg.page_title_;
	Replace(a_template, "%title", title);
	Replace(a_template, "%caption", title);
	Replace(a_template, "%page_color", ColorToText(dlg.rgb_page_backgnd_));
	Replace(a_template, "%text_color", ColorToText(dlg.rgb_page_text_));

	Replace(img_template, "%page_color", ColorToText(dlg.rgb_page_backgnd_));
	Replace(img_template, "%text_color", ColorToText(dlg.rgb_page_text_));

	int grid_cols= use_frame_set ? static_cast<int>(photos.size()) : dlg.grid_columns_;
	String path_to_first_img;
	bool img_text_items[]= { !!dlg.text_1_, !!dlg.text_2_, !!dlg.text_3_, !!dlg.text_4_ };
	std::string table= GenerateTable(a_template, img_template, grid_cols, photos, dlg.GetThumbsDir(),
		dlg.GetPhotosDir(), dlg.item_text_type_, img_text_items, path_to_first_img, String(dlg.page_file_name_));

	Path pathTarget= pathRoot;
	if (use_frame_set)
	{
		pathTarget.AppendDir(dlg.page_file_name_, false);
		pathTarget.AppendToFileName(_T("_frame"));
	}
	else
		pathTarget.AppendDir(dlg.page_file_name_, false);

	CFile file(pathTarget.c_str(), CFile::modeWrite | CFile::modeCreate);
	file.Write(table.data(), static_cast<UINT>(table.size()));
	file.Close();

	if (use_frame_set)	// write frame set HTML file
	{
		std::string frame_set;
		ReadTemplate(templates + _T("FrameSet.html"), frame_set);

		Replace(frame_set, "%title", title);
		Replace(frame_set, "%thumbnails", pathTarget.GetFileNameAndExt());
		Replace(frame_set, "%target_image", path_to_first_img);

		Path path= pathRoot;
		path.AppendDir(dlg.page_file_name_, false);

		CFile file(path.c_str(), CFile::modeWrite | CFile::modeCreate);
		file.Write(frame_set.data(), static_cast<UINT>(frame_set.size()));
		file.Close();

		pathTarget = path;
	}

	if (dlg.open_in_browser_)
		::ShellExecute(0, _T("open"), pathTarget.c_str(), 0, 0, SW_SHOWNORMAL);

	return true;
}


std::string CTaskGenHTMLAlbum::GenerateTable(const std::string& a_template, const std::string& img_template,
										int grid_cols, const std::vector<ResizePhotoInfo>& photos,
										Path pathThumbs, Path pathPhotos, int item_text_type,
										bool img_text_items[], String& path_to_first_img, String main_page)
{
	const char* row_start=  "[row_start]";
	const char* row_end=    "[row_end]";
	const char* cell_start= "[cell_start]";
	const char* cell_end=   "[cell_end]";
	const char* cell_size=  "[cell_size]";

	std::string page_before, page_after;
	std::string row_str= FindText(a_template, row_start, row_end, page_before, page_after);
	if (row_str.empty())
	{
		AfxMessageBox(_T("HTML template error: missing row definition"), MB_ICONERROR | MB_OK);
		return std::string();
	}

	std::string row_before, row_after;
	std::string cell= FindText(row_str, cell_start, cell_end, row_before, row_after);
	if (cell.empty())
	{
		AfxMessageBox(_T("HTML template error: missing cell definition"), MB_ICONERROR | MB_OK);
		return std::string();
	}

	pathThumbs.AppendDirSeparator();
	pathThumbs.BackslashToForwardslash();

	pathPhotos.AppendDirSeparator();
	pathPhotos.BackslashToForwardslash();

	int count= static_cast<int>(photos.size());
	int row= grid_cols;

	std::string table;
	table.reserve(count * cell.size());	// rough estimation
	table = row_before;

	std::string photo;
	for (int index= 0; index < count; ++index, --row)
	{
		if (row == 0)
		{
			row = grid_cols;
			table += row_after;
			table += row_before;
		}

		// image text label
		std::string item_text;
		switch (item_text_type)
		{
		case 0:		// no text
			break;
		case 1:		// numbers
			{
				std::ostringstream ost;
				ost << index + 1 << '.';
				item_text = ost.str();
			}
			break;
		case 2:		// file name
			item_text = WStringToUTF8(photos[index].src_file_.GetFileName());
			break;
		case 3:		// date & time
			item_text = WStringToUTF8(photos[index].date_time_);
			break;
		case 4:		// file description
			item_text = WStringToUTF8(photos[index].description_);
			break;
		default:
			ASSERT(false);
			break;
		}

		const TCHAR* PAGE= _T("image_");
		// image HTML page
		oStringstream ost;
		ost << PAGE << index + 1 << _T(".html");

		oStringstream previous;
		previous << _T("<a href=\"") << PAGE << index + 0 << _T(".html") << _T("\">Previous</a>&nbsp;");

		oStringstream next;
		next << _T("&nbsp;<a href=\"") << PAGE << index + 2 << _T(".html") << _T("\">Next</a>");

		oStringstream back;
		{
			back << _T("&nbsp;<a href=\"");

			size_t start= pathPhotos.find(L"/", 0);
			while (start != String::npos)
			{
				back << L"../";
				start = pathPhotos.find(L"/", start + 1);
			}

			back << main_page << _T("\">Go Up</a>");
		}

		std::string img_page= img_template;
		Replace(img_page, "%image_href", photos[index].dest_file_.GetFileNameAndExt());
		Replace(img_page, "%title", item_text);
		Replace(img_page, "%link_prev", index > 0 ? previous.str() : _T(""));
		Replace(img_page, "%link_next", index + 1 < count ? next.str() : _T(""));
		Replace(img_page, "%link_back", back.str());

		std::string image_text;
		const char BR[]= "\r\n<br>\r\n";
		if (img_text_items[0])	// number
		{
			std::ostringstream ost;
			ost << index + 1 << '.';
			image_text = ost.str();
		}
		if (img_text_items[1])	// file name
		{
			if (!image_text.empty())	image_text += BR;
			image_text += WStringToUTF8(photos[index].src_file_.GetFileName());
		}
		if (img_text_items[2])	// file date & time
		{
			if (!image_text.empty())	image_text += BR;
			image_text += WStringToUTF8(photos[index].date_time_);
		}
		if (img_text_items[3])	// description
		{
			if (!image_text.empty())	image_text += BR;
			image_text += WStringToUTF8(photos[index].description_);
		}

		Replace(img_page, "%photo_text", image_text);

		// write image page
		Path pathPage= photos[index].dest_file_.GetDir();
		pathPage.AppendDir(ost.str().c_str(), false);
		CFile filePage(pathPage.c_str(), CFile::modeWrite | CFile::modeCreate);
		filePage.Write(img_page.data(), static_cast<UINT>(img_page.size()));
		filePage.Close();

		photo = cell;
		Replace(photo, "%photo_href", pathPhotos + ost.str());
		Replace(photo, "%thumb_width", photos[index].thumbnail_size_.cx);
		Replace(photo, "%thumb_height", photos[index].thumbnail_size_.cy);
		Replace(photo, "%thumb_src", pathThumbs + photos[index].dest_thumbnail_.GetFileNameAndExt());
		Replace(photo, "%photo_text", item_text);

		table += photo;

		if (index == 0)
			path_to_first_img = pathPhotos + ost.str();
	}
	table += row_after;

	return page_before + table + page_after;
}


std::string CTaskGenHTMLAlbum::FindText(const std::string& a_template, const char* start, const char* end, std::string& before, std::string& after)
{
	std::string::size_type start_pos= a_template.find(start);
	std::string::size_type end_pos= a_template.find(end);

	if (start_pos == std::string::npos || end_pos == std::string::npos || start_pos > end_pos)
		return std::string();

	before = a_template.substr(0, start_pos);
	after = a_template.substr(end_pos + strlen(end));

	start_pos += strlen(start);
	return a_template.substr(start_pos, end_pos - start_pos);
}


std::string CTaskGenHTMLAlbum::WStringToUTF8(const std::wstring& value)
{
	int len= ::WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), 0, 0, 0, 0);
	if (len > 0)
	{
		std::string str(len, '\0');
		::WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), &str[0], len, 0, 0);
		return str;
	}
	else
		return std::string();
}


void CTaskGenHTMLAlbum::Replace(std::string& text, const char* key, const std::wstring& value)
{
	int len= ::WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), 0, 0, 0, 0);
	if (len > 0)
	{
		std::string str(len, '\0');
		::WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), &str[0], len, 0, 0);
		Replace(text, key, str);
	}
	else
		Replace(text, key, std::string());
}

void CTaskGenHTMLAlbum::Replace(std::string& text, const char* key, int value)
{
	std::ostringstream ost;
	ost << value;
	Replace(text, key, ost.str());
}

void CTaskGenHTMLAlbum::Replace(std::string& text, const char* key, const std::string& value)
{
	std::string::size_type pos= text.find(key);
	if (pos != std::string::npos)
		text.replace(pos, strlen(key), value);
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExifExport.h"
#include "ExportExifDlg.h"

bool CTaskExportEXIF::Go(CWnd* parent)
{
	CExportExifDlg dlgExp(all_, parent);
	HeaderDialog dlgHdr(dlgExp, _T("EXIF Export"), HeaderDialog::IMG_EXIF, parent);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	CExifExport exprt(dlgExp.out_file_, sel_columns_, col_order_, columns_, dlgExp.separator_, !!dlgExp.tags_);
	exprt.ExportHeader();

	CToolDlg dlg(RString(IDS_EXPORTING), 0);
	dlg.SetProgress(static_cast<int>(selected_.size()));
	dlg.ShowWindow(SW_SHOW);
	dlg.UpdateWindow();

	for (VectPhotoInfo::iterator it= selected_.begin(); it != selected_.end(); ++it)
	{
		PhotoInfoPtr photo= *it;

		// display file name
		dlg.SetFile(photo->GetName().c_str());

		exprt.ExportPhoto(*photo);

		dlg.Step();
	}
	dlg.DestroyWindow();

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "RotateDlg.h"

bool CTaskRotatePhotos::Go()
{
	CRotateDlg dlg(selected_, all_);
	HeaderDialog dlgHdr(dlg, all_ ? _T("Rotate All") : _T("Rotate"), HeaderDialog::IMG_ROTATE);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	RotationTransformation transform= dlg.GetOperation();
	//if (dlg.operation_ == 0)
	//	transform = ROTATE_90_DEG_CW;
	//else if (dlg.operation_ == 1)
	//	transform = ROTATE_90_DEG_COUNTERCW;
	//else if (dlg.operation_ < 0)
	//	transform = FIX_ROTATED_THUMBNAIL;
	RotatePhotos(transform, !!dlg.mirror_);

	return true;
}


void CTaskRotatePhotos::RotatePhotos(RotationTransformation transform, bool mirror)
{
	int count= static_cast<int>(selected_.size());

	CToolDlg dlg(_T("Rotating..."), 0);
	dlg.SetProgress(count);
	dlg.ShowWindow(SW_SHOW);
	dlg.UpdateWindow();

	int failed= 0;
	int not_supported= 0;
	for (VectPhotoInfo::iterator it= selected_.begin(); it != selected_.end(); ++it)
	{
		PhotoInfoPtr photo= *it;

		// display file name
		dlg.SetFile(photo->GetName().c_str());

		switch (photo->RotatePhoto(transform, mirror, parent_))
		{
		case -99:	// error: cannot rotate this type of file
			++not_supported;
			break;
		case -1:	// error
			++failed;
			break;
		case 0:		// file skipped
			break;
		case 1:		// ok
			break;
		}
/*
		int operation= transform;
		if (transform == 0)	// auto-rotation?
		{
			// determine necessary rotation
			if (photo->GetOrientation() == PhotoInfo::ORIENT_90CW)
				operation = CONDITIONAL_ROTATE_90_DEG_COUNTERCW;
			else if (photo->GetOrientation() == PhotoInfo::ORIENT_90CCW)
				operation = CONDITIONAL_ROTATE_90_DEG_CW;
		}

		if (operation != 0)	// any rotation necessary?
		{
			int new_width= 0;
			int new_height= 0;
			if (int result= Transform(operation, photo->GetWidth(), photo->GetHeight(), photo->path_.c_str(), &new_width, &new_height))
			{
				if (result != 9999)	// 9999 means file was skipped
					++failed;
			}
			else	// file rotated
			{
				photo->GetWidth() = new_width;
				photo->height_ = new_height;
			}
		}
*/
		dlg.Step();		// indicate progress
	}
	dlg.DestroyWindow();

	if (failed > 0 || not_supported > 0)
	{
		oStringstream ost;
		ost << _T("Problems: ") << failed + not_supported << _T(" file(s) were not processed.");
		if (not_supported > 0)
			ost << _T("\nFor ") << not_supported << _T(" file(s) rotation is not supported.");
		AfxMessageBox(ost.str().c_str(), MB_OK | MB_ICONERROR);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PrintDlg.h"


bool CTaskPrint::Go()
{
	PrintDlg dlg(selected_, false, folder_path_.c_str());
	HeaderDialog dlgHdr(dlg, _T("Print Photographs"), HeaderDialog::IMG_PRINT);
	if (dlgHdr.DoModal() != IDOK)
		return false;
	return true;
}


bool CTaskPrintThumbnails::Go()
{
	PrintDlg dlg(selected_, true, folder_path_.c_str());
	HeaderDialog dlgHdr(dlg, _T("Print Thumbnails"), HeaderDialog::IMG_PRINT_THUMBS);
	if (dlgHdr.DoModal() != IDOK)
		return false;
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "CopyTaggedDlg.h"


bool CTaskCopyTagged::Go()
{
	CCopyTaggedDlg dlg;
	HeaderDialog dlgHdr(dlg, _T("Copy Tagged"), HeaderDialog::IMG_COPY_TAGGED);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	CopyTaggedPhotos(tagged_photos_, Path(dlg.path_), dlg.separate_folders_ == 1);

	return true;
}


void CTaskCopyTagged::CopyTaggedPhotos(VectPhotoRanges& tagged_photos, Path dest_folder, bool create_separate_folders)
{
	DWORD attrib= ::GetFileAttributes(dest_folder.c_str());
	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		if (!::CreateDirectory(dest_folder.c_str(), 0))
		{
			String msg= _T("Cannot create folder: ");
			msg += dest_folder;
			AfxMessageBox(msg.c_str(), MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		attrib = ::GetFileAttributes(dest_folder.c_str());
	}

	if ((attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		String msg= _T("Given path doesn't point to valid folder: ");
		msg += dest_folder;
		AfxMessageBox(msg.c_str(), MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	dest_folder.AppendDirSeparator();
	const TCHAR* multiple= _T("Multiple Tags");

	std::vector<Path> dest_folders;

	// place each groups of photos in their own folder (based on their tag)?
	if (create_separate_folders)
	{
		dest_folders.reserve(tagged_photos.size());

		for (VectPhotoRanges::iterator it= tagged_photos.begin(); it != tagged_photos.end(); ++it)
		{
			Path folder= dest_folder;

			PhotoInfoPtr photo= *it->first;

			if (photo->GetTags().size() > 1)
				folder.AppendDir(multiple, false);
			else
				folder.AppendDir(TagNameToFolderName(photo->GetTags()[0]).c_str(), false);

			dest_folders.push_back(folder);

			DWORD attrib= ::GetFileAttributes(folder.c_str());
			if (attrib == INVALID_FILE_ATTRIBUTES || (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				if (!::CreateDirectory(folder.c_str(), 0))
				{
					String msg= _T("Cannot create folder: ");
					msg += folder;
					AfxMessageBox(msg.c_str(), MB_OK | MB_ICONEXCLAMATION);
					return;
				}
			}
		}
	}

	// prepare source files
	String src;
	src.reserve(0x10000);

	// prepare destination paths
	String dest;
	dest.reserve(0x10000);

	int index= 0;
	for (VectPhotoRanges::iterator it= tagged_photos.begin(); it != tagged_photos.end(); ++it, ++index)
	{
		VectPhotoInfo::iterator begin= it->first;
		VectPhotoInfo::iterator end= it->second;

		for (VectPhotoInfo::iterator photo= begin; photo != end; ++photo)
			src += (*photo)->GetOriginalPath() + _T('\0');

		if (create_separate_folders)
		{
			Path dest_dir= dest_folders.at(index);
			dest_dir.AppendDirSeparator();

			for (VectPhotoInfo::iterator photo= begin; photo != end; ++photo)
				dest += dest_dir + (*photo)->GetOriginalPath().GetFileNameAndExt() + _T('\0');
		}
	}

	if (!create_separate_folders)
		dest = dest_folder;

	src += _T('\0');
	dest += _T('\0');

	SHFILEOPSTRUCT op;
	op.hwnd = *parent_;
	op.wFunc = FO_COPY;
	op.pFrom = src.data();
	op.pTo = dest.data();
	op.fFlags = FOF_WANTNUKEWARNING;
	if (create_separate_folders)
		op.fFlags |= FOF_MULTIDESTFILES;
	op.fAnyOperationsAborted = false;
	op.hNameMappings = 0;
	op.lpszProgressTitle = 0;
	if (::SHFileOperation(&op) == 0)
	{
		//AfxMessageBox?
	}
}


String CTaskCopyTagged::TagNameToFolderName(const String& tag)
{
	return ReplaceIllegalChars(tag);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#include "DescriptionDlg.h"

bool CTaskEditDescription::Go()
{
	CDescriptionDlg dlg(parent_, photo_, photos_, selected_, photo_changed_);
	HeaderDialog dlgHdr(dlg, _T("Description"), HeaderDialog::IMG_PENCIL);
	dlgHdr.SetTitleDuringInit(false);	// description dlg provides its own title
	if (dlgHdr.DoModal() != IDOK)
		return false;

	return true;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ImageManipulationDlg.h"


bool CTaskImgManipulation::Go()
{
	ImageManipulationDlg dlg(parent_, photos_, global_photo_cache.get());
	HeaderDialog dlgHdr(dlg, _T("Adjustments"), HeaderDialog::IMG_ADJUST);
	if (dlgHdr.DoModal() != IDOK)
		return false;
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "DateAdjustDlg.h"
#include "DateTimeAdjThread.h"


bool CTaskDateTimeAdjustment::Go()
{
	DateAdjustDlg dlg(parent_, photos_);
	HeaderDialog dlgHdr(dlg, _T("Adjust Date & Time"), HeaderDialog::IMG_DATE_TIME);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	std::auto_ptr<ImgProcessingThread> worker_thread(
		dlg.RelativeChange() ?
			new DateTimeAdjThread(photos_, dlg.GetDays(), dlg.GetHours(), dlg.GetMinutes(), dlg.GetSeconds()) :
			new DateTimeAdjThread(photos_, dlg.GetDateTime())
		);

	ImgProcessingPool proc(worker_thread);

	ProcessingProgressDlg progress(parent_, proc, _T("Adjusting Image Date/Time in Progress"),
		_T("Adjusting time stamp"), ProcessingProgressDlg::AUTO_CLOSE | ProcessingProgressDlg::OUTPUT_ONLY);

	if (progress.DoModal() != IDOK)
		return false;

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SendEMailDlg.h"


bool CTaskSendEMail::Go()
{
	try
	{
		return Send();
	}
	CATCH_ALL_W(parent_)
	return false;
}


bool CTaskSendEMail::Send()
{
	double ratio= FindCommonRatio(photos_);
	if (ratio == -1.0)
		ratio = 1.0;

	CSendEMailDlg dlg(0, photos_, ratio);
	HeaderDialog dlgHdr(dlg, _T("E-Mail Images"), HeaderDialog::IMG_EMAIL, parent_);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	std::vector<ResizePhotoInfo> photos;
	photos.resize(photos_.size());

	std::vector<Path> files(photos_.size());
	std::vector<String> names(photos_.size());

	TCHAR tempDir[MAX_PATH];
	::GetTempPath(MAX_PATH, tempDir);

	size_t index= 0;
	// prepare info for resizing photos -----------------------------
	for (VectPhotoInfo::iterator it= photos_.begin(); it != photos_.end(); ++it)
	{
		PhotoInfo& photo= **it;

		ResizePhotoInfo& inf= photos[index];
		inf.orientation_	= photo.OrientationField();
		inf.photo_size_.cx	= photo.GetWidth();
		inf.photo_size_.cy	= photo.GetHeight();
		inf.src_file_		= photo.GetOriginalPath();
		inf.decoder_		= photo.GetDecoder();
		inf.date_time_		= photo.DateTimeStr();
		inf.dest_file_		= tempDir;
		inf.dest_file_		+= inf.src_file_.GetFileName() + _T(".jpg");
		inf.dest_file_.AppendToFileName(_T("_cpy"));
		inf.fit_this_size_	= false;

		files[index] = inf.dest_file_;
		names[index] = inf.src_file_.GetFileNameAndExt();

		++index;
	}

	// resize photos ------------------------------------------------
	bool progressive_jpeg= false;
	bool baseline_jpeg= false;
	bool tempFiles= true;
	CSize thumb_size(0,0);
	CSize image_size(dlg.width_, dlg.height_);
	//if (dlg.size_ == 0) // send originals?
	//{
	//	image_size.cx = image_size.cy = 0;
	//	tempFiles = false;
	//}
	ResizeFormat fmt(image_size, dlg.GetJPEGQuality(), progressive_jpeg, baseline_jpeg,
		0, thumb_size, Dib::RESIZE_CUBIC, false, false);

	ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ResizingThread(photos, fmt)));

	ProcessingProgressDlg progress(parent_, proc, _T("Preparing Images to Send by E-Mail"), 0,
		ProcessingProgressDlg::INPUT_OUTPUT | ProcessingProgressDlg::AUTO_CLOSE);

	if (progress.DoModal() != IDOK)
		return false;

	// calc size of reduced images

	if (0)
	{
		ULONGLONG fileSize= 0;
		for (std::vector<ResizePhotoInfo>::const_iterator it= photos.begin(); it != photos.end(); ++it)
		{
			CFileStatus stat;
			if (!CFile::GetStatus(it->dest_file_.c_str(), stat))
				throw String(_T("Cannot find resized image file ")) + it->dest_file_;

			fileSize += stat.m_size;
		}

		extern String FormatFileSize(uint64 fileSize);

		oStringstream msg;
		msg << _T("\r\n\r\nSize of attached files: ");
		msg << FormatFileSize(fileSize);
		//if (fileSize > 1024*1024)
		//	msg << static_cast<uint32>(fileSize / (1024*1024)) << _T(" MB");
		//else if (fileSize > 1024)
		//	msg << static_cast<uint32>(fileSize / (1024)) << _T(" KB");
		//else //if (fileSize > 1024*1024)
		//	msg << static_cast<uint32>(fileSize) << _T(" bytes");
	}

	// send e-mail ------------------------------------------

	try
	{
		FileSend mail;

		mail.SendFiles(files, names);//, &msg.str());
	}
	CATCH_ALL_W(parent_)

	if (tempFiles)
	{
		ASSERT(files.size() == index);

		// is it safe to delete files right now?
		for (size_t i= 0; i < index; ++i)
			::DeleteFile(files[i].c_str());
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ExtractJpegDlg.h"
#include "ExtractingThread.h"


bool CTaskExtractJpegs::Go()
{
	try
	{
		return Extract();
	}
	CATCH_ALL_W(parent_)
	return false;
}


bool CTaskExtractJpegs::Extract()
{
	size_t valid= 0;

	for (VectPhotoInfo::iterator it= photos_.begin(); it != photos_.end(); ++it)
	{
		PhotoInfo& photo= **it;

		uint32 jpeg_data_offset_= 0;
		uint32 jpeg_data_size_= 0;

		if (photo.GetEmbeddedJpegImage(jpeg_data_offset_, jpeg_data_size_))
			valid++;
	}

	if (valid == 0)
	{
		AfxMessageBox(_T("None of the selected images contain embedded JPEG preview to extract."), MB_OK);
		return false;
	}

	//double ratio= FindCommonRatio(photos_);
	//if (ratio == -1.0)
	//	ratio = 1.0;

	//OrientationOfImages orientation= FindCommonOrientation(photos_);

	ExtractJpegDlg dlg(parent_);
	HeaderDialog dlgHdr(dlg, _T("Extract JPEGs"), HeaderDialog::IMG_EXTRACT);
	if (dlgHdr.DoModal() != IDOK)
		return false;

	std::vector<ExtractJpegInfo> photos;
	photos.reserve(photos_.size());

	int skip= 0;
	for (VectPhotoInfo::iterator it= photos_.begin(); it != photos_.end(); ++it)
	{
		PhotoInfo& photo= **it;

		ExtractJpegInfo info;
		info.jpeg_data_offset_ = 0;
		info.jpeg_data_size_ = 0;

		if (!photo.GetEmbeddedJpegImage(info.jpeg_data_offset_, info.jpeg_data_size_))
		{
			skip++;
			continue;
		}

		info.photo_ = &photo;
		info.orientation_ = photo.OrientationField();
		//inf.photo_size_.cx	= photo.GetWidth();
		//inf.photo_size_.cy	= photo.GetHeight();
		info.src_file_path_ = photo.GetOriginalPath();
		info.dest_file_path_ = dlg.same_dir_ == 0 ? info.src_file_path_.GetDir() : dlg.GetDestPath();
		info.dest_file_path_.AppendDirSeparator();
		info.dest_file_path_ += info.src_file_path_.GetFileName();
		info.dest_file_path_ += ReplaceIllegalChars(static_cast<const TCHAR*>(dlg.suffix_));
		info.dest_file_path_ += _T(".jpg");

		photos.push_back(info);
	}

	ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ExtractingThread(photos, dlg.GetParams())));

	ProcessingProgressDlg progress(parent_, proc, _T("Extracting JPEG Images from Raw Photos in Progress"), 0,
		ProcessingProgressDlg::AUTO_CLOSE | ProcessingProgressDlg::OUTPUT_ONLY);

	if (progress.DoModal() != IDOK)
		return false;

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ImageProcessorDlg.h"

bool CTaskImageProcessor::Go()
{
	try
	{
		return Process();
	}
	CATCH_ALL_W(parent_)
	return false;
}

bool CTaskImageProcessor::Process()
{
/*
	size_t valid= 0;

	for (VectPhotoInfo::iterator it= photos_.begin(); it != photos_.end(); ++it)
	{
		PhotoInfo& photo= **it;

		uint32 jpeg_data_offset_= 0;
		uint32 jpeg_data_size_= 0;

		if (photo.GetEmbeddedJpegImage(jpeg_data_offset_, jpeg_data_size_))
			valid++;
	}

	if (valid == 0)
	{
		AfxMessageBox(_T("None of the selected images contain embedded JPEG preview to extract."), MB_OK);
		return false;
	}

	//double ratio= FindCommonRatio(photos_);
	//if (ratio == -1.0)
	//	ratio = 1.0;

	//OrientationOfImages orientation= FindCommonOrientation(photos_);
*/
	ImageProcessorDlg dlg(parent_, photos_);
	HeaderDialog dlgHdr(dlg, _T("Process Images"), HeaderDialog::IMG_EXTRACT);
	if (dlgHdr.DoModal() != IDOK)
		return false;
/*
	std::vector<ExtractJpegInfo> photos;
	photos.reserve(photos_.size());

	int skip= 0;
	for (VectPhotoInfo::iterator it= photos_.begin(); it != photos_.end(); ++it)
	{
		PhotoInfo& photo= **it;

		ExtractJpegInfo info;
		info.jpeg_data_offset_ = 0;
		info.jpeg_data_size_ = 0;

		if (!photo.GetEmbeddedJpegImage(info.jpeg_data_offset_, info.jpeg_data_size_))
		{
			skip++;
			continue;
		}

		info.photo_ = &photo;
		info.orientation_ = photo.orientation_;
		//inf.photo_size_.cx	= photo.GetWidth();
		//inf.photo_size_.cy	= photo.GetHeight();
		info.src_file_path_ = photo.GetOriginalPath();
		info.dest_file_path_ = dlg.same_dir_ == 0 ? info.src_file_path_.GetDir() : dlg.GetDestPath();
		info.dest_file_path_.AppendDirSeparator();
		info.dest_file_path_ += info.src_file_path_.GetFileName();
		info.dest_file_path_ += ReplaceIllegalChars(static_cast<const TCHAR*>(dlg.suffix_));
		info.dest_file_path_ += _T(".jpg");

		photos.push_back(info);
	}

	auto_ptr<ImgProcessingThread> worker_thread(new ExtractingThread(photos, dlg.GetParams()));

	ProcessingProgressDlg progress(parent_, worker_thread, _T("Extracting JPEG Images from Raw Photos in Progress"),
		ProcessingProgressDlg::AUTO_CLOSE | ProcessingProgressDlg::OUTPUT_ONLY);

	if (progress.DoModal() != IDOK)
		return false;
*/
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ApplyMetadataThread.h"
#include "PhotoCollection.h"
#include "Config.h"


extern bool ApplyTagToPhotos(VectPhotoInfo photos, String tag, bool apply, CWnd* parent)
{
	try
	{
		if (photos.size() == 1)
		{
			// don't bother creating worker thread and flashing progress dlg for a single photo

			ApplyTagToPhoto(photos[0], tag, apply, g_Settings.save_tags_to_photo_, PhotoInfo::WriteAccessFn());
		}
		else
		{
			oStringstream operation;
			operation << (apply ? _T("Applying tag '") : _T("Removing tag '")) << tag << _T("'");

			ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ApplyMetadataThread(photos, tag, apply, 0)));

			ProcessingProgressDlg progress(parent, proc, _T("Changing Tags in Progress"),
				operation.str().c_str(), ProcessingProgressDlg::AUTO_CLOSE | ProcessingProgressDlg::OUTPUT_ONLY);

			progress.DoModal();
		}

		// notify about changes; not all 'photos' might have changed, but still...
		PhotoCollection::Instance().FireTagsChanged(photos);
	}
	CATCH_ALL_W(parent)
	return true;
}


extern bool ApplyRatingToPhotos(VectPhotoInfo photos, int rating, CWnd* parent)
{
	try
	{
		if (photos.size() == 1)
		{
			// don't bother creating worker thread and flashing progress dlg for single photo

			photos[0]->SaveRating(rating, PhotoInfo::WriteAccessFn());
		}
		else
		{
			ImgProcessingPool proc(std::auto_ptr<ImgProcessingThread>(new ApplyMetadataThread(photos, rating, 0)));

			ProcessingProgressDlg progress(parent, proc, _T("Changing Rating in Progress"),
				_T("Changing rating"), ProcessingProgressDlg::AUTO_CLOSE | ProcessingProgressDlg::OUTPUT_ONLY);

			progress.DoModal();
		}

		// notify about changes; not all 'photos' might have changed, but still
		PhotoCollection::Instance().FireRatingChanged(photos);
	}
	CATCH_ALL_W(parent)
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
#include "RenameToolDlg.h"


bool CTaskImageRename::Go()
{
	try
	{
		return Rename();
	}
	CATCH_ALL_W(parent_)
	return false;
}


bool CTaskImageRename::Rename()
{
	RenameToolDlg dlg(parent_, photos_);
	HeaderDialog dlg_with_header(dlg, _T("Rename Images"), HeaderDialog::IMG_EXTRACT);
	if (dlg_with_header.DoModal() != IDOK)
		return false;

	//

	return true;
}
