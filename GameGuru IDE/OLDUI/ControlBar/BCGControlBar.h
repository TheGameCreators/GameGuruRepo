#ifndef __BCGCONTROLBAR_H
#define __BCGCONTROLBAR_H

// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __AFXCMN_H__
	#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_
	#ifdef _BCGCONTROLBAR_
	   #define BCGCONTROLBARDLLEXPORT  _declspec(dllexport)
	#else
	   #define BCGCONTROLBARDLLEXPORT  _declspec(dllimport)
	#endif
#else
	#define BCGCONTROLBARDLLEXPORT
#endif

#include "BCGUserToolsManager.h"

inline BOOL IsStandardCommand (UINT uiCmd)
{
	return	((uiCmd >= ID_FILE_MRU_FILE1 && 
				uiCmd <= ID_FILE_MRU_FILE16)		||	// MRU commands,
			(uiCmd >= 0xF000 && uiCmd < 0xF1F0)		||	// system commands,
			((int) uiCmd >= AFX_IDM_FIRST_MDICHILD)	||	// windows commands
			(uiCmd >= ID_OLE_VERB_FIRST && uiCmd <= ID_OLE_VERB_LAST) ||		// OLE commands
			g_pUserToolsManager != NULL && uiCmd == g_pUserToolsManager->GetToolsEntryCmd ());
}

BCGCONTROLBARDLLEXPORT void BCGCBSetResourceHandle (
	HINSTANCE hinstResDLL);

BCGCONTROLBARDLLEXPORT void BCGCBCleanUp ();

extern CFrameWnd* g_pMyTopLevelFrame;

inline BCGCONTROLBARDLLEXPORT void BCGSetTopLevelFrame (CFrameWnd* pFrame)
{
	g_pMyTopLevelFrame = pFrame;
}

inline BCGCONTROLBARDLLEXPORT CFrameWnd* BCGGetTopLevelFrame (const CWnd* pWnd)
{
	ASSERT_VALID (pWnd);
	return g_pMyTopLevelFrame == NULL ? pWnd->GetTopLevelFrame () : g_pMyTopLevelFrame;
}

#define BCG_GET_X_LPARAM(lp)		((int)(short)LOWORD(lp))
#define BCG_GET_Y_LPARAM(lp)		((int)(short)HIWORD(lp))

#endif // __BCGCONTROLBAR_H
