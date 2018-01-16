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
// BCGWinXPVisualManager.h: interface for the CBCGWinXPVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_)
#define AFX_BCGWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"
#include "BCGVisualManagerXP.h"

class CBCGButton;

#ifndef _UXTHEME_H_

// From uxtheme.h:
typedef HANDLE HTHEME;          // handle to a section of theme data for class

#endif // THEMEAPI

typedef HTHEME (__stdcall * OPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (__stdcall * CLOSETHEMEDATA)(HTHEME hTheme);
typedef HRESULT (__stdcall * DRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
						int iPartId, int iStateId, const RECT *pRect, 
						OPTIONAL const RECT *pClipRect);

class BCGCONTROLBARDLLEXPORT CBCGWinXPVisualManager : public CBCGVisualManagerXP
{
	DECLARE_DYNCREATE(CBCGWinXPVisualManager)

public:
	CBCGWinXPVisualManager(BOOL bIsTemporary = FALSE);
	virtual ~CBCGWinXPVisualManager();

	static BOOL IsWinXPThemeAvailible ();

	void SetOfficeStyleMenus (BOOL bOn = TRUE);
	BOOL IsOfficeStyleMenus () const
	{
		return m_bOfficeStyleMenus;
	}

	virtual void OnUpdateSystemColors ();

	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CControlBar* pBar);
	virtual void OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGSCBButton* pButton, BOOL bHorz, BOOL bMaximized, BOOL bDisabled);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);

	virtual void OnDrawStatusBarProgress (CDC* pDC, CBCGStatusBar* pStatusBar,
				CRect rectProgress, int nProgressTotal, int nProgressCurr,
				COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
				BOOL bProgressText);
	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGStatusBar* pStatBar,
				CRect rectSizeBox);

	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect);
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
	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnHighlightMenuItem (CDC *pDC, CBCGToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);

	// Outlook bar page buttons:
	virtual BOOL OnFillOutlookPageButton (	CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText);
	virtual BOOL OnDrawOutlookPageButtonBorder (CBCGButton* pButton, 
												CDC* pDC, CRect& rectClient, UINT uiState);

	// Customization dialog:
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual CSize GetButtonExtraBorder () const;

	void CleanUpThemes ();

	virtual BOOL DrawPushButton (CDC* pDC, CRect rect, CBCGButton* pButton, UINT uiState);

	virtual void OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlSortArrow (CBCGHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp);

	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState, CBCGTabWnd* pWndTab);
protected:

	HTHEME	m_hThemeToolBar;
	HTHEME	m_hThemeRebar;
	HTHEME	m_hThemeButton;
	HTHEME	m_hThemeStatusBar;
	HTHEME	m_hThemeWindow;
	HTHEME	m_hThemeComboBox;
	HTHEME	m_hThemeProgress;
	HTHEME	m_hThemeHeader;
	HTHEME	m_hThemeScrollBar;

	HINSTANCE	m_hinstUXDLL;

	OPENTHEMEDATA		m_pfOpenThemeData;
	CLOSETHEMEDATA		m_pfCloseThemeData;
	DRAWTHEMEBACKGROUND m_pfDrawThemeBackground;

	BOOL	m_bOfficeStyleMenus;
};

#endif // !defined(AFX_BCGWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_)
