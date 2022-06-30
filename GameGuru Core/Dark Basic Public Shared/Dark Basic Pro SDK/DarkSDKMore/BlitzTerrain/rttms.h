#ifndef _RTTMS_H
#define _RTTMS_H

#include "main.h"

struct BT_RTTMS_STRUCT
{
//Counts
	unsigned short VertexCount;

//Data
	float* Vertices;

//Updated vertices
	bool ChangedAVertex;
	unsigned short FirstUpdatedVertex;
	unsigned short LastUpdatedVertex;

//Internals
	void* Internals;
};

struct BT_RTTMS_STRUCTINTERNALS
{
//Terrain, LODLevel and Sector
	unsigned long TerrainID;
	unsigned long LODLevelID;
	unsigned long SectorID;

//Sector pointer
	s_BT_Sector* SectorPtr;

//Delete meshdata
	bool DeleteMeshData;
};

typedef void(*BT_RTTMS_UpdateHandler_t)(unsigned long TerrainID,unsigned long LODLevelID,unsigned long SectorID,unsigned short StartVertex,unsigned short EndVertex,float* Vertices);

void BT_RTTMS_AddUpdateHandler(unsigned long TerrainID,BT_RTTMS_UpdateHandler_t UpdateHandler);
static void BT_RTTMS_CallUpdateHandlers(unsigned long TerrainID,unsigned long LODLevelID,unsigned long SectorID,unsigned short StartVertex,unsigned short EndVertex,float* Vertices);
void BT_RTTMS_DeleteUpdateHandlers(unsigned long TerrainID);
void* BT_RTTMS_LockSectorVertexData(unsigned long TerrainID,unsigned long LODLevelID,unsigned long SectorID);
void BT_RTTMS_UnlockSectorVertexData(void* StructPtr);
unsigned short BT_RTTMS_FindVertex(void* StructPtr,unsigned short Vrow,unsigned short Vcol);
struct s_BT_terrain;
void BT_RTTMS_UnlockTerrain(s_BT_terrain* TerrainPtr);

#endif