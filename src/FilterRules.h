#pragma once
class FilterRules;
namespace AdvFilter
{

enum { MAX_TAGS= 10000 };

//FilterRules& GetFilterRules();
//bool LoadFilters(const TCHAR* filename, FilterRules& FilterRules, CWnd* parent);
String GetFiltersPathName();
void SaveFilters(const TCHAR* filename, String FilterRules);

} // namespace