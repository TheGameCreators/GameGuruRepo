// BCGCaptionBar.h: interface for the CBCGCaptionBar class.
//
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGCAPTIONBAR_H__AA2360F2_0462_411A_821D_BB78BCA54E49__INCLUDED_)
#define AFX_BCGCAPTIONBAR_H__AA2360F2_0462_411A_821D_BB78BCA54E49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"
						    
#include "BCGButton.h"
#include "BCGToolBarImages.h"

class BCGCONTROLBARDLLEXPORT CBCGCaptionBar : public CControlBar  
{
	DECLARE_DYNCREATE(CBCGCaptionBar)

public:
	CBCGCaptionBar();
	virtual ~CBCGCaptionBar();

	BOOL Create (DWORD dwStyle, CWnd* pParentWnd, UINT uID, int nHeight = -1);

protected:
	//{{AFX_MSG(CBCGCaptionBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysColorChange();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

//Attributes
public:
	enum BarElement
	{
		ELEM_BUTTON,
		ELEM_TEXT,
		ELEM_ICON
	};
	enum BarElementAlignment
	{
		ALIGN_INVALID,
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};

	void SetFlatBorder (BOOL bFlat = TRUE)
	{
		m_bFlatBorder = bFlat;
	}

	void SetButton		(LPCTSTR lpszLabel, UINT uiCmdUI, 
						BarElementAlignment btnAlignmnet = ALIGN_LEFT);
	void RemoveButton	();
	void EnableButton	(BOOL bEnable = TRUE);
	void SetButtonPressed (BOOL bPresed = TRUE);

	void SetIcon		(HICON hIcon, BarElementAlignment iconAlignment = ALIGN_RIGHT);
	void RemoveIcon		();

	void SetBitmap		(HBITMAP hBitmap, COLORREF clrTransparent, 
							BOOL bStretch = FALSE,
							BarElementAlignment bmpAlignment = ALIGN_RIGHT);
	void RemoveBitmap	();

	void SetText		(const CString& strText, BarElementAlignment textAlignment = ALIGN_RIGHT);
	void RemoveText		();

	void SetBorderSize (int nSize) {m_nBorderSize = nSize;}
	int  GetBorderSize () const {return m_nBorderSize;}

	void SetMargin (int nMargin) {m_nMargin = nMargin;}
	int  GetMargin () const {return m_nMargin;}

	BarElementAlignment GetAlignment (BarElement elem);

	COLORREF m_clrBarText;
	COLORREF m_clrBarBackground;
	COLORREF m_clrBarBorder;

	CRect GetButtonRect () const
	{
		return m_rectButton;
	}

protected:
// Overridables
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual void OnDrawBackground (CDC* pDC, CRect rect);
	virtual void OnDrawBorder	  (CDC* pDC, CRect rect);
	virtual void OnDrawText		  (CDC* pDC, CRect rect, const CString& strText);
	virtual void OnDrawImage	  (CDC* pDC, CRect rect);
	virtual void OnDrawButton	  (CDC* pDC, CRect rect, const CString& strButton, BOOL bEnabled);

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	virtual void AdjustLayout ();
	virtual void RecalcLayout ();

protected:

	//------------------
	// Image attributes:
	//------------------
	HICON				m_hIcon;
	CBCGToolBarImages	m_Bitmap;
	BOOL				m_bStretchImage;
	BarElementAlignment	m_iconAlignment;
	CRect				m_rectImage;

	//-----------------------
	// Text label attributes:
	//-----------------------
	HFONT				m_hFont;
	CString				m_strText;
	BarElementAlignment	m_textAlignment;
	CRect				m_rectText;
	CRect				m_rectDrawText;

	//-------------------
	// Button attributes:
	//-------------------
	CString				m_strBtnText;
	UINT				m_uiBtnID;
	BarElementAlignment	m_btnAlignnment;
	CRect				m_rectButton;
	BOOL				m_bIsBtnPressed;
	BOOL				m_bIsBtnHighlighted;
	BOOL				m_bIsBtnForcePressed;
	BOOL				m_bTracked;
	BOOL				m_bBtnEnabled;

	int					m_nBorderSize;
	int					m_nMargin;
	int					m_nHorzElementOffset;

	int					m_nDefaultHeight;
	int					m_nCurrentHeight;

	BOOL				m_bFlatBorder;
	COLORREF			m_clrTransparent;

protected:
	CSize GetImageSize () const;

	BOOL CheckRectangle		(CRect& rectSrc, const CRect& rectOther, BOOL bLeftOf);
	void AdjustRectToMargin (CRect& rectSrc, const CRect& rectClient, int nMargin, 
							 BOOL bRetainSize = FALSE);

	bool IsImageSet () const
	{
		return (m_hIcon != NULL || m_Bitmap.GetCount () > 0);
	}
};

#endif // !defined(AFX_BCGCAPTIONBAR_H__AA2360F2_0462_411A_821D_BB78BCA54E49__INCLUDED_)
