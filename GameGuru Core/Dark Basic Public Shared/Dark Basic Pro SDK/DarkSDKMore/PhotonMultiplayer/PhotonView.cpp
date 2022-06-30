#include "stdafx.h"
#include "PhotonView.h"

PhotonView::PhotonView ( void )
{
	// reset
	sRoomList.clear();
	sPlayerList.clear();

	// global vars
	memset ( &GlobalStates, 0, sizeof(GlobalStates) );
}

PhotonView::~PhotonView ( void )
{
	// free any resources used
	ClearRoomList();
	ClearPlayerList();
}

// Room Handling

void PhotonView::ClearRoomList ( void ) 
{
	for ( int i=0; i < (int)sRoomList.size(); i++ )
	{
		LPSTR pStr = sRoomList[i];
		if ( pStr ) delete pStr;
	}
	sRoomList.clear(); 
}
void PhotonView::AddRoomToList ( LPSTR pRoomName )
{ 
	LPSTR pNewRoomNameString = new char[1024];
	strcpy ( pNewRoomNameString, pRoomName );
	sRoomList.push_back ( pNewRoomNameString ); 
}
int PhotonView::GetRoomCount ( void )
{ 
	return sRoomList.size(); 
}
LPSTR PhotonView::GetRoomName ( int iRoomIndex ) 
{ 
	return sRoomList[iRoomIndex]; 
}

// Player Handling

void PhotonView::ClearPlayerList ( void ) 
{
	for ( int i=0; i < (int)sPlayerList.size(); i++ )
	{
		LPSTR pStr = sPlayerList[i];
		if ( pStr ) delete pStr;
	}
	sPlayerList.clear(); 
}
void PhotonView::AddPlayerToList ( LPSTR pPlayerName )
{ 
	LPSTR pNewPlayerNameString = new char[1024];
	strcpy ( pNewPlayerNameString, pPlayerName );
	sPlayerList.push_back ( pNewPlayerNameString ); 
}
int PhotonView::GetPlayerCount ( void )
{ 
	return sPlayerList.size(); 
}
LPSTR PhotonView::GetPlayerName ( int iPlayerIndex ) 
{ 
	return sPlayerList[iPlayerIndex]; 
}
