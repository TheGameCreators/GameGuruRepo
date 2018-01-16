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

// ImageHash.h: interface for the CImageHash class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEHASH_H__FBA4F684_D756_11D1_A64E_00A0C93A70EC__INCLUDED_)
#define AFX_IMAGEHASH_H__FBA4F684_D756_11D1_A64E_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CImageHash : public CObject  
{

public:
	CImageHash();
	virtual ~CImageHash();

	void Set (UINT uiCmd, int iImage, BOOL bUserImage);
	int Get (UINT uiCmd, BOOL bUserImage) const;
	void Clear (UINT uiCmd);
	void ClearAll ();

	static int GetImageOfCommand (UINT uiCmd, BOOL bUserImage);

protected:
	CMap<UINT, UINT, int, int>	m_StdImages;
	CMap<UINT, UINT, int, int>	m_UserImages;
};

extern CImageHash	g_ImageHash;

#endif // !defined(AFX_IMAGEHASH_H__FBA4F684_D756_11D1_A64E_00A0C93A70EC__INCLUDED_)
