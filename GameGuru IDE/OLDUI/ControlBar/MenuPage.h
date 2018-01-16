//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

#if !defined(AFX_MENUPAGE_H__385604B4_DDC0_11D1_A64F_00A0C93A70EC__INCLUDED_)
#define AFX_MENUPAGE_H__385604B4_DDC0_11D1_A64F_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MenuPage.h : header file
//

#ifndef BCG_NO_CUSTOMIZATION

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgbarres.h"

class CBCGMenuBar;
class CBCGPopupMenu;

/////////////////////////////////////////////////////////////////////////////
// CBCGMenuPage dialog

class CBCGMenuPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CBCGMenuPage)

// Construction
public:
	CBCGMenuPage (CFrameWnd* pParentFrame = NULL, BOOL bAutoSet = FALSE);
	~CBCGMenuPage();

// Operations
public:
	void CloseContextMenu (CBCGPopupMenu* pMenu);
	BOOL SelectMenu (CDocTemplate* pTemplate, BOOL bSaveCurr = TRUE);

// Dialog Data
	//{{AFX_DATA(CBCGMenuPage)
	enum { IDD = IDD_BCGBARRES_PROPPAGE3 };
	CStatic	m_wndMenuAnimationsLabel;
	CComboBox	m_wndMenuAnimations;
	CButton	m_wndMenuShadows;
	CButton	m_wndContextFrame;
	CStatic	m_wndContextHint;
	CButton	m_wndResetMenuButton;
	CStatic	m_wndContextMenuCaption;
	CComboBox	m_wndContextMenus;
	CStatic	m_wndIcon;
	CComboBox	m_wndMenuesList;
	CString	m_strMenuDescr;
	CString	m_strContextMenuName;
	CString	m_strMenuName;
	int		m_iMenuAnimationType;
	BOOL	m_bMenuShadows;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGMenuPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGMenuPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeMenuList();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeContextMenuList();
	afx_msg void OnResetMenu();
	afx_msg void OnBcgbarresResetFrameMenu();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void SaveMenu ();

// Attributes:
protected:
	CBCGMenuBar*	m_pMenuBar;
	HMENU			m_hmenuCurr;
	HMENU			m_hmenuSelected;
	CBCGPopupMenu*	m_pContextMenu;
	UINT			m_uiContextMenuResId;

	BOOL			m_bIsDefaultMDIMenu;
	static CPoint	m_ptMenuLastPos;
	BOOL			m_bAutoSet;

	CFrameWnd*		m_pParentFrame;
};

#endif // BCG_NO_CUSTOMIZATION

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MENUPAGE_H__385604B4_DDC0_11D1_A64F_00A0C93A70EC__INCLUDED_)
