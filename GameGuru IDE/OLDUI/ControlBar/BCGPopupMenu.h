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

#if !defined(AFX_BCGPOPUPMENU_H__A6E1AD75_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPOPUPMENU_H__A6E1AD75_D511_11D1_A64D_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGPopupMenu.h : header file
//

#include "bcgcontrolbar.h"
#include "BCGPopupMenuBar.h"

class CBCGToolbarMenuButton;
class CBCGMenuBar;
class CBCGMenuPage;

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupMenu frame

class BCGCONTROLBARDLLEXPORT CBCGPopupMenu : public CMiniFrameWnd
{
	friend class CCustomizeButton;
	friend class CBCGToolbarMenuButton;
	friend class CBCGMenuPage;
	friend class CBCGFrameWnd;
	friend class CBCGMDIFrameWnd;
	friend class CBCGOleIPFrameWnd;
	friend class CBCGOleDocIPFrameWnd;
	friend class CBCGFrameImpl;
	friend class CBCGDialog;
	friend class CBCGContextMenuManager;

	DECLARE_SERIAL(CBCGPopupMenu)

public:
	CBCGPopupMenu();
	virtual ~CBCGPopupMenu();

protected:
	CBCGPopupMenu(CBCGMenuPage* pCustPage, LPCTSTR lpszTitle);	// For customization only

// Attributes
public:
	enum ANIMATION_TYPE
	{
		NO_ANIMATION,
		UNFOLD,
		SLIDE,
		FADE
	};

	enum MENUAREA_TYPE
	{
		MENU,
		SHADOW_RIGHT,
		SHADOW_BOTTOM,
		LOGO,
		OUTSIDE,
		TEAROFF_CAPTION
	};

	enum DROP_DIRECTION
	{
		DROP_DIRECTION_NONE,
		DROP_DIRECTION_BOTTOM,
		DROP_DIRECTION_TOP,
		DROP_DIRECTION_RIGHT,
		DROP_DIRECTION_LEFT,
	};

	BOOL m_bAutoDestroyParent;
	CMenu* GetMenu () const
	{
		return (CMenu*)m_hMenu;
	}

	virtual CBCGPopupMenuBar* GetMenuBar ()
	{
		return &m_wndMenuBar;
	}

	void SetAutoDestroy (BOOL bAutoDestroy = TRUE)
	{
		m_bAutoDestroy = bAutoDestroy;
	}

	CBCGToolbarMenuButton* GetParentButton () const
	{
		return m_pParentBtn;
	}

    // by Oz Solomonovich - force menu shadows even when menus are appear
    // out-side of main-frame
    static void SetForceShadow(BOOL bValue)
    {
        m_bForceShadow = bValue;
    }

    static BOOL GetForceShadow()
    {
        return m_bForceShadow;
    }

    static void SetForceMenuFocus (BOOL bValue)
    {
        m_bForceMenuFocus = bValue;
    }

    static BOOL GetForceMenuFocus ()
    {
        return m_bForceMenuFocus;
    }

	static CBCGPopupMenu::ANIMATION_TYPE GetAnimationType ()
	{
		return m_AnimationType;
	}

	static void SetAnimationType (CBCGPopupMenu::ANIMATION_TYPE type)
	{
		m_AnimationType = type;
	}

	static void SetAnimationSpeed (UINT nElapse);

	static UINT GetAnimationSpeed ()
	{
		return m_AnimationSpeed;
	}

	static void EnableMenuSound (BOOL bEnable = TRUE)
	{
		m_bMenuSound = bEnable;
	}

	static UINT IsMenuSound ()
	{
		return m_bMenuSound;
	}

	BOOL IsScrollable () const
	{
		return m_bScrollable;
	}

	void EnableScrolling (BOOL /*bEnable*/ = TRUE)
	{
		// This method is obsolete;
	}

	void EnableMenuLogo (int iLogoWidth);
	void SetMaxWidth (int iMaxWidth);

	// -- By Alex Corazzin (2) --
	CWnd* GetMessageWnd () const
	{
		return m_pMessageWnd; 
	}

	// yurig: support for cascade menus
	void SetMessageWnd (CWnd* pMsgWnd)
	{
		m_pMessageWnd = pMsgWnd;
	}

	BOOL PostCommand (UINT uiCommandID);
	// ------------------------

   void MoveTo (const CPoint& pt)
   {
	   m_ptLocation = pt;
	   RecalcLayout ();
   }

   virtual BOOL InCommand ()
   {
	   CBCGPopupMenuBar* pBar = GetMenuBar ();
	   return pBar == NULL ? FALSE : pBar->m_bInCommand;
   }

	static CBCGPopupMenu* GetActiveMenu ()
	{
		return m_pActivePopupMenu;
	}

	DROP_DIRECTION GetDropDirection () const
	{
		return m_DropDirection;
	}

protected:
	static CBCGPopupMenu*	m_pActivePopupMenu;
	static BOOL				m_bForceMenuFocus;	// Menu takes a focus when activated

	CWnd*					m_pMessageWnd; // By Alex Corazzin (2)

	CBCGToolbarMenuButton*	m_pParentBtn;
	DROP_DIRECTION			m_DropDirection;
	CBCGPopupMenuBar		m_wndMenuBar;
	HMENU					m_hMenu;
	CPoint					m_ptLocation;
	CPoint					m_ptLocationInitial;
	CSize					m_FinalSize;
	int						m_nMenuBarHeight;
	BOOL					m_bAutoDestroy;
	BOOL					m_bTrackMode;

	BOOL					m_bShown;
	BOOL					m_bTobeDstroyed;

	int						m_iMaxWidth;
	int						m_iLogoWidth;

	CBCGMenuPage* const		m_pMenuCustomizationPage;
	CString					m_strCaption;

	static BOOL				m_bMenuSound;

	//----------------------
	// Animation attributes:
	//----------------------
	static ANIMATION_TYPE	m_AnimationType;
	static UINT				m_AnimationSpeed;
	CSize					m_AnimSize;
	BOOL					m_bIsAnimRight;
	BOOL					m_bIsAnimDown;
	BOOL					m_bAnimationIsDone;

	BOOL					m_bScrollable;
	CRect					m_rectScrollUp;
	CRect					m_rectScrollDn;
	int						m_iScrollMode;	// -1 - Up, 0 - None, 1 - Down
	int						m_iScrollBtnHeight;

	CBitmap					m_bmpScreenSrc;
	CBitmap					m_bmpScreenDst;
	CBitmap					m_bmpScreenTmp;

	int						m_iFadePercent;
	
	COLORREF*				m_cFadeSrcBits;
	COLORREF*				m_cFadeDstBits;
	COLORREF*				m_cFadeTmpBits;

	//-------------------
	// Shadow attributes:
	//-------------------
	int						m_iShadowSize;
    static BOOL             m_bForceShadow;     // when TRUE paints shadows even outside main-frame
	CBitmap					m_bmpShadowRight;	// Saved shadow butmaps
	CBitmap					m_bmpShadowBottom;

	//---------------------
	// Tear-off attributes:
	//---------------------
	BOOL					m_bTearOffTracking;
	BOOL					m_bIsTearOffCaptionActive;
	CRect					m_rectTearOffCaption;
	CToolTipCtrl			m_wndToolTip;

// Operations
public:
	void SaveState ();

	CBCGPopupMenu* GetParentPopupMenu () const;
	CBCGToolBar* GetParentToolBar () const;

	CBCGToolbarMenuButton* GetSelItem ();

	int InsertItem (const CBCGToolbarMenuButton& button, int iInsertAt = -1);
	int InsertSeparator (int iInsertAt = -1);

	int GetMenuItemCount () const;
	CBCGToolbarMenuButton* GetMenuItem (int iIndex) const;
	
	BOOL RemoveItem (int iIndex);
	void RemoveAllItems ();

	void SetDefaultItem (UINT uiCmd)
	{
		m_wndMenuBar.m_uiDefaultMenuCmdId = uiCmd;
	}

	static BOOL ActivatePopupMenu (CFrameWnd* pTopFrame, CBCGPopupMenu* pPopupMenu);
	void CloseMenu (BOOL bSetFocusToBar = FALSE);
	void ShowAllCommands ();

	BOOL HideRarelyUsedCommands () const;
	BOOL AreAllCommandsShown () const
	{
		return m_wndMenuBar.m_bAreAllCommandsShown;
	}

	MENUAREA_TYPE CheckArea (const CPoint& ptScreen) const;

	void UpdateShadow (LPRECT lprectScreen = NULL);
	static void UpdateAllShadows (LPRECT lprectScreen = NULL); // Repaint all opened menu shadows

protected:
	void Initialize ();

	BOOL IsScrollUpAvailable ();
	BOOL IsScrollDnAvailable ();

	void CollapseSubmenus ();
	void DrawImage (CDC* pDC, const CRect& rect, int iImage, BOOL bDrawFrame = FALSE);

	BOOL InitMenuBar ();

	void UpdateBottomWindows (BOOL bCheckOnly = FALSE);

	void DoPaint (CDC* pPaintDC);
	void DrawFade (CDC* pPaintDC);

	BOOL AdjustScroll (BOOL bForceMenuBarResize = FALSE);

	virtual BOOL TearOff (CPoint point);
	virtual CControlBar* CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName);

	BOOL NotifyParentDlg (BOOL bActivate);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPopupMenu)
	public:
    virtual BOOL Create(CWnd* pWndParent, int x, int y, HMENU hMenu, BOOL bLocked = FALSE, BOOL bOwnMessage=FALSE); // Add: Alex Corazzin (2)
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPopupMenu)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
#if _MSC_VER >= 1300
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
#else
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#endif
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOPUPMENU_H__A6E1AD75_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
