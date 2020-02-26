/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "RecentStrings.h"
extern String TrimSpaces(const String& str);


class EditHistory // : public IUnknown //: public IEnumString
{
public:
	EditHistory(const String& reg_section) : reg_section_(reg_section)
	{
		position_ = 0;
		ref_count_ = 1;	// ref counter value tuned to ensure all objects will be deleted

		::RecentStrings(history_, MAX_COUNT, false, reg_section_.c_str());
	}

	~EditHistory()
	{
	}

	void SaveHistory()
	{
		::RecentStrings(history_, MAX_COUNT, true, reg_section_.c_str());
	}

	void AddString(const TCHAR* text)
	{
		if (text && *text)
		{
			String str= text;
			TrimSpaces(str);
			if (!str.empty())
				AddRecentString(history_, str.c_str());
		}
	}

	// history limit in entries (it may be too small for keywords for instance)
	enum { MAX_COUNT= 100 };
#if 0
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ LPOLESTR* rgelt,
            /* [out] */ ULONG* celt_fetched)
	{
		if (rgelt == 0)
			return E_FAIL;

		size_t count= 0;

		for (size_t i= 0; i < celt; ++i)
		{
			if (position_ >= history_.size())
				break;

			const String& str= history_[position_++];

			rgelt[i] = (LPOLESTR)::CoTaskMemAlloc((str.length() + 1) * sizeof(wchar_t));

			if (rgelt[i] == 0)
				return E_OUTOFMEMORY;

			_tcscpy(rgelt[i], str.c_str());

			count++;
		}

		if (celt_fetched != 0)
			*celt_fetched = count;

		return count == celt ? S_OK : S_FALSE;
	}
        
	virtual HRESULT STDMETHODCALLTYPE Skip( 
		/* [in] */ ULONG celt)
	{
		position_ += celt;
		return position_ < history_.size() ? S_OK : S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE Reset()
	{
		position_ = 0;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Clone(
		/* [out] */ IEnumString** enum)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		if (riid == __uuidof(IEnumString))
		{
			*ppvObject = this;
			return S_OK;
		}
		else if (riid == __uuidof(IUnknown))
		{
			*ppvObject = static_cast<IUnknown*>(this);
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++ref_count_;
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		// local object; don't delete
		if (--ref_count_ == 0)
		{
			delete this;
			return 0;
		}

		return ref_count_;
	}
#endif
	const std::vector<String>* Array() const
	{
		return &history_;
	}

private:
	size_t position_;
	size_t ref_count_;
	std::vector<String> history_;
	String reg_section_;
};

//_COM_SMARTPTR_TYPEDEF(IAutoComplete, __uuidof(IAutoComplete));
//_COM_SMARTPTR_TYPEDEF(IAutoComplete2, __uuidof(IAutoComplete2));
//_COM_SMARTPTR_TYPEDEF(IEnumString, __uuidof(IEnumString));

