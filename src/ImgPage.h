/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DlgAutoResize.h"
class ImgPage;
class Dib;
class Path;
#include "PhotoInfoPtr.h"


class ImgPageNotifications
{
public:
	virtual void ParamChanged(ImgPage* wnd, bool reset) = 0;
};


// ImgPage dialog

class ImgPage : public CDialog
{
public:
	ImgPage(UINT dlg_id, CWnd* parent= NULL, bool need_preview= true);   // standard constructor
	virtual ~ImgPage();

	bool Create(CWnd* parent);

	void SetHost(ImgPageNotifications* host)	{ host_ = host; }

	// do transform an image
	virtual void Transform(Dib& dib, bool preview) = 0;

	virtual void TransformFile(Path& pathPhoto, Path& pathOutput);

	// if transformation is exclusive it cannot be combined with others
	virtual bool IsExclusive(PhotoInfoPtr photo) const	{ return false; }

	virtual void Initialize(const Dib& dibOriginal);

	bool NeedPreview() const					{ return need_preview_; }

	virtual void RestoreSettings(const TCHAR* key) {}
	virtual void StoreSettings(const TCHAR* key) {}

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	BOOL OnEraseBkgnd(CDC* dc);
	void OnPaint();
	LPARAM OnPrintClient(WPARAM HDC, LPARAM flags);
//	LPARAM OnCtrlColor(WPARAM HDC, LPARAM lParam);
	void OnSize(UINT type, int cx, int cy);
	HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT flags);

	DECLARE_MESSAGE_MAP()
	CBitmap background_bmp_;
	CSize bitmap_size_;
	CBrush br_background_;
	CDC background_dc_;
	DlgAutoResize dlg_resize_map_;
	ImgPageNotifications* host_;
	bool need_preview_;
	bool exclusive_;

	void ParamChanged(ImgPage* wnd, bool reset= false);
	void ParamChanged();
};
