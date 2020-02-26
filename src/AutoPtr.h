/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
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
	operator void* () const				{ return ptr_; }

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

	AutoPtr<T>& operator = (T* p)
	{
		if (p != ptr_)
		{
			free();
			ptr_ = p;
		}
		return *this;
	}

	void swap(AutoPtr<T>& src)
	{
		std::swap(ptr_, src.ptr_);
	}

private:
	mutable T* ptr_;
};


#endif	// _auto_pointer_h_
