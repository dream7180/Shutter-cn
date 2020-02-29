/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// File Info edit: editing XMP metadata

#include "stdafx.h"
#include "resource.h"
#include "PropertyDlg.h"
#include "BalloonMsg.h"
#include "PhotoInfo.h"
#include "CatchAll.h"
#include "DlgAutoResize.h"
#include "UniqueLetter.h"
#include "PropertyFormStar.h"
#include "DlgListCtrl.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include "ToolBarWnd.h"
#include "viewer/FancyToolBar.h"
#include "ArrowButton.h"
#include "Profile.h"
#include "DescriptionPane.h"
#include "EnableCtrl.h"
#include "Color.h"
#include "WhistlerLook.h"
//#include <comdef.h>		// _COM_SMARTPTR_TYPEDEF
#include "ACListWnd.h"
#include "PopupMenuCtrl.h"
#include "PropertyPane.h"
#include "PropertyField.h"
#include "RecentStrings.h"
#include "EditHistory.h"
#include "Color.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString ReadAppVersion(bool incl_build);

using namespace Property;


/////////////////////////////////////////////////////////////////////////////
// CPropertyDlg dialog

struct CPropertyDlg::Impl
{
	XmpData data_;
	PhotoInfoPtr photo_;
	CString title_;
	AutoCompletePopup auto_complete_popup_;

	boost::ptr_vector<Field> fields0_;
	boost::ptr_vector<Field> fields1_;
	boost::ptr_vector<Field> fields2_;
	boost::ptr_vector<Field> fields3_;
	DescriptionPane* description_pane_;
	boost::ptr_vector<Pane> panes_;

	DlgListCtrl dlg_container_;
	FancyToolBar templates_tb_;
	CButton save_btn_;
	CButton cancel_btn_;
	ArrowButton btn_prev_;
	ArrowButton btn_next_;
	size_t count_of_images_;	// if > 1, user selected multiple images for FileInfo, the same xmp will be written to all of them
	bool enable_next_prev_;
	String registry_section_;

	Callback callback_;
	InitMenuCallback init_popup_menu_;

	Profile<bool> expand_pane_0_;
	Profile<bool> expand_pane_1_;
	Profile<bool> expand_pane_2_;
	Profile<bool> expand_pane_3_;

	bool Notify(CPropertyDlg* dlg, Action action, int index= -1);

	void InitDlg(DialogChild* wnd);

	void CheckNextPrev(CPropertyDlg* dlg);
	CRect GetShadowRect(CWnd* wnd) const;
	void TemplatePopup(CWnd* wnd);
	void OnTbDropDown(CWnd* parent, int cmd, size_t btn_index);
	bool ReadText();
	void SaveHistory();
	void WriteToDlgControls();
};


CPropertyDlg::CPropertyDlg(const Callback& callback, const InitMenuCallback& init_menu, CWnd* parent, size_t count_of_images,
						   bool enable_next_prev, const TCHAR* registry_section)
	: pImpl_(new Impl), DialogChild(CPropertyDlg::IDD, parent)
{
	pImpl_->photo_ = 0;
	pImpl_->callback_ = callback;
	pImpl_->init_popup_menu_ = init_menu;
	pImpl_->count_of_images_ = count_of_images;
	pImpl_->enable_next_prev_ = enable_next_prev;
	pImpl_->registry_section_ = registry_section;
	pImpl_->description_pane_ = 0;

	const TCHAR* SECTION= _T("FileInfoDlg");
	pImpl_->expand_pane_0_.Register(SECTION, _T("expPane-1"), true);
	pImpl_->expand_pane_1_.Register(SECTION, _T("expPane-2"), true);
	pImpl_->expand_pane_2_.Register(SECTION, _T("expPane-3"), false);
	pImpl_->expand_pane_3_.Register(SECTION, _T("expPane-4"), false);

	pImpl_->auto_complete_popup_.Create();
}


CPropertyDlg::~CPropertyDlg()
{}


void CPropertyDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	DDX_Control(DX, IDC_CONTAINER, pImpl_->dlg_container_);
//	DDX_Control(DX, IDC_TEMPLATES, pImpl_->templates_tb_);
	DDX_Control(DX, IDC_PREVIOUS, pImpl_->btn_prev_);
	DDX_Control(DX, IDC_NEXT, pImpl_->btn_next_);
	DDX_Control(DX, IDOK, pImpl_->save_btn_);
	DDX_Control(DX, IDCANCEL, pImpl_->cancel_btn_);
}


BEGIN_MESSAGE_MAP(CPropertyDlg, DialogChild)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_DESTROY()
//	ON_NOTIFY(TBN_DROPDOWN, IDC_TEMPLATES, OnTbDropDown)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_BN_CLICKED(IDC_PREVIOUS, OnPrev)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_COMMAND(ID_SAVE, OnSaveTemplate)
	ON_COMMAND(ID_LOAD, OnLoadTemplate)
	ON_COMMAND_RANGE(ID_BROWSE_FOLDER_1, ID_BROWSE_FOLDER_1 + 999, OnLoadTemplateFile)	// reusing IDs here
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyDlg message handlers


BOOL CPropertyDlg::OnInitDialog()
{
	try
	{
//		CRect client(0,0,0,0);
//		GetClientRect(client);
//		CSize minimal= client.Size();
//		minimal.cy /= 2;
//		SetMinimalDlgSize(minimal);

		GetWindowText(pImpl_->title_);

		pImpl_->Notify(this, Load);

		pImpl_->InitDlg(this);

		pImpl_->CheckNextPrev(this);

		BuildResizingMap();

		SetWndResizing(IDC_CONTAINER, DlgAutoResize::RESIZE);
		SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_TEMPLATES, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_PREVIOUS, DlgAutoResize::MOVE);
		SetWndResizing(IDC_NEXT, DlgAutoResize::MOVE);
		SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
		SetWndResizing(IDOK, DlgAutoResize::MOVE);

		return true;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);

	return true;
}


// read or write text from/to edit controls
//
bool ExchangeText(boost::ptr_vector<Field>& fields, bool read)
{
	const size_t count= fields.size();
	bool modified= false;

	for (size_t i= 0; i < count; ++i)
	{
		Field& field= fields[i];

		if (field.text == 0 || field.edit.get() == 0)
			continue;

		if (field.type == Field::EDITBOX)
		{
			if (read)
			{
				CString s;
				field.edit->GetWindowText(s);
				*field.text = s;
			}
			else
			{
				if (field.edit->m_hWnd)
					field.edit->SetWindowText(field.text->c_str());
			}
		}
		else if (field.type == Field::STARS)
		{
			PropertyFormStar* star= static_cast<PropertyFormStar*>(field.edit.get());

			if (read)
			{
				*field.text = star->Read();
				if (star->IsModified())
					modified = true;
			}
			else
				star->Reset(field.text);
		}
		else if (field.type == Field::DATE_TIME)
		{
			CDateTimeCtrl* ctrl= static_cast<CDateTimeCtrl*>(field.edit.get());

			if (read)
			{
				field.text->clear();
				COleDateTime tm;
				if (ctrl->GetTime(tm) && tm.GetStatus() == COleDateTime::valid)
					*field.text = ::DateTimeISO(tm, true);
			}
			else
			{
				bool set= true;

				if (field.text->size() >= 10)
				{
					const TCHAR* p= field.text->c_str();

					// parse ISO date (YYYY-MM-DD)
					int y= _ttoi(p);
					int m= _ttoi(p + 5);
					int d= _ttoi(p + 8);

					if (y >= 100 && m >= 1 && m <= 12 && d >= 1 && d <= 31)
					{
						COleDateTime tm(y, m, d, 0, 0, 0);
						ctrl->SetTime(tm);
						set = false;
					}
				}

				if (set)
					ctrl->SetTime();
			}
		}
	}

	return modified;
}


std::auto_ptr<EditHistory> CreateHistory(const String& reg_section, int pane_id, int field_id)
{
	oStringstream section;
	section << reg_section << _T("\\pane-") << pane_id << _T("\\field-") << field_id;
	return std::auto_ptr<EditHistory>(new EditHistory(section.str()));
}


void AddAutoComplete(CWnd* pane, Field& fld, const String& reg_section, CPoint right_top, int pane_id, int field_id, int popup_id)
{
	fld.history_ = ::CreateHistory(reg_section, pane_id, field_id);
//	fld.edit_history_.Attach(fld.history_, true);
//	fld.auto_complete_->Init(fld.edit->m_hWnd, fld.edit_history_.GetInterfacePtr(), 0, 0);
//	IAutoComplete2Ptr opt= fld.auto_complete_;
//	if (opt)
//		opt->SetOptions(ACO_AUTOSUGGEST | ACO_AUTOAPPEND | ACO_UPDOWNKEYDROPSLIST);
	fld.popup_btn_.Create(pane, popup_id);
	fld.popup_btn_.SetWindowPos(0, right_top.x, right_top.y - 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
}


bool CreatePane(Pane& pane, CWnd* parent, CFont* font, UniqueLetter& unique, const String& reg_section, int pane_id)
{
	pane.Create(IDD_PROPERTY_FORM, parent);

	CWnd* label= pane.GetDlgItem(IDC_LABEL);
	CWnd* edit= pane.GetDlgItem(IDC_EDIT);

	if (label == 0 || edit == 0 || font == 0)
		return false;

	WINDOWPLACEMENT wp;
	label->GetWindowPlacement(&wp);
	CRect r1= wp.rcNormalPosition;

	edit->GetWindowPlacement(&wp);
	CRect r2= wp.rcNormalPosition;

	const size_t count= pane.FieldCount();

	int row_height= std::max(r1.Height(), r2.Height());
	int extra_space= row_height * 3 / 10;

	for (size_t i= 0; i < count; ++i)
	{
		Field& fld= pane.GetField(i);

		if (fld.name)
		{
			String title= fld.name;
			unique.SelectUniqueLetter(title);
			fld.label.CreateEx(label->GetExStyle(), _T("STATIC"), title.c_str(), label->GetStyle(),
				r1.left, r1.top, r1.Width(), r1.Height(), pane, 0, 0);

			fld.label.SetDlgCtrlID(static_cast<int>(10000 + i));
			fld.label.SetFont(font, false);
		}

		int offset= row_height;
		int height= r2.Height();
		bool multiline= false;

		if (fld.type == Field::EDITBOX)
		{
			if (fld.lines > 1)
			{
				CDC dc;
				dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
				dc.SelectObject(font);
				TEXTMETRIC tm;
				dc.GetTextMetrics(&tm);
				height += tm.tmHeight * (fld.lines - 1);
				offset = height;
				multiline = true;
			}
			offset += extra_space;

			DWORD style= edit->GetStyle();
			if (multiline)
			{
				style |= ES_WANTRETURN | ES_MULTILINE | ES_AUTOVSCROLL;
				style &= ~ES_AUTOHSCROLL;	// wrap words
				//turn on vscroll?
				style |= WS_VSCROLL;
			}
			fld.edit.reset(new CEdit);
			fld.edit->CreateEx(edit->GetExStyle(), _T("EDIT"), 0, style, r2.left, r2.top, r2.Width(), height, pane, 0, 0);

			try
			{
				::AddAutoComplete(&pane, fld, reg_section, CPoint(r2.right, r2.top), pane_id, i, Pane::POPUP_ID + i);
			}
			catch (_com_error&)
			{
				ASSERT(false);
			}
		}
		else if (fld.type == Field::SEPARATOR)
		{
			CStatic* separator= 0;
			fld.edit.reset(separator = new CStatic);

			offset = row_height * 2 / 3;
			CRect rect(0, 0, 0, 0);
			rect.left = r1.left + offset;
			rect.right = r2.right - offset;
			rect.top = r2.top - extra_space + offset / 2 + 2;	// v center
			rect.bottom = rect.top + 1;

			separator->Create(0, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, rect, &pane);
		}
		else if (fld.type == Field::DATE_TIME)
		{
			offset += extra_space;

			CDateTimeCtrl* ctrl= new CDateTimeCtrl();
			fld.edit.reset(ctrl);

			ctrl->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | DTS_SHOWNONE | DTS_SHORTDATECENTURYFORMAT, r2, &pane, 0);
			COleDateTime min(100, 1, 1, 0, 0, 0);
			COleDateTime max(9999, 1, 1, 0, 0, 0);
			ctrl->SetRange(&min, &max);
		}
		else
		{
			offset += extra_space;

			fld.edit.reset(new PropertyFormStar);
			static_cast<PropertyFormStar*>(fld.edit.get())->Create(&pane, fld.text);

			fld.edit->SetWindowPos(0, r2.left, r2.top - 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
		}

		fld.edit->SetDlgCtrlID(Pane::EDIT_ID + i);
		fld.edit->SetFont(font, false);

		r1.OffsetRect(0, offset);
		r2.OffsetRect(0, offset);
	}

	label->DestroyWindow();
	edit->DestroyWindow();

	CRect rect(0,0,0,0);
	pane.GetClientRect(rect);
	pane.SetWindowPos(0, 0, 0, rect.Width(), r2.top + extra_space, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	pane.resize_.BuildMap(&pane);

	for (size_t i= 0; i < count; ++i)
	{
		pane.resize_.SetWndResizing(Pane::EDIT_ID + i, DlgAutoResize::RESIZE_H);
		if (::GetDlgItem(pane, Pane::POPUP_ID + i) != 0)
			pane.resize_.SetWndResizing(Pane::POPUP_ID + i, DlgAutoResize::MOVE_H);
	}

	return true;
}


void CPropertyDlg::Impl::InitDlg(DialogChild* wnd)
{
	btn_next_.DrawLeftArrow(false);

	wnd->DialogChild::OnInitDialog();

	if (count_of_images_ > 1)
	{
		::EnableCtrl(&btn_prev_, false);
		::EnableCtrl(&btn_next_, false);
		save_btn_.SetWindowText(_T("Save to All"));
		// shift 'Cancel' button
		WINDOWPLACEMENT wp;
		btn_next_.GetWindowPlacement(&wp);
		cancel_btn_.SetWindowPos(0, wp.rcNormalPosition.left, wp.rcNormalPosition.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else if (!enable_next_prev_)
	{
		btn_prev_.EnableWindow(false);
		btn_next_.EnableWindow(false);
	}

	if (CWnd* tb= wnd->GetDlgItem(IDC_TEMPLATES))
	{ // toolbar
		FancyToolBar::Params p;
		p.shade = -0.4f;
		p.desaturate = -1.0f;
		int cmd= IDC_POPUP;
		if (!templates_tb_.Create(wnd, "p", &cmd, IDB_INFO_BAND_OPT_HOT, &p))
			throw String(_T("Failed to create a toolbar."));

		templates_tb_.SetPadding(CRect(2,2,2,2));
	//	templates_tb_.SetOption(FancyToolBar::BEVEL_LOOK, false);
		templates_tb_.SetOption(FancyToolBar::HOT_OVERLAY, false);
	//	templates_tb_.SetOption(FancyToolBar::SHIFT_BTN, false);

		CSize s= templates_tb_.Size();
		WINDOWPLACEMENT wp;
		tb->GetWindowPlacement(&wp);
		templates_tb_.SetWindowPos(0, wp.rcNormalPosition.left, wp.rcNormalPosition.top,
			s.cx, s.cy, SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);

		tb->DestroyWindow();

		templates_tb_.SetCommandCallback(boost::bind(&Impl::OnTbDropDown, this, wnd, _1, _2));
		templates_tb_.SetDlgCtrlID(IDC_TEMPLATES);
		templates_tb_.SetOnIdleUpdateState(false);

		//templates_tb_.SetOnIdleUpdateState(false);
		//templates_tb_.SetPadding(6, 7);
		//int cmd= IDC_POPUP;
		//templates_tb_.AddButtons("V", &cmd, IDB_TEMPLATES, IDS_TEMPLATES);
	}

	wnd->SubclassHelpBtn(_T("ToolFileInfo.htm"));

	fields0_.push_back(new Field(_T("descr"), &data_.Description));
	//-------------------------------------------------
	{
		boost::ptr_vector<Field>& fields= fields1_;
		fields.reserve(4);
		fields.push_back(new Field(_T("作者"), &data_.Author));
		fields.push_back(new Field(_T("图像评级"), Field::STARS, &data_.ImageRating));
		fields.push_back(new Field(_T("版权信息"), 1, &data_.CopyrightNotice));
		fields.push_back(new Field(_T("关键字 (标记)"), 3, &data_.Keywords));
	}
	//-------------------------------------------------
	{
		boost::ptr_vector<Field>& fields= fields2_;
		fields.reserve(14);
		fields.push_back(new Field(_T("描述作者"), &data_.DescriptionWriter));
		fields.push_back(new Field(_T("标题"), 2, &data_.Headline));
		fields.push_back(new Field());
		fields.push_back(new Field(_T("创作者工作"), &data_.CreatorsJob));
		fields.push_back(new Field(_T("地址"), 2, &data_.Address));
		fields.push_back(new Field(_T("城市"), &data_.City));
		fields.push_back(new Field(_T("省"), &data_.State));
		fields.push_back(new Field(_T("邮编"), &data_.PostalCode));
		fields.push_back(new Field(_T("国家"), &data_.Country));
		fields.push_back(new Field());
		fields.push_back(new Field(_T("电话"), 2, &data_.Phones));
		fields.push_back(new Field(_T("邮件"), 2, &data_.EMails));
		fields.push_back(new Field(_T("网址"), 2, &data_.WebSites));
	}
	//-------------------------------------------------
	{
		boost::ptr_vector<Field>& fields= fields3_;
		fields.reserve(18);
	//	fields.push_back(new Field(_T("Title (Object Name)"), &data_.Title));
		fields.push_back(new Field(_T("文档标题"), &data_.DocumentTitle));
		fields.push_back(new Field(_T("作业标识"), &data_.JobIdentifier));
		fields.push_back(new Field(_T("说明"), 3, &data_.Instructions));
		fields.push_back(new Field(_T("提供者"), &data_.Provider));
		fields.push_back(new Field(_T("来源"), &data_.Source));
		fields.push_back(new Field(_T("使用期限"), 3, &data_.RightsUsageTerms));
		fields.push_back(new Field(_T("版权信息网址"), 2, &data_.CopyrightInfoURL));
		fields.push_back(new Field());
		fields.push_back(new Field(_T("创建日期"), Field::DATE_TIME, &data_.CreationDate));
		fields.push_back(new Field(_T("流派"), &data_.IntellectualGenre));
		fields.push_back(new Field(_T("位置"), &data_.Location));
		fields.push_back(new Field(_T("城市"), &data_.City2));
		fields.push_back(new Field(_T("省"), &data_.StateProvince));
		fields.push_back(new Field(_T("国家"), &data_.Country2));
		fields.push_back(new Field(_T("ISO 国家代码"), &data_.ISOCountryCode));
		fields.push_back(new Field());
		fields.push_back(new Field(_T("IPTC 场景"), 2, &data_.IPTCScene));
		fields.push_back(new Field(_T("IPTC 主体代码"), 2, &data_.IPTCSubjectCode));
	}

	CFont* font= wnd->GetFont();

	panes_.reserve(4);
	panes_.push_back(description_pane_ = new DescriptionPane(auto_complete_popup_, fields0_));
	panes_.push_back(new Pane(auto_complete_popup_, fields1_));
	panes_.push_back(new Pane(auto_complete_popup_, fields2_));
	panes_.push_back(new Pane(auto_complete_popup_, fields3_));

	UniqueLetter unique;
	// reserve some combinations
	unique.Reserve('g');	// (looks ugly)
	unique.Reserve('y');	// (looks ugly)
	unique.Reserve('p');	// prev
	unique.Reserve('n');	// next
	unique.Reserve('s');	// save

//	suppress 'cancel' option?
//  highlight current field as an option

	WINDOWPLACEMENT wp;
	dlg_container_.GetWindowPlacement(&wp);
	dlg_container_.SetRightMargin(wp.rcNormalPosition.left);

	int pane_id= 1;
	CWnd* parent= &dlg_container_;
	description_pane_->text_ = &data_.Description;
	description_pane_->Create(parent);
	if (CWnd* edit= description_pane_->GetDlgItem(Pane::EDIT_ID))
	{
		WINDOWPLACEMENT wp;
		edit->GetWindowPlacement(&wp);
		int field_id= 0;
		CPoint pos(wp.rcNormalPosition.right, wp.rcNormalPosition.top);
		::AddAutoComplete(description_pane_, fields0_[0], registry_section_, pos, pane_id++, field_id, Pane::POPUP_ID);
		description_pane_->resize_.AddControl(Pane::POPUP_ID, fields0_[0].popup_btn_);
		description_pane_->resize_.SetWndResizing(Pane::POPUP_ID, DlgAutoResize::MOVE_H);
	}
	else
	{
		ASSERT(false);
		throw String(_T("创建描述面板出错"));
	}

	unique.Reserve('d');	// &Description
//	::CreatePane(panes_[0], parent, font, unique, registry_section_, 1);
	::CreatePane(panes_[1], parent, font, unique, registry_section_, pane_id++);
	::CreatePane(panes_[2], parent, font, unique, registry_section_, pane_id++);
	::CreatePane(panes_[3], parent, font, unique, registry_section_, pane_id++);

	WriteToDlgControls();

	dlg_container_.AddSubDialog(&panes_[0], 0, _T("图像描述"), expand_pane_0_);
	dlg_container_.AddSubDialog(&panes_[1], 0, _T("图像信息"), expand_pane_1_);
	dlg_container_.AddSubDialog(&panes_[2], 0, _T("IPTC 联系"), expand_pane_2_);
	dlg_container_.AddSubDialog(&panes_[3], 0, _T("IPTC 信息"), expand_pane_3_);

//	ShowImage(photo_);
}


void CPropertyDlg::OnSize(UINT type, int cx, int cy)
{
	Invalidate();

	DialogChild::OnSize(type, cx, cy);

	CRect shadow= pImpl_->GetShadowRect(this);
	if (!shadow.IsRectEmpty())
		InvalidateRect(shadow);
}


// read modified text from edit fields to the XmpData
bool CPropertyDlg::Impl::ReadText()
{
	bool modified= ::ExchangeText(fields0_, true);
	if (::ExchangeText(fields1_, true)) modified = true;
	if (::ExchangeText(fields2_, true)) modified = true;
	if (::ExchangeText(fields3_, true)) modified = true;

	if (modified)
		return true;

	const size_t count= panes_.size();
	for (size_t i= 0 ; i < count; ++i)
		if (panes_[i].modified_)
			return true;

	return false;	// not modified
}


bool CPropertyDlg::Impl::Notify(CPropertyDlg* dlg, Action action, int index)
{
	bool modified= false;
	if (action != Load && action != LoadTemplate && action != HasNext && action != HasPrevious)
		modified = ReadText();

	//TODO: validation for action != Cancel
	//

	return callback_(dlg, action, photo_, data_, modified, index);
}


void CPropertyDlg::Impl::CheckNextPrev(CPropertyDlg* dlg)
{
	if (!enable_next_prev_)
		return;

	btn_next_.EnableWindow(Notify(dlg, HasNext));
	btn_prev_.EnableWindow(Notify(dlg, HasPrevious));
}


void CPropertyDlg::OnNext()
{
	pImpl_->Notify(this, Next);
	pImpl_->CheckNextPrev(this);
}


void CPropertyDlg::OnPrev()
{
	pImpl_->Notify(this, Previous);
	pImpl_->CheckNextPrev(this);
}


void CPropertyDlg::OnOK()
{
	try
	{
		//TODO: validate
		//IPTC_

//		ReadText();

		pImpl_->Notify(this, Save);

	// transfer keywords to IPTC_
/*
	if (!keywords_.empty())
	{
		CString keywords= keywords_.c_str();
		keywords.Replace(_T("\xd\xa"), _T("\n"));

		int count= 0;
		for (int i= 0; i < 9999; ++i)
		{
			CString keyword;
			if (!AfxExtractSubString(keyword, keywords, i))
				break;
			if (keyword.GetLength() > 64)
			{
				CEditFld* edit= edits_[4].get();
				edit->EnterEdit();
				new BalloonMsg(edit, _T("Keyword too long."),
					_T("Keywords cannot exceed 64 characters."), BalloonMsg::IERROR);
				return;
			}
			++count;
		}

		IPTC_.keywords_.clear();
		IPTC_.keywords_.reserve(count);

		for (int j= 0; j < count; ++j)
		{
			CString keyword;
			if (!AfxExtractSubString(keyword, keywords, j))
				break;
			if (!keyword.IsEmpty())
				IPTC_.keywords_.push_back(String(keyword));
		}
	}
	else
		IPTC_.keywords_.clear();
*/

//		EndDialog(IDOK);
//		pImpl_->SaveHistory();
	}
	CATCH_ALL
}


void CPropertyDlg::SaveHistory()
{
	pImpl_->SaveHistory();
}


void CPropertyDlg::OnCancel()
{
	pImpl_->Notify(this, Cancel);
}


void CPropertyDlg::OnDestroy()
{
	if (pImpl_->dlg_container_.GetCount() == 4)
	{
		pImpl_->expand_pane_0_ = pImpl_->dlg_container_.IsSubDialogExpanded(0);
		pImpl_->expand_pane_1_ = pImpl_->dlg_container_.IsSubDialogExpanded(1);
		pImpl_->expand_pane_2_ = pImpl_->dlg_container_.IsSubDialogExpanded(2);
		pImpl_->expand_pane_3_ = pImpl_->dlg_container_.IsSubDialogExpanded(3);
	}
	else
	{
		ASSERT(false);
	}
}


BOOL CPropertyDlg::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	COLORREF back= ::GetSysColor(COLOR_3DFACE);
	dc->FillSolidRect(rect, back);

	CRect shadow= pImpl_->GetShadowRect(this);
	if (!shadow.IsRectEmpty())
	{
		shadow.bottom = shadow.top + 1;

		if (WhistlerLook::IsAvailable())
		{
			static const float shade[4]= { -33.0f, -16.5f, -5.5f, -1.2f };
			for (int i= 0; i < 4; ++i)
			{
				dc->FillSolidRect(shadow, CalcShade(back, shade[i]));
				shadow.OffsetRect(0, 1);
			}
		}
		else
		{
			dc->FillSolidRect(shadow, ::GetSysColor(COLOR_3DSHADOW));
		}
	}

	return true;
}


CRect CPropertyDlg::Impl::GetShadowRect(CWnd* wnd) const
{
	CRect rect(0,0,0,0);
	wnd->GetClientRect(rect);

	WINDOWPLACEMENT wp;

	if (dlg_container_.m_hWnd != 0 && dlg_container_.GetWindowPlacement(&wp))
	{
		rect.top = wp.rcNormalPosition.bottom;
		rect.bottom = rect.top + 4;
	}
	else
		rect.SetRectEmpty();

	return rect;
}


void CPropertyDlg::Impl::OnTbDropDown(CWnd* parent, int cmd, size_t btn_index)
{
	switch (cmd)
	{
	case IDC_POPUP:
		TemplatePopup(parent);
		break;
	}
}


void CPropertyDlg::Impl::TemplatePopup(CWnd* wnd)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_XMP_TEMPLATES))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	if (popup == 0)
	{
		ASSERT(false);
		return;
	}

	init_popup_menu_(popup, ID_BROWSE_FOLDER_1);

	CRect rect;
	templates_tb_.GetRect(IDC_POPUP, rect);
	CPoint pos(rect.right, rect.top);
	templates_tb_.ClientToScreen(&pos);

	popup->TrackPopupMenu(TPM_LEFTALIGN | /*TPM_LEFTBUTTON |*/ TPM_RIGHTBUTTON, pos.x, pos.y, wnd);
}


LRESULT CPropertyDlg::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		OnEraseBkgnd(dc);

	return 1;
}


void CPropertyDlg::PhotoLoaded(PhotoInfoPtr photo, const XmpData& data)
{
	// metadata for 'photo' has been loaded; refresh dialog controls

	ASSERT(photo);

	pImpl_->photo_ = photo;

	SetXmp(data, true);

	ShowImage(photo);

	if (CWnd* wnd= GetParent())
	{
		if (pImpl_->count_of_images_ == 1)
		{
			CString title= pImpl_->title_ + _T(": ") + photo->GetName().c_str();
			wnd->SetWindowText(title);
		}
		else
		{
			CString title;
#ifdef _UNICODE
			title.Format(_T("%s\x2014%d 图像被选定"),
#else
			title.Format(_T("%s - %d 图像被选定"),
#endif
				static_cast<const TCHAR*>(pImpl_->title_), static_cast<int>(pImpl_->count_of_images_));

			wnd->SetWindowText(title);
		}
	}
}


void CPropertyDlg::EndDialog(int code)
{
	DialogChild::EndDialog(code);
}


void CPropertyDlg::OnSaveTemplate()
{
	pImpl_->Notify(this, SaveTemplate);
}


void CPropertyDlg::OnLoadTemplate()
{
	pImpl_->Notify(this, LoadTemplate);
}


void CPropertyDlg::OnLoadTemplateFile(UINT id)
{
	pImpl_->Notify(this, LoadTemplate, id - ID_BROWSE_FOLDER_1);
}


extern String GetAppIdentifier(bool including_build)
{
	return String(_T("ExifPro ") + ReadAppVersion(including_build));
}


void CPropertyDlg::SetXmp(const XmpData& data, bool clear_modified_flag)
{
	pImpl_->data_ = data;

	pImpl_->WriteToDlgControls();

	// mark app's version
	pImpl_->data_.CreatorTool = GetAppIdentifier(false);

	if (!clear_modified_flag)
	{
		const size_t count= pImpl_->panes_.size();
		for (size_t i= 0 ; i < count; ++i)
			pImpl_->panes_[i].modified_ = true;
	}
}


void CPropertyDlg::Impl::WriteToDlgControls()
{
	::ExchangeText(fields0_, false);
	::ExchangeText(fields1_, false);
	::ExchangeText(fields2_, false);
	::ExchangeText(fields3_, false);

	const size_t count= panes_.size();
	for (size_t i= 0 ; i < count; ++i)
		panes_[i].modified_ = false;
}


void CPropertyDlg::Impl::SaveHistory()
{
	boost::ptr_vector<Field>* fields[]= { &fields0_, &fields1_, &fields2_, &fields3_ };

	for (size_t i= 0; i < array_count(fields); ++i)
	{
		boost::ptr_vector<Field>& f= *fields[i];

		for_each(f.begin(), f.end(), boost::bind(&Field::AddCurTextToHistory, _1));
		for_each(f.begin(), f.end(), boost::bind(&Field::SaveHistory, _1));
	}
}
