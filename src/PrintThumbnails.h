/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintThumbnails.h: interface for the PrintThumbnails class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRINTTHUMBNAILS_H__1134C36C_E56D_4932_B825_65FEF216C89A__INCLUDED_)
#define AFX_PRINTTHUMBNAILS_H__1134C36C_E56D_4932_B825_65FEF216C89A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PrintEngine.h"


class PrintThumbnails : public PrintEngine
{
public:
	PrintThumbnails();
	virtual ~PrintThumbnails();

	virtual int GetPageCount(int items_count) const;

	virtual CSize GetImageSize() const;

	virtual void GetDefaultFont(LOGFONT& lf) const;

	virtual void SetDefaultFont(const LOGFONT& lf);

private:
	bool CalcDrawingParams(int& rows, int& item_size, int& offset, int& line_h) const;

	virtual bool PrintFn(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page);

	LOGFONT font_;
};

#endif // !defined(AFX_PRINTTHUMBNAILS_H__1134C36C_E56D_4932_B825_65FEF216C89A__INCLUDED_)
