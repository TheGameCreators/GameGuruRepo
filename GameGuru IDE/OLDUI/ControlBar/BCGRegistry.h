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

#if !defined(AFX_BCGREGISTRY_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
#define AFX_BCGREGISTRY_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Shane Martin
// All rights reserved
//
// Distribute freely, except: don't remove my name from the source or
// documentation (don't take credit for my work), mark your changes (don't
// get me blamed for your possible bugs), don't alter or remove this
// notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    shane.kim@kaiserslautern.netsurf.de
/////////////////////////////////////////////////////////////////////////////
#include <winreg.h>

#define REG_RECT	0x0001
#define REG_POINT	0x0002

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CBCGRegistry : public CObject
{
	friend class CBCGRegistrySP;

	DECLARE_DYNCREATE(CBCGRegistry)

// Construction
public:
	CBCGRegistry (BOOL bAdmin, BOOL bReadOnly);
	virtual ~CBCGRegistry();

	struct REGINFO
	{
		LONG lMessage;
		DWORD dwType;
		DWORD dwSize;
	}
	m_Info;

protected:
	CBCGRegistry ();

// Operations
public:
	virtual BOOL ReadKeyValues(CStringArray& Values);
	virtual BOOL VerifyKey (LPCTSTR pszPath);
	virtual BOOL VerifyValue (LPCTSTR pszValue);
	virtual BOOL CreateKey (LPCTSTR pszPath);
	virtual BOOL Open (LPCTSTR pszPath);
	virtual void Close();

	virtual BOOL DeleteValue (LPCTSTR pszValue);
	virtual BOOL DeleteKey (LPCTSTR pszPath, BOOL bAdmin = FALSE);

	virtual BOOL Write (LPCTSTR pszKey, int iVal);
	virtual BOOL Write (LPCTSTR pszKey, DWORD dwVal);
	virtual BOOL Write (LPCTSTR pszKey, LPCTSTR pszVal);
	virtual BOOL Write (LPCTSTR pszKey, CStringList& scStringList);
	virtual BOOL Write (LPCTSTR pszKey, CByteArray& bcArray);
	virtual BOOL Write (LPCTSTR pszKey, CStringArray& scArray);
	virtual BOOL Write (LPCTSTR pszKey, CDWordArray& dwcArray);
	virtual BOOL Write (LPCTSTR pszKey, CWordArray& wcArray);
	virtual BOOL Write (LPCTSTR pszKey, const CRect& rect);
	virtual BOOL Write (LPCTSTR pszKey, LPPOINT& lpPoint);
	virtual BOOL Write (LPCTSTR pszKey, LPBYTE pData, UINT nBytes);
	virtual BOOL Write (LPCTSTR pszKey, CObList& list);
	virtual BOOL Write (LPCTSTR pszKey, CObject& obj);
	virtual BOOL Write (LPCTSTR pszKey, CObject* pObj);

	virtual BOOL Read (LPCTSTR pszKey, int& iVal);
	virtual BOOL Read (LPCTSTR pszKey, DWORD& dwVal);
	virtual BOOL Read (LPCTSTR pszKey, CString& sVal);
	virtual BOOL Read (LPCTSTR pszKey, CStringList& scStringList);
	virtual BOOL Read (LPCTSTR pszKey, CStringArray& scArray);
	virtual BOOL Read (LPCTSTR pszKey, CDWordArray& dwcArray);
	virtual BOOL Read (LPCTSTR pszKey, CWordArray& wcArray);
	virtual BOOL Read (LPCTSTR pszKey, CByteArray& bcArray);
	virtual BOOL Read (LPCTSTR pszKey, LPPOINT& lpPoint);
	virtual BOOL Read (LPCTSTR pszKey, CRect& rect);
	virtual BOOL Read (LPCTSTR pszKey, BYTE** ppData, UINT* pBytes);
	virtual BOOL Read (LPCTSTR pszKey, CObList& list);
	virtual BOOL Read (LPCTSTR pszKey, CObject& obj);
	virtual BOOL Read (LPCTSTR pszKey, CObject*& pObj);

	virtual BOOL ReadSubKeys(CStringArray& SubKeys);

protected:

	HKEY		m_hKey;
	BOOL		m_bAdmin;
	CString		m_sPath;
	BOOL		m_bReadOnly;
	DWORD		m_dwUserData;
};

//////////////////////////////////////////////////////////////////////////////
// CBCGRegistrySP - Helper class that manages "safe" CBCGRegistry pointer

class BCGCONTROLBARDLLEXPORT CBCGRegistrySP
{
public:
	static BOOL SetRuntimeClass (CRuntimeClass* pRTI);

	CBCGRegistrySP(DWORD dwUserData = 0) :
		m_pRegistry (NULL),
		m_dwUserData (dwUserData)
	{
	}

	~CBCGRegistrySP()
	{
		if (m_pRegistry != NULL)
		{
			ASSERT_VALID (m_pRegistry);
			delete m_pRegistry;
		}
	}

	CBCGRegistry& Create (BOOL bAdmin, BOOL bReadOnly);

protected:
	CBCGRegistry*			m_pRegistry;
	static CRuntimeClass*	m_pRTIDefault;

	DWORD					m_dwUserData;
};


#endif // #if !defined(AFX_BCGREGISTRY_H__A6E1AD76_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
