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

// BCGToolbarMenuButton.h: interface for the CBCGToolbarMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARMENUBUTTON_H__B715CF84_CA00_11D1_A647_00A0C93A70EC__INCLUDED_)
#define AFX_BCGTOOLBARMENUBUTTON_H__B715CF84_CA00_11D1_A647_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "bcgcontrolbar.h"
#include "BCGToolbarButton.h"
#include "BCGPopupMenu.h"

static const int TEXT_MARGIN = 3;

class BCGCONTROLBARDLLEXPORT CBCGToolbarMenuButton : public CBCGToolbarButton  
{
	friend class CBCGPopupMenu;
	friend class CBCGPopupMenuBar;
	friend class CBCGToolBar;
	friend class CBCGMenuBar;

	DECLARE_SERIAL(CBCGToolbarMenuButton)

public:
	CBCGToolbarMenuButton();
	CBCGToolbarMenuButton(const CBCGToolbarMenuButton& src);
	CBCGToolbarMenuButton (UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText = NULL,
								BOOL bUserButton = FALSE);
	virtual ~CBCGToolbarMenuButton();

// Operations:
protected:
	void Initialize ();
	void DrawMenuItem (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages, 
					BOOL bCustomizeMode, BOOL bHighlight, BOOL bGrayDisabledButtons);

	static void GetTextHorzOffsets (int& xOffsetLeft, int& xOffsetRight);

public:
	virtual BOOL OpenPopupMenu (CWnd* pWnd = NULL);
	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);

	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void CreateFromMenu (HMENU hMenu);
	virtual HMENU CreateMenu () const;
	virtual BOOL HaveHotBorder () const			{	return FALSE;	}
	virtual void OnCancelMode ();
	virtual BOOL OnContextHelp (CWnd* pWnd)
	{
		return OnClick (pWnd, FALSE);
	}

	virtual int OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual BOOL IsDroppedDown () const
	{
		return m_pPopupMenu != NULL;
	}

	virtual CBCGPopupMenu* CreatePopupMenu ()
	{
		return new CBCGPopupMenu;
	}

	virtual BOOL IsEmptyMenuAllowed () const
	{
		return FALSE;
	}

	virtual BOOL OnBeforeDrag () const;
	virtual void SaveBarState ();

	void GetImageRect (CRect& rectImage);
	
	virtual void SetRadio ();
	virtual void ResetImageToDefault ();
	virtual BOOL CompareWith (const CBCGToolbarButton& other) const;

	virtual BOOL IsBorder () const
	{
		return TRUE;
	}

	virtual BOOL OnClickMenuItem ()
	{
		return FALSE;	// Return TRUE for the custom process
	}

// Attributes:
public:
	CWnd* GetParentWnd () const
	{
		return m_pWndParent;
	}

	const CObList& GetCommands () const
	{
		return m_listCommands;
	}

	CBCGPopupMenu* GetPopupMenu () const
	{
		return m_pPopupMenu;
	}

	void SetMenuOnly (BOOL bMenuOnly)
	{ 
		m_bMenuOnly	= bMenuOnly;
	}	//JRG

	virtual BOOL IsTearOffMenu () const
	{
		return m_uiTearOffBarID != 0;
	}

	virtual void SetTearOff (UINT uiBarID);

	static BOOL		m_bAlwaysCallOwnerDraw;
						// CMainFrame::OnDrawMenuImage will be called 
						// even after default image was drawn

protected:
	CObList			m_listCommands;	// List of CBCGToolbarButton
	BOOL			m_bDrawDownArrow;
	BOOL			m_bMenuMode;

	CBCGPopupMenu*	m_pPopupMenu;
	CWnd*			m_pWndParent;
	BOOL			m_bDefault;

	BOOL			m_bClickedOnMenu;
	BOOL			m_bHorz;
	BOOL			m_bToBeClosed;

	BOOL			m_bMenuOnly;	//JRG
	UINT			m_uiTearOffBarID;

	BOOL			m_bIsRadio;
public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif // !defined(AFX_BCGTOOLBARMENUBUTTON_H__B715CF84_CA00_11D1_A647_00A0C93A70EC__INCLUDED_)
