#if !defined(AFX_BCGSHELLTREE_H__3791A756_78B8_4EEA_8520_849389480AB1__INCLUDED_)
#define AFX_BCGSHELLTREE_H__3791A756_78B8_4EEA_8520_849389480AB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGShellTree.h : header file
//

#include "bcgcontrolbar.h"
#include "BCGShellManager.h"

class CBCGShellList;

/////////////////////////////////////////////////////////////////////////////
// CBCGShellTree window

class BCGCONTROLBARDLLEXPORT CBCGShellTree : public CTreeCtrl
{
	friend class CBCGShellList;

	DECLARE_DYNAMIC(CBCGShellTree)

// Construction
public:
	CBCGShellTree();

// Attributes
public:
	BOOL GetItemPath (CString& strPath, HTREEITEM htreeItem = NULL /* NULL - selected */) const;
	CBCGShellList* GetRelatedList () const;

	// Flags are same as in IShellFolder::EnumObjects

	DWORD GetFlags () const
	{
		return m_dwFlags;
	}

	void SetFlags (DWORD dwFlags, BOOL bRefresh = TRUE);

// Operations
public:
	void Refresh ();
	BOOL SelectPath (LPCTSTR lpszPath);
	BOOL SelectPath (LPCITEMIDLIST lpidl);

	void EnableShellContextMenu (BOOL bEnable = TRUE);

	void SetRelatedList (CBCGShellList* pShellList);

// Overrides
public:
	virtual CString OnGetItemText (LPBCGCBITEMINFO pItem);
	virtual int OnGetItemIcon (LPBCGCBITEMINFO pItem, BOOL bSelected);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGShellTree)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGShellTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGShellTree)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	BOOL GetRootItems ();
	BOOL GetChildItems (HTREEITEM hParentItem);
	virtual HRESULT EnumObjects (HTREEITEM hParentItem, 
					  LPSHELLFOLDER pParentFolder, 
					  LPITEMIDLIST pidlParent);
	void OnShowContextMenu (CPoint point);
	void InitTree ();

	static int CALLBACK CompareProc(LPARAM, LPARAM, LPARAM);

	static IContextMenu2*	m_pContextMenu2;

	BOOL	m_bContextMenu;

	HWND	m_hwndRelatedList;
	BOOL	m_bNoNotify;
	DWORD	m_dwFlags;	// Flags for IShellFolder::EnumObjects
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGSHELLTREE_H__3791A756_78B8_4EEA_8520_849389480AB1__INCLUDED_)
