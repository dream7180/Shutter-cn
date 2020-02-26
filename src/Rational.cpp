/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Rational.cpp: implementation of the CRational class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rational.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

uint32 FindGCD(uint32 m, uint32 n)
{
	if (m == 0 || n == 0)
		return 0;

	for (int i= 0; i < 100; ++i)
	{
		uint32 r= m % n;

		if (r == 0)
			return n;

		m = n;
		n = r;
	}

	ASSERT(false);
	return 0;
}


String Rational::AsString(bool dec_rational/*= true*/) const
{
	oStringstream ost;
	if (dec_rational && denominator_ == 1)
		ost << numerator_ << _T(".0");
	else if (dec_rational && denominator_ == 10)
		ost << numerator_ / denominator_ << _T(".") << numerator_ % denominator_;
	else if (dec_rational && denominator_ == 100)
	{
		ost << numerator_ / denominator_ << _T(".");
		ost.width(2);
		ost.fill(_T('0'));
		ost << numerator_ % denominator_;
	}
	else if (denominator_ == 0)
		ost << _T("-");
	else if (numerator_ == 0)
		ost << _T("0");
	else if (numerator_ == denominator_)
		ost << _T("1");
	else
	{
		uint32 div= FindGCD(numerator_, denominator_);
		if (div > 1)
			ost << numerator_ / div << _T("/") << denominator_ / div;
		else
			ost << numerator_ << _T("/") << denominator_;
	}
	return ost.str();
}


String Rational::UnitNumerator(bool round/*= false*/) const
{
	oStringstream ost;
	if (denominator_ == 0)
		ost << _T("-");
	else if (denominator_ > numerator_)	// fractional value?
	{
		int dec= 0;
//		uint32 div= 0;
		switch (numerator_)
		{
		case 10000000:
			++dec;
		case 1000000:
			++dec;
		case 100000:
			++dec;
		case 10000:
			++dec;
		case 1000:
			++dec;
		case 100:
			++dec;
		case 10:
			++dec;
		case 1:
			if (round)
			{
				ost << _T("1/");
				double div= double(denominator_) / double(numerator_);
				ost << uint32(div);
				if (div < 10.0)
				{
					int digit= int(div * 10.0);
					digit %= 10;
					if (digit != 0)
						ost << _T(".") << digit;
				}
			}
			else
			{
				uint32 div= denominator_ / numerator_;
				ost << _T("1/") << div;
				if (dec)
				{
					if (uint32 mod= denominator_ % numerator_)
					{
						ost << _T(".");
						ost.fill(_T('0'));
						ost.width(dec);
						ost << denominator_ % numerator_;
					}
				}
			}
			break;
		default:
			{
				double val= double(denominator_) / numerator_;
				if (val >= 10.0)
					ost << _T("1/") << int(val + 0.5);
				else
				{
					double integer;
					double fraction= modf(val + 0x0001, &integer);
					if (fraction <= 0.0001)
					{
						ost.precision(3);
						ost << _T("1/") << val;
					}
					else
					{
						//ost << numerator_ << _T("/") << denominator_;
						if (denominator_ == 10)
							ost.precision(1);
						else if (denominator_ == 100)
							ost.precision(2);
						else
							ost.precision(3);
						ost << std::fixed << Double();
					}
				}
			}
//			ost << numerator_ << "/" << denominator_;
			break;
		}
	}
	else
	{
		int dec= 0;
		switch (denominator_)
		{
		case 10000000:
			++dec;
		case 1000000:
			++dec;
		case 100000:
			++dec;
		case 10000:
			++dec;
		case 1000:
			++dec;
		case 100:
			++dec;
		case 10:
			++dec;
		case 1:
			if (dec == 0)
				ost << numerator_;
			else
			{
				ost << numerator_ / denominator_;
				uint32 mod= numerator_ % denominator_;
				{
					ost << _T(".");
					ost.fill(_T('0'));
					ost.width(mod == 0 ? 1 : dec);
					ost << mod;
				}
			}
			break;
		default:
//			ost << numerator_ << _T("/") << denominator_;
			ost << std::fixed << std::setprecision(1) << Double();
			break;
		}
	}

	return ost.str();
}


String SRational::String(bool dec_rational/*= true*/, bool show_plus_sign/*= false*/) const
{
	int32 num= numerator_;
	int32 denom= denominator_;
	if (num < 0 && denom < 0)
		num = -num, denom = -denom;
	else if (denom < 0)
		num = -num, denom = -denom;

	oStringstream ost;

	if (show_plus_sign && num > 0)
		ost << _T("+");

	if (dec_rational && denom == 1)
		ost << num; // << _T(".0");
	else if (dec_rational && denom == 10)
	{
		int32 s= num / 10;
		if (s == 0 && num < 0)
			ost << _T("-");
		ost << s << _T(".") << abs(num) % 10;
	}
	else if (dec_rational && denom == 100)
	{
		int32 s= num / 100;
		if (s == 0 && num < 0)
			ost << _T("-");
		ost << s << _T(".");
		ost.width(2);
		ost.fill(_T('0'));
		ost << abs(num) % 100;
	}
	else if (denom == 0)
		ost << _T("-");
	else if (num == 0)
		ost << _T("0");
	else if (num == denom)
		ost << _T("1");
	else if (num == -denom)
		ost << _T("-1");
	else
	{
		int32 div= static_cast<int32>(FindGCD(abs(num), abs(denom)));
		if (div > 1)
			return SRational(num / div, denom / div).String(dec_rational, show_plus_sign);

		ost << num << _T("/") << denom;
	}
	return ost.str();
}


String Rational::AsDegMinSec(const Rational val[3])
{
	oStringstream ost;
	ost << std::setfill(_T('0'));
//TODO: fixed rounds up printed numbers; either disable it (preferable) or make sure 60 propagates to minutes
	if (val[0].denominator_ == 1 && val[1].denominator_ == 1)
	{
		ost << val[0].numerator_ << _T("° ");
		ost << std::setw(2) << val[1].numerator_ << _T("' ");
		ost << std::fixed << std::setprecision(1);	// decimal places
		ost << std::setw(4) << val[2].Double() << _T("\"");
	}
	else
	{
		ost << val[0].numerator_ << _T("° ");
		ost << std::fixed << std::setprecision(2);	// two decimal places
		ost << std::setw(5) << val[1].Double() << _T("'");
	}

	return ost.str();
}
