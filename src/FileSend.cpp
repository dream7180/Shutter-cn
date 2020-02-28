/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "FileSend.h"
#pragma warning(disable: 4228)
#include <mapi.h>
#pragma warning(default: 4228)
#include <shlwapi.h>

#define _countof(array) (sizeof(array)/sizeof(array[0]))

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

AFX_STATIC_DATA BOOL g_afxIsMailAvail= (BOOL)-1;    // start out not determined

/////////////////////////////////////////////////////////////////////////////
// _AFX_MAIL_STATE

class _AFX_MAIL_STATE : public CNoTrackObject
{
public:
	HINSTANCE inst_mail_;      // handle to MAPI32.DLL
	virtual ~_AFX_MAIL_STATE();
};

_AFX_MAIL_STATE::~_AFX_MAIL_STATE()
{
	if (inst_mail_ != NULL)
		::FreeLibrary(inst_mail_);
}

EXTERN_PROCESS_LOCAL(_AFX_MAIL_STATE, g_afxMailState)

////////////////////////////////////////////////////////////////////////////

FileSend::FileSend()
{
	if (g_afxIsMailAvail == (BOOL)-1)
	{
		g_afxIsMailAvail = ::GetProfileInt(_T("MAIL"), _T("MAPI"), 0) != 0 &&
			SearchPath(NULL, _T("MAPI32.DLL"), NULL, 0, NULL, NULL) != 0;
	}
}

FileSend::~FileSend()
{
}


void FileSend::SendFile(const TCHAR* filename)
{
	ASSERT(filename && *filename);

	if (!IsSendCmdAvailable())
		return;

	//path_name_ = filename;
	FileSendMail(filename);
}


bool FileSend::IsSendCmdAvailable()
{
	return g_afxIsMailAvail != 0;
/*
	struct CUI : public CCmdUI
	{
		CUI() : enabled_(false) {}

		virtual void Enable(BOOL on= TRUE)	{ enabled_ = !!on; }

		bool enabled_;
	};

	CUI cui;
	OnUpdateFileSendMail(&cui);

	return cui.enabled_; */
}


typedef ULONG (PASCAL* SEND_MAIL_FN)(ULONG, ULONG_PTR, MapiMessage*, FLAGS, ULONG);


SEND_MAIL_FN GetSendMailFunction()
{
	ASSERT(g_afxIsMailAvail);   // update handler always gets called first

	CWaitCursor wait;

	_AFX_MAIL_STATE* mail_state = g_afxMailState;
	if (mail_state->inst_mail_ == NULL)
		mail_state->inst_mail_ = ::LoadLibraryA("MAPI32.DLL");

	if (mail_state->inst_mail_ == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_MAPI_LOAD);
		return 0;
	}
	ASSERT(mail_state->inst_mail_ != NULL);

	ULONG (PASCAL *fnSendMail)(ULONG, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
	(FARPROC&)fnSendMail = GetProcAddress(mail_state->inst_mail_, "MAPISendMail");
	if (fnSendMail == NULL)
	{
		AfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
		return 0;
	}
	ASSERT(fnSendMail != NULL);

	return fnSendMail;
}


void FileSend::SendFiles(const std::vector<Path>& files, const std::vector<String>& names, const String* msg, const char* subject, const char* dest_addr)
{
	ASSERT(files.size() == names.size());

	const size_t count= files.size();

	//if (count == 0)
	//	return;

	SEND_MAIL_FN fnSendMail= GetSendMailFunction();

	if (fnSendMail == 0)
		return;

	std::vector<std::string> filePaths(count);
	std::vector<std::string> fileNames(count);
	std::vector<MapiFileDesc> desc(count);

	for (size_t i= 0; i < count; ++i)
	{
		char path[MAX_PATH];
		char name[MAX_PATH];
#ifdef _UNICODE
		_wcstombsz(path, files[i].c_str(), _countof(path));
		_wcstombsz(name, names[i].c_str(), _countof(name));
#else
		strcpy(path, files[i].c_str());
		strcpy(name, names[i].c_str());
#endif
		filePaths[i] = path;
		fileNames[i] = name;

		MapiFileDesc& fileDesc= desc[i];

		memset(&fileDesc, 0, sizeof(fileDesc));
		fileDesc.nPosition = (ULONG)-1;
		fileDesc.lpszPathName = const_cast<char*>(filePaths[i].c_str());
		fileDesc.lpszFileName = const_cast<char*>(fileNames[i].c_str());
	}

	// prepare the message (empty with attachments)
	MapiMessage message;
	memset(&message, 0, sizeof(message));
	message.nFileCount = static_cast<ULONG>(desc.size());
	message.lpFiles = &desc.front();

	MapiRecipDesc addr;
	memset(&addr, 0, sizeof(addr));
	std::vector<char> addr_buf;

	if (dest_addr && *dest_addr)
	{
		addr_buf.assign(dest_addr, dest_addr + strlen(dest_addr) + 1);
		addr.lpszName = &addr_buf.front();
		addr.ulRecipClass = MAPI_TO;
		message.lpRecips = &addr;
		message.nRecipCount = 1;
	}

	std::vector<char> subj_buf;
	if (subject && *subject)
	{
		subj_buf.assign(subject, subject + strlen(subject) + 1);
		message.lpszSubject = &subj_buf.front();
	}

	std::string note;
	if (msg && !msg->empty())
	{
#ifdef _UNICODE
		note.resize(msg->size() * 2);
		_wcstombsz(&note[0], msg->c_str(), static_cast<ULONG>(note.size()));
#else
		note = *msg;
#endif
		message.lpszNoteText = const_cast<char*>(note.c_str());
	}

	// prepare for modal dialog box
	AfxGetApp()->EnableModeless(FALSE);
	HWND wnd_top= 0;
	CWnd* parent_wnd= CWnd::GetSafeOwner(NULL, &wnd_top);

	// some extra precautions are required to use MAPISendMail as it
	// tends to enable the parent window in between dialogs (after
	// the login dialog, but before the send note dialog).
	parent_wnd->SetCapture();
	::SetFocus(NULL);
	parent_wnd->m_nFlags |= WF_STAYDISABLED;

	int error= fnSendMail(0, (ULONG_PTR)parent_wnd->GetSafeHwnd(), &message, MAPI_LOGON_UI | MAPI_DIALOG, 0);

	// after returning from the MAPISendMail call, the window must
	// be re-enabled and focus returned to the frame to undo the workaround
	// done before the MAPI call.
	::ReleaseCapture();
	parent_wnd->m_nFlags &= ~WF_STAYDISABLED;

	parent_wnd->EnableWindow(TRUE);
	::SetActiveWindow(NULL);
//	parent_wnd->SetActiveWindow();
//	parent_wnd->SetFocus();
	if (wnd_top != NULL)
		::EnableWindow(wnd_top, TRUE);
	AfxGetApp()->EnableModeless(TRUE);

	if (error != SUCCESS_SUCCESS && error != MAPI_USER_ABORT && error != MAPI_E_LOGIN_FAILURE)
	{
		TCHAR* msg= 0;
		switch (error)
		{
		case MAPI_E_FAILURE:
			msg = _T("失败");
			break;
		case MAPI_E_LOGIN_FAILURE:
			msg = _T("登录失败");
			break;
		case MAPI_E_DISK_FULL:
			msg = _T("磁盘已满");
			break;
		case MAPI_E_INSUFFICIENT_MEMORY:
			msg = _T("内存不足");
			break;
		case MAPI_E_ACCESS_DENIED:
			msg = _T("拒绝访问");
			break;
		case MAPI_E_TOO_MANY_SESSIONS:
			msg = _T("会话太多");
			break;
		case MAPI_E_TOO_MANY_FILES:
			msg = _T("文件太多");
			break;
		case MAPI_E_TOO_MANY_RECIPIENTS:
			msg = _T("收件人太多");
			break;
		case MAPI_E_ATTACHMENT_NOT_FOUND:
			msg = _T("未找到附件");
			break;
		case MAPI_E_ATTACHMENT_OPEN_FAILURE:
			msg = _T("附件打开失败");
			break;
		case MAPI_E_ATTACHMENT_WRITE_FAILURE:
			msg = _T("失败");
			break;
		case MAPI_E_UNKNOWN_RECIPIENT:
			msg = _T("未知收件人");
			break;
		case MAPI_E_BAD_RECIPTYPE:
			msg = _T("错误的收件人");
			break;
		case MAPI_E_NO_MESSAGES:
			msg = _T("无消息");
			break;
		case MAPI_E_INVALID_MESSAGE:
			msg = _T("无效的消息");
			break;
		case MAPI_E_TEXT_TOO_LARGE:
			msg = _T("文本太大");
			break;
		case MAPI_E_INVALID_SESSION:
			msg = _T("无效的会话");
			break;
		case MAPI_E_TYPE_NOT_SUPPORTED:
			msg = _T("类型不支持");
			break;
		case MAPI_E_AMBIGUOUS_RECIPIENT:
			msg = _T("收件人不明确");
			break;
		case MAPI_E_MESSAGE_IN_USE:
			msg = _T("消息正在使用");
			break;
		case MAPI_E_NETWORK_FAILURE:
			msg = _T("网络错误");
			break;
		case MAPI_E_INVALID_EDITFIELDS:
			msg = _T("无效的编辑字段");
			break;
		case MAPI_E_INVALID_RECIPS:
			msg = _T("无效的方法");
			break;
		case MAPI_E_NOT_SUPPORTED:
			msg = _T("不支持");
			break;
		default:
			msg = _T("未知错误");
			break;
		}

		TCHAR err[200];
		wsprintf(err, _T("发送邮件错误, 信息为 '%s', code %d."), msg, error);

		AfxMessageBox(err, MB_OK | MB_ICONERROR);

//		AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
	}
}



UINT AFXAPI AfxGetFileName(LPCTSTR path_name, LPTSTR title, UINT max);


//#ifdef AFX_MAPI_SEG
//#pragma code_seg(AFX_MAPI_SEG)
//#endif


/////////////////////////////////////////////////////////////////////////////
// MAPI implementation helpers and globals


/////////////////////////////////////////////////////////////////////////////
// CDocument MAPI support

void FileSend::FileSendMail(const TCHAR* file)
{
	CString path_name= file;
//	ASSERT_VALID(this);
	ASSERT(g_afxIsMailAvail);   // update handler always gets called first
/*
	CWaitCursor wait;

	_AFX_MAIL_STATE* mail_state = g_afxMailState;
	if (mail_state->inst_mail_ == NULL)
		mail_state->inst_mail_ = ::LoadLibraryA("MAPI32.DLL");

	if (mail_state->inst_mail_ == NULL)
	{
		AfxMessageBox(AFX_IDP_FAILED_MAPI_LOAD);
		return;
	}
	ASSERT(mail_state->inst_mail_ != NULL);

	ULONG (PASCAL *fnSendMail)(ULONG, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
	(FARPROC&)fnSendMail = GetProcAddress(mail_state->inst_mail_, "MAPISendMail");
	if (fnSendMail == NULL)
	{
		AfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
		return;
	}
	ASSERT(fnSendMail != NULL);
*/
	SEND_MAIL_FN fnSendMail= GetSendMailFunction();

	TCHAR temp_name[_MAX_PATH];
	//TCHAR path[_MAX_PATH];
//	BOOL remove_temp = FALSE;
/*	if (path_name.IsEmpty())// || IsModified())
	{
		// save to temporary path
		VERIFY(GetTempPath(_countof(path), path) != 0);
		VERIFY(GetTempFileName(path, _T("afx"), 0, temp_name) != 0);

		// save it, but remember original modified flag
		BOOL modified = IsModified();
		BOOL result = DoSave(temp_name, FALSE);
		SetModifiedFlag(modified);
		if (!result)
		{
			TRACE(traceAppMsg, 0, "Warning: file save failed during File.Send Mail.\n");
			return;
		}
		remove_temp = TRUE;
	}
	else */
	{
		// use actual file since it isn't modified
		lstrcpyn(temp_name, path_name, _countof(temp_name));
	}
#ifdef _UNICODE
	char temp_name_a[_MAX_PATH];
	_wcstombsz(temp_name_a, temp_name, _countof(temp_name_a));
#endif

	// build an appropriate title for the attachment
	TCHAR title[_MAX_PATH];
	if (!path_name.IsEmpty())
		AfxGetFileName(path_name, title, _countof(title));
	else
	{
		ASSERT(false);
/*
		lstrcpyn(title, title_, _countof(title));
		if (::PathFindExtension(title_) == NULL) // no extension
		{
			// append the default suffix if there is one
			CString ext;
			CDocTemplate* a_template = GetDocTemplate();
			if (a_template != NULL &&
				a_template->GetDocString(ext, CDocTemplate::filterExt))
			{
				if( lstrlen(title) + lstrlen(ext) < _countof(title) )
				{
					lstrcat(title, ext);
				}
			}
		} */
	}

#ifdef _UNICODE
	char title_a[_MAX_PATH];
	_wcstombsz(title_a, title, _countof(title_a));
#endif

	// prepare the file description (for the attachment)
	MapiFileDesc fileDesc;
	memset(&fileDesc, 0, sizeof(fileDesc));
	fileDesc.nPosition = (ULONG)-1;
//fileDesc.position = 1;
#ifdef _UNICODE
	fileDesc.lpszPathName = temp_name_a;
	fileDesc.lpszFileName = title_a;
#else
	fileDesc.lpszPathName = temp_name;
	fileDesc.lpszFileName = title;
#endif
//fileDesc.flFlags = MAPI_OLE;

	// prepare the message (empty with 1 attachment)
	MapiMessage message;
	memset(&message, 0, sizeof(message));
	message.nFileCount = 1;
	message.lpFiles = &fileDesc;
//message.note_text = "test";

	// prepare for modal dialog box
	AfxGetApp()->EnableModeless(FALSE);
	HWND wnd_top;
	CWnd* parent_wnd = CWnd::GetSafeOwner(NULL, &wnd_top);

	// some extra precautions are required to use MAPISendMail as it
	// tends to enable the parent window in between dialogs (after
	// the login dialog, but before the send note dialog).
	parent_wnd->SetCapture();
	::SetFocus(NULL);
	parent_wnd->m_nFlags |= WF_STAYDISABLED;

	int error= fnSendMail(0, (ULONG_PTR)parent_wnd->GetSafeHwnd(), &message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

	// after returning from the MAPISendMail call, the window must
	// be re-enabled and focus returned to the frame to undo the workaround
	// done before the MAPI call.
	::ReleaseCapture();
	parent_wnd->m_nFlags &= ~WF_STAYDISABLED;

	parent_wnd->EnableWindow(TRUE);
	::SetActiveWindow(NULL);
	parent_wnd->SetActiveWindow();
	parent_wnd->SetFocus();
	if (wnd_top != NULL)
		::EnableWindow(wnd_top, TRUE);
	AfxGetApp()->EnableModeless(TRUE);

	if (error != SUCCESS_SUCCESS && error != MAPI_USER_ABORT && error != MAPI_E_LOGIN_FAILURE)
	{
		TCHAR* msg= 0;
		switch (error)
		{
		case MAPI_E_FAILURE:
			msg = _T("failure");
			break;
		case MAPI_E_LOGIN_FAILURE:
			msg = _T("login failure");
			break;
		case MAPI_E_DISK_FULL:
			msg = _T("disk full");
			break;
		case MAPI_E_INSUFFICIENT_MEMORY:
			msg = _T("insufficient memory");
			break;
		case MAPI_E_ACCESS_DENIED:
			msg = _T("access denied");
			break;
		case MAPI_E_TOO_MANY_SESSIONS:
			msg = _T("too many sessions");
			break;
		case MAPI_E_TOO_MANY_FILES:
			msg = _T("too many files");
			break;
		case MAPI_E_TOO_MANY_RECIPIENTS:
			msg = _T("too many recipients");
			break;
		case MAPI_E_ATTACHMENT_NOT_FOUND:
			msg = _T("attachment not found");
			break;
		case MAPI_E_ATTACHMENT_OPEN_FAILURE:
			msg = _T("attachment open failure");
			break;
		case MAPI_E_ATTACHMENT_WRITE_FAILURE:
			msg = _T("failure");
			break;
		case MAPI_E_UNKNOWN_RECIPIENT:
			msg = _T("unknown recipient");
			break;
		case MAPI_E_BAD_RECIPTYPE:
			msg = _T("bad recipient");
			break;
		case MAPI_E_NO_MESSAGES:
			msg = _T("no messages");
			break;
		case MAPI_E_INVALID_MESSAGE:
			msg = _T("invalid message");
			break;
		case MAPI_E_TEXT_TOO_LARGE:
			msg = _T("text too large");
			break;
		case MAPI_E_INVALID_SESSION:
			msg = _T("invalid session");
			break;
		case MAPI_E_TYPE_NOT_SUPPORTED:
			msg = _T("type not supported");
			break;
		case MAPI_E_AMBIGUOUS_RECIPIENT:
			msg = _T("ambiguous recipient");
			break;
		case MAPI_E_MESSAGE_IN_USE:
			msg = _T("message in use");
			break;
		case MAPI_E_NETWORK_FAILURE:
			msg = _T("network failure");
			break;
		case MAPI_E_INVALID_EDITFIELDS:
			msg = _T("invalid edit fields");
			break;
		case MAPI_E_INVALID_RECIPS:
			msg = _T("invalid recips");
			break;
		case MAPI_E_NOT_SUPPORTED:
			msg = _T("not supported");
			break;
		default:
			msg = _T("unknown failure");
			break;
		}

		TCHAR err[200];
		wsprintf(err, _T("Sending mail failed with message '%s', code %d."), msg, error);

		AfxMessageBox(err, MB_OK | MB_ICONERROR);
//		AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
	}

	// remove temporary file, if temporary file was used
//	if (remove_temp)
//		CFile::Remove(temp_name);
}

/*
void FileSend::OnUpdateFileSendMail(CCmdUI* cmd_ui)
{
//	ASSERT_VALID(this);

	if (g_afxIsMailAvail == (BOOL)-1)
	{
		g_afxIsMailAvail = ::GetProfileInt(_T("MAIL"), _T("MAPI"), 0) != 0 &&
			SearchPath(NULL, _T("MAPI32.DLL"), NULL, 0, NULL, NULL) != 0;
	}

	// enable the Send... menu item if available
	cmd_ui->Enable(g_afxIsMailAvail);
	//CMenu* menu = cmd_ui->menu_;
	//if (!g_afxIsMailAvail && menu != NULL)
	//{
	//	// remove the Send... menu and surrounding separators
	//	UINT state_above = menu->GetMenuState(cmd_ui->index_-1, MF_BYPOSITION);
	//	UINT state_below = menu->GetMenuState(cmd_ui->index_+1, MF_BYPOSITION);
	//	menu->RemoveMenu(cmd_ui->index_, MF_BYPOSITION);
	//	if (state_above & state_below & MF_SEPARATOR)
	//	{
	//		// a separator must be removed since the Send... is gone
	//		if (state_above != (UINT)-1)
	//			menu->RemoveMenu(cmd_ui->index_-1, MF_BYPOSITION);
	//		else if (state_below != (UINT)-1)
	//			menu->RemoveMenu(cmd_ui->index_, MF_BYPOSITION);
	//	}
	//}
}


/////////////////////////////////////////////////////////////////////////////
// COleDocument MAPI support

void COleDocument::OnFileSendMail()
{
	ASSERT_VALID(this);
	ASSERT(remember_);

	LPSTORAGE orig_stg = root_stg_;
	root_stg_ = NULL;

	TRY
	{
		remember_ = FALSE;
		CDocument::OnFileSendMail();
	}
	CATCH_ALL(e)
	{
		root_stg_ = orig_stg;
		remember_ = TRUE;
		THROW_LAST();
	}
	END_CATCH_ALL

	root_stg_ = orig_stg;
	remember_ = TRUE;
}
*/
//#ifdef AFX_INIT_SEG
//#pragma code_seg(AFX_INIT_SEG)
//#endif

#pragma warning(disable: 4074)
//#pragma init_seg(lib)

PROCESS_LOCAL(_AFX_MAIL_STATE, g_afxMailState)

/////////////////////////////////////////////////////////////////////////////
