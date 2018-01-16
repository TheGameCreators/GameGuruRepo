#include "Zone.h"
#include "Entity.h"
#include "Container.h"

Zone::Zone ( )
{
	iID = 0;
	fMinx = 0;
	fMinz = 0;
	fMaxx = 0;
	fMaxz = 0;

	pContainer = 0;
	pMemberList = 0;
	pNextZone = 0;
}

Zone::Zone ( int id, float minx, float minz, float maxx, float maxz, Container *pInContainer )
{
	iID = id;
	fMinx = minx;
	fMinz = minz;
	fMaxx = maxx;
	fMaxz = maxz;

	pContainer = pInContainer;
	pMemberList = 0;
	pNextZone = 0;
}

Zone::~Zone ( )
{
	ZoneMember *pZoneMember = pMemberList;

	while ( pZoneMember )
	{
		pMemberList = pMemberList->pNextMember;
		delete pZoneMember;
		pZoneMember = pMemberList;
	}
}

int Zone::GetID ( )
{
	return iID;
}

Container* Zone::GetContainer ( )
{
	return pContainer;
}

void Zone::AddEntity ( Entity *pEntity )
{
	if ( !pEntity ) return;

	ZoneMember *pZoneMember = new ZoneMember ( );
	pZoneMember->pEntity = pEntity;
	pZoneMember->pNextMember = pMemberList;

	pMemberList = pZoneMember;
}

void Zone::RemoveEntity ( Entity *pEntity )
{
	if ( !pEntity ) return;

	ZoneMember *pZoneMember = pMemberList;
	if ( !pZoneMember ) return;

	while ( pZoneMember && pZoneMember->pEntity->GetID( ) == pEntity->GetID( ) )
	{
		pMemberList = pMemberList->pNextMember;
		delete pZoneMember;
		pZoneMember = pMemberList;
	}

	if ( !pZoneMember ) return;

	ZoneMember *pPrevMember;

	do
	{
		pPrevMember = pZoneMember;
		pZoneMember = pZoneMember->pNextMember;

		while ( pZoneMember && pZoneMember->pEntity->GetID( ) == pEntity->GetID( ) )
		{
			ZoneMember *pTemp = pZoneMember;

			pPrevMember->pNextMember = pZoneMember->pNextMember;
			pZoneMember = pZoneMember->pNextMember;
			delete pTemp;
		}
	} while ( pZoneMember );
}

bool Zone::InZone ( float x, float z, Container *pEntityContainer )
{
	if ( x < fMinx || x > fMaxx ) return false;
	if ( z < fMinz || z > fMaxz ) return false;
	if ( pContainer && pContainer != pEntityContainer ) return false;

	return true;
}

void Zone::NotifyEnter ( Entity *pEntityIn )
{
	ZoneMember *pZoneMember = pMemberList;

	while ( pZoneMember )
	{
		pZoneMember->pEntity->ZoneEnterEvent ( pEntityIn );
		pZoneMember = pZoneMember->pNextMember;
	}
}

void Zone::NotifyEnter ( Hero *pHeroIn )
{
	ZoneMember *pZoneMember = pMemberList;

	while ( pZoneMember )
	{
		pZoneMember->pEntity->ZoneEnterEvent ( pHeroIn );
		pZoneMember = pZoneMember->pNextMember;
	}
}

void Zone::NotifyLeave ( Entity *pEntityOut )
{
	ZoneMember *pZoneMember = pMemberList;

	while ( pZoneMember )
	{
		pZoneMember->pEntity->ZoneLeaveEvent ( pEntityOut );
		pZoneMember = pZoneMember->pNextMember;
	}
}

void Zone::NotifyLeave ( Hero *pHeroOut )
{
	ZoneMember *pZoneMember = pMemberList;

	while ( pZoneMember )
	{
		pZoneMember->pEntity->ZoneLeaveEvent ( pHeroOut );
		pZoneMember = pZoneMember->pNextMember;
	}
}