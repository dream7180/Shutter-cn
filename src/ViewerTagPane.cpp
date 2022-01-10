/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ViewerTagPane.cpp: implementation of the ViewerTagPane class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ViewerTagPane.h"
#include "CatchAll.h"
#include "Dib.h"
//#include "viewer/FancyToolBar.h"
#include "SetWindowSize.h"
#include "viewer/PreviewBandWnd.h"
#include "Config.h"
#include "BmpFunc.h"
#include "Color.h"
#include "TagsCommonCode.h"
#include "signals.h"
#include "TagsBarCommon.h"
#include "PhotoTagsCollection.h"
#include "PhotoCtrl.h"
#include "Tasks.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern bool LoadPingFromRsrc(LPCTSTR resource_id, Dib& bmp);


struct ViewerTagPane::Impl
{
	Impl(PhotoTagsCollection& tag_collection) : tag_ctrl_(tag_collection, L"ViewerTagBar")
	{
		ui_gamma_correction_ = 1.0;
	}

	//Dib header_;
	//Dib separator_;
	//FancyToolBar toolbar_;
	ToolBarWnd toolbar_;
	PreviewBandWnd ctrl_;
	//FancyToolBar close_btn_;
	ToolBarWnd close_btn_;
	TagsBarCommon tag_ctrl_;

	double ui_gamma_correction_;

	//void PaintSeparator(CDC& dc, const CRect& rect);
	//void ItemClicked(size_t item, AnyPointer key, PreviewBandWnd::ClickAction action);
	//void DrawItem(CDC& dc, CRect rect, size_t item, AnyPointer key);
	//String ItemToolTipText(size_t item, AnyPointer key);
	//void AddPhotoItem(PhotoInfoPtr p);
	//void PhotoModified(PhotoInfoPtr photo);

	//CSize CalcItemSize(PhotoInfoPtr p) const;
	//void TagsPopup(CWnd* parent);
	//void LoadBitmaps(double gamma);
};


/*
void ViewerTagPane::Impl::PaintSeparator(CDC& dc, const CRect& rect)
{
	separator_.Draw(&dc, rect);
}
*/
//////////////////////////////////////////////////////////////////////

ViewerTagPane::ViewerTagPane(PhotoTagsCollection& tag_collection) 
 : DockedPane(DockedPane::PANEL_ALIGN_RIGHT, 150), impl_(*new Impl(tag_collection))
{}

ViewerTagPane::~ViewerTagPane()
{}


BEGIN_MESSAGE_MAP(ViewerTagPane, DockedPane)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

//const static COLORREF TITLE_TEXT_COLOR= RGB(200,200,200);
//const static COLORREF NORMAL_TEXT_COLOR= RGB(0,0,0);
//const static COLORREF DISABLED_TEXT_COLOR= RGB(160,160,160);
/*
void ViewerTagPane::Impl::LoadBitmaps(double gamma)
{
	//VERIFY(::LoadPingFromRsrc(MAKEINTRESOURCE(IDB_LIGHT_TABLE_HEADER), header_));
	separator_.Load(IDB_LIGHT_TABLE_SEP);

	if (gamma != 1.0)
	{
		//::ApplyGammaInPlace(&header_, gamma, -1, -1);
		::ApplyGammaInPlace(&separator_, gamma, -1, -1);
	}
}
*/

bool ViewerTagPane::Create(CWnd* parent, UINT id, int width)
{
	if (!DockedPane::Create(parent, id, width))
		return false;

	//impl_.LoadBitmaps(impl_.ui_gamma_correction_);

	// --------- toolbar -----------
	//impl_.toolbar_.SetPadding(CRect(4,3,4,3));
	impl_.toolbar_.SetPadding(4,3);
	{
		const int cmds[]= { ID_TAGS_MANAGE, ID_TAGS_OPTIONS };
		//FancyToolBar::Params p;
		//p.string_rsrc_id = 0;
		//p.text_color = TITLE_TEXT_COLOR;
		//p.hot_text_color = RGB(255,255,255);
		//p.dis_text_color = RGB(111,111,130);
		//if (!impl_.toolbar_.Create(this, "pv", cmds, IDB_TAGS_TOOLBAR, &p))
		if (!impl_.toolbar_.Create("pv", cmds, IDB_TAGS_TOOLBAR, 0, this, TagsBarCommon::ID_TOOLBAR))
			return false;
	}
	
	impl_.toolbar_.CreateDisabledImageList(IDB_TAGS_TOOLBAR, -0.6f, +0.5f);
	impl_.toolbar_.AutoResize();
	impl_.toolbar_.SetOwnerDraw(true);

	//impl_.toolbar_.SetDlgCtrlID(TagsBarCommon::ID_TOOLBAR);
	impl_.toolbar_.SetOnIdleUpdateState(false);

	// --------- preview of images (background only) -----------

	if (!impl_.ctrl_.Create(this, false))
		return false;

	impl_.ctrl_.SetOrientation(false);
	impl_.ctrl_.EnableSelectionDisp(false);

	// --------- close btn -----------
	{
		const int close_cmd[]= { 1, 1, ID_HIDE_TAG_PANE };
		//FancyToolBar::Params p;
		//p.shade = -0.28f;
		//if (!impl_.close_btn_.Create(this, "..p", close_cmd, IDR_CLOSEBAR_PNG, &p))
		if (!impl_.close_btn_.Create("..p", close_cmd, IDB_PANE_TOOLBAR, 0, this, id))
			return false;
	}
	impl_.close_btn_.SetOwner(parent);
	impl_.close_btn_.AutoResize();
	impl_.close_btn_.SetOnIdleUpdateState(false);

	//impl_.close_btn_.SetOption(FancyToolBar::HOT_OVERLAY, false);

	// --------- separator painting -----------

	//SetEraseCallback(boost::bind(&ViewerTagPane::Impl::PaintSeparator, &impl_, _1, _2));

	// --------- tags control -----------

	impl_.tag_ctrl_.UseParentBackground(false);

	impl_.ctrl_.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	if (!impl_.tag_ctrl_.Create(&impl_.ctrl_))
		return false;

	impl_.tag_ctrl_.SetTagCallback(&::ApplyTagToPhotos);
	impl_.toolbar_.SetOwner(&impl_.tag_ctrl_);

	//std::vector<COLORREF> colors(PhotoCtrl::C_MAX_COLORS);
	//colors[PhotoCtrl::C_TEXT] = colors[PhotoCtrl::C_SEL_TEXT] = NORMAL_TEXT_COLOR;
	//colors[PhotoCtrl::C_DISABLED_TEXT] = DISABLED_TEXT_COLOR;
	std::vector<std::pair<::AppColors, ColorCfg>> colors;
	colors.reserve(20);

	COLORREF text = g_Colorsets.color_text;//NORMAL_TEXT_COLOR;
	COLORREF bkgnd = g_Colorsets.color_gui;
	colors.push_back(std::make_pair(AppColors::Background, bkgnd));
	colors.push_back(std::make_pair(AppColors::Text, text));
	colors.push_back(std::make_pair(AppColors::DimText, CalcNewColor(bkgnd, text, 0.60f)));
	colors.push_back(std::make_pair(AppColors::DisabledText, g_Colorsets.color_text_disabled));//DISABLED_TEXT_COLOR));
	colors.push_back(std::make_pair(AppColors::SelectedText, text));
	//colors.push_back(std::make_pair(AppColors::Separator, CalcShade(bkgnd, -36.0f)));
	colors.push_back(std::make_pair(AppColors::EditBox, CalcShade(bkgnd, -13.0f)));
	colors.push_back(std::make_pair(AppColors::Selection, g_Settings.AppColors()[AppColors::Selection]/*RGB(0x33, 0x99, 0xff)*/)); //::GetSysColor(COLOR_HIGHLIGHT)

	impl_.tag_ctrl_.SetColors(ApplicationColors(colors));

	// ---------

	Resize();

	return true;
}


LRESULT ViewerTagPane::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		OnEraseBkgnd(dc);

	return 1;
}


void ViewerTagPane::Resize()
{
	if (impl_.ctrl_.m_hWnd == 0 || impl_.toolbar_.m_hWnd == 0 || impl_.close_btn_.m_hWnd == 0 || impl_.tag_ctrl_.m_hWnd == 0)
		return;

	CRect rect(0,0,0,0);
	GetClientRect(rect);
	//CRect r(0,0,0,0);
	//impl_.toolbar_.GetWindowRect(r);

	int height_= impl_.toolbar_.tb_size.cy;//r.Height();
	header_h = height_ * 2 + 2;//VIEWER_TAGPANE_H;//impl_.header_.GetHeight();

	//CSize s= (rect_tb.Width(), rect_tb.Height());//impl_.toolbar_.Size();
	//SetWindowSize(impl_.toolbar_, rect.left, rect.top + height - s.cy, s.cx, s.cy);
	impl_.toolbar_.SetWindowPos(nullptr, 0, height_, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SetWindowSize(impl_.ctrl_, 0, header_h, rect.Width(), rect.Height() - header_h);

	SetWindowSize(impl_.tag_ctrl_, 0, 0, rect.Width(), rect.Height() - header_h);

	//CSize c= impl_.close_btn_.Size();
	//impl_.close_btn_.GetWindowRect(r);
	SetWindowSize(impl_.close_btn_, rect.right - impl_.close_btn_.tb_size.cx-2, 2, impl_.close_btn_.tb_size.cx, impl_.close_btn_.tb_size.cy);
}


void ViewerTagPane::OnSize(UINT type, int cx, int cy)
{
	DockedPane::OnSize(type, cx, cy);

	Resize();
}


BOOL ViewerTagPane::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	try
	{
		//int bottom= rect.bottom;
		//rect.bottom = rect.top + VIEWER_TAGPANE_H;//impl_.header_.GetHeight();
		dc->FillSolidRect(rect, g_Colorsets.color_gui);
		dc->FillSolidRect(rect.left, rect.top, rect.Width(), 1, g_Colorsets.color_sepline);
		dc->FillSolidRect(rect.left, header_h - 1, rect.Width()-7, 1, g_Colorsets.color_sepline_light);
		//dc->FillSolidRect(0,0,1,rect.Height(), RGB(180,180,180));
		//impl_.header_.Draw(dc, rect);

		CFont* old= dc->SelectObject(&GetDefaultGuiBoldFont());//&_font);
		//dc->SetTextColor(TITLE_TEXT_COLOR);
		dc->SetBkMode(TRANSPARENT);
		CString str= L"应用标签";
//		str.LoadString(IDS_LIGHT_TABLE);
		dc->TextOut(rect.left + 5, rect.top + 6, str);
		dc->SelectObject(old);

		//COLORREF light= CalcNewColor(RGB(78,81,96), impl_.ui_gamma_correction_);
		//COLORREF dark= CalcNewColor(RGB(57,59,74), impl_.ui_gamma_correction_);

		//TODO: alpha transp. needed here
		//dc->FillSolidRect(rect.left + 9, rect.top + 21, rect.Width() - 18, 1, light);
		//dc->FillSolidRect(rect.left + 9, rect.top + 22, rect.Width() - 18, 1, dark);
		//DrawHorzSeparatorBar(Dib& dest, int x, int y, int width, float black_opacity, float white_opacity)

		//rect.top = rect.bottom;
		//rect.bottom = bottom;
	}
	catch(...)
	{}

	return true;
}


void ViewerTagPane::SetUIBrightness(double gamma)
{
	impl_.ctrl_.SetUIBrightness(gamma);
	impl_.ui_gamma_correction_ = gamma;
	//impl_.LoadBitmaps(gamma);
	Invalidate();
}


void ViewerTagPane::PhotoSelected(PhotoInfoPtr photo, bool selection, bool still_loading)
{
	impl_.tag_ctrl_.PhotoSelected(photo, selection, still_loading);
}


void ViewerTagPane::AssignTag(int index)
{
	impl_.tag_ctrl_.AssignTag(index);
}
