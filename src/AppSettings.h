/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class AppSettings
{
public:
	AppSettings();
	~AppSettings();

	bool ValueExists(const TCHAR* section, const TCHAR* key);

	bool GetProfileBool(const TCHAR* section, const TCHAR* key, bool default_val);
	bool WriteProfileBool(const TCHAR* section, const TCHAR* key, bool value);

	int GetProfileInt(const TCHAR* section, const TCHAR* key, int default_val);
	bool WriteProfileInt(const TCHAR* section, const TCHAR* key, int value);

	double GetProfileDouble(const TCHAR* section, const TCHAR* key, double default_val);
	bool WriteProfileDouble(const TCHAR* section, const TCHAR* key, double value);

	String GetProfileString(const TCHAR* section, const TCHAR* key, const TCHAR* default_val= 0);
	bool WriteProfileString(const TCHAR* section, const TCHAR* key, const TCHAR* value);

	bool GetProfileBinary(const TCHAR* section, const TCHAR* key, std::vector<BYTE>& bytes);
	bool WriteProfileBinary(const TCHAR* section, const TCHAR* key, void* data, size_t size);

	// save all settings to the file
	bool Save(const TCHAR* file_name);

	bool Load(const TCHAR* file_name);

private:
	struct Impl;
	std::auto_ptr<Impl> impl_;

	AppSettings(const AppSettings&);
	AppSettings& operator = (const AppSettings&);
};
