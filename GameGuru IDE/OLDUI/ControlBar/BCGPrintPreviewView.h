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
//
// Created by Rui Godinho Lopes <ruiglopes@yahoo.com>
//
//////////////////////////////////////////////////////////////////////

//********************************************************************
//
// BCGPrintPreviewView.h : header file
//
// REVISION HISTORY
// ----------------
// 0.00 9february2000
//   creation
// 0.01 14february2000
//   + CBCGPrintPreviewView for disabling user access to the
//     UserPopupToolBar and Customize Dialog
//   + "Pages" image icon toggling implemented (by Stas Levin)
// 0.02 7april2000
//   + The status bar now only displays page number information
//
//********************************************************************

#if !defined(AFX_BCGPRINTPREVIEWVIEW_H__7AA67963_DFEF_11D3_AA95_000000000000__INCLUDED_)
#define AFX_BCGPRINTPREVIEWVIEW_H__7AA67963_DFEF_11D3_AA95_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxpriv.h> //for CPreviewView
#include "BCGToolBar.h"
#include "bcgcontrolbar.h"

BCGCONTROLBARDLLEXPORT void BCGPrintPreview (CView* pView);

/////////////////////////////////////////////////////////////////////////////
// CBCGPrintPreviewToolBar toolbar

class CBCGPrintPreviewToolBar : public CBCGToolBar
{
protected:
	//{{AFX_MSG(CBCGPrintPreviewToolBar)
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPrintPreviewView window

class CBCGPrintPreviewView : public CPreviewView
{
protected:
	CBCGPrintPreviewView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBCGPrintPreviewView)

// Attributes
public:

// Operations
protected:
	void SetToolbarSize ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPrintPreviewView)
	//}}AFX_VIRTUAL
	void OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed);

// Implementation
protected:
	virtual ~CBCGPrintPreviewView();

	CBCGPrintPreviewToolBar m_wndToolBar;

	int m_iOnePageImageIndex;
	int m_iTwoPageImageIndex;

	int m_iPagesBtnIndex;

	CStatusBar*	m_pWndStatusBar;
	int			m_nSimpleType;
	BOOL		m_bIsStatusBarSimple;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPrintPreviewView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdatePreviewNumPage(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRINTPREVIEWVIEW_H__7AA67963_DFEF_11D3_AA95_000000000000__INCLUDED_)
