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

// BCGMouseManager.h: interface for the CBCGMouseManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGMOUSEMANAGER_H__0A141F65_E9CA_11D1_90D2_00A0C9B05590__INCLUDED_)
#define AFX_BCGMOUSEMANAGER_H__0A141F65_E9CA_11D1_90D2_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CBCGMouseManager : public CObject
{
	DECLARE_SERIAL(CBCGMouseManager)

public:
	CBCGMouseManager();
	virtual ~CBCGMouseManager();

// Operations:
public:
	BOOL AddView (int iViewId, UINT uiViewNameResId, UINT uiIconId = 0);
	BOOL AddView (int iId, LPCTSTR lpszViewName, UINT uiIconId = 0);
	UINT GetViewDblClickCommand (int iId) const;

	BOOL LoadState (LPCTSTR lpszProfileName = NULL);
	BOOL SaveState (LPCTSTR lpszProfileName = NULL);

// Customization operations:
	void GetViewNames (CStringList& listOfNames) const;
	int GetViewIdByName (LPCTSTR lpszName) const;
	UINT GetViewIconId (int iViewId) const;

	void SetCommandForDblClick (int iViewId, UINT uiCmd);

// Overrides:
protected:
	virtual void Serialize (CArchive& ar);

// Attributes:
protected:
	CMap<CString, LPCTSTR, int, int> 	m_ViewsNames;
	CMap<int, int, UINT, UINT>			m_ViewsToCommands;
	CMap<int, int, UINT, UINT>			m_ViewsToIcons;
};

extern CBCGMouseManager*	g_pMouseManager;

#endif // !defined(AFX_BCGMOUSEMANAGER_H__0A141F65_E9CA_11D1_90D2_00A0C9B05590__INCLUDED_)
