#define WIN32_LEAN_AND_MEAN					// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdio.h>
#include <vector>
#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include ".\..\..\..\Include\DarkAI.h"

#include "globstruct.h"
extern GlobStruct *g_pGlob;

#include "Entity.h"
#include "World.h"
#include "Hero.h"
#include "Beacon.h"
#include "StateMachine\StateSet.h"
#include "TeamController.h"
#include "Zone.h"
#include "DBPro Functions.h"
#include <cctype>
#include <stdlib.h>
#include <time.h>

#include "CObjectsC.h"
#include "CMemblocks.h"

extern std::vector <int> ListOfEntitiesToUpdateMovement;
vector <tempCoverPoint> tempCoverPoints;
vector <float> coverInUseX;
vector <float> coverInUseZ;
vector <Entity*> coverInUseEntity;

int g_GUIShaderEffectID = 0;

//using namespace std;

bool bShowErrors = false;
bool bAIInitialised = false;
int iThreadCount = 4;
char errAIStr [ 256 ];
int iThreadMode = 0;
World cWorld;

// associated with AI Get Last Obstacle Center X()
float g_fShapeMidX = -1.0f;
float g_fShapeMidZ = -1.0f;
float g_fShapeRadius = -1.0f;

//DarkAI main funcitons
void AIStart ( int iGUIShaderEffectIndex )
{
	if ( bAIInitialised ) return;

	g_GUIShaderEffectID = iGUIShaderEffectIndex;
	
	srand ( (unsigned int) time (NULL) );
	AIReset ( 0 );

	bAIInitialised = true;
}

void AIReset ( int iStopTheadLoop ) 
{ 
	if ( iStopTheadLoop == 1 )
	{
		// signal to stop thead
		cWorld.StopWorld();
	}

	coverInUseX.clear();
	coverInUseZ.clear();
	coverInUseEntity.clear();
	tempCoverPoints.clear();
	cWorld.Reset ( );
	ListOfEntitiesToUpdateMovement.clear();
}

void AISetAvoidMode ( int iNewMode )
{
	if ( iNewMode < 0 ) iNewMode = 0;

	Entity::iAvoidMode = iNewMode;
}

inline Entity* CheckEntity ( int iObjID )
{
	Entity* pEntity = cWorld.GetEntityCopy ( iObjID );
	
	//Dave set bShowErrors to false and commented this out as everywhere that uses this function checks if it is null and returns out anyway
	/*if ( !pEntity && bShowErrors ) 
	{
		sprintf_s ( errAIStr, 255, "AI Entity (%d) Does Not Exist", iObjID );
		MessageBox ( NULL, errAIStr, "AI Error", 0 );
		//exit ( 1 );
	}*/

	return pEntity;
}

inline Container* CheckContainer ( int iContainerID )
{
	Container* pContainer = cWorld.GetContainer ( iContainerID );
	
	if ( !pContainer && bShowErrors ) 
	{
		sprintf_s ( errAIStr, 255, "AI Container (%d) Does Not Exist", iContainerID );
		MessageBox ( NULL, errAIStr, "AI Error", 0 );
		//exit ( 1 );
	}

	return pContainer;
}

inline bool CheckAIInit ( )
{
	if ( !bAIInitialised )
	{
		//MessageBox ( NULL, "AI System Not Initialised, Call 'AI Start' First", "AI Error", 0 );
		RunTimeError ( 0, "AI System Not Initialised, Call 'AI Start' First" );
		////exit ( 1 );
		return 0;
	}
	else
		return 1;
}

float AILastObstacleCenterX ( )
{
	float fReturn = g_fShapeMidX;
	g_fShapeMidX=-1;
	return fReturn;
}

float AILastObstacleCenterZ ( )
{
	float fReturn = g_fShapeMidZ;
	g_fShapeMidZ=-1;
	return fReturn;
}

float AILastObstacleRadius ( )
{
	float fReturn = g_fShapeRadius;
	g_fShapeRadius=-1;
	return fReturn;
}

void AIAddStaticObstacle ( int iObjID, int iHeight, int iContainerID ) 
{ 
	if ( CheckAIInit ( )==0 ) return;
	
	if ( ObjectExist( iObjID ) == 0 ) 
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Static Object (%d) Does Not Exist", iObjID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;
	
	sObject *pObject = GetObjectData ( iObjID );
	
	cWorld.AddStatic ( iObjID, iContainerID, pObject, iHeight, 0 );
}

void AIAddStaticObstacle ( int iObjID, int iHeight ) 
{ 
	AIAddStaticObstacle ( iObjID, iHeight, 0 );
}

void AIAddStaticObstacle ( int iObjID ) 
{ 
	AIAddStaticObstacle ( iObjID, 1, 0 );
}

void AIAddViewBlockingObstacle ( int iObjID, int iHeight, int iContainerID ) 
{ 
	if ( CheckAIInit ( )==0 ) return;
	
	if ( ObjectExist( iObjID ) == 0 ) 
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Object (%d) Does Not Exist", iObjID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;
	
	sObject *pObject = GetObjectData ( iObjID );
	
	cWorld.AddStatic ( iObjID, iContainerID, pObject, iHeight, 1 );
}

void AIAddViewBlockingObstacle ( int iObjID, int iHeight ) 
{ 
	AIAddViewBlockingObstacle ( iObjID, iHeight, 0 );
}

void AIAddViewBlockingObstacle ( int iObjID ) 
{ 
	AIAddViewBlockingObstacle ( iObjID, 1, 0 );
}

void AIRemoveObstacle ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.RemoveStatic ( iObjID );
}

void AIAddObstacleFromLevel ( int iObjID, int iContainerID, int iHeight, float fPlaneHeight, float fMinLength, float fMinHeight, int iOutputToFile )
{
	if ( CheckAIInit ( )==0 ) return;

	sObject *pObject = GetObjectData( iObjID );
	if ( !pObject ) return;

	cWorld.AddObstacleFromObject ( pObject, iContainerID, iHeight, fPlaneHeight, fMinLength, fMinHeight, ( iOutputToFile != 0 ) );
}

void AIAddObstacleFromLevel ( int iObjID, float fPlaneHeight, float fMinLength, float fMinHeight )
{
	AIAddObstacleFromLevel( iObjID, 0, 1, fPlaneHeight, fMinLength, fMinHeight, 0 );
}

void AIAddObstacleFromLevel ( int iObjID, int iContainerID, float fPlaneHeight, float fMinLength, float fMinHeight )
{
	AIAddObstacleFromLevel( iObjID, iContainerID, 1, fPlaneHeight, fMinLength, fMinHeight, 0 );
}

void AIAddObstacleFromLevel ( int iObjID, int iContainerID, int iHeight, float fPlaneHeight, float fMinLength, float fMinHeight )
{
	AIAddObstacleFromLevel( iObjID, iContainerID, iHeight, fPlaneHeight, fMinLength, fMinHeight, 0 );
}

//type: 0 = no obstruction, otherwise polygon
void AIAddAlternateVisibilityObject( int iObjID, int type )
{
	if ( CheckAIInit ( )==0 ) return;

	sObject *pObject = GetObjectData( iObjID );
	if ( !pObject ) return;

	cWorld.AddAlternateVisibilityObject( iObjID, pObject, type );
}

void AIRemoveAlternateVisibilityObject( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.RemoveAlternateVisibilityObject( iObjID );
}

void AICompleteObstacles ( )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.CompleteObstacles( -1 );
}

void AICompleteObstacles ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.CompleteObstacles( iContainerID );
}

void AIAddPlayer ( int iObjID, int iUseObject ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	if ( cWorld.pTeamController->GetHero ( iObjID ) )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "AI Player (%d) Already Exists", iObjID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
	
	sObject *pObject = 0;
	
	if ( iUseObject > 0 )
	{
		if ( ObjectExist( iObjID ) == 0 ) 
		{
			if ( bShowErrors )
			{
				sprintf_s ( errAIStr, 255, "Player Object (%d) Does Not Exist", iObjID );
				MessageBox ( NULL, errAIStr, "AI Error", 0 );
				//exit ( 1 );
			}

			return;
		}

		pObject = GetObjectData ( iObjID );
	}

	cWorld.AddHero ( iObjID, 0, pObject, 1 );
}

void AIAddPlayer ( int iObjID ) 
{
	AIAddPlayer ( iObjID, 1 );
}

void AIAddFriendly ( int iObjID, int iUseObject, int iContainerID ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	if ( cWorld.pTeamController->GetEntity ( iObjID ) )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Friendly (%d) Already Exists", iObjID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
	
	sObject *pObject = 0;
	
	if ( iUseObject > 0 )
	{
		if ( ObjectExist( iObjID ) == 0 )  
		{
			if ( bShowErrors )
			{
				sprintf_s ( errAIStr, 255, "Friendly Object (%d) Does Not Exist", iObjID );
				MessageBox ( NULL, errAIStr, "AI Error", 0 );
				//exit ( 1 );
			}

			return;
		}

		pObject = GetObjectData ( iObjID );
	}

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	cWorld.AddEntity ( iObjID, iContainerID, pObject, GetObjectData ( iObjID ), 1 );
}

void AIAddFriendly ( int iObjID, int iUseObject ) 
{
	AIAddFriendly ( iObjID, iUseObject, 0 );
}

void AIAddFriendly ( int iObjID ) 
{
	AIAddFriendly ( iObjID, 1, 0 );
}

void AIAddNeutral ( int iObjID, int iUseObject, int iContainerID ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	if ( cWorld.pTeamController->GetEntity ( iObjID ) )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Neutral (%d) Already Exists", iObjID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
	
	sObject *pObject = 0;
	
	if ( iUseObject > 0 )
	{
		if ( !ObjectExist( iObjID ) ) 
		{
			if ( bShowErrors )
			{
				sprintf_s ( errAIStr, 255, "Neutral Object (%d) Does Not Exist", iObjID );
				MessageBox ( NULL, errAIStr, "AI Error", 0 );
				//exit ( 1 );
			}

			return;
		}

		pObject = GetObjectData ( iObjID );
	}

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	cWorld.AddEntity ( iObjID, iContainerID, pObject, GetObjectData ( iObjID ), 0 );
}

void AIAddNeutral ( int iObjID, int iUseObject ) 
{
	AIAddNeutral ( iObjID, iUseObject, 0 );
}

void AIAddNeutral ( int iObjID ) 
{
	AIAddNeutral ( iObjID, 1, 0 );
}

void AIAddEnemy ( int iObjID, int iUseObject, int iContainerID ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	if ( cWorld.pTeamController->GetEntity ( iObjID ) )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Enemy (%d) Already Exists", iObjID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	sObject *pObject = 0;

	if ( iUseObject > 0 )
	{
		if ( !ObjectExist( iObjID ) ) 
		{
			if ( bShowErrors )
			{
				sprintf_s ( errAIStr, 255, "Enemy Object (%d) Does Not Exist", iObjID );
				MessageBox ( NULL, errAIStr, "AI Error", 0 );
				//exit ( 1 );
			}

			return;
		}

		pObject = GetObjectData ( iObjID );
	}

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	cWorld.AddEntity ( iObjID, iContainerID, pObject, GetObjectData ( iObjID ), 2 );
}

void AIAddEnemy ( int iObjID, int iUseObject ) 
{
	AIAddEnemy ( iObjID, iUseObject, 0 );
}

void AIAddEnemy ( int iObjID ) 
{
	AIAddEnemy ( iObjID, 1, 0 );
}

void AIStartNewObstacle ( int id )
{
	if ( CheckAIInit ( )==0 ) return;
	if ( cWorld.pManualPolygon )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Cannot Start New Obstacle, Previous Has Not Been Finished" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
		}
		return;
	}
	cWorld.StartNewPolygon ( id );
}

void AIStartNewObstacle ( )
{
	AIStartNewObstacle ( 0 );
}

void AIAddObstacleVertex ( float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	if ( !cWorld.pManualPolygon )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Cannot Add Vertex, New Obstacle Has Not Been Started" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			////exit ( 1 );
		}

		return;
	}

	cWorld.AddVertex ( x, z );
}

void AIEndNewObstacle ( int iContainerID, int iHeight, int iObstacleType )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	
	if ( !cWorld.pManualPolygon )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Cannot End New Obstacle, New Obstacle Has Not Been Started" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	cWorld.EndNewPolygon ( iContainerID, iHeight, iObstacleType );
}

void AIEndNewObstacle ( int iContainerID, int iHeight )
{
	AIEndNewObstacle ( iContainerID, iHeight, 0 );
}

void AIDiscardNewObstacle ( )
{
	if ( CheckAIInit ( )==0 ) return;

	if ( !cWorld.pManualPolygon )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Cannot Discard New Obstacle, New Obstacle Has Not Been Started" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	cWorld.DiscardNewPolygon ( );
}

void AIAddCoverPoint( float fx, float fy, float fz, float angle, LPSTR pStringPtr )
{
	tempCoverPoint cp;
	cp.fx = fx;
	cp.fy = fy;
	cp.fz = fz;
	cp.angle = angle;
	if ( pStringPtr )
	{
		if ( strlen ( pStringPtr ) < 64 )
			strcpy ( cp.pCoverName, pStringPtr );
		else
			strcpy ( cp.pCoverName, "<string too long>" );
	}
	else
		strcpy ( cp.pCoverName, "" );
		
	tempCoverPoints.push_back(cp);
}
tempCoverPoint* GetCoverPointPtr ( int iFindIndex )
{
	if ( iFindIndex >= 1 && iFindIndex <= (int)tempCoverPoints.size() )
		return &tempCoverPoints[iFindIndex-1];
	else
		return NULL;
}
int AIGetTotalCover()
{
	return (int)tempCoverPoints.size();
}
float AICoverGetPointX ( int iIndex )
{
	tempCoverPoint* pCoverPtr = GetCoverPointPtr ( iIndex );
	if ( pCoverPtr )
		return pCoverPtr->fx;
	else
		return 0.0f;
}
float AICoverGetPointY ( int iIndex )
{
	tempCoverPoint* pCoverPtr = GetCoverPointPtr ( iIndex );
	if ( pCoverPtr )
		return pCoverPtr->fy;
	else
		return 0.0f;
}
float AICoverGetPointZ ( int iIndex )
{
	tempCoverPoint* pCoverPtr = GetCoverPointPtr ( iIndex );
	if ( pCoverPtr )
		return pCoverPtr->fz;
	else
		return 0.0f;
}
float AICoverGetAngle ( int iIndex )
{
	tempCoverPoint* pCoverPtr = GetCoverPointPtr ( iIndex );
	if ( pCoverPtr )
		return pCoverPtr->angle;
	else
		return 0.0f;
}
LPSTR AICoverGetIfUsed ( int iIndex )
{
	tempCoverPoint* pCoverPtr = GetCoverPointPtr ( iIndex );
	if ( pCoverPtr )
		return pCoverPtr->pCoverName;
	else
		return "";
}

void AIAddContainer ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;
	Container *pContainer = cWorld.GetContainer ( iContainerID );
	if ( pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Already Exists", iContainerID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
		}
		return;
	}
	cWorld.AddContainer ( iContainerID );
}

int AIConnectContainers( int iFromContainer, float x, float z, int iToContainer, float x2, float z2 )
{
	CheckAIInit( );

	Container *pFromContainer = cWorld.GetContainer ( iFromContainer );
	
	if ( !pFromContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", iFromContainer );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return 1;
	}

	Container *pContainer = cWorld.GetContainer ( iToContainer );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", iToContainer );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return 1;
	}

	return cWorld.ConnectContainers( iFromContainer, x,z, iToContainer, x2,z2 );
}

void AIRemoveContainer ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	if ( iContainerID == 0 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Cannot Remove AI Container %d", iContainerID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	cWorld.RemoveContainer ( iContainerID );
}

int AIContainerExist ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Container* pContainer = cWorld.GetContainer ( iContainerID );
	
	return pContainer ? 1 : 0;
}

void AISetContainerActive ( int iContainerID, int iActive )
{
	if ( CheckAIInit ( )==0 ) return;

	Container* pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	( iActive != 0 ) ? pContainer->Activate( ) : pContainer->DeActivate( );
}

void AIAddZone ( int iID, float minx, float minz, float maxx, float maxz, int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = 0;

	if ( iContainerID >= 0 )
	{
		pContainer = CheckContainer ( iContainerID );
		if ( !pContainer ) return;
	}

	cWorld.AddZone ( iID, minx, minz, maxx, maxz, pContainer );
}

void AIAddZone ( int iID, float minx, float minz, float maxx, float maxz )
{
	AIAddZone ( iID, minx, minz, maxx, maxz, -1 );
}

void AIRemoveZone ( int iID )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.DeleteZone ( iID );
}

int AIZoneExist( int iID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Zone *pZone = cWorld.GetZone( iID );
	
	return pZone ? 1 : 0;
}

void AIEntityAssignZone ( int iEntityID, int iZoneID )
{
	if ( CheckAIInit ( )==0 ) return;

	Zone *pZone = cWorld.GetZone ( iZoneID );

	if ( !pZone ) 
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Zone (%d) Does Not Exist", iZoneID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	Entity *pEntity = CheckEntity ( iEntityID );
	if ( !pEntity ) return;

	pZone->AddEntity ( pEntity );
}

void AIEntityRemoveZone ( int iEntityID, int iZoneID )
{
	if ( CheckAIInit ( )==0 ) return;

	Zone *pZone = cWorld.GetZone ( iZoneID );

	if ( !pZone ) 
	{
		sprintf_s ( errAIStr, 255, "Zone (%d) Does Not Exist", iZoneID );
		MessageBox ( NULL, errAIStr, "AI Error", 0 );
		//exit ( 1 );
		return;
	}

	Entity *pEntity = CheckEntity ( iEntityID );
	if ( !pEntity ) return;

	pZone->RemoveEntity ( pEntity );
}

void AIKillEntity ( int iEntityID )
{
	if ( CheckAIInit ( )==0 ) return;
	cWorld.DeleteEntity ( iEntityID );
}

void AIKillPlayer ( )
{
	CheckAIInit( );

	cWorld.DeletePlayer( );
}

void AIMakePath ( int iPathID )
{
	if ( CheckAIInit ( )==0 ) return;

	int iResult = cWorld.AddPath ( iPathID );

	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path ID (%d) Is Out Of Range (1-%d)", iPathID, cWorld.GetMaxPath( ) );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	if ( iResult == 2 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Already Exists", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
}

void AIDeletePath ( int iPathID )
{
	if ( CheckAIInit ( )==0 ) return;

	int iResult = cWorld.DeletePath ( iPathID );

	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path ID (%d) Is Out Of Range (1-%d)", iPathID, cWorld.GetMaxPath( ) );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	if ( iResult == 2 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Does Not Exist", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
}

void AIPathAddPoint ( int iPathID, float x, float y, float z )
{
	AIPathAddPoint ( iPathID, x, y, z, -1 );
}

void AIPathAddPoint ( int iPathID, float x, float y, float z, int container )
{
	if ( CheckAIInit ( )==0 ) return;
	int iResult = cWorld.AddPathPoint ( iPathID, x, y, z, container );
	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path ID (%d) Is Out Of Range (1-%d)", iPathID, cWorld.GetMaxPath( ) );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
		}
		return;
	}
	if ( iResult == 2 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Does Not Exist", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
		}
		return;
	}
}

int AIPathCountPoints ( int iPathID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	return cWorld.CountPathPoints ( iPathID );
}

void AIMakeMemblockFromPath ( int iMemblockID, int iPathID )
{
	if ( CheckAIInit ( )==0 ) return;

	/*
	if ( !MemblockExist )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Include At Least One (non-AI) Memblock Command In Your Code To Use The AI Memblock Commands", "AI Error", 0 );
			exit(1);
		}

		return;
	}*/
	
	int iResult = cWorld.MakeMemblockFromPath ( iMemblockID, iPathID );

	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Does Not Exist", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
}

void AIMakePathFromMemblock ( int iPathID, int iMemblockID )
{
	if ( CheckAIInit ( )==0 ) return;

	/*
	if ( !MemblockExist )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Include At Least One (non-AI) Memblock Command In Your Code To Use The AI Memblock Commands", "AI Error", 0 );
			exit(1);
		}

		return;
	}
	*/
	
	int iResult = cWorld.MakePathFromMemblock ( iPathID, iMemblockID );

	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path ID (%d) Is Out Of Range (0-%d)", iPathID, cWorld.GetMaxPath( ) );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	if ( iResult == 2 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Already Exists", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	if ( iResult == 3 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Memblock (%d) Does Not Exist", iMemblockID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
}

void AIMakePathBetweenPoints ( int iPathID, int iContainerID, float x, float z, float x2, float z2, float fMaxEdgeCost, int destContainer )
{
	if ( CheckAIInit ( )==0 ) return;

	int iResult = cWorld.AddObstaclePath ( iPathID, iContainerID, x, z, x2, z2, fMaxEdgeCost, destContainer );

	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path ID (%d) Is Out Of Range (0-%d)", iPathID, cWorld.GetMaxPath( ) );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	if ( iResult == 2 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Already Exists", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
}

void AIMakePathBetweenPoints ( int iPathID, int iContainerID, float x, float z, float x2, float z2, float fMaxEdgeCost )
{
	AIMakePathBetweenPoints( iPathID, iContainerID, x, z, x2, z2, -1, iContainerID );
}

void AIMakePathBetweenPoints ( int iPathID, int iContainerID, float x, float z, float x2, float z2 )
{
	AIMakePathBetweenPoints( iPathID, iContainerID, x, z, x2, z2, -1, iContainerID );
}

void AIMakePathBetweenPoints ( int iPathID, float x, float z, float x2, float z2 )
{
	AIMakePathBetweenPoints ( iPathID, 0, x, z, x2, z2, -1, 0 );
}

void AIMakePathFromClosestWaypoints ( int iPathID, int iContainerID, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( pContainer ) return;

	int iResult = cWorld.AddPath ( iPathID );
	
	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Out Of Range (1-1024)", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	if ( iResult == 2 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Path (%d) Already Exists", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}
	
	Path *pPath = cWorld.GetPath ( iPathID );
	pContainer->pPathFinder->SearchPoints ( x, z, pPath, 2 );
}

void AIEntityAssignPatrolPath ( int iEntityID, int iPathID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iEntityID );
	if ( !pEntity ) return;

	int iResult = cWorld.AssignEntityToPatrolPath ( pEntity, iPathID );
	
	if ( iResult == 1 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Path (%d) Does Not Exist", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	if ( iResult == 2 )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Path (%d) Is Out Of Range (1-1024)", iPathID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pEntity->SetDefending ( false );
}

void AIEntityAddTarget( int iEntityID, int iTargetID )
{
	CheckAIInit( );

	Entity *pEntity = CheckEntity( iEntityID );
	if ( !pEntity ) return;

	Entity* pTargetEntity = cWorld.GetEntityCopy ( iTargetID );
	if ( pTargetEntity ) 
	{
		pEntity->AddTarget( pTargetEntity, false );
	}
	else
	{
		Hero* pTargetHero = cWorld.GetHeroCopy( iTargetID );
		if ( pTargetHero )
		{
			pEntity->AddTarget( pTargetHero, false );
		}
	}
}

void AIEntityRemoveTarget( int iEntityID, int iTargetID )
{
	CheckAIInit( );

	Entity *pEntity = CheckEntity( iEntityID );
	if ( !pEntity ) return;

	Entity* pTargetEntity = cWorld.GetEntityCopy ( iTargetID );
	if ( pTargetEntity ) 
	{
		pEntity->RemoveTarget( pTargetEntity );
	}
	else
	{
		Hero* pTargetHero = cWorld.GetHeroCopy( iTargetID );
		if ( pTargetHero )
		{
			pEntity->RemoveTarget( pTargetHero );
		}
	}
}

void AISetEntityCanSelectTargets( int iEntityID, int iCanSelect )
{
	CheckAIInit( );

	Entity *pEntity = CheckEntity( iEntityID );
	if ( !pEntity ) return;

	pEntity->SetCanSelectTargets( iCanSelect != 0 );
}

int AIGetEntityInZone ( int iEntityID, int iZoneID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iEntityID );
	if ( !pEntity ) return 0;
	Zone *pZone = cWorld.GetZone ( iZoneID );

	if ( !pZone )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Zone (%d) Does Not Exist", iZoneID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return 0;
	}

	bool bInZone = pZone->InZone ( pEntity->GetX( ), pEntity->GetZ( ), pEntity->GetContainer( ) );
	return bInZone ? 1 : 0;
}

int AIGetPlayerInZone ( int iZoneID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Hero *pHero = cWorld.pTeamController->GetPlayer( );
	if ( !pHero ) 
	{
		if ( bShowErrors )
		{
			sprintf_s(errAIStr, 255, "Player Does Not Exist"); // , iZoneID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return 0;
	}

	Zone *pZone = cWorld.GetZone ( iZoneID );

	if ( !pZone )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Zone (%d) Does Not Exist", iZoneID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return 0;
	}

	bool bInZone = pZone->InZone ( pHero->GetX( ), pHero->GetZ( ), pHero->GetContainer( ) );
	return bInZone ? 1 : 0;
}

void AISetEntitySpeed ( int iObjID, float fNewSpeed ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetMaxSpeed ( fNewSpeed );
}

void AISetEntityMaxPathStartCost( int iObjID, float fNewDistance )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetPathStartCostLimit( fNewDistance );
}

void AISetEntityTurnSpeed ( int iObjID, float fNewSpeed ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetTurnSpeed ( fNewSpeed );
}

void AISetEntityPatrolTime ( int iObjID, float fNewTime )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetAvgPatrolTime ( fNewTime );
}

void AISetEntityStance ( int iObjID, int iNewValue ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDefending ( false );
	pEntity->SetAggressiveness( iNewValue );
}

void AISetEntityAggressive ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDefending ( false );
	pEntity->SetAggressiveness( 1 );
}

void AISetEntityMeleeMode( int iObjID, int mode )
{
	CheckAIInit( );

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetMeleeMode( mode > 0 );
}

void AISetEntityActive ( int iObjID, int iActive )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetActive( iActive != 0 );
}

void AISetEntityRunAwayWhenHit ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDefending ( false );
	pEntity->SetAggressiveness( 3 );
}

void AIEntityHoldPosition ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	
	pEntity->Stop ( );
	pEntity->SetDefending ( false );
	pEntity->SetAggressiveness( 2 );
}

void AIStayWithinContainer ( int iObjID, float fLastX, float fLastZ, float* pX, float* pZ )
{
	if ( CheckAIInit ( )==0 ) return;
	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	pEntity->StayWithinContainer ( fLastX, fLastZ, pX, pZ );
}

void AISetEntityPosition ( int iObjID, float x, float z )
{
	/*
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetPosition ( x, 0, z );
	*/
	MessageBox( NULL, "The command \"AI Set Entity Position <id>,<x>,<z>\" has been removed, a y value must now be specified as well", "AI Error", 0 );
	exit(-1);
}

//the distance from the object's center to the entity's head, used for line of sight checks
void AISetEntityHeight( int iObjID, float height )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetHeight( height );
}

void AISetEntityPosition ( int iObjID, float x, float y, float z )
{
	if ( CheckAIInit ( )==0 ) return;
	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	pEntity->SetPosition ( x, y, z );
}

void AISetEntityYPosition ( int iObjID, float y )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetYPosition( y );
}

void AISetEntityAngleY( int iObjID, float angY )
{
	CheckAIInit( );

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetAngleY ( angY );
}

void AISetPlayerPosition ( float x, float z )
{
	MessageBox ( NULL, "\"AI Set Player Position <x>,<z>\" now requires a Y value", "AI Error", 0 );
	exit(-1);
}

void AISetPlayerPosition ( float x, float y, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Player has not been added to the AI system", "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pHero->SetPosition ( x, y, z );
}

void AISetPlayerAngleY( float angY )
{
	CheckAIInit( );

	Hero* pHero = cWorld.pTeamController->GetPlayer( );
	if ( !pHero )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Player has not been added to the AI system", "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pHero->SetAngleY ( angY );
}

void AISetPlayerContainer ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Player has not been added to the AI system", "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) 
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Container does not exist", "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pHero->SetContainer ( pContainer );
}

void AISetEntityContainer ( int iObjID, int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pEntity->SetContainer ( pContainer );
}

int AIGetEntityContainer ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return -2;

	Container *pContainer = pEntity->GetContainer( );
	if ( !pContainer ) return -1;

	if ( pEntity->bChangingContainers == true )
		return -3;

	return pContainer->GetID( );
}

//specify an angle of 360 for all round
void AISetEntityViewArc ( int iObjID, float fNewValue, float fNewValue2 ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	
	fNewValue  /= 2.0f; //convert between the two descriptions
	fNewValue2 /= 2.0f;
	pEntity->SetViewArc ( fNewValue, fNewValue2 );	//specify and angle of 180 for all round
}

void AISetEntityViewRange ( int iObjID, float fNewValue ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetViewRange ( fNewValue );
}

void AISetEntityHearingRange ( int iObjID, float fNewValue )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetHearingRange ( fNewValue );
}

void AISetEntityHearingThreshold ( int iObjID, int iNewValue )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetHearingThreshold ( iNewValue );
}

void AISetEntityFireArc ( int iObjID, float fNewValue ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetFireArc ( fNewValue / 2.0f );
}

void AISetEntityAttackDistance( int iObjID, float fNewDist )
{
	CheckAIInit( );

	Entity* pEntity = CheckEntity ( iObjID );
	pEntity->SetAttackDist( fNewDist );
}

void AISetEntityAvoidDistance( int iObjID, float fNewDist )
{
	CheckAIInit( );

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetAvoidDist( fNewDist );
}

void AISetEntityAvoidMode( int iObjID, int iNewMode )
{
	CheckAIInit( );

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetAvoidMode( iNewMode );
}

void AISetEntityAlwaysActive( int iObjID, bool flag )
{
	CheckAIInit( );
	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetAlwaysActive( flag );
}

int AIGetEntityAvoidMode( int iObjID )
{
	CheckAIInit( );

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) 
		return pEntity->GetAvoidMode( );
	else
		return 0;
}

void AISetEntityCanDuck ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetCanDuck( iMode > 0 );
}

void AISetEntityCanAttack ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetCanAttack( iMode > 0 );
}

void AISetEntityCanStrafe ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetCanStrafe( iMode > 0 );
}

void AISetEntityCanHear ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetCanHear( iMode > 0 );
}

void AISetEntityCanRoam ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetCanRoam( iMode > 0 );
}

void AISetEntityCanSearch ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( pEntity ) pEntity->SetCanSearch( iMode > 0 );
}

void AISetPlayerDucking ( int iDuck )
{
	if ( CheckAIInit ( )==0 ) return;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Player has not been added to the AI system", "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pHero->SetDucking ( (iDuck > 0) );
}

void AISetPlayerHeight ( float height )
{
	if ( CheckAIInit ( )==0 ) return;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Player has not been added to the AI system", "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pHero->SetHeight( height );
}

/*
void AISetRadius ( int iObjID, float fNewRadius )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = cWorld.GetEntity ( iObjID );

	if ( pEntity )
	{
		pEntity->SetRadius ( fNewRadius );
	}
	else
	{
		Hero *pHero = cWorld.GetHero ( iObjID );

		if ( pHero ) pHero->SetRadius ( fNewRadius );
		else
		{
			sprintf_s ( errAIStr, 255, "AI Object (%d) Does Not Exist", iObjID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
			return;
		}
	}
}*/

void AISetRadius ( float fNewRadius )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.SetRadius ( fNewRadius );
}

//0 is follow hero, 1+ is fend for themselves
//negative distance preserves current entity defend distance
void AITeamFollowPlayer ( float fDist )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.pTeamController->SetTeamStance ( 1, 0, fDist );
}

void AITeamSeparate ( )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.pTeamController->SetTeamStance ( 1, 1, -1 );
}

void AIEntityFollowPlayer ( int iObjID, float fDist )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	if ( !pEntity->IsFriendly( ) ) return;

	pEntity->SetDefendHero ( fDist );
}

void AIEntitySeparate ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	
	pEntity->SetAggressiveness( 1 );
	pEntity->SetFollowing ( false );
}

int AIGetEntityFollowing( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;
	
	return pEntity->GetFollowing( ) ? 1 : 0;
}

void AIEntityReset ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->ResetState( );
}

void AISetEntityDefendDist ( int iObjID, float fDist )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDefendDist ( fDist );
}

void AISetEntityDefending ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDefending ( iMode != 1 );
}

int AIGetEntityCanFire ( int iObjID ) 
{ 
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return (int) pEntity->GetIsFiring ( );
}

int AIGetEntityIsDucking ( int iObjID ) 
{ 
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return (int) pEntity->GetIsDucking ( );
}

float AIGetEntityViewRange ( int iObjID ) 
{ 
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return (float) pEntity->GetViewRange ();
}
float AIGetEntitySpeed ( int iObjID ) 
{ 
	if ( CheckAIInit ( )==0 ) return 0;
	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;
	return (float) pEntity->GetMaxSpeed();
}

int AIGetTotalPaths()
{
	return cWorld.GetTotalPaths();
}

int AIGetPathCountPoints( int iID )
{
	return cWorld.CountPathPoints( iID );
}

int AICouldSee ( int iObjID , float fX , float fY , float fZ )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return (int) pEntity->CouldSee ( fX , fY , fZ , true );
}

float AIPathGetPointX ( int iPath, int iPoint )
{
	return cWorld.GetPathPointX(iPath,iPoint);
}
float AIPathGetPointY ( int iPath, int iPoint )
{
	return cWorld.GetPathPointY(iPath,iPoint);
}
float AIPathGetPointZ ( int iPath, int iPoint )
{
	return cWorld.GetPathPointZ(iPath,iPoint);
}

int AIGetEntityChangingContainers( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return (int) pEntity->IsChangingContainers( );
}

void AISetEntityRadius( int iObjID, float fRadius )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetRadius( fRadius );
}

void AISetPlayerRadius( float fRadius )
{
	if ( CheckAIInit ( )==0 ) return;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			MessageBox ( NULL, "Player has not been added to the AI system", "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pHero->SetRadius( fRadius );
}

void AISetEntityHit ( int iObjID, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetGettingHit ( x, 0.0f, z );
}

int AIEntityExist ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = cWorld.GetEntityCopy ( iObjID );

	return pEntity ? 1 : 0;
}

void AISetThreadMode( int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	if ( iMode < 0 ) iMode = 0;
	if ( iMode > 2 ) iMode = 2;

	iThreadMode = iMode;
	cWorld.pTeamController->SetThreadNumber( iThreadCount );
}

void AISetThreadCount( int iCount )
{
	if ( CheckAIInit ( )==0 ) return;

	if ( iCount < 1 ) iCount = 1;
	
	iThreadCount = iCount;
	cWorld.pTeamController->SetThreadNumber( iCount );
}

void AISetUndesirableGridSpace( int container, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = cWorld.GetContainer ( container );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", container );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pContainer->pPathFinder->GridSetUndesirablePosition( x,z );
}

void AIClearUndesirableGridSpace( int container, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = cWorld.GetContainer ( container );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", container );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pContainer->pPathFinder->GridClearUndesirablePosition( x,z );
}

int AIGetGridSpace( int container, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Container *pContainer = cWorld.GetContainer ( container );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", container );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return 0;
	}

	return pContainer->pPathFinder->GridGetPosition( x,z );
}

int AIGetUndesirableGridSpace( int container, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Container *pContainer = cWorld.GetContainer ( container );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", container );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return 0;
	}

	return pContainer->pPathFinder->GridGetUndesirablePosition( x,z );
}

void AISetBlockingThreshold( int iValue )
{
	if ( CheckAIInit ( )==0 ) return;

	if ( iValue < 1 ) iValue = 1;

	PathFinderAdvanced::iUndesirableThreshold = iValue;
}

void AIAddDynamicBlocker( int id, int container, float x, float z, float radius )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer( container );
	if ( !pContainer ) return;

	pContainer->pPathFinder->AddDynamicBlocker( id, x, z, radius );
}

void AIRemoveDynamicBlocker( int id )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = cWorld.pContainerList;
	while ( pContainer )
	{
		pContainer->pPathFinder->RemoveDynamicBlocker( id );
		pContainer = pContainer->pNextContainer;
	}
}

void AISaveObstacleData( int iContainer, char* szFilename )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = cWorld.GetContainer ( iContainer );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", iContainer );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

#pragma warning (disable:4312)
	pContainer->pPathFinder->SaveObstacleData( szFilename );
#pragma warning (default:4312)
}

void AILoadObstacleData( int iContainer, char* szFilename )
{
	if ( CheckAIInit ( )==0 ) return;
	Container *pContainer = cWorld.GetContainer ( iContainer );
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", iContainer );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}
		return;
	}

	#pragma warning (disable:4312)
	pContainer->pPathFinder->LoadObstacleData( szFilename );
	#pragma warning (default:4312)
}

int AICountContainerObstacles( int iContainer )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Container *pContainer = cWorld.GetContainer ( iContainer );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", iContainer );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return -1;
	}

	return pContainer->pPathFinder->CountObstacles( );
}

int AICountContainerEdges( int iContainer )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Container *pContainer = cWorld.GetContainer ( iContainer );
	
	if ( !pContainer )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "AI Container (%d) Does Not Exist", iContainer );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return -1;
	}

	return pContainer->pPathFinder->CountEdges( );
}

void AIUpdate ( )
{ 
	if ( CheckAIInit ( )==0 ) return;
	
	cWorld.Update ( );
}

void AIUpdate ( float fUserTime )
{ 
	if ( CheckAIInit ( )==0 ) return;

	cWorld.Update ( fUserTime );
}

int AIGetEntityAvoiding ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = cWorld.GetEntityCopy ( iObjID );
	
	if ( !pEntity ) return 0;

	int iAvoid = (int) pEntity->IsAvoiding ( );
	
	return iAvoid;
}

//DarkSDK function return FLOATS
#ifdef DARKSDK_COMPILE

float AIGetPathPointX ( int iPathID, int iIndex ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	return cWorld.GetPathPointX ( iPathID, iIndex ); 
}

float AIGetPathPointZ ( int iPathID, int iIndex ) 
{ 
	if ( CheckAIInit ( )==0 ) return;

	return cWorld.GetPathPointZ ( iPathID, iIndex ); 
}

float AIGetEntityX ( int iObjID ) 
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetX( );
}

float AIGetEntityZ ( int iObjID ) 
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetZ( );
}

float AIGetEntityAngleY ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetAngleY ( );
}

float AIGetEntityTargetX ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetTargetX( );
}

float AIGetEntityTargetZ ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetTargetZ( );
}

float AIGetEntityMoveX ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetX( ) - pEntity->vecLastPos.x;
}

float AIGetEntityMoveZ ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetZ( ) - pEntity->vecLastPos.z;
}

float AIGetEntityDestinationX ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetDestX( );
}

float AIGetEntityDestinationZ ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetDestZ( );
}


float AIGetPlayerX ( )
{
	if ( CheckAIInit ( )==0 ) return;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Player Has Not Been Added To The AI System" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			exit ( 0 );
		}

		return 0;
	}

	return pHero->GetX( );
}

float AIGetPlayerZ ( )
{
	if ( CheckAIInit ( )==0 ) return;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Player Has Not Been Added To The AI System" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			exit ( 0 );
		}

		return 0;
	}

	return pHero->GetZ( );
}

char* AIGetEntityState ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = cWorld.GetEntity ( iObjID );
	
	if ( !pEntity )	return "Error! Entity Does Not Exist";

	return pEntity->GetStateName( );
}

char* AIGetEntityAction ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = cWorld.GetEntity ( iObjID );
	
	if ( !pEntity ) return "Error! Entity Does Not Exist";

	bool bAttack = false;
	
	char* szNewString = new char[256];
	strcpy_s( szNewString, 255, pEntity->GetStateName ( ) );

	if ( strcmp( szNewString, "Attack" )
	  || strcmp( szNewString, "Run and Attack" )
	  || strcmp( szNewString, "Strafe and Attack" ) )
	{
		bAttack = true;
	}

	float fMoveX = pEntity->GetX( ) - pEntity->vecLastPos.x;
	float fMoveZ = pEntity->GetZ( ) - pEntity->vecLastPos.z;
	float fMoveAng;

	if ( fabs ( fMoveX ) + fabs ( fMoveZ ) < 0.00001f ) 
	{
		strcpy_s( szNewString, 255, "Stopped" );
	}
	else
	{
		fMoveAng = fMoveZ / sqrt ( fMoveX*fMoveX + fMoveZ*fMoveZ );
		fMoveAng = acos ( fMoveAng ) * RADTODEG;
		if ( fMoveX < 0.0f ) fMoveAng = 360.0f - fMoveAng;

		float fAngY = pEntity->GetAngleY( );
		float fDiffAng = fMoveAng - fAngY;
		if ( fDiffAng > 180.0f ) fDiffAng = fDiffAng - 360.0f;
		if ( fDiffAng < -180.0f ) fDiffAng = fDiffAng + 360.0f;
		
		if ( fDiffAng < 45 && fDiffAng > -45 ) strcpy_s( szNewString, 255, "Moving Forward" );
		else if ( fDiffAng < 135 && fDiffAng > 0 ) strcpy_s( szNewString, 255, "Strafing Left" );
		else if ( fDiffAng > -135 && fDiffAng < 0 ) strcpy_s( szNewString, 255, "Strafing Right" );
		else strcpy_s( szNewString, 255, "Moving Backwards" );
	}

	if ( bAttack )
	{
		strcat_s( szNewString, 255, " And Attacking" );
	}
	
	return szNewString;
}

#else

//DarkBasic functions return DWORDS
float AIGetPathPointX ( int iPathID, int iIndex )
{
	if ( CheckAIInit ( )==0 ) return 0;

	float fX = cWorld.GetPathPointX ( iPathID, iIndex );

	return fX;
}

float AIGetPathPointZ ( int iPathID, int iIndex )
{
	if ( CheckAIInit ( )==0 ) return 0;

	float fZ = cWorld.GetPathPointZ ( iPathID, iIndex );

	return fZ;
}

float AIGetEntityX ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetX( );
	return fValue;
}

float AIGetEntityY ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetY( );
	return fValue;
}

float AIGetEntityZ ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetZ( );
	return fValue;
}

float AIGetEntityAngleY ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetAngleY ( );
	return fValue;
}

float AIGetEntityTargetX ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetTargetX( );
	return fValue;
}

float AIGetEntityTargetZ ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetTargetZ( );
	return fValue;
}

float AIGetEntityMoveX ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetX( ) - pEntity->GetLastPos( ENT_X );
	return fValue;
}

float AIGetEntityMoveZ ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetZ( ) - pEntity->GetLastPos( ENT_Z );
	return fValue;
}

float AIGetEntityDestinationX ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetDestX( );

	return fValue;
}

float AIGetEntityDestinationZ ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	float fValue = pEntity->GetDestZ( );

	return fValue;
}

float AIGetPlayerX ( )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Player Has Not Been Added To The AI System" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			exit ( 0 );
		}

		return 0;
	}

	float fX = pHero->GetX( );
	return fX;
}

float AIGetPlayerZ ( )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Hero *pHero = cWorld.pTeamController->GetPlayer ( );

	if ( !pHero )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, 255, "Player Has Not Been Added To The AI System" );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			exit ( 0 );
		}

		return 0;
	}

	float fZ = pHero->GetZ( );
	return fZ;
}

LPSTR AIGetEntityState ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

#pragma warning ( disable : 4312 )	//convert DWORD to 'char *'
		
	Entity *pEntity = cWorld.GetEntityCopy ( iObjID );
	
	if ( !pEntity )
	{
		LPSTR szReturnString = NULL;
		g_pGlob->CreateDeleteString ( (char**) &szReturnString, 30 );
		strcpy_s ( (char*) szReturnString, 30, "Error! Entity Does Not Exist" );
		return szReturnString;
	}
	
	char* szNewString = pEntity->GetStateName ( );
	
	DWORD dwSize = (DWORD) strlen ( (char*) szNewString );

	LPSTR szReturnString = NULL;
	g_pGlob->CreateDeleteString ( (char**) &szReturnString, dwSize+1 );
	
	strcpy_s ( (char*) szReturnString, dwSize+1, szNewString);

	//delete [] szNewString;

#pragma warning ( default : 4312 )
	
	return szReturnString;
}

LPSTR AIGetEntityAction ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

#pragma warning ( disable : 4312 )	//convert DWORD to 'char *'
		
	Entity *pEntity = cWorld.GetEntityCopy ( iObjID );
	
	if ( !pEntity )
	{
		LPSTR szReturnString = NULL;
		strcpy_s ( (char*) szReturnString, 30, "Error! Entity Does Not Exist" );
		return szReturnString;
	}

	bool bAttack = false;
	
	char* szNewString = new char[256];
	strcpy_s( szNewString, 255, pEntity->GetStateName ( ) );

	if ( !strcmp( szNewString, "Attack" )
	  || !strcmp( szNewString, "Run and Attack" )
	  || !strcmp( szNewString, "Strafe and Attack" ) )
	{
		bAttack = true;
	}

	float fMoveX = pEntity->GetX( ) - pEntity->GetLastPos( ENT_X );
	float fMoveZ = pEntity->GetZ( ) - pEntity->GetLastPos( ENT_Z );
	float fMoveAng;

	if ( fabs ( fMoveX ) + fabs ( fMoveZ ) < 0.00001f ) 
	{
		strcpy_s( szNewString, 255, "Stopped" );
	}
	else
	{
		fMoveAng = fMoveZ / sqrt ( fMoveX*fMoveX + fMoveZ*fMoveZ );
		fMoveAng = acos ( fMoveAng ) * RADTODEG;
		if ( fMoveX < 0.0f ) fMoveAng = 360.0f - fMoveAng;

		float fAngY = pEntity->GetAngleY( );
		float fDiffAng = fMoveAng - fAngY;
		if ( fDiffAng > 180.0f ) fDiffAng = fDiffAng - 360.0f;
		if ( fDiffAng < -180.0f ) fDiffAng = fDiffAng + 360.0f;
		
		if ( fDiffAng < 45 && fDiffAng > -45 ) strcpy_s( szNewString, 255, "Moving Forwards" );
		else if ( fDiffAng < 135 && fDiffAng > 0 ) strcpy_s( szNewString, 255, "Strafing Right" );
		else if ( fDiffAng > -135 && fDiffAng < 0 ) strcpy_s( szNewString, 255, "Strafing Left" );
		else strcpy_s( szNewString, 255, "Moving Backwards" );
	}

	if ( bAttack )
	{
		strcat_s( szNewString, 255, " And Attacking" );
	}
	
	DWORD dwSize = (DWORD) strlen ( (char*) szNewString );

	LPSTR szReturnString = NULL;
	g_pGlob->CreateDeleteString ( (char**) &szReturnString, dwSize+1 );
	
	strcpy_s ( (char*) szReturnString, dwSize+1, szNewString);
	
#pragma warning ( default : 4312 )
	
	return szReturnString;
}

#endif
//End of DarkSDK/DarkBasic differences.

int AIGetEntityTargetID ( int iObjID, int iIndex )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetTargetID ( iIndex );
}

int AIGetEntityTargetContainer ( int iObjID, int iIndex )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return -1;

	int iTarget = pEntity->GetTargetID ( iIndex );

	Entity *pTarget = cWorld.GetEntityCopy ( iTarget );
	if ( !pTarget ) 
	{
		Hero *pTargetHero = cWorld.GetHero( iTarget );
		if ( !pTargetHero ) return -1;
		else
		{
			Container *pTargetContainer = pTargetHero->GetContainer( );
			if ( !pTargetContainer ) return -1;

			return pTargetContainer->GetID( );
		}
	}
	else
	{	
		Container *pTargetContainer = pTarget->GetContainer( );
		if ( !pTargetContainer ) return -1;

		return pTargetContainer->GetID( );
	}
}

int AIGetEntityTeam ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	if ( pEntity->IsNeutral( ) ) return 0;
	return pEntity->IsFriendly( ) ? 1 : 2;
}

int AIGetEntityHeardSound ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;
	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;
	return pEntity->HeardSound( ) ? 1 : 0;
}

int AIGetEntityCanSee ( int iObjID, float x, float z, int ground )
{
	/*
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->CanSee ( x, pEntity->GetY( ), z, ground == 0 );
	*/
	MessageBox( NULL, "The command \"AI Get Entity Can See <id>,<x>,<z>,<ground>\" has been removed, use \"AI Get Entity Can See <id>,<x>,<y>,<z>,<ground>\" instead", "AI Error", 0 );
	exit(-1);
}

int AIGetEntityCanSee ( int iObjID, float x, float y, float z, int ground )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->CanSee ( x, y, z, ground == 0 );
}

float AIRayCast ( float x, float y, float z, float x2, float y2, float z2 )
{
	if ( CheckAIInit ( )==0 ) return 0;

	float dist = 0;
	bool bHit = cWorld.GlobalVisibilityCheck( x,y,z, x2,y2,z2, &dist );

	return dist;
}

int AIGetEntityStance ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetAggressiveness();
}

void AISetEntityNoLookAtPoint ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetNoLookPoint ( );
}

void AIEntityMoveToClosestSound ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	if ( pEntity->HeardSound( ) ) pEntity->MoveToInterest ( );
}

void AIEntityMoveAwayFromSound ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	if ( pEntity->HeardSound( ) ) pEntity->MoveAwayFromSound ( );
}

int AIGetEntityIsMoving ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	///if ( pEntity->bRedoPath ) return 1; // chganged beahiour of the moving flag

	return pEntity->IsMoving ( ) ? 1 : 0;
}

int AIGetEntityIsTurning ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->IsTurning ( ) ? 1 : 0;
}

void AISetEntityIdlePosition ( int iObjID, float x, float z, int container )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetIdlePos ( x, pEntity->GetY( ), z, container );
}

void AISetEntityIdlePosition ( int iObjID, float x, float z )
{
	//AISetEntityIdlePosition( iObjID, x, z, -1 );
	MessageBox( NULL, "\"AI Set Entity Idle Position <entity>,<x>,<z>\" Now requires a container ID", "AI Error", 0 );
	exit( 1 );
}

void AIEntityMoveToIdlePosition ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->MoveToIdlePos ( );
}

void AISetEntityCollide ( int iObjID, int iObjHit )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetHitEntity( NULL );
	if ( iObjHit > 0 ) 
	{
		Entity *pOtherEntity = cWorld.GetEntityCopy ( iObjHit );
		if ( pOtherEntity ) 
		{
			pEntity->SetHitEntity( pOtherEntity );
			pOtherEntity->SetHitByEntity( pEntity );
			pOtherEntity->SetHitBySomething( true );
		}
	}

	pEntity->SetHitSomething( true );
}

void AICreateSound ( float x, float y, float z, int iType, float fSize, int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = 0;

	if ( iContainerID >= 0 )
	{
		pContainer = CheckContainer ( iContainerID );
		if ( !pContainer ) return;
	}

	//char str [ 256 ];
	//sprintf_s( str, 256, "High Priority Sound Created: %d, Container: %d", iType, iContainerID );
	//if ( iType >= 10 ) MessageBox( NULL, str, "Info", 0 );

	Beacon *pBeacon = new Beacon ( 0.4f );
	pBeacon->SetPosition ( x, y, z );
	pBeacon->SetSound ( iType, fSize, pContainer );

	cWorld.AddBeacon ( pBeacon );
}

void AICreateSound ( float x, float y, float z, int iType, float fSize )
{
	AICreateSound ( x, y, z, iType, fSize, -1 );
}

void AICreateSound ( float x, float z, int iType, float fSize, int iContainerID )
{
	AICreateSound ( x, iContainerID*100.0f + 20.0f, z, iType, fSize, iContainerID );
	//MessageBox( NULL, "'AI CREATE SOUND <x>,<z>,<type>,<size>,<container>' now requires a y value", "AI Error", 0 );
	//exit(-1);
}

void AICreateSound ( float x, float z, int iType, float fSize )
{
	MessageBox( NULL, "'AI CREATE SOUND <x>,<z>,<type>,<size>' now requires a y value", "AI Error", 0 );
	exit(-1);
}

void AISetEntityControl ( int iObjID, int iMode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return ;

	pEntity->SetManualControl( true );
	pEntity->ChangeState ( cWorld.pEntityStates->pStateManual );
}

void AISetEntityMoveBoostPriority ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;
	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	// does nothing yet
}

void AIEntityGoToPosition ( int iObjID, float x, float z, int container )
{
	if ( CheckAIInit ( )==0 ) return;
	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	pEntity->SetDestination ( x, pEntity->GetY( ), z, container );
	if ( !pEntity->GetManualControl() )
	{
		pEntity->ChangeState ( cWorld.pEntityStates->pStateGoToDest );
	}
	pEntity->bLegacyForceMove = false;
}

void AIEntityGoToPosition ( int iObjID, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;
	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;
	AIEntityGoToPosition( iObjID, x, z, -1 );
	pEntity->bLegacyForceMove = true;
}

void AIEntitySearchArea ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SearchArea ( );
}

int AIGetEntityCountTargets ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->CountTargets ( );
}

void AIEntityDefendArea ( int iObjID, float x, float z, float dist, int container )
{
	MessageBox( NULL, "\"AI Entity Defend Area <id>,<x>,<z>,<dist>,<container>\" now requires a Y value as well", "AI Error", 0 );
}

void AIEntityDefendArea ( int iObjID, float x, float y, float z, float dist, int container )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDefendPos( x,y,z, ENT_X|ENT_Y|ENT_Z );
	pEntity->SetDefendArea ( dist, container );
}

void AIEntityDefendPoint ( int iObjID, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetAggressiveness( 2 );
	if ( !pEntity->GetManualControl() ) pEntity->ChangeState ( cWorld.pEntityStates->pStateIdle );
	pEntity->SetOrigPos( x,0,z, ENT_X|ENT_Z );
	pEntity->SetFinalDest( x,pEntity->GetOrigPos( ENT_Y ),z, ENT_X|ENT_Y|ENT_Z );
}

void AIEntityStop ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->Stop ( );
}

void AIEntityStopNoMoveAddition ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->StopNoMoveAddition();
}

void AIEntityLookAtPosition ( int iObjID, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->LookAt ( x, z );
}

void AIEntityLookAround ( int iObjID, float minAng, float maxAng )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->LookAround ( minAng, maxAng );
}

void AIEntityRandomMove ( int iObjID, float min, float max )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->RandomMove ( min, max );
}

void AIEntityMoveClose ( int iObjID, float x, float z, float maxDist )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->MoveClose ( x, pEntity->GetY(), z, maxDist, -1 );
}

int AIEntityMoveToCover ( int iObjID, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	/*pEntity->SetInterestPos( x,pEntity->GetY( ),z, ENT_X|ENT_Y|ENT_Z );
	if ( pEntity->MoveToCover () )
		return 1;
	else
		return 0;*/

	// check if entity has already chosen a cover point
	if ( pEntity->GetCoverPoint() )
	{
		for ( int c = 0; c < (int)coverInUseEntity.size() ; c++ )
		{
			if ( coverInUseEntity[c] == pEntity )
			{
				coverInUseEntity.erase( coverInUseEntity.begin()+c );
				coverInUseX.erase( coverInUseX.begin()+c );
				coverInUseZ.erase( coverInUseZ.begin()+c );
				break;
			}
		}

		pEntity->ClearCoverPoint();
	}

	// look for a cover point
	Path cPath;
	pEntity->GetContainer( )->pPathFinder->SearchCoverPoints ( pEntity->GetX(), pEntity->GetZ(), x, z, &cPath );

	int iNumPoints = cPath.CountPoints ( );
	if ( iNumPoints == 0 )
	{
		//no where to go
		return 0;
	}

	pEntity->Stand ( );
	
	//get entity->target vector
	float fDirX = x - pEntity->GetX ( );
	float fDirZ = z - pEntity->GetZ ( );

	float fRange = ( fDirX*fDirX + fDirZ*fDirZ );
	float fTargetDist = sqrt( fRange );

	bool bFound = false;
	int iCount = 0;

	int iIndex = -1;
	float fClosest = 1000000.0f;

	float fDistX, fDistZ, fCoverPointDist;
	bool btFound;

	for ( int i = 0; i < iNumPoints; i++ )
	{

		fDistX = x - cPath.GetPoint ( i ).x;
		fDistZ = z - cPath.GetPoint ( i ).y;

		// dont pick a cover point that is further away
		fCoverPointDist = sqrt( fDistX*fDistX + fDistZ*fDistZ );
		if ( fCoverPointDist > fTargetDist ) continue;

		float fX = cPath.GetPoint ( i ).x - pEntity->GetX ( );
		float fZ = cPath.GetPoint ( i ).y - pEntity->GetZ ( );

		btFound = false;

		if (  (int)pEntity->prevCoverX.size() < iNumPoints )
		{
			for ( int c = 0 ; c < (int)pEntity->prevCoverX.size(); c++ )
			{
				if ( pEntity->prevCoverX[c] == cPath.GetPoint ( i ).x && pEntity->prevCoverZ[c] == cPath.GetPoint ( i ).y && iNumPoints > 1 )
				{
					btFound = true;
					break;
				}
				else
				{

					for ( int t = 0; t < (int)coverInUseEntity.size() ; t++ )
					{
						fDistX = pEntity->GetX ( ) - coverInUseX[t];
						fDistZ = pEntity->GetZ ( ) - coverInUseZ[t];
						fCoverPointDist = sqrt( fDistX*fDistX + fDistZ*fDistZ );

						if ( fCoverPointDist < 200 )
						{
							btFound = true;
							break;
						}

					}

					fDistX = pEntity->GetX ( ) - cPath.GetPoint ( i ).x;
					fDistZ = pEntity->GetZ ( ) - cPath.GetPoint ( i ).y;
					fCoverPointDist = sqrt( fDistX*fDistX + fDistZ*fDistZ );

					if ( fCoverPointDist < 100 )
					{
						btFound = true;
						break;
					}
				}
			}
		}
		else
		{
			pEntity->prevCoverX.clear();
			pEntity->prevCoverZ.clear();
		}

		if ( btFound ) continue;

		float fDist = fX*fX + fZ*fZ;

		// bias distance for entity stance

			// aggressive, favour forward positions
			float fDotP = (fX*fDirX + fZ*fDirZ)/fTargetDist;
			if ( fDotP < 0 ) fDist -= fDotP*2;


		/*if ( pEntity->GetAggressiveness() == 0 )
		{
			// cautious, favour backward positions
			float fDotP = (fX*fDirX + fZ*fDirZ)/fTargetDist;
			if ( fDotP > 0 ) fDist += fDotP*2;
		}*/

		//choose closest
		if ( fDist < fClosest )
		{
			fClosest = fDist;
			iIndex = i;
		}
	}

	if ( iIndex == -1 ) return 0;

	int coverID = cPath.GetPoint ( iIndex ).container;
	sCoverPoint *pCoverPoint = pEntity->GetContainer( )->pPathFinder->GetCoverPoint( coverID );
	pEntity->SetCoverPoint( pCoverPoint );
	coverInUseEntity.push_back(pEntity);
	coverInUseX.push_back(pCoverPoint->fX);
	coverInUseZ.push_back(pCoverPoint->fY);
	//pEntity->SetDestination ( cPath.GetPoint ( iIndex ).x, pEntity->GetY( ), cPath.GetPoint ( iIndex ).y, -1 );
	//pEntity->MoveToCoverPoint();

	pEntity->MoveToCoverPoint();	

	return 1;

}

void AIEntityDuck ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->Duck ( );
}

void AIEntityStand ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->Stand ( );
}

void AIEntityStrafeTarget ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity *pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->StrafeTarget ( );
}

void AIMakeMemblockFromWaypoints ( int iMemblockID, int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;
	
	if ( MemblockExist ( iMemblockID ) )
	{
		DeleteMemblock ( iMemblockID );
	}
	
	pContainer->pPathFinder->MakeMemblockFromWaypoints ( iMemblockID );
}

void AIMakeWaypointsFromMemblock ( int iContainerID, int iMemblockID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = cWorld.GetContainer ( iContainerID );

	if ( !pContainer )
	{
		cWorld.AddContainer ( iContainerID );
		pContainer = cWorld.GetContainer ( iContainerID );
	}
	
	if ( !MemblockExist ( iMemblockID ) )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Memblock (%d) Does Not Exist", iMemblockID );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pContainer->pPathFinder->MakeWaypointsFromMemblock ( iMemblockID );
}

int AICountWaypoints ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return 0;

	return pContainer->pPathFinder->CountWaypoints ( );
}

void AIAddWaypoint ( int iContainerID, float x, float z )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->AddWaypoint ( x, z, false );
}

void AIRemoveWaypoint ( int iContainerID, int iIndex )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	if ( iIndex < 1 || iIndex > pContainer->pPathFinder->CountWaypoints( ) )
	{
		if ( bShowErrors )
		{
			sprintf_s ( errAIStr, "Waypoint Index (%d) Out Of Range", iIndex );
			MessageBox ( NULL, errAIStr, "AI Error", 0 );
			//exit ( 1 );
		}

		return;
	}

	pContainer->pPathFinder->RemoveWaypoint ( iIndex );
}

void AIUpdateWaypointVisibility ( int iContainerID, float fLimit )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->UpdateVisibility ( fLimit );
}

void AIUpdateWaypointVisibility ( int iContainerID )
{
	AIUpdateWaypointVisibility ( iContainerID, -1 );
}

void AIClearWaypoints ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->ClearWaypoints ( );
}

void AISetWaypointCost ( int iContainerID, int iIndex, float fCost )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->SetWaypointCost ( iIndex, fCost );
}

void AIHideErrors ( )
{
	if ( CheckAIInit ( )==0 ) return;

	bShowErrors = false;
}

void AIShowErrors ( )
{
	if ( CheckAIInit ( )==0 ) return;

	bShowErrors = true;
}

void AIDebugShowWaypoints ( int iContainerID, float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->DebugDrawWaypoints ( fHeight );
	pContainer->pPathFinder->DebugDrawCoverPoints ( fHeight );
}

void AIDebugHideWaypoints ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->DebugHideWaypoints ( );
	pContainer->pPathFinder->DebugHideCoverPoints ( );
}

void AIDebugShowWaypointEdges ( int iContainerID, float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->DebugDrawWaypointEdges ( fHeight );
}

void AIDebugHideWaypointEdges ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->DebugHideWaypointEdges ( );
}

void AIDebugShowHideObstacleBounds ( int iShowMode, int iContainerID, float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;
	if ( iContainerID == -1 )
	{
		// all containers
		Container *pContainer = cWorld.GetContainerList();
		while ( pContainer )
		{
			if ( iShowMode == 1 )
				pContainer->pPathFinder->DebugDrawPolygonBounds ( fHeight );
			else
				pContainer->pPathFinder->DebugHidePolygonBounds ( );
			pContainer = pContainer->pNextContainer;
		}
	}
	else
	{
		// one container
		Container *pContainer = CheckContainer ( iContainerID );
		if ( !pContainer ) return;
		if ( iShowMode == 1 )
			pContainer->pPathFinder->DebugDrawPolygonBounds ( fHeight );
		else
			pContainer->pPathFinder->DebugHidePolygonBounds ( );
	}
}
void AIDebugShowObstacleBounds ( int iContainerID, float fHeight ) { AIDebugShowHideObstacleBounds ( 1, iContainerID, fHeight ); }
void AIDebugHideObstacleBounds ( int iContainerID ) { AIDebugShowHideObstacleBounds ( 0, iContainerID, 0.0f ); }

void AIDebugShowAvoidanceGrid ( int iContainerID, float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->DebugDrawAvoidanceGrid ( fHeight );
}

void AIDebugHideAvoidanceGrid ( int iContainerID )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->DebugHideAvoidanceGrid ( );
}

void AIDebugShowPaths ( float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity::bShowPaths = true;
	Path::fDebugObjHeight = fHeight;
}

void AIDebugHidePaths ( )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity::bShowPaths = false;
}

void AIDebugShowAvoidanceAngles ( float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity::bShowAvoidanceAngles = true;
	Entity::fDebugObstacleAnglesObjHeight = fHeight;
}

void AIDebugHideAvoidanceAngles ( )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity::bShowAvoidanceAngles = false;
}

void AIDebugShowViewArcs ( float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity::bShowViewArcs = true;
	Entity::fDebugViewObjHeight = fHeight;
}

void AIDebugHideViewArcs ( )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity::bShowViewArcs = false;
}

void AIDebugShowSounds ( float fHeight )
{
	if ( CheckAIInit ( )==0 ) return;

	World::bShowBeacons = true;
	Beacon::fObjHeight = fHeight;
}

void AIDebugHideSounds ( )
{
	if ( CheckAIInit ( )==0 ) return;

	World::bShowBeacons = false;
	cWorld.DebugHideBeacons ( );
}

void AISetConsoleOutputOn ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.SetConsoleOn ( iObjID );
}

void AIChangeConsoleObject ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.ChangeConsoleObject ( iObjID );
}

void AISetConsoleOutputOff ( )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.SetConsoleOff ( );
}

/*
#ifndef DARKSDK_COMPILE
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif
*/

// new for Reloaded

void AISetEntityDiveSpeedMultiplier ( int iObjID, float speed )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDiveMultiplier ( speed );
}

void AISetEntityLeapSpeedMultiplier ( int iObjID, float speed )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetLeapMultiplier ( speed );
}

void AISetAvoidanceGridRadius ( float size )
{
	if ( CheckAIInit ( )==0 ) return;

	cWorld.SetAvoidanceRadius ( size );
}

int AIGetEntityIsBehindCorner ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetIsBehindCorner();
}

void AISetEntityCanHideBehindCorner ( int iObjID, int mode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetCanHideBehindCorner( mode > 0 );
}

void AISetEntityDiveRange ( int iObjID, float range )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetDiveRange( range );
}

int AIGetEntityIsDiving ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetIsDiving();
}

void AISetEntityCanDive ( int iObjID, int mode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetCanDive( mode > 0 );
}

void AIEntityForceOutOfCover ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->ForceOutOfCover( true );
}

void AISetEntityLeapRange ( int iObjID, float range )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetLeapRange( range );
}

int AIGetEntityIsLeaping ( int iObjID )
{
	if ( CheckAIInit ( )==0 ) return 0;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return 0;

	return pEntity->GetIsLeaping( );
}

void AISetEntityCanLeap ( int iObjID, int mode )
{
	if ( CheckAIInit ( )==0 ) return;

	Entity* pEntity = CheckEntity ( iObjID );
	if ( !pEntity ) return;

	pEntity->SetCanLeap( mode > 0 );
}

void AIAddDoor ( int iObjID, int container, float x1, float y1, float x2, float y2 )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( container );
	if ( !pContainer ) return;

	cWorld.AddDoor( iObjID, container, x1,y1, x2,y2 );
}

void AIRemoveDoor ( int iObjID, int container )
{
	if ( CheckAIInit ( )==0 ) return;

	Container *pContainer = CheckContainer ( container );
	if ( !pContainer ) return;

	cWorld.RemoveDoor( iObjID, container );
}
