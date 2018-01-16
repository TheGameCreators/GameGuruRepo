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

#if !defined(AFX_BCGPOPUPMENUBAR_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPOPUPMENUBAR_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGPopupMenuBar.h : header file
//

#include "bcgcontrolbar.h"
#include "BCGToolBar.h"
#include "BCGMenuBar.h"

class CBCGToolbarMenuButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupMenuBar command target

class BCGCONTROLBARDLLEXPORT CBCGPopupMenuBar : public CBCGToolBar
{
	friend class CBCGPopupMenu;

	DECLARE_SERIAL(CBCGPopupMenuBar)

public:
	CBCGPopupMenuBar();
	virtual ~CBCGPopupMenuBar();

// Attributes
public:
	UINT GetDefaultMenuId () const
	{
		return m_uiDefaultMenuCmdId;
	}

	int GetOffset () const
	{
		return m_iOffset;
	}

	void SetOffset (int iOffset)
	{
		if (m_iOffset != iOffset)
		{
			m_iOffset = iOffset;
			AdjustLocations ();
		}
	}

protected:
	UINT						m_uiDefaultMenuCmdId;
	CBCGToolbarMenuButton*		m_pDelayedPopupMenuButton;
	CBCGToolbarMenuButton*		m_pDelayedClosePopupMenuButton;
	BOOL						m_bFirstClick;
	int							m_iOffset;	// rows
	int							m_xSeparatorOffsetLeft;
	int							m_xSeparatorOffsetRight;
	BOOL						m_bAreAllCommandsShown;
	int							m_iMaxWidth;
	CUIntArray					m_arColumns;
	CMap<UINT, UINT,UINT,UINT>	m_HiddenItemsAccel;	// Accell. <==> Command
	BOOL						m_bInCommand;
	BOOL						m_bTrackMode;
	BOOL						m_bIsClickOutsideItem;

	static UINT					m_uiPopupTimerDelay;

// Operations
public:
	BOOL ImportFromMenu (HMENU hMenu, BOOL bShowAllCommands = FALSE);
	HMENU ExportToMenu () const;

	void StartPopupMenuTimer (CBCGToolbarMenuButton* pMenuButton, int nDelayFactor = 1);
	CBCGToolBar* FindDestBar (CPoint point);

	void CloseDelayedSubMenu ();
	void RestoreDelayedSubMenu ();

	// by yurig
	BOOL LoadFromHash (HMENU hMenu);

protected:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPopupMenuBar)
	//}}AFX_VIRTUAL

	virtual BOOL OnSendCommand (const CBCGToolbarButton* pButton);
	virtual void AdjustLocations ();
	virtual void DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz);
	virtual CSize CalcSize (BOOL bVertDock);
	virtual BOOL AllowSelectDisabled () const	{	return CBCGMenuBar::IsHighlightDisabledItems ();	}
	virtual BOOL AllowShowOnList () const		{	return FALSE;	}
	virtual int FindDropIndex (const CPoint p, CRect& rectDrag) const;
	virtual void DrawDragMarker (CDC* pDC);
	virtual CBCGToolbarButton* CreateDroppedButton (COleDataObject* pDataObject);
	virtual void OnChangeHot (int iHot);
	virtual BOOL OnKey (UINT nChar);
	virtual BOOL OnSetDefaultButtonText (CBCGToolbarButton* pButton);
	virtual BOOL EnableContextMenuItems (CBCGToolbarButton* pButton, CMenu* pPopup);
	virtual void OnFillBackground (CDC* pDC);
	virtual void SetButtonStyle(int nIndex, UINT nStyle);

	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	virtual BOOL OnUserToolTip (CBCGToolbarButton* /*pButton*/, CString& /*strTTText*/) const
	{
		return FALSE;
	}

	virtual CWnd* GetCommandTarget () const;
	void InvokeMenuCommand (UINT uiCmdId);

	void SetInCommand (BOOL bInCommand = TRUE);

	virtual BOOL TranslateChar (UINT /*nChar*/)
	{
		return FALSE;
	}

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return FALSE;
	}

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPopupMenuBar)
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnToolbarImageAndText();
	afx_msg void OnToolbarText();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOPUPMENUBAR_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
