/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifExport.h: interface for the CExifExport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXIFEXPORT_H__554FF7AC_9F13_401C_A08A_646B9F0EBACE__INCLUDED_)
#define AFX_EXIFEXPORT_H__554FF7AC_9F13_401C_A08A_646B9F0EBACE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class Columns;
class PhotoInfo;


class CExifExport
{
public:
	CExifExport(const TCHAR* output_file, const std::vector<uint16>& sel_columns, const std::vector<INT>& col_order,
		Columns& cols, const TCHAR* separator, bool export_tags);
	virtual ~CExifExport();

	void ExportPhoto(PhotoInfo& inf);
	void ExportHeader();

private:
	Columns& columns_;
	const std::vector<INT>& col_order_;
	const std::vector<uint16>& sel_columns_;	// vector of selected columns (in detailed view)
	CStdioFile file_;
	const TCHAR* separator_;
	bool export_tags_;
};

#endif // !defined(AFX_EXIFEXPORT_H__554FF7AC_9F13_401C_A08A_646B9F0EBACE__INCLUDED_)
