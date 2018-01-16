// BCGMultiDocTemplate.h: interface for the CBCGMultiDocTemplate class.
//
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGMULTIDOCTEMPLATE_H__916B7A46_556A_11D2_B110_FD3F2C81B152__INCLUDED_)
#define AFX_BCGMULTIDOCTEMPLATE_H__916B7A46_556A_11D2_B110_FD3F2C81B152__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//--------------------------------------------------------
// The class CBCGMultiDocTemplate is created for access
// to protected member 'uiMenuResId' of CMultiDocTemplate!
//--------------------------------------------------------

#pragma warning (disable : 4510 4610)

class CBCGMultiDocTemplate : public CMultiDocTemplate  
{
public:
	UINT GetResId () const
	{
		return m_nIDResource;
	}
};

#pragma warning (default : 4510 4610)

#endif // !defined(AFX_BCGMULTIDOCTEMPLATE_H__916B7A46_556A_11D2_B110_FD3F2C81B152__INCLUDED_)
