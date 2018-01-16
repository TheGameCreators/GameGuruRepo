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

//
// bcgtoolbar.h : definition of CBCGToolBar
//
// This code is based on the Microsoft Visual C++ sample file
// TOOLBAR.C from the OLDBARS example
//

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "BCGToolbarDropTarget.h"
#include "BCGToolbarDropSource.h"
#include "BCGToolBarImages.h"
#include "BCGToolbarButton.h"
#include "BCGControlBarImpl.h"
#include "CmdUsageCount.h"
#include "BCGCommandManager.h"
#include "BCGVisualManager.h"
#include "globals.h"

class CBCGToolbarMenuButton;
class CCustomizeButton;
class CBCGRegistry;

//-----------------------------------------------
// CBCGToolbar LoadBitmap/LoadToolbar parameters:
//-----------------------------------------------
class BCGCONTROLBARDLLEXPORT CBCGToolBarParams
{
public:
	CBCGToolBarParams();

	UINT	m_uiColdResID;			// Toolbar regular images
	UINT	m_uiHotResID;			// Toolbar "hot" images
	UINT	m_uiDisabledResID;		// Toolbar disabled images
	UINT	m_uiLargeColdResID;		// Toolbar large regular images
	UINT	m_uiLargeHotResID;		// Toolbar large "hot" images
	UINT	m_uiLargeDisabledResID;	// Toolbar large disabled images
	UINT	m_uiMenuResID;			// Menu images
	UINT	m_uiMenuDisabledResID;	// Menu disabled images
};

//-----------------------------------
// CBCGToolbar notification messages:
//-----------------------------------
BCGCONTROLBARDLLEXPORT extern UINT BCGM_TOOLBARMENU;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_CUSTOMIZETOOLBAR;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_CREATETOOLBAR;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_DELETETOOLBAR;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_CUSTOMIZEHELP;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_RESETTOOLBAR;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_RESETMENU;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_SHOWREGULARMENU;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_RESETCONTEXTMENU;
BCGCONTROLBARDLLEXPORT extern UINT BCGM_RESETKEYBOARD;

static const int dwDefaultToolbarStyle = (WS_CHILD | WS_VISIBLE | CBRS_TOP | 
										  CBRS_GRIPPER | CBRS_HIDE_INPLACE);

class BCGCONTROLBARDLLEXPORT CBCGToolBar : public CControlBar
{
	friend class CBCGToolbarDropTarget;
	friend class CBCGToolbarsPage;
	friend class CBCGOptionsPage;
	friend class CButtonsTextList;
	friend class CBCGCommandManager;

	DECLARE_SERIAL(CBCGToolBar)

	//--------------
	// Construction:
	//--------------
public:
	CBCGToolBar();
	virtual BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = dwDefaultToolbarStyle,
			UINT nID = AFX_IDW_TOOLBAR);
	virtual BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = TBSTYLE_FLAT,
		DWORD dwStyle = dwDefaultToolbarStyle,
		CRect rcBorders = CRect(1, 1, 1, 1),
		UINT nID = AFX_IDW_TOOLBAR);

	//----------------------------------------------------------------
	// Set toolbar buttons image resources.
	// You should use SINGLE CBCGToolBarImages for ALL your toolbars!
	//----------------------------------------------------------------
	static BOOL SetUserImages (CBCGToolBarImages* pUserImages);

	virtual void ResetImages ();	// Reset all toolbar images exept user-derfined to the default

	//------------------------------
	// Clear all images except user:
	//------------------------------
	static void ResetAllImages();

	//--------------------------------
	// Dimension manipulation methods:
	//--------------------------------
	static void SetSizes (SIZE sizeButton, SIZE sizeImage);
	static void SetMenuSizes (SIZE sizeButton, SIZE sizeImage);
	static CSize GetMenuImageSize ();
	static CSize GetMenuButtonSize ();

	void SetLockedSizes (SIZE sizeButton, SIZE sizeImage);
	void SetHeight (int cyHeight);

	//-----------------
	// Toolbar context:
	//-----------------
	virtual BOOL LoadBitmap (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL bLocked = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0);
	virtual BOOL LoadToolBar (UINT uiResID, UINT uiColdResID = 0, 
					UINT uiMenuResID = 0, BOOL bLocked = FALSE,
					UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0,
  				    UINT uiHotResID = 0);

	virtual BOOL LoadBitmapEx (CBCGToolBarParams& params, BOOL bLocked = FALSE);
	virtual BOOL LoadToolBarEx (UINT uiToolbarResID, CBCGToolBarParams& params, 
								BOOL bLocked = FALSE);

	//----------------------------
	// Toolbar buttons add/remove:
	//----------------------------
	BOOL SetButtons(const UINT* lpIDArray, int nIDCount, BOOL bRemapImages = TRUE);

	int InsertButton (const CBCGToolbarButton& button, int iInsertAt = -1);
	int InsertSeparator (int iInsertAt = -1);

	int ReplaceButton (UINT uiCmd, const CBCGToolbarButton& button, BOOL bAll = FALSE);
	
	BOOL RemoveButton (int iIndex);
	virtual void RemoveAllButtons ();

	static BOOL IsLastCommandFromButton (CBCGToolbarButton* pButton);
	static BOOL AddToolBarForImageCollection (UINT uiResID, UINT uiBmpResID = 0,
									UINT uiColdResID = 0, UINT uiMenuResID = 0,
									UINT uiDisabledResID = 0, UINT uiMenuDisabledResID = 0);

	static void SetNonPermittedCommands (CList<UINT, UINT>& lstCommands);
	static BOOL IsCommandPermitted (UINT uiCmd)
	{
		return m_lstUnpermittedCommands.Find (uiCmd) == NULL;
	}

	static void SetBasicCommands (CList<UINT, UINT>& lstCommands);
	static void AddBasicCommand (UINT uiCmd);
	
	static BOOL IsBasicCommand (UINT uiCmd)
	{
		return m_lstBasicCommands.Find (uiCmd) != NULL;
	}

	static const CList<UINT, UINT>& GetBasicCommands ()
	{
		return m_lstBasicCommands;
	}

	static BOOL IsCommandRarelyUsed (UINT uiCmd);

	static void AddCommandUsage (UINT uiCommand)
	{
		m_UsageCount.AddCmd (uiCommand);
	}

	static BOOL SetCommandUsageOptions (UINT nStartCount, UINT nMinUsagePercentage = 5);

	virtual int GetRowHeight () const
	{
		if (m_bDrawTextLabels)
		{
			ASSERT (m_nMaxBtnHeight > 0);
			return m_nMaxBtnHeight;
		}

		return max (globalData.GetTextHeight (m_dwStyle & CBRS_ORIENT_HORZ),
			(m_bMenuMode ?
			(m_sizeMenuButton.cy > 0 ?
				m_sizeMenuButton.cy : m_sizeButton.cy) :
			GetButtonSize ().cy));
	}

	virtual int GetColumnWidth () const
	{
		return m_bMenuMode ?
			m_sizeMenuButton.cx > 0 ?
				m_sizeMenuButton.cx : m_sizeButton.cx :
			GetButtonSize ().cx;
	}

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return TRUE;
	}

	static void SetHelpMode (BOOL bOn = TRUE);
	virtual void Deactivate ();
	virtual void RestoreFocus ();

	void SetToolBarBtnText (UINT nBtnIndex,
							LPCTSTR szText = NULL,
							BOOL bShowText = TRUE,
							BOOL bShowImage = TRUE);

	// mike - 210205
	void SetToolBarBtnTooltip (UINT nBtnIndex, LPCTSTR szText = NULL );


	void EnableLargeIcons (BOOL bEnable);

	static void SetLargeIcons (BOOL bLargeIcons = TRUE);
	static BOOL IsLargeIcons ()
	{
		return m_bLargeIcons;
	}

	CSize GetButtonSize () const
	{
		CSize size = m_bLocked ?
			m_bLargeIconsAreEnbaled ? m_sizeCurButtonLocked : m_sizeButtonLocked :
			m_bLargeIconsAreEnbaled ? m_sizeCurButton : m_sizeButton;
		
		if (IsButtonExtraSizeAvailable ())
		{
			size += CBCGVisualManager::GetInstance ()->GetButtonExtraBorder ();
		}

		return size;
	}

	CSize GetImageSize () const
	{
		return m_bLocked ?
			m_bLargeIconsAreEnbaled ? m_sizeCurImageLocked : m_sizeImageLocked :
			m_bLargeIconsAreEnbaled ? m_sizeCurImage : m_sizeImage;
	}

	//------------
	// Attributes:
	//------------
public: 
	BOOL IsLocked () const
	{
		return m_bLocked;
	}

	UINT GetResourceID () const
	{
		return m_uiOriginalResID;
	}

	virtual BOOL IsResourceChanged () const
	{
		return m_bResourceWasChanged;
	}

	static const CObList& GetAllToolbars ();

	// standard control bar things
	int CommandToIndex(UINT nIDFind, int iIndexFirst = 0) const;
	UINT GetItemID(int nIndex) const;
	
	virtual void GetItemRect(int nIndex, LPRECT lpRect) const;
	virtual void GetInvalidateItemRect(int nIndex, LPRECT lpRect) const;

	UINT GetButtonStyle(int nIndex) const;
	virtual void SetButtonStyle(int nIndex, UINT nStyle);

	int GetCount () const;
	int ButtonToIndex(const CBCGToolbarButton* pButton) const;
	CBCGToolbarButton* GetButton (int iIndex) const;

	const CObList& GetAllButtons() const { return m_Buttons; }

	// Find all buttons specified by the given command ID from the all
	// toolbars:
	static int GetCommandButtons (UINT uiCmd, CObList& listButtons);
	
	static BOOL SetCustomizeMode (BOOL bSet = TRUE);
	static BOOL IsCustomizeMode ()
	{
		return m_bCustomizeMode;
	}

	BOOL IsUserDefined () const;

	static CBCGToolBar* FromHandlePermanent (HWND hwnd);
	static CBCGToolBarImages* GetImages ()
	{
		return &m_Images;
	}
	static CBCGToolBarImages* GetUserImages ()
	{
		return m_pUserImages;
	}

	static int GetDefaultImage (UINT uiID)
	{
		int iImage;
		if (m_DefaultImages.Lookup (uiID, iImage))
		{
			return iImage;
		}

		return -1;
	}

	int GetImagesOffset () const
	{
		return m_iImagesOffset;
	}

	CBCGToolbarButton* GetHighlightedButton () const;

	static void SetHotTextColor (COLORREF clrText);
	static COLORREF GetHotTextColor ();

	void SetHotBorder (BOOL bShowHotBorder)
	{
		m_bShowHotBorder = bShowHotBorder;
	}

	BOOL GetHotBorder () const
	{
		return m_bShowHotBorder;
	}

	void SetGrayDisabledButtons (BOOL bGrayDisabledButtons)
	{
		m_bGrayDisabledButtons = bGrayDisabledButtons;
	}

	BOOL GetGrayDisabledButtons () const
	{
		return m_bGrayDisabledButtons;
	}

	//------------------------------------------------------
	// Enable/disable quick customization mode ("Alt+drag"):
	//------------------------------------------------------
	static void EnableQuickCustomization (BOOL bEnable = TRUE)
	{
		m_bAltCustomization = bEnable;
	}

	static void SetLook2000 (BOOL bLook2000 = TRUE);
	static BOOL IsLook2000 ();
	
	void EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, const CString& strCustomizeText);
	void EnableCustomizeButton (BOOL bEnable, UINT uiCustomizeCmd, UINT uiCustomizeTextResId);

	void EnableTextLabels (BOOL bEnable = TRUE);

	virtual BOOL AllowChangeTextLabels () const
	{
		return TRUE;
	}

	BOOL AreTextLabels () const
	{
		return m_bTextLabels;
	}

	void SetMaskMode (BOOL bMasked)
	{
		m_bMasked = bMasked;
	}

	void SetPermament (BOOL bPermament = TRUE)
	{
		m_bPermament = bPermament;
	}

	// By Jay Giganti 
	BOOL GetIgnoreSetText () const
	{
		return m_bIgnoreSetText;
	}

	void SetIgnoreSetText (BOOL bValue)
	{
		m_bIgnoreSetText = bValue;
	}
	///

	BOOL GetRouteCommandsViaFrame () 
	{
		return m_bRouteCommandsViaFrame;
	}

	void SetRouteCommandsViaFrame (BOOL bValue)
	{
		m_bRouteCommandsViaFrame = bValue;
	}

	static BOOL GetShowTooltips()				{return m_bShowTooltips;}		//JRG Modified 3/21/2000	
	static void SetShowTooltips(BOOL bValue)	{m_bShowTooltips = bValue;}		//JRG Modified 3/21/2000

	HWND GetHwndLastFocus() const				{return m_hwndLastFocus;}; 

public:
	// for changing button info
	void GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const;
	void SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage);

	BOOL SetButtonText(int nIndex, LPCTSTR lpszText);
	CString GetButtonText( int nIndex ) const;
	void GetButtonText( int nIndex, CString& rString ) const;

	// Save/load toobar state + buttons:
	void Serialize (CArchive& ar);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL RemoveStateFromRegistry (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	static BOOL LoadParameters (LPCTSTR lpszProfileName = NULL);
	static BOOL SaveParameters (LPCTSTR lpszProfileName = NULL);

	virtual BOOL CanBeRestored () const;
	virtual BOOL CanBeClosed () const
	{
		return !m_bPermament;
	}

	virtual BOOL RestoreOriginalstate ();
	virtual void OnReset () {}

	static void ResetAll ();

	virtual void AdjustLayout ();
	virtual int HitTest(CPoint point);
	virtual BOOL TranslateChar (UINT nChar);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual BOOL PrevMenu ();
	virtual BOOL NextMenu ();
	BOOL ProcessCommand (CBCGToolbarButton* pButton);
	CBCGToolbarMenuButton* GetDroppedDownMenu (int* pIndex = NULL) const;

	BOOL SetHot (CBCGToolbarButton *pMenuButton);

	virtual BOOL OnSetDefaultButtonText (CBCGToolbarButton* pButton);

	BOOL IsDragButton (const CBCGToolbarButton* pButton) const
	{
		return pButton == m_pDragButton;
	}

	virtual void  OnFillBackground (CDC* /*pDC*/) {}

	static BOOL	m_bExtCharTranslation;

	static void CBCGToolBar::CleanUpImages ();

// Implementation
public:
	virtual ~CBCGToolBar();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout (int nLength, DWORD dwMode);

	virtual CSize CalcSize (BOOL bVertDock);
	int WrapToolBar (int nWidth, int nHeight = 32767);

	virtual void OnChangeHot (int iHot);

protected:
	virtual CSize CalcLayout (DWORD dwMode, int nLength = -1);
	void  SizeToolBar (int nLength, BOOL bVert = FALSE);

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int  InsertButton (CBCGToolbarButton* pButton, int iInsertAt = -1);
	void InvalidateButton(int nIndex);
	void UpdateButton(int nIndex);

	virtual BOOL DrawButton (CDC* pDC, CBCGToolbarButton* pButton, 
		CBCGToolBarImages* pImages, BOOL bHighlighted,
		BOOL bDrawDisabledImages);
	virtual void DrawDragMarker (CDC* pDC);
	CBCGToolBarImages* GetImageList (CBCGToolBarImages& images, CBCGToolBarImages& imagesLocked, 
							 CBCGToolBarImages& largeImages, CBCGToolBarImages& largeImagesLocked) const;

	void RebuildAccelerationKeys ();
	virtual CWnd* GetCommandTarget () const
	{
		// determine target of command update
		CFrameWnd* pTarget = (CFrameWnd*) GetOwner();
		if (pTarget == NULL || (m_bRouteCommandsViaFrame && !pTarget->IsFrameWnd ()))
		{
			pTarget = GetParentFrame ();
		}

		return pTarget;
	}

protected:
	friend class CBCGWorkspace;
	
	static CBCGToolBar*			m_pSelToolbar;			// "Selected" toolbar in the customization mode

	static CBCGToolBarImages	m_Images;				// Shared toolbar images
	static CBCGToolBarImages	m_ColdImages;			// Shared toolbar "cold" images
	static CBCGToolBarImages	m_DisabledImages;		// Shared disabled images

	static CBCGToolBarImages	m_LargeImages;			// Shared toolbar large "hot" images
	static CBCGToolBarImages	m_LargeColdImages;		// Shared toolbar large "cold" images
	static CBCGToolBarImages	m_LargeDisabledImages;	// Shared disabled large images

	static CBCGToolBarImages	m_MenuImages;
	static CBCGToolBarImages	m_DisabledMenuImages;

	static CBCGToolBarImages*	m_pUserImages;			// Shared user-defined images

	CBCGToolBarImages	m_ImagesLocked;					// "Locked" toolbar images
	CBCGToolBarImages	m_ColdImagesLocked;				// "Locked" toolbar "cold" images
	CBCGToolBarImages	m_DisabledImagesLocked;			// "Locked" toolbar disabled images
	CBCGToolBarImages	m_LargeImagesLocked;			// "Locked" toolbar large images
	CBCGToolBarImages	m_LargeColdImagesLocked;		// "Locked" toolbar large "cold" images
	CBCGToolBarImages	m_LargeDisabledImagesLocked;	// "Locked" toolbar large disabled images
	CBCGToolBarImages	m_MenuImagesLocked;				// "Locked" toolbar menu images
	CBCGToolBarImages	m_DisabledMenuImagesLocked;		// "Locked" toolbar menu disabled images

	BOOL				m_bLocked;
	BOOL				m_bLargeIconsAreEnbaled;

	CBCGControlBarImpl	m_Impl;

	BOOL				m_bMasked;
	BOOL				m_bPermament;	// Can't be closed

	BOOL				m_bTextLabels;	// Text labels below the image are available
	BOOL				m_bDrawTextLabels;
	int					m_nMaxBtnHeight;// Actual only if m_bTextLabels is TRUE

	static CMap<UINT, UINT, int, int>	m_DefaultImages;

	static CSize m_sizeButton;			// original size of button
	static CSize m_sizeImage;			// original size of glyph
	static CSize m_sizeMenuButton;		// size of button on the menu
	static CSize m_sizeMenuImage;		// size of image on the menu
	static CSize m_sizeCurButton;		// size of button
	static CSize m_sizeCurImage;		// size of glyph

	CSize m_sizeButtonLocked;			// original size of button of the locked toolbars
	CSize m_sizeImageLocked;			// original size of glyph of the locked toolbars
	CSize m_sizeCurButtonLocked;		// size of button
	CSize m_sizeCurImageLocked;			// size of glyph

	int m_iButtonCapture;				// index of button with capture (-1 => none)
	int m_iHighlighted;					// highlighted button index
	int m_iSelected;					// selected button index
	int	m_iHot;

	CObList		m_Buttons;
	CObList		m_OrigButtons;			// Original (not customized) items
	BOOL		m_bResourceWasChanged;	// Resource was changed since last session

#ifndef BCG_NO_CUSTOMIZATION

	CBCGToolbarDropTarget	m_DropTarget;
	static CBCGToolbarDropSource m_DropSource;

#endif // BCG_NO_CUSTOMIZATION

	static BOOL m_bCustomizeMode;
	static BOOL m_bAltCustomizeMode;

	int			m_iDragIndex;
	CRect		m_rectDrag;
	CPen		m_penDrag;
	CBCGToolbarButton* m_pDragButton;
	CPoint		m_ptStartDrag;
	BOOL		m_bIsDragCopy;

	BOOL		m_bStretchButton;
	CRect		m_rectTrack;

	int			m_iImagesOffset;
	UINT		m_uiOriginalResID;	// Toolbar resource ID

	BOOL		m_bTracked;
	CPoint		m_ptLastMouse;

	BOOL		m_bMenuMode;

	CWnd*		m_pWndLastCapture;
	HWND		m_hwndLastFocus;

	BOOL		m_bDisableControlsIfNoHandler;
	BOOL		m_bRouteCommandsViaFrame;

	static COLORREF	m_clrTextHot;
	
	static HHOOK m_hookMouseHelp;	// Mouse hook for the help mode
	static CBCGToolBar* m_pLastHookedToolbar;

	CMap<UINT, UINT&, CBCGToolbarButton*, CBCGToolbarButton*&>	m_AcellKeys;	// Keyborad acceleration keys

	static BOOL m_bShowTooltips;
	static BOOL m_bShowShortcutKeys;
	static BOOL m_bLargeIcons;

	static CList<UINT, UINT>	m_lstUnpermittedCommands;
	static CList<UINT, UINT>	m_lstBasicCommands;

	static CCmdUsageCount	m_UsageCount;

	BOOL		m_bShowHotBorder;
	BOOL		m_bGrayDisabledButtons;
	BOOL		m_bIgnoreSetText;

	static BOOL	m_bAltCustomization;

	CCustomizeButton*	m_pCustomizeBtn;

	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	virtual void DoPaint(CDC* pDC);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual DWORD RecalcDelayShow(AFX_SIZEPARENTPARAMS* lpLayout);

	virtual int FindDropIndex (const CPoint point, CRect& rectDrag) const;
	virtual void AdjustLocations ();

	virtual BOOL OnSendCommand (const CBCGToolbarButton* /*pButton*/)	{	return FALSE;	}

	virtual BOOL AllowSelectDisabled () const	{	return FALSE;	}
	virtual BOOL AllowShowOnList () const		{	return TRUE;	}

	virtual void DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz);
	virtual CBCGToolbarButton* CreateDroppedButton (COleDataObject* pDataObject);
	virtual BOOL OnKey (UINT /*nChar*/)				{	return FALSE;	}
	virtual void OnCustomizeMode (BOOL bSet);

	virtual BOOL EnableContextMenuItems (CBCGToolbarButton* pButton, CMenu* pPopup);
	virtual BOOL IsPureMenuButton (CBCGToolbarButton* /*pButton*/) const
	{
		return m_bMenuMode;
	}

	void AddRemoveSeparator (const CBCGToolbarButton* pButton,
						const CPoint& ptStart, const CPoint& ptDrop);
	virtual void ShowCommandMessageString (UINT uiCmdId);

	static LRESULT CALLBACK BCGToolBarMouseProc (int nCode, WPARAM wParam, LPARAM lParam);

	BOOL DropDownMenu (CBCGToolbarButton* pButton);

	virtual int CalcMaxButtonHeight ();
	virtual BOOL OnUserToolTip (CBCGToolbarButton* pButton, CString& strTTText) const;

	// SmartUpdate methods:
	virtual void SaveOriginalState (CBCGRegistry& reg);
	virtual BOOL LoadLastOriginalState (CBCGRegistry& reg);
	virtual BOOL SmartUpdate (const CObList& lstPrevButtons);

	//{{AFX_MSG(CBCGToolBar)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnSysColorChange();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnToolbarAppearance();
	afx_msg void OnToolbarDelete();
	afx_msg void OnToolbarImage();
	afx_msg void OnToolbarImageAndText();
	afx_msg void OnToolbarStartGroup();
	afx_msg void OnToolbarText();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnBcgbarresToolbarReset();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBcgbarresCopyImage();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnBcgbarresToolbarNewMenu();
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnHelpHitTest(WPARAM,LPARAM);
	afx_msg LRESULT OnGetButtonCount(WPARAM,LPARAM);
	afx_msg LRESULT OnGetItemRect(WPARAM,LPARAM);
	afx_msg LRESULT OnGetButton(WPARAM,LPARAM);
	afx_msg LRESULT OnGetButtonText(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif //!_TOOLBAR_H_
