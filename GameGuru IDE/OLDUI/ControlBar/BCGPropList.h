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

#if !defined(AFX_BCGPROPLIST_H__8E7648AE_215A_4D66_B6B1_6656A7D6D357__INCLUDED_)
#define AFX_BCGPROPLIST_H__8E7648AE_215A_4D66_B6B1_6656A7D6D357__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPropList.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "comdef.h"
#include "bcgcontrolbar.h"
#include "ColorPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGProp object

class BCGCONTROLBARDLLEXPORT CBCGProp : public CObject
{
	DECLARE_DYNAMIC(CBCGProp)

	friend class CBCGPropList;

// Construction
public:
	// Group constructor
	CBCGProp(const CString& strGroupName, DWORD dwData = 0);

	// Simple property
	CBCGProp(const CString& strName, const _variant_t& varValue, 
		LPCTSTR lpszDescr = NULL, DWORD dwData = 0);

	virtual ~CBCGProp();

	CString	m_FPSCDir;

	enum ClickArea
	{
		ClickExpandBox,
		ClickName,
		ClickValue
	};

// Operations:
public:
	int GetExpandedSubItems () const;
	BOOL AddSubItem (CBCGProp* pProp);

	BOOL AddOption (LPCTSTR lpszOption);
	void RemoveAllOptions ();

	CBCGProp* HitTest (CPoint point, CBCGProp::ClickArea* pnArea = NULL);

	void Expand (BOOL bExpand = TRUE);
	void Redraw ();

protected:
	void Init ();
	void SetFlags ();
	void SetOwnerList (CBCGPropList* pWndList);
	void Repos (int& y);
	void AddTerminalProp (CList<CBCGProp*, CBCGProp*>& lstProps);

	BOOL IsSubItem (CBCGProp* pProp) const;

// Overrides
public:
	virtual void OnDrawName (CDC* pDC, CRect rect);
	virtual void OnDrawValue (CDC* pDC, CRect rect);
	virtual void OnDrawExpandBox (CDC* pDC, CRect rectExpand);
	virtual void OnDrawButton (CDC* pDC, CRect rectButton);

	virtual CString FormatProperty ();

	virtual BOOL OnUpdateValue ();
	virtual BOOL OnEdit (LPPOINT lptClick);

	virtual BOOL OnEndEdit ();

	virtual void OnClickButton (CPoint point);
	virtual BOOL OnDblClick (CPoint point);

	virtual void OnSelectCombo ();
	virtual void OnCloseCombo();

	virtual BOOL OnSetCursor () const;
	virtual BOOL PushChar (UINT nChar);

	virtual CString GetNameTooltip ();
	virtual CString GetValueTooltip ();

protected:
	virtual HBRUSH OnCtlColor(CDC* pDC, UINT nCtlColor);
	virtual CComboBox* CreateCombo (CWnd* pWndParent, CRect rect);
	virtual void OnDestroyWindow ();

	virtual BOOL OnKillFocus (CWnd* /*pNewWnd*/)
	{
		return TRUE;
	}

	virtual BOOL HasButton () const;

// Attributes
public:
	const _variant_t& GetValue () const
	{
		return m_varValue;
	}

	void SetValue (const _variant_t& varValue);

	const CString& GetDesciption () const
	{
		return m_strDescr;
	}

	void SetDesciption (const CString& strDescr)
	{
		m_strDescr = strDescr;
	}

	DWORD GetData () const
	{
		return m_dwData;
	}

	void SetData (DWORD dwData)
	{
		m_dwData = dwData;
	}

	BOOL IsGroup () const
	{
		return m_bGroup;
	}

	BOOL IsExpanded () const
	{
		return m_bExpanded;
	}

	BOOL IsParentExpanded () const;

	BOOL IsSelected () const;
	int GetHierarchyLevel () const;

	void Enable (BOOL bEnable = TRUE)
	{
		m_bEnabled = bEnable;
	}

	BOOL IsEnabled () const
	{
		return m_bEnabled;
	}

	void AllowEdit (BOOL bAllow = TRUE)
	{
		ASSERT (m_varValue.vt != VT_BOOL);
		m_bAllowEdit = bAllow;
	}

	BOOL IsAllowEdit () const
	{
		return m_bAllowEdit;
	}

	CRect GetRect () const
	{
		return m_Rect;
	}

	int GetSubItemsCount () const
	{
		return m_lstSubItems.GetCount ();
	}

	CBCGProp* GetSubItem (int nIndex) const;

	CBCGProp* GetParent () const
	{
		return m_pParent;
	}

	

public:
	// Data formats
	static CString	m_strFormatShort;
	static CString	m_strFormatLong;
	static CString	m_strFormatFloat;
	static CString	m_strFormatDouble;

	// MIKE - 260804
	//protected:
	public:
	CString			m_strName;		// Property name
	_variant_t		m_varValue;		// Property value
	CBCGPropList*	m_pWndList;		// Pointer to the PropertyList window
	DWORD			m_dwData;		// User-defined data
	CString			m_strDescr;		// Property desciption

	CStringList		m_lstOptions;	// List of combobox items

	BOOL			m_bInPlaceEdit;	// Is in InPalce editing mode

	CWnd*			m_pWndInPlace;
	CComboBox*		m_pWndCombo;

	CRect			m_Rect;
	CRect			m_rectButton;
	BOOL			m_bButtonIsDown;
	
	BOOL			m_bGroup;
	BOOL			m_bExpanded;
	BOOL			m_bEnabled;
	BOOL			m_bAllowEdit;
	DWORD			m_dwFlags;

	CBCGProp*		m_pParent;
	CList<CBCGProp*, CBCGProp*>	m_lstSubItems;

	BOOL			m_bNameIsTrancated;
	BOOL			m_bValueIsTrancated;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGColorProp object

class BCGCONTROLBARDLLEXPORT CBCGColorProp : public CBCGProp
{
	friend class CBCGPropList;

	DECLARE_DYNAMIC(CBCGColorProp)

// Construction
public:
	CBCGColorProp(const CString& strName, const COLORREF& color, 
		CPalette* pPalette = NULL, LPCTSTR lpszDescr = NULL, DWORD dwData = 0);
	virtual ~CBCGColorProp();

// Overrides
public:
	virtual void OnDrawValue (CDC* pDC, CRect rect);
	virtual void OnClickButton (CPoint point);
	virtual BOOL OnEdit (LPPOINT lptClick);
	virtual BOOL OnUpdateValue ();
	virtual CString FormatProperty ();

protected:
	virtual BOOL OnKillFocus (CWnd* pNewWnd)
	{
		return pNewWnd->GetSafeHwnd () != m_pPopup->GetSafeHwnd ();
	}

// Attributes
public:
	COLORREF GetColor () const
	{
		return m_Color;
	}

	void SetColor (COLORREF color);

	// Color popup attributes:
	void EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable = TRUE);
	void EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg = TRUE, BOOL bEnable = TRUE);

// Attributes
protected:
	COLORREF					m_Color;
	COLORREF					m_ColorAutomatic;
	CString						m_strAutoColor;
	BOOL						m_bStdColorDlg;
	CString						m_strOtherColor;
	CColorPopup*				m_pPopup;
	CArray<COLORREF, COLORREF>	m_Colors;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGFileProp object

class BCGCONTROLBARDLLEXPORT CBCGFileProp : public CBCGProp
{
	DECLARE_DYNAMIC(CBCGFileProp)

// Construction
public:
	CBCGFileProp(const CString& strName, const CString& strFolderName, DWORD dwData = 0, LPCTSTR lpszDescr = NULL);
	CBCGFileProp(const CString& strName, BOOL bOpenFileDialog, const CString& strFileName, 
				LPCTSTR lpszDefExt = NULL,
				DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
				LPCTSTR lpszFilter = NULL,
				LPCTSTR lpszDescr = NULL, DWORD dwData = 0);
	virtual ~CBCGFileProp();

	CString m_Mike_szDirectory;
	CString m_Mike_szFilter;
	CString m_Mike_szTitle;
	bool	m_Mike_Type;

// Overrides
public:
	virtual void OnClickButton (CPoint point);

// Attributes
protected:
	BOOL m_bIsFolder;

	// File open dialog atributes:
	BOOL	m_bOpenFileDialog;
	DWORD	m_dwFileOpenFlags;
	CString	m_strDefExt;
	CString m_strFilter;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGFontProp object

class BCGCONTROLBARDLLEXPORT CBCGFontProp : public CBCGProp
{
	DECLARE_DYNAMIC(CBCGFontProp)

// Construction
public:
	CBCGFontProp(	const CString& strName, LOGFONT& lf, 
					DWORD dwFontDialogFlags = CF_EFFECTS | CF_SCREENFONTS, 
					LPCTSTR lpszDescr = NULL, DWORD dwData = 0);
	virtual ~CBCGFontProp();

// Overrides
public:
	virtual void OnClickButton (CPoint point);

// Attributes
public:
	LPLOGFONT GetLogFont ()
	{
		return &m_lf;
	}

protected:
	LOGFONT	m_lf;
	DWORD	m_dwFontDialogFlags;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPropList window

class BCGCONTROLBARDLLEXPORT CBCGPropList : public CWnd
{
	friend class CBCGProp;
	friend class CBCGColorProp;

// Construction
public:
	CBCGPropList();

// Attributes
public:
	void EnableHeaderCtrl (BOOL bEnable = TRUE,
		LPCTSTR lpszLeftColumn = _T("Property"), 
		LPCTSTR lpszRightColumn = _T("Name"));
	BOOL IsHeaderCtrl () const
	{
		return m_bHeaderCtrl;
	}

	void EnableDesciptionArea (BOOL bEnable = TRUE);
	BOOL IsDesciptionArea () const
	{
		return m_bDesciptionArea;
	}

	int GetDesciptionHeight () const
	{
		return m_nDescrHeight;
	}

	void SetAlphabeticMode (BOOL bSet = TRUE);
	BOOL IsAlphabeticMode () const
	{
		return m_bAlphabeticMode;
	}

	void SetVSDotNetLook (BOOL bSet = TRUE);
	BOOL IsVSDotNetLook () const
	{
		return m_bVSDotNetLook;
	}

	void SetBoolLabels (LPCTSTR lpszTrue, LPCTSTR lpszFalse);

	int			m_nVertScrollOffset;	// In rows

protected:
	CHeaderCtrl	m_wndHeader;
	BOOL		m_bHeaderCtrl;

	BOOL		m_bDesciptionArea;
	int			m_nDescrHeight;

	CToolTipCtrl	m_ToolTip;
	int			m_nTooltipsCount;

	BOOL		m_bAlphabeticMode;
	BOOL		m_bVSDotNetLook;

	CString		m_strTrue;
	CString		m_strFalse;

	CScrollBar	m_wndScrollVert;
	HFONT		m_hFont;
	CFont		m_fontBold;

	int			m_nBorderSize;

	int			m_nHeaderHeight;
	CRect		m_rectList;
	int			m_nRowHeight;
	int			m_nLeftColumnWidth;

	//int			m_nVertScrollOffset;	// In rows
	int			m_nVertScrollTotal;
	int			m_nVertScrollPage;

	CRect		m_rectTrackHeader;
	CRect		m_rectTrackDescr;
	BOOL		m_bTracking;
	BOOL		m_bTrackingDescr;

	CList<CBCGProp*, CBCGProp*>	m_lstProps;
	CList<CBCGProp*, CBCGProp*>	m_lstTerminalProps;

	CBCGProp*	m_pSel;
	
	BOOL		m_bFocused;

	COLORREF	m_clrGray;

// Operations
public:
	int AddProperty (CBCGProp* pProp, BOOL bRedraw = TRUE);
	void RemoveAll ();

	CBCGProp* GetProperty (int nIndex) const;
	int GetPropertyCount () const
	{
		return m_lstProps.GetCount ();
	}

	CBCGProp* FindItemByData (DWORD dwData) const;

	CBCGProp* HitTest (CPoint pt, CBCGProp::ClickArea* pnArea = NULL) const;

	void SetCurSel (CBCGProp* pProp, BOOL bRedraw = TRUE);
	CBCGProp* GetCurSel () const
	{
		return m_pSel;
	}

	void EnsureVisible (CBCGProp* pProp);

	void CloseColorPopup ();
	void UpdateColor (COLORREF color);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPropList)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

	virtual void OnChangeSelection (CBCGProp* /*pNewSel*/, CBCGProp* /*pOldSel*/) {}

	virtual BOOL EditItem (CBCGProp* pProp, LPPOINT lptClick = NULL);
	virtual void OnClickButton (CPoint point);
	virtual BOOL EndEditItem (BOOL bUpdateData = TRUE);
	virtual BOOL ValidateItemData (CBCGProp* /*pProp*/)
	{
		return TRUE;
	}

	virtual int OnDrawProperty (CDC* pDC, CBCGProp* pProp) const;

protected:
	virtual void Init ();
	virtual void AdjustLayout ();

	virtual void OnDrawBorder (CDC* pDC);
	virtual void OnDrawList (CDC* pDC);
	virtual void OnDrawDesciption (CDC* pDC, CRect rect);

// Implementation
public:
	virtual ~CBCGPropList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPropList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSysColorChange();
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderEndTrack(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStyleChanging(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnSelectCombo();
	afx_msg void OnCloseCombo();
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	HFONT SetFont (CDC* pDC);
	void TrackHeader (int nOffset);
	void TrackDescr (int nOffset);

	void SetScrollSizes ();

	int GetTotalItems () const;
	void ReposProperties ();

	void CreateBoldFont ();
	void UpdateColors ();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPROPLIST_H__8E7648AE_215A_4D66_B6B1_6656A7D6D357__INCLUDED_)
