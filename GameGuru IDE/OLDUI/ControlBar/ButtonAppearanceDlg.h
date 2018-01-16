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

#if !defined(AFX_BUTTONAPPEARANCEDLG_H__07E27534_C6D9_11D1_A647_00A0C93A70EC__INCLUDED_)
#define AFX_BUTTONAPPEARANCEDLG_H__07E27534_C6D9_11D1_A647_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ButtonAppearanceDlg.h : header file
//

#ifndef BCG_NO_CUSTOMIZATION

#include "ButtonsList.h"
#include "bcgbarres.h"

class CBCGToolBarImages;
class CBCGUserTool;

/////////////////////////////////////////////////////////////////////////////
// CButtonAppearanceDlg dialog

class CButtonAppearanceDlg : public CDialog
{
// Construction
public:
	CButtonAppearanceDlg(CBCGToolbarButton* pButton,
						CBCGToolBarImages* pImages, CWnd* pParent = NULL,
						int iStartImage = 0,
						BOOL bMenuMode = FALSE);   // standard constructor
	virtual ~CButtonAppearanceDlg ();

// Dialog Data
	//{{AFX_DATA(CButtonAppearanceDlg)
	enum { IDD = IDD_BCGBARRES_BUTTON_PROPS };
	CButton	m_wndDefautImageBtn;
	CButton	m_wndUserImageBtn;
	CStatic	m_wndDefaultImageArea;
	CEdit	m_wndButtonText;
	CButton	m_wndAddImage;
	CButtonsList	m_wndButtonList;
	CButton	m_wndEditImage;
	CString	m_strButtonText;
	CString	m_strButtonDescr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonAppearanceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CButtonAppearanceDlg)
	afx_msg void OnAddImage();
	afx_msg void OnEditImage();
	afx_msg void OnImageList();
	afx_msg void OnImage();
	afx_msg void OnImageText();
	afx_msg void OnText();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnUserImage();
	afx_msg void OnDefaultImage();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Operations:
protected:
	void RebuildImageList ();
	void EnableControls ();

// Attributes:
protected:
	CBCGToolbarButton*	m_pButton;
	CBCGToolBarImages*	m_pImages;
	int					m_iStartImage;
	CObList				m_Buttons;
	int					m_iSelImage;
	BOOL				m_bImage;
	BOOL				m_bText;
	BOOL				m_bMenuMode;
	BOOL				m_bUserButton;
	CRect				m_rectDefaultImage;
	CString				m_strAccel;
	CBCGUserTool*		m_pUserTool;
};

#endif // BCG_NO_CUSTOMIZATION

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONAPPEARANCEDLG_H__07E27534_C6D9_11D1_A647_00A0C93A70EC__INCLUDED_)
