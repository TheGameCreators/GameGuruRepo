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

#if !defined(AFX_TOOLBARNAMEDLG_H__E52278EA_EB02_11D1_90D8_00A0C9B05590__INCLUDED_)
#define AFX_TOOLBARNAMEDLG_H__E52278EA_EB02_11D1_90D8_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ToolbarNameDlg.h : header file
//

#include "bcgbarres.h"

/////////////////////////////////////////////////////////////////////////////
// CToolbarNameDlg dialog

class CToolbarNameDlg : public CDialog
{
// Construction
public:
	CToolbarNameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CToolbarNameDlg)
	enum { IDD = IDD_BCGBARRES_TOOLBAR_NAME };
	CButton	m_btnOk;
	CString	m_strToolbarName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolbarNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CToolbarNameDlg)
	afx_msg void OnUpdateToolbarName();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARNAMEDLG_H__E52278EA_EB02_11D1_90D8_00A0C9B05590__INCLUDED_)
