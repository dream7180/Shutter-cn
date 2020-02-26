/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoAttr.h: interface for the PhotoAttr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOATTR_H__7504B09B_BF91_4317_B23A_A4A97A99BB39__INCLUDED_)
#define AFX_PHOTOATTR_H__7504B09B_BF91_4317_B23A_A4A97A99BB39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma pack(1)


struct PhotoAttr
{
public:
	PhotoAttr();
	~PhotoAttr();

	void Init();

	bool IsValid() const;
	void FillDescription(const std::wstring& description);

	// +1 - rotated cw
	//  0 - upright
	// -1 - rotated ccw
	void SetThumbnailOrientation2(int rotated_clockwise, bool mirrored);

	// as above, but ignore current state
	void ResetThumbnailOrientation(int rotated_clockwise, bool mirrored);

	enum { MAX_DESC_LEN= 512 };
	enum { MIRROR_FLAG= 0x40, MODIFIED_FLAG= 0x80 };

	uint8  header_[4];
	uint8  magic_[4];
	uint16 version_major_;
	uint16 version_minor_;
	uint8  orientation_info_;
	uint8  reserved01_;
	uint8  reserved02_;
	uint8  reserved03_;
	uint32 flags2_;
	uint32 flags3_;
	uint32 flags4_;
	uint32 data_[64];
	uint8  desc_format_[4];
	uint16 description_[MAX_DESC_LEN];
};


#pragma pack()

#endif // !defined(AFX_PHOTOATTR_H__7504B09B_BF91_4317_B23A_A4A97A99BB39__INCLUDED_)
