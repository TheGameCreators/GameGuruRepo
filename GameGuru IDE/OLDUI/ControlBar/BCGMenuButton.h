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

#if !defined(AFX_BCGMENUBUTTON_H__49B675A4_C3E1_11D3_A722_009027900694__INCLUDED_)
#define AFX_BCGMENUBUTTON_H__49B675A4_C3E1_11D3_A722_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGMenuButton.h : header file
//

#include "BCGButton.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGMenuButton window

class BCGCONTROLBARDLLEXPORT CBCGMenuButton : public CBCGButton
{
	DECLARE_DYNAMIC(CBCGMenuButton)

	friend class CBCGPopupMenu;

// Construction
public:
	CBCGMenuButton();

// Attributes
public:
	BOOL	m_bRightArrow;
	HMENU	m_hMenu;
	int		m_nMenuResult;
	BOOL	m_bStayPressed;
	BOOL	m_bOSMenu;		// User standard Windows menu either than library

protected:
	BOOL	m_bMenuIsActive;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGMenuButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGMenuButton();
	virtual CSize SizeToContent (BOOL bCalcOnly = FALSE);

protected:
	virtual void OnDraw (CDC* pDC, const CRect& rect, UINT uiState);
	virtual void OnShowMenu ();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGMenuButton)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGMENUBUTTON_H__49B675A4_C3E1_11D3_A722_009027900694__INCLUDED_)
