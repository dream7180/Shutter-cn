/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemoryStatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "MemoryStatDlg.h"
#include "Toolhelp.h"
#include <tlhelp32.h>
#include <windowsx.h>
#include <stdio.h>

// GetMappedFileName is only on Windows 2000 in PSAPI.DLL
// If this function exists on the host system, we'll use it
typedef DWORD (WINAPI* FNGETMAPPEDFILENAME)(HANDLE, PVOID, PTSTR, DWORD);
static FNGETMAPPEDFILENAME pfnGetMappedFileName= 0;


// MemoryStatDlg dialog

MemoryStatDlg::MemoryStatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MemoryStatDlg::IDD, pParent)
{
	expand_blocks_ = false;
	pfnGetMappedFileName = 0;
	showing_picture_ = false;

	// Try to load PSAPI.DLL and get the address of GetMappedFileName
	PSAPI_module_ = LoadLibrary(_T("PSAPI"));

	if (PSAPI_module_)
	{
#ifdef UNICODE
		pfnGetMappedFileName = (FNGETMAPPEDFILENAME) GetProcAddress(PSAPI_module_, "GetMappedFileNameW");
#else
		pfnGetMappedFileName = (FNGETMAPPEDFILENAME) GetProcAddress(PSAPI_module_, "GetMappedFileNameA");
#endif      
	}
}

MemoryStatDlg::~MemoryStatDlg()
{
	if (PSAPI_module_)
		::FreeModule(PSAPI_module_);
}

void MemoryStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MemoryStatDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_EXPAND, OnExpand)
	ON_BN_CLICKED(IDC_PICTURE, OnShowPicture)
END_MESSAGE_MAP()


void MemoryStatDlg::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);

	resize_.Resize();
}


//static DWORD g_dwProcessId = 0;  // Which process to walk?


/******************************************************************************
Module:  VMQuery
Notices: Copyright (c) 2000 Jeffrey Richter
******************************************************************************/


struct VMQUERY
{
   // Region information
   PVOID  pvRgnBaseAddress;
   DWORD  dwRgnProtection;  // PAGE_*
   SIZE_T RgnSize;
   DWORD  dwRgnStorage;     // MEM_*: Free, Image, Mapped, Private
   DWORD  dwRgnBlocks;
   DWORD  dwRgnGuardBlks;   // If > 0, region contains thread stack
   BOOL   fRgnIsAStack;     // TRUE if region contains thread stack

   // Block information
   PVOID  pvBlkBaseAddress;
   DWORD  dwBlkProtection;  // PAGE_*
   SIZE_T BlkSize;
   DWORD  dwBlkStorage;     // MEM_*: Free, Reserve, Image, Mapped, Private
};


// Helper structure
struct VMQUERY_HELP
{
   SIZE_T RgnSize;
   DWORD  dwRgnStorage;     // MEM_*: Free, Image, Mapped, Private
   DWORD  dwRgnBlocks;
   DWORD  dwRgnGuardBlks;   // If > 0, region contains thread stack
   BOOL   fRgnIsAStack;     // TRUE if region contains thread stack
};


// This global, static variable holds the allocation granularity value for 
// this CPU platform. Initialized the first time VMQuery is called.
static DWORD gs_dwAllocGran = 0;


///////////////////////////////////////////////////////////////////////////////


// Iterates through a region's blocks and returns findings in VMQUERY_HELP
static BOOL VMQueryHelp(HANDLE hProcess, LPCVOID pvAddress, VMQUERY_HELP* pVMQHelp)
{
   // Each element contains a page protection
   // (i.e.: 0=reserved, PAGE_NOACCESS, PAGE_READWRITE, etc.)
   DWORD dwProtectBlock[4] = { 0 }; 

   ZeroMemory(pVMQHelp, sizeof(*pVMQHelp));

   // Get address of region containing passed memory address.
   MEMORY_BASIC_INFORMATION mbi;
   BOOL fOk = (VirtualQueryEx(hProcess, pvAddress, &mbi, sizeof(mbi))
      == sizeof(mbi));

   if (!fOk)
      return(fOk);   // Bad memory address, return failure

   // Walk starting at the region's base address (which never changes)
   PVOID pvRgnBaseAddress = mbi.AllocationBase;

   // Walk starting at the first block in the region (changes in the loop)
   PVOID pvAddressBlk = pvRgnBaseAddress;

   // Save the memory type of the physical storage block.
   pVMQHelp->dwRgnStorage = mbi.Type;

   for (;;) {
      // Get info about the current block.
      fOk = (VirtualQueryEx(hProcess, pvAddressBlk, &mbi, sizeof(mbi))
         == sizeof(mbi));
      if (!fOk)
         break;   // Couldn't get the information, end loop.

      // Is this block in the same region?
      if (mbi.AllocationBase != pvRgnBaseAddress)
         break;   // Found a block in the next region; end loop.

      // We have a block contained in the region.

      // The following if statement is for detecting stacks in Windows 98.
      // A Windows 98 stack region's last 4 blocks look like this:
      //    reserved block, no access block, read-write block, reserved block
      if (pVMQHelp->dwRgnBlocks < 4) {
         // 0th through 3rd block, remember the block's protection
         dwProtectBlock[pVMQHelp->dwRgnBlocks] = 
            (mbi.State == MEM_RESERVE) ? 0 : mbi.Protect;
      } else {
         // We've seen 4 blocks in this region.
         // Shift the protection values down in the array.
         MoveMemory(&dwProtectBlock[0], &dwProtectBlock[1], 
            sizeof(dwProtectBlock) - sizeof(DWORD));

         // Add the new protection value to the end of the array.
         dwProtectBlock[3] = (mbi.State == MEM_RESERVE) ? 0 : mbi.Protect;
      }

      pVMQHelp->dwRgnBlocks++;             // Add another block to the region
      pVMQHelp->RgnSize += mbi.RegionSize; // Add block's size to region size

      // If block has PAGE_GUARD attribute, add 1 to this counter
      if ((mbi.Protect & PAGE_GUARD) == PAGE_GUARD)
         pVMQHelp->dwRgnGuardBlks++;

      // Take a best guess as to the type of physical storage committed to the
      // block. This is a guess because some blocks can convert from MEM_IMAGE
      // to MEM_PRIVATE or from MEM_MAPPED to MEM_PRIVATE; MEM_PRIVATE can
      // always be overridden by MEM_IMAGE or MEM_MAPPED.
      if (pVMQHelp->dwRgnStorage == MEM_PRIVATE)
         pVMQHelp->dwRgnStorage = mbi.Type;

      // Get the address of the next block.
      pvAddressBlk = (PVOID) ((PBYTE) pvAddressBlk + mbi.RegionSize);
   }

   // After examining the region, check to see whether it is a thread stack
   // Windows 2000: Assume stack if region has at least 1 PAGE_GUARD block
   // Windows 9x:   Assume stack if region has at least 4 blocks with
   //               3rd block from end: reserved
   //               2nd block from end: PAGE_NOACCESS
   //               1st block from end: PAGE_READWRITE
   //               block at end: another reserved block.
   pVMQHelp->fRgnIsAStack =
      (pVMQHelp->dwRgnGuardBlks > 0)         ||
      ((pVMQHelp->dwRgnBlocks >= 4)          &&
       (dwProtectBlock[0] == 0)              && 
       (dwProtectBlock[1] == PAGE_NOACCESS)  &&
       (dwProtectBlock[2] == PAGE_READWRITE) &&
       (dwProtectBlock[3] == 0));

   return(TRUE);
}


///////////////////////////////////////////////////////////////////////////////


BOOL VMQuery(HANDLE hProcess, LPCVOID pvAddress, VMQUERY* pVMQ)
{
   if (gs_dwAllocGran == 0) {
      // Set allocation granularity if this is the first call
      SYSTEM_INFO sinf;
      GetSystemInfo(&sinf);
      gs_dwAllocGran = sinf.dwAllocationGranularity;
   }

   ZeroMemory(pVMQ, sizeof(*pVMQ));

   // Get the MEMORY_BASIC_INFORMATION for the passed address.
   MEMORY_BASIC_INFORMATION mbi;
   BOOL fOk = (VirtualQueryEx(hProcess, pvAddress, &mbi, sizeof(mbi))
      == sizeof(mbi));

   if (!fOk)
      return(fOk);   // Bad memory address, return failure

   // The MEMORY_BASIC_INFORMATION structure contains valid information.
   // Time to start setting the members of our own VMQUERY structure.

   // First, fill in the block members. We'll fill the region members later.
   switch (mbi.State) {
      case MEM_FREE:       // Free block (not reserved)
         pVMQ->pvBlkBaseAddress = NULL;
         pVMQ->BlkSize = 0;
         pVMQ->dwBlkProtection = 0;
         pVMQ->dwBlkStorage = MEM_FREE;
         break;

      case MEM_RESERVE:    // Reserved block without committed storage in it.
         pVMQ->pvBlkBaseAddress = mbi.BaseAddress;
         pVMQ->BlkSize = mbi.RegionSize;

         // For an uncommitted block, mbi.Protect is invalid. So we will 
         // show that the reserved block inherits the protection attribute 
         // of the region in which it is contained.
         pVMQ->dwBlkProtection = mbi.AllocationProtect;  
         pVMQ->dwBlkStorage = MEM_RESERVE;
         break;

      case MEM_COMMIT:     // Reserved block with committed storage in it.
         pVMQ->pvBlkBaseAddress = mbi.BaseAddress;
         pVMQ->BlkSize = mbi.RegionSize;
         pVMQ->dwBlkProtection = mbi.Protect;   
         pVMQ->dwBlkStorage = mbi.Type;
         break;

      default:
          DebugBreak();
          break;
   }

   // Now fill in the region data members.
   VMQUERY_HELP VMQHelp;
   switch (mbi.State) {
      case MEM_FREE:       // Free block (not reserved)
         pVMQ->pvRgnBaseAddress = mbi.BaseAddress;
         pVMQ->dwRgnProtection  = mbi.AllocationProtect;
         pVMQ->RgnSize          = mbi.RegionSize;
         pVMQ->dwRgnStorage     = MEM_FREE;
         pVMQ->dwRgnBlocks      = 0;
         pVMQ->dwRgnGuardBlks   = 0;
         pVMQ->fRgnIsAStack     = FALSE;
         break;

      case MEM_RESERVE:    // Reserved block without committed storage in it.
         pVMQ->pvRgnBaseAddress = mbi.AllocationBase;
         pVMQ->dwRgnProtection  = mbi.AllocationProtect;

         // Iterate through all blocks to get complete region information.         
         VMQueryHelp(hProcess, pvAddress, &VMQHelp);

         pVMQ->RgnSize          = VMQHelp.RgnSize;
         pVMQ->dwRgnStorage     = VMQHelp.dwRgnStorage;
         pVMQ->dwRgnBlocks      = VMQHelp.dwRgnBlocks;
         pVMQ->dwRgnGuardBlks   = VMQHelp.dwRgnGuardBlks;
         pVMQ->fRgnIsAStack     = VMQHelp.fRgnIsAStack;
         break;

      case MEM_COMMIT:     // Reserved block with committed storage in it.
         pVMQ->pvRgnBaseAddress = mbi.AllocationBase;
         pVMQ->dwRgnProtection  = mbi.AllocationProtect;

         // Iterate through all blocks to get complete region information.         
         VMQueryHelp(hProcess, pvAddress, &VMQHelp);

         pVMQ->RgnSize          = VMQHelp.RgnSize;
         pVMQ->dwRgnStorage     = VMQHelp.dwRgnStorage;
         pVMQ->dwRgnBlocks      = VMQHelp.dwRgnBlocks;
         pVMQ->dwRgnGuardBlks   = VMQHelp.dwRgnGuardBlks;
         pVMQ->fRgnIsAStack     = VMQHelp.fRgnIsAStack;
         break;

      default:
          DebugBreak();
          break;
   }

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


PCTSTR GetMemStorageText(DWORD dwStorage)
{
	PCTSTR p= TEXT("Unknown");

	switch (dwStorage)
	{
	case MEM_FREE:    p = TEXT("Free   "); break;
	case MEM_RESERVE: p = TEXT("Reserve"); break;
	case MEM_IMAGE:   p = TEXT("Image  "); break;
	case MEM_MAPPED:  p = TEXT("Mapped "); break;
	case MEM_PRIVATE: p = TEXT("Private"); break;
	}

	return(p);
}


///////////////////////////////////////////////////////////////////////////////


PTSTR GetProtectText(DWORD dwProtect, PTSTR szBuf, BOOL fShowFlags)
{
	PCTSTR p = TEXT("Unknown");
	switch (dwProtect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE))
	{
	case PAGE_READONLY:          p = TEXT("-R--"); break;
	case PAGE_READWRITE:         p = TEXT("-RW-"); break;
	case PAGE_WRITECOPY:         p = TEXT("-RWC"); break;
	case PAGE_EXECUTE:           p = TEXT("E---"); break;
	case PAGE_EXECUTE_READ:      p = TEXT("ER--"); break;
	case PAGE_EXECUTE_READWRITE: p = TEXT("ERW-"); break;
	case PAGE_EXECUTE_WRITECOPY: p = TEXT("ERWC"); break;
	case PAGE_NOACCESS:          p = TEXT("----"); break;
	}
	_tcscpy(szBuf, p);
	if (fShowFlags)
	{
		_tcscat(szBuf, TEXT(" "));
		_tcscat(szBuf, (dwProtect & PAGE_GUARD)        ? TEXT("G") : TEXT("-"));
		_tcscat(szBuf, (dwProtect & PAGE_NOCACHE)      ? TEXT("N") : TEXT("-"));
		_tcscat(szBuf, (dwProtect & PAGE_WRITECOMBINE) ? TEXT("W") : TEXT("-"));
	}
	return szBuf;
}


///////////////////////////////////////////////////////////////////////////////


void ConstructRgnInfoLine(HANDLE hProcess, VMQUERY* pVMQ, PTSTR szLine, int nMaxLen, CToolhelp& toolhelp)
{
	_stprintf_s(szLine, size_t(nMaxLen), TEXT("%p\t%s \t%0.8x"),
		pVMQ->pvRgnBaseAddress,
		GetMemStorageText(pVMQ->dwRgnStorage),
		pVMQ->RgnSize);

	if (pVMQ->dwRgnStorage != MEM_FREE)
	{
		wsprintf(_tcschr(szLine, 0), TEXT("\t%5u "), pVMQ->dwRgnBlocks);
		GetProtectText(pVMQ->dwRgnProtection, _tcschr(szLine, 0), FALSE);
	}

	_tcscat(szLine, TEXT("\t"));

	// Try to obtain the module pathname for this region.
	int nLen = _tcslen(szLine);
	if (pVMQ->pvRgnBaseAddress != NULL)
	{
		MODULEENTRY32 me = { sizeof(me) };

		if (toolhelp.ModuleFind(pVMQ->pvRgnBaseAddress, &me))
		{
			lstrcat(&szLine[nLen], me.szExePath);
		}
		else
		{
			// This is not a module; see if it's a memory-mapped file
			if (pfnGetMappedFileName != NULL)
			{
				DWORD d = pfnGetMappedFileName(hProcess, pVMQ->pvRgnBaseAddress, szLine + nLen, nMaxLen - nLen);
				if (d == 0)
				{
					// NOTE: GetMappedFileName modifies the string when it fails
					szLine[nLen] = 0;
				}
			}
		}
	}

	if (pVMQ->fRgnIsAStack)
		_tcscat(szLine, TEXT("Thread Stack"));
}


///////////////////////////////////////////////////////////////////////////////


void ConstructBlkInfoLine(VMQUERY* pVMQ, PTSTR szLine, int nMaxLen)
{
	_stprintf_s(szLine, nMaxLen, TEXT("  %p\t%s \t%0.8x\t"),
		pVMQ->pvBlkBaseAddress,
		GetMemStorageText(pVMQ->dwBlkStorage),
		pVMQ->BlkSize);

	if (pVMQ->dwBlkStorage != MEM_FREE)
		GetProtectText(pVMQ->dwBlkProtection, _tcschr(szLine, 0), TRUE);
}


///////////////////////////////////////////////////////////////////////////////


void Refresh(HWND hwndLB, DWORD dwProcessId, BOOL fExpandRegions)
{
	// Delete contents of list box & add a horizontal scroll bar
	ListBox_ResetContent(hwndLB);
	ListBox_SetHorizontalExtent(hwndLB, 300 * LOWORD(GetDialogBaseUnits()));

	// Is the process still running?
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);

	if (hProcess == NULL)
	{
		ListBox_AddString(hwndLB, TEXT(""));   // Blank line, looks better
		ListBox_AddString(hwndLB, TEXT("    The process ID identifies a process that is not running"));
		return;
	}

	// Grab a new snapshot of the process
	CToolhelp toolhelp;
	toolhelp.CreateSnapshot(TH32CS_SNAPALL, dwProcessId);

	// Walk the virtual address space, adding entries to the list box.
//	BOOL fOk = TRUE;
	PVOID pvAddress = NULL;

	SetWindowRedraw(hwndLB, FALSE);

	for (BOOL ok= true; ok; )
	{
		VMQUERY vmq;
		if (!::VMQuery(hProcess, pvAddress, &vmq))
			break;

		// Construct the line to be displayed, and add it to the list box.
		TCHAR szLine[1024];
		ConstructRgnInfoLine(hProcess, &vmq, szLine, array_count(szLine), toolhelp);
		ListBox_AddString(hwndLB, szLine);

		if (fExpandRegions)
		{
			for (DWORD dwBlock = 0; ok && (dwBlock < vmq.dwRgnBlocks); dwBlock++)
			{
				ConstructBlkInfoLine(&vmq, szLine, array_count(szLine));
				ListBox_AddString(hwndLB, szLine);

				// Get the address of the next region to test.
				pvAddress = ((PBYTE) pvAddress + vmq.BlkSize);
				if (dwBlock < vmq.dwRgnBlocks - 1)
				{
					// Don't query the memory info after the last block.
					ok = !!::VMQuery(hProcess, pvAddress, &vmq);
				}
			}
		}

		// Get the address of the next region to test.
		pvAddress = ((PBYTE) vmq.pvRgnBaseAddress + vmq.RgnSize);
	}

	SetWindowRedraw(hwndLB, TRUE);

	CloseHandle(hProcess);
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryStatDlg::Refresh()
{
	DWORD processId= ::GetCurrentProcessId();

	// Force the list box to refresh itself
	::Refresh(listBox_, processId, expand_blocks_);

	MEMORYSTATUS ms;
	memset(&ms, 0, sizeof ms);
	ms.dwLength = sizeof ms;
	::GlobalMemoryStatus(&ms);
	TCHAR avail[200];
	wsprintf(avail, _T("Virtual Available: %d MB"), ms.dwAvailVirtual / (1024 * 1024));

//	SYSTEM_INFO si;
//	memset(&si, 0, sizeof si);
//	::GetSystemInfo(&si);

	virtLabel_.SetWindowText(avail);
}


BOOL MemoryStatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	listBox_.SubclassDlgItem(IDC_LISTBOX, this);
	virtLabel_.SubclassDlgItem(IDC_VIRT, this);

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), false);

	resize_.BuildMap(this);
	resize_.SetWndResizing(IDC_LISTBOX, DlgAutoResize::RESIZE);
	resize_.SetWndResizing(IDOK, DlgAutoResize::MOVE);
	resize_.SetWndResizing(IDC_EXPAND, DlgAutoResize::MOVE_V);
	resize_.SetWndResizing(IDC_PICTURE, DlgAutoResize::MOVE_V);
	resize_.SetWndResizing(IDC_VIRT, DlgAutoResize::MOVE_V);

	CToolhelp::EnableDebugPrivilege();

	DWORD processId= ::GetCurrentProcessId();

	TCHAR szCaption[MAX_PATH * 2];
	//GetWindowText(hwnd, szCaption, chDIMOF(szCaption));
	CToolhelp toolhelp;
	toolhelp.CreateSnapshot(TH32CS_SNAPALL, processId);
	PROCESSENTRY32 pe = { sizeof(pe) };
	wsprintf(szCaption, TEXT("PID=%u \"%s\""), processId, toolhelp.ProcessFind(processId, &pe) ? pe.szExeFile : TEXT("unknown"));
	SetWindowText(szCaption);

	font_.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, FIXED_PITCH, _T("Lucida Console"));

	listBox_.SetFont(&font_);

	Refresh();

	return true;
}


void MemoryStatDlg::OnExpand()
{
	expand_blocks_ = !!IsDlgButtonChecked(IDC_EXPAND);

	Refresh();
}


void FillStripe(BYTE* buf, DWORD size, DWORD from, DWORD to, COLORREF color)
{
	if (from > size || to > size)
	{
		ASSERT(false);
		return;
	}

	BYTE* limit= buf + to;
	BYTE r= GetRValue(color);
	BYTE g= GetGValue(color);
	BYTE b= GetBValue(color);
	for (BYTE* p= buf + from; p < limit; p += 3)
	{
		p[2] = r;
		p[1] = g;
		p[0] = b;
	}
}


COLORREF GetBlockColor(DWORD storage)
{
	COLORREF color= 0;

	switch (storage)
	{
	case MEM_FREE:    color = RGB(255,255,255); break;	// free
	case MEM_RESERVE: color = RGB(0,212,0); break;		// Reserve
	case MEM_IMAGE:   color = RGB(22,77,233); break;	// Image
	case MEM_MAPPED:  color = RGB(255,156,0); break;	// Mapped
	case MEM_PRIVATE: color = RGB(235,0,0); break;		// Private
	default: color = RGB(192,192,192); break;
	}

	return color;
}


bool CreateMemoryPicture(Dib& dib, DWORD processId)
{
	HANDLE process= OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
	if (process == 0)
		return false;


	dib.Create(1024, 512, 24, 0x60);
	const int bpp= dib.GetColorComponents();
	DWORD buf_size= dib.GetWidth() * dib.GetHeight() * bpp;
	DWORD img_scale= bpp * 1;

	BYTE* buf= dib.GetBuffer();

	PVOID address= NULL;
	DWORD from= 0;

	for (bool ok= true; ok; )
	{
		VMQUERY vmq;
		if (!::VMQuery(process, address, &vmq))
			break;

		const DWORD scale= 12;
		DWORD base= (DWORD)vmq.pvRgnBaseAddress >> scale;
		DWORD size= vmq.RgnSize >> scale;

		COLORREF color= GetBlockColor(vmq.dwRgnStorage);

		FillStripe(buf, buf_size, base * img_scale, (base + size) * img_scale, color);

		for (DWORD block= 0; ok && block < vmq.dwRgnBlocks; ++block)
		{
			COLORREF color= GetBlockColor(vmq.dwBlkStorage);

			DWORD base= (DWORD)vmq.pvBlkBaseAddress >> scale;
			DWORD size= vmq.BlkSize >> scale;

			FillStripe(buf, buf_size, base * img_scale, (base + size) * img_scale, color);

			//_stprintf(szLine, TEXT("  %p\t%s \t%0.8x\t"),
			//	pVMQ->pvBlkBaseAddress,
			//	GetMemStorageText(pVMQ->dwBlkStorage),
			//	pVMQ->BlkSize);

			//if (pVMQ->dwBlkStorage != MEM_FREE)
			//	GetProtectText(pVMQ->dwBlkProtection, _tcschr(szLine, 0), TRUE);

//			ConstructBlkInfoLine(&vmq, szLine, sizeof(szLine));
//			ListBox_AddString(hwndLB, szLine);

			// Get the address of the next region to test.
			address = ((PBYTE) address + vmq.BlkSize);
			if (block < vmq.dwRgnBlocks - 1)
			{
				// Don't query the memory info after the last block.
				ok = !!::VMQuery(process, address, &vmq);
			}
		}

		// Get the address of the next region to test.
		address = ((PBYTE) vmq.pvRgnBaseAddress + vmq.RgnSize);
	}

	return true;
}


void MemoryStatDlg::OnShowPicture()
{
	Invalidate();

	if (!showing_picture_)
	{
		DWORD processId= ::GetCurrentProcessId();
		if (::CreateMemoryPicture(dib_, processId))
		{
			showing_picture_ = true;
			listBox_.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		//

		showing_picture_ = false;
		listBox_.ShowWindow(SW_SHOWNA);
	}

	CheckDlgButton(IDC_PICTURE, showing_picture_);
}


BOOL MemoryStatDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	pDC->FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));

	if (showing_picture_ && dib_.IsValid())
	{
		dib_.Draw(pDC, CPoint(4, 4));
	}

	return true;
}
