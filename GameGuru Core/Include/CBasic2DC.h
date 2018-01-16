#ifndef _CBASIC2D_H_
#define _CBASIC2D_H_

#include "directx-macros.h"

#define DARKSDK 

struct tagLinesVertexDesc
{
	float x, y, z, rhw;
	DWORD dwColour;
};

DARKSDK void	Basic2DConstructor		( void );
DARKSDK void	Basic2DDestructor		( void );
DARKSDK void	SetBasic2DErrorHandler	( LPVOID pErrorHandlerPtr );
DARKSDK void	PassBasic2DCoreData		( LPVOID pGlobPtr );
DARKSDK void	Basic2DRefreshGRAFIX		( int iMode );
DARKSDK void	UpdateBPP				( void );

DARKSDK void	CLS						( void );
DARKSDK void	CLS						( int iRed, int iGreen, int iBlue );
DARKSDK void	Ink						( int iRedF, int iGreenF, int iBlueF, int iRedB, int iGreenB, int iBlueB );
DARKSDK bool	GetLockable				( void );
DARKSDK void	CLS						( DWORD RGBBackColor );
DARKSDK void	Ink						( DWORD RGBForeColor, DWORD RGBBackColor );
DARKSDK void	Dot						( int iX, int iY );
DARKSDK void	Box						( int iLeft, int iTop, int iRight, int iBottom );
DARKSDK void	Line					( int iXa, int iYa, int iXb, int iYb );
DARKSDK void	Circle					( int iX, int iY, int iRadius );
DARKSDK void	Elipse					( int iX, int iY, int iXRadius, int iYRadius );
DARKSDK DWORD	Rgb						( int iRed, int iGreen, int iBlue );
DARKSDK int		RgbR					( DWORD iRGB );
DARKSDK int		RgbG					( DWORD iRGB );
DARKSDK int		RgbB					( DWORD iRGB );
DARKSDK DWORD	GetPoint				( int iX, int iY );
DARKSDK void	CopyArea				( int iDestX, int iDestY, int iWidth, int iHeight, int iSourceX, int iSourceY );
DARKSDK void	Dot						( int iX, int iY, DWORD dwColor );
//DARKSDK void	BoxGradient				( int iLeft, int iTop, int iRight, int iBottom, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4 );
DARKSDK void	LockPixels				( void );
DARKSDK void	UnlockPixels			( void );
DARKSDK DWORD	GetPixelPtr				( void );
DARKSDK DWORD	GetPixelPitch			( void );

#endif
