// ProgressSpinner.cpp : implementation file
//

#include "stdafx.h"
#include "Editor.h"
#include "ProgressSpinner.h"

#include <math.h>


// CProgressSpinner

IMPLEMENT_DYNAMIC(CProgressSpinner2, CStatic)

CProgressSpinner2::CProgressSpinner2()
{
	m_bSpinning = false;
	m_iAngle = true;
}

CProgressSpinner2::~CProgressSpinner2()
{
}


BEGIN_MESSAGE_MAP(CProgressSpinner2, CStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()


void CProgressSpinner2::StartSpinning( )
{
	m_bSpinning = true;
	SetTimer( 1, 5, NULL );
}

void CProgressSpinner2::StopSpinning( )
{
	m_bSpinning = false;
	KillTimer( 1 );
}


// CProgressSpinner message handlers


void CProgressSpinner2::OnPaint()
{
	CPaintDC dc( this );
	CRect clientRect;
	GetClientRect( &clientRect );

	int diff = clientRect.Width() - clientRect.Height();

	if ( diff < 0 )
	{
		clientRect.top += diff/2;
		clientRect.bottom -= diff/2;
	}
	else
	{
		clientRect.left += diff/2;
		clientRect.right -= diff/2;
	}

	dc.SetBkMode( TRANSPARENT );
	dc.SelectStockObject( BLACK_PEN );
	dc.SelectStockObject( LTGRAY_BRUSH );

	dc.Ellipse( &clientRect );

	if ( !m_bSpinning ) return;

	int iLength = clientRect.Width() / 2;
	int iStartX = clientRect.left + clientRect.Width() / 2;
	int iStartY = clientRect.top + clientRect.Height() / 2;

	int iEndX = (int) ( iStartX + sin( (float)m_iAngle * 0.017453292f ) * iLength );
	int iEndY = (int) ( iStartY + cos( (float)m_iAngle * 0.017453292f ) * iLength );

	dc.MoveTo( iStartX, iStartY );
	dc.LineTo( iEndX, iEndY );
}

BOOL CProgressSpinner2::OnEraseBkgnd( CDC *pDC )
{
	return TRUE;
}

void CProgressSpinner2::OnTimer( UINT_PTR nIDEvent )
{
	if ( nIDEvent == 1 )
	{
		if ( !m_bSpinning ) KillTimer( 1 );

		m_iAngle = ( m_iAngle - 5 ) % 360;

		Invalidate( );
	}
}