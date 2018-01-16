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

// BCGControlBarImpl.h: interface for the CBCGControlBarImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGCONTROLBARIMPL_H__F8388FA2_30D5_11D3_95C5_00A0C9289F1B__INCLUDED_)
#define AFX_BCGCONTROLBARIMPL_H__F8388FA2_30D5_11D3_95C5_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CBCGControlBarImpl  
{
	friend class CBCGToolBar;
	friend class CBCGDialogBar;
	friend class CBCGSizingControlBar;

public:
	CBCGControlBarImpl(CControlBar* pBar);
	virtual ~CBCGControlBarImpl();

// Attributes:
protected:
	CControlBar*	m_pBar;

// Operations:
protected:
	void CalcNcSize (NCCALCSIZE_PARAMS FAR* lpncsp);

	void DrawNcArea ();
	BOOL GetBackgroundFromParent (CDC* pDC);
};

#endif // !defined(AFX_BCGCONTROLBARIMPL_H__F8388FA2_30D5_11D3_95C5_00A0C9289F1B__INCLUDED_)
