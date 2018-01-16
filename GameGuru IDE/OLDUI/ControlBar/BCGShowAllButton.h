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

// BCGShowAllButton.h: interface for the CBCGShowAllButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGSHOWALLBUTTON_H__DE028D43_36EB_11D3_95C5_00A0C9289F1B__INCLUDED_)
#define AFX_BCGSHOWALLBUTTON_H__DE028D43_36EB_11D3_95C5_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGToolbarMenuButton.h"

class CBCGShowAllButton : public  CBCGToolbarMenuButton  
{
	DECLARE_DYNCREATE(CBCGShowAllButton)

public:
	CBCGShowAllButton();
	virtual ~CBCGShowAllButton();

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual BOOL OpenPopupMenu (CWnd* pWnd = NULL);
	virtual BOOL OnToolHitTest (const CWnd* pWnd, TOOLINFO* pTI);
};

#endif // !defined(AFX_BCGSHOWALLBUTTON_H__DE028D43_36EB_11D3_95C5_00A0C9289F1B__INCLUDED_)
