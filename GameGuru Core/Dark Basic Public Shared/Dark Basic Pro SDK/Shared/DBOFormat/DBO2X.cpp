#include "windows.h"
#include "stdio.h"
//#include "stdafx.h"
#include "DBOBlock.h"

#include "CFileC.h"

#define CONVERTTOX
#define RELEASEIT
#define MAX_STRING 512 // thin only max 256 is used.

char convertfrom[MAX_PATH];
char convertto[MAX_PATH];
char *tmpchr;
char *str_replace(char *search, char *replace, char *subject);
wchar_t * wcstring1 = new wchar_t[MAX_PATH * 4];
wchar_t * wcstring2 = new wchar_t[MAX_PATH * 4];
char currentDir[MAX_PATH];
bool GetFileExist(char * filename);
FILE *inputf;
DWORD frame_count = 0;
#define MAXFRAMES 4500

struct FrameStruct
{
	char Name[MAX_STRING];
} myFrame[MAXFRAMES];

int overWrite = false;
int removePlacementMarker = false;

bool GetFileExist(char * filename)
{
	FILE *file;
	file = GG_fopen(filename, "r");
	if (file == NULL) return false;
	fclose(file);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////
// DBORAWMESH HEADER ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MyReadOffsetListData(int** pwIndexData, DWORD dwIndexCount, LPSTR* ppBlock);
bool MyReadMemory(BYTE** ppData, DWORD dwBytes, LPSTR* ppBlock);
bool MyReadBOOL(bool* ppDest, LPSTR* ppBlock);
DWORD* ReadString(LPSTR pString, LPSTR ppBlock);
DWORD* ReadDWORD(DWORD* ppDest, DWORD* ppBlock);
DWORD* ReadCODE(DWORD* pdwCode, DWORD* pdwCodeSize, DWORD* ppBlock);
DWORD* ConstructFrame(LPSTR pFrameName, DWORD* ppBlock);
DWORD* ConstructMesh(LPSTR pFrameName, DWORD* ppBlock);
DWORD* ReadVertexData(BYTE* pbVertexData, DWORD dwFVFSize, DWORD dwVertexCount, DWORD* ppBlock);
DWORD* ReadIntoMemory(BYTE* pData, DWORD dwBytes, DWORD* ppBlock);
bool GetFVFValueOffsetMap(DWORD dwFVF);
bool GetFVFOffsetMap(DWORD dwFVF, DWORD dwFVFSize);
DWORD* ReadIndexData(DWORD* pwIndexData, DWORD dwIndexCount, DWORD* ppBlock);
DWORD pdwSize;
DWORD *pBlock;
DWORD *pLoadObject;
DWORD g_dwMyVersion;
DWORD Dummy;
char FrameName[MAX_STRING];
float matrix[32];
float vector[6];

#define MAX_FVF_DECL_SIZE 65

struct MYGGVERTEXELEMENT
{
	WORD    Stream;     // Stream index 2
	WORD    Offset;     // Offset in the stream in bytes 2
	BYTE    Type;       // Data type 1 , always 2 = float ?
	BYTE    Method;     // Processing method 1
	BYTE    Usage;      // Semantics 1, 0=Vertex,
	BYTE    UsageIndex; // Semantic index 1 
} MYGGVERTEXELEMENT[MAX_FVF_DECL_SIZE + 1]; // sizeof 8*65 = 520

struct sMyOffsetMap
{
	// stores a list of offsets to each part of fvf data
	DWORD   dwSize,            // size of data
		dwByteSize,         // size in bytes
		dwFVF,            // actual fvf
		dwX, dwY, dwZ, // vertex x,y,z
		dwRWH,            // rhw offset
		dwBlend1,         // blend 1 offset
		dwBlend2,         // blend 2 offset
		dwBlend3,         // blend 3 offset
		dwNX, dwNY, dwNZ, // normal x,y,z offset.
		dwPointSize,      // point size offset
		dwDiffuse,         // diffuse offset
		dwSpecular,         // specular offset
		dwTU[8],         // tu offsets
		dwTV[8],         // tv offsets
		dwTZ[8],         // tz offsets
		dwTW[8];         // tw offsets
} myOffsetMap;

bool g_bHasAnimation = false;

bool dbo2xConvert(LPSTR pFilefrom, LPSTR pFileto)
{	
	// some resets
	g_bHasAnimation = false;
	frame_count = 0;

	// write .x headers.
	inputf = GG_fopen(pFileto, "w");
	fprintf(inputf, "xof 0303txt 0032\n"
		"template XSkinMeshHeader {\n"
		" <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>\n"
		" WORD nMaxSkinWeightsPerVertex;\n"
		" WORD nMaxSkinWeightsPerFace;\n"
		" WORD nBones;\n"
		"}\n"
		"template VertexDuplicationIndices {\n"
		" <b8d65549-d7c9-4995-89cf-53a9a8b031e3>\n"
		" DWORD nIndices;\n"
		" DWORD nOriginalVertices;\n"
		" array DWORD indices[nIndices];\n"
		"}\n"

		"template SkinWeights {\n"
		"<6f0d123b-bad2-4167-a0d0-80224f25fabb>\n"
		"STRING transformNodeName;\n"
		" DWORD nWeights;\n"
		" array DWORD vertexIndices[nWeights];\n"
		" array FLOAT weights[nWeights];\n"
		" Matrix4x4 matrixOffset;\n"
		"}\n"

		"template AnimTicksPerSecond {\n"
		"	<9e415a43-7ba6-4a73-8743-b73d47e88476>\n"
		" DWORD AnimTicksPerSecond;\n"
		"}\n"
		"AnimTicksPerSecond {\n"
		" 24;\n"
		"}\n");

	DWORD dwVersion = 0;
	DWORD dwRes1 = 0, dwRes2 = 0;
	DWORD dwCode = 0, dwCodeSize = 0;
	bool bResult = true;
	char pMagicString[MAX_STRING];
	char *fileMem = NULL;
	// Load DBO
	HANDLE hfile = GG_CreateFile(pFilefrom, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		DWORD bytesread = 0;
		pdwSize = GetFileSize(hfile, NULL);
		fileMem = new char[pdwSize + 10];
		pBlock = (DWORD *)fileMem;
		if (fileMem != NULL)
			ReadFile(hfile, (LPSTR)(pBlock), pdwSize, &bytesread, NULL);
		CloseHandle(hfile);
	}
	else
	{
		// could not load file
		printf("File not found.\n");
		return false;
	}

	if (fileMem != NULL) {
		pBlock = ReadString(pMagicString, (LPSTR)pBlock);
		if (_stricmp(pMagicString, "MAGICDBO") == NULL)
		{
			pBlock = ReadDWORD(&dwVersion, pBlock);
			pBlock = ReadDWORD(&dwRes1, pBlock);
			pBlock = ReadDWORD(&dwRes2, pBlock);
			g_dwMyVersion = dwVersion;
			// read frame data
			pBlock = ReadCODE(&dwCode, &dwCodeSize, pBlock);
			sprintf(FrameName, "FirstFrame");
			pBlock = ConstructFrame(FrameName, pBlock);
		}
		else
		{
			printf("Missing MAGICDBO in dbo file.");
		}
	}
	else {
		printf("could not allocate memory to fit file - size:%d.", pdwSize);
	}

	// free used memory.
	if (fileMem != NULL) 
	{
		delete(fileMem);
		fileMem = NULL;
	}

	// close X file
	fclose(inputf);

	// if was flagged as having animation, delete it (not appropriate)
	if (g_bHasAnimation == true)
	{
		if (GetFileExist(pFileto) == true)
		{
			DeleteFile(pFileto);
			bResult = false;
		}
	}

	// complete
	return bResult;
}


DWORD vertexCount;
DWORD indexCount;
DWORD FVFSize;
DWORD dwFVF;
FLOAT *vertexData;
DWORD *indexData;
DWORD dwDummy;
BYTE bDummy;

struct VertexInfo {
	float vertexX;
	float vertexY;
	float vertexZ;
	float normalX;
	float normalY;
	float normalZ;
	float uV;
	float uU;
} *pVertexInfo, mVertexInfo;

DWORD* ReadVertexData(BYTE* pbVertexData, DWORD dwFVFSize, DWORD dwVertexCount, DWORD* ppBlock)
{
	DWORD dwLength = dwFVFSize * dwVertexCount;
	if (dwLength > 0)
	{
		//(*pbVertexData) = (BYTE*)new char[dwLength];
		memcpy((pbVertexData), ppBlock, dwLength);
	}
	return (DWORD*)((char *)ppBlock + dwLength);
}

DWORD* ConstructMesh(LPSTR pFrameName, DWORD* ppBlock)
{
	// get code
	DWORD dwCode = 0;
	DWORD dwCodeSize = 0;
	DWORD dwVdatasize;
	DWORD indexSize;
	char debugtmp[512];
	size_t convertedChars = 0;
	int addpos;
	char *indexMem = NULL, *vertexMem = NULL;

	strcpy(myFrame[frame_count].Name, pFrameName);

	ppBlock = ReadCODE(&dwCode, &dwCodeSize, ppBlock);

	while (dwCode > 0 && dwCodeSize < 8500000 )
	{
		switch (dwCode)
		{
			// Read vertex information
		case DBOBLOCK_MESH_FVF:			ppBlock = ReadDWORD(&dwFVF, ppBlock);													break;
		case DBOBLOCK_MESH_FVFSIZE:		ppBlock = ReadDWORD(&FVFSize, ppBlock);													break;
		case DBOBLOCK_MESH_VERTEXCOUNT:	ppBlock = ReadDWORD(&vertexCount, ppBlock);													break;
		case DBOBLOCK_MESH_INDEXCOUNT:		ppBlock = ReadDWORD(&indexCount, ppBlock);													break;

		case DBOBLOCK_MESH_VERTEXDEC:
			//Dont include a total data size but always 0x4 , so it must be processed.
			// GGVERTEXELEMENT               pVertexDeclaration [ MAX_FVF_DECL_SIZE ];
			//#define MAX_FVF_DECL_SIZE 65
			//
			//typedef struct _GGVERTEXELEMENT
			//{
			//	WORD    Stream;     // Stream index 2
			//	WORD    Offset;     // Offset in the stream in bytes 2
			//	BYTE    Type;       // Data type 1 , always 2 = float ? , GGDECLTYPE_FLOAT2 , GGDECLTYPE_FLOAT3 , GGDECLTYPE_FLOAT4
			//	BYTE    Method;     // Processing method 1
			// GGDECLUSAGE_POSITIONT include dwRWH 4 floats.
			//	BYTE    Usage;      // Semantics 1, 0=Vertex, GGDECLUSAGE_POSITION ,GGDECLUSAGE_NORMAL ,GGDECLUSAGE_TEXCOORD ,( GGDECLUSAGE_TANGENT,GGDECLUSAGE_BINORMAL not needed)
			//	BYTE    UsageIndex; // Semantic index 1 
			//} GGVERTEXELEMENT; // sizeof 8*65 = 520
			//Stream = 0xff = end/last.
			//
			//ppBlock = (DWORD*)((char *)ppBlock + 520);
			// MAX_FVF_DECL_SIZE * sizeof(_GGVERTEXELEMENT) . not sure if this will work when 64bit.
			ppBlock = ReadIntoMemory((BYTE*)&MYGGVERTEXELEMENT[0], MAX_FVF_DECL_SIZE * 8, ppBlock);
			break;

			//		case DBOBLOCK_MESH_SUBFRAMES:
			//		{
			//			ReadDWORD((DWORD*)&(*ppMesh)->dwSubMeshListCount, ppBlock);
			//			if ((*ppMesh)->dwSubMeshListCount > 0)
			//			{
			//				(*ppMesh)->pSubFrameList = new sFrame[(*ppMesh)->dwSubMeshListCount];
			//				for (DWORD dwSubMesh = 0; dwSubMesh < (*ppMesh)->dwSubMeshListCount; dwSubMesh++)
			//				{
			//					sFrame* pFrames = new sFrame;
			//					ppBlock = ConstructFrame(&pFrames, ppBlock);
			//					(*ppMesh)->pSubFrameList[dwSubMesh] = *pFrames;
			//				}
			//			}
			//		}
			//		break;

		case DBOBLOCK_MESH_VERTEXDATA:
			dwVdatasize = FVFSize * vertexCount;
			//vertexData = (FLOAT *) new char[dwVdatasize *4]; // float.
			vertexMem = new char[(dwVdatasize * 4) + 10];
			vertexData = (FLOAT *)vertexMem;
			if (vertexMem == NULL) exit(1);

			ppBlock = ReadVertexData((BYTE *)vertexData, FVFSize, vertexCount, ppBlock);
			//debug.
			//pFrameName
			//Get offsets.
			// Parse dwFVF for included data flags. Missing all defines.

			GetFVFOffsetMap(dwFVF, FVFSize);

			//GGDECLUSAGE_TEXCOORD is not always in the dbo but stores under GGDECLUSAGE_TANGENT ?
			if (myOffsetMap.dwTU[0] == 0 && myOffsetMap.dwTU[1] != 0) myOffsetMap.dwTU[0] = myOffsetMap.dwTU[1];
			if (myOffsetMap.dwTV[0] == 0 && myOffsetMap.dwTU[1] != 0) myOffsetMap.dwTV[0] = myOffsetMap.dwTU[1];

#ifdef CONVERTTOOBJ
			inputf = GG_fopen(convertto, "w");
			fprintf(inputf, "# This file was generated using GameGuru Loader - GG Loader dbo to obj converter.\n\n");
			fprintf(inputf, "o %s\n", pFrameName);
			fprintf(inputf, "s1\n"); // smooth shading.

									 //Write V vertex data to obj.
			for (int a = 0; a < vertexCount; a++) {
				addpos = (FVFSize * a) + (myOffsetMap.dwX * sizeof(DWORD)); // missing GGDECLUSAGE_POSITION , guess GGDECLUSAGE_POSITION=0
				mVertexInfo.vertexX = *(FLOAT *)((BYTE *)vertexData + addpos);
				mVertexInfo.vertexY = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
				mVertexInfo.vertexZ = *(FLOAT *)((BYTE *)vertexData + 8 + addpos);
				sprintf(debugtmp, "v %f %f %f\n", mVertexInfo.vertexX, mVertexInfo.vertexY, mVertexInfo.vertexZ);
				fprintf(inputf, "%s", debugtmp);
			}
			//Write VT texturecords.
			for (int a = 0; a < vertexCount; a++) {
				int uvOffset = myOffsetMap.dwTU[0] * sizeof(DWORD);; // missing GGDECLUSAGE_TEXCOORD ... so guess for now. GGDECLUSAGE_TEXCOORD=6
				addpos = FVFSize * a + uvOffset;
				mVertexInfo.uU = *(FLOAT *)((BYTE *)vertexData + addpos);
				mVertexInfo.uV = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
				sprintf(debugtmp, "vt %f %f\n", mVertexInfo.uU, mVertexInfo.uV);
				fprintf(inputf, "%s", debugtmp);

			}
			//Write VN normals
			for (int a = 0; a < vertexCount; a++) {

				int normalOffset = myOffsetMap.dwNX * sizeof(DWORD); // missing GGDECLUSAGE_NORMAL ... so guess for now GGDECLUSAGE_NORMAL=3.
				addpos = FVFSize * a + normalOffset;
				mVertexInfo.normalX = *(FLOAT *)((BYTE *)vertexData + addpos);
				mVertexInfo.normalY = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
				mVertexInfo.normalZ = *(FLOAT *)((BYTE *)vertexData + 8 + addpos);

				sprintf(debugtmp, "vn %f %f %f\n", mVertexInfo.normalX, mVertexInfo.normalY, mVertexInfo.normalZ);
				fprintf(inputf, "%s", debugtmp);
			}
			fclose(inputf);
#endif
			/*
			//TEST .obj write.
			for (int a = 0; a < vertexCount; a++) {
			addpos = (FVFSize * a)+ (myOffsetMap.dwX * sizeof(DWORD)); // missing GGDECLUSAGE_POSITION , guess GGDECLUSAGE_POSITION=0
			mVertexInfo.vertexX = *(FLOAT *)((BYTE *)vertexData + addpos);
			mVertexInfo.vertexY = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
			mVertexInfo.vertexZ = *(FLOAT *)((BYTE *)vertexData + 8 + addpos);

			int normalOffset = myOffsetMap.dwNX * sizeof(DWORD); // missing GGDECLUSAGE_NORMAL ... so guess for now GGDECLUSAGE_NORMAL=3.
			addpos = FVFSize * a + normalOffset;
			mVertexInfo.normalX = *(FLOAT *)((BYTE *)vertexData + addpos);
			mVertexInfo.normalY = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
			mVertexInfo.normalZ = *(FLOAT *)((BYTE *)vertexData + 8 + addpos);

			int uvOffset = myOffsetMap.dwTU[0] * sizeof(DWORD);; // missing GGDECLUSAGE_TEXCOORD ... so guess for now. GGDECLUSAGE_TEXCOORD=6
			addpos = FVFSize * a + uvOffset;
			mVertexInfo.uU = *(FLOAT *)((BYTE *)vertexData + addpos);
			mVertexInfo.uV = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);

			//missing 24 bytes , USAGE=7 , data: (1,0,0 - 0,0,1) guess this is a combined GGDECLUSAGE_TANGENT,GGDECLUSAGE_BINORMAL ?

			sprintf(debugtmp, "vertexdata%d - f: %s - vd: %f,%f,%f nd: %f, %f, %f UV: %f,%f\n", a, pFrameName, mVertexInfo.vertexX, mVertexInfo.vertexY, mVertexInfo.vertexZ, mVertexInfo.normalX, mVertexInfo.normalY, mVertexInfo.normalZ, mVertexInfo.uU, mVertexInfo.uV);
			}
			//			sprintf(debugtmp, "vertexdata - f: %s", pFrameName);
			mbstowcs_s(&convertedChars, wcstring1, MAX_STRING * 4, debugtmp, _TRUNCATE);
			//			MessageBoxW(NULL, wcstring1, L"TestConvert", MB_OK);
			*/

			break;

		case DBOBLOCK_MESH_INDEXDATA:
		{
			indexSize = sizeof(WORD) * indexCount;
			//indexData = (DWORD *) new char[indexSize]; // float.
			indexMem = new char[indexSize + 10];
			indexData = (DWORD *)indexMem;
			if (indexMem == NULL) exit(1);

			ppBlock = ReadIndexData(indexData, indexCount, ppBlock);
			if (indexCount == 0)
				*ppBlock += dwCodeSize;

			WORD* pIndexArray = (WORD*)indexData;
			int split = 1;

#ifdef CONVERTTOX

			//sprintf(convertto, "%ld-%s.x", frame_count, myFrame[frame_count].Name); // TEST split into differet .x per mesh group.
			
			if (!(removePlacementMarker && frame_count == 0)) 
			{
				if (removePlacementMarker) {
					if (frame_count == 1) {
						fprintf(inputf, "Mesh Group {\n");
					}
					else
					{
						fprintf(inputf, "Mesh Group%ld {\n", frame_count-1);
					}
				}
				else {

					if (frame_count == 0) {
						fprintf(inputf, "Mesh Group {\n");
					}
					else
					{
						fprintf(inputf, "Mesh Group%ld {\n", frame_count);
					}
				}

				fprintf(inputf, " %d;\n", vertexCount);
				//Write V vertex data to x.
				for (int a = 0; a < vertexCount; a++) {
					addpos = (FVFSize * a) + (myOffsetMap.dwX * sizeof(DWORD)); // missing GGDECLUSAGE_POSITION , guess GGDECLUSAGE_POSITION=0
					mVertexInfo.vertexX = *(FLOAT *)((BYTE *)vertexData + addpos);
					mVertexInfo.vertexY = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
					mVertexInfo.vertexZ = *(FLOAT *)((BYTE *)vertexData + 8 + addpos);
					if (a < vertexCount - 1)
						sprintf(debugtmp, " %f;%f;%f;,\n", mVertexInfo.vertexX, mVertexInfo.vertexY, mVertexInfo.vertexZ);
					else
						sprintf(debugtmp, " %f;%f;%f;;\n", mVertexInfo.vertexX, mVertexInfo.vertexY, mVertexInfo.vertexZ);
					fprintf(inputf, "%s", debugtmp);
				}
				//write index data.
				fprintf(inputf, " %d;\n", indexCount / 3);
				for (DWORD i = 0; i < indexCount; i++) {

					//pIndexArray[i] += dwVertexStart;
					//pIndexArray[i] ++; // .obj start with vertex 1 , dbo/.x start at 0.
					if (split == 3) {
						if (i < indexCount - 1)
							sprintf(debugtmp, " 3;%d,%d,%d;,\n", pIndexArray[i - 2], pIndexArray[i - 1], pIndexArray[i]);
						else
							sprintf(debugtmp, " 3;%d,%d,%d;;\n", pIndexArray[i - 2], pIndexArray[i - 1], pIndexArray[i]);
						fprintf(inputf, "%s", debugtmp);
						split = 0;
					}
					split++;
				}


				//				" 0.000000;0.000000;0.000000;,\n"
				//				" 254.000000;0.000000;-254.000000;,\n"
				//				" 254.000000;0.000000;0.000000;,\n"
				//				" 0.000000;0.000000;-254.000000;;\n"
				//					" 2;\n"
				//				" 3;0,2,1;,\n"
				//				" 3;0,1,3;;\n"

				fprintf(inputf, " MeshNormals {\n"
					" %d;\n", vertexCount);

				//Write VN normals
				for (int a = 0; a < vertexCount; a++) {
					int normalOffset = myOffsetMap.dwNX * sizeof(DWORD);
					addpos = FVFSize * a + normalOffset;
					mVertexInfo.normalX = *(FLOAT *)((BYTE *)vertexData + addpos);
					mVertexInfo.normalY = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
					mVertexInfo.normalZ = *(FLOAT *)((BYTE *)vertexData + 8 + addpos);
					if (a < vertexCount - 1)
						sprintf(debugtmp, " %f;%f;%f;,\n", mVertexInfo.normalX, mVertexInfo.normalY, mVertexInfo.normalZ);
					else
						sprintf(debugtmp, " %f;%f;%f;;\n", mVertexInfo.normalX, mVertexInfo.normalY, mVertexInfo.normalZ);
					fprintf(inputf, "%s", debugtmp);
				}
				//write index data.
				fprintf(inputf, " %d;\n", indexCount / 3);
				for (DWORD i = 0; i < indexCount; i++) {

					//pIndexArray[i] += dwVertexStart;
					//pIndexArray[i] ++; // .obj start with vertex 1 , dbo/.x start at 0.
					if (split == 3) {
						if (i < indexCount - 1)
							sprintf(debugtmp, " 3;%d,%d,%d;,\n", pIndexArray[i - 2], pIndexArray[i - 1], pIndexArray[i]);
						else
							sprintf(debugtmp, " 3;%d,%d,%d;;\n", pIndexArray[i - 2], pIndexArray[i - 1], pIndexArray[i]);
						fprintf(inputf, "%s", debugtmp);
						split = 0;
					}
					split++;
				}
				fprintf(inputf, " }\n"); // end mesh normals.

										 //				" MeshNormals {\n"
										 //				"  4;\n"
										 //				"  0.000000;1.000000;0.000000;,\n"
										 //				"  0.000000;1.000000;0.000000;,\n"
										 //				"  0.000000;1.000000;0.000000;,\n"
										 //				"  0.000000;1.000000;0.000000;;\n"
										 //				"  2;\n"
										 //				"  3;0,2,1;,\n"
										 //				"  3;0,1,3;;\n"
										 //				" }\n"

				fprintf(inputf, " MeshTextureCoords {\n"
					" %d;\n", vertexCount);

				//Write VT texturecords.
				for (int a = 0; a < vertexCount; a++) {
					int uvOffset = myOffsetMap.dwTU[0] * sizeof(DWORD);; // missing GGDECLUSAGE_TEXCOORD ... so guess for now. GGDECLUSAGE_TEXCOORD=6
					addpos = FVFSize * a + uvOffset;
					mVertexInfo.uU = *(FLOAT *)((BYTE *)vertexData + addpos);
					mVertexInfo.uV = *(FLOAT *)((BYTE *)vertexData + 4 + addpos);
					if (a < vertexCount - 1)
						sprintf(debugtmp, " %f;%f;,\n", mVertexInfo.uU, mVertexInfo.uV);
					else
						sprintf(debugtmp, " %f;%f;;\n", mVertexInfo.uU, mVertexInfo.uV);
					fprintf(inputf, "%s", debugtmp);
				}
				fprintf(inputf, " }\n"); // end MeshTextureCoords.

										 //				" MeshTextureCoords {\n"
										 //				"  4;\n"
										 //				"  0.506506;-0.505899;,\n"
										 //				"  0.994746;-0.994139;,\n"
										 //				"  0.994746;-0.505899;,\n"
										 //				"  0.506506;-0.994139;;\n"
										 //				" }\n"

										 //				" VertexDuplicationIndices {\n"
										 //					"  4;\n"
										 //					"  4;\n"
										 //					"  0,\n"
										 //					"  1,\n"
										 //					"  2,\n"
										 //					"  3;\n"
										 //					" }\n"


										 // MeshMaterialList Need to match facecount.
										 //				" MeshMaterialList {\n"
										 //					"  1;\n"
										 //					"  2;\n"
										 //					"  0,\n"
										 //					"  0;\n"

										 //MeshMaterialList{
										 //	1;
										 //	92;
										 //0, // 92 lines of 0,

				fprintf(inputf, "\n"

					"  Material no_material {\n"
					"   0.000000;0.000000;0.000000;0.000000;;\n"
					"   0.000000;\n"
					"   0.000000;0.000000;0.000000;;\n"
					"   0.000000;0.000000;0.000000;;\n"
					"  }\n"
					//				" }\n"

					" XSkinMeshHeader {\n"
					"  0;\n"
					"  0;\n"
					"  0;\n"
					" }\n"
					"}\n");
			}
#endif

#ifdef CONVERTTOOBJ
			FILE *inputf;
			inputf = GG_fopen(convertto, "a+");

			for (DWORD i = 0; i < indexCount; i++) {

				//pIndexArray[i] += dwVertexStart;
				pIndexArray[i] ++; // .obj start with vertex 1 , dbo/.x start at 0.
				if (split == 3) {
					sprintf(debugtmp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", pIndexArray[i - 2], pIndexArray[i - 2], pIndexArray[i - 2], pIndexArray[i - 1], pIndexArray[i - 1], pIndexArray[i - 1], pIndexArray[i], pIndexArray[i], pIndexArray[i]);
					fprintf(inputf, "%s", debugtmp);
					split = 0;
				}
				split++;
			}
			if (split == 2) {
				sprintf(debugtmp, "f %d/%d/%d\n", pIndexArray[indexCount - 1], pIndexArray[indexCount - 1], pIndexArray[indexCount - 1]);
				fprintf(inputf, "%s", debugtmp);
			}
			if (split == 3) {
				sprintf(debugtmp, "f %d/%d/%d %d/%d/%d\n", pIndexArray[indexCount - 2], pIndexArray[indexCount - 2], pIndexArray[indexCount - 2], pIndexArray[indexCount - 1], pIndexArray[indexCount - 1], pIndexArray[indexCount - 1]);
				fprintf(inputf, "%s", debugtmp);
			}
			fclose(inputf);
#endif
		}
		break;

		
		case DBOBLOCK_MESH_PRIMTYPE:		ppBlock = ReadDWORD(&dwDummy, ppBlock);	break;
		case DBOBLOCK_MESH_DRAWVERTCOUNT:	ppBlock = ReadDWORD(&dwDummy, ppBlock);	break;
		case DBOBLOCK_MESH_DRAWPRIMCOUNT:	ppBlock = ReadDWORD(&dwDummy, ppBlock);	break;
		case DBOBLOCK_MESH_BONECOUNT:		
		{
			ppBlock = ReadDWORD(&dwDummy, ppBlock);	
			if ( dwDummy > 0 ) g_bHasAnimation = true;
			break;
		}
		case DBOBLOCK_MESH_SPECULAROVERRIDE: ppBlock = ReadDWORD(&dwDummy, ppBlock);	break;
		case DBOBLOCK_MESH_TEXTURECOUNT: 	ppBlock = ReadDWORD(&dwDummy, ppBlock);	break;
		case DBOBLOCK_MESH_MULTIMATCOUNT: 	ppBlock = ReadDWORD(&dwDummy, ppBlock);	break;
		case DBOBLOCK_MESH_GHOSTMODE:		ppBlock = ReadDWORD(&dwDummy, ppBlock);	break;

		
		//		case DBOBLOCK_MESH_SPECULAROVERRIDE: ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->fSpecularOverride, ppBlock);							break;
		//		case DBOBLOCK_MESH_TEXTURECOUNT: 	ppBlock = ReadDWORD(&(*ppMesh)->dwTextureCount, ppBlock);										break;
		//		case DBOBLOCK_MESH_MULTIMATCOUNT: 	ppBlock = ReadDWORD(&(*ppMesh)->dwMultiMaterialCount, ppBlock);								break;
		//		case DBOBLOCK_MESH_PRIMTYPE:		ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->iPrimitiveType, ppBlock);								break;
		//		case DBOBLOCK_MESH_DRAWVERTCOUNT:	ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->iDrawVertexCount, ppBlock);							break;
		//		case DBOBLOCK_MESH_DRAWPRIMCOUNT:	ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->iDrawPrimitives, ppBlock);							break;
		//		case DBOBLOCK_MESH_BONECOUNT:		ppBlock = ReadDWORD(&(*ppMesh)->dwBoneCount, ppBlock);										break;
		//		case DBOBLOCK_MESH_GHOSTMODE:		ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->iGhostMode, ppBlock);									break;

		//		case DBOBLOCK_MESH_USEMATERIAL: 	ppBlock = ReadBOOL(&(*ppMesh)->bUsesMaterial, ppBlock);										break;
		//		case DBOBLOCK_MESH_USEMULTIMAT: 	ppBlock = ReadBOOL(&(*ppMesh)->bUseMultiMaterial, ppBlock);									break;
		//		case DBOBLOCK_MESH_WIREFRAME:		ppBlock = ReadBOOL(&(*ppMesh)->bWireframe, ppBlock);											break;
		//		case DBOBLOCK_MESH_LIGHT:			ppBlock = ReadBOOL(&(*ppMesh)->bLight, ppBlock);												break;
		//		case DBOBLOCK_MESH_CULL:			ppBlock = ReadBOOL(&(*ppMesh)->bCull, ppBlock);												break;
		//		case DBOBLOCK_MESH_FOG:			ppBlock = ReadBOOL(&(*ppMesh)->bFog, ppBlock);												break;
		//		case DBOBLOCK_MESH_AMBIENT:		ppBlock = ReadBOOL(&(*ppMesh)->bAmbient, ppBlock);											break;
		//		case DBOBLOCK_MESH_TRANSPARENCY:	ppBlock = ReadBOOL(&(*ppMesh)->bTransparency, ppBlock);										break;
		//		case DBOBLOCK_MESH_GHOST:			ppBlock = ReadBOOL(&(*ppMesh)->bGhost, ppBlock);												break;
		//		case DBOBLOCK_MESH_VISIBLE:		ppBlock = ReadBOOL(&(*ppMesh)->bVisible, ppBlock);											break;
		//		case DBOBLOCK_MESH_LINKED:			ppBlock = ReadBOOL(&(*ppMesh)->bLinked, ppBlock);											break;


		//		case DBOBLOCK_MESH_BONESDATA: 		ppBlock = ConstructBones(&(*ppMesh)->pBones, (*ppMesh)->dwBoneCount, ppBlock);						break;
		//		case DBOBLOCK_MESH_MATERIAL: 		ppBlock = ReadMaterial(&(*ppMesh)->mMaterial, ppBlock);											break;
		//		case DBOBLOCK_MESH_TEXTURES:		ppBlock = ConstructTexture(&(*ppMesh)->pTextures, (*ppMesh)->dwTextureCount, ppBlock);				break;
		//		case DBOBLOCK_MESH_MULTIMAT: 		ppBlock = ConstructMultiMaterial(&(*ppMesh)->pMultiMaterial, (*ppMesh)->dwMultiMaterialCount, ppBlock);	break;

		//		case DBOBLOCK_MESH_FXEFFECTNAME:
		//		{
		//			// 250704 - create vertex effect object
		//			ppBlock = ReadString((*ppMesh)->pEffectName, ppBlock);
		//			(*ppMesh)->bUseVertexShader = true;
		//			break;
		//		}

		//		case DBOBLOCK_MESH_ARBITARYVALUE:
		//		{
		//			// 190804 - retain this value
		//			ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->Collision.dwArbitaryValue, ppBlock);
		//			break;
		//		}

		//		case DBOBLOCK_MESH_ZBIASFLAG:		ppBlock = ReadBOOL(&(*ppMesh)->bZBiasActive, ppBlock);										break;
		//		case DBOBLOCK_MESH_ZBIASSLOPE:		ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->fZBiasSlopeScale, ppBlock);							break;
		//		case DBOBLOCK_MESH_ZBIASDEPTH:		ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->fZBiasDepth, ppBlock);								break;
		//		case DBOBLOCK_MESH_ZREAD:			ppBlock = ReadBOOL(&(*ppMesh)->bZRead, ppBlock);												break;
		//		case DBOBLOCK_MESH_ZWRITE:			ppBlock = ReadBOOL(&(*ppMesh)->bZWrite, ppBlock);											break;
		//		case DBOBLOCK_MESH_ALPHATESTVALUE: ppBlock = ReadDWORD((DWORD*)&(*ppMesh)->dwAlphaTestValue, ppBlock);									break;

		default:
			//*ppBlock += dwCodeSize;
			ppBlock = (DWORD*)((char *)ppBlock + dwCodeSize);
			break;
		}

		// get next code
		if( dwCode > 0 && dwCodeSize < 8500000 )
			ppBlock = ReadCODE(&dwCode, &dwCodeSize, ppBlock);
	}

	if (vertexMem != NULL) {
		delete(vertexMem);
		vertexMem = NULL;
	}
	if (indexMem != NULL) {
		delete(indexMem);
		indexMem = NULL;
	}
	frame_count++;
	return (DWORD*)ppBlock;
}


bool GetFVFOffsetMap(DWORD dwFVF, DWORD dwFVFSize)
{
	// clear to begin with
	memset(&myOffsetMap, 0, sizeof(sOffsetMap));

	// FVF or declaration
	if (dwFVF == 0)
	{
		// Find Offsets
		for (int iElem = 0; MYGGVERTEXELEMENT[iElem].Stream != 255 && iElem < MAX_FVF_DECL_SIZE; iElem++)
		{
			int iIndex = MYGGVERTEXELEMENT[iElem].UsageIndex;
			int iElementOffset = MYGGVERTEXELEMENT[iElem].Offset / sizeof(DWORD);
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_POSITION)
			{
				myOffsetMap.dwX = iElementOffset + 0;
				myOffsetMap.dwY = iElementOffset + 1;
				myOffsetMap.dwZ = iElementOffset + 2;
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_POSITIONT)
			{
				myOffsetMap.dwX = iElementOffset + 0;
				myOffsetMap.dwY = iElementOffset + 1;
				myOffsetMap.dwZ = iElementOffset + 2;
				myOffsetMap.dwRWH = iElementOffset + 3;
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_PSIZE)
			{
				myOffsetMap.dwPointSize = iElementOffset + 0;
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_NORMAL)
			{
				myOffsetMap.dwNX = iElementOffset + 0;
				myOffsetMap.dwNY = iElementOffset + 1;
				myOffsetMap.dwNZ = iElementOffset + 2;
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_COLOR && iIndex == 0)
			{
				myOffsetMap.dwDiffuse = iElementOffset + 0;
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_COLOR && iIndex == 1)
			{
				myOffsetMap.dwSpecular = iElementOffset + 0;
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_TEXCOORD)
			{
				myOffsetMap.dwTU[iIndex] = iElementOffset + 0;
				if (MYGGVERTEXELEMENT[iElem].Type == GGDECLTYPE_FLOAT2)
				{
					myOffsetMap.dwTV[iIndex] = iElementOffset + 1;
				}
				if (MYGGVERTEXELEMENT[iElem].Type == GGDECLTYPE_FLOAT3)
				{
					myOffsetMap.dwTV[iIndex] = iElementOffset + 1;
					myOffsetMap.dwTZ[iIndex] = iElementOffset + 2;
				}
				if (MYGGVERTEXELEMENT[iElem].Type == GGDECLTYPE_FLOAT4)
				{
					myOffsetMap.dwTV[iIndex] = iElementOffset + 1;
					myOffsetMap.dwTZ[iIndex] = iElementOffset + 2;
					myOffsetMap.dwTW[iIndex] = iElementOffset + 3;
				}
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_TANGENT)
			{
				myOffsetMap.dwTU[1] = iElementOffset + 0;
				myOffsetMap.dwTV[1] = iElementOffset + 1;
				myOffsetMap.dwTZ[1] = iElementOffset + 2;
				myOffsetMap.dwTW[1] = iElementOffset + 3;
			}
			if (MYGGVERTEXELEMENT[iElem].Usage == GGDECLUSAGE_BINORMAL)
			{
				myOffsetMap.dwTU[2] = iElementOffset + 0;
				myOffsetMap.dwTV[2] = iElementOffset + 1;
				myOffsetMap.dwTZ[2] = iElementOffset + 2;
				myOffsetMap.dwTW[2] = iElementOffset + 3;
			}
		}

		// calculate byte offset
		myOffsetMap.dwByteSize = dwFVFSize;

		// store number of offsets
		myOffsetMap.dwSize = dwFVFSize / sizeof(DWORD);

		// complete
		return true;
	}
	else
	{
		return GetFVFValueOffsetMap(dwFVF);
	}
}

bool GetFVFValueOffsetMap(DWORD dwFVF)
{

	memset(&myOffsetMap, 0, sizeof(sOffsetMap));

	int iOffset = 0;
	int iPosition = 0;
	DWORD dwFVFSize = 0;

	if (dwFVF & GGFVF_XYZ)
	{
		myOffsetMap.dwX = iOffset + 0;
		myOffsetMap.dwY = iOffset + 1;
		myOffsetMap.dwZ = iOffset + 2;
		iOffset += 3;
	}

	if (dwFVF & GGFVF_XYZRHW)
	{
		myOffsetMap.dwRWH = iOffset + 0;
		iOffset += 1;
	}

	if (dwFVF & GGFVF_NORMAL)
	{
		myOffsetMap.dwNX = iOffset + 0;
		myOffsetMap.dwNY = iOffset + 1;
		myOffsetMap.dwNZ = iOffset + 2;
		iOffset += 3;
	}

	if (dwFVF & GGFVF_PSIZE)
	{
		myOffsetMap.dwPointSize = iOffset + 0;
		iOffset += 1;
	}

	if (dwFVF & GGFVF_DIFFUSE)
	{
		myOffsetMap.dwDiffuse = iOffset + 0;
		iOffset += 1;
	}

	if (dwFVF & GGFVF_SPECULAR)
	{
		myOffsetMap.dwSpecular = iOffset + 0;
		iOffset += 1;
	}

	DWORD dwTexCount = 0;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX1) dwTexCount = 1;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX2) dwTexCount = 2;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX3) dwTexCount = 3;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX4) dwTexCount = 4;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX5) dwTexCount = 5;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX6) dwTexCount = 6;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX7) dwTexCount = 7;
	if ((dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX8) dwTexCount = 8;
	for (DWORD dwTexCoordSet = 0; dwTexCoordSet<dwTexCount; dwTexCoordSet++)
	{
		DWORD dwTexCoord = dwFVF & GGFVF_TEXCOORDSIZE1(dwTexCoordSet);
		if (dwTexCoord == (DWORD)GGFVF_TEXCOORDSIZE1(dwTexCoordSet))
		{
			myOffsetMap.dwTU[dwTexCoordSet] = iOffset + 0;
			iOffset += 1;
		}
		if (dwTexCoord == (DWORD)GGFVF_TEXCOORDSIZE2(dwTexCoordSet))
		{
			myOffsetMap.dwTU[dwTexCoordSet] = iOffset + 0;
			myOffsetMap.dwTV[dwTexCoordSet] = iOffset + 1;
			iOffset += 2;
		}
		if (dwTexCoord == (DWORD)GGFVF_TEXCOORDSIZE3(dwTexCoordSet))
		{
			myOffsetMap.dwTU[dwTexCoordSet] = iOffset + 0;
			myOffsetMap.dwTV[dwTexCoordSet] = iOffset + 1;
			myOffsetMap.dwTZ[dwTexCoordSet] = iOffset + 2;
			iOffset += 3;
		}
		if (dwTexCoord == (DWORD)GGFVF_TEXCOORDSIZE4(dwTexCoordSet))
		{
			myOffsetMap.dwTU[dwTexCoordSet] = iOffset + 0;
			myOffsetMap.dwTV[dwTexCoordSet] = iOffset + 1;
			myOffsetMap.dwTZ[dwTexCoordSet] = iOffset + 2;
			myOffsetMap.dwTW[dwTexCoordSet] = iOffset + 3;
			iOffset += 4;
		}
	}

	// calculate byte offset
	myOffsetMap.dwByteSize = sizeof(DWORD) * iOffset;

	// store number of offsets
	myOffsetMap.dwSize = iOffset;

	// check if matches byte size of actual FVF
#ifdef DX11
#else
	//	dwFVFSize = D3DXGetFVFVertexSize(dwFVF);
	//	if (dwFVFSize != myOffsetMap.dwByteSize)
	//	{
	//		// Offsets not being calculated correctly!
	//		return false;
	//	}
#endif

	// complete
	return true;
}




DWORD* ConstructFrame(LPSTR pFrameName, DWORD* ppBlock)
{
	// get code
	DWORD dwCode = 0;
	DWORD dwCodeSize = 0;

	char tmpFrameName[MAX_STRING];
	strcpy(tmpFrameName, pFrameName);
	ppBlock = ReadCODE(&dwCode, &dwCodeSize, ppBlock);
	if (dwCode > 0)
	{
		// create frame
		//(*ppFrame) = new sFrame;
	}

	while (dwCode > 0 && dwCodeSize < 8500000)
	{
		switch (dwCode)
		{
		case DBOBLOCK_FRAME_NAME:     ppBlock = ReadString(tmpFrameName, (LPSTR)ppBlock);			break;
			//		case DBOBLOCK_FRAME_MATRIX:   ppBlock = ReadMatrix(matrix, ppBlock);	break; // 16 float.
		case DBOBLOCK_FRAME_MESH:     ppBlock = ConstructMesh(tmpFrameName, ppBlock);			break; // pFrameName to save obj.
		case DBOBLOCK_FRAME_CHILD:
			ppBlock = ConstructFrame(tmpFrameName, ppBlock);
			//if ((*ppFrame)->pChild)
			//{
			//	(*ppFrame)->pChild->pParent = *ppFrame;
			//}
			break;
		case DBOBLOCK_FRAME_SIBLING:  ppBlock = ConstructFrame(tmpFrameName, ppBlock);		break;
			//		case DBOBLOCK_FRAME_OFFSET:   ppBlock = ReadVector(vector, ppBlock);		break; // vecoffset 3 float
			//		case DBOBLOCK_FRAME_ROTATION: ppBlock = ReadVector(vector, ppBlock);	break; // vecrotation
			//		case DBOBLOCK_FRAME_SCALE:    ppBlock = ReadVector(vector, ppBlock);		break; // vecscale

		default:
			//*ppBlock += dwCodeSize;
			ppBlock = (DWORD*)((char *)ppBlock + dwCodeSize);
			break;
		}

		// get next code
		if(dwCode > 0 && dwCodeSize < 8500000)
			ppBlock = ReadCODE(&dwCode, &dwCodeSize, ppBlock);
	}

	return (DWORD*)ppBlock;
}

DWORD* ReadCODE(DWORD* pdwCode, DWORD* pdwCodeSize, DWORD* ppBlock)
{
	// code and codesize
	*(pdwCode) = (DWORD)*ppBlock;
	ppBlock++;
	*(pdwCodeSize) = (DWORD)*ppBlock;
	ppBlock++;
	return (DWORD*)ppBlock;
}


DWORD* ReadIntoMemory(BYTE* pData, DWORD dwBytes, DWORD* ppBlock)
{
	memcpy(pData, ppBlock, dwBytes);
	//*ppBlock += dwBytes;
	return (DWORD*)((char *)ppBlock + dwBytes);
}

DWORD* ReadIndexData(DWORD* pwIndexData, DWORD dwIndexCount, DWORD* ppBlock)
{
	DWORD dwLength = sizeof(WORD) * dwIndexCount;
	if (dwLength > 0)
	{
		//(*pwIndexData) = (WORD*)new char[dwLength];
		memcpy(pwIndexData, ppBlock, dwLength);
	}
	return (DWORD*)((char *)ppBlock + dwLength);
}

DWORD* ReadDWORD(DWORD* ppDest, DWORD* ppBlock)
{
	*(ppDest) = (DWORD)*ppBlock;
	return (DWORD*)((char *)ppBlock + 4);
}

DWORD* ReadString(LPSTR pString, LPSTR ppBlock)
{
	// length of string
	DWORD dwFullLength = (DWORD)*ppBlock;
	ppBlock += 4;

	// limit string size
	DWORD dwLength = dwFullLength;
	if (dwLength > 255) dwLength = 255;
	if (dwLength > 0)
	{
		// create if not present
		if (pString == NULL) pString = new char[dwLength + 1];

		// copy string
		memcpy(pString, ppBlock, dwLength);
		pString[dwLength] = 0;
		ppBlock += dwFullLength;
	}

	// okay
	return (DWORD*)ppBlock;
}

//////////////////////////////////////////////////////////////////////////////////
// INTERNAL READ FUNCTIONS ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MyReadBOOL(bool* ppDest, LPSTR* ppBlock)
{
	// length of string
	if (*(BYTE*)*ppBlock == 0)
		*(ppDest) = false;
	else
		*(ppDest) = true;

	*ppBlock += 1;

	// okay
	return true;
}

//bool ReadVector(GGVECTOR3* pVector, LPSTR* ppBlock)
//{
//	DWORD dwLength = sizeof(GGVECTOR3);
//	memcpy(pVector, *ppBlock, dwLength);
//	*ppBlock += dwLength;
//	return true;
//}

//bool ReadMatrix(GGMATRIX* pMatrix, LPSTR* ppBlock)
//{
//	DWORD dwLength = sizeof(GGMATRIX);
//	memcpy(pMatrix, *ppBlock, dwLength);
//	*ppBlock += dwLength;
//	return true;
//}

//bool ReadMaterial(D3DMATERIAL9* pMaterial, LPSTR* ppBlock)
//{
//	DWORD dwLength = sizeof(D3DMATERIAL9);
//	memcpy(pMaterial, *ppBlock, dwLength);
//	*ppBlock += dwLength;
//	return true;
//}

bool MyReadMemory(BYTE** ppData, DWORD dwBytes, LPSTR* ppBlock)
{
	if ((*ppData) == NULL) (*ppData) = (BYTE*)new char[dwBytes];
	memcpy((*ppData), *ppBlock, dwBytes);
	*ppBlock += dwBytes;
	return true;
}

bool MyReadOffsetListData(int** pwIndexData, DWORD dwIndexCount, LPSTR* ppBlock)
{
	DWORD dwLength = sizeof(int) * dwIndexCount;
	if (dwLength > 0)
	{
		(*pwIndexData) = (int*)new char[dwLength];
		memcpy((*pwIndexData), *ppBlock, dwLength);
		*ppBlock += dwLength;
	}
	return true;
}


#define MAXREPLACE 1024
char replaced[MAXREPLACE];
char *str_replace(char *search, char *replace, char *subject)
{
	char  *p = NULL, *old = NULL, *new_subject = NULL;
	char tmp[MAXREPLACE];
	int c = 0, search_size;

	strcpy(tmp, subject);

	search_size = strlen(search);

	//Count how many occurences
	for (p = strstr(tmp, search); p != NULL; p = strstr(p + search_size, search))
	{
		c++;
	}

	//Final size
	c = (strlen(replace) - search_size)*c + strlen(tmp);

	//The start position
	for (int a = 0; a < 1024; a++) replaced[a] = 0;
	new_subject = &replaced[0];

	//Set it to blank
	strcpy(new_subject, "");
	old = &tmp[0];

	for (p = strstr(tmp, search); p != NULL; p = strstr(p + search_size, search))
	{
		//move ahead and copy some text from original subject , from a certain position
		int tlen = p - old;
		strncpy(new_subject + strlen(new_subject), old, p - old);

		//move ahead and copy the replacement text
		strcpy(new_subject + strlen(new_subject), replace);

		//The new start position after this search match
		old = p + search_size;
	}

	//Copy the part after the last search match
	strcpy(new_subject + strlen(new_subject), old);

	return new_subject;
}

