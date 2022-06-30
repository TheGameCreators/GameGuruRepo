/*
 10k polys 1ms - Dave's Mighty CPU Occluder
*/

#include "stdafx.h"
#include ".\globstruct.h"
#include "CPU3D.h"
#include ".\..\..\..\Include\SoftwareCulling.h"
#include "..\..\Shared\DBOFormat\DBOData.h"
#include "..\..\Shared\Camera\CCameraDataC.h"
#include "math.h"
#include "stdio.h"
#include "mmsystem.h"
#include <vector>

#include "CBasic2DC.h"
#include "CObjectsC.h"

extern "C" FILE* GG_fopen( const char* filename, const char* mode );

float gCPUOccluderCamShiftX = 0.0f;
float gCPUOccluderCamShiftZ = 0.0f;

int howManyOccluders = 0;
int howManyOccludersDrawn = 0;
int howManyOccludees = 0;
int howManyOccludeesHidden = 0;
int howManyMarkers = 0;
float trackingSize = 0;
float occludeeMaxDistance = 12000;

float smallDistanceMulti = 1;
float prevSmallDistanceMulti = 0;

float AggresiveMode = 0;
bool occluderBigVisualChange = false;
int OccluderQuickTimeDelay = 0;
float oldCameraAngleY;

extern GlobStruct* g_pGlob;

void Sync ( void );

// occluder control globals
HANDLE	g_hOccluderBegin = NULL;
HANDLE	g_hOccluderEnd = NULL;
bool	g_occluderOn = false;
bool	g_occluderf9Mode = false;
bool    g_enabeleverything = false;
int cpu3dMaxPolys = CPU_3D_MAX_POLY_SETTING;

#define MAX_CACHED_OBJECTS 130000
#define USE_SCREEN_SPACE_OCCLUDER
#define SHOW_LESS_OCCLUDEES_ON_HIGH_PRIM_CALLS

 //occluder lists
 std::vector <int> OccluderList;
 std::vector <int> OccluderListClosest;
 std::vector <int> OccluderListTemp;
 std::vector <float> OccluderListTempSize;
 std::vector <int> OccluderListDrawn;

 struct tPolyList
 {
	float max_radius;
	VECTOR4D world_pos;
	std::vector <POLYF4DV2> polys;
 };

 tPolyList* cachedPolys[MAX_CACHED_OBJECTS];

 //occludee list
 std::vector <int> OccludeeList;
 std::vector <int> OccludeeListNotVis;
 std::vector <int> OccludeeListIsVis;
 std::vector <bool> OccludeeListIsCharacter;

 int CameraIndex = 0;

 int ShowZbuffer = 0;

 #define DB __declspec(dllexport)
 
#ifdef _TIME_TAKEN_
__int64 g_i64TimeFreq		= 0;
__int64 g_i64CurrentTime	= 0;
__int64 g_i64StartTime		= 0;
double	g_fTotalTimeTaken	= 0;
double  g_fTimeStart		= 0.0f;
double  g_fTimeCurr			= 0.0f;
bool	g_bTimer			= false;
FILE*	g_pFile				= NULL;
int		SettleTime			= 0;
DWORD oldtime = 0;
DWORD timegettime = 0;

void SetupTimer ( void )
{
 QueryPerformanceCounter   ( ( LARGE_INTEGER* ) &g_i64StartTime );
 QueryPerformanceFrequency ( ( LARGE_INTEGER* ) &g_i64TimeFreq  );

 g_fTotalTimeTaken = 0;

 if ( g_i64TimeFreq == 0 )
  g_i64TimeFreq = 1;

 g_bTimer = true;
}

void ClearTimer ( void )
{
	g_fTotalTimeTaken = 0;
}

void StoreTimer ( void )
{

	if ( SettleTime < 60 )
	{
		SettleTime++;
		return;
	}

	if ( !g_pFile )
	{
		g_pFile = GG_fopen("Software CPU Time.txt" , "w" );
	}

	if ( g_pFile )
	{
		char str[256];
		sprintf ( str , "Time Taken is %Lf milliseconds, " , g_fTotalTimeTaken );
		fputs ( str , g_pFile );
		sprintf ( str , "timegettime is %d\n" , timegettime );
		fputs ( str , g_pFile );
	}
}

void ResetTimer ( void )
{
 if ( !g_bTimer )
  SetupTimer ( );

 QueryPerformanceCounter ( ( LARGE_INTEGER* ) &g_i64CurrentTime );
 g_fTimeCurr = ( g_i64CurrentTime - g_i64StartTime ) / ( ( double ) g_i64TimeFreq );
 g_fTimeStart = g_fTimeCurr;
 g_i64StartTime = g_i64CurrentTime;
}

void UpdateTimer ( void )
{
 if ( !g_bTimer )
  SetupTimer ( );

 QueryPerformanceCounter ( ( LARGE_INTEGER* ) &g_i64CurrentTime );
 g_fTimeCurr = ( g_i64CurrentTime - g_i64StartTime ) / ( ( double ) g_i64TimeFreq );
 DWORD ms = ( ( g_i64CurrentTime - g_i64StartTime ) * 1000 / g_i64TimeFreq );
 g_fTotalTimeTaken += ms;
 ResetTimer();
}
#endif

// initialize camera position and direction
POINT4D  cam_pos    = {0,0,0,1};
POINT4D  cam_target = {0,0,0,1};
VECTOR4D cam_dir    = {0,0,0,1};

// all your initialization code goes here...
VECTOR4D vscale={1.0,1.0,1.0,1}, 
         vpos = {0,0,0,1}, 
         vrot = {0,0,0,1};

CAMERA        cam;       // the single camera

RENDERLIST rend_list; // the render list
RENDERLIST* pRendlist;

ZBUFFERV1 zbuffer; // zbuffer
#ifndef METHOD_TWO
UINT* mipMaps[9];
#endif

//============================================================================

 void OccluderConstructor ( void )
 {
	// make lookup tables
	BuildSinCosTables();

	// initialize the camera
	CameraInit(&cam, CAM_MODEL_EULER, &cam_pos, &cam_dir, &cam_target, 100.0f, 12000.0f, 70.0f, SCREEN_WIDTH, SCREEN_HEIGHT );

	// create the z buffer
	ZBufferCreate(&zbuffer, SCREEN_WIDTH, SCREEN_HEIGHT );

	OccluderList.reserve(200);
	OccluderListClosest.reserve(200);
	OccluderListTemp.reserve(200);
	OccluderListTempSize.reserve(200);
	OccluderListDrawn.reserve(200);
	OccludeeList.reserve(10000);

	for ( int c = 0 ; c < MAX_CACHED_OBJECTS ; c++ )
		cachedPolys[c] = NULL;

#ifndef METHOD_TWO
	// create mipmaps
	int tSize=128;
	mipMaps[0] = (UINT*)zbuffer.zbuffer;
	for ( int c = 1; c < 9 ; c++ )
	{
		mipMaps[c] = (UINT*)malloc((tSize*tSize) * sizeof(UINT));
		tSize /= 2;
	}
#endif

 }

 void OccluderDestructor ( void )
 {
	 ZBufferDelete(&zbuffer);

#ifndef METHOD_TWO
	 for ( int c = 1; c < 9 ; c++ )
		free(mipMaps[c]);
#endif
 }

DB void Draw ( void )
{

#ifdef _TIME_TAKEN_
ResetTimer();
#endif

	// if no render ist ptr, early exit
	if ( !pRendlist )
		return;

   if ( pRendlist->num_polys > 0 )
   {
		pRendlist->poly_data[0].next = NULL;
		pRendlist->poly_data[0].prev = NULL;
   }

	// remove backfaces
    // and also apply world to camera transform
	RenderListRemoveBackfaces(&rend_list, &cam);
	RenderListClipPolys(&rend_list, &cam, CLIP_POLY_X_PLANE | CLIP_POLY_Y_PLANE | CLIP_POLY_Z_PLANE );

	// apply camera to perspective transformation
	// Now does screen also (perspective to screen)
	RenderListCameraToPerspective(&rend_list, &cam);

	// apply screen transform
	// merged with previous transform loop
	RenderListPerspectiveToScreen(&rend_list, &cam);

	// Draw
	ZBufferClear(&zbuffer, (16000 << FIXP16_SHIFT));
	RenderListDrawZBuffer(&rend_list, NULL, 0 , (UCHAR *)zbuffer.zbuffer, SCREEN_WIDTH*4);

	UINT   *zb_ptr    =  (UINT *)zbuffer.zbuffer;
	int max = 0;
	int min = 100000;

#ifndef METHOD_TWO
	// make mipmaps
	int tSize = 256/2;
	int tSizeY = 144/2;
	int samples = 2;
	int mipMap = 0;
	UINT highest;
	for ( int a = 1 ; a < 9 ; a++ )
	{
		for ( int y = 0 ; y < tSize ; y++ )
		{
			for ( int x = 0 ; x < tSize ; x++ )
			{
				highest = 0;
				for ( int yy = y*samples; yy < (y*samples) + samples ; yy++ )
				{
					if ( yy >= tSizeY*2 ) break;

					for ( int xx = x*samples; xx < (x*samples) + samples ; xx++ )
					{
						if ( xx >= tSize*2 ) continue;

						if ( mipMaps[a-1][xx + yy*(tSize*2)] > highest )
							highest = mipMaps[a-1][xx + yy*(tSize*2)];							
					}
				}
				mipMaps[a][x + y*tSize] = highest;
			}
		}
		tSize /= 2;
		tSizeY = tSize;
	}
#endif

#ifdef _TIME_TAKEN_
UpdateTimer();
timegettime = timeGetTime() - oldtime;
StoreTimer();
#endif

}

DWORD OccluderGetColor ( int r , int g , int b )
{
	return b | g << 8 | r << 16;
}

void OccluderPix( int x, int y , int col )
{
	//col = 255;
	Dot ( x , y , OccluderGetColor(col,col,col ) );
}

extern int min_clip_x, max_clip_x, min_clip_y, max_clip_y;

void CPUShiftXZ ( float x, float z )
{
	gCPUOccluderCamShiftX = x;
	gCPUOccluderCamShiftZ = z;
}

DB void begin()
{
	#ifdef _TIME_TAKEN_
	oldtime = timeGetTime();
	ResetTimer();
	ClearTimer();
	#endif

	// create local camera structure from real camera
	tagCameraData* camData = (tagCameraData*)GetCameraInternalData ( CameraIndex );
	if ( camData==NULL ) return;
	cam.pos.x = camData->vecPosition.x + gCPUOccluderCamShiftX;
	cam.pos.y = camData->vecPosition.y;
	cam.pos.z = camData->vecPosition.z + gCPUOccluderCamShiftZ;
	cam.dir.x = camData->fXRotate;
	cam.dir.y = camData->fYRotate;
	cam.dir.z = camData->fZRotate;
	cam.target.x = camData->vecLook.x;
	cam.target.y = camData->vecLook.y;
	cam.target.z = camData->vecLook.z;
	cam.near_clip_z = 10.0f;
	cam.fov = camData->fFOV;
	cam.aspect_ratio = camData->fAspect;
	cam.viewport_width = SCREEN_WIDTH;
	cam.viewport_height = SCREEN_WIDTH / cam.aspect_ratio;

	// clipping rectangle 
	min_clip_x = 0,                             
    max_clip_x = (SCREEN_WIDTH),
    min_clip_y = 0,
    max_clip_y = (cam.viewport_height);

	// generate camera matrix
	CameraBuildMatrix(&cam, CAM_ROT_SEQ_ZYX);

	#ifdef _TIME_TAKEN_
	UpdateTimer();
	#endif
}

float camOldFar = 0;

void CPU3DSetCameraFar ( float f )
{
	if ( camOldFar != f )
	{
		cam.far_clip_z	= f;
		camOldFar = f;
		occludeeMaxDistance = f;
	}
}

DB int DrawOccluder( int id )                                    
{

#ifdef _TIME_TAKEN_
ResetTimer();
#endif

	int cachedID = id-70000;

	if ( cachedID >= MAX_CACHED_OBJECTS || id < 70000 ) return 0;

	//================================================================================
	// has this object been cached already?
	if ( cachedPolys[cachedID] )
	{
		VECTOR4D world_pos = cachedPolys[cachedID]->world_pos;

		// step 1: transform the center of the object's bounding
		// sphere into camera space

		POINT4D sphere_pos; // hold result of transforming center of bounding sphere

		// transform point
		MatMulVector4D4X4(&world_pos, &cam.mcam, &sphere_pos);

		// step 2:  based on culling flags remove the object
		// cull only based on z clipping planes

		// lee - 161014 - smaller boxes too near to camera are clipped prematurely, so do NOT clip objects within range of camera
		if ( fabs ( sphere_pos.z ) > 200.0f )
		{
			// test far plane
			if ( ((sphere_pos.z - cachedPolys[cachedID]->max_radius) > cam.far_clip_z) || ((sphere_pos.z + cachedPolys[cachedID]->max_radius) < cam.near_clip_z) )
			   return 0;

			// cull only based on x clipping planes
			// test the the right and left clipping planes against the leftmost and rightmost
			// points of the bounding sphere
			float z_test = (0.5)*cam.viewplane_width*sphere_pos.z/cam.view_dist;
			if ( ((sphere_pos.x-cachedPolys[cachedID]->max_radius) > z_test)  || ((sphere_pos.x+cachedPolys[cachedID]->max_radius) < -z_test) )  
			   return 0;

			// cull only based on y clipping planes
			// test the the top and bottom clipping planes against the bottommost and topmost
			// points of the bounding sphere
			z_test = (0.5)*cam.viewplane_height*sphere_pos.z/cam.view_dist;
			if ( ((sphere_pos.y-cachedPolys[cachedID]->max_radius) > z_test)  || ((sphere_pos.y+cachedPolys[cachedID]->max_radius) < -z_test) )
			   return 0;
		}

		// eliminate occluder objects on the RT fly (find problem occluders)
		if ( cachedPolys[cachedID]->max_radius==-999999 )
			return 0;

		for ( unsigned int j = 0; j < cachedPolys[cachedID]->polys.size() ; j++ )
		{
			// now insert this polygon
			if (pRendlist->num_polys >= cpu3dMaxPolys)
			{	              
			   // the whole object didn't fit!
			   return 0;
			} // end if

			// step 1: copy polygon into next opening in polygon render list

			// point pointer to polygon structure
			pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

			// copy fields 
			pRendlist->poly_data[pRendlist->num_polys].state = POLY4DV2_STATE_ACTIVE;

			pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = cachedPolys[cachedID]->polys[j].vertexList[0].x;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = cachedPolys[cachedID]->polys[j].vertexList[0].y;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = cachedPolys[cachedID]->polys[j].vertexList[0].z;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

			pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = cachedPolys[cachedID]->polys[j].vertexList[1].x;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = cachedPolys[cachedID]->polys[j].vertexList[1].y;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = cachedPolys[cachedID]->polys[j].vertexList[1].z;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

			pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = cachedPolys[cachedID]->polys[j].vertexList[2].x;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = cachedPolys[cachedID]->polys[j].vertexList[2].y;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = cachedPolys[cachedID]->polys[j].vertexList[2].z;
			pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

			// first set this node to point to previous node and next node (null)
			pRendlist->poly_data[pRendlist->num_polys].next = NULL;
			pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

			// now set previous node to point to this node
			pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

			// increment number of polys in list
			pRendlist->num_polys++;
		}

		return 1;
	}
	// End of used cached object
	//================================================================================

	POLYF4DV2 tempPoly;

	pRendlist = &rend_list;
	BYTE* pVertex;
	GGVECTOR3 vecPosition;
	float fX, fY, fZ;

	sObject* p = GetObjectData ( id );
	sObject* p2;
	if (!p ) return 0;

	//sObject* pParent = p->pInstanceOfObject;

	if ( p->pInstanceOfObject )
		p2 = p->pInstanceOfObject;
	else
		p2 = p;

	VECTOR4D world_pos;
	world_pos.x = p->position.vecPosition.x;
	world_pos.y = p->position.vecPosition.y;
	world_pos.z = p->position.vecPosition.z;
	world_pos.w = 1.0f;

	float max_radius;
	max_radius = p2->collision.fScaledLargestRadius * 1.25f;

	// step 1: transform the center of the object's bounding
	// sphere into camera space

	POINT4D sphere_pos; // hold result of transforming center of bounding sphere

	// transform point
	MatMulVector4D4X4(&world_pos, &cam.mcam, &sphere_pos);

	// step 2:  based on culling flags remove the object
	// cull only based on z clipping planes

	// test far plane
	if ( ((sphere_pos.z - max_radius) > cam.far_clip_z) || ((sphere_pos.z + max_radius) < cam.near_clip_z) )
	   return 0;

	// cull only based on x clipping planes
	// test the the right and left clipping planes against the leftmost and rightmost
	// points of the bounding sphere
	float z_test = (0.5)*cam.viewplane_width*sphere_pos.z/cam.view_dist;

	if ( ((sphere_pos.x-max_radius) > z_test)  || ((sphere_pos.x+max_radius) < -z_test) )  
	   return 1;

	// cull only based on y clipping planes
	// test the the top and bottom clipping planes against the bottommost and topmost
	// points of the bounding sphere
	z_test = (0.5)*cam.viewplane_height*sphere_pos.z/cam.view_dist;

	if ( ((sphere_pos.y-max_radius) > z_test)  || ((sphere_pos.y+max_radius) < -z_test) )
	   return 1;

	// if not culled, then lets cache it as we add
	cachedPolys[cachedID] = new tPolyList();
	cachedPolys[cachedID]->max_radius = max_radius;
	cachedPolys[cachedID]->world_pos = world_pos;

	///============================

	for ( int i = 0; i < p2->iMeshCount; i++ )
	{
		if ( p2->ppMeshList [ i ]->iPrimitiveType == GGPT_TRIANGLESTRIP )
			ConvertLocalMeshToTriList ( p2->ppMeshList [ i ] );
	}

	int iLODHIGH = -1;
	int iLODMEDIUM = -1;
	int iLODLOW = -1;
	int frameWithLOD = -1;

	for ( DWORD iFrameScan = 0; iFrameScan < (DWORD)p2->iFrameCount; iFrameScan++ )
	{
		LPSTR pFrameName = p2->ppFrameList[iFrameScan]->szName;
		if ( stricmp ( pFrameName, "lod_0" ) == NULL ) iLODHIGH = iFrameScan;
		if ( stricmp ( pFrameName, "lod_1" ) == NULL ) iLODMEDIUM = iFrameScan;
		if ( stricmp ( pFrameName, "lod_2" ) == NULL ) iLODLOW = iFrameScan;
	}

	if ( iLODLOW > -1 )
		frameWithLOD = iLODLOW;
	else if ( iLODMEDIUM > -1 )
		frameWithLOD = iLODMEDIUM;
	else if ( iLODHIGH > -1 )
		frameWithLOD = iLODHIGH;

	int start = 0;
	int end = p2->iFrameCount;

	if ( frameWithLOD > -1 )
	{
		start = frameWithLOD;
		end = frameWithLOD+1;
	}

	GGMATRIX matWorld;

	for ( int i = start; i < end; i++ )
	{
		sMesh* pMesh = p2->ppFrameList [ i ]->pLOD[0];

		if ( !pMesh )
			pMesh = p2->ppFrameList [ i ]->pMesh;

		if ( !pMesh )
			continue;

		matWorld = p2->ppFrameList [ i ]->matCombined * p->position.matWorld;
		
		sOffsetMap	offsetMap;

		GetFVFOffsetMap ( pMesh, &offsetMap );

		for ( unsigned int j = 0; j < pMesh->dwIndexCount; j+=3 )
		{
			WORD* pIndex = pMesh->pIndices;

			// now insert this polygon
			if (pRendlist->num_polys >= cpu3dMaxPolys)
			{	              
			   // the whole object didn't fit!
			   return 0;
			} // end if

			// step 1: copy polygon into next opening in polygon render list

			// always 3 vertices
			pVertex = pMesh->pVertexData;

			fX  = *( ( float* ) pVertex + offsetMap.dwX + ( offsetMap.dwSize * (int)pIndex[j] ) );
			fY  = *( ( float* ) pVertex + offsetMap.dwY + ( offsetMap.dwSize * (int)pIndex[j] ) );
			fZ  = *( ( float* ) pVertex + offsetMap.dwZ + ( offsetMap.dwSize * (int)pIndex[j] ) );			
			vecPosition = GGVECTOR3 ( fX, fY, fZ );

			//World space
			GGVec3TransformCoord ( &vecPosition, &vecPosition, &matWorld );

			tempPoly.vertexList[0].x = vecPosition.x;
			tempPoly.vertexList[0].y = vecPosition.y;
			tempPoly.vertexList[0].z = vecPosition.z;

			fX  = *( ( float* ) pVertex + offsetMap.dwX + ( offsetMap.dwSize * (int)pIndex[j+1] ) );
			fY  = *( ( float* ) pVertex + offsetMap.dwY + ( offsetMap.dwSize * (int)pIndex[j+1] ) );
			fZ  = *( ( float* ) pVertex + offsetMap.dwZ + ( offsetMap.dwSize * (int)pIndex[j+1] ) );			
			vecPosition = GGVECTOR3 ( fX, fY, fZ );
			//World space
			GGVec3TransformCoord ( &vecPosition, &vecPosition, &matWorld );

			tempPoly.vertexList[1].x = vecPosition.x;
			tempPoly.vertexList[1].y = vecPosition.y;
			tempPoly.vertexList[1].z = vecPosition.z;

			fX  = *( ( float* ) pVertex + offsetMap.dwX + ( offsetMap.dwSize * (int)pIndex[j+2] ) );
			fY  = *( ( float* ) pVertex + offsetMap.dwY + ( offsetMap.dwSize * (int)pIndex[j+2] ) );
			fZ  = *( ( float* ) pVertex + offsetMap.dwZ + ( offsetMap.dwSize * (int)pIndex[j+2] ) );			
			vecPosition = GGVECTOR3 ( fX, fY, fZ );
			//World space
			GGVec3TransformCoord ( &vecPosition, &vecPosition, &matWorld );

			tempPoly.vertexList[2].x = vecPosition.x;
			tempPoly.vertexList[2].y = vecPosition.y;
			tempPoly.vertexList[2].z = vecPosition.z;

			cachedPolys[cachedID]->polys.push_back(tempPoly);
		} 

	}

	return 0;

#ifdef _TIME_TAKEN_
UpdateTimer();
#endif

}

// Calculates log2 of number.  
double OccluderLog2( double n )  
{  
    // log(n)/log(2) is log2.  
    return log( n ) / log( (double)2 );  
}

int lightmappedterrainoffset = MAXINT32;
int lightmappedobjectoffset = MAXINT32;

bool forceIsVisCheck;
float fLargeRadiusValue;

DB void OccludeeCheck( int id , bool isCharacter )                                    
{
	#ifdef _TIME_TAKEN_
	ResetTimer();
	#endif

	forceIsVisCheck = false;

	pRendlist = &rend_list;
	GGVECTOR3 vecPosition;

	// clear renderlist so we can use it for occludees
	pRendlist->num_polys = 0;

	sObject* p = GetObjectData ( id );
	sObject* p2;

	if (!p ) return;

	if ( p->pInstanceOfObject )
		p2 = p->pInstanceOfObject;
	else
		p2 = p;

	VECTOR4D world_pos;
	world_pos.x = p->position.vecPosition.x + p->collision.vecCentre.x;
	world_pos.y = p->position.vecPosition.y + p->collision.vecCentre.y;
	world_pos.z = p->position.vecPosition.z + p->collision.vecCentre.z;
	world_pos.w = 1.0f;

	float max_radius;
	float original_radius;
	max_radius = p->collision.fScaledLargestRadius * 1.25f;
	if ( p->collision.fLargestRadius > max_radius )
		max_radius = p->collision.fLargestRadius * 1.25f;
	else if ( p->collision.fScaledRadius > max_radius )
		max_radius = p->collision.fScaledRadius * 1.25f;

	// when characters ragdoll their radius is set to 0, so checking for 0 here and just make them visible since they dissapear when you move away anyway
	if ( p->collision.fScaledLargestRadius == 0 || p->collision.fScaledRadius == 0 )
	{
		p->dwCountdownToUniverseVisOff = 0;
		p->bUniverseVisible = true;	
		return;
	}

	original_radius = max_radius;

	//Stop characters dissapearing early and fix objects that dissapear from store
	if ( isCharacter )
		max_radius *= 3;
	else
		max_radius *= 2;
	
	float dist = sqrt( ((world_pos.x - cam.pos.x)*(world_pos.x - cam.pos.x)) + ((world_pos.y - cam.pos.y)*(world_pos.y - cam.pos.y)) +((world_pos.z - cam.pos.z)*(world_pos.z - cam.pos.z)) );

	// For stuff that is miles away
	if ( dist > occludeeMaxDistance && AggresiveMode > 0.1f )
	{
		p->bUniverseVisible = false;	
		return;
	}

#ifdef HIDE_SMALL_OBJECTS
	if ( AggresiveMode > 0.1f )
	{
		float hideSmallObjectsMultiplier = 1.0f / AggresiveMode;
		if ( max_radius < 5.0f && dist > (3000.0f * hideSmallObjectsMultiplier) )
		{
			p->bUniverseVisible = false;	
			return;
		}
		if ( max_radius < 100.0f && dist > (5000.0f * hideSmallObjectsMultiplier) )
		{
			p->bUniverseVisible = false;	
			return;
		}
		if ( max_radius < 5.0f && dist > (2800.0f * hideSmallObjectsMultiplier) && p->bUniverseVisible == false )
		{
			p->bUniverseVisible = false;	
			return;
		}
		if ( max_radius < 100.0f && dist > (4800.0f * hideSmallObjectsMultiplier) && p->bUniverseVisible == false )
		{
			p->bUniverseVisible = false;	
			return;
		}
	}
	#endif

	// added to allow very close objects (with large animation expanses like chainlink fence gates) to stay visible
	float fEpsilonMultiplierForCameraFustrumClip = 3.0f;
	fLargeRadiusValue = original_radius * fEpsilonMultiplierForCameraFustrumClip;
	if ( dist < fLargeRadiusValue )
	{
		p->dwCountdownToUniverseVisOff = 0;
		p->bUniverseVisible = true;	
		return;
	}

	// step 1: transform the center of the object's bounding
	// sphere into camera space
	{
		POINT4D sphere_pos; // hold result of transforming center of bounding sphere

		// transform point
		MatMulVector4D4X4(&world_pos, &cam.mcam, &sphere_pos);
	
		// step 2:  based on culling flags remove the object
		// cull only based on z clipping planes
		// test far plane
		if ( ((sphere_pos.z - max_radius) > cam.far_clip_z) || ((sphere_pos.z + max_radius) < cam.near_clip_z) )
		{
			p->bUniverseVisible = false;
			return;
		}

		// cull only based on x clipping planes
		// test the the right and left clipping planes against the leftmost and rightmost
		// points of the bounding sphere
		float z_test = (0.5)*cam.viewplane_width*sphere_pos.z/cam.view_dist;
		if ( ((sphere_pos.x-max_radius) > z_test)  || ((sphere_pos.x+max_radius) < -z_test) )  
		{
			p->bUniverseVisible = false;
			return;
		}

		// cull only based on y clipping planes
		// test the the top and bottom clipping planes against the bottommost and topmost
		// points of the bounding sphere
		z_test = (0.5)*cam.viewplane_height*sphere_pos.z/cam.view_dist;
		if ( ((sphere_pos.y-max_radius) > z_test)  || ((sphere_pos.y+max_radius) < -z_test) )
		{
			p->bUniverseVisible = false;
			return;
		}
	}

	float fWidth1  = p2->collision.vecMin.x * p->position.vecScale.x;
	float fHeight1 = p2->collision.vecMin.y * p->position.vecScale.y;
	float fDepth1  = p2->collision.vecMin.z * p->position.vecScale.z;
	float fWidth2  = p2->collision.vecMax.x * p->position.vecScale.x;
	float fHeight2 = p2->collision.vecMax.y * p->position.vecScale.y;
	float fDepth2  = p2->collision.vecMax.z * p->position.vecScale.z;

	// create the box, unrolled for speed
	// POLY 1
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================	
	// POLY 2
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 3
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 4
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 5
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 6
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 7
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 8
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 9
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 10
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 11
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 12
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================

   if ( pRendlist->num_polys > 0 )
   {
		pRendlist->poly_data[0].next = NULL;
		pRendlist->poly_data[0].prev = NULL;
   }

	// apply world to camera transform
	RenderListWorldToCamera(&rend_list, &cam);

	RenderListClipPolys(&rend_list, &cam, CLIP_POLY_X_PLANE | CLIP_POLY_Y_PLANE | CLIP_POLY_Z_PLANE );

	// apply camera to perspective transformation
	RenderListCameraToPerspective(&rend_list, &cam);

	// apply screen transform
	RenderListPerspectiveToScreen(&rend_list, &cam);

	float tl_x = 10000; 
	float tl_y = 10000;
	float br_x = -10000;
	float br_y = -10000;
	UINT depth = 0xffffffff;

	for ( int c = 0 ; c < rend_list.num_polys; c++ )
	{
		for ( int v = 0 ; v < 3 ; v++ )
		{
			// is this polygon valid?
			if ((rend_list.poly_ptrs[c]==NULL) || !(rend_list.poly_ptrs[c]->state & POLY4DV2_STATE_ACTIVE) ||
				 (rend_list.poly_ptrs[c]->state & POLY4DV2_STATE_CLIPPED ) )
				   continue; // move onto next poly

			if ( rend_list.poly_ptrs[c]->vertexList[v].x < tl_x ) tl_x = rend_list.poly_ptrs[c]->vertexList[v].x;
			if ( rend_list.poly_ptrs[c]->vertexList[v].y < tl_y ) tl_y = rend_list.poly_ptrs[c]->vertexList[v].y;
			if ( rend_list.poly_ptrs[c]->vertexList[v].x > br_x ) br_x = rend_list.poly_ptrs[c]->vertexList[v].x;
			if ( rend_list.poly_ptrs[c]->vertexList[v].y > br_y ) br_y = rend_list.poly_ptrs[c]->vertexList[v].y;
			if ( (UINT)rend_list.poly_ptrs[c]->vertexList[v].z < depth ) depth = (UINT)rend_list.poly_ptrs[c]->vertexList[v].z;
		}
	}

	if ( depth == 0xffffffff )
	{
		//p->bUniverseVisible = true;
		return;
	}

	#ifndef METHOD_TWO
    float W = sqrt( ((br_x - tl_x)*(br_x - tl_x)) + ((br_y - tl_y)*(br_y - tl_y)) ) * (SCREEN_WIDTH/2);

	//reject tiny objects
	if ( W < TINY_OBJECT_REJECT_SIZE )
	{
		p->bUniverseVisible = false;
	   return;
	}

	if ( tl_x < 0 ) tl_x = 0;
	if ( tl_y < 0 ) tl_y = 0;

	if ( tl_x > SCREEN_WIDTH-1 )
	{
		p->bUniverseVisible = false;
		return;
	}
	if ( tl_y > SCREEN_HEIGHT-1 )
	{
		p->bUniverseVisible = false;
		return;
	}

	if ( br_x < 0 ) 
	{
		p->bUniverseVisible = false;
		return;
	}
	if ( br_y < 0 )
	{
		p->bUniverseVisible = false;
		return;
	}

	if ( br_x > SCREEN_WIDTH-1 ) br_x = SCREEN_WIDTH-1;
	if ( br_y > SCREEN_HEIGHT-1 ) br_y = SCREEN_HEIGHT-1;
       
    int fLOD = ceil(log2( W )) - 12;
	if ( fLOD < 0 ) fLOD = 0;
	if ( fLOD > 8 ) fLOD = 8;

	float x,y;

	UINT zpixel;
	switch (fLOD )
	{
	case 0:
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[0][(int)x + (int)y*SCREEN_WIDTH]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[0][(int)x + (int)y*SCREEN_WIDTH]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[0][(int)x + (int)y*SCREEN_WIDTH]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[0][(int)x + (int)y*SCREEN_WIDTH]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

	} break;
	case 1: 
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[1][(int)(x/(SCREEN_WIDTH/128)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/128)]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[1][(int)(x/(SCREEN_WIDTH/128)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/128)]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[1][(int)(x/(SCREEN_WIDTH/128)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/128)]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[1][(int)(x/(SCREEN_WIDTH/128)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/128)]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

	} break;
	case 2:
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[2][(int)(x/(SCREEN_WIDTH/64)) + (int)(y/(SCREEN_HEIGHT/64))*(SCREEN_WIDTH/64)]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[2][(int)(x/(SCREEN_WIDTH/64)) + (int)(y/(SCREEN_HEIGHT/64))*(SCREEN_WIDTH/64)];  
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[2][(int)(x/(SCREEN_WIDTH/64)) + (int)(y/(SCREEN_HEIGHT/64))*(SCREEN_WIDTH/64)];  
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[2][(int)(x/(SCREEN_WIDTH/64)) + (int)(y/(SCREEN_HEIGHT/64))*(SCREEN_WIDTH/64)]; 
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}
		
	} break;
	case 3:
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[3][(int)(x/(SCREEN_WIDTH/32)) + (int)(y/(SCREEN_HEIGHT/32))*(SCREEN_WIDTH/32)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[3][(int)(x/(SCREEN_WIDTH/32)) + (int)(y/(SCREEN_HEIGHT/32))*(SCREEN_WIDTH/32)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[3][(int)(x/(SCREEN_WIDTH/32)) + (int)(y/(SCREEN_HEIGHT/32))*(SCREEN_WIDTH/32)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[3][(int)(x/(SCREEN_WIDTH/32)) + (int)(y/(SCREEN_HEIGHT/32))*(SCREEN_WIDTH/32)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}
		
	} break;
	case 4:
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[4][(int)(x/(SCREEN_WIDTH/16)) + (int)(y/(SCREEN_HEIGHT/16))*(SCREEN_WIDTH/16)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[4][(int)(x/(SCREEN_WIDTH/16)) + (int)(y/(SCREEN_HEIGHT/16))*(SCREEN_WIDTH/16)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[4][(int)(x/(SCREEN_WIDTH/16)) + (int)(y/(SCREEN_HEIGHT/16))*(SCREEN_WIDTH/16)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[4][(int)(x/(SCREEN_WIDTH/16)) + (int)(y/(SCREEN_HEIGHT/16))*(SCREEN_WIDTH/16)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}
		
	} break;
	case 5:
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[5][(int)(x/(SCREEN_WIDTH/8)) + (int)(y/(SCREEN_HEIGHT/8))*(SCREEN_WIDTH/8)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[5][(int)(x/(SCREEN_WIDTH/8)) + (int)(y/(SCREEN_HEIGHT/8))*(SCREEN_WIDTH/8)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[5][(int)(x/(SCREEN_WIDTH/8)) + (int)(y/(SCREEN_HEIGHT/8))*(SCREEN_WIDTH/8)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[5][(int)(x/(SCREEN_WIDTH/8)) + (int)(y/(SCREEN_HEIGHT/8))*(SCREEN_WIDTH/8)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}
		
	} break;
	case 6:
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[6][(int)(x/(SCREEN_WIDTH/4)) + (int)(y/(SCREEN_HEIGHT/4))*(SCREEN_WIDTH/4)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[6][(int)(x/(SCREEN_WIDTH/4)) + (int)(y/(SCREEN_HEIGHT/4))*(SCREEN_WIDTH/4)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[6][(int)(x/(SCREEN_WIDTH/4)) + (int)(y/(SCREEN_HEIGHT/4))*(SCREEN_WIDTH/4)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[6][(int)(x/(SCREEN_WIDTH/4)) + (int)(y/(SCREEN_HEIGHT/4))*(SCREEN_WIDTH/4)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}
		
	} break;
	case 7:
	{

		x = tl_x;
		y = tl_y;
		zpixel = mipMaps[7][(int)(x/(SCREEN_WIDTH/2)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/2)];
		if ( depth <  zpixel ) 
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = tl_y;
		zpixel = mipMaps[7][(int)(x/(SCREEN_WIDTH/2)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/2)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = tl_x;
		y = br_y;
		zpixel = mipMaps[7][(int)(x/(SCREEN_WIDTH/2)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/2)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

		x = br_x;
		y = br_y;
		zpixel = mipMaps[7][(int)(x/(SCREEN_WIDTH/2)) + (int)(y/(SCREEN_HEIGHT/2))*(SCREEN_WIDTH/2)];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}

	} break;
	case 8:
	{
		zpixel = zpixel = mipMaps[8][0];
		if ( depth <  zpixel )
		{
			p->bUniverseVisible = true;
		   return;
		}
	} break;
	}
	#else

	// Rejects ocludee if too small on screen
	UINT zpixel;
	if ( AggresiveMode > 0.1f )
	{
		float fScrAreaWidth = fabs(br_x - tl_x);
		float fScrAreaHeight = fabs(br_y - tl_y);
		float W = fScrAreaWidth; if ( fScrAreaHeight > W ) W = fScrAreaHeight;
		float smallsize = TINY_OBJECT_REJECT_SIZE;
		if ( isCharacter || dist < 2000 ) 
		{
			smallsize = TINY_CHARACTER_REJECT_SIZE;
			if ( dist < 2000 ) W *= 3;
		}
		else
		{
			smallsize *= smallDistanceMulti;
			if ( smallDistanceMulti > 1 ) forceIsVisCheck = true;
		}

		// reject tiny objects if actual scr area size is smaller than min size determed above
		if ( W < smallsize )
		{
			forceIsVisCheck = true;
			p->bUniverseVisible = false;
			return;
		}
	}

	if ( tl_x < 0 ) tl_x = 0;
	if ( tl_y < 0 ) tl_y = 0;

	if ( tl_x > SCREEN_WIDTH-1 )
	{
		p->bUniverseVisible = false;
		return;
	}
	if ( tl_y > cam.viewport_height-1 )
	{
		p->bUniverseVisible = false;
		return;
	}
	if ( br_x < 0 ) 
	{
		p->bUniverseVisible = false;
		return;
	}
	if ( br_y < 0 )
	{
		p->bUniverseVisible = false;
		return;
	}

	if ( br_x > SCREEN_WIDTH-1 ) br_x = SCREEN_WIDTH-1;
	if ( br_y > cam.viewport_height-1 ) br_y = cam.viewport_height-1;
	UINT   *zb_ptr    =  (UINT *)zbuffer.zbuffer;
	int step = 1;
	static float veryCloseResult = 0;
	for ( int y = tl_y ; y < br_y ; y += step )
	{
		int iScreenPitch = (int)y*SCREEN_WIDTH;
		for ( int x = tl_x; x < br_x ; x += step )
		{
			zpixel = (zb_ptr[x + iScreenPitch]);
			if ( zpixel == (16000 << FIXP16_SHIFT) )
			{
				// 190216 - no occluder pixel, just the zbuffer backdrop color
				p->dwCountdownToUniverseVisOff = 0;
				p->bUniverseVisible = true;
				return;
			}
			else
			{
				zpixel = UINT ( zpixel / 66191.36f );
				veryCloseResult = abs((float)depth-zpixel);
				// 220216 - threshold increases with distance as fixed float errors increase
				// when the occludee is also the occluder and attempts to occlude itself
				float fThreshold = depth / 100.0f;
				if ( fThreshold < 10.0f ) fThreshold = 10.0f;
				if ( depth < zpixel || veryCloseResult < fThreshold )
				{
					p->dwCountdownToUniverseVisOff = 0;
					p->bUniverseVisible = true;
					return;
				}
			}
		}
	}
	#endif

	// set a dountdown before bUniverseVisible is set to false
	// this is so the predictive occluder when t would otherwise have instantly hidden an ocludee in view
	// has to wait until you are definately not seeing it or have stopped and the velocity predictor resets to 0,0
	// which means the occluder has a grace period before items are hidden too quickly
	//p->bUniverseVisible = false;
	if ( p->dwCountdownToUniverseVisOff == 0 ) 
	{
		p->dwCountdownToUniverseVisOff = 50;
	}
	return;

	#ifdef _TIME_TAKEN_
	UpdateTimer();
	#endif
}

DB float OccluderCheck( int id )                           
{
#ifdef _TIME_TAKEN_
ResetTimer();
#endif

	pRendlist = &rend_list;
	GGVECTOR3 vecPosition;

	// clear renderlist so we can use it for occluders
	pRendlist->num_polys = 0;

	sObject* p = GetObjectData ( id );
	sObject* p2;

	if (!p ) return 0;

	if ( p->pInstanceOfObject )
		p2 = p->pInstanceOfObject;
	else
		p2 = p;

	VECTOR4D world_pos;
	world_pos.x = p->position.vecPosition.x;
	world_pos.y = p->position.vecPosition.y;
	world_pos.z = p->position.vecPosition.z;
	world_pos.w = 1.0f;

	float max_radius;
	float original_radius;
	max_radius = p2->collision.fScaledLargestRadius * 1.25f;
	if ( p2->collision.fScaledRadius > max_radius )
		max_radius = p2->collision.fScaledRadius * 1.25f;
	else if ( p->collision.fRadius > max_radius )
		max_radius = p->collision.fRadius * 1.25f;

	original_radius = max_radius;

	float dist = sqrt( ((world_pos.x - cam.pos.x)*(world_pos.x - cam.pos.x)) + ((world_pos.y - cam.pos.y)*(world_pos.y - cam.pos.y)) +((world_pos.z - cam.pos.z)*(world_pos.z - cam.pos.z)) );

#ifdef HIDE_SMALL_OBJECTS
	if ( max_radius < 5.0f && dist > 3000.0f )
	{
		return 0;
	}
	//if ( max_radius < 100.0f && dist > 5000.0f )
	if ( dist > 5000.0f )
	{
		return 0;
	}
#endif

	POINT4D sphere_pos; // hold result of transforming center of bounding sphere

	// transform point
	MatMulVector4D4X4(&world_pos, &cam.mcam, &sphere_pos);
	
	// step 2:  based on culling flags remove the object
	// cull only based on z clipping planes
	// test far plane

	if ( ((sphere_pos.z - max_radius) > cam.far_clip_z) || ((sphere_pos.z + max_radius) < cam.near_clip_z) )
	{
		return 0;
	}

	// cull only based on x clipping planes
	// test the the right and left clipping planes against the leftmost and rightmost
	// points of the bounding sphere
	float z_test = (0.5)*cam.viewplane_width*sphere_pos.z/cam.view_dist;

	if ( ((sphere_pos.x-max_radius) > z_test)  || ((sphere_pos.x+max_radius) < -z_test) )  
	{
		return 0;
	}

	// cull only based on y clipping planes
	// test the the top and bottom clipping planes against the bottommost and topmost
	// points of the bounding sphere
	z_test = (0.5)*cam.viewplane_height*sphere_pos.z/cam.view_dist;

	if ( ((sphere_pos.y-max_radius) > z_test)  || ((sphere_pos.y+max_radius) < -z_test) )
	{
		return 0;
	}

	//================================================================================
	float fWidth1  = p2->collision.vecMin.x;// / p->position.vecScale.x;
	float fHeight1 = p2->collision.vecMin.y;// / p->position.vecScale.y;
	float fDepth1  = p2->collision.vecMin.z;// / p->position.vecScale.z;
	float fWidth2  = p2->collision.vecMax.x;// * p->position.vecScale.x;
	float fHeight2 = p2->collision.vecMax.y;// * p->position.vecScale.y;
	float fDepth2  = p2->collision.vecMax.z;// * p->position.vecScale.z;

	//================================================================================				
	// create the box, unrolled for speed

	// POLY 1
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================	
	// POLY 2
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 3
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 4
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 5
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 6
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 7
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 8
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 9
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 10
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth2, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 11
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================
	// POLY 12
	// point pointer to polygon structure
	pRendlist->poly_ptrs[pRendlist->num_polys] = &pRendlist->poly_data[pRendlist->num_polys];

	// copy fields 
	pRendlist->poly_data[pRendlist->num_polys].state   = POLY4DV2_STATE_ACTIVE;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight1, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[0].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth2 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[1].w = 1;

	vecPosition = GGVECTOR3 ( fWidth1, fHeight2, fDepth1 );
	GGVec3TransformCoord ( &vecPosition, &vecPosition, &p->position.matWorld );

	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].x = vecPosition.x;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].y = vecPosition.y;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].z = vecPosition.z;
	pRendlist->poly_data[pRendlist->num_polys].vertexList[2].w = 1;

	// first set this node to point to previous node and next node (null)
	pRendlist->poly_data[pRendlist->num_polys].next = NULL;
	pRendlist->poly_data[pRendlist->num_polys].prev = &pRendlist->poly_data[pRendlist->num_polys-1];

	// now set previous node to point to this node
	pRendlist->poly_data[pRendlist->num_polys-1].next = &pRendlist->poly_data[pRendlist->num_polys];

	// increment number of polys in list
	pRendlist->num_polys++;
	//================================================================================

   if ( pRendlist->num_polys > 0 )
   {
		pRendlist->poly_data[0].next = NULL;
		pRendlist->poly_data[0].prev = NULL;
   }

	// apply world to camera transform
	RenderListWorldToCamera(&rend_list, &cam);


	RenderListClipPolys(&rend_list, &cam, CLIP_POLY_X_PLANE | CLIP_POLY_Y_PLANE | CLIP_POLY_Z_PLANE );

	// apply camera to perspective transformation
	RenderListCameraToPerspective(&rend_list, &cam);

	// apply screen transform
	RenderListPerspectiveToScreen(&rend_list, &cam);

	float tl_x = 10000; 
	float tl_y = 10000;
	float br_x = -10000;
	float br_y = -10000;
	UINT depth = 0xffffffff;

	for ( int c = 0 ; c < rend_list.num_polys; c++ )
	{
		for ( int v = 0 ; v < 3 ; v++ )
		{

			// is this polygon valid?
			if ((rend_list.poly_ptrs[c]==NULL) || !(rend_list.poly_ptrs[c]->state & POLY4DV2_STATE_ACTIVE) ||
				 (rend_list.poly_ptrs[c]->state & POLY4DV2_STATE_CLIPPED ) )
				   continue; // move onto next poly

			if ( rend_list.poly_ptrs[c]->vertexList[v].x < tl_x ) tl_x = rend_list.poly_ptrs[c]->vertexList[v].x;
			if ( rend_list.poly_ptrs[c]->vertexList[v].y < tl_y ) tl_y = rend_list.poly_ptrs[c]->vertexList[v].y;

			if ( rend_list.poly_ptrs[c]->vertexList[v].x > br_x ) br_x = rend_list.poly_ptrs[c]->vertexList[v].x;
			if ( rend_list.poly_ptrs[c]->vertexList[v].y > br_y ) br_y = rend_list.poly_ptrs[c]->vertexList[v].y;

			if ( (UINT)rend_list.poly_ptrs[c]->vertexList[v].z < depth ) depth = (UINT)rend_list.poly_ptrs[c]->vertexList[v].z;
		}
	}

	if ( tl_x == 10000 || tl_y == 10000 || br_x == -10000 || br_y == 10000 ) return 0;

	UINT zpixel;

	if ( tl_x > SCREEN_WIDTH-10 )
	{
		return 0;
	}
	if ( tl_y > cam.viewport_height-10 )
	{
		return 0;
	}

	if ( br_x < 10 ) 
	{
		return 0;
	}
	if ( br_y < 10 )
	{
		return 0;
	}

	if ( tl_x < 0 ) tl_x = 0;
	if ( tl_y < 0 ) tl_y = 0;

	if ( br_x >  SCREEN_WIDTH-1 ) br_x = SCREEN_WIDTH-1;
	if ( br_y > cam.viewport_height-1 ) br_y = cam.viewport_height-1;

	if ( tl_x > SCREEN_WIDTH * 0.7f ) return 0;
	if ( br_x < SCREEN_WIDTH * 0.3f ) return 0;

	if ( tl_y > cam.viewport_height * 0.7f ) return 0;
	if ( br_y < cam.viewport_height * 0.3f ) return 0;

	float W = (original_radius / dist) * 150.0f;

	return W;

#ifdef _TIME_TAKEN_
UpdateTimer();
#endif

}

void CPU3DAddOccluder ( int id )
{
	OccluderList.push_back ( id );
}

void CPU3DAddOccludee ( int id , bool isCharacter )
{
	OccludeeList.push_back ( id );
	OccludeeListNotVis.push_back ( 0 );
	OccludeeListIsVis.push_back ( 2 );
	OccludeeListIsCharacter.push_back ( isCharacter );
}

void CPU3DSetCameraIndex ( int id )
{
	CameraIndex = id;
}

void CPU3DClear()
{

	sObject* p;

	lightmappedterrainoffset = MAXINT32;
	lightmappedobjectoffset = MAXINT32;

	OccluderList.clear();
	OccludeeList.clear();
	OccludeeListNotVis.clear();
	OccludeeListIsVis.clear();
	OccludeeListIsCharacter.clear();

	OccluderListClosest.clear();
	OccluderListTemp.clear();
	OccluderListTempSize.clear();
	OccluderListDrawn.clear();

	smallDistanceMulti = 1;
	prevSmallDistanceMulti = 0;

	occluderBigVisualChange = false;

	OccluderQuickTimeDelay = 0;

	for ( int c = 0 ; c < MAX_CACHED_OBJECTS ; c++ )
	{
		if ( cachedPolys[c] != NULL )
		{
			delete cachedPolys[c];
			cachedPolys[c] = NULL;
		}
	}
}

void CPU3DSetPolyCount( int amount )
{
	if ( amount < 0 ) amount = 0;
	if ( amount > 100 ) amount = 100;

	AggresiveMode = amount / 100.0f;

	//Changed to use max polys always	
	if (amount == 0) {
		cpu3dMaxPolys = 0;
		g_enabeleverything = true;
	}
	else
		cpu3dMaxPolys = CPU_3D_MAX_POLY_SETTING;

	//Recent small distance multi to force a change
	smallDistanceMulti = 1.0f;
}

void CPU3DShow( int show )
{
	ShowZbuffer = show;
}

bool doneOcclude = false;

void Sync ( void );

float camax, camay, camaz;

void CPU3DOcclude ()
{
	if ( cpu3dMaxPolys <= 0 )
	{
		camOldFar = 0;
		return;
	}

	begin();

	tagCameraData* camData = (tagCameraData*)GetCameraInternalData ( CameraIndex );
	if ( camData==NULL ) return;
	camax = CameraAngleX(0); camay = CameraAngleY(0); camaz = CameraAngleZ(0);

	#define SHOWZBUFFER
	#ifdef SHOWZBUFFER
	if ( ShowZbuffer )
	{
		UINT   *zb_ptr    =  (UINT *)zbuffer.zbuffer;
		int max = 0;
		int min = 100000;

		#ifdef DRAW_ZBUFFER
		LockPixels();

		for (int y = 0; y < SCREEN_HEIGHT; y+=8)
		{
			for (int x = 0; x < SCREEN_WIDTH; x+=8)
			{
				UINT zpixel = zb_ptr[x + y*SCREEN_WIDTH];

				zpixel = UINT(zpixel / 66191.36f);

				#ifdef ZBUFFER_DRAW_MONO
				zpixel *= 0.1f;
				zpixel = 256 - zpixel;
				if ( zpixel > 0 )
					pix (x,y,255);
				#else
				zpixel *= 0.1f;
				zpixel = 256 - zpixel;
				if ( zpixel < min ) min = zpixel;
				if ( zpixel < 0 ) zpixel = 0;
				if ( zpixel > 256) zpixel = 256;
				if ( zpixel > max && zpixel < 59392 ) max = zpixel;
				if ( zpixel > 0 )
					OccluderPix ( x/8 , y/8 , (int)zpixel);
				#endif
			}
		}

		#ifndef METHOD_TWO
			// Draw Mip Maps
			int size = 128;
			int yOffset = 144;
			for ( int a = 1 ; a < 9 ; a++ )
			{
				for (int y = 0; y < size; y++)
				{
					for (int x = 0; x < size; x++)
					{
						UINT zpixel = mipMaps[a][x + y*size];
			#ifdef ZBUFFER_DRAW_MONO
						zpixel *= 0.1f;
						zpixel = 256 - zpixel;
						if ( zpixel > 0 )
							pix (x,y,255);
			#else
						zpixel *= 0.1f;
						zpixel = 256 - zpixel;
						if ( zpixel < min ) min = zpixel;
						if ( zpixel < 0 ) zpixel = 0;
						if ( zpixel > 256) zpixel = 256;
						if ( zpixel > max && zpixel < 59392 ) max = zpixel;
						if ( zpixel > 0 )
							pix ( x , y+yOffset , (int)zpixel);
			#endif
					}
				}

				yOffset += size+1;
				size /= 2;
			}
		#endif

		UnlockPixels();

		#endif
	}
	#endif
}

extern int iGridObjectStart, iGridObjectEnd;
extern int Timer(void);
extern float CameraPositionX ( int iID );
extern float CameraPositionY ( int iID );
extern float CameraPositionZ ( int iID );

extern float CameraAngleX ( int iID );
extern float CameraAngleY ( int iID );
extern float CameraAngleZ ( int iID );

extern bool OccluderCheckingForMultiplayer();

int occluderLastTime = 0;
float oldcamx, oldcamy, oldcamz, oldcamrx, oldcamry, oldcamrz;
int oldPolyCount = 0;
float multiPosX, multiPosY, multiPosZ;

float oldAggresiveMode = 0;

void CPU3DDoOcclude()
{
	rend_list.num_polys = 0;

	if ( !g_occluderOn )
	{
		sObject* p;
		for ( int c = 0 ; c < OccluderList.size() ; c++ )
		{
			p = GetObjectData ( OccluderList[c] );
			if ( p ) p->bUniverseVisible = true;
		}
		for ( int c = 0 ; c < OccludeeList.size() ; c++ )
		{
			p = GetObjectData ( OccludeeList[c] );
			if ( p ) p->bUniverseVisible = true;
		}

		doneOcclude = false;

		CPU3DClear();

		smallDistanceMulti = 1;
		prevSmallDistanceMulti = 0;

		smallDistanceMulti = 1;
		oldcamx = -9999;
		g_occluderf9Mode = false;

		if ( g_hOccluderEnd ) SetEvent ( g_hOccluderEnd );

		return;
	}

	//If f9 mode is active, show everything.
	if ( g_occluderf9Mode )
	{
		sObject* p;

		for ( int c = 0 ; c < OccluderList.size() ; c++ )
		{
			p = GetObjectData ( OccluderList[c] );
			if ( p ) p->bUniverseVisible = true;
		}

		for ( int c = 0 ; c < OccludeeList.size() ; c++ )
		{
			p = GetObjectData ( OccludeeList[c] );
			if ( p ) p->bUniverseVisible = true;
		}

		// 121115 - also need to show all GRASS if we ar ein F9 mode
		for ( int c = iGridObjectStart; c < iGridObjectEnd ; c++ )
		{
			if ( c > 0 )
			{
				p = GetObjectData ( c );
				if ( p ) p->bUniverseVisible = true;
			}
		}

		doneOcclude = false;
		oldcamx = -9999;
		smallDistanceMulti = 1;
		return;
	}

	if (cpu3dMaxPolys <= 0)
	{
		//force updated next time occluder is on
		oldcamx = -9999;

		if (doneOcclude)
		{
			sObject* p;

			for (int c = 0; c < OccludeeList.size(); c++)
			{
				p = GetObjectData(OccludeeList[c]);
				if (p) p->bUniverseVisible = true;
			}

			doneOcclude = false;
			smallDistanceMulti = 1;
		}

		if (g_enabeleverything)
		{
			//PE: If they do stuff in tab tab, we need to display all object again, if occluder is moved to 0.
			sObject* p;
			for (int c = 0; c < OccluderList.size(); c++)
			{
				p = GetObjectData(OccluderList[c]);
				if (p) p->bUniverseVisible = true;
			}
			for (int c = 0; c < OccludeeList.size(); c++)
			{
				p = GetObjectData(OccludeeList[c]);
				if (p) p->bUniverseVisible = true;
			}
			for (int c = iGridObjectStart; c < iGridObjectEnd; c++)
			{
				if (c > 0)
				{
					p = GetObjectData(c);
					if (p) p->bUniverseVisible = true;
				}
			}
			g_enabeleverything = false;
		}
		return;
	}

	// If the camera has been spun quite a bit, reset everything
	// This is to fix fast spinning around
	if ( abs(camay - oldCameraAngleY ) > 10 )
	{

		sObject* p;

		for ( int c = 0 ; c < OccludeeList.size() ; c++ )
		{
			OccludeeListNotVis[c] = 0;
		}
		OccluderQuickTimeDelay = Timer();
		occluderLastTime = 0;
	}

	if ( Timer() - OccluderQuickTimeDelay < 500 ) 
	{
		sObject* p;
		for ( int c = 0 ; c < OccludeeList.size() ; c++ )
		{
			OccludeeListNotVis[c] = 0;
		}
	}
	else
		OccluderQuickTimeDelay = 0;

	oldCameraAngleY = camay;
	

	if ( oldPolyCount != cpu3dMaxPolys ) oldcamx = -9999;
	oldPolyCount = cpu3dMaxPolys;

	//If the aggresive mode has changed, ensure we re calc visibility
	if ( AggresiveMode != oldAggresiveMode )
	{
		oldcamx = -9999;
	}
	oldAggresiveMode = AggresiveMode;

	howManyOccluders = OccluderList.size();
	howManyOccludersDrawn = 0;
	howManyOccludees = OccludeeList.size();
	howManyOccludeesHidden = 0;

	oldcamx = cam.pos.x; oldcamy = cam.pos.y; oldcamz = cam.pos.z;
	oldcamrx = camax; oldcamry = camay; oldcamrz = camaz;

	doneOcclude = true;

	OccluderListClosest.clear();
	OccluderListTemp.clear();
	OccluderListTempSize.clear();
	OccluderListDrawn.clear();

	float size;
	float biggest = 0;
	float biggestIndex;
	float dist;
	float closest;
	float closestIndex;
	sObject* p;
	VECTOR4D world_pos;	

	for ( int c = 0 ; c < OccluderList.size() ; c++ )
	{
		OccluderListTemp.push_back ( OccluderList[c] );
		OccluderListTempSize.push_back ( 99999 );
	}

	while ( OccluderListTemp.size() > 0 )
	{
		closest = 0xffffffff;
		biggest = -1;

		for ( int c = 0 ; c < OccluderListTemp.size() ; c++ )
		{
#ifndef USE_SCREEN_SPACE_OCCLUDER
			p = GetObjectData ( OccluderListTemp[c] );

			world_pos.x = p->position.vecPosition.x;
			world_pos.y = p->position.vecPosition.y;
			world_pos.z = p->position.vecPosition.z;

			dist = sqrt( ((world_pos.x - cam.pos.x)*(world_pos.x - cam.pos.x)) + ((world_pos.y - cam.pos.y)*(world_pos.y - cam.pos.y)) +((world_pos.z - cam.pos.z)*(world_pos.z - cam.pos.z)) );			

			if ( dist < closest )
			{
				closest = dist;
				closestIndex = c;
			}
#else
			if ( OccluderListTempSize[c] == 99999 )
				OccluderListTempSize[c] = OccluderCheck ( OccluderListTemp[c] );

			pRendlist->num_polys = 0;

			if (  OccluderListTemp[c] == 70002 ) trackingSize = OccluderListTempSize[c];

			size = OccluderListTempSize[c];

			if ( size > biggest )
			{
				biggest = size;
				biggestIndex = c;
			}
#endif

		}

#ifndef USE_SCREEN_SPACE_OCCLUDER
		OccluderListClosest.push_back ( OccluderListTemp[closestIndex] );

		p = GetObjectData ( OccluderListTemp[closestIndex] );
		if ( p ) p->bUniverseVisible = true;

		OccluderListTemp.erase (OccluderListTemp.begin()+closestIndex);
#else
		OccluderListClosest.push_back ( OccluderListTemp[biggestIndex] );

		p = GetObjectData ( OccluderListTemp[biggestIndex] );

		OccluderListTemp.erase (OccluderListTemp.begin()+biggestIndex);
#endif

	}

	for ( int c = 0 ; c < OccluderListClosest.size() ; c++ )
	{
		if ( DrawOccluder ( OccluderListClosest[c]) ) 
		{
			OccluderListDrawn.push_back ( OccluderListClosest[c]);
			howManyOccludersDrawn++;
		}
	}		

	Draw();	

	bool found = false;

	for ( int c = 0 ; c < OccludeeList.size() ; c++ )
	{
		found = false;
		if ( !found )
		{
			OccludeeCheck ( OccludeeList[c] , OccludeeListIsCharacter[c] );		

			// to stop flickering where something might go in and out of vis, objects must be not visible for 2 frames before they will get set to not visible
			p = GetObjectData ( OccludeeList[c] );
			if ( p )
			{
				// either the ocl is invisible but slowly ready to be visible thanks to the delayed OccludeeListNotVis action
				// OR the ocl is triggered for eventual hiding so should not fail this condition otherwise OccludeeListNotVis=0 would make it appear again
				if (  p->bUniverseVisible == false || p->dwCountdownToUniverseVisOff > 0 )
				{
					if ( OccludeeListNotVis[c] < 2 )//&& !forceIsVisCheck )
					{
						OccludeeListNotVis[c]++;
						p->bUniverseVisible = true;
					}
					else
					{
						howManyOccludeesHidden++;
					}
				}
				else // if the are visible, we set the counter back to 0
				{
					OccludeeListNotVis[c] = 0;
					if ( OccludeeListIsVis[c] < 1 )
					{
						OccludeeListIsVis[c]++;
						p->bUniverseVisible = false;
						howManyOccludeesHidden++;
					}
					else
					{						
						OccludeeListNotVis[c] = 0;
					}
				}
			}
		}
		else
		{
			p = GetObjectData ( OccludeeList[c] );
			if ( p )
			{
				OccludeeListNotVis[c] = 0;
				p->bUniverseVisible = true;
			}
		}
	}

	// 051016 - A special occludee system added which delays hiding ocludees until a countdown has completed
	for ( int c = 0 ; c < OccludeeList.size() ; c++ )
	{
		p = GetObjectData ( OccludeeList[c] );
		if ( p )
		{
			if ( p->dwCountdownToUniverseVisOff > 0 ) 
			{
				p->dwCountdownToUniverseVisOff = p->dwCountdownToUniverseVisOff - 1;
				if ( p->dwCountdownToUniverseVisOff == 0 )
				{
					p->bUniverseVisible = false;
				}
			}
		}
	}

	// Veg Check too
	for ( int c = iGridObjectStart; c < iGridObjectEnd ; c++ )
	{
		if ( c > 0 )
		{
			if ( ObjectExist ( c ) == 1 ) OccludeeCheck ( c , false );
		}
	}

	//If too many occludees drawn, cut the range down they will be drawn within
#ifdef SHOW_LESS_OCCLUDEES_ON_HIGH_PRIM_CALLS
	if ( g_pGlob->dwNumberOfPrimCalls > 500 && smallDistanceMulti != prevSmallDistanceMulti ) // 500
	{
		if  ( AggresiveMode == 0 ) 
			smallDistanceMulti = 0.0f;
		else
			smallDistanceMulti = (2.0f * AggresiveMode) + 1.0f;

		prevSmallDistanceMulti = smallDistanceMulti;
		oldcamx = -9999;
		//smallDistanceMulti = 3;
		multiPosX = cam.pos.x;
		multiPosY = cam.pos.y;
		multiPosZ = cam.pos.z;
	}
	// Once we have moved from the original position the draw calls dropped and the draw calls have dropped down
	// We can switch back to showing again
	else if ( g_pGlob->dwNumberOfPrimCalls < 200 && smallDistanceMulti != 1 ) //200
	{
		float dx2 = multiPosX - cam.pos.x;
		float dy2 = multiPosY - cam.pos.y;
		float dz2 = multiPosZ - cam.pos.z;
		float dist2 = sqrt( (dx2*dx2) + (dy2*dy2) + (dz2*dz2) );

		if ( dist2 > 3000 )
		{
			smallDistanceMulti = 1;
			prevSmallDistanceMulti = 0;
			oldcamx = -9999;
		}		
	}
#endif
}
