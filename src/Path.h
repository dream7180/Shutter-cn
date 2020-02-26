/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Path.h: interface for the Path class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATH_H__F5546635_CC6A_11D3_B61E_000000000000__INCLUDED_)
#define AFX_PATH_H__F5546635_CC6A_11D3_B61E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Path : public String
{
public:
	Path(String path) : String(path) {}
	Path(const TCHAR* path) : String(path) {}
	Path(STRRET& str, bool release_str_ret);
	Path() {}
//	virtual ~Path();

	// append dir separator (if needed)
	void AppendDirSeparator();
	// append dir
	void AppendDir(const TCHAR* dir, bool add_separator= true);
	// append *.* mask
	void AppendAllMask();
	// append mask
	void AppendMask(const TCHAR* mask);

	// rename existing file name (leaving path and file extension intact)
	void RenameFileName(const TCHAR* new_name);

	// replace existing file name and extension with a new one
	void ReplaceFileNameExt(const TCHAR* new_name);

	// append suffix to file name
	void AppendToFileName(const TCHAR* name_suffix);

	// compare extension
	bool MatchExtension(const TCHAR* ext) const;

	// match file spec
	bool MatchFileSpec(const TCHAR* spec) const;

	// replace extension
	bool ReplaceExtension(const TCHAR* new_ext);

	String GetFileName() const;
	String GetDir() const;
	String GetParentDir() const;
	String GetParentParentDir() const;
	String GetExtension() const;
	String GetFileNameAndExt() const;
	String GetRoot() const;

	bool FileExists() const;

	uint64 GetFileLength() const;

	// create directory if it doesn't exist
	bool CreateIfDoesntExist(CWnd* msg_parent) const;

	// create folders along the path
	bool CreateFolders() const;

	// returns true if path points to folder
	bool IsFolder() const;

	// returns true if path points to a link file
	bool IsLink() const;

	// resolve link file
	ITEMIDLIST* GetLinkedObject();

	// replace all '\\' by '/'
	void BackslashToForwardslash();
};


extern Path GetDocumentsFolder(const TCHAR* fallback_dir);
//extern Path GetApplicationDataFolder(const TCHAR* fallback_dir);


#endif // !defined(AFX_PATH_H__F5546635_CC6A_11D3_B61E_000000000000__INCLUDED_)
