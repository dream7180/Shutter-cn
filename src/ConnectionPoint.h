/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ConnectionPoint.h: interface for the ConnectionPointer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTIONPOINT_H__2321C9F6_CD20_4060_9F46_28680D14F0BB__INCLUDED_)
#define AFX_CONNECTIONPOINT_H__2321C9F6_CD20_4060_9F46_28680D14F0BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// connection pointer: set explicitly, clear on destruction of pointed object
//

class ConnectionPointer
{
public:
	ConnectionPointer();
	ConnectionPointer(ConnectionPointer* link_to);
	virtual ~ConnectionPointer();

	virtual void Disconnect();

	void Connect(ConnectionPointer* link)	{ link_ = link; }

private:
	ConnectionPointer* link_;
};


// connection: connection to some data controlled by different object via pointer
//

template<class T>
class CConnection : public ConnectionPointer
{
public:
	CConnection() : data_(0) {}
//	CConnection(T* data) : data_(data) {}
	CConnection(ConnectionPointer* link) : ConnectionPointer(link), data_(0) {}
	virtual ~CConnection() {}

//	void Connect(ConnectionPointer* link)	{ ConnectionPointer::Connect(link); }
	template<class LNK>
		void Connect(LNK* data)			{ data_ = data; ConnectionPointer::Connect(data); }

	//bool operator ! () const				{ return data_ == 0; }
	bool IsValid() const					{ return data_ != 0; }
	//operator bool () const					{ return data_ != 0; }

	T* get()								{ return data_; }

	T* operator -> ()						{ return data_; }

	const T* operator -> () const			{ return data_; }

	virtual void Disconnect()				{ ConnectionPointer::Disconnect(); data_ = 0; }

private:
	T* data_;
};


template<class T>
class WndConnection : public CConnection<T>
{
public:
	WndConnection() {}
	virtual ~WndConnection()
	{
		if (IsValid())
			get()->DestroyWindow();
	}

private:
//	virtual void Disconnect()				{ if (IsValid()) get()->DestroyWindow(); CConnection<CWnd>::Disconnect(); }
};


#endif // !defined(AFX_CONNECTIONPOINT_H__2321C9F6_CD20_4060_9F46_28680D14F0BB__INCLUDED_)
