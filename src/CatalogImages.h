/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class Database;


class CatalogImages
{
public:
	CatalogImages(HWND parent_wnd, bool read_only_photos_withEXIF, Database& dbImages);
	~CatalogImages();

	void ProcessFiles(const String& dir, bool subdirs, const std::vector<bool>& scan_types,
		const String& title, const String& description, int img_size, bool folder_scan, int jpeg_compr);

	enum { MESSAGE= WM_USER + 99 };

	enum Progress { DIR_SCANNING, DIR_SCAN_DONE, IMG_PROCESSING, IMG_PROC_DONE };

	size_t Count() const;

	String GetFile(size_t index) const;

	void Break();

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;

	CatalogImages(const CatalogImages&);
	CatalogImages& operator = (const CatalogImages&);
};
