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

// BCGVisualManagerXP.h: interface for the CBCGVisualManagerXP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_)
#define AFX_BCGVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"
#include "BCGVisualManager.h"

class BCGCONTROLBARDLLEXPORT CBCGVisualManagerXP : public CBCGVisualManager  
{
	DECLARE_DYNCREATE(CBCGVisualManagerXP)

protected:
	CBCGVisualManagerXP(BOOL bIsTemporary = FALSE);

public:
	virtual ~CBCGVisualManagerXP();

protected:
	virtual void OnUpdateSystemColors ();

	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CControlBar* pBar);
	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea);
	virtual void OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bIsHoriz);
	
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnHighlightMenuItem (CDC*pDC, CBCGToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState, CBCGTabWnd* pWndTab);

	virtual void OnDrawCaptionButton (CDC* pDC, CBCGSCBButton* pButton,
									BOOL bHorz, BOOL bMaximized, BOOL bDisabled);

	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);
	virtual void OnDrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton);
	virtual void OnDrawComboBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton);
	virtual void OnDrawEditBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsHighlighted,
										CBCGToolbarEditBoxButton* pButton);

	virtual COLORREF GetToolbarButtonTextColor (CBCGToolbarButton* pButton,
												CBCGVisualManager::BCGBUTTON_STATE state);
	virtual int GetMenuImageMargin () const
	{
		return 3;
	}

	BOOL IsLook2000Allowed () const
	{
		return FALSE;
	}

	CBrush	m_brGripperHorz;
	CBrush	m_brGripperVert;

	COLORREF	m_clrBarBkgnd;			// Control bar background color (expect menu bar)
	CBrush		m_brBarBkgnd;

	COLORREF	m_clrMenuLight;			// Color of the light menu area
	CBrush		m_brMenuLight;

	COLORREF	m_clrHighlight;			// Highlighted toolbar/menu item color
	CBrush		m_brHighlight;

	COLORREF	m_clrHighlightDn;		// Highlighted and pressed toolbar item color
	CBrush		m_brHighlightDn;

	COLORREF	m_clrHighlightChecked;
	CBrush		m_brHighlightChecked;

	COLORREF	m_clrInactiveTabText;
	CBrush		m_brTabBack;

	CPen		m_penSeparator;

	COLORREF	m_clrPaneBorder;		// Status bar pane border

	COLORREF	m_clrMenuBorder;		// Menu border
	COLORREF	m_clrMenuItemBorder;	// Highlighted menu item border

	COLORREF	m_clrGripper;

	BOOL		m_bConnectMenuToParent;

	virtual void CreateGripperBrush ();
	virtual void ExtendMenuButton (CBCGToolbarMenuButton* pMenuButton, CRect& rect);
};

#endif // !defined(AFX_BCGVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_)
