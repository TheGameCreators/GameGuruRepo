#ifndef LMOBJECT_H
#define LMOBJECT_H

#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include "Thread.h"

class LMPoly;
class LMPolyGroup;
class LMTexture;
class Light;
class CollisionTreeLightmapper;

class LMObject : public Thread
{
public:
	bool bIgnoreNormals;
	sMesh *pMesh;

private:

	sFrame *pFrame;
	sObject *pObject;

	LMPoly *pPolyList;
	LMPolyGroup *pPolyGroupList;
	int iNumPolys;
	int iBaseStage;
	int iDynamicLightMode;
	float fQuality;
	bool bShaded;

	float fObjCenX, fObjCenY, fObjCenZ, fObjRadius;

	const Light* pLocalLightList;
	const CollisionTreeLightmapper* pLocalColTree;
	int iLocalBlur;
	HANDLE pLocalSemaphore;

	unsigned int Run( );
	
public:

	int iID, iLimbID;
	LMTexture *pLMTexture;
	LMObject *pNextObject;

	LMObject( sObject *pParentObject, sFrame *pParentFrame, sMesh *pNewMesh );
	~LMObject( );

	sObject* GetObjectPtr( );
	sFrame* GetFramePtr( );
	sMesh* GetMeshPtr( );

	void New ( sObject *pParentObject, sFrame *pParentFrame, sMesh *pNewMesh );
	void Free ( void );

	void SetBaseStage( int iNewStage );
	void SetDynamicLightMode( int iNewMode );

	void SetShaded( int iLightMapStage );
	bool GetIsShaded( );

	LMPolyGroup* GetFirstGroup( );
	int GetNumPolys( );
	float GetQuality( );

	float RecomendedQaulity( int iTexSizeU, int iTexSizeV );
	bool WillFit( int iTexSizeU, int iTexSizeV );

	void BuildPolyList( bool bFlatShaded );
	void GroupPolys( float fQuality );
	void SortGroups( );		//best to lightmap large groups first so smaller ones can fill in the gaps later
	void ReScaleGroups( float fQuality );

	void SetLocalValues( const Light *pLightList, const CollisionTreeLightmapper *pColTree, int iBlur, HANDLE pSemaphore );

	void WaitForThreads( );
	void CalculateLight( const Light *pLightList, const CollisionTreeLightmapper *pColTree, int iBlur, bool bIgnoreNormals, int iThreadLimit ); 
	void ApplyToTexture( );
	void CalculateVertexUV( int iTexSizeU, int iTexSizeV );

	void UpdateObject( int iBlendMode );
	void CreateTriOnlyAndApplyUVData ( void );
};

#endif