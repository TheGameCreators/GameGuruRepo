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

#if !defined(AFX_BCGDOCKBAR_H__9440A463_E272_11D1_A089_00A0C9B05590__INCLUDED_)
#define AFX_BCGDOCKBAR_H__9440A463_E272_11D1_A089_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGDockBar.h : header file
//

#include "BCGSizingControlBar.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGDockBar dummy class for access to protected members

class CBCGDockBar : public CDockBar
{
	friend class CBCGMenuBar;
	friend class CBCGSizingControlBar;
	friend class CBCGToolbarCustomize;
	friend class CBCGMiniDockFrameWnd;

	void SetCustomizationMode (BOOL bCustMode)
	{
		ASSERT_VALID (this);

#ifndef BCG_NO_SIZINGBAR

		CWnd* pWndChild = GetWindow (GW_CHILD);
		while (pWndChild != NULL)
		{
			CBCGSizingControlBar* pWndSizingCb = DYNAMIC_DOWNCAST (CBCGSizingControlBar, 
				pWndChild);

			if (pWndSizingCb != NULL)
			{
				pWndSizingCb->SetCustomizationMode (bCustMode);
			}

			pWndChild = pWndChild->GetNextWindow ();
		}

#endif // BCG_NO_SIZINGBAR

	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGDOCKBAR_H__9440A463_E272_11D1_A089_00A0C9B05590__INCLUDED_)
