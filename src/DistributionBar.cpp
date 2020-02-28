/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DistributionBar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DistributionBar.h"
#include "WhistlerLook.h"
#include "MemoryDC.h"
#include "CatchAll.h"
#include <boost/function.hpp>
#include "ToolBarWnd.h"
#include "UIElements.h"
#include "Color.h"
#include "DateTimeUtils.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


DistributionBar::~DistributionBar()
{
}


BEGIN_MESSAGE_MAP(DistributionBar, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()
	ON_COMMAND(ID_BACK, OnBack)
	ON_UPDATE_COMMAND_UI(ID_BACK, OnUpdateBack)
	ON_COMMAND(ID_NEXT, OnNext)
	ON_UPDATE_COMMAND_UI(ID_NEXT, OnUpdateNext)
	ON_COMMAND(ID_CANCEL_FILTER, OnCancelFilter)
	ON_UPDATE_COMMAND_UI(ID_CANCEL_FILTER, OnUpdateCancelFilter)
END_MESSAGE_MAP()


namespace {


int DaysPerMonth(int year, int month)
{
	static const uint8 days[]= { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if (month < 1 || month > 12)
	{
		ASSERT(false);
		return 0;
	}

	int cnt= days[month - 1];

	if (month != 2)
		return cnt;

	// leap year?
	if ((year & 3) == 0 && (year % 100 != 0 || year % 400 == 0))
		cnt++;

	return cnt;
}

// Simple class to accurately represent date consisting of year, month, and day;
// note that date_ is expressed in days and it may possibly contain invalid values
// like 30-th of Feb.
// However what counts here is speed of constructing date, and comparing it with another.
// Illegal values do not arise in practice, because there are no operations to add/subtract
// time span to/from the date
//
class Date
{
public:
	Date() : date_(0)
	{}
	Date(int index) : date_(index)
	{}
	Date(const Date& d) : date_(d.date_)
	{}
	Date(int year, int month, int day) : date_(ToIndex(year, month, day))
	{}

	bool operator < (Date d) const	{ return date_ < d.date_; }
	bool operator > (Date d) const	{ return date_ > d.date_; }
	bool operator <= (Date d) const	{ return date_ <= d.date_; }
	bool operator >= (Date d) const	{ return date_ >= d.date_; }
	bool operator != (Date d) const	{ return date_ != d.date_; }
	bool operator == (Date d) const	{ return date_ == d.date_; }

	// date span in days (may indicate more days than needed as each month has 31 reserved days)
	int operator - (Date d) const	{ return date_ - d.date_; }

	// offset date
//	Date operator + (int offset) const;

	int Year() const		{ return date_ / (31 * 12); }
	int Month() const		{ return ((date_ / 31) % 12) + 1; }
	int Day() const			{ return (date_ % 31) + 1; }
	int Index() const		{ return date_; }
	int MonthIndex() const	{ return date_ / 31; }

	static int ToIndex(int year, int month, int day)
	{
		ASSERT(month >= 1 && month <= 12);
		ASSERT(day >= 1 && month <= 31);
		return 31 * (12 * year + (month - 1)) + (day - 1);
	}

	void Set(int year, int month, int day)
	{
		date_ = ToIndex(year, month, day);
	}

private:
	int date_;
};


Date GetYearMonth(DateTime time)
{
	if (time.is_not_a_date_time())
		return 0;

	auto ymd= time.date().year_month_day();
	return Date(ymd.year, ymd.month, ymd.day);

//	SYSTEMTIME tm;
//	if (time.GetAsSystemTime(tm))
//		return Date(tm.wYear, tm.wMonth, tm.wDay);
//	return 0;
}


//Date Date::operator + (int offset) const
//{
//	Date date(date_ + offset);
//	int days= DaysPerMonth(date.Year(), date.Month());
//
//	int d= date.Day();
//	if (d > days)
//		date.date_ -= d - days;
//
//	return date;
//}


int YearsSpan(Date from, Date to)
{
	ASSERT(from.Index() <= to.Index());
	return to.Year() - from.Year() + 1;
}

int MonthsSpan(Date from, Date to)
{
	ASSERT(from.Index() <= to.Index());
	return to.MonthIndex() - from.MonthIndex() + 1;
}

int DaysSpan(Date from, Date to)
{
	ASSERT(from.Index() <= to.Index());

	int from_year= from.Year();
	int from_month= from.Month();
	int from_day= from.Day();

	int to_year= to.Year();
	int to_month= to.Month();
	int to_day= to.Day();

	if (from_year == to_year && from_month == to_month)
	{
		return to_day - from_day + 1;
	}

	int span= 0;
	for (int y= from_year; y <= to_year; ++y)
	{
		int limit= y == to_year ? to_month : 12;

		for (int m= y == from_year ? from_month : 1; m <= limit; ++m)
		{
			if (y == from_year && m == from_month)
			{
				span += std::max(DaysPerMonth(y, m) - from_day + 1, 0);
			}
			else if (y == to_year && m == to_month)
			{
				span += std::min(DaysPerMonth(y, m), to_day);
			}
			else
				span += DaysPerMonth(y, m);
		}
	}

	return span;
}


Date AddDays(Date date, int days)
{
	ASSERT(days >= 0);

	int year= date.Year();
	int month= date.Month();
	int day= date.Day();

	for (;;)
	{
		int rest= DaysPerMonth(year, month) - day;

		if (rest >= days)
			return Date(year, month, day + days);

		day = 0;
		days -= rest;

		if (++month > 12)
		{
			month = 1;
			++year;
		}
	}
}


struct DateRange
{
	DateRange()
	{}

	DateRange(Date from, Date to) : from(from), to(to)
	{}

	void Clear()
	{
		from = to = Date(0);
	}

	void Reset(Date d)
	{
		from = to = d;
	}

	void Normalize()
	{
		if (from > to)
			std::swap(from, to);
	}

	bool operator == (const DateRange& d) const		{ return from == d.from && to == d.to; }

	bool operator != (const DateRange& d) const		{ return from != d.from || to != d.to; }

	Date from;
	Date to;
};


bool SingleDaySpan(const DateRange& range)
{
//	return range.from.Year() == range.to.Year() &&
	// As long as Date class has no time part, comparing it directly suffices (instead
	// of comparing day/month/year)
	return range.from == range.to;
}


bool MoreThanSingleMonthSpan(const DateRange& range)
{
	return range.from.Year() == range.to.Year() && range.from.Month() != range.to.Month();
}



class Histogram
{
public:
	void Set(Date from, std::vector<int>& hist);

	bool SelectedRange(const DateRange& selected);

	int GetCount(int year) const;
	int GetCount(int year, int month) const;
	int GetCount(int year, int month, int day) const;

	bool IsValid() const;

	int MaxCountPerDays() const;
	int MaxCountPerMonths() const;
	int MaxCountPerYears() const;

	Date StartingDate() const	{ return starting_date_; }
	Date EndingDate() const		{ return ending_date_; }

	DateRange Range() const		{ return DateRange(starting_date_, ending_date_); }

	// check if there is any data in the [from..to] range (inclusive 'from' & 'to')
	bool IsRangeNonEmpty(Date from, Date to) const;

	const DateRange& VisibleRange() const	{ return selected_range_; }

	bool SpansMultipleDays(DateRange range) const;

	bool SpansMultipleMonths(DateRange range) const;

private:
	Date starting_date_;
	Date ending_date_;
	DateRange selected_range_;
	std::vector<int> hist_;

	int Accumulate(int year, int from_month, int to_month, int from_day, int to_day) const;
	std::pair<size_t, size_t> TransformRange(Date from, Date to) const;
};


void Histogram::Set(Date from, std::vector<int>& hist)
{
	hist_.swap(hist);
	starting_date_ = from;
	if (hist_.empty())
		ending_date_ = Date(0);
	else
		ending_date_ = Date(starting_date_.Index() + static_cast<int>(hist_.size()) - 1);

	selected_range_.Clear();
}


bool Histogram::SelectedRange(const DateRange& selected)
{
	bool show_selection= false;
	selected_range_ = selected;

	if (selected.from != 0 && selected.to != 0 &&
		(selected.from > starting_date_ || selected.to < ending_date_))
		show_selection = true;

	return show_selection;
}


int Histogram::MaxCountPerDays() const
{
	std::vector<int>::const_iterator it= max_element(hist_.begin(), hist_.end());
	if (it != hist_.end())
		return *it;
	return 0;
}


int Histogram::MaxCountPerMonths() const
{
	int from_year= starting_date_.Year();
	int from_month= starting_date_.Month();

	int to_year= ending_date_.Year();
	int to_month= ending_date_.Month();

	int max_count= 0;
	for (int y= from_year; y <= to_year; ++y)
	{
		int limit= y == to_year ? to_month : 12;

		for (int m= y == from_year ? from_month : 1; m <= limit; ++m)
		{
			int count= GetCount(y, m);
			if (count > max_count)
				max_count = count;
		}
	}

	return max_count;
}


int Histogram::MaxCountPerYears() const
{
	int from_year= starting_date_.Year();
	int to_year= ending_date_.Year();

	int max_count= 0;
	for (int y= from_year; y <= to_year; ++y)
	{
		int count= GetCount(y);
		if (count > max_count)
			max_count = count;
	}

	return max_count;
}


int Histogram::Accumulate(int year, int from_month, int to_month, int from_day, int to_day) const
{
	if (hist_.empty())
		return 0;

	int from= Date::ToIndex(year, from_month, from_day) - starting_date_.Index();
	int to= Date::ToIndex(year, to_month, to_day) - starting_date_.Index() + 1;

	if (from < 0) from = 0;
	if (from > hist_.size()) from = static_cast<int>(hist_.size());
	if (to < 0) to = 0;
	if (to > hist_.size()) to = static_cast<int>(hist_.size());

	if (from >= to)
		return 0;

	return accumulate(hist_.begin() + from, hist_.begin() + to, 0);
}


int Histogram::GetCount(int year) const
{
	return Accumulate(year, 1, 12, 1, 31);
}


int Histogram::GetCount(int year, int month) const
{
	return Accumulate(year, month, month, 1, 31);
}


int Histogram::GetCount(int year, int month, int day) const
{
	int idx= Date::ToIndex(year, month, day) - starting_date_.Index();
	if (static_cast<size_t>(idx) < hist_.size())
		return hist_[idx];

	ASSERT(false);	// out of range
	return 0;
}


bool Histogram::IsValid() const
{
	return !hist_.empty() && starting_date_ <= ending_date_;
}


std::pair<size_t, size_t> Histogram::TransformRange(Date from, Date to) const
{
	ASSERT(from <= to);

	int start= from < starting_date_ ? 0 : from.Index() - starting_date_.Index();

	int end= to.Index() - starting_date_.Index() + 1;
	if (static_cast<size_t>(end) >= hist_.size())
		end = static_cast<int>(hist_.size());

	if (static_cast<size_t>(start) >= hist_.size() || start > end)
	{
		ASSERT(false);
		return std::make_pair(0, 0);
	}

	return std::make_pair(start, end);
}


bool Histogram::IsRangeNonEmpty(Date from, Date to) const
{
	if (!IsValid())
		return false;

	ASSERT(from <= to);

	std::pair<size_t, size_t> range= TransformRange(from, to);

	for (size_t i= range.first; i < range.second; ++i)
		if (hist_[i] != 0)
			return true;

	return false;
}


bool Histogram::SpansMultipleDays(DateRange range) const
{
	if (!IsValid())
		return false;

	range.Normalize();

	std::pair<size_t, size_t> indices= TransformRange(range.from, range.to);

	int days= 0;
	for (size_t i= indices.first; i < indices.second; ++i)
		if (hist_[i] != 0 && ++days > 1)
			return true;

	return false;
}


bool Histogram::SpansMultipleMonths(DateRange range) const
{
return false;
/*
	if (!IsValid())
		return false;

	range.Normalize();

	std::pair<size_t, size_t> indices= TransformRange(range.from, range.to);
*/

}


}	// namespace

///////////////////////////////////////////////////////////////////////////////

static const int TOP_DIST= 4;
static const int HIST_HEIGHT= 20;

extern int GetDefaultLineHeight();

int DistributionBar::GetHeight() const
{
	return Pixels(48);
}


struct DistributionBar::Impl
{
	Impl()
	{}

	Histogram hist_;
	int leftMargin_;
	int rightMargin_;
	CFont fontLabels_;
	CFont fontBold_;
	CRect hist_area_;
	bool area_valid_;
	bool tracking_cursor_;
	enum Precision { NONE, DAYS, MONTHS, YEARS } precision_;
	boost::function<void (DateTime from, DateTime to, DateTime hist_from, DateTime hist_to)> filter_callback_;
	boost::function<void ()> cancel_callback_;
	boost::function<bool ()> is_filter_active_;
	DateRange date_range_;
	DateRange selection_range_;	// interactive (user) selection
	bool show_selection_;
	DateRange selection_;		// selected/filtered range (displayed with a color above bars)
	DateTime tick_;				// a tick pointing to the selected time

	class History
	{
	public:
		History()
		{
			current_ = -1;
		}

		void Add(DateRange filter, DateRange histogram)
		{
			// remove all entries following current one
			if (current_ + 1 < history_.size())
				history_.erase(history_.begin() + current_ + 1, history_.end());

			history_.push_back(std::make_pair(filter, histogram));
			current_ = static_cast<int>(history_.size()) - 1;
		}

		void SetCur(DateRange filter, DateRange histogram)
		{
			if (history_.empty())
				Add(filter, histogram);
			else
				history_[current_] = std::make_pair(filter, histogram);
		}

		std::pair<DateRange, DateRange> Back()
		{
			return history_.at(--current_);
		}

		std::pair<DateRange, DateRange> Next()
		{
			return history_.at(++current_);
		}

		void Clear()
		{
			history_.clear();
			current_ = -1;
		}

		bool HasPrevious() const
		{
			return current_ > 0;	// more than just current entry?
		}

		bool HasNext() const
		{
			return current_ + 1 < history_.size();
		}

		bool HasHistory()
		{
			return history_.size() > 1;
		}

	private:
		// each vector entry consists of two date ranges; first range is filter selection
		// while second range is for the time line histogram; second ran
		std::vector<std::pair<DateRange, DateRange>> history_;
		int current_;
	} history_;

	ToolBarWnd toolbar_;

	Precision DrawTimeHistogram(CDC& dc, const CRect& rect, COLORREF rgb_tick, COLORREF base_color, const Histogram& hist);
	Date HitTest(CPoint pos);
	void SetSelection(Date date);
	void InitSelection(Date date);
	void CancelSelecting();
	void AddHistory(DateRange filter, DateRange histogram);
	void AddCurrent(DateRange filter, DateRange histogram);
	void Resize(CWnd* parent);
	void Filter(DateRange filter, DateRange histogram);
	// determine if selected 'range' should be used as a new histogram range
	bool DrillDown(DateRange range) const;

	void DrawTick(CDC& dc, const CRect& hist_rect, DateTime tick);
	std::pair<Date, Date> GetHistRange();
};


DistributionBar::DistributionBar() : impl_(new Impl)
{
	impl_->leftMargin_ = 0;
	impl_->rightMargin_ = 0;
	impl_->fontLabels_.CreateFont(-Pixels(10), 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, _T("Tahoma"));
	impl_->fontBold_.CreateFont(-Pixels(10), 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, _T("Tahoma"));
	impl_->area_valid_ = false;
	impl_->tracking_cursor_ = false;
	impl_->hist_area_.SetRectEmpty();
	impl_->precision_ = Impl::NONE;
	impl_->show_selection_ = false;
}


bool DistributionBar::Impl::DrillDown(DateRange range) const
{
	if (!hist_.IsValid())
	{
		ASSERT(false);
		return false;
	}

	// if histogram shows days already, this is max of its current resolution;
	// no way to drill down any further (until Date type resolution is increased)
	if (precision_ == DAYS)
		return false;

//	range.Normalize();

	if (precision_ == MONTHS && hist_.SpansMultipleDays(range))
		return true;

	if (precision_ == YEARS && hist_.SpansMultipleDays(range)) //SpansMultipleMonths(range))
		return true;

	return false;
}


void DistributionBar::SetCancelCallback(const boost::function<void ()>& callback, const boost::function<bool ()>& is_filter_active)
{
	impl_->cancel_callback_ = callback;
	impl_->is_filter_active_ = is_filter_active;
}


void DistributionBar::SetFilterCallback(const boost::function<void (DateTime from, DateTime to, DateTime hist_from, DateTime hist_to)>& callback)
{
	impl_->filter_callback_ = callback;
}


// DistributionBar message handlers


bool DistributionBar::Create(CWnd* parent)
{
	if (!CWnd::Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;
/*
	static int cmds[]= { ID_VIEW_SORT };
	tool_bar_wnd_.SetPadding(3, 10);
	if (!tool_bar_wnd_.Create("V", cmds, IDB_EMPTY2, IDS_RESERVE_SPACE, this))
		return false;

	tool_bar_wnd_.SetWindowPos(0, 2, 9, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	CRect rect;
	tool_bar_wnd_.GetClientRect(rect);
	left_margin_ = rect.right;
*/

	impl_->toolbar_.SetPadding(7, 9);

	static const int cmds[]= { ID_BACK, ID_NEXT, ID_CANCEL_FILTER };
	if (!impl_->toolbar_.Create("ppp", cmds, IDB_DISTRIBUTION_TB, IDS_DISTRIBUTION_TB, this))
		return false;

	impl_->toolbar_.CreateDisabledImageList(IDB_DISTRIBUTION_TB, 0.0f, 0.0f, 0.5f);
	//impl_->toolbar_.SetDisabledImageList(IDB_DISTRIBUTION_TB_DIS);

	CRect rect(0,0,0,0);
	impl_->toolbar_.GetClientRect(rect);

	impl_->leftMargin_ = 6;
	impl_->rightMargin_ = rect.Width();

	impl_->Resize(this);

	return true;
}


void DistributionBar::Impl::Resize(CWnd* parent)
{
	if (toolbar_.m_hWnd == 0)
		return;

	CRect client(0,0,0,0);
	parent->GetClientRect(client);

	CRect rect(0,0,0,0);
	toolbar_.GetClientRect(rect);

	int x= std::max<int>(0, client.right - rect.Width() - Pixels(3));
	int y= std::max<int>(0, (client.Height() - rect.Height()) / 3 - 1);

	toolbar_.SetWindowPos(0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void DistributionBar::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	impl_->Resize(this);
}


BOOL DistributionBar::OnEraseBkgnd(CDC* dc_ptr)
{
	CRect rect;
	GetClientRect(rect);

	MemoryDC dc(*dc_ptr, this);

	dc.SetBkMode(OPAQUE);

	COLORREF rgbBARS = RGB(45, 200, 100);//g_Settings.AppColors()[AppColors::DimText];//  RGB(105, 130, 191);
	COLORREF rgbTICK = g_Settings.AppColors()[AppColors::DimText];//rgbBARS;// CalcShade(rgb_color, -30.0f);// RGB(105, 130, 191);

	::DrawPanelBackground(dc, rect);

	CRect hist_rect= rect;
	hist_rect.top += Pixels(TOP_DIST);
	hist_rect.bottom = hist_rect.top + Pixels(dc, HIST_HEIGHT);
	hist_rect.left += impl_->leftMargin_ + Pixels(dc, 6);
	hist_rect.right -= impl_->rightMargin_ + Pixels(dc, 10);

	if (hist_rect.Width() > 0 && hist_rect.Height() > 0)
	{
		::Draw3DWell(dc, hist_rect);
		hist_rect.DeflateRect(1, 1);

		impl_->precision_ = impl_->DrawTimeHistogram(dc, hist_rect, rgbTICK, rgbBARS, impl_->hist_);

		impl_->area_valid_ = impl_->precision_ != Impl::NONE;
		impl_->hist_area_ = hist_rect;

		if (!impl_->tick_.is_not_a_date_time())
			impl_->DrawTick(dc, hist_rect, impl_->tick_);
	}

	dc.BitBlt();

	return true;
}


DateRange BuildHistogram(const VectPhotoInfo& photos, std::vector<int>& histogram_out)
{
	if (photos.empty())
	{
		histogram_out.clear();
		return DateRange(0, 0);
	}

	Date lower(1900, 1, 1);
	Date upper(2100, 1, 1);

	std::vector<int> histogram(upper - lower, 0);

	Date min_time(upper);
	Date max_time(0);

	VectPhotoInfo::const_iterator end= photos.end();
	for (VectPhotoInfo::const_iterator it= photos.begin(); it != end; ++it)
	{
		if (PhotoInfoPtr photo= *it)
		{
			Date time= GetYearMonth(photo->GetDateTime());

			if (time < upper && time >= lower)
			{
				histogram[time - lower]++;

				if (time < min_time)
					min_time = time;
				if (time > max_time)
					max_time = time;
			}
		}
	}

	if (max_time >= min_time)	// has valid range?
	{
		histogram.erase(histogram.begin(), histogram.begin() + (min_time - lower));

		histogram.erase(histogram.end() - (upper - max_time - 1), histogram.end());
	}

	histogram_out.assign(histogram.begin(), histogram.end());

	return DateRange(min_time, max_time);
}


void DistributionBar::BuildHistogram(const VectPhotoInfo& visible_photos, const VectPhotoInfo& histo)
{
	try
	{
		// first build histogram for visible photos only
		std::vector<int> histogram;
		DateRange visible_range= ::BuildHistogram(histo.empty() ? visible_photos : histo, histogram);
		DateRange histo_range= visible_range;
		std::vector<int> temp;
		DateRange selected= ::BuildHistogram(visible_photos, temp);

		histo_range.Normalize();

		impl_->hist_.Set(histo_range.from, histogram);
		impl_->show_selection_ = impl_->hist_.SelectedRange(selected);
		if (impl_->show_selection_)
			impl_->selection_ = selected;
		else
			impl_->selection_.Clear();

		impl_->date_range_ = histo_range;

		// 'current' time line
		impl_->AddCurrent(selected /*visible_range*/ /*impl_->date_range_*/, impl_->hist_.Range());

		if (m_hWnd)
			Invalidate();
		if (impl_->toolbar_.m_hWnd)
			impl_->toolbar_.Invalidate();	// same labels may be under the toolbar and will be clipped without redrawing it
	}
	CATCH_ALL
}


CString GetMonthName(int month)
{
	switch (month)
	{
	case 1:		return _T("一月");
	case 2:		return _T("二月");
	case 3:		return _T("三月");
	case 4:		return _T("四月");
	case 5:		return _T("五月");
	case 6:		return _T("六月");
	case 7:		return _T("七月");
	case 8:		return _T("八月");
	case 9:		return _T("九月");
	case 10:	return _T("十月");
	case 11:	return _T("十一月");
	case 12:	return _T("十二月");

	default:	ASSERT(false); return _T("");
	}
}


static void DrawBar(CDC& dc, int x, int y, int h, int w, COLORREF bar, COLORREF outline)
{
/*
	Gdiplus::Graphics g(dc);

	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.TranslateTransform(-0.5f, -0.5f);

	Gdiplus::RectF rect= CRectToRectF(x, y - h, w, h);
	{
		Gdiplus::Color top(c2c(CalcShade(outline, 20.0f)));
		Gdiplus::Color bottom(c2c(CalcShade(outline, -1.0f)));
		Gdiplus::LinearGradientBrush fill(rect, top, bottom, Gdiplus::LinearGradientModeVertical);
		g.FillRectangle(&fill, rect);
	}

	{
		rect = CRectToRectF(x + 1, y - h + 1, w - 2, h - 1);
		Gdiplus::Color top(c2c(CalcShade(bar, 25.0f)));
		Gdiplus::Color bottom(c2c(bar));
		Gdiplus::LinearGradientBrush fill(rect, top, bottom, Gdiplus::LinearGradientModeVertical);
		g.FillRectangle(&fill, rect);
	} */

	// bar
	dc.FillSolidRect(x, y - h, w, h, bar);
	// outline
	//dc.FillSolidRect(x, y - h - 1, w, 1, rgb_outline);
	//dc.FillSolidRect(x, y - h, 1, h, rgb_outline);
	//dc.FillSolidRect(x + w - 1, y - h, 1, h, rgb_outline);
}


extern inline double log2(double x)
{
	const double ln2= 0.69314718055994530942;
	return log(x) * (1.0 / ln2);
}


CSize GetTextExtent(CDC& dc, CFont& font, const TCHAR* text)
{
	dc.SelectObject(font);
	return dc.GetTextExtent(text, static_cast<int>(_tcslen(text)));
}


static void DrawHistBar(CDC& dc, int count, int max_count, int x, int y, CSize bar_space, COLORREF rgb_bar, COLORREF rgb_outline)
{
	if (max_count <= 0)
		return;

	COLORREF back= dc.GetBkColor();
	COLORREF text= dc.GetTextColor();
	int mode= dc.GetBkMode();

	int bar_width= bar_space.cx * 8 / 10;
	if (((bar_space.cx - bar_width) & 1) == 0)
		bar_width--;
	int bar_offset= (bar_space.cx - bar_width) / 2 + 1;

	double height_value= 0.0;
	if (count > 0)
	{
		if (count == max_count)
			height_value = 1.0;
		else
			height_value = log2(count) / log2(max_count);
	}

	int height= static_cast<int>(height_value * (bar_space.cy - 2));
	if (height == 0 && count > 0)
		height = 1;
	if (height > 0)
		DrawBar(dc, x + bar_offset, y, height, bar_width, rgb_bar, rgb_outline);

	dc.SetBkColor(back);
	dc.SetTextColor(text);
	dc.SetBkMode(mode);
}

// helper fn
static int DrawLabelText(CDC& dc, CFont& font, CRect text_rect, const TCHAR* value, UINT flags)
{
	dc.SelectObject(font);
	dc.DrawText(value, static_cast<int>(_tcslen(value)), text_rect, flags | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
	return dc.GetTextExtent(value, static_cast<int>(_tcslen(value))).cx;
}

static int DrawLabelText(CDC& dc, CFont& font, CRect text_rect, int value, UINT flags)
{
	dc.SelectObject(font);
	CString str;
	str.Format(_T("%d"), value);
	dc.DrawText(str, text_rect, flags | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
	return dc.GetTextExtent(str).cx;
}


DistributionBar::Impl::Precision DistributionBar::Impl::DrawTimeHistogram(CDC& dc, const CRect& rect, COLORREF rgb_tick, COLORREF base_color, const Histogram& hist)
{
	if (!hist.IsValid())
		return NONE;

	if (rect.Width() <= 0 || rect.Height() <= 0)
		return NONE;

	const COLORREF rgb_bar = base_color;// CalcShade(base_color, 40.0f);
	const COLORREF rgb_selection= CalcShade(base_color, -35.0f);
	const COLORREF rgb_outline= CalcShade(base_color, 30.0f);
	const COLORREF selected_range_color = RGB(247, 123, 0);//::GetSysColor(COLOR_HIGHLIGHT);
	const COLORREF label_color = g_Settings.AppColors()[AppColors::DimText];

	const Date from_date= hist.StartingDate();
	const Date to_date= hist.EndingDate();

	const int years_span= YearsSpan(from_date, to_date);
	const int months_span= MonthsSpan(from_date, to_date);
	const int days_span= DaysSpan(from_date, to_date);

	const CSize year_size= ::GetTextExtent(dc, fontBold_, _T(" 2000 "));
	const CSize month_size= ::GetTextExtent(dc, fontLabels_, _T(" June "));
	const CSize day_size= ::GetTextExtent(dc, fontLabels_, _T(" 30 "));
	const int SPACE= ::GetTextExtent(dc, fontBold_, _T(" ")).cx;

	const int monthSpace= rect.Width() / months_span;
	const int yearSpace= rect.Width() / years_span;
	const int daySpace= rect.Width() / days_span;

	const int TICK_LIMIT= Pixels(5);	// tick density limit: one tick at most every 5 pixels

	const bool drawDays= daySpace >= day_size.cx;
	const bool drawMonthsNames= monthSpace > month_size.cx;
	const bool drawYearsNames= yearSpace > year_size.cx;

	const bool drawMonthsTicks= monthSpace >= TICK_LIMIT;
	const bool drawYearsTicks= yearSpace >= TICK_LIMIT;
	const bool drawDaysTicks= daySpace >= TICK_LIMIT;

	const int TICK_H= Pixels(2);
	const int y= rect.bottom;

	const int SEL_HEIGHT= Pixels(2);

	Precision precision= NONE;

	Date selection_from= selection_range_.from;
	Date selection_to= selection_range_.to;
	if (selection_from > selection_to)
		std::swap(selection_to, selection_from);

	DateRange visible_range= hist.VisibleRange();

	dc.SetTextColor(label_color);
	dc.SetBkMode(TRANSPARENT);

	if (drawDaysTicks)	// the highest resolution: draw days, months and years
	{
		int max_count= hist.MaxCountPerDays();
		int label_right_pos= rect.left;

		for (int year= from_date.Year(), i= 0; year <= to_date.Year(); ++year)
		{
			int from= year == from_date.Year() ? from_date.Month() : 1;
			int to= year == to_date.Year() ? to_date.Month() : 12;

			for (int month= from; month <= to; ++month)
			{
				int d_from= year == from_date.Year() && month == from_date.Month() ? from_date.Day() : 1;
				int d_to= year == to_date.Year() && month == to_date.Month() ? to_date.Day() : DaysPerMonth(year, month);

				for (int day= d_from; day <= d_to; ++day, ++i)
				{
					int x= rect.left + i * rect.Width() / days_span;

					int h= TICK_H;
					//if (month == 1)
					//	h += TICK_H;
					if (day == 1)
						h += TICK_H;
					dc.FillSolidRect(x, y + 1, 1, h, rgb_tick);

					{ // selection
						Date cur(year, month, day);
						int right= rect.left + (i + 1) * rect.Width() / days_span;

						if (cur >= selection_from && cur <= selection_to)
							dc.FillSolidRect(x, rect.top, right - x, rect.Height(), rgb_selection);

						if (show_selection_ &&
							cur >= visible_range.from && cur <= visible_range.to)
							dc.FillSolidRect(x, rect.top, right - x, SEL_HEIGHT, selected_range_color);
					}

					int cnt= hist.GetCount(year, month, day);
					DrawHistBar(dc, cnt, max_count, x, y, CSize(daySpace, rect.Height()), rgb_bar, rgb_outline);

					CPoint label_pos(x + Pixels(dc, 1), Pixels(dc, y) + 2);

					// label for a year?
					if ((i == 0 && years_span == 1) || (drawYearsNames && month == 1 && day == 1))
					{
						CRect text_rect(label_pos, CSize(yearSpace, year_size.cy));
						label_right_pos = text_rect.left + DrawLabelText(dc, fontBold_, text_rect, year, DT_LEFT) + SPACE;
					}

					// label for a month?
					if ((i == 0 /*&& months_span == 1*/) || (drawMonthsNames && day == 1))
					{
						CRect text_rect(CPoint(std::max<int>(label_pos.x, label_right_pos), label_pos.y), CSize(monthSpace, month_size.cy));
						label_right_pos = text_rect.left + DrawLabelText(dc, fontBold_, text_rect, GetMonthName(month), DT_LEFT);
					}

					if (drawDays || (i == 0 && days_span == 1) || (cnt > 0 && label_pos.x >= label_right_pos))
					{
						CRect text_rect(label_pos, CSize(daySpace, day_size.cy));
						if (text_rect.left < label_right_pos)
						{
							text_rect.left = label_right_pos;
							if (text_rect.Width() < day_size.cx)
								continue;
						}
						CString str;
						str.Format(_T("%d"), day);
						const CSize size= ::GetTextExtent(dc, fontLabels_, str);
						if (size.cx <= daySpace)
							label_right_pos = text_rect.left + DrawLabelText(dc, fontLabels_, text_rect, day, DT_CENTER);
					}
				}
			}
		}
		precision = DAYS;
	}
	else if (drawMonthsTicks)	// draw months & years, but no days
	{
		int max_count= hist.MaxCountPerMonths();

		for (int year= from_date.Year(), i= 0; year <= to_date.Year(); ++year)
		{
			int from= year == from_date.Year() ? from_date.Month() : 1;
			int to= year == to_date.Year() ? to_date.Month() : 12;

			for (int month= from; month <= to; ++month, ++i)
			{
				int x= rect.left + i * rect.Width() / months_span;

				dc.FillSolidRect(x, y + 1, 1, month == 1 ? 2 * TICK_H : TICK_H, rgb_tick);

				{ // selection
					int right= rect.left + (i + 1) * rect.Width() / months_span;

					if (Date(year, month, 31) >= selection_from && Date(year, month, 1) <= selection_to)
						dc.FillSolidRect(x, rect.top, right - x, rect.Height(), rgb_selection);

					if (show_selection_ &&
						visible_range.from <= Date(year, month, 31) && visible_range.to >= Date(year, month, 1))
						dc.FillSolidRect(x, rect.top, right - x, SEL_HEIGHT, selected_range_color);
				}

				int cnt= hist.GetCount(year, month);
				DrawHistBar(dc, cnt, max_count, x, y, CSize(monthSpace, rect.Height()), rgb_bar, rgb_outline);

				CPoint label_pos(x + 1, y + 2);
				int month_text_width= monthSpace;
				bool year_label= (i == 0 && years_span == 1) || (drawYearsNames && month == 1);

				if (year_label)
				{
					CRect text_rect(label_pos, CSize(yearSpace, year_size.cy));
					int w= DrawLabelText(dc, fontBold_, text_rect, year, DT_LEFT) + SPACE;
					month_text_width -= w;
					label_pos.x += w;
				}
				if (drawMonthsNames || (i == 0 && months_span == 1))
				{
					CRect text_rect(label_pos, CSize(month_text_width, month_size.cy));
					if (!year_label || dc.GetTextExtent(GetMonthName(month)).cx <= text_rect.Width())
						DrawLabelText(dc, fontLabels_, text_rect, GetMonthName(month), DT_CENTER);
				}
			}
		}
		precision = MONTHS;
	}
	else if (drawYearsTicks)	// draw years only
	{
		int max_count= hist.MaxCountPerYears();
		int text_x= 0;

		for (int year= from_date.Year(), i= 0; year <= to_date.Year(); ++year, ++i)
		{
			int x= rect.left + i * rect.Width() / years_span;

			dc.FillSolidRect(x, y + 1, 1, TICK_H, rgb_tick);

			{ // selection
				int right= rect.left + (i + 1) * rect.Width() / years_span;

				if (Date(year, 12, 31) >= selection_from && Date(year, 1, 1) <= selection_to)
					dc.FillSolidRect(x, rect.top, right - x, rect.Height(), rgb_selection);

				if (show_selection_ &&
					visible_range.from <= Date(year, 12, 31) && visible_range.to >= Date(year, 1, 1))
					dc.FillSolidRect(x, rect.top, right - x, SEL_HEIGHT, selected_range_color);
			}

			int cnt= hist.GetCount(year);
			DrawHistBar(dc, cnt, max_count, x, y, CSize(yearSpace, rect.Height()), rgb_bar, rgb_outline);

			if ((drawYearsNames || i == 0 || cnt > 0) && x >= text_x)
			{
				CRect text_rect(CPoint(x + 1, y + 2), year_size); // CSize(yearSpace, year_size.cy));
				DrawLabelText(dc, fontBold_, text_rect, year, DT_LEFT);
				text_x = x + year_size.cx;
			}
		}
		precision = YEARS;
	}

	dc.SelectStockObject(SYSTEM_FONT);

	return precision;
}


void DistributionBar::Impl::DrawTick(CDC& dc, const CRect& hist_rect, DateTime tick)
{
	if (!area_valid_ || hist_area_.Width() < 1)
		return;

	int size= GetDefaultLineHeight() / 3;
	CSize tick_size((size - 1) * 2, size);
	if (hist_area_.Width() < tick_size.cx)
		return;

	Date date= GetYearMonth(tick);
//		(tick.GetYear(), tick.GetMonth(), tick.GetDay());

	std::pair<Date, Date> range= GetHistRange();
	Date beg= range.first;
	Date end= range.second;

	int span= DaysSpan(beg, end);

	if (date < beg || date > end || span < 1)
		return;

	double day= 24 * 60;	// minutes per day
	double hist_span= span * day;
	int days= DaysSpan(beg, date) - 1;
	double timestamp= days * day + tick.time_of_day().hours() * 60 + tick.time_of_day().minutes();
	int tick_position= static_cast<int>(timestamp * hist_area_.Width() / hist_span);

	CRect rect= hist_rect;

	if (tick_position - tick_size.cx / 2 < 0)
		tick_position += tick_size.cx / 2;
	else if (tick_position + tick_size.cx / 2 > rect.Width())
		tick_position -= tick_size.cx / 2;

	::DrawTick(dc, rect, tick_size, tick_position, true);
	::DrawTick(dc, rect, tick_size, tick_position, false);
}


std::pair<Date, Date> DistributionBar::Impl::GetHistRange()
{
	Date beg= hist_.StartingDate();
	Date end= hist_.EndingDate();

	if (precision_ == MONTHS)
	{
		beg = Date(beg.Year(), beg.Month(), 1);
		end = Date(end.Year(), end.Month(), DaysPerMonth(end.Year(), end.Month()));
	}
	else if (precision_ == YEARS)
	{
		beg = Date(beg.Year(), 1, 1);
		end = Date(end.Year(), 12, 31);
	}

	return std::make_pair(beg, end);
}


Date DistributionBar::Impl::HitTest(CPoint pos)
{
	if (!area_valid_ || pos.y < Pixels(TOP_DIST))
		return Date(0);

	if (pos.x >= hist_area_.left && pos.x < hist_area_.right && hist_area_.Width() > 0)
	{
		double x= pos.x - hist_area_.left;
		x /= hist_area_.Width();

		std::pair<Date, Date> span= GetHistRange();
		Date beg= span.first;
		Date end= span.second;
/*
		Date beg= hist_.StartingDate();
		Date end= hist_.EndingDate();

		if (precision_ == MONTHS)
		{
			beg = Date(beg.Year(), beg.Month(), 1);
			end = Date(end.Year(), end.Month(), DaysPerMonth(end.Year(), end.Month()));
		}
		else if (precision_ == YEARS)
		{
			beg = Date(beg.Year(), 1, 1);
			end = Date(end.Year(), 12, 31);
		}
*/
		int range= DaysSpan(beg, end);

		return AddDays(beg, static_cast<int>(x * range));
	}

	return Date(0);
}


void DistributionBar::Impl::InitSelection(Date date)
{
	if (precision_ == DAYS)
	{
		selection_range_.Reset(date);
	}
	else if (precision_ == MONTHS)
	{
		selection_range_.from = Date(date.Year(), date.Month(), 1);
		selection_range_.to = Date(date.Year(), date.Month(), DaysPerMonth(date.Year(), date.Month()));
	}
	else if (precision_ == YEARS)
	{
		selection_range_.from = Date(date.Year(), 1, 1);
		selection_range_.to = Date(date.Year(), 12, 31);
	}
	else
		selection_range_.Clear();
}


void DistributionBar::Impl::SetSelection(Date date)
{
	if (precision_ == DAYS)
		selection_range_.to = date;
	else if (precision_ == MONTHS)
	{
		if (date < selection_range_.from)
			selection_range_.to = Date(date.Year(), date.Month(), 1);
		else
			selection_range_.to = Date(date.Year(), date.Month(), DaysPerMonth(date.Year(), date.Month()));
	}
	else if (precision_ == YEARS)
	{
		if (date < selection_range_.from)
			selection_range_.to = Date(date.Year(), 1, 1);
		else
			selection_range_.to = Date(date.Year(), 12, 31);
	}
	else
		selection_range_.to = Date(0);
}


void DistributionBar::Impl::CancelSelecting()
{
	tracking_cursor_ = false;
	selection_range_.Clear();
}


void DistributionBar::Impl::AddHistory(DateRange filter, DateRange histogram)
{
	history_.Add(filter, histogram);
}


void DistributionBar::Impl::AddCurrent(DateRange filter, DateRange histogram)
{
	history_.SetCur(filter, histogram);
}


void DistributionBar::OnLButtonDown(UINT flags, CPoint pos)
{
	CWnd::OnLButtonDown(flags, pos);

	Date date= impl_->HitTest(pos);

	if (date.Index())
	{
		SetCapture();
		impl_->tracking_cursor_ = true;
		SetCursor();
	}

	impl_->InitSelection(date);

	Invalidate();
}


void DistributionBar::OnRButtonDown(UINT flags, CPoint pos)
{
	if (impl_->tracking_cursor_)
	{
		impl_->CancelSelecting();
		SetCursor();
		Invalidate();
	}

	CWnd::OnRButtonDown(flags, pos);
}


void DistributionBar::OnMouseMove(UINT flags, CPoint point)
{
	CWnd::OnMouseMove(flags, point);

	if (impl_->tracking_cursor_ && (flags & MK_LBUTTON) == 0)
	{
		impl_->CancelSelecting();
		SetCursor();
		return;
	}

	if (impl_->tracking_cursor_)
	{
		Date date= impl_->HitTest(point);

		if (date.Index())
		{
			impl_->SetSelection(date);
			Invalidate();
		}
	}

//	SendMouseNotification(point);
}


void DistributionBar::OnLButtonUp(UINT flags, CPoint point)
{
	CWnd::OnLButtonUp(flags, point);

	ReleaseCapture();

	bool notify= impl_->tracking_cursor_ && impl_->selection_range_.to.Index() && impl_->selection_range_.from.Index();

	impl_->tracking_cursor_ = false;

	SetCursor();

	Invalidate();

	if (notify)
	{
		try
		{
			impl_->selection_range_.Normalize();

			if (impl_->date_range_ != impl_->selection_range_ ||
				(impl_->show_selection_ && impl_->selection_range_ != impl_->selection_))
			{
				//Date from= impl_->selection_range_.from;
				//Date to= impl_->selection_range_.to;

				//ASSERT(from <= to);

				// is there anything there?
				if (impl_->hist_.IsRangeNonEmpty(impl_->selection_range_.from, impl_->selection_range_.to))
				{
					// current histogram range
					DateRange histo_range= impl_->hist_.Range();

					// narrow down?
					if (impl_->DrillDown(impl_->selection_range_))
						histo_range = impl_->selection_range_;

					DateRange visible_range= impl_->hist_.VisibleRange();
					impl_->AddHistory(visible_range/*impl_->date_range_*/, impl_->hist_.Range());
					impl_->Filter(impl_->selection_range_, histo_range);
				}
				else
				{
					// beep: no images in the selected range
					::MessageBeep(MB_ICONASTERISK);
					impl_->CancelSelecting();
				}
			}
			else
				impl_->CancelSelecting();
		}
		CATCH_ALL
	}
}


BOOL DistributionBar::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		SetCursor();
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}


void DistributionBar::SetCursor()
{
	//CPoint point;
	//GetCursorPos(&point);
	//ScreenToClient(&point);

	if (impl_->tracking_cursor_)
		::SetCursor(AfxGetApp()->LoadCursor(IDC_POINTING));
//	else if (histogram_.GetHistogramRect().PtInRect(point))
//		::SetCursor(AfxGetApp()->LoadCursor(IDC_HORZ_LINE_RESIZE));
	else
		::SetCursor(::LoadCursor(0, IDC_ARROW));
}


void DistributionBar::ClearHistory()
{
	impl_->history_.Clear();
}


void DistributionBar::ClearSelection()
{
	impl_->CancelSelecting();
	Invalidate();
}


void DistributionBar::Impl::Filter(DateRange filter, DateRange histogram)
{
	if (filter_callback_)
	{
		filter.Normalize();
		histogram.Normalize();

		Date ff= filter.from;
		Date ft= filter.to;

		Date hf= histogram.from;
		Date ht= histogram.to;

		filter_callback_(
			ToDateTime(ff.Year(), ff.Month(), ff.Day(), 0, 0, 0), ToDateTime(ft.Year(), ft.Month(), ft.Day(), 23, 59, 59),
			ToDateTime(hf.Year(), hf.Month(), hf.Day(), 0, 0, 0), ToDateTime(ht.Year(), ht.Month(), ht.Day(), 23, 59, 59)
			);
	}
}


void DistributionBar::OnBack()
{
	if (impl_->history_.HasPrevious())
	{
		try
		{
			std::pair<DateRange, DateRange> ranges= impl_->history_.Back();
			impl_->Filter(ranges.first, ranges.second);
		}
		CATCH_ALL
	}
}


void DistributionBar::OnNext()
{
	if (impl_->history_.HasNext())
	{
		try
		{
			std::pair<DateRange, DateRange> ranges= impl_->history_.Next();
			impl_->Filter(ranges.first, ranges.second);
		}
		CATCH_ALL
	}
}


void DistributionBar::OnUpdateBack(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(impl_->history_.HasPrevious());
}

void DistributionBar::OnUpdateNext(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(impl_->history_.HasNext());
}


void DistributionBar::OnCancelFilter()
{
	if (impl_->cancel_callback_)
		impl_->cancel_callback_();
}

void DistributionBar::OnUpdateCancelFilter(CCmdUI* cmd_ui)
{
	if (impl_->is_filter_active_)
		cmd_ui->Enable(impl_->is_filter_active_());
}


BOOL DistributionBar::IsFrameWnd() const
{
	return true;	// intercept toolbar's update cmd
}


void DistributionBar::SetTick(DateTime time)
{
	impl_->tick_ = time;

	if (m_hWnd)
		Invalidate();
}
