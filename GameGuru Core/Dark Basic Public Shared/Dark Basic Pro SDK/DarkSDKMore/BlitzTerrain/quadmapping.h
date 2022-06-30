#ifndef _QUADMAP_H
#define _QUADMAP_H

///struct GGVECTOR4;
struct s_BT_Sector;

struct BT_Quadmap_Generator
{
	bool QuadRotation;
	bool QuadReduction;
	float TileSize;
	unsigned char Size;
	float* heights;
	bool* exclusion;
	unsigned char TileSpan;
	bool Optimise;
	s_BT_Sector* Sector;
	unsigned long LODLevel;
	bool RemoveFarX;
	bool RemoveFarZ;
};

struct BT_Quadmap_Vertex
{
	//Exclude
	bool Exclude;

	//Position
	float Pos_y;
	unsigned char Vrow,Vcol;

	//Normals
	signed char Nrm_x, Nrm_y, Nrm_z;

	//Index
	unsigned short Index;
	BT_Quadmap_Vertex* NewPtr;

};

struct BT_Quadmap_Edge
{
	char Edge; //0 - top, 1 - right,2 - bottom,3 - left
	unsigned char Position;
	BT_Quadmap_Vertex* Vertex;
};

struct BT_Quadmap_Quad
{
	//Exclude
	bool Exclude;

	//Vertices
	BT_Quadmap_Vertex* V1;
	BT_Quadmap_Vertex* V2;
	BT_Quadmap_Vertex* V3;
	BT_Quadmap_Vertex* V4;

	//Size
	float Size;

	//Rotation
	bool Rotation;
};

struct BT_Meshdata_Vertex
{
	//Position
	float Pos_x,Pos_y,Pos_z;

	//Normals
	float Nrm_x,Nrm_y,Nrm_z;

	//UVs
	float U0,V0,U1,V1;
};

struct s_BT_DrawBuffer;
struct BT_SectorMeshData;

#include "rttms.h"
struct BT_RTTMS_STRUCT;
struct BT_RTTMS_VERTEX;

class BT_QuadMap
{
public:
	void Generate(BT_Quadmap_Generator Generator);
	void CalculateNormals();
	void CalculateBounds();
	void GenerateMeshData();
	void DeleteMeshData();
	void SetToMesh(s_BT_DrawBuffer* DrawBuffer);
	void UpdateMesh(s_BT_DrawBuffer* DrawBuffer,bool ClearUpdateInfo);
	unsigned long GetVertexCount() {return Mesh_Vertices;}
	unsigned long GetIndexCount() {return Mesh_Indices;}
	s_BT_DrawBuffer* GeneratePlain();
	void GenerateDBPMesh(sMesh* Mesh);
	void UpdateDBPMesh(sMesh* Mesh);
	bool GetPointExcluded(float x,float z);
	float GetPointHeight(float x,float y,bool Round);
	void SetSideLOD(unsigned char Side,unsigned long LODLevel);
	float GetHighestPoint() {return HighestPoint;}
	float GetLowestPoint() {return LowestPoint;}
	void FillMeshData(BT_RTTMS_STRUCT* Meshdata);
	void DeleteInternalData();
	void ReduceQuad(unsigned short QuadTL,unsigned short QuadTR,unsigned short QuadBL,unsigned short QuadBR,BT_Quadmap_Quad* Quads, bool CheckHeights);
	void ChangeMeshData(unsigned short VertexStart,unsigned short VertexEnd,float* Vertices);
	unsigned short FindVertex(unsigned short Vrow,unsigned short Vcol);

	BT_QuadMap* Above;
	BT_QuadMap* Below;
	BT_QuadMap* Left;
	BT_QuadMap* Right;

private:
	//Generated boolean
	bool Generated;

	//Vertex list
	BT_Quadmap_Vertex* Vertex;

	//Quad list
	BT_Quadmap_Quad* Quad;

	//QuadMap
	BT_Quadmap_Quad** QuadMap;

	//Mesh data
	bool MeshMade;
	unsigned short Mesh_Vertices;
	unsigned long Mesh_Indices;
	BT_Meshdata_Vertex* Mesh_Vertex;
	unsigned short* Mesh_Index;

	//Mesh updates
	bool UpdateVertices;
	bool UpdateIndices;
	bool RefreshNormals;

	//Buffer updates
	bool UpdateVertexBuffer;
	bool UpdateIndexBuffer;

	//Sizes
	unsigned short Vertices;
	unsigned short Quads;
	float Size;
	float TileSize;
	unsigned char TileSpan;
	unsigned char QuadsAccross;

	//Highest and lowest points
	float HighestPoint;
	float LowestPoint;

	//Optimisation
	bool Optimised;

	//Sector ID
	s_BT_Sector* Sector;
};

void BT_Intern_StartQuadMapGeneration(BT_Quadmap_Generator Generator);
void BT_Intern_EndQuadMapGeneration();

struct BT_QuadMap_Main
{
	bool Locked;
	BT_Quadmap_Vertex* TempVertexdata;
	BT_Quadmap_Quad* TempQuaddata;
	BT_Quadmap_Vertex*** TempVertexMap;
	unsigned long TempVertices;
	unsigned long TempQuads;
};

#endif