/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PropertyPane.h"
#include "DlgListCtrl.h"
#include "ACListWnd.h"
#include "PropertyField.h"

namespace Property
{


Pane::Pane(AutoCompletePopup& auto_complete, boost::ptr_vector<Field>& fields)
	: auto_complete_(auto_complete), fields_(fields)
{
	modified_ = false;
}

Pane::Pane(AutoCompletePopup& auto_complete, boost::ptr_vector<Field>& fields, int dlg_id)
	: CDialog(dlg_id), auto_complete_(auto_complete), fields_(fields)
{
	modified_ = false;
}


BEGIN_MESSAGE_MAP(Pane, CDialog)
	ON_WM_SIZE()
	ON_CONTROL_RANGE(EN_CHANGE, EDIT_ID, EDIT_ID + 999, OnEditChange)
	ON_CONTROL_RANGE(EN_SETFOCUS, EDIT_ID, EDIT_ID + 999, OnFocusSet)
	ON_CONTROL_RANGE(EN_KILLFOCUS, EDIT_ID, EDIT_ID + 999, OnFocusKill)
	ON_COMMAND_RANGE(POPUP_ID, POPUP_ID + 999, OnAutoCompleteDropDown)
	ON_NOTIFY_RANGE(NM_SETFOCUS, EDIT_ID, EDIT_ID + 999, OnFocusSetNotification)
	ON_NOTIFY_RANGE(DTN_DATETIMECHANGE, EDIT_ID, EDIT_ID + 999, OnChangeNotification)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


void Pane::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);
	resize_.Resize();
}


BOOL Pane::OnInitDialog()
{

//	if (g_AutoCompleteList.m_hWnd == 0)
//		g_AutoCompleteList.Create();

	CDialog::OnInitDialog();
	return true;
}


HBRUSH Pane::OnCtlColor(CDC* dc, CWnd* ctrl, UINT code)
{
	HBRUSH hbr= CDialog::OnCtlColor(dc, ctrl, code);
/*
	if (ctrl != 0 && GetFocus() == ctrl)
	{
		COLORREF color= ::CalcNewColor(::GetSysColor(COLOR_HIGHLIGHT), ::GetSysColor(COLOR_WINDOW), 0.88f);
		if (current_.m_hObject == 0)
			current_.CreateSolidBrush(color);

		hbr = current_;
		dc->SetBkColor(color);
	}
*/
	return hbr;
}


Field& Pane::GetField(size_t index)
{ return fields_[index]; }


size_t Pane::FieldCount() const
{ return fields_.size(); }


void Pane::OnEditChange(UINT id)
{
	modified_ = true;
}


void Pane::OnFocusKill(UINT id)
{
//TRACE(L"kill focus\n");
	if (DlgListCtrl* parent= dynamic_cast<DlgListCtrl*>(GetParent()))
		if (CWnd* ctrl= GetDlgItem(id))
		{
			ctrl->Invalidate();

			//TODO: unbind current auto complete window

			if (CEdit* edit= dynamic_cast<CEdit*>(ctrl))
			{
				size_t index= edit->GetDlgCtrlID() - EDIT_ID;
				if (index < FieldCount())
				{
					Field& fld= GetField(index);
					auto_complete_.ControlEditBox(0, 0);
					fld.ShowAutoCompleteBtn(false);
				}
			}
//			else
//				auto_complete_.ControlEditBox(0, 0);
		}
}


void Pane::OnFocusSet(UINT id)
{
//TRACE(L"set focus\n");
	if (DlgListCtrl* parent= dynamic_cast<DlgListCtrl*>(GetParent()))
		if (CWnd* ctrl= GetDlgItem(id))
		{
			WINDOWPLACEMENT wp;
			if (ctrl->GetWindowPlacement(&wp))
				parent->EnsureVisible(this, wp.rcNormalPosition);

			ctrl->Invalidate();

			if (CEdit* edit= dynamic_cast<CEdit*>(ctrl))
			{
				size_t index= edit->GetDlgCtrlID() - EDIT_ID;
				if (index < FieldCount())
				{
					Field& fld= GetField(index);
					auto_complete_.ControlEditBox(edit, fld.GetHistory());
					fld.ShowAutoCompleteBtn(true);
				}
			}
			else
				auto_complete_.ControlEditBox(0, 0);
		}
}


void Pane::OnChangeNotification(UINT code, NMHDR* hdr, LRESULT* res)
{
	modified_ = true;
	if (res)
		*res = 0;
}


void Pane::OnFocusSetNotification(UINT code, NMHDR* hdr, LRESULT* res)
{
	if (hdr)
		OnFocusSet(static_cast<UINT>(hdr->idFrom));
}


void Pane::OnAutoCompleteDropDown(UINT code)
{
	size_t index= code - POPUP_ID;
	if (index < FieldCount())
	{
		if (CEdit* edit= auto_complete_.CurrentEditCtrl())
		{
			// sanity check:
			if (edit->GetDlgCtrlID() == EDIT_ID + index)
			{
				if (auto_complete_.IsWindowVisible())
					auto_complete_.ShowWindow(SW_HIDE);
				else
					auto_complete_.ShowList();
			}
		}
	}
}


} // namespace
