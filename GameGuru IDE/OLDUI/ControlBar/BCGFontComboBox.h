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

#if !defined(AFX_BCGFONTCOMBOBOX_H__36B2AFC4_0E1E_47DD_9C7B_CF90FB9CFF83__INCLUDED_)
#define AFX_BCGFONTCOMBOBOX_H__36B2AFC4_0E1E_47DD_9C7B_CF90FB9CFF83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGFontComboBox.h : header file
//

#include "bcgcontrolbar.h"

class CBCGFontDesc;

/////////////////////////////////////////////////////////////////////////////
// CBCGFontComboBox window

class BCGCONTROLBARDLLEXPORT CBCGFontComboBox : public CComboBox
{
// Construction
public:
	CBCGFontComboBox();

// Attributes
protected:
	CImageList	m_Images;
	BOOL		m_bToolBarMode;

// Operations
public:
	BOOL Setup (int nFontType = DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE,
				BYTE nCharSet = DEFAULT_CHARSET,
				BYTE nPitchAndFamily = DEFAULT_PITCH);
	BOOL SelectFont (CBCGFontDesc* pDesc);
	BOOL SelectFont (LPCTSTR lpszName, BYTE nCharSet = DEFAULT_CHARSET);
	
	CBCGFontDesc* GetSelFont () const;

protected:
	void Init ();
	void CleanUp ();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGFontComboBox)
	public:
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGFontComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGFontComboBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGFONTCOMBOBOX_H__36B2AFC4_0E1E_47DD_9C7B_CF90FB9CFF83__INCLUDED_)
