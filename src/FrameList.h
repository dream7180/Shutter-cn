/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FrameList.h: interface for the CFrameList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRAMELIST_H__B8ED1FC8_B2FD_426B_B093_650C02B856D9__INCLUDED_)
#define AFX_FRAMELIST_H__B8ED1FC8_B2FD_426B_B093_650C02B856D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFrame;


class CFrameList : public list<CFrame*>
{
public:
	CFrameList();
	virtual ~CFrameList();

	// get no of elements
	int GetCount() const					{ return size(); }

	// get first element
	CFrame* GetHead() const					{ return front(); }

	// find frame at given coords
	CFrame* FrameAt(const CPoint& pos, CFrame* sel_frm/*= 0*/) const;

	// returns true if given frame belongs to this list
	bool Find(const CFrame* frame) const;

	// remove all elements
	void Clear()							{ clear(); }

	// remove given frame from the list
	bool Remove(const CFrame* frame);

	// append frame at the end of the list
	void Append(CFrame* frame);

	// return rect encompassing all frames
	bool GetEncompassingRect(CRect& rect, bool skip_locked_frames) const;

	// is list empty
	bool IsEmpty() const					{ return empty(); }
};

#endif // !defined(AFX_FRAMELIST_H__B8ED1FC8_B2FD_426B_B093_650C02B856D9__INCLUDED_)
