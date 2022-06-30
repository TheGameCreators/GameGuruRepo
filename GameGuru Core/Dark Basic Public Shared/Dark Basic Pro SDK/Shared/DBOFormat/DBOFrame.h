//
// DBOFrame Functions Header
//

#include "preprocessor-flags.h"
#include "..\global.h"

#include "DBOFormat.h"

// Internal Frame Update Functions

DARKSDK_DLL bool	UpdateFrame					( sFrame *pFrame, GGMATRIX *pMatrix );
DARKSDK_DLL void	ResetFrameMatrices			( sFrame* pFrame );
DARKSDK_DLL bool	UpdateAllFrameData			( sObject* pObject, float fTime );
DARKSDK_DLL void	UpdateRealtimeFrameVectors	( sObject* pObject, sFrame* pFrame );
DARKSDK_DLL bool	AppendAnimationDataCore		( sAnimation* pCurrentAnimationToAppend, sAnimationSet** ppAnimationSetToAddTo, int iFrameStart, bool bCompleteLoopWithFrameZeroCopy);
DARKSDK_DLL bool	AppendAnimationFromFile		( sObject* pObject, LPSTR szFilename, int iFrame );
DARKSDK_DLL void	ClearAnimationFromObject	( sObject* pObject);
DARKSDK_DLL void	UpdateUserMatrix			( sFrame* pFrame );
DARKSDK_DLL void    CopyFrameAnimToUserFrame	( sObject* pObject, float fAnimFrame );
DARKSDK_DLL void    SlerpFrameAnimToUserFrame	( sObject* pObject, float fAnimFrame, float fInterpolationTime );

// Frame Basic Functions

DARKSDK_DLL void	Offset						( sFrame* pFrame, float fX, float fY, float fZ );
DARKSDK_DLL void	Rotate						( sFrame* pFrame, float fX, float fY, float fZ );
DARKSDK_DLL void	Scale						( sFrame* pFrame, float fX, float fY, float fZ );

// Frame Hierarchy Construction Functions

DARKSDK_DLL bool	AddNewFrame					( sObject* pObject, sMesh* pMesh, LPSTR pName );
DARKSDK_DLL bool	RemoveFrame					( sObject* pObject, sFrame* pFrame );
DARKSDK_DLL bool	LinkFrame					( sObject* pObject, sFrame* pFrameToMove, sFrame* pFrameToLinkTo );
DARKSDK_DLL bool	ReplaceFrameMesh			( sFrame* pFrame, sMesh* pMesh );

// Frame Shadow Meshes

DARKSDK_DLL bool	CreateShadowFrame			( sFrame* pFrame, int iUseShader );
DARKSDK_DLL bool	UpdateShadowFrame			( sFrame* pFrame, GGMATRIX* pmatWorld, int iLightMode, GGVECTOR3 vecLightPos );

// Frame Bound Meshes

DARKSDK_DLL bool	CreateBoundBoxMesh			( sFrame* pFrame );
DARKSDK_DLL bool	UpdateBoundBoxMesh			( sFrame* pFrame );
DARKSDK_DLL bool	CreateBoundSphereMesh		( sFrame* pFrame );
DARKSDK_DLL bool	UpdateBoundSphereMesh		( sFrame* pFrame );
