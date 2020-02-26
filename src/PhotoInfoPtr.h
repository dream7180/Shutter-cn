/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class PhotoInfo;


#ifdef PHOTO_INFO_SMART_PTR


typedef mik::intrusive_ptr<PhotoInfo> PhotoInfoPtr;

typedef mik::intrusive_ptr<const PhotoInfo> ConstPhotoInfoPtr;

inline size_t GetHashValue(const ConstPhotoInfoPtr& key)	{ return reinterpret_cast<size_t>(key.get()); }

inline PhotoInfoPtr ConstCast(const ConstPhotoInfoPtr& p)	{ return PhotoInfoPtr(const_cast<PhotoInfo*>(p.get())); }

inline PhotoInfo*			GetRawPointer(const PhotoInfoPtr& p)		{ return get_pointer(p); }
inline const PhotoInfo*		GetRawPointer(const ConstPhotoInfoPtr& p)	{ return get_pointer(p); }

typedef PhotoInfoPtr SmartPhotoPtr;	// intrusive ptr is already smart
inline PhotoInfoPtr ReleaseSmartPhotoPtr(SmartPhotoPtr& p)	{ return p; }	// for symmetry only; not releaseing anything here


#else // -------------------------------


typedef PhotoInfo* PhotoInfoPtr;

typedef const PhotoInfo* ConstPhotoInfoPtr;

inline size_t GetHashValue(const ConstPhotoInfoPtr& key)	{ return reinterpret_cast<size_t>(key); }

inline PhotoInfoPtr ConstCast(const ConstPhotoInfoPtr& p)	{ return const_cast<PhotoInfo*>(p); }

inline PhotoInfo*			GetRawPointer(const PhotoInfoPtr& p)		{ return p; }
inline const PhotoInfo*		GetRawPointer(const ConstPhotoInfoPtr& p)	{ return p; }

typedef AutoPtr<PhotoInfo> SmartPhotoPtr;
inline PhotoInfoPtr ReleaseSmartPhotoPtr(SmartPhotoPtr& p)	{ return p.release(); }

#endif
