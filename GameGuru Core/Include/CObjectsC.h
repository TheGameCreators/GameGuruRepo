
#ifndef _COBJECTS_H_
#define _COBJECTS_H_

//////////////////////////////////////////////////////////////////////////////////
// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#include ".\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Objects\CommonC.h"

struct IntersectRay
{
	float direction[3];
	float origin[3];
};
struct IntersectBox
{
	float min[3];
	float max[3];
};

// Internal functions (not actual commands)
DARKSDK void RefreshMeshShortList ( sMesh* pMesh );
DARKSDK void ConvertToFVF ( sMesh* pMesh, DWORD dwFVF );
DARKSDK void SmoothNormals ( sMesh* pMesh, float fAngle );
DARKSDK void ConvertLocalMeshToVertsOnly ( sMesh* pMesh );
DARKSDK bool CalcObjectWorld ( sObject* pObject );
DARKSDK void CalculateAbsoluteWorldMatrix ( sObject* pObject, sFrame* pFrame, sMesh* pMesh );
DARKSDK void ConvertLocalMeshToFVF ( sMesh* pMesh, DWORD dwFVF );
DARKSDK void SetObjectBlendMap ( int iID, int iLimbID, int iStage, int iImage, int iTexCoordMode, int iMode, int iA, int iB, int iC, int iR );
DARKSDK bool GetFVFOffsetMap ( sMesh* pMesh, sOffsetMap* psOffsetMap );
DARKSDK void CloneMeshToNewFormat ( int iID, DWORD dwFVF, DWORD dwEraseBones );
DARKSDK_DLL void UpdateOnce ( void );

//////////////////////////////////////////////////////////////////////////////////
// COMMAND FUNCTIONS /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK void Basic3DConstructor		( void );
DARKSDK void Basic3DPassCoreData	( LPVOID pGlobPtr );
DARKSDK void Basic3DRefreshGRAFIX		( int iMode );
DARKSDK void Basic3DDestructor		( void );

DARKSDK void LoadObject				( LPSTR szFilename, int iID );
DARKSDK void LoadObject				( LPSTR szFilename, int iID, int iDBProMode );
DARKSDK void LoadObject				( LPSTR szFilename, int iID, int iDBProMode, int iDivideTextureSize );
DARKSDK void EnsureObjectDBOIsFVF	( int iID, LPSTR pFileToLoad, DWORD dwRequiredFVF );
DARKSDK void SaveObject				( LPSTR szFilename, int iID );
DARKSDK void DeleteObject			( int iID );
DARKSDK bool DeleteObjectSpecial	( int iID );
DARKSDK void SetObject				( int iID, SDK_BOOL bWireframe, SDK_BOOL bTransparency, SDK_BOOL bCull );
DARKSDK void SetObject				( int iID, SDK_BOOL bWireframe, SDK_BOOL bTransparency, SDK_BOOL bCull, int iFilter );
DARKSDK void SetObject				( int iID, SDK_BOOL bWireframe, SDK_BOOL bTransparency, SDK_BOOL bCull, int iFilter, SDK_BOOL bLight );
DARKSDK void SetObject				( int iID, SDK_BOOL bWireframe, SDK_BOOL bTransparency, SDK_BOOL bCull, int iFilter, SDK_BOOL bLight, SDK_BOOL bFog );
DARKSDK void SetObject				( int iID, SDK_BOOL bWireframe, SDK_BOOL bTransparency, SDK_BOOL bCull, int iFilter, SDK_BOOL bLight, SDK_BOOL bFog, SDK_BOOL bAmbient );
DARKSDK void SetObjectWireframe		( int iID, SDK_BOOL bFlag    );
DARKSDK void SetObjectTransparency	( int iID, SDK_BOOL bFlag    );
DARKSDK void SetObjectCull			( int iID, SDK_BOOL bCull    );
DARKSDK void SetLimbCull			( int iID, int iLimbIndex, SDK_BOOL bCull );
DARKSDK void SetObjectFilter		( int iID, int  iFilter  );
DARKSDK void SetObjectFilterStage	( int iID, int iStage, int iFilter );
DARKSDK void SetObjectLight			( int iID, SDK_BOOL bLight   );
DARKSDK void SetObjectFog			( int iID, SDK_BOOL bFog     );
DARKSDK void SetObjectAmbient		( int iID, SDK_BOOL bAmbient );
DARKSDK void SetObjectRenderMatrixMode	( int iID, int iRenderMatrixMode );
DARKSDK void MakeObject				( int iID, int iMeshID, int iTextureID );
DARKSDK void MakeSphere			( int iID, float fRadius, int iRings, int iSegments );
DARKSDK void CloneObject		( int iDestinationID, int iSourceID, int iCloneSharedData );
DARKSDK void CloneObject				( int iDestinationID, int iSourceID );
DARKSDK void InstanceObject			( int iDestinationID, int iSourceID );

DARKSDK void MakeObjectSphere			( int iID, float fRadius );
DARKSDK void MakeObjectSphere			( int iID, float fRadius, int iRings, int iSegments );
DARKSDK void MakeObjectCube			( int iID, float iSize );
DARKSDK void MakeObjectBox			( int iID, float fWidth, float fHeight, float fDepth );
DARKSDK void MakeObjectPyramid		( int iID, float fSize );
DARKSDK void MakeObjectCylinder		( int iID, float fSize );
DARKSDK void MakeObjectCone			( int iID, float fSize );
DARKSDK void MakeObjectPlane			( int iID, float fWidth, float fHeight );
DARKSDK void MakeObjectPlane			( int iID, float fWidth, float fHeight, int iFlag );
DARKSDK void MakeObjectTriangle		( int iID, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3 );
DARKSDK void AppendObject				( SDK_LPSTR pString, int iID, int iFrame );
DARKSDK void PlayObject				( int iID );
DARKSDK void PlayObject 				( int iID, int iStart );
DARKSDK void PlayObject				( int iID, int iStart, int iEnd );
DARKSDK void LoopObject				( int iID );
DARKSDK void LoopObject				( int iID, int iStart );
DARKSDK void LoopObject				( int iID, int iStart, int iEnd );
DARKSDK void StopObject				( int iID );
DARKSDK void AddVisibilityListMask ( int iID );
DARKSDK void HideObject				( int iID );
DARKSDK void ShowObject				( int iID );
DARKSDK void ColorObject				( int iID, DWORD dwRGB );
DARKSDK void SetObjectFrame			( int iID, int iFrame );
DARKSDK void SetObjectFrameEx			( int iID, float fFrame );
DARKSDK void SetObjectFrameEx			( int iID, float fFrame, int iRecalculateBounds );
DARKSDK void SetObjectFrameEx			( int iID, int iLimbID, float fFrame, int iEnableOverride );
DARKSDK void SetObjectSpeed			( int iID, int iSpeed );
DARKSDK void SetObjectDefAnim		( int iID, int iSkipDefFrameInIntersectAll );
DARKSDK void SetObjectInterpolation	( int iID, int iJump );
DARKSDK void SetObjectRotationXYZ		( int iID );
DARKSDK void SetObjectRotationZYX		( int iID );
DARKSDK void SetObjectRotationZXY		( int iID );
DARKSDK void GhostObjectOn			( int iID );
DARKSDK void GhostObjectOn			( int iID, int iFlag );
DARKSDK void GhostObjectOff			( int iID );
DARKSDK void FadeObject				( int iID, float iPercentage );
DARKSDK void GlueObjectToLimbEx		( int iSource, int iTarget, int iLimb, int iGlueMode );
DARKSDK void GlueObjectToLimb			( int iSource, int iTarget, int iLimb );
DARKSDK void UnGlueObject				( int iID );
DARKSDK void LockObjectOn				( int iID );
DARKSDK void LockObjectOff			( int iID );
DARKSDK void DisableObjectZDepth		( int iID );
DARKSDK void DisableObjectZDepthEx	( int iID, int iKeepUpdateStage );
DARKSDK void EnableObjectZDepth		( int iID );
DARKSDK void DisableObjectZRead		( int iID );
DARKSDK void EnableObjectZRead		( int iID );
DARKSDK void DisableObjectZWrite	( int iID );
DARKSDK void DisableLimbZWrite		( int iID, int iLimbIndex );
DARKSDK void EnableObjectZWrite		( int iID );
DARKSDK void SetObjectFOV			( int iID, float fRadius );
DARKSDK void DeleteObjects			( int iFrom, int iTo );
DARKSDK void ClearObjectsOfTextureRef ( LPGGTEXTURE pTextureRef );
DARKSDK void DisableObjectZBias		( int iID );
DARKSDK void EnableObjectZBias		( int iID, float fSlopeScale, float fDepth );
DARKSDK void ReverseObjectFrames ( int iID );

// Additional Color commands
DARKSDK void SetObjectDiffuseEx		( int iID, DWORD dwRGB, int iMaterialOrVertexData );
DARKSDK void SetObjectDiffuse			( int iID, DWORD dwRGB );
DARKSDK void SetObjectAmbience		( int iID, DWORD dwRGB );
DARKSDK void SetObjectSpecular		( int iID, DWORD dwRGB );
DARKSDK void SetObjectSpecularPower	( int iID, float fPower );
DARKSDK void SetObjectScrollScaleUV ( int iID, float fScrU, float fScrV, float fScaU, float fScaV );
DARKSDK void SetObjectSpecular		( int iID, DWORD dwRGB, float fPower );
DARKSDK void SetObjectEmissive		( int iID, DWORD dwRGB );
DARKSDK void SetObjectArbitaryValue			( int iID, DWORD dwArbValue );

// Additional Euler/Freeflight commands
DARKSDK void FixObjectPivot						( int iID );
DARKSDK void ResetObjectPivot					( int iID );
DARKSDK void SetObjectToObjectOrientation		( int iID, int iWhichID );
DARKSDK void SetObjectToObjectOrientation		( int iID, int iWhichID, int iMode );
DARKSDK void SetObjectToCameraOrientation		( int iID );

// Texture commands
DARKSDK void TextureObject					( int iID, int iImage );
DARKSDK void TextureObject					( int iID, int iStage, int iImage );
DARKSDK void SetObjectTextureStageEx		( int iID, int iStage, int iImage, int iDoNotSortTextures );
DARKSDK void ScrollObjectTexture			( int iID, int iStage, float fU, float fV );
DARKSDK void ScrollObjectTexture			( int iID, float fU, float fV );
DARKSDK void ScaleObjectTexture				( int iID, int iStage, float fU, float fV );
DARKSDK void ScaleObjectTexture				( int iID, float fU, float fV );
DARKSDK void SetObjectSmoothing				( int iID, float fAngle );
DARKSDK void SetObjectNormalsEx				( int iID, int iMode );
DARKSDK void SetObjectNormals				( int iID );
DARKSDK void SetObjectTextureMode			( int iID, int iMode, int iMipGeneration );
DARKSDK void SetObjectTextureModeStage		( int iID, int iStage, int iMode, int iMipGeneration );
DARKSDK void SetObjectLightMap				( int iID, int iImage );
DARKSDK void SetObjectLightMap				( int iID, int iImage, int iAddDIffuseToStageZero );
DARKSDK void SetObjectSphereMap				( int iID, int iSphereImage );
DARKSDK void SetObjectCubeMapStage			( int iID, int iStage, int i1, int i2, int i3, int i4, int i5, int i6 );
DARKSDK void SetObjectCubeMap				( int iID, int i1, int i2, int i3, int i4, int i5, int i6 );
DARKSDK void SetObjectDetailMap				( int iID, int iImage );
DARKSDK void SetObjectBlendMap				( int iID, int iLimbNumber, int iStage, int iImage, int iTexCoordMode, int iMode, int iA, int iB, int iC, int iR );
DARKSDK void SetObjectBlendMap				( int iID, int iStage, int iImage, int iTexCoordMode, int iMode, int iA, int iB, int iC, int iR );
DARKSDK void SetObjectBlendMap				( int iID, int iStage, int iImage, int iTexCoordMode, int iMode );
DARKSDK void SetObjectBlendMap				( int iID, int iImage, int iMode );
DARKSDK void SetObjectTextureMD3              ( int iID, int iH0, int iH1, int iL0, int iL1, int iL2, int iU0 );

// 2790515 - can be called from external DLLs (blitzterrain to render to depth texture)
DARKSDK int  SwitchRenderTargetToDepthTexture ( int iID );

// Additional texture commands
DARKSDK void SetAlphaMappingOn				( int iID, float fPercentage, bool bForceUnTransparency );
DARKSDK void SetAlphaMappingOn				( int iID, float fPercentage );

// bolt-on effects
///DARKSDK void SetObjectEffectOn				( int iID, SDK_LPSTR pFilename, int iUseDefaultTextures );

// occlusion commands
DARKSDK void SetOcclusionMode			( int iMode );
DARKSDK void SetObjectOcclusion			( int iID, int iOcclusionShape, int iMeshOrLimbID, int iA, int iIsOccluder, int iDeleteFromOccluder );
DARKSDK int  GetObjectOcclusionValue	( int iID );

// FX effects commands
DARKSDK void LoadEffect					( LPSTR pFilename, int iEffectID, int iUseDefaultTextures );
DARKSDK void LoadEffectEx				( LPSTR pFilename, int iEffectID, int iUseDefaultTextures, int iDoNotGenerateExtraData );
DARKSDK void DeleteEffectCore			( int iEffectID, bool bAlsoEraseObjReferences );
DARKSDK void DeleteEffect				( int iEffectID );
DARKSDK void ChangeShadowMappingPrimary ( int iEffectID );
DARKSDK void SetEffectToShadowMappingEx	( int iEffectID, int iDebugObjStart, int iDebugEffectIndex, int iHideDistantShadows, int iRealShadowResolution, int iRealShadowCascadeCount, int iC0, int iC1, int iC2, int iC3, int iC4, int iC5, int iC6, int iC7 );
DARKSDK void SetEffectToShadowMapping	( int iEffectID );
DARKSDK void SetEffectShadowMappingMode	( int iMode );
DARKSDK void RenderEffectShadowMapping	( int iEffectID );
DARKSDK void SetObjectEffectCore		( int iID, int iEffectID, int iEffectNoBoneID, int iForceCPUAnimationMode );
DARKSDK void SetObjectEffectCore		( int iID, int iEffectID, int iForceCPUAnimationMode );
DARKSDK void SetDefaultCPUAnimState		( int iCPUAnimMode );
DARKSDK void SetObjectEffect			( int iID, int iEffectID );
DARKSDK void SetLimbEffect				( int iID, int iLimbID, int iEffectID );
DARKSDK void PerformChecklistForEffectValues ( int iEffectID );
DARKSDK void PerformChecklistForEffectErrors ( void );
DARKSDK void PerformChecklistForEffectErrors ( int iEffectID );
DARKSDK void SetEffectTranspose			( int iEffectID, int iTransposeFlag );
DARKSDK void ResetEffect				( int iEffectID );
DARKSDK void EraseEffectParameterIndex  ( int iEffectID, LPSTR pConstantName );
DARKSDK DWORD GetEffectParameterIndex	( int iEffectID, LPSTR pConstantName );
DARKSDK void SetEffectConstantB			( int iEffectID, LPSTR pConstantName, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectConstantB			( int iEffectID, LPSTR pConstantName, int iValue );
DARKSDK void SetEffectConstantBEx		( int iEffectID, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectConstantI			( int iEffectID, LPSTR pConstantName, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectConstantI			( int iEffectID, LPSTR pConstantName, int iValue );
DARKSDK void SetEffectConstantIEx		( int iEffectID, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectConstantF			( int iEffectID, LPSTR pConstantName, DWORD dwParamIndex, float fValue );
DARKSDK void SetEffectConstantF			( int iEffectID, LPSTR pConstantName, float fValue );
DARKSDK void SetEffectConstantFEx		( int iEffectID, DWORD dwParamIndex, float fValue );
DARKSDK void SetEffectConstantV			( int iEffectID, LPSTR pConstantName, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectConstantV			( int iEffectID, LPSTR pConstantName, int iValue );
DARKSDK void SetEffectConstantVEx		( int iEffectID, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectConstantM			( int iEffectID, LPSTR pConstantName, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectConstantM			( int iEffectID, LPSTR pConstantName, int iValue );
DARKSDK void SetEffectConstantMEx		( int iEffectID, DWORD dwParamIndex, int iValue );
DARKSDK void SetEffectTechnique			( int iEffectID, LPSTR pTechniqueName );
DARKSDK void SetEffectTechniqueEx		( int iEffectID, DWORD dwPtr );
DARKSDK DWORD GetEffectTechniqueEx		( int iEffectID );
DARKSDK void SetEffectLODTechnique		( int iEffectID, LPSTR pTechniqueName );

// new commands - need help additions too!
DARKSDK void SetGlobalDepthSkipSystem	( bool bSkipDepthRenderings );
DARKSDK int GetEffectExist				( int iEffectID );
DARKSDK int GetObjectPolygonCount		( int iObjectNumber );
DARKSDK int GetObjectVertexCount		( int iObjectNumber );

// vertex and pixel shaders
DARKSDK void SetVertexShaderOn          ( int iID, int iShader );
DARKSDK void SetVertexShaderOff         ( int iID );
DARKSDK void CloneMeshToNewFormat       ( int iID, DWORD dwFVF );
DARKSDK void CloneMeshToNewFormat       ( int iID, DWORD dwFVF, DWORD dwEraseBones );
DARKSDK void SetVertexShaderStreamCount ( int iID, int iCount );
DARKSDK void SetVertexShaderStream      ( int iID, int iStreamPos, int iData, int iDataType );
DARKSDK void CreateVertexShaderFromFile ( int iID, SDK_LPSTR szFile );
DARKSDK void SetVertexShaderVector      ( int iID, DWORD dwRegister, int iVector, DWORD dwConstantCount );
DARKSDK void SetVertexShaderMatrix      ( int iID, DWORD dwRegister, int iMatrix, DWORD dwConstantCount );
DARKSDK void DeleteVertexShader			( int iShader );

DARKSDK void SetPixelShaderOn			( int iID, int iShader );
DARKSDK void SetPixelShaderOff			( int iID );
DARKSDK void CreatePixelShaderFromFile	( int iID, SDK_LPSTR szFile );
DARKSDK void DeletePixelShader			( int iShader );
DARKSDK void SetPixelShaderTexture		( int iShader, int iSlot, int iTexture );

// Animation
DARKSDK void SaveObjectAnimation				( int iID, SDK_LPSTR pFilename );
DARKSDK void AppendObjectAnimation			( int iID, SDK_LPSTR pFilename );
DARKSDK void ClearAllKeyFrames			( int iID );
DARKSDK void ClearKeyFrame				( int iID, int iFrame );
DARKSDK void SetObjectKeyFrame				( int iID, int iFrame );

DARKSDK int ObjectBlocking				( int iID, float X1, float Y1, float Z1, float X2, float Y2, float Z2 );

DARKSDK void AddMemblockToObject		( int iMemblock, int iID );
DARKSDK void GetMemblockFromObject		( int iMemblock, int iID );
DARKSDK void DeleteMemblockFromObject	( int iID );
DARKSDK void SetObjectStatisticsInteger ( int iID, int iIndex, int dwValue );
DARKSDK int ObjectStatisticsInteger     ( int iID, int iIndex );

// Shadows - not used for now
DARKSDK void SetGlobalShadowsOn			( void );
DARKSDK void SetGlobalShadowsOff		( void );

// Collision
DARKSDK void SetObjectCollisionOn		( int iID );
DARKSDK void SetObjectCollisionOff		( int iID );
DARKSDK void MakeCollisionBox			( int iID, float iX1, float iY1, float iZ1, float iX2, float iY2, float iZ2, int iRotatedBoxFlag );
DARKSDK void DeleteCollisionBox			( int iID );
DARKSDK void SetCollisionToSpheres		( int iID );
DARKSDK void SetCollisionToBoxes		( int iID );
DARKSDK void SetCollisionToPolygons		( int iID );
DARKSDK void SetGlobalCollisionOn		( void );
DARKSDK void SetGlobalCollisionOff		( void );
DARKSDK void SetSphereRadius			( int iID, float fRadius );
DARKSDK float IntersectObject			( int iObjectID, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ );
DARKSDK int IntersectAll				( int iStart, int iEnd, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, int iIgnoreObjNo );
DARKSDK void SetObjectCollisionProperty ( int iObjectID, int iPropertyValue );
DARKSDK void AutomaticObjectCollision	( int iObjectID, float fRadius, int iResponse );
DARKSDK void AutomaticCameraCollision	( int iCameraID, float fRadius, int iResponse, int iStandGroundMode );
DARKSDK void AutomaticCameraCollision	( int iCameraID, float fRadius, int iResponse );
DARKSDK void ForceAutomaticEnd			( void );
DARKSDK void HideBounds					( int iID );
DARKSDK void ShowBoundsEx				( int iID, int iBoxOnly );
DARKSDK void ShowBounds					( int iID, int iLimb ); // mike 160505 - new function for specifying a limb bound box drae
DARKSDK void ShowBounds					( int iID );
bool intersectRayAABox2(const IntersectRay &ray, const IntersectBox &box, int& tnear, int& tfar);

// Limb
DARKSDK void PerformCheckListForLimbs	( int iID );
DARKSDK void PerformCheckListForOnscreenObjects ( int iMode );
DARKSDK void HideLimb					( int iID, int iLimbID );
DARKSDK void ShowLimb					( int iID, int iLimbID );
DARKSDK void OffsetLimb					( int iID, int iLimbID, float fX, float fY, float fZ );
DARKSDK void OffsetLimb					( int iID, int iLimbID, float fX, float fY, float fZ, int iBoundFlag );
DARKSDK void RotateLimb					( int iID, int iLimbID, float fX, float fY, float fZ );
DARKSDK void RotateLimb					( int iID, int iLimbID, float fX, float fY, float fZ, int iBoundFlag );
DARKSDK void ScaleLimb					( int iID, int iLimbID, float fX, float fY, float fZ );
DARKSDK void ScaleLimb					( int iID, int iLimbID, float fX, float fY, float fZ, int iBoundFlag );
DARKSDK void AddLimb					( int iID, int iLimbID, int iMeshID );
DARKSDK void RemoveLimb					( int iID, int iLimbID );
DARKSDK void LinkLimb					( int iID, int iParentID, int iLimbID );
DARKSDK void TextureLimb				( int iID, int iLimbID, int iImageID );
DARKSDK void TextureLimbStage			( int iID, int iLimbID, int iStage, int iImageID );
DARKSDK void ColorLimb					( int iID, int iLimbID, DWORD dwColor );
DARKSDK void ScrollLimbTexture			( int iID, int iLimbID, int iStage, float fU, float fV );
DARKSDK void ScrollLimbTexture			( int iID, int iLimbID, float fU, float fV );
DARKSDK void ScaleLimbTexture			( int iID, int iLimbID, int iStage, float fU, float fV );
DARKSDK void ScaleLimbTexture			( int iID, int iLimbID, float fU, float fV );
DARKSDK void SetLimbSmoothing			( int iID, int iLimbID, float fPercentage );
DARKSDK void SetLimbNormals				( int iID, int iLimbID );

DARKSDK void MakeObjectFromLimbEx		( int iNewID, int iSrcID, int iLimbID, int iCopyAllFromLimb );
DARKSDK void MakeObjectFromLimb			( int iID, int iSrcObj, int iLimbID );
DARKSDK void SetObjectLOD				( int iCurrentID, int iLODLevel, float fDistanceOfLOD );
DARKSDK void AddLODToObject				( int iCurrentID, int iLODModelID, int iLODLevel, float fDistanceOfLOD );

// Mesh
//DARKSDK void  LoadMesh				( LPSTR pFilename, int iID );
DARKSDK void  DeleteMesh				( int iID );
DARKSDK void  SaveMesh					( LPSTR pFilename, int iMeshID );
DARKSDK void  ChangeMesh				( int iObjectID, int iLimbID, int iMeshID );
DARKSDK void  MakeMeshFromObject		( int iID, int iObjectID );
DARKSDK void  MakeMeshFromObject		( int iID, int iObjectID, int iIgnoreMode );
DARKSDK void  MakeMeshFromLimb			( int iMeshID, int iObjectID, int iLimbNumber );
DARKSDK void  ConvertMeshToVertexData	( int iMeshID );
DARKSDK void  ReduceMesh				( int iMeshID, int iBlockMode, int iNearMode, int iGX, int iGY, int iGZ );

// Mesh/Limb Manipulation 
DARKSDK void  LockVertexDataForLimbCore			( int iID, int iLimbID, int iReplaceOrUpdate );
DARKSDK void  LockVertexDataForLimb				( int iID, int iLimbID );
DARKSDK void  LockVertexDataForMesh				( int iID );
DARKSDK void  UnlockVertexData					( void );
DARKSDK void  SetVertexDataPosition				( int iVertex, float fX, float fY, float fZ );
DARKSDK void  SetVertexDataNormals				( int iVertex, float fNX, float fNY, float fNZ );
DARKSDK void  SetVertexDataDiffuse				( int iVertex, DWORD dwDiffuse );
DARKSDK void  SetVertexDataUV					( int iVertex, float fU, float fV );
DARKSDK void  SetVertexDataUV					( int iVertex, int iIndex, float fU, float fV );
DARKSDK void  SetVertexDataSize 				( int iVertex, float fSize );

DARKSDK void  SetIndexData						( int iIndex, int iValue );
DARKSDK int   GetIndexData						( int iIndex );

DARKSDK void  AddMeshToVertexData				( int iMeshID );
DARKSDK void  DeleteMeshFromVertexData			( int iVertex1, int iVertex2, int iIndex1, int iIndex2 );
DARKSDK int   GetVertexDataVertexCount			( void );
DARKSDK int   GetVertexDataIndexCount			( void );
DARKSDK float	GetVertexDataPositionX		( int iVertex );
DARKSDK float	GetVertexDataPositionY		( int iVertex );
DARKSDK float	GetVertexDataPositionZ		( int iVertex );
DARKSDK float	GetVertexDataNormalsX		( int iVertex );
DARKSDK float	GetVertexDataNormalsY		( int iVertex );
DARKSDK float	GetVertexDataNormalsZ		( int iVertex );
DARKSDK DWORD		GetVertexDataDiffuse		( int iVertex );
DARKSDK float	GetVertexDataU				( int iVertex );
DARKSDK float	GetVertexDataU				( int iVertex, int iIndex );
DARKSDK float	GetVertexDataV				( int iVertex );
DARKSDK float	GetVertexDataV				( int iVertex, int iIndex );
DARKSDK DWORD		GetVertexDataPtr			( void );

// Misc
DARKSDK void		SetFastBoundsCalculation	( int iMode );
DARKSDK void		SetMipmapMode				( int iMode );
DARKSDK void		FlushVideoMemory			( void );
DARKSDK void		DisableTNL					( void );
DARKSDK void		EnableTNL					( void );
DARKSDK void		Convert3DStoX				( DWORD pFilename1, DWORD pFilename2 );

// Pick Commands
DARKSDK int			PickScreenObjectEx			( int iX, int iY, int iObjectStart, int iObjectEnd, int iIgnoreCamera );
DARKSDK int			PickScreenObject			( int iX, int iY, int iObjectStart, int iObjectEnd );
DARKSDK void		PickScreen2D23D				( int iX, int iY, float fDistance );
DARKSDK SDK_FLOAT	GetPickDistance				( void );
DARKSDK SDK_FLOAT	GetPickVectorX				( void );
DARKSDK SDK_FLOAT	GetPickVectorY				( void );
DARKSDK SDK_FLOAT	GetPickVectorZ				( void );

// Emitters
DARKSDK void MakeEmitter     ( int iID, int iSize );
DARKSDK void GetEmitterData  ( int iID, BYTE** ppVertices, DWORD* pdwVertexCount, int** ppiDrawCount );
DARKSDK void UpdateEmitter   ( int iID );
DARKSDK void GetPositionData ( int iID, sPositionData** ppPosition );

DARKSDK void  ExcludeOn							( int iID );
DARKSDK void  ExcludeOff						( int iID );
DARKSDK void  SetGlobalObjectCreationMode	    ( int iMode );
DARKSDK void  ExcludeLimbOn						( int iID, int iLimbID );
DARKSDK void  ExcludeLimbOff					( int iID, int iLimbID );

// New construction commands for multiplayer nameplates

DARKSDK int MakeNewObjectPanel	( int iID , int iNumberOfCharacters );
//DARKSDK void SetObjectPanelQuad	( int iID, int index, float fX, float fY, float fWidth, float fHeight, float fU1, float fV1, float fU2, float fV2 );
DARKSDK void SetObjectPanelQuad	( int iID, int index, float fX, float fY, float fWidth, float fHeight, float fU1, float fV1, float fU2, float fV2, int r , int g , int b );
DARKSDK void FinishObjectPanel	( int iID, float fWidth, float fHeight );

//Dave Performance
DARKSDK void SetObjectStatic ( int iID , bool isStatic );
DARKSDK void SetIgnoreObject ( int iID , bool mode );
DARKSDK void ClearIgnoredObjects ( void );
DARKSDK void DoTextureListSort ( void );
DARKSDK void SetObjectAsCharacter ( int iID, bool mode );
DARKSDK void ShowIgnoredObjects ( void );
DARKSDK void HideIgnoredObjects ( void );

//////////////////////////////////////////////////////////////////////////////////
// Expressions ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Object
DARKSDK int ObjectExist    	  ( int iID );
DARKSDK int GetNumberOfFrames ( int iID );
DARKSDK float ObjectSize  ( int iID, int iAccountForScale );
DARKSDK float ObjectSizeX ( int iID, int iAccountForScale );
DARKSDK float ObjectSizeY ( int iID, int iAccountForScale );
DARKSDK float ObjectSizeZ ( int iID, int iAccountForScale );
DARKSDK float ObjectSize  ( int iID );
DARKSDK float ObjectSizeX ( int iID );
DARKSDK float ObjectSizeY ( int iID );
DARKSDK float ObjectSizeZ ( int iID );
DARKSDK float ObjectScaleX ( int iID );
DARKSDK float ObjectScaleY ( int iID );
DARKSDK float ObjectScaleZ ( int iID );
DARKSDK int GetVisible       ( int iID );
DARKSDK int GetPlaying       ( int iID );
DARKSDK int GetLooping       ( int iID );
DARKSDK float GetFrame         ( int iID );
DARKSDK float GetFrameEx       ( int iID, int iLimbID );
DARKSDK float GetSpeed         ( int iID );
DARKSDK float GetInterpolation ( int iID );
DARKSDK int GetScreenX       ( int iID );
DARKSDK int GetScreenY       ( int iID );
DARKSDK int GetInScreen      ( int iID );

DARKSDK float GetObjectCollisionRadius ( int iID );
DARKSDK float GetObjectCollisionCenterX ( int iID );
DARKSDK float GetObjectCollisionCenterY ( int iID );
DARKSDK float GetObjectCollisionCenterZ ( int iID );

// Collsion
DARKSDK int GetCollision ( int iObjectA, int iObjectB );
DARKSDK int GetHit		 ( int iObjectA, int iObjectB );
DARKSDK int GetLimbCollision ( int iObjectA, int iLimbA, int iObjectB, int iLimbB );
DARKSDK int GetLimbHit		 ( int iObjectA, int iLimbA, int iObjectB, int iLimbB );
DARKSDK SDK_FLOAT GetCollisionX			( void );
DARKSDK SDK_FLOAT GetCollisionY			( void );
DARKSDK SDK_FLOAT GetCollisionZ			( void );

// Static Expressions
DARKSDK int GetStaticHit ( float fOldX1, float fOldY1, float fOldZ1, float fOldX2, float fOldY2, float fOldZ2, float fNX1,   float fNY1,   float fNZ1,   float fNX2,   float fNY2,   float fNZ2   );
DARKSDK int GetStaticLineOfSight ( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz, float fWidth, float fAccuracy );
DARKSDK int GetStaticRayCast ( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz );
DARKSDK int GetStaticVolumeCast ( float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, float fSize );
DARKSDK SDK_FLOAT GetStaticX ( void );
DARKSDK SDK_FLOAT GetStaticY ( void );
DARKSDK SDK_FLOAT GetStaticZ ( void );
DARKSDK int GetStaticFloor ( void );
DARKSDK int GetStaticColCount ( void );
DARKSDK int GetStaticColValue ( void );
DARKSDK SDK_FLOAT GetStaticLineOfSightX ( void );
DARKSDK SDK_FLOAT GetStaticLineOfSightY ( void );
DARKSDK SDK_FLOAT GetStaticLineOfSightZ ( void );

// Limb
DARKSDK int LimbExist ( int iID, int iLimbID );
DARKSDK float LimbOffsetX    ( int iID, int iLimbID );
DARKSDK float LimbOffsetY    ( int iID, int iLimbID );
DARKSDK float LimbOffsetZ    ( int iID, int iLimbID );
DARKSDK float LimbAngleX     ( int iID, int iLimbID );
DARKSDK float LimbAngleY     ( int iID, int iLimbID );
DARKSDK float LimbAngleZ     ( int iID, int iLimbID );
DARKSDK float LimbPositionX  ( int iID, int iLimbID );
DARKSDK float LimbPositionY  ( int iID, int iLimbID );
DARKSDK float LimbPositionZ  ( int iID, int iLimbID );
DARKSDK float LimbDirectionX ( int iID, int iLimbID );
DARKSDK float LimbDirectionY ( int iID, int iLimbID );
DARKSDK float LimbDirectionZ ( int iID, int iLimbID );
DARKSDK int LimbTexture ( int iID, int iLimbID, int iTextureStage );
DARKSDK int LimbTexture ( int iID, int iLimbID );
DARKSDK int GetLimbTexturePtr ( int iID, int iLimbID );
DARKSDK int LimbVisible ( int iID, int iLimbID );
DARKSDK int LimbLink ( int iID, int iLimbID );
DARKSDK int GetLimbPolygonCount ( int iID, int iLimbID );
DARKSDK int GetMultiMaterialCount ( int iID );
DARKSDK LPSTR LimbTextureName ( int iID, int iLimbID, int iTextureStage );
DARKSDK LPSTR LimbTextureName ( int iID, int iLimbID );

// Misc
DARKSDK int   GetMeshExist				( int iID );
DARKSDK int   GetAlphaBlending			( void );
DARKSDK int   GetBlending				( void );
DARKSDK int   GetFog					( void );
DARKSDK int   Get3DBLITAvailable		( void );
DARKSDK int   GetStatistic				( int iCode );
DARKSDK int   GetTNLAvailable			( void );

// new limb commands
DARKSDK LPSTR	LimbName			  ( int iID, int iLimbID );
DARKSDK int		GetLimbCount		  ( int iID );
DARKSDK float	LimbScaleX			  ( int iID, int iLimbID );
DARKSDK float	LimbScaleY			  ( int iID, int iLimbID );
DARKSDK float	LimbScaleZ			  ( int iID, int iLimbID );

// vertex shader expressions
DARKSDK SDK_BOOL	VertexShaderExist			  ( int iShader );
DARKSDK SDK_BOOL	PixelShaderExist			  ( int iShader );

// hardware information
DARKSDK int			GetDeviceType                 ( void );
DARKSDK SDK_BOOL	GetCalibrateGamma             ( void );
DARKSDK SDK_BOOL	GetRenderWindowed             ( void );
DARKSDK SDK_BOOL    GetFullScreenGamma            ( void );
DARKSDK SDK_BOOL    GetBlitSysOntoLocal           ( void );
DARKSDK SDK_BOOL    GetRenderAfterFlip            ( void );
DARKSDK SDK_BOOL    GetDrawPrimTLVertex           ( void );
DARKSDK SDK_BOOL    GetHWTransformAndLight        ( void );
DARKSDK SDK_BOOL    GetSeparateTextureMemories    ( void );
DARKSDK SDK_BOOL    GetTextureSystemMemory        ( void );
DARKSDK SDK_BOOL    GetTextureVideoMemory         ( void );
DARKSDK SDK_BOOL    GetTextureNonLocalVideoMemory ( void );
DARKSDK SDK_BOOL    GetTLVertexSystemMemory       ( void );
DARKSDK SDK_BOOL    GetTLVertexVideoMemory        ( void );
DARKSDK SDK_BOOL    GetClipAndScalePoints         ( void );
DARKSDK SDK_BOOL    GetClipTLVerts                ( void );
DARKSDK SDK_BOOL    GetColorWriteEnable           ( void );
DARKSDK SDK_BOOL    GetCullCCW                    ( void );
DARKSDK SDK_BOOL    GetCullCW                     ( void );
DARKSDK SDK_BOOL    GetAnisotropicFiltering       ( void );
DARKSDK SDK_BOOL    GetAntiAliasEdges             ( void );
DARKSDK SDK_BOOL    GetColorPerspective           ( void );
DARKSDK SDK_BOOL    GetDither                     ( void );
DARKSDK SDK_BOOL    GetFogRange                   ( void );
DARKSDK SDK_BOOL    GetFogTable                   ( void );
DARKSDK SDK_BOOL    GetFogVertex                  ( void );
DARKSDK SDK_BOOL    GetMipMapLODBias              ( void );
DARKSDK SDK_BOOL    GetWBuffer                    ( void );
DARKSDK SDK_BOOL    GetWFog                       ( void );
DARKSDK SDK_BOOL    GetZFog                       ( void );
DARKSDK SDK_BOOL    GetAlpha                      ( void );
DARKSDK SDK_BOOL    GetCubeMap                    ( void );
DARKSDK SDK_BOOL    GetMipCubeMap                 ( void );
DARKSDK SDK_BOOL    GetMipMap                     ( void );
DARKSDK SDK_BOOL    GetMipMapVolume               ( void );
DARKSDK SDK_BOOL    GetNonPowerOf2Textures        ( void );
DARKSDK SDK_BOOL    GetPerspective                ( void );
DARKSDK SDK_BOOL    GetProjected                  ( void );
DARKSDK SDK_BOOL    GetSquareOnly                 ( void );
DARKSDK SDK_BOOL    GetVolumeMap                  ( void );
DARKSDK SDK_BOOL    GetAlphaComparision           ( void );
DARKSDK SDK_BOOL    GetTexture                    ( void );
DARKSDK SDK_BOOL    GetZBuffer                    ( void );
DARKSDK int			GetMaxTextureWidth            ( void );
DARKSDK int			GetMaxTextureHeight           ( void );
DARKSDK int			GetMaxVolumeExtent            ( void );
DARKSDK SDK_FLOAT	GetVertexShaderVersion        ( void );
DARKSDK SDK_FLOAT	GetPixelShaderVersion         ( void );
DARKSDK int			GetMaxVertexShaderConstants   ( void );
DARKSDK SDK_FLOAT	GetMaxPixelShaderValue        ( void );
DARKSDK int			GetMaxLights                  ( void );

DARKSDK void			SetDeleteCallBack   ( int iID, ON_OBJECT_DELETE_CALLBACK pfn, int userData );
DARKSDK void			SetDisableTransform ( int iID, bool bTransform );
DARKSDK void			CreateMeshForObject ( int iID, DWORD dwFVF, DWORD dwVertexCount, DWORD dwIndexCount );
DARKSDK void			SetWorldMatrix ( int iID, GGMATRIX* pMatrix );
DARKSDK void			UpdatePositionStructure ( sPositionData* pPosition );
DARKSDK void			GetWorldMatrix ( int iID, int iLimb, GGMATRIX* pMatrix );
DARKSDK GGVECTOR3		GetCameraLook     ( void );
DARKSDK GGVECTOR3		GetCameraPosition ( void );
DARKSDK GGVECTOR3		GetCameraUp       ( void );
DARKSDK GGVECTOR3		GetCameraRight    ( void );
DARKSDK GGMATRIX 		GetCameraMatrix   ( void );
DARKSDK sObject*		GetObjectData ( int iID );

// mike - 300905 - command to update object bounds
DARKSDK void			CalculateObjectBounds ( int iID );
DARKSDK void			CalculateObjectBoundsEx ( int iID, int iOnlyUpdateFrames );

// mike - 230505 - need to be able to set mip map LOD bias on a per mesh basis
void		SetObjectMipMapLODBias	( int iID, float fBias );
void		SetObjectMipMapLODBias	( int iID, int iLimb, float fBias );

DARKSDK void SetObjectMask		( int iID, int iMASK );
DARKSDK void AddObjectMask		( int iID, DWORD dwAddMASK );
DARKSDK void SetObjectMask		( int iID, int iMASK, int iShadowMASK );
DARKSDK void SetObjectMask		( int iID, int iMASK, int iShadowMASK, int iCubeMapMASK );
DARKSDK void SetObjectMask		( int iID, int iMASK, int iShadowMASK, int iCubeMapMASK, int iForeColorWipe );

DARKSDK void SetArrayMap		( int iID, int iStage, int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8, int i9, int i10 );
DARKSDK void SetArrayMapEx		( int iID, int iStage, int iSrcObject, int iSrcStage );
DARKSDK void Instance			( int iDestinationID, int iSourceID, int iInstanceValue );
DARKSDK void SetNodeTreeEffect	( int iEffectID );
DARKSDK void DrawSingle			( int iObjectID, int iCameraID );
DARKSDK void ResetStaticLights	( void );
DARKSDK void AddStaticLight		( int iIndex, float fX, float fY, float fZ, float fRange );
DARKSDK void UpdateStaticLights	( void );

// lee - 071108 - u71 - post processing screen commands (from SetupDLL)
DARKSDK void TextureScreen 				( int iStageIndex, int iImageID );
DARKSDK void SetScreenEffect			( int iEffectID );

DARKSDK void SetLegacyMode				( int iUseLegacy );

// from positionc.h

// Internal Functions
DARKSDK float ObjectPositionX ( int iID );
DARKSDK float ObjectPositionY ( int iID );
DARKSDK float ObjectPositionZ ( int iID );
DARKSDK float ObjectAngleX ( int iID );
DARKSDK float ObjectAngleY ( int iID );
DARKSDK float ObjectAngleZ ( int iID );

// DBV1 Euler
DARKSDK void ScaleObject     ( int iID, float fX, float fY, float fZ );
DARKSDK void PositionObject  ( int iID, float fX, float fY, float fZ, int iUpdateAbsWorldImmediately );
DARKSDK void PositionObject  ( int iID, float fX, float fY, float fZ );
DARKSDK void RotateObject    ( int iID, float fX, float fY, float fZ );
DARKSDK void XRotateObject   ( int iID, float fX );
DARKSDK void YRotateObject   ( int iID, float fY );
DARKSDK void ZRotateObject   ( int iID, float fZ );
DARKSDK void PointObject     ( int iID, float fX, float fY, float fZ );
DARKSDK void MoveObject      ( int iID, float fStep );
DARKSDK void MoveObjectUp    ( int iID, float fStep );
DARKSDK void MoveObjectDown  ( int iID, float fStep );
DARKSDK void MoveObjectLeft  ( int iID, float fStep );
DARKSDK void MoveObjectRight ( int iID, float fStep );

// DBV1 Freeflight
DARKSDK void TurnObjectLeft  ( int iID, float fAngle );
DARKSDK void TurnObjectRight ( int iID, float fAngle );
DARKSDK void PitchObjectUp   ( int iID, float fAngle );
DARKSDK void PitchObjectDown ( int iID, float fAngle );
DARKSDK void RollObjectLeft  ( int iID, float fAngle );
DARKSDK void RollObjectRight ( int iID, float fAngle );

// DBV1 Expressions
DARKSDK float ObjectPositionX ( int iID );
DARKSDK float ObjectPositionY ( int iID );
DARKSDK float ObjectPositionZ ( int iID );
DARKSDK void SetCharacterCreatorTones ( int iID, int index, float red, float green, float blue, float mix );

#endif _COBJECTS_H_
