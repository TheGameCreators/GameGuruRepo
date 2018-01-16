// StoreButton.cpp : implementation file
//

#include "stdafx.h"
#include "StoreButton.h"
//#include "MarketWindow.h"
#include "Editor.h"


// CStoreButton

IMPLEMENT_DYNAMIC(CStoreButton, CButton)

CStoreButton::CStoreButton()
{
	m_bLButtonDown = false;
}

CStoreButton::~CStoreButton()
{
}


BEGIN_MESSAGE_MAP(CStoreButton, CButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CStoreButton message handlers

void CStoreButton::OnLButtonDown( UINT nFlags, CPoint point )
{
	CButton::OnLButtonDown( nFlags, point );
	m_bLButtonDown = true;
	SetCapture( );
}

void CStoreButton::OnLButtonUp( UINT nFlags, CPoint point )
{
	ReleaseCapture( );
	if ( m_bLButtonDown )
	{
		CRect rect;
		GetClientRect( rect );
		if ( rect.PtInRect( point ) )
		{
			DWORD dwState = 0;
			BOOL bOnline = InternetGetConnectedState( &dwState, 0 );
			if ( !bOnline )
			{
				MessageBox( "Could not detect an internet connection", theApp.m_szErrorString );
			}
			else
			{
				// lee - 130709 - switched to DLL store
				if ( g_bDisableStoreMode==false )
				{
					// Store Launch
					theApp.OpenStore();
				}
				else
				{
					// Portal Launch - Upload My Game
					theApp.OpenUploadGame();
				}
			}
		}
	}
	m_bLButtonDown = false;
}
