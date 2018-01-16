#if !defined(AFX_BCGLISTCTRL_H__50D57A86_4742_41F6_BE36_37DCEBDCDA13__INCLUDED_)
#define AFX_BCGLISTCTRL_H__50D57A86_4742_41F6_BE36_37DCEBDCDA13__INCLUDED_

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
// BCGListCtrl.h : header file
//

#include "bcgcontrolbar.h"
#include "BCGHeaderCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGListCtrl window

class BCGCONTROLBARDLLEXPORT CBCGListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CBCGListCtrl)

// Construction
public:
	CBCGListCtrl();

// Attributes
public:
	CBCGHeaderCtrl& GetHeaderCtrl ()
	{
		return m_wndHeader;
	}

	// Mark sorted column by background color
	void EnableMarkSortedColumn (BOOL bMark = TRUE, BOOL bRedraw = TRUE);

protected:
	CBCGHeaderCtrl	m_wndHeader;
	int				m_iSortedColumn;
	BOOL			m_bAscending;
	BOOL			m_bMarkSortedColumn;
	COLORREF		m_clrSortedColumn;

// Operations
public:

	// Sorting operations:
	void Sort (int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	void SetSortColumn (int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	void RemoveSortColumn (int iColumn);
	void EnableMultipleSort (BOOL bEnable = TRUE);
	BOOL IsMultipleSort () const;

// Overrides
	virtual int OnCompareItems (LPARAM lParam1, LPARAM lParam2, int iColumn);

	// Support for individual cells text/background colors:
	virtual COLORREF OnGetCellTextColor (int /*nRow*/, int /*nColum*/)
	{
		return GetTextColor ();
	}

	virtual COLORREF OnGetCellBkColor (int /*nRow*/, int /*nColum*/)
	{
		return GetBkColor ();
	}

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGListCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGListCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	static int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	BOOL InitList ();
	void InitColors ();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGLISTCTRL_H__50D57A86_4742_41F6_BE36_37DCEBDCDA13__INCLUDED_)
