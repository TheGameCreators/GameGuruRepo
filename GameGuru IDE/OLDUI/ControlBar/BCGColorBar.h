// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#if !defined(AFX_BCGCOLORBAR_H__9B759C19_6B1C_48C4_A884_C0DEDD60DD77__INCLUDED_)
#define AFX_BCGCOLORBAR_H__9B759C19_6B1C_48C4_A884_C0DEDD60DD77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGColorBar.h : header file
//

#ifndef BCG_NO_COLOR

#include "bcgcontrolbar.h"
#include "BCGPopupMenuBar.h"

class CBCGColorButton;
class CBCGPropList;

/////////////////////////////////////////////////////////////////////////////
// CBCGColorBar window

class BCGCONTROLBARDLLEXPORT CBCGColorBar : public CBCGPopupMenuBar
{
	friend class CColorPopup;
	friend class CBCGColorMenuButton;
	friend class CBCGColorButton;
	friend class CColorButton;
	friend class CBCGImageEditDlg;
	friend class CBCGColorProp;

	DECLARE_SERIAL(CBCGColorBar)

// Construction
public:
	CBCGColorBar();

	virtual BOOL Create(
			CWnd*		pParentWnd,
			DWORD		dwStyle,
			UINT		nID,
			CPalette*	pPalette = NULL,	// Palette color, NULL - standard 20 colors
			int			nColumns = 0,		// Number of columns, -1 - default
			int			nRowsDockHorz = 0,	// Number of rows for horz. docking
			int			nColDockVert = 0);	// Number of columns for vert. docking

protected:
	CBCGColorBar(	const CArray<COLORREF, COLORREF>& colors,
					COLORREF color, 
					LPCTSTR lpszAutoColor,
					LPCTSTR lpszOtherColor,
					LPCTSTR lpszDocColors,
					CList<COLORREF,COLORREF>& lstDocColors,
					int nColumns,
					int	nRowsDockHorz,
					int	nColDockVert,
					COLORREF colorAutomatic,
					UINT nCommandID,
					CBCGColorButton* pParentBtn);

	CBCGColorBar (CBCGColorBar& src, UINT uiCommandID);

// Operations:
public:
	void SetDocumentColors (LPCTSTR lpszCaption, 
							CList<COLORREF,COLORREF>& lstDocColors,
							BOOL bShowWhenDocked = FALSE);

	void ContextToSize (BOOL bSquareButtons = TRUE,
						BOOL bCenterButtons = TRUE);

// Attributes
public:
	void SetColor (COLORREF color);
	COLORREF GetColor () const
	{
		return m_ColorSelected;
	}

	static void SetColorName (COLORREF color, const CString& strName)
	{
		m_ColorNames.SetAt (color, strName);
	}

	void EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable = TRUE);
	void EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg = TRUE, BOOL bEnable = TRUE);

	UINT GetCommandID () const
	{
		return m_nCommandID;
	}

	void SetCommandID (UINT nCommandID)
	{
		m_nCommandID = nCommandID;
	}

	BOOL IsTearOff () const
	{
		return m_bIsTearOff;
	}

protected:
    int							m_nNumColumns;
    int							m_nNumRowsHorz;
    int							m_nNumColumnsVert;
	CArray<COLORREF, COLORREF>	m_colors;
	CPalette					m_Palette;
	CSize						m_BoxSize;
	int							m_nRowHeight;
	COLORREF					m_ColorSelected;	// (-1) - auto
	COLORREF					m_ColorAutomatic;
	CString						m_strAutoColor;
	CString						m_strOtherColor;
	CString						m_strDocColors;
	CList<COLORREF,COLORREF>	m_lstDocColors;
	UINT						m_nCommandID;
	BOOL						m_bStdColorDlg;
	BOOL						m_bIsEnabled;
	BOOL						m_bIsTearOff;
	BOOL						m_bShowDocColorsWhenDocked;
	CBCGColorButton*			m_pParentBtn;
	CBCGPropList*				m_pWndPropList;
	int							m_nHorzOffset;
	int							m_nVertOffset;
	BOOL						m_bInternal;	// Used in customization

	static CMap<COLORREF,COLORREF,CString, LPCTSTR>	m_ColorNames;

// Overrides
	virtual BOOL OnSendCommand (const CBCGToolbarButton* pButton);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void AdjustLocations ();
	virtual CSize CalcSize (BOOL bVertDock);
	virtual void DoPaint(CDC* pDC);
	virtual BOOL OnKey (UINT nChar);

	virtual void Serialize (CArchive& ar);
	virtual void ShowCommandMessageString (UINT uiCmdId);

	virtual BOOL AllowChangeTextLabels () const
	{
		return FALSE;
	}

	virtual BOOL AllowShowOnList () const
	{	
		return TRUE;	
	}

	virtual BOOL OpenColorDialog (const COLORREF colorDefault, COLORREF& colorRes);

	virtual void Rebuild ();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGColorBar)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGColorBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGColorBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

	static int InitColors (CPalette* pPalette, 
						CArray<COLORREF, COLORREF>& arColors);
	static BOOL CreatePalette (const CArray<COLORREF, COLORREF>& arColors,
								CPalette& palette);

	CSize GetColorGridSize (BOOL bVertDock) const;
	int GetExtraHeight (int nNumColumns) const;
	CPalette* SelectPalette (CDC* pDC);

	void SetPropList (CBCGPropList* pWndList)
	{
		m_pWndPropList = pWndList;
	}
};

#endif // BCG_NO_COLOR

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGCOLORBAR_H__9B759C19_6B1C_48C4_A884_C0DEDD60DD77__INCLUDED_)
