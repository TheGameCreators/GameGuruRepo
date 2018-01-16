#if !defined(AFX_COLORPOPUP_H__6ABB7DC6_CCA2_4B65_9269_3F41D3F21D3D__INCLUDED_)
#define AFX_COLORPOPUP_H__6ABB7DC6_CCA2_4B65_9269_3F41D3F21D3D__INCLUDED_

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
// ColorPopup.h : header file
//

#ifndef BCG_NO_COLOR

#include "BCGPopupMenu.h"
#include "BCGColorBar.h"

class CBCGPropList;

/////////////////////////////////////////////////////////////////////////////
// CColorPopup window

class CColorPopup : public CBCGPopupMenu
{
	friend class CBCGColorMenuButton;
	friend class CBCGColorButton;
	friend class CBCGColorProp;

	DECLARE_DYNAMIC(CColorPopup)

// Construction
protected:
    CColorPopup (	const CArray<COLORREF, COLORREF>& colors,
					COLORREF color, 
					LPCTSTR lpszAutoColor,
					LPCTSTR lpszOtherColor,
					LPCTSTR lpszDocColors,
					CList<COLORREF,COLORREF>& lstDocColors,
					int nColumns,
					int nHorzDockRows,
					int nVertDockColumns,
					COLORREF colorAutomatic,
					UINT uiCommandID)
		: m_wndColorBar (colors, color, lpszAutoColor, lpszOtherColor,
						lpszDocColors, lstDocColors, nColumns, 
						nHorzDockRows, nVertDockColumns, colorAutomatic,
						uiCommandID, NULL)
	{
		m_bEnabledInCustomizeMode = FALSE;
	}

    CColorPopup (	CBCGColorButton* pParentBtn,
					const CArray<COLORREF, COLORREF>& colors,
					COLORREF color, 
					LPCTSTR lpszAutoColor,
					LPCTSTR lpszOtherColor,
					LPCTSTR lpszDocColors,
					CList<COLORREF,COLORREF>& lstDocColors,
					int nColumns,
					COLORREF colorAutomatic)
		: m_wndColorBar (colors, color, lpszAutoColor, lpszOtherColor,
						lpszDocColors, lstDocColors, nColumns, 
						-1, -1, colorAutomatic,
						(UINT)-1,
						pParentBtn)
	{
		m_bEnabledInCustomizeMode = FALSE;
	}

	virtual ~CColorPopup();

// Attributes
protected:
	CBCGColorBar	m_wndColorBar;
	BOOL			m_bEnabledInCustomizeMode;

	void SetPropList (CBCGPropList* pWndList)
	{
		m_wndColorBar.SetPropList (pWndList);
	}

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CColorPopup)
	//}}AFX_VIRTUAL

	virtual CBCGPopupMenuBar* GetMenuBar ()
	{
		return &m_wndColorBar;
	}

	virtual CControlBar* CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName);

    // Generated message map functions
protected:
    //{{AFX_MSG(CColorPopup)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // BCG_NO_COLOR

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPOPUP_H__6ABB7DC6_CCA2_4B65_9269_3F41D3F21D3D__INCLUDED_)
