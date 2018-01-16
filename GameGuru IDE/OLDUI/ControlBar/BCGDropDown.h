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

#if !defined(AFX_DROPDOWN_TOOLBAR)
#define AFX_DROPDOWN_TOOLBAR

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// BCGDropDown.h : header file
//

#include "bcgcontrolbar.h"
#include "BCGPopupMenuBar.h"

class CBCGDropDownToolbarButton;
class CBCGMenuBar;
class CBCGToolbarMenuButton;

class BCGCONTROLBARDLLEXPORT CBCGDropDownToolBar : public CBCGToolBar
{
	friend class CBCGDropDownToolbarButton;
	friend class CBCGDropDownFrame;

	DECLARE_SERIAL(CBCGDropDownToolBar)

public:
	CBCGDropDownToolBar()
	{
		m_bLocked = TRUE;
	}

	virtual ~CBCGDropDownToolBar() {}
   
	virtual BOOL OnSendCommand (const CBCGToolbarButton* pButton);

	virtual BOOL LoadBitmap (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL /*bLocked*/ = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0)
	{
		return CBCGToolBar::LoadBitmap (uiResID, uiColdResID, 
					uiMenuResID, TRUE /* Locked */,
					uiDisabledResID, uiMenuDisabledResID);
	}

	virtual BOOL LoadToolBar (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL /*bLocked*/ = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0,
					UINT uiHotResID = 0)
	{
		return CBCGToolBar::LoadToolBar (uiResID, uiColdResID, 
					uiMenuResID, /*bLocked*/ TRUE,
					uiDisabledResID, uiMenuDisabledResID, uiHotResID);
	}

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	// Generated message map functions
	//{{AFX_MSG(CBCGDropDownToolBar)
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);
   afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGDropDownFrame frame

class CBCGDropDownFrame : public CMiniFrameWnd
{
	friend CBCGDropDownToolbarButton;
	friend CBCGDropDownToolBar;

	DECLARE_SERIAL(CBCGDropDownFrame)

public:
	CBCGDropDownFrame();
	virtual ~CBCGDropDownFrame();

// Attributes
public:
	BOOL					m_bAutoDestroyParent;
	void SetAutoDestroy (BOOL bAutoDestroy = TRUE)
	{
		m_bAutoDestroy = bAutoDestroy;
	}

protected:
	CBCGDropDownToolbarButton*	m_pParentBtn;
	CBCGDropDownToolBar			m_wndToolBar;
	CBCGDropDownToolBar*		m_pWndOriginToolbar;

	int							m_x;
	int							m_y;
	BOOL						m_bAutoDestroy;

	CString						m_strCaption;

protected:
	static CString	m_strClassName;

// Operations
public:
	CBCGDropDownFrame* GetParentPopupMenu () const;
	CBCGMenuBar* GetParentMenuBar () const;

protected:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGDropDownFrame)
	public:
	virtual BOOL Create(CWnd* pWndParent, int x, int y, CBCGDropDownToolBar* pWndOriginToolbar);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGDropDownFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
#if _MSC_VER >= 1300
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
#else
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#endif
	DECLARE_MESSAGE_MAP()
};

class BCGCONTROLBARDLLEXPORT CBCGDropDownToolbarButton : public CBCGToolbarButton  
{
	friend class CBCGDropDownFrame;

	DECLARE_SERIAL(CBCGDropDownToolbarButton)

public:
	CBCGDropDownToolbarButton();
	CBCGDropDownToolbarButton (LPCTSTR lpszName, CBCGDropDownToolBar* pToolBar);
	virtual ~CBCGDropDownToolbarButton();

// Overrides:
public:
	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual BOOL OnClickUp();
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void OnCancelMode ();
	virtual BOOL OnContextHelp (CWnd* pWnd)
	{
		return OnClick (pWnd, FALSE);
	}

	virtual int OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, 
										BOOL bSelected);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL ExportToMenuButton (CBCGToolbarMenuButton& menuButton) const;
	virtual BOOL IsExtraSize () const
	{
		return m_bExtraSize && !m_bInternalDraw;
	}

	virtual BOOL OnCustomizeMenu (CMenu* pMenu);

	void SetDefaultCommand (UINT uiCmd);
	BOOL DropDownToolbar (CWnd* pWnd);

// Attributes:
public:
	BOOL IsDropDown () const
	{
		return m_pPopupMenu != NULL;
	}

protected:
	CBCGDropDownToolBar*	m_pToolBar;
	CString					m_strName;
	CBCGDropDownFrame*		m_pPopupMenu;
	CWnd*					m_pWndParent;

	UINT					m_uiTimer;
	int						m_iSelectedImage;
	BOOL					m_bInternalDraw;

// diagnostics:

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DROPDOWN_TOOLBAR)
