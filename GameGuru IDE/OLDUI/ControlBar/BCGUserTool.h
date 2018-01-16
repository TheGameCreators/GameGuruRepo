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

// BCGUserTool.h: interface for the CBCGUserTool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGUSERTOOL_H__80D80814_B943_11D3_A713_009027900694__INCLUDED_)
#define AFX_BCGUSERTOOL_H__80D80814_B943_11D3_A713_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class BCGCONTROLBARDLLEXPORT CBCGUserTool : public CObject  
{
	friend class CBCGUserToolsManager;

	DECLARE_SERIAL(CBCGUserTool)

public:
// Construction
	CBCGUserTool();
	virtual ~CBCGUserTool();

// Overrides
	virtual void Serialize (CArchive& ar);
	virtual BOOL Invoke ();
	virtual HICON SetToolIcon ();

// Operations:
public:
	void DrawToolIcon (CDC* pDC, const CRect& rectImage);
	void SetCommand (LPCTSTR lpszCmd);
	BOOL CopyIconToClipboard ();

protected:
	virtual HICON LoadDefaultIcon ();
	virtual void DeleteIcon ();

// Attributes:
public:
	UINT GetCommandId () const
	{
		return m_uiCmdId;
	}

	const CString& GetCommand () const
	{
		return m_strCommand;
	}

	CString		m_strLabel;
	CString		m_strArguments;
	CString		m_strInitialDirectory;

protected:
	UINT		m_uiCmdId;
	CString		m_strCommand;
	HICON		m_hIcon;
};

#endif // !defined(AFX_BCGUSERTOOL_H__80D80814_B943_11D3_A713_009027900694__INCLUDED_)
