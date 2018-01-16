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

#if !defined(AFX_BCGMDIFRAMEWND_H__08B9EC04_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_)
#define AFX_BCGMDIFRAMEWND_H__08B9EC04_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGMDIFrameWnd.h : header file
//

#include "bcgcontrolbar.h"
#include "BCGFrameImpl.h"
#include "BCGMainClientAreaWnd.h"
#include "BCGPopupMenu.h"

class COleClientItem;
class CBCGPopupMenu;
class CBCGToolBar;
class CBCGToolbarMenuButton;
class CBCGToolbarButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGMDIFrameWnd frame

class BCGCONTROLBARDLLEXPORT CBCGMDIFrameWnd : public CMDIFrameWnd
{
	friend class CBCGMainClientAreaWnd;
	friend class CBCGToolBar;
	friend class CBCGMenuBar;
	friend class CBCGToolbarsPage;
	friend class CBCGWorkspace;
	friend class CBCGPopupMenu;
	friend class CBCGToolbarCustomize;
	friend class CBCGMiniDockFrameWnd;
	friend class CBCGMDIChildWnd;

	DECLARE_DYNCREATE(CBCGMDIFrameWnd)

protected:
	CBCGMDIFrameWnd();           // protected constructor used by dynamic creation

// Attributes
public:
	BOOL IsMenuBarAvailable () const
	{
		return m_Impl.GetMenuBar () != NULL;
	}

	const CBCGMenuBar* GetMenuBar () const
	{
		return m_Impl.GetMenuBar ();
	}

	UINT GetDefaultResId () const
	{
		return m_Impl.m_nIDDefaultResource;
	}

	CBCGPopupMenu*	GetActivePopup () const
	{
		return CBCGPopupMenu::m_pActivePopupMenu;
	}

	const CObList& GetTearOffBars () const
	{
		return m_Impl.m_listTearOffToolbars;
	}

protected:
	CBCGMainClientAreaWnd	m_wndClientArea;
	HMENU					m_hmenuWindow;
	CBCGFrameImpl			m_Impl;
	BOOL					m_bContextHelp;
	BOOL					m_bDoSubclass;
	UINT					m_uiWindowsDlgMenuId;
	CString					m_strWindowsDlgMenuText;
	BOOL					m_bShowWindowsDlgAlways;
	BOOL					m_bShowWindowsDlgHelpButton;

// Operations
public:
	void EnableDocking (DWORD dwDockStyle);
	void DockControlBarLeftOf (CControlBar* pBar, CControlBar* pLeftOf);

	void SetupToolbarMenu (	CMenu& menu,
							const UINT uiViewUserToolbarCmdFirst,
							const UINT uiViewUserToolbarCmdLast)
	{
		m_Impl.SetupToolbarMenu (menu, uiViewUserToolbarCmdFirst, uiViewUserToolbarCmdLast);
	}

	void EnableWindowsDialog (UINT uiMenuId, 
		LPCTSTR lpszMenuText,
		BOOL bShowAllways = FALSE,
		BOOL bShowHelpButton = FALSE);
	void EnableWindowsDialog (UINT uiMenuId, 
		UINT uiMenuTextResId,
		BOOL bShowAllways = FALSE,
		BOOL bShowHelpButton = FALSE);
		// Enable built-in CBCGWindowsManagerDlg dialog. When bShowAllways
		// is TRUE, show windows dialog everytime; otherwise only instead of
		// the statndard "Windows..." dialog (appear if more than 9 windows 
		// are open)
	void ShowWindowsDialog ();
	
	// Enable/Disable MDI child tabs:
	void EnableMDITabs (BOOL bEnable = TRUE, BOOL bIcons = TRUE,
						CBCGTabWnd::Location tabLocation = CBCGTabWnd::LOCATION_BOTTOM,
						BOOL bHideNoTabs = FALSE, BOOL bTabCloseButton = FALSE);

protected:
	virtual BOOL OnSetMenu (HMENU hmenu);

// Implementation:
	void InitUserToobars (LPCTSTR lpszRegEntry, UINT uiUserToolbarFirst, UINT uiUserToolbarLast)
	{
		m_Impl.InitUserToobars (lpszRegEntry, uiUserToolbarFirst, uiUserToolbarLast);
	}

	void LoadUserToolbars ()
	{
		m_Impl.LoadUserToolbars ();
	}

	void SaveUserToolbars ()
	{
		m_Impl.SaveUserToolbars ();
	}

	void LoadTearOffMenus ()
	{
		m_Impl.LoadTearOffMenus ();
	}

	void SaveTearOffMenus ()
	{
		m_Impl.SaveTearOffMenus ();
	}

	BOOL IsDockStateValid (const CDockState& state)
	{
		return m_Impl.IsDockStateValid (state);
	}

	CBCGToolBar* GetUserBarByIndex (int iIndex) const
	{
		return m_Impl.GetUserBarByIndex (iIndex);
	}

	void SetDockState(const CDockState& state)
	{
		m_Impl.SetDockState (state);
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGMDIFrameWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual HMENU GetWindowMenuPopup (HMENU hMenuBar);
	BOOL ShowPopupMenu (CBCGPopupMenu* pMenuPopup);

public:
	virtual BOOL OnShowPopupMenu (CBCGPopupMenu* /*pMenuPopup*/)
	{
		return TRUE;
	}

	virtual void OnClosePopupMenu (CBCGPopupMenu* pMenuPopup);
	virtual BOOL OnDrawMenuImage (	CDC* /*pDC*/, 
									const CBCGToolbarMenuButton* /*pMenuButton*/, 
									const CRect& /*rectImage*/)
	{
		return FALSE;
	}

	virtual BOOL OnMenuButtonToolHitTest (CBCGToolbarButton* /*pButton*/, TOOLINFO* /*pTI*/)
	{
		return FALSE;
	}

	virtual BOOL GetToolbarButtonToolTipText (CBCGToolbarButton* /*pButton*/, CString& /*strTTText*/)
	{
		return FALSE;
	}

	virtual BOOL OnEraseMDIClientBackground (CDC* /*pDC*/)
	{
		return FALSE;
	}

	virtual void OnDrawMenuLogo (CDC* /*pDC*/, CBCGPopupMenu* /*pMenu*/, const CRect& /*rectLogo*/)	{}
	virtual void OnUpdateFrameMenu (HMENU hMenuAlt);

	virtual BOOL OnTearOffMenu (CBCGPopupMenu* /*pMenuPopup*/, CControlBar* /*pBar*/)
	{
		return TRUE;
	}

	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);

// Implementation
protected:
	virtual ~CBCGMDIFrameWnd();
	virtual COleClientItem*	GetInPlaceActiveItem ();

	// Generated message map functions
	//{{AFX_MSG(CBCGMDIFrameWnd)
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnToolbarCreateNew(WPARAM,LPARAM);
	afx_msg LRESULT OnToolbarDelete(WPARAM,LPARAM);
	afx_msg void OnContextHelp ();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGMDIFRAMEWND_H__08B9EC04_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_)
