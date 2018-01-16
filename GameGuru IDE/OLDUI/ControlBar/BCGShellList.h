#if !defined(AFX_BCGSHELLLIST_H__E4BD89AE_F335_4BC6_854F_751F8DBBE0D8__INCLUDED_)
#define AFX_BCGSHELLLIST_H__E4BD89AE_F335_4BC6_854F_751F8DBBE0D8__INCLUDED_

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
// BCGShellList.h : header file
//

#include "bcgcontrolbar.h"
#include "BCGShellManager.h"
#include "BCGListCtrl.h"

class CBCGShellTree;

/////////////////////////////////////////////////////////////////////////////
// CBCGShellList window

class BCGCONTROLBARDLLEXPORT CBCGShellList : public CBCGListCtrl
{
	friend class CBCGShellTree;

	DECLARE_DYNAMIC(CBCGShellList)

// Construction
public:
	CBCGShellList();

// Attributes
public:
	enum
	{
		BCGShellList_ColumnName = 0,
		BCGShellList_ColumnSize = 1,
		BCGShellList_ColumnType = 2,
		BCGShellList_ColumnModified = 3,
	}
	BCGShellListColumns;

	const IShellFolder* GetCurrentShellFolder () const
	{
		return m_psfCurFolder;
	}

	LPITEMIDLIST GetCurrentItemIdList () const
	{
		return m_pidlCurFQ;
	}

	BOOL IsDesktop () const
	{
		return m_bIsDesktop;
	}

protected:
	IShellFolder*	m_psfCurFolder;
	LPITEMIDLIST	m_pidlCurFQ;
	BOOL			m_bContextMenu;
	HWND			m_hwndRelatedTree;
	BOOL			m_bIsDesktop;
	BOOL			m_bNoNotify;
	SHCONTF			m_nTypes;

	static IContextMenu2*	m_pContextMenu2;

// Operations
public:
	BOOL GetItemPath (CString& strPath, int iItem) const;
	BOOL GetCurrentFolder (CString& strPath) const;
	BOOL GetCurrentFolderName (CString& strName) const;

	virtual HRESULT Refresh ();
	virtual HRESULT DisplayFolder (LPCTSTR lpszPath);
	virtual HRESULT DisplayFolder (LPBCGCBITEMINFO lpItemInfo);
	virtual HRESULT DisplayParentFolder ();

	void SetItemTypes (SHCONTF nTypes);
	SHCONTF GetItemTypes () const
	{
		return m_nTypes;
	}

	void EnableShellContextMenu (BOOL bEnable = TRUE);

// Overrides
	virtual void OnSetColumns ();
	virtual CString OnGetItemText (int iItem, int iColumn, LPBCGCBITEMINFO pItem);
	virtual int OnGetItemIcon (int iItem, LPBCGCBITEMINFO pItem);

	virtual void OnFormatFileSize (LONG lFileSize, CString& str);
	virtual void OnFormatFileDate (const CTime& tmFile, CString& str);

	virtual int OnCompareItems (LPARAM lParam1, LPARAM lParam2, int iColumn);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGShellList)
	protected:
	virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGShellList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGShellList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	HIMAGELIST GetShellImageList (BOOL bLarge);
	HRESULT LockCurrentFolder (LPBCGCBITEMINFO pItemInfo);
	void ReleaseCurrFolder ();
	HRESULT EnumObjects (LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent);
	void DoDefault (int iItem);

	BOOL InitList ();
	CBCGShellTree* GetRelatedTree () const;
};

extern BCGCONTROLBARDLLEXPORT UINT BCGM_CHANGE_CURRENT_FOLDER;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGSHELLLIST_H__E4BD89AE_F335_4BC6_854F_751F8DBBE0D8__INCLUDED_)
