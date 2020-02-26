/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SEException.cpp: implementation of the SEException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SEException.h"
#include <dbghelp.h>
#include "StringConversions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern String GetAppIdentifier(bool including_build);


//////////////////////////////////////////////////////////////////////
using namespace std;

#ifdef _UNICODE // =========================================

static CMutex g_single_entry;
static bool g_handling_exception= false;

#pragma pack(push)
#pragma pack(1)

struct Symbols
{
	Symbols()
	{
		memset(&s, 0, sizeof s);
		memset(buf, 0, sizeof buf);

		s.SizeOfStruct = sizeof *this;
		s.MaxNameLength = array_count(buf);
	}

	IMAGEHLP_SYMBOL s;
	CHAR buf[1000];
};

#pragma pack(pop)


bool GetModuleInfo(void* addr, char* moduleName, DWORD bufSize, DWORD& sectionNum, DWORD& offset)
{
	moduleName[0] = 0;

	MEMORY_BASIC_INFORMATION mbi;

	if (::VirtualQuery(addr, &mbi, sizeof mbi) != sizeof mbi || mbi.AllocationBase == 0)
		return false;

	BYTE* base= reinterpret_cast<BYTE*>(mbi.AllocationBase);

	if (!::GetModuleFileNameA(reinterpret_cast<HMODULE>(base), moduleName, bufSize))
		return false;

	// Point to the DOS header in memory
	IMAGE_DOS_HEADER* dosHdr= reinterpret_cast<IMAGE_DOS_HEADER*>(base);

	// From the DOS header, find the NT (PE) header
	IMAGE_NT_HEADERS* NtHdr= reinterpret_cast<IMAGE_NT_HEADERS*>(base + dosHdr->e_lfanew);

	IMAGE_SECTION_HEADER* section= IMAGE_FIRST_SECTION(NtHdr);

	// RVA is offset from module load address
	DWORD rva= reinterpret_cast<DWORD>(addr) - reinterpret_cast<DWORD>(base);

	// find section containing address of interest
	for (UINT i= 0; i < NtHdr->FileHeader.NumberOfSections; ++i, ++section)
	{
		DWORD sectionBegin= section->VirtualAddress;
		DWORD sectionEnd= sectionBegin + max(section->SizeOfRawData, section->Misc.VirtualSize);

		if (rva >= sectionBegin && rva <= sectionEnd)
		{
			sectionNum = i + 1;
			offset = rva - sectionBegin;
			return true;
		}
	}

	return false;
}

#ifdef _WIN64
	typedef STACKFRAME64 StackFrame;
	typedef DWORD64 Displacement;
	const DWORD MACHINE= IMAGE_FILE_MACHINE_AMD64;
	#define StackWalkFn StackWalk64
	void Context2StackFrame(const CONTEXT* context, StackFrame& sf)
	{
		sf.AddrPC.Offset	= context->Rip;
		sf.AddrPC.Mode		= AddrModeFlat;
		sf.AddrStack.Offset	= context->Rsp;
		sf.AddrStack.Mode	= AddrModeFlat;
		sf.AddrFrame.Offset	= context->Rbp;
		sf.AddrFrame.Mode	= AddrModeFlat;
	}
	const int PARAM_SIZE= 16;
#else
	typedef STACKFRAME StackFrame;
	typedef DWORD Displacement;
	const DWORD MACHINE= IMAGE_FILE_MACHINE_I386;
	#define StackWalkFn StackWalk
	void Context2StackFrame(const CONTEXT* context, StackFrame& sf)
	{
		sf.AddrPC.Offset	= context->Eip;
		sf.AddrPC.Mode		= AddrModeFlat;
		sf.AddrStack.Offset	= context->Esp;
		sf.AddrStack.Mode	= AddrModeFlat;
		sf.AddrFrame.Offset	= context->Ebp;
		sf.AddrFrame.Mode	= AddrModeFlat;
	}
	const int PARAM_SIZE= 8;
#endif


string WalkTheStack(CONTEXT* context)
{
	ostringstream dump;

	dump << "Thread's Call Stack:\n"
#ifdef _WIN64
	"Address          Frame            RetAddr          Param-1          Param-2          Param-3          Param-4          Function\n"
	"(RIP)            (RBP)                             (RBP+8)          (RBP+c)          (RBP+10)         (RBP+14)\n";
#else
		"Address  Frame    RetAddr  Param-1  Param-2  Param-3  Param-4  Function\n"
		"(EIP)    (EBP)             (EBP+8)  (EBP+c)  (EBP+10) (EBP+14)\n";
#endif
	dump.fill('0');

	StackFrame sf;
	memset(&sf, 0, sizeof sf);

	Context2StackFrame(context, sf);

	// pseudo handles; no need to dispose of
	HANDLE process= GetCurrentProcess();
	HANDLE thread= GetCurrentThread();

	for (int i= 0; i < 100; ++i)	// safety valve
	{
		if (!::StackWalkFn(MACHINE, process, thread, &sf, context, 0, SymFunctionTableAccess, SymGetModuleBase, 0))
			break;

		if (sf.AddrFrame.Offset == 0)
			break;

		dump << hex << setw(PARAM_SIZE) << sf.AddrPC.Offset << ' '
			<< setw(PARAM_SIZE) << sf.AddrFrame.Offset << ' '
			<< setw(PARAM_SIZE) << sf.AddrReturn.Offset << ' '
			<< setw(PARAM_SIZE) << sf.Params[0] << ' '
			<< setw(PARAM_SIZE) << sf.Params[1] << ' '
			<< setw(PARAM_SIZE) << sf.Params[2] << ' '
			<< setw(PARAM_SIZE) << sf.Params[3];

		Symbols symbols;
		Displacement symbolDisplacement= 0;

		// show function name
		if (::SymGetSymFromAddr(process, sf.AddrPC.Offset, &symbolDisplacement, &symbols.s))
			dump << "  function: " << symbols.s.Name << " displacement: " << setw(8) << hex << symbolDisplacement;

		// line and number
		IMAGEHLP_LINE line;
		memset(&line, 0, sizeof line);
		line.SizeOfStruct = sizeof line;
		DWORD disp= static_cast<DWORD>(symbolDisplacement);

		if (::SymGetLineFromAddr(process, sf.AddrPC.Offset, &disp, &line))
			dump << " file: " << line.FileName << " line: " << dec << line.LineNumber;
		else
		{
			// no file/line number available; show module info instead

			char moduleName[MAX_PATH];
			DWORD section= 0;
			DWORD offset= 0;

			dump.fill('0');

			if (GetModuleInfo((void*)sf.AddrPC.Offset, moduleName, array_count(moduleName), section, offset))
				dump << " module: " << moduleName << ", section:offset: " << hex << section << ':' << hex << setw(8) << offset;
		}

		dump << endl;
	}

	return dump.str();
}


extern string DumpTheStack(CONTEXT* context)
{
	static bool initialized= false;
	try
	{
		if (!initialized)
		{
			::SymInitialize(::GetCurrentProcess(), 0, true);
			initialized = true;
		}

		return WalkTheStack(context);
	}
	catch (...)
	{
		ASSERT(false);
	}

	return string("Stack dump failed.");
}


// exception info, registers, module at fault
//
string DumpExceptionInfo(EXCEPTION_POINTERS* exceptionInfo)
{
	ostringstream dump;

	EXCEPTION_RECORD* record= exceptionInfo->ExceptionRecord;

	dump.fill('0');
	dump << "Exception code: " << setw(8) << hex << record->ExceptionCode << " at: " << setw(8) << hex << record->ExceptionAddress;

	char moduleName[MAX_PATH];
	DWORD section= 0;
	DWORD offset= 0;

	if (GetModuleInfo(record->ExceptionAddress, moduleName, array_count(moduleName), section, offset))
		dump << " in module: " << moduleName << ", section:offset: " << hex << section << ':' << hex << setw(8) << offset;

	dump << '\n';

	CONTEXT* ctx= exceptionInfo->ContextRecord;

	dump.fill('0');
	dump << hex;

#ifdef _WIN64
	dump << "RIP: " << setw(16) << ctx->Rip << ' ';
	dump << "Flags: " << setw(8) << ctx->EFlags << '\n';

	dump << "RAX: " << setw(16) << ctx->Rax << ' ';
	dump << "RBX: " << setw(16) << ctx->Rbx << ' ';
	dump << "RCX: " << setw(16) << ctx->Rcx << ' ';
	dump << "RDX: " << setw(16) << ctx->Rdx << '\n';

	dump << "RSP: " << setw(16) << ctx->Rsp << ' ';
	dump << "RBP: " << setw(16) << ctx->Rbp << ' ';
	dump << "RSI: " << setw(16) << ctx->Rsi << ' ';
	dump << "RDI: " << setw(16) << ctx->Rdi << '\n';

	dump << "R8:  " << setw(16) << ctx->R8 << ' ';
	dump << "R9:  " << setw(16) << ctx->R9 << ' ';
	dump << "R10: " << setw(16) << ctx->R10 << ' ';
	dump << "R11: " << setw(16) << ctx->R11 << '\n';

	dump << "R12: " << setw(16) << ctx->R12 << ' ';
	dump << "R13: " << setw(16) << ctx->R13 << ' ';
	dump << "R14: " << setw(16) << ctx->R14 << ' ';
	dump << "R15: " << setw(16) << ctx->R15 << '\n';
#else
	dump << "EIP: " << setw(8) << ctx->Eip << ' ';
	dump << "Flags: " << setw(8) << ctx->EFlags << '\n';

	dump << "EAX: " << setw(8) << ctx->Eax << ' ';
	dump << "EBX: " << setw(8) << ctx->Ebx << ' ';
	dump << "ECX: " << setw(8) << ctx->Ecx << ' ';
	dump << "EDX: " << setw(8) << ctx->Edx << '\n';

	dump << "ESI: " << setw(8) << ctx->Esi << ' ';
	dump << "EDI: " << setw(8) << ctx->Edi << ' ';
	dump << "ESP: " << setw(8) << ctx->Esp << ' ';
	dump << "EBP: " << setw(8) << ctx->Ebp << '\n';
#endif
	return dump.str();
}


extern std::string DumpMemoryStatus()
{
	MEMORYSTATUSEX ms;
	ms.dwLength = sizeof ms;

	if (!::GlobalMemoryStatusEx(&ms))
		return "";

	ostringstream ost;
	size_t MEG= 1024 * 1024;

	ost << "Memory status:\n";
	ost << " memory load: " << ms.dwMemoryLoad << "%\n";
	ost << " total memory: \t\t" << ms.ullTotalPhys / MEG << '\n';
	ost << " available memory:\t" << ms.ullAvailPhys / MEG << '\n';
	ost << " total page file:\t" << ms.ullTotalPageFile / MEG << '\n';
	ost << " available page file:\t" << ms.ullAvailPageFile / MEG << '\n';
	ost << " total virtual memory:\t" << ms.ullTotalVirtual / MEG << '\n';
	ost << " available virtual:\t" << ms.ullAvailVirtual / MEG << '\n';
	ost << " available ext virtual:\t" << ms.ullAvailExtendedVirtual / MEG << '\n';

	return ost.str();
}


extern void ReportCrashInfo(ofstream& dump, EXCEPTION_POINTERS* exp)
{
	dump << "---------------------------------------------------------------------------------------------------\n";

	// report when exception occured
	dump.fill('0');

	SYSTEMTIME st;
	::GetLocalTime(&st);
	dump << st.wYear << '-' << setw(2) << st.wMonth << '-' << setw(2) << st.wDay << ' ' <<
		setw(2) << st.wHour << ':' << setw(2) << st.wMinute << ':' << setw(2) << st.wSecond << '.' << st.wMilliseconds;

	dump << '\n';

	// app version
	string app;
	WideStringToMultiByte(GetAppIdentifier(true), app);
	dump << app << '\n';

	// dump exception information
	if (exp)
		dump << DumpExceptionInfo(exp) << '\n';

	dump << DumpMemoryStatus() << '\n';

	// and call stack
	if (exp)
		dump << DumpTheStack(exp->ContextRecord) << endl;
}


void ReportCrashInfo(char* fileName, EXCEPTION_POINTERS* exp)
{
	ofstream dump(fileName, ios::ate | ios::app);

	ReportCrashInfo(dump, exp);

	dump.close();
}


void SEException::SETransFunction(unsigned int code, EXCEPTION_POINTERS* exp)
{
//	ASSERT(false);

	// fatal exception encountered: dump context info to a crash file

	CSingleLock lock(&g_single_entry, false);

    if (lock.Lock(10))	// wait for 10 ms, then skip reporting
	{
		// there is no space to create stack dump when stack overflow happens...
		if (!g_handling_exception && exp->ExceptionRecord->ExceptionCode != EXCEPTION_STACK_OVERFLOW)
		{
			try
			{
				char name[_MAX_PATH + 8];
				VERIFY(::GetModuleFileNameA(AfxGetInstanceHandle(), name, _MAX_PATH));
				strcat(name, "-crash.txt");

				ReportCrashInfo(name, exp);
				/*
				CFile crash;
				if (crash.Open(name, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate))
				{
				crash.Write(exp->ContextRecord, sizeof(*exp->ContextRecord));
				crash.Write("\n\n\n\n", 4);
				UINT elem= sizeof(exp->ExceptionRecord->ExceptionInformation[0]);
				UINT size= exp->ExceptionRecord->NumberParameters * elem;
				size += sizeof(*exp->ExceptionRecord) - elem;
				crash.Write(exp->ExceptionRecord, size);
				crash.Write("\n\n\n\n", 4);
				__time64_t time= CTime::GetCurrentTime().GetTime();
				crash.Write(&time, sizeof time);
				crash.Write("\r\r\r\r\r\r\r\r", 8);
				crash.Close();
				}
				*/
			}
			catch (...)
			{}
		}
	}

	throw SEException(code);
}


#else


void SEException::SETransFunction(unsigned int code, EXCEPTION_POINTERS* exp)
{
	throw SEException(code);
}


#endif // _UNICODE =========================================


void SEException::Install()
{
	_set_se_translator(SETransFunction);
}
