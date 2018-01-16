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

// BCGContextMenuManager.h: interface for the CBCGContextMenuManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGCONTEXTMENUMANAGER_H__64F13A54_E5A9_11D1_A08F_00A0C9B05590__INCLUDED_)
#define AFX_BCGCONTEXTMENUMANAGER_H__64F13A54_E5A9_11D1_A08F_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"

class CBCGPopupMenu;

class BCGCONTROLBARDLLEXPORT CBCGContextMenuManager : public CObject
{
	friend class CBCGPopupMenuBar;

public:
	CBCGContextMenuManager();
	virtual ~CBCGContextMenuManager();

// Opreations:
public:
	BOOL AddMenu (UINT uiMenuNameResId, UINT uiMenuResId);
	BOOL AddMenu (LPCTSTR lpszName, UINT uiMenuResId);

	BOOL ShowPopupMenu (UINT uiMenuResId, int x, int y, CWnd* pWndOwner, BOOL bOwnMessage = FALSE);
	CBCGPopupMenu* ShowPopupMenu (HMENU hmenuPopup, 
								int x, int y, 
								CWnd* pWndOwner, BOOL bOwnMessage = FALSE,
								BOOL bAutoDestroy = TRUE);
	UINT TrackPopupMenu (HMENU hmenuPopup, int x, int y, CWnd* pWndOwner);

	BOOL LoadState (LPCTSTR lpszProfileName = NULL);
	BOOL SaveState (LPCTSTR lpszProfileName = NULL);

	BOOL ResetState ();

// Customization operations:
	void GetMenuNames (CStringList& listOfNames) const;
	HMENU GetMenuByName (LPCTSTR lpszName, UINT* puiOrigResID = NULL) const;

// Attributes:
protected:
	CMap<UINT, UINT, HMENU, HMENU>			m_Menus;
	CMap<CString, LPCTSTR, HMENU, HMENU> 	m_MenuNames;
	UINT									m_nLastCommandID;
	BOOL									m_bTrackMode;
};

extern CBCGContextMenuManager*	g_pContextMenuManager;

#endif // !defined(AFX_BCGCONTEXTMENUMANAGER_H__64F13A54_E5A9_11D1_A08F_00A0C9B05590__INCLUDED_)
