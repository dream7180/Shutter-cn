/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

struct HeaderDlg
{
	virtual void SetBigTitle(const TCHAR* title) = 0;

	virtual void SetMinimalDlgSize(CSize minimal) = 0;

	virtual void SetExtraImage(Dib* image) = 0;
	virtual void SetExtraImage(PhotoInfoPtr photo) = 0;

	virtual void SetFooterDlg(CDialog* dlg) = 0;
	virtual void ShowFooterDlg(bool show) = 0;

	virtual void SetRightSide(int width, COLORREF color, const std::vector<float>& shades) = 0;
	virtual void SetRightSide(int width) = 0;

	virtual void Resize() = 0;
};
