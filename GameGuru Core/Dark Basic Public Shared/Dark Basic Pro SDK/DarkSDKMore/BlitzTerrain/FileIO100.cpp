#include "FileIO.h"
#include "CreateObject.h"
#include <fstream>

extern s_BT_main BT_Main;


#ifndef COMPILE_GDK
	extern t_dbGetDirect3DDevice dbGetDirect3DDevice;
#endif

unsigned long LoadTerrain100(ifstream *File,unsigned long ObjectID,bool GenerateTerrain)
{
//Set current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_LOADTERRAIN;

//Variables
	s_BT_terrain* TerrainPtr;
	unsigned short TempShort;
	s_BT_FileTerrainHeader100* TerrainHeader;
	unsigned long TerrainID;

//Make a new terrain
	TerrainID=BT_MakeTerrain();
	TerrainPtr=&BT_Main.Terrains[TerrainID];

//Make a terrain header
	TerrainHeader=BT_Intern_MakeDefaultTerrainHeader100();

//Load header from file
	File->read((char*)&TempShort,sizeof(unsigned short));
	File->read((char*)TerrainHeader,TempShort);

//Check that no full version features are used
#ifndef C_BT_FULLVERSION
	if((TerrainHeader->Flags>>3)&0x1 || (TerrainHeader->Flags>>1)&0x1 ||
		TerrainHeader->HeightmapSize>C_BT_MAXTERRAINSIZE || TerrainHeader->LODLevels>C_BT_MAXLODLEVELS)
		BT_Intern_Error(C_BT_ERROR_USESFULLVERSION);
#endif

//Check heightmap size
	if(TerrainHeader->HeightmapSize>C_BT_MAXTERRAINSIZE)
	{
		BT_Intern_Error(C_BT_ERROR_HEIGHTMAPSIZEINVALID);
		return NULL;
	}

//Insert terrain header into terrain
	TerrainPtr->Built=true;
	TerrainPtr->Smoothing=(TerrainHeader->Flags>>2)&0x1;
	TerrainPtr->QuadReduction=(TerrainHeader->Flags>>3)&0x1;
	TerrainPtr->QuadRotation=(TerrainHeader->Flags>>4)&0x1;
	TerrainPtr->LODLevels=TerrainHeader->LODLevels;
	TerrainPtr->Scale=TerrainHeader->Scale;
	TerrainPtr->YScale=TerrainHeader->YScale;
	TerrainPtr->Tile=TerrainHeader->Tile;
	TerrainPtr->MeshOptimisation=(TerrainHeader->Flags>>5)&0x1;
	TerrainPtr->Sectors=TerrainHeader->Sectors;
	TerrainPtr->Heightmapsize=TerrainHeader->HeightmapSize;
	TerrainPtr->TerrainSize=TerrainHeader->TerrainSize;
	TerrainPtr->ExclusionThreshold=TerrainHeader->ExclusionThreshold;
	TerrainPtr->ATMode=TerrainHeader->ATMode;
	TerrainPtr->DetailBlendMode=TerrainHeader->DetailBlendMode;
	free(TerrainHeader);

//Terrain info
	BT_TerrainInfo* TerrainInfo=(BT_TerrainInfo*)malloc(sizeof(BT_TerrainInfo));
	if(TerrainInfo==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
	memset(TerrainInfo,0,sizeof(BT_TerrainInfo));
	TerrainPtr->Info=(void*)TerrainInfo;
	TerrainInfo->Built=true;
	TerrainInfo->Generated=GenerateTerrain;
	TerrainInfo->Smoothing=(TerrainPtr->Smoothing>0);
	TerrainInfo->QuadReduction=TerrainPtr->QuadReduction;
	TerrainInfo->QuadRotation=TerrainPtr->QuadRotation;
	TerrainInfo->LODLevels=TerrainPtr->LODLevels;
	TerrainInfo->Scale=TerrainPtr->Scale;
	TerrainInfo->YScale=TerrainPtr->YScale;
	TerrainInfo->Tile=TerrainPtr->Tile;
	TerrainInfo->MeshOptimisation=TerrainPtr->MeshOptimisation;
	TerrainInfo->Sectors=TerrainPtr->Sectors;
	TerrainInfo->Heightmapsize=TerrainPtr->Heightmapsize;
	TerrainInfo->TerrainSize=TerrainPtr->TerrainSize;
	TerrainInfo->ExclusionThreshold=TerrainPtr->ExclusionThreshold;
	TerrainInfo->ATMode=TerrainPtr->ATMode;
	TerrainInfo->DetailBlendMode=TerrainPtr->DetailBlendMode;

//Make terrain object
	TerrainPtr->ObjectID=ObjectID;
	TerrainPtr->Object=BT_Intern_CreateBlankObject(ObjectID,1);
	BT_Intern_SetupMesh(TerrainPtr->Object->pFrame->pMesh,3,0,D3DFVF_DIFFUSE);
	BT_Intern_FinnishObject(ObjectID);

	//TerrainPtr->Object->pDelete=new sObjectDelete::sDelete;
	//TerrainPtr->Object->pDelete->onDelete=&BT_Intern_DeleteCallback;
	//TerrainPtr->Object->pDelete->userData=TerrainID;
	//TerrainPtr->Object->iDeleteCount=0;
	//TerrainPtr->Object->iDeleteID=1;

//Make the LODLevel and Sector header
	s_BT_FileLODLevelHeader100* LODLevelHeader=BT_Intern_MakeDefaultLODLevelHeader100();
	s_BT_FileSectorHeader100* SectorHeader=BT_Intern_MakeDefaultSectorHeader100();

//Allocate LODLevels
	TerrainPtr->LODLevel=(s_BT_LODLevel*)malloc(TerrainPtr->LODLevels*sizeof(s_BT_LODLevel));
	if(TerrainPtr->LODLevel==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

//Loop through LODLevels
	for(unsigned char LODLevel=0;LODLevel<TerrainPtr->LODLevels;LODLevel++)
	{
	//Find LODLevel Ptr
		s_BT_LODLevel* LODLevelPtr=&TerrainPtr->LODLevel[LODLevel];

	//Read header
		File->read((char*)&TempShort,sizeof(unsigned short));
		File->read((char*)LODLevelHeader,TempShort);

	//Insert LODLevel header into LODLevel
		LODLevelPtr->SectorSize=LODLevelHeader->SectorSize;
		LODLevelPtr->SectorDetail=LODLevelHeader->SectorDetail;
		LODLevelPtr->Sectors=LODLevelHeader->Sectors;
		LODLevelPtr->Distance=LODLevelHeader->Distance;
		LODLevelPtr->Split=LODLevelHeader->Split;
		LODLevelPtr->TileSpan=LODLevelHeader->TileSpan;
		LODLevelPtr->Terrain=TerrainPtr;
		LODLevelPtr->ID=LODLevel;

	//Allocate segment data
		if(LODLevel==0)
			BT_Intern_AllocateSegmentData(TerrainPtr);

	//LODLevel info
		BT_LODLevelInfo* LODLevelInfo=(BT_LODLevelInfo*)malloc(sizeof(BT_LODLevelInfo));
		if(LODLevelInfo==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		memset(LODLevelInfo,0,sizeof(BT_LODLevelInfo));
		LODLevelPtr->Info=(void*)LODLevelInfo;
		LODLevelInfo->SectorSize=LODLevelPtr->SectorSize;
		LODLevelInfo->SectorDetail=LODLevelPtr->SectorDetail;
		LODLevelInfo->Sectors=LODLevelPtr->Sectors;
		LODLevelInfo->Distance=LODLevelPtr->Distance;
		LODLevelInfo->Split=LODLevelPtr->Split;
		LODLevelInfo->TileSpan=LODLevelPtr->TileSpan;

	//Allocate sectors
		LODLevelPtr->Sector=(s_BT_Sector*)malloc(LODLevelPtr->Sectors*sizeof(s_BT_Sector));
		if(LODLevelPtr->Sector==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

	//Loop through sectors
		for(unsigned short Sector=0;Sector<LODLevelPtr->Sectors;Sector++)
		{
		//Find Sector Ptr
			s_BT_Sector* SectorPtr=&LODLevelPtr->Sector[Sector];

		//Read header
			File->read((char*)&TempShort,sizeof(unsigned short));
			File->read((char*)SectorHeader,TempShort);

		//Insert Sector header into Sector
			SectorPtr->Excluded=SectorHeader->Excluded;
			SectorPtr->Row=SectorHeader->Row;
			SectorPtr->Column=SectorHeader->Column;
			SectorPtr->Pos_x=SectorHeader->PosX;
			SectorPtr->Pos_y=SectorHeader->PosY;
			SectorPtr->Pos_z=SectorHeader->PosZ;
			SectorPtr->LODLevel=LODLevelPtr;
			SectorPtr->Terrain=TerrainPtr;
			/* OLD SYSTEM
			SectorPtr->TopSideNeedsUpdate=false;
			SectorPtr->LeftSideNeedsUpdate=false;
			SectorPtr->RightSideNeedsUpdate=false;
			SectorPtr->BottomSideNeedsUpdate=false;
			SectorPtr->TopSideLODLevel=LODLevel;
			SectorPtr->LeftSideLODLevel=LODLevel;
			SectorPtr->RightSideLODLevel=LODLevel;
			SectorPtr->BottomSideLODLevel=LODLevel;
			*/
			SectorPtr->UpdateMesh=false;
			SectorPtr->ID=Sector;

		//Allocate segment info
			SectorPtr->SegmentsPerSide=unsigned char((float)pow(2.0f,(float)LODLevelPtr->ID));
			SectorPtr->SegmentLODLevel=(unsigned char*)malloc(4*SectorPtr->SegmentsPerSide*sizeof(unsigned char));
			if(SectorPtr->SegmentLODLevel==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
			memset(SectorPtr->SegmentLODLevel,0,4*SectorPtr->SegmentsPerSide*sizeof(unsigned char));
			SectorPtr->SegmentNeedsUpdate=(bool*)malloc(4*SectorPtr->SegmentsPerSide*sizeof(bool));
			if(SectorPtr->SegmentNeedsUpdate==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
			memset(SectorPtr->SegmentNeedsUpdate,0,4*SectorPtr->SegmentsPerSide*sizeof(bool));

		//Sector info
			BT_SectorInfo* SectorInfo=(BT_SectorInfo*)malloc(sizeof(BT_SectorInfo));
			if(SectorInfo==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
			memset(SectorInfo,0,sizeof(BT_SectorInfo));
			SectorPtr->Info=(void*)SectorInfo;
			SectorInfo->Excluded=SectorPtr->Excluded;
			SectorInfo->Row=SectorPtr->Row;
			SectorInfo->Column=SectorPtr->Column;
			SectorInfo->Pos_x=SectorPtr->Pos_x;
			SectorInfo->Pos_y=SectorPtr->Pos_y;
			SectorInfo->Pos_z=SectorPtr->Pos_z;
			SectorInfo->WorldMatrix=&SectorPtr->WorldMatrix;

		//Create QuadMap
			if(SectorPtr->Excluded==false)
			{
			//Read QuadMap
				unsigned long QuadMapBufferSize;
				char* QuadMapBuffer;
				File->read((char*)&QuadMapBufferSize,sizeof(unsigned long));
				QuadMapBuffer=(char*)malloc(QuadMapBufferSize*sizeof(char));
				if(QuadMapBuffer==nullptr)
					BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
				File->read(QuadMapBuffer,QuadMapBufferSize);

			//RTTMS
				TerrainPtr->LODLevel[LODLevel].Sector[Sector].VertexDataRTTMS=(BT_RTTMS_STRUCT*)malloc(sizeof(BT_RTTMS_STRUCT));
				if(TerrainPtr->LODLevel[LODLevel].Sector[Sector].VertexDataRTTMS==nullptr)
					BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
				BT_RTTMS_STRUCTINTERNALS* RTTMSStructInternals=(BT_RTTMS_STRUCTINTERNALS*)malloc(sizeof(BT_RTTMS_STRUCTINTERNALS));
				if(RTTMSStructInternals==nullptr)
					BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
				RTTMSStructInternals->TerrainID=TerrainID;
				RTTMSStructInternals->LODLevelID=LODLevel;
				RTTMSStructInternals->SectorID=Sector;
				RTTMSStructInternals->SectorPtr=&TerrainPtr->LODLevel[LODLevel].Sector[Sector];
				TerrainPtr->LODLevel[LODLevel].Sector[Sector].VertexDataRTTMS->Internals=(void*)RTTMSStructInternals;

			//Create quadmap
				SectorPtr->QuadMap=(BT_QuadMap*)malloc(sizeof(BT_QuadMap));
				if(SectorPtr->QuadMap==nullptr)
					BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
				memset(SectorPtr->QuadMap,0,sizeof(BT_QuadMap));
				SectorPtr->QuadMap->GenerateFromBuffer100(QuadMapBuffer,QuadMapBufferSize,SectorPtr);
				free(QuadMapBuffer);
			}

		//Clear Sector Header
			BT_Intern_FillSectorHeaderDefault100(SectorHeader);
		}

	//Clear LODLevel Header
		BT_Intern_FillLODLevelHeaderDefault100(LODLevelHeader);
	}

//Cleanup
	free(LODLevelHeader);
	free(SectorHeader);

//Return pointer
	return TerrainID;
}



s_BT_FileTerrainHeader100 *BT_Intern_MakeDefaultTerrainHeader100()
{
//Make Header
	s_BT_FileTerrainHeader100* Header=(s_BT_FileTerrainHeader100*)malloc(sizeof(s_BT_FileTerrainHeader100));
	if(Header==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

//Set default values
	BT_Intern_FillTerrainHeaderDefault100(Header);

//Return header
	return Header;
}

void BT_Intern_FillTerrainHeader100(s_BT_FileTerrainHeader100* Header,s_BT_terrain* Terrain)
{
//Fill the header
	Header->Flags=(Terrain->Environmentmap!=NULL);
	Header->Flags|=(Terrain->Exclusionmap!=NULL)<<1;
	Header->Flags|=(Terrain->Smoothing>0)<<2;
	Header->Flags|=(Terrain->QuadReduction)<<3;
	Header->Flags|=(Terrain->QuadRotation)<<4;
	Header->Flags|=(Terrain->MeshOptimisation)<<5;
	Header->LODLevels=Terrain->LODLevels;
	Header->Scale=Terrain->Scale;
	Header->YScale=Terrain->YScale;
	Header->Tile=Terrain->Tile;
	Header->Sectors=Terrain->Sectors;
	Header->HeightmapSize=Terrain->Heightmapsize;
	Header->TerrainSize=Terrain->TerrainSize;
	Header->ExclusionThreshold=Terrain->ExclusionThreshold;
	Header->FVF=0;
	Header->ATMode=Terrain->ATMode;
	Header->DetailBlendMode=Terrain->DetailBlendMode;
}


void BT_Intern_FillTerrainHeaderDefault100(s_BT_FileTerrainHeader100* Header)
{

}


s_BT_FileLODLevelHeader100 *BT_Intern_MakeDefaultLODLevelHeader100()
{
//Make Header
	s_BT_FileLODLevelHeader100* Header=(s_BT_FileLODLevelHeader100*)malloc(sizeof(s_BT_FileLODLevelHeader100));
	if(Header==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

//Set default values
	BT_Intern_FillLODLevelHeaderDefault100(Header);
	
//Return header
	return Header;
}

void BT_Intern_FillLODLevelHeader100(s_BT_FileLODLevelHeader100* Header,s_BT_LODLevel* LODLevel)
{
//Fill the header
	Header->SectorSize=LODLevel->SectorSize;
	Header->SectorDetail=LODLevel->SectorDetail;
	Header->Sectors=LODLevel->Sectors;
	Header->Distance=LODLevel->Distance;
	Header->Split=LODLevel->Split;
	Header->TileSpan=LODLevel->TileSpan;
}

void BT_Intern_FillLODLevelHeaderDefault100(s_BT_FileLODLevelHeader100* Header)
{

}


s_BT_FileSectorHeader100 *BT_Intern_MakeDefaultSectorHeader100()
{
//Make Header
	s_BT_FileSectorHeader100* Header=(s_BT_FileSectorHeader100*)malloc(sizeof(s_BT_FileSectorHeader100));
	if(Header==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

//Set default values
	BT_Intern_FillSectorHeaderDefault100(Header);

//Return header
	return Header;
}

void BT_Intern_FillSectorHeader100(s_BT_FileSectorHeader100* Header,s_BT_Sector* Sector)
{
//Fill the header
	Header->Excluded=Sector->Excluded;
	Header->Row=Sector->Row;
	Header->Column=Sector->Column;
	Header->PosX=Sector->Pos_x;
	Header->PosY=Sector->Pos_y;
	Header->PosZ=Sector->Pos_z;
}

void BT_Intern_FillSectorHeaderDefault100(s_BT_FileSectorHeader100* Header)
{

}