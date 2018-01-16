#if !defined(AFX_COLORPAGE1_H__9C4A9AB9_351E_4D5A_9548_7FA57A113573__INCLUDED_)
#define AFX_COLORPAGE1_H__9C4A9AB9_351E_4D5A_9548_7FA57A113573__INCLUDED_

// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorPage1.h : header file
//

#ifndef BCG_NO_COLOR

#include "PowerColorPicker.h"

class CBCGColorDialog;

/////////////////////////////////////////////////////////////////////////////
// CColorPage1 dialog

class CColorPage1 : public CPropertyPage
{
	friend class CBCGColorDialog;

	DECLARE_DYNCREATE(CColorPage1)

// Construction
public:
	CColorPage1();

	CBCGColorDialog*	m_pDialog;

// Dialog Data
	//{{AFX_DATA(CColorPage1)
	enum { IDD = IDD_BCGBARRES_COLOR_PAGE_ONE };
	CBCGColorPickerCtrl	m_hexpicker;
	CBCGColorPickerCtrl	m_hexpicker_greyscale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CColorPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	friend void hexfunc(COLORREF ref,DWORD ClientData);
	friend void hexfunc(COLORREF ref,void *ClientData);
	
	friend void hexgreyscalefunc(COLORREF ref,DWORD ClientData);
	friend void hexgreyscalefunc(COLORREF ref,void *ClientData);

	// Generated message map functions
	//{{AFX_MSG(CColorPage1)
	virtual BOOL OnInitDialog();
	afx_msg void OnGreyscale();
	afx_msg void OnHexColor();
	afx_msg void OnDoubleClickedColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // BCG_NO_COLOR

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPAGE1_H__9C4A9AB9_351E_4D5A_9548_7FA57A113573__INCLUDED_)
