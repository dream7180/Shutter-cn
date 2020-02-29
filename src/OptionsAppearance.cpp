/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsAppearance.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsAppearance.h"
#include "PhotoInfoJPEG.h"
#include <boost/bind.hpp>
#include "DefaultColors.h"
#include "Config.h"	// default fonts
#include "GridCtrl.h"
#include "LinkWnd.h"
#include <boost/function.hpp>
#include "ViewerExampleWnd.h"
#include "PhotoCtrl.h"
//#include "SnapFrame/CaptionWindow.h"
#include "SnapFrame/SnapView.h"
#include "SnapFrame/SnapFrame.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsAppearance property page


enum UIType { Color, Font, Brightness };

struct UIElem
{
	const TCHAR* name;
	bool* use_custom;
	UIType type;
	COLORREF* color;
	COLORREF default_color;
	LOGFONT* font;
	LOGFONT* default_font;
	double* gamma;
	typedef boost::function<void (void)> UpdateFn;
	UpdateFn update;

	UIElem(const TCHAR* name, ColorCfg& c, const UpdateFn& fn) : name(name), type(Color), color(&c.color), default_color(c.default_color), use_custom(&c.use_custom), update(fn)
	{
		gamma = 0;
		font = default_font = 0;
	}

	UIElem(const TCHAR* name, LOGFONT* font, LOGFONT* def_font, const UpdateFn& fn) : name(name), type(Font), font(font), default_font(def_font), use_custom(0), update(fn)
	{
		use_custom = 0;
		color = 0;
		gamma = 0;
		default_color = 0;
	}

	UIElem(const TCHAR* name, double* gamma, const UpdateFn& fn) : name(name), type(Brightness), gamma(gamma), update(fn)
	{
		use_custom = 0;
		color = 0;
		font = default_font = 0;
		default_color = 0;
	}

	void Update()
	{
		update();
	}

	COLORREF SelColor() const
	{
		ASSERT(color && use_custom);
		return *use_custom ? *color : default_color;
	}
};

//-------------------------------------------------------------------

class Separator : public CWnd
{
public:
	Separator()
	{
		color_ = 0;
	}

	bool Create(CWnd* parent, CRect rect)
	{
		return !!CWnd::Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW)), 0, WS_CHILD | WS_VISIBLE, rect, parent, -1);
	}

	void SetColor(COLORREF color)
	{
		color_ = color;
		Invalidate();
	}

	DECLARE_MESSAGE_MAP()
	BOOL OnEraseBkgnd(CDC* dc);

private:
	COLORREF color_;
};


BEGIN_MESSAGE_MAP(Separator, CWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


BOOL Separator::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	SnapView::DrawHorzSeparator(dc, color_, rect);
	return true;
}

//-------------------------------------------------------------------

static int g_cur_element= 0;	// remember selection for a session

struct OptionsAppearance::Impl : GridCtrlNotification, PhotoCtrlNotification
{
	Impl() : ui_items_(false), example_viewer_(photos_2_)
	{
		default_description_font_ = g_Settings.default_description_font_;
		default_tag_font_ = g_Settings.img_default_tag_font_;
		cur_element_ = g_cur_element;
	}

	void InitDlg(OptionsAppearance* wnd)
	{
		// main window color/font settings
		// those labels correspond to PhotoCtrl::Colors apart from 'font' item...
		{
			const size_t count= 4;//wnd->main_wnd_colors_.size() + 1;
			main_wnd_.reserve(count);

			static const TCHAR* names[]=
			{
				//_T("Background"),
				//_T("Text"),
				_T("选定项"),
				//_T("Selected Text"),
				//_T("Disabled Text"),
				_T("标记背景"),
				_T("标记文本"),
				_T("标记字体")//,
				//_T("Sorting Column"),
				//_T("Group Separator"),
				//_T("Tile Dim Text"),
				//0
			};

			for (size_t idx= 0, i= 0; idx < count; ++idx)
			{
				if (idx == 3)	// tag font?
					main_wnd_.push_back(UIElem(names[idx], &wnd->tag_font_, &default_tag_font_, boost::bind(&OptionsAppearance::UpdateTagFont, wnd)));
				else// if(idx < 3)
				{
					main_wnd_.push_back(UIElem(names[idx], wnd->main_wnd_colors_[(i+1)*(5-i)-3], boost::bind(&OptionsAppearance::UpdateColorsWnd, wnd)));
					++i;
				}
			}
		}

		// viewer window colors/fonts
		{
			const size_t count= wnd->viewer_wnd_colors_.size() + 2 - 1;
			viewer_wnd_.reserve(count);

			static const TCHAR* names[]=
			{
				_T("背景"),
				_T("描述文本"),
				_T("描述字体"),
				_T("预览栏亮度"),
			//	_T("Reserved, unused"),
				_T("选定的照片"),
				_T("标记背景"),
				_T("标记文本"),
				0
			};

			for (size_t idx= 0, i= 0; idx < count; ++idx)
			{
				if (idx == 2)
					viewer_wnd_.push_back(UIElem(names[idx], &wnd->description_font_, &default_description_font_, boost::bind(&OptionsAppearance::UpdateDescFont, wnd)));
				else if (idx == 3)	// brightness
					viewer_wnd_.push_back(UIElem(names[idx], &wnd->ui_gamma_correction_, boost::bind(&OptionsAppearance::Impl::UpdateViewerBrightness, this, &wnd->ui_gamma_correction_)));
				else
				{
					viewer_wnd_.push_back(UIElem(names[idx], wnd->viewer_wnd_colors_[i], boost::bind(&OptionsAppearance::UpdateColorsViewer, wnd)));
					++i;
					if (i == 2)
						++i;	// skip reserved element
				}
			}
		}

		/*/ pane caption colors
		{
			const size_t count= wnd->pane_caption_colors_.size();
			pane_captions_.reserve(count);

			// SnapFrame::Colors:
			static const TCHAR* names[]=
			{
				_T("Pane Bar"),
				_T("Active Pane Caption"),
				_T("Inactive Pane Caption"),
				_T("Active Caption Text"),
				_T("Inactive Caption Text"),
				_T("Pane Separator"),
				0
			};

			for (size_t idx= 0, i= 0; idx < count; ++idx)
			{
				//if (idx == 2)
				//	pane_captions_.push_back(UIElem(names[idx], &wnd->description_font_, &default_description_font_, boost::bind(&OptionsAppearance::UpdateDescFont, wnd)));
				//else
				{
					pane_captions_.push_back(UIElem(names[idx], wnd->pane_caption_colors_[i], boost::bind(&OptionsAppearance::UpdateCaptionColors, wnd)));
					++i;
				}
			}
		}*/

		cur_wnd_ = &main_wnd_;

		ui_items_.SetHost(this);

		ui_items_.InsertColumn(0, _T("UI 元素"), LVCFMT_LEFT, Pixels(140));
		ui_items_.InsertColumn(1, _T("使用默认值"), LVCFMT_CENTER, Pixels(80), 0);
		ui_items_.InsertColumn(2, _T("当前设置"), LVCFMT_LEFT, Pixels(170), 0);
	}

	// notifications from the grid ctrl
	virtual void GetCellText(GridCtrl& ctrl, size_t row, size_t col, CString& text);
	virtual void CellTextChanged(GridCtrl& ctrl, size_t row, size_t col, const CString& text);
	virtual void Delete(GridCtrl& ctrl, size_t row, size_t col);
	virtual bool StartEditing(GridCtrl& ctrl, size_t row, size_t col);
	virtual UINT GetItemFlags(GridCtrl& ctrl, size_t row, size_t col);
	virtual void PostPaintCell(GridCtrl& ctrl, size_t row, size_t col, CDC& dc);

	virtual void GetItemText(PhotoInfoPtr photo, int column, String& buffer);
	virtual String GetItemLabel(PhotoInfoPtr photo, CDC& dc, int label_space);
	virtual Dib* RequestThumbnail(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available);

	void UpdateViewerBrightness(double* gamma)
	{
		example_viewer_.SetUIBrightness(*gamma);
	}

	PhotoCtrl example_wnd_;
	CListCtrl border_wnd_;
	PhotoInfoStorage photos_1_;
	ViewerExampleWnd example_viewer_;
	//CaptionWindow active_caption_;
	//CaptionWindow inactive_caption_;
	Separator separator1_wnd_;
	//Separator separator2_wnd_;
	VectPhotoInfo photos_2_;
	int cur_element_;
	std::vector<UIElem> main_wnd_;
	std::vector<UIElem> viewer_wnd_;
	//std::vector<UIElem> pane_captions_;
	std::vector<UIElem>* cur_wnd_;
	CLinkWnd reset_to_defaults_;
	GridCtrl ui_items_;
	LOGFONT default_tag_font_;
	LOGFONT default_description_font_;
};


OptionsAppearance::OptionsAppearance()
 : RPropertyPage(OptionsAppearance::IDD), impl_(new Impl())
{
	ui_gamma_correction_ = 1.0;
	memset(&description_font_, 0, sizeof description_font_);
	memset(&tag_font_, 0, sizeof tag_font_);
}


OptionsAppearance::~OptionsAppearance()
{}


void OptionsAppearance::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	DDX_Radio(DX, IDC_ELEMENT, impl_->cur_element_);
	DDX_Control(DX, IDC_LIST, impl_->ui_items_);
	DDX_Control(DX, IDC_RESET, impl_->reset_to_defaults_);
}


BEGIN_MESSAGE_MAP(OptionsAppearance, RPropertyPage)
	ON_BN_CLICKED(IDC_ELEMENT, ChangeActiveElement)
	ON_BN_CLICKED(IDC_VIEWER, ChangeActiveElement)
	//ON_BN_CLICKED(IDC_CAPTIONS, ChangeActiveElement)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsAppearance message handlers

BOOL OptionsAppearance::OnInitDialog()
{
	RPropertyPage::OnInitDialog();

	if (CWnd* wnd= GetDlgItem(IDC_EXAMPLE))
	{
		WINDOWPLACEMENT wp;
		wnd->GetWindowPlacement(&wp);
		wnd->DestroyWindow();
		CRect rect= wp.rcNormalPosition;

		impl_->border_wnd_.CWnd::CreateEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, 0, WS_VISIBLE | WS_CHILD | WS_DISABLED, rect, this, IDC_EXAMPLE);
		impl_->border_wnd_.SetBkColor(::GetSysColor(COLOR_WINDOW));
		impl_->example_wnd_.Create(&impl_->border_wnd_, impl_.get(), IDC_EXAMPLE);
		impl_->border_wnd_.GetClientRect(rect);
		impl_->example_wnd_.MoveWindow(rect);
		impl_->example_wnd_.EnableWindow(false);

		// prepare some fake photos and fill in PhotoCtrl window

		impl_->photos_2_.reserve(8);

		for (int i= 0; i < 8; ++i)
			impl_->photos_1_.Append(new PhotoInfoJPEG);

		impl_->photos_2_.push_back(impl_->photos_1_.GetNthItem(0));
		impl_->photos_2_.push_back(impl_->photos_1_.GetNthItem(3));
		impl_->photos_2_.push_back(impl_->photos_1_.GetNthItem(4));
		impl_->photos_2_.push_back(impl_->photos_1_.GetNthItem(6));

		VectPhotoInfo v;
		impl_->photos_1_.Copy(v);

		const String TAG= _T("Miki");

		v[0]->SetPhotoName(_T("DSC00001"));
		v[0]->bmp_ = new Dib(IDB_PHOTO_7);
		v[0]->exif_data_present_ = true;
		v[0]->tags_.push_back(TAG);

		v[1]->SetPhotoName(_T("DSC00002"));
		v[1]->bmp_ = new Dib(IDB_PHOTO_6);
		v[1]->exif_data_present_ = true;
		v[1]->tags_.push_back(TAG);

		v[2]->SetPhotoName(_T("DSC00003"));
		v[2]->bmp_ = new Dib(IDB_PHOTO_3);
		v[2]->exif_data_present_ = true;
		v[2]->tags_.push_back(TAG);

		v[3]->SetPhotoName(_T("DSC00004"));
		v[3]->bmp_ = new Dib(IDB_PHOTO_4);
		v[3]->exif_data_present_ = true;
		v[3]->SetSize(3, 4);

		v[4]->SetPhotoName(_T("DSC00005"));
		v[4]->bmp_ = new Dib(IDB_PHOTO_2);

		v[5]->SetPhotoName(_T("DSC00006"));
		v[5]->bmp_ = new Dib(IDB_PHOTO_1);
		v[5]->exif_data_present_ = true;

		v[6]->SetPhotoName(_T("DSC00007"));
		v[6]->bmp_ = new Dib(IDB_PHOTO_5);
		v[6]->exif_data_present_ = true;
		v[6]->SetSize(3, 4);

		v[7]->SetPhotoName(_T("DSC00008"));
		v[7]->bmp_ = new Dib(IDB_PHOTO_8);
		v[7]->exif_data_present_ = true;
		v[7]->SetSize(3, 4);

		impl_->example_wnd_.SetImageSize(Pixels(62));
		impl_->example_wnd_.SetHalftoneDrawing(true);

		impl_->example_wnd_.AddItems(v.begin(), v.begin() + 3, _T("Tag \"Miki\""), PhotoCtrl::LABEL, 1);

		impl_->example_wnd_.AddItems(v.begin() + 3, v.end(), _T("C:\\Fairchild Tropical Garden"), PhotoCtrl::FILM_ROLL, 2);
/*
int n= sizeof PhotoInfo;
TCHAR sz[8];
wsprintf(sz, L"$%x $%x", &PhotoInfo::compressed_bpp_, &PhotoInfo::subject_distance_);
AfxMessageBox(sz, MB_OK);
*/
		impl_->example_wnd_.SelectItem(v[5], false);
		impl_->example_wnd_.SelectItem(v[6], false);

		impl_->example_wnd_.SetColors(main_wnd_colors_.Colors());

		//-------------------------------------------------------------------------------

		impl_->example_viewer_.Create(&impl_->border_wnd_, rect);
//		example_viewer_.ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
		impl_->example_viewer_.EnableWindow(false);

		impl_->example_viewer_.SetColors(viewer_wnd_colors_.Colors());
		impl_->example_viewer_.SetUIBrightness(ui_gamma_correction_);

		int y= rect.top;
		const int h= SnapView::GetSeparatorThickness().cy;
		impl_->separator1_wnd_.Create(&impl_->border_wnd_, CRect(CPoint(rect.left, y), CSize(rect.Width(), h)));
/*
		impl_->active_caption_.Create(&impl_->border_wnd_, _T("Active Pane"));
		impl_->active_caption_.SetWindowPos(0, 0, y + h, rect.Width(), CaptionWindow::GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
		impl_->active_caption_.EnableWindow(false);
		impl_->active_caption_.Activate(true);

		y = rect.Height() / 3;
		impl_->separator2_wnd_.Create(&impl_->border_wnd_, CRect(CPoint(rect.left, y), CSize(rect.Width(), h)));

		impl_->inactive_caption_.Create(&impl_->border_wnd_, _T("Inactive Pane"));
		impl_->inactive_caption_.SetWindowPos(0, 0, y + h, rect.Width(), CaptionWindow::GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
		impl_->inactive_caption_.EnableWindow(false);
		impl_->inactive_caption_.Activate(false);

		UpdateCaptionColors();*/
	}
	else
	{ ASSERT(false); }

	impl_->InitDlg(this);

	ChangeActiveElement();

	ResizeMgr().BuildMap(this);
	ResizeMgr().SetWndResizing(IDC_EXAMPLE, DlgAutoResize::RESIZE_H);
	ResizeMgr().SetWndResizing(IDC_LIST, DlgAutoResize::RESIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void OptionsAppearance::Impl::GetItemText(PhotoInfoPtr photo, int column, String& buffer)
{
	if (photo == 0)
	{
		ASSERT(false);
		return;
	}
	buffer = photo->GetName();
}


String OptionsAppearance::Impl::GetItemLabel(PhotoInfoPtr photo, CDC& dc, int label_space)
{
	return photo ? photo->GetName() : String();
}


Dib* OptionsAppearance::Impl::RequestThumbnail(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available)
{
	return photo ? photo->bmp_.get() : 0;
}


void OptionsAppearance::ChangeActiveElement()
{
	if (impl_->example_viewer_.m_hWnd && impl_->example_wnd_.m_hWnd/* && impl_->active_caption_.m_hWnd */&& UpdateData())
	{
		impl_->example_viewer_.ShowWindow(SW_HIDE);
		//impl_->active_caption_.ShowWindow(SW_HIDE);
		//impl_->inactive_caption_.ShowWindow(SW_HIDE);
		impl_->example_wnd_.ShowWindow(SW_HIDE);
		impl_->separator1_wnd_.ShowWindow(SW_HIDE);
		//impl_->separator2_wnd_.ShowWindow(SW_HIDE);

		switch (impl_->cur_element_)
		{
		case 0:
			impl_->cur_wnd_ = &impl_->main_wnd_;
			impl_->example_wnd_.ShowWindow(SW_SHOWNA);
			break;

		case 1:
			impl_->cur_wnd_ = &impl_->viewer_wnd_;
			impl_->example_viewer_.ShowWindow(SW_SHOWNA);
			break;

		/*case 2:
			impl_->cur_wnd_ = &impl_->pane_captions_;
			impl_->active_caption_.ShowWindow(SW_SHOWNA);
			impl_->inactive_caption_.ShowWindow(SW_SHOWNA);
			impl_->separator1_wnd_.ShowWindow(SW_SHOWNA);
			impl_->separator2_wnd_.ShowWindow(SW_SHOWNA);
			break;*/
		}

		impl_->ui_items_.SetItemCount(impl_->cur_wnd_->size());

		g_cur_element = impl_->cur_element_;
	}
}


const TCHAR* StyleName(const LOGFONT& lf)
{
	if (lf.lfWeight >= FW_BOLD)
		return lf.lfItalic ? _T("粗斜体") : _T("粗体");
	else
		return lf.lfItalic ? _T("斜体") : _T("常规");
}


void OptionsAppearance::Impl::GetCellText(GridCtrl& ctrl, size_t row, size_t col, CString& text)
{
	text.Empty();

	if (row >= cur_wnd_->size())
		return;

	const UIElem& el= (*cur_wnd_)[row];

	if (col == 0)
		text = el.name;
	else if (col == 1)
	{
		if (el.use_custom)
			text = *el.use_custom ? _T("自定义") : _T("默认");
		else
			text = _T("-");
	}
	else
	{
		if (el.type == Color)
		{
		// commented out - too strong
			//COLORREF c= el.SelColor();
			//int red= GetRValue(c);
			//int green= GetGValue(c);
			//int blue= GetBValue(c);
			//text.Format(_T("            R(%d) G(%d) B(%d)"), red, green, blue);
		}
		else if (el.type == Font)
		{
			LOGFONT* font= el.font;
			CDC dc;
			dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
			int pt= -MulDiv(font->lfHeight, 72, dc.GetDeviceCaps(LOGPIXELSY));
			text.Format(_T("%s, %s, %d pt"), font->lfFaceName, StyleName(*font), pt);
		}
		else if (el.type == Brightness)
			text.Format(_T("%0.2f"), *el.gamma);
	}
}


void OptionsAppearance::Impl::CellTextChanged(GridCtrl& ctrl, size_t row, size_t col, const CString& text)
{
	if (row >= cur_wnd_->size())
		return;

	UIElem& el= (*cur_wnd_)[row];

	if (el.type == Brightness)
	{
		double b= ::_tstof(text);

		if (b >= 0.5 && b <= 2.0)
		{
			*el.gamma = b;
			el.Update();
		}
	}
}


void OptionsAppearance::Impl::Delete(GridCtrl& ctrl, size_t row, size_t col)
{}


bool OptionsAppearance::Impl::StartEditing(GridCtrl& ctrl, size_t row, size_t col)
{
	if (row >= cur_wnd_->size())
		return false;

	UIElem& el= (*cur_wnd_)[row];

	if (col == 1)	// use defaults column
	{
		if (el.use_custom == 0)
			return false;

		CMenu menu;
		if (menu.LoadMenu(IDR_USE_DEFAULTS))
			if (CMenu* popup= menu.GetSubMenu(0))
			{
				CRect rect;
				if (ctrl.GetSubItemRect(static_cast<int>(row), static_cast<int>(col), LVIR_BOUNDS, rect))
				{
					CPoint pos(rect.left, rect.bottom);
					ctrl.ClientToScreen(&pos);
					int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, &ctrl);
					bool cancel= false;
					if (cmd == ID_DEFAULTS)
						*el.use_custom = false;
					else if (cmd == ID_CUSTOM)
						*el.use_custom = true;
					else
						cancel = true;

					if (!cancel)
					{
						ctrl.Invalidate();
						el.Update();
					}
				}
			}
	}
	else if (col == 2)	// color or font (current setting) column
	{
		if (el.type == Color)
		{
			CColorDialog dlg(*el.color, CC_FULLOPEN, &ctrl);

			if (dlg.DoModal() == IDOK && (*el.color != dlg.GetColor() || !*el.use_custom))
			{
				*el.color = dlg.GetColor();
				*el.use_custom = true;
				el.Update();
				ctrl.Invalidate();
			}
		}
		else if (el.type == Font)
		{
			CFontDialog dlg;

			dlg.m_cf.lpLogFont = el.font;
			dlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
			dlg.m_cf.Flags &= ~CF_EFFECTS;

			if (::GetVersion() < DWORD(0x80000000))
				dlg.m_cf.Flags |= CF_NOSCRIPTSEL;	// WinNT doesn't need this

			if (dlg.DoModal() == IDOK)
			{
				el.Update();
				ctrl.Invalidate();
			}
		}
		else if (el.type == Brightness)
		{
			return true;	// edit box editing
		}
		else
		{
			ASSERT(false);
		}
	}

	return false;	// return false to prevent GridCtrl from entering edit box
}


UINT OptionsAppearance::Impl::GetItemFlags(GridCtrl& ctrl, size_t row, size_t col)
{
	if (row >= cur_wnd_->size())
		return 0;

	UIElem& el= (*cur_wnd_)[row];

	if (col == 1)
		return el.use_custom ? GridCtrlNotification::DRAW_BTN_DOWN : 0;
	else if (col == 2)
		return GridCtrlNotification::DRAW_BTN_RIGHT;
	else
		return 0;
}


void OptionsAppearance::Impl::PostPaintCell(GridCtrl& ctrl, size_t row, size_t col, CDC& dc)
{
	if (col != 2)
		return;

	if (row >= cur_wnd_->size())
		return;

	const UIElem& el= (*cur_wnd_)[row];

	CRect rect;
	if (el.type == Color && ctrl.GetSubItemRect(static_cast<int>(row), static_cast<int>(col), LVIR_BOUNDS, rect))
	{
		rect.DeflateRect(4, 2);
		rect.bottom--;	// skip grid lines

		if (rect.Width() >= 3 && rect.Height() >= 3)
		{
			const int W= 30;
			if (rect.Width() > W)
				rect.right = rect.left + W;

			COLORREF gray= RGB(128,128,128);
			dc.Draw3dRect(rect, gray, gray);

			rect.DeflateRect(1, 1);

			dc.FillSolidRect(rect, el.SelColor());
		}
	}
}


void OptionsAppearance::OnReset()
{
	const size_t count= impl_->cur_wnd_->size();
	for (size_t i= 0; i < count; ++i)
	{
		UIElem& el= (*impl_->cur_wnd_)[i];

		if (el.use_custom)
			*el.use_custom = false;
		else if (el.type == Brightness)
			*el.gamma = 1.0;
		else
			*el.font = *el.default_font;
	}

	impl_->ui_items_.Invalidate();

	UpdateColorsViewer();
	impl_->example_viewer_.SetUIBrightness(ui_gamma_correction_);
	UpdateDescFont();

	UpdateColorsWnd();
	UpdateTagFont();
}


void OptionsAppearance::UpdateTagFont()
{
	impl_->example_wnd_.SetTagFont(tag_font_);
}


void OptionsAppearance::UpdateColorsViewer()
{
	impl_->example_viewer_.SetColors(viewer_wnd_colors_.Colors());
}


void OptionsAppearance::UpdateColorsWnd()
{
	impl_->example_wnd_.SetColors(main_wnd_colors_.Colors());
}


void OptionsAppearance::UpdateDescFont()
{
	impl_->example_viewer_.SetDescriptionFont(description_font_);
}

/*
void OptionsAppearance::UpdateCaptionColors()
{
//	vector<COLORREF> colors= pane_caption_colors_.Colors();

	impl_->active_caption_.SetTabColors(pane_caption_colors_);
	impl_->inactive_caption_.SetTabColors(pane_caption_colors_);
	impl_->separator1_wnd_.SetColor(pane_caption_colors_[SnapFrame::C_SEPARATOR].SelectedColor());
	impl_->separator2_wnd_.SetColor(pane_caption_colors_[SnapFrame::C_SEPARATOR].SelectedColor());
}
*/

void SetWidth(CWnd& wnd, int width)
{
	CRect rect(0,0,0,0);
	wnd.GetWindowRect(rect);
	wnd.SetWindowPos(0, 0, 0, width, rect.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void OptionsAppearance::OnSize(UINT type, int cx, int cy)
{
	RPropertyPage::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED && impl_->example_wnd_.m_hWnd)
	{
		CRect rect(0,0,0,0);
		impl_->border_wnd_.GetClientRect(rect);
		impl_->example_wnd_.MoveWindow(rect);
		impl_->example_viewer_.MoveWindow(rect);

		//SetWidth(impl_->active_caption_, rect.Width());
		//SetWidth(impl_->inactive_caption_, rect.Width());
		SetWidth(impl_->separator1_wnd_, rect.Width());
		//SetWidth(impl_->separator2_wnd_, rect.Width());
	}
}
