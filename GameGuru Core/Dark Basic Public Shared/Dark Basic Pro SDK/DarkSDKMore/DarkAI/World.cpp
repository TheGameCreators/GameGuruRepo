#define _CRT_SECURE_NO_DEPRECATE

#include ".\..\..\Shared\DBOFormat\DBOData.h"

#include "DBPro Functions.h"
#include "Hero.h"
#include "Entity.h"
#include "StateMachine\StateSet.h"
#include "Polygon.h"
#include "Beacon.h"
#include "PathFinderAdvanced.h"
#include "TeamController.h"
#include "World.h"
#include "Zone.h"
#include "Container.h"
#include "Point.h"
#include "Vector.h"
#include "CollisionTree.h"
#include "AIThread.h"
#include "LeeThread.h"

extern "C" FILE* GG_fopen( const char* filename, const char* mode );

#define PI 3.14159265f

bool World::bShowBeacons = false;

extern float g_fShapeMidX;
extern float g_fShapeMidZ;
extern float g_fShapeRadius;
extern int g_GUIShaderEffectID;

FILE *pDebugFile;
AIThread tAIThread;

// Global to host second thread to handle path finding perf hit
bool g_bLeeThreadStarted = false;
LeeThread g_LeeThread;

World::World ( )
{
	iMaxPath = 1024;

	pBeaconList = 0;
	pZoneList = 0;
	pContainerList = new Container ( 0 );
	ppPathArray = new Path* [ iMaxPath ];

	pVisibilityData = 0;

	for ( int i = 0; i < iMaxPath; i++ ) ppPathArray [ i ] = 0;

	pEntityStates = new StateSet ( );
	//pPathFinder = new PathFinderAdvanced ( );
	pTeamController = new TeamController ( );
	//pPathFinder->SetRadius ( 2.5f );

	Entity::bShowPaths = false;
	Entity::pWorld = this;
	State::pWorld = this;
	State::pStateSet = pEntityStates;
	bShowBeacons = false;

	fRadius = 2.5f;
	pContainerList->pPathFinder->SetRadius ( fRadius );

	hDebugConsoleOut = 0;

	Reset ( );

	QueryPerformanceFrequency ( (LARGE_INTEGER*) &i64TimeFreq );
    
    QueryPerformanceCounter ( (LARGE_INTEGER*) &i64CurrentTime );
    i64LastTime = i64CurrentTime;

	// Start off secondary thread to assist in path finding perf hit
	g_bLeeThreadStarted = false;
}

World::~World ( )
{
	// final resets
	Reset ( );
	
	if ( pEntityStates ) delete pEntityStates;
	//if ( pPathFinder ) delete pPathFinder;
	if ( pTeamController ) delete pTeamController;
	if ( pManualPolygon ) delete pManualPolygon;
	if ( pContainerList ) delete pContainerList;

	FreeConsole ( );
}

//Dave
int World::GetTotalPaths()
{
	int count = 0;

	for ( int c = 0 ; c < iMaxPath ; c++ )
	{
		if ( ppPathArray [ c ] ) count++;
	}

	return count;
}

void World::Reset ( )
{
	// ensure thread reset for next foray
	g_LeeThread.EndWork();

	pManualPolygon = 0;

	while ( pBeaconList )
	{
		Beacon *pTemp = pBeaconList;
		pBeaconList = pBeaconList->pNextBeacon;
		delete pTemp;
	}

	while ( pZoneList )
	{
		Zone *pTemp = pZoneList;
		pZoneList = pZoneList->pNextZone;
		delete pTemp;
	}

	while ( pContainerList )
	{
		Container *pTemp = pContainerList;
		pContainerList = pContainerList->pNextContainer;
		delete pTemp;
	}

	while ( pVisibilityData )
	{
		CollisionObject *pTemp = pVisibilityData;
		pVisibilityData = pVisibilityData->pNextObject;
		delete pTemp;
	}
	
	pContainerList = new Container ( 0 );

	if ( ppPathArray )
	{
		for ( int i = 0; i < iMaxPath; i++ )
		{
			if ( ppPathArray [ i ] ) 
			{
				delete ppPathArray [ i ];
				ppPathArray [ i ] = 0;
			}
		}
	}

	pTeamController->Reset ( );
	
	//pHeroList = 0;
	//pEntityList = 0;
	pBeaconList = 0;
	pZoneList = 0;
	iDebugEntityNum = 0;
	pVisibilityData = 0;
}

void World::SetRadius ( float fNewRadius )
{
	Container *pContainer = pContainerList;

	while ( pContainer )
	{
		pContainer->pPathFinder->SetRadius ( fNewRadius );
		pContainer = pContainer->pNextContainer;
	}

	pTeamController->SetRadius ( fNewRadius );

	fRadius = fNewRadius;
}

void World::SetAvoidanceRadius ( float fNewRadius )
{
	Container *pContainer = pContainerList;

	while ( pContainer )
	{
		pContainer->pPathFinder->SetGridRadius ( fNewRadius );
		pContainer = pContainer->pNextContainer;
	}
}

void World::SetConsoleOn ( int iObjID )
{
	if ( hDebugConsoleOut ) return;
	iDebugEntityNum = iObjID;

	AllocConsole ( );
	hDebugConsoleOut = GetStdHandle ( STD_OUTPUT_HANDLE );
	
	DWORD dwWritten = 0;
	char* str = "**** AI Debugging Output ****";
	WriteConsole ( hDebugConsoleOut, str, (DWORD) strlen( str ), &dwWritten, NULL );
}

void World::ChangeConsoleObject ( int iObjID )
{
	iDebugEntityNum = iObjID;
}

void World::SetConsoleOff ( )
{
	FreeConsole ( );
	hDebugConsoleOut = 0;
	iDebugEntityNum = 0;
}

int World::RandInt ( float fAvgDiff )
{
	if ( fTimeDelta == 0 ) return 1;

	int iMod = (int) ( fAvgDiff / fTimeDelta );
	if ( iMod == 0 ) return 1;

	return rand( ) % iMod;
}

//void World::SetTerrain ( int id, sObject *pObject )
//{
	//work out the avoidance shape here and add it to the internal structure
//}

//suitable for convex objects, creates a bounding polygon containing all vertices
inline Polygon2D CalculateConvexHull ( vector<sVertex> sVertex_list )
{
	Polygon2D cNewPolygon;

	if ( sVertex_list.size( ) == 0 ) return cNewPolygon;

	vector<sVertex>::iterator vIter = sVertex_list.begin ( );
	vector<sVertex>::iterator vIndex = vIter;

	float fMinX;
	if ( vIter < sVertex_list.end( ) ) fMinX = (float) vIter->x;
	
	while ( vIter < sVertex_list.end ( ) )
	{
		if ( vIter->x < fMinX ) 
		{
			fMinX = vIter->x;
			vIndex = vIter;
		}

		vIter++;
	}
	
	bool bComplete = false;
	float fBeginX = vIndex->x;
	float fBeginY = vIndex->y;
	cNewPolygon.AddVertex ( fBeginX, fBeginY );
	
	//sVertex_list.erase ( vIndex );

	float fCurrX = fBeginX;
	float fCurrY = fBeginY;

	while ( sVertex_list.size ( ) > 0 && !bComplete )
	{
		vIter = sVertex_list.begin ( );
		vIndex = vIter;

		float fMinVecX = vIter->x - fCurrX;
		float fMinVecY = vIter->y - fCurrY;
		float fVecX, fVecY;

		vIter++;

		while ( vIter < sVertex_list.end ( ) )
		{
			//get leftmost vertex
			
			fVecX = vIter->x - fCurrX;
			fVecY = vIter->y - fCurrY;

			if ( ( fMinVecY * fVecX ) - ( fMinVecX * fVecY ) < 0 )
			{
				//if vertex is not right next to current vertex
				if ( fVecX*fVecX + fVecY*fVecY > 0.001 )
				{
					fMinVecX = fVecX;
					fMinVecY = fVecY;

					vIndex = vIter;
				}
			}
			else
			{
				if ( (( fMinVecY * fVecX ) - ( fMinVecX * fVecY ) < 0.0001) && (fVecX*fVecX + fVecY*fVecY > 0.001) )
				{
					if ( ( fVecX*fVecX + fVecY*fVecY ) > ( fMinVecX*fMinVecX + fMinVecY*fMinVecY ) 
						 || ( fabs(vIter->x - fBeginX) + fabs(vIter->y - fBeginY) < 0.001 ) )
					{
						fMinVecX = fVecX;
						fMinVecY = fVecY;

						vIndex = vIter;
					}
				}
			}

			vIter++;
		}

		fCurrX = vIndex->x;
		fCurrY = vIndex->y;
		
		if ( fabs(fCurrX - fBeginX) + fabs(fCurrY - fBeginY) < 0.001 )
		{
			//reached the beginning again
			bComplete = true;
		}
		else
		{
			cNewPolygon.AddVertex ( fCurrX, fCurrY );
			sVertex_list.erase ( vIndex );
		}
	}

	return cNewPolygon;
}

void World::AddStatic ( int id, int iContainerID, sObject *pObject, int iHeight, int iObstacleType )
{
	if ( !pObject ) return;

	//work out the avoidance shape here and add it to the internal structure
	Container *pContainer = GetContainer ( iContainerID );
	if ( !pContainer ) return;

	CalcObjectWorld( pObject );

	sObject *pDataPointer = pObject;
	if ( pObject->pInstanceOfObject ) pDataPointer = pObject->pInstanceOfObject;

	vector<sVertex> sVertex_list;
	sVertex sNewVertex;
	//float fX, fY, fZ;
	GGVECTOR3 vecVert;

	//char str[256];

	for ( int i = 0; i < pDataPointer->iFrameCount; i++ )
	{
		//sprintf_s( str,255,"Frame: %d/%d, %p",i,pDataPointer->iFrameCount, pObject->ppFrameList[i]->pMesh );
		//MessageBox(NULL, str, "Info", 0 );

		if ( !pDataPointer->ppFrameList [ i ] ) continue;
		//MessageBox(NULL, "Valid1", "Info", 0 );
		if ( !pDataPointer->ppFrameList [ i ]->pMesh ) continue;
		//MessageBox(NULL, "Valid2", "Info", 0 );

		DWORD dwFVFSize = pDataPointer->ppFrameList [ i ]->pMesh->dwFVFSize;
		BYTE *pVertexData = pDataPointer->ppFrameList [ i ]->pMesh->pVertexData;

		//not big enough for FVF_XYZ
		if ( dwFVFSize < 12 ) continue;

		CalculateAbsoluteWorldMatrix( pObject, pDataPointer->ppFrameList [ i ], pDataPointer->ppFrameList [ i ]->pMesh );
		
		for ( DWORD dwVert = 0; dwVert < pDataPointer->ppFrameList [ i ]->pMesh->dwVertexCount; dwVert++ )
		{
			//sprintf_s( str,255,"Vertex: %d",dwVert );
			//MessageBox(NULL, str, "Info", 0 );

			vecVert.x = *( (float*) ( pVertexData + dwVert * dwFVFSize )	 );
			vecVert.y = *( (float*) ( pVertexData + dwVert * dwFVFSize + 4 ) );
			vecVert.z = *( (float*) ( pVertexData + dwVert * dwFVFSize + 8 ) );
			
			//need to transform the vertices into world space
			GGVec3TransformCoord( &vecVert, &vecVert, &pDataPointer->ppFrameList [ i ]->matAbsoluteWorld );

			sNewVertex.x = vecVert.x;
			sNewVertex.y = vecVert.z;

			sVertex_list.push_back ( sNewVertex );
		}
	}

	if ( sVertex_list.size( ) == 0 ) return;

	//sprintf_s( str,255,"Vertices: %d",sVertex_list.size( ) );
	//MessageBox(NULL, str, "Info", 0 );
	
	//make a polygon out of the vertices
	Polygon2D cPolygon = CalculateConvexHull ( sVertex_list );
	if ( iHeight <= 0 ) cPolygon.bHalfHeight = true;
	if ( iHeight == 2 ) cPolygon.bTerrain = true;
	if ( iHeight == -1 ) cPolygon.bDiveOver = true;
	cPolygon.id = id;

	/*
	FILE* pModelData = GG_fopen("ModelData.txt","a");
	char str[256];

	for ( int i=0; i< cPolygon.CountVertices(); i++ )
	{
		sprintf(str, "X: %3.3f, Z: %3.3f\n", cPolygon.GetVertex( i ).x, cPolygon.GetVertex( i ).y );
		fputs(str,pModelData);
	}

	fclose(pModelData);
	*/

	//cPolygon_list.push_back ( cPolygon );
	
	//debug output
	/*
	int iObjNum = 2001;

	for (int i = 0; i < cPolygon.CountVertices ( ); i++ )
	{
		fX = cPolygon.GetVertex ( i ).x;
		fZ = cPolygon.GetVertex ( i ).y;
		fY = 10.0;

		MakeObjectSphere ( iObjNum, 2.0f );
		PositionObject ( iObjNum, fX, fY, fZ );

		iObjNum++;
	}
	*/
	
	if ( iObstacleType == 1 )
	{
		pContainer->pPathFinder->AddViewBlocker ( &cPolygon );
	}
	else
	{
		pContainer->pPathFinder->AddPolygon ( &cPolygon );
	}

	//pPathFinder->SetPolygons ( &cPolygon_list );
	//pPathFinder->SetRadius ( 2.5f );
}

void World::RemoveStatic ( int id )
{
	Container *pContainer = pContainerList;

	while ( pContainer )
	{
		pContainer->pPathFinder->RemovePolygon ( id );
		pContainer = pContainer->pNextContainer;
	}
}

void World::AddDoor ( int id, int iContainerID, float x1, float y1, float x2, float y2 )
{
	Container *pContainer = GetContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->AddDoor( id, x1, y1, x2, y2 );
}

void World::RemoveDoor ( int id, int iContainerID )
{
	Container *pContainer = GetContainer ( iContainerID );
	if ( !pContainer ) return;

	pContainer->pPathFinder->RemoveDoor( id );
}

void World::StartNewPolygon ( int id )
{
	if ( pManualPolygon ) return;
	pManualPolygon = new Polygon2D ( );
	pManualPolygon->id = id;
}

void World::AddVertex ( float x, float z )
{
	if ( !pManualPolygon ) return;
	pManualPolygon->AddVertex ( x, z );
}

void World::EndNewPolygon ( int iContainerID, int iHeight, int iObstacleType )
{
	EndNewPolygon ( iContainerID, iHeight, iObstacleType, true );
}

void World::EndNewPolygon ( int iContainerID, int iHeight, int iObstacleType, bool bUpdate )
{
	if ( !pManualPolygon ) return;
	pManualPolygon->bHalfHeight = ( iHeight <= 0 );
	if ( iHeight == 2 ) pManualPolygon->bTerrain = true;
	if ( iHeight == -1 ) pManualPolygon->bDiveOver = true;

	Container *pContainer = GetContainer ( iContainerID );
	if ( !pContainer ) 
	{
		char errAIStr[256];
		sprintf_s( errAIStr, 255, "Cannot Add Obstacle, Container (%d) Does Not Exist", iContainerID );
		MessageBox( NULL, errAIStr, "AI Error", 0 );
		delete pManualPolygon;
		return;
	}

	//cPolygon_list.push_back ( *pManualPolygon );
	if ( iObstacleType == 1 )
	{
		pContainer->pPathFinder->AddViewBlocker ( pManualPolygon );
	}
	else
	{
		pContainer->pPathFinder->AddPolygon ( pManualPolygon, bUpdate );
	}
	
	delete pManualPolygon;
	pManualPolygon = 0;
}

void World::DiscardNewPolygon ( )
{
	if ( !pManualPolygon ) return;

	delete pManualPolygon;
	pManualPolygon = 0;
}

inline float GetSqrDist ( GGVECTOR3 vecStart, GGVECTOR3 vecEnd )
{
	GGVECTOR3 vecDir = vecEnd - vecStart;
	return vecDir.x*vecDir.x + vecDir.y*vecDir.y + vecDir.z*vecDir.z;
}

inline float GetSqrDistFromLine( GGVECTOR3 vecStart, GGVECTOR3 vecEnd, GGVECTOR3 vecPoint )
{
	GGVECTOR3 vecDir = vecEnd - vecStart;
	GGVECTOR3 vecDir2 = vecPoint - vecStart;
	
	float fSqrLength = vecDir.x*vecDir.x + vecDir.y*vecDir.y + vecDir.z*vecDir.z;
	float fDotP = ( vecDir.x*vecDir2.x + vecDir.y*vecDir2.y + vecDir.z*vecDir2.z )/fSqrLength;

	if ( fDotP < 0 )
	{
		return vecDir2.x*vecDir2.x + vecDir2.y*vecDir2.y + vecDir2.z*vecDir2.z;
	}
	if ( fDotP > 1 )
	{
		vecDir2 = vecPoint - vecEnd;
		return vecDir2.x*vecDir2.x + vecDir2.y*vecDir2.y + vecDir2.z*vecDir2.z;
	}

	vecDir *= fDotP;
	vecDir2 -= vecDir;

	return vecDir2.x*vecDir2.x + vecDir2.y*vecDir2.y + vecDir2.z*vecDir2.z;
}

inline GGVECTOR3 GetClosestPointOnLine( GGVECTOR3 vecStart, GGVECTOR3 vecEnd, GGVECTOR3 vecPoint )
{
	GGVECTOR3 vecDir = vecEnd - vecStart;
	GGVECTOR3 vecDir2 = vecPoint - vecStart;
	
	float fSqrLength = vecDir.x*vecDir.x + vecDir.y*vecDir.y + vecDir.z*vecDir.z;
	float fDotP = ( vecDir.x*vecDir2.x + vecDir.y*vecDir2.y + vecDir.z*vecDir2.z )/fSqrLength;

	vecDir *= fDotP;
	vecStart += vecDir;

	return vecStart;
}

inline float GetDistAlongLine( GGVECTOR3 vecStart, GGVECTOR3 vecEnd, GGVECTOR3 vecPoint )
{
	GGVECTOR3 vecDir = vecEnd - vecStart;
	GGVECTOR3 vecDir2 = vecPoint - vecStart;
	
	float fSqrLength = vecDir.x*vecDir.x + vecDir.y*vecDir.y + vecDir.z*vecDir.z;
	float fDotP = ( vecDir.x*vecDir2.x + vecDir.y*vecDir2.y + vecDir.z*vecDir2.z )/fSqrLength;

	return fDotP;
}

void World::AddObstacleFromObject ( sObject *pObject, int iContainerID, int iHeight, float fPlaneHeight, float fMinLength, float fMinHeightOrCollisionScaling, bool bOutputToFile )
{
	struct Intersection
	{
		GGVECTOR3 v1,v2;
		float fAngY, fLength;
		Intersection *pNextSection;
	};

	struct Face
	{
		GGVECTOR3 v1,v2,v3;
		//GGVECTOR3 norm;
	};

	// new mode which will create a simple box shape to encompass all valid verts in slice
	bool bOnlyProduceSingleShape = false;
	if ( fMinLength==0.0f )
		bOnlyProduceSingleShape = true;

	if ( !pObject ) return;
	CalcObjectWorld ( pObject );

	sObject *pPosObject = pObject;
	if ( pObject->pInstanceOfObject )
	{
		pObject = pObject->pInstanceOfObject;
	}

	int iNumFaces = 0;
	for ( int i = 0; i < pObject->iFrameCount; i++ )
	{
		if ( !pObject->ppFrameList [ i ] ) continue;
		if ( !pObject->ppFrameList [ i ]->pMesh ) continue;
		if ( pObject->ppFrameList [ i ]->pMesh->dwFVFSize < 12 ) continue;
		int iNumIndices = pObject->ppFrameList [ i ]->pMesh->dwIndexCount;
		if ( iNumIndices <= 0 ) iNumIndices = pObject->ppFrameList [ i ]->pMesh->dwVertexCount;
		if ( iNumIndices <= 0 ) continue;
		iNumFaces += (iNumIndices / 3);		
		CalculateAbsoluteWorldMatrix ( pPosObject, pObject->ppFrameList [ i ], pObject->ppFrameList [ i ]->pMesh );
	}

	Face *pFaceList = new Face [ iNumFaces ];
	int iTotalFaces = 0;
	for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
	{
		if ( !pObject->ppFrameList [ iFrame ] ) continue;
		if ( !pObject->ppFrameList [ iFrame ]->pMesh ) continue;

		DWORD dwFVFSize = pObject->ppFrameList [ iFrame ]->pMesh->dwFVFSize;
		if ( dwFVFSize < 12 ) continue;
		
		int iIndices = pObject->ppFrameList [ iFrame ]->pMesh->dwIndexCount;
		int iVertices = pObject->ppFrameList [ iFrame ]->pMesh->dwVertexCount;

		BYTE *pVertexData = pObject->ppFrameList [ iFrame ]->pMesh->pVertexData;
		#ifdef X10
		DWORD *pIndexData = pObject->ppFrameList [ iFrame ]->pMesh->pIndices;
		#else
		WORD *pIndexData = pObject->ppFrameList [ iFrame ]->pMesh->pIndices;
		#endif

		GGVECTOR3 vecVert;
		
		if ( iIndices > 0 )
		{
			for ( int i = 0; i < iIndices; i+=3 )
			{
				if ( iTotalFaces >= iNumFaces ) continue;

				DWORD dwIndex = *( pIndexData + i ) * dwFVFSize;
				
				vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );

				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				pFaceList [ iTotalFaces ].v1 = vecVert;

				dwIndex = *( pIndexData + i + 1 ) * dwFVFSize;
				
				vecVert.x = *( (float*) ( pVertexData + dwIndex ) );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );

				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				pFaceList [ iTotalFaces ].v2 = vecVert;

				dwIndex = *( pIndexData + i + 2 ) * dwFVFSize;

				vecVert.x = *( (float*) ( pVertexData + dwIndex ) );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );

				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				pFaceList [ iTotalFaces ].v3 = vecVert;

				iTotalFaces++;
			}
		}
		else
		{
			for (int i = 0; i < iVertices; i+=3 )
			{
				if ( iTotalFaces >= iNumFaces ) continue;

				DWORD dwIndex = i*dwFVFSize;

				vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );

				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				pFaceList [ iTotalFaces ].v1 = vecVert;

				dwIndex = ( i + 1 )*dwFVFSize;

				vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );

				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				pFaceList [ iTotalFaces ].v2 = vecVert;

				dwIndex = ( i + 2 )*dwFVFSize;

				vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );

				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				pFaceList [ iTotalFaces ].v3 = vecVert;

				iTotalFaces++;
			}
		}
	}
	
	//int iNumIntersections = 0;
	float h1,h2,h3,normY;
	GGVECTOR3 v1_2, v1_3;
	Intersection *pIntersectionList = 0;
	for (int i=0; i < iTotalFaces; i++ )
	{
		int iIndex = 0;

		// work out products and normalize
		v1_2 = pFaceList [ i ].v2 - pFaceList [ i ].v1;
		v1_3 = pFaceList [ i ].v3 - pFaceList [ i ].v1;
		GGVec3Normalize ( &v1_2, &v1_2 );
		GGVec3Normalize ( &v1_3, &v1_3 );
		
		// remove faces that are more or less horizontal
		normY = v1_2.z*v1_3.x - v1_2.x*v1_3.z;
		if ( normY > 0.7071 || normY < -0.7071 ) continue; // <45

		h1 = pFaceList [ i ].v1.y - fPlaneHeight;
		h2 = pFaceList [ i ].v2.y - fPlaneHeight;
		h3 = pFaceList [ i ].v3.y - fPlaneHeight;
	
		float maxY = h1;
		float minY = h1;

		if ( h2>maxY ) maxY = h2;
		if ( h3>maxY ) maxY = h3;
		if ( h2<minY ) minY = h2;
		if ( h3<minY ) minY = h3;

		if ( bOnlyProduceSingleShape==false && maxY - minY < fMinHeightOrCollisionScaling ) continue; 

		if ( h1 == 0 ) h1 = 0.001f;
		if ( h2 == 0 ) h2 = 0.001f;
		if ( h3 == 0 ) h3 = 0.001f;

		if ( h1*h2>0 && h2*h3>0 && h3*h1>0 ) continue;

		Intersection* pNewSection = new Intersection();
		pNewSection->pNextSection = pIntersectionList;
		pIntersectionList = pNewSection;

		GGVECTOR3 vecDir;
		float fScale;

		if ( h1*h2 < 0 ) 
		{ 
			vecDir = pFaceList [ i ].v2 - pFaceList [ i ].v1;
			if ( vecDir.y == 0 ) fScale = 1.0f;
			else fScale = -h1 / vecDir.y;
			vecDir *= fScale;

			pNewSection->v1 = pFaceList [ i ].v1 + vecDir;

			iIndex++;
		}

		if ( h2*h3 < 0 ) 
		{ 
			vecDir = pFaceList [ i ].v3 - pFaceList [ i ].v2;
			if ( vecDir.y == 0 ) fScale = 1.0f;
			else fScale = -h2 / vecDir.y;
			vecDir *= fScale;

			if ( iIndex == 0 ) pNewSection->v1 = pFaceList [ i ].v2 + vecDir;
			else pNewSection->v2 = pFaceList [ i ].v2 + vecDir;

			iIndex++;
		}

		if ( h3*h1 < 0 )
		{
			vecDir = pFaceList [ i ].v1 - pFaceList [ i ].v3;
			if ( vecDir.y == 0 ) fScale = 1.0f;
			else fScale = -h3 / vecDir.y;
			vecDir *= fScale;
			
			pNewSection->v2 = pFaceList [ i ].v3 + vecDir;

			iIndex++;
		}
	}

	delete [] pFaceList;

	Intersection *pIntersection = pIntersectionList;
	Intersection *pLastSection = 0;
	int iNumValidLines = 0;

	// default value to allow removal process to thin out candidates
	if ( bOnlyProduceSingleShape==true ) fMinLength = 4.0f;

	//remove intersections that are too small
	//add in extra information to valid lines
	while ( pIntersection )
	{
		GGVECTOR3 vecDir = pIntersection->v2 - pIntersection->v1;
		if ( vecDir.x*vecDir.x + vecDir.z*vecDir.z > fMinLength*fMinLength ) 
		{
			pIntersection->fLength = sqrt( vecDir.x*vecDir.x + vecDir.z*vecDir.z );
			pIntersection->fAngY = acos ( vecDir.z / pIntersection->fLength ) * 57.295779513f;
			if ( vecDir.x < 0 ) pIntersection->fAngY = 180 - pIntersection->fAngY;

			pLastSection = pIntersection;
			pIntersection = pIntersection->pNextSection;

			iNumValidLines++;
		}
		else
		{
			if ( !pLastSection )
			{
				Intersection *pTemp = pIntersectionList;
				pIntersectionList = pIntersectionList->pNextSection;
				delete pTemp;
				pIntersection = pIntersectionList;
			}
			else
			{
				Intersection *pTemp = pIntersection;
				pLastSection->pNextSection = pIntersection->pNextSection;
				pIntersection = pIntersection->pNextSection;
				delete pTemp;
			}
		}
	}
	
	iNumValidLines = 0;

	FILE *pInfo = 0;
	char infoStr[256];
	if ( bOutputToFile ) 
	{
		pInfo = GG_fopen( "Obstacle Data.txt","w" );
		sprintf(infoStr, "Obstacle List:\n\n" ); // , pInfo );
		fputs( infoStr, pInfo );
	}

	// complex poly or simple shape
	if ( bOnlyProduceSingleShape==true )
	{
		// work out extends of intersection points to produce shape dimensions
		if ( pIntersectionList )
		{
			float fShapeMinX = pIntersectionList->v1.x;
			float fShapeMinZ = pIntersectionList->v1.z;
			float fShapeMaxX = pIntersectionList->v1.x;
			float fShapeMaxZ = pIntersectionList->v1.z;
			if ( pIntersectionList->v2.x < fShapeMinX ) fShapeMinX = pIntersectionList->v2.x;
			if ( pIntersectionList->v2.x > fShapeMaxX ) fShapeMaxX = pIntersectionList->v2.x;
			if ( pIntersectionList->v2.z < fShapeMinZ ) fShapeMinZ = pIntersectionList->v2.z;
			if ( pIntersectionList->v2.z > fShapeMaxZ ) fShapeMaxZ = pIntersectionList->v2.z;
			while( pIntersectionList )
			{
				Intersection *pFirstSection = pIntersectionList;
				if ( pIntersectionList->v2.x < fShapeMinX ) fShapeMinX = pIntersectionList->v2.x;
				if ( pIntersectionList->v2.x > fShapeMaxX ) fShapeMaxX = pIntersectionList->v2.x;
				if ( pIntersectionList->v2.z < fShapeMinZ ) fShapeMinZ = pIntersectionList->v2.z;
				if ( pIntersectionList->v2.z > fShapeMaxZ ) fShapeMaxZ = pIntersectionList->v2.z;
				pIntersectionList = pIntersectionList->pNextSection;
				delete pFirstSection;
			}

			// work out center point of this shape
			float fHalfWidth = ((fShapeMaxX-fShapeMinX)/2.0f);
			float fHalfHeight = ((fShapeMaxZ-fShapeMinZ)/2.0f);
			float fShapeMidX = fShapeMinX + fHalfWidth;
			float fShapeMidZ = fShapeMinZ + fHalfHeight;
			g_fShapeMidX = fShapeMidX;
			g_fShapeMidZ = fShapeMidZ;

			// if scaling being applied
			if ( fMinHeightOrCollisionScaling>0.0f && fMinHeightOrCollisionScaling!=1.0f )
			{
				fShapeMinX = fShapeMidX - ((fShapeMidX-fShapeMinX)*fMinHeightOrCollisionScaling);
				fShapeMaxX = fShapeMidX + ((fShapeMaxX-fShapeMidX)*fMinHeightOrCollisionScaling);
				fShapeMinZ = fShapeMidZ - ((fShapeMidZ-fShapeMinZ)*fMinHeightOrCollisionScaling);
				fShapeMaxZ = fShapeMidZ + ((fShapeMaxZ-fShapeMidZ)*fMinHeightOrCollisionScaling);
			}

			// radius calc AFTER scaling
			fHalfWidth = ((fShapeMaxX-fShapeMinX)/2.0f);
			fHalfHeight = ((fShapeMaxZ-fShapeMinZ)/2.0f);
			g_fShapeRadius = fHalfWidth;
			if ( fHalfHeight > g_fShapeRadius ) g_fShapeRadius = fHalfHeight;

			// clip final shape a little to tighter fit onto tree
			fShapeMinX = fShapeMidX - ((fShapeMidX-fShapeMinX)*0.65f);
			fShapeMaxX = fShapeMidX + ((fShapeMaxX-fShapeMidX)*0.65f);
			fShapeMinZ = fShapeMidZ - ((fShapeMidZ-fShapeMinZ)*0.65f);
			fShapeMaxZ = fShapeMidZ + ((fShapeMaxZ-fShapeMidZ)*0.65f);

			// create the obstacle
			StartNewPolygon(0);
			AddVertex( fShapeMaxX, fShapeMinZ );
			AddVertex( fShapeMinX, fShapeMinZ );
			AddVertex( fShapeMinX, fShapeMaxZ );
			AddVertex( fShapeMaxX, fShapeMaxZ );
			EndNewPolygon ( iContainerID, iHeight, 0, false);
		}
		else
		{
			// no obstacle created today!
			g_fShapeMidX = -1;
			g_fShapeMidZ = -1;
			g_fShapeRadius = -1;
		}
	}
	else
	{
		//merge lines that are close/joined
		while( pIntersectionList )
		{
			pIntersection = pIntersectionList->pNextSection;
			Intersection *pLastSection = pIntersectionList;
			Intersection *pFirstSection = pIntersectionList;
			float fSqrRadius = fRadius*fRadius;

			//sprintf( infoStr, "Line Number: %d, Pos1X: %3.3f, Pos1Z: %3.3f, Pos2X: %3.3f, Pos2Z: %3.3f, Angle: %3.3f\n", iNumValidLines, pIntersectionList->v1.x, pIntersectionList->v1.z, pIntersectionList->v2.x, pIntersectionList->v2.z, pIntersectionList->fAngY );
			//fputs( infoStr, pInfo );

			while ( pIntersection )
			{
				float ang = fabs(pIntersection->fAngY - pIntersectionList->fAngY);
				if ( ang > 90 ) ang = 180 - ang;

				float fDist1, fDist2, fPos1, fPos2;

				if ( ang < 5 )
				{
					//sprintf( infoStr, "Checking:       Pos1X: %3.3f, Pos1Z: %3.3f, Pos2X: %3.3f, Pos2Z: %3.3f, Angle: %3.3f\n", pIntersection->v1.x, pIntersection->v1.z, pIntersection->v2.x, pIntersection->v2.z, pIntersection->fAngY );
					//fputs( infoStr, pInfo );

					if ( pIntersectionList->fLength > pIntersection->fLength )
					{
						fDist1 = GetSqrDistFromLine( pIntersectionList->v1, pIntersectionList->v2, pIntersection->v1 );
						fDist2 = GetSqrDistFromLine( pIntersectionList->v1, pIntersectionList->v2, pIntersection->v2 );
					}
					else
					{
						fDist1 = GetSqrDistFromLine( pIntersection->v1, pIntersection->v2, pIntersectionList->v1 );
						fDist2 = GetSqrDistFromLine( pIntersection->v1, pIntersection->v2, pIntersectionList->v2 );
					}

					if ( fDist1 < fSqrRadius || fDist2 < fSqrRadius )
					{
						fPos1 = GetDistAlongLine( pIntersectionList->v1, pIntersectionList->v2, pIntersection->v1 );
						fPos2 = GetDistAlongLine( pIntersectionList->v1, pIntersectionList->v2, pIntersection->v2 );
					
						GGVECTOR3 vecOld = pIntersectionList->v1;
						GGVECTOR3 vecOld2 = pIntersectionList->v2;

						if ( fPos1 < 0 ) pIntersectionList->v1 = GetClosestPointOnLine( vecOld,pIntersectionList->v2,pIntersection->v1 );
						if ( fPos2 < 0 && fPos2 < fPos1 ) pIntersectionList->v1 = GetClosestPointOnLine( vecOld,pIntersectionList->v2,pIntersection->v2 );

						if ( fPos1 > 1 ) pIntersectionList->v2 = GetClosestPointOnLine( vecOld,pIntersectionList->v2,pIntersection->v1 );
						if ( fPos2 > 1 && fPos2 > fPos1 ) pIntersectionList->v2 = GetClosestPointOnLine( vecOld,vecOld2,pIntersection->v2 );
					
						vecOld = pIntersectionList->v2 - pIntersectionList->v1;
						pIntersectionList->fLength = sqrt( vecOld.x*vecOld.x + vecOld.y*vecOld.y + vecOld.z*vecOld.z );

						//sprintf( infoStr, "Result:         Pos1X: %3.3f, Pos1Z: %3.3f, Pos2X: %3.3f, Pos2Z: %3.3f\n\n", pIntersectionList->v1.x, pIntersectionList->v1.z, pIntersectionList->v2.x, pIntersectionList->v2.z );
						//fputs( infoStr, pInfo );

						Intersection *pTemp = pIntersection;
						pLastSection->pNextSection = pIntersection->pNextSection;
						pIntersection = pIntersection->pNextSection;
						delete pTemp;
						continue;
					}

					//sprintf( infoStr, "No Result\n\n" );
					//fputs( infoStr, pInfo );
				}

				pLastSection = pIntersection;
				pIntersection = pIntersection->pNextSection;
			}
		
			if ( bOutputToFile )
			{
				sprintf( infoStr, "AI Start New Obstacle\n" );
				fputs( infoStr, pInfo );

				sprintf( infoStr, "AI Add Obstacle Vertex %3.3f, %3.3f\n", pIntersectionList->v1.x, pIntersectionList->v1.z );
				fputs( infoStr, pInfo );

				sprintf( infoStr, "AI Add Obstacle Vertex %3.3f, %3.3f\n", pIntersectionList->v2.x, pIntersectionList->v2.z );
				fputs( infoStr, pInfo );

				sprintf( infoStr, "AI End New Obstacle %d,%d,0\n\n", iContainerID, iHeight );
				fputs( infoStr, pInfo );
			}
			else
			{
				StartNewPolygon(0);
				AddVertex( pIntersectionList->v1.x, pIntersectionList->v1.z );
				AddVertex( pIntersectionList->v2.x, pIntersectionList->v2.z );
				EndNewPolygon(iContainerID,iHeight,0, false);
			}

			iNumValidLines++;

			pIntersectionList = pIntersectionList->pNextSection;
			delete pFirstSection;
		}
	}

	if ( bOutputToFile ) fclose( pInfo );

	/*
	FILE *pOutput = GG_fopen("Indo.txt","w");
	char str[256];
	sprintf(str,"Intersections: %d",iNumValidLines );
	fputs(str,pOutput);
	fclose(pOutput);
	*/
	
	//pContainerList->pPathFinder->SetRadius( fRadius );
}

void World::CompleteObstacles ( int iContainerID )
{
	Container *pContainer = pContainerList;
	
	while ( pContainer )
	{
		if ( iContainerID < 0 || pContainer->GetID( ) == iContainerID ) pContainer->pPathFinder->CompleteObstacles( );

		//if ( pContainer->GetID( )==6 ) pContainer->pPathFinder->DebugDrawAvoidanceGrid( 615 );
		//if ( pContainer->GetID( )==5 ) pContainer->pPathFinder->DebugDrawPolygonBounds( 515 );

		pContainer = pContainer->pNextContainer;
	}
}

bool World::UsingGlobalVisibility( ) 
{ 
	return pVisibilityData ? true : false; 
}

void World::AddAlternateVisibilityObject( int id, sObject *pObject, int type )
{
	CollisionTree *pNewObject = new CollisionTree;

	pNewObject->iObjID = id;
	pNewObject->MakeCollisionObject( pObject );
	pNewObject->pNextObject = pVisibilityData;
	pVisibilityData = pNewObject;
}

void World::RemoveAlternateVisibilityObject( int id )
{
	CollisionObject *pObject = pVisibilityData;
	CollisionObject *pLast = 0;

	while ( pObject )
	{
		if ( pObject->iObjID == id )
		{
			if ( pLast ) pLast->pNextObject = pObject->pNextObject;
			else pVisibilityData = pObject->pNextObject;

			delete pObject;
			break;
		}

		pLast = pObject;
		pObject = pObject->pNextObject;
	}
}

bool World::GlobalVisibilityCheck( float x, float y, float z, float x2, float y2, float z2, float *dist )
{
	bool bResult = false;
	if ( dist ) *dist = -1;

	CollisionObject *pColObject = pVisibilityData;
	Point p;
	Vector v;

	p.set( x,y,z );
	v.set( x2-x, y2-y, z2-z );

	while( pColObject )
	{
		if ( pColObject->GetCollisionType() > 0 )
		{
			if ( pColObject->Intersects( &p, &v, 0, dist ) )
			{
				bResult = true;
				if ( !dist ) return true;	//if distance not needed any collision will do
			}
		}
		pColObject = pColObject->pNextObject;
	}

	return bResult;
}

void World::SetupAllPathFinders( )
{
	Container *pContainer = pContainerList;; 
	
	while ( pContainer )
	{
		pContainer->pPathFinder->ClearDistanceData( );

		pContainer = pContainer->pNextContainer;
	}
}

void World::AddContainer ( int iContainerID )
{
	if ( iContainerID < 1 ) return;

	Container *pNewContainer = new Container ( iContainerID );
	pNewContainer->pNextContainer = pContainerList;
	pContainerList = pNewContainer;

	pContainerList->pPathFinder->SetRadius ( fRadius );
}

int World::ConnectContainers( int iFromContainer, float x, float z, int iToContainer, float x2, float z2 )
{
	Container* pFromContainer = GetContainer( iFromContainer );
	Container* pToContainer = GetContainer( iToContainer );

	int in = pFromContainer->pPathFinder->InPolygons( x,z );
	if ( in > 0 ) 
	{
		pFromContainer->pPathFinder->FindClosestOutsidePoint( &x,&z );
		if ( pFromContainer->pPathFinder->InPolygons( x,z ) ) return 2;
	}

	in = pToContainer->pPathFinder->InPolygons( x2,z2 );
	if ( in > 0 ) 
	{
		pToContainer->pPathFinder->FindClosestOutsidePoint( &x2,&z2 );
		if ( pToContainer->pPathFinder->InPolygons( x2,z2 ) ) return 3;
	}

	float dist = sqrt((x2-x)*(x2-x) + (z2-z)*(z2-z));

	PathFinderAdvanced::sWaypoint *pStartWP = pFromContainer->pPathFinder->AddWaypoint( x,z, false, true );
	PathFinderAdvanced::sWaypoint *pEndWP = pToContainer->pPathFinder->AddWaypoint( x2,z2, false, true );

	pFromContainer->pPathFinder->UpdateSingleVisibility( pStartWP, -1 );
	pToContainer->pPathFinder->UpdateSingleVisibility( pEndWP, -1 );

	pStartWP->AddEdge( pEndWP, dist );
	pEndWP->AddEdge( pStartWP, dist );

	return 0;
}

void World::RemoveContainer ( int iContainerID )
{
	if ( iContainerID < 1 ) return;
	if ( !pContainerList ) return;

	while ( pContainerList && pContainerList->GetID( ) == iContainerID )
	{
		Container *pTemp = pContainerList;
		pContainerList = pContainerList->pNextContainer;
		delete pTemp;
	}

	if ( !pContainerList ) return;

	Container *pPrevContainer = pContainerList;
	Container *pContainer = pContainerList->pNextContainer;

	while ( pContainer )
	{
		if ( pContainer->GetID( ) == iContainerID )
		{
			pPrevContainer->pNextContainer = pContainer->pNextContainer;
			delete pContainer;
			pContainer = pPrevContainer->pNextContainer;
		}
		else
		{
			pPrevContainer = pContainer;
			pContainer = pContainer->pNextContainer;
		}
	}
}

Container* World::GetContainer ( int iContainerID )
{
	if ( iContainerID < 0 ) return 0;
	if ( !pContainerList ) return 0;

	Container* pContainer = pContainerList;

	while ( pContainer )
	{
		if ( pContainer->GetID ( ) == iContainerID ) return pContainer;

		pContainer = pContainer->pNextContainer;
	}

	return 0;
}

bool World::ContainerExist ( Container *pFindContainer )
{
	if ( !pFindContainer ) return false;
	if ( !pContainerList ) return false;

	Container *pContainer = pContainerList;
	
	while ( pContainer )
	{
		if ( pContainer == pFindContainer ) return true;
		pContainer = pContainer->pNextContainer;
	}

	return false;
}

void World::AddHero ( int id, int iContainerID, sObject *pObject, int iTeam )
{
	Container *pContainer = GetContainer ( iContainerID );
	
	Hero* pNewHero = new Hero ( id, pObject );
	pNewHero->SetContainer ( pContainer );

	pTeamController->AddHero ( pNewHero, iTeam );
	pNewHero->SetRadius( fRadius );
}

void World::DeleteHero ( int id )
{
	pTeamController->DeleteHero ( id );
}

void World::DeletePlayer( )
{
	pTeamController->DeletePlayer( );
}

void World::AddEntity ( int id, int iContainerID, sObject *pObject, sObject *pObjectRef, int iTeam )
{
	Container *pContainer = GetContainer ( iContainerID );
	
	Entity* pNewEntity = new Entity ( id, pObject, pObjectRef, pContainer );

	pTeamController->AddEntity ( pNewEntity, iTeam );
	pNewEntity->SetRadius( fRadius );
}

void World::DeleteEntity ( int id )
{
	Entity *pEntity = GetEntity ( id );
	if ( !pEntity ) return;
	pTeamController->RemoveEntity ( pEntity );
	
	Zone *pZone = pZoneList;
	while ( pZone )
	{
		pZone->RemoveEntity ( pEntity );
		pZone = pZone->pNextZone;
	}

	// must also remove entity from thread (if being acted on)
	pEntity->RemoveFromThread();

	// finally delete entity AI
	delete pEntity;
}

Hero* World::GetHero ( int id )
{
	return pTeamController->GetHero ( id );
}

Hero* World::GetHeroCopy ( int id )
{
	return pTeamController->GetHeroCopy ( id );
}

void World::AddZone ( int iID, float minx, float minz, float maxx, float maxz, Container *pContainer ) 
{
	Zone *pNewZone = new Zone ( iID, minx, minz, maxx, maxz, pContainer );
	pNewZone->pNextZone = pZoneList;

	pZoneList = pNewZone;
}

Zone* World::GetZone ( int iID )
{
	Zone *pZone = pZoneList;

	while ( pZone )
	{
		if ( pZone->GetID( ) == iID ) return pZone;
		pZone = pZone->pNextZone;
	}

	return 0;
}

void World::DeleteZone ( int iID )
{
	if ( !pZoneList ) return;

	if ( pZoneList->GetID( ) == iID )
	{
		Zone *pTemp = pZoneList;
		pZoneList = pZoneList->pNextZone;
		delete pTemp;
		return;
	}

	Zone *pPrevZone = pZoneList;
	Zone *pZone = pZoneList->pNextZone;

	while ( pZone )
	{
		if ( pZone->GetID( ) == iID )
		{
			pPrevZone->pNextZone = pZone->pNextZone;
			delete pZone;
			return;
		}

		pPrevZone = pZone;
		pZone = pZone->pNextZone;
	}
}

int World::AddPath ( int iPathID )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 1;
	if ( ppPathArray [ iPathID ] ) return 2;

	ppPathArray [ iPathID ] = new Path( );

	return 0;
}

int World::DeletePath ( int iPathID )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 1;
	if ( !ppPathArray [ iPathID ] ) return 2;

	delete ppPathArray [ iPathID ];
	ppPathArray [ iPathID ] = 0;

	return 0;
}

int World::AddPathPoint ( int iPathID, float x, float y, float z, int container )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 1;
	if ( !ppPathArray [ iPathID ] ) return 2;
	ppPathArray [ iPathID ]->AddPoint ( x, y, z, container );
	return 0;
}

float World::GetPathPointX ( int iPathID, int iIndex )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 0.0f;
	if ( !ppPathArray [ iPathID ] ) return 0.0f;
	if ( iIndex < 1 || iIndex > ppPathArray [ iPathID ]->CountPoints( ) ) return 0.0f;
	return ppPathArray [ iPathID ]->GetPoint( iIndex - 1 ).x;
}
float World::GetPathPointY ( int iPathID, int iIndex )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 0.0f;
	if ( !ppPathArray [ iPathID ] ) return 0.0f;
	if ( iIndex < 1 || iIndex > ppPathArray [ iPathID ]->CountPoints( ) ) return 0.0f;
	return ppPathArray [ iPathID ]->GetPoint( iIndex - 1 ).realy;
}
float World::GetPathPointZ ( int iPathID, int iIndex )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 0.0f;
	if ( !ppPathArray [ iPathID ] ) return 0.0f;
	if ( iIndex < 1 || iIndex > ppPathArray [ iPathID ]->CountPoints( ) ) return 0.0f;
	return ppPathArray [ iPathID ]->GetPoint( iIndex - 1 ).y;
}

int World::CountPathPoints ( int iPathID )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 0;
	if ( !ppPathArray [ iPathID ] ) return 0;

	return ppPathArray [ iPathID ]->CountPoints( );
}

int World::MakePathFromMemblock ( int iPathID, int iMemblockID )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 1;
	if ( ppPathArray [ iPathID ] ) return 2;
	if ( !MemblockExist( iMemblockID ) ) return 3;

	Path *pPath = new Path( );
	ppPathArray [ iPathID ] = pPath;

#pragma warning ( disable : 4312 ) //conversion from 'DWORD' to 'float*' of greater size
	int *pIData = (int*) GetMemblockPtr( iMemblockID );
	float *pFData = (float*) GetMemblockPtr( iMemblockID );
#pragma warning ( default : 4312 )

	int iNumPoints = *pIData;

	for ( int i = 0; i < iNumPoints; i++ )
	{
		float x = *( pFData + 1 + i*2 );
		float z = *( pFData + 2 + i*2 );
		pPath->AddPoint( x, 0, z );
	}

	return 0;
}

int World::MakeMemblockFromPath ( int iMemblockID, int iPathID )
{
	Path *pPath = GetPath( iPathID );
	if ( !pPath ) return 1;

	if ( MemblockExist( iMemblockID ) )
	{
		DeleteMemblock ( iMemblockID );
	}
	
	int iNumPoints = pPath->CountPoints( );
	int iSize = 4 + iNumPoints*8;

	MakeMemblock ( iMemblockID, iSize );

#pragma warning ( disable : 4312 ) //conversion from 'DWORD' to 'float*' of greater size
	int *pIData = (int*) GetMemblockPtr ( iMemblockID );
	float *pFData = (float*) GetMemblockPtr ( iMemblockID );
#pragma warning ( default : 4312 )

	*pIData = iNumPoints;

	for ( int i = 0; i < iNumPoints; i++ )
	{
		*( pFData + 1 + i*2 ) = pPath->GetPoint( i ).x;
		*( pFData + 2 + i*2 ) = pPath->GetPoint( i ).y;
	}

	return 0;
}

int World::AddObstaclePath ( int iPathID, int iContainerID, float x, float z, float x2, float z2, float fMaxEdgeCost, int destContainer )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 1;
	if ( ppPathArray [ iPathID ] ) return 2;

	if ( destContainer < 0 ) destContainer = iContainerID;
	
	Container *pContainer = GetContainer ( iContainerID );
	if ( !pContainer ) return 3;

	ppPathArray [ iPathID ] = new Path( );

	pContainer->pPathFinder->CalculatePath ( x, z, x2, z2, ppPathArray [ iPathID ], fMaxEdgeCost, destContainer ); 

	return 0;
}

Path* World::GetPath ( int iPathID )
{
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 0;

	return ppPathArray [ iPathID ];
}

int World::GetMaxPath ( )
{
	return iMaxPath;
}

int World::AssignEntityToPatrolPath ( Entity *pEntity, int iPathID )
{
	if ( !pEntity ) return 3;
	if ( !ppPathArray ) return 1;
	if ( iPathID < 1 || iPathID >= iMaxPath ) return 2;
	if ( !ppPathArray [ iPathID ] ) return 1;

	pEntity->SetPatrolPath ( ppPathArray [ iPathID ], iPathID );

	return 0;
}

void World::AddBeacon ( Beacon* pNewBeacon )
{
	if ( !pNewBeacon ) return;

	pNewBeacon->pNextBeacon = pBeaconList;
	pBeaconList = pNewBeacon;
}

void World::CleanUpWorldBeacons ( float fTimeDelta )
{
	Beacon *pBeacon = pBeaconList;
	Beacon *pLastBeacon = 0;

	while ( pBeacon )
	{
		bool bDelete = pBeacon->IsOld ( fTimeDelta );
		
		if ( bDelete )
		{
			Beacon *pTemp = pBeacon;

			if ( !pLastBeacon )  pBeaconList = pBeacon->pNextBeacon;
			else	pLastBeacon->pNextBeacon = pBeacon->pNextBeacon;

			pBeacon = pBeacon->pNextBeacon;

			delete pTemp;
		}
		else
		{
			pLastBeacon = pBeacon;
			pBeacon = pBeacon->pNextBeacon;
		}
	}
}

float World::GetTimeDelta ( )
{
	return fTimeDelta;
}

Entity* World::GetEntity ( int id )
{
	//check neutral entities
	return pTeamController->GetEntity ( id );
	
	//NULL if not found
	//return 0;
}

Entity* World::GetEntityCopy ( int id )
{
	return pTeamController->GetEntityCopy ( id );
}

void World::ClearCoverPoints()
{
	pTeamController->ClearCoverPoints();
}

void World::UpdateWorld ( )
{
	//SetConsoleOn( pTeamController->GetFirstFriendly()->GetID() );

	if ( hDebugConsoleOut && ( i64CurrentTime - i64LastDebugTime ) / ( (float) i64TimeFreq ) > 0.5f )
	{
		i64LastDebugTime = i64CurrentTime;
		
		COORD home = { 0, 0 };
		DWORD dwWritten;

		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(hDebugConsoleOut, &consoleInfo);
		
		FillConsoleOutputCharacter(hDebugConsoleOut, ' ', consoleInfo.dwSize.X * consoleInfo.dwSize.Y, home, &dwWritten);
		SetConsoleCursorPosition ( hDebugConsoleOut, home );

		this->DebugOutput ( );
	}

	Container* pContainer = pContainerList;

	while ( pContainer )
	{
		pContainer->pPathFinder->DebugUpdatePolygonColour( fTimeDelta );
		pContainer->pPathFinder->DebugUpdateAvoidanceGrid( fTimeDelta );
		pContainer->pPathFinder->Update( fTimeDelta );

		pContainer = pContainer->pNextContainer;
	}

	if ( bShowBeacons ) DebugShowBeacons ( );

	//if ( iThreadMode == 0 )
	{	
		pTeamController->Update ( fTimeDelta, pBeaconList, pZoneList );
		CleanUpWorldBeacons ( fTimeDelta );
	}
	/*else
	{
		tAIThread.Join( );
		CleanUpWorldBeacons ( fTimeDelta );

		tAIThread.SetupData( pTeamController, fTimeDelta, pBeaconList, pZoneList );
		tAIThread.Start( );
	}
	*/

	// Start off secondary thread to assist in path finding perf hit
	if ( g_bLeeThreadStarted == false )
	{
		g_bLeeThreadStarted = true;
		g_LeeThread.Join();
		g_LeeThread.SetupData();
		g_LeeThread.Start();
	}
}

void World::StopWorld ( )
{
	// Stop secondary thread when finished in game
	if ( g_bLeeThreadStarted == true )
	{
		g_bLeeThreadStarted = false;
		g_LeeThread.StopRunning();
		while ( g_LeeThread.IsReadyToRun() == false )
		{
			// wait for thread to be ready for next time
		}
	}
}

void World::Update ( float fUserTime )
{
	QueryPerformanceCounter ( (LARGE_INTEGER*) &i64CurrentTime );

	fTimeDelta = fUserTime;

	i64LastTime = i64CurrentTime;

	UpdateWorld ( );
}

void World::Update ( )
{
	//GG_fopen_s( &pDebugFile, "AIDebug.txt", "ab" );
	//char str [ 256 ];

	QueryPerformanceCounter ( (LARGE_INTEGER*) &i64CurrentTime );

	//sprintf_s( str, 256, "\nOther: %10d ", i64CurrentTime - i64LastTime );
	//fputs( str, pDebugFile );

	fTimeDelta = ( i64CurrentTime - i64LastTime ) / ( (float) i64TimeFreq );

	UpdateWorld ( );

	QueryPerformanceCounter ( (LARGE_INTEGER*) &i64LastTime );
	
	i64LastTime = i64CurrentTime;
}

void World::DebugShowBeacons ( )
{
}

void World::DebugHideBeacons ( )
{
}

void World::DebugOutput ( )
{
	pTeamController->DebugOutputEntityData ( hDebugConsoleOut, iDebugEntityNum );
}
