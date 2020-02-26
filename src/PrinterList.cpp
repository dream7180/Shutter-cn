/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrinterList.cpp: implementation of the PrinterList class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PrinterList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PrinterList::PrinterList(DWORD flags/*= PRINTER_ENUM_LOCAL | PRINTER_ENUM_NETWORK*/)
{
	count_ = 0;

	if (::GetVersion() < DWORD(0x80000000))	// NT?
		level_ = 4;
	else
		level_ = 5;

	EnumPrinters(flags);
}


PrinterList::~PrinterList()
{}


void PrinterList::EnumPrinters(DWORD flags)
{
	DWORD needed= 0;
	DWORD returned= 0;
	::EnumPrinters(flags, 0, level_, 0, 0, &needed, &returned);

	if (needed == 0)
		return;

	buffer_.resize(needed);

	if (!::EnumPrinters(flags, NULL, level_, &buffer_.front(), buffer_.size(), &needed, &returned))
		return;

	count_ = returned;
}


const TCHAR* PrinterList::GetPrinterName(int index) const
{
	if (level_ == 4)
		if (const PRINTER_INFO_4* info= GetPrinterInfo4(index))
			return info->pPrinterName;
	else
		if (const PRINTER_INFO_5* info= GetPrinterInfo5(index))
			return info->pPrinterName;

	return 0;
}

/*
const TCHAR* PrinterList::GetPortName(int index) const
{
	if (const PRINTER_INFO_5* info= GetPrinterInfo5(index))
		return info->port_name;
	return 0;
}
*/
