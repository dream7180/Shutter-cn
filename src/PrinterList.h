/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrinterList.h: interface for the MPrinterList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRINTERLIST_H__C6CAC022_6988_11D1_A91F_444553540000__INCLUDED_)
#define AFX_PRINTERLIST_H__C6CAC022_6988_11D1_A91F_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <winspool.h>


class PrinterList
{
public:
	PrinterList(DWORD flags= PRINTER_ENUM_LOCAL | PRINTER_ENUM_NETWORK /* | PRINTER_ENUM_REMOTE */);
	~PrinterList();

	int GetCount() const		{ return count_; }

	const TCHAR* GetPrinterName(int index) const;
//	const TCHAR* GetPortName(int index) const;

private:
	int count_;
	int level_;
	std::vector<BYTE> buffer_;

	void EnumPrinters(DWORD flags);

	const PRINTER_INFO_5* GetPrinterInfo5(int index) const
	{
		return index >= 0 && index < count_ ? reinterpret_cast<const PRINTER_INFO_5*>(&buffer_.front()) + index : NULL;
	}

	const PRINTER_INFO_4* GetPrinterInfo4(int index) const
	{
		return index >= 0 && index < count_ ? reinterpret_cast<const PRINTER_INFO_4*>(&buffer_.front()) + index : NULL;
	}
};

#endif // !defined(AFX_PRINTERLIST_H__C6CAC022_6988_11D1_A91F_444553540000__INCLUDED_)
