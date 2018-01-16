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

#if !defined(AFX_BUTTONSTEXTLIST_H__DA15C743_C687_11D1_B110_B5F0ACB7A449__INCLUDED_)
#define AFX_BUTTONSTEXTLIST_H__DA15C743_C687_11D1_B110_B5F0ACB7A449__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ButtonsTextList.h : header file
//

#ifndef BCG_NO_CUSTOMIZATION

/////////////////////////////////////////////////////////////////////////////
// CButtonsTextList window

class CButtonsTextList : public CListBox
{
// Construction
public:
	CButtonsTextList();

// Attributes
protected:
	CSize	m_sizeButton;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonsTextList)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButtonsTextList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CButtonsTextList)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // BCG_NO_CUSTOMIZATION

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONSTEXTLIST_H__DA15C743_C687_11D1_B110_B5F0ACB7A449__INCLUDED_)
