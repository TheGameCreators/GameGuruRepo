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

#if !defined(AFX_BCGOLEDOCIPFRAMEWND_H__D6152532_1ACC_48E8_9A46_F9BBFB227B87__INCLUDED_)
#define AFX_BCGOLEDOCIPFRAMEWND_H__D6152532_1ACC_48E8_9A46_F9BBFB227B87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGOleDocIPFrameWnd.h : header file
//

#ifndef __AFXDOCOB_H__
#include <afxdocob.h>
#endif

#include "BCGFrameImpl.h"
#include "bcgcontrolbar.h"
#include "BCGPopupMenu.h"

class CBCGMenuBar;
class CBCGToolbarMenuButton;
class CBCGToolbarButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGOleDocIPFrameWnd frame

class BCGCONTROLBARDLLEXPORT CBCGOleDocIPFrameWnd : public COleDocIPFrameWnd
{
	friend class CBCGMenuBar;
	friend class CBCGToolBar;
	friend class CToolbarsPage;
	friend class CBCGWorkspace;
	friend class CBCGPopupMenu;
	friend class CBCGMiniDockFrameWnd;

	DECLARE_DYNCREATE(CBCGOleDocIPFrameWnd)

// Construction
protected:
	CBCGOleDocIPFrameWnd();           // protected constructor used by dynamic creation

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
	HMENU			m_hmenuWindow;
	CBCGFrameImpl	m_Impl;
	BOOL			m_bContextHelp;
	HWND			m_hwndLastTopLevelFrame;

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

	void SetDockState(const CDockState& state)
	{
		m_Impl.SetDockState (state);
	}

protected:

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

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGOleDocIPFrameWnd)
	public:
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBCGOleDocIPFrameWnd();

	BOOL OnSetMenu (HMENU hmenu);
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

	virtual void OnDrawMenuLogo (CDC* /*pDC*/, CBCGPopupMenu* /*pMenu*/, const CRect& /*rectLogo*/)	{}

	virtual BOOL OnTearOffMenu (CBCGPopupMenu* /*pMenuPopup*/, CControlBar* /*pBar*/)
	{
		return TRUE;
	}

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGOleDocIPFrameWnd)
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnSetMenu (WPARAM wp, LPARAM);
	afx_msg LRESULT OnToolbarCreateNew(WPARAM,LPARAM);
	afx_msg LRESULT OnToolbarDelete(WPARAM,LPARAM);
	afx_msg void OnContextHelp ();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGOLEDOCIPFRAMEWND_H__D6152532_1ACC_48E8_9A46_F9BBFB227B87__INCLUDED_)
