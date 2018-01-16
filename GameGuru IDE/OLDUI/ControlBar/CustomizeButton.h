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

// CustomizeButton.h: interface for the CCustomizeButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUSTOMIZEBUTTON_H__7DC72143_689E_11D3_95C6_00A0C9289F1B__INCLUDED_)
#define AFX_CUSTOMIZEBUTTON_H__7DC72143_689E_11D3_95C6_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGToolbarMenuButton.h"

class CBCGToolBar;

class CCustomizeButton : public  CBCGToolbarMenuButton
{
	friend class CBCGToolBar;

	DECLARE_SERIAL(CCustomizeButton)

public:
	CCustomizeButton();
	CCustomizeButton(UINT uiCustomizeCmdId, const CString& strCustomizeText);
	virtual ~CCustomizeButton();

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual CBCGPopupMenu* CreatePopupMenu ();

	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual BOOL IsEmptyMenuAllowed () const
	{
		return TRUE;
	}

	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);

	virtual BOOL IsEditable () const
	{
		return FALSE;
	}

	virtual BOOL CanBeStored () const			{	return FALSE;	}

protected:
	UINT			m_uiCustomizeCmdId;
	CString			m_strCustomizeText;
	CBCGToolBar*	m_pWndParentToolbar;
	BOOL			m_bIsEmpty;

	CObList			m_lstInvisibleButtons;	// List of invisible butons on 
											// the parent toolbar.
};

#endif // !defined(AFX_CUSTOMIZEBUTTON_H__7DC72143_689E_11D3_95C6_00A0C9289F1B__INCLUDED_)
