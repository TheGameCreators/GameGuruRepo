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

// BCGToolbarComboBoxButton.h: interface for the CBCGToolbarComboBoxButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)
#define AFX_BCGTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "BCGToolbarButton.h"

class CBCGToolbarMenuButton;

class BCGCONTROLBARDLLEXPORT CBCGToolbarComboBoxButton : public CBCGToolbarButton  
{
	friend class CBCGComboEdit;

	DECLARE_SERIAL(CBCGToolbarComboBoxButton)

public:
	CBCGToolbarComboBoxButton();
	CBCGToolbarComboBoxButton(UINT uiID, int iImage, DWORD dwStyle = CBS_DROPDOWNLIST, int iWidth = 0);
	virtual ~CBCGToolbarComboBoxButton();

// Operations:
	int AddItem (LPCTSTR lpszItem, DWORD dwData = 0);
	int GetCount () const;
	LPCTSTR GetItem (int iIndex = -1) const;
	DWORD GetItemData (int iIndex = -1) const;
	int GetCurSel () const
	{
		return m_iSelIndex;
	}
	void RemoveAllItems ();
	BOOL SelectItem (int iIndex, BOOL bNotify = TRUE);
	BOOL SelectItem (DWORD dwData);
	BOOL SelectItem (LPCTSTR lpszText);

	void SetDropDownHeight (int nHeight);

// Overrides:
	virtual CComboBox* CreateCombo (CWnd* pWndParent, const CRect& rect);

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual void CopyFrom (const CBCGToolbarButton& src);
	virtual void Serialize (CArchive& ar);
	virtual void SerializeContent (CArchive& ar);
	virtual SIZE OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz);
	virtual BOOL OnClick (CWnd* pWnd, BOOL bDelay = TRUE);
	virtual void OnChangeParentWnd (CWnd* pWndParent);
	virtual void OnMove ();
	virtual void OnSize (int iSize);
	virtual HWND GetHwnd ()
	{	
		return m_pWndCombo->GetSafeHwnd ();
	}

	virtual void EnableWindow (BOOL bEnable = TRUE)
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL)
		{
			m_pWndCombo->EnableWindow (bEnable);
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL)
		{
			m_pWndEdit->EnableWindow (bEnable);
		}
	}

	virtual BOOL IsWindowVisible ()
	{
		return ((m_pWndCombo->GetSafeHwnd () != NULL &&
				m_pWndCombo->GetStyle () & WS_VISIBLE) ||
				(m_pWndEdit->GetSafeHwnd () != NULL &&
				m_pWndEdit->GetStyle () & WS_VISIBLE));
	}

	virtual BOOL IsOwnerOf (HWND hwnd)
	{
		if (m_pWndCombo->GetSafeHwnd () != NULL &&
			(m_pWndCombo->GetSafeHwnd () == hwnd || 
			::IsChild (m_pWndCombo->GetSafeHwnd (), hwnd)))
		{
			return TRUE;
		}

		if (m_pWndEdit->GetSafeHwnd () != NULL &&
			(m_pWndEdit->GetSafeHwnd () == hwnd || 
			::IsChild (m_pWndEdit->GetSafeHwnd (), hwnd)))
		{
			return TRUE;
		}

		return TRUE;
	}

	virtual BOOL NotifyCommand (int iNotifyCode);
	
	virtual BOOL CanBeStretched () const
	{	
		return TRUE;	
	}
	virtual void OnAddToCustomizePage ();
	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);
	virtual int OnDrawOnCustomizeList (
			CDC* pDC, const CRect& rect, BOOL bSelected);

	virtual void DuplicateData () {}
	virtual void ClearData () {}

	virtual void OnShow (BOOL bShow);
	virtual BOOL ExportToMenuButton (CBCGToolbarMenuButton& menuButton) const;

	virtual void SetStyle (UINT nStyle);

	virtual BOOL IsAutoSynchSelection () const	{	return TRUE;	}

protected:
	void Initialize ();
	void AdjustRect ();
	void SetHotEdit (BOOL bHot = TRUE);

// Attributes:
public:
	static void SetFlatMode (BOOL bFlat = TRUE)
	{
		m_bFlat = bFlat;
	}

	static BOOL IsFlatMode ()
	{
		return m_bFlat;
	}

	CComboBox* GetComboBox () const
	{
		return m_pWndCombo;
	}

	LPCTSTR GetText () const
	{
		return m_strEdit;
	}

	void SetText (LPCTSTR lpszText);

	BOOL IsSerializeContent () const
	{
		return m_bSerializeContent;
	}

	void SetSerializeContent (BOOL bSet = TRUE)
	{
		m_bSerializeContent = bSet;
	}

	static CBCGToolbarComboBoxButton* GetByCmd (UINT uiCmd, BOOL bIsFocus = FALSE);
	static BOOL SelectItemAll (UINT uiCmd, int iIndex);
	static BOOL SelectItemAll (UINT uiCmd, DWORD dwData);
	static BOOL SelectItemAll (UINT uiCmd, LPCTSTR lpszText);
	static int GetCountAll (UINT uiCmd);
	static int GetCurSelAll (UINT uiCmd);
	static LPCTSTR GetItemAll (UINT uiCmd, int iIndex = -1);
	static DWORD GetItemDataAll (UINT uiCmd, int iIndex = -1);
	static void* GetItemDataPtrAll (UINT uiCmd, int iIndex = -1);
	static LPCTSTR GetTextAll (UINT uiCmd);

	virtual BOOL HasFocus () const;

protected:
	DWORD				m_dwStyle;
	CComboBox*			m_pWndCombo;
	CEdit*				m_pWndEdit;

	CStringList			m_lstItems;
	CList<DWORD, DWORD>	m_lstItemData;
	int					m_iWidth;
	int					m_iSelIndex;

	BOOL				m_bSerializeContent;

	BOOL				m_bHorz;
	CString				m_strEdit;
	CRect				m_rectCombo;
	CRect				m_rectButton;

	int					m_nDropDownHeight;
	BOOL				m_bIsHotEdit;

	static BOOL			m_bFlat;
};

#endif // !defined(AFX_BCGTOOLBARCOMBOBOXBUTTON_H__D5B381B4_CC65_11D1_A648_00A0C93A70EC__INCLUDED_)

/////////////////////////////////////////////////////////////////////////////
