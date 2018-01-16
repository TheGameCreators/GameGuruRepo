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

#if !defined(AFX_BCGMENUBAR_H__644FE23B_D1DB_11D1_A649_00A0C93A70EC__INCLUDED_)
#define AFX_BCGMENUBAR_H__644FE23B_D1DB_11D1_A649_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGMenuBar.h : header file
//

#include "BCGToolBar.h"
#include "bcgcontrolbar.h"

class CBCGToolbarMenuButton;
class CBCGToolbarSystemMenuButton;
class CBCGToolbarMenuButtonsButton;
class CBCGMenuPage;

#define BCG_IDW_MENUBAR               0xE806  // CBCGMenuBar

/////////////////////////////////////////////////////////////////////////////
// CBCGMenuBar window

class BCGCONTROLBARDLLEXPORT CBCGMenuBar : public CBCGToolBar
{
	friend class CBCGOptionsPage;
	friend class CBCGToolBar;
	friend class CBCGPopupMenuBar;
	friend class CBCGMenuPage;

	DECLARE_SERIAL(CBCGMenuBar)

// Construction
public:
	CBCGMenuBar();

// Attributes
public:
	static BOOL IsShowAllCommands ()
	{
		return	m_bShowAllCommands || !m_bRecentlyUsedMenus ||
				!m_UsageCount.HasEnouthInformation ();
	}

	static BOOL IsRecentlyUsedMenus ()
	{
		return m_bRecentlyUsedMenus;
	}

	static BOOL IsShowAllCommandsDelay ()
	{
		return m_bShowAllMenusDelay;
	}

	static void SetShowAllCommands (BOOL bShowAllCommands = TRUE);
	static void SetRecentlyUsedMenus (BOOL bOn = TRUE);

	static BOOL SetMenuFont (LPLOGFONT lpLogFont, BOOL bHorz = TRUE);
	static const CFont& GetMenuFont (BOOL bHorz = TRUE);

	static void EnableMenuShadows (BOOL bEnable = TRUE)
	{
		m_bMenuShadows = bEnable;
	}

	static BOOL IsMenuShadows ()
	{
		return m_bMenuShadows;
	}

	static void HighlightDisabledItems (BOOL bHighlight = TRUE)
	{
		m_bHighlightDisabledItems = bHighlight;
	}

	static BOOL IsHighlightDisabledItems ()
	{
		return m_bHighlightDisabledItems ;
	}

	BOOL	m_bAutoDocMenus;	// Automatically load/save  document menus

    // By Oz Solomonovich
    BOOL GetForceDownArrows() { return m_bForceDownArrows; }
    void SetForceDownArrows(BOOL bValue) { m_bForceDownArrows = bValue; }
    ///

protected:
	CWnd*			m_pParentWnd;
	BOOL			m_bMaximizeMode;
	HMENU			m_hMenu;
	HMENU			m_hDefaultMenu;
	HMENU			m_hSysMenu;
	HICON			m_hSysIcon;
	UINT			m_uiDefMenuResId;
	int				m_nSystemButtonsNum;
	int				m_nSystemButtonsNumSaved;
	BOOL			m_bHaveButtons;
	CSize			m_szSystemButton;	// System button size
	CBCGMenuPage*	m_pMenuPage;
    BOOL			m_bForceDownArrows;

	static BOOL	m_bShowAllCommands;
	static BOOL	m_bRecentlyUsedMenus;
	static BOOL	m_bShowAllMenusDelay;

	static HHOOK		m_hookMouseDrag;
	static CBCGMenuBar*	m_pDraggedMenuBar;

	static BOOL m_bMenuShadows;
	static BOOL m_bHighlightDisabledItems;

// Operations
public:
	virtual void CreateFromMenu (HMENU hMenu, BOOL bDefaultMenu = FALSE,
						BOOL bForceUpdate = FALSE);
	void SetMaximizeMode (BOOL bMax, CWnd* pWnd = NULL, BOOL bRecalcLayout = TRUE);

	void SetDefaultMenuResId (UINT uiResId);
	UINT GetDefaultMenuResId () const
	{
		return m_uiDefMenuResId;
	}

	CMenu* GetMenu () const
	{
		return (CMenu*)m_hMenu;
	}

	HMENU GetDefaultMenu () const
	{
		return m_hDefaultMenu;
	}

	CBCGToolbarButton* GetMenuItem (int iItem) const;
	CBCGToolbarSystemMenuButton* GetSystemMenu () const;
	CBCGToolbarMenuButtonsButton* GetSystemButton (UINT uiBtn, BOOL bByCommand = TRUE) const;
	int GetSystemButtonsCount () const
	{
		return m_nSystemButtonsNum;
	}

protected:
	void RestoreMaximizeMode ();
	void SetDragMode (BOOL bOn);

	static LRESULT CALLBACK BCGMenuBarMouseProc (int nCode, WPARAM wParam, LPARAM lParam);
	static BOOL FindMenuItemText (HMENU hMenu, const UINT nID, CString& strText);

	void CalcSysButtonSize ();
	
	BOOL BuildOrigItems (UINT uiMenuResID); // Required for the SmartResourceUpdate
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGMenuBar)
	//}}AFX_VIRTUAL

public:
	virtual BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = dwDefaultToolbarStyle,
			UINT nID = BCG_IDW_MENUBAR);
	virtual BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = TBSTYLE_FLAT,
			DWORD dwStyle = dwDefaultToolbarStyle,
			CRect rcBorders = CRect(1, 1, 1, 1),
			UINT nID = BCG_IDW_MENUBAR);

	virtual CSize CalcDynamicLayout (int nLength, DWORD dwMode);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcLayout(DWORD dwMode, int nLength = -1);

	virtual int CalcMaxButtonHeight ();

	virtual void AdjustLocations ();
	virtual BOOL OnSendCommand (const CBCGToolbarButton* pButton);

	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual BOOL CanBeRestored () const
	{
		return TRUE;
	}

	virtual BOOL CanBeClosed () const
	{
		return FALSE;
	}

	virtual BOOL AllowChangeTextLabels () const
	{
		return FALSE;
	}

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return FALSE;
	}

	// ET: Hook for dynamic changes of the default menu
	virtual void OnDefaultMenuLoaded (HMENU) {};
	virtual BOOL RestoreOriginalstate ();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual int GetRowHeight () const
	{
		return max (globalData.GetTextHeight (m_dwStyle & CBRS_ORIENT_HORZ),
			m_bHaveButtons ?
			GetButtonSize ().cy :
				m_sizeMenuButton.cy <= 0 ? 
					m_sizeButton.cy - 2 : m_sizeMenuButton.cy - 2);
	}

	virtual int GetColumnWidth () const
	{
		return m_bHaveButtons ?
			GetButtonSize ().cx :
				m_sizeMenuButton.cx <= 0 ? 
					m_sizeButton.cx - 2 : m_sizeMenuButton.cx - 2;
	}

	virtual BOOL OnSetDefaultButtonText (CBCGToolbarButton* pButton);
	virtual void OnChangeHot (int iHot);

protected:
	virtual int FindDropIndex (const CPoint point, CRect& rectDrag) const;
	virtual void ResetImages ();

	virtual BOOL IsPureMenuButton (CBCGToolbarButton* pButton) const;

	// Implementation
public:
	virtual ~CBCGMenuBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGMenuBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGMENUBAR_H__644FE23B_D1DB_11D1_A649_00A0C93A70EC__INCLUDED_)
