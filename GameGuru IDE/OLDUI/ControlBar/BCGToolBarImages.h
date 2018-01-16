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

// BCGToolBarImages.h: interface for the CBCGToolBarImages class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_)
#define AFX_BCGTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

// for custom drawing
typedef struct tagDrawState
{
	HBITMAP hbmMono;
	HBITMAP hbmMonoOld;
	HBITMAP hbmOldGlyphs;
}
CBCGDrawState;

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CBCGToolBarImages : public CObject
{
	friend class CBCGDropDownFrame;

public:
	CBCGToolBarImages();
	virtual ~CBCGToolBarImages();

// Operations:
public:
	BOOL IsValid () const
	{
		return (m_hbmImageWell != NULL);
	}

	BOOL IsReadOnly () const
	{
		return m_bReadOnly;
	}

	HBITMAP GetImageWell () const
	{
		return m_hbmImageWell;
	}

	BOOL IsUserImagesList () const
	{
		return m_bUserImagesList;
	}

	void SetImageSize (SIZE sizeImage)
	{
		m_sizeImage = sizeImage;
	}

	SIZE GetImageSize (BOOL bDest = FALSE) const
	{
		return bDest ? m_sizeImageDest : m_sizeImage;
	}

	int GetCount () const
	{
		return m_iCount;
	}

	int GetResourceOffset (UINT uiResId) const
	{
		int iOffset = -1;
		if (m_mapOrigResOffsets.Lookup (uiResId, iOffset))
		{
			return iOffset;
		}

		return -1;
	}

	COLORREF SetTransparentColor (COLORREF clrTransparent)
	{
		COLORREF clrOld = m_clrTransparent;

		if (clrTransparent != clrOld)
		{
			m_clrTransparent = clrTransparent;
			UpdateLightImage ();
		}

		return clrOld;
	}

	COLORREF GetTransparentColor () const
	{
		return m_clrTransparent;
	}

	BOOL Load (UINT uiResID, HINSTANCE hinstRes = NULL, BOOL bAdd = FALSE);
	BOOL Load (LPCTSTR lpszBmpFileName);
	BOOL Save (LPCTSTR lpszBmpFileName = NULL);

	void Clear ();

	BOOL PrepareDrawImage (CBCGDrawState& ds,
						CSize sizeImageDest = CSize (0, 0),
						BOOL bFadeInactive = FALSE);

	BOOL Draw (CDC* pDC, 
				int x, int y,
				int iImageIndex,
				BOOL bHilite = FALSE, 
				BOOL bDisabled = FALSE,
				BOOL bIndeterminate = FALSE,
				BOOL bShadow = FALSE,
				BOOL bInactive = FALSE);
	void EndDrawImage (CBCGDrawState& ds);

	int AddImage (HBITMAP hbmp);
	BOOL UpdateImage (int iImage, HBITMAP hbmp);
	BOOL DeleteImage (int iImage);

	BOOL CopyImageToClipboard (int iImage);

	void OnSysColorChange();

	BOOL MapTo3dColors (BOOL bUseRGBQUAD = TRUE,
						COLORREF clrSrc = (COLORREF)-1,
						COLORREF clrDest = (COLORREF)-1);
	static COLORREF MapToSysColor (COLORREF color, BOOL bUseRGBQUAD = TRUE);
	static COLORREF MapFromSysColor (COLORREF color, BOOL bUseRGBQUAD = TRUE);
	static void FillDitheredRect (CDC* pDC, const CRect& rect);

protected:
	static void TransparentBlt (HDC hdcDest, int nXDest, int nYDest, int nWidth, 
								int nHeight, CDC* pDcSrc, int nXSrc, int nYSrc,
								COLORREF colorTransparent,
								int nWidthDest = -1, int nHeightDest = -1);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void CreateMask(int iImage,
				BOOL bHilite, 
				BOOL bHiliteShadow);
	static HBITMAP CreateDitherBitmap();
	void UpdateCount ();
	BOOL UpdateLightImage ();

	void CopyTemp (CBCGToolBarImages& imagesDest);

// Attributes:
public:

protected:
	CSize				m_sizeImage;        // size of glyph
	CSize				m_sizeImageDest;	// destination size glyph
	HBITMAP				m_hbmImageWell;		// glyphs only
	HBITMAP				m_hbmImageLight;	// "faded" version (hi-color only)
	HINSTANCE			m_hInstImageWell;	// instance handle to load image well from
	BOOL				m_bUserImagesList;	// is user-defined images list?
	CString				m_strUDLPath;		// user-defined images path
	BOOL				m_bModified;		// is image modified?
	int					m_iCount;			// image counter
	CList<UINT, UINT>	m_lstOrigResIds;	// original resource ids
	CMap<UINT, UINT, int, int> m_mapOrigResOffsets;// original resource offsets
	BOOL				m_bStretch;			// stretch images
	CDC					m_dcMem;			// DC for the drawing in memory
	CBitmap				m_bmpMem;			// bitmap for the drawing in memory
	CBitmap*			m_pBmpOriginal;
	COLORREF			m_clrTransparent;	// Transparent color
	BOOL				m_bReadOnly;		// Loaded from read-only file
	BOOL				m_bIsTemporary;		// Temporary copy of another image
	COLORREF			m_clrImageShadow;	// Color of the shadow
	BOOL				m_bFadeInactive;	// Inactive image will be drawn with the "fading" effect
	int					m_nBitsPerPixel;	// Bitmap color depth
};

#endif // !defined(AFX_BCGTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_)
