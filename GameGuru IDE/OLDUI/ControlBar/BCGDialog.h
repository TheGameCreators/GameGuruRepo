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

// Created by Rui Godinho Lopes <ruiglopes@yahoo.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef BCG_NO_DIALOG

//********************************************************************
//
// BCGDialog.h : header file
//
// REVISION HISTORY
// ----------------
// 0.00 31june2000 - Rui Godinho Lopes <ruiglopes@yahoo.com>
//   creation
//
//********************************************************************

#if !defined(AFX_BCGDIALOG_H__09800B11_4F60_11D4_AB0C_000000000000__INCLUDED_)
#define AFX_BCGDIALOG_H__09800B11_4F60_11D4_AB0C_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGDialog dialog

class BCGCONTROLBARDLLEXPORT CBCGDialog : public CDialog
{
	friend class CBCGPopupMenu;
	friend class CBCGContextMenuManager;

	DECLARE_DYNAMIC(CBCGDialog)

// Construction
public:
	CBCGDialog();
	CBCGDialog(UINT nIDTemplate, CWnd *pParent= NULL);
	CBCGDialog(LPCTSTR lpszTemplateName, CWnd *pParentWnd= NULL);

protected:
	void CommonConstruct ();

// Attributes:
public:
	enum BackgroundLocation
	{
		BACKGR_TILE,
		BACKGR_TOPLEFT,
		BACKGR_TOPRIGHT,
		BACKGR_BOTTOMLEFT,
		BACKGR_BOTTOMRIGHT,
	};

protected:
	HBITMAP				m_hBkgrBitmap;
	CSize				m_sizeBkgrBitmap;
	CBrush				m_brBkgr;
	BackgroundLocation	m_BkgrLocation;
	BOOL				m_bAutoDestroyBmp;
	static HHOOK		m_hookMouse;
	static CBCGDialog*	m_pMenuDlg;

// Operations:
public:
	void SetBackgroundColor (COLORREF color, BOOL bRepaint = TRUE);
	void SetBackgroundImage (HBITMAP hBitmap, 
							BackgroundLocation location = BACKGR_TILE,
							BOOL bAutoDestroy = TRUE,
							BOOL bRepaint = TRUE);
	BOOL SetBackgroundImage (UINT uiBmpResId,
							BackgroundLocation location = BACKGR_TILE,
							BOOL bRepaint = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	//{{AFX_MSG(CBCGDialog)
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL ProcessMouseClick (POINT pt);
	BOOL ProcessMouseMove (POINT pt);
	void SetActiveMenu (CBCGPopupMenu* pMenu);

	static LRESULT CALLBACK BCGDlgMouseProc (int nCode, WPARAM wParam, LPARAM lParam);
};

#endif // BCG_NO_DIALOG

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGDIALOG_H__09800B11_4F60_11D4_AB0C_000000000000__INCLUDED_)
