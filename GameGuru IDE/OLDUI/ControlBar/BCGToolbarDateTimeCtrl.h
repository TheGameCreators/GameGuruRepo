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

// BCGToolbarDateTimeCtrl.h: interface for the CBCGToolbarDateTimeCtrl class.
//
//////////////////////////////////////////////////////////////////////
//
//
// Created by Doug Swallow 
//

#if !defined(AFX_BCGTOOLBARDATETIMEPICKER_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)
#define AFX_BCGTOOLBARDATETIMEPICKER_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXDTCTL_H__
	#include <afxdtctl.h>
#endif

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "BCGToolbarButton.h"

class CBCGDateTimeCtrlWin : public CDateTimeCtrl
{
public:
	CBCGDateTimeCtrlWin () : m_bMonthCtrlDisplayed(false) {}

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGDateTimeCtrlWin)
	afx_msg void OnDateTimeChange (NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnDateTimeDropDown (NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnDateTimeCloseUp (NMHDR* pNotifyStruct, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	bool m_bMonthCtrlDisplayed;
};


class CBCGToolbarMenuButton;

class BCGCONTROLBARDLLEXPORT CBCGToolbarDateTimeCtrl : public CBCGToolbarButton  
{
	friend class CBCGDateTimeCtrlWin;

	DECLARE_SERIAL (CBCGToolbarDateTimeCtrl)

public:
	CBCGToolbarDateTimeCtrl ();
	CBCGToolbarDateTimeCtrl (UINT uiID, int iImage, DWORD dwStyle = 0, int iWidth = 0);
	virtual ~CBCGToolbarDateTimeCtrl ();

// Operations:

// Overrides:
	virtual CBCGDateTimeCtrlWin* CreateDateTimeCtrl (CWnd* pWndParent, const CRect& rect);

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
		return m_pWndDateTime->GetSafeHwnd ();
	}
	virtual BOOL NotifyCommand (int iNotifyCode);
	
	virtual BOOL CanBeStretched () const
	{	
		return TRUE;	
	}
	virtual void OnAddToCustomizePage ();
	virtual HBRUSH OnCtlColor (CDC* pDC, UINT nCtlColor);
	virtual BOOL HaveHotBorder () const
	{
		return m_pWndDateTime->GetSafeHwnd () == NULL ||
			(m_pWndDateTime->GetStyle () & WS_VISIBLE) == 0;
	}

	virtual int OnDrawOnCustomizeList (
			CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual void DuplicateData () {}
	virtual void OnShow (BOOL bShow);
	virtual BOOL ExportToMenuButton (CBCGToolbarMenuButton& menuButton) const;

	virtual void SetStyle (UINT nStyle);

protected:
	void Initialize ();
	void AdjustRect ();

// Attributes:
public:
	CDateTimeCtrl* GetDateTimeCtrl () const
	{
		return m_pWndDateTime;
	}

	BOOL SetTime (LPSYSTEMTIME pTimeNew = NULL);
	BOOL SetTime (const COleDateTime& timeNew);
	BOOL SetTime (const CTime* timeNew);
	DWORD GetTime (LPSYSTEMTIME pTimeDest) const {return m_pWndDateTime->GetTime(pTimeDest);}
	BOOL GetTime (COleDateTime& timeDest) const {return m_pWndDateTime->GetTime(timeDest);}
	DWORD GetTime (CTime& timeDest) const {return m_pWndDateTime->GetTime(timeDest);}

	static CBCGToolbarDateTimeCtrl* GetByCmd (UINT uiCmd);
	static BOOL SetTimeAll (UINT uiCmd, LPSYSTEMTIME pTimeNew = NULL);
	static BOOL SetTimeAll (UINT uiCmd, const COleDateTime& timeNew);
	static BOOL SetTimeAll (UINT uiCmd, const CTime* pTimeNew);
	static DWORD GetTimeAll (UINT uiCmd, LPSYSTEMTIME pTimeDest);
	static BOOL GetTimeAll (UINT uiCmd, COleDateTime& timeDest);
	static DWORD GetTimeAll (UINT uiCmd, CTime& timeDest);

protected:
	DWORD					m_dwStyle;
	CBCGDateTimeCtrlWin*	m_pWndDateTime;

	int						m_iWidth;

	BOOL					m_bHorz;

	DWORD					m_dwTimeStatus;
	CTime					m_time;
};

#endif // !defined(AFX_BCGTOOLBARDATETIMEPICKER_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)
