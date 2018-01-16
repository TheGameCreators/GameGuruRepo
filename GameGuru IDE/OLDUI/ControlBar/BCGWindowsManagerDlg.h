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
//
// This code is based on Ivan Zhakov (vanya@mail.axon.ru)'s
// MDI Windows Manager dialog 
// http://codeguru.developer.com/doc_view/WindowsManager.shtml
//

#if !defined(AFX_BCGWINDOWSMANAGERDLG_H__014E9113_B3C3_11D3_A712_009027900694__INCLUDED_)
#define AFX_BCGWINDOWSMANAGERDLG_H__014E9113_B3C3_11D3_A712_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGWindowsManagerDlg.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "bcgbarres.h"

class CBCGMDIFrameWnd;


BCGCONTROLBARDLLEXPORT extern UINT WM_BCGWINDOW_HELP; //WDW
/////////////////////////////////////////////////////////////////////////////
// CBCGWindowsManagerDlg dialog

class BCGCONTROLBARDLLEXPORT CBCGWindowsManagerDlg : public CDialog
{
// Construction
public:
	CBCGWindowsManagerDlg(CBCGMDIFrameWnd* pMDIFrame, BOOL bHelpButton = FALSE);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGWindowsManagerDlg)
	enum { IDD = IDD_BCGBARRES_WINDOWS_DLG };
	CListBox	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGWindowsManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void			MDIMessage(UINT uMsg,WPARAM flag);
	void			FillWindowList(void);
	void			SelActive(void);
	void			UpdateButtons(void);

	CBCGMDIFrameWnd*	m_pMDIFrame;
	const BOOL			m_bHelpButton;

	BOOL				m_bMDIActions;
	CList<HWND,HWND>	m_lstCloseDisabled;

	// Generated message map functions
	//{{AFX_MSG(CBCGWindowsManagerDlg)
	afx_msg void OnActivate();
	afx_msg void OnSave();
	afx_msg void OnClose();
	afx_msg void OnCascade();
	afx_msg void OnTilehorz();
	afx_msg void OnTilevert();
	afx_msg void OnMinimize();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBcgbarresList();
	afx_msg void OnBcgbarresWindowHelp();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGWINDOWSMANAGERDLG_H__014E9113_B3C3_11D3_A712_009027900694__INCLUDED_)
