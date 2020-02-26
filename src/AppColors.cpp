/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "StdAfx.h"
#include "AppColors.h"
#include "Config.h"


ApplicationColors& GetAppColors()
{
	return g_Settings.AppColors();
}
