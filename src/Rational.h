/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Rational.h: interface for the Rational class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RATIONAL_H__382D86A8_9BA5_4BB6_AE1D_487857128769__INCLUDED_)
#define AFX_RATIONAL_H__382D86A8_9BA5_4BB6_AE1D_487857128769__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct Rational
{
	Rational(uint32 numerator, uint32 denominator) : numerator_(numerator), denominator_(denominator)
	{}
	Rational()
	{ numerator_ = 0; denominator_ = 0; }

	bool operator > (const Rational& a) const	{ return Double() > a.Double(); }
	bool operator < (const Rational& a) const	{ return Double() < a.Double(); }
	bool operator == (const Rational& a) const	{ return numerator_ == a.numerator_ && denominator_ == a.denominator_; }

	uint32 numerator_;
	uint32 denominator_;

	String AsString(bool dec_rational= true) const;
	String UnitNumerator(bool round= false) const;

	double Double() const		{ return denominator_ ? double(numerator_) / double(denominator_) : 0.0; }
	uint32 Integer() const		{ return denominator_ ? numerator_ / denominator_ : 0; }
	uint32 FractPart() const	{ return denominator_ ? numerator_ % denominator_ : 0; }

	bool Valid() const			{ return denominator_ != 0; }

	// rational numbers representing degrees, minutes & seconds
	static String AsDegMinSec(const Rational val[3]);
};


struct SRational
{
	SRational(int32 numerator, int32 denominator) : numerator_(numerator), denominator_(denominator)
	{}
	SRational()
	{ numerator_ = denominator_ = 0; }
	SRational(const Rational& src) : numerator_(src.numerator_), denominator_(src.denominator_)
	{}

	bool operator > (const SRational& a) const	{ return Double() > a.Double(); }
	bool operator < (const SRational& a) const	{ return Double() < a.Double(); }
	bool operator == (const SRational& a) const	{ return numerator_ == a.numerator_ && denominator_ == a.denominator_; }

	int32 numerator_;
	int32 denominator_;

	String String(bool dec_rational= true, bool show_plus_sign= false) const;
	bool Valid() const		{ return denominator_ != 0; }
	double Double() const	{ return denominator_ ? double(numerator_) / double(denominator_) : 0.0; }
};


#endif // !defined(AFX_RATIONAL_H__382D86A8_9BA5_4BB6_AE1D_487857128769__INCLUDED_)
