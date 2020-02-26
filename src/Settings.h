/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

namespace settings {

typedef std::vector<BYTE> Blob;


class Settings
{
public:
	Settings();
	~Settings();

	void Store(const String& id, const String& v);
	void Store(const String& id, const Blob& v);
	void Store(const String& id, int v);

	void Remove(const String& id);

	String RestoreString(const String& id, const TCHAR* default_val) const;
	int RestoreInt(const String& id, int default_val) const;
	Blob RestoreBlob(const String& id) const;

	bool Exist(const String& id) const;

	void SaveToFile(const TCHAR* filename);
	bool ReadFromFile(const TCHAR* filename);

private:
	struct Impl;
	std::auto_ptr<Impl> impl_;

	Settings(const Settings&);
	Settings& operator = (const Settings&);
};


Settings& Instance();

// are setings persisted in registry (true) or file (false)?
bool SettingsInRegistry();
// persist settings in registry or file in ExifPro's folder
void SetSettingsInRegistry(bool use_registry);


void Store(const TCHAR* section, const TCHAR* key, const String& v);
void Store(const TCHAR* section, const TCHAR* key, int v);
void Store(const TCHAR* section, const TCHAR* key, const std::vector<BYTE>& blob);

CString Restore(const TCHAR* section, const TCHAR* key, const TCHAR* default_val);
int Restore(const TCHAR* section, const TCHAR* key, int default_val);


} // namespace
