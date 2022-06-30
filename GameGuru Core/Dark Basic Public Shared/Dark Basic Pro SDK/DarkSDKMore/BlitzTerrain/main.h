#ifndef _BLITZTERRAINMAIN_H
#define _BLITZTERRAINMAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <setjmp.h>
#include <math.h>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <sstream>
#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include ".\..\..\Shared\camera\ccameradatac.h"
#include "directx-macros.h"
#include "BlitzTerrain.h"
#include "globstruct.h"

#define C_BT_FULLVERSION
#define C_BT_VERSION 200
#define C_BT_MAXTERRAINS 255
#define C_BT_MAXLODLEVELS 16 //PE: org 255 , just waste mem and cpu.
#define C_BT_MAXTERRAINSIZE 4096
#define BT_VERSION "2.02 R2 FULL"

using namespace std;

struct s_BT_Sector;

#include "quadmapping.h"
#include "EnvironmentMapping.h"

struct BTVector3
{
	float x,y,z;
};

struct s_BT_DrawBuffer
{
	IGGVertexBuffer* VertexBuffer;
	IGGIndexBuffer* IndexBuffer;
	IGGIndexBuffer* EdgeLineIndexBuffer;
	unsigned short Vertices;
	unsigned long Indices;
	unsigned short EdgeIndexCount;
	unsigned long FVF;
	unsigned char FVF_Size;
	unsigned short Primitives;
};

struct s_BT_LODLevel;
struct s_BT_terrain;
struct s_BT_QuadTree;
class BT_QuadMap;

struct s_BT_Sector
{
	//Excluded
	bool Excluded;

	//ID
	unsigned long ID;

	//Positions
	unsigned short Row;
	unsigned short Column;
	float Pos_x;
	float Pos_y;
	float Pos_z;

	//Quadmap
	BT_QuadMap* QuadMap;

	//Drawbuffer
	s_BT_DrawBuffer* DrawBuffer;

	//World Matrix
	GGMATRIX WorldMatrix;

	//LODLevel and Terrain
	s_BT_LODLevel* LODLevel;
	s_BT_terrain* Terrain;

	// Edge LOD
	bool TopSideNeedsUpdate;
	bool LeftSideNeedsUpdate;
	bool RightSideNeedsUpdate;
	bool BottomSideNeedsUpdate;
	int TopSideLODLevel;
	int LeftSideLODLevel;
	int RightSideLODLevel;
	int BottomSideLODLevel;

	//Quadtree
	s_BT_QuadTree* QuadTree;

	//Update mesh bool
	bool UpdateMesh;

	//RTTMS
	bool VertexDataLocked;
	BT_RTTMS_STRUCT* VertexDataRTTMS;

	//Object
	sObject* DBPObject;
	sFrame* LODLevelObjectFrame;

	//Info
	void* Info;
};

struct BT_SectorInfo
{
	bool Excluded;
	unsigned short Row;
	unsigned short Column;
	float Pos_x;
	float Pos_y;
	float Pos_z;
	void* WorldMatrix;
	void* InternalData;
};


struct s_BT_LODLevel
{
	unsigned char ID;
	float SectorSize;
	unsigned char SectorDetail;
	unsigned short Sectors;
	float Distance;
	s_BT_Sector* Sector;
	unsigned short Split;
	unsigned char TileSpan;
	s_BT_terrain* Terrain;
	sObject* DBPObject;

	//Info
	void* Info;
};

struct BT_LODLevelInfo
{
	float SectorSize;
	unsigned char SectorDetail;
	unsigned short Sectors;
	float Distance;
	unsigned short Split;
	unsigned char TileSpan;
	void* InternalData;
};


struct s_BT_CullBox
{
	float Front,Back,
		  Left,Right,
		  Top,Bottom;
};

struct s_BT_QuadTree
{
	s_BT_QuadTree* n1;
	s_BT_QuadTree* n2;
	s_BT_QuadTree* n3;
	s_BT_QuadTree* n4;

	s_BT_QuadTree* Parent;

	unsigned char Level;
	unsigned char row;
	unsigned char collumn;

	s_BT_Sector* Sector;
	bool Excluded;
	s_BT_CullBox* CullBox;
	float PosX,PosY,PosZ;
	bool DrawThis;
	bool Culled;
	bool CullboxChanged;
};

struct s_BT_LODMap
{
	int Level;
};

struct s_BT_terrain
{
	//Exists
	bool Exists;

	//ID
	unsigned long ID;

	//Build stuff
	bool Built;
	bool Generated;

	//Point lists
	float* HeightPoint;

	//Object stuff
	sObject* Object;
	unsigned long ObjectID;

	//Images
	unsigned long Heightmap;
	unsigned long Texture;
	unsigned long Detailmap;
	unsigned long Environmentmap;
	unsigned long Exclusionmap;

	//Settings
	unsigned long Smoothing;
	bool QuadRotation;
	bool QuadReduction;
	unsigned char LODLevels;
	float Scale;
	float YScale;
	float Tile;
	bool MeshOptimisation;
	unsigned short Sectors;

	//LOD Levels
	s_BT_LODLevel* LODLevel;

	//Sizes and details
	unsigned short Heightmapsize;
	float TerrainSize;

	//Quadtree
	s_BT_QuadTree* QuadTree;
	unsigned char QuadTreeLevels;

	//Exclusion threshold
	unsigned char ExclusionThreshold;

	//LODMap
	s_BT_LODMap** LODMap;

	//AT Mode
	unsigned char ATMode;

	//Detail BlendMode
	unsigned char DetailBlendMode;

	//Vertex declaration
	LPGGVERTEXLAYOUT VertexDeclaration;

	//Environment Map
	BT_EnvironmentMap* EnvironmentMap;

	//Multidetailmapping
	bool MultiDetailmapping;
	unsigned long DetailMapCount;

	//Info
	void* Info;
};

struct BT_TerrainInfo
{
	bool Built;
	bool Generated;
	unsigned long Texture;
	unsigned long Detailmap;
	bool Smoothing;
	bool QuadReduction;
	bool QuadRotation;
	unsigned char LODLevels;
	float Scale;
	float YScale;
	float Tile;
	bool MeshOptimisation;
	unsigned short Sectors;
	unsigned short Heightmapsize;
	float TerrainSize;
	bool MultiDetailmapping;
	unsigned long DetailMapCount;
	bool Exclusion;
	unsigned char ExclusionThreshold;
	unsigned char ATMode;
	unsigned char DetailBlendMode;
	void* InternalData;
	//Added in 2.2
	unsigned long SmoothAmount;
	void* DBPObjectPtr;
};


struct BT_QuadMap_Main;

struct s_BT_main
{
	//Terrains
	s_BT_terrain Terrains[C_BT_MAXTERRAINS];

	//Build info
	bool Building;
	bool BuildType;
	s_BT_terrain* CurrentBuildTerrain;
	unsigned long CurrentBuildSector;
	unsigned long CurrentBuildRow;
	unsigned long CurrentBuildColumn;
	unsigned long CurrentBuildLODLevel;
	unsigned long CurrentBuildTerrainSector;

	//Current function
	unsigned long CurrentFunction;

	//BuildStep
	unsigned long buildstep;

	//Updating
	tagCameraData* CurrentUpdateCamera;
	bool FrustumExtracted;

	//Frustum Culling
	GGVECTOR3 CullOffset;
	GGVECTOR3 CullScale;
	float Frustum[5][4];

	//LOD Cam
	GGVECTOR3 LODCamPosition;

	//Statistics
	unsigned long DrawPrimitiveCount;
	unsigned long DrawCalls;
	unsigned long CullChecks;

	//Current render terrain
	s_BT_terrain* CurrentRenderTerrain;

	//Quadmap info
	BT_QuadMap_Main QuadmapInfo;

	//Instruction queue
	char* InstructionQueue;
	unsigned long InstructionQueueSize;
	unsigned long InstructionQueueUsed;

	//Advanced Terrain mode
	bool ATMode;

	//Auto render
	bool AutoRender;

	//Effect
	cSpecialEffect* CurrentEffect;

	//Initialised boolean (for GDK)
	bool Initialised;
};

struct BT_SectorMeshData
{
	bool DeleteMeshData;
	void* Vertex_Data;
	unsigned short Vertex_Count;
	unsigned char Vertex_FVFSize;

	unsigned short* Index_Data;
	unsigned long Index_Count;

	float Scale;
};

void *BT_GetSectorInternalData(unsigned long TerrainID,unsigned long LODLevel,unsigned long SectorID);
void BT_DeleteSectorInternalData(unsigned long TerrainID,unsigned long LODLevel,unsigned long SectorID);

void BT_Intern_Render();
static void BT_Intern_AddToInstructionQueue(char Instruction,char Data);
static void BT_Intern_ClearStatistics();
static void BT_Intern_CalculateLODLevelsRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level,unsigned char LODLevelToDraw);
static void BT_Intern_FixLODSeams(s_BT_terrain* Terrain);
static void BT_Intern_FixSectorLODSeams(s_BT_Sector* SectorPtr);
static void BT_Intern_CalculateCullingRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level,bool IntersectingFrustum);
static void BT_Intern_UpdateCullBoxesRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level);
static void BT_Intern_RenderTerrainRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level,int iQualityPass);
static void BT_Intern_UnlockSectorsRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level);
static void BT_Intern_RenderSector(s_BT_Sector* Sector);
static float BT_Intern_DistanceToLODCamera(s_BT_terrain* Terrain,s_BT_CullBox* CullBox);
static void BT_Intern_ContinueBuild();
static void BT_Intern_RenderTerrain(s_BT_terrain* Terrain);
static void BT_Intern_NoRenderTerrain(s_BT_terrain* Terrain);
bool BT_Intern_TerrainExist(unsigned long terrainid);
static long BT_Intern_ImageExist(unsigned long imageid);
void BT_Intern_Error(int number);
static char* BT_Intern_GetErrorString(int number);
static char* BT_Intern_GetFunctionName(int number);
static void BT_Intern_GetSectorHeights(s_BT_terrain* Terrain,unsigned long LODLevel,unsigned long row,unsigned long column,float* buffer);
static bool BT_Intern_GetSectorExclusion(s_BT_terrain* Terrain,unsigned long LODLevel,unsigned long excludememblock,unsigned long row,unsigned long column,bool* buffer);
static void BT_Intern_BuildSector(s_BT_Sector* Sector);
static void BT_Intern_DeleteTerrain(unsigned long TerrainID,bool DeleteObject);
float BT_Intern_GetPointHeight(s_BT_terrain* Terrain,float Px,float Pz,char LODLevel,bool Round);
BTVector3 BT_Intern_GetPointNormal(s_BT_terrain* Terrain,float Px,float Pz);
static bool BT_Intern_GetPointExcluded(s_BT_terrain* Terrain,float Px,float Pz);
s_BT_QuadTree* BT_Intern_AllocateQuadTree(s_BT_terrain* Terrain);
static s_BT_QuadTree* BT_Intern_AllocateQuadTreeRec(s_BT_terrain* Terrain,unsigned char Levels,s_BT_QuadTree* Parent,unsigned char row,unsigned char collumn);

static void BT_Intern_DeAllocateQuadTree(s_BT_QuadTree* Quadtree);
static void BT_Intern_DeAllocateQuadTreeRec(s_BT_QuadTree* Quadtree);

static float BT_Intern_GetHeightFromColor(unsigned long Colour);

void BT_Intern_RefreshVB(s_BT_DrawBuffer* DrawBuffer,unsigned long FirstVertex,unsigned long LastVertex,BT_Meshdata_Vertex* Vertex);
void BT_Intern_RefreshIB(s_BT_DrawBuffer* DrawBuffer,unsigned long FirstIndex,unsigned long LastIndex,unsigned short* Index);

static void BT_Intern_ExtractFrustum();
static int BT_Intern_CullBox(s_BT_CullBox* CullBox);

static BT_RTTMS_STRUCT* BT_Intern_LockSectorVertexData(s_BT_Sector* Sector);
static void BT_Intern_UnlockSectorVertexData(s_BT_Sector* Sector);

struct BT_RTTMS_VERTEX;

void BT_Intern_RTTMSUpdateHandler(unsigned long TerrainID,unsigned long LODLevelID,unsigned long SectorID,unsigned short StartVertex,unsigned short EndVertex,float* VerticesPtr);

static void BT_Intern_SmoothTerrain(s_BT_terrain* Terrain);


#define C_BT_ERROR_MAXTERRAINSEXCEDED 1
#define C_BT_ERROR_TERRAINDOESNTEXIST 2
#define C_BT_ERROR_INVALIDLODLEVELS 3
#define C_BT_ERROR_HEIGHTMAPDOESNTEXIST 4
#define C_BT_ERROR_LODLEVELDOESNTEXIST 5
#define C_BT_ERROR_HEIGHTMAPSIZEINVALID 6
#define C_BT_ERROR_EXCLUSIONMAPSIZEINVALID 7
#define C_BT_ERROR_ALREADYBUILDING 8
#define C_BT_ERROR_OBJECTIDILLEGAL 9
#define C_BT_ERROR_TERRAINNOTGENERATED 10
#define C_BT_ERROR_CANNOTCREATEVB 11
#define C_BT_ERROR_CANNOTCREATEIB 12
#define C_BT_ERROR_SECTORDOESNTEXIST 13
#define C_BT_ERROR_TERRAINNOTBUILT 14
#define C_BT_ERROR_INVALIDFILE 15
#define C_BT_ERROR_VERSIONCANNOTREADFILE 16
#define C_BT_ERROR_TERRAINALREADYBUILT 17
#define C_BT_ERROR_CANNOTUSEFUNCTIONONBUILTTERRAIN 18
#define C_BT_ERROR_SECTORALREADYLOCKED 19
#define C_BT_ERROR_SECTORNOTUNLOCKED 20
#define C_BT_ERROR_USESFULLVERSION 21
#define C_BT_ERROR_SECTORTOOBIG 22
#define C_BT_ERROR_SPLITTOOHIGH 23
#define C_BT_ERROR_SECTORALREADYHASOBJECT 24
#define C_BT_ERROR_LODLEVELALREADYHASOBJECT 25
#define C_BT_ERROR_SECTORISEXCLUDED 26
#define C_BT_ERROR_MEMORYERROR 27

#define C_BT_FUNCTION_MAKETERRAIN 1
#define C_BT_FUNCTION_SETTERRAINHEIGHTMAP 2
#define C_BT_FUNCTION_SETTERRAINTEXTURE 4
#define C_BT_FUNCTION_SETTERRAINEXCLUSION 5
#define C_BT_FUNCTION_SETTERRAINDETAIL 6
#define C_BT_FUNCTION_SETTERRAINENVIRONMENT 7
#define C_BT_FUNCTION_SETTERRAINLOD 8
#define C_BT_FUNCTION_SETTERRAINSPLIT 9
#define C_BT_FUNCTION_SETTERRAINDETAILTILE 10
#define C_BT_FUNCTION_SETTERRAINQUADREDUCTION 12
#define C_BT_FUNCTION_SETTERRAINQUADROTATION 13
#define C_BT_FUNCTION_SETTERRAINSMOOTHING 14
#define C_BT_FUNCTION_SETTERRAINSCALE 15
#define C_BT_FUNCTION_SETTERRAINYSCALE 16
#define C_BT_FUNCTION_SETTERRAINLODDISTANCES 17
#define C_BT_FUNCTION_BUILDTERRAIN 18
#define C_BT_FUNCTION_CONTINUEBUILD 19
#define C_BT_FUNCTION_TERRAINEXIST 20
#define C_BT_FUNCTION_DELETETERRAIN 21
#define C_BT_FUNCTION_GETGROUNDHEIGHT 22
#define C_BT_FUNCTION_GETTERRAINSIZE 23
#define C_BT_FUNCTION_GETVERSION 25
#define C_BT_FUNCTION_SETBUILDSTEP 26
#define C_BT_FUNCTION_UPDATETERRAIN 28
#define C_BT_FUNCTION_UPDATE 29
#define C_BT_FUNCTION_RENDER 32
#define C_BT_FUNCTION_ENABLEAUTORECOVERY 33
#define C_BT_FUNCTION_GETSTATISTIC 34
#define C_BT_FUNCTION_CLEARSTATISTICS 35
#define C_BT_FUNCTION_GETOBJECTID 36
#define C_BT_FUNCTION_MAKESECTOROBJECT 38
#define C_BT_FUNCTION_GETSECTORPOSITIONX 39
#define C_BT_FUNCTION_GETSECTORPOSITIONY 40
#define C_BT_FUNCTION_GETSECTORPOSITIONZ 41
#define C_BT_FUNCTION_GETSECTORCOUNT 42
#define C_BT_FUNCTION_GETSECTORSIZE 43
#define C_BT_FUNCTION_GETSECTOREXCLUDED 44
#define C_BT_FUNCTION_GETSECTORROW 45
#define C_BT_FUNCTION_GETSECTORCOLLUMN 46
#define C_BT_FUNCTION_SETCURRENTCAMERA 47
#define C_BT_FUNCTION_UPDATETERRAINLOD 48
#define C_BT_FUNCTION_UPDATETERRAINCULL 49
#define C_BT_FUNCTION_SETPOINTHEIGHT 50
#define C_BT_FUNCTION_SETTERRAINEXCLUSIONTHRESHOLD 51
#define C_BT_FUNCTION_LOADTERRAIN 52
#define C_BT_FUNCTION_SAVETERRAIN 53
#define C_BT_FUNCTION_GETPOINTEXCLUDED 54
#define C_BT_FUNCTION_SETATMODE 55
#define C_BT_FUNCTION_SETTERRAINDETAILBLENDMODE 56
#define C_BT_FUNCTION_INIT 57
#define C_BT_FUNCTION_ADDTERRAINENVIRONMENT 58
#define C_BT_FUNCTION_GETPOINTENVIRONMENT 59
#define C_BT_FUNCTION_GETTERRAININFO 60
#define C_BT_FUNCTION_GETLODLEVELINFO 61
#define C_BT_FUNCTION_GETSECTORINFO 62
#define C_BT_FUNCTION_MAKETERRAINOBJECT 63
#define C_BT_FUNCTION_ENABLEAUTORENDER 64

#define C_BT_INSTRUCTION_SETCURRENTCAMERA 1
#define C_BT_INSTRUCTION_UPDATETERRAINCULL 3
#define C_BT_INSTRUCTION_UPDATETERRAINLOD 4
#define C_BT_INSTRUCTION_RENDERTERRAIN 5
#define C_BT_INSTRUCTION_NORENDERTERRAIN 6


#define C_BT_INTERNALSCALE 128.0f

#endif
