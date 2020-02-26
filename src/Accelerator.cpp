/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Accelerator.cpp: implementation of the Accelerator class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Accelerator.h"
#include <boost/iterator/transform_iterator.hpp>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace boost;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Accelerator::Accelerator()
{
	accel_ = NULL;
	loaded_ = false;
}


Accelerator::~Accelerator()
{
	Destroy();
}


bool Accelerator::Create(ACCEL* accel_table, int entries)
{
	accel_ = ::CreateAcceleratorTable(accel_table, entries);
	loaded_ = false;
	return accel_ != NULL;
}


bool Accelerator::Load(HINSTANCE instance, LPCTSTR table_name)
{
	accel_ = ::LoadAccelerators(instance, table_name);
	loaded_ = true;
	return accel_ != NULL;
}


bool Accelerator::Load(int table_id)
{
	return Load(AfxGetResourceHandle(), MAKEINTRESOURCE(table_id));
}


namespace {
	struct CmpAccel
	{
		bool operator () (const ACCEL& a1, const ACCEL& a2) const
		{
			return a1.cmd < a2.cmd;
		}

		//bool operator () (const ACCEL& a1, WORD cmd) const
		//{
		//	return a1.cmd < cmd;
		//}
	};

	struct GetCmd : public std::unary_function<const ACCEL&, DWORD>
	{
		DWORD operator () (const ACCEL& a) const
		{
			return a.cmd;
		}
	};

	typedef boost::transform_iterator<GetCmd, std::vector<ACCEL>::iterator> cmd_iterator;
}


ACCEL* Accelerator::BinarySearch(std::vector<ACCEL>& accel, WORD cmd)
{
	cmd_iterator end= make_transform_iterator(accel.end(), GetCmd());

	cmd_iterator it= std::lower_bound(
		make_transform_iterator(accel.begin(), GetCmd()),
		end,
		cmd, std::less<DWORD>()
		);

//	vector<ACCEL>::iterator it= lower_bound(accel.begin(), accel.end(), cmd, CmpAccel());

	if (it == end || *it != cmd)	// not quite correct according to Scott Mayers
		return 0;

	return &*(it.base());
}


void Accelerator::GetSortedData(std::vector<ACCEL>& accel) const
{
	GetData(accel_, accel);
	sort(accel.begin(), accel.end(), CmpAccel());
}


void Accelerator::GetData(std::vector<ACCEL>& accel) const
{
	GetData(accel_, accel);
}


void Accelerator::GetData(HACCEL accel, std::vector<ACCEL>& out)
{
	ASSERT(accel);
	int size= ::CopyAcceleratorTable(accel, NULL, 0);
	out.resize(size + 1);
	ACCEL* first= &out.front();
	::CopyAcceleratorTable(accel, first, size);
	memset(&out.back(), 0, sizeof(out.back()));	// clear last ACCEL entry
}


void Accelerator::Attach(HACCEL accel, bool loaded/*= true*/)
{
	ASSERT(accel);
	if (accel_)
		Destroy();
	ASSERT(accel_ == NULL);
	accel_ = accel;
	loaded_ = loaded;
}


void Accelerator::Destroy()
{
	if (accel_ && !loaded_)
		::DestroyAcceleratorTable(accel_);
	accel_ = NULL;
	loaded_ = false;
}


String Accelerator::KeyName(const ACCEL& accel)
{
	const int MAX= 63;
	TCHAR buf[MAX + 1];
	String name;
	name.reserve(128);

	if (accel.fVirt & FCONTROL)
	{
		buf[0] = 0;
		::GetKeyNameText((::MapVirtualKey(VK_CONTROL, 0) << 16) | 0x03000000, buf, MAX);
		name += buf;
		name += _T('+');
	}
	if (accel.fVirt & FSHIFT)
	{
		buf[0] = 0;
		::GetKeyNameText((::MapVirtualKey(VK_SHIFT, 0) << 16) | 0x02000000, buf, MAX);
		name += buf;
		name += _T('+');
	}
	if (accel.fVirt & FALT)
	{
		buf[0] = 0;
		::GetKeyNameText((::MapVirtualKey(VK_MENU, 0) << 16) | 0x03000000, buf, MAX);
		name += buf;
		name += _T('+');
	}

	if (accel.key == 0)
	{
		name += _T("None");
	}
	else if (accel.key == 0x20)
	{
		name += _T("Space");
	}
	else
	{
		buf[0] = 0;
		::GetKeyNameText((::MapVirtualKey(accel.key, 0) << 16) | (accel.key < VK_LWIN ? 0x03000000 : 0x02000000), buf, MAX);

		name += buf;
	}

	return name;
}
