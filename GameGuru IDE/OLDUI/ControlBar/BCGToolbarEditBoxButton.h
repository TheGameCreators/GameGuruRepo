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

// BCGToolbarEditBoxButton.h: interface for the CBCGToolbarEditBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBAREDITBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)
#define AFX_BCGTOOLBAREDITBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "bcgcontrolbar.h"
#include "BCGToolbarButton.h"

class BCGCONTROLBARDLLEXPORT CBCGToolbarEditBoxButton : public CBCGToolbarButton  
{
	friend class CBCGToolbarEditCtrl;

	DECLARE_SERIAL(CBCGToolbarEditBoxButton)

public:
	CBCGToolbarEditBoxButton();
	CBCGToolbarEditBoxButton(UINT uiID, int iImage, DWORD dwStyle = ES_AUTOHSCROLL, int iWidth = 0);
	virtual ~CBCGToolbarEditBoxButton();

// Operations:

// Overrides:
	virtual CEdit* CreateEdit (CWnd* pWndParent, const CRect& rect);

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void OnMove ();
	virtual void OnSize (int iSize);
	virtual HWND GetHwnd ()
	{	
		return m_pWndEdit->GetSafeHwnd ();
	}
	virtual BOOL NotifyCommand (int iNotifyCode);
	
	virtual BOOL CanBeStretched () const
	{	
		return TRUE;	
	}
	virtual void OnAddToCustomizePage ();
	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);
	virtual BOOL HaveHotBorder () const
	{
		return m_pWndEdit->GetSafeHwnd () == NULL ||
			(m_pWndEdit->GetStyle () & WS_VISIBLE) == 0;
	}

	virtual int OnDrawOnCustomizeList (
			CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual void OnShow (BOOL bShow);
	virtual void SetContents(const CString& sContents);

	virtual const CRect GetInvalidateRect () const;
	virtual void SetStyle (UINT nStyle);

	static void SetFlatMode (BOOL bFlat = TRUE)
	{
		m_bFlat = bFlat;
	}

	static BOOL IsFlatMode ()
	{
		return m_bFlat;
	}

protected:
	void Initialize ();
	void SetHotEdit (BOOL bHot);

// Attributes:
public:
	CEdit* GetEditBox () const
	{
		return m_pWndEdit;
	}

	static CBCGToolbarEditBoxButton* GetByCmd (UINT uiCmd);
	static BOOL SetContentsAll (UINT uiCmd, const CString& strContents);
	static CString GetContentsAll (UINT uiCmd);

protected:
	DWORD				m_dwStyle;
	CEdit*				m_pWndEdit;
	CString				m_strContents;
	BOOL				m_bChangingText;

	int					m_iWidth;

	BOOL				m_bHorz;
	BOOL				m_bIsHotEdit;

	static BOOL			m_bFlat;
};

#endif // !defined(AFX_BCGTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)
