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

// MenuImages.h: interface for the CMenuImages class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUIMAGES_H__DE028D41_36EB_11D3_95C5_00A0C9289F1B__INCLUDED_)
#define AFX_MENUIMAGES_H__DE028D41_36EB_11D3_95C5_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcontrolbar.h"
#include "BCGToolBarImages.h"

class BCGCONTROLBARDLLEXPORT CMenuImages  
{
public:
	enum IMAGES_IDS
	{
		IdArowDown,
		IdArowDownDsbl,
		IdArowLeft,
		IdArowLeftDsbl,
		IdArowLeftWhite,
		IdArowLeftWhiteDsbl,
		IdCheck,
		IdCheckDsbl,
		IdMinimize,
		IdMinimizeDsbl,
		IdRestore,
		IdRestoreDsbl,
		IdClose,
		IdCloseDsbl,
		IdMaximize,
		IdMaximizeDsbl,
		IdArowUp,
		IdArowUpDisabled,
		IdArowShowAll,
		IdArowRight,
		IdCloseSmall,
		IdMoreButtons,
		IdRadio,
		IdRadioDsbl,
		IdArowDownLarge,
		IdArowDownLargeDsbl,
		IdArowLeftLarge,
		IdArowLeftLargeDsbl,
		IdArowDownWhite,
		IdMinimizeWhite,
		IdRestoreWhite,
		IdCloseWhite,
		IdArowFirst,
		IdArowLast,
		IdArowRightLarge,
	};

	static void Draw (CDC* pDC, CMenuImages::IMAGES_IDS id, const CPoint& ptImage,
					  const CSize& sizeImage = CSize (0, 0));
	static CSize Size()
	{
		Initialize ();
		return m_Images.GetImageSize ();
	}

	static void CleanUp ();

protected:
	static BOOL Initialize ();
	static CBCGToolBarImages	m_Images;
};

#endif // !defined(AFX_MENUIMAGES_H__DE028D41_36EB_11D3_95C5_00A0C9289F1B__INCLUDED_)
