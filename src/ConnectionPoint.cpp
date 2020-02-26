/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ConnectionPoint.cpp: implementation of the ConnectionPointer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ConnectionPoint.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ConnectionPointer::ConnectionPointer() : link_(0)
{}


ConnectionPointer::ConnectionPointer(ConnectionPointer* link_to) : link_(link_to)
{}


ConnectionPointer::~ConnectionPointer()
{
	if (link_)
		link_->Disconnect();
}

void ConnectionPointer::Disconnect()
{
	link_ = 0;
}


///////////////////////////////////////////////////////////////////////////////
