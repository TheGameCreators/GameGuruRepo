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

#if !defined(AFX_BCGTABVIEW_H__78A92E9C_04DB_11D5_95C8_00A0C9289F1B__INCLUDED_)
#define AFX_BCGTABVIEW_H__78A92E9C_04DB_11D5_95C8_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGTabView.h : header file
//

#ifndef BCG_NO_TABCTRL

#include "bcgcontrolbar.h"
#include "bcgtabwnd.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGTabView view

class BCGCONTROLBARDLLEXPORT CBCGTabView : public CView
{
protected:
	CBCGTabView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBCGTabView)

// Attributes
public:
	CBCGTabWnd& GetTabControl ()
	{
		return m_wndTabs;
	}

protected:
	CBCGTabWnd	m_wndTabs;

// Operations
public:
	int AddView (CRuntimeClass* pViewClass, const CString& strViewLabel,
				int iIndex = -1, CCreateContext* pContext = NULL);
	int FindTab (HWND hWndView) const;

	BOOL RemoveView (int iTabNum);
	BOOL SetActiveView (int iTabNum);
	CView* GetActiveView () const;           // active view or NULL

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGTabView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

	virtual void OnActivateView (CView* /*pView*/) {}
	virtual BOOL IsScrollBar () const
	{
		return FALSE;
	}

// Implementation
protected:
	virtual ~CBCGTabView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGTabView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	afx_msg LRESULT OnChangeActiveTab(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif // !BCG_NO_TABCTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGTABVIEW_H__78A92E9C_04DB_11D5_95C8_00A0C9289F1B__INCLUDED_)
