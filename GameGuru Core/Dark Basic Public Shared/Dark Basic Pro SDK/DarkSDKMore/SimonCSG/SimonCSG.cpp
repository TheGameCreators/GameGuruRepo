#include "stdafx.h"
#include "globstruct.h"
#include "DBOData.h"
#include <vector>
//#include <algorithm>
//#include <math.h>

enum AXISTYPE{
	X_AXIS,
	Y_AXIS,
	Z_AXIS
};

struct fPoint2D{
	float x,y;
};

struct dbp_vertex{
	float fX,fY,fZ;
	float fNX,fNY,fNZ;
	float fU,fV;
};

struct testTriangleStruct{
	int iMesh,iIndexStart,iAxisType;
	int iQuadLink;
	float fX1,fX2,fX3,fY1,fY2,fY3,fZ1,fZ2,fZ3;
	bool bDelete;
};

typedef sObject*(*t_dbGetObject)(long);
typedef bool(*t_dbConvertLocalMeshToTriList) ( sMesh *pMesh );
typedef bool(*t_dbGetFVFOffsetMap)( sMesh* pMesh, sOffsetMap* psOffsetMap );
typedef bool(*t_dbCreateNewObject)( int iID, LPSTR pName );  
typedef bool(*t_dbSetupMeshFVFData)(sMesh* pMesh, DWORD dwFVF, DWORD dwSize, DWORD dwIndexCount);
typedef void(*t_dbSetupStandardVertex)( DWORD dwFVF, BYTE* pVertex, int iOffset, float x, float y, float z, float nx, float ny, float nz, DWORD dwDiffuseColour, float tu, float tv );
typedef bool(*t_dbSetNewObjectFinalProperties)( int iID, float fRadius );
typedef void(*t_dbRefreshMeshShortList)(sMesh *pMesh);

typedef void(*t_dbYRotateCamera)(float);
typedef int(*t_dbObjectExist)(int);
typedef void(*t_dbDeleteObject)(int);
typedef void(*t_dbMakeObject)(int,int,int);
typedef void(*t_dbLockVertexDataForMesh)(int);
typedef void(*t_dbLockVertexDataForLimb)(int,int,int);
typedef void(*t_dbUnlockVertexData)(void);
typedef int(*t_dbGetVertexCount)(void);
typedef void(*t_dbSetVertexPosition)(int,float,float,float);
typedef DWORD(*t_dbGetVertexPositionX)(int);
typedef DWORD(*t_dbGetVertexPositionY)(int);
typedef DWORD(*t_dbGetVertexPositionZ)(int);
typedef void(*t_dbAddLimb)(int,int,int);
typedef void(*t_dbOffsetLimb)(int,int,float,float,float);
typedef void(*t_dbRotateLimb)(int,int,float,float,float);
typedef void(*t_dbScaleLimb)(int,int,float,float,float);
typedef int(*t_dbMeshExists)(int);
typedef void(*t_dbDeleteMesh)(int);
typedef void(*t_dbMakeMeshFromObject)(int,int);
typedef void(*t_dbTextureObject)(int,int,int);
typedef void(*t_dbSetObjectEffect)(int,int);
typedef void(*t_dbSetObjectMask)(int,int);
typedef void(*t_dbHideObject)(int);
typedef void(*t_dbShowObject)(int);
typedef void(*t_dbExcludeObjectOn)(int);
typedef void(*t_dbExcludeObjectOff)(int);
typedef int(*t_dbObjectVisible)(int);
typedef float(*t_dbObjectPositionX)(int);
typedef float(*t_dbObjectPositionY)(int);
typedef float(*t_dbObjectPositionZ)(int);
typedef void(*t_dbPositionObject)(int,float,float,float);
typedef int(*t_dbGetMemBlockByte)(int,int);
typedef void(*t_dbWriteMemBlockByte)(int,int,int);
typedef DWORD(*t_btGetGroundHeight)(unsigned int,float,float);
typedef int(*t_dbMemBlockExist)(int);

t_dbGetObject dbGetObject;
t_dbConvertLocalMeshToTriList dbConvertLocalMeshToTriList;
t_dbGetFVFOffsetMap dbGetFVFOffsetMap;
t_dbCreateNewObject dbCreateNewObject;
t_dbSetupMeshFVFData dbSetupMeshFVFData;
t_dbSetupStandardVertex dbSetupStandardVertex;
t_dbSetNewObjectFinalProperties dbSetNewObjectFinalProperties;
t_dbRefreshMeshShortList dbRefreshMeshShortList;

t_dbYRotateCamera dbYRotateCamera;
t_dbObjectExist dbObjectExist;
t_dbDeleteObject dbDeleteObject;
t_dbMakeObject dbMakeObject;
t_dbLockVertexDataForMesh dbLockVertexDataForMesh;
t_dbLockVertexDataForLimb dbLockVertexDataForLimb;
t_dbUnlockVertexData dbUnlockVertexData;
t_dbGetVertexCount dbGetVertexCount;
t_dbSetVertexPosition dbSetVertexPosition;
t_dbGetVertexPositionX dbGetVertexPositionX;
t_dbGetVertexPositionY dbGetVertexPositionY;
t_dbGetVertexPositionZ dbGetVertexPositionZ;
t_dbAddLimb dbAddLimb;
t_dbOffsetLimb dbOffsetLimb;
t_dbRotateLimb dbRotateLimb;
t_dbScaleLimb dbScaleLimb;
t_dbMeshExists dbMeshExists;
t_dbDeleteMesh dbDeleteMesh;
t_dbMakeMeshFromObject dbMakeMeshFromObject;
t_dbTextureObject dbTextureObject;
t_dbSetObjectEffect dbSetObjectEffect;
t_dbSetObjectMask dbSetObjectMask;
t_dbHideObject dbHideObject;
t_dbShowObject dbShowObject;
t_dbExcludeObjectOn dbExcludeObjectOn;
t_dbExcludeObjectOff dbExcludeObjectOff;
t_dbObjectVisible dbObjectVisible;
t_dbObjectPositionX dbObjectPositionX;
t_dbObjectPositionY dbObjectPositionY;
t_dbObjectPositionZ dbObjectPositionZ;
t_dbPositionObject dbPositionObject;
t_dbGetMemBlockByte dbGetMemBlockByte;
t_dbWriteMemBlockByte dbWriteMemBlockByte;
t_btGetGroundHeight btGetGroundHeight;
t_dbMemBlockExist dbMemBlockExist;

GlobStruct* g_pGlob = NULL;
// The size of the objects in the grid. While the grid is 50x50x50 in Reloaded, objects are 5x5x5
// and then scaled up by a factor of 10
const int iGridSize = 5;
// This is the maximum number of triangle that will be considered in face removal. For performance
// reasons, any objects submitted with more than 1000 triangles will still be processed, but the
// triangles beyond 1000 won't be considered
const int iMaxTriangles = 1000;
// A value used in vertex snapping to deal with imprecision in vertex positions in the models
const int iRoundPrecision = 10;
// These structures store the triangles that will be considered in face removal. Target is the
// model having faces removed, and subtract is the model it is compared with to remove faces
testTriangleStruct targetTriangle[iMaxTriangles];
testTriangleStruct subtractTriangle[iMaxTriangles];
// This is the structure which holds the current working model. When objects are combined together
// this is an expanding vector to which vertices are added until the model is ready for creation.
std::vector<dbp_vertex>modelVerts;

#define SIMONCSG __declspec ( dllexport )

SIMONCSG void Constructor ( void )
{
}

SIMONCSG void Destructor ( void )
{
}

SIMONCSG void ReceiveCoreDataPtr ( LPVOID pCore )
{
	// Get Core Data Pointer here
	g_pGlob = (GlobStruct*)pCore;
	dbGetObject=(t_dbGetObject)GetProcAddress( g_pGlob->g_Basic3D , "?GetObjectA@@YAPAUsObject@@H@Z" );
	dbConvertLocalMeshToTriList=(t_dbConvertLocalMeshToTriList)GetProcAddress( g_pGlob->g_Basic3D , "?ConvertLocalMeshToTriList@@YA_NPAUsMesh@@@Z" );
	dbGetFVFOffsetMap=(t_dbGetFVFOffsetMap)GetProcAddress( g_pGlob->g_Basic3D , "?GetFVFOffsetMap@@YA_NPAUsMesh@@PAUsOffsetMap@@@Z" );
	dbCreateNewObject=(t_dbCreateNewObject)GetProcAddress( g_pGlob->g_Basic3D , "?CreateNewObject@@YA_NHPAD@Z");
	dbSetupMeshFVFData=(t_dbSetupMeshFVFData)GetProcAddress( g_pGlob->g_Basic3D , "?SetupMeshFVFData@@YA_NPAUsMesh@@KKK@Z");
	dbSetupStandardVertex=(t_dbSetupStandardVertex)GetProcAddress( g_pGlob->g_Basic3D , "?SetupStandardVertex@@YA_NKPAEHMMMMMMKMM@Z");
	dbSetNewObjectFinalProperties=(t_dbSetNewObjectFinalProperties)GetProcAddress( g_pGlob->g_Basic3D , "?SetNewObjectFinalProperties@@YA_NHM@Z");
	dbRefreshMeshShortList=(t_dbRefreshMeshShortList)GetProcAddress( g_pGlob->g_Basic3D , "?RefreshMeshShortList@@YAXPAUsMesh@@@Z");

	dbYRotateCamera=(t_dbYRotateCamera)GetProcAddress(g_pGlob->g_Camera3D,"?SetYRotate@@YAXM@Z");
	dbObjectExist=(t_dbObjectExist)GetProcAddress(g_pGlob->g_Basic3D,"?GetExist@@YAHH@Z");
	dbDeleteObject=(t_dbDeleteObject)GetProcAddress(g_pGlob->g_Basic3D,"?DeleteObject@@YA_NH@Z");
	dbMakeObject=(t_dbMakeObject)GetProcAddress(g_pGlob->g_Basic3D,"?MakeObject@@YAXHHH@Z");
	dbLockVertexDataForMesh=(t_dbLockVertexDataForMesh)GetProcAddress(g_pGlob->g_Basic3D,"?LockVertexDataForMesh@@YAXH@Z");
	dbUnlockVertexData=(t_dbUnlockVertexData)GetProcAddress(g_pGlob->g_Basic3D,"?UnlockVertexData@@YAXXZ");
	dbGetVertexCount=(t_dbGetVertexCount)GetProcAddress(g_pGlob->g_Basic3D,"?GetVertexDataVertexCount@@YAHXZ");	
	dbAddLimb=(t_dbAddLimb)GetProcAddress(g_pGlob->g_Basic3D,"?AddLimb@@YAXHHH@Z");	
	dbOffsetLimb=(t_dbOffsetLimb)GetProcAddress(g_pGlob->g_Basic3D,"?OffsetLimb@@YAXHHMMM@Z");	
	dbRotateLimb=(t_dbRotateLimb)GetProcAddress(g_pGlob->g_Basic3D,"?RotateLimb@@YAXHHMMM@Z");	
	dbScaleLimb=(t_dbScaleLimb)GetProcAddress(g_pGlob->g_Basic3D,"?ScaleLimb@@YAXHHMMM@Z");	
	dbMeshExists=(t_dbMeshExists)GetProcAddress(g_pGlob->g_Basic3D,"?GetMeshExist@@YAHH@Z");	
	dbDeleteMesh=(t_dbDeleteMesh)GetProcAddress(g_pGlob->g_Basic3D,"?DeleteMesh@@YAXH@Z");	
	dbMakeMeshFromObject=(t_dbMakeMeshFromObject)GetProcAddress(g_pGlob->g_Basic3D,"?MakeMeshFromObject@@YAXHH@Z");	
	dbTextureObject=(t_dbTextureObject)GetProcAddress(g_pGlob->g_Basic3D,"?SetTextureStage@@YAXHHH@Z");	
	dbSetObjectEffect=(t_dbSetObjectEffect)GetProcAddress(g_pGlob->g_Basic3D,"?SetObjectEffect@@YAXHH@Z");	
	dbSetObjectMask=(t_dbSetObjectMask)GetProcAddress(g_pGlob->g_Basic3D,"?SetMask@@YAXHH@Z");	
	dbHideObject=(t_dbHideObject)GetProcAddress(g_pGlob->g_Basic3D,"?Hide@@YAXH@Z");
	dbLockVertexDataForLimb=(t_dbLockVertexDataForLimb)GetProcAddress(g_pGlob->g_Basic3D,"?LockVertexDataForLimbCore@@YAXHHH@Z");
	dbSetVertexPosition=(t_dbSetVertexPosition)GetProcAddress(g_pGlob->g_Basic3D,"?SetVertexDataPosition@@YAXHMMM@Z");
	dbGetVertexPositionX=(t_dbGetVertexPositionX)GetProcAddress(g_pGlob->g_Basic3D,"?GetVertexDataPositionX@@YAKH@Z");
	dbGetVertexPositionY=(t_dbGetVertexPositionY)GetProcAddress(g_pGlob->g_Basic3D,"?GetVertexDataPositionY@@YAKH@Z");
	dbGetVertexPositionZ=(t_dbGetVertexPositionZ)GetProcAddress(g_pGlob->g_Basic3D,"?GetVertexDataPositionZ@@YAKH@Z");
	dbShowObject=(t_dbShowObject)GetProcAddress(g_pGlob->g_Basic3D,"?Show@@YAXH@Z");
	dbExcludeObjectOn=(t_dbExcludeObjectOn)GetProcAddress(g_pGlob->g_Basic3D,"?ExcludeOn@@YAXH@Z");
	dbExcludeObjectOff=(t_dbExcludeObjectOff)GetProcAddress(g_pGlob->g_Basic3D,"?ExcludeOff@@YAXH@Z");
	dbObjectVisible=(t_dbObjectVisible)GetProcAddress(g_pGlob->g_Basic3D,"?GetVisible@@YAHH@Z");
	dbObjectPositionX=(t_dbObjectPositionX)GetProcAddress(g_pGlob->g_Basic3D,"?GetXPosition@@YAMH@Z");
	dbObjectPositionY=(t_dbObjectPositionY)GetProcAddress(g_pGlob->g_Basic3D,"?GetYPosition@@YAMH@Z");
	dbObjectPositionZ=(t_dbObjectPositionZ)GetProcAddress(g_pGlob->g_Basic3D,"?GetZPosition@@YAMH@Z");
	dbPositionObject=(t_dbPositionObject)GetProcAddress(g_pGlob->g_Basic3D,"?Position@@YAXHMMM@Z");
	dbGetMemBlockByte=(t_dbGetMemBlockByte)GetProcAddress(g_pGlob->g_Memblocks,"?ReadMemblockByte@@YAHHH@Z");
	dbWriteMemBlockByte=(t_dbWriteMemBlockByte)GetProcAddress(g_pGlob->g_Memblocks,"?WriteMemblockByte@@YAXHHH@Z");
	dbMemBlockExist=(t_dbMemBlockExist)GetProcAddress(g_pGlob->g_Memblocks,"?MemblockExist@@YAHH@Z");
}

// All models in the face removal process must be "vertex only". i.e. not be a list of vertices and
// indices which reference shared vertices. This function takes the current model and builds a model
// which is just a list of non-shared vertices which represent all the triangles, then rebuilds the
// model. It returns 0 for failure and 1 for success.
// This function must be called for any model that is going to be used in the conkit.
SIMONCSG int CSG_ConvertToVertexOnly(int iObject){

	sObject* pInputObject = dbGetObject(iObject); if (pInputObject == NULL) 
		return 0;
	
	
    // Make sure all meshes are triangle lists
	for ( int i = 0; i < pInputObject->iMeshCount; i++ )
		if ( pInputObject->ppMeshList [ i ]->iPrimitiveType != D3DPT_TRIANGLELIST )
			dbConvertLocalMeshToTriList ( pInputObject->ppMeshList [ i ] );
	
	// Cycle through meshes in model and count required vertices
	int iNumVerts = 0;
	for (int iMesh = 0; iMesh < pInputObject->iFrameCount; iMesh++ ){
		sMesh* pMesh = pInputObject->ppFrameList[iMesh]->pMesh; 
		if (pMesh == NULL) continue;
		iNumVerts += pMesh->dwIndexCount;
	}

	// Now store vertices
	dbp_vertex* vertex = new dbp_vertex[iNumVerts];
	for (int iMesh = 0; iMesh < pInputObject->iFrameCount; iMesh++ ){
		sMesh* pMesh = pInputObject->ppFrameList[iMesh]->pMesh; 
		if (pMesh == NULL) continue;
		sOffsetMap offsetMap; dbGetFVFOffsetMap (pMesh, &offsetMap);
		WORD* pIndex = pMesh->pIndices;
		BYTE* pVertex = pMesh->pVertexData;
		
		// Cycle through all triangles in this mesh
		for (DWORD iVert = 0; iVert < pMesh->dwIndexCount; iVert++ ){
			int iVertexOffset = offsetMap.dwSize * (int)pIndex[iVert];
			vertex[iVert].fX = floor(*( ( float* ) pVertex + offsetMap.dwX + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;;
			vertex[iVert].fY = floor(*( ( float* ) pVertex + offsetMap.dwY + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;;
			vertex[iVert].fZ = floor(*( ( float* ) pVertex + offsetMap.dwZ + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;;
			vertex[iVert].fNX = *( ( float* ) pVertex + offsetMap.dwNX + iVertexOffset);
			vertex[iVert].fNY = *( ( float* ) pVertex + offsetMap.dwNY + iVertexOffset);
			vertex[iVert].fNZ = *( ( float* ) pVertex + offsetMap.dwNZ + iVertexOffset);
			vertex[iVert].fU = *( ( float* ) pVertex + offsetMap.dwTU[0] + iVertexOffset);
			vertex[iVert].fV = *( ( float* ) pVertex + offsetMap.dwTV[0] + iVertexOffset);
		}
	}

	// Delete old object
	dbDeleteObject(iObject);

	// Create new object	
	if ( !dbCreateNewObject(iObject,"mesh")) 
		return 0;

	// setup general object data	
	sObject* pObject = dbGetObject(iObject); if (pObject == NULL) 
		return 0;
	sMesh* pMesh = pObject->pFrame->pMesh; if (pMesh == NULL) 
		return 0;

	// create vertex memory
	DWORD dwVertexCount = iNumVerts;
	DWORD dwIndexCount  = iNumVerts;									
	if (!dbSetupMeshFVFData(pMesh, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, dwVertexCount,dwIndexCount)) 
		return 0;
	
	// create vertices and indices
	for(int iVert = 0; iVert < (int)dwVertexCount; iVert++){
		dbp_vertex thisVert = vertex[iVert];
		dbSetupStandardVertex(pMesh->dwFVF,pMesh->pVertexData,iVert,
							  thisVert.fX,thisVert.fY,thisVert.fZ,
							  thisVert.fNX,thisVert.fNY,thisVert.fNZ,
							  D3DCOLOR_ARGB ( 255, 255, 255, 255 ),
							  thisVert.fU,thisVert.fV);
		pMesh->pIndices[iVert] = iVert;
	}
		
	// setup mesh drawing properties
	pMesh->iPrimitiveType   = D3DPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = dwVertexCount;
	pMesh->iDrawPrimitives  = dwVertexCount/3;

	// setup new object and introduce to buffers
	dbSetNewObjectFinalProperties(iObject,10);

	delete[] vertex;

	return 1;	
}

// This function translates a coordinate in jumps of 90 degrees around the Y axis, and also by their
// grid positions. This is a really fast way of converting conkit vertices and normals.
void CSG_TranslateCoord(float &fX, float &fY, float &fZ, int iGridX, int iGridY, int iGridZ, int iGridAng){
	if (iGridAng == 0 || iGridAng == 360){
		// No translation required
	}else if(iGridAng == 90){
		float fTempX = fX;
		fX = fZ;
		fZ = -fTempX;
	}else if(iGridAng == 180){		
		fX = -fX;
		fZ = -fZ;
	}else{
		float fTempX = fX;
		fX = -fZ;
		fZ = fTempX;
	}	
	if (iGridX != 0) fX += iGridX * iGridSize;
	if (iGridY != 0) fY += iGridY * iGridSize;
	if (iGridZ != 0) fZ += iGridZ * iGridSize;
}

// This function takes a DBP object pointer, mesh index and grid position/angle and then finds
// all the triangles that are axis aligned (i.e. flat against the X, Y or Z axis). It then 
// populates the passed in triangle vector with descriptions of these triangles.
// This will be called for the target model and the subtraction model and is the first step in
// face removal. 
int CSG_FillArrayWithAxisAlignedTriangles(sObject* pObject, int iMeshIndex, testTriangleStruct* triangles, int iStartTriangle,
										  int iGridX, int iGridY, int iGridZ, int iGridAng){
	
	float fX1,fY1,fZ1,fX2,fY2,fZ2,fX3,fY3,fZ3;
	int iNumTriangles = iStartTriangle;
	D3DXVECTOR3 vecPosition;

	sMesh* pMesh = pObject->ppFrameList[iMeshIndex]->pMesh; if (pMesh == NULL) return 0;	
	D3DXMATRIX matrix = pObject->ppFrameList[iMeshIndex]->matCombined * pObject->position.matWorld;	
	sOffsetMap offsetMap; dbGetFVFOffsetMap (pMesh, &offsetMap);
	BYTE* pVertex = pMesh->pVertexData;
	
	// Cycle through all triangles in this mesh
	for (DWORD iTriangle = 0; iTriangle < pMesh->dwVertexCount; iTriangle+=3 )
	{
		if (iNumTriangles >= iMaxTriangles) return iMaxTriangles;

		// Vertex 1
		int iVertexOffset = offsetMap.dwSize * iTriangle;
		fX1 = floor(*( ( float* ) pVertex + offsetMap.dwX + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		fY1 = floor(*( ( float* ) pVertex + offsetMap.dwY + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		fZ1 = floor(*( ( float* ) pVertex + offsetMap.dwZ + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		// Vertex 2
		iVertexOffset = offsetMap.dwSize * (iTriangle + 1);
		fX2 = floor(*( ( float* ) pVertex + offsetMap.dwX + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		fY2 = floor(*( ( float* ) pVertex + offsetMap.dwY + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		fZ2 = floor(*( ( float* ) pVertex + offsetMap.dwZ + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		// Vertex 3
		iVertexOffset = offsetMap.dwSize * (iTriangle + 2);
		fX3 = floor(*( ( float* ) pVertex + offsetMap.dwX + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		fY3 = floor(*( ( float* ) pVertex + offsetMap.dwY + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		fZ3 = floor(*( ( float* ) pVertex + offsetMap.dwZ + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
		// Does it lie on the X,Y or Z plane?
		if ((fX1 == fX2 && fX1 == fX3) ||
			(fY1 == fY2 && fY1 == fY3) ||
			(fZ1 == fZ2 && fZ1 == fZ3)){
			
			// We've found a triangle to consider. Now let's store this triangle with correctly translated 
			// vertex positions
			CSG_TranslateCoord(fX1,fY1,fZ1,iGridX,iGridY,iGridZ,iGridAng);
			CSG_TranslateCoord(fX2,fY2,fZ2,iGridX,iGridY,iGridZ,iGridAng);
			CSG_TranslateCoord(fX3,fY3,fZ3,iGridX,iGridY,iGridZ,iGridAng);
			
			testTriangleStruct newTriangle;
			newTriangle.bDelete = false;
			newTriangle.iMesh = iMeshIndex;
			newTriangle.iIndexStart = iTriangle;
			newTriangle.fX1 = fX1;
			newTriangle.fY1 = fY1;
			newTriangle.fZ1 = fZ1;
			newTriangle.fX2 = fX2;
			newTriangle.fY2 = fY2;
			newTriangle.fZ2 = fZ2;
			newTriangle.fX3 = fX3;
			newTriangle.fY3 = fY3;
			newTriangle.fZ3 = fZ3;
			
			// Store which axis this triangle is aligned with
			if (fX1 == fX2 && fX1 == fX3) newTriangle.iAxisType = X_AXIS;
			else if (fY1 == fY2 && fY1 == fY3) newTriangle.iAxisType = Y_AXIS;
			else newTriangle.iAxisType = Z_AXIS;
			
			triangles[iNumTriangles] = newTriangle;
			iNumTriangles++;			
		}
	}

	return iNumTriangles;
}

// A quick function to test if a vertex (fPoint2D) is at the same position as any of the other
// 3 passed in vertices (a triangle).
bool VertsMatch(fPoint2D p, fPoint2D v1, fPoint2D v2, fPoint2D v3){
	if (p.x == v1.x && p.y == v1.y) return true;
	if (p.x == v2.x && p.y == v2.y) return true;
	if (p.x == v3.x && p.y == v3.y) return true;
	return false;
}

// This function takes a list of axis aligned triangles (already calculated in 
// CSG_FillArrayWithAxisAlignedTriangles()), and for each triangle, attempts to find a partner
// triangle which would create a quad. This partnership is then stored in the triangle structure.
// This allows for better face-mating detection later, as a triangle from the target model may not
// lie wholey in the a triangle of the subtraction model, but it may lie inside a quad. 
// This could be expanded to consider more complex shapes (i.e. more than 2 triangles), so that face
// mating detection could be improved and more triangle removed.
void CSG_FindQuadLinks(testTriangleStruct* triangles, int iNumTriangles){
	fPoint2D triVert1,triVert2,triVert3,linkVert1,linkVert2,linkVert3;

	// Reset all quad links
	for (int iTri = 0; iTri < iNumTriangles; iTri++) triangles[iTri].iQuadLink = -1; // No link
	
	// See if each triangle in the list can link to another triangle to create a quad
	for (int iTri = 0; iTri < iNumTriangles; iTri++){		
		testTriangleStruct testTri = triangles[iTri];
		if (testTri.iQuadLink != -1) continue;
		for (int iTriLink = iTri+1; iTriLink < iNumTriangles; iTriLink++){
			testTriangleStruct linkTri = triangles[iTriLink];			
			if (linkTri.iQuadLink != -1) continue;
			if (testTri.iAxisType == linkTri.iAxisType){
				bool bTestLink = false;
				// Do faces potentially meet on the X-Axis?
				if (testTri.iAxisType == X_AXIS && testTri.fX1 == linkTri.fX1){
					triVert1.x = testTri.fY1; triVert1.y = testTri.fZ1;
					triVert2.x = testTri.fY2; triVert2.y = testTri.fZ2;
					triVert3.x = testTri.fY3; triVert3.y = testTri.fZ3;
					linkVert1.x = linkTri.fY1; linkVert1.y = linkTri.fZ1;
					linkVert2.x = linkTri.fY2; linkVert2.y = linkTri.fZ2;
					linkVert3.x = linkTri.fY3; linkVert3.y = linkTri.fZ3;
					bTestLink = true;
				} 
				// Do faces potentially meet on the Y-Axis?
				else if (testTri.iAxisType == Y_AXIS && testTri.fY1 == linkTri.fY1){
					triVert1.x = testTri.fX1; triVert1.y = testTri.fZ1;
					triVert2.x = testTri.fX2; triVert2.y = testTri.fZ2;
					triVert3.x = testTri.fX3; triVert3.y = testTri.fZ3;
					linkVert1.x = linkTri.fX1; linkVert1.y = linkTri.fZ1;
					linkVert2.x = linkTri.fX2; linkVert2.y = linkTri.fZ2;
					linkVert3.x = linkTri.fX3; linkVert3.y = linkTri.fZ3;
					bTestLink = true;
				}
				// Do faces potentially meet on the Z-Axis?
				else if (testTri.iAxisType == Z_AXIS && testTri.fZ1 == linkTri.fZ1){
					triVert1.x = testTri.fX1; triVert1.y = testTri.fY1;
					triVert2.x = testTri.fX2; triVert2.y = testTri.fY2;
					triVert3.x = testTri.fX3; triVert3.y = testTri.fY3;
					linkVert1.x = linkTri.fX1; linkVert1.y = linkTri.fY1;
					linkVert2.x = linkTri.fX2; linkVert2.y = linkTri.fY2;
					linkVert3.x = linkTri.fX3; linkVert3.y = linkTri.fY3;
					bTestLink = true;
				}
				// 2 verts must match their position for this to be a quad
				if (bTestLink){
					int iMatchCount = 0;
					if (VertsMatch(triVert1,linkVert1,linkVert2,linkVert3)) iMatchCount++;
					if (VertsMatch(triVert2,linkVert1,linkVert2,linkVert3)) iMatchCount++;
					if (VertsMatch(triVert3,linkVert1,linkVert2,linkVert3)) iMatchCount++;
					if (iMatchCount == 2){
						// Quad found. Link these two tris together
						triangles[iTri].iQuadLink = iTriLink;
						triangles[iTriLink].iQuadLink = iTri;
						break;
					}
				}			
			}
		}
	}
}

// The following two functions were taken from the net. They work out if a 2D point lies
// within a 2D triangle
float sign(fPoint2D p1, fPoint2D p2, fPoint2D p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle(fPoint2D pt, fPoint2D v1, fPoint2D v2, fPoint2D v3)
{
	if (pt.x == v1.x && pt.y == v1.y) return true;
	if (pt.x == v2.x && pt.y == v2.y) return true;
	if (pt.x == v3.x && pt.y == v3.y) return true;
	float f1 = sign(pt, v1, v2);
	float f2 = sign(pt, v2, v3);
	float f3 = sign(pt, v3, v1);
	bool b1 = f1 <= 0.0f;
	bool b2 = f2 <= 0.0f;
	bool b3 = f3 <= 0.0f;

	return ((b1 == b2) && (b2 == b3));
}

// This function takes a target DBP object number and it's conkit grid position and angle, and a 
// subtraction DBP object and it's conkit grid position and angle, then removes the faces from the
// target object that mate with the subtract object. It is the main step in face removal and is called
// from the conkit for each part/feature to trim faces before they are combined.
SIMONCSG void CSG_RemoveFaces(int iTarget, int iTargetX, int iTargetY, int iTargetZ, int iTargetAng, 
							  int iSubtract, int iSubtractX, int iSubtractY, int iSubtractZ, int iSubtractAng){
	
	D3DXMATRIX matTarget, matSubtract;
	sOffsetMap mapTarget, mapSubtract;
	sMesh* pTargetMesh;
	sMesh* pSubtractMesh;
	BYTE* pTargetVertex;
	BYTE* pSubtractVertex;
	WORD* pTargetIndex;
	WORD* pSubtractIndex;
	int iTargetTriangleCount = 0;
	int iSubtractTriangleCount = 0;
	
	sObject* pTarget = dbGetObject(iTarget); if (pTarget == NULL) return;
	sObject* pSubtract = dbGetObject(iSubtract); if (pSubtract == NULL) return;

	// Cycle through meshes in target model and cache/translate all the triangles we will consider
	for (int iTargetMesh = 0; iTargetMesh < pTarget->iFrameCount; iTargetMesh++ ){
		// Create an array of axis aligned, translated triangles
		iTargetTriangleCount = CSG_FillArrayWithAxisAlignedTriangles(pTarget,iTargetMesh,targetTriangle,iTargetTriangleCount,
																	 iTargetX,iTargetY,iTargetZ,iTargetAng);
	}

	// Cycle through meshes in subtraction model and cache/translate all the triangles we will consider
	for (int iSubtractMesh = 0; iSubtractMesh < pSubtract->iFrameCount; iSubtractMesh++ ){
		// Create an array of axis aligned, translated triangles
		iSubtractTriangleCount = CSG_FillArrayWithAxisAlignedTriangles(pSubtract,iSubtractMesh,subtractTriangle,iSubtractTriangleCount,
																	   iSubtractX,iSubtractY,iSubtractZ,iSubtractAng);
	}
	// Now link triangles in the subtract model together into quads
	CSG_FindQuadLinks(subtractTriangle,iSubtractTriangleCount);

	// At this point we have two arrays, each storing all the axis aligned triangles that can be used 
	// for deletion checking. Now we need to loop through both arrays and compare triangles to see if 
	// any should be removed from the target object.
	bool bTriangleDeleted = false;
	testTriangleStruct testTargetTri,testSubtractTri,testQuadTri;
	fPoint2D targetVert1,targetVert2,targetVert3,subtractVert1,subtractVert2,subtractVert3,quadVert1,quadVert2,quadVert3;
	for (int iTargetTri = 0; iTargetTri < iTargetTriangleCount; iTargetTri++){
		testTargetTri = targetTriangle[iTargetTri];		
		for (int iSubtractTri = 0; iSubtractTri < iSubtractTriangleCount; iSubtractTri++){
			testSubtractTri = subtractTriangle[iSubtractTri];
			bool bCheckQuad = (testSubtractTri.iQuadLink != -1);
			if (bCheckQuad) testQuadTri = subtractTriangle[testSubtractTri.iQuadLink];
			// Are these triangles on the same axis?
			if (testTargetTri.iAxisType == testSubtractTri.iAxisType){				
				bool bDoTest = false;				
				// Do faces potentially mate on the X-Axis?
				if (testTargetTri.iAxisType == X_AXIS && testTargetTri.fX1 == testSubtractTri.fX1){
					targetVert1.x = testTargetTri.fY1; targetVert1.y = testTargetTri.fZ1;
					targetVert2.x = testTargetTri.fY2; targetVert2.y = testTargetTri.fZ2;
					targetVert3.x = testTargetTri.fY3; targetVert3.y = testTargetTri.fZ3;
					subtractVert1.x = testSubtractTri.fY1; subtractVert1.y = testSubtractTri.fZ1;
					subtractVert2.x = testSubtractTri.fY2; subtractVert2.y = testSubtractTri.fZ2;
					subtractVert3.x = testSubtractTri.fY3; subtractVert3.y = testSubtractTri.fZ3;
					if (bCheckQuad){
						quadVert1.x = testQuadTri.fY1; quadVert1.y = testQuadTri.fZ1;
						quadVert2.x = testQuadTri.fY2; quadVert2.y = testQuadTri.fZ2;
						quadVert3.x = testQuadTri.fY3; quadVert3.y = testQuadTri.fZ3;
					}
					bDoTest = true;
				} 
				// Do faces potentially mate on the Y-Axis?
				else if (testTargetTri.iAxisType == Y_AXIS && testTargetTri.fY1 == testSubtractTri.fY1){
					targetVert1.x = testTargetTri.fX1; targetVert1.y = testTargetTri.fZ1;
					targetVert2.x = testTargetTri.fX2; targetVert2.y = testTargetTri.fZ2;
					targetVert3.x = testTargetTri.fX3; targetVert3.y = testTargetTri.fZ3;
					subtractVert1.x = testSubtractTri.fX1; subtractVert1.y = testSubtractTri.fZ1;
					subtractVert2.x = testSubtractTri.fX2; subtractVert2.y = testSubtractTri.fZ2;
					subtractVert3.x = testSubtractTri.fX3; subtractVert3.y = testSubtractTri.fZ3;
					if (bCheckQuad){
						quadVert1.x = testQuadTri.fX1; quadVert1.y = testQuadTri.fZ1;
						quadVert2.x = testQuadTri.fX2; quadVert2.y = testQuadTri.fZ2;
						quadVert3.x = testQuadTri.fX3; quadVert3.y = testQuadTri.fZ3;
					}
					bDoTest = true;
				}
				// Do faces potentially mate on the Z-Axis?
				else if (testTargetTri.iAxisType == Z_AXIS && testTargetTri.fZ1 == testSubtractTri.fZ1){
					targetVert1.x = testTargetTri.fX1; targetVert1.y = testTargetTri.fY1;
					targetVert2.x = testTargetTri.fX2; targetVert2.y = testTargetTri.fY2;
					targetVert3.x = testTargetTri.fX3; targetVert3.y = testTargetTri.fY3;
					subtractVert1.x = testSubtractTri.fX1; subtractVert1.y = testSubtractTri.fY1;
					subtractVert2.x = testSubtractTri.fX2; subtractVert2.y = testSubtractTri.fY2;
					subtractVert3.x = testSubtractTri.fX3; subtractVert3.y = testSubtractTri.fY3;
					if (bCheckQuad){
						quadVert1.x = testQuadTri.fX1; quadVert1.y = testQuadTri.fY1;
						quadVert2.x = testQuadTri.fX2; quadVert2.y = testQuadTri.fY2;
						quadVert3.x = testQuadTri.fX3; quadVert3.y = testQuadTri.fY3;
					}
					bDoTest = true;
				}	
				// Are all 3 vertices in the target triangle inside the subtraction triangle
				// or in it's neighbouring triangle if their is a quad to test
				if (bDoTest){
					bool bPoint1 = PointInTriangle(targetVert1,subtractVert1,subtractVert2,subtractVert3)
						|| (bCheckQuad && PointInTriangle(targetVert1,quadVert1,quadVert2,quadVert3));
					bool bPoint2 = PointInTriangle(targetVert2,subtractVert1,subtractVert2,subtractVert3)
						|| (bCheckQuad && PointInTriangle(targetVert2,quadVert1,quadVert2,quadVert3));
					bool bPoint3 = PointInTriangle(targetVert3,subtractVert1,subtractVert2,subtractVert3)
						|| (bCheckQuad && PointInTriangle(targetVert3,quadVert1,quadVert2,quadVert3));
					if (bPoint1 && bPoint2 && bPoint3){
						targetTriangle[iTargetTri].bDelete = true;
						bTriangleDeleted = true;
						break;
					}
				}
			}
		}
	}

	if (bTriangleDeleted == false) return; // Nothing more to do as no triangles can be deleted

	// Triangles need to be deleted. For each triangle marked for deletion, remove it from the model by
	// overwriting it's vertex data with that of the triangle last in the sequence, then reduce the size
	// of the sequence	
	for (int iTargetTri = 0; iTargetTri < iTargetTriangleCount; iTargetTri++){
		testTriangleStruct testTargetTri = targetTriangle[iTargetTri];	
		if (testTargetTri.bDelete){
			pTargetMesh = pTarget->ppFrameList[testTargetTri.iMesh]->pMesh;			
			// dont shuffle vert data if the last triangle!
			if (testTargetTri.iIndexStart < pTargetMesh->dwVertexCount - 3){
				dbGetFVFOffsetMap(pTargetMesh, &mapTarget);
				pTargetVertex = pTargetMesh->pVertexData;
				pTargetIndex = pTargetMesh->pIndices;
				int iVertexWriteOffset = mapTarget.dwSize * 4 * testTargetTri.iIndexStart;
				int iVertexReadOffset = mapTarget.dwSize * 4 * (pTargetMesh->dwVertexCount - 3);
				memcpy(pTargetVertex + iVertexWriteOffset, pTargetVertex + iVertexReadOffset, mapTarget.dwSize * 12);
				// Index data not required
				//int iIndexWriteOffset = testTargetTri.iIndexStart * 2; 
				//int iIndexReadOffset = (pTargetMesh->dwIndexCount - 3) * 2;
				//memcpy(pTargetIndex + iIndexWriteOffset, pTargetIndex + iIndexReadOffset, 6);
			}
						
			pTargetMesh->dwVertexCount -= 3;
			pTargetMesh->dwIndexCount -= 3;		
			pTargetMesh->iDrawPrimitives  = pTargetMesh->dwVertexCount/3;			
			pTargetMesh->iDrawVertexCount -= 3;
			
			// The triangle at the end has now been moved to overwrite the triangle we deleted. 
			// If this triangle exists in the delete list, we'll need to update it's index
			// position to point to where it was moved to
			for (int iMoveTri = 0; iMoveTri < iTargetTriangleCount; iMoveTri++){
				if (targetTriangle[iMoveTri].iIndexStart == pTargetMesh->dwVertexCount){
					targetTriangle[iMoveTri].iIndexStart = testTargetTri.iIndexStart;
					break;
				}
			}
		}
	}	

	// Trigger refresh of meshes
	for (int iTargetMesh = 0; iTargetMesh < pTarget->iFrameCount; iTargetMesh++ ){
		pTargetMesh = pTarget->ppFrameList[testTargetTri.iMesh]->pMesh;
		if (pTargetMesh == NULL) continue;
		pTargetMesh->bVBRefreshRequired = true;
		dbRefreshMeshShortList(pTargetMesh);
	}
}

// The final step in the conkit consolidation process is to combine all the objects together into
// larger meshes. The first step is normally to clear the modelVerts vector which will store all the
// data for the object we're about to make
SIMONCSG void CSG_CombineClear(void){
	modelVerts.clear();	
}

// The next step in the consolidation process is to start adding models to the modelVerts vector.
// This submitted model will be a model that has already had faces removed by CSG_RemoveFaces() 
SIMONCSG void CSG_CombineAdd(int iObjectNumber, int iGridX, int iGridY, int iGridZ, int iGridAng){
	sObject* pModel = dbGetObject(iObjectNumber); if (pModel == NULL) return;
	for (int iMesh = 0; iMesh < pModel->iFrameCount; iMesh++ ){
		sMesh* pMesh = pModel->ppFrameList[iMesh]->pMesh;
		if (pMesh == NULL) continue;
		sOffsetMap offsetMap; dbGetFVFOffsetMap (pMesh, &offsetMap);
		BYTE* pVertex = pMesh->pVertexData;
		for (DWORD iVert = 0; iVert < pMesh->dwVertexCount; iVert++){
			dbp_vertex addVert;
			int iVertexOffset = offsetMap.dwSize * iVert;
			addVert.fX = floor(*( ( float* ) pVertex + offsetMap.dwX + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
			addVert.fY = floor(*( ( float* ) pVertex + offsetMap.dwY + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
			addVert.fZ = floor(*( ( float* ) pVertex + offsetMap.dwZ + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
			CSG_TranslateCoord(addVert.fX,addVert.fY,addVert.fZ,iGridX,iGridY,iGridZ,iGridAng);
			addVert.fNX = floor(*( ( float* ) pVertex + offsetMap.dwNX + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
			addVert.fNY = floor(*( ( float* ) pVertex + offsetMap.dwNY + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
			addVert.fNZ = floor(*( ( float* ) pVertex + offsetMap.dwNZ + iVertexOffset)*iRoundPrecision + 0.5)/iRoundPrecision;
			CSG_TranslateCoord(addVert.fNX,addVert.fNY,addVert.fNZ,0,0,0,iGridAng);
			addVert.fU = *( ( float* ) pVertex + offsetMap.dwTU[0] + iVertexOffset);
			addVert.fV = *( ( float* ) pVertex + offsetMap.dwTV[0] + iVertexOffset);
			modelVerts.push_back(addVert);
		}
	}
}

// The final step in the consolidation process is to build a new model from all of the vertex data
// stored in modelVerts. This is our final consolidated mesh. 
SIMONCSG void CSG_CombineMake(int iObjectNumber){
	if (dbObjectExist(iObjectNumber)) dbDeleteObject(iObjectNumber);
	if ( !dbCreateNewObject(iObjectNumber,"mesh")) return;

	// setup general object data	
	sObject* pObject = dbGetObject(iObjectNumber); if (pObject == NULL) return;
	sMesh* pMesh = pObject->pFrame->pMesh; if (pMesh == NULL) return;

	// create vertex memory
	DWORD dwVertexCount = modelVerts.size(); if (dwVertexCount < 3) return;
	DWORD dwIndexCount  = dwVertexCount;									
	
	if (!dbSetupMeshFVFData(pMesh, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, dwVertexCount,dwIndexCount)) return;
	
	// create vertices and indices
	for(int iVert = 0; iVert < (int)dwVertexCount; iVert++){
		dbp_vertex thisVert = modelVerts[iVert];
		dbSetupStandardVertex(pMesh->dwFVF,pMesh->pVertexData,iVert,
							  thisVert.fX,thisVert.fY,thisVert.fZ,
							  thisVert.fNX,thisVert.fNY,thisVert.fNZ,
							  D3DCOLOR_ARGB ( 255, 255, 255, 255 ),
							  thisVert.fU,thisVert.fV);
		pMesh->pIndices[iVert] = iVert;
	}
		
	// setup mesh drawing properties
	pMesh->iPrimitiveType   = D3DPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = dwVertexCount;
	pMesh->iDrawPrimitives  = dwVertexCount/3;

	// setup new object and introduce to buffers
	dbSetNewObjectFinalProperties(iObjectNumber,10);
}