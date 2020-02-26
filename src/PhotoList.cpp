/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski

____________________________________________________________________________*/

// PhotoList.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PhotoList.h"
#include "WhistlerLook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPhotoList

CPhotoList::CPhotoList(const VectPhotoInfo& photos) : photos_(photos)
{
	img_size_ = CSize(26, 19);
	text_size_ = CSize(50, img_size_.cy + 2);
	horz_max_ = 0;
	vert_max_ = 0;
	CreatePopupMenu();
	current_photo_index_ = 0;

	CDC info_dc;
	info_dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);

	int count= photos.size();

	items_text_width_.resize(count, 0);

	for (int i= 0; i < count; ++i)
		AddPhoto(i, i, info_dc);
}


CPhotoList::~CPhotoList()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPhotoList message handlers


void CPhotoList::AddPhoto(int index, int data, CDC& dc)
{
	index++;	// start numbering items (cmd Ids) from 1
	AppendMenu(MFT_STRING, index, _T(""));

	MENUITEMINFO mii;
	memset(&mii, 0, sizeof mii);
	mii.cbSize = sizeof mii;
	mii.fMask = MIIM_TYPE | MIIM_DATA;
	::GetMenuItemInfo(*this, index, false, &mii);
	mii.fType |= MFT_OWNERDRAW;
	mii.dwItemData = data;
	if (index > 1 && (index - 1) % vert_max_ == 0)
		mii.fType |= MFT_MENUBARBREAK;
	::SetMenuItemInfo(*this, index, false, &mii);

	dc.SelectStockObject(DEFAULT_GUI_FONT);
	PhotoInfo& inf= *photos_[mii.dwItemData];
	CSize ext_size= dc.GetTextExtent(inf.GetName().c_str(), inf.GetName().size());

	items_text_width_[data] = ext_size.cx;

	if (ext_size.cx > text_size_.cx)
		text_size_.cx = ext_size.cx;
	if (ext_size.cy > text_size_.cy)
		text_size_.cy = ext_size.cy;

	vert_max_ = ::GetSystemMetrics(SM_CYSCREEN) / text_size_.cy - 1;
}


void CPhotoList::MeasureItem(LPMEASUREITEMSTRUCT measure_item_struct)
{
	ASSERT(measure_item_struct->itemData < items_text_width_.size());
	measure_item_struct->itemWidth = LEFT_MARGIN + items_text_width_[measure_item_struct->itemData] + img_size_.cx;
	measure_item_struct->itemHeight = text_size_.cy;
}


void CPhotoList::DrawItem(LPDRAWITEMSTRUCT draw_item_struct)
{
	if (draw_item_struct->itemData < 0 || draw_item_struct->itemData >= photos_.size())
	{
		ASSERT(false);
		return;
	}

	CDC* pdc= CDC::FromHandle(draw_item_struct->hDC);
	if (pdc == 0)
		return;

	CDC& dc= *pdc;
	int save= dc.SaveDC();

	bool selected= !!(draw_item_struct->itemState & ODS_SELECTED);
	COLORREF rgb_back= ::GetSysColor(selected ? COLOR_HIGHLIGHT : COLOR_MENU);
	COLORREF rgb_text= ::GetSysColor(selected ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT);

	CRect rect(draw_item_struct->rcItem);

	dc.SetBkMode(OPAQUE);
	dc.SelectStockObject(DEFAULT_GUI_FONT);

	// current photo has different backgnd color
	if (draw_item_struct->itemData == current_photo_index_ && !selected)
	{
		COLORREF rgb_frame= ::GetSysColor(COLOR_3DSHADOW);
		dc.Draw3dRect(rect, rgb_frame, rgb_frame);
		dc.FillSolidRect(rect.left + 1, rect.top + 1, rect.Width() - 2, rect.Height() - 2,
			::GetSysColor(WhistlerLook::IsAvailable() ? COLOR_3DFACE : COLOR_3DSHADOW));
	}
	else
		dc.FillSolidRect(rect, rgb_back);

	PhotoInfo& inf= *photos_[draw_item_struct->itemData];

	rect.left += LEFT_MARGIN;
	CPoint pos= rect.TopLeft();
	pos.y++;
	inf.Draw(&dc, CRect(pos, img_size_), 0, 0, 0, 0, 0, PhotoInfo::DRAW_FAST);

	dc.SetTextColor(rgb_text);

	rect.left += img_size_.cx + TEXT_IMG_SPACE;
	PCSTR text= reinterpret_cast<PCSTR>(draw_item_struct->itemData);
	dc.DrawText(inf.GetName().c_str(), inf.GetName().size(), rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	dc.RestoreDC(save);
}


int CPhotoList::TrackPopupMenu(CWnd* parent, CPoint left_top, int current)
{
	current_photo_index_ = current;
	// return item no (0..count - 1) or -1
	return CMenu::TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		left_top.x, left_top.y, parent) - 1;
}
