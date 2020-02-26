/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_SHORTCUTSPAGE_H__D1A36241_97DC_11D1_A91F_444553540000__INCLUDED_)
#define AFX_SHORTCUTSPAGE_H__D1A36241_97DC_11D1_A91F_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ShortcutsPage.h : header file
//
class Accelerator;
#include "RPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// OptionsShortcuts dialog

class OptionsShortcuts : public RPropertyPage
{
  CImageList image_list_;
  void CollectCommands();
  void EnumMenuItems(HMENU menu, HTREEITEM parent);
  void EnumMenuItems(CMenu& menu, HTREEITEM parent);
  void AddEmbededCommands();
  ACCEL* FindAccel(WORD id_cmd);
  ACCEL* FindCurSelAccel();
  ACCEL* CheckDuplicate(ACCEL* accel, ACCEL& accel_new);
  bool modified_;
  void CopyEntries(const Accelerator* accel);
  typedef std::vector<ACCEL> AccelArray;
  AccelArray arr_accel_;

  DECLARE_DYNCREATE(OptionsShortcuts)

// Construction
public:
  OptionsShortcuts();
  ~OptionsShortcuts();

// Dialog Data
  //{{AFX_DATA(OptionsShortcuts)
  enum { IDD = IDD_OPTIONS_SHORTCUTS };
  CButton    btn_assign_;
  CHotKeyCtrl ctrl_hot_key_;
  CTreeCtrl ctrl_tree_;
  //}}AFX_DATA


// Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(OptionsShortcuts)
protected:
  virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  bool IsModified() { return modified_; }

public:
  bool ModifyAccel();

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(OptionsShortcuts)
  virtual BOOL OnInitDialog();
  afx_msg void OnSelChanged(NMHDR* nmhdr, LRESULT* result);
  afx_msg void OnAssign();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHORTCUTSPAGE_H__D1A36241_97DC_11D1_A91F_444553540000__INCLUDED_)
