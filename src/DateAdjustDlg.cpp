/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DateAdjustDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DateAdjustDlg.h"
#include "PhotoInfo.h"
#include "BalloonMsg.h"
#include "CatchAll.h"
#include "ExifTags.h"
#include "DateTimeUtils.h"
#include <boost/date_time/posix_time/time_serialize.hpp>
#include "Color.h"
#include "UIElements.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// DateAdjustDlg dialog

DateAdjustDlg::DateAdjustDlg(CWnd* parent, VectPhotoInfo& photos)
	: DialogChild(DateAdjustDlg::IDD, parent), photos_(photos), time_span_(0, 0, 0, 0)
{
	days_ = hours_ = minutes_ = seconds_ = 0;
	adj_mode_ = 0;
	valid_ = ready_ = false;

	const TCHAR* SECTION= _T("DateTimeAdjDlg");
	profile_adj_mode_.Register(SECTION, _T("AdjMode"), adj_mode_);
	profile_days_.Register(SECTION, _T("Days"), days_);
	profile_hours_.Register(SECTION, _T("Hours"), hours_);
	profile_minutes_.Register(SECTION, _T("Minutes"), minutes_);
	profile_seconds_.Register(SECTION, _T("Seconds"), seconds_);

	SYSTEMTIME tm;
	::GetLocalTime(&tm);
	profile_date_time_.Register(SECTION, L"DateTime", ToISOString(SytemTimeToDateTime(tm)));
}

DateAdjustDlg::~DateAdjustDlg()
{}

void DateAdjustDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	DDX_Control(DX, IDC_DAYS, edit_days_);
	DDX_Control(DX, IDC_HOURS, edit_hours_);
	DDX_Control(DX, IDC_MINUTES, edit_minutes_);
	DDX_Control(DX, IDC_SECONDS, edit_seconds_);
	DDX_Control(DX, IDC_DATE, date_edit_);
	DDX_Control(DX, IDC_TIME, time_edit_);
	DDX_Radio(DX, IDC_ADJUST, adj_mode_);
	DDX_Control(DX, IDC_SPIN1, spin_days_);
	DDX_Control(DX, IDC_SPIN2, spin_hours_);
	DDX_Control(DX, IDC_SPIN3, spin_minutes_);
	DDX_Control(DX, IDC_SPIN4, spin_seconds_);
	DDX_Control(DX, IDC_LIST, results_);
	DDX_Control(DX, IDOK, ok_btn_);
}


BEGIN_MESSAGE_MAP(DateAdjustDlg, DialogChild)
	ON_BN_CLICKED(IDC_ADJUST, OnAdjustMode)
	ON_BN_CLICKED(IDC_SET_MODE, OnSetMode)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE, OnDateChange)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIME, OnTimeChange)
	ON_EN_CHANGE(IDC_DAYS, OnChangeDays)
	ON_EN_CHANGE(IDC_HOURS, OnChangeHours)
	ON_EN_CHANGE(IDC_MINUTES, OnChangeMinutes)
	ON_EN_CHANGE(IDC_SECONDS, OnChangeSeconds)
	ON_NOTIFY_EX(LVN_GETDISPINFO, IDC_LIST, OnGetDispInfo)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, OnCustomDraw)
END_MESSAGE_MAP()


// DateAdjustDlg message handlers


BOOL DateAdjustDlg::OnInitDialog()
{
	try
	{
		return InitDlg();
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


BOOL DateAdjustDlg::InitDlg()
{
	if (photos_.empty())
	{
		EndDialog(IDCANCEL);
		return true;
	}

	adj_mode_ = profile_adj_mode_;
	days_ = profile_days_;
	hours_ = profile_hours_;
	minutes_ = profile_minutes_;
	seconds_ = profile_seconds_;

	DialogChild::OnInitDialog();

	SetDlgItemInt(IDC_DAYS, days_);
	SetDlgItemInt(IDC_HOURS, hours_);
	SetDlgItemInt(IDC_MINUTES, minutes_);
	SetDlgItemInt(IDC_SECONDS, seconds_);

	BuildResizingMap();
	SetWndResizing(IDC_LIST, DlgAutoResize::RESIZE);
	//SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);
	SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
	SetWndResizing(IDOK, DlgAutoResize::MOVE);

	results_.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
	//results_.InsertColumn(0, _T("File Name"), LVCFMT_LEFT, 100);
	results_.InsertColumn(0, _T("现有的日期/时间"), LVCFMT_LEFT, Pixels(150));
	results_.InsertColumn(1, _T("新的日期/时间"), LVCFMT_LEFT, Pixels(150));
	results_.SetItemCount(static_cast<int>(photos_.size()));

	time_edit_.ModifyStyle(0, DTS_TIMEFORMAT);

	//SubclassHelpBtn(_T("ToolDateTime.htm"));

	spin_days_.SetRange32(-100000, 100000);
	spin_hours_.SetRange32(-1000000, 1000000);
	spin_minutes_.SetRange32(-10000000, 10000000);
	spin_seconds_.SetRange32(-2000000000, 2000000000);

	// some reasonable text length limits (include sign, digits, and thousand separators)
	edit_days_.SetLimitText(8);
	edit_hours_.SetLimitText(16);
	edit_minutes_.SetLimitText(16);
	edit_seconds_.SetLimitText(16);

	// set min/max date
	WPARAM flags= GDTR_MIN | GDTR_MAX;
	SYSTEMTIME sys[2];
	memset(sys, 0, sizeof(sys));
	sys[0].wYear = 1601;
	sys[0].wMonth = 1;
	sys[0].wDay = 2;
	sys[1].wYear = 9999;
	sys[1].wMonth = 12;
	sys[1].wDay = 31;
	date_edit_.SendMessage(DTM_SETRANGE, flags, reinterpret_cast<LPARAM>(sys));
	time_edit_.SendMessage(DTM_SETRANGE, flags, reinterpret_cast<LPARAM>(sys));

	auto dt= DateTimeToSytemTime(FromISOString(profile_date_time_));
	DateTime_SetSystemtime(date_edit_, GDT_VALID, &dt);
	DateTime_SetSystemtime(time_edit_, GDT_VALID, &dt);

	example_time_ = photos_.front()->GetDateTime();

	ready_ = true;

	EnableGroup(adj_mode_ == 0);

	UpdateExampleAndOkBtn();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void DateAdjustDlg::OnOK()
{
	if (!ready_)
		return;

	DateTime date_time;
	TimeDuration span;
	PhotoInfoPtr invalid= nullptr;
	bool adjust_time_mode= !!IsDlgButtonChecked(IDC_ADJUST);

	if (adjust_time_mode)
	{
		CWaitCursor wait;

		int d= 0, h= 0, m= 0, s= 0;
		if (!CheckAdjustment(photos_, invalid, span, d, h, m, s))
		{
			if (invalid)
			{
				example_time_ = invalid->GetDateTime();
				UpdateExampleAndOkBtn();

				oStringstream ost;
				ost << _T("本次调整不会对图像生效 ") << invalid->GetName() << _T(".");
				new BalloonMsg(GetDlgItem(IDC_ADJUST), _T("相关调整超出范围"),
					ost.str().c_str(), BalloonMsg::IERROR);
			}
			// if invalid == null then span itself is bogus
			return;
		}
		else
		{
			// store current entries in registry
			profile_days_ = d;
			profile_hours_ = h;
			profile_minutes_ = m;
			profile_seconds_ = s;

			days_ = d;
			hours_ = h;
			minutes_ = m;
			seconds_ = s;
		}
	}
	else
	{
		if (!GetDateTime(date_time))
			return;

		// store d/t in registry
		profile_date_time_ = ToISOString(date_time);
	}

	// store current entries in registry
	profile_adj_mode_ = adj_mode_;

	DialogChild::OnOK();
}


void DateAdjustDlg::OnAdjustMode()	{ adj_mode_ = 0; EnableGroup(true);  UpdateExampleAndOkBtn(); }
void DateAdjustDlg::OnSetMode()		{ adj_mode_ = 1; EnableGroup(false); UpdateExampleAndOkBtn(); }


void DateAdjustDlg::EnableGroup(bool first)
{
	if (!ready_)
		return;

	edit_days_.EnableWindow(first);
	edit_hours_.EnableWindow(first);
	edit_minutes_.EnableWindow(first);
	edit_seconds_.EnableWindow(first);

	spin_days_.Invalidate();
	spin_hours_.Invalidate();
	spin_minutes_.Invalidate();
	spin_seconds_.Invalidate();

	date_edit_.EnableWindow(!first);
	time_edit_.EnableWindow(!first);
}


void DateAdjustDlg::OnDateChange(NMHDR* nmhdr, LRESULT* result)
{
	LPNMDATETIMECHANGE dt_change= reinterpret_cast<LPNMDATETIMECHANGE>(nmhdr);
	*result = 0;

	UpdateExampleAndOkBtn();
}

void DateAdjustDlg::OnTimeChange(NMHDR* nmhdr, LRESULT* result)
{
	LPNMDATETIMECHANGE dt_change= reinterpret_cast<LPNMDATETIMECHANGE>(nmhdr);
	*result = 0;

	UpdateExampleAndOkBtn();
}


bool GetInt(const CWnd& wnd, int& val)
{
	CString str;
	wnd.GetWindowText(str);

	str.Trim(_T(" \t\n\r"));	// trim all leading and trailing whitespace characters

	val = 0;

	int len= str.GetLength();
	if (len == 0)
		return false;

	bool negative= false;

	for (int i= 0; i < len; ++i)
	{
		int c= str[i];

		switch (c)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				// use higher precision type to detect overflow
				int64 tmp= val;
				tmp = 10 * tmp + (c - '0');
				val = static_cast<int>(tmp);
				if (tmp != val)
					return false;	// overlow
			}
			break;

		case '-':
			if (i == 0 && len > 1)
				negative = true;
			else
				return false;
			break;

		case ',':
		case '\'':
			break;

		default:
			return false;
		}
	}

	if (negative)
		val = -val;

	return true;
}


bool DateAdjustDlg::GetDateTime(DateTime& dt) const
{
	SYSTEMTIME time, date;

	if (date_edit_.GetTime(&date) == GDT_VALID && time_edit_.GetTime(&time) == GDT_VALID)
	{
		// copy date portion
		time.wYear = date.wYear;
		time.wMonth = date.wMonth;
		time.wDay = date.wDay;

		try
		{
			auto new_time= SytemTimeToDateTime(time);
			dt = new_time;
			return true;
		}
		catch (std::exception&)
		{}
		catch (...)
		{ ASSERT(false); }
	}

	return false;
}


bool DateAdjustDlg::GetAdjustedTimeSpan(TimeDuration& span_ret) const
{
	if (!ready_)
		return false;

	span_ret = TimeDuration(0, 0, 0);

	try
	{
		ASSERT(adj_mode_ == 0);

		int days= 0, hours= 0, minutes= 0, seconds= 0;
		if (GetInt(edit_days_, days) && GetInt(edit_hours_, hours) && GetInt(edit_minutes_, minutes) && GetInt(edit_seconds_, seconds))
		{
			span_ret = CreateTimeDuration(days, hours, minutes, seconds);
			return true;
		}
	}
	catch (...)
	{
	}

	return false;
}


void DateAdjustDlg::UpdateExampleAndOkBtn()
{
	if (!ready_)
		return;

	if (adj_mode_ == 0)	// relative
		valid_ = GetAdjustedTimeSpan(time_span_);
	else
		valid_ = GetDateTime(time_set_);

	//TODO: check limits?

//	str = ::DateTimeFmt(dt).c_str();

	results_.Invalidate(false);
	ok_btn_.EnableWindow(valid_);
}


void DateAdjustDlg::OnChangeDays()			{ UpdateExampleAndOkBtn(); }
void DateAdjustDlg::OnChangeHours()			{ UpdateExampleAndOkBtn(); }
void DateAdjustDlg::OnChangeMinutes()		{ UpdateExampleAndOkBtn(); }
void DateAdjustDlg::OnChangeSeconds()		{ UpdateExampleAndOkBtn(); }


bool DateAdjustDlg::CheckAdjustment(VectPhotoInfo& photos, PhotoInfoPtr& invalid, TimeDuration& span_ret, int& d, int& h, int& m, int& s)
{
	ASSERT(adj_mode_ == 0);

	invalid = 0;

	int days= 0, hours= 0, minutes= 0, seconds= 0;
	if (GetInt(edit_days_, days) && GetInt(edit_hours_, hours) && GetInt(edit_minutes_, minutes) && GetInt(edit_seconds_, seconds))
	{
		auto span= CreateTimeDuration(days, hours, minutes, seconds);

		VectPhotoInfo::const_iterator end= photos.end();
		for (VectPhotoInfo::const_iterator it= photos.begin(); it != end; ++it)
		{
			auto tm= (*it)->GetDateTime();
			try
			{
				tm = AdjustDateTime(tm, span);
			}
			catch (std::exception&)
			{
				invalid = *it;	// photo causing problems
				return false;
			}
		}

		span_ret = span;

		d = days;
		h = hours;
		m = minutes;
		s = seconds;

		return true;
	}
	else
		return false;
}


String DateAdjustDlg::FormatAdjustedDT(DateTime date) const
{
	try
	{
		if (!date.is_not_a_date_time())
		{
			if (adj_mode_ == 0)
				date = AdjustDateTime(date, time_span_);
			else
				date = time_set_;

			return ::DateTimeFmt(date);
		}
	}
	catch (std::exception&)
	{}

	return String();
}


void DateAdjustDlg::OnCustomDraw(NMHDR* nm_hdr, LRESULT* result)
{
	NMLVCUSTOMDRAW* custom_draw= reinterpret_cast<NMLVCUSTOMDRAW*>(nm_hdr);
	*result = CDRF_DODEFAULT;

	if (custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*result = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYITEMDRAW;
	}
	else if (custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		auto line= static_cast<size_t>(custom_draw->nmcd.dwItemSpec);
		if (valid_ && line < photos_.size())
		{
			bool error= true;

			try
			{
				auto str= FormatAdjustedDT(photos_[line]->GetDateTime());
				error = str.empty();
			}
			catch (...)
			{}

			if (error)
				custom_draw->clrTextBk = ::CalcNewColor(::GetSysColor(COLOR_WINDOW), RGB(255,0,0), 0.5);
		}
	}
}


BOOL DateAdjustDlg::OnGetDispInfo(UINT id, NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	if ((disp_info->item.mask & LVIF_TEXT) == 0)
		return false;

	disp_info->item.pszText[0] = _T('\0');

	size_t line= disp_info->item.iItem;
	if (line >= photos_.size())
		return false;

	try
	{
		auto date= photos_[line]->GetDateTime();

		switch (disp_info->item.iSubItem)
		{
//		case 0:	// filename
//			_tcsncpy(disp_info->item.pszText, photos_[line]->GetName().c_str(), disp_info->item.cchTextMax);
//			break;

		case 0:	// existing date/time
			_tcsncpy(disp_info->item.pszText, ::DateTimeFmt(date).c_str(), disp_info->item.cchTextMax);
			break;

		case 1:	// new date/time
			if (valid_)
			{
				// in case of failure show error msg
				_tcsncpy(disp_info->item.pszText, _T("无效的日期"), disp_info->item.cchTextMax);

				try
				{
					auto str= FormatAdjustedDT(date);
					if (!str.empty())
						_tcsncpy(disp_info->item.pszText, str.c_str(), disp_info->item.cchTextMax);
				}
				catch (std::exception&)
				{}
			}
			else
			{
				disp_info->item.pszText[0] = _T('-');
				disp_info->item.pszText[1] = _T('\0');
			}
			break;
		}
	}
	catch(...)
	{
		ASSERT(false);
	}

	return true;
}


bool DateAdjustDlg::RelativeChange() const
{
	return adj_mode_ == 0;
}

DateTime DateAdjustDlg::GetDateTime() const
{
	return time_set_;
}

int DateAdjustDlg::GetDays() const
{
	return days_;
}

int DateAdjustDlg::GetHours() const
{
	return hours_;
}

int DateAdjustDlg::GetMinutes() const
{
	return minutes_;
}

int DateAdjustDlg::GetSeconds() const
{
	return seconds_;
}
