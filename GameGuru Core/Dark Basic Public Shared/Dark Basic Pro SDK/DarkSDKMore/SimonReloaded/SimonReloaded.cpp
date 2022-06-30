#include "stdafx.h"
#include ".\globstruct.h"
#include "SimonReloaded.h"
#include "CObjectsC.h"
#include "CMemblocks.h"
#include "BlitzTerrain.h"


#define GRASSALPHACLIP 0.41f

extern GlobStruct* g_pGlob;

int iGridSize,iVegAreaWidth,iVegHalfAreaWidth,iVegAreaRad,iGridDimension;
float fVegWidth,fVegHeight;
int iGrassClumpVerts,iVegPerMesh,iBuildMesh,iCameraMask;
float fOldViewPointX,fOldViewPointZ;
int iOldLeftEdge,iOldRightEdge,iOldFrontEdge,iOldBackEdge;
int iGridObjectStart, iGridObjectEnd, iGrassObj, iGrassImg, iShadowImg, iShader, iGrassMemBlock, iGrassMemBlockRes;
int iPBRAGEDImg, iPBRSpecImg, iPBRCubeImg, iPBRCurveImg;
bool bResourcesSet,bGridMade;
int iLeftVert,iRightVert,iFrontVert,iBackVert,iTopVert,iBottomVert;
bool **bGridExist;
int iGrassMemblockThreshhold;
float fWorldSize;
bool bDisplayBelowWater;

void InfiniteVegetationConstructor ( void )
{
	bResourcesSet = false;
	bGridMade = false;
	iGridObjectStart = 0;
	iGridObjectEnd = 0;
	iGrassObj = 0;
	iGrassImg = 0;
	iShadowImg = 0;
	iShader = 0;
	iGrassMemBlock = 0;
	iGrassMemBlockRes = 0;
	iBuildMesh = 0;
	iCameraMask = 0;
	iGridSize = 0;
	iVegAreaWidth = 0;
	iVegHalfAreaWidth = 0;
	iVegAreaRad = 0;
	iGrassClumpVerts = 0;
	iVegPerMesh = 0;	
	fVegWidth = 0;
	fVegHeight = 0;
	iGrassMemblockThreshhold = 74; // Grass not drawn when memblock value <= this value
	fWorldSize = 51200.0f;
}
 void InfiniteVegetationDestructor ( void )
{
	// Free memory here
}
void InfiniteVegetationReceiveCoreDataPtr ( LPVOID pCore )
{
}

float floatFromBits( DWORD const bits )
{
    return *reinterpret_cast< float const* >( &bits );
}

void SetPBRResourceValues ( int iPBRAGED, int iPBRSpec, int iPBRCube, int iPBRCurve )
{ 
	iPBRAGEDImg = iPBRAGED;
	iPBRSpecImg = iPBRSpec;
	iPBRCubeImg = iPBRCube;
	iPBRCurveImg = iPBRCurve;
}

void SetResourceValues(int iGrassObjIN, int iGridObjectStartIN, int iGrassImgIN, int iShadowImgIN, 
									 int iBuildMeshIN, int iShaderIN, int iGrassMemBlockIN, int iGrassMemBlockResIN,
									 int iCameraMaskIN)
{ 
	if (bResourcesSet) return;

	iGrassObj = iGrassObjIN;
	iGridObjectStart = iGridObjectStartIN;
	iGrassImg = iGrassImgIN;
	iShadowImg = iShadowImgIN;
	iShader = iShaderIN;
	iGrassMemBlock = iGrassMemBlockIN;
	iGrassMemBlockRes = iGrassMemBlockResIN;
	iBuildMesh = iBuildMeshIN;
	iCameraMask = iCameraMaskIN;

	bResourcesSet = true;
}

 void SetTerrainMask(int iMask)
{
	iCameraMask = iMask;
	if (!bGridMade) return;
	
	for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++){
		if (ObjectExist(iObj) == 1) SetObjectMask(iObj,iCameraMask);	
	}
}
 
 int InfiniteVegetationRnd(int iMax)
{
	return (int)(rand()*((float)iMax)/RAND_MAX);
}

 void DeleteVegetationGrid(void)
{
	if (!bGridMade) return;

	for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++){
		if (ObjectExist(iObj) == 1) DeleteObject(iObj);
	}	

	// Delete old grid exist array
	for (int iG = 0; iG < iGridDimension; iG++){
		delete [] bGridExist[iG];		
	}
	delete [] bGridExist;

	bGridMade = false;
}

void SetVegetationGridVisible(bool bShow)
{
	if (!bGridMade) return;

	int iX,iZ;
	for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++){
		if (ObjectExist(iObj) == 1){
			if (bShow){
				iX = (int)(ObjectPositionX(iObj)/iVegAreaWidth);
				iZ = (int)(ObjectPositionZ(iObj)/iVegAreaWidth);
				if (bGridExist[iX][iZ])	ShowObject(iObj);
			}else{
				HideObject(iObj);
			}
		}
	}
}

void SetVegetationGridVisibleForce(bool bShow)
{
	int iX, iZ;
	for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++) {
		if (ObjectExist(iObj) == 1) {
			if (bShow) {
				iX = (int)(ObjectPositionX(iObj) / iVegAreaWidth);
				iZ = (int)(ObjectPositionZ(iObj) / iVegAreaWidth);
				if (bGridExist[iX][iZ])	ShowObject(iObj);
			}
			else {
				HideObject(iObj);
			}
		}
	}
}

// Sets memblock values to 0 when grass exists in invalid locations (slopes and underwater)
 void DeleteInvalidGrass(int iTerrainID, float fWaterHeight, float fSlopeMax)
{
	if (!bResourcesSet) return;
	if (MemblockExist(iGrassMemBlock) == 0) return; // Safety check

	float fStepRatio = fWorldSize/iGrassMemBlockRes;
	float fXPos,fZPos,fHeightLeft,fHeightRight,fHeightFront,fHeightBack;
	int iGrassMemPos = 4+4+4;
	float fSlopeDistFull = fSlopeMax * 10; // Distance between height sample points front/back and left/right
	float fSlopeDistDiag = (float)sqrt((fSlopeDistFull/2.0)*(fSlopeDistFull/2.0)*2); // Distance between diagonal sample points
	for (int iZ = 0; iZ < iGrassMemBlockRes; iZ ++)
	{
		fZPos = iZ * fStepRatio;
		for (int iX = 0; iX < iGrassMemBlockRes; iX ++)
		{
			// No point trying to delete grass if there's none there already!
			if (ReadMemblockByte(iGrassMemBlock,iGrassMemPos+2) > iGrassMemblockThreshhold)
			{
				fXPos = iX * fStepRatio;
				// Check water height and slopes. This tries to check as efficiently as possible with the
				// minimum number of getgroundheight calls, which is why the logic is somewhat unintuitive
				#ifndef NOSTEAMORVIDEO
				fHeightLeft = (BT_GetGroundHeight(iTerrainID,fXPos-5.0f,fZPos));
				#else
				fHeightLeft = 0;
				#endif
				// First check we're not underwater using our first height check
				if (fHeightLeft < fWaterHeight)
				{
					//WriteMemblockByte(iGrassMemBlock,iGrassMemPos,0);
					WriteMemblockByte(iGrassMemBlock,iGrassMemPos+0,0);
					WriteMemblockByte(iGrassMemBlock,iGrassMemPos+1,0);
					WriteMemblockByte(iGrassMemBlock,iGrassMemPos+2,0);
					WriteMemblockByte(iGrassMemBlock,iGrassMemPos+3,0);
				}
				else
				{
					// Now get the right height and check left/right
					#ifndef NOSTEAMORVIDEO
					fHeightRight = (BT_GetGroundHeight(iTerrainID,fXPos + 5.0f,fZPos));
					#else
					fHeightRight = 0;
					#endif
					if (abs(fHeightLeft - fHeightRight) > fSlopeDistFull)
					{
						WriteMemblockByte(iGrassMemBlock,iGrassMemPos+0,0);
						WriteMemblockByte(iGrassMemBlock,iGrassMemPos+1,0);
						WriteMemblockByte(iGrassMemBlock,iGrassMemPos+2,0);
						WriteMemblockByte(iGrassMemBlock,iGrassMemPos+3,0);
					}
					else
					{
						// Now get the front height and check two diagonals
						#ifndef NOSTEAMORVIDEO
						fHeightFront = (BT_GetGroundHeight(iTerrainID,fXPos,fZPos + 5.0f));
						#else
						fHeightFront = 0;
						#endif
						if (abs(fHeightLeft - fHeightFront) > fSlopeDistDiag || abs(fHeightFront - fHeightRight) > fSlopeDistDiag)
						{
							WriteMemblockByte(iGrassMemBlock,iGrassMemPos+0,0);
							WriteMemblockByte(iGrassMemBlock,iGrassMemPos+1,0);
							WriteMemblockByte(iGrassMemBlock,iGrassMemPos+2,0);
							WriteMemblockByte(iGrassMemBlock,iGrassMemPos+3,0);
						}
						else
						{
							// Now get the back height, and check two diagonals and front to back
							#ifndef NOSTEAMORVIDEO
							fHeightBack = (BT_GetGroundHeight(iTerrainID,fXPos,fZPos - 5.0f));
							#else
							fHeightBack = 0;
							#endif
							if (abs(fHeightFront - fHeightBack) > fSlopeDistFull || abs(fHeightLeft - fHeightBack) > fSlopeDistDiag || abs(fHeightFront - fHeightBack) > fSlopeDistDiag)
							{
								WriteMemblockByte(iGrassMemBlock,iGrassMemPos+0,0);
								WriteMemblockByte(iGrassMemBlock,iGrassMemPos+1,0);
								WriteMemblockByte(iGrassMemBlock,iGrassMemPos+2,0);
								WriteMemblockByte(iGrassMemBlock,iGrassMemPos+3,0);
							}
						}
					}			
				}
			}
			iGrassMemPos+=4;
		}
	}
}

// Store VEG mesh UV pattern, so can recreate for many grasses
bool g_bVegMeshPatternFilled = false;
int g_iNumOfVertsInVegMesh = 0;
float g_fUVPatternForVegMesh[128][5];

// Makes the actual 3D model for the grid square of grass
void MakeVegPatch(int iVegObj, float fVegHeight, float fVegWidth, int iX, int iZ, int iOptionalSkipGrassMemblock)
{
	if (!bResourcesSet) return;
	float fGrassX,fGrassZ,fGrassHeight,fGrassWidth;
	int iVegBoom;
	int iBoomChance = (int)(iVegPerMesh * 0.6f);
	bool bWeHaveGrassHere = true;


	// now create veg object (full or dummy)
	if (ObjectExist(iVegObj)) DeleteObject(iVegObj);
	if ( bWeHaveGrassHere == true )
	{
		if ( iVegObj == iGridObjectStart )
		{
			// determine grass obj (mesh!)
			int iGrassMeshToUse = iGrassObj;

			// create original grass clump
			MakeObject(iVegObj,iGrassMeshToUse,0);
			for (int iGrass = 1; iGrass < iVegPerMesh + 1; iGrass++)
			{
				// Add and position grass clump in mesh
				fGrassX=(float)(InfiniteVegetationRnd(iVegAreaWidth)-iVegHalfAreaWidth);
				fGrassZ=(float)(InfiniteVegetationRnd(iVegAreaWidth)-iVegHalfAreaWidth);
				AddLimb(iVegObj,iGrass,iGrassMeshToUse);
				OffsetLimb(iVegObj,iGrass,fGrassX,0,fGrassZ);
				RotateLimb(iVegObj,iGrass,0,(float)InfiniteVegetationRnd(360),0);
				// Occassionally make a huge piece of grass
				if (InfiniteVegetationRnd(iBoomChance) == 1){iVegBoom=2;} else {iVegBoom=1;}
				fGrassHeight = (InfiniteVegetationRnd((int)(fVegHeight/1.5)) + fVegHeight) * iVegBoom;
				fGrassWidth = fVegWidth*2*iVegBoom;
				ScaleLimb(iVegObj,iGrass,fGrassWidth,fGrassHeight,fGrassWidth);
			}
			// Turn the multi limb object into a single limb object and setup
			if (GetMeshExist(iBuildMesh)) DeleteMesh(iBuildMesh);
			MakeMeshFromObject(iBuildMesh,iVegObj);
			DeleteObject(iVegObj);
			MakeObject(iVegObj,iBuildMesh,iGrassImg);
			DeleteMesh(iBuildMesh);
			SetObjectEffect(iVegObj,iShader);
		}
		else
		{
			// simply clone clump to speed up process for others
			CloneObject ( iVegObj, iGridObjectStart );
		}
		SetObjectMask(iVegObj,iCameraMask);
		if( bDisplayBelowWater )
			SetObjectTransparency(iVegObj,8);
		else
			SetObjectTransparency(iVegObj, 6);
		SetObjectCull ( iVegObj, 0 );
	}
}

void UpdateVegPatch(int iVegObj, int iTerrainID, float fVegX, float fVegZ)
{
	if (ObjectExist(iVegObj) == 0) return; // Safety check
	if (MemblockExist(iGrassMemBlock) == 0) return; // Safety check

	int iVertexCount,iGrassMemX,iGrassMemZ,iGrassMemPos;
	float fCenterHeight,fMiddleX,fMiddleZ,fLow,fHigh,fFloor,fNewHigh;
	float fVertX,fVertY,fVertZ;
	float fMemStepRatio = iGrassMemBlockRes/fWorldSize;

	#ifndef NOSTEAMORVIDEO
	fCenterHeight = (BT_GetGroundHeight(iTerrainID,fVegX,fVegZ));
	#else
	fCenterHeight = 0;
	#endif
	PositionObject(iVegObj,fVegX,fCenterHeight,fVegZ);

	// Now conform the grass pieces to the terrain height
	LockVertexDataForLimbCore(iVegObj,0,1);
	iVertexCount = 0;
	for (int iGrass=0; iGrass < iVegPerMesh + 1; iGrass++)
	{
		// Extract a central point, low point and high point for this piece of grass. 
		fMiddleX = fVegX + ((GetVertexDataPositionX(iVertexCount + iLeftVert)) + (GetVertexDataPositionX(iVertexCount + iRightVert))) / 2.0f;
		fMiddleZ = fVegZ + ((GetVertexDataPositionZ(iVertexCount + iFrontVert)) + (GetVertexDataPositionZ(iVertexCount + iBackVert))) / 2.0f;
		fLow = (GetVertexDataPositionY(iVertexCount + iBottomVert));
		fHigh = (GetVertexDataPositionY(iVertexCount + iTopVert));
		
		// If the grass isn't painted for this area, we can simply set the verts to the magic high value of 200.
		// The vertex shader will do minimal processing on these verts and their pixels will be clipped in the
		// pixel shader
		iGrassMemX = (int)(fMiddleX * fMemStepRatio);
		if (iGrassMemX < 0) iGrassMemX = 0;
		if (iGrassMemX >= iGrassMemBlockRes) iGrassMemX = iGrassMemBlockRes - 1;
		iGrassMemZ = (int)(fMiddleZ * fMemStepRatio);
		if (iGrassMemZ < 0) iGrassMemZ = 0;
		if (iGrassMemZ >= iGrassMemBlockRes) iGrassMemZ = iGrassMemBlockRes - 1;
		iGrassMemPos = 4+4+4+((iGrassMemX + iGrassMemZ * iGrassMemBlockRes)*4);

		float fScalePerGrassSize = 1.0f;

		if (ReadMemblockByte(iGrassMemBlock,iGrassMemPos+2) <= iGrassMemblockThreshhold)
		{
			fFloor=200;
		}
		else
		{
			#ifndef NOSTEAMORVIDEO
			fFloor = (BT_GetGroundHeight(iTerrainID,fMiddleX,fMiddleZ)) - fCenterHeight;
			#endif
		}

		// final height of grass
		fNewHigh = fFloor + ((fHigh - fLow)*fScalePerGrassSize);

		// Now reposition all of our vertices for this grass clump					
		int iLocalVertexCount = iVertexCount;
		for (int fGrassVert = 0; fGrassVert < iGrassClumpVerts; fGrassVert++)
		{
			// set grass quad position
			fVertX = (GetVertexDataPositionX(iLocalVertexCount));
			fVertY = (GetVertexDataPositionY(iLocalVertexCount));
			fVertZ = (GetVertexDataPositionZ(iLocalVertexCount));
			if (abs(fVertY - fLow) < 0.01f)
			{
				SetVertexDataPosition(iLocalVertexCount,fVertX,fFloor,fVertZ);
			}
			else
			{
				SetVertexDataPosition(iLocalVertexCount,fVertX,fNewHigh,fVertZ);
			}				
			iLocalVertexCount++;
		}				

		// Now adjust UV data to reflect grass type

		// ensure vertex count keeps pace with transition through data!
		iVertexCount = iLocalVertexCount;
	}
	UnlockVertexData();
}

// Takes a float and returns an integer clamped between 0 and grass memblock max width
 int ClampToMemblockRes(float fValue)
{
	if (fValue < 0) return 0;
	if (fValue >= iGrassMemBlockRes) return iGrassMemBlockRes - 1;
	return (int)fValue;
}

 bool GridSquareContainsGrass(int iX, int iZ)
{
	// Work out area in the grass existance memblock we're looking at
	float fMemStepRatio = iVegAreaWidth * iGrassMemBlockRes/fWorldSize;
	int iMemLeft = ClampToMemblockRes(iX * fMemStepRatio);	
	int iMemRight = ClampToMemblockRes(iMemLeft + fMemStepRatio);
	int iMemBack = ClampToMemblockRes(iZ * fMemStepRatio);
	int iMemFront = ClampToMemblockRes(iMemBack + fMemStepRatio);
		
	int iMemPos, iMemX, iMemZ;
	for (iMemX = iMemLeft; iMemX <= iMemRight; iMemX ++){
		for (iMemZ = iMemBack; iMemZ <= iMemFront; iMemZ ++)
		{
			iMemPos = 4+4+4+((iMemX + iMemZ * iGrassMemBlockRes)*4);
			if (ReadMemblockByte(iGrassMemBlock,iMemPos+2) > iGrassMemblockThreshhold) 
			{
				return true;
			}
		}
	}
	return false;
}

 void RefreshGridExistArray(void)
{	
	 if (!bGridMade) return;

	for (int iX = 0; iX < iGridDimension; iX++){				
		for (int iZ = 0; iZ < iGridDimension; iZ++){			
			bGridExist[iX][iZ] = GridSquareContainsGrass(iX,iZ);
		}
	}
}

void MakeVegetationGrid(int iVegPerMeshIN,  float fVegWidthIN, float fVegHeightIN, int iVegAreaWidthIN, int iGridSizeIN, int iTerrainID, int iOptionalSkipGrassMemblock, bool bBelowWater)
{
	if (!bResourcesSet) return;
	if (GetMeshExist(iGrassObj) == 0) return; // Safety check

	bDisplayBelowWater = bBelowWater;

	// Set our 'old' positions miles away so we force a position update on the first VEG UPDATE call
	fOldViewPointX = -10000;
	fOldViewPointZ = -10000;
    iOldLeftEdge = -10000;
	iOldRightEdge = -10000;
	iOldFrontEdge = -10000;
	iOldBackEdge = -10000;

	// If the grid is already made to the exact same spec (quantity and size) then there's no point remaking it
	if (bGridMade && iVegPerMesh == iVegPerMeshIN && iGridSize == iGridSizeIN && fVegWidth == fVegWidthIN && fVegHeight == fVegHeightIN){
		// Update the grid square visibility array as new grass may have been painted
		RefreshGridExistArray();
		SetVegetationGridVisible(true);
		// Let's refresh all veg objects already in place unless we're in superflat mode (iTerrainID = 0) as heights
		// may have changed
		if (iTerrainID != 0)
		{
			int iX,iZ;
			for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++)
			{
				if (ObjectExist(iObj) == 1)
				{
					iX = (int)(ObjectPositionX(iObj)/iVegAreaWidth);
					iZ = (int)(ObjectPositionZ(iObj)/iVegAreaWidth);
					if (bGridExist[iX][iZ]) UpdateVegPatch(iObj,iTerrainID,ObjectPositionX(iObj),ObjectPositionZ(iObj));
				}
			}
		}
		return;
	}

	DeleteVegetationGrid();

	if (iGridSizeIN < 1) return;	 // Can't make a grid if we have no dimensions
	if (iVegAreaWidthIN < 1) return; // Can't make a grid if each veg patch has no size
	if (iVegPerMeshIN < 1) return;   // Can't make a grid if we don't have a valid number of clumps per patch
	
	int iVegObj = iGridObjectStart;
    iVegPerMesh = iVegPerMeshIN;
	srand(12345);	

	// Set the veg area and width values
	iVegAreaWidth = iVegAreaWidthIN; 
	iGridSize = iGridSizeIN;
	fVegWidth = fVegWidthIN;
	fVegHeight = fVegHeightIN;
	iVegHalfAreaWidth = iVegAreaWidth/2;
	iVegAreaRad = iVegHalfAreaWidth + iVegAreaWidth * (iGridSize/2 - 1);
		
	LockVertexDataForMesh(iGrassObj);
	// Extract number of verts in this mesh for later use
	iGrassClumpVerts = GetVertexDataVertexCount();
	// Figure out which verts are on the extremities
	float fLeft = 10000;
	float fRight = -10000;
	float fFront = -10000;
	float fBack = 10000;
	float fTop = -10000;
	float fBottom = 10000;
	float fVertX,fVertY,fVertZ;
	for (int iV = 0; iV < iGrassClumpVerts; iV++)
	{
		fVertX = (GetVertexDataPositionX(iV));
		fVertY = (GetVertexDataPositionY(iV));
		fVertZ = (GetVertexDataPositionZ(iV));
		if (fVertX < fLeft)  { fLeft = fVertX;   iLeftVert = iV;}
		if (fVertX > fRight) { fRight = fVertX;  iRightVert = iV;}
		if (fVertY < fBottom){ fBottom = fVertY; iBottomVert = iV;}
		if (fVertY > fTop)   { fTop = fVertY;    iTopVert = iV;}
		if (fVertZ < fBack)  { fBack = fVertZ;   iBackVert = iV;}
		if (fVertZ > fFront) { fFront = fVertZ;  iFrontVert = iV;}
	}
	UnlockVertexData();

	// Now make our veg grid patches

	for (int iX = 0; iX < iGridSize; iX++)
	{
		for (int iZ = 0; iZ < iGridSize; iZ++)
		{
			MakeVegPatch(iVegObj,fVegHeight,fVegWidth,iX,iZ,iOptionalSkipGrassMemblock);
			iVegObj++;
		}
	}
	iGridObjectEnd = iVegObj - 1;

	// Define an array of bools which specify whether a grid block is populated or not
	iGridDimension = (int)(fWorldSize/iVegAreaWidth);
	//bGridExist = new bool*[iGridDimension]; // lee - 170614 - crashed when put grass on lower right edge
	bGridExist = new bool*[iGridDimension+1];
	//for (int iG = 0; iG < iGridDimension; iG++)
	for (int iG = 0; iG <= iGridDimension; iG++)
	{
		bGridExist[iG] = new bool[iGridDimension+1];
	}

	// lee - 170614 - ensure its completely clear
	for (int iX = 0; iX <= iGridDimension; iX++){				
		for (int iZ = 0; iZ <= iGridDimension; iZ++){			
			bGridExist[iX][iZ] = false;
		}
	}

	bGridMade = true; //PE: Moved here so grid exists are updated.

	RefreshGridExistArray();
		
}

bool bFullVegUpdate = true;

void MakeVegetationGridQuick(int iVegPerMeshIN, float fVegWidthIN, float fVegHeightIN, int iVegAreaWidthIN, int iGridSizeIN, int iTerrainID, int iOptionalSkipGrassMemblock, bool bBelowWater)
{
	if (!bResourcesSet) return;
	if (GetMeshExist(iGrassObj) == 0) return; // Safety check

	bDisplayBelowWater = bBelowWater;

	// Set our 'old' positions miles away so we force a position update on the first VEG UPDATE call
	fOldViewPointX = -10000;
	fOldViewPointZ = -10000;
	iOldLeftEdge = -10000;
	iOldRightEdge = -10000;
	iOldFrontEdge = -10000;
	iOldBackEdge = -10000;

	// If the grid is already made to the exact same spec (quantity and size) then there's no point remaking it
	if (bGridMade && iVegPerMesh == iVegPerMeshIN && iGridSize == iGridSizeIN && fVegWidth == fVegWidthIN && fVegHeight == fVegHeightIN) 
	{
		// Update the grid square visibility array as new grass may have been painted
		RefreshGridExistArray();
		SetVegetationGridVisible(true);
		// Let's refresh all veg objects already in place unless we're in superflat mode (iTerrainID = 0) as heights may have changed
		if (iTerrainID != 0)
		if(bFullVegUpdate)
		{
			bFullVegUpdate = false;
			int iX, iZ;
			for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++)
			{
				if (ObjectExist(iObj) == 1)
				{
					iX = (int)(ObjectPositionX(iObj) / iVegAreaWidth);
					iZ = (int)(ObjectPositionZ(iObj) / iVegAreaWidth);
					if (bGridExist[iX][iZ]) UpdateVegPatch(iObj, iTerrainID, ObjectPositionX(iObj), ObjectPositionZ(iObj));
				}
			}
		}
		return;
	}

	// create new veg from scratch (happens when terrain is loaded, height data availabke and need height corrected grass)
	DeleteVegetationGrid();

	if (iGridSizeIN < 1) return;	 // Can't make a grid if we have no dimensions
	if (iVegAreaWidthIN < 1) return; // Can't make a grid if each veg patch has no size
	if (iVegPerMeshIN < 1) return;   // Can't make a grid if we don't have a valid number of clumps per patch

	int iVegObj = iGridObjectStart;
	iVegPerMesh = iVegPerMeshIN;
	srand(12345);

	// Set the veg area and width values
	iVegAreaWidth = iVegAreaWidthIN;
	iGridSize = iGridSizeIN;
	fVegWidth = fVegWidthIN;
	fVegHeight = fVegHeightIN;
	iVegHalfAreaWidth = iVegAreaWidth / 2;
	iVegAreaRad = iVegHalfAreaWidth + iVegAreaWidth * (iGridSize / 2 - 1);

	LockVertexDataForMesh(iGrassObj);
	// Extract number of verts in this mesh for later use
	iGrassClumpVerts = GetVertexDataVertexCount();
	// Figure out which verts are on the extremities
	float fLeft = 10000;
	float fRight = -10000;
	float fFront = -10000;
	float fBack = 10000;
	float fTop = -10000;
	float fBottom = 10000;
	float fVertX, fVertY, fVertZ;
	for (int iV = 0; iV < iGrassClumpVerts; iV++)
	{
		fVertX = (GetVertexDataPositionX(iV));
		fVertY = (GetVertexDataPositionY(iV));
		fVertZ = (GetVertexDataPositionZ(iV));
		if (fVertX < fLeft) { fLeft = fVertX;   iLeftVert = iV; }
		if (fVertX > fRight) { fRight = fVertX;  iRightVert = iV; }
		if (fVertY < fBottom) { fBottom = fVertY; iBottomVert = iV; }
		if (fVertY > fTop) { fTop = fVertY;    iTopVert = iV; }
		if (fVertZ < fBack) { fBack = fVertZ;   iBackVert = iV; }
		if (fVertZ > fFront) { fFront = fVertZ;  iFrontVert = iV; }
	}
	UnlockVertexData();

	// Now make our veg grid patches
	for (int iX = 0; iX < iGridSize; iX++)
	{
		for (int iZ = 0; iZ < iGridSize; iZ++)
		{
			MakeVegPatch(iVegObj, fVegHeight, fVegWidth, iX, iZ, iOptionalSkipGrassMemblock);
			iVegObj++;
		}
	}
	iGridObjectEnd = iVegObj - 1;

	// Define an array of bools which specify whether a grid block is populated or not
	iGridDimension = (int)(fWorldSize / iVegAreaWidth);
	//bGridExist = new bool*[iGridDimension]; // lee - 170614 - crashed when put grass on lower right edge
	bGridExist = new bool*[iGridDimension + 1];
	//for (int iG = 0; iG < iGridDimension; iG++)
	for (int iG = 0; iG <= iGridDimension; iG++)
	{
		bGridExist[iG] = new bool[iGridDimension + 1];
	}

	// lee - 170614 - ensure its completely clear
	for (int iX = 0; iX <= iGridDimension; iX++) {
		for (int iZ = 0; iZ <= iGridDimension; iZ++) {
			bGridExist[iX][iZ] = false;
		}
	}

	bGridMade = true; //PE: Moved here to make sure grass gets visible, and gridexists update.
	RefreshGridExistArray();

	int iX, iZ;
	for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++)
	{
		if (ObjectExist(iObj) == 1)
		{
			iX = (int)(ObjectPositionX(iObj) / iVegAreaWidth);
			iZ = (int)(ObjectPositionZ(iObj) / iVegAreaWidth);
			if (bGridExist[iX][iZ]) UpdateVegPatch(iObj, iTerrainID, ObjectPositionX(iObj), ObjectPositionZ(iObj));
		}
	}
	SetVegetationGridVisible(true);

}

 void UpdateVegZone(bool bSuperFlat, float fX1, float fZ1, float fX2, float fZ2, int iTerrainID, float fSuperFlatHeight)
{
	if (!bGridMade) return; // Can't update if grid doesn't exist

	// Work out the update zone grid squares
	int iLeftEdge = (int)fX1 - iVegHalfAreaWidth; 	
	int iRightEdge = (int)fX2 + iVegHalfAreaWidth; 
	int iBackEdge = (int)fZ1 - iVegHalfAreaWidth;
	int iFrontEdge = (int)fZ2 + iVegHalfAreaWidth;
	if (iLeftEdge < iVegHalfAreaWidth) iLeftEdge = iVegHalfAreaWidth;
	if (iRightEdge > fWorldSize - iVegHalfAreaWidth) iRightEdge = (int)(fWorldSize - iVegHalfAreaWidth);
	if (iBackEdge < iVegHalfAreaWidth) iBackEdge = iVegHalfAreaWidth;
	if (iFrontEdge > fWorldSize - iVegHalfAreaWidth) iFrontEdge = (int)(fWorldSize - iVegHalfAreaWidth);
 
	// Now round this off to veggie grid positions
	iLeftEdge /= iVegAreaWidth;
	iRightEdge /= iVegAreaWidth;
	iBackEdge /= iVegAreaWidth;
	iFrontEdge /= iVegAreaWidth;

	// Cycle through our grid and process each veg obj
	int iVegObj;
	float fVegX,fVegZ;
	for (int iX = iLeftEdge; iX < iRightEdge + 1; iX++)
	{		
		for (int iZ = iBackEdge; iZ < iFrontEdge + 1; iZ++)
		{			
			// Because this function is called when grass is being newly painted in F9 mode, we'll also need to
			// update the grid existance array for this updated zone			
			bGridExist[iX][iZ] = GridSquareContainsGrass(iX,iZ);		
			if (bGridExist[iX][iZ])
			{
				// Work out which veg obj should be at this location and update it
				iVegObj = iGridObjectStart + (iZ % iGridSize) + iGridSize * (iX % iGridSize);
				if (ObjectExist(iVegObj) == 1)
				{	
					// Safety check
					fVegX = (float)(iX * iVegAreaWidth + iVegHalfAreaWidth);
					fVegZ = (float)(iZ * iVegAreaWidth + iVegHalfAreaWidth);
					if (bSuperFlat)
					{
						PositionObject(iVegObj,fVegX,fSuperFlatHeight,fVegZ);
					}
					else
					{
						UpdateVegPatch(iVegObj,iTerrainID,fVegX,fVegZ);
					}
					ShowObject(iVegObj);
				}
			}
		}
	}
}

void UpdateVegetation(bool bSuperFlat, float fViewPointX, float fViewPointZ, int iTerrainID, float fSuperFlatHeight, int iDynamicShadowImage)
{
	if (!bGridMade) return; // Can't update if grid doesn't exist

	// Exit if we haven't moved
	if (fOldViewPointX == fViewPointX && fOldViewPointZ == fViewPointZ) return;
	fOldViewPointX = fViewPointX;
	fOldViewPointZ = fViewPointZ;

	// Work out the grid square around our player to populate with veggies
	int iLeftEdge = (int)fViewPointX - iVegAreaRad; 	
	int iRightEdge = (int)fViewPointX + iVegAreaRad; 
	int iBackEdge = (int)fViewPointZ - iVegAreaRad;
	int iFrontEdge = (int)fViewPointZ + iVegAreaRad;
	if (iLeftEdge < iVegHalfAreaWidth) iLeftEdge = iVegHalfAreaWidth;
	if (iRightEdge > fWorldSize - iVegHalfAreaWidth) iRightEdge = (int)(fWorldSize - iVegHalfAreaWidth);
	if (iBackEdge < iVegHalfAreaWidth) iBackEdge = iVegHalfAreaWidth;
	if (iFrontEdge > fWorldSize - iVegHalfAreaWidth) iFrontEdge = (int)(fWorldSize - iVegHalfAreaWidth);
 
	// Now round this off to veggie grid positions
	iLeftEdge /= iVegAreaWidth;
	iRightEdge /= iVegAreaWidth;
	iBackEdge /= iVegAreaWidth;
	iFrontEdge /= iVegAreaWidth;

	// Exit if our grid extremities haven't changed
	if (iLeftEdge == iOldLeftEdge && iRightEdge == iOldRightEdge && iBackEdge == iOldBackEdge && iFrontEdge == iOldFrontEdge) return;
	iOldLeftEdge = iLeftEdge;
	iOldRightEdge = iRightEdge;
	iOldBackEdge = iBackEdge;
	iOldFrontEdge = iFrontEdge;

	// Cycle through our grid and process each veg obj
	int iVegObj;
	float fVegX,fVegZ;
	for (int iX = iLeftEdge; iX < iRightEdge + 1; iX++)
	{
		for (int iZ = iBackEdge; iZ < iFrontEdge + 1; iZ++)
		{	
			// Work out which veg obj should be at this location
			iVegObj = iGridObjectStart + (iZ % iGridSize) + iGridSize * (iX % iGridSize);
			if (ObjectExist(iVegObj) == 1)
			{
				// Does this grid square have any grass? If not, hide the object now, otherwise show it
				if (bGridExist[iX][iZ])
				{
					// if determined for use, create it now

					// show veg chunk
					ShowObject(iVegObj);

					// veg position and updating step
					fVegX = (float)(iX * iVegAreaWidth + iVegHalfAreaWidth);
					fVegZ = (float)(iZ * iVegAreaWidth + iVegHalfAreaWidth);
					// Work out the position for this veg obj. If it's already at this position, we don't need to process it.
					// If it isn't, then we'll move it there and setup
					if (abs(ObjectPositionX(iVegObj) - fVegX) > 1 || abs(ObjectPositionZ(iVegObj) - fVegZ) > 1)
					{ 
						if (bSuperFlat)
						{
							PositionObject(iVegObj,fVegX,fSuperFlatHeight,fVegZ);
						}
						else
						{
							UpdateVegPatch(iVegObj,iTerrainID,fVegX,fVegZ);
						}
					}							
				}
				else
				{
					HideObject(iVegObj);
				}
			}
		}
	}
}

 void UpdateBlitzTerrain(float fViewPointX, float fViewPointZ, int iTerrainID, int iDynamicShadowImage)
{
	UpdateVegetation(false, fViewPointX, fViewPointZ, iTerrainID, 0, iDynamicShadowImage);
}

 void UpdateSuperFlat(float fViewPointX, float fViewPointZ, float fHeight, int iDynamicShadowImage )
{
	UpdateVegetation(true, fViewPointX, fViewPointZ, 0, fHeight, iDynamicShadowImage);
}

 void UpdateVegZoneBlitzTerrain(float fX1, float fZ1, float fX2, float fZ2, int iTerrainID)
{
	UpdateVegZone(false,fX1,fZ1,fX2,fZ2,iTerrainID,0);
	UpdateGrassTexture(0); // ensure newly added veg object has its texture
}

 void UpdateVegZoneSuperFlat(float fX1, float fZ1, float fX2, float fZ2, float fSuperFlatHeight)
{
	UpdateVegZone(true,fX1,fZ1,fX2,fZ2,0,fSuperFlatHeight);
}

 void UpdateGrassTexture ( int iPBRMode )
{
	if (!bGridMade) return; // Can't update if grid doesn't exist
	for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++)
	{
		if (ObjectExist(iObj) == 1)
		{
			if ( iPBRMode == 0 )
			{
				// non-PBR
				TextureObject(iObj,0,iGrassImg);
				//TextureObject(iObj,1,iShadowImg); // not used
				// texture stage 2 reserved for passed in shadow (not used)
			}
			else
			{
				// PBR
				TextureObject ( iObj, 0, iGrassImg ); //Diffuse
				TextureObject ( iObj, 6, iPBRCubeImg );//EnvironmentMap
			}
		}
	}


	#ifndef PRODUCTCLASSIC
	//PE: Cant do this in classic , as grass will show in editor, and F9 grass paint dont work.
	#ifndef PRODUCTV3
	// LB: also do not do in VRQ as it wipes out grass flag which we need to see grass in editor!
	bGridMade = false;
	#endif
	#endif

}

void UpdateGrassShader ( int iNewShaderIndex )
{
	if (!bGridMade) return; // Can't update if grid doesn't exist
	for (int iObj = iGridObjectStart; iObj < iGridObjectEnd + 1; iObj++)
	{
		if (ObjectExist(iObj) == 1)
		{
			SetObjectEffect ( iObj, iNewShaderIndex );
		}
	}	
}

void HideVegetationGrid(void)
{
	SetVegetationGridVisible(false);
}

 void ShowVegetationGrid(void)
{
	SetVegetationGridVisible(true);
}

 int GetFirstGrassObject(void)
{
	return iGridObjectStart;
}

 int GetLastGrassObject(void)
{
	return iGridObjectEnd;
}