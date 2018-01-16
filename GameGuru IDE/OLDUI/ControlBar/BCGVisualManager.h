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

// BCGVisualManager.h: interface for the CBCGVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGVISUALMANAGER_H__22769E42_AB66_11D4_95C7_00A0C9289F1B__INCLUDED_)
#define AFX_BCGVISUALMANAGER_H__22769E42_AB66_11D4_95C7_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"

class CBCGPopupMenu;
class CBCGToolbarButton;
class CBCGToolbarMenuButton;
class CBCGSCBButton;
class CBCGTabWnd;
class CBCGStatusBar;
class CBCGToolbarComboBoxButton;
class CBCGToolbarEditBoxButton;
class CBCGButton;
class CBCGCaptionBar;
class CBCGHeaderCtrl;

class BCGCONTROLBARDLLEXPORT CBCGVisualManager : public CObject
{
	friend class CBCGSkinManager;

	DECLARE_DYNCREATE (CBCGVisualManager)

public:
	CBCGVisualManager(BOOL bIsTemporary = FALSE);
	virtual ~CBCGVisualManager();

	static void SetDefaultManager (CRuntimeClass* pRTI);

// Operations:
public:
	static void RedrawAll ();

// Overrides:
public:
	virtual void OnUpdateSystemColors () {};

	virtual void OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect);
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

	enum BCGBUTTON_STATE
	{
		ButtonsIsRegular,
		ButtonsIsPressed,
		ButtonsIsHighlighted,
	};

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state);

	virtual void OnHighlightMenuItem (CDC *pDC, CBCGToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);

	virtual COLORREF GetToolbarButtonTextColor (CBCGToolbarButton* pButton,
												CBCGVisualManager::BCGBUTTON_STATE state);
	virtual COLORREF GetCaptionBarTextColor (CBCGCaptionBar* pBar);

	// Outlook bar page buttons:
	virtual BOOL OnFillOutlookPageButton (	CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText);
	virtual BOOL OnDrawOutlookPageButtonBorder (CBCGButton* pButton, 
												CDC* pDC, CRect& rectClient, UINT uiState);

	// Tab overrides:
	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGButton* pButton, UINT uiState, CBCGTabWnd* pWndTab);

	// Customization dialog:
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual CSize GetButtonExtraBorder () const
	{
		return CSize (0, 0);
	}

	// Header control:
	virtual void OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
								CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlSortArrow (CBCGHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp);

// Attributes:
public:

	void SetMenuShadowDepth (int nDepth)	// Pixels
	{
		m_nMenuShadowDepth = nDepth;
	}

	int GetMenuShadowDepth () const
	{
		return m_nMenuShadowDepth;
	}
	
	static CBCGVisualManager* GetInstance ()
	{
		if (m_pVisManager != NULL)
		{
			ASSERT_VALID (m_pVisManager);
			return m_pVisManager;
		}

		if (m_pRTIDefault == NULL)
		{
			m_pVisManager = new CBCGVisualManager;
		}
		else
		{
			m_pVisManager = (CBCGVisualManager*) m_pRTIDefault->CreateObject ();
			ASSERT_VALID (m_pVisManager);
		}

		m_pVisManager->m_bAutoDestroy = TRUE;
		return m_pVisManager;
	}

	BOOL IsLook2000 () const			{	return m_bLook2000; }
	void SetLook2000 (BOOL bLook2000 = TRUE);

	BOOL IsMenuFlatLook () const			{	return m_bMenuFlatLook; }
	void SetMenuFlatLook (BOOL bMenuFlatLook = TRUE)
	{
		m_bMenuFlatLook = bMenuFlatLook;
		RedrawAll ();
	}

	BOOL IsAutoDestroy () const
	{
		return m_bAutoDestroy;
	}

	void SetShadowHighlightedImage (BOOL bShadow = TRUE)
	{
		m_bShadowHighlightedImage = bShadow;
	}

	BOOL IsShadowHighlightedImage () const
	{
		return m_bShadowHighlightedImage;
	}

	BOOL IsEmbossDisabledImage () const
	{
		return m_bEmbossDisabledImage;
	}

	void SetEmbossDisabledImage (BOOL bEmboss = TRUE)
	{
		m_bEmbossDisabledImage = bEmboss;
	}

	BOOL IsFadeInactiveImage () const
	{
		return m_bFadeInactiveImage;
	}

	void SetFadeInactiveImage (BOOL bFade = TRUE)
	{
		m_bFadeInactiveImage = bFade;
	}

	virtual int GetMenuImageMargin () const
	{
		return 2;
	}

	virtual BOOL IsLook2000Allowed () const
	// Allows choose "Look 2000" in the customization dialog
	{
		return TRUE;
	}

protected:
	static CBCGVisualManager* CreateVisualManager (CRuntimeClass* pVisualManager);
	static void DestroyInstance ();

// Attributes:
protected:
	static CRuntimeClass*		m_pRTIDefault;
	static CBCGVisualManager*	m_pVisManager;

	BOOL	m_bLook2000;				// Single grippers
	int		m_nMenuShadowDepth;
	BOOL	m_bMenuFlatLook;			// Menu item is always stil unpressed
	BOOL	m_bShadowHighlightedImage;
	BOOL	m_bEmbossDisabledImage;
	BOOL	m_bFadeInactiveImage;

	BOOL	m_bIsTemporary;

private:
	BOOL	m_bAutoDestroy;
};

#endif // !defined(AFX_BCGVISUALMANAGER_H__22769E42_AB66_11D4_95C7_00A0C9289F1B__INCLUDED_)
