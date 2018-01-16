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

// BCGToolbarRegularMenuButton.h: interface for the CBCGToolbarRegularMenuButton class.
// By Sven Ritter
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARREGULARMENUBUTTON_H__0321FCC6_8BD5_11D3_A711_0090274409AC__INCLUDED_)
#define AFX_BCGTOOLBARREGULARMENUBUTTON_H__0321FCC6_8BD5_11D3_A711_0090274409AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGToolbarMenuButton.h"

class BCGCONTROLBARDLLEXPORT CBCGToolbarRegularMenuButton : public CBCGToolbarMenuButton  
{
	DECLARE_SERIAL (CBCGToolbarRegularMenuButton)

public:
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
};

#endif // !defined(AFX_BCGTOOLBARREGULARMENUBUTTON_H__0321FCC6_8BD5_11D3_A711_0090274409AC__INCLUDED_)
