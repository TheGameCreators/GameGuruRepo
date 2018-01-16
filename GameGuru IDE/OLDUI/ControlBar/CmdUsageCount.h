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

// CmdUsageCount.h: interface for the CCmdUsageCount class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMDUSAGECOUNT_H__B7BDB593_32E1_11D3_A724_0090274409AC__INCLUDED_)
#define AFX_CMDUSAGECOUNT_H__B7BDB593_32E1_11D3_A724_0090274409AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

class CCmdUsageCount : public CObject  
{
public:
	CCmdUsageCount();
	virtual ~CCmdUsageCount();

	virtual void Serialize (CArchive& ar);

	void AddCmd (UINT uiCmd);
	void Reset ();

	UINT GetCount (UINT uiCmd) const;
	BOOL HasEnouthInformation () const;

	BOOL IsFreqeuntlyUsedCmd (UINT uiCmd) const;

	static BOOL SetOptions (UINT nStartCount, UINT nMinUsagePercentage);

protected:
	CMap<UINT, UINT, UINT, UINT>	m_CmdUsage;
	UINT							m_nTotalUsage;

	static UINT						m_nStartCount;
	static UINT						m_nMinUsagePercentage;
};

#endif // !defined(AFX_CMDUSAGECOUNT_H__B7BDB593_32E1_11D3_A724_0090274409AC__INCLUDED_)
