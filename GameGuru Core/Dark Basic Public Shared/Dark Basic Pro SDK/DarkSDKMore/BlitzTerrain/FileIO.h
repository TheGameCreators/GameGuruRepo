#ifndef _FILEIO_H
#define _FILEIO_H

#include "main.h"


#define C_FILE_FILESYSTEMVERSION 1     //Filesystem version
#define C_FILE_MINBTREADVERSION 200    //Minimum version of blitzterrain needed to read the file

struct s_BT_FileTerrainHeader100
{
	unsigned long Flags;
	unsigned char LODLevels;
	float Scale;
	float YScale;
	float Tile;
	unsigned short Sectors;
	unsigned short HeightmapSize;
	float TerrainSize;
	unsigned char ExclusionThreshold;
	unsigned long FVF;
	unsigned char ATMode;
	unsigned char DetailBlendMode;
};

struct s_BT_FileLODLevelHeader100
{
	float SectorSize;
	unsigned char SectorDetail;
	unsigned short Sectors;
	float Distance;
	unsigned short Split;
	unsigned char TileSpan;
};

struct s_BT_FileSectorHeader100
{
	bool Excluded;
	unsigned short Row;
	unsigned short Column;
	float PosX;
	float PosY;
	float PosZ;
};

struct s_BT_FileTerrainHeader101
{
	unsigned long Flags;
	unsigned char LODLevels;
	float Scale;
	float YScale;
	float Tile;
	unsigned short Sectors;
	unsigned short HeightmapSize;
	float TerrainSize;
	unsigned char ExclusionThreshold;
	unsigned long FVF;
	unsigned char ATMode;
	unsigned char DetailBlendMode;
};

struct s_BT_FileLODLevelHeader101
{
	float SectorSize;
	unsigned char SectorDetail;
	unsigned short Sectors;
	float Distance;
	unsigned short Split;
	unsigned char TileSpan;
};

struct s_BT_FileSectorHeader101
{
	bool Excluded;
	unsigned short Row;
	unsigned short Column;
};

EXPORT unsigned long BT_LoadTerrain(char* FileName,unsigned long ObjectID);
EXPORT unsigned long BT_LoadTerrain(char* FileName,unsigned long ObjectID,bool GenerateTerrain);
EXPORT void BT_SaveTerrain(unsigned long TerrainID,char* FileName);

unsigned long LoadTerrain100(ifstream *File,unsigned long ObjectID,bool GenerateTerrain);
s_BT_FileTerrainHeader100 *BT_Intern_MakeDefaultTerrainHeader100();
void BT_Intern_FillTerrainHeader100(s_BT_FileTerrainHeader100* Header,s_BT_terrain* Terrain);
void BT_Intern_FillTerrainHeaderDefault100(s_BT_FileTerrainHeader100* Header);
s_BT_FileLODLevelHeader100 *BT_Intern_MakeDefaultLODLevelHeader100();
void BT_Intern_FillLODLevelHeader100(s_BT_FileLODLevelHeader100* Header,s_BT_LODLevel* LODLevel);
void BT_Intern_FillLODLevelHeaderDefault100(s_BT_FileLODLevelHeader100* Header);
s_BT_FileSectorHeader100 *BT_Intern_MakeDefaultSectorHeader100();
void BT_Intern_FillSectorHeader100(s_BT_FileSectorHeader100* Header,s_BT_Sector* Sector);
void BT_Intern_FillSectorHeaderDefault100(s_BT_FileSectorHeader100* Header);

unsigned long LoadTerrain101(ifstream *File,unsigned long ObjectID,bool GenerateTerrain);
s_BT_FileTerrainHeader101 *BT_Intern_MakeDefaultTerrainHeader101();
void BT_Intern_FillTerrainHeader101(s_BT_FileTerrainHeader101* Header,s_BT_terrain* Terrain);
void BT_Intern_FillTerrainHeaderDefault101(s_BT_FileTerrainHeader101* Header);
s_BT_FileLODLevelHeader101 *BT_Intern_MakeDefaultLODLevelHeader101();
void BT_Intern_FillLODLevelHeader101(s_BT_FileLODLevelHeader101* Header,s_BT_LODLevel* LODLevel);
void BT_Intern_FillLODLevelHeaderDefault101(s_BT_FileLODLevelHeader101* Header);
s_BT_FileSectorHeader101 *BT_Intern_MakeDefaultSectorHeader101();
void BT_Intern_FillSectorHeader101(s_BT_FileSectorHeader101* Header,s_BT_Sector* Sector);
void BT_Intern_FillSectorHeaderDefault101(s_BT_FileSectorHeader101* Header);


#endif