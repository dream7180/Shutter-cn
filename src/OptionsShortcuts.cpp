/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsShortcuts.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "BrowserFrame.h"
#include "OptionsShortcuts.h"
#include "Accelerator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsShortcuts property page

IMPLEMENT_DYNCREATE(OptionsShortcuts, CPropertyPage)

OptionsShortcuts::OptionsShortcuts() : RPropertyPage(OptionsShortcuts::IDD)
{
	//{{AFX_DATA_INIT(OptionsShortcuts)
	//}}AFX_DATA_INIT
	modified_ = false;
}


OptionsShortcuts::~OptionsShortcuts()
{
}


void OptionsShortcuts::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	//{{AFX_DATA_MAP(OptionsShortcuts)
	DDX_Control(DX, IDC_ASSIGN, btn_assign_);
	DDX_Control(DX, IDC_HOTKEY, ctrl_hot_key_);
	DDX_Control(DX, IDC_TREE, ctrl_tree_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionsShortcuts, RPropertyPage)
	//{{AFX_MSG_MAP(OptionsShortcuts)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnSelChanged)
	ON_BN_CLICKED(IDC_ASSIGN, OnAssign)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsShortcuts message handlers


BOOL OptionsShortcuts::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	arr_accel_.reserve(100);
	CollectCommands();

	//  MBmpButton::SubclassBmpButtons(m_hWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


///////////////////////////////////////////////////////////////////////////////
// Wype³nienie drzewa 'ctrl_tree_' opisami operacji i ich kodami

void OptionsShortcuts::CollectCommands()
{
	ctrl_tree_.DeleteAllItems();
	image_list_.Create(IDB_SHORTCUTS, 16, 0, RGB(255,0,0));
	ctrl_tree_.SetImageList(&image_list_, TVSIL_NORMAL);

	BrowserFrame* main_frame= dynamic_cast<BrowserFrame*>(AfxGetMainWnd());
	ASSERT(main_frame);
	if (main_frame)
	{
		HMENU menu= main_frame->GetMenu();
		if (menu == NULL)
			return;

		EnumMenuItems(menu, TVI_ROOT);

		AddEmbededCommands();

		const Accelerator& accel= main_frame->GetAccelerator();
		CopyEntries(&accel);
	}
}


///////////////////////////////////////////////////////////////////////////////
// Spacer po wszystkich pozycjach menu g³ównego

void OptionsShortcuts::EnumMenuItems(HMENU hmenu, HTREEITEM parent)
{
	CMenu menu;
	try
	{
		menu.Attach(hmenu);
		EnumMenuItems(menu, parent);
	}
	catch (...)
	{}
	menu.Detach();
}


void OptionsShortcuts::EnumMenuItems(CMenu& menu, HTREEITEM parent)
{
	int count= menu.GetMenuItemCount();
	for (int i= 0; i < count; i++)
	{
		int id_cmd= menu.GetMenuItemID(i);
		if (id_cmd == 0)                    // separator?
			continue;

		CMenu* sub_menu= menu.GetSubMenu(i);

		CString item_str;
		menu.GetMenuString(i, item_str, MF_BYPOSITION);
		CString buffer;
		int len= item_str.GetLength();
		const TCHAR* item= item_str;
		TCHAR* buf= buffer.GetBufferSetLength(len + 1);
		for (int j= 0; j < len; j++)       // usuniêcie znaku '&' z napisów
		{
			if (*item != _T('&'))
				*buf++ = *item;
			item++;
		}
		*buf = 0;
		buffer.ReleaseBuffer();

		HTREEITEM hitem= ctrl_tree_.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE,
			buffer.SpanExcluding(_T("\t")),
			sub_menu ? 0 : 1,		// image
			sub_menu ? 0 : 1,		// selected image
			0,0, id_cmd, parent, TVI_LAST);

		ACCEL accel;
		accel.fVirt = 0;
		accel.key = 0;
		accel.cmd = WORD(id_cmd);
		arr_accel_.push_back(accel);

		if (sub_menu)
			EnumMenuItems(*sub_menu, hitem);
	}
}


///////////////////////////////////////////////////////////////////////////////
// Dopisanie do drzewa komend niedostêpnych z menu

void OptionsShortcuts::AddEmbededCommands()
{
	static const WORD awIDs[]=
	{
		//    ID_WINDOW_CASCADE,
		//    ID_WINDOW_TILE_HORZ,
		//    ID_WINDOW_TILE_VERT,
		ID_CONTEXT_HELP,
		ID_HELP_INDEX,
		ID_HELP_USING,
		0
	};

	HTREEITEM parent= ctrl_tree_.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE,
		_T("Inne operacje"),
		2,		// image
		2,		// selected image
		0,0, -1, TVI_ROOT, TVI_LAST);

	for (int i=0 ; awIDs[i]; i++)
	{
		CString desc;
		desc.LoadString(awIDs[i]);
		CString buf;
		AfxExtractSubString(buf, desc, 1);
		ASSERT(!buf.IsEmpty());  // brak opisu podpowiedzi (tooltips) do komendy 'awIDs[i]'

		HTREEITEM item= ctrl_tree_.InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE,
			buf,
			3,		// image
			3,		// selected image
			0,0, awIDs[i], parent, TVI_LAST);

		ACCEL accel;
		accel.fVirt = 0;
		accel.key = 0;
		accel.cmd = awIDs[i];
		arr_accel_.push_back(accel);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Wybrana nowa pozycja w drzewie 'ctrl_tree_'

void OptionsShortcuts::OnSelChanged(NMHDR* nmhdr, LRESULT* result)
{
	NM_TREEVIEW* NM_tree_view = (NM_TREEVIEW*)nmhdr;

	HTREEITEM sel_item= ctrl_tree_.GetSelectedItem();
	if (sel_item == NULL)
	{
		SetDlgItemText(IDC_DESCRIPTION, _T(""));
		SetDlgItemText(IDC_SHORTCUT, _T(""));
		ctrl_hot_key_.SetHotKey(0, 0);
		return;
	}
	WORD id_cmd= static_cast<WORD>(ctrl_tree_.GetItemData(sel_item));
	if (ACCEL* accel= FindAccel(id_cmd))
	{
		ctrl_hot_key_.EnableWindow(true);
		btn_assign_.EnableWindow(true);

		WORD modifiers= accel->key >= VK_F1 ? 0 : HOTKEYF_EXT;
		if (accel->fVirt & FSHIFT)
			modifiers |= HOTKEYF_SHIFT;
		if (accel->fVirt & FCONTROL)
			modifiers |= HOTKEYF_CONTROL;
		if (accel->fVirt & FALT)
			modifiers |= HOTKEYF_ALT;
		ctrl_hot_key_.SetHotKey(accel->key, modifiers);
		SetDlgItemText(IDC_SHORTCUT, Accelerator::KeyName(*accel).c_str());

		CString descr;			// opis wybranej komendy
		if (descr.LoadString(id_cmd))
		{
			CString buf;
			AfxExtractSubString(buf, descr, 0);
			SetDlgItemText(IDC_DESCRIPTION, buf);
		}
		else
			SetDlgItemText(IDC_DESCRIPTION, _T(""));
	}
	else
	{
		ctrl_hot_key_.EnableWindow(false);
		ctrl_hot_key_.SetHotKey(0, 0);
		btn_assign_.EnableWindow(false);
		SetDlgItemText(IDC_DESCRIPTION, _T(""));
		SetDlgItemText(IDC_SHORTCUT, _T(""));
	}

	*result = 0;
}


///////////////////////////////////////////////////////////////////////////////
// Odszukanie wpisu akceleratora odp. podanej komendzie

ACCEL* OptionsShortcuts::FindAccel(WORD id_cmd)
{
	if (id_cmd == WORD(0xFFFF))
		return NULL;

	for (int i= 0; i < arr_accel_.size(); i++)
		if (arr_accel_[i].cmd == id_cmd)
			return &arr_accel_[i];

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Odszukanie wpisu akceleratora dla aktualnie wybranej pozycji z drzewa 'ctrl_tree_'

ACCEL* OptionsShortcuts::FindCurSelAccel()
{
	HTREEITEM sel_item= ctrl_tree_.GetSelectedItem();
	if (sel_item == NULL)
		return NULL;
	int id_cmd= static_cast<int>(ctrl_tree_.GetItemData(sel_item));
	return FindAccel(id_cmd);
}


///////////////////////////////////////////////////////////////////////////////
// Przypisanie (zmiana) skrótu klawiaturowego dla wybranej pozycji z drzewa

void OptionsShortcuts::OnAssign()
{
	if (ACCEL* accel= FindCurSelAccel())
	{
		ACCEL Accel;
		WORD virt_key, modifiers;
		ctrl_hot_key_.GetHotKey(virt_key, modifiers);
		if (accel->key != virt_key)
			modified_ = true;
		Accel.key = virt_key;
		BYTE fVirt= FVIRTKEY | FNOINVERT;
		if (modifiers & HOTKEYF_SHIFT)
			fVirt |= FSHIFT;
		if (modifiers & HOTKEYF_CONTROL)
			fVirt |= FCONTROL;
		if (modifiers & HOTKEYF_ALT)
			fVirt |= FALT;
		if (accel->fVirt != fVirt)
			modified_ = true;
		Accel.fVirt = fVirt;
		if (virt_key)
			if (ACCEL* old= CheckDuplicate(accel, Accel))
			{
				if (AfxMessageBox(_T("Podana kombinacja klawiszy jest ju¿ stosowana.\nCzy pomimo to u¿yæ jej?"),
					MB_YESNO|MB_ICONQUESTION) != IDYES)
					return;
				old->key = 0;
				old->fVirt = 0;
				modified_ = true;
			}
			accel->key = Accel.key;
			accel->fVirt = Accel.fVirt;
			SetDlgItemText(IDC_SHORTCUT, Accelerator::KeyName(*accel).c_str());
	}
}


///////////////////////////////////////////////////////////////////////////////
// Spr. czy dany shortcut nie jest ju¿ u¿ywany

ACCEL* OptionsShortcuts::CheckDuplicate(ACCEL* accel, ACCEL& accel_new)
{
	for (int i= 0; i < arr_accel_.size(); i++)
	{
		const ACCEL& crAccel= arr_accel_[i];
		if (crAccel.key == accel_new.key &&
			crAccel.fVirt == accel_new.fVirt &&
			&crAccel != accel)
			return &arr_accel_[i];
	}

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Skopiowanie danych z akceleratora 'accel' do tablicy 'arr_accel_'

void OptionsShortcuts::CopyEntries(const Accelerator* accel)
{
	std::vector<ACCEL> accels;
	accel->GetSortedData(accels);

	for (int i= 0; i < arr_accel_.size(); i++)
		if (ACCEL* accel= Accelerator::BinarySearch(accels, arr_accel_[i].cmd))
		{
			//accel = MainFrame::FindCmd(accel, arr_accel_[i].cmd);
			arr_accel_[i] = *accel;
		}
}


///////////////////////////////////////////////////////////////////////////////
// Modyfikacja u¿ywanego w programie akceleratora, zgodnie ze zmianami usera

bool OptionsShortcuts::ModifyAccel()
{
	if (IsModified())
	{
		MainFrame* main_frame= dynamic_cast<MainFrame*>(AfxGetMainWnd());
		ASSERT(main_frame);
		if (main_frame)
		{
			/*      arr_accel_.Add(const_cast<ACCEL&>(MainFrame::accel_cut_));
			arr_accel_.Add(const_cast<ACCEL&>(MainFrame::accel_copy_));
			arr_accel_.Add(const_cast<ACCEL&>(MainFrame::accel_paste_));
			arr_accel_.Add(const_cast<ACCEL&>(MainFrame::accel_undo_));
			Accelerator accel;
			accel.Create(arr_accel_.GetData(), arr_accel_.GetSize());
			main_frame->GetAccelerator()->Attach(accel, false);
			main_frame->accel_table_ = *main_frame->GetAccelerator();
			accel.accel_ = NULL;
			main_frame->ModifyMenuShortcutsDescription();
			return true; */
		}
	}
	return false;
}
