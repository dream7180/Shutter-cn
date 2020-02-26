////////////////////////////////////////////////////////////////
// Copyright 1998 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CSubclassWnd is a generic class for hooking another window's messages.

#include "StdAfx.h"
#include "Subclass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////
// The message hook map is derived from CMapPtrToPtr, which associates
// a pointer with another pointer. It maps an HWND to a CSubclassWnd, like
// the way MFC's internal maps map HWND's to CWnd's. The first CSubclassWnd
// attached to a window is stored in the map; all other CSubclassWnd's for that
// window are then chained via CSubclassWnd::next_.
//
class CSubclassWndMap : private CMapPtrToPtr {
public:
	CSubclassWndMap();
	~CSubclassWndMap();
	static CSubclassWndMap& GetHookMap();
	void Add(HWND hwnd, CSubclassWnd* subclass_wnd);
	void Remove(CSubclassWnd* subclass_wnd);
	void RemoveAll(HWND hwnd);
	CSubclassWnd* Lookup(HWND hwnd);
};

// This trick is used so the hook map isn't
// instantiated until someone actually requests it.
//
#define	theHookMap	(CSubclassWndMap::GetHookMap())

IMPLEMENT_DYNAMIC(CSubclassWnd, CWnd);

CSubclassWnd::CSubclassWnd()
{
	next_ = NULL;
	old_wnd_proc_ = NULL;	
	m_hWnd  = NULL;
}

CSubclassWnd::~CSubclassWnd()
{
	if (m_hWnd)
		HookWindow((HWND)NULL);		// unhook window
}

//////////////////
// Hook a window.
// This installs a new window proc that directs messages to the CSubclassWnd.
// wnd=NULL to remove.
//
BOOL CSubclassWnd::HookWindow(HWND hwnd)
{
	ASSERT_VALID(this);
	if (hwnd) {
		// Hook the window
		ASSERT(m_hWnd==NULL);
		ASSERT(::IsWindow(hwnd));
		theHookMap.Add(hwnd, this);			// Add to map of hooks

	} else if (m_hWnd) {
		// Unhook the window
		theHookMap.Remove(this);				// Remove from map
		old_wnd_proc_ = NULL;
	}
	m_hWnd = hwnd;
	return TRUE;
}

//////////////////
// Window proc-like virtual function which specific CSubclassWnds will
// override to do stuff. Default passes the message to the next hook; 
// the last hook passes the message to the original window.
// You MUST call this at the end of your WindowProc if you want the real
// window to get the message. This is just like CWnd::WindowProc, except that
// a CSubclassWnd is not a window.
//
LRESULT CSubclassWnd::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
//	ASSERT_VALID(this);  // removed for speed
	ASSERT(old_wnd_proc_);
	return next_ ? next_->WindowProc(msg, wp, lp) :	
		::CallWindowProc(old_wnd_proc_, m_hWnd, msg, wp, lp);
}

//////////////////
// Like calling base class WindowProc, but with no args, so individual
// message handlers can do the default thing. Like CWnd::Default
//
LRESULT CSubclassWnd::Default()
{
	// MFC stores current MSG in thread state
	MSG& curMsg = AfxGetThreadState()->m_lastSentMsg;
	// Note: must explicitly call CSubclassWnd::WindowProc to avoid infinte
	// recursion on virtual function
	return CSubclassWnd::WindowProc(curMsg.message, curMsg.wParam, curMsg.lParam);
}

#ifdef _DEBUG
void CSubclassWnd::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(m_hWnd==NULL || ::IsWindow(m_hWnd));
	if (m_hWnd)
	{
		CSubclassWnd* p= 0;
		for (p = theHookMap.Lookup(m_hWnd); p; p=p->next_)
			if (p == this)
				break;

		ASSERT(p); // should have found it!
	}
}

void CSubclassWnd::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}

#endif

//////////////////
// Subclassed window proc for message hooks. Replaces AfxWndProc (or whatever
// else was there before.)
//
LRESULT CALLBACK
HookWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
#ifdef _USRDLL
	// If this is a DLL, need to set up MFC state
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

	// Set up MFC message state just in case anyone wants it
	// This is just like AfxCallWindowProc, but we can't use that because
	// a CSubclassWnd is not a CWnd.
	//
	MSG& curMsg = AfxGetThreadState()->m_lastSentMsg;
	MSG  oldMsg = curMsg;   // save for nesting
	curMsg.hwnd		= hwnd;
	curMsg.message = msg;
	curMsg.wParam  = wp;
	curMsg.lParam  = lp;

	// Get hook object for this window. Get from hook map
	CSubclassWnd* subclass_wnd = theHookMap.Lookup(hwnd);
	ASSERT(subclass_wnd);

	LRESULT lr;
	if (msg==WM_NCDESTROY) {
		// Window is being destroyed: unhook all hooks (for this window)
		// and pass msg to orginal window proc
		//
		WNDPROC wndproc = subclass_wnd->old_wnd_proc_;
		theHookMap.RemoveAll(hwnd);
		lr = ::CallWindowProc(wndproc, hwnd, msg, wp, lp);

	} else {
		// pass to msg hook
		lr = subclass_wnd->WindowProc(msg, wp, lp);
	}
	curMsg = oldMsg;			// pop state
	return lr;
}

////////////////////////////////////////////////////////////////
// CSubclassWndMap implementation
//
CSubclassWndMap::CSubclassWndMap()
{
}

CSubclassWndMap::~CSubclassWndMap()
{
// This assert bombs when posting WM_QUIT, so I've deleted it.
//	ASSERT(IsEmpty());	// all hooks should be removed!	
}

//////////////////
// Get the one and only global hook map
// 
CSubclassWndMap& CSubclassWndMap::GetHookMap()
{
	// By creating theMap here, C++ doesn't instantiate it until/unless
	// it's ever used! This is a good trick to use in C++, to
	// instantiate/initialize a static object the first time it's used.
	//
	static CSubclassWndMap theMap;
	return theMap;
}

/////////////////
// Add hook to map; i.e., associate hook with window
//
void CSubclassWndMap::Add(HWND hwnd, CSubclassWnd* subclass_wnd)
{
	ASSERT(hwnd && ::IsWindow(hwnd));

	// Add to front of list
	subclass_wnd->next_ = Lookup(hwnd);
	SetAt(hwnd, subclass_wnd);
	
	if (subclass_wnd->next_==NULL) {
		// If this is the first hook added, subclass the window
		subclass_wnd->old_wnd_proc_ = 
			(WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, LONG_PTR(HookWndProc));

	} else {
		// just copy wndproc from next hook
		subclass_wnd->old_wnd_proc_ = subclass_wnd->next_->old_wnd_proc_;
	}
	ASSERT(subclass_wnd->old_wnd_proc_);
}

//////////////////
// Remove hook from map
//
void CSubclassWndMap::Remove(CSubclassWnd* un_hook)
{
	HWND hwnd = un_hook->m_hWnd;
	ASSERT(hwnd && ::IsWindow(hwnd));

	CSubclassWnd* hook = Lookup(hwnd);
	ASSERT(hook);
	if (hook==un_hook) {
		// hook to remove is the one in the hash table: replace w/next
		if (hook->next_)
			SetAt(hwnd, hook->next_);
		else {
			// This is the last hook for this window: restore wnd proc
			RemoveKey(hwnd);
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, LONG_PTR(hook->old_wnd_proc_));
		}
	} else {
		// Hook to remove is in the middle: just remove from linked list
		while (hook->next_!=un_hook)
			hook = hook->next_;
		ASSERT(hook && hook->next_==un_hook);
		hook->next_ = un_hook->next_;
	}
}

//////////////////
// Remove all the hooks for a window
//
void CSubclassWndMap::RemoveAll(HWND hwnd)
{
	CSubclassWnd* subclass_wnd;
	while ((subclass_wnd = Lookup(hwnd))!=NULL)
		subclass_wnd->HookWindow((HWND)NULL);	// (unhook)
}

/////////////////
// Find first hook associate with window
//
CSubclassWnd* CSubclassWndMap::Lookup(HWND hwnd)
{
	CSubclassWnd* found = NULL;
	if (!CMapPtrToPtr::Lookup(hwnd, (void*&)found))
		return NULL;
	ASSERT_KINDOF(CSubclassWnd, found);
	return found;
}

