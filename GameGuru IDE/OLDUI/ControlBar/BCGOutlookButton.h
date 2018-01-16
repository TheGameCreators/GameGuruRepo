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

// BCGOutlookButton.h: interface for the CBCGOutlookButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGOUTLOOKBUTTON_H__EF4340F6_720D_11D3_A746_0090274409AC__INCLUDED_)
#define AFX_BCGOUTLOOKBUTTON_H__EF4340F6_720D_11D3_A746_0090274409AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGToolbarButton.h"

class CBCGOutlookBar;

class BCGCONTROLBARDLLEXPORT CBCGOutlookButton : public CBCGToolbarButton  
{
	friend class CBCGOutlookBar;

	DECLARE_SERIAL(CBCGOutlookButton)

public:
	CBCGOutlookButton();
	virtual ~CBCGOutlookButton();

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual void OnChangeParentWnd (CWnd* pWndParent);

	virtual BOOL CanBeDropped (CBCGToolBar* pToolbar);
	virtual void SetImage (int iImage);

	virtual void Serialize (CArchive& ar);
	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual BOOL CompareWith (const CBCGToolbarButton& other) const;

protected:
	CBCGOutlookBar*	m_pWndParentBar;
	UINT			m_uiPageID;
	CSize			m_sizeImage;
	BOOL			m_bIsWholeText;
};

#endif // !defined(AFX_BCGOUTLOOKBUTTON_H__EF4340F6_720D_11D3_A746_0090274409AC__INCLUDED_)
