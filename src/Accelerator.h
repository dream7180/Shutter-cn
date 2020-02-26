/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Accelerator.h: interface for the Accelerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCELERATOR_H__D1A36243_97DC_11D1_A91F_444553540000__INCLUDED_)
#define AFX_ACCELERATOR_H__D1A36243_97DC_11D1_A91F_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class Accelerator  
{
public:
	Accelerator();
	virtual ~Accelerator();

	bool Create(ACCEL* accel_table, int entries);
	bool Load(HINSTANCE instance, LPCTSTR table_name);
	bool Load(int table_id);

	void Attach(HACCEL accel, bool loaded= true);

	static ACCEL* BinarySearch(std::vector<ACCEL>& accel, WORD cmd);

	static String KeyName(const ACCEL& accel);

	HACCEL GetHandle() { return accel_; }

	void GetSortedData(std::vector<ACCEL>& accel) const;
	void GetData(std::vector<ACCEL>& accel) const;
	// copy accel data from HACCEL to a vector
	static void GetData(HACCEL accel, std::vector<ACCEL>& out);

	void Destroy();

private:
	HACCEL accel_;
	bool loaded_;

	Accelerator(const Accelerator&);
	Accelerator& operator = (const Accelerator&);

//  static void SortData(ACCEL* data, int size);
//  static ACCEL* BinarySearch(ACCEL* data, int size, WORD cmd);
//  static int __cdecl accel_cmp(const void *elem1, const void *elem2);
};

#endif // !defined(AFX_ACCELERATOR_H__D1A36243_97DC_11D1_A91F_444553540000__INCLUDED_)
