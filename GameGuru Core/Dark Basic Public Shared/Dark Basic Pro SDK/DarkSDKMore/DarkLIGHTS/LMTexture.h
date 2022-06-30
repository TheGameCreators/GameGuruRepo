#ifndef LMTEXTURE_H
#define LMTEXTURE_H

#include "directx-macros.h"

class Lumel;
class LumelLite;
class LMObject;
class LMTexNode;
class Light;
class CollisionTreeLightmapper;

//#include "Thread.h"

class LMTexture //: public Thread
{

private:
	
	int iSizeU, iSizeV;
	LumelLite **ppTexLumel;			//the pixels of this texture
	bool bEmpty;

	LMTexNode* pNodeSpace;

	char pFilename[256];
	IGGTexture *pTexture;

public:
	IGGTexture *pTextureDDS;

private:
	DWORD *pPixels;
	int iPitch;
	UINT iWidth;
	UINT iHeight;

public:

	LMTexture *pNextLMTex;		//the next light map in the list, should get progressively less filled.
	//Light* pLocalLightList;
	//CollisionTreeLightmapper* pLocalColTree;
	
	LMTexture( int sizeU, int sizeV );
	~LMTexture( );

	void New ( int sizeU, int sizeV );
	void Free ( );

	bool IsEmpty( );

	int GetSizeU( );
	int GetSizeV( );

	bool AddLMObject( LMObject* pLMObject );
	//void CalculateLight ( Light* pLightList, CollisionTreeLightmapper* pColTree );

	void SetLumel( int u, int v, float red, float green, float blue );

	void CopyToTexture( );
	void SaveTexture( char* pFilename );
	IGGTexture* GetDXTextureRef( );
	char* GetFilename( );

	//unsigned int Run( );
};

#endif