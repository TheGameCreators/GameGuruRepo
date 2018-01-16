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

#if !defined(AFX_BCGCOMMANDMANAGER_H__C089C790_8038_11D3_A9E7_005056800000__INCLUDED_)
#define AFX_BCGCOMMANDMANAGER_H__C089C790_8038_11D3_A9E7_005056800000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

/**
 * This class is responsible for:
 * - knowledge about all commands
 * - images for all commands
 * - command categories
 * - default toolbars
 */

#include "bcgcontrolbar.h"
#include "BCGToolBarImages.h"
#include "BCGToolbarButton.h"
#include "CmdUsageCount.h"

class CBCGCommandManager;

BCGCONTROLBARDLLEXPORT CBCGCommandManager* GetCmdMgr();
#define CMD_MGR (*GetCmdMgr())

#ifndef _NO_BCG_LEGACY_
class BCGCONTROLBARDLLEXPORT CImageHash
{
public:
	static UINT	GetImageOfCommand(UINT nID, BOOL bUser = false);
};
#endif

class BCGCONTROLBARDLLEXPORT CBCGCommandManager
{
friend class _STATIC_CREATOR_;
protected:
	CBCGCommandManager();

public:
	virtual ~CBCGCommandManager();

	void CleanUp ();	// Final cleaning

	// Image functions
	void SetCmdImage (UINT uiCmd, int iImage, BOOL bUserImage);
	int  GetCmdImage (UINT uiCmd, BOOL bUserImage) const;
	void ClearCmdImage (UINT uiCmd);
	void ClearAllCmdImages ();

	void EnableMenuItemImage (UINT uiCmd, BOOL bEnable = TRUE);
	
	BOOL IsMenuItemWithoutImage (UINT uiCmd) const
	{
		return m_lstCommandsWithoutImages.Find (uiCmd) != NULL;
	}

	BOOL LoadState (LPCTSTR lpszProfileName);
	BOOL SaveState (LPCTSTR lpszProfileName);

protected:
	CMap<UINT, UINT, int, int>	m_CommandIndex;
	CMap<UINT, UINT, int, int>	m_CommandIndexUser;
	CMap<UINT, UINT, int, int>	m_DefaultImages;

	// List of menu items where image appearance is disabled:
	CList<UINT,UINT>			m_lstCommandsWithoutImages;
};

#endif // !defined(AFX_BCGCOMMANDMANAGER_H__C089C790_8038_11D3_A9E7_005056800000__INCLUDED_)
