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

#ifndef __GLOBAL_DATA_H
#define __GLOBAL_DATA_H

#include "bcgcontrolbar.h"

/////////////////////////////////////////////////////////////////////////////
// Auxiliary System/Screen metrics

struct BCGCONTROLBARDLLEXPORT GLOBAL_DATA
{
	// solid brushes with convenient gray colors and system colors
	HBRUSH hbrBtnHilite, hbrBtnShadow;

	// color values of system colors used for CToolBar
	COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF clrBtnText, clrWindowFrame;
	COLORREF clrBtnDkShadow, clrBtnLight;
	COLORREF clrGrayedText;
	COLORREF clrHilite;
	COLORREF clrTextHilite;
	COLORREF clrHotText;

	CBrush	brBtnFace;
	CBrush	brHilite;
	CBrush	brLight;
	CBrush	brBlack;

	HCURSOR	m_hcurStretch;
	HCURSOR	m_hcurStretchVert;
	HCURSOR	m_hcurHand;

	HICON	m_hiconTool;

	// Toolbar and menu fonts:
	CFont	fontRegular;
	CFont	fontTooltip;
	CFont	fontBold;
	CFont	fontUnderline;
	CFont	fontVert;
	CFont	fontVertCaption;

	CFont	fontMarlett;	// Standard Windows menu symbols

	BOOL	bIsWindowsNT4;
	int		m_nBitsPerPixel;

// Implementation
	GLOBAL_DATA();
	~GLOBAL_DATA();

	void UpdateSysColors();
	void UpdateFonts();

	BOOL SetMenuFont (LPLOGFONT lpLogFont, BOOL bHorz);

	int GetTextHeight (BOOL bHorz = TRUE)
	{
		return bHorz ? m_nTextHeightHorz : m_nTextHeightVert;
	}

	void CleanUp ();

protected:

	void UpdateTextMetrics ();
	HBITMAP CreateDitherBitmap (HDC hDC);
	
	int	m_nTextHeightHorz;
	int	m_nTextHeightVert;
};

extern BCGCONTROLBARDLLEXPORT GLOBAL_DATA globalData;

/////////////////////////////////////////////////////////////////////////////

#endif // __GLOBAL_DATA_H
