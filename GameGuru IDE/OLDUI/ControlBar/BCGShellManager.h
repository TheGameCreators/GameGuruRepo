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
// BCGShellManager.h: interface for the CBCGShellManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGSHELLMANAGER_H__E19B2D53_BD01_4B0B_94B9_EB32C246CFA5__INCLUDED_)
#define AFX_BCGSHELLMANAGER_H__E19B2D53_BD01_4B0B_94B9_EB32C246CFA5__INCLUDED_

#ifndef _SHLOBJ_H_
#include <shlobj.h>
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"

typedef struct _BCGCBITEMINFO
{
	LPSHELLFOLDER  pParentFolder;
	LPITEMIDLIST   pidlFQ;
	LPITEMIDLIST   pidlRel;

	_BCGCBITEMINFO()
	{
		pParentFolder = NULL;
		pidlFQ = NULL;
		pidlRel = NULL;
	}
}
BCGCBITEMINFO, FAR *LPBCGCBITEMINFO;

class BCGCONTROLBARDLLEXPORT CBCGShellManager : public CObject
{
	friend class CBCGShellList;

// Construction
public:
	CBCGShellManager();
	virtual ~CBCGShellManager();

// Operations
public:
	BOOL BrowseForFolder (	CString& strOutFolder,
							CWnd* pWndParent = NULL,
							LPCTSTR lplszInitialFolder = NULL,
							LPCTSTR lpszTitle = NULL, 
							UINT ulFlags = BIF_RETURNONLYFSDIRS, // Same as flags in BROWSEINFO structure
							LPINT piFolderImage = NULL);

	LPITEMIDLIST CreateItem (UINT cbSize);
	void FreeItem (LPITEMIDLIST pidl);

	UINT GetItemCount (LPCITEMIDLIST pidl);
	UINT GetItemSize (LPCITEMIDLIST pidl);

	LPITEMIDLIST ConcatenateItem (LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
	LPITEMIDLIST CopyItem (LPCITEMIDLIST pidlSource);

	LPITEMIDLIST GetNextItem(LPCITEMIDLIST pidl);
	int GetParentItem (LPCITEMIDLIST lpidl, LPITEMIDLIST& lpidlParent);

	HRESULT ItemFromPath (LPCTSTR lpszPath, LPITEMIDLIST& pidl);

protected:
	static int CALLBACK BrowseCallbackProc (HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

// Attributes
protected:
	LPMALLOC	m_pBCGMalloc;
	LPCTSTR		m_lpszInitialPath;	// Valid in BrowseForFolder only
};

extern CBCGShellManager* g_pShellManager;

#endif // !defined(AFX_BCGSHELLMANAGER_H__E19B2D53_BD01_4B0B_94B9_EB32C246CFA5__INCLUDED_)
