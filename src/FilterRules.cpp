#include "stdafx.h"
#include "FilterRules.h"
#include "Path.h"
#include <boost/algorithm/string/replace.hpp>
#include "Exception.h"

using namespace boost::algorithm;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace AdvFilter {

String GetFiltersPathName()
{
	TCHAR prog_path[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), prog_path, _MAX_PATH));
	Path path= Path(prog_path).GetDir();
	path.AppendDir(_T("FilterRules.dat"), false);
	return path;
}

void SaveFilters(const TCHAR* filename, String FiltersDef)
{
	CFile file(filename, CFile::modeWrite | CFile::modeCreate);

	if (FiltersDef.empty())
	{
		ASSERT(false);
		return;
	}

#ifdef _UNICODE
	// insert unicode signature, so Notepad will handle this file properly
	FiltersDef.insert(0, 1, wchar_t(0xfeff));
//	tags.insert(0, 1, L'\xfeff');
#endif

	// DOS end of lines
	replace_all(FiltersDef, _T("\xa"), _T("\xd\xa"));

	size_t size= FiltersDef.length() * sizeof TCHAR;
	if (size > 1 << 31)	// 2 GB
		THROW_EXCEPTION(L"保存出错", L"超出文件大小限制.");

	file.Write(&FiltersDef[0], static_cast<UINT>(size));
}

} // namespace