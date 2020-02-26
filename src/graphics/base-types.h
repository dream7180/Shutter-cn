/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

namespace gr {

namespace types {

template<class T>
struct base_type_traits
{
	static const T Default= T();
};


template<class T>
struct Size
{
public:
	Size()									{ cx = cy = T(); }
	Size(T w, T h)							{ cx = w; cy = h; }
	Size(const Size<T>& size)				{ cx = size.cx; cy = size.cy; }
//	Size(Point<T> pt)							{ cx = pt.x; cx = pt.y; }

	void SetSize(T w, T h)					{ cx = w; cy = h; }

	bool operator == (Size<T> sz) const		{ return cx == sz.cx && cy == sz.cy; }
	bool operator != (Size<T> sz) const		{ return cx != sz.cx || cy != sz.cy; }

	Size<T>& operator += (Size<T> s)		{ cx += s.cx; cy += s.cy; return *this; }
	Size<T> operator + (Size<T> s) const	{ Size tmp(*this); return tmp += s; }

	Size<T>& operator -= (Size<T> s)		{ cx -= s.cx; cy -= s.cy; return *this; }
	Size<T> operator - (Size<T> s) const	{ Size tmp(*this); return tmp -= s; }

	T cx;
	T cy;
};


//inline Size CSizeFromDWord(DWORD dw)
//{
//	return Size(static_cast<short>(LOWORD(dw)), static_cast<short>(HIWORD(dw)));
//}


template<class T>
struct Point
{
	Point()									{ x = y = T(); }
	Point(T x, T y)							{ this->x = x; this->y = y; }
	Point(Size<T> sz)						{ x = sz.cx; y = sz.cy; }
	Point(const Point<T>& pt)				{ x = pt.x ; y = pt.y; }

	void Offset(T dx, T dy)					{ x += dx; y += dy; }
	void SetPoint(T x, T y)					{ this->x = x; this->y = y; }

	BOOL operator == (Point<T> pt) const	{ return ((x == pt.x) && (y == pt.y)); }
	BOOL operator != (Point<T> pt) const	{ return ((x != pt.x) || (y != pt.y)); }

	Point& operator += (Size<T> s)			{ x += s.cx; y += s.cy; return *this; }
	Point operator + (Size<T> s) const		{ Point<T> tmp(*this); return tmp += s; }

	Point& operator -= (Size<T> s)			{ x -= s.cx; y -= s.cy; return *this; }
	Point operator - (Size<T> s) const		{ Point<T> tmp(*this); return tmp -= s; }

	Point& operator += (Point<T> p)			{ x += p.x; y += p.y; return *this; }
	Point operator + (Point<T> p) const		{ Point<T> tmp(*this); return tmp += p; }

	Point& operator -= (Point<T> p)			{ x -= p.x; y -= p.y; return *this; }
	Point operator - (Point<T> p) const		{ Point<T> tmp(*this); return tmp -= p; }

	Size<T> ToSize() const					{ return Size<T>(x, y); }

	T x;
	T y;
};

template<class T>
inline Point<T> operator - (Point<T> p)			{ return Point<T>(-p.x, -p.y); }


//inline Point PointFromLParam(LPARAM pt)
//{
//	return Point(static_cast<short>(LOWORD(pt)), static_cast<short>(HIWORD(pt)));
//}


template<class T>
class Rect
{
	typedef Size<T> SizeT;
	typedef Point<T> PointT;
public:
	Rect()											{ left = top = right = bottom = T(); }
	Rect(T l, T t, T r, T b)						{ left = l; top = t; right = r; bottom = b; }
	Rect(const Rect<T>& rc)							{ left = rc.left; top = rc.top; right = rc.right; bottom = rc.bottom; }
	Rect(PointT pt, SizeT sz)						{ SetRect(pt, sz); }
	Rect(PointT top_left, PointT bottom_right)		{ SetRect(top_left, bottom_right); }

//	operator Rect<T>* ()							{ return this; }
//	operator const Rect<T>* () const				{ return this; }

	bool operator == (const Rect<T>& rc) const		{ return left == rc.left && right == rc.right && top == rc.top && bottom == rc.bottom; }
	bool operator != (const Rect<T>& rc) const		{ return !(*this == rc); }

	bool EqualRect(const Rect<T>* rect) const		{ return *this == *rect; }

	//Rect& operator = (const Rect<T>& src)	default;		{ left = src.left; top = src.top; right = src.right; bottom = src.bottom; return *this; }
	void CopyRect(const Rect<T>* rect)				{ *this = *rect; }

	T Height() const								{ return bottom - top; }
	T Width() const									{ return right - left; }
	SizeT Size() const								{ return SizeT(Width(), Height()); }

	void InflateRect(T l, T t, T r, T b)			{ left += l; right += r; top += t; bottom += b; }
	void InflateRect(T dx, T dy)					{ InflateRect(dx, dy, dx, dy); }
	void InflateRect(SizeT s)						{ InflateRect(s.cx, s.cy); }
	void InflateRect(const Rect<T>* r)				{ InflateRect(r->left, r->top, r->right, r->bottom); }

	void DeflateRect(T l, T t, T r, T b)			{ left -= l; right -= r; top -= t; bottom -= b; }
	void DeflateRect(T dx, T dy)					{ DeflateRect(dx, dy); }
	void DeflateRect(SizeT s)						{ DeflateRect(s.cx, s.cy); }
	void DeflateRect(const Rect<T>* r)				{ DeflateRect(r->left, r->top, r->right, r->bottom); }

	bool IsRectEmpty() const						{ return left == right || top == bottom;}
	bool IsRectNull() const							{ return left == T() && right == T() && top == T() && bottom == T(); }

	void OffsetRect(T dx, T dy)						{ left += dx; right += dx; top += dy; bottom += dy; }
	void OffsetRect(SizeT s)						{ OffsetRect(s.cx, s.cy); }
	void OffsetRect(PointT p)						{ OffsetRect(p.x, p.y); }

	bool PtInRect(PointT pt) const					{ return ::PtInRect(this, pt) != 0; }

	Rect& NormalizeRect()
	{
		if (left > right)	std::swap(left, right);
		if (top > bottom)	std::swap(top, bottom);
		return *this;
	}

	Rect& SetRect(T left, T top, T right, T bottom)
	{ this->left = left; this->top = top; this->right = right; this->bottom = bottom; return *this; }
	Rect& SetRect(PointT pt, SizeT sz)
	{ left = pt.x, right = left + sz.cx; top = pt.y; bottom = top + sz.cy; return *this; }
	Rect& SetRect(PointT top_left, PointT bottom_right)
	{ left = top_left.x; top = top_left.y; right = bottom_right.x; bottom = bottom_right.y; return *this; }

	Rect& SetRectEmpty()						{ return SetRect(T(), T(), T(), T()); }

//	bool SubtractRect(const Rect<T>* rect1, const Rect<T>* rect2)
//	{ return ::SubtractRect(this, rect1, rect2) != 0; }

	// sets *this to the union of rect1 and rect2
	void UnionRect(const Rect<T>* rect1, const Rect<T>* rect2)
	{
		left = std::min(rect1.left, rect2.left);
		right = std::max(rect1.right, rect2.right);
		top = std::min(rect1.top, rect2.top);
		bottom = std::max(rect1.bottom, rect2.bottom);
	}

	// sets *this to the intersection of rect1 and rect2
	void IntersectRect(const Rect<T>* rect1, const Rect<T>* rect2)
	{
		IntersectRange(rect1.left, rect1.right, rect2.left, rect2.right, &left, &right);
		IntersectRange(rect1.top, rect1.bottom, rect2.top, rect2.bottom, &top, &bottom);
	}

	Rect& operator |= (const Rect<T>& rect)		{ UnionRect(this, &rect); return *this; }
	Rect operator | (const Rect<T>& rect) const	{ Rect tmp(*this); return tmp |= rect; }

	Rect& operator &= (const Rect<T>& rect)		{ IntersectRect(this, &rect); return *this; }
	Rect operator & (const Rect<T>& rect) const	{ Rect tmp(*this); return tmp &= rect; }

	PointT CenterPoint() const					{ return PointT((left + right) / 2, (top + bottom) / 2); }

	// no references returned
	PointT TopLeft() const						{ return PointT(left, top); }
	PointT TopRight() const						{ return PointT(right, top); }
	PointT BottomLeft() const					{ return PointT(left, bottom); }
	PointT BottomRight() const					{ return PointT(right, bottom); }

	// MoveToX/Y

private:
	T left;
	T top;
	T right;
	T bottom;

	static void IntersectRange(T from1, T to1, T from2, T to2, T* from, T* to)
	{
		if (from1 >= from2 && from1 <= to2)
			*from = from2, *to = std::min(to1, to2);
		else (from2 >= from1 && from2 <= to1)
			*from = from1, *to = std::min(to1, to2);
		else
			*from = *to = T();
	}
};


// scale rect
template<class T>
inline Rect<T> RectMulDiv(const Rect<T>& rect, T multiplier, T divisor)
{
	return Rect(
		::MulDiv(rect.left, multiplier, divisor),
		::MulDiv(rect.top, multiplier, divisor),
		::MulDiv(rect.right, multiplier, divisor),
		::MulDiv(rect.bottom, multiplier, divisor)
		);
}


} // namespace

} // namespace
