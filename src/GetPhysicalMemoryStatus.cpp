/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


extern DWORDLONG GetPhysicalMemoryStatus()
{
	DWORDLONG phys_memory= 0;

	//if (HINSTANCE hinstDll= ::LoadLibrary(_T("KERNEL32.dll")))
	//{
	//	// look for the GlobalMemoryStatusEx function

	//	typedef /*WINBASEAPI*/ BOOL (WINAPI * fnGlobalMemoryStatusEx)(IN OUT LPMEMORYSTATUSEX buffer);

	//	fnGlobalMemoryStatusEx pfnGlobalMemory= reinterpret_cast<fnGlobalMemoryStatusEx>(::GetProcAddress(hinstDll, "GlobalMemoryStatusEx"));

	//	if (pfnGlobalMemory != 0)
	//	{
	//		MEMORYSTATUSEX ms;
	//		ms.dwLength = sizeof ms;

	//		if ((*pfnGlobalMemory)(&ms))
	//			phys_memory = ms.ullTotalPhys;
	//	}

	//	::FreeLibrary(hinstDll);
	//}

	//if (phys_memory == 0)
	//{
	//	MEMORYSTATUS ms;
	//	::GlobalMemoryStatus(&ms);
	//	phys_memory = ms.dwTotalPhys;
	//}

	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof ms;

	if (::GlobalMemoryStatusEx(&ms))
		phys_memory = ms.ullTotalPhys;

	if (phys_memory == 0)
	{
		MEMORYSTATUS ms;
		::GlobalMemoryStatus(&ms);
		phys_memory = ms.dwTotalPhys;
	}

	return phys_memory;
}


// GetLogicalProcessorInformation
typedef BOOL (WINAPI *GetLogicalProcessorInformationFn)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer, PDWORD ReturnedLength);


static DWORD CountSetBits(ULONG_PTR bitMask)
{
    const int LSHIFT= sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    

    for (int i= 0; i <= LSHIFT; ++i)
    {
		if (bitMask & bitTest)
			++bitSetCount;
        bitTest/=2;
    }

    return bitSetCount;
}


static std::pair<int, int> GetLogicalProcessorInfoHelper()
{
	std::pair<int, int> empty= std::make_pair(0, 0);

	GetLogicalProcessorInformationFn get_lpi=
		reinterpret_cast<GetLogicalProcessorInformationFn>(
			::GetProcAddress(GetModuleHandle(_T("kernel32")), "GetLogicalProcessorInformation"));

	if (get_lpi == 0)
		return empty;

	DWORD length= 0;
	if (get_lpi(0, &length) != 0 || GetLastError() != ERROR_INSUFFICIENT_BUFFER || length == 0)
		return empty;

	std::vector<BYTE> buffer(length);
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION* info= reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION*>(&buffer[0]);

	if (get_lpi(info, &length) == 0)
		return empty;

	int cores= 0;
	int logical_cores= 0;
	const size_t count= length / sizeof(*info);
	for (size_t i= 0; i < count; ++i)
		if (info[i].Relationship == RelationProcessorCore)
		{
			++cores;

			// A hyperthreaded core supplies more than one logical processor
			logical_cores += CountSetBits(info[i].ProcessorMask);
		}

	return std::make_pair(cores, logical_cores);
}


extern std::pair<int, int> GetLogicalProcessorInfoEx()
{
	static int no_of_cores= 0;
	static int no_of_threads= 0;

	if (no_of_cores == 0)
	{
		std::pair<int, int> cores= GetLogicalProcessorInfoHelper();
		no_of_cores = cores.first > 0 ? cores.first : 1;
		no_of_threads = cores.second > 0 ? cores.second : 1;
	}

	return std::make_pair(no_of_cores, no_of_threads);
}


extern int GetLogicalProcessorInfo()
{
	return GetLogicalProcessorInfoEx().first;	// no of cores
}


extern int GetLogicalProcessorCores()
{
	return GetLogicalProcessorInfoEx().second;	// number of cores (times 2 for hyperthreading)
}
