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

#if !defined(AFX_BCGWORKSPACE_H__C82F6ED0_8263_11D3_A9EF_005056800000__INCLUDED_)
#define AFX_BCGWORKSPACE_H__C82F6ED0_8263_11D3_A9EF_005056800000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGUserTool.h"

class CBCGMouseManager;
class CBCGContextMenuManager;
class CBCGKeyboardManager;
class CBCGUserToolsManager;
class CBCGShellManager;
class CBCGFrameImpl;
class CBCGMDIFrameWnd;
class CBCGFrameWnd;
class CBCGOleIPFrameWnd;
class CBCGOleDocIPFrameWnd;

class CBCGWorkspace;

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
class CBCGSkinManager;
#endif

// Some accessor functions
BCGCONTROLBARDLLEXPORT CBCGWorkspace* GetWorkspace();
#define WORKSPACE (*GetWorkspace())

#define BCG_DEFAULT_SKINS_DIR	(LPCTSTR)-1

//---------------------------------------------------------------------------------
//  *
//  * This class is useful to store all the customizations at once
//  *
//  *
//  * you may use this as a mixin class for your CWinApp object !
//  *
//  * i.g. "class CMyApp : public CWinApp, public CBCGWorkspace {...};"
//  *
//  * but you may also use it standalone. Just call once UseWorkspaceManager
//  *
//---------------------------------------------------------------------------------
class BCGCONTROLBARDLLEXPORT CBCGWorkspace
{
public:
	CBCGWorkspace (BOOL bResourceSmartUpdate = FALSE);
	virtual ~CBCGWorkspace();

	static BOOL UseWorkspaceManager(LPCTSTR lpszSectionName = NULL);
	LPCTSTR SetRegistryBase(LPCTSTR lpszSectionName = NULL);

	// Saved data version:
	int		GetDataVersionMajor () const	{	return m_iSavedVersionMajor;	}
	int		GetDataVersionMinor () const	{	return m_iSavedVersionMinor;	}

	BOOL	InitMouseManager ();
	BOOL	InitContextMenuManager ();
	BOOL	InitKeyboardManager ();
	BOOL	InitShellManager ();

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	BOOL	InitSkinManager (LPCTSTR lpszSkinsDirectory = BCG_DEFAULT_SKINS_DIR);
				// BCG_DEFAULT_SKINS_DIR is "<program files>\Common Files\BCGSoft\Skins
#endif

	virtual void OnAfterDownloadSkins (const CString& /*strSkinsDirectory*/) {}

	BOOL	EnableUserTools (const UINT uiCmdToolsDummy,
							const UINT uiCmdFirst, const UINT uiCmdLast,
							CRuntimeClass* pToolRTC = RUNTIME_CLASS (CBCGUserTool),
							UINT uArgMenuID = 0, UINT uInitDirMenuID = 0);

	BOOL	EnableTearOffMenus (LPCTSTR lpszRegEntry,
								const UINT uiCmdFirst, const UINT uiCmdLast);


	BOOL IsResourceSmartUpdate () const
	{
		return m_bResourceSmartUpdate;
	}

	CBCGMouseManager*		GetMouseManager ();
	CBCGContextMenuManager*	GetContextMenuManager ();
	CBCGKeyboardManager*	GetKeyboardManager ();
	CBCGUserToolsManager*	GetUserToolsManager ();
	CBCGShellManager*		GetShellManager ();

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	CBCGSkinManager*		GetSkinManager();
#endif

	// Call on of those in CMyApp::InitInstance just after ProcessShellCommand()
	// and before pMainFrame->ShowWindow(). See BCGDevStudioExample
	BOOL LoadState (CBCGMDIFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL LoadState (CBCGFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL LoadState (CBCGOleIPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL LoadState (CBCGOleDocIPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	virtual BOOL LoadState (LPCTSTR lpszSectionName = NULL, CBCGFrameImpl* pFrameImpl = NULL);

	virtual BOOL CleanState (LPCTSTR lpszSectionName = NULL);
	virtual BOOL SaveState (LPCTSTR lpszSectionName = NULL, CBCGFrameImpl* pFrameImpl = NULL);

	BOOL SaveState (CBCGMDIFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL SaveState (CBCGFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL SaveState (CBCGOleIPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);
	BOOL SaveState (CBCGOleDocIPFrameWnd* pFrame, LPCTSTR lpszSectionName = NULL);

	virtual BOOL OnViewDoubleClick (CWnd* pWnd, int iViewId);
	virtual BOOL ShowPopupMenu (UINT uiMenuResId, const CPoint& point, CWnd* pWnd);

	CString	GetRegSectionPath (LPCTSTR szSectionAdd = _T(""));

	// These functions load and store values from the "Custom" subkey
	// To use subkeys of the "Custom" subkey use GetSectionInt() etc.
	// instead
	int		GetInt(LPCTSTR lpszEntry, int nDefault = 0);
	CString	GetString(LPCTSTR lpszEntry, LPCTSTR lpzDefault = _T(""));
	BOOL	GetBinary(LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes);
	BOOL	GetObject(LPCTSTR lpszEntry, CObject& obj);

	BOOL	WriteInt(LPCTSTR lpszEntry, int nValue );
	BOOL	WriteString(LPCTSTR lpszEntry, LPCTSTR lpszValue );
	BOOL	WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
	BOOL	WriteObject(LPCTSTR lpszEntry, CObject& obj);

	
	// These functions load and store values from a given subkey
	// of the "Custom" subkey. For simpler access you may use
	// GetInt() etc.
	int		GetSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nDefault = 0);
	CString	GetSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = _T(""));
	BOOL	GetSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes);
	BOOL	GetSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj);

	BOOL	WriteSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nValue );
	BOOL	WriteSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszValue );
	BOOL	WriteSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes);
	BOOL	WriteSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj);

	// WinHelp override:
	virtual void OnAppContextHelp (CWnd* pWndControl, const DWORD dwHelpIDArray []);

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	virtual void OnSelectSkin ();
#endif

protected:
	friend class CBCGFrameImpl;

	// Overidables for customization
	virtual	void OnClosingMainFrame (CBCGFrameImpl* pFrameImpl);
	
	// called before anything is loaded
	virtual void PreLoadState() {}

	// called after everything is loaded
	virtual void LoadCustomState() {}

	// called before anything is saved
	virtual void PreSaveState() {}

	// called after everything is saved
	virtual void SaveCustomState() {}

	virtual BOOL LoadWindowPlacement (
					CRect& rectNormalPosition, int& nFflags, int& nShowCmd);
	virtual BOOL StoreWindowPlacement (
				const CRect& rectNormalPosition, int nFflags, int nShowCmd);
protected:
	CString	m_strRegSection;

	BOOL	m_bKeyboardManagerAutocreated;
	BOOL	m_bContextMenuManagerAutocreated;
	BOOL	m_bMouseManagerAutocreated;
	BOOL	m_bUserToolsManagerAutoCreated;
	BOOL	m_bTearOffManagerAutoCreated;
	BOOL	m_bSkinManagerAutocreated;
	BOOL	m_bShellManagerAutocreated;

	int		m_iSavedVersionMajor;
	int		m_iSavedVersionMinor;

	BOOL	m_bForceDockStateLoad;	// Load dock bars state even it's not valid
	BOOL	m_bLoadSaveFrameBarsOnly;

	BOOL	m_bSaveState;			// Automatically save state when the main frame is closed.

	const BOOL	m_bResourceSmartUpdate;	// Automatic toolbars/menu resource update
	BOOL	m_bForceImageReset;			// Force image reset every time when the frame is loaded
};

#endif // !defined(AFX_BCGWORKSPACE_H__C82F6ED0_8263_11D3_A9EF_005056800000__INCLUDED_)
