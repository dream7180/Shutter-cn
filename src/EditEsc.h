/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_EDITESC_H__E8DA1421_B9FB_4F65_916A_F30835AEFA18__INCLUDED_)
#define AFX_EDITESC_H__E8DA1421_B9FB_4F65_916A_F30835AEFA18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditEsc.h : header file

#include "signals.h"

/////////////////////////////////////////////////////////////////////////////
// EditEsc window

class EditEsc : public CEdit
{
// Construction
public:
	EditEsc();

// Attributes
public:

// Operations
public:
	// events
	typedef boost::signals2::signal<void (int key)> EndEditKeyPress;

	// connect handler to the event
	slot_connection ConnectEndEditKeyPress(EndEditKeyPress::slot_function_type fn);

// Implementation
public:
	virtual ~EditEsc();

	// Generated message map functions
protected:
	afx_msg void OnChar(UINT chr, UINT rep_cnt, UINT flags);
	void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);

	DECLARE_MESSAGE_MAP()

private:
	EndEditKeyPress end_event_;
};


#endif // !defined(AFX_EDITESC_H__E8DA1421_B9FB_4F65_916A_F30835AEFA18__INCLUDED_)
