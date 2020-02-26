/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Profile.h: definition and implementation of the CProfileBase class template.
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROFILEBASE_H__4F2E685D_A779_492B_880D_93278D628F36__INCLUDED_)
#define AFX_PROFILEBASE_H__4F2E685D_A779_492B_880D_93278D628F36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
extern bool IsRegKeyPresent(const TCHAR* section, const TCHAR* key);


// Profile class is meant as a wrapper around Write/GetProfile(Type) functions

template <class T> class Profile
{
public:
	Profile() : value_(T()), default_(T())
	{
		section_ = 0;
		key_ = 0;
		up_to_date_ = false;
	}

	Profile(const TCHAR* section, const TCHAR* key, const T& tDefault) : value_(tDefault), default_(tDefault)
	{
		section_ = section;
		key_ = key;
		up_to_date_ = false;
	}

	// set section and key names (danger: they are only stored as pointers to save space)
	void Register(const TCHAR* section, const TCHAR* key, const T& tDefault)
	{
		default_ = value_ = tDefault;
		section_ = section;
		key_ = key;
	}

	T& operator = (const T& value)
	{
		value_ = value;
		Store();
		up_to_date_ = true;
		return value_;
	}

	operator T ()
	{
		return Value();
	}

	T Value()
	{
		if (!up_to_date_)
		{
			Restore();
			up_to_date_ = true;
		}
		return value_;
	}

	T Value(size_t index)
	{
		if (section_ && key_)
		{
			CString key;
			key.Format(_T("%s-%d"), key_, int(index));
			T val;
			RestoreFromRegistry(section_, key, val, default_);
			return val;
		}
		return default_;
	}

	void Store(size_t index, T val)
	{
		if (section_ && key_)
		{
			CString key;
			key.Format(_T("%s-%d"), key_, int(index));
			StoreInRegistry(section_, key, val);
		}
	}

	bool IsRegEntryPresent()
	{
		return IsRegKeyPresent(section_, key_);
	}

protected:
	const TCHAR* section_;
	const TCHAR* key_;
	bool up_to_date_;
	T value_;
	T default_;

	void Store() const
	{
		if (section_ && key_)
			StoreInRegistry(section_, key_, value_);
	}

	void Restore()
	{
		if (section_ && key_)
			RestoreFromRegistry(section_, key_, value_, default_);
	}

	// those two functions have to be specialized for particular type(s)
	static void StoreInRegistry(const TCHAR* section, const TCHAR* key, const T& tValue);
	static void RestoreFromRegistry(const TCHAR* section, const TCHAR* key, T& tValue, const T& tDefault);
};


//NOTE: Profile<TCHAR*> would have to be defined separately if it was to be useful (because of value_ storage)

template<> class Profile<TCHAR*>
{};

#endif // !defined(AFX_PROFILEBASE_H__4F2E685D_A779_492B_880D_93278D628F36__INCLUDED_)
