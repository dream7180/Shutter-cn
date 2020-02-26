/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DataExchange/h

#pragma once
#include <boost/ptr_container/ptr_vector.hpp>

// Simplistic data exchange mechanism for copying items between dialog and external data
// Proper data exchange is handled by MFC (DDX_*), and this vector only maps those dialog
// specific C++ variables to the data structures in the app, and exchanges data between them

namespace dx_detail
{

struct DataExchangeItem
{
	virtual bool DialogToData() = 0;
};


template<typename T1, typename T2>
class DataExchangeImpl : public DataExchangeItem
{
public:
	DataExchangeImpl(T1& dialog, T2& data) : data_(data), dialog_(dialog)
	{
		dialog_ = data_;
	}

	virtual ~DataExchangeImpl()
	{}

	virtual bool DialogToData()
	{
		bool changed= dialog_ != data_;
		if (changed)
			data_ = dialog_;
		return changed;
	}

private:
	T1& dialog_;
	T2& data_;
};

template<typename T>
class DataExchangeImplFn : public DataExchangeItem
{
public:
	DataExchangeImplFn(const boost::function<T (void)>& get_dlg_data, const boost::function<void (T)>& set_dlg_data, T& data)
		: get_dlg_data_(get_dlg_data), data_(data)
	{
		set_dlg_data(data_);
	}

	virtual ~DataExchangeImplFn()
	{}

	virtual bool DialogToData()
	{
		T dlg= get_dlg_data_()
		bool changed= dlg != data_;
		if (changed)
			data_ = dlg;
		return changed;
	}

private:
	boost::function<const T& (void)> get_dlg_data_;
	T& data_;
};


template<>
class DataExchangeImpl<BOOL, bool> : public DataExchangeItem
{
public:
	DataExchangeImpl(BOOL& dialog, bool& data) : data_(data), dialog_(dialog)
	{
		dialog_ = data_;
	}

	virtual ~DataExchangeImpl()
	{}

	virtual bool DialogToData()
	{
		bool dlg= !!dialog_;
		bool changed= dlg != data_;
		if (changed)
			data_ = dlg;
		return changed;
	}

private:
	BOOL& dialog_;
	bool& data_;
};

}	// namespace


class DataExchange
{
public:
	DataExchange()
	{}

	// add new pair to the exchange vector; data is held by reference,
	// so it's assumed not to disappear during the lifetime of this DataExchange object
	template<typename T1, typename T2>
	void Add(T1& dlg, T2& data)	{ dx_.push_back(new dx_detail::DataExchangeImpl<T1, T2>(dlg, data)); }

	//template<typename T>
	//void Add(const boost::function<T (void)>& get_dlg_data, const boost::function<void (T)>& set_dlg_data, T& data)
	//{ dx_.push_back(new dx_detail::DataExchangeImplFn<T>(get_dlg_data, set_dlg_data, data)); }

	// read all items from dialog and put them in the place where they came from;
	// returns true if something has been changed
	bool Read()
	{
		const size_t size= dx_.size();
		bool changed= false;
		for (size_t i= 0; i < size; ++i)
			if (dx_[i].DialogToData())
				changed = true;
		return changed;
	}

private:
	boost::ptr_vector<dx_detail::DataExchangeItem> dx_;
};
