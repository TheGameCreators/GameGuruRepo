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
// RebarManager.h: interface for the CRebarManager class.
// By Nick Hodapp
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGREBARMANAGER_H__872AD541_859C_11D3_8735_FCF55404122E__INCLUDED_)
#define AFX_BCGREBARMANAGER_H__872AD541_859C_11D3_8735_FCF55404122E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CBCGRebarState  
{
private:

	static BOOL CALLBACK LoadRebarStateProc(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK SaveRebarStateProc(HWND hwnd, LPARAM lParam);

public:
	static void LoadState (CString& strKey, CFrameWnd* pFrrame);
	static void SaveState (CString& strKey, CFrameWnd* pFrrame);
};

#endif // !defined(AFX_BCGREBARMANAGER_H__872AD541_859C_11D3_8735_FCF55404122E__INCLUDED_)
