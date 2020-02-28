/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifExport.cpp: implementation of the CExifExport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ExifExport.h"
#include "Columns.h"
#include "PhotoInfo.h"
#include "StringConversions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExifExport::CExifExport(const TCHAR* output_file, const std::vector<uint16>& sel_columns,
		const std::vector<INT>& col_order, Columns& cols, const TCHAR* separator, bool export_tags)
 : columns_(cols), sel_columns_(sel_columns), col_order_(col_order), separator_(separator),
   file_(output_file, CFile::modeCreate | CFile::modeWrite | CFile::typeText), export_tags_(export_tags)
{
	if (separator_ == 0 || *separator_ == 0)
		separator_ = _T("\t");
}

CExifExport::~CExifExport()
{}


void CExifExport::ExportPhoto(PhotoInfo& inf)
{
	String line;

	for (int i= 0; i < col_order_.size(); ++i)
	{
		ASSERT(col_order_[i] < sel_columns_.size());
		String text;
		columns_.GetInfo(text, sel_columns_[col_order_[i]], inf);
		CString replace= text.c_str();
		replace.Replace(_T("\n"), _T(" "));
		replace.Replace(_T("\r"), _T(" "));
		line += replace;
		if (i != col_order_.size() - 1)
			line += separator_;
	}

	if (export_tags_)
	{
		const size_t count= inf.GetTags().size();
		for (size_t i= 0; i < count; ++i)
		{
			CString str= inf.GetTags()[i].c_str();
			str.Replace(_T("\n"), _T(" "));
			str.Replace(_T("\r"), _T(" "));
			if (!line.empty())
				line += separator_;
			line += str;
		}
	}

	line += _T("\n");
#ifdef _UNICODE
	// convert here; CStdioFile doesn't handle char conversion gracefully
	std::string chars;
	::WideStringToMultiByte(line, chars);
#else
	const string& chars= line;
#endif

	if (fputs(chars.c_str(), file_.m_pStream) == EOF)
		AfxThrowFileException(CFileException::diskFull, _doserrno, file_.GetFilePath());

//	file_.WriteString(line.c_str());
}


void CExifExport::ExportHeader()
{
	String line;

	for (int i= 0; i < col_order_.size(); ++i)
	{
		line += columns_.ShortName(sel_columns_[col_order_[i]]);
		if (i != col_order_.size() - 1)
			line += separator_;
	}

	if (export_tags_)
	{
		if (!line.empty())
			line += separator_;
		line += _T("标记");
	}

	line += _T("\n\n");
	file_.WriteString(line.c_str());
}
