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

#if !defined(AFX_BCGURLLINKBUTTON_H__E903B416_9AB5_11D3_A70E_009027900694__INCLUDED_)
#define AFX_BCGURLLINKBUTTON_H__E903B416_9AB5_11D3_A70E_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGURLLinkButton.h : header file
//

#include "BCGButton.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGURLLinkButton window

class BCGCONTROLBARDLLEXPORT CBCGURLLinkButton : public CBCGButton
{
	DECLARE_DYNAMIC(CBCGURLLinkButton)

// Construction
public:
	CBCGURLLinkButton();

// Attributes
protected:
	CString			m_strURL;
	CString			m_strPrefix;

// Operations
public:
	void SetURL (LPCTSTR lpszURL);			// By default, window text will be used
	void SetURLPrefix (LPCTSTR lpszPrefix);	// For example "mailto:"

	virtual CSize SizeToContent (BOOL bVCenter = FALSE, BOOL bHCenter = FALSE);

protected:
	virtual void OnDraw (CDC* pDC, const CRect& rect, UINT uiState);
	virtual void OnDrawFocusRect (CDC* pDC, const CRect& rectClient);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGURLLinkButton)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGURLLinkButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGURLLinkButton)
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGURLLINKBUTTON_H__E903B416_9AB5_11D3_A70E_009027900694__INCLUDED_)
