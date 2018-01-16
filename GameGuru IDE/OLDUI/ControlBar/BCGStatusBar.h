#if !defined(AFX_BCGSTATUSBAR_H__97D13A73_CCC4_4968_A259_03E4AE7FD116__INCLUDED_)
#define AFX_BCGSTATUSBAR_H__97D13A73_CCC4_4968_A259_03E4AE7FD116__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGStatusBar.h : header file
//

#include "bcgcontrolbar.h"

class CBCGStatusBarPaneInfo;

/////////////////////////////////////////////////////////////////////////////
// CBCGStatusBar window

class BCGCONTROLBARDLLEXPORT CBCGStatusBar : public CControlBar
{
	DECLARE_DYNAMIC(CBCGStatusBar)

// Construction
public:
	CBCGStatusBar();
	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
			UINT nID = AFX_IDW_STATUS_BAR);
	BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = 0,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
		UINT nID = AFX_IDW_STATUS_BAR);
	BOOL SetIndicators(const UINT* lpIDArray, int nIDCount);

// Attributes
public: 
	// standard control bar things
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	void GetItemRect(int nIndex, LPRECT lpRect) const;

	// specific to CStatusBar
	void GetPaneText(int nIndex, CString& s) const;
	CString GetPaneText(int nIndex) const;
	BOOL SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate = TRUE);
	void GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle, int& cxWidth) const;
	void SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth);
	UINT GetPaneStyle(int nIndex) const;
	void SetPaneStyle(int nIndex, UINT nStyle);
	CString GetTipText(int nIndex) const;
	void SetTipText(int nIndex, LPCTSTR pszTipText);

	// specific to CBCGStatusBar
	int GetPaneWidth (int nIndex) const;
	void SetPaneWidth (int nIndex, int cx);

	void SetPaneIcon (int nIndex, HICON hIcon, BOOL bUpdate = TRUE);
	void SetPaneIcon (int nIndex, HBITMAP hBmp, 
					COLORREF clrTransparent = RGB (255, 0, 255), BOOL bUpdate = TRUE);
	void SetPaneAnimation (int nIndex, HIMAGELIST hImageList, 
							UINT nFrameRate = 500 /* ms */, BOOL bUpdate = TRUE);
	void SetPaneTextColor (int nIndex, 
		COLORREF clrText = (COLORREF)-1 /* Default */, BOOL bUpdate = TRUE);

	void EnablePaneProgressBar (int nIndex, long nTotal = 100 /* -1 - disable */,
								BOOL bDisplayText = FALSE, /* display "x%" */
								COLORREF clrBar = -1, COLORREF clrBarDest = -1 /* for gradient fill */,
								COLORREF clrProgressText = -1);
	void SetPaneProgress (int nIndex, long nCurr, BOOL bUpdate = TRUE);

	void InvalidatePaneContent (int nIndex);

	void EnablePaneDoubleClick (BOOL bEnable = TRUE);

// Implementation
public:
	virtual ~CBCGStatusBar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HFONT			m_hFont;
	int				m_cxSizeBox;        // style size box in corner
	BOOL			m_bHideSizeBox;		// hide size box if TRUE
	BOOL			m_bPaneDoubleClick;
	CRect			m_rectSizeBox;

	inline CBCGStatusBarPaneInfo* _GetPanePtr(int nIndex) const;
	void RecalcLayout ();
	CBCGStatusBarPaneInfo* HitTest (CPoint pt) const;

// Overrides:
protected:
	virtual void OnDrawPane (CDC* pDC, CBCGStatusBarPaneInfo* pPane);
	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	//{{AFX_MSG(CBCGStatusBar)
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGSTATUSBAR_H__97D13A73_CCC4_4968_A259_03E4AE7FD116__INCLUDED_)
