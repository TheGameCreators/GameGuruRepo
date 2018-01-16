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

#if !defined(AFX_BCGSKINENTRY_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_)
#define AFX_BCGSKINENTRY_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef BOOL (__stdcall * GETBCGSKINVERSION)(int&,int&);
typedef BOOL (__stdcall * ISUNICODE)();
typedef int (__stdcall * GETBCGSKINCOUNT)(void);
typedef LPCTSTR (__stdcall * GETBCGSKINNAME)(int);
typedef LPCTSTR (__stdcall * GETBCGSKINAUTHOR)(int);
typedef LPCTSTR (__stdcall * GETBCGSKINAUTHORMAIL)(int);
typedef LPCTSTR (__stdcall * GETBCGSKINAUTHORURL)(int);
typedef CRuntimeClass* (__stdcall * GETBCGSKIN)(int);
typedef BOOL (__stdcall * BCGPREVIEWSKIN)(CDC*, int,CRect);

class CBCGSkinEntry
{
	friend class CBCGSkinManager;

public:
	CBCGSkinEntry (	const CString&	strLibraryPath,
					const CString&	strSkinName,
					const CString&	strSkinAuthor,
					const CString&	strSkinAuthorMail,
					const CString&	strSkinAuthorURL,
					int	iLibraryIndex,
					int	iSkinIndexInLibrary) :
		m_strLibraryPath (strLibraryPath),
		m_strSkinName (strSkinName),
		m_strSkinAuthor (strSkinAuthor),
		m_strSkinAuthorURL (strSkinAuthorURL),
		m_strSkinAuthorMail (strSkinAuthorMail),
		m_iLibraryIndex (iLibraryIndex),
		m_iSkinIndexInLibrary (iSkinIndexInLibrary)
	{
	}

	CBCGSkinEntry () :
		m_iLibraryIndex (-1),
		m_iSkinIndexInLibrary (-1)
	{
	}

	CBCGSkinEntry(const CBCGSkinEntry& source)
	{
		m_strLibraryPath		= source.m_strLibraryPath;
		m_strSkinName			= source.m_strSkinName;
		m_strSkinAuthor			= source.m_strSkinAuthor;
		m_strSkinAuthorMail		= source.m_strSkinAuthorMail;
		m_strSkinAuthorURL		= source.m_strSkinAuthorURL;
		m_iLibraryIndex			= source.m_iLibraryIndex;
		m_iSkinIndexInLibrary	= source.m_iSkinIndexInLibrary;
	}

	virtual ~CBCGSkinEntry()
	{
	}

// Attributes:
private:
	CString		m_strLibraryPath;
	CString		m_strSkinName;
	CString		m_strSkinAuthor;
	CString		m_strSkinAuthorMail;
	CString		m_strSkinAuthorURL;
	int			m_iLibraryIndex;
	int			m_iSkinIndexInLibrary;
};

class CBCGSkinLibrary
{
public:
// Opreations:
	CBCGSkinLibrary ()
	{
		m_pfGetSkinVersion = NULL;
		m_pfIsUNICODE = NULL;
		m_pfGetSkinCount = NULL;
		m_pfGetSkinName = NULL;
		m_pfGetSkinAuthor = NULL;
		m_pfGetSkinAuthorURL = NULL;
		m_pfGetSkinAuthorMail = NULL;
		m_pfGetSkin = NULL;
		m_hInstance = NULL;
	}

	CBCGSkinLibrary(const CBCGSkinLibrary& source)
	{
		m_pfGetSkinVersion = source.m_pfGetSkinVersion;
		m_pfIsUNICODE = source.m_pfIsUNICODE;
		m_pfGetSkinCount = source.m_pfGetSkinCount;
		m_pfGetSkinName = source.m_pfGetSkinName;
		m_pfGetSkinAuthor = source.m_pfGetSkinAuthor;
		m_pfGetSkinAuthorURL = source.m_pfGetSkinAuthorURL;
		m_pfGetSkinAuthorMail = source.m_pfGetSkinAuthorMail;
		m_pfGetSkin = source.m_pfGetSkin;
		m_hInstance = source.m_hInstance;
		m_pfSkinPreview = source.m_pfSkinPreview;
	}

	virtual ~CBCGSkinLibrary()
	{
	}

	BOOL Init (HINSTANCE hInstance);

	int GetSkinsCount () const
	{
		return (m_pfGetSkinCount == NULL) ? -1 : m_pfGetSkinCount ();
	}

	CRuntimeClass* GetSkin (int iIndex) const
	{
		return (m_pfGetSkin == NULL) ? NULL : m_pfGetSkin(iIndex);
	}

	BOOL GetSkinName (int iIndex, CString& strName) const
	{
		strName.Empty ();

		if (m_pfGetSkinName == NULL)
		{
			return FALSE;
		}

		LPCTSTR lpszName = (LPCTSTR) m_pfGetSkinName (iIndex);
		if (lpszName == NULL)
		{
			return FALSE;
		}

		strName = lpszName;
		return TRUE;
	}

	BOOL GetSkinAuthor (int iIndex, CString& strAuthor) const
	{
		strAuthor.Empty ();

		if (m_pfGetSkinAuthor == NULL)
		{
			return FALSE;
		}

		LPCTSTR lpszAuthor = (LPCTSTR) m_pfGetSkinAuthor (iIndex);
		if (lpszAuthor == NULL)
		{
			return FALSE;
		}

		strAuthor = lpszAuthor;
		return TRUE;
	}

	BOOL GetSkinAuthorURL (int iIndex, CString& strAuthorURL) const
	{
		strAuthorURL.Empty ();

		if (m_pfGetSkinAuthorURL == NULL)
		{
			return FALSE;
		}

		LPCTSTR lpszAuthorURL = (LPCTSTR) m_pfGetSkinAuthorURL (iIndex);
		if (lpszAuthorURL == NULL)
		{
			return FALSE;
		}

		strAuthorURL = lpszAuthorURL;
		return TRUE;
	}

	BOOL GetSkinAuthorMail (int iIndex, CString& strAuthorMail) const
	{
		strAuthorMail.Empty ();

		if (m_pfGetSkinAuthorMail == NULL)
		{
			return FALSE;
		}

		LPCTSTR lpszAuthorMail = (LPCTSTR) m_pfGetSkinAuthorMail (iIndex);
		if (lpszAuthorMail == NULL)
		{
			return FALSE;
		}

		strAuthorMail = lpszAuthorMail;
		return TRUE;
	}

	BOOL GetSkinVersion(int& iVersionMajor,int& iVersionMinor) const
	{
		return (m_pfGetSkinVersion == NULL) ? 
			FALSE : m_pfGetSkinVersion (iVersionMajor,iVersionMinor);
	}

	BOOL IsUNICODE() const
	{
		return (m_pfIsUNICODE == NULL) ? FALSE : m_pfIsUNICODE();
	}

	HINSTANCE GetInstance() const
	{
		return m_hInstance;
	}

	BOOL PreviewSkin (CDC* pDC, int iSkinIndex, CRect rect)
	{
		return (m_pfSkinPreview == NULL) ? FALSE : m_pfSkinPreview (pDC, iSkinIndex, rect);
	}

	HINSTANCE Detach ()
	{
		m_pfGetSkinVersion = NULL;
		m_pfIsUNICODE = NULL;
		m_pfGetSkinCount = NULL;
		m_pfGetSkinName = NULL;
		m_pfGetSkinAuthor = NULL;
		m_pfGetSkinAuthorURL = NULL;
		m_pfGetSkinAuthorMail = NULL;
		m_pfGetSkin = NULL;

		HINSTANCE hInstance = m_hInstance;
		m_hInstance = NULL;

		return hInstance;
	}

// Attributes:
public:
	GETBCGSKINVERSION	m_pfGetSkinVersion;
	ISUNICODE			m_pfIsUNICODE;
	GETBCGSKINCOUNT		m_pfGetSkinCount;
	GETBCGSKINNAME		m_pfGetSkinName;
	GETBCGSKINAUTHOR	m_pfGetSkinAuthor;
	GETBCGSKINAUTHOR	m_pfGetSkinAuthorMail;
	GETBCGSKINAUTHOR	m_pfGetSkinAuthorURL;
	GETBCGSKIN			m_pfGetSkin;
	BCGPREVIEWSKIN		m_pfSkinPreview;
	HINSTANCE			m_hInstance;
};

#endif // !defined(AFX_BCGSKINENTRY_H__5FD2EEA5_7D66_4687_A64F_31A271C233FD__INCLUDED_)
