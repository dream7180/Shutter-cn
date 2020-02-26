/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Dib.h"
#include "JPEGDecoder.h"
#include "MemoryDataSource.h"
#include "JPEGException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


AutoPtr<Dib> LoadJpeg(UINT img_id)
{
	const TCHAR* resource_name= MAKEINTRESOURCE(img_id);
	const TCHAR* resource_type= _T("JPEG");

	HINSTANCE inst= AfxFindResourceHandle(resource_name, resource_type);
	HRSRC rsrc= ::FindResource(inst, resource_name, resource_type);
	if (rsrc == NULL)
		return 0;

	HGLOBAL global= ::LoadResource(inst, rsrc);
	if (global == NULL)
		return 0;

	DWORD size= ::SizeofResource(inst, rsrc);

	BYTE* data= reinterpret_cast<BYTE*>(::LockResource(global));
	if (data == NULL)
		return 0;

	AutoPtr<Dib> image(new Dib);

	try
	{
		CMemoryDataSource memsrc(data, size);
		JPEGDecoder dec(memsrc);
		dec.DecodeImg(*image);
	}
	catch (JPEGException&)
	{
		image.free();
	}

	::UnlockResource(global);
	::FreeResource(global);

	return image;
}
