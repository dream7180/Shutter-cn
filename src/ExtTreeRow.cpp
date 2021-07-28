/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtTreeRow.cpp: implementation of the ExtTreeRow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExtTreeRow.h"
#include "ExtTreeNode.h"
#include "ExtTreeCtrl.h"
#include "ColorProfile.h"
#include "resource.h"
#include "CtrlDraw.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFont ExtTreeRow::bold_fnt_;

//////////////////////////////////////////////////////////////////////

ExtTreeRow::ExtTreeRow(ExtTreeNode* Parent, ICMProfilePtr icm, UINT flags/*= RENDERING_INTENT*/)
 : parent_(Parent), icm_(icm)
{
	output_profile_ = !!(flags & OUTPUT);
	rendering_intent_ = !!(flags & RENDERING_INTENT);

	if (parent_)
		parent_->AddLeaf(this);

	bold_ = false;

	if (bold_fnt_.m_hObject == 0)
	{
		// create bold font
		LOGFONT lf;
		/*::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof lf, &lf);
		lf.lfWeight = FW_BOLD;
		//lf.lfQuality = ANTIALIASED_QUALITY;
		//lf.lfHeight += 1;
		_tcscpy(lf.lfFaceName, _T("Tahoma"));*/
		::GetDefaultGuiBoldFont(lf);
		bold_fnt_.CreateFontIndirect(&lf);
	}
}

ExtTreeRow::~ExtTreeRow()
{}


void ExtTreeRow::GetColumn(int column_index, CString& buff) const
{
	if (icm_ == 0)
		return;

	switch (column_index)
	{
	case COL_ITEM_NAME:
		buff = icm_->name_.c_str();
		break;

	case COL_PROFILE:
		if (icm_->default_s_rgb_)
			buff = _T("默认 sRGB 颜色配置");
		else
			buff = icm_->profile_path_.GetFileNameAndExt().c_str();
		break;

	case COL_RENDERING:
		if (IsPopupMenuColumn(COL_RENDERING))
			buff = icm_->GetRenderingIntent().c_str();
		break;

	case COL_ENABLE_FLAG:
		buff = icm_->enabled_ ? _T("[是]") : _T("[否]");
		break;

	default:
		buff.Empty();
		break;
	}
}


CString ExtTreeRow::GetColumn(int column_index) const
{
	CString buff;
	GetColumn(column_index, buff);
	return buff;
}


void ExtTreeRow::GetDisplayText(int column, CString& rstrBuffer, bool& bold_font) const
{
	bold_font = false;
	GetColumn(column, rstrBuffer);

	if (IsBold())
		bold_font = true;
}



// image index
//
int ExtTreeRow::GetImageIndex() const
{
	if (parent_)
		return parent_->GetImageIndex() + 3;

	return -1;
}


bool ExtTreeRow::IsBold() const
{
	return bold_;
}


void ExtTreeRow::Draw(CDC* dc, std::vector<std::pair<int, int>> column_info, COLORREF rgb_text, COLORREF rgb_back, CRect rect)
{
	if (column_info.empty())
		return;

	CRect bar= rect;
	CRect cell= rect;
	int x_pos= column_info[0].first;

	CString buffer;

	for (int col= 1; ; ++col)
	{
		// draw vertical line
		bar.left = x_pos;
		bar.right = bar.left + 1;

		dc->SetBkMode(OPAQUE);
		dc->FillSolidRect(bar, ::GetSysColor(COLOR_3DFACE));

		cell.left = x_pos + 1;

		if (col == column_info.size())
			break;

		x_pos += column_info[col].first;

		if (col == 1)	// first column?
		{
			// erase row background
			dc->FillSolidRect(cell, rgb_back);
		}

		cell.right = x_pos - 3;

		// text format flags
		UINT fmt= DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
		if ((column_info[col].second & HDF_JUSTIFYMASK) == HDF_RIGHT)
			fmt |= DT_RIGHT;

		// get column text
		buffer.Empty();
		bool bold= false;
		GetDisplayText(col, buffer, bold);

		bool on= buffer == _T("[是]");
		if (on || buffer == _T("[否]"))
		{
			CSize size= CtrlDraw::GetCheckBoxSize(dc->GetWindow());
			if (cell.Width() >= size.cx)
			{
				// center check box
				CPoint pos((cell.left + cell.right - size.cx) / 2, (cell.top + cell.bottom - size.cy) / 2 - 1); 
				// draw checkbox instead of text
				CtrlDraw::DrawCheckBox(*dc, CRect(pos, size), on ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL);
			}
		}
		else
		{
			dc->SetBkColor(rgb_back);
			dc->SetTextColor(rgb_text);

			CFont* old_font= 0;
			if (bold)
				old_font = dc->SelectObject(&bold_fnt_);

			// draw text
			dc->SetBkMode(TRANSPARENT);
			CRect text_rect= cell;
			text_rect.DeflateRect(4, 1, 4, 0);

			bool popup= IsPopupMenuColumn(col);

			if (popup)
				text_rect.right -= 6;

			if (text_rect.Width() > 0)
				dc->DrawText(buffer, text_rect, fmt);

			if (popup)
			{
				int x= text_rect.right + 3;
				if (x >= cell.left)
				{
					int y= (text_rect.top + text_rect.bottom - 3) / 2;

					dc->FillSolidRect(x + 0, y + 0, 5, 1, rgb_text);
					dc->FillSolidRect(x + 1, y + 1, 3, 1, rgb_text);
					dc->FillSolidRect(x + 2, y + 2, 1, 1, rgb_text);
				}
			}

			if (bold)
				dc->SelectObject(old_font);
		}
	}
}


bool ExtTreeRow::IsPopupMenuColumn(int column) const
{
	if (rendering_intent_ && column == COL_RENDERING)
		return true;

	if (column == COL_PROFILE)
		return true;

	return false;
}


bool ExtTreeRow::IsOutputProfile() const
{
	return output_profile_;
}


bool ExtTreeRow::IsMatchingProfile(ColorProfilePtr profile) const
{
	if (output_profile_)
	{
		if (profile->GetDeviceClass() == icSigInputClass ||
			profile->GetColorSpace() != icSigRgbData)
			return false;
	}
	else
	{
		if (profile->GetDeviceClass() == icSigOutputClass ||
			profile->GetColorSpace() != icSigRgbData)
			return false;
	}
	return true;
}
