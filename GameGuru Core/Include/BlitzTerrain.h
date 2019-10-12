#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Blitz Terrain
void BTConstructor();
void BTDestructor();
unsigned long BT_MakeTerrain();
void BT_SetTerrainHeightmap(unsigned long terrainid,unsigned long image);
void BT_SetTerrainTexture(unsigned long terrainid,unsigned long image);
void BT_SetTerrainExclusion(unsigned long terrainid,unsigned long image);
void BT_SetTerrainExclusionThreshold(unsigned long terrainid,unsigned long threshold);
void BT_SetTerrainDetail(unsigned long terrainid,unsigned long image);
void BT_SetTerrainDetailBlendMode(unsigned long terrainid,unsigned char mode);
void BT_SetTerrainEnvironment(unsigned long terrainid,unsigned long image);
unsigned long BT_AddTerrainEnvironment(unsigned long terrainid,unsigned long Colour);
void BT_SetTerrainLOD(unsigned long terrainid,unsigned char LODLevels);
void BT_SetTerrainSplit(unsigned long terrainid,unsigned long Split);
void BT_SetTerrainDetailTile(unsigned long terrainid,float Tile);
void BT_SetTerrainQuadReduction(unsigned long terrainid,bool Enabled);
void BT_SetTerrainQuadRotation(unsigned long terrainid,bool Enabled);
void BT_SetTerrainSmoothing(unsigned long terrainid,unsigned long Amount);
void BT_SetTerrainScale(unsigned long terrainid,float Scale);
void BT_SetTerrainYScale(unsigned long terrainid,float YScale);
void BT_SetTerrainLODDistance(unsigned long terrainid,unsigned char LODLevel,float value);
void BT_BuildTerrain(unsigned long terrainid,unsigned long ObjectID);
void BT_BuildTerrain(unsigned long terrainid,unsigned long ObjectID,bool GenerateTerrain);
int BT_ContinueBuild();
unsigned long BT_TerrainExist(unsigned long TerrainID);
void BT_DeleteTerrain(unsigned long TerrainID);
void BT_ForceTerrainTechnique(unsigned long QualityTechniqueMode);
float BT_GetGroundHeight(unsigned long terrainid,float x,float z);
float BT_GetGroundHeight(unsigned long terrainid,float x,float z,bool Round);
float BT_GetTerrainSize(unsigned long terrainid);
unsigned long BT_GetPointExcluded(unsigned long terrainid,float x,float z);
unsigned long BT_GetPointEnvironment(unsigned long terrainid,float x,float z);
DWORD BT_GetVersion();
void BT_SetBuildStep(unsigned long step);
void BT_SetATMode(bool ATMode);
void BT_EnableAutoRender(bool AutoRender);
unsigned long BT_GetStatistic(unsigned long code);
unsigned long BT_GetTerrainObjectID(unsigned long terrainid);
void BT_MakeSectorObject(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID,unsigned long ObjectID);
void BT_MakeTerrainObject(unsigned long terrainid,unsigned long LODLevel,unsigned long ObjectID);
float BT_GetSectorPositionX(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID);
float BT_GetSectorPositionY(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID);
float BT_GetSectorPositionZ(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID);
unsigned long BT_GetSectorCount(unsigned long TerrainID,unsigned long LODLevel);
unsigned long BT_GetSectorSize(unsigned long TerrainID,unsigned long LODLevel);
unsigned long BT_GetSectorExcluded(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID);
unsigned long BT_GetSectorRow(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID);
unsigned long BT_GetSectorCollumn(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID);
void BT_SetCurrentCamera(unsigned long CameraID);
void BT_UpdateTerrainLOD(unsigned long TerrainID);
void BT_UpdateTerrainCull(unsigned long TerrainID);
void BT_UpdateTerrain(unsigned long TerrainID);
void BT_RenderTerrain(unsigned long TerrainID);
void BT_NoRenderTerrain(unsigned long TerrainID);
void SetTerrainRenderLevel( int size );
void BT_Intern_Render( void );

// Internal functions for RTTMS calls
bool BT_Intern_TerrainExist(unsigned long terrainid);
void* BT_GetTerrainInfo(unsigned long terrainid);
void* BT_GetLODLevelInfo(unsigned long terrainid,unsigned long LODLevelID);

// Blitz Terrain RTTMS

void BTRTTMSConstructor();
void BTRTTMSDestructor();
void BT_RaiseTerrain(unsigned long TerrainID,float X,float Z,float Radius,float Amount);
void BT_RaiseTerrain(unsigned long TerrainID,float X,float Z,float Radius,float Amount,float capheight);
void BT_SetPointHeight(unsigned long TerrainID,unsigned long TVrow,unsigned long TVcol,float Height);
