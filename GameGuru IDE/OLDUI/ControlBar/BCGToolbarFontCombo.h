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
// BCGToolbarFontCombo.h: interface for the CBCGToolbarFontCombo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARFONTCOMBO_H__A2F6A543_B1F8_4EBD_9F47_E8FA7430221B__INCLUDED_)
#define AFX_BCGTOOLBARFONTCOMBO_H__A2F6A543_B1F8_4EBD_9F47_E8FA7430221B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable : 4312)

#include "bcgcontrolbar.h"
#include "BCGToolbarComboBoxButton.h"

class BCGCONTROLBARDLLEXPORT CBCGFontDesc : public CObject
{
public:
	CBCGFontDesc(	LPCTSTR lpszName, LPCTSTR lpszScript, BYTE nCharSet,
					BYTE nPitchAndFamily, int nType) :
		m_strName (lpszName),
		m_strScript (lpszScript),
		m_nCharSet (nCharSet),
		m_nPitchAndFamily (nPitchAndFamily),
		m_nType (nType)
	{
	}

	CBCGFontDesc (const CBCGFontDesc& src) :
		m_strName (src.m_strName),
		m_strScript (src.m_strScript),
		m_nCharSet (src.m_nCharSet),
		m_nPitchAndFamily (src.m_nPitchAndFamily),
		m_nType (src.m_nType)
	{
	}

	const CString	m_strName;
	const CString	m_strScript;
	const BYTE		m_nCharSet;
	const BYTE		m_nPitchAndFamily;
	const int		m_nType;	// DEVICE_FONTTYPE, RASTER_FONTTYPE or TRUETYPE_FONTTYPE

	CString GetFullName () const
	{
		CString str = m_strName;
		if (!m_strScript.IsEmpty ())
		{
			str += _T(" (") + m_strScript + _T(")");
		}

		return str;
	}

	bool operator == (const CBCGFontDesc& other)
	{
		return	m_strName			== other.m_strName &&
				m_strScript			== other.m_strScript &&
				m_nCharSet			== other.m_nCharSet &&
				m_nPitchAndFamily	== other.m_nPitchAndFamily &&
				m_nType				== other.m_nType;
	}

};

//////////////////////////////////////////
//
class BCGCONTROLBARDLLEXPORT CBCGToolbarFontCombo : public CBCGToolbarComboBoxButton  
{
	DECLARE_SERIAL(CBCGToolbarFontCombo)

// Construction/Destruction
public:
	CBCGToolbarFontCombo(UINT uiID, int iImage,
						int nFontType = DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE,
						BYTE nCharSet = DEFAULT_CHARSET,
						DWORD dwStyle = CBS_DROPDOWN, int iWidth = 0,
						BYTE nPitchAndFamily = DEFAULT_PITCH);
	virtual ~CBCGToolbarFontCombo();

protected:
	CBCGToolbarFontCombo();

// Oprerations:
public:
	BOOL SetFont (LPCTSTR lpszName, BYTE nCharSet = DEFAULT_CHARSET, BOOL bExact = FALSE);
	const CBCGFontDesc* GetFontDesc (int iIndex = -1) const
	{
		return (CBCGFontDesc*) GetItemData (iIndex);
	}

protected:
	void SetContext ();
	void RebuildFonts ();
	static void ClearFonts ();
	static int GetFontsCount (LPCTSTR lpszName);

// Overrides:
protected:
	virtual CComboBox* CreateCombo (CWnd* pWndParent, const CRect& rect);
	virtual BOOL AddFont (ENUMLOGFONT* pelf, int nType, LPCTSTR lpszScript);
	virtual void Serialize (CArchive& ar);

	static BOOL CALLBACK AFX_EXPORT EnumFamScreenCallBackEx(
		ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX* /*lpntm*/, int FontType,
		LPVOID pThis);
	static BOOL CALLBACK AFX_EXPORT EnumFamPrinterCallBackEx(
		ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX* /*lpntm*/, int FontType, 
		LPVOID pThis);

// Attributes:
public:
	static int m_nFontHeight;

protected:
	int				m_nFontType;// Combination of DEVICE_FONTTYPE, RASTER_FONTTYPE and TRUETYPE_FONTTYPE
	BYTE			m_nCharSet;
	BYTE			m_nPitchAndFamily;
	static CObList	m_lstFonts;
	static int		m_nCount;	// CBCGToolbarFontCombo count
};

////////////////////////////////////////
// 
class BCGCONTROLBARDLLEXPORT CBCGToolbarFontSizeCombo : public CBCGToolbarComboBoxButton  
{
	DECLARE_SERIAL(CBCGToolbarFontSizeCombo)

// Construction/Destruction
public:
	CBCGToolbarFontSizeCombo(UINT uiID, int iImage,
						DWORD dwStyle = CBS_DROPDOWN, int iWidth = 0);
	virtual ~CBCGToolbarFontSizeCombo();

protected:
	CBCGToolbarFontSizeCombo();

// Attributes:
public:
	int GetTwipsLast () const
	{
		return m_nTwipsLast;
	}

protected:
	int m_nLogVert;
	int m_nTwipsLast;

// Operations:
public:
	void RebuildFontSizes (const CString& strFontName);
	
	void SetTwipSize (int nSize);
	int GetTwipSize () const;

protected:
	CString TwipsToPointString (int nTwips);
	void InsertSize(int nSize);
	static BOOL FAR PASCAL EnumSizeCallBack(LOGFONT FAR* lplf,
		LPNEWTEXTMETRIC lpntm,int FontType, LPVOID lpv);

// Overrides:
protected:
	virtual CComboBox* CreateCombo (CWnd* pWndParent, const CRect& rect);
};

#pragma warning (default : 4312)

#endif // !defined(AFX_BCGTOOLBARFONTCOMBO_H__A2F6A543_B1F8_4EBD_9F47_E8FA7430221B__INCLUDED_)
