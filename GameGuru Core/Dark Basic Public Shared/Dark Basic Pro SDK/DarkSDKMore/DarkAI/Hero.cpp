#include ".\..\..\Shared\DBOFormat\DBOData.h"

#include "Hero.h"
#include "Team.h"
#include "Zone.h"
#include "DBPro Functions.h"
#include "PathFinderAdvanced.h"
#include "Container.h"

Hero::Hero ( )
{
	bActive = false;
	pObject = 0;
	pNextHero = 0;
	fAngY = 0;

	fPosX = 0; fPosY = 0; fPosZ = 0;
	vecLastPos.x = fPosX; vecLastPos.y = fPosY; vecLastPos.z = fPosZ;

	fRadius = 2.5f;
	bIsDucking = false;
	pContainer = 0;
}

Hero::Hero ( int id, sObject *pObj )
{
	bActive = true;
	iID = id;
	pObject = pObj;
	pNextHero = 0;
	fAngY = 0;

	if ( !pObj )
	{
		fPosX = 0; fPosY = 0; fPosZ = 0;
	}
	else
	{
		fPosX = pObj->position.vecPosition.x;
		fPosY = pObj->position.vecPosition.y;
		fPosZ = pObj->position.vecPosition.z;
	}

	vecLastPos.x = fPosX; vecLastPos.y = fPosY; vecLastPos.z = fPosZ;

	fRadius = 2.5f;
	bIsDucking = false;
}

Hero::~Hero ( )
{

}

void Hero::SetRadius ( float fNewRadius )
{
	if ( fNewRadius < 0.0f ) fNewRadius = 0.0f;
	fRadius = fNewRadius;
}

float Hero::GetRadius ( )
{
	return fRadius;
}

void Hero::SetContainer ( Container *pNewContainer )
{
	/*
	if ( pContainer ) 
	{
		if ( pObject ) pContainer->pPathFinder->GridClearEntityPosition( pObject->position.vecPosition.x, pObject->position.vecPosition.z );
		else pContainer->pPathFinder->GridClearEntityPosition( fPosX, fPosZ );
	}
	*/

	if ( pNewContainer ) pContainer = pNewContainer;
}

Container* Hero::GetContainer ( )
{
	return pContainer;
}

bool Hero::Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide )
{
	float fVX = fEX - fSX;
	float fVZ = fEZ - fSZ;
	float fSqrDist = fVX*fVX + fVZ*fVZ;
	if ( fSqrDist < 0.00001f ) return false;

	float fV2X = GetX( ) - fSX;
	float fV2Z = GetZ( ) - fSZ;
	float fTValue = ( fVX*fV2X + fVZ*fV2Z ) / fSqrDist;
	float fNewSide = fVZ*fV2X - fVX*fV2Z;

	if ( fTValue < 0.0f ) fTValue = 0.0f;
	if ( fTValue > 1.0f ) fTValue = 1.0f;

	float fClosestX = fVX*fTValue + fSX;
	float fClosestZ = fVZ*fTValue + fSZ;

	fVX = fClosestX - GetX( );
	fVZ = fClosestZ - GetZ( );
	fSqrDist = fVX*fVX + fVZ*fVZ;

	if ( fSqrDist < fRadius*fRadius ) 
	{
		*iSide = fNewSide > 0 ? 1 : -1;
		return true;
	}

	return false;
}

int Hero::GetID ( )
{
	return iID;
}

float Hero::GetX ( )
{
	if ( pObject ) return pObject->position.vecPosition.x;
	else return fPosX;
}

float Hero::GetY ( )
{
	if ( pObject ) return pObject->position.vecPosition.y;
	else return fPosY;
}

float Hero::GetZ ( )
{
	if ( pObject ) return pObject->position.vecPosition.z;
	else return fPosZ;
}

float Hero::GetAngleY ( )
{
	if ( pObject ) return pObject->position.vecRotate.y;
	else return fAngY;
}

float Hero::GetHeight( )
{
	// LEEADD - 300813 - due to game objects using physics and object positions
	// now in the center of the object, adjusting player duck height values so that
	// ducking produces zero height and stood produces height value. This allows
	// me to set a smaller player height value to act as the difference between
	// stood and ducking (where raw unaltered plater obj position is duck position)
	//return bIsDucking ? fHeight/2.0f : fHeight;
	return bIsDucking ? 0.0f : fHeight;
}

bool Hero::GetIsDucking ( )
{
	return bIsDucking;
}

void Hero::SetDucking ( bool bDuck )
{
	bIsDucking = bDuck;
}

void Hero::SetPosition ( float x, float y, float z )
{
	if ( pObject )
	{
		if ( pContainer ) pContainer->pPathFinder->GridClearEntityPosition( pObject->position.vecPosition.x, pObject->position.vecPosition.z );

		pObject->position.vecPosition.x = x;
		pObject->position.vecPosition.y = y;
		pObject->position.vecPosition.z = z;
	}
	else
	{
		if ( pContainer ) pContainer->pPathFinder->GridClearEntityPosition( fPosX, fPosZ );

		fPosX = x;
		fPosY = y;
		fPosZ = z;
	}

	if ( pContainer ) pContainer->pPathFinder->GridSetEntityPosition( x, z, iID );
}

void Hero::SetAngleY( float angY )
{
	fAngY = angY;
	if ( pObject ) YRotateObject( iID, angY );
}

void Hero::SetHeight( float height )
{
	fHeight = height;
}

void Hero::SetTeam ( Team *pNewTeam )
{
	pTeam = pNewTeam;
}

void Hero::CheckZones ( Zone *pZone )
{
	while ( pZone )
	{
		bool bInNow = pZone->InZone ( GetX( ), GetZ( ), GetContainer( ) );
		bool bInLast = pZone->InZone ( vecLastPos.x, vecLastPos.z, GetContainer( ) );

		if ( !bInLast && bInNow ) pZone->NotifyEnter ( this );
		if ( bInLast && !bInNow ) pZone->NotifyLeave ( this );

		pZone = pZone->pNextZone;
	}
}

void Hero::Update ( float fTimeDelta )
{
	vecLastPos.x = GetX( );
	vecLastPos.y = GetY( );
	vecLastPos.z = GetZ( );
}