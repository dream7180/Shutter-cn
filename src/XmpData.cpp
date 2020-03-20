/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "XmpData.h"

const TCHAR* GetXmpDataFieldName(size_t index)
{
	switch (index)
	{
	case 0: return _T("文档标题");
	case 1: return _T("作者");
	case 2: return _T("描述");
	case 3: return _T("图像评级");
	case 4: return _T("版权标志");
	case 5: return _T("关键字");

	case 6: return _T("描述编写者");
	case 7: return _T("标题");
//	case 0: return _T("Title");	// object name
//	case 0: return _T("Creator");	= author
	case 8: return _T("创建者的职业");	//
	case 9: return _T("创建者的地址");
	case 10: return _T("创建者的城市");
	case 11: return _T("创建者的州");
	case 12: return _T("创建者的邮编");
	case 13: return _T("创建者的国家");

	case 14: return _T("电话");
	case 15: return _T("邮件地址");
	case 16: return _T("网站");

	case 17: return _T("作业标识符");
	case 18: return _T("用法说明");
	case 19: return _T("提供者");
	case 20: return _T("来源");
	case 21: return _T("使用条款权利");
	case 22: return _T("版权信息 URL");

	case 23: return _T("创建日期");
	case 24: return _T("艺术流派");
	case 25: return _T("拍摄地");
	case 26: return _T("城市");
	case 27: return _T("州/省");
	case 28: return _T("国家");
	case 29: return _T("ISO 国家代码");

	case 30: return _T("IPTC 场景");
	case 31: return _T("IPTC 附属代码");

	case 32: return _T("创建者工具");

	default:
		ASSERT(false);
		throw "Invalid index in " __FUNCTION__;
	}
}


const String& GetXmpDataField(const XmpData& xmp, size_t index)
{
	switch (index)
	{
	case 0: return xmp.DocumentTitle;
	case 1: return xmp.Author;
	case 2: return xmp.Description;
	case 3: return xmp.ImageRating;
	case 4: return xmp.CopyrightNotice;
	case 5: return xmp.Keywords;

	case 6: return xmp.DescriptionWriter;
	case 7: return xmp.Headline;
//	case 0: return xmp.Title;	// object name
//	case 0: return xmp.Creator;	= author
	case 8: return xmp.CreatorsJob;	//
	case 9: return xmp.Address;
	case 10: return xmp.City;
	case 11: return xmp.State;
	case 12: return xmp.PostalCode;
	case 13: return xmp.Country;

	case 14: return xmp.Phones;
	case 15: return xmp.EMails;
	case 16: return xmp.WebSites;

	case 17: return xmp.JobIdentifier;
	case 18: return xmp.Instructions;
	case 19: return xmp.Provider;
	case 20: return xmp.Source;
	case 21: return xmp.RightsUsageTerms;
	case 22: return xmp.CopyrightInfoURL;

	case 23: return xmp.CreationDate;
	case 24: return xmp.IntellectualGenre;
	case 25: return xmp.Location;
	case 26: return xmp.City2;
	case 27: return xmp.StateProvince;
	case 28: return xmp.Country2;
	case 29: return xmp.ISOCountryCode;

	case 30: return xmp.IPTCScene;
	case 31: return xmp.IPTCSubjectCode;

	case 32: return xmp.CreatorTool;

	default:
		ASSERT(false);
		throw "Invalid index in " __FUNCTION__;
	}
}
