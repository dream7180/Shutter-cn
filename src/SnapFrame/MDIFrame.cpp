/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MDIFramex.cpp : implementation file
//

#include "stdafx.h"
#include "MDIFrame.h"
#include "../Resource.h"
#include "../LMSetup.h"
#include "../mainfrm.h"
#include "../iADC.h"
#include "../LMSetupDoc.h"
#include "../UIMessages.h"

extern CLMSetupApp theApp;
#include "../GetPaneLayoutInfo.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR* const REGISTRY_ENTRY_SETTINGS= _T("Settings");
const TCHAR* const REG_MAXIMIZE= _T("MaximizeMDIWnd");

/////////////////////////////////////////////////////////////////////////////

CFramePages::CFramePages(CMDIFrame* parent) : parent_(parent)
{
	ASSERT(parent_);
	current_page_index_ = - 1;
	reserve(32);
}

CFramePages::~CFramePages()
{
}


int CFramePages::CreatePage(CWnd* parent, const TCHAR* title, int icon,
		 const PaneLayoutInfoArray& PanesInfo, FramePageCreateContext* context)
{
/*   // Tab number 2 can be either "iADC" or "iADC Link"
   if( size() > 0 && ( _tcscmp(title,_T("iADC"))== 0 || _tcscmp(title,_T("iADC Link"))== 0))
   {
      CSnapFrame* frm = new CSnapFrame();
      if( frm)
      {
         if( !frm->Create(parent, title, icon, PanesInfo, context))
         {
            delete frm;
            return false;
         }
         insert( begin() + 1, 1, frm);
         return true;
      }
      return false;
   }
   else */
   {
	   push_back(new CSnapFrame());

	   if (!back()->Create(parent, title, icon, PanesInfo, context))
		   return -1;

	   return size() - 1;
   }
}


void CFramePages::DestroyPages()
{
	for (iterator it= begin(); it != end(); ++it)
		(*it)->DestroyWindow();
   clear();
}


CSnapFrame* CFramePages::GetCurrent()
{
	if (current_page_index_ >= 0 && current_page_index_ < size())
		return (*this)[current_page_index_];

	return 0;
}


bool CFramePages::SelectPage(int index, bool change_notification)
{
	if (index < 0 || index >= size())
	{
		ASSERT(false);
		return false;
	}

	if (current_page_index_ != index && current_page_index_ >= 0 &&
		current_page_index_ < size())
	{
		CSnapFrame* wnd= (*this)[current_page_index_];

		// send tab change notification?
		if (change_notification)
			if (wnd->SendTabChangeNotification())
				return false;	// prevent tab change

		wnd->ShowFrame(false);
	}

	current_page_index_ = index;

	CSnapFrame* wnd= (*this)[current_page_index_];
	wnd->ModifyStyle(WS_DISABLED, 0);
	parent_->RecalcLayout();
	wnd->ShowFrame(true);

	parent_->PageSelected(current_page_index_, wnd->GetTabTitle());

	return true;
}


bool CFramePages::SelectPage(CSnapFrame* page)
{
	int tab= 0;
	for (const_iterator it= begin(); it != end(); ++it, ++tab)
		if (*it == page)
			return SelectPage(tab, true);

	return false;
}

CSnapFrame* CFramePages::GetPage(int index)
{
	if (index >= 0 && index < size())
		return (*this)[index];

	ASSERT(false);
	return 0;
}


// activate next/previous page
//
void CFramePages::NextPage()
{
	if (current_page_index_ >= 0 && current_page_index_ < size())
	{
		int next= current_page_index_ + 1;
		if (next == size())
			next = 0;
		SelectPage(next, true);
	}
}

void CFramePages::PrevPage()
{
	if (current_page_index_ >= 0 && current_page_index_ < size())
	{
		int prev= current_page_index_ - 1;
		if (prev < 0)
			prev = size() - 1;
		SelectPage(prev, true);
	}
}


// call InitialUpdateFrame() for each page pane windows
void CFramePages::InitialUpdateFrames(CDocument* doc, bool make_visible)
{
	for (iterator it= begin(); it != end(); ++it)
	{
		// send initial update to all views (and other controls) in the frame
		(*it)->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
	}
}


// resize each page window
void CFramePages::ResizeWindowsAndHide(const CRect& rect)
{
	for (iterator it= begin(); it != end(); ++it)
	{
		CSnapFrame* frm= *it;
		frm->ModifyStyle(WS_VISIBLE, WS_DISABLED);
		frm->SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


CSnapFrame* CFramePages::FindPage(const TCHAR* title)
{
	for (const_iterator it= begin(); it != end(); ++it)
		if( _tcscmp(title,(*it)->GetTabTitle()) == 0)
			return *it;
		return 0;

}

void CFramePages::RemovePage(CSnapFrame* page)
{
	for (iterator it= begin(); it != end(); ++it)
	{
		if (*it == page)
		{
			(*it)->DestroyWindow();
			erase(it);
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMDIFrame

IMPLEMENT_DYNCREATE(CMDIFrame, CMDIChildWnd)

CMDIFrame::CMDIFrame() : snap_frames_(GetThis())
{
	initialized_ = false;
	document_ = 0;
}

CMDIFrame::~CMDIFrame()
{
}


BEGIN_MESSAGE_MAP(CMDIFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CMDIFrame)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_TAB_NEXT, OnTabNext)
	ON_COMMAND(ID_TAB_PREV, OnTabPrev)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMDIFramex message handlers

BOOL CMDIFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWnd::PreCreateWindow(cs))
		return FALSE;

	cs.style |= WS_CLIPCHILDREN;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	cs.style |= WS_MAXIMIZE;

	return TRUE;
}


BOOL CMDIFrame::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);
	dc->FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));
	return true;
}


BOOL CMDIFrame::OnCreateClient(LPCREATESTRUCT, CCreateContext* context)
{
	create_context_ = *context;
	return true;
}


int CMDIFrame::AddPage(const PaneLayoutInfoArray* panes_info, recipe_type recipe)
{
	int page_index= -1;

	if (panes_info != 0)
	{
		FramePageCreateContext cc(create_context_, recipe);

        page_index = snap_frames_.CreatePage(this, panes_info->page_tab_name_,
					panes_info->icon_index_, *panes_info, &cc);
	}

	if (page_index < 0)
		throw _T("Pane window creation failure");

	return page_index;
}


void CMDIFrame::CreatePaneWindows(bool show_docking_bar)
{
	tabs_wnd_.Create(this, &snap_frames_, show_docking_bar);

	// resize all page windows now: this will in turn force pane window
	// to create their view windows
	CRect rect;
	GetClientRect(rect);
	snap_frames_.ResizeWindowsAndHide(rect);

	snap_frames_.SelectPage(0, false);

	tabs_wnd_.Refresh();
}


void CMDIFrame::OnDestroy()
{
	document_ = 0;

	if (initialized_)
	{
		// when closing document store MDI window state
		WINDOWPLACEMENT wp;
		wp.length = sizeof wp;
		GetWindowPlacement(&wp);
		AfxGetApp()->WriteProfileInt(REGISTRY_ENTRY_SETTINGS, REG_MAXIMIZE, wp.showCmd == SW_SHOWMAXIMIZED);
	}

	snap_frames_.DestroyPages();

	CMDIChildWnd::OnDestroy();
}


void CMDIFrame::RecalcLayout(BOOL /*notify*/)
{
	CRect cl_rect;
	GetClientRect(cl_rect);

	int y_pos= 1;

	if (tabs_wnd_.m_hWnd && (tabs_wnd_.GetStyle() & WS_VISIBLE))
	{
		CRect rect;
		tabs_wnd_.GetWindowRect(rect);
		tabs_wnd_.SetWindowPos(0, 1, y_pos, cl_rect.Width() - 2, rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		y_pos += rect.Height() + 2;
	}

	if (CSnapFrame* wnd= snap_frames_.GetCurrent())
		if (wnd->m_hWnd)
			wnd->SetWindowPos(0, 0, y_pos, cl_rect.Width(), cl_rect.Height() - y_pos, SWP_NOZORDER | SWP_NOACTIVATE);
}


void CMDIFrame::OnTabNext()
{
	snap_frames_.NextPage();
	tabs_wnd_.Refresh();
}

void CMDIFrame::OnTabPrev()
{
	snap_frames_.PrevPage();
	tabs_wnd_.Refresh();
}


BOOL CMDIFrame::OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info)
{
	if (CMDIChildWnd::OnCmdMsg(id, code, extra, handler_info))
		return true;

	if (CSnapFrame* frame= GetCurrentPage())
		return frame->OnCmdMsg(id, code, extra, handler_info);

	return false;
}


void CMDIFrame::ActivateFrame(int cmd_show)
{
	// when creating first MDI window show it maximized if last
	// closed MDI window was also maximized
	if (cmd_show == -1)
	{
		CMDIFrameWnd* frame_wnd = GetMDIFrame();
		BOOL maximized;
		if (frame_wnd->MDIGetActive(&maximized) == 0)	// first MDI window?
			if (AfxGetApp()->GetProfileInt(REGISTRY_ENTRY_SETTINGS, REG_MAXIMIZE, 1))
				cmd_show = SW_SHOWMAXIMIZED;
	}
	
	CMDIChildWnd::ActivateFrame(cmd_show);
}


int CMDIFrame::OnCreate(LPCREATESTRUCT create_struct)
{	
	if (CWnd* wnd= GetParent())
	{
		CRect rect;
		wnd->GetClientRect(rect);
		rect.DeflateRect(10, 10);
		SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
	}

	if (CMDIChildWnd::OnCreate(create_struct) == -1)
		return -1;

	MDICREATESTRUCT* mcs= reinterpret_cast<MDICREATESTRUCT*>(create_struct->create_params);
	ASSERT(mcs);
	CCreateContext* context= reinterpret_cast<CCreateContext*>(mcs->lParam);
	ASSERT(context);
	document_ = context->m_pCurrentDoc;

	if (LMSetupDoc* doc = dynamic_cast<LMSetupDoc*>(document_))
		doc->SetFrameWnd(this);
	else
	{
		ASSERT(false);
	}

	return 0;
}


CDocument* CMDIFrame::GetActiveDocument()
{
	return document_;
}


void CMDIFrame::OnClose()
{
	if (document_ && !document_->SaveModified())
		return;

	CMDIChildWnd::OnClose();
}


void CMDIFrame::OnFileClose()
{
	OnClose();
}


void CMDIFrame::OnUpdateFrameTitle(BOOL add_to_title)
{
	CMDIChildWnd::OnUpdateFrameTitle(add_to_title);

}


void CMDIFrame::InitialUpdateFrames(CDocument* doc, bool make_visible)
{
	initialized_ = true;

	int cmd_show= -1;
	ActivateFrame(cmd_show);

	// paint 'dock bar' first; it looks better when there's no glaring hole at the top of frame
	tabs_wnd_.UpdateWindow();

	if (doc != NULL)
		doc->UpdateFrameCounts();

	OnUpdateFrameTitle(TRUE);

	snap_frames_.InitialUpdateFrames(doc, make_visible);
}


void CMDIFrame::SetCurrentPage(CSnapFrame* tab)
{
	if (GetCurrentPage() == tab)
		return;

	snap_frames_.SelectPage(tab);
	tabs_wnd_.Refresh();
}


/*
bool CMDIFrame::AddiADCLinkPage()
{
   LMSetupDoc* doc = static_cast<LMSetupDoc*>(document_);
   CSnapFrame* frm = snap_frames_.FindPage(_T("iADC"));
   if( frm)
   {
      // Send recipe to framework
      if( doc->IsFeatureLicensed(LF_RBC_PEO))
         doc->UpdateAllViews(NULL, GUI_MSG_GET_RECIPE_LWC);
      //  Remoce iADC tab
      snap_frames_.RemovePage(frm);
   }

   // activate iADC Link page if existed
   frm = snap_frames_.FindPage(_T("iADC Link"));
   if( frm)
   {
      snap_frames_.SelectPage(frm);
      return true;
   }
   //else
   // return AddPage(_T("iADC Link"));

   // otherwise, create the new iADC Link page.
   // Based on licensing, iADC Link page can also include RBC PEO
   // Refer to GetPaneLayoutInfo( page) for pane layout and page number

   if (!AddPage(GetPaneLayoutInfo(doc->IsFeatureLicensed(LF_RBC_PEO) ? TAB_iADC_LINK : TAB_iADC_LINK_WITHOUT_RBC), RT_IADC31))
	   return false;

   CRect rect;
   GetClientRect(rect);
   snap_frames_.ResizeWindowsAndHide(rect);
   // send initial update to all views (and other controls) in the frame
   CSnapFrame* page = snap_frames_.FindPage(_T("iADC Link"));
   if( page)
   {
      page->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
	   snap_frames_.SelectPage(page);
   }
   tabs_wnd_.Refresh(true);
   
   // get recipe from framework
   if( doc->IsFeatureLicensed(LF_RBC_PEO))
      doc->UpdateAllViews(0, GUI_MSG_SET_RECIPE_LWC);
   return true;
   
}


void CMDIFrame::OnRemoveiADCLinkClassifier()
{
   LMSetupDoc* doc = static_cast<LMSetupDoc*>(document_);
   // remove both iADC Link Page and iADC BLOB
   CSnapFrame* frm = snap_frames_.FindPage(_T("iADC Link"));
   if( frm)
   {
      // Send recipe to framework
      if( doc->IsFeatureLicensed(LF_RBC_PEO))
         doc->UpdateAllViews(NULL, GUI_MSG_GET_RECIPE_LWC);
      // Remove iADC Link page
      snap_frames_.RemovePage(frm);
   }
  
   doc->GetADC(RT_IADC31).RemoveLinkBlob();

   // restore normal iADC tab;
   frm = snap_frames_.FindPage(_T("iADC"));
   if( frm)
   {
      snap_frames_.SelectPage(frm);
      return;
   }
   //else
   //{
   //   AddPage(_T("iADC"));
   //}

   // re create iADC page
   // Based on licensing, iADC page can also have RBC PEO
   // Refer to GetPaneLayoutInfo( page) for pane layout and page number
   if (!AddPage(GetPaneLayoutInfo(doc->IsFeatureLicensed(LF_RBC_PEO) ? TAB_iADC : TAB_iADC_WITHOUT_RBC), RT_IADC31))
	   return;// false;

   CRect rect;
   GetClientRect(rect);
   snap_frames_.ResizeWindowsAndHide(rect);
   // send initial update to all views (and other controls) in the frame
   CSnapFrame* page = snap_frames_.FindPage(_T("iADC"));
   if( page)
   {
      page->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
	   snap_frames_.SelectPage(page);
   }

   // update RBC PEO
   if( doc->IsFeatureLicensed(LF_RBC_PEO))
      doc->UpdateAllViews(0, GUI_MSG_SET_RECIPE_LWC);
   tabs_wnd_.Refresh(true);
   doc->UpdateAllViews(0, GUI_MSG_REMOVE_iADC_LINK_CLASSIFIER);
}


void CMDIFrame::OnUpdateRemoveiADCLinkClassifier(CCmdUI* cmd_ui)
{
   CSnapFrame* frm = snap_frames_.FindPage(_T("iADC Link"));
   cmd_ui->Enable( frm != 0);
}
*/


bool CMDIFrame::RemovePage(const TCHAR* title)
{
	if (CSnapFrame* frm= snap_frames_.FindPage(title))
	{
		snap_frames_.RemovePage(frm);

		tabs_wnd_.Refresh(true);

		return true;
	}

	return false;
}


void CMDIFrame::OnMDIActivate(BOOL activate, CWnd *activate_wnd, CWnd *deactivate_wnd)
{
	if (activate)
		if (CMainFrame* wnd= dynamic_cast<CMainFrame*>(AfxGetMainWnd()))
			wnd->TopLevelTabChanged(GetCurrentPageIndex(), GetCurrentPage()->GetTabTitle());

	CMDIChildWnd::OnMDIActivate(activate, activate_wnd, deactivate_wnd);
}


void CMDIFrame::PageSelected(int page_index, const CString& title)
{
	if (CMainFrame* wnd= dynamic_cast<CMainFrame*>(AfxGetMainWnd()))
		wnd->TopLevelTabChanged(page_index, title);

	if (LMSetupDoc* doc = dynamic_cast<LMSetupDoc*>(document_))
		doc->TopLevelTabChanged(page_index, title);
}


CSnapFrame* CMDIFrame::GetPageFrame(int index)
{
	if (this == 0 || index < 0 || index >= snap_frames_.GetCount())
		return 0;

	return snap_frames_.GetPage(index);
}
