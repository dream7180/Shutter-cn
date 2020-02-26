/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RadioListBox.cpp : implementation file
//

#include "stdafx.h"
#include "RadioListBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// RadioListBox

RadioListBox::RadioListBox()
{
	radio_buttons_ = true;
}

RadioListBox::~RadioListBox()
{
}


BEGIN_MESSAGE_MAP(RadioListBox, CCheckListBox)
	//ON_CLBN_CHKCHANGE(IDC_LIST, OnCheckChanged)
	ON_CONTROL_REFLECT_EX(CLBN_CHKCHANGE, OnCheckChanged)
END_MESSAGE_MAP()


// RadioListBox message handlers

void RadioListBox::RadioButtons(bool radio)
{
	if (radio_buttons_ != radio)
	{
		radio_buttons_ = radio;

		if (m_hWnd)
			Invalidate();
	}
}

/*void RadioListBox::DrawItem(LPDRAWITEMSTRUCT draw_item_struct)
{

}*/

// verbatim copy from MFC
BOOL RadioListBox::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* result)
{
	switch (message)
	{
	case WM_DRAWITEM:
		ASSERT(result == NULL);       // no return value expected
		PreDrawItem((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_MEASUREITEM:
		ASSERT(result == NULL);       // no return value expected
		PreMeasureItem((LPMEASUREITEMSTRUCT)lParam);
		break;
	case WM_COMPAREITEM:
		ASSERT(result != NULL);       // return value expected
		*result = PreCompareItem((LPCOMPAREITEMSTRUCT)lParam);
		break;
	case WM_DELETEITEM:
		ASSERT(result == NULL);       // no return value expected
		PreDeleteItem((LPDELETEITEMSTRUCT)lParam);
		break;
	default:
		return CListBox::OnChildNotify(message, wParam, lParam, result);
	}
	return TRUE;
}


// verbatim copy from MFC
void RadioListBox::PreDrawItem(LPDRAWITEMSTRUCT draw_item_struct)
{
	DRAWITEMSTRUCT drawItem = *draw_item_struct;

	if ((((LONG)drawItem.itemID) >= 0) &&
	   ((drawItem.itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) != 0))
	{
		int cyItem = GetItemHeight(drawItem.itemID);

		CDC* dc = CDC::FromHandle(drawItem.hDC);

		COLORREF newBkColor = GetSysColor(COLOR_WINDOW);

		BOOL disabled = !IsWindowEnabled() || !IsEnabled(drawItem.itemID);
		if ((drawItem.itemState & ODS_SELECTED) && !disabled)
			newBkColor = RGB(247, 123, 0);//GetSysColor(COLOR_HIGHLIGHT);

		COLORREF oldBkColor = dc->SetBkColor(newBkColor);

		int check = GetCheck(drawItem.itemID);
		bool themed = false;

/*		_AFX_CHECKLIST_STATE* checklist_state = _afxChecklistState;		
		// use of comctl32 6.0 or greater indicates that the control can be themed.
		if (checklist_state->ver_com_ctl32_ == 0)
		{
			DWORD minor;

			HRESULT hr = AtlGetCommCtrlVersion(&checklist_state->ver_com_ctl32_, &minor);
			if (FAILED(hr))
			{
				// Could not get comctl32's version. Default to < 6
				checklist_state->ver_com_ctl32_ = 5;
			}
		}

		if (checklist_state->ver_com_ctl32_ >= 6)
		{
			themed = PreDrawItemThemed(dc, drawItem, check, cyItem);
		} */

		if (!themed)
		{
			if (radio_buttons_)
				radioPreDrawItemNonThemed(dc, drawItem, check, cyItem);
			else
				PreDrawItemNonThemed(dc, drawItem, check, cyItem);
		}

		dc->SetBkColor(oldBkColor);
	}
	PreDrawItemHelper(&drawItem);
}


// draw radio buttons
void RadioListBox::radioPreDrawItemNonThemed(CDC* dc, DRAWITEMSTRUCT& drawItem, int check, int cyItem)
{
	CRect rect= drawItem.rcItem;
	int width= rect.Height() + 2;
	rect.right = rect.left + width;

	dc->FillSolidRect(rect, dc->GetBkColor());

	rect.DeflateRect(1, 2);
	dc->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONRADIO | DFCS_FLAT | (check != 0 ? DFCS_CHECKED : 0));

	drawItem.rcItem.left += width;
}


BOOL RadioListBox::OnCheckChanged()
{
	if (!radio_buttons_)
		return false;

	int cur_item= GetCurSel();

	if (cur_item < 0)
		return false;

	int checked= GetCheck(cur_item);

	if (checked == 0)
	{
		// one item has to be checked
		SetCheck(cur_item, 1);
	}
	else
	{
		// clear all but current items
		int count= GetCount();
		for (int i= 0; i < count; ++i)
			if (i != cur_item)
				SetCheck(i, 0);
	}

	return false;
}
