#include "main.h"
#include "createobject.h"
#include "rttms.h"
#include "code-from-dbp.h"
#include "CObjectsC.h"
#include "CMemblocks.h"
#include "CGfxC.h"
#include ".\..\..\Shared\Objects\ShadowMapping\cShadowMaps.h"

bool update_mesh_light(sMesh* pMesh, sObject* pObject, sFrame* pFrame);

s_BT_main BT_Main;

// shadow mapping
extern CascadedShadowsManager g_CascadedShadow;

extern GlobStruct* g_pGlob;
extern void AddToRenderList(LPVOID pFunction, int iPriority);

int g_LevelToRender = 5;
int g_iQualityTechniqueMode = 0;
int g_iTerrainIDForShadowMap = 0;
bool g_bRenderTerrainForShadowMap = false;

// New DX11 Constant buffer structure
#ifdef DX11
struct CBChangePerTerrsainChunk
{
	KMaths::Matrix mWorld;
	KMaths::Matrix mView;
	KMaths::Matrix mProjection;
};
struct CBChangePerTerrsainChunkPS
{
	GGCOLOR vMaterialEmissive;
	float fAlphaOverride;
	float fRes1;
	float fRes2;
	float fRes3;
	KMaths::Matrix mViewInv;
	KMaths::Matrix mViewProj;
	KMaths::Matrix mPrevViewProj;
};
ID3D11Buffer* m_pCBChangePerTerrsainChunk = NULL;
ID3D11Buffer* m_pCBChangePerTerrsainChunkPS	= NULL;
#else
#endif

void SetTerrainRenderLevel( int size )
{
	float s = (float)size;
	if ( s < 1 ) s = 1;
	if ( s > 100 ) s = 100;

	// 1-5
	g_LevelToRender = (int)(ceil(s /= 20.0f));
}

#ifndef COMPILE_GDK

	void BTConstructor()
	{
		BT_Main.buildstep=1;

		// from passcore
		//CO_ReceiveCoreDataPtr(NULL);
		AddToRenderList(&BT_Intern_Render,7500);
		memset(&BT_Main,0,sizeof(s_BT_main));
		BT_Main.InstructionQueueSize=1000;
		BT_Main.InstructionQueueUsed=0;
		BT_Main.InstructionQueue=new char[BT_Main.InstructionQueueSize];
		BT_Main.InstructionQueue[0]=NULL;
		BT_Main.Initialised=true;
		BT_Main.buildstep=1;
	}
	
	void BTReceiveCoreDataPtr(LPVOID CorePtr)
	{
	}

	void BTDestructor()
	{
	}

#endif

//========================
// === BT MAKE TERRAIN ===
//========================
unsigned long BT_MakeTerrain()
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_MAKETERRAIN;

//Variables
	unsigned long TerrainNum;
	bool FoundID=0;

//Check that BT is initialised
	if(BT_Main.Initialised)
	{
	//Find a free terrain ID
		TerrainNum=1;
		while(FoundID==0)
		{
		//Check if we have gone over terrain limit and call an error
			if(TerrainNum>C_BT_MAXTERRAINS+1) 
			{
				BT_Intern_Error(C_BT_ERROR_MAXTERRAINSEXCEDED);
				return 0;
			}

			if(BT_Main.Terrains[TerrainNum].Exists)
			{
				TerrainNum++;
			}else{
				FoundID=1;
			}
		}

	//Set default values
		memset(&BT_Main.Terrains[TerrainNum],0,sizeof(s_BT_terrain));
		BT_Main.Terrains[TerrainNum].Exists=1;
		BT_Main.Terrains[TerrainNum].ID=TerrainNum;
		BT_Main.Terrains[TerrainNum].Scale=1.0f;
		BT_Main.Terrains[TerrainNum].YScale=1.0f;
		BT_Main.Terrains[TerrainNum].Tile=1.0f;
		BT_Main.Terrains[TerrainNum].LODLevels=1;
		BT_Main.Terrains[TerrainNum].LODLevel=(s_BT_LODLevel*)malloc(sizeof(s_BT_LODLevel)*C_BT_MAXLODLEVELS);
		if(BT_Main.Terrains[TerrainNum].LODLevel==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		memset ( BT_Main.Terrains[TerrainNum].LODLevel, 0, sizeof(s_BT_LODLevel)*C_BT_MAXLODLEVELS );
		BT_Main.Terrains[TerrainNum].Built=0;
		BT_Main.Terrains[TerrainNum].Generated=0;
		BT_Main.Terrains[TerrainNum].MeshOptimisation=true;
		BT_Main.Terrains[TerrainNum].ExclusionThreshold=128;
		BT_Main.Terrains[TerrainNum].DetailBlendMode=8;

	//Setup Environment map
		BT_Main.Terrains[TerrainNum].EnvironmentMap=(BT_EnvironmentMap*)malloc(sizeof(BT_EnvironmentMap));
		if(BT_Main.Terrains[TerrainNum].EnvironmentMap==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		memset(BT_Main.Terrains[TerrainNum].EnvironmentMap,0,sizeof(BT_EnvironmentMap));
		BT_Main.Terrains[TerrainNum].EnvironmentMap->EnvironmentBuffer=(unsigned long*)malloc(4);
		if(BT_Main.Terrains[TerrainNum].EnvironmentMap->EnvironmentBuffer==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

	//Setup TerrainInfo
		BT_TerrainInfo* TerrainInfo=(BT_TerrainInfo*)malloc(sizeof(BT_TerrainInfo));
		if(TerrainInfo==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		memset(TerrainInfo,0,sizeof(BT_TerrainInfo));
		TerrainInfo->Scale=1.0f;
		TerrainInfo->YScale=1.0f;
		TerrainInfo->Tile=1.0f;
		TerrainInfo->LODLevels=1;
		TerrainInfo->MeshOptimisation=true;
		TerrainInfo->ExclusionThreshold=128;
		TerrainInfo->DetailBlendMode=8;
		BT_Main.Terrains[TerrainNum].Info=(void*)TerrainInfo;

	//Return the terrain id
		return TerrainNum;

	}
	return NULL;
}
// === END FUNCTION ===



//=================================
// === BT SET TERRAIN HEIGHTMAP ===
//=================================
void BT_SetTerrainHeightmap(unsigned long terrainid,unsigned long image)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINHEIGHTMAP;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built==false)
		{
		//Set the values
			BT_Main.Terrains[terrainid].Heightmap=image;
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINALREADYBUILT);
			return;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}

// ==============================
// === BT SET TERRAIN TEXTURE ===
// ==============================
void BT_SetTerrainTexture(unsigned long terrainid,unsigned long image)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINTEXTURE;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built)
		{
		//Set the texture
			BT_Main.Terrains[terrainid].Texture=image;
			SetObjectBlendMap(BT_Main.Terrains[terrainid].ObjectID,0,image,10,4);
		}else{
		//Set the texture
			BT_Main.Terrains[terrainid].Texture=image;
		}
	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->Texture=image;
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ================================
// === BT SET TERRAIN EXCLUSION ===
// ================================
void BT_SetTerrainExclusion(unsigned long terrainid,unsigned long image)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINEXCLUSION;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built==false)
		{
		//Set the exclusionmap
#ifdef C_BT_FULLVERSION
			BT_Main.Terrains[terrainid].Exclusionmap=image;
		//Update in terrain info
			((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->Exclusion=true;
#endif
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINALREADYBUILT);
			return;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===


// ==========================================
// === BT SET TERRAIN EXCLUSION THRESHOLD ===
// ==========================================
void BT_SetTerrainExclusionThreshold(unsigned long terrainid,unsigned long threshold)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINEXCLUSIONTHRESHOLD;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built==false)
		{
		//Set the exclusion threshold
#ifdef C_BT_FULLVERSION
			BT_Main.Terrains[terrainid].ExclusionThreshold=unsigned char(threshold);

		//Update in terrain info
			((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->ExclusionThreshold=unsigned char(threshold);
#endif
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINALREADYBUILT);
			return;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// =============================
// === BT SET TERRAIN DETAIL ===
// =============================
void BT_SetTerrainDetail(unsigned long terrainid,unsigned long image)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINDETAIL;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built)
		{
		//Set the detailmap
			BT_Main.Terrains[terrainid].Detailmap=image;
			SetObjectBlendMap(BT_Main.Terrains[terrainid].ObjectID,1,image,11,BT_Main.Terrains[terrainid].DetailBlendMode);
		}else{
			BT_Main.Terrains[terrainid].Detailmap=image;
		}
	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->Detailmap=image;
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// =======================================
// === BT SET TERRAIN DETAIL BLENDMODE ===
// =======================================
void BT_SetTerrainDetailBlendMode(unsigned long terrainid,unsigned char mode)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINDETAILBLENDMODE;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built==true)
		{
		//Set the detail blendmode
			BT_Main.Terrains[terrainid].DetailBlendMode=mode;
			if(BT_Main.Terrains[terrainid].Detailmap)
				SetObjectBlendMap(BT_Main.Terrains[terrainid].ObjectID,1,BT_Main.Terrains[terrainid].Detailmap,11,mode);
		}else{
		//Set the detail blendmode
			BT_Main.Terrains[terrainid].DetailBlendMode=mode;
		}
	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->DetailBlendMode=mode;
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ==================================
// === BT SET TERRAIN ENVIRONMENT ===
// ==================================
void BT_SetTerrainEnvironment(unsigned long terrainid,unsigned long image)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINENVIRONMENT;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Set the environmentmap
		BT_Main.Terrains[terrainid].Environmentmap=image;
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ==================================
// === BT ADD TERRAIN ENVIRONMENT ===
// ==================================
unsigned long BT_AddTerrainEnvironment(unsigned long terrainid,unsigned long Colour)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_ADDTERRAINENVIRONMENT;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built==true)
		{
			BT_Intern_Error(C_BT_ERROR_CANNOTUSEFUNCTIONONBUILTTERRAIN);
			return 0;
		}else{
		//Add the environment
			return BT_Intern_AddEnvironment(BT_Main.Terrains[terrainid].EnvironmentMap,Colour);
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return 0;
	}
}
// === END FUNCTION ===



// ==========================
// === BT SET TERRAIN LOD ===
// ==========================
void BT_SetTerrainLOD(unsigned long terrainid,unsigned char LODLevels)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINLOD;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check the LOD Levels
		if(LODLevels>0 && LODLevels<=C_BT_MAXLODLEVELS)
		{
		//Set the LODLevels
			BT_Main.Terrains[terrainid].LODLevels=LODLevels;

		//Update in terrain info
			((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->LODLevels=LODLevels;
			
		}else{
			BT_Intern_Error(C_BT_ERROR_INVALIDLODLEVELS);
			return;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ============================
// === BT SET TERRAIN SPLIT ===
// ============================
void BT_SetTerrainSplit(unsigned long terrainid,unsigned long Split)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINSPLIT;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Set the Collision LODLevel
		BT_Main.Terrains[terrainid].LODLevel[0].Split=unsigned short(Split);

	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ==================================
// === BT SET TERRAIN DETAIL TILE ===
// ==================================
void BT_SetTerrainDetailTile(unsigned long terrainid,float Tile)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINDETAILTILE;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Set the Collision LODLevel
		BT_Main.Terrains[terrainid].Tile=Tile;

	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->Tile=Tile;

	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// =====================================
// === BT SET TERRAIN QUAD REDUCTION ===
// =====================================
void BT_SetTerrainQuadReduction(unsigned long terrainID, bool enabled)
{
	// Set Current function
	BT_Main.CurrentFunction = C_BT_FUNCTION_SETTERRAINQUADREDUCTION;

	// Check that the terrain exists
	if (!BT_Intern_TerrainExist(terrainID))
		return;

	// Check that this is the full version
	#ifndef C_BT_FULLVERSION 
		return;
	#endif

	// Set quad reduction
	BT_Main.Terrains[terrainID].QuadReduction=enabled;
	((BT_TerrainInfo*)BT_Main.Terrains[terrainID].Info)->QuadReduction=enabled;
}
// === END FUNCTION ===



// ====================================
// === BT SET TERRAIN QUAD ROTATION ===
// ====================================
void BT_SetTerrainQuadRotation(unsigned long terrainid,bool Enabled)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINQUADROTATION;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Set Quad rotation
#ifdef C_BT_FULLVERSION
		BT_Main.Terrains[terrainid].QuadRotation=Enabled;
#endif
	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->QuadRotation=Enabled;
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ================================
// === BT SET TERRAIN SMOOTHING ===
// ================================
void BT_SetTerrainSmoothing(unsigned long terrainid,unsigned long Amount)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINSMOOTHING;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Limit amount
		if(Amount>100)
			Amount=100;
			
	//Set the Collision LODLevel
		BT_Main.Terrains[terrainid].Smoothing=Amount;

	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->Smoothing=(Amount>0);
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->SmoothAmount=Amount;
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ============================
// === BT SET TERRAIN SCALE ===
// ============================
void BT_SetTerrainScale(unsigned long terrainid,float Scale)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINSCALE;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Set the Scale
		BT_Main.Terrains[terrainid].Scale=Scale;

	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->Scale=Scale;

	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// =============================
// === BT SET TERRAIN YSCALE ===
// =============================
void BT_SetTerrainYScale(unsigned long terrainid,float YScale)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINYSCALE;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Set the Collision LODLevel
		BT_Main.Terrains[terrainid].YScale=YScale;

	//Update in terrain info
		((BT_TerrainInfo*)BT_Main.Terrains[terrainid].Info)->YScale=YScale;

	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ===================================
// === BT SET TERRAIN LOD DISTANCE ===
// ===================================
void BT_SetTerrainLODDistance(unsigned long terrainid,unsigned char LODLevel,float value)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETTERRAINLODDISTANCES;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(LODLevel<BT_Main.Terrains[terrainid].LODLevels)
		{
		//Check if the terrain is built
			if(BT_Main.Terrains[terrainid].Built==true){
				((BT_LODLevelInfo*)BT_Main.Terrains[terrainid].LODLevel[LODLevel].Info)->Distance=value;
				BT_Main.Terrains[terrainid].LODLevel[LODLevel].Distance=value/BT_Main.Terrains[terrainid].Scale*C_BT_INTERNALSCALE;
			}else{
				BT_Main.Terrains[terrainid].LODLevel[LODLevel].Distance=value;
			}

		}else{

			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return;
		}

	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// ========================
// === BT BUILD TERRAIN ===
// ========================
void BT_BuildTerrain(unsigned long terrainid,unsigned long ObjectID)
{
	BT_BuildTerrain(terrainid,ObjectID,false);
}

void BT_BuildTerrain(unsigned long terrainid,unsigned long ObjectID,bool GenerateTerrain)
{

//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_BUILDTERRAIN;

//Variables
	unsigned long HeightmapImg;
	int size;
	unsigned long Heightmapmemblock;
	unsigned short x;
	unsigned short y;
	unsigned short lx;
	unsigned short ly;
	unsigned long currentvertex;
	unsigned short Sectors;
	unsigned short Sector;
	unsigned short Column;
	unsigned short Row;
	unsigned char LODLevel;
	unsigned long MaxLODLevels;
	BT_Quadmap_Generator Generator;
	unsigned long Excludememblock;
	s_BT_terrain* Terrain;

//Check that the system isnt building
	if(BT_Main.Building)
		BT_Intern_Error(C_BT_ERROR_ALREADYBUILDING);

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Get the terrain pointer
		Terrain=&BT_Main.Terrains[terrainid];

	//Check that the terrain isnt built
		if(Terrain->Built==true)
			BT_Intern_Error(C_BT_ERROR_TERRAINALREADYBUILT);

	//Find the heightmap image
		HeightmapImg=Terrain->Heightmap;

	//Check all the images

		//Heightmap
		if(BT_Intern_ImageExist(HeightmapImg)==false)
		{
			BT_Intern_Error(C_BT_ERROR_HEIGHTMAPDOESNTEXIST);
			return;
		}

		//Texture
		if(BT_Intern_ImageExist(Terrain->Texture)==false)
			Terrain->Texture=0;

		//Detailmap
		if(BT_Intern_ImageExist(Terrain->Detailmap)==false)
			Terrain->Detailmap=0;

		//Environmentmap
		if(BT_Intern_ImageExist(Terrain->Environmentmap)==false)
			Terrain->Environmentmap=0;

		//Exclusionmap
		if(BT_Intern_ImageExist(Terrain->Exclusionmap)==false)
			Terrain->Exclusionmap=0;


	//Open heightmap
		//Find free memblock
		Heightmapmemblock=0;
		do {
			Heightmapmemblock++;
		} while ( !( MemblockExist(Heightmapmemblock) == 0 ) );

		//Make memblock from heightmap
		CreateMemblockFromImage(Heightmapmemblock,HeightmapImg);
		unsigned long* HeightmapMemblockPtr=(unsigned long*)GetMemblockPtr(Heightmapmemblock);


	//Check heightmap size
		//Width and height must be equal
		if(HeightmapMemblockPtr[0]!=HeightmapMemblockPtr[1])
			BT_Intern_Error(C_BT_ERROR_HEIGHTMAPSIZEINVALID);

		//Dimensions must be power of 2 numbers
		size=HeightmapMemblockPtr[0];
		if((size & -size) != size)
			BT_Intern_Error(C_BT_ERROR_HEIGHTMAPSIZEINVALID);

		//Size must be equal to or greater than 64
		if(size<128)
			BT_Intern_Error(C_BT_ERROR_HEIGHTMAPSIZEINVALID);

		//Size must be less or equal to C_BT_MAXTERRAINSIZE
		if(size>C_BT_MAXTERRAINSIZE)
			BT_Intern_Error(C_BT_ERROR_HEIGHTMAPSIZEINVALID);

	//Set heightmap size
		Terrain->Heightmapsize=unsigned short(HeightmapMemblockPtr[0]);
		((BT_TerrainInfo*)Terrain->Info)->Heightmapsize=Terrain->Heightmapsize;

	//Exclusionmap must be same size as heightmap
		if(Terrain->Exclusionmap>0)
		{
			size=ImageWidth(Terrain->Exclusionmap);
			if(size=!ImageWidth(HeightmapImg))
				BT_Intern_Error(C_BT_ERROR_EXCLUSIONMAPSIZEINVALID);
		}

	//Give split an autovalue if not set
		if(Terrain->LODLevel[0].Split==0)
			Terrain->LODLevel[0].Split=Terrain->Heightmapsize/32;

	//Check that split isnt too small
		if(Terrain->Heightmapsize/Terrain->LODLevel[0].Split>64)
			BT_Intern_Error(C_BT_ERROR_SECTORTOOBIG);

	//Work out some variables
		//Terrain size
		Terrain->TerrainSize=(Terrain->Heightmapsize-1)*C_BT_INTERNALSCALE;
		unsigned short Split=Terrain->LODLevel[0].Split;
		for(LODLevel=0;LODLevel<Terrain->LODLevels;LODLevel++)
		{
			//Split
			Terrain->LODLevel[LODLevel].Split=Split;
			Split=Split/2;

			//Sector size*
			Terrain->LODLevel[LODLevel].SectorSize=(Terrain->TerrainSize+C_BT_INTERNALSCALE)/Terrain->LODLevel[LODLevel].Split;

			//Sector detail
			Terrain->LODLevel[LODLevel].SectorDetail=Terrain->Heightmapsize/Terrain->LODLevel[0].Split;
		}

	//Check the LOD levels
		MaxLODLevels=int(log10((float)Terrain->LODLevel[0].Split)/log10((float)2))+1;
		if(Terrain->LODLevels>(char)MaxLODLevels)
		{
			Terrain->LODLevels=(char)MaxLODLevels;
			Terrain->LODLevel=(s_BT_LODLevel*)realloc(Terrain->LODLevel,MaxLODLevels*sizeof(s_BT_LODLevel));
			if(Terrain->LODLevel==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		}

	//Load Heightmap

		//Allocate points
		Terrain->HeightPoint=(float*)malloc(Terrain->Heightmapsize*Terrain->Heightmapsize*sizeof(float));
		if(Terrain->HeightPoint==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

		//Fill the points with data
		currentvertex=0;
		Terrain->ATMode=BT_Main.ATMode;
		for(ly=0;ly<Terrain->Heightmapsize;ly++)
		{
			for(lx=0;lx<Terrain->Heightmapsize;lx++)
			{
				if(Terrain->ATMode)
				{
					x=lx;
					y=Terrain->Heightmapsize-ly-1;
				}else{
					x=lx;
					y=ly;
				}
				Terrain->HeightPoint[currentvertex]=BT_Intern_GetHeightFromColor(HeightmapMemblockPtr[3+x+y*Terrain->Heightmapsize])*Terrain->YScale;
				currentvertex++;
			}
		}

	//Delete memblock
		DeleteMemblock(Heightmapmemblock);

	//Smooth terrain
		// LEENOTE - 070314 - Dave, why did you comment this out on 070314?
		BT_Intern_SmoothTerrain(Terrain);

	//Make exclusion memblock
		if(Terrain->Exclusionmap>0)
		{
			Excludememblock=Heightmapmemblock;
			CreateMemblockFromImage(Excludememblock,Terrain->Exclusionmap);
		}else{
			Excludememblock=0;
		}

	//Make object
		Terrain->ObjectID=ObjectID;
		Terrain->Object=BT_Intern_CreateBlankObject(ObjectID,1);
		BT_Intern_SetupMesh(Terrain->Object->pFrame->pMesh,3,0,GGFVF_DIFFUSE);
		BT_Intern_FinnishObject(ObjectID);
		if(Terrain->Texture!=NULL)
			SetObjectBlendMap(ObjectID,0,Terrain->Texture,10,4);
		if(Terrain->Detailmap!=NULL)
			SetObjectBlendMap(ObjectID,1,Terrain->Detailmap,11,Terrain->DetailBlendMode);

	//Make sectors
		Terrain->Sectors=0;
		Generator.Size=Terrain->LODLevel[0].SectorDetail;
		if(Excludememblock>0)
		{
			Generator.exclusion=(bool*)malloc((Generator.Size+1)*(Generator.Size+1)*sizeof(bool));
			if(Generator.exclusion==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		}else{
			Generator.exclusion=NULL;
		}
		Generator.heights=(float*)malloc((Generator.Size+1)*(Generator.Size+1)*sizeof(float));
		if(Generator.heights==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		BT_Intern_StartQuadMapGeneration(Generator);

		for(LODLevel=0;LODLevel<Terrain->LODLevels;LODLevel++)
		{
			s_BT_LODLevel* LODLevelPtr=&Terrain->LODLevel[LODLevel];

		//Scale distance
			LODLevelPtr->Distance=LODLevelPtr->Distance/Terrain->Scale*C_BT_INTERNALSCALE;

		//Allocate sectors
			Sectors=LODLevelPtr->Split*LODLevelPtr->Split;
			Terrain->Sectors+=Sectors;
			LODLevelPtr->Sectors=Sectors;
			LODLevelPtr->Sector=(s_BT_Sector*)malloc(Sectors*sizeof(s_BT_Sector));
			if(LODLevelPtr->Sector==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
			memset(LODLevelPtr->Sector, 0, Sectors*sizeof(s_BT_Sector));
			LODLevelPtr->Terrain=Terrain;
			LODLevelPtr->ID=LODLevel;

		//Initialise QuadMap Generator
			Generator.QuadRotation=Terrain->QuadRotation;
			Generator.QuadReduction=Terrain->QuadReduction;
			LODLevelPtr->TileSpan=Terrain->LODLevel[0].Split/LODLevelPtr->Split;
			Generator.TileSize=C_BT_INTERNALSCALE*LODLevelPtr->TileSpan;
			Generator.Optimise=Terrain->MeshOptimisation;
			Generator.LODLevel=LODLevel;
			Generator.TileSpan=1;

		//LODLevel Info
			BT_LODLevelInfo* LODLevelInfo=(BT_LODLevelInfo*)malloc(sizeof(BT_LODLevelInfo));
			if(LODLevelInfo==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
			memset ( LODLevelInfo, 0, sizeof(BT_LODLevelInfo) );
			LODLevelInfo->Distance=Terrain->LODLevel[LODLevel].Distance;
			LODLevelInfo->SectorDetail=Terrain->LODLevel[LODLevel].SectorDetail;
			LODLevelInfo->Sectors=Terrain->LODLevel[LODLevel].Sectors;
			LODLevelInfo->SectorSize=Terrain->LODLevel[LODLevel].SectorSize;
			LODLevelInfo->Split=Terrain->LODLevel[LODLevel].Split;
			LODLevelInfo->TileSpan=Terrain->LODLevel[LODLevel].TileSpan;
			LODLevelInfo->InternalData=(void*)LODLevelPtr;
			LODLevelPtr->Info=(void*)LODLevelInfo;
			LODLevelPtr->DBPObject = 0;

			Row=0;
			Column=0;
			for(Sector=0;Sector<Sectors;Sector++)
			{
			//Calculate row and column
				if(Row==LODLevelPtr->Split)
				{
					Column++;
					Row=0;
				}

			//Get sector pointer
				s_BT_Sector* SectorPtr=&LODLevelPtr->Sector[Sector];

			//Fill Sector structure
				SectorPtr->ID=Sector;
				SectorPtr->Column=Column;
				SectorPtr->Row=Row;
				SectorPtr->Pos_x=float(Column*Terrain->LODLevel[LODLevel].SectorSize+0.5*Terrain->LODLevel[LODLevel].SectorSize);
				SectorPtr->Pos_y=0.0f;
				SectorPtr->Pos_z=float(Row*Terrain->LODLevel[LODLevel].SectorSize+0.5*Terrain->LODLevel[LODLevel].SectorSize);
				Generator.Sector=SectorPtr;
				SectorPtr->LODLevel=&Terrain->LODLevel[LODLevel];
				SectorPtr->Terrain=Terrain;
				SectorPtr->DBPObject = 0;
				Generator.RemoveFarX=false;
				if(Row==LODLevelPtr->Split-1)
					Generator.RemoveFarX=true;
				Generator.RemoveFarZ=false;
				if(Column==LODLevelPtr->Split-1)
					Generator.RemoveFarZ=true;

			//Get heights for this sector
				BT_Intern_GetSectorHeights(Terrain,LODLevel,Row,Column,Generator.heights);

				Terrain->LODLevel[LODLevel].Sector[Sector].Excluded=false;
				if(Excludememblock>0)
				{
					if(BT_Intern_GetSectorExclusion(Terrain,LODLevel,Excludememblock,Row,Column,Generator.exclusion)==true)
						SectorPtr->Excluded=true;
				}

				if(Terrain->LODLevel[LODLevel].Sector[Sector].Excluded==false)
				{
				//RTTMS
					Terrain->LODLevel[LODLevel].Sector[Sector].VertexDataRTTMS=(BT_RTTMS_STRUCT*)malloc(sizeof(BT_RTTMS_STRUCT));
					if(Terrain->LODLevel[LODLevel].Sector[Sector].VertexDataRTTMS==nullptr)
						BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
					memset ( Terrain->LODLevel[LODLevel].Sector[Sector].VertexDataRTTMS, 0, sizeof(BT_RTTMS_STRUCT) );
					BT_RTTMS_STRUCTINTERNALS* RTTMSStructInternals=(BT_RTTMS_STRUCTINTERNALS*)malloc(sizeof(BT_RTTMS_STRUCTINTERNALS));
					if(RTTMSStructInternals==nullptr)
						BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
					RTTMSStructInternals->TerrainID=terrainid;
					RTTMSStructInternals->LODLevelID=LODLevel;
					RTTMSStructInternals->SectorID=Sector;
					RTTMSStructInternals->SectorPtr=SectorPtr;
					SectorPtr->VertexDataRTTMS->Internals=(void*)RTTMSStructInternals;

					SectorPtr->QuadMap=(BT_QuadMap*)malloc(sizeof(BT_QuadMap));
					if(SectorPtr->QuadMap==nullptr)
						BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
					memset(SectorPtr->QuadMap,0,sizeof(BT_QuadMap));
					SectorPtr->QuadMap->Generate(Generator);
				}

				//Sector Info
				BT_SectorInfo* SectorInfo=(BT_SectorInfo*)malloc(sizeof(BT_SectorInfo));
				if(SectorInfo==nullptr)
					BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
				memset ( SectorInfo, 0, sizeof(BT_SectorInfo) );
				SectorInfo->Column=Column;
				SectorInfo->Row=Row;
				SectorInfo->Excluded=SectorPtr->Excluded;
				SectorInfo->Pos_x=SectorPtr->Pos_x;
				SectorInfo->Pos_y=0.0f;
				SectorInfo->Pos_z=SectorPtr->Pos_z;
				SectorInfo->WorldMatrix=&SectorPtr->WorldMatrix;
				SectorInfo->InternalData=(void*)SectorPtr;
				SectorPtr->Info=(void*)SectorInfo;

				Row++;
			}
		}

	//Post process quadmap
		for(LODLevel=0;LODLevel<Terrain->LODLevels;LODLevel++)
		{
			Generator.TileSize=C_BT_INTERNALSCALE*Terrain->LODLevel[LODLevel].TileSpan;
			Generator.LODLevel=LODLevel;
			for(Sector=0;Sector<Terrain->LODLevel[LODLevel].Sectors;Sector++)
			{
				s_BT_Sector* SectorPtr=&Terrain->LODLevel[LODLevel].Sector[Sector];
				if(SectorPtr->Excluded==false)
				{
					SectorPtr->QuadMap->Above=SectorPtr->QuadMap->Below=SectorPtr->QuadMap->Left=SectorPtr->QuadMap->Right=0;
					if(SectorPtr->Column!=0)
					{
						s_BT_Sector* OtherSectorPtr=&Terrain->LODLevel[LODLevel].Sector[Sector-Terrain->LODLevel[LODLevel].Split];
						if(OtherSectorPtr->Excluded==false)
							SectorPtr->QuadMap->Above=OtherSectorPtr->QuadMap;
					}
					if(SectorPtr->Row!=Terrain->LODLevel[LODLevel].Split-1)
					{
						s_BT_Sector* OtherSectorPtr=&Terrain->LODLevel[LODLevel].Sector[Sector+1];
						if(OtherSectorPtr->Excluded==false)
							SectorPtr->QuadMap->Right=OtherSectorPtr->QuadMap;
					}
					if(SectorPtr->Row!=0)
					{
						s_BT_Sector* OtherSectorPtr=&Terrain->LODLevel[LODLevel].Sector[Sector-1];
						if(OtherSectorPtr->Excluded==false)
							SectorPtr->QuadMap->Left=OtherSectorPtr->QuadMap;
					}
					if(SectorPtr->Column!=Terrain->LODLevel[LODLevel].Split-1)
					{
						s_BT_Sector* OtherSectorPtr=&Terrain->LODLevel[LODLevel].Sector[Sector+Terrain->LODLevel[LODLevel].Split];
						if(OtherSectorPtr->Excluded==false)
							SectorPtr->QuadMap->Below=OtherSectorPtr->QuadMap;
					}
					
					SectorPtr->QuadMap->CalculateNormals();
				}
			}
		}

	//Delete exclusion
		if(Excludememblock>0)
			DeleteMemblock(Excludememblock);
		if(Generator.exclusion!=NULL)
			free(Generator.exclusion);

	//Load Environment map
		if(Terrain->Environmentmap>0)
		{
		//Convert Image into Memblock (recycle heightmap memblock)
			CreateMemblockFromImage(Heightmapmemblock,Terrain->Environmentmap);

		//Create the environment map
			BT_Intern_CreateEnvironmentMap(Terrain->EnvironmentMap,ImageWidth(Terrain->Environmentmap),ImageHeight(Terrain->Environmentmap),(unsigned long*)(GetMemblockPtr(Heightmapmemblock)+4));

		//Delete the Memblock
			DeleteMemblock(Heightmapmemblock);
		}

	//Allocate quadtree
		Terrain->QuadTree=BT_Intern_AllocateQuadTree(Terrain);

	//Create LOD Map
		Terrain->LODMap=(s_BT_LODMap**)malloc(Terrain->LODLevel[0].Split*sizeof(s_BT_LODMap*));
		if(Terrain->LODMap==nullptr)
			BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
		memset ( Terrain->LODMap, 0, Terrain->LODLevel[0].Split*sizeof(s_BT_LODMap*) );
		for(unsigned long i=0;i<Terrain->LODLevel[0].Split;i++)
		{
			Terrain->LODMap[i]=(s_BT_LODMap*)malloc(Terrain->LODLevel[0].Split*sizeof(s_BT_LODMap));
			if(Terrain->LODMap[i]==nullptr)
				BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
			memset(Terrain->LODMap[i],0,Terrain->LODLevel[0].Split*sizeof(s_BT_LODMap));
		}

		//Vertex Declaration
		#ifdef DX11

		// VD Array (critical that shaders are loaded prior to this step, unlike DX9)
		int iLayoutSize = 4;
		D3D11_INPUT_ELEMENT_DESC* pLayout = new D3D11_INPUT_ELEMENT_DESC [ iLayoutSize ];
		D3D11_INPUT_ELEMENT_DESC layout [ ] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		std::memcpy ( pLayout, layout, sizeof ( layout ) );

		// Get FIRST vertex shader input layout found in shader
		ID3DBlob* pBlob = g_sShaders[SHADERSTERRAINBASIC].pBlob;
		DWORD tIndex = 0;
		ID3DX11EffectTechnique* tech = NULL;
		while((tech = g_sShaders[SHADERSTERRAINBASIC].pEffect->GetTechniqueByIndex(tIndex++))->IsValid())
		{
			DWORD pIndex = 0;
			ID3DX11EffectPass* pass = NULL;
			while((pass = tech->GetPassByIndex(pIndex++))->IsValid())
			{
				D3DX11_PASS_SHADER_DESC vs_desc;
				pass->GetVertexShaderDesc(&vs_desc);
				D3DX11_EFFECT_SHADER_DESC s_desc;
				vs_desc.pShaderVariable->GetShaderDesc(0, &s_desc);
				HRESULT hr = m_pD3D->CreateInputLayout ( pLayout, iLayoutSize, s_desc.pBytecode, s_desc.BytecodeLength, &Terrain->VertexDeclaration );
				break;
			}
			if ( Terrain->VertexDeclaration != NULL ) break;
		}
		SAFE_DELETE_ARRAY(pLayout);

		#else
		//VD Array
		IGGDevice* D3DDevice=m_pD3D;
		const GGVERTEXELEMENT VD[5] =
		{
			{0, 0,  GGDECLTYPE_FLOAT3, GGDECLMETHOD_DEFAULT, GGDECLUSAGE_POSITION,0},
			{0, 12, GGDECLTYPE_FLOAT3, GGDECLMETHOD_DEFAULT, GGDECLUSAGE_NORMAL,  0},
			{0, 24, GGDECLTYPE_FLOAT2, GGDECLMETHOD_DEFAULT, GGDECLUSAGE_TEXCOORD,0},
			{0, 32, GGDECLTYPE_FLOAT2, GGDECLMETHOD_DEFAULT, GGDECLUSAGE_TEXCOORD,1},
			GDECL_END()
		};
		//Make Vertex Declaration
		D3DDevice->CreateVertexDeclaration(VD,&Terrain->VertexDeclaration);
		#endif

	//Cleanup
		BT_Intern_EndQuadMapGeneration();
		free(Generator.heights);
		free(Terrain->HeightPoint);

	//Terrain Info
		((BT_TerrainInfo*)Terrain->Info)->Built=true;
		((BT_TerrainInfo*)Terrain->Info)->Generated=GenerateTerrain;
		((BT_TerrainInfo*)Terrain->Info)->ATMode=Terrain->ATMode;
		((BT_TerrainInfo*)Terrain->Info)->Sectors=Terrain->Sectors;
		((BT_TerrainInfo*)Terrain->Info)->TerrainSize=Terrain->TerrainSize/C_BT_INTERNALSCALE*Terrain->Scale;
		((BT_TerrainInfo*)Terrain->Info)->InternalData=(void*)Terrain;
		((BT_TerrainInfo*)Terrain->Info)->DBPObjectPtr=(void*)Terrain->Object;

	//Initialise build
		BT_Main.Building=1;
		BT_Main.BuildType=0;
		BT_Main.CurrentBuildTerrain=Terrain;
		BT_Main.CurrentBuildSector=0;
		BT_Main.CurrentBuildRow=0;
		BT_Main.CurrentBuildColumn=0;
		BT_Main.CurrentBuildTerrainSector=0;
		Terrain->Built=1;

	//Build
		if(GenerateTerrain==true)
		{
			unsigned long tempbuildstep=BT_Main.buildstep;
			BT_Main.buildstep=0;
			int null=BT_ContinueBuild();
			BT_Main.buildstep=tempbuildstep;
		}

	//Add RTTMS update handler
		BT_RTTMS_AddUpdateHandler(terrainid,BT_Intern_RTTMSUpdateHandler);

	//Fix LOD seams
		BT_Intern_FixLODSeams(Terrain);

	//Set autocam
		// TODO: Stick some proper values here
		SetAutoCam(0.0,0.0,0.0,0.0);
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===



// =========================
// === BT CONTINUE BUILD ===
// =========================
int BT_ContinueBuild()
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_CONTINUEBUILD;

//Variables
	int Progress=0;
	int numsectorstomake;
	int I;
	float Progress_flt;

//Check that its building
	if(BT_Main.Building==true)
	{
		if(BT_Main.buildstep>BT_Main.CurrentBuildTerrain->Sectors)
			BT_Main.buildstep=0;

	//Work out the amount of sectors which need to be made
		if(BT_Main.buildstep==0)
		{
			numsectorstomake=BT_Main.CurrentBuildTerrain->Sectors-BT_Main.CurrentBuildTerrainSector;
		}else{
			numsectorstomake=BT_Main.buildstep;
		}
		if(BT_Main.CurrentBuildTerrainSector+numsectorstomake>BT_Main.CurrentBuildTerrain->Sectors)
			numsectorstomake=BT_Main.CurrentBuildTerrain->Sectors-BT_Main.CurrentBuildTerrainSector;

	//Loop
		for(I=0;I<numsectorstomake;I++)
		{
			BT_Intern_ContinueBuild();

		}

		if(BT_Main.CurrentBuildTerrainSector==BT_Main.CurrentBuildTerrain->Sectors)
		{
			Progress=-1;
			BT_Main.Building=0;
			BT_Main.CurrentBuildTerrain->Generated=true;
			((BT_TerrainInfo*)BT_Main.CurrentBuildTerrain->Info)->Generated=true;
			BT_Main.CurrentBuildTerrain=NULL;
			BT_Main.CurrentBuildLODLevel=0;
			BT_Main.CurrentBuildSector=0;
			BT_Main.CurrentBuildRow=0;
			BT_Main.CurrentBuildColumn=0;
			return -1;
		}

	//Get the progress
		Progress_flt=float((float(BT_Main.CurrentBuildTerrainSector)/float(BT_Main.CurrentBuildTerrain->Sectors))*100.0);
		Progress=int(Progress_flt);
	}
	return Progress;
}
// === END FUNCTION ===



// ========================
// === BT TERRAIN EXIST ===
// ========================
unsigned long BT_TerrainExist(unsigned long TerrainID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_TERRAINEXIST;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
		return 1;
	}

//Return false
	return 0;
}
// === END FUNCTION ===



// =========================
// === BT DELETE TERRAIN ===
// =========================
void BT_DeleteTerrain(unsigned long TerrainID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_DELETETERRAIN;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
	//Delete the terrain
		BT_Intern_DeleteTerrain(TerrainID,true);
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}

}
// === END FUNCTION ===

void BT_ForceTerrainTechnique(unsigned long QualityTechniqueMode)
{
	g_iQualityTechniqueMode = QualityTechniqueMode;
}

// ===========================
// === BT GET GROUND HEIGHT ==
// ===========================
float BT_GetGroundHeight(unsigned long terrainid,float x,float z,bool Round)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETGROUNDHEIGHT;

//Variables
	float Height=0.0;

//Check that the terrain exists and has been built
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built)
		{
		//Get the height
			Height=BT_Intern_GetPointHeight(&BT_Main.Terrains[terrainid],x/BT_Main.Terrains[terrainid].Scale*C_BT_INTERNALSCALE,z/BT_Main.Terrains[terrainid].Scale*C_BT_INTERNALSCALE,0,Round);
			return Height;
		}
	}
	return Height;
}

float BT_GetGroundHeight(unsigned long terrainid,float x,float z)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETGROUNDHEIGHT;

//Variables
	float Height=0.0;

//Check that the terrain exists and has been built
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built)
		{
		//Get the height
			Height=BT_Intern_GetPointHeight(&BT_Main.Terrains[terrainid],x/BT_Main.Terrains[terrainid].Scale*C_BT_INTERNALSCALE,z/BT_Main.Terrains[terrainid].Scale*C_BT_INTERNALSCALE,0,0);
			return Height;
		}
	}
	return Height;
}

// ===========================
// === BT GET TERRAIN SIZE ===
// ===========================
float BT_GetTerrainSize(unsigned long terrainid)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETTERRAINSIZE;

//Variables
	float Size=0.0;

//Check that the terrain exists and has been built
	if(BT_Intern_TerrainExist(terrainid))
	{
		if(BT_Main.Terrains[terrainid].Built)
		{
		//Get the size
			Size=BT_Main.Terrains[terrainid].TerrainSize/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
			return Size;
		}
	}
	return Size;
}



// =============================
// === BT GET POINT EXCLUDED ===
// =============================
unsigned long BT_GetPointExcluded(unsigned long terrainid,float x,float z)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETPOINTEXCLUDED;

//Variables
	float Height=0.0;

//Check that the terrain exists and has been built
	if(BT_Intern_TerrainExist(terrainid)==true)
	{
		if(BT_Main.Terrains[terrainid].Built==true)
		{
		//Return if it is excluded
			return BT_Intern_GetPointExcluded(&BT_Main.Terrains[terrainid],x/BT_Main.Terrains[terrainid].Scale*C_BT_INTERNALSCALE,z/BT_Main.Terrains[terrainid].Scale*C_BT_INTERNALSCALE);
		}
	}
	return false;
}
// === END FUNCTION ===



// ================================
// === BT GET POINT ENVIRONMENT ===
// ================================
unsigned long BT_GetPointEnvironment(unsigned long terrainid,float x,float z)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETPOINTENVIRONMENT;

//Variables
	float Height=0.0;

//Check that the terrain exists and has been built
	if(BT_Intern_TerrainExist(terrainid)==true)
	{
		if(BT_Main.Terrains[terrainid].Built==true)
		{
		//Check the range
			if(	x>0 && BT_Main.Terrains[terrainid].TerrainSize/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale>x &&
				z>0 && BT_Main.Terrains[terrainid].TerrainSize/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale>z)
			{
			//Transform coordinates to fit on environment map
				x=(x*BT_Main.Terrains[terrainid].EnvironmentMap->Width)/BT_Main.Terrains[terrainid].Heightmapsize;
				z=(z*BT_Main.Terrains[terrainid].EnvironmentMap->Width)/BT_Main.Terrains[terrainid].Heightmapsize;

			//Return the points environment
				return BT_Intern_GetPointEnvironment(BT_Main.Terrains[terrainid].EnvironmentMap,unsigned long(x/BT_Main.Terrains[terrainid].Scale),unsigned long(z/BT_Main.Terrains[terrainid].Scale));
			}else{
				return false;
			}
		}
	}
	return false;
}
// === END FUNCTION ===



// ======================
// === BT GET VERSION ===
// ======================
DWORD BT_GetVersion()
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETVERSION;

//Variables
	LPSTR Version=BT_VERSION;

	return (DWORD)Version;
}

// =========================
// === BT SET BUILD STEP ===
// =========================
void BT_SetBuildStep(unsigned long step)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETBUILDSTEP;

//Set build step
	BT_Main.buildstep=step;
}
// === END FUNCTION ===



// ======================
// === BT SET AT MODE ===
// ======================
void BT_SetATMode(bool ATMode)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETATMODE;

//Set AT mode
	BT_Main.ATMode=ATMode;
}
// === END FUNCTION ===



// =============================
// === BT ENABLE AUTO RENDER ===
// =============================
void BT_EnableAutoRender(bool AutoRender)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_ENABLEAUTORENDER;

//Set AT mode
	BT_Main.AutoRender=AutoRender;
}
// === END FUNCTION ===



// ========================
// === BT GET STATISTIC ===
// ========================
unsigned long BT_GetStatistic(unsigned long code)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSTATISTIC;

//Find stat code and return it
	if(code==1)
		return BT_Main.DrawPrimitiveCount;

	if(code==2)
		return BT_Main.DrawCalls;

	if(code==3)
		return BT_Main.CullChecks;

	return 0;
}
// === END FUNCTION ===



// ================================
// === BT GET TERRAIN OBJECT ID ===
// ================================
unsigned long BT_GetTerrainObjectID(unsigned long terrainid)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETOBJECTID;

//Check if terrain exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Return objectID
		return BT_Main.Terrains[terrainid].ObjectID;
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return 0;
	}
}
// === END FUNCTION ===



// =============================
// === BT MAKE SECTOR OBJECT ===
// =============================
void BT_MakeSectorObject(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID,unsigned long ObjectID)
{
	//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_MAKESECTOROBJECT;

	//Check that the Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
		//Check that the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
			//Check that the Sector exists
			if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors>SectorID)
			{
				//Check that the sector is not excluded
				if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Excluded==0)
				{
					// Add or Wipe
					if ( ObjectID==0 )
					{
						BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].DBPObject = 0;
						BT_Main.Terrains[terrainid].LODLevel[LODLevel].DBPObject=0;
					}
					else
					{
						//Generate the object
						sObject* Object=BT_Intern_CreateBlankObject(ObjectID,1);

						//Generate the mesh
						BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].QuadMap->GenerateDBPMesh(Object->pFrame->pMesh);

						// for now only reference the actual LOD1 sector as we can guarentee this clipping is perfect.
						BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].DBPObject = Object;

						//Finnish the object
						Object->bExcluded=true;
						BT_Intern_FinnishObject(ObjectID);

						//Position and scale object to terrains position and scale
						Object->position.vecPosition.x=BT_Main.Terrains[terrainid].Object->position.vecPosition.x+BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_x/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
						Object->position.vecPosition.y=BT_Main.Terrains[terrainid].Object->position.vecPosition.y+BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_y/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
						Object->position.vecPosition.z=BT_Main.Terrains[terrainid].Object->position.vecPosition.z+BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_z/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
						Object->position.vecScale=BT_Main.Terrains[terrainid].Object->position.vecScale;

						//Record object ref (for later collision mesh updating when raise terrain)
						BT_Main.Terrains[terrainid].LODLevel[LODLevel].DBPObject=Object;
					}

				}else{
					BT_Intern_Error(C_BT_ERROR_SECTORISEXCLUDED);
				}
			}else{
				BT_Intern_Error(C_BT_ERROR_SECTORDOESNTEXIST);
			}
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
	}
}
// === END FUNCTION ===



// ==============================
// === BT MAKE TERRAIN OBJECT ===
// ==============================
void BT_MakeTerrainObject(unsigned long terrainid,unsigned long LODLevel,unsigned long ObjectID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_MAKETERRAINOBJECT;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
		//Variables
			unsigned long SectorID=0;

		//Generate the object
			sObject* Object=BT_Intern_CreateBlankObject(ObjectID,BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors);

		//Loop through mesh list
			sFrame* CurrentFrame=Object->pFrame;
			for(SectorID=0;SectorID<BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors;SectorID++){
			//Check that the sector is not excluded
				if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Excluded==0){
				//Generate the mesh
					BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].QuadMap->GenerateDBPMesh(CurrentFrame->pMesh);

				//Set LODLevelFrame to sector
					BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].LODLevelObjectFrame=CurrentFrame;

				//Position mesh
					CurrentFrame->vecOffset.x=BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_x/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
					CurrentFrame->vecOffset.y=BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_y/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
					CurrentFrame->vecOffset.z=BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_z/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
					
				//Get next frame and link it properly
					CurrentFrame->pSibling=CurrentFrame->pChild;
					CurrentFrame->pChild=0;
					CurrentFrame=CurrentFrame->pSibling;
				}
			}

		//Position and scale object to terrains position and scale
			Object->position.vecPosition=BT_Main.Terrains[terrainid].Object->position.vecPosition;
			Object->position.vecScale=BT_Main.Terrains[terrainid].Object->position.vecScale;
			BT_Main.Terrains[terrainid].LODLevel[LODLevel].DBPObject=Object;

		//Finnish the object
			Object->bExcluded=true;
			BT_Intern_FinnishObject(ObjectID);
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
	}
}
// === END FUNCTION ===



// =================================
// === BT UPADTE TERRAIN OBJECTS ===
// =================================
void BT_UpdateTerrainObjects(unsigned long terrainid)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_MAKETERRAINOBJECT;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(terrainid)){
	//Check that the terrain is built
		if(BT_Main.Terrains[terrainid].Built){
		//Get Terrain ptr
			s_BT_terrain* TerrainPtr=&BT_Main.Terrains[terrainid];

		//Loop through LOD Levels
			for(unsigned long LODLevel=0;LODLevel<TerrainPtr->LODLevels;LODLevel++){
			//Get LODLevel ptr
				s_BT_LODLevel* LODLevelPtr=&TerrainPtr->LODLevel[LODLevel];

			//Loop through sectors
				for(unsigned long Sector=0;Sector<LODLevelPtr->Sectors;Sector++){
				//Get Sector ptr
					s_BT_Sector* SectorPtr=&LODLevelPtr->Sector[Sector];

				//Check that the sector has its own object
					if(SectorPtr->DBPObject!=0){
					//Update object
						SectorPtr->DBPObject->position.vecPosition.x=BT_Main.Terrains[terrainid].Object->position.vecPosition.x+SectorPtr->Pos_x/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
						SectorPtr->DBPObject->position.vecPosition.y=BT_Main.Terrains[terrainid].Object->position.vecPosition.y+SectorPtr->Pos_y/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
						SectorPtr->DBPObject->position.vecPosition.z=BT_Main.Terrains[terrainid].Object->position.vecPosition.z+SectorPtr->Pos_z/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
						SectorPtr->DBPObject->position.vecScale=BT_Main.Terrains[terrainid].Object->position.vecScale;
					}

				//Check if the LODLevel has an object
					if(LODLevelPtr->DBPObject!=0){
					//Update object
						LODLevelPtr->DBPObject->position.vecPosition=BT_Main.Terrains[terrainid].Object->position.vecPosition;
						LODLevelPtr->DBPObject->position.vecScale=BT_Main.Terrains[terrainid].Object->position.vecScale;
					}
				}
			}
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
	}
}
// === END FUNCTION ===



// ================================
// === BT GET SECTOR POSITION X ===
// ================================
float BT_GetSectorPositionX(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTORPOSITIONX;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
		//Check if the Sector exists
			if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors>SectorID)
			{
			//Return X Position
				float PosX=BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_x/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
				return PosX;
			}else{
				BT_Intern_Error(C_BT_ERROR_SECTORDOESNTEXIST);
				return NULL;
			}
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
}

// ================================
// === BT GET SECTOR POSITION Y ===
// ================================
float BT_GetSectorPositionY(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTORPOSITIONY;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
		//Check if the Sector exists
			if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors>SectorID)
			{
			//Return Y Position
				float PosY=BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_y/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
				return PosY;
			}else{
				BT_Intern_Error(C_BT_ERROR_SECTORDOESNTEXIST);
				return NULL;
			}
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
}

// ================================
// === BT GET SECTOR POSITION Z ===
// ================================
float BT_GetSectorPositionZ(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTORPOSITIONZ;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
		//Check if the Sector exists
			if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors>SectorID)
			{
			//Return Z Position
				float PosZ=BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Pos_z/C_BT_INTERNALSCALE*BT_Main.Terrains[terrainid].Scale;
				return PosZ;
			}else{
				BT_Intern_Error(C_BT_ERROR_SECTORDOESNTEXIST);
				return NULL;
			}
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
}

// ===========================
// === BT GET SECTOR COUNT ===
// ===========================
unsigned long BT_GetSectorCount(unsigned long TerrainID,unsigned long LODLevel)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTORCOUNT;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[TerrainID].LODLevels>LODLevel)
		{
			return BT_Main.Terrains[TerrainID].LODLevel[LODLevel].Sectors;
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
	return NULL;
}
// === END FUNCTION ===



// ===========================
// === BT GET SECTOR SIZE ===
// ===========================
unsigned long BT_GetSectorSize(unsigned long TerrainID,unsigned long LODLevel)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTORSIZE;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[TerrainID].LODLevels>LODLevel)
		{
			float Size=BT_Main.Terrains[TerrainID].LODLevel[LODLevel].SectorSize/C_BT_INTERNALSCALE*BT_Main.Terrains[TerrainID].Scale;
			return *(DWORD*)&Size;
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
	return NULL;
}

// ==============================
// === BT GET SECTOR EXCLUDED ===
// ==============================
unsigned long BT_GetSectorExcluded(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTOREXCLUDED;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
		//Check if the Sector exists
			if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors>SectorID)
			{
			//Return excluded
				return BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Excluded;
			}else{
				BT_Intern_Error(C_BT_ERROR_SECTORDOESNTEXIST);
				return NULL;
			}
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
}
// === END FUNCTION ===



// =========================
// === BT GET SECTOR ROW ===
// =========================
unsigned long BT_GetSectorRow(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTORROW;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
		//Check if the Sector exists
			if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors>SectorID)
			{
			//Return row
				return BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Row;
			}else{
				BT_Intern_Error(C_BT_ERROR_SECTORDOESNTEXIST);
				return NULL;
			}
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
}
// === END FUNCTION ===



// =============================
// === BT GET SECTOR COLLUMN ===
// =============================
unsigned long BT_GetSectorCollumn(unsigned long terrainid,unsigned long LODLevel,unsigned long SectorID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_GETSECTORCOLLUMN;

//Check if Terrain Exists
	if(BT_Intern_TerrainExist(terrainid))
	{
	//Check if the LOD level exists
		if(BT_Main.Terrains[terrainid].LODLevels>LODLevel)
		{
		//Check if the Sector exists
			if(BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sectors>SectorID)
			{
			//Return row
				return BT_Main.Terrains[terrainid].LODLevel[LODLevel].Sector[SectorID].Column;
			}else{
				BT_Intern_Error(C_BT_ERROR_SECTORDOESNTEXIST);
				return NULL;
			}
		}else{
			BT_Intern_Error(C_BT_ERROR_LODLEVELDOESNTEXIST);
			return NULL;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return NULL;
	}
}
// === END FUNCTION ===



// =============================
// === BT SET CURRENT CAMERA ===
// =============================
void BT_SetCurrentCamera(unsigned long CameraID)
{
//Set current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_SETCURRENTCAMERA;

//Check if the camera exists
	if(GetCameraInternalData(CameraID)!=NULL)
	{
	//Add to queue
		BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_SETCURRENTCAMERA,(char)CameraID);
	}

}
// === END FUNCTION ===



// =============================
// === BT UPDATE TERRAIN LOD ===
// =============================
void BT_UpdateTerrainLOD(unsigned long TerrainID)
{
//Set current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_UPDATETERRAINLOD;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{

	//Check that the terrain is generated
		if(BT_Main.Terrains[TerrainID].Generated==true)
		{
		//Add to queue
			BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_UPDATETERRAINLOD,(char)TerrainID);
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINNOTGENERATED);
			return;
		}

	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}

}
// === END FUNCTION ===



// ==============================
// === BT UPDATE TERRAIN CULL ===
// ==============================
void BT_UpdateTerrainCull(unsigned long TerrainID)
{
//Set current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_UPDATETERRAINCULL;

//Check that the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{

	//Check that the terrain is generated
		if(BT_Main.Terrains[TerrainID].Generated==true)
		{
		//Add to queue
			BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_UPDATETERRAINCULL,(char)TerrainID);
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINNOTGENERATED);
			return;
		}

	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}

}
// === END FUNCTION ===



// =========================
// === BT RENDER TERRAIN ===
// =========================
void BT_RenderTerrain(unsigned long TerrainID)
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_RENDER;

//Check if the terrain exists
	if(BT_Intern_TerrainExist(TerrainID))
	{
	//Check that the terrain is generated
		if(BT_Main.Terrains[TerrainID].Generated==true)
		{
		//Add to queue
			BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_RENDERTERRAIN,(char)TerrainID);
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINNOTGENERATED);
			return;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}
// === END FUNCTION ===

void BT_NoRenderTerrain(unsigned long TerrainID)
{
	BT_Main.CurrentFunction=C_BT_FUNCTION_RENDER;
	if(BT_Intern_TerrainExist(TerrainID))
	{
		if(BT_Main.Terrains[TerrainID].Generated==true)
		{
			BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_NORENDERTERRAIN,(char)TerrainID);
		}else{
			BT_Intern_Error(C_BT_ERROR_TERRAINNOTGENERATED);
			return;
		}
	}else{
		BT_Intern_Error(C_BT_ERROR_TERRAINDOESNTEXIST);
		return;
	}
}



// ===========================
// === BT GET TERRAIN INFO ===
// ===========================
void* BT_GetTerrainInfo(unsigned long terrainid)
{
	return BT_Main.Terrains[terrainid].Info;
}
// === END FUNCTION ===



// ============================
// === BT GET LODLEVEL INFO ===
// ============================
void* BT_GetLODLevelInfo(unsigned long terrainid,unsigned long LODLevelID)
{
	return BT_Main.Terrains[terrainid].LODLevel[LODLevelID].Info;
}
// === END FUNCTION ===



// ==========================
// === BT GET SECTOR INFO ===
// ==========================
void* BT_GetSectorInfo(unsigned long terrainid,unsigned long LODLevelID,unsigned long SectorID)
{
	return BT_Main.Terrains[terrainid].LODLevel[LODLevelID].Sector[SectorID].Info;
}
// === END FUNCTION ===

// ========================
// === BT INTERN RENDER === THIS AUTOMATICALY GETS CALLED BY DBPRO
// ========================

bool g_bSkipTerrainRender = false;
unsigned long g_CurrentTerrainCameraID = 0;

void BT_Intern_Render()
{
	// Dave - added this as going into the importer can crash in this, in debug mode, so may be dodgy
	if ( g_bSkipTerrainRender )
	{
		BT_Main.InstructionQueue[0]=NULL;
		BT_Main.InstructionQueueUsed=0;
		g_bSkipTerrainRender = false;
		return;
	}

	// 100418 - seems when skip terrain render (.superflat), viewport is not set (and needs to be)
	// look FURTHER into this to determine if 1920 or 1772 width viewport is correct for terrain
	// 160418 - ensure this fix does not interfere with 64x64 viewport setting for bitmap capture
	if ( g_pGlob->iCurrentBitmapNumber < 32 )
	{
		// but only if not rendering to a shadow map
		if (g_bRenderTerrainForShadowMap == false)
		{
			tagCameraData* Camera = (tagCameraData*)GetCameraInternalData(0);
			D3D11_VIEWPORT vp;
			GGVIEWPORT* pvp = &Camera->viewPort3D;
			vp.TopLeftX = pvp->X;
			vp.TopLeftY = pvp->Y;
			vp.Width = (FLOAT)pvp->Width;
			vp.Height = (FLOAT)pvp->Height;
			vp.MinDepth = pvp->MinZ;
			vp.MaxDepth = pvp->MaxZ;
			SetupSetViewport(g_pGlob->dwRenderCameraID, &vp, NULL);
		}
	}

	try
	{
		//Clear statistics
		BT_Intern_ClearStatistics();

		//Check if autorender is enabled
		if(BT_Main.AutoRender==true)
		{
			//Loop through cameras
			for(unsigned long CameraID=0;CameraID<32;CameraID++)
			{
				if(GetCameraInternalData(CameraID)!=0)
				{
					//Add camera to queue
					BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_SETCURRENTCAMERA,(char)CameraID);

					//Loop through terrains
					for(unsigned long TerrainID=1;TerrainID<C_BT_MAXTERRAINS;TerrainID++)
					{
						//Check that the terrain exists
						if(BT_Main.Terrains[TerrainID].Exists==true)
						{
							//Add terrain cull update to queue
							BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_UPDATETERRAINCULL,(char)TerrainID);

							//Check if the terrain has LOD enabled
							if(BT_Main.Terrains[TerrainID].LODLevels>1)
							{
								//Add terrain LOD update to queue
								BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_UPDATETERRAINLOD,(char)TerrainID);
							}

							//Add render terrain to queue
							BT_Intern_AddToInstructionQueue(C_BT_INSTRUCTION_RENDERTERRAIN,(char)TerrainID);
						}
					}
				}
			}
		}

		//Read instruction queue
		unsigned long CurrentPos=0;
		do
		{
			if(BT_Main.InstructionQueue[CurrentPos]==C_BT_INSTRUCTION_SETCURRENTCAMERA)
			{
				//Variables
				unsigned long CameraID = BT_Main.InstructionQueue[CurrentPos+1];
				g_CurrentTerrainCameraID = CameraID;

				//Set current camera
				BT_Main.CurrentUpdateCamera=(tagCameraData*)GetCameraInternalData(CameraID);
				BT_Main.FrustumExtracted=false;

				//Increase position
				CurrentPos+=2;
			}
			else if(BT_Main.InstructionQueue[CurrentPos]==C_BT_INSTRUCTION_UPDATETERRAINCULL)
			{
				//Variables
				unsigned long TerrainID=BT_Main.InstructionQueue[CurrentPos+1];

				//Cull Offset and scale
				BT_Main.CullOffset=-BT_Main.Terrains[TerrainID].Object->position.vecPosition;
				BT_Main.CullScale=BT_Main.Terrains[TerrainID].Object->position.vecScale;
				BT_Main.CullScale.x=BT_Main.CullScale.x*BT_Main.Terrains[TerrainID].Scale/C_BT_INTERNALSCALE;
				BT_Main.CullScale.z=BT_Main.CullScale.z*BT_Main.Terrains[TerrainID].Scale/C_BT_INTERNALSCALE;

				//Extract frustum
				if(BT_Main.FrustumExtracted==false)
				{
					BT_Intern_ExtractFrustum();
					BT_Main.FrustumExtracted=true;
				}

				//Update Cull
				BT_RTTMS_UnlockTerrain(&BT_Main.Terrains[TerrainID]);
				BT_Intern_UpdateCullBoxesRec(&BT_Main.Terrains[TerrainID],BT_Main.Terrains[TerrainID].QuadTree,BT_Main.Terrains[TerrainID].QuadTreeLevels);

				// camera 30 (cube rendering) does not cull terrain visibility
				bool bIntersectingCull = false;
				if ( g_CurrentTerrainCameraID == 30 ) bIntersectingCull = true;
				BT_Intern_CalculateCullingRec(&BT_Main.Terrains[TerrainID],BT_Main.Terrains[TerrainID].QuadTree,BT_Main.Terrains[TerrainID].QuadTreeLevels,bIntersectingCull);

				//Increase position
				CurrentPos+=2;
			}
			else if(BT_Main.InstructionQueue[CurrentPos]==C_BT_INSTRUCTION_UPDATETERRAINLOD)
			{
				//Variables
				unsigned long TerrainID=BT_Main.InstructionQueue[CurrentPos+1];

				// if no camera at this time, leave
				if ( BT_Main.CurrentUpdateCamera==NULL ) return;

				//Set main camera
				BT_Main.LODCamPosition=BT_Main.CurrentUpdateCamera->vecPosition;

				//Find LODLevels
				if(BT_Main.Terrains[TerrainID].LODLevels>1)
					BT_Intern_CalculateLODLevelsRec(&BT_Main.Terrains[TerrainID],BT_Main.Terrains[TerrainID].QuadTree,BT_Main.Terrains[TerrainID].QuadTreeLevels,0);

				//Fix LOD seams
				BT_Intern_FixLODSeams(&BT_Main.Terrains[TerrainID]);

				//Increase position
				CurrentPos+=2;
			}
			else if(BT_Main.InstructionQueue[CurrentPos]==C_BT_INSTRUCTION_RENDERTERRAIN)
			{
				// Variables
				unsigned long TerrainID=BT_Main.InstructionQueue[CurrentPos+1];

				// Check if the object is visible (lee - 050115 - if present!)
				if ( BT_Main.Terrains[TerrainID].Object->pFrame )
				{
					if(BT_Main.Terrains[TerrainID].Object->pFrame->pMesh->bVisible==true)
					{
						// Set main camera
						BT_Main.LODCamPosition=BT_Main.CurrentUpdateCamera->vecPosition;

						// Render terrain
						BT_Intern_RenderTerrain(&BT_Main.Terrains[TerrainID]);
					}
				}

				// Increase position
				CurrentPos+=2;
			}
			else if(BT_Main.InstructionQueue[CurrentPos]==C_BT_INSTRUCTION_NORENDERTERRAIN)
			{
				// this mimics the above, but calls BT_Intern_NoRenderTerrain 
				// which renders no terrain but keeps state changes as though it was (fixes VR rendering)
				unsigned long TerrainID=BT_Main.InstructionQueue[CurrentPos+1];
				if ( BT_Main.Terrains[TerrainID].Object->pFrame )
				{
					if(BT_Main.Terrains[TerrainID].Object->pFrame->pMesh->bVisible==true)
					{
						BT_Main.LODCamPosition=BT_Main.CurrentUpdateCamera->vecPosition;
						BT_Intern_NoRenderTerrain(&BT_Main.Terrains[TerrainID]);
					}
				}

				// Increase position
				CurrentPos+=2;
			}
		}
		while(BT_Main.InstructionQueue[CurrentPos]!=NULL);

		//Clear instruction queue
		BT_Main.InstructionQueue[0]=NULL;
		BT_Main.InstructionQueueUsed=0;
	}
	catch (...)
	{
		//MessageBox ( NULL, "Terrain module error", "Terrain Module", MB_OK );
	}
}
// === END FUNCTION ===



// ==========================================
// === BT INTERN ADD TO INSTRUCTION QUEUE ===
// ==========================================
static void BT_Intern_AddToInstructionQueue(char Instruction,char Data)
{
//Check the queue size
	if(BT_Main.InstructionQueueUsed+2<BT_Main.InstructionQueueSize)
	{
		BT_Main.InstructionQueue[BT_Main.InstructionQueueUsed]=Instruction;
		BT_Main.InstructionQueue[BT_Main.InstructionQueueUsed+1]=Data;
		BT_Main.InstructionQueueUsed+=2;
		BT_Main.InstructionQueue[BT_Main.InstructionQueueUsed]=NULL;
	}
}
// === END FUNCTION ===



// ==================================
// === BT INTERN CLEAR STATISTICS ===
// ==================================
static void BT_Intern_ClearStatistics()
{
//Set Current function
	BT_Main.CurrentFunction=C_BT_FUNCTION_CLEARSTATISTICS;

//Clear
	BT_Main.DrawPrimitiveCount=0;
	BT_Main.DrawCalls=0;
	BT_Main.CullChecks=0;

}
// === END FUNCTION ===

// ================================
// === BT INTERN RENDER TERRAIN ===
// ================================
static void BT_Intern_RenderTerrain(s_BT_terrain* Terrain)
{
	// Variables
	tagCameraData* Camera = BT_Main.CurrentUpdateCamera;
	GGMATRIX World;
	sMesh* Mesh = Terrain->Object->pFrame->pMesh;

	#ifdef DX11
	// create constant buffer for quick world position changes
	if ( m_pCBChangePerTerrsainChunk == NULL )
	{
		D3D11_BUFFER_DESC bdChangePerTerrsainChunkBuffer;
		std::memset ( &bdChangePerTerrsainChunkBuffer, 0, sizeof ( bdChangePerTerrsainChunkBuffer ) );
		bdChangePerTerrsainChunkBuffer.Usage          = D3D11_USAGE_DEFAULT;
		bdChangePerTerrsainChunkBuffer.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		bdChangePerTerrsainChunkBuffer.CPUAccessFlags = 0;
		bdChangePerTerrsainChunkBuffer.ByteWidth      = sizeof ( CBChangePerTerrsainChunk );
		if ( FAILED ( m_pD3D->CreateBuffer ( &bdChangePerTerrsainChunkBuffer, NULL, &m_pCBChangePerTerrsainChunk ) ) )
			return;
	}
	if ( m_pCBChangePerTerrsainChunkPS == NULL )
	{
		D3D11_BUFFER_DESC bdChangePerTerrsainChunkBuffer;
		std::memset ( &bdChangePerTerrsainChunkBuffer, 0, sizeof ( bdChangePerTerrsainChunkBuffer ) );
		bdChangePerTerrsainChunkBuffer.Usage          = D3D11_USAGE_DEFAULT;
		bdChangePerTerrsainChunkBuffer.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		bdChangePerTerrsainChunkBuffer.CPUAccessFlags = 0;
		bdChangePerTerrsainChunkBuffer.ByteWidth      = sizeof ( CBChangePerTerrsainChunkPS );
		if ( FAILED ( m_pD3D->CreateBuffer ( &bdChangePerTerrsainChunkBuffer, NULL, &m_pCBChangePerTerrsainChunkPS ) ) )
			return;
	}

	// when rendering terrain for shadow mapping, these transforms are already set (for the light camera view)
	if (g_bRenderTerrainForShadowMap == false)
	{
		// Transforms
		GGSetTransform(GGTS_PROJECTION, &Camera->matProjection);
		GGSetTransform(GGTS_VIEW, &Camera->matView);
	}

	// Viewport for terrain rendering
	if (g_bRenderTerrainForShadowMap == true)
	{
		// the call to RSSetViewports already done in ShadowMap code
	}
	else
	{
		D3D11_VIEWPORT vp;
		GGVIEWPORT* pvp = &Camera->viewPort3D;
		vp.TopLeftX = pvp->X;
		vp.TopLeftY = pvp->Y;
		vp.Width = (FLOAT)pvp->Width;
		vp.Height = (FLOAT)pvp->Height;
		vp.MinDepth = pvp->MinZ;
		vp.MaxDepth = pvp->MaxZ;
		SetupSetViewport ( g_pGlob->dwRenderCameraID, &vp, NULL );
	}

	// Set current render terrain
	BT_Main.CurrentRenderTerrain=Terrain;

	// Unlock sectors
	BT_Intern_UnlockSectorsRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels);

	// Check if theres an effect
	if ( Mesh->pVertexShaderEffect != NULL )
	{
		// Vertex Declaration
		m_pImmediateContext->IASetInputLayout ( Terrain->VertexDeclaration );

		// Variables
		LPGGEFFECT Effect = Mesh->pVertexShaderEffect->m_pEffect;

		// Obtain technique handles
		GGTECHNIQUE hNearTechnique = Mesh->pVertexShaderEffect->m_hCurrentTechnique;
		GGTECHNIQUE hDistantTechnique = Effect->GetTechniqueByName ( "Distant" );

		// Can also render terrain into the shadow map, so use super quick render type
		int iQualityPassCount = 2;
		if (g_bRenderTerrainForShadowMap == true)
		{
			iQualityPassCount = 1;
			hNearTechnique = Effect->GetTechniqueByName ( "DepthMap" );
			hDistantTechnique = Effect->GetTechniqueByName ( "DepthMap" );
		}
		// Two passes, one NORMAL technique and one VERY LOW technique (distant terrain)
		BT_Main.CurrentEffect=Mesh->pVertexShaderEffect;
		for ( int iQualityPass=0; iQualityPass<iQualityPassCount; iQualityPass++ )
		{
			//PE: fullshadowsoreditor was always 1 (g_iQualityTechniqueMode==1) , now always use hDistantTechnique.
			// Set correct technique
			GGTECHNIQUE hTechniqueUsed = NULL;
//			if ( g_iQualityTechniqueMode==0 )
//			{
				if ( iQualityPass==0 ) hTechniqueUsed = hNearTechnique;
				if ( iQualityPass==1 ) hTechniqueUsed = hDistantTechnique;
//			}
//			else
//			{
//				if ( g_iQualityTechniqueMode==1 ) hTechniqueUsed = hNearTechnique;
//				if ( g_iQualityTechniqueMode==2 ) hTechniqueUsed = hDistantTechnique;
//			}

			// only one pass (removed secondary depth pass for DX11)
			if ( hTechniqueUsed->IsValid() )
			{
				// Move plenty of effect setup to here (performance)
				m_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				if (g_bRenderTerrainForShadowMap == true)
				{
					m_pImmediateContext->RSSetState(m_pRasterStateTerrainShadow);
					m_pImmediateContext->OMSetBlendState(m_pBlendStateNoAlpha, 0, 0xffffffff);
					m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, 0);
				}
				else
				{
					// state blocks for raster, blend and depthstencil
					m_pImmediateContext->RSSetState(m_pRasterState);
					m_pImmediateContext->OMSetBlendState(m_pBlendStateNoAlpha, 0, 0xffffffff);
					m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, 0);
				}


				// Update shadow textures of terrain shader (added for DX11 - not sure where DX9 did this)
				if (iQualityPass == 0 && g_bRenderTerrainForShadowMap == false)
				{
					DWORD dwEffectIndex = Mesh->pVertexShaderEffect->m_dwEffectIndex;
					if (dwEffectIndex < EFFECT_INDEX_SIZE)
					{
						if (g_CascadedShadow.m_pEffectParam[dwEffectIndex])
						{
							GGHANDLE hdepthHandle0 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX1;
							GGHANDLE hdepthHandle1 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX2;
							GGHANDLE hdepthHandle2 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX3;
							GGHANDLE hdepthHandle3 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX4;
							GGHANDLE hdepthHandle4 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX5;
							GGHANDLE hdepthHandle5 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX6;
							GGHANDLE hdepthHandle6 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX7;
							GGHANDLE hdepthHandle7 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX8;
							if (hdepthHandle0 && g_CascadedShadow.m_depthTexture[0]) hdepthHandle0->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[0]->getTextureResourceView());
							if (hdepthHandle1 && g_CascadedShadow.m_depthTexture[1]) hdepthHandle1->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[1]->getTextureResourceView());
							if (hdepthHandle2 && g_CascadedShadow.m_depthTexture[2]) hdepthHandle2->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[2]->getTextureResourceView());
							if (hdepthHandle3 && g_CascadedShadow.m_depthTexture[3]) hdepthHandle3->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[3]->getTextureResourceView());
							if (hdepthHandle4 && g_CascadedShadow.m_depthTexture[4]) hdepthHandle4->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[4]->getTextureResourceView());
							if (hdepthHandle5 && g_CascadedShadow.m_depthTexture[5]) hdepthHandle5->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[5]->getTextureResourceView());
							if (hdepthHandle6 && g_CascadedShadow.m_depthTexture[6]) hdepthHandle6->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[6]->getTextureResourceView());
							if (hdepthHandle7 && g_CascadedShadow.m_depthTexture[7]) hdepthHandle7->AsShaderResource()->SetResource(g_CascadedShadow.m_depthTexture[7]->getTextureResourceView());
						}
					}
				}

				// pass clipping data to shader
				if ( Mesh->pVertexShaderEffect->m_VecClipPlaneEffectHandle )
				{
					GGVECTOR4 vec;
					tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwRenderCameraID );
					if ( m_Camera_Ptr )
					{
						if ( m_Camera_Ptr->iClipPlaneOn==1 )
						{
							// special mode which creates plane but does not use RenderState to set clip
							// as you cannot mix FF clip and HLSL clip in same scene (artefacts)
							vec.x = m_Camera_Ptr->planeClip.a;
							vec.y = m_Camera_Ptr->planeClip.b;
							vec.z = m_Camera_Ptr->planeClip.c;
							vec.w = m_Camera_Ptr->planeClip.d;
						}
						else
						{
							// ensure shader stops using clip plane when not being clipped!
							vec = GGVECTOR4( 0.0f, 1.0f, 0.0f, 99999.0f );
						}
					}
					else
					{
						// ensure shader stops using clip plane when not being clipped!
						vec = GGVECTOR4( 0.0f, 1.0f, 0.0f, 99999.0f );
					}
					Mesh->pVertexShaderEffect->m_VecClipPlaneEffectHandle->AsVector()->SetFloatVector ( (float*)&vec );
				}

				// Ensure normal invert in effect for terrain (NOTE: not liking duplicated of code)
				#ifdef DX11
				//PE: removed , terrain always invert normal.
//				GGHANDLE pArtFlags = Mesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "ArtFlagControl1" );
//				if ( pArtFlags )
//				{
//					float fInvertNormal = 0.0f;
//					if ( Mesh->dwArtFlags & 0x1 ) fInvertNormal = 1.0f;
//					GGVECTOR4 vec4 = GGVECTOR4 ( fInvertNormal, 0.0f, 0.0f, 0.0f );
//					pArtFlags->AsVector()->SetFloatVector ( (float*)&vec4 );
//				}
				#endif

				// apply effect ready for rendering
				hTechniqueUsed->GetPassByIndex(0)->Apply(0,m_pImmediateContext);

				// Set textures (AFTER Apply which overrides texture view ptrs)
				if (g_bRenderTerrainForShadowMap == false)
				{
					for (unsigned long i = 0; i < Mesh->dwTextureCount; i++) // terrain now has lots of textures
					{
						if (i != 1 && i != 5 && i != 7)
						{
							//PE: only for more gpu mem (cache) they are not used.
							ID3D11ShaderResourceView* lpTexture = NULL;
							if (Mesh->dwTextureCount > i) lpTexture = Mesh->pTextures[i].pTexturesRefView;
							m_pImmediateContext->PSSetShaderResources(i, 1, &lpTexture);
						}
					}
				}

				// assign constants
				GGMATRIX matWorld;
				GGGetTransform ( GGTS_WORLD, &matWorld );
				Mesh->pVertexShaderEffect->Start ( Mesh, matWorld );

				//PE: Terrain should be split into smaller meshes for better light.
				if (g_bRenderTerrainForShadowMap == false)
					update_mesh_light(Mesh, NULL,NULL);

				// Render many terrain chunks (only CB changes for world position for faster rendering)
				BT_Intern_RenderTerrainRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels,iQualityPass);

				// End effect
				BT_Main.CurrentEffect=NULL;

				// Free textures (especially camera image texture which needs to be output bound next cycle)
				if (g_bRenderTerrainForShadowMap == false)
				{
					for (unsigned long i = 0; i < Mesh->dwTextureCount; i++)
					{
						if (i != 1 && i != 5 && i != 7)
						{
							//PE: 
							ID3D11ShaderResourceView* lpTexture = NULL;
							m_pImmediateContext->PSSetShaderResources(i, 1, &lpTexture);
						}
					}
				}
			}
		}
	}
	else
	{
		// no rendering if no effect shader
	}

	// zero current render terrain
	BT_Main.CurrentRenderTerrain=NULL;

	#else
	// Set textures
	IGGDevice* D3DDevice=m_pD3D;
	for(unsigned long i=0;i<8;i++)
	{
		if(Mesh->dwTextureCount>i)
		{
			D3DDevice->SetTexture(Mesh->pTextures[i].dwStage,Mesh->pTextures[i].pTexturesRef);
			D3DDevice->SetTextureStageState(Mesh->pTextures[i].dwStage,D3DTSS_COLOROP,Mesh->pTextures[i].dwBlendMode);
			D3DDevice->SetTextureStageState(Mesh->pTextures[i].dwStage,D3DTSS_COLORARG1,Mesh->pTextures[i].dwBlendArg1);
			D3DDevice->SetTextureStageState(Mesh->pTextures[i].dwStage,D3DTSS_COLORARG2,Mesh->pTextures[i].dwBlendArg2);
			D3DDevice->SetSamplerState(Mesh->pTextures[i].dwStage,D3DSAMP_MAGFILTER,Mesh->pTextures[i].dwMagState);
			D3DDevice->SetSamplerState(Mesh->pTextures[i].dwStage,D3DSAMP_MINFILTER,Mesh->pTextures[i].dwMinState);
			D3DDevice->SetSamplerState(Mesh->pTextures[i].dwStage,D3DSAMP_MIPFILTER,Mesh->pTextures[i].dwMipState);

			// Clamp base texture
			if(i==0)
			{
				D3DDevice->SetSamplerState(Mesh->pTextures[i].dwStage,D3DSAMP_ADDRESSU,GGTADDRESS_CLAMP);
				D3DDevice->SetSamplerState(Mesh->pTextures[i].dwStage,D3DSAMP_ADDRESSV,GGTADDRESS_CLAMP);
			}
		}
		else
		{
			D3DDevice->SetTexture(i,NULL);
		}
	}

	// Set shader
	if(Mesh->bOverridePixelShader)
	{
		D3DDevice->SetPixelShader(Mesh->pPixelShader);
	}
	else
	{
		D3DDevice->SetPixelShader(NULL);
	}
	if(Mesh->bUseVertexShader)
	{
		D3DDevice->SetVertexShader(Mesh->pVertexShader);
	}
	else
	{
		D3DDevice->SetVertexShader(NULL);
	}

	// Transforms
	D3DDevice->SetTransform(GGTS_PROJECTION,&Camera->matProjection);
	D3DDevice->SetTransform(GGTS_VIEW,&Camera->matView);

	// Store old VertDec to restore later
	IDirect3DVertexDeclaration9* pDecl = NULL;
	D3DDevice->GetVertexDeclaration(&pDecl);
	DWORD dwFVF = 0;
	D3DDevice->GetFVF(&dwFVF);

	// Vertex Declaration
	D3DDevice->SetVertexDeclaration(Terrain->VertexDeclaration);

	// Viewport
	D3DDevice->SetViewport((GGVIEWPORT*)&Camera->viewPort3D);

	// DBPRO RENDERING ENGINE
	DBPRO_SetMeshRenderStates(Mesh);

	// Set current render terrain
	BT_Main.CurrentRenderTerrain=Terrain;

	// Unlock sectors
	BT_Intern_UnlockSectorsRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels);

	// Check if theres an effect
	if(Mesh->pVertexShaderEffect!=NULL)
	{
		// Variables
		LPGGEFFECT Effect=Mesh->pVertexShaderEffect->m_pEffect;

		// Obtain technique handles
		GGHANDLE hNearTechnique = Effect->GetCurrentTechnique();
		GGHANDLE hDistantTechnique = Effect->GetTechniqueByName ( "Distant" );

		// Two passes, one NORMAL technique and one VERY LOW technique (distant terrain)
		BT_Main.CurrentEffect=Mesh->pVertexShaderEffect;
		for ( int iQualityPass=0; iQualityPass<2; iQualityPass++ )
		{
			// Set correct technique
			GGHANDLE hTechniqueUsed = NULL;
			if ( g_iQualityTechniqueMode==0 )
			{
				if ( iQualityPass==0 ) hTechniqueUsed = hNearTechnique;
				if ( iQualityPass==1 ) hTechniqueUsed = hDistantTechnique;
			}
			else
			{
				if ( g_iQualityTechniqueMode==1 ) hTechniqueUsed = hNearTechnique;
				if ( g_iQualityTechniqueMode==2 ) hTechniqueUsed = hDistantTechnique;
			}
			Effect->SetTechnique ( hTechniqueUsed );

			// Begin effect
			UINT Passes;
			Effect->Begin(&Passes,NULL);

			// store main camera render target and depth stencil buffer
			IGGSurface* pCurrentRenderTarget = NULL;
			IGGSurface* pCurrentDepthTarget = NULL;
			m_pD3D->GetRenderTarget( 0, &pCurrentRenderTarget );
			m_pD3D->GetDepthStencilSurface( &pCurrentDepthTarget );

			// Begin loop
			for(unsigned int Pass=0;Pass<Passes;Pass++)
			{
				// check which render target we are writing to (camera or depth-texture)
				GGHANDLE hPass = Effect->GetPass( hTechniqueUsed, Pass );
				GGHANDLE hRT = Effect->GetAnnotationByName( hPass, "RenderColorTarget" );
				const char* szRT = 0;
				if ( hRT ) Effect->GetString( hRT, &szRT );
				if ( szRT && strnicmp( szRT, "[depthtexture]", strlen("[depthtexture]") )==NULL && g_pGlob->dwRenderCameraID==0 ) 
				{
					// render to depth texture from main basic3D DLL
					int iSuccess = SwitchRenderTargetToDepthTexture(0);
				}
				else
				{
					// render to original render target
					m_pD3D->SetRenderTarget( 0, pCurrentRenderTarget );
					m_pD3D->SetDepthStencilSurface( pCurrentDepthTarget );
				}

				// Begin pass
				Effect->BeginPass(Pass);

				// Render
				BT_Intern_RenderTerrainRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels,iQualityPass);

				// End pass
				Effect->EndPass();
			}

			// End effect
			BT_Main.CurrentEffect=NULL;
			Effect->End();

			// restore render target in any event
			m_pD3D->SetRenderTarget( 0, pCurrentRenderTarget );
			m_pD3D->SetDepthStencilSurface( pCurrentDepthTarget );
		}

		// Restore terrain shader technique
		Effect->SetTechnique ( hNearTechnique );
	}
	else
	{
		// Render
		BT_Main.CurrentEffect=NULL;
		BT_Intern_RenderTerrainRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels,0);
		BT_Intern_RenderTerrainRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels,1);
	}

	//Zero current render terrain
	BT_Main.CurrentRenderTerrain=NULL;

	// Restore vertex decl
	D3DDevice->SetVertexDeclaration(pDecl);
	D3DDevice->SetFVF(dwFVF);
	#endif
}
// === END FUNCTION ===

static void BT_Intern_NoRenderTerrain(s_BT_terrain* Terrain)
{
	// Variables
	tagCameraData* Camera = BT_Main.CurrentUpdateCamera;
	GGMATRIX World;
	sMesh* Mesh = Terrain->Object->pFrame->pMesh;

	// create constant buffer for quick world position changes
	if ( m_pCBChangePerTerrsainChunk == NULL )
	{
		D3D11_BUFFER_DESC bdChangePerTerrsainChunkBuffer;
		std::memset ( &bdChangePerTerrsainChunkBuffer, 0, sizeof ( bdChangePerTerrsainChunkBuffer ) );
		bdChangePerTerrsainChunkBuffer.Usage          = D3D11_USAGE_DEFAULT;
		bdChangePerTerrsainChunkBuffer.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		bdChangePerTerrsainChunkBuffer.CPUAccessFlags = 0;
		bdChangePerTerrsainChunkBuffer.ByteWidth      = sizeof ( CBChangePerTerrsainChunk );
		if ( FAILED ( m_pD3D->CreateBuffer ( &bdChangePerTerrsainChunkBuffer, NULL, &m_pCBChangePerTerrsainChunk ) ) )
			return;
	}
	if ( m_pCBChangePerTerrsainChunkPS == NULL )
	{
		D3D11_BUFFER_DESC bdChangePerTerrsainChunkBuffer;
		std::memset ( &bdChangePerTerrsainChunkBuffer, 0, sizeof ( bdChangePerTerrsainChunkBuffer ) );
		bdChangePerTerrsainChunkBuffer.Usage          = D3D11_USAGE_DEFAULT;
		bdChangePerTerrsainChunkBuffer.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		bdChangePerTerrsainChunkBuffer.CPUAccessFlags = 0;
		bdChangePerTerrsainChunkBuffer.ByteWidth      = sizeof ( CBChangePerTerrsainChunkPS );
		if ( FAILED ( m_pD3D->CreateBuffer ( &bdChangePerTerrsainChunkBuffer, NULL, &m_pCBChangePerTerrsainChunkPS ) ) )
			return;
	}

	// Transforms
	GGSetTransform(GGTS_PROJECTION,&Camera->matProjection);
	GGSetTransform(GGTS_VIEW,&Camera->matView);

	// Viewport
    D3D11_VIEWPORT vp;
	GGVIEWPORT* pvp = &Camera->viewPort3D;
    vp.TopLeftX = pvp->X;
    vp.TopLeftY = pvp->Y;
    vp.Width = (FLOAT)pvp->Width;
    vp.Height = (FLOAT)pvp->Height;
    vp.MinDepth = pvp->MinZ;
    vp.MaxDepth = pvp->MaxZ;
	SetupSetViewport ( g_pGlob->dwRenderCameraID, &vp, NULL );

	/*
	// Set current render terrain
	BT_Main.CurrentRenderTerrain=Terrain;

	// Unlock sectors
	BT_Intern_UnlockSectorsRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels);

	// Check if theres an effect
	if ( Mesh->pVertexShaderEffect != NULL )
	{
		// Vertex Declaration
		m_pImmediateContext->IASetInputLayout ( Terrain->VertexDeclaration );

		// Variables
		LPGGEFFECT Effect = Mesh->pVertexShaderEffect->m_pEffect;

		// Obtain technique handles
		GGTECHNIQUE hNearTechnique = Mesh->pVertexShaderEffect->m_hCurrentTechnique;
		GGTECHNIQUE hDistantTechnique = Effect->GetTechniqueByName ( "Distant" );
		
		// Two passes, one NORMAL technique and one VERY LOW technique (distant terrain)
		BT_Main.CurrentEffect=Mesh->pVertexShaderEffect;
		for ( int iQualityPass=0; iQualityPass<2; iQualityPass++ )
		{
			//PE: Todo fullshadowsoreditor is always 1 , so hDistantTechnique is never used ?
			// Set correct technique
			GGTECHNIQUE hTechniqueUsed = NULL;
			if ( g_iQualityTechniqueMode==0 )
			{
				if ( iQualityPass==0 ) hTechniqueUsed = hNearTechnique;
				if ( iQualityPass==1 ) hTechniqueUsed = hDistantTechnique;
			}
			else
			{
				if ( g_iQualityTechniqueMode==1 ) hTechniqueUsed = hNearTechnique;
				if ( g_iQualityTechniqueMode==2 ) hTechniqueUsed = hDistantTechnique;
			}

			// only one pass (removed secondary depth pass for DX11)
			if ( hTechniqueUsed->IsValid() )
			{
				// Move plenty of effect setup to here (performance)
				m_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// state blocks for raster, blend and depthstencil
				m_pImmediateContext->RSSetState(m_pRasterState);
				m_pImmediateContext->OMSetBlendState(m_pBlendStateNoAlpha, 0, 0xffffffff);
				m_pImmediateContext->OMSetDepthStencilState( m_pDepthStencilState, 0 );

				// Update shadow textures of terrain shader (added for DX11 - not sure where DX9 did this)
				DWORD dwEffectIndex = Mesh->pVertexShaderEffect->m_dwEffectIndex;
				if ( dwEffectIndex < EFFECT_INDEX_SIZE )
				{
					if ( g_CascadedShadow.m_pEffectParam[dwEffectIndex] )
					{
						GGHANDLE hdepthHandle0 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX1;
						GGHANDLE hdepthHandle1 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX2;
						GGHANDLE hdepthHandle2 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX3;
						GGHANDLE hdepthHandle3 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX4;
						GGHANDLE hdepthHandle4 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX5;
						GGHANDLE hdepthHandle5 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX6;
						GGHANDLE hdepthHandle6 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX7;
						GGHANDLE hdepthHandle7 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX8;
						if ( hdepthHandle0 && g_CascadedShadow.m_depthTexture[0] ) hdepthHandle0->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[0]->getTextureResourceView() );
						if ( hdepthHandle1 && g_CascadedShadow.m_depthTexture[1] ) hdepthHandle1->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[1]->getTextureResourceView() );
						if ( hdepthHandle2 && g_CascadedShadow.m_depthTexture[2] ) hdepthHandle2->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[2]->getTextureResourceView() );
						if ( hdepthHandle3 && g_CascadedShadow.m_depthTexture[3] ) hdepthHandle3->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[3]->getTextureResourceView() );
						if ( hdepthHandle4 && g_CascadedShadow.m_depthTexture[4] ) hdepthHandle4->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[4]->getTextureResourceView() );
						if ( hdepthHandle5 && g_CascadedShadow.m_depthTexture[5] ) hdepthHandle5->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[5]->getTextureResourceView() );
						if ( hdepthHandle6 && g_CascadedShadow.m_depthTexture[6] ) hdepthHandle6->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[6]->getTextureResourceView() );
						if ( hdepthHandle7 && g_CascadedShadow.m_depthTexture[7] ) hdepthHandle7->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[7]->getTextureResourceView() );
					}
				}

				// pass clipping data to shader
				if ( Mesh->pVertexShaderEffect->m_VecClipPlaneEffectHandle )
				{
					GGVECTOR4 vec;
					tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwRenderCameraID );
					if ( m_Camera_Ptr )
					{
						if ( m_Camera_Ptr->iClipPlaneOn==1 )
						{
							// special mode which creates plane but does not use RenderState to set clip
							// as you cannot mix FF clip and HLSL clip in same scene (artefacts)
							vec.x = m_Camera_Ptr->planeClip.a;
							vec.y = m_Camera_Ptr->planeClip.b;
							vec.z = m_Camera_Ptr->planeClip.c;
							vec.w = m_Camera_Ptr->planeClip.d;
						}
						else
						{
							// ensure shader stops using clip plane when not being clipped!
							vec = GGVECTOR4( 0.0f, 1.0f, 0.0f, 99999.0f );
						}
					}
					else
					{
						// ensure shader stops using clip plane when not being clipped!
						vec = GGVECTOR4( 0.0f, 1.0f, 0.0f, 99999.0f );
					}
					Mesh->pVertexShaderEffect->m_VecClipPlaneEffectHandle->AsVector()->SetFloatVector ( (float*)&vec );
				}

				// Ensure normal invert in effect for terrain (NOTE: not liking duplicated of code)
				#ifdef DX11
				//PE: removed , terrain always invert normal.
//				GGHANDLE pArtFlags = Mesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "ArtFlagControl1" );
//				if ( pArtFlags )
//				{
//					float fInvertNormal = 0.0f;
//					if ( Mesh->dwArtFlags & 0x1 ) fInvertNormal = 1.0f;
//					GGVECTOR4 vec4 = GGVECTOR4 ( fInvertNormal, 0.0f, 0.0f, 0.0f );
//					pArtFlags->AsVector()->SetFloatVector ( (float*)&vec4 );
//				}
				#endif

				// apply effect ready for rendering
				hTechniqueUsed->GetPassByIndex(0)->Apply(0,m_pImmediateContext);

				// Set textures (AFTER Apply which overrides texture view ptrs)
				for ( unsigned long i = 0; i < Mesh->dwTextureCount; i++ ) // terrain now has lots of textures
				{
					if (i != 1 && i != 5 && i != 7) 
					{ 
						//PE: only for more gpu mem (cache) they are not used.
						ID3D11ShaderResourceView* lpTexture = NULL; 
						if ( Mesh->dwTextureCount > i ) lpTexture = Mesh->pTextures[i].pTexturesRefView;
						m_pImmediateContext->PSSetShaderResources ( i, 1, &lpTexture );
					}
				}

				// assign constants
				GGMATRIX matWorld;
				GGGetTransform ( GGTS_WORLD, &matWorld );
				Mesh->pVertexShaderEffect->Start ( Mesh, matWorld );

				//PE: Terrain should be split into smaller meshes for better light.
				update_mesh_light(Mesh, NULL,NULL);

				// Render many terrain chunks (only CB changes for world position for faster rendering)
				BT_Intern_RenderTerrainRec(Terrain,Terrain->QuadTree,Terrain->QuadTreeLevels,iQualityPass);

				// End effect
				BT_Main.CurrentEffect=NULL;

				// Free textures (especially camera image texture which needs to be output bound next cycle)
				for ( unsigned long i = 0; i < Mesh->dwTextureCount; i++ )
				{
					if (i != 1 && i != 5 && i != 7) 
					{ 
						//PE: 
						ID3D11ShaderResourceView* lpTexture = NULL; 
						m_pImmediateContext->PSSetShaderResources ( i, 1, &lpTexture );
					}
				}
			}
		}
	}
	else
	{
		// no rendering if no effect shader
	}
	*/

	// zero current render terrain
	BT_Main.CurrentRenderTerrain=NULL;
}


// ======================================
// === BT INTERN CALCULATE LOD LEVELS ===
// ======================================
static void BT_Intern_CalculateLODLevelsRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level,unsigned char LODLevelToDraw)
{
//Initialise Drawthis variable
	Quadtree->DrawThis=false;

//Check if we have to draw this LOD level
	if(Quadtree->Sector!=NULL && LODLevelToDraw==0)
	{
		if(BT_Intern_DistanceToLODCamera(Terrain,Quadtree->CullBox)>Terrain->LODLevel[Level].Distance*Terrain->LODLevel[Level].Distance)
		{
			Quadtree->DrawThis=true;
			LODLevelToDraw=unsigned char(Level);
		}
	}

//Check if LOD level is greater than 0
	if(Level>0)
	{
	//Part 1
		if(Quadtree->n1->Culled==false)
			BT_Intern_CalculateLODLevelsRec(Terrain,Quadtree->n1,Level-1,LODLevelToDraw);

	//Part 2
		if(Quadtree->n2->Culled==false)
			BT_Intern_CalculateLODLevelsRec(Terrain,Quadtree->n2,Level-1,LODLevelToDraw);

	//Part 3
		if(Quadtree->n3->Culled==false)
			BT_Intern_CalculateLODLevelsRec(Terrain,Quadtree->n3,Level-1,LODLevelToDraw);

	//Part 4
		if(Quadtree->n4->Culled==false)
			BT_Intern_CalculateLODLevelsRec(Terrain,Quadtree->n4,Level-1,LODLevelToDraw);
	}else{
		if(Terrain->LODMap[Quadtree->row][Quadtree->collumn].Level!=LODLevelToDraw)
		{
			Terrain->LODMap[Quadtree->row][Quadtree->collumn].Level=LODLevelToDraw;
		}

	}
}
// === END FUNCTION ===



// ===============================
// === BT INTERN FIX LOD SEAMS ===
// ===============================
static void BT_Intern_FixLODSeams(s_BT_terrain* Terrain)
{
//Find seams to be fixed
	for(unsigned char LODLevel=0;LODLevel<Terrain->LODLevels;LODLevel++)
	{
		unsigned long Span=0x1<<LODLevel;
		for(unsigned long Sector=0;Sector<Terrain->LODLevel[LODLevel].Sectors;Sector++)
		{
			s_BT_Sector* SectorPtr=&Terrain->LODLevel[LODLevel].Sector[Sector];
			if(SectorPtr->QuadTree!=NULL)
			{
				if(SectorPtr->QuadTree->Culled==false && SectorPtr->QuadTree->DrawThis==true)
				{
				//Find row and collumn
					s_BT_QuadTree* CurrentQuadTree=SectorPtr->QuadTree;
					unsigned long Row=SectorPtr->Row*Span;
					unsigned long Collumn=SectorPtr->Column*Span;

				//Left
					if(Collumn>0)
					{
						int LeftSideLODLevel = Terrain->LODMap[Row][Collumn-1].Level - LODLevel;
						if (LeftSideLODLevel < 0)
							LeftSideLODLevel = 0;
						if(SectorPtr->LeftSideLODLevel!=LeftSideLODLevel)
						{
							SectorPtr->LeftSideLODLevel=LeftSideLODLevel;
							SectorPtr->LeftSideNeedsUpdate=true;
						}
					}

				//Top
					if(Row>0)
					{
						int TopSideLODLevel = Terrain->LODMap[Row-1][Collumn].Level - LODLevel;
						if (TopSideLODLevel < 0)
							TopSideLODLevel = 0;
						if(SectorPtr->TopSideLODLevel!=TopSideLODLevel)
						{
							SectorPtr->TopSideLODLevel=TopSideLODLevel;
							SectorPtr->TopSideNeedsUpdate=true;
						}
					}

				//Right
					if(Collumn+Span<Terrain->LODLevel[0].Split)
					{
						int RightSideLODLevel = Terrain->LODMap[Row][Collumn+Span].Level - LODLevel;
						if (RightSideLODLevel < 0)
							RightSideLODLevel = 0;
						if(SectorPtr->RightSideLODLevel!=RightSideLODLevel)
						{
							SectorPtr->RightSideLODLevel=RightSideLODLevel;
							SectorPtr->RightSideNeedsUpdate=true;
						}
					}

				//Bottom
					if(Row+Span<Terrain->LODLevel[0].Split)
					{
						int BottomSideLODLevel = Terrain->LODMap[Row+Span][Collumn].Level - LODLevel;
						if (BottomSideLODLevel < 0)
							BottomSideLODLevel = 0;
						if(SectorPtr->BottomSideLODLevel!=BottomSideLODLevel)
						{
							SectorPtr->BottomSideLODLevel=BottomSideLODLevel;
							SectorPtr->BottomSideNeedsUpdate=true;
						}
					}
				}
			}
		}
	}
}
// === END FUNCTION ===



// ======================================
// === BT INTERN FIX SECTOR LOD SEAMS ===
// ======================================
static void BT_Intern_FixSectorLODSeams(s_BT_Sector* SectorPtr)
{
//Fix the seams
	if(SectorPtr->QuadTree!=NULL && SectorPtr->Excluded==false)
	{
		if(SectorPtr->QuadTree->Culled==false && SectorPtr->QuadTree->DrawThis==true)
		{
		//Make sure that the sector is unlocked
			BT_Intern_UnlockSectorVertexData(SectorPtr);

		//Update sides
			// Top
			if(SectorPtr->TopSideNeedsUpdate) {
				SectorPtr->QuadMap->SetSideLOD(0, SectorPtr->TopSideLODLevel);
				SectorPtr->TopSideNeedsUpdate=false;
				SectorPtr->UpdateMesh=true;
			}

			// Right
			if(SectorPtr->RightSideNeedsUpdate) {
				SectorPtr->QuadMap->SetSideLOD(1, SectorPtr->RightSideLODLevel);
				SectorPtr->RightSideNeedsUpdate=false;
				SectorPtr->UpdateMesh=true;
			}

			// Bottom
			if(SectorPtr->BottomSideNeedsUpdate) {
				SectorPtr->QuadMap->SetSideLOD(2, SectorPtr->BottomSideLODLevel);
				SectorPtr->BottomSideNeedsUpdate=false;
				SectorPtr->UpdateMesh=true;
			}

			// Left
			if(SectorPtr->LeftSideNeedsUpdate) {
				SectorPtr->QuadMap->SetSideLOD(3, SectorPtr->LeftSideLODLevel);
				SectorPtr->LeftSideNeedsUpdate=false;
				SectorPtr->UpdateMesh=true;
			}
		}
	}
}
// === END FUNCTION ===



// =======================================
// === BT INTERN CALCULATE CULLING REC ===
// =======================================
static void BT_Intern_CalculateCullingRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level,bool IntersectingFrustum){

	//Initialise Culled variable
	if (g_bRenderTerrainForShadowMap == true)
	{
		// no culling when rendering terrain shadow
		Quadtree->Culled = false;
	}
	else
	{
		Quadtree->Culled = true;
		if (IntersectingFrustum == true)
		{
			char Culled = BT_Intern_CullBox(Quadtree->CullBox);
			if (Culled > 0)
			{
				Quadtree->Culled = false;
				if (Culled == 2)
					IntersectingFrustum = false;
			}
		}
		else {
			Quadtree->Culled = false;
		}
	}

//Check if LOD level is greater than 0 and the sector isnt culled
	if(Level>0 && Quadtree->Culled==false)
	{
	//Part 1
		BT_Intern_CalculateCullingRec(Terrain,Quadtree->n1,Level-1,IntersectingFrustum); //-x,-z

	//Part 2
		BT_Intern_CalculateCullingRec(Terrain,Quadtree->n2,Level-1,IntersectingFrustum); //+x,-z

	//Part 3
		BT_Intern_CalculateCullingRec(Terrain,Quadtree->n3,Level-1,IntersectingFrustum); //-x,+z

	//Part 4
		BT_Intern_CalculateCullingRec(Terrain,Quadtree->n4,Level-1,IntersectingFrustum); //+x,+z
	}
}
// === END FUNCTION ===



// =======================================
// === BT INTERN UPDATE CULLBOXES REC ===
// =======================================
static void BT_Intern_UpdateCullBoxesRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level){
//Check if the cullbox needs updating
	if(Quadtree->CullboxChanged && Quadtree->Excluded==false)
	{
	//Check if LOD level is greater than 0
		if(Level>0)
		{
		//Part 1
			if(Quadtree->n1->CullboxChanged)
				BT_Intern_UpdateCullBoxesRec(Terrain,Quadtree->n1,Level-1); //-x,-z

		//Part 2
			if(Quadtree->n2->CullboxChanged)
				BT_Intern_UpdateCullBoxesRec(Terrain,Quadtree->n2,Level-1); //+x,-z

		//Part 3
			if(Quadtree->n3->CullboxChanged)
				BT_Intern_UpdateCullBoxesRec(Terrain,Quadtree->n3,Level-1); //-x,+z

		//Part 4
			if(Quadtree->n4->CullboxChanged)
				BT_Intern_UpdateCullBoxesRec(Terrain,Quadtree->n4,Level-1); //+x,+z
		}

	//Update cullboxes
		if(Quadtree->Sector!=NULL)
		{
			Quadtree->CullBox->Top=Quadtree->Sector->Pos_y+Quadtree->Sector->QuadMap->GetHighestPoint();
			Quadtree->CullBox->Bottom=Quadtree->Sector->Pos_y+Quadtree->Sector->QuadMap->GetLowestPoint();
		}else{
			Quadtree->CullBox->Top=max(max(Quadtree->n1->CullBox->Top,Quadtree->n2->CullBox->Top),max(Quadtree->n3->CullBox->Top,Quadtree->n4->CullBox->Top));
			Quadtree->CullBox->Bottom=min(min(Quadtree->n1->CullBox->Bottom,Quadtree->n2->CullBox->Bottom),min(Quadtree->n3->CullBox->Bottom,Quadtree->n4->CullBox->Bottom));
		}
		Quadtree->CullboxChanged=false;
	}
}
// === END FUNCTION ===



// ====================================
// === BT INTERN RENDER TERRAIN REC ===
// ====================================
static void BT_Intern_RenderTerrainRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level,int iQualityPass)
{
	// Check if were not at the bottom
	if(Level>0)
	{
		// If we have to draw this LOD level, draw it. If not, continue down the tree
		if(Quadtree->DrawThis && Quadtree->Sector!=NULL) // Check if current LOD level is active and sector is near the camera
		{
			// Only render if iQualityPass is DISTANT
			if ( iQualityPass==1 )
			{
				// Only render if it is below our required TERRAIN SIZE slider setting
				if ( Level < (unsigned long)g_LevelToRender ) BT_Intern_RenderSector(Quadtree->Sector);
			}

		}else{

			if(BT_Main.LODCamPosition.z/Terrain->Scale*C_BT_INTERNALSCALE<Quadtree->PosZ)
			{
				if(BT_Main.LODCamPosition.x/Terrain->Scale*C_BT_INTERNALSCALE<Quadtree->PosX)
				{
				//Part 1
					if(Quadtree->n1->Excluded==false && Quadtree->n1->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n1,Level-1,iQualityPass);    //-x,-z

				//Part 2
					if(Quadtree->n2->Excluded==false && Quadtree->n2->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n2,Level-1,iQualityPass);    //+x,-z

				//Part 3
					if(Quadtree->n3->Excluded==false && Quadtree->n3->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n3,Level-1,iQualityPass);    //-x,+z

				//Part 4
					if(Quadtree->n4->Excluded==false && Quadtree->n4->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n4,Level-1,iQualityPass);   //+x,+z
				}else{
				//Part 2
					if(Quadtree->n2->Excluded==false && Quadtree->n2->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n2,Level-1,iQualityPass);    //+x,-z

				//Part 1
					if(Quadtree->n1->Excluded==false && Quadtree->n1->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n1,Level-1,iQualityPass);    //-x,-z

				//Part 4
					if(Quadtree->n4->Excluded==false && Quadtree->n4->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n4,Level-1,iQualityPass);   //+x,+z

				//Part 3
					if(Quadtree->n3->Excluded==false && Quadtree->n3->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n3,Level-1,iQualityPass);    //-x,+z
				}

			}else{
				if(BT_Main.LODCamPosition.x/Terrain->Scale*C_BT_INTERNALSCALE<Quadtree->PosX)
				{
				//Part 3
					if(Quadtree->n3->Excluded==false && Quadtree->n3->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n3,Level-1,iQualityPass);    //-x,+z

				//Part 4
					if(Quadtree->n4->Excluded==false && Quadtree->n4->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n4,Level-1,iQualityPass);   //+x,+z

				//Part 1
					if(Quadtree->n1->Excluded==false && Quadtree->n1->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n1,Level-1,iQualityPass);    //-x,-z

				//Part 2
					if(Quadtree->n2->Excluded==false && Quadtree->n2->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n2,Level-1,iQualityPass);    //+x,-z
				}else{
				//Part 4
					if(Quadtree->n4->Excluded==false && Quadtree->n4->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n4,Level-1,iQualityPass);   //+x,+z

				//Part 3
					if(Quadtree->n3->Excluded==false && Quadtree->n3->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n3,Level-1,iQualityPass);    //-x,+z

				//Part 2
					if(Quadtree->n2->Excluded==false && Quadtree->n2->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n2,Level-1,iQualityPass);    //+x,-z

				//Part 1
					if(Quadtree->n1->Excluded==false && Quadtree->n1->Culled==false)
						BT_Intern_RenderTerrainRec(Terrain,Quadtree->n1,Level-1,iQualityPass);    //-x,-z
				}
			}
		}
	}
	else
	{
		if(Quadtree->Sector!=NULL)
		{
			// Only render if iQualityPass is NEAR
			if ( iQualityPass==0 )
			{
				BT_Intern_RenderSector(Quadtree->Sector);
			}
		}
	}
}
// === END FUNCTION ===



// ====================================
// === BT INTERN UNLOCK SECTORS REC ===
// ====================================
static void BT_Intern_UnlockSectorsRec(s_BT_terrain* Terrain,s_BT_QuadTree* Quadtree,unsigned long Level)
{
//Check if were not at the bottom
	if(Level>0){
	//If we have to draw this LOD level, draw it. If not, continue down the tree
		if(Quadtree->DrawThis && Quadtree->Sector!=NULL)//Check if current LOD level is active and sector is near the camera
		{
			BT_Intern_UnlockSectorVertexData(Quadtree->Sector);
		}else{
		//Part 1
			if(Quadtree->n1->Excluded==false && Quadtree->n1->Culled==false)
				BT_Intern_UnlockSectorsRec(Terrain,Quadtree->n1,Level-1);    //-x,-z

		//Part 2
			if(Quadtree->n2->Excluded==false && Quadtree->n2->Culled==false)
				BT_Intern_UnlockSectorsRec(Terrain,Quadtree->n2,Level-1);    //+x,-z

		//Part 3
			if(Quadtree->n3->Excluded==false && Quadtree->n3->Culled==false)
				BT_Intern_UnlockSectorsRec(Terrain,Quadtree->n3,Level-1);    //-x,+z

		//Part 4
			if(Quadtree->n4->Excluded==false && Quadtree->n4->Culled==false)
				BT_Intern_UnlockSectorsRec(Terrain,Quadtree->n4,Level-1);   //+x,+z
		}
	}else{
		if(Quadtree->Sector!=NULL){
			BT_Intern_UnlockSectorVertexData(Quadtree->Sector);
		}
	}
}

// === END FUNCTION ===



// ===============================
// === BT INTERN RENDER SECTOR ===
// ===============================
static void BT_Intern_RenderSector(s_BT_Sector* Sector)
{
	//Render sector
	if(Sector->Excluded==false)
	{
		// if sector DBP object has been made universe invisible, it means the engine has occluded it
		//if ( Sector->DBPObject==NULL || (Sector->DBPObject && Sector->DBPObject->bUniverseVisible==true ) ) / 070314 not always work :(
		if ( 1 )
		{
			//Fix LOD seams
			BT_Intern_FixSectorLODSeams(Sector);

			//Check if the sector needs to update its drawbuffer
			//Sector->QuadMap->GenerateMeshData();
			if(Sector->UpdateMesh==true)
			{
				// if often can be locking the index and vertex buffers each time which would be slow
				Sector->QuadMap->UpdateMesh(Sector->DrawBuffer,true);
				Sector->UpdateMesh=false;
			}

			#ifdef DX11

			// calculate world/v/p position of terrain chunk
			
			GGMATRIX World = Sector->WorldMatrix*BT_Main.CurrentRenderTerrain->Object->position.matScale*BT_Main.CurrentRenderTerrain->Object->position.matTranslation;
			if ( m_pCBChangePerTerrsainChunk )
			{
				// as terrain would self-shadow
				if (g_bRenderTerrainForShadowMap == true)
				{
					// sink it a little so not interfere with itself
					World._42 -= 10.0f;
				}

				CBChangePerTerrsainChunk cb;
				cb.mWorld = World;
				GGMatrixTranspose(&cb.mWorld,&cb.mWorld);
				GGGetTransform(GGTS_VIEW,&cb.mView);
				GGGetTransform(GGTS_PROJECTION,&cb.mProjection);
				GGMatrixTranspose(&cb.mView,&cb.mView);
				GGMatrixTranspose(&cb.mProjection,&cb.mProjection);
				m_pImmediateContext->UpdateSubresource( m_pCBChangePerTerrsainChunk, 0, NULL, &cb, 0, 0 );
				m_pImmediateContext->VSSetConstantBuffers ( 0, 1, &m_pCBChangePerTerrsainChunk );
				m_pImmediateContext->PSSetConstantBuffers ( 0, 1, &m_pCBChangePerTerrsainChunk );
			}
			if ( 1==1 )
			{
				CBChangePerTerrsainChunkPS cbps;
				//cbps.vMaterialEmissive = GGCOLOR(pMesh->mMaterial.Emissive.r,pMesh->mMaterial.Emissive.g,pMesh->mMaterial.Emissive.b,pMesh->mMaterial.Emissive.a);
				//if ( pMesh->bAlphaOverride == true )
				//	cbps.fAlphaOverride = (pMesh->dwAlphaOverride>>24)/255.0f;
				//else
				//	cbps.fAlphaOverride = 1.0f;
				cbps.vMaterialEmissive = GGCOLOR(0,0,0,0);
				cbps.fAlphaOverride = 1.0f;

				// feed camera zero matrices into pixel shader constant buffer
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( 0 );
				float fDet = 0.0f;
				GGMatrixInverse ( &cbps.mViewInv, &fDet, &m_Camera_Ptr->matView );
				GGMatrixTranspose(&cbps.mViewInv,&cbps.mViewInv);
				//cbps.mViewProj = g_matThisViewProj;
				//GGMatrixTranspose(&cbps.mViewProj,&cbps.mViewProj);
				//cbps.mPrevViewProj = g_matPreviousViewProj;
				//GGMatrixTranspose(&cbps.mPrevViewProj,&cbps.mPrevViewProj);
				m_pImmediateContext->UpdateSubresource( m_pCBChangePerTerrsainChunkPS, 0, NULL, &cbps, 0, 0 );
				m_pImmediateContext->PSSetConstantBuffers ( 1, 1, &m_pCBChangePerTerrsainChunkPS );
			}

			// Index buffers
			//This line is ruin cascade 0 ?
			m_pImmediateContext->IASetIndexBuffer(Sector->DrawBuffer->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

			// Vertex buffers
			unsigned int stride;
			unsigned int offset;
			stride = Sector->DrawBuffer->FVF_Size;
			offset = 0;
			m_pImmediateContext->IASetVertexBuffers ( 0, 1, &Sector->DrawBuffer->VertexBuffer, &stride, &offset);

			//Draw
			m_pImmediateContext->DrawIndexed(Sector->DrawBuffer->Indices, 0, 0);

			#else
			//World matrix
			IGGDevice* D3DDevice=m_pD3D;
			GGMATRIX World;
			World=Sector->WorldMatrix*BT_Main.CurrentRenderTerrain->Object->position.matScale*BT_Main.CurrentRenderTerrain->Object->position.matTranslation;
			D3DDevice->SetTransform(GGTS_WORLD,&World);

			//DBPRO RENDERING ENGINE
			//Dave - applying effect could be slow
			sMesh* Mesh=Sector->Terrain->Object->pFrame->pMesh;
			if(Mesh->pVertexShaderEffect!=NULL)
				DBPRO_ApplyEffect(Mesh,BT_Main.CurrentUpdateCamera);

			//Index and vertex buffers
			D3DDevice->SetStreamSource(0,Sector->DrawBuffer->VertexBuffer,0,Sector->DrawBuffer->FVF_Size);
			D3DDevice->SetIndices(Sector->DrawBuffer->IndexBuffer);

			//Draw
			D3DDevice->DrawIndexedPrimitive(GGPT_TRIANGLELIST,0,0,Sector->DrawBuffer->Vertices,0,Sector->DrawBuffer->Primitives);

			//Draw edge
			D3DDevice->SetIndices(Sector->DrawBuffer->EdgeLineIndexBuffer);
			D3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST,0,0,Sector->LODLevel->SectorDetail*2,0,Sector->DrawBuffer->EdgeIndexCount/2);
			#endif

			//Stats
			BT_Main.DrawCalls++;
			BT_Main.DrawPrimitiveCount+=Sector->DrawBuffer->Primitives;
			g_pGlob->dwNumberOfPrimCalls++;
			g_pGlob->dwNumberOfPolygonsDrawn+=Sector->DrawBuffer->Primitives;
		}
	}
}
// === END FUNCTION ===


//LEFT = x-size
//RIGHT = x+size
//TOP = y+size
//BOTTOM = y-size
//FRONT = z-size
//BACK = z+size

// ========================================
// === BT INTERN DISTANCE TO LOD CAMERA ===
// ========================================
static float BT_Intern_DistanceToLODCamera(s_BT_terrain* Terrain,s_BT_CullBox* CullBox)
{
//Variables
	float CamX=(BT_Main.LODCamPosition.x-Terrain->Object->position.vecPosition.x)/Terrain->Scale*C_BT_INTERNALSCALE;
	float CamY=(BT_Main.LODCamPosition.y-Terrain->Object->position.vecPosition.y)/Terrain->Scale*C_BT_INTERNALSCALE;
	float CamZ=(BT_Main.LODCamPosition.z-Terrain->Object->position.vecPosition.z)/Terrain->Scale*C_BT_INTERNALSCALE;

//Calculate distances
	float XDistA=-CamX+CullBox->Left;
	float XDistB=CamX-CullBox->Right;
	float YDistA=-CamY+CullBox->Bottom;
	float YDistB=CamY-CullBox->Top;
	float ZDistA=-CamZ+CullBox->Front;
	float ZDistB=CamZ-CullBox->Back;

//Calculate X Distance
	float XDist;
	if(XDistA>0.0f){
		XDist=-XDistA;
	}else if(XDistB>0.0f){
		XDist=XDistB;
	}else{
		XDist=0.0f;
	}

//Calculate Y Distance
	float YDist;
	if(YDistA>0.0f){
		YDist=-YDistA;
	}else if(YDistB>0.0f){
		YDist=YDistB;
	}else{
		YDist=0.0f;
	}

//Calculate Z Distance
	float ZDist;
	if(ZDistA>0.0f){
		ZDist=-ZDistA;
	}else if(ZDistB>0.0f){
		ZDist=ZDistB;
	}else{
		ZDist=0.0f;
	}

//Return distance
	return XDist*XDist+YDist*YDist+ZDist*ZDist;
}
// === END FUNCTION ===



// ================================
// === BT INTERN CONTINUE BUILD ===
// ================================
static void BT_Intern_ContinueBuild()
{
//Variables
	s_BT_Sector* Sector;

//Get sector
	Sector=&BT_Main.CurrentBuildTerrain->LODLevel[BT_Main.CurrentBuildLODLevel].Sector[BT_Main.CurrentBuildSector];

//Check that the sector is not excluded
	if(Sector->Excluded==false)
	{
	//Build
		if(BT_Main.BuildType==true)
		{ // Rebuild
			BT_Intern_BuildSector(Sector);
		}else{ //Firstbuild
			BT_Intern_BuildSector(Sector);
		}
	}

//Set Current values
	BT_Main.CurrentBuildRow=Sector->Row;
	BT_Main.CurrentBuildColumn=Sector->Column;

//Increase sector number
	BT_Main.CurrentBuildSector++;
	BT_Main.CurrentBuildTerrainSector++;

//Check if this LOD Level is finnished
	if(BT_Main.CurrentBuildSector==BT_Main.CurrentBuildTerrain->LODLevel[BT_Main.CurrentBuildLODLevel].Sectors)
	{
		BT_Main.CurrentBuildSector=0;
		BT_Main.CurrentBuildLODLevel++;
	}

	return;
}
// === END FUNCTION ===



// ===============================
// === BT INTERN TERRAIN EXIST ===
// ===============================
bool BT_Intern_TerrainExist(unsigned long terrainid)
{
//Check the range of the value
	if(terrainid>0 && terrainid<=C_BT_MAXTERRAINS)
	{
	//Return the exist varaible
		return BT_Main.Terrains[terrainid].Exists;
	}
	return 0;
}
// === END FUNCTION ===



// =============================
// === BT INTERN IMAGE EXIST ===
// =============================
static long BT_Intern_ImageExist(unsigned long imageid)
{
//Check the range of the value
	if(imageid>0)
	{
	//Check if it exists and return the answer
		return GetImageExistEx(imageid);
	}
	return 0;
}
// === END FUNCTION ===



// =======================
// === BT INTERN ERROR ===
// =======================
void BT_Intern_Error(int number)
{
//Variables
	char Message[100];
	Message[0]=NULL;

//Create message string
	strcat(Message,"BT Error: ");
	strcat(Message,BT_Intern_GetErrorString(number));
	strcat(Message,"\nFunction: ");
	strcat(Message,BT_Intern_GetFunctionName(BT_Main.CurrentFunction));

//Display string and terminate app
	// now use DBP error system
	RunTimeError ( 0, Message );

}
// === END FUNCTION ===



// ==================================
// === BT INTERN GET ERROR STRING ===
// ==================================
static char* BT_Intern_GetErrorString(int number)
{
//Set default error message
	char* Error="Unknown";

//Get error
	if(number==C_BT_ERROR_MAXTERRAINSEXCEDED){
		Error="Max terrains Exceded";
	}else if(number==C_BT_ERROR_TERRAINDOESNTEXIST){
		Error="Terrain doesnt exist";
	}else if(number==C_BT_ERROR_INVALIDLODLEVELS){
		Error="Invalid LOD levels";
	}else if(number==C_BT_ERROR_HEIGHTMAPDOESNTEXIST){
		Error="Heightmap doesnt exist";
	}else if(number==C_BT_ERROR_LODLEVELDOESNTEXIST){
		Error="LOD level doesnt exist";
	}else if(number==C_BT_ERROR_HEIGHTMAPSIZEINVALID){
		Error="Heightmap size invalid";
	}else if(number==C_BT_ERROR_EXCLUSIONMAPSIZEINVALID){
		Error="Exclusion map size invalid";
	}else if(number==C_BT_ERROR_ALREADYBUILDING){
		Error="Already building";
	}else if(number==C_BT_ERROR_OBJECTIDILLEGAL){
		Error="Object ID illegal";
	}else if(number==C_BT_ERROR_TERRAINNOTGENERATED){
		Error="Terrain not generated";
	}else if(number==C_BT_ERROR_CANNOTCREATEVB){
		Error="Cannot create Vertex Buffer";
	}else if(number==C_BT_ERROR_CANNOTCREATEIB){
		Error="Cannot Create Index Buffer";
	}else if(number==C_BT_ERROR_SECTORDOESNTEXIST){
		Error="Sector doesnt exist";
	}else if(number==C_BT_ERROR_TERRAINNOTBUILT){
		Error="Terrain not built";
	}else if(number==C_BT_ERROR_INVALIDFILE){
		Error="Invalid file";
	}else if(number==C_BT_ERROR_VERSIONCANNOTREADFILE){
		Error="Cannot read file";
	}else if(number==C_BT_ERROR_TERRAINALREADYBUILT){
		Error="Terrain already built";
	}else if(number==C_BT_ERROR_CANNOTUSEFUNCTIONONBUILTTERRAIN){
		Error="Cannot use function with a built terrain";
	}else if(number==C_BT_ERROR_SECTORALREADYLOCKED){
		Error="Sector already locked";
	}else if(number==C_BT_ERROR_SECTORNOTUNLOCKED){
		Error="Sector not unlocked";
	}else if(number==C_BT_ERROR_USESFULLVERSION){
		Error="Terrain uses full version features. Cannot load.";
	}else if(number==C_BT_ERROR_SECTORTOOBIG){
		Error="Sectors too big. Increase split.";
	}else if(number==C_BT_ERROR_SPLITTOOHIGH){
		Error="Split too big. (Max: 32)";
	}else if(number==C_BT_ERROR_SECTORALREADYHASOBJECT){
		Error="This sector already has an object";
	}else if(number==C_BT_ERROR_LODLEVELALREADYHASOBJECT){
		Error="This LOD level already has an object";
	}else if(number==C_BT_ERROR_SECTORISEXCLUDED){
		Error="Sector is excluded";
	}else if(number==C_BT_ERROR_MEMORYERROR){
		Error="Memory error";
	}

//Return the error message
	return Error;
}
// === END FUNCTION ===



// ===================================
// === BT INTERN GET FUNCTION NAME ===
// ===================================
static char* BT_Intern_GetFunctionName(int number)
{
//Set default name
	char* Name="Unknown";

//Get name
	if(number==C_BT_FUNCTION_MAKETERRAIN){
		Name="BT MakeTerrain";
	}else if(number==C_BT_FUNCTION_SETTERRAINHEIGHTMAP){
		Name="BT SetTerrainHeightmap";
	}else if(number==C_BT_FUNCTION_SETTERRAINTEXTURE){
		Name="BT SetTerrainTexture";
	}else if(number==C_BT_FUNCTION_SETTERRAINEXCLUSION){
		Name="BT SetTerrainExclusion";
	}else if(number==C_BT_FUNCTION_SETTERRAINDETAIL){
		Name="BT SetTerrainDetail";
	}else if(number==C_BT_FUNCTION_SETTERRAINENVIRONMENT){
		Name="BT SetTerrainEnvironment";
	}else if(number==C_BT_FUNCTION_SETTERRAINLOD){
		Name="BT SetTerrainLOD";
	}else if(number==C_BT_FUNCTION_SETTERRAINSPLIT){
		Name="BT SetTerrainSplit";
	}else if(number==C_BT_FUNCTION_SETTERRAINDETAILTILE){
		Name="BT SetTerrainDetailTile";
	}else if(number==C_BT_FUNCTION_SETTERRAINQUADREDUCTION){
		Name="BT SetTerrainQuadReduction";
	}else if(number==C_BT_FUNCTION_SETTERRAINQUADROTATION){
		Name="BT SetTerrainQuadRotation";
	}else if(number==C_BT_FUNCTION_SETTERRAINSMOOTHING){
		Name="BT SetTerrainSmoothing";
	}else if(number==C_BT_FUNCTION_SETTERRAINSCALE){
		Name="BT SetTerrainScale";
	}else if(number==C_BT_FUNCTION_SETTERRAINYSCALE){
		Name="BT SetTerrainYScale";
	}else if(number==C_BT_FUNCTION_SETTERRAINLODDISTANCES){
		Name="BT SetTerrainLODDistances";
	}else if(number==C_BT_FUNCTION_BUILDTERRAIN){
		Name="BT BuildTerrain";
	}else if(number==C_BT_FUNCTION_CONTINUEBUILD){
		Name="BT ContinueBuild";
	}else if(C_BT_FUNCTION_TERRAINEXIST){
		Name="BT TerrainExist";
	}else if(number==C_BT_FUNCTION_DELETETERRAIN){
		Name="BT DeleteTerrain";
	}else if(number==C_BT_FUNCTION_GETGROUNDHEIGHT){
		Name="BT GetGroundHeight";
	}else if(number==C_BT_FUNCTION_GETTERRAINSIZE){
		Name="BT GetTerrainSize";
	}else if(number==C_BT_FUNCTION_GETVERSION){
		Name="BT GetVersion";
	}else if(number==C_BT_FUNCTION_SETBUILDSTEP){
		Name="BT SetBuildStep";
	}else if(number==C_BT_FUNCTION_UPDATETERRAIN){
		Name="BT UpdateTerrain";
	}else if(number==C_BT_FUNCTION_UPDATE){
		Name="BT Update";
	}else if(number==C_BT_FUNCTION_RENDER){
		Name="BT Render";
	}else if(number==C_BT_FUNCTION_ENABLEAUTORECOVERY){
		Name="BT EnableAutoRecovery";
	}else if(number==C_BT_FUNCTION_GETSTATISTIC){
		Name="BT GetStatistic";
	}else if(number==C_BT_FUNCTION_CLEARSTATISTICS){
		Name="BT ClearStatistics";
	}else if(number==C_BT_FUNCTION_GETOBJECTID){
		Name="BT GetObjectID";
	}else if(number==C_BT_FUNCTION_MAKESECTOROBJECT){
		Name="BT MakeSectorObject";
	}else if(number==C_BT_FUNCTION_GETSECTORPOSITIONX){
		Name="BT GetSectorPositionX";
	}else if(number==C_BT_FUNCTION_GETSECTORPOSITIONY){
		Name="BT GetSectorPositionY";
	}else if(number==C_BT_FUNCTION_GETSECTORPOSITIONZ){
		Name="BT GetSectorPositionZ";
	}else if(number==C_BT_FUNCTION_GETSECTORCOUNT){
		Name="BT GetSectorCount";
	}else if(number==C_BT_FUNCTION_GETSECTORSIZE){
		Name="BT GetSectorSize";
	}else if(number==C_BT_FUNCTION_GETSECTOREXCLUDED){
		Name="BT GetSectorExcluded";
	}else if(number==C_BT_FUNCTION_GETSECTORROW){
		Name="BT GetSectorRow";
	}else if(number==C_BT_FUNCTION_GETSECTORCOLLUMN){
		Name="BT GetSectorCollumn";
	}else if(number==C_BT_FUNCTION_SETCURRENTCAMERA){
		Name="BT SetCurrentCamera";
	}else if(number==C_BT_FUNCTION_UPDATETERRAINLOD){
		Name="BT UpdateTerrainLOD";
	}else if(number==C_BT_FUNCTION_UPDATETERRAINCULL){
		Name="BT UpdateTerrainCull";
	}else if(number==C_BT_FUNCTION_SETPOINTHEIGHT){
		Name="BT SetPointHeight";
	}else if(number==C_BT_FUNCTION_SETTERRAINEXCLUSIONTHRESHOLD){
		Name="BT SetTerrainExclusionThreshold";
	}else if(number==C_BT_FUNCTION_LOADTERRAIN){
		Name="BT LoadTerrain";
	}else if(number==C_BT_FUNCTION_SAVETERRAIN){
		Name="BT SaveTerrain";
	}else if(number==C_BT_FUNCTION_GETPOINTEXCLUDED){
		Name="BT GetPointExcluded";
	}else if(number==C_BT_FUNCTION_SETATMODE){
		Name="BT SetATMode";
	}else if(number==C_BT_FUNCTION_SETTERRAINDETAILBLENDMODE){
		Name="BT SetTerrainDetailBlendMode";
	}else if(number==C_BT_FUNCTION_INIT){
		Name="BT Init";
	}else if(number==C_BT_FUNCTION_ADDTERRAINENVIRONMENT){
		Name="BT AddTerrainEnvironment";
	}else if(number==C_BT_FUNCTION_GETPOINTENVIRONMENT){
		Name="BT GetPointEnvironment";
	}else if(number==C_BT_FUNCTION_GETTERRAININFO){
		Name="BT GetTerrainInfo";
	}else if(number==C_BT_FUNCTION_GETLODLEVELINFO){
		Name="BT GetLODLevelInfo";
	}else if(number==C_BT_FUNCTION_GETSECTORINFO){
		Name="BT GetSectorInfo";
	}else if(number==C_BT_FUNCTION_MAKETERRAINOBJECT){
		Name="BT MakeTerrainObject";
	}else if(number==C_BT_FUNCTION_ENABLEAUTORENDER){
		Name="BT EnableAutoRender";
	}

//Return the name
	return Name;
}
// === END FUNCTION ===



// ====================================
// === BT INTERN GET SECTOR HEIGHTS ===
// ====================================
static void BT_Intern_GetSectorHeights(s_BT_terrain* Terrain,unsigned long LODLevel,unsigned long row,unsigned long column,float* buffer)
{
//Variables
	unsigned long StartX;
	unsigned long StartY;
	unsigned long X;
	unsigned long Y;
	unsigned long Xb;
	unsigned long Yb;
	unsigned long BufferPos;
	unsigned long HeightPos;

//Set start positions
	StartX=column*Terrain->LODLevel[LODLevel].SectorDetail*Terrain->LODLevel[LODLevel].TileSpan;
	StartY=row*Terrain->LODLevel[LODLevel].SectorDetail*Terrain->LODLevel[LODLevel].TileSpan;

//Loop through points
	for(Y=0;Y<=Terrain->LODLevel[LODLevel].SectorDetail;Y++)
	{
		for(X=0;X<=Terrain->LODLevel[LODLevel].SectorDetail;X++)
		{
			BufferPos=(X+Y*(Terrain->LODLevel[LODLevel].SectorDetail+1));

			if(X*Terrain->LODLevel[LODLevel].TileSpan+StartX>unsigned(Terrain->Heightmapsize-1) )
			{
				Xb=X*Terrain->LODLevel[LODLevel].TileSpan-1;
			}else{
				Xb=X*Terrain->LODLevel[LODLevel].TileSpan;
			}

			if(Y*Terrain->LODLevel[LODLevel].TileSpan+StartY>unsigned(Terrain->Heightmapsize-1))
			{
				Yb=Y*Terrain->LODLevel[LODLevel].TileSpan-1;
			}else{;
				Yb=Y*Terrain->LODLevel[LODLevel].TileSpan;
			}

			HeightPos=((Xb+StartX)+(Yb+StartY)*Terrain->Heightmapsize);
			buffer[BufferPos]=Terrain->HeightPoint[HeightPos];
		}
	}
}
// === END FUNCTION ===



// =====================================
// === BT INTERN GET SECTOR EXCLUSION ===
// ======================================
static bool BT_Intern_GetSectorExclusion(s_BT_terrain* Terrain,unsigned long LODLevel,unsigned long excludememblock,unsigned long row,unsigned long column,bool* buffer)
{
//Variables
	bool WholeSectorExcluded=true;
	unsigned long X;
	unsigned long Y;
	unsigned long Xb;
	unsigned long Yb;
	unsigned long StartX;
	unsigned long StartY;
	unsigned long BufferPos;
	unsigned long ExcludePos;
	bool Excluded;

//Set start positions
	StartX=column*Terrain->LODLevel[LODLevel].SectorDetail*Terrain->LODLevel[LODLevel].TileSpan;
	StartY=row*Terrain->LODLevel[LODLevel].SectorDetail*Terrain->LODLevel[LODLevel].TileSpan;

//Loop through points
	for(Y=0;Y<=Terrain->LODLevel[LODLevel].SectorDetail;Y++)
	{
		for(X=0;X<=Terrain->LODLevel[LODLevel].SectorDetail;X++)
		{

			BufferPos=(X+Y*(Terrain->LODLevel[LODLevel].SectorDetail+1));

			if(X*Terrain->LODLevel[LODLevel].TileSpan+StartX>unsigned(Terrain->Heightmapsize-1)) 
			{
				Xb=X*Terrain->LODLevel[LODLevel].TileSpan-1;
			}else{
				Xb=X*Terrain->LODLevel[LODLevel].TileSpan;
			}

			if(Y*Terrain->LODLevel[LODLevel].TileSpan+StartY>unsigned(Terrain->Heightmapsize-1))
			{
				Yb=Y*Terrain->LODLevel[LODLevel].TileSpan-1;
			}else{;
				Yb=Y*Terrain->LODLevel[LODLevel].TileSpan;
			}
			ExcludePos=12+((Xb+StartX)+(Yb+StartY)*Terrain->Heightmapsize)*4;
			#ifdef DX11
			Excluded=false;// no holes in terrain for DX11
			#else
			Excluded=Terrain->ExclusionThreshold>unsigned(GGCOLOR(ReadMemblockDWord(excludememblock,ExcludePos)).r*255);
			buffer[BufferPos]=Excluded;
			#endif
			if(Excluded==false)
				WholeSectorExcluded=false;
		}
	}

//Return
	return WholeSectorExcluded;
}
// === END FUNCTION ===



// ==============================
// === BT INTERN BUILD SECTOR ===
// ==============================
static void BT_Intern_BuildSector(s_BT_Sector* Sector)
{
//Generate meshdata
	Sector->QuadMap->GenerateMeshData();

//Create object
	Sector->DrawBuffer=Sector->QuadMap->GeneratePlain();

//Make world matrix
	GGMatrixTranslation(&Sector->WorldMatrix,Sector->Pos_x*Sector->Terrain->Scale/C_BT_INTERNALSCALE,Sector->Pos_y,Sector->Pos_z*Sector->Terrain->Scale/C_BT_INTERNALSCALE);
}
// === END FUNCTION ===



// ================================
// === BT INTERN DELETE TERRAIN ===
// ================================
static void BT_Intern_DeleteTerrain(unsigned long TerrainID,bool DeleteObjectFlag)
{
//Check that the terrain is built
	if(BT_Main.Terrains[TerrainID].Built==true)
	{
	//Get terrain pointer
		s_BT_terrain* Terrain;
		Terrain=&BT_Main.Terrains[TerrainID];

	//Delete LODMap
		for(unsigned long i=0;i<Terrain->LODLevel[0].Split;i++)
			free(Terrain->LODMap[i]);
		free(Terrain->LODMap);

	//Delete LODLevels
		for(unsigned long LODLevel=0;LODLevel<Terrain->LODLevels;LODLevel++)
		{
		//Delete sectors
			unsigned long Sector=0;
			for(Sector=0;Sector<Terrain->LODLevel[LODLevel].Sectors;Sector++)
			{
			//Check if the sector is not excluded
				if(Terrain->LODLevel[LODLevel].Sector[Sector].Excluded==false)
				{
					#ifdef DX11
					// delete draw buffer in DX11
					#else
					//Delete drawbuffer
					Terrain->LODLevel[LODLevel].Sector[Sector].DrawBuffer->VertexBuffer->Release();
					Terrain->LODLevel[LODLevel].Sector[Sector].DrawBuffer->IndexBuffer->Release();
					Terrain->LODLevel[LODLevel].Sector[Sector].DrawBuffer->EdgeLineIndexBuffer->Release();
					#endif
					free(Terrain->LODLevel[LODLevel].Sector[Sector].DrawBuffer);

				//Delete Quadmap
					Terrain->LODLevel[LODLevel].Sector[Sector].QuadMap->DeleteInternalData();
					free(Terrain->LODLevel[LODLevel].Sector[Sector].QuadMap);

				//Delete RTTMS
					free(Terrain->LODLevel[LODLevel].Sector[Sector].VertexDataRTTMS);

				//Delete Info
					free(Terrain->LODLevel[LODLevel].Sector[Sector].Info);
				}
			}
			free(Terrain->LODLevel[LODLevel].Sector);

		//Delete Info
			free(Terrain->LODLevel[LODLevel].Info);
		}
		free(Terrain->LODLevel);

	//Delete Quadtree
		BT_Intern_DeAllocateQuadTree(Terrain->QuadTree);

	//Delete Environment map
		BT_Intern_DeleteEnvironmentMap(Terrain->EnvironmentMap);
		free(Terrain->EnvironmentMap);

	//Delete object
		if(DeleteObjectFlag==true)
			DeleteObject(Terrain->ObjectID);

	//Delete update handlers
		BT_RTTMS_DeleteUpdateHandlers(TerrainID);

	//Delete Info
		free(Terrain->Info);

	//Zero terrain array
		memset(Terrain,0,sizeof(s_BT_terrain));
	}else{
	//Zero terrain array
		memset(&BT_Main.Terrains[TerrainID],0,sizeof(s_BT_terrain));
	}
}
// === END FUNCTION ===

// ==================================
// === BT INTERN GET POINT HEIGHT ===
// ==================================
float BT_Intern_GetPointHeight(s_BT_terrain* Terrain,float Px,float Pz,char LODLevel,bool Round)
{
//Variables
	float Height;
	unsigned short SRow;
	unsigned short SCollumn;
	unsigned short SectorID;
	s_BT_Sector* Sector;
	BT_QuadMap* Quadmap;

//Clamp values
	if(Px>Terrain->TerrainSize)
		return 0.0;

	if(Pz>Terrain->TerrainSize)
		return 0.0;

	if(Px<0)
		return 0.0;

	if(Pz<0)
		return 0.0;

//Find the row and collumn of the sector
	SRow=(unsigned short)floor(Px/Terrain->LODLevel[LODLevel].SectorSize);
	SCollumn=(unsigned short)floor(Pz/Terrain->LODLevel[LODLevel].SectorSize);

//As the above clamping sometimes makes mistakes, we will correct them here
	if(SRow>unsigned(Terrain->LODLevel[LODLevel].Split-1))
		SRow=Terrain->LODLevel[LODLevel].Split-1;

	if(SCollumn>unsigned(Terrain->LODLevel[LODLevel].Split-1))
		SCollumn=Terrain->LODLevel[LODLevel].Split-1;

	SectorID=SCollumn+SRow*Terrain->LODLevel[LODLevel].Split;

//Find the sector pointer
	Sector=&Terrain->LODLevel[LODLevel].Sector[SectorID];

//Get point on sector
	Px=Px-Sector->Pos_x;
	Pz=Pz-Sector->Pos_z;

//Find the quadmap pointer
	Quadmap=Sector->QuadMap;

//Check that the sector exists
	if(Sector->Excluded==false)
	{
	//Get the height
		Height=Quadmap->GetPointHeight(Px,Pz,Round);

	//Return the height
		return Height;

	}else{
	//Return nothing
		return 0.0;
	}
}
// === END FUNCTION ===



// ==================================
// === BT INTERN GET POINT NORMAL ===
// ==================================
BTVector3 BT_Intern_GetPointNormal(s_BT_terrain* Terrain,float Px,float Pz)
{
//Variables
	BTVector3 Normal;
	float Tilesize;
	float Top;
	float Left;
	float Bottom;
	float Right;
	float Dx;
	float Dy;
	float Dz;

//Work out tilesize (halved to make this code a little faster)
	Tilesize=C_BT_INTERNALSCALE/2.0;

//Find point heights
	Top=BT_Intern_GetPointHeight(Terrain,Px,float(Pz+Tilesize),0,0);
	Left=BT_Intern_GetPointHeight(Terrain,float(Px-Tilesize),Pz,0,0);
	Bottom=BT_Intern_GetPointHeight(Terrain,Px,float(Pz-Tilesize),0,0);
	Right=BT_Intern_GetPointHeight(Terrain,float(Px+Tilesize),Pz,0,0);

//If Top or Right heights are 0 then set height to middle (bug fix)
	if(Top==0.0f || Left==0.0f || Bottom==0.0f || Right==0.0f)
	{
		float Middle=BT_Intern_GetPointHeight(Terrain,Px,Pz,0,0);
		if(Top==0.0f)
			Top=Middle*2-Bottom;
		if(Left==0.0f)
			Left=Middle*2-Right;
		if(Bottom==0.0f)
			Bottom=Middle*2-Top;
		if(Right==0.0f)
			Right=Middle*2-Left;
	}

//Get distances
	Dx=(Bottom-Top)/Tilesize;
	Dz=(Right-Left)/Tilesize;
	Dx=Dx/Terrain->Scale*C_BT_INTERNALSCALE;
	Dz=Dz/Terrain->Scale*C_BT_INTERNALSCALE;
	Dy=float(1.0/sqrt(1.0+Dx*Dx+Dz*Dz));

//Work out normal
	Normal.x=-Dz*Dy;
	Normal.y=Dy;
	Normal.z=Dx*Dy;

	return Normal;
}
// === END FUNCTION ===



// ====================================
// === BT INTERN GET POINT EXCLUDED ===
// ====================================
static bool BT_Intern_GetPointExcluded(s_BT_terrain* Terrain,float Px,float Pz)
{
//Variables
	unsigned short SRow;
	unsigned short SCollumn;
	unsigned short SectorID;
	s_BT_Sector* Sector;
	BT_QuadMap* Quadmap;

//Clamp values
	if(Px>Terrain->TerrainSize-C_BT_INTERNALSCALE)
		return true;

	if(Pz>Terrain->TerrainSize-C_BT_INTERNALSCALE)
		return true;

	if(Px<0)
		return true;

	if(Pz<0)
		return true;

//Find the row and collumn of the sector
	SRow=(unsigned short) floor(Px/Terrain->LODLevel[0].SectorSize);
	SCollumn=(unsigned short) floor(Pz/Terrain->LODLevel[0].SectorSize);

//As the above clamping sometimes makes mistakes, we will correct them here
	if(SRow>unsigned(Terrain->LODLevel[0].Split-1))
		SRow=Terrain->LODLevel[0].Split-1;

	if(SCollumn>unsigned(Terrain->LODLevel[0].Split-1))
		SCollumn=Terrain->LODLevel[0].Split-1;

	SectorID=SCollumn+SRow*Terrain->LODLevel[0].Split;

//Find the sector pointer
	Sector=&Terrain->LODLevel[0].Sector[SectorID];

//Get point on sector
	Px=Px-Sector->Pos_x;
	Pz=Pz-Sector->Pos_z;

//Find the quadmap pointer
	Quadmap=Sector->QuadMap;

//Check that the sector exists
	if(Sector->Excluded==false)
	{
	//Get the excluded
		return Quadmap->GetPointExcluded(Px,Pz);
	}else{
	//Return nothing
		return false;
	}
}
// === END FUNCTION ===



// ===================================
// === BT INTERN ALLOCATE QUADTREE ===
// ===================================
s_BT_QuadTree* BT_Intern_AllocateQuadTree(s_BT_terrain* Terrain)
{
//Variables
	s_BT_QuadTree* QuadTree;
	unsigned char Levels;

//Calculate levels
	unsigned char i=1;
	while(((Terrain->LODLevel[0].Split>>i)&0x1)==NULL)
		i++;

	Levels=i;
	Terrain->QuadTreeLevels=Levels;

//Allocate quadtree
	QuadTree=BT_Intern_AllocateQuadTreeRec(Terrain,Levels,NULL,0,0);

	return QuadTree;
}
// === END FUNCTION ===



// =======================================
// === BT INTERN ALLOCATE QUADTREE REC ===
// =======================================
static s_BT_QuadTree* BT_Intern_AllocateQuadTreeRec(s_BT_terrain* Terrain,unsigned char Levels,s_BT_QuadTree* Parent,unsigned char row,unsigned char collumn)
{
//Variables
	s_BT_QuadTree* Quadtree;
	bool QHasSector=false;

//Allocate Quadtree
	Quadtree=(s_BT_QuadTree*)malloc(sizeof(s_BT_QuadTree));
	if(Quadtree==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);
	memset ( Quadtree, 0, sizeof(s_BT_QuadTree) );

//Set values
	Quadtree->Level=Levels;
	Quadtree->row=row;
	Quadtree->collumn=collumn;
	Quadtree->Parent=Parent;
	Quadtree->Excluded=false;
	Quadtree->Culled=false;

//Allocate cullbox
	Quadtree->CullBox=(s_BT_CullBox*)malloc(sizeof(s_BT_CullBox));
	if(Quadtree->CullBox==nullptr)
		BT_Intern_Error(C_BT_ERROR_MEMORYERROR);

//Check if this LOD level exists
	if(Levels<Terrain->LODLevels)
	{
	//Find sector ID
		Quadtree->Sector=&Terrain->LODLevel[Levels].Sector[row+collumn*Terrain->LODLevel[Levels].Split];
		Quadtree->Sector->QuadTree=Quadtree;

	//Set cullbox values
		if(Quadtree->Sector->Excluded==false)
		{
			Quadtree->CullBox->Left=float(Quadtree->Sector->Pos_x-Terrain->LODLevel[Levels].SectorSize/2.0);
			Quadtree->CullBox->Right=float(Quadtree->Sector->Pos_x+Terrain->LODLevel[Levels].SectorSize/2.0);
			Quadtree->CullBox->Front=float(Quadtree->Sector->Pos_z-Terrain->LODLevel[Levels].SectorSize/2.0);
			Quadtree->CullBox->Back=float(Quadtree->Sector->Pos_z+Terrain->LODLevel[Levels].SectorSize/2.0);
			Quadtree->CullBox->Top=Quadtree->Sector->Pos_y+Quadtree->Sector->QuadMap->GetHighestPoint();
			Quadtree->CullBox->Bottom=Quadtree->Sector->Pos_y+Quadtree->Sector->QuadMap->GetLowestPoint();
		}
		QHasSector=true;

		Quadtree->Excluded=Quadtree->Sector->Excluded;
	}else{
		Quadtree->Sector=NULL;
	}

//Allocate children (if any)
	if(Levels>0)
	{
		Levels--;
		Quadtree->n1=BT_Intern_AllocateQuadTreeRec(Terrain,Levels,Quadtree,row*2,collumn*2);
		Quadtree->n2=BT_Intern_AllocateQuadTreeRec(Terrain,Levels,Quadtree,row*2,collumn*2+1);
		Quadtree->n3=BT_Intern_AllocateQuadTreeRec(Terrain,Levels,Quadtree,row*2+1,collumn*2);
		Quadtree->n4=BT_Intern_AllocateQuadTreeRec(Terrain,Levels,Quadtree,row*2+1,collumn*2+1);

		Quadtree->Excluded=Quadtree->n1->Excluded && Quadtree->n2->Excluded && Quadtree->n3->Excluded && Quadtree->n4->Excluded;

		if(QHasSector==false)
		{
			Quadtree->CullBox->Left=Quadtree->n1->CullBox->Left;
			Quadtree->CullBox->Right=Quadtree->n4->CullBox->Right;
			Quadtree->CullBox->Front=Quadtree->n1->CullBox->Front;
			Quadtree->CullBox->Back=Quadtree->n4->CullBox->Back;

			Quadtree->CullBox->Top=Quadtree->n1->CullBox->Top;
			if(Quadtree->n2->CullBox->Top>Quadtree->CullBox->Top)
				Quadtree->CullBox->Top=Quadtree->n2->CullBox->Top;
			if(Quadtree->n3->CullBox->Top>Quadtree->CullBox->Top)
				Quadtree->CullBox->Top=Quadtree->n3->CullBox->Top;
			if(Quadtree->n4->CullBox->Top>Quadtree->CullBox->Top)
				Quadtree->CullBox->Top=Quadtree->n4->CullBox->Top;

			Quadtree->CullBox->Bottom=Quadtree->n1->CullBox->Bottom;
			if(Quadtree->n2->CullBox->Bottom<Quadtree->CullBox->Bottom)
				Quadtree->CullBox->Bottom=Quadtree->n2->CullBox->Bottom;
			if(Quadtree->n3->CullBox->Bottom<Quadtree->CullBox->Bottom)
				Quadtree->CullBox->Bottom=Quadtree->n3->CullBox->Bottom;
			if(Quadtree->n4->CullBox->Bottom<Quadtree->CullBox->Bottom)
				Quadtree->CullBox->Bottom=Quadtree->n4->CullBox->Bottom;
		}
	}else{
		Quadtree->n1=NULL;
		Quadtree->n2=NULL;
		Quadtree->n3=NULL;
		Quadtree->n4=NULL;
	}

//Find position
	Quadtree->PosX=(Quadtree->CullBox->Left+Quadtree->CullBox->Right)/2.0f;
	Quadtree->PosY=(Quadtree->CullBox->Top+Quadtree->CullBox->Bottom)/2.0f;
	Quadtree->PosZ=(Quadtree->CullBox->Front+Quadtree->CullBox->Back)/2.0f;

//Return
	return Quadtree;
}
// === END FUNCTION ===

//LEFT = x-size
//RIGHT = x+size
//TOP = y+size
//BOTTOM = y-size
//FRONT = z-size
//BACK = z+size

// =====================================
// === BT INTERN DEALLOCATE QUADTREE ===
// =====================================
static void BT_Intern_DeAllocateQuadTree(s_BT_QuadTree* Quadtree)
{
//DeAllocate quadtree
	BT_Intern_DeAllocateQuadTreeRec(Quadtree);
	free(Quadtree);
}
// === END FUNCTION ===



// =========================================
// === BT INTERN DEALLOCATE QUADTREE REC ===
// =========================================
static void BT_Intern_DeAllocateQuadTreeRec(s_BT_QuadTree* Quadtree)
{
//Deallocate children
	if(Quadtree->n1!=NULL){
		BT_Intern_DeAllocateQuadTreeRec(Quadtree->n1);
		free(Quadtree->n1);
	}

	if(Quadtree->n2!=NULL){
		BT_Intern_DeAllocateQuadTreeRec(Quadtree->n2);
		free(Quadtree->n2);
	}

	if(Quadtree->n3!=NULL){
		BT_Intern_DeAllocateQuadTreeRec(Quadtree->n3);
		free(Quadtree->n3);
	}

	if(Quadtree->n4!=NULL){
		BT_Intern_DeAllocateQuadTreeRec(Quadtree->n4);
#pragma message("TEMPORARY")
		//free(Quadtree->n4);
	}

	if(Quadtree->CullBox!=NULL)
		free(Quadtree->CullBox);
}
// === END FUNCTION ===



void BT_Intern_RefreshVB(s_BT_DrawBuffer* DrawBuffer,unsigned long FirstVertex,unsigned long LastVertex,BT_Meshdata_Vertex* Vertex)
{
//Variables

	//FVF size
	unsigned long FVFSize;

//Get variables from drawbuffer
	FVFSize=DrawBuffer->FVF_Size;

//Lock vertexdata
#ifdef DX11
	// lock and write to vertex buffer in DX11
	if ( 0 ) 
	{
		// if VB is dynamic
		D3D11_MAPPED_SUBRESOURCE resource;
		std::memset ( &resource, 0, sizeof ( resource ) );
		if ( FAILED ( m_pImmediateContext->Map ( DrawBuffer->VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) ) return;
		BT_Meshdata_Vertex* LockedVertex = (BT_Meshdata_Vertex*)resource.pData;
		std::memcpy(LockedVertex,Vertex+FirstVertex*FVFSize,(LastVertex-FirstVertex)*FVFSize);
		m_pImmediateContext->Unmap ( DrawBuffer->VertexBuffer, 0 );
	}
	else
	{
		// if VB is default (faster)
		D3D11_BOX box;
		box.left = FirstVertex*FVFSize;
		box.right = (FirstVertex*FVFSize) + ( (LastVertex-FirstVertex)*FVFSize );
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		m_pImmediateContext->UpdateSubresource ( DrawBuffer->VertexBuffer, 0, &box, Vertex, 0, 0 );
	}
#else
	//Locked vertex list
	BT_Meshdata_Vertex* LockedVertex;
	DrawBuffer->VertexBuffer->Lock(FirstVertex*FVFSize,(LastVertex-FirstVertex)*FVFSize,(void**)&LockedVertex,NULL);

//Copy vertexdata
	memcpy(LockedVertex,Vertex+FirstVertex*FVFSize,(LastVertex-FirstVertex)*FVFSize);

//Unlock vertexdata
	DrawBuffer->VertexBuffer->Unlock();
#endif
}

void BT_Intern_RefreshIB(s_BT_DrawBuffer* DrawBuffer,unsigned long FirstIndex,unsigned long LastIndex,unsigned short* Index)
{
#ifdef DX11
	// lock index buffer and write to it
	if ( 0 )
	{
		// if IB is dynamic
		D3D11_MAPPED_SUBRESOURCE resource;
		std::memset ( &resource, 0, sizeof ( resource ) );
		if ( FAILED ( m_pImmediateContext->Map ( DrawBuffer->IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) ) return;
		unsigned short* LockedIndex = (unsigned short*)resource.pData;
		std::memcpy(LockedIndex,Index+FirstIndex*sizeof(unsigned short),(LastIndex-FirstIndex)*sizeof(unsigned short));
		m_pImmediateContext->Unmap ( DrawBuffer->IndexBuffer, 0 );
	}
	else
	{
		// if IB is default (faster)
		D3D11_BOX box;
		box.left = FirstIndex*sizeof(unsigned short);
		box.right = (FirstIndex*sizeof(unsigned short)) + ( (LastIndex-FirstIndex)*sizeof(unsigned short) );
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		m_pImmediateContext->UpdateSubresource ( DrawBuffer->IndexBuffer, 0, &box, Index, 0, 0 );
	}
#else
	//Locked index list
	unsigned short* LockedIndex;
//Lock indexdata
	DrawBuffer->IndexBuffer->Lock(FirstIndex*sizeof(unsigned short),(LastIndex-FirstIndex)*sizeof(unsigned short),(void**)&LockedIndex,NULL);

//Copy indexdata
	memcpy(LockedIndex,Index+FirstIndex*sizeof(unsigned short),(LastIndex-FirstIndex)*sizeof(unsigned short));

//Unlock indexdata
	DrawBuffer->IndexBuffer->Unlock();
#endif
}



static void BT_Intern_ExtractFrustum()
{
//Variables
	GGMATRIX ClipMatrix;

//Multiply view and projection matrices together
	GGMatrixMultiply(&ClipMatrix,&BT_Main.CurrentUpdateCamera->matView,&BT_Main.CurrentUpdateCamera->matProjection);

//Get right plane
	BT_Main.Frustum[0][0]=ClipMatrix._14-ClipMatrix._11;
	BT_Main.Frustum[0][1]=ClipMatrix._24-ClipMatrix._21;
	BT_Main.Frustum[0][2]=ClipMatrix._34-ClipMatrix._31;
	BT_Main.Frustum[0][3]=ClipMatrix._44-ClipMatrix._41;

//Get left plane
	BT_Main.Frustum[1][0]=ClipMatrix._14+ClipMatrix._11;
	BT_Main.Frustum[1][1]=ClipMatrix._24+ClipMatrix._21;
	BT_Main.Frustum[1][2]=ClipMatrix._34+ClipMatrix._31;
	BT_Main.Frustum[1][3]=ClipMatrix._44+ClipMatrix._41;

//Get bottom plane
	BT_Main.Frustum[2][0]=ClipMatrix._14+ClipMatrix._12;
	BT_Main.Frustum[2][1]=ClipMatrix._24+ClipMatrix._22;
	BT_Main.Frustum[2][2]=ClipMatrix._34+ClipMatrix._32;
	BT_Main.Frustum[2][3]=ClipMatrix._44+ClipMatrix._42;

//Get top plane
	BT_Main.Frustum[3][0]=ClipMatrix._14-ClipMatrix._12;
	BT_Main.Frustum[3][1]=ClipMatrix._24-ClipMatrix._22;
	BT_Main.Frustum[3][2]=ClipMatrix._34-ClipMatrix._32;
	BT_Main.Frustum[3][3]=ClipMatrix._44-ClipMatrix._42;

//Get far plane
	BT_Main.Frustum[4][0]=ClipMatrix._14-ClipMatrix._13;
	BT_Main.Frustum[4][1]=ClipMatrix._24-ClipMatrix._23;
	BT_Main.Frustum[4][2]=ClipMatrix._34-ClipMatrix._33;
	BT_Main.Frustum[4][3]=ClipMatrix._44-ClipMatrix._43;

}


//LEFT = x-size
//RIGHT = x+size
//TOP = y+size
//BOTTOM = y-size
//FRONT = z-size
//BACK = z+size


//Returns 0 if box isnt in frustum, 1 if intersecting and 2 if completely in frustum
static int BT_Intern_CullBox(s_BT_CullBox* pCullBox)
{
//Increase cull checks
	BT_Main.CullChecks++;

//Subtract Cull offset
	s_BT_CullBox CullBox;
	CullBox.Left=pCullBox->Left*BT_Main.CullScale.x-BT_Main.CullOffset.x;
	CullBox.Right=pCullBox->Right*BT_Main.CullScale.x-BT_Main.CullOffset.x;
	CullBox.Top=pCullBox->Top*BT_Main.CullScale.y-BT_Main.CullOffset.y;
	CullBox.Bottom=pCullBox->Bottom*BT_Main.CullScale.y-BT_Main.CullOffset.y;
	CullBox.Front=pCullBox->Front*BT_Main.CullScale.z-BT_Main.CullOffset.z;
	CullBox.Back=pCullBox->Back*BT_Main.CullScale.z-BT_Main.CullOffset.z;

//Variables
	int p;
	int c;
	int c2 = 0;

//Loop
	for( p = 0; p < 5; p++ )
	{
		c = 0;
		if( BT_Main.Frustum[p][0] * CullBox.Left + BT_Main.Frustum[p][1] * CullBox.Bottom + BT_Main.Frustum[p][2] * CullBox.Front + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( BT_Main.Frustum[p][0] * CullBox.Right + BT_Main.Frustum[p][1] * CullBox.Bottom + BT_Main.Frustum[p][2] * CullBox.Front + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( BT_Main.Frustum[p][0] * CullBox.Left + BT_Main.Frustum[p][1] * CullBox.Top + BT_Main.Frustum[p][2] * CullBox.Front + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( BT_Main.Frustum[p][0] * CullBox.Right + BT_Main.Frustum[p][1] * CullBox.Top + BT_Main.Frustum[p][2] * CullBox.Front + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( BT_Main.Frustum[p][0] * CullBox.Left + BT_Main.Frustum[p][1] * CullBox.Bottom + BT_Main.Frustum[p][2] * CullBox.Back + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( BT_Main.Frustum[p][0] * CullBox.Right + BT_Main.Frustum[p][1] * CullBox.Bottom + BT_Main.Frustum[p][2] * CullBox.Back + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( BT_Main.Frustum[p][0] * CullBox.Left + BT_Main.Frustum[p][1] * CullBox.Top + BT_Main.Frustum[p][2] * CullBox.Back + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( BT_Main.Frustum[p][0] * CullBox.Right + BT_Main.Frustum[p][1] * CullBox.Top + BT_Main.Frustum[p][2] * CullBox.Back + BT_Main.Frustum[p][3] > 0 )
			c++;
		if( c == 0 )
			return 0;
		if( c == 8 )
			c2++;
	}
	return (c2 == 4) ? 2 : 1;
}



// =========================================
// === BT INTERN LOCK SECTOR VERTEX DATA ===
// =========================================
static BT_RTTMS_STRUCT* BT_Intern_LockSectorVertexData(s_BT_Sector* Sector)
{
//Lock sector
	return (BT_RTTMS_STRUCT*)BT_RTTMS_LockSectorVertexData(Sector->Terrain->ID,Sector->LODLevel->ID,Sector->ID);
}
// === END FUNCTION ===



// ===========================================
// === BT INTERN UNLOCK SECTOR VERTEX DATA ===
// ===========================================
static void BT_Intern_UnlockSectorVertexData(s_BT_Sector* Sector)
{
//Unlock vertexdata
	if(Sector->VertexDataLocked==true)
	{
		BT_RTTMS_UnlockSectorVertexData((void*)Sector->VertexDataRTTMS);
	}
}
// === END FUNCTION ===


// ======================================
// === BT INTERN RTTMS UPDATE HANDLER ===
// ======================================
void BT_Intern_RTTMSUpdateHandler(unsigned long TerrainID,unsigned long LODLevelID,unsigned long SectorID,unsigned short StartVertex,unsigned short EndVertex,float* VerticesPtr)
{
#ifdef C_BT_FULLVERSION
//Cast vertices
	float* Vertices=(float*)VerticesPtr;

//Get sector
	s_BT_Sector* Sector=&BT_Main.Terrains[TerrainID].LODLevel[LODLevelID].Sector[SectorID];

//Change mesh data
	if(EndVertex>0)
	{
	//Update meshdata
		Sector->QuadMap->ChangeMeshData(StartVertex,EndVertex,Vertices);
		Sector->UpdateMesh=true;

	//Force update of edges
		Sector->LeftSideNeedsUpdate=true;
		Sector->RightSideNeedsUpdate=true;
		Sector->TopSideNeedsUpdate=true;
		Sector->BottomSideNeedsUpdate=true;

	//Update collision
		if(Sector->DBPObject!=0){
			Sector->QuadMap->UpdateDBPMesh(Sector->DBPObject->pFrame->pMesh);
		}
		if(Sector->LODLevel->DBPObject!=0){
		//Find the sector
			if ( Sector->LODLevelObjectFrame )
				Sector->QuadMap->UpdateDBPMesh(Sector->LODLevelObjectFrame->pMesh);
		}

	//Say that the cull box has changed
		s_BT_QuadTree* QuadTree=Sector->QuadTree;
		do{
			QuadTree->CullboxChanged=true;
			QuadTree=QuadTree->Parent;
		}while(QuadTree!=NULL);
	}
#endif
}
// === END FUNCTION ===



// ========================================
// === BT INTERN GET HEIGHT FROM COLOUR ===
// ========================================
static float BT_Intern_GetHeightFromColor(unsigned long Colour)
{
	GGCOLOR D3DColour = GGCOLOR(Colour);
#ifdef C_BT_FULLVERSION
	return D3DColour.r * 256.0f + D3DColour.g + D3DColour.b / 256.0f;
#else
	return D3DColour.r * 256.0f;
#endif
}
// === END FUNCTION ===



// =========================
// === BT SMOOTH TERRAIN ===
// =========================
static void BT_Intern_SmoothTerrain(s_BT_terrain* Terrain)
{
//Loop through smooth levels
	for(unsigned char SmoothLevel=0;SmoothLevel<Terrain->Smoothing;SmoothLevel++){
		for(unsigned short y=1;y<Terrain->Heightmapsize-1;y++){
			for(unsigned short x=1;x<Terrain->Heightmapsize-1;x++){
				float CornA=Terrain->HeightPoint[(x-1)+(y-1)*Terrain->Heightmapsize];
				float CornB=Terrain->HeightPoint[(x+1)+(y-1)*Terrain->Heightmapsize];
				float CornC=Terrain->HeightPoint[(x-1)+(y+1)*Terrain->Heightmapsize];
				float CornD=Terrain->HeightPoint[(x+1)+(y+1)*Terrain->Heightmapsize];
				float NexA=Terrain->HeightPoint[(x-1)+y*Terrain->Heightmapsize];
				float NexB=Terrain->HeightPoint[(x+1)+y*Terrain->Heightmapsize];
				float NexC=Terrain->HeightPoint[x+(y-1)*Terrain->Heightmapsize];
				float NexD=Terrain->HeightPoint[x+(y+1)*Terrain->Heightmapsize];
				float Middle=Terrain->HeightPoint[x+y*Terrain->Heightmapsize];
				float CornerAverage=(CornA+CornB+CornC+CornD)/4.0f;
				float NeighborAverage=(CornerAverage+NexA+NexB+NexC+NexD)/5.0f;
				Terrain->HeightPoint[x+y*Terrain->Heightmapsize]=(Middle+NeighborAverage)/2.0f;
			}
		}
	}
}
// === END FUNCTION ===
