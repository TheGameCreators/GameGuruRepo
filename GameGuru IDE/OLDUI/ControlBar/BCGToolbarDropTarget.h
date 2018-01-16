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

#if !defined(AFX_BCGTOOLBARDROPTARGET_H__E0D2D9D6_C494_11D1_A646_00A0C93A70EC__INCLUDED_)
#define AFX_BCGTOOLBARDROPTARGET_H__E0D2D9D6_C494_11D1_A646_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGToolbarDropTarget.h : header file
//

#ifndef BCG_NO_CUSTOMIZATION

#ifndef __AFXOLE_H__
#include <afxole.h>
#endif

class CBCGToolBar;

/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarDropTarget command target

class CBCGToolbarDropTarget : public COleDropTarget
{
public:
	CBCGToolbarDropTarget();
	virtual ~CBCGToolbarDropTarget();

// Attributes
public:
protected:
	CBCGToolBar*	m_pOwner;

// Operations
public:
	BOOL Register (CBCGToolBar *pOwner);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGToolbarDropTarget)
	public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropList, CPoint point);
	//}}AFX_VIRTUAL

// Implementation

	// Generated message map functions
	//{{AFX_MSG(CBCGToolbarDropTarget)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // BCG_NO_CUSTOMIZATION

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGTOOLBARDROPTARGET_H__E0D2D9D6_C494_11D1_A646_00A0C93A70EC__INCLUDED_)
