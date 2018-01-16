#if !defined(AFX_BCGSELECTSKINDLG_H__3CE82760_C9CE_4A09_8693_E6EDA4C891E0__INCLUDED_)
#define AFX_BCGSELECTSKINDLG_H__3CE82760_C9CE_4A09_8693_E6EDA4C891E0__INCLUDED_

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
// BCGSelectSkinDlg.h : header file
//

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version

#include "BCGButton.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGSelectSkinDlg dialog

class CBCGSelectSkinDlg : public CDialog
{
// Construction
public:
	CBCGSelectSkinDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGSelectSkinDlg)
	enum { IDD = IDD_BCGBARRES_SKINS_DLG };
	CBCGButton	m_btnMail;
	CBCGButton	m_btnURL;
	CButton	m_btnDownload;
	CButton	m_btnOK;
	CButton	m_btnCancel;
	CListBox	m_wndSkinsList;
	CStatic	m_wndSkindPreviewArea;
	CString	m_strAuthor;
	//}}AFX_DATA

protected:
	CRect	m_rectPreview;
	int		m_iSelectedSkin;
	CString	m_strClose;
	CString	m_srDefault;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGSelectSkinDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGSelectSkinDlg)
	afx_msg void OnDblclkBcgbarresSkinsList();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSelchangeBcgbarresSkinsList();
	afx_msg void OnApply();
	afx_msg void OnDownloadSkins();
	afx_msg void OnDestroy();
	afx_msg void OnMail();
	afx_msg void OnURL();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void RefreshSkinsList ();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif

#endif // !defined(AFX_BCGSELECTSKINDLG_H__3CE82760_C9CE_4A09_8693_E6EDA4C891E0__INCLUDED_)
