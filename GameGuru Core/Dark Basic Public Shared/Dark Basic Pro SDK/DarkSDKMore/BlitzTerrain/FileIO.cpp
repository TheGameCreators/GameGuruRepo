#include "FileIO.h"
#include "CreateObject.h"
#include <fstream>

extern s_BT_main BT_Main;


#ifndef COMPILE_GDK
	extern t_dbGetDirect3DDevice dbGetDirect3DDevice;
#endif

EXPORT unsigned long BT_LoadTerrain(char* FileName,unsigned long ObjectID)
{
	return BT_LoadTerrain(FileName,ObjectID,false);
}

EXPORT unsigned long BT_LoadTerrain(char* FileName,unsigned long ObjectID,bool GenerateTerrain)
{
//Set current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_LOADTERRAIN;

//Variables
	ifstream File;
	s_BT_terrain* TerrainPtr;
	unsigned short TempShort;
	unsigned long TerrainID;

//Open the file
	File.open(FileName,ios::in | ios::app | ios::binary);

//Check the magic number
	File.read((char*)&TempShort,sizeof(unsigned short));
	if(TempShort!=21570)
	{
		BT_Intern_Error(C_BT_ERROR_INVALIDFILE);
		return 0;
	}

//Check the file version
	File.read((char*)&TempShort,sizeof(unsigned short));
	if(TempShort!=1 && TempShort!=101)
	{
		BT_Intern_Error(C_BT_ERROR_VERSIONCANNOTREADFILE);
		return 0;
	}
	unsigned short Version=TempShort;
	File.read((char*)&TempShort,sizeof(unsigned short));
	if(TempShort>C_BT_VERSION)
	{
		BT_Intern_Error(C_BT_ERROR_VERSIONCANNOTREADFILE);
		return 0;
	}

//Load the map
	TerrainID=0;
	 if(Version==1)
		 TerrainID=LoadTerrain100(&File,ObjectID,GenerateTerrain);

	 if(Version==101)
		 TerrainID=LoadTerrain101(&File,ObjectID,GenerateTerrain);

	 if(TerrainID==NULL)
		 return 0;
	 TerrainPtr=&BT_Main.Terrains[TerrainID];
	 File.close();

//Allocate quadtree
	TerrainPtr->QuadTree=BT_Intern_AllocateQuadTree(TerrainPtr);

//Create LOD Map
	TerrainPtr->LODMap=(s_BT_LODMap**)malloc(TerrainPtr->LODLevel[0].Split*sizeof(s_BT_LODMap*));
	if(TerrainPtr->LODMap==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
	for(unsigned long i=0;i<TerrainPtr->LODLevel[0].Split;i++)
	{
		TerrainPtr->LODMap[i]=(s_BT_LODMap*)malloc(TerrainPtr->LODLevel[0].Split*sizeof(s_BT_LODMap));
		if(TerrainPtr->LODMap[i]==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		memset(TerrainPtr->LODMap[i],0,TerrainPtr->LODLevel[0].Split*sizeof(s_BT_LODMap));
	}

//Vertex Declaration
	IDirect3DDevice9* D3DDevice=dbGetDirect3DDevice();
	//VD Array
		const D3DVERTEXELEMENT9 VD[5] =
		{
		  {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
		  {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,  0},
		  {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},
		  {0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,1},
		  D3DDECL_END()
		};
	//Make Vertex Declaration
		D3DDevice->CreateVertexDeclaration(VD,&TerrainPtr->VertexDeclaration);

//Initialise build
	BT_Main.Building=1;
	BT_Main.BuildType=0;
	BT_Main.CurrentBuildTerrain=TerrainPtr;
	BT_Main.CurrentBuildSector=0;
	BT_Main.CurrentBuildRow=0;
	BT_Main.CurrentBuildColumn=0;
	BT_Main.CurrentBuildTerrainSector=0;
	TerrainPtr->Built=1;

//Build
	if(GenerateTerrain==true)
	{
		unsigned long tempbuildstep=BT_Main.buildstep;
		BT_Main.buildstep=0;
		int null=BT_ContinueBuild();
		BT_Main.buildstep=tempbuildstep;
	}

//Add RTTMS update handler
	BT_RTTMS_AddUpdateHandler(TerrainID,BT_Intern_RTTMSUpdateHandler);

	return TerrainID;
}

EXPORT void BT_SaveTerrain(unsigned long TerrainID,char* FileName)
{
//Set current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SAVETERRAIN;

//Variables
	ofstream File;
	s_BT_terrain* TerrainPtr;
	unsigned short TempShort;
	s_BT_FileTerrainHeader101* TerrainHeader;

//Check that the terrain exists and it is built
	if(BT_Intern_TerrainExist(TerrainID)==true)
	{
		TerrainPtr=&BT_Main.Terrains[TerrainID];
		if(TerrainPtr->Built==true)
		{
		//Open the file
			File.open(FileName,ios::out | ios::app | ios::binary);

		//Write the magic number
			TempShort=21570; //This magically turns into "BT"
			File.write((char*)&TempShort,sizeof(unsigned short));

		//Write the filesystem version
			TempShort=101;
			File.write((char*)&TempShort,sizeof(unsigned short));

		//Write the minimum version of blitzterrain to read the file
			TempShort=C_FILE_MINBTREADVERSION;
			File.write((char*)&TempShort,sizeof(unsigned short));

		//Write the header size
			TempShort=sizeof(s_BT_FileTerrainHeader101);
			File.write((char*)&TempShort,sizeof(unsigned short));

		//Make the terrain header
			TerrainHeader=BT_Intern_MakeDefaultTerrainHeader101();
			BT_Intern_FillTerrainHeader101(TerrainHeader,TerrainPtr);

		//Write the terrain header
			File.write((char*)TerrainHeader,sizeof(s_BT_FileTerrainHeader101));

		//Free the terrain header
			free(TerrainHeader);

		//Make the LODLevel and Sector header
			s_BT_FileLODLevelHeader101* LODLevelHeader=BT_Intern_MakeDefaultLODLevelHeader101();
			s_BT_FileSectorHeader101* SectorHeader=BT_Intern_MakeDefaultSectorHeader101();

		//Loop through LODLevels
			for(unsigned long LODLevel=0;LODLevel<TerrainPtr->LODLevels;LODLevel++)
			{
			//Get the LODLevel pointer and fill the LODLevel Header
				s_BT_LODLevel* LODLevelPtr=&TerrainPtr->LODLevel[LODLevel];
				BT_Intern_FillLODLevelHeader101(LODLevelHeader,LODLevelPtr);

			//Write the header size
				TempShort=sizeof(s_BT_FileLODLevelHeader101);
				File.write((char*)&TempShort,sizeof(unsigned short));

			//Write the header
				File.write((char*)LODLevelHeader,sizeof(s_BT_FileLODLevelHeader101));

			//Loop through Sectors
				for(unsigned long Sector=0;Sector<LODLevelPtr->Sectors;Sector++)
				{
				//Get the Sector pointer and fill the Sector Header
					s_BT_Sector* SectorPtr=&LODLevelPtr->Sector[Sector];
					BT_Intern_FillSectorHeader101(SectorHeader,SectorPtr);

				//Write the header size
					TempShort=sizeof(s_BT_FileSectorHeader101);
					File.write((char*)&TempShort,sizeof(unsigned short));

				//Write the header
					File.write((char*)SectorHeader,sizeof(s_BT_FileSectorHeader101));

				//Make the quadmap
					//char* QuadMapBuffer;
					//unsigned long QuadMapBufferSize;
					//SectorPtr->QuadMap->MakeBuffer(&QuadMapBuffer,&QuadMapBufferSize);

				//Write the quadmap size
					//File.write((char*)&QuadMapBufferSize,sizeof(unsigned long));

				//Write the quadmap
					//File.write(QuadMapBuffer,QuadMapBufferSize);

				//Delete the quadmap
					//free((void*)QuadMapBuffer);
				}
			}

		//Free headers
			free(LODLevelHeader);
			free(SectorHeader);

		//Close the file
			File.close();
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINNOTBUILT);
			return;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}