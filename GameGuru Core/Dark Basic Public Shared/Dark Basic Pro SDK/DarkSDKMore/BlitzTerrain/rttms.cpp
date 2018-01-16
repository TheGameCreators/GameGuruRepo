#include "rttms.h"
#include <stdlib.h>

extern s_BT_main BT_Main;

#ifdef C_BT_FULLVERSION
BT_RTTMS_UpdateHandler_t* BT_RTTMS_UpdateHandlers[C_BT_MAXTERRAINS]={0};
unsigned long BT_RTTMS_UpdateHandlerCount[C_BT_MAXTERRAINS]={0};
#endif

void BT_RTTMS_AddUpdateHandler(unsigned long TerrainID,BT_RTTMS_UpdateHandler_t UpdateHandler)
{
#ifdef C_BT_FULLVERSION
//Increase update handler count
	BT_RTTMS_UpdateHandlerCount[TerrainID]++;

//Allocate new update handler list
	if(BT_RTTMS_UpdateHandlers[TerrainID]==NULL){
		BT_RTTMS_UpdateHandlers[TerrainID]=(BT_RTTMS_UpdateHandler_t*)malloc(sizeof(BT_RTTMS_UpdateHandler_t)*BT_RTTMS_UpdateHandlerCount[TerrainID]);
		if(BT_RTTMS_UpdateHandlers[TerrainID]==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
	}else{
		BT_RTTMS_UpdateHandlers[TerrainID]=(BT_RTTMS_UpdateHandler_t*)realloc(BT_RTTMS_UpdateHandlers[TerrainID],sizeof(BT_RTTMS_UpdateHandler_t)*BT_RTTMS_UpdateHandlerCount[TerrainID]);
		if(BT_RTTMS_UpdateHandlers[TerrainID]==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
	}

//Set new update handler
	BT_RTTMS_UpdateHandlers[TerrainID][BT_RTTMS_UpdateHandlerCount[TerrainID]-1]=UpdateHandler;
#endif
}

static void BT_RTTMS_CallUpdateHandlers(unsigned long TerrainID,unsigned long LODLevelID,unsigned long SectorID,unsigned short StartVertex,unsigned short EndVertex,float* Vertices)
{
#ifdef C_BT_FULLVERSION
//Loop through update handlers and call each one
	for(unsigned long i=1;i<=BT_RTTMS_UpdateHandlerCount[TerrainID];i++)
		BT_RTTMS_UpdateHandlers[TerrainID][i-1](TerrainID,LODLevelID,SectorID,StartVertex,EndVertex,Vertices);
#endif
}

void BT_RTTMS_DeleteUpdateHandlers(unsigned long TerrainID)
{
#ifdef C_BT_FULLVERSION
//Delete update handlers
	if(BT_RTTMS_UpdateHandlerCount[TerrainID]!=0){
		free(BT_RTTMS_UpdateHandlers[TerrainID]);
		BT_RTTMS_UpdateHandlers[TerrainID]=NULL;
		BT_RTTMS_UpdateHandlerCount[TerrainID]=0;
	}
#endif
}

void* BT_RTTMS_LockSectorVertexData(unsigned long TerrainID,unsigned long LODLevelID,unsigned long SectorID)
{
//Check that the data exists
	if(BT_Intern_TerrainExist(TerrainID)==false)
		return NULL;
	s_BT_terrain* Terrain=&BT_Main.Terrains[TerrainID];
	if(Terrain->Built==false)
		return NULL;
	if(LODLevelID>=Terrain->LODLevels)
		return NULL;
	if(SectorID>=Terrain->LODLevel[LODLevelID].Sectors)
		return NULL;
	if(Terrain->LODLevel[LODLevelID].Sector[SectorID].VertexDataLocked==true)
		return (void*)Terrain->LODLevel[LODLevelID].Sector[SectorID].VertexDataRTTMS;

//Get struct
	BT_RTTMS_STRUCT* RTTMSStruct=Terrain->LODLevel[LODLevelID].Sector[SectorID].VertexDataRTTMS;

//Lock vertexdata
	Terrain->LODLevel[LODLevelID].Sector[SectorID].VertexDataLocked=true;

//Fill RTTMSStruct
	Terrain->LODLevel[LODLevelID].Sector[SectorID].QuadMap->FillMeshData(RTTMSStruct);

//Return
	return (void*)RTTMSStruct;
}


void BT_RTTMS_UnlockSectorVertexData(void* StructPtr)
{
//Get struct
	BT_RTTMS_STRUCT* RTTMSStruct=(BT_RTTMS_STRUCT*)StructPtr;
	BT_RTTMS_STRUCTINTERNALS* RTTMSStructInternals=(BT_RTTMS_STRUCTINTERNALS*)RTTMSStruct->Internals;

//Unlock vertexdata
	RTTMSStructInternals->SectorPtr->VertexDataLocked=false;

//(full version only) If the vertexdata was updated, send the new vertexdata around
#ifdef C_BT_FULLVERSION
	if(RTTMSStruct->ChangedAVertex==true)
		BT_RTTMS_CallUpdateHandlers(RTTMSStructInternals->TerrainID,RTTMSStructInternals->LODLevelID,RTTMSStructInternals->SectorID,RTTMSStruct->FirstUpdatedVertex,RTTMSStruct->LastUpdatedVertex,RTTMSStruct->Vertices);
#endif

//Delete Vertices
	free(RTTMSStruct->Vertices);

//Delete mesh data if it has to be deleted
	if(RTTMSStructInternals->DeleteMeshData==true)
	{
		RTTMSStructInternals->SectorPtr->QuadMap->DeleteMeshData();
	}
}

unsigned short BT_RTTMS_FindVertex(void* StructPtr,unsigned short Vrow,unsigned short Vcol)
{
//Get struct
	BT_RTTMS_STRUCT* RTTMSStruct=(BT_RTTMS_STRUCT*)StructPtr;
	BT_RTTMS_STRUCTINTERNALS* RTTMSStructInternals=(BT_RTTMS_STRUCTINTERNALS*)RTTMSStruct->Internals;

//Return vertexid
	return RTTMSStructInternals->SectorPtr->QuadMap->FindVertex(Vrow,Vcol);
}


void BT_RTTMS_UnlockTerrain(s_BT_terrain* TerrainPtr)
{
	for(unsigned char LODLevel=0;LODLevel<TerrainPtr->LODLevels;LODLevel++)
	{
		s_BT_LODLevel* LODLevelPtr=&TerrainPtr->LODLevel[LODLevel];
		for(unsigned short Sector=0;Sector<LODLevelPtr->Sectors;Sector++)
		{
			s_BT_Sector* SectorPtr=&LODLevelPtr->Sector[Sector];
			if(SectorPtr->Excluded!=true && SectorPtr->VertexDataLocked)
			{
				BT_RTTMS_UnlockSectorVertexData((void*)SectorPtr->VertexDataRTTMS);
			}
		}
	}
}