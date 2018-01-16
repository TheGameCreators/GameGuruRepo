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

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EBD69F5A_C937_11D1_A647_00A0C93A70EC__INCLUDED_)
#define AFX_STDAFX_H__EBD69F5A_C937_11D1_A647_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

/* X10 redefined so remove
#if _MSC_VER >= 1300
	#define _WIN32_WINNT	0x0400
#else
	#define WINVER			0x0400
#endif
*/

// Uncomment the following definitions to reduce the library size:

//#define BCG_NO_REBAR			// No IE-style rebars support
//#define BCG_NO_OUTLOOKBAR		// No shortcut bars support
//#define BCG_NO_SIZINGBAR		// No CBCGSizingControlBar bars support
//#define BCG_NO_CUSTOMIZATION	// No customization stuff
//#define BCG_NO_COLOR			// No color controls support
//#define BCG_NO_TABCTRL		// No tab control support
//#define BCG_NO_DIALOG			// No dialog box support
//#define BCG_NO_ANIMCONTROL	// No animation control support

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#include <AfxAdv.h>
#endif // _AFX_NO_OLE_SUPPORT

#include <afxtempl.h>
#include <afxpriv.h>

#if _MSC_VER >= 1300
	#include <..\atlmfc\src\mfc\afximpl.h>
#else
	#include <..\src\afximpl.h>
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#pragma warning (disable : 4201)
	#include "mmsystem.h"
	#pragma comment(lib,"winmm.lib") 
	#pragma message("Automatically linking with winmm.lib")
#pragma warning (default : 4201)

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EBD69F5A_C937_11D1_A647_00A0C93A70EC__INCLUDED_)
