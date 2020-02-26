/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000 Michal Kowalski

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
____________________________________________________________________________*/

#ifndef _auto_pointer_h_
#define _auto_pointer_h_


template <class T>
class AutoPtr
{
public:
	AutoPtr(T* p) : ptr_(p) {}
	AutoPtr() : ptr_(0) {}
	AutoPtr(const AutoPtr<T>& p) : ptr_(0) { *this = p; }
	~AutoPtr()
	{ if (ptr_) delete ptr_; }

	T* get() const throw ()				{ return ptr_; }
	T& operator * () const throw ()		{ return *get(); }
	T* operator -> () const throw ()	{ return get(); }
	T* release() throw ()				{ T* p= ptr_; ptr_ = 0; return p; }
	void free()							{ if (ptr_) delete ptr_; ptr_ = 0; }

	bool empty() const					{ return ptr_ == 0; }
	bool not_empty() const				{ return ptr_ != 0; }

	AutoPtr<T>& operator = (const AutoPtr<T>& p)
	{
		if (&p != this)
		{
			free();
			ptr_ = p.ptr_;
			p.ptr_ = 0;
		}
		return *this;
	}

private:
	mutable T* ptr_;
};


#endif	// _auto_pointer_h_
