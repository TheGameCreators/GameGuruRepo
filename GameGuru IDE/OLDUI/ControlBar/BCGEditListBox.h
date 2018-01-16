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

#if !defined(AFX_BCGEDITLISTBOX_H__80D80812_B943_11D3_A713_009027900694__INCLUDED_)
#define AFX_BCGEDITLISTBOX_H__80D80812_B943_11D3_A713_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGEditListBox.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "BCGButton.h"

#pragma warning (disable : 4244)

#define BGCEDITLISTBOX_BTN_NEW			0x0001
#define BGCEDITLISTBOX_BTN_DELETE		0x0002
#define BGCEDITLISTBOX_BTN_UP			0x0004
#define BGCEDITLISTBOX_BTN_DOWN			0x0008

#define BGCEDITLISTBOX_BTN_NEW_ID		(UINT)(-11)
#define BGCEDITLISTBOX_BTN_DELETE_ID	(UINT)(-12)
#define BGCEDITLISTBOX_BTN_UP_ID		(UINT)(-13)
#define BGCEDITLISTBOX_BTN_DOWN_ID		(UINT)(-14)

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBase window - edit list control abstract base class
// 
class BCGCONTROLBARDLLEXPORT CBCGEditListBase : public CStatic
{
// Construction
protected:
	CBCGEditListBase();

// Attributes:
	CList<CBCGButton*, CBCGButton*>	m_lstButtons;
	CList<DWORD, DWORD>				m_lstKeyAccell;
	CMap<int,int,UINT,UINT>			m_mapButtonIDs;
	CSize							m_sizeButton;
	CRect							m_rectCaption;
	UINT							m_uiStandardBtns;
	BOOL							m_bNewItem;
	CFont							m_font;
	BOOL							m_bIsActualDelete;	// Indicated that Items is really deletd, not moved
	BOOL							m_bBrowseButton;
	BOOL							m_bGrayDisabledButtons;

// Operations
public:
	BOOL SetStandardButtons (UINT uiBtns =
		BGCEDITLISTBOX_BTN_NEW | BGCEDITLISTBOX_BTN_DELETE | BGCEDITLISTBOX_BTN_UP | BGCEDITLISTBOX_BTN_DOWN);

	BOOL AddButton (UINT uiImageResId, LPCTSTR lpszTooltip = NULL,
					WORD wKeyAccelerator = 0,
					BYTE fVirt = 0,
					UINT uiButtonID = 0);

	BOOL EnableButton (int iButtonNum, BOOL bEnable = TRUE);
	
	int GetButtonsCount () const
	{
		return m_lstButtons.GetCount ();
	}

	UINT GetButtonID (int iButtonNum) const;
	int GetButtonNum (UINT uiID) const;

	// Enable/disable "Browse" button (appear right of edit control)
	void EnableBrowseButton (BOOL bEnable = TRUE);
	BOOL IsBrowseButton () const
	{
		return m_bBrowseButton;
	}

	// Allow to draw disabled button grayed. By default, the disabled button will be drawn
	// with the regular look
	void SetGrayDisabledButtons (BOOL bOn = TRUE);
	BOOL IsGrayDisabledButtons () const
	{
		return m_bGrayDisabledButtons;
	}

	//-------------------------------------------------------------
	// These methods should be implemented in the actual edit list:
	//-------------------------------------------------------------
	virtual int AddItem (const CString& strIext, DWORD dwData = 0, int iIndex = -1) = 0;
	virtual BOOL RemoveItem (int iIndex) = 0;
	virtual int GetCount () const = 0;

	virtual int GetSelItem () const = 0;
	virtual BOOL SelectItem (int iItem) = 0;

	virtual CString GetItemText (int iIndex) const = 0;
	virtual void SetItemText (int iIndex, const CString& strText) = 0;
	virtual DWORD GetItemData (int iIndex) const = 0;
	virtual void SetItemData (int iIndex, DWORD dwData) = 0;

	virtual BOOL EditItem (int iIndex) = 0;

protected:
	void Init ();
	int GetStdButtonNum (UINT uiStdBtn) const;
	void AdjustLayout ();

// Overrides
public:
	virtual void OnClickButton (int iButton);
	virtual void OnKey (WORD wKey, BYTE fFlags);
	virtual void OnEndEditLabel (LPCTSTR lpszLabel);
	virtual int OnGetImage (LV_ITEM* /*pItem*/)
	{
		return -1;
	}
	
	virtual void OnSelectionChanged () {}

	// "Standard" action overrides
	virtual BOOL OnBeforeRemoveItem (int /*iItem*/)	{	return TRUE;	}
	virtual void OnAfterAddItem (int /*iItem*/) {}
	virtual void OnAfterRenameItem (int /*iItem*/) {}
	virtual void OnAfterMoveItemUp (int /*iItem*/) {}
	virtual void OnAfterMoveItemDown (int /*iItem*/) {}

	// "Browse" button drawing:
	virtual void OnDrawBrowseButton (CDC* pDC, CRect rectBtn, 
									BOOL bPressed, BOOL bHighlighted);

	virtual void OnBrowse () {}

protected:
	virtual void CreateNewItem ();

	virtual HWND GetListHwnd () const = 0;
	virtual CWnd* OnCreateList () = 0;
	virtual void OnSizeList () = 0;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGEditListBase)
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGEditListBase();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGEditListBase)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
    afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetFont(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListEdit window

class CBCGEditListEdit : public CEdit
{
// Construction
public:
	CBCGEditListEdit();

// Attributes
protected:
	BOOL				m_bLocked;
	BOOL				m_bBrowseBtn;
	CRect				m_rectBtn;
	BOOL				m_bIsButtonPressed;
	BOOL				m_bIsButtonCaptured;
	CBCGEditListBase*	m_pParentList;

// Operations
public:
	void LockSize (CBCGEditListBase* pParent, BOOL bLock = TRUE);
	void EnableBrowse (BOOL bEnable = TRUE)
	{
		m_bBrowseBtn = bEnable;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGEditListEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGEditListEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGEditListEdit)
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBox window

class BCGCONTROLBARDLLEXPORT CBCGEditListBox : public CBCGEditListBase
{
// Construction
public:
	CBCGEditListBox();

// Attributes
public:
	CListCtrl*	m_pWndList;	// Embedded list control

protected:
	CPoint				m_ptClick;
	CBCGEditListEdit	m_wndEdit;

// Operations
public:
	virtual int AddItem (const CString& strIext, DWORD dwData = 0, int iIndex = -1);
	virtual BOOL RemoveItem (int iIndex);
	virtual int GetCount () const;

	virtual int GetSelItem () const;
	virtual BOOL SelectItem (int iItem);

	virtual CString GetItemText (int iIndex) const;
	virtual DWORD GetItemData (int iIndex) const;
	virtual void SetItemData (int iIndex, DWORD dwData);

	virtual BOOL EditItem (int iIndex);

protected:
	virtual HWND GetListHwnd () const
	{
		return m_pWndList->GetSafeHwnd ();
	}

	virtual CWnd* OnCreateList ();
	virtual void OnSizeList ();
	virtual void SetItemText (int iIndex, const CString& strText);

// Overrides
public:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGEditListBox)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGEditListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGEditListBox)
	//}}AFX_MSG
#if _MSC_VER >= 1300
	afx_msg void OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult);
#else
	afx_msg void OnKeyDown (LPNMLVKEYDOWN pKeyDown, LRESULT* pResult);
#endif
	afx_msg void OnDblclkList (NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#pragma warning (default : 4244)

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGEDITLISTBOX_H__80D80812_B943_11D3_A713_009027900694__INCLUDED_)
