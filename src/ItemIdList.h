/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ItemIdList.h: interface for the ItemIdList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMIDLIST_H__1895DBC3_261F_45A9_976A_756120324B0B__INCLUDED_)
#define AFX_ITEMIDLIST_H__1895DBC3_261F_45A9_976A_756120324B0B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ItemIdList
{
public:
	ItemIdList();
	ItemIdList(const TCHAR* reg_section, const TCHAR* reg_entry);
	ItemIdList(ITEMIDLIST* idl, bool copy);
	ItemIdList(const ITEMIDLIST* idl);
	ItemIdList(const TCHAR* abs_path);
	ItemIdList(const int cs_idl);
	ItemIdList(const ItemIdList& src);
	virtual ~ItemIdList();

	// calculate list length (in bytes)
	UINT GetLength() const;

	// concatenate IDLs
	ITEMIDLIST* operator += (const ITEMIDLIST* idl);

	// assign IDL (make copy)
	ITEMIDLIST* operator = (const ITEMIDLIST* idl);

	// assign copy of IDL
	ItemIdList& operator = (const ItemIdList& src);

	// compare
	bool operator == (const ItemIdList& src);

	// create empty IDL
	void CreateEmptyIDL();

	operator ITEMIDLIST* ()					{ return idl_; }
	operator const ITEMIDLIST* () const		{ return idl_; }

	const ITEMIDLIST* GetPidl() const		{ return idl_; }
	ITEMIDLIST* GetPidl()					{ return idl_; }

	LPCITEMIDLIST* GetConstPIDL() const		{ return (LPCITEMIDLIST*)(&idl_); }

	static UINT GetLength(const ITEMIDLIST* idl);

	// store IDL in registry
	bool Store(const TCHAR* reg_section, const TCHAR* reg_entry) const;

	// retrieve IDL from registry
	bool Retrieve(const TCHAR* reg_section, const TCHAR* reg_entry);

	// verify whether IDL points to something
	bool IsEmpty() const;

	// verify whether IDL is initialized
	bool IsInitialized() const;

	// get folder path
	bool GetPath(TCHAR* path) const;
	CString GetPath() const;

	// get folder name
	bool GetName(TCHAR* name) const;
	CString GetName() const;

	// get index of an icon in system image list
	int GetIconIndex(bool large= false) const;

	// assign passed pidl, do not make a copy of it
	void AssignNoCopy(ITEMIDLIST* idl);

	// if true returns copy of parent PIDL in 'parent'
	bool GetParent(ItemIdList& parent) const;

	// free IDL list
	void Free();

	// release pidl
	ITEMIDLIST* Detach();

private:
//	CIP<IMalloc, &IID_IMalloc> malloc_;
	IMallocPtr malloc_;
	void GetMalloc();

	ITEMIDLIST* idl_;

//	ItemIdList(const ItemIdList&);
};

#endif // !defined(AFX_ITEMIDLIST_H__1895DBC3_261F_45A9_976A_756120324B0B__INCLUDED_)
