/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// HeaderDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "HeaderDialog.h"
#include "DialogChild.h"
#include "Dib.h"
#include "PhotoInfo.h"
#include "WhistlerLook.h"
#include "DlgTemplateEx.h"
#include "LoadImageList.h"
#include "PhotoInfo.h"
#include "Color.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HeaderDialog dialog

namespace {
	static bool g_initialized = false;
	static int g_HEADER= 30 + 3;
	//static int g_IMG_OFFSET= 12;
	static COLORREF g_rgb_background_color= RGB(255,255,255);
	static COLORREF g_rgb_text_color= RGB(0,0,0);
}


HeaderDialog::HeaderDialog(DialogChild& dlg, const TCHAR* title, CWnd* parent)
 : CDialog(IDD_HEADER_DLG, parent), dlg_(&dlg), wnd_pos_(_T("Tools\\dlg"), dlg.GetId()),
   title_(title), image_(IMG_COPY), extra_image_(0), photo_image_(0)
{
	Init();
	has_header_ = false;
}


HeaderDialog::HeaderDialog(DialogChild& dlg, const TCHAR* title, Image image, CWnd* parent/*= 0*/)
 : CDialog(IDD_HEADER_DLG, parent), dlg_(&dlg), wnd_pos_(_T("Tools\\dlg"), dlg.GetId()),
   title_(title), image_(image), extra_image_(0), photo_image_(0)
{
	Init();
}


void HeaderDialog::Init()
{
	footer_dlg_ = 0;
	footer_visible_ = false;
	child_location_rect_.SetRectEmpty();
	minimal_size_ = CSize(0, 0);
	child_dlg_ready_ = false;
	resizable_dlg_ = dlg_->IsResizable();
//		IsResizable();
	set_window_title_ = true;
	has_header_ = false;
	if (resizable_dlg_)
	{
		m_lpszTemplateName = MAKEINTRESOURCE(IDD_HEADER_DLG_RESIZABLE);
		m_nIDHelp = IDD_HEADER_DLG_RESIZABLE;
	}
	right_side_ = 0;
	right_side_color_ = 0;
	if (dlg_)
		dlg_->SetResizeCallback(this);
}


void HeaderDialog::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(HeaderDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(HeaderDialog, CDialog)
	//{{AFX_MSG_MAP(HeaderDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HeaderDialog message handlers

int HeaderDialog::HeaderHeight() const
{
	return has_header_ ? Pixels(g_HEADER) : 0;
}


BOOL HeaderDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

//	if (!LoadImageList(image_list_, IDB_BROWSER_TOOLS, 50, RGB(255,255,255)))
	//if (!LoadImageList(image_list_, IDB_HEADER_IMAGES, 80, RGB(255,255,255)))
	//{
		//ASSERT(false);
		//EndDialog(IDCANCEL);
	//	return true;
	//}

	//{ // create img list
		//HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDB_HEADER_IMG), RT_BITMAP);
		//HIMAGELIST img_list= ImageList_LoadImage(inst, MAKEINTRESOURCE(IDB_HEADER_IMG),
		//		72, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
		//ASSERT(img_list != 0);
		//if (img_list)
		//	image_list_.Attach(img_list);
	//	image_list_.SetBkColor(g_rgb_background_color);
	//}

	// create font
	font_.CreateFont(-Pixels(14), 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, _T("Tahoma"));

	struct xCDialog : public CDialog
	{
		LPCTSTR Id() const	{ return m_lpszTemplateName; }
	};

	CDialog* dlg= dlg_;
	LPCTSTR template_name= static_cast<xCDialog*>(dlg)->Id();
	ASSERT(template_name != NULL);

	HINSTANCE inst= AfxFindResourceHandle(template_name, RT_DIALOG);
	HRSRC resource= ::FindResource(inst, template_name, RT_DIALOG);
	HGLOBAL dialog_template= LoadResource(inst, resource);

	if (dialog_template == NULL)
	{
		ASSERT(false);
		EndDialog(IDCANCEL);
		return true;
	}

	DWORD size= ::SizeofResource(inst, resource);
	void* dialog_template_ptr= ::LockResource(dialog_template);

	std::vector<BYTE> a_template;
	a_template.resize(size);
	DLGTEMPLATEEX* dlg_template= reinterpret_cast<DLGTEMPLATEEX*>(&a_template.front());
	memcpy(dlg_template, dialog_template_ptr, size);

	// DLGTEMPLATEEX expected (DIALOGEX resource)
	ASSERT(dlg_template->dlgVer == 1 && dlg_template->signature == 0xffff);

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(dialog_template);
	if (m_lpszTemplateName != NULL)
		FreeResource(dialog_template);

	dlg_template->style &= ~(WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | WS_THICKFRAME);
	dlg_template->style |= WS_CHILD;
	dlg_template->exStyle |= WS_EX_CONTROLPARENT;

	if (!dlg_->CreateIndirect(dlg_template, this))
	{
		ASSERT(false);
		EndDialog(IDCANCEL);
		return true;
	}

	if (set_window_title_)
		SetWindowText(dlg_->GetDialogTitle());

	CRect rect;
	dlg_->GetWindowRect(rect);

	if (minimal_size_.cx && minimal_size_.cy)
	{
		rect.right = rect.left + minimal_size_.cx;
		rect.bottom = rect.top + minimal_size_.cy;
	}

	::AdjustWindowRectEx(rect, GetStyle(), false, GetExStyle());

	SetWindowPos(0, 0, 0, rect.Width(), rect.Height() + HeaderHeight(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	dlg_->SetWindowPos(0, 0, HeaderHeight(), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	bool maximize= false;

	if (resizable_dlg_)
	{
		SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), false);
		child_location_rect_ = rect;
		child_location_rect_.bottom += HeaderHeight();

		child_dlg_ready_ = true;

		if (wnd_pos_.IsRegEntryPresent())
		{
			CRect rect= wnd_pos_.GetLocation(true);
			SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

			maximize = wnd_pos_.IsMaximized();
		}

		// resize child dlg now; if above size is no different than 'rect' above, no resizing will occur
		// and child dialog may have a wrong size
		Resize();
	}
	else
	{
		if (wnd_pos_.IsRegEntryPresent())
		{
			CPoint pos= wnd_pos_.GetPosition();
			SetWindowPos(0, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		child_dlg_ready_ = true;
	}

	if (maximize)
		ShowWindow(SW_MAXIMIZE);

	dlg_->ShowWindow(SW_SHOWNA);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


CRect HeaderDialog::GetImgRect() const
{
	//CPoint start(Pixels(g_IMG_OFFSET), 0);
	CPoint start(20, 0);
	CRect rect(start, CSize(HeaderHeight() + 2-3, HeaderHeight() - 5-3));
	rect.OffsetRect(-1, 2);
	return rect;
}


double Shade(int index, int steps)
{
	ASSERT(index >= 0 && index < steps);

	double x = 1.0 + index * 3.0 / steps;
	double v = std::pow(x, 1.6);
	return v;
}

double Shadow(int index, int steps)
{
	ASSERT(index >= 0 && index < steps);

	double x = 1.0 + (steps - index - 1) * 4.0 / steps;
	double v = std::pow(x, 2.3);
	return v;
}


BOOL HeaderDialog::OnEraseBkgnd(CDC* dc)
{
	CRect cl_rect;
	GetClientRect(cl_rect);

//	rect.top = HeaderHeight();
	// no need to erase an area below child dlg
//	dc->FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));

	CRect rect= cl_rect;
	COLORREF back_color= ::GetSysColor(COLOR_3DFACE);

	if (has_header_)
	{
		rect.top = 0;
		int pix = Pixels(3);
		rect.bottom = HeaderHeight() - pix;
		dc->FillSolidRect(rect, g_rgb_background_color);

		rect.top = rect.bottom;
		rect.bottom++;

		if (WhistlerLook::IsAvailable())
		{
			//static const float shade[4]= { -33.0f, -16.5f, -5.5f, -1.2f };
			for (int i= 0; i < pix; ++i)
			{
				float s = -1.2f * static_cast<float>(Shadow(i, pix));
				if (right_side_ > 0)
				{
					dc->FillSolidRect(rect.left, rect.top, right_side_, 1, CalcShade(right_side_color_, s));

					CRect r(CPoint(rect.left + right_side_, rect.top), CSize(1, 1));
					for (int j= 0; j < right_side_shades_.size(); ++j)
					{
						COLORREF color= CalcShade(back_color, right_side_shades_[j]);
						dc->FillSolidRect(r, CalcShade(color, s));
						r.OffsetRect(1, 0);
					}

					int right= r.right - 1;
					dc->FillSolidRect(right, rect.top, rect.Width() - right, 1, CalcShade(back_color, s));
				}
				else
					dc->FillSolidRect(rect, CalcShade(back_color, s));

				rect.OffsetRect(0, 1);
			}
		}
		else
		{
			dc->FillSolidRect(rect, ::GetSysColor(COLOR_3DSHADOW));
			rect.OffsetRect(0, 1);
			if (right_side_ > 0)
			{
				dc->FillSolidRect(rect.left, rect.top, right_side_, 3, right_side_color_);
				dc->FillSolidRect(rect.left + right_side_, rect.top, rect.Width() - right_side_, 3, back_color);
			}
			else
				dc->FillSolidRect(rect.left, rect.top, rect.Width(), 3, back_color);
		}

//		rect.top = HeaderHeight() - 1;
//		dc->FillSolidRect(rect, ::GetSysColor(COLOR_3DSHADOW));

		if (extra_image_)
			extra_image_->DrawScaled(dc, GetImgRect(), g_rgb_background_color);
		else if (photo_image_)
			photo_image_->Draw(dc, GetImgRect(), g_rgb_background_color);

		//CPoint start(Pixels(g_IMG_OFFSET), 0);
		CPoint start(20, 0);
		//if (image_list_.m_hImageList)
		//{
			//image_list_.Draw(dc, image_, start, ILD_TRANSPARENT);
			//start.x += Pixels(image_ == IMG_COPY || image_ == IMG_MOVE || image_ == IMG_HISTOGRAM ? 80 : 56);
		//}

		if (font_.m_hObject)
		{
			CFont* old= dc->SelectObject(&font_);
			dc->SetBkMode(OPAQUE);
			dc->SetBkColor(g_rgb_background_color);
			dc->SetTextColor(g_rgb_text_color);
			dc->TextOut(start.x, start.y + Pixels(11), title_);
			dc->SelectObject(old);
		}
	}
	else
		dc->FillSolidRect(cl_rect, g_rgb_background_color);

	if (footer_visible_)
	{
		int y= std::max(rect.bottom, cl_rect.bottom - GetFooterHeight());

		int sep = Pixels(SEPARATOR_H);
		for (int i = 0; i < sep; ++i)
		{
			float s = -4.0f * static_cast<float>(Shade(i, sep));
			dc->FillSolidRect(cl_rect.left, y + i, cl_rect.Width(), 1, CalcShade(back_color, s));
		}
	}

	return true;
}


void HeaderDialog::OnOK()
{
	dlg_->OnOK();
}

void HeaderDialog::OnCancel()
{
	dlg_->OnCancel();
}


void HeaderDialog::SetExtraImage(PhotoInfoPtr photo)
{
	photo_image_ = photo;
	extra_image_ = 0;

	if (m_hWnd)
		InvalidateRect(GetImgRect());
}


void HeaderDialog::SetExtraImage(Dib* image)
{
	photo_image_ = 0;
	extra_image_ = image;

	if (m_hWnd)
		InvalidateRect(GetImgRect());
}


int HeaderDialog::GetFooterHeight()
{
	if (footer_dlg_ && footer_dlg_->m_hWnd)
	{
		CRect rect;
        footer_dlg_->GetClientRect(rect);
		// height plus extra space for separator lines
		return rect.Height() + Pixels(SEPARATOR_H);
	}

	return 0;
}


void HeaderDialog::Resize()
{
	if (dlg_ == 0)
		return;

	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (rect.Height() <= HeaderHeight())
		return;

	int cx= rect.Width();
	int cy= rect.Height();

	if (footer_visible_ && footer_dlg_)
	{
		CRect opt_rect;
		footer_dlg_->GetClientRect(opt_rect);
		int y= cy - opt_rect.Height();
		cy = std::max(0, cy - GetFooterHeight());
		footer_dlg_->SetWindowPos(0, 0, y, cx, opt_rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		// hack: 3D line drawn above expanded options dlg gets darker while resizing dlg window, so redraw it
		CRect rect(0,0,0,0);
		GetClientRect(rect);
		rect.top = y - 5;
		rect.bottom = y + 1;
		InvalidateRect(rect);
	}

	cy = std::max(0, cy - HeaderHeight());
	dlg_->SetWindowPos(0, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	dlg_->Resize();
}


void HeaderDialog::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);

	if (dlg_ && child_dlg_ready_ && type != SIZE_MINIMIZED && cx > 0 && cy > HeaderHeight())
		Resize();
}


void HeaderDialog::OnGetMinMaxInfo(MINMAXINFO* MMI)
{
	CDialog::OnGetMinMaxInfo(MMI);
	MMI->ptMinTrackSize.x = child_location_rect_.Width();
	MMI->ptMinTrackSize.y = child_location_rect_.Height();
	if (footer_visible_)
		MMI->ptMinTrackSize.y += GetFooterHeight();
}


BOOL HeaderDialog::PreCreateWindow(CREATESTRUCT& cs)
{
	if (resizable_dlg_)
	{
		cs.style |= WS_THICKFRAME;
		cs.style &= ~DS_MODALFRAME;
	}

	return CDialog::PreCreateWindow(cs);
}

/*
bool HeaderDialog::IsResizable()
{
	struct xCDialog : public CDialog
	{
		LPCTSTR Id() const	{ return m_lpszTemplateName; }
	};

	CDialog* dlg= dlg_;
	LPCTSTR template_name= static_cast<xCDialog*>(dlg)->Id();
	ASSERT(template_name != NULL);

	HINSTANCE inst= AfxFindResourceHandle(template_name, RT_DIALOG);
	HRSRC resource= ::FindResource(inst, template_name, RT_DIALOG);
	HGLOBAL dialog_template= LoadResource(inst, resource);

	if (dialog_template == NULL)
	{
		ASSERT(false);
		return false;
	}

	DWORD size= ::SizeofResource(inst, resource);
	void* dialog_template= ::LockResource(dialog_template);

	vector<BYTE> a_template;
	a_template.resize(size);
	DLGTEMPLATEEX* dlg_template= reinterpret_cast<DLGTEMPLATEEX*>(&a_template.front());
	memcpy(dlg_template, dialog_template, size);

	ASSERT(dlg_template->dlgVer == 1 && dlg_template->signature == 0xffff);

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || dialog_template_ != NULL)
		UnlockResource(dialog_template);
	if (m_lpszTemplateName != NULL)
		FreeResource(dialog_template);

	return (dlg_template->style & WS_THICKFRAME) != 0;
}
*/

void HeaderDialog::OnDestroy()
{
	if (dlg_ && child_dlg_ready_)
	{
		WINDOWPLACEMENT wp;
		if (GetWindowPlacement(&wp))
		{
			if (resizable_dlg_)
			{
				if (footer_visible_)
					wp.rcNormalPosition.bottom -= GetFooterHeight();

				wnd_pos_.StoreState(wp);
			}
			else
				wnd_pos_.StorePosition(wp);
		}
	}

	CDialog::OnDestroy();
}


BOOL HeaderDialog::ContinueModal()
{
	if (!CDialog::ContinueModal())
		return false;

	if (dlg_)
		return dlg_->ContinueModal();

	return true;
}


void HeaderDialog::SetFooterDlg(CDialog* dlg)
{
	footer_dlg_ = dlg;

	if (footer_dlg_ == 0)
		footer_visible_ = false;
}


void HeaderDialog::ShowFooterDlg(bool show)
{
	if (footer_dlg_ == 0 || footer_dlg_->m_hWnd == 0)
		return;

	WINDOWPLACEMENT wp;
	if (!GetWindowPlacement(&wp))
		return;

	CRect rect= wp.rcNormalPosition;

	int height= GetFooterHeight();

	if (show)
	{
		rect.bottom += height;
	}
	else
	{
		footer_dlg_->ShowWindow(SW_HIDE);
		footer_dlg_->EnableWindow(false);

		rect.bottom -= height;
	}

	footer_visible_ = show;

	if (IsZoomed())
		ShowWindow(SW_RESTORE);

	SetWindowPos(0, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

	if (show)
	{
		footer_dlg_->EnableWindow();
		footer_dlg_->ShowWindow(SW_SHOWNA);
	}
}


void HeaderDialog::SetRightSide(int width, COLORREF color, const std::vector<float>& shades)
{
	right_side_color_ = color;
	right_side_ = width;
	right_side_shades_ = shades;
}


void HeaderDialog::SetRightSide(int width)
{
	right_side_ = width;
	if (m_hWnd)
		Invalidate();
}


void HeaderDialog::SetBigTitle(const TCHAR* title)
{
	if (title_ != title)
	{
		title_ = title;
		if (m_hWnd)
			Invalidate();
	}
}


void HeaderDialog::SetMinimalDlgSize(CSize minimal)
{
	// today this function is only useful during dlg creation; it may be invoked from inside child's InitDialog

	minimal_size_ = minimal;
}
