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
// BCGSkinManager.h: interface for the CBCGSkinManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGSKINMANAGER_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_)
#define AFX_BCGSKINMANAGER_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version

#include "bcgcontrolbar.h"
#include "BCGSkinEntry.h"
#include "BCGWorkspace.h"

#define BCG_DEFUALT_SKIN	(-1)

#pragma warning (disable : 4244)

class BCGCONTROLBARDLLEXPORT CBCGSkinManager : public CObject
{
	friend class CBCGSelectSkinDlg;

public:
	CBCGSkinManager (LPCTSTR lpszSkinsDirectory = BCG_DEFAULT_SKINS_DIR);
	virtual ~CBCGSkinManager ();

// Opreations:
public:
	BOOL AddSkinLibrary (const CString& strLibraryPath, BOOL bLoadLibrary = TRUE);

	int GetSkinsCount () const
	{
		return m_Skins.GetSize ();
	}

	LPCTSTR GetSkinName (int iSkinIndex) const;
	LPCTSTR GetSkinAuthor (int iSkinIndex) const;
	LPCTSTR GetSkinAuthorMail (int iSkinIndex) const;
	LPCTSTR GetSkinAuthorURL (int iSkinIndex) const;

	BOOL PreviewSkin (CDC* pDC, int iSkinIndex, CRect rect);
	
	BOOL SetActiveSkin (int iSkinIndex);
	int GetActiveSkin () const
	{
		return m_iActiveSkin;
	}

	BOOL LoadState (LPCTSTR lpszProfileName = NULL);
	BOOL SaveState (LPCTSTR lpszProfileName = NULL);

	BOOL ShowSelectSkinDlg ();

	void EnableSkinsDownload (LPCTSTR lpszURL,
							LPCTSTR lpszUserName = NULL,
							LPCTSTR lpszPassword = NULL,
							LPCTSTR lpszDownloadDLLName = NULL);
	BOOL IsDownloadAvailable () const
	{
		return !m_strSkinsURL.IsEmpty ();
	}

	virtual BOOL DownloadSkins ();

protected:
	virtual void ScanSkinsLocation ();
	virtual void RemoveAllSkins ();
	virtual void LoadAllSkins ();
	virtual void UnLoadAllSkins ();

	virtual BOOL RenameTempLibs ();

// Attributes:
protected:
	int											m_iActiveSkin;
	CArray<CBCGSkinEntry, CBCGSkinEntry&>		m_Skins;
	CArray<CBCGSkinLibrary, CBCGSkinLibrary&>	m_SkinLibraresInstances;

	CString	m_strSkinsDirectory;
	CString	m_strSkinsURL;
	CString	m_strDownloadDllName;
	CString	m_strUserName;
	CString	m_strUserPassword;
};

extern CBCGSkinManager*	g_pSkinManager;

#pragma warning (default : 4244)

#endif // _AFXDLL

#endif // !defined(AFX_BCGSKINMANAGER_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_)
