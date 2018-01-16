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

/////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998, 1999 by Cristi Posea
// All rights reserved
//
// THIS SOURCE CODE WAS INCLUDED INTO THE BCGCONTROLBAR LIBRARY UNDER THE
// PERSONAL PERMISSION OF CRISTI POSEA
//
// Use and distribute freely, except: don't remove my name from the
// source or documentation (don't take credit for my work), mark your
// changes (don't get me blamed for your possible bugs), don't alter
// or remove this notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// This class is intended to be used as a base class. Do not simply add
// your code to this file - instead create a new class derived from
// CSizingControlBar and put there what you need.
// Modify this file only to fix bugs, and don't forget to send me a copy.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc.,
// and I'll try to keep a version up to date.  I can be reached at:
//    cristip@dundas.com
//
// More details at MFC Programmer's SourceBook
// http://www.codeguru.com/docking/docking_window.shtml or search
// www.codeguru.com for my name if the article was moved.
//
// Partialy modified by Stas Levin - bcgsoft@yahoo.com. See changes list in
// the C++ source file.
//
/////////////////////////////////////////////////////////////////////////


#if !defined(AFX_BCGSIZINGCONTROLBAR_H__91380D71_4461_11D3_A72A_0090274409AC__INCLUDED_)
#define AFX_BCGSIZINGCONTROLBAR_H__91380D71_4461_11D3_A72A_0090274409AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGSizingControlBar.h : header file
//

#ifndef BCG_NO_SIZINGBAR

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "MenuImages.h"

/////////////////////////////////////////////////////////////////////////
// CBCGSCBButton (button info) helper class

static const int nBtnMargin = 2;

class CBCGSizingControlBar;

class BCGCONTROLBARDLLEXPORT CBCGSCBButton
{
public:
    CBCGSCBButton ();

    void Move (const CPoint& ptTo, BOOL bHide = FALSE)
	{
		m_ptOrg = ptTo; 
		m_bHidden = bHide;
	};

    CRect GetRect() const
	{ 
		return CRect (m_ptOrg, m_bHidden ? CSize (0, 0) : GetSize ());
	};

	static CSize GetSize ()
	{
		return CMenuImages::Size () + CSize (2 * nBtnMargin + 1, 2 * nBtnMargin + 1);
	}

    void Paint (CDC* pDC, BOOL bHorz, BOOL bMaximized, BOOL bDisabled);

    BOOL		m_bPushed;
    BOOL		m_bFocused;
	UINT		m_nHit;
	UINT		m_uiTT;
	BOOL		m_bHidden;

	CBCGSizingControlBar* m_pParentBar;

protected:
    CPoint		m_ptOrg;
};

/////////////////////////////////////////////////////////////////////////
// CBCGSizingControlBar control bar styles

#define SCBS_EDGELEFT       0x00000001
#define SCBS_EDGERIGHT      0x00000002
#define SCBS_EDGETOP        0x00000004
#define SCBS_EDGEBOTTOM     0x00000008
#define SCBS_EDGEALL        0x0000000F
#define SCBS_SHOWEDGES      0x00000010
#define SCBS_GRIPPER        0x00000020


// added by Quentin Pouplard
#define BCG_ROLLUP_NONE              0x00000000 // no rollup feature
#define BCG_ROLLUP_NORMAL            0x00000001 // standard two state rollup
#define BCG_ROLLUP_DYNAMIC_ON        0x00000002 // PSP-like dynamic rollup on
#define BCG_ROLLUP_DYNAMIC_OFF       0x00000003 // PSP-like dynamic rollup off
#define BCG_ROLLUP_DYNAMICSIZED_ON   0x00000004 // PSP-like dynamic rollup on WITH dynamic caption size
#define BCG_ROLLUP_DYNAMICSIZED_OFF  0x00000005 // PSP-like dynamic rollup off WITH dynamic caption size

// added by Quentin Pouplard
// used in CBCGSizingControlBar::SetRollupState(...).
#define BCG_ROLLUP_STATE_FORCESTAYUNROLL 0x00000000 // force the control bar to stay visible (if unroll, has no direct effect if the bar is rolled). (useful if the controlbar lost focus but must stay visible)
#define BCG_ROLLUP_STATE_LETROLL         0x00000001 // to allow again the controlbar to be rolled 
#define BCG_ROLLUP_STATE_UNROLL          0x00000002 // to unroll (make visible) the controlbar... (it will roll again, as if the user moved the mouse on the caption).

#define HTCLOSE_BCG	HTOBJECT	// To prevent standard Windows 98/2000 close tooltip

/////////////////////////////////////////////////////////////////////////////
// CBCGSizingControlBar window

class CBCGSizingControlBar;
typedef CTypedPtrArray <CPtrArray, CBCGSizingControlBar*> CBCGSCBArray;

#define CBCGSIZINGCONTROLBAR_BUTTONS_NUM	2	// Hide + Expand

class BCGCONTROLBARDLLEXPORT CBCGSizingControlBar : public CControlBar
{
	friend class CBCGDockBar;
	friend class CBCGMiniDockFrameWnd;

    DECLARE_DYNAMIC(CBCGSizingControlBar);

// Construction
protected:
    CBCGSizingControlBar();

public:
    virtual BOOL Create(LPCTSTR lpszWindowName, CWnd* pParentWnd,
        CSize sizeDefault, BOOL bHasGripper, UINT nID,
        DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP);

// Attributes
public:
    CSize m_szHorz;
    CSize m_szVert;
    CSize m_szFloat;
    BOOL IsFloating() const;
    BOOL IsHorzDocked() const;
    BOOL IsVertDocked() const;
    BOOL IsSideTracking() const;

    void SetBorderSize (int nBorderSize)
    {
        m_nBorderSize = nBorderSize;
    }

    int GetBorderSize () const
    {
        return m_nBorderSize;
    }

	virtual void SetMinSize (CSize minSize);
 
    // [ET:] Support for maximum size
 	virtual void SetMaxSize (CSize minSize);

// Operations
public:
    virtual void LoadState(LPCTSTR lpszProfileName = NULL);
    virtual void SaveState(LPCTSTR lpszProfileName = NULL);
    static void GlobalLoadState(LPCTSTR lpszProfileName);
    static void GlobalSaveState(LPCTSTR lpszProfileName);

	static void SetCaptionStyle (BOOL bDrawText, BOOL bForceGradient = FALSE,
								 BOOL bHideDisabledButtons = FALSE);
		// bForceGradient is meaning if bDrawText is true

	static BOOL IsDrawCaption ()
	{
		return m_bCaptionText;
	}

	static BOOL IsCaptionGradient ()
	{
		return m_bCaptionGradient;
	}

	static BOOL IsHideDisabledButtons ()
	{
		return m_bHideDisabledButtons;
	}

	void EnableDocking(DWORD dwDockStyle);
	void Expand (BOOL bToggle = FALSE);
	void EnableRollUp (int nRollingType = BCG_ROLLUP_NORMAL);
	void ToggleDocking();

// Overridables
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

// Overrides
public:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBCGSizingControlBar)
	public:
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
    virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
    virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetRollupState(int state);

	virtual BOOL QueryClose()
	{
		// modified/added by Quentin Pouplard
		// allow bar to be destroyed on close...
		return TRUE;
	}

    virtual ~CBCGSizingControlBar();
    
protected:
    // implementation helpers
    UINT GetEdgeHTCode(int nEdge);
    BOOL GetEdgeRect(CRect rcWnd, UINT nHitTest, CRect& rcEdge);
    virtual void StartTracking(UINT nHitTest);
    virtual void StopTracking();
    virtual void OnTrackUpdateSize(CPoint& point);
    virtual void OnTrackInvertTracker();

    virtual void AlignControlBars();
    const int FindSizingBar(CControlBar* pBar) const;
    void GetRowInfo(int& nFirst, int& nLast, int& nThis);
    void GetRowSizingBars(CBCGSCBArray& arrSCBars);
    BOOL NegociateSpace(int nLengthAvail, BOOL bHorz);
	BOOL IsEdgeVisible (UINT nHitTest);
	BOOL IsNotFirst ();
	BOOL IsAlmostRight ();

	virtual void DrawCaption (CDC* pDC, const CRect& rectCaption);
	void TrackButtons (const CPoint& ptScreen);
	void RedrawButton (const CBCGSCBButton& btn);

	virtual void SetCustomizationMode (BOOL bCustMode);

	virtual BOOL ClipPaint () const	{	return TRUE;	}

protected:
    static CBCGSCBArray    m_arrBars;

    DWORD			m_dwSCBStyle;
    UINT			m_htEdge;

    CSize			m_szMax;    // [ET:] Support for maximum size
    CSize			m_szMin;
    CSize			m_szMinT;
    CSize			m_szMaxT;
    CSize			m_szOld;
    CPoint			m_ptOld;
    BOOL			m_bTracking;
    BOOL			m_bKeepSize;
    BOOL			m_bParentSizing;
    UINT			m_nDockBarID;
    int				m_cxEdge;
    int				m_cyGripper;
	BOOL			m_bMaximized;
	CToolTipCtrl	m_wndToolTip;
	BOOL			m_bIsSingleInRow;
	BOOL			m_bIsFlatButtons;
	BOOL			m_bActive;
	CRect			m_rectRedraw;
	int				m_nBorderSize;
	BOOL			m_nRollingType;
	BOOL			m_bIsRolled;

// modified/added by Quentin Pouplard
	int				m_NoRollup;

    CBCGSCBButton		m_Buttons [CBCGSIZINGCONTROLBAR_BUTTONS_NUM];

	static BOOL		m_bCaptionText;
	static BOOL		m_bCaptionGradient;
	static BOOL		m_bHideDisabledButtons;

// Generated message map functions
protected:
    //{{AFX_MSG(CBCGSizingControlBar)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnNcPaint();
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#endif // BCG_NO_SIZINGBAR

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGSIZINGCONTROLBAR_H__91380D71_4461_11D3_A72A_0090274409AC__INCLUDED_)
