//
// ConvX Functions Header
//
#ifndef __CONVX_H__
#define __CONVX_H__

#include "directx-macros.h"

/*
#ifdef DX11

// DX11 should call a DX9 compiled silent app to convert X to DBO files (no Direct X file support in DX11)
void	SetLegacyModeOn		( void );
void	SetLegacyModeOff	( void );
bool	ConvXConvert		( LPSTR pFilename, DWORD *pBlock, DWORD* pdwSize );
void	ConvXFree			( LPSTR );

#else
*/

//////////////////////////////////////////////////////////////////////////////////
// INCLUDE COMMON FILES //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <mmsystem.h>
#include <objbase.h>
#include <malloc.h> // _alloca
#include <stdio.h>
#include <eh.h>
#include "DXFile.h"

//////////////////////////////////////////////////////////////////////////////////////
// DBOFORMAT INCLUDE /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOBlock.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOFormat.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOFrame.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOMesh.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBORawMesh.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOEffects.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOFile.h"
#include "globstruct.h"

    #undef DARKSDK
    #undef DBPRO_GLOBAL
	#define DARKSDK 
	#define DBPRO_GLOBAL

//////////////////////////////////////////////////////////////////////////////////
// GUIDS NOT PART OF RM GUI DLIST ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
DEFINE_GUID(TID_D3DXSkinMeshHeader,
0x3cf169ce, 0xff7c, 0x44ab, 0x93, 0xc0, 0xf7, 0x8f, 0x62, 0xd1, 0x72, 0xe2);
DEFINE_GUID(TID_D3DVertexDuplicationIndices,
0xb8d65549, 0xd7c9, 0x4995, 0x89, 0xcf, 0x53, 0xa9, 0xa8, 0xb0, 0x31, 0xe3);
DEFINE_GUID(TID_D3DSkinWeights,
0x6f0d123b, 0xbad2, 0x4167, 0xa0, 0xd0, 0x80, 0x22, 0x4f, 0x25, 0xfa, 0xbb);

//////////////////////////////////////////////////////////////////////////////////////
// STRUCTURES ////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// main mesh data structure (pos,norm,uv)
struct sVertex
{
	float  x,  y,  z;				// position
	float nx, ny, nz;				// normal
	float tu1, tv1;					// texture coordinates 0
};

// structures of vertex color (index,rgba)
struct sColorRGBA
{
	float red;
	float green;
	float blue;
	float alpha;
};

struct sVertexColor
{
	DWORD index;
	sColorRGBA color;
};

// internal functions
DARKSDK bool	ConvXLoad ( char* szFilename );
DARKSDK bool	ConvXLoadModelData ( char* szFilename, sFrame* pFrame, bool bAnim );
DARKSDK bool	ConvXDelete ( void );

DARKSDK bool    ParseXFileData ( IDirectXFileData* pDataObj, sFrame* pParentFrame, char* szTexturePath, sAnimationSet *pParentAnim, sAnimation *pCurrentAnim, bool bAnim );
DARKSDK bool    MapFramesToBones ( sMesh** pMesh, sFrame *Frame, int iCount );
DARKSDK bool    MapFramesToAnimations ( sObject* pObject );
DARKSDK void    SetupTextures ( sMesh* pMesh, D3DMATERIAL9* pMaterials, DWORD dwCount, DWORD* pAttribute, DWORD dwIndexCount );
DARKSDK sFrame* FindFrame ( char* szName, sFrame* pFrame );
DARKSDK bool    UpdateAnimation ( sObject* pObject, float fTime );
//DARKSDK void    ResetFrameMatrices ( sFrame* pFrame );
DARKSDK bool    UpdateAnimationData ( sAnimation* pAnim, float fTime );

DARKSDK bool	ProduceMaterialIndiceTables ( void );
DARKSDK bool    SplitAnySolidMultiMeshes ( void );
//#endif

extern "C"
{
	DARKSDK void	SetLegacyModeOn		( void );
	DARKSDK void	SetLegacyModeOff	( void );
	DARKSDK	bool	ConvXConvert				( LPSTR pFilename, DWORD *pBlock, DWORD* pdwSize );
	DARKSDK void	ConvXFree				( LPSTR );
}

#endif

//#endif
