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

#if !defined(AFX_BCGBUTTON_H__C159C4C6_B79B_11D3_A712_009027900694__INCLUDED_)
#define AFX_BCGBUTTON_H__C159C4C6_B79B_11D3_A712_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGButton.h : header file
//

#include "MenuImages.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGButton window

class BCGCONTROLBARDLLEXPORT CBCGButton : public CButton
{
	DECLARE_DYNAMIC(CBCGButton)

// Construction
public:
	CBCGButton();

// Attributes
public:
	enum FlatStyle
	{
		BUTTONSTYLE_3D,
		BUTTONSTYLE_FLAT,
		BUTTONSTYLE_SEMIFLAT,
		BUTTONSTYLE_NOBORDERS
	};

	enum AlignStyle
	{
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};
	
	FlatStyle		m_nFlatStyle;
	AlignStyle		m_nAlignStyle;
	BOOL			m_bRighImage;	// Is image on the right side?
	BOOL			m_bTransparent;
	BOOL			m_bDrawFocus;
	BOOL			m_bHighlightChecked;
	BOOL			m_bGrayDisabled;

	CToolTipCtrl& GetToolTipCtrl ()
	{
		return m_wndToolTip;
	}

	BOOL IsPushed () const
	{
		return m_bPushed;
	}

	BOOL IsPressed () const
	{
		return m_bPushed && m_bHighlighted;
	}

	BOOL IsHighlighted () const
	{
		return m_bHighlighted;
	}

	BOOL IsCheckBox () const 
	{ 
		return m_bCheckButton;
	}

	BOOL IsRadioButton () const 
	{ 
		return m_bRadioButton; 
	}

	BOOL IsAutoCheck () const 
	{ 
		return m_bAutoCheck; 
	}

protected:
	BOOL			m_bPushed;
	BOOL			m_bClickiedInside;
	BOOL			m_bHighlighted;
	BOOL			m_bCaptured;
	BOOL			m_bHover;
	BOOL			m_bChecked;
	BOOL			m_bCheckButton;
	BOOL			m_bRadioButton;
	BOOL			m_bAutoCheck;

	CSize			m_sizeImage;
	CImageList		m_Image;
	CImageList		m_ImageHot;
	CImageList		m_ImageDisabled;
	CImageList		m_ImageChecked;
	CImageList		m_ImageCheckedHot;
	CImageList		m_ImageCheckedDisabled;
	BOOL			m_bAutoDestroyImage;
	BOOL			m_bFullTextTooltip;
	BOOL			m_bDelayFullTextTooltipSet;

	CMenuImages::IMAGES_IDS		m_nStdImageId;

	CToolTipCtrl	m_wndToolTip;

	HCURSOR			m_hCursor;

	CSize			m_sizePushOffset;

	COLORREF		m_clrRegular;
	COLORREF		m_clrHover;
	COLORREF		m_clrFace;

	HFONT			m_hFont;
	int				m_nAutoRepeatTimeDelay;

	static BOOL		m_bWinXPTheme;

// Operations
public:
	void SetImage (HICON hIcon, BOOL bAutoDestroy = TRUE, HICON hIconHot = NULL);
	void SetImage (HBITMAP hBitmap, BOOL bAutoDestroy = TRUE, HBITMAP hBitmapHot = NULL, BOOL bMap3dColors = TRUE);
	void SetImage (UINT uiBmpResId, UINT uiBmpHotResId = 0);

	void SetCheckedImage (HICON hIcon, BOOL bAutoDestroy = TRUE, HICON hIconHot = NULL);
	void SetCheckedImage (HBITMAP hBitmap, BOOL bAutoDestroy = TRUE, HBITMAP hBitmapHot = NULL, BOOL bMap3dColors = TRUE);
	void SetCheckedImage (UINT uiBmpResId, UINT uiBmpHotResId = 0);

	void SetStdImage (CMenuImages::IMAGES_IDS id);

	void SetTooltip (LPCTSTR lpszToolTipText);	// lpszToolTip == NULL: disable tooltip

	void SetMouseCursor (HCURSOR hcursor);
	void SetMouseCursorHand ();

	void SetTextColor (COLORREF clrText)
	{
		m_clrRegular = clrText;
	}

	void SetTextHotColor (COLORREF clrTextHot)
	{
		m_clrHover = clrTextHot;
	}

	void SetFaceColor (COLORREF crFace, BOOL bRedraw = TRUE);

	void EnableMenuFont (BOOL bOn = TRUE, BOOL bRedraw = TRUE);
	void EnableFullTextTooltip (BOOL bOn = TRUE);

	void SetAutorepeatMode (int nTimeDelay = 500 /* ms */);
	BOOL IsAutorepeatCommandMode () const
	{
		return m_nAutoRepeatTimeDelay > 0;
	}

	// Allow to draw button borders using current WinXP theme.
	// CBCGWinXPVisualManager should be active visual manager:
	static void EnableWinXPTheme (BOOL bEnable = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGButton();
	virtual void CleanUp ();

	virtual CSize SizeToContent (BOOL bCalcOnly = FALSE);

protected:
	virtual void OnFillBackground (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState);
	virtual void OnDrawFocusRect (CDC* pDC, const CRect& rectClient);
	virtual void OnDraw (CDC* pDC, const CRect& rect, UINT uiState);

	virtual void OnDrawText (CDC* pDC, const CRect& rect, const CString& strText,
							UINT uiDTFlags, UINT uiState);

	virtual BOOL CreateDisabledImage (HBITMAP hBitmapOrig, BOOL bChecked);
	virtual CFont* SelectFont (CDC* pDC);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGButton)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnCancelMode();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetCheck (WPARAM, LPARAM);
	afx_msg LRESULT OnSetCheck (WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()

	void InitStyle (DWORD dwStyle);

	BOOL CheckNextPrevRadioButton (BOOL bNext);
	void UncheckRadioButtonsInGroup ();

	void SetImageInternal (HICON hIcon, BOOL bAutoDestroy, HICON hIconHot, BOOL bChecked);
	void SetImageInternal (HBITMAP hBitmap, BOOL bAutoDestroy, HBITMAP hBitmapHot, BOOL bMap3dColors, BOOL bChecked);
	void SetImageInternal (UINT uiBmpResId, UINT uiBmpHotResId, BOOL bChecked);

	void ClearImages (BOOL bChecked);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGBUTTON_H__C159C4C6_B79B_11D3_A712_009027900694__INCLUDED_)
