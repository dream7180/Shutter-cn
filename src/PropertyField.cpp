/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PropertyField.h"

using namespace Property;


void Field::AddCurTextToHistory()
{
	if (/*edit_history_ == 0 ||*/ history_.get() == 0)
		return;

	if (CEdit* edit_ctrl= dynamic_cast<CEdit*>(edit.get()))
	{
		if (edit_ctrl->GetWindowTextLength() > 0)
		{
			CString str;
			edit_ctrl->GetWindowText(str);
			history_->AddString(str);
		}
	}
}


void Field::SaveHistory()
{
	if (/*edit_history_ != 0 &&*/ history_.get() != 0)
		history_->SaveHistory();
}


const std::vector<String>* Field::GetHistory()
{
	if (history_.get() == 0)
		return 0;
	return history_->Array();
}


void Field::ShowAutoCompleteBtn(bool show)
{
	if (popup_btn_.m_hWnd)
	{
		// don't show this btn, if there is no history available
		if (show && (history_.get() == 0 || history_->Array() == 0 || history_->Array()->empty()))
			show = false;
		popup_btn_.ShowWindow(show ? SW_SHOWNA : SW_HIDE);
	}
}
