// SampleList.cpp : implementation file
//

#include "stdafx.h"
#include "SampleList.h"
#include "resource.h"
#include "Editor.h"
#include "StringFunc.h"
#include <atlimage.h>

// Externals
extern bool g_bX9InterfaceMode;

// CSampleList

IMPLEMENT_DYNAMIC(CSampleList, CListCtrl)

CSampleList::CSampleList()
{

}

CSampleList::~CSampleList()
{
}

void CSampleList::Init( )
{
	this->DeleteAllItems ( );

	m_Images.DeleteImageList ( );
	m_Images.Create     ( 64, 64, ILC_COLOR32 | ILC_MASK, 1, 16 );
	//m_Images.SetBkColor ( RGB ( 255, 255, 255 ) );

	this->SetImageList   ( &m_Images, LVSIL_NORMAL );
	this->SetItemCount   ( 64 );

	CBitmap bmp;
	bmp.LoadBitmap( IDB_SAMPLE_IMAGE );
	m_Images.Add( &bmp, (CBitmap*)NULL );

	//InsertItem ( 0, "Sample Item", 0 );
	LoadSampleFiles( );
}

void CSampleList::LoadSampleFiles( )
{
}

bool CSampleList::GetFilenameSelected( int iBufferSize, TCHAR *szFilename )
{
	if ( m_iItemClicked < 0 )
	{
		MessageBox( GetLanguageData( _T("Getting Started"), _T("NoSelect2") ), theApp.m_szErrorString );
		return false;
	}

	TCHAR szNoSamples [ 64 ];
	GetPrivateProfileString( _T("Getting Started"), _T("NoSample"), _T(""), szNoSamples, 64, theApp.m_szLanguage );

	CString sName = GetItemText( m_iItemClicked, 0 );
	if ( sName.Compare( szNoSamples ) == 0 )
	{
		MessageBox( GetLanguageData( _T("Getting Started"), _T("InvalidSample") ), theApp.m_szErrorString );
		return false;
	}

	_tcscpy_s( szFilename, iBufferSize, sName );

	return true;
}

BEGIN_MESSAGE_MAP(CSampleList, CListCtrl)
END_MESSAGE_MAP()



// CSampleList message handlers

void CSampleList::OnClickList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	// retrieve message info.
	LPNMITEMACTIVATE pItemAct = ( LPNMITEMACTIVATE ) pNMHDR;

	// determine which item receives the click
	LVHITTESTINFO  hitTest;

	// determine item clicked on
	memset ( &hitTest, '\0', sizeof ( LVHITTESTINFO ) );
	hitTest.pt = pItemAct->ptAction;
	this->SendMessage ( LVM_SUBITEMHITTEST, 0, ( LPARAM ) &hitTest );

	m_iItemClicked = hitTest.iItem;
}
