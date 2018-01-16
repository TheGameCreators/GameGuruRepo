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

#if !defined(AFX_IMAGEPAINTAREA_H__07E27537_C6D9_11D1_A647_00A0C93A70EC__INCLUDED_)
#define AFX_IMAGEPAINTAREA_H__07E27537_C6D9_11D1_A647_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ImagePaintArea.h : header file
//

#include "bcgcontrolbar.h"

class CBCGImageEditDlg;

/////////////////////////////////////////////////////////////////////////////
// CImagePaintArea window

class BCGCONTROLBARDLLEXPORT CImagePaintArea : public CButton
{
// Construction
public:
	CImagePaintArea(CBCGImageEditDlg* pParentDlg);

// Attributes
public:
	enum IMAGE_EDIT_MODE
	{
		IMAGE_EDIT_MODE_PEN = 0,
		IMAGE_EDIT_MODE_FILL,
		IMAGE_EDIT_MODE_LINE,
		IMAGE_EDIT_MODE_RECT,
		IMAGE_EDIT_MODE_ELLIPSE,
		IMAGE_EDIT_MODE_COLOR
	};

	CRect				m_rectParentPreviewArea;

protected:
	COLORREF			m_rgbColor;
	CDC					m_memDC;
	CSize				m_sizeImage;
	CBitmap*			m_pBitmap;
	IMAGE_EDIT_MODE		m_Mode;
	HCURSOR				m_hcurPen;
	HCURSOR				m_hcurFill;
	HCURSOR				m_hcurLine;
	HCURSOR				m_hcurRect;
	HCURSOR				m_hcurEllipse;
	HCURSOR				m_hcurColor;
	CSize				m_sizeCell;
	CBCGImageEditDlg*	m_pParentDlg;
	CPen				m_penDraw;
	CPen				m_penStretch;
	CRect				m_rectDraw;

// Operations
public:
	void SetBitmap (CBitmap* pBitmap);
	void SetColor (COLORREF color);
	void SetMode (IMAGE_EDIT_MODE mode)
	{
		m_Mode = mode;
	}

	IMAGE_EDIT_MODE GetMode () const
	{
		return m_Mode;
	}

protected:
	void ScreenToBitmap (CPoint& point);
	void BitmapToClient (CRect& rect);

	void FloodFil (const CPoint& point);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImagePaintArea)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImagePaintArea();

	// Generated message map functions
protected:
	//{{AFX_MSG(CImagePaintArea)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	virtual void DrawItem (LPDRAWITEMSTRUCT lpDIS);
	void DrawPixel (POINT point);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEPAINTAREA_H__07E27537_C6D9_11D1_A647_00A0C93A70EC__INCLUDED_)
