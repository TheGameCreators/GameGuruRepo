//
// DBOFormat Functions Implementation
//

#include "DBOFormat.h"
#include "DBOFrame.h"
#include "DBORawMesh.h"
#include "DBOMesh.h"
#include "DBOFile.h"
#include "..\..\..\Include\ConvX.h"
#include "..\..\..\Include\CGfxC.h"
#include "..\Objects\CommonC.h"
#include "DBOAssImp.h"
#include "CFileC.h"

// 291116 - Defined in DBDLLCORE to improve timer precision
DARKSDK float timeGetSecond ( void );

// Prototype for controlled execution of apps
BOOL DB_ExecuteFile(HANDLE* phExecuteFileProcess, char* Operation, char* Filename, char* String, char* Path, bool bWaitForTermination);
int GetExecutableRunning(DWORD pHandle);

#pragma warning(disable : 4786)
#include "Extras\NVMeshMender.h"
#include "mmsystem.h"
#include "direct.h"
#include "string.h"
#include "shellapi.h"
#include ".\..\Objects\CObjectManagerC.h"
#include "..\..\..\Include\CImageC.h"
#include <thread>

// External error helper
extern char g_strErrorClue[512];

extern		LPGGDEVICE							m_pD3D;				// d3d device
extern		LPGGIMMEDIATECONTEXT				m_pImmediateContext;
extern		sEffectConstantData					g_EffectConstant;
extern		sObject**							g_ObjectList;
extern      float								g_fShrinkObjectsTo;
extern		CObjectManager						m_ObjectManager;

// Global
bool		g_bGracefulWarningAboutOldXFiles	= false;
bool		g_bSwitchLegacyOn					= false;
float		g_fLastDeltaTime					= 0.0f;
bool		g_bFastBoundsCalculation			= true;
GGMATRIX	g_matThisViewProj;
GGMATRIX	g_matThisCameraView;
GGMATRIX	g_matPreviousViewProj;

GGPLANE								g_Planes [ 20 ][ NUM_CULLPLANES ];			// list of planes for frustum culling
DBPRO_GLOBAL GGVECTOR3				g_PlaneVector [ 20 ] [ NUM_CULLPLANES ];
DBPRO_GLOBAL sEffectConstantData	g_EffectConstant;							// used to store shader constants data
DWORD								g_dwEffectErrorMsgSize=0;
LPSTR								g_pEffectErrorMsg=NULL;
DWORD*								g_pConversionMap=NULL;						// single conversion array ptr held
char								g_WindowsTempDirectory[_MAX_PATH];
		
//Dave Performance
bool								g_bEarlyExcludeMode = false;

// globals for single thread (X File loading is NOT thread safe, can run in parallel if we know X files are not being loaded elsewhere)
std::thread* g_pT2 = NULL;
bool g_bT2 = false;
bool g_bRequestCleanInteruptionT2 = false;
std::vector<sPreLoadedObjectData> g_object_list;
std::vector<sPreLoadedObjectData> g_object_outputv;

// function to execute thread code
void object_thread_function(const std::vector<sPreLoadedObjectData> &v)
{
	g_bT2 = true;
	// in this thread, load each object data in turn
	g_object_outputv.clear();
	for ( int n = 0; n < v.size(); n++ )
	{
		sPreLoadedObjectData item = v[n];
		if ( item.pData == NULL ) if ( LoadDBODataBlock ( item.pFilename, &item.dwDataSize, (void**)&item.pData ) == false ) strcpy ( item.pFilename, "" );
		g_object_outputv.push_back(item);
		if (g_bRequestCleanInteruptionT2 == true)
		{
			// this flag can be set when we want to interupt this loading thread, and keep
			// what we have up to this point, allowing main thread to continue quickly (stops a possible 12 second pause in some cases!)
			break;
		}
	}
	g_bT2 = false;
}

void object_preload_files_start ( void )
{
	// clear list ready for new files to thread load
	g_object_list.clear();
}

void object_preload_files_add ( LPSTR pFilename )
{
	//Moved here so we can check if its already in the list.
	char *cUseFilename;
	char cResolvePath[MAX_PATH];
	if (GetFullPathNameA(pFilename, MAX_PATH, &cResolvePath[0], NULL) > 0) 
	{
		cUseFilename = &cResolvePath[0];
	}
	else 
	{
		cUseFilename = pFilename;
	}

	// check to make sure we don't add something we already have in the list
	for ( int n = 0; n < g_object_list.size(); n++ )
		if ( stricmp ( g_object_list[n].pFilename, cUseFilename) == NULL )
			return;

	// add item to list of work
	sPreLoadedObjectData item;
	strcpy ( item.pFilename, cUseFilename);
	item.dwDataSize = 0;
	item.pData = NULL;
	g_object_list.push_back(item);
}

void object_preload_files_finish ( void )
{
	// before send list to thread, load up list with previous preloaded file 'data' still in memory
	for ( int n = 0; n < g_object_list.size(); n++ )
	{
		if ( n < g_object_outputv.size() )
		{
			if ( stricmp ( g_object_list[n].pFilename, g_object_outputv[n].pFilename ) == NULL )
			{
				// this ensures we avoid reloading something we 'might need' that we have already preloaded previously
				g_object_list[n].pData = g_object_outputv[n].pData;
				g_object_list[n].dwDataSize = g_object_outputv[n].dwDataSize;
			}
		}
	}

	// start preloading
	g_pT2 = new std::thread(object_thread_function, std::ref(g_object_list));
}

void object_preload_files_strictwaittoend ( void )
{
	// wait for all work to finish
	if ( g_pT2 )
	{
		g_pT2->join();
		delete g_pT2;
		g_pT2 = NULL;
		g_bRequestCleanInteruptionT2 = false;
	}
}


void object_preload_files_wait(void)
{
	g_bRequestCleanInteruptionT2 = true;
	object_preload_files_strictwaittoend();
	
}

void object_preload_files_reset ( void )
{
	// clear finished list for next batch of work
	for ( int n = 0; n < g_object_outputv.size(); n++ )
	{
		if ( g_object_outputv[n].pData ) 
		{
			delete g_object_outputv[n].pData;
			g_object_outputv[n].pData = NULL;
			//g_object_outputv[n].dwDataSize = 0;
		}
	}
	g_object_outputv.clear();
}

bool object_preload_files_in_progress(void)
{
	return g_bT2;
}

///

cSpecialEffect::cSpecialEffect ( )
{
	// reset effect ptr
	memset ( this, 0, sizeof(cSpecialEffect) );
}

cSpecialEffect::~cSpecialEffect ( )
{
	if ( m_dwRTTexCount>0 )
	{
		for ( DWORD t=0; t<m_dwRTTexCount; t++ )
		{
			SAFE_RELEASE ( m_pRTTex[t] );
			SAFE_RELEASE ( m_pRTTexView[t] );
		}
	}

	// free default zfile mesh
	SAFE_DELETE ( m_pXFileMesh );

	// free effect
	SAFE_RELEASE ( m_pEffect );
	SAFE_DELETE ( m_pDefaultXFile );
}

bool cSpecialEffect::CorrectFXFile ( LPSTR pFile, LPSTR pModifiedFile )
{
	// result var
	bool bResult=false;

	// read in original file
	HANDLE hreadfile = GG_CreateFile(pFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hreadfile!=INVALID_HANDLE_VALUE)
	{
		// read file into memory
		DWORD bytesread;
		int filebuffersize = GetFileSize(hreadfile, NULL);	
		char* filebuffer = (char*)GlobalAlloc(GMEM_FIXED, filebuffersize);
		ReadFile(hreadfile, filebuffer, filebuffersize, &bytesread, NULL); 
		CloseHandle(hreadfile);		

		// Items an FX file may be missing
		bool bDCLTokensExist=false;
		bool bInvalidTargetTokenUsed=false;

		// scan and modify for corrections
		LPSTR pPtr = filebuffer;
		LPSTR pPtrEnd = filebuffer+filebuffersize;
		while ( pPtr<pPtrEnd )
		{
			// check for existance of commonly missing data
			if ( _strnicmp ( pPtr, "dcl_", 4 )==NULL ) bDCLTokensExist=true;
			if ( _strnicmp ( pPtr, "target[", 7 )==NULL ) bInvalidTargetTokenUsed=true;

			// next byte
			pPtr++;
		}

		// write corrections in new data
		int newbuffersize = filebuffersize*2;	
		char* newbuffer = (char*)GlobalAlloc(GMEM_FIXED, newbuffersize);
		LPSTR pWritePtr = newbuffer;
		LPSTR pWritePtrEnd = newbuffer+newbuffersize;

		// go through data again
		pPtr = filebuffer;
		while ( pPtr<pPtrEnd )
		{
			// change any VS to include standard declarations
			if ( bDCLTokensExist==false )
			{
				// from vs.1.1 to vs.1.1 dcl_position v0 dcl_normal v3 etc
				if ( _strnicmp ( pPtr, "vs.", 3 )==NULL )
				{
					// vs.x.x
					memcpy ( pWritePtr, pPtr, 6 ); //vs.x.x
					pWritePtr+=6;
					pPtr+=6;

					// add dcls
					memcpy ( pWritePtr, (LPSTR)" dcl_position v0", 16 ); pWritePtr+=16;
					memcpy ( pWritePtr, (LPSTR)" dcl_normal v3", 14 ); pWritePtr+=14;
					memcpy ( pWritePtr, (LPSTR)" dcl_color v6", 13 ); pWritePtr+=13;
					memcpy ( pWritePtr, (LPSTR)" dcl_texcoord v7", 16 ); pWritePtr+=16;
				}
			}

			// change any target tokens by commenting them out
			if ( bInvalidTargetTokenUsed==true )
			{
				if ( _strnicmp ( pPtr, "target[", 7 )==NULL )
				{
					*pWritePtr = '/'; pWritePtr++;
					*pWritePtr = '/'; pWritePtr++;
					pPtr+=2;
				}
			}

			// write from original data to new buffer
			*pWritePtr = *pPtr;

			// next bytes
			pWritePtr++;
			pPtr++;
		}

		// write new temp file
		DWORD actualnewdatasize = pWritePtr-newbuffer;
		HANDLE hwritefile = GG_CreateFile(pModifiedFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hwritefile!=INVALID_HANDLE_VALUE)
		{
			// write new data
			DWORD byteswritten;
			WriteFile(hwritefile, newbuffer, actualnewdatasize, &byteswritten, NULL); 
			CloseHandle(hwritefile);

			// success
			bResult=true;
		}

		// free usages
		if ( filebuffer ) { GlobalFree ( filebuffer ); }
		if ( newbuffer ) { GlobalFree ( newbuffer ); }
	}

	// failed
	return bResult;
}

bool cSpecialEffect::Load ( int iEffectID, LPSTR pEffectFile, bool bUseXFile, bool bUseTextures )
{
	// record effect name
	strcpy ( m_pEffectName, pEffectFile );
	m_iEffectID = iEffectID;

	// split off path and switch for local resource loading
	char pPath[_MAX_PATH];
	char pFile[_MAX_PATH];
	strcpy ( pPath, "" );
	strcpy ( pFile, pEffectFile );
	for ( int n=strlen(pEffectFile); n>0; n--)
	{
		if ( pEffectFile[n]=='\\' ||  pEffectFile[n]=='/' )
		{
			// get path and file
			strcpy ( pFile, pEffectFile+n+1 );
			strcpy ( pPath, pEffectFile );
			pPath[n]=0;
			break;
		}
	}

	#ifdef DX11

	// need blob filename
	char pBlobFilename[1024];
	strcpy ( pBlobFilename, pEffectFile );
	pBlobFilename[strlen(pBlobFilename)-3] = 0;
	strcat ( pBlobFilename, ".blob" );

	// load effect from local file first
	m_pEffect = SETUPLoadShader ( pEffectFile, pBlobFilename, iEffectID );
	if ( m_pEffect == NULL ) 
		return false;

	// Associate data in effect with app data
	ParseEffect ( bUseXFile, bUseTextures );
	m_bUseShaderTextures = bUseTextures;

	// find valid technique (first one)
	m_hCurrentTechnique = m_pEffect->GetTechniqueByIndex(0);

	// complete
	return true;
	#else
	// buffer to hold error
	ID3DXBuffer* pErrorBuffer = NULL;

	// store old directory and set local one
	char pOldDir[_MAX_PATH];
	_getcwd(pOldDir, _MAX_PATH);
	if ( strlen(pPath)>0 ) _chdir(pPath);

	// Shader Legacy Mode for later DXSDK
	DWORD dwShaderLegacyMode = 0;
	#ifdef DARKSDK_COMPILE
        #ifdef D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
		    dwShaderLegacyMode = D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
        #else
			// lee - 131010 - DarkGDK should support The October 2006 SDK (for legacy users)
			// previous DarkGDK builds may have used Aug 2007 (35.DLL)
            // #error You should be using DX SDK Aug 2007 or later
        #endif
	#endif

	// 131213 - improve shader loading for Reloaded
	dwShaderLegacyMode = D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY;
	//dwShaderLegacyMode = D3DXSHADER_SKIPVALIDATION; // 200417 - fixed shaders but no speed increase, ah well

	// 160214 - so I can debug in PIX
	//#if defined( DEBUG ) || defined( _DEBUG )
	//	dwShaderLegacyMode |= D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	//#endif

	// load effect from local file first
	if(FAILED(hr = D3DXCreateEffectFromFileA(m_pD3D, pFile,
					NULL, NULL, dwShaderLegacyMode, NULL, &m_pEffect, &pErrorBuffer )))
	{
		// calculate temp folder location
		DBOCalculateLoaderTempFolder();

		// leefix - 310305 - alter path if in debugmode(plguins last), swiotch to effect
		char pFinalFile[_MAX_PATH];
		strcpy ( pFinalFile, g_WindowsTempDirectory );
		if ( _strnicmp ( "plugins\\", (g_WindowsTempDirectory + strlen(g_WindowsTempDirectory)) - 8, 8 )==NULL ) 
		{
			// switch from plugins to effects (for finding an internal FX file)
			strcpy ( pFinalFile, g_WindowsTempDirectory );
			pFinalFile [ strlen(pFinalFile) - 8 ] = 0;
			strcat ( pFinalFile, "effects\\" );
		}

		// try from dbpdata folder
		strcat ( pFinalFile, pFile );

		// load effect from internal file second
		if(FAILED(hr = D3DXCreateEffectFromFileA(m_pD3D, pFinalFile,
						NULL, NULL, dwShaderLegacyMode, NULL, &m_pEffect, &pErrorBuffer )))
		{
			// third possibility is that it is in the TEMP media folder
			strcpy ( pFinalFile, g_WindowsTempDirectory );
			strcat ( pFinalFile, "media\\" );
			strcat ( pFinalFile, pEffectFile );
			if(FAILED(hr = D3DXCreateEffectFromFileA(m_pD3D, pFinalFile,
							NULL, NULL, dwShaderLegacyMode, NULL, &m_pEffect, &pErrorBuffer )))
			{
				// try from dbpdata folder
				char pModifiedFile[_MAX_PATH];
				strcpy ( pModifiedFile, g_WindowsTempDirectory );
				strcat ( pModifiedFile, "_modified_fx.fx" );

				// forth possibility is that the local FX file cannot be parsed (nvidia)
				if ( CorrectFXFile ( pFile, pModifiedFile ) )
				{
					// load effect from local file first
					if(FAILED(hr = D3DXCreateEffectFromFileA(m_pD3D, pModifiedFile,
									NULL, NULL, dwShaderLegacyMode, NULL, &m_pEffect, &pErrorBuffer )))
					{
						// simply cannot payse the FX file
						DeleteFile ( pModifiedFile );
						goto failed;
					}

					// Remove temp file
					DeleteFile ( pModifiedFile );
				}
				else
				{
					// cannot correct FX file
					goto failed;
				}
			}
			else
			{
				// The TEMP\MEDIA was the right folder, need to switch CWD to it
				strcpy ( pFinalFile, g_WindowsTempDirectory );
				strcat ( pFinalFile, "media\\" );
				strcat ( pFinalFile, pPath );
				_chdir(pFinalFile);
			}
		}
	}

	// Associate data in effect with app data
	ParseEffect ( bUseXFile, bUseTextures );
	m_bUseShaderTextures = bUseTextures;

	// find valid technique
	GGHANDLE hTechnique;
	if(FAILED(hr = m_pEffect->FindNextValidTechnique(NULL, &hTechnique)))
	{
		SAFE_RELEASE(pErrorBuffer);
		_chdir(pOldDir);
		return false;
	}

	// set the first valid technique (DBPro FX files orders best to worst)
	if ( hTechnique )
		m_pEffect->SetTechnique(hTechnique);

	// 091115 - find a pass named 'RenderDepthPixelsPass' and flag if found
	// as we can skip this pass if engine does not use depth related stuff like DOF and MOTION BLUR (performance)
	if ( hTechnique ) m_DepthRenderPassHandle = m_pEffect->GetPassByName ( hTechnique, "RenderDepthPixelsPass");

	// restores
	SAFE_RELEASE(pErrorBuffer);
	_chdir(pOldDir);

	// complete
	return true;

	failed:

	// handle error buffer
	if ( pErrorBuffer ) 
	{
		g_dwEffectErrorMsgSize = pErrorBuffer->GetBufferSize();
		SAFE_DELETE(g_pEffectErrorMsg);
		g_pEffectErrorMsg = new char[g_dwEffectErrorMsgSize+1];
		memcpy ( g_pEffectErrorMsg, pErrorBuffer->GetBufferPointer(), g_dwEffectErrorMsgSize );
		MessageBox ( NULL, g_pEffectErrorMsg, g_pEffectErrorMsg, MB_OK );
	}

	// restores
	SAFE_RELEASE(pErrorBuffer);
	_chdir(pOldDir);
	#endif

	// failure
	return false;
}

bool cSpecialEffect::Setup ( sMesh* pMesh )
{
	// alter mesh with default xfile mesh
	if ( m_pXFileMesh ) MakeLocalMeshFromOtherLocalMesh ( pMesh, m_pXFileMesh );

	// complete
	return true;
}

void cSpecialEffect::Mesh ( sMesh* pMesh )
{
	// change mesh to suit effect
	// leeadd - 200204 - add bone data to mesh FVF if shader requests it
	// leeadd - 121208 - U71 - was (==0) now go into generate if 0 to 2 (first bit zero)
	if ( m_bDoNotGenerateExtraData==0 || m_bDoNotGenerateExtraData==2 )
	{
		// leeadd - 050906 - auto-generate if not flagged off to keep object pure
		if ( pMesh->dwVertexCount > 3 )
		{
			GenerateExtraDataForMeshEx ( pMesh, m_bGenerateNormals, m_bUsesTangents, m_bUsesBinormals, m_bUsesDiffuse, m_bUsesBoneData, m_bDoNotGenerateExtraData );
		}
	}

	// lee - 230306 - u6b4 - also generate a new 'original vertexdata store', otherwise CPU bone animate will be out of phase and crash
	if ( pMesh )
	{
		SAFE_DELETE ( pMesh->pOriginalVertexData );
		CollectOriginalVertexData ( pMesh );
	}
}

DWORD cSpecialEffect::Start	( sMesh* pMesh, GGMATRIX matObject )
{
	// if a valid FX effect
	#ifdef DX11
	if ( m_pEffect )
	{
		// now replace effect textures (usually none) with mesh textures
		#ifdef DX11
		#else
		int iParamTexArrayLimit = m_dwTextureCount;
		if ( iParamTexArrayLimit>32 ) iParamTexArrayLimit=32;
		for ( DWORD t=0; t<(DWORD)iParamTexArrayLimit; t++ )
		{
			int iParam = m_iParamOfTexture[t];
			if ( t<pMesh->dwTextureCount )
			{
				GGHANDLE pParam = m_pEffect->GetParameter( NULL, iParam );
				if ( pParam )
				{
					//D3DXPARAMETER_DESC PDesc; cube map just wont show
					//m_pEffect->GetParameterDesc(pParam, &PDesc );
					LPGGTEXTURE pTexToUse = pMesh->pTextures [ t ].pTexturesRef;
					m_pEffect->SetTexture( pParam, pTexToUse );
				}
			}
		}
		#endif

		// var
		UINT uPasses = 0;

		// begin effect and return number of required passes
		ID3DX11EffectTechnique* pTech = m_hCurrentTechnique;//m_pEffect->GetTechniqueByIndex(0);
		D3DX11_TECHNIQUE_DESC desc;
		pTech->GetDesc(&desc);
		uPasses = desc.Passes;

		// Apply associations to effect using latest application data
		ApplyEffect ( pMesh );

		// passes count
		return (DWORD)uPasses;
	}
	#else
	if ( m_pEffect )
	{
		// now replace effect textures (usually none) with mesh textures
		// LEELEE : Performance warning - is this slow or fast - good flexibility though!
		// LEELEE : This seems to cause massive slowdown when texturing NODETREE meshes!
		// Probably because it is swapping the same texture in the effect many times.
		// Solution is we should be sorting by EFFECT, then by TEXTURE..TODO!
		int iParamTexArrayLimit = m_dwTextureCount;
		if ( iParamTexArrayLimit>32 ) iParamTexArrayLimit=32;
		for ( DWORD t=0; t<(DWORD)iParamTexArrayLimit; t++ )
		{
			int iParam = m_iParamOfTexture[t];
			if ( t<pMesh->dwTextureCount )
			{
				GGHANDLE pParam = m_pEffect->GetParameter( NULL, iParam );
				if ( pParam )
				{
					//D3DXPARAMETER_DESC PDesc; cube map just wont show
					//m_pEffect->GetParameterDesc(pParam, &PDesc );
					LPGGTEXTURE pTexToUse = pMesh->pTextures [ t ].pTexturesRef;
					m_pEffect->SetTexture( pParam, pTexToUse );
				}
			}
		}

		// var
		UINT uPasses;

		// begin effect and return number of required passes
		m_pEffect->Begin(&uPasses, 0 );

		// Apply associations to effect using latest application data
		ApplyEffect ( pMesh );

		// passes count
		return (DWORD)uPasses;
	}
	#endif
	return 0;
}

void cSpecialEffect::End ( void )
{
	// if a valid FX effect
	#ifdef DX11
	#else
	if ( m_pEffect )
	{
		m_pEffect->End();
	}
	#endif
}

bool cSpecialEffect::AssignValueHookCore ( LPSTR pName, GGHANDLE hParam, DWORD dwClass, bool bRemove )
{
	#define ASSIGNNAME(a,b)	if ( bRemove ) { if ( b==hParam ) { b=NULL; return true; } } else { if ( _stricmp ( pName, a )==0 ) { if ( hParam ) { b=hParam; return true; } else { if ( b ) return true;  }; } };

	// auto-fail if no name
	if ( bRemove==false && pName==NULL ) return false;

	// COMMON UNTWEAKABLES
	ASSIGNNAME ( "world", m_MatWorldEffectHandle );
	ASSIGNNAME ( "view", m_MatViewEffectHandle );
	ASSIGNNAME ( "projection", m_MatProjEffectHandle );
	ASSIGNNAME ( "worldview", m_MatWorldViewEffectHandle );
	ASSIGNNAME ( "viewprojection", m_MatViewProjEffectHandle );
	ASSIGNNAME ( "worldviewprojection", m_MatWorldViewProjEffectHandle );

	// MS UNTWEAKABLES
	ASSIGNNAME ( "worldviewit", m_MatWorldViewInverseEffectHandle );
	ASSIGNNAME ( "worldit", m_MatWorldInverseEffectHandle );
	ASSIGNNAME ( "viewit", m_MatViewInverseEffectHandle );

	// NVIDIA UNTWEAKABLES
	ASSIGNNAME ( "WorldInverse", m_MatWorldInverseEffectHandle );
	ASSIGNNAME ( "WorldTranspose", m_MatWorldTEffectHandle );
	ASSIGNNAME ( "WorldInverseTranspose", m_MatWorldInverseTEffectHandle );
	ASSIGNNAME ( "ViewInverse", m_MatViewInverseEffectHandle );
	ASSIGNNAME ( "ViewTranspose", m_MatViewTEffectHandle );
	ASSIGNNAME ( "ViewInverseTranspose", m_MatViewInverseTEffectHandle );
	ASSIGNNAME ( "ProjectionInverse", m_MatProjectionInverseEffectHandle );
	ASSIGNNAME ( "ProjectionTranspose", m_MatProjTEffectHandle );
	ASSIGNNAME ( "WorldViewTranspose", m_MatWorldViewTEffectHandle );
	ASSIGNNAME ( "ViewProjectionTranspose", m_MatViewProjTEffectHandle );
	ASSIGNNAME ( "WorldViewProjectionTranspose", m_MatWorldViewProjTEffectHandle );
	ASSIGNNAME ( "WorldViewInverse", m_MatWorldViewInverseEffectHandle );

	// vectors
	ASSIGNNAME ( "cameraposition", m_VecCameraPosEffectHandle );
	ASSIGNNAME ( "eyeposition", m_VecEyePosEffectHandle );
	ASSIGNNAME ( "clipplane", m_VecClipPlaneEffectHandle );

	// MS lighting
	ASSIGNNAME ( "UIDirectional", m_LightDirHandle );
	ASSIGNNAME ( "UIDirectionalInv", m_LightDirInvHandle );
	ASSIGNNAME ( "UIPosition", m_LightPosHandle );

	// NVIDIA lighting
	ASSIGNNAME ( "directionalight", m_LightDirHandle );
	ASSIGNNAME ( "pointlight", m_LightPosHandle );
	ASSIGNNAME ( "spotlight", m_LightPosHandle );

	// rogue scalars
	ASSIGNNAME ( "time", m_TimeEffectHandle );
	ASSIGNNAME ( "sintime", m_SinTimeEffectHandle );
	ASSIGNNAME ( "deltatime", m_DeltaTimeEffectHandle );
	ASSIGNNAME ( "uvscaling", m_UVScalingHandle );
	
	//
	// DBPRO UNTWEAKABLES
	//

	ASSIGNNAME ( "alphaoverride", m_AlphaOverrideHandle );
	///ASSIGNNAME ( "bonecount", m_BoneCountHandle );
	ASSIGNNAME ( "bonematrixpalette", m_BoneMatrixPaletteHandle );

	// non-handle hook values
	if ( pName )
		if ( _strcmpi ( pName, "xfile" )==0 )
			return true;

	// could not find name match
	return false;
}

bool cSpecialEffect::AssignValueHook ( LPSTR pName, GGHANDLE hParam )
{
	return AssignValueHookCore ( pName, hParam, 0, false );
}

bool cSpecialEffect::ParseEffect ( bool bUseEffectXFile, bool bUseEffectTextures )
{
	// if no effect, skip
	if( m_pEffect == NULL )
		return false;

	// Used to assign from mesh textures
	m_dwTextureCount = 0;

	// get effect description
	m_pEffect->GetDesc( &m_EffectDesc );

	// u64 - 180107 - new mask to hold dynamic tecture flags
	m_dwUseDynamicTextureMask = 0; // default is effect uses NO dynamic textures
	DWORD dwCountTexturesInEffect = 0;

	// U75 - 200310 - clear RT mask as well
	m_dwCreatedRTTextureMask = 0; 
	m_bUsesAtLeastOneRT = false;

	// Look at parameters for semantics and annotations that we know how to interpret
	#ifdef DX11
	GGHANDLE hParam;
	D3DX11_EFFECT_VARIABLE_DESC ParamDesc;
	UINT iParametersCount = m_EffectDesc.GlobalVariables;
	for( UINT iParam = 0; iParam < iParametersCount; iParam++ )
	{
		// temp vars
		LPCSTR pstrName = NULL;
		LPCSTR pstrFunction = NULL;
		LPCSTR pstrTarget = NULL;
		LPCSTR pstrTextureType = NULL;
		INT Width = 0;
		INT Height= 0;
		INT Depth = 0;

		// get this parameter handle and description
		hParam = m_pEffect->GetVariableByIndex(iParam);
		hParam->GetDesc ( &ParamDesc );

		// annotations are associated with global variables
	    GGHANDLE hAnnot = NULL;
		/*
		// light handles from DX9 FX files
	    hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIDirectional" );
        if( hAnnot != NULL ) AssignValueHook ( "UIDirectional", hParam );
	    hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIDirectionalInv" );
        if( hAnnot != NULL ) AssignValueHook ( "UIDirectionalInv", hParam );
	    hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIPosition" );
        if( hAnnot != NULL ) AssignValueHook ( "UIPosition", hParam );

		// light handles from NVIDIA FX files
	    hAnnot = m_pEffect->GetAnnotationByName( hParam, "Object" );
	    if( hAnnot == NULL ) hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIObject" );
        if( hAnnot != NULL )
		{
			// light type
			LPCSTR pstrLightType = NULL;
			if( hAnnot != NULL ) m_pEffect->GetString( hAnnot, &pstrLightType );

			// light space
			LPCSTR pstrLightSpace = NULL;
		    hAnnot = m_pEffect->GetAnnotationByName( hParam, "Space" );
			if( hAnnot != NULL ) m_pEffect->GetString( hAnnot, &pstrLightSpace );

			// assign light position hanle
			AssignValueHook ( (char*)pstrLightType, hParam );
		}
		*/

		// get type of variable
		ID3DX11EffectType* pEffectType = hParam->GetType();
		D3DX11_EFFECT_TYPE_DESC typedesc;
		pEffectType->GetDesc(&typedesc);

		// basic matrix semantics
		if( ParamDesc.Semantic != NULL && ( typedesc.Type == D3D10_SVC_MATRIX_COLUMNS | typedesc.Type == D3D10_SVC_MATRIX_ROWS ) )
			AssignValueHookCore ( (char*)ParamDesc.Semantic, hParam, 0, false );

		// basic vector semantics
		if( ParamDesc.Semantic != NULL && ( typedesc.Type == D3D10_SVC_VECTOR ))
			AssignValueHook ( (char*)ParamDesc.Semantic, hParam );

		// basic value semantics
		if( ParamDesc.Semantic != NULL && ( typedesc.Type == D3D10_SVC_SCALAR ))
			AssignValueHook ( (char*)ParamDesc.Semantic, hParam );

		// go through any annotations associated with variable
		D3DX11_EFFECT_VARIABLE_DESC AnnotDesc;
		for( UINT iAnnot = 0; iAnnot < ParamDesc.Annotations; iAnnot++ )
		{
			// get annotation description
			hAnnot = hParam->GetAnnotationByIndex(iAnnot);
			hAnnot->GetDesc(&AnnotDesc);

			// texture name
			if ( _strcmpi( AnnotDesc.Name, "resourcename" ) == 0 )	hAnnot->AsString()->GetString( &pstrName );
			if ( _strcmpi( AnnotDesc.Name, "name" ) == 0 )			hAnnot->AsString()->GetString( &pstrName );
			if ( _strcmpi( AnnotDesc.Name, "file" ) == 0 )			hAnnot->AsString()->GetString( &pstrName );

			// texture type
			if ( _strcmpi( AnnotDesc.Name, "resourcetype" ) == 0 )	hAnnot->AsString()->GetString( &pstrTextureType );
			if ( _strcmpi( AnnotDesc.Name, "type" ) == 0 )			hAnnot->AsString()->GetString( &pstrTextureType );
			if ( _strcmpi( AnnotDesc.Name, "texturetype" ) == 0 )	hAnnot->AsString()->GetString( &pstrTextureType );

			// texture details
			if ( _strcmpi( AnnotDesc.Name, "function" ) == 0 )		hAnnot->AsString()->GetString( &pstrFunction );
			if ( _strcmpi( AnnotDesc.Name, "target" ) == 0 )		hAnnot->AsString()->GetString( &pstrTarget );
			if ( _strcmpi( AnnotDesc.Name, "width" ) == 0 )			hAnnot->AsScalar()->GetInt ( &Width );
			if ( _strcmpi( AnnotDesc.Name, "height" ) == 0 )		hAnnot->AsScalar()->GetInt ( &Height );
			if ( _strcmpi( AnnotDesc.Name, "depth" ) == 0 )			hAnnot->AsScalar()->GetInt ( &Depth );
		}
		if( pstrName != NULL )
		{
			// detect RENDERCOLORTARGET semantic here
			bool bIsThisAnRT = false;
			if ( ParamDesc.Semantic != NULL ) if ( _strcmpi ( ParamDesc.Semantic, "RENDERCOLORTARGET" ) == 0 ) bIsThisAnRT = true;
			if ( bIsThisAnRT==true )
			{
				// this indicates an RT texture we want our shader to render to during the passes, so we need to create a render target for it
				IGGTexture* pRTTex = NULL;
				LPGGSHADERRESOURCEVIEW pRTTexView = NULL;
				GGSURFACE_DESC desc;
				g_pGlob->pCurrentBitmapSurface->GetDesc(&desc);
				if ( Width == 0 ) Width = desc.Width;
				if ( Height == 0 ) Height = desc.Height;
				GGSURFACE_DESC StagedDesc = { Width, Height, 1, 1, GGFMT_A8R8G8B8, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, 0 };
				m_pD3D->CreateTexture2D( &StagedDesc, NULL, (ID3D11Texture2D**)&pRTTex );

				// shader resource view
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
				ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
				shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Format = GGFMT_A8R8G8B8;
				shaderResourceViewDesc.Texture2D.MipLevels = 1;
				m_pD3D->CreateShaderResourceView ( pRTTex, &shaderResourceViewDesc, &pRTTexView );
				hParam->AsShaderResource()->SetResource ( pRTTexView );

				// render target view
				ID3D11RenderTargetView* pRTTexRenderView = NULL;
				m_pD3D->CreateRenderTargetView( pRTTex, NULL, &pRTTexRenderView );

				// set flag to indicate this specialeffect object uses at least one RT (render target)
				m_bUsesAtLeastOneRT = true;

				// mark in a bitfield which textures are RT (so we can release them when this shader is deleted)
				DWORD dwCorrectBitForThisTexture = 1 << m_dwTextureCount;
				m_dwCreatedRTTextureMask = m_dwCreatedRTTextureMask | dwCorrectBitForThisTexture;

				// record this texture and step through texture indexes
				if ( m_dwTextureCount<=31 ) 
				{
					m_iParamOfTexture [ m_dwTextureCount ] = iParam;
					m_pParamOfTextureRenderView [ m_dwTextureCount ] = pRTTexRenderView;
				}
				m_dwTextureCount++;

				// record this now for later release
				if ( m_dwRTTexCount<=31 ) 
				{
					m_pRTTex [ m_dwRTTexCount ] = pRTTex;
					m_pRTTexView [ m_dwRTTexCount ] = pRTTexView;
				}
				m_dwRTTexCount++;
			}
			else
			{
				/*
				// texture from effect or mesh
				if ( bUseEffectTextures )
				{
					// texture holder
					LPGGBASETEXTURE pTex = NULL;

					// 2D texture is stadnard texture
					if (pstrTextureType != NULL) 
						if( _strcmpi( pstrTextureType, "2d" ) == 0 )
							pstrTextureType=NULL;

					// assign effect texture from FX file
					if (pstrTextureType != NULL) 
					{
						if( _strcmpi( pstrTextureType, "volume" ) == 0 )
						{
							// support for internal volume textures
							LPDIRECT3DVOLUMETEXTURE9 pVolumeTex = NULL;
							if( SUCCEEDED( D3DXCreateVolumeTextureFromFileEx( m_pD3D, pstrName, 
								Width, Height, Depth, 1, 0, GGFMT_UNKNOWN, D3DPOOL_MANAGED,
								D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pVolumeTex ) ) )
							{
								// iTextureStage
								pTex = pVolumeTex;
							}
						}
						else if( _strcmpi( pstrTextureType, "cube" ) == 0 )
						{
							// support for internal cube textures
							LPGGCUBETEXTURE pCubeTex = NULL;
							if( SUCCEEDED( D3DXCreateCubeTextureFromFileEx( m_pD3D, pstrName, 
								Width, D3DX_DEFAULT, 0, GGFMT_UNKNOWN, D3DPOOL_MANAGED,
								D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pCubeTex ) ) )
							{
								// iTextureStage
								pTex = pCubeTex;
							}
						}

						// record this now for later release
						if ( m_dwRTTexCount<=31 ) m_pRTTex [ m_dwRTTexCount ] = pTex;
						m_dwRTTexCount++;
					}
					else
					{
						// support for internal basic textures
						int iImageIndex = LoadOrFindTextureAsImage ( (char*)pstrName, "" );
						pTex = GetImagePointer ( iImageIndex );
					}

					// assign texture to effect
					if ( pTex )
					{
						// assigns effect texture directly to effect
						m_pEffect->SetTexture( m_pEffect->GetParameter( NULL, iParam ), pTex );
					}
					else
					{
						// u64 - 180107 - set the bit to say this texture stage should use a dynamic texture from texture object command
						DWORD dwCorrectBitForThisStage = 1 << dwCountTexturesInEffect;
						m_dwUseDynamicTextureMask = m_dwUseDynamicTextureMask | dwCorrectBitForThisStage;
					}
					dwCountTexturesInEffect++;
				}
				else
				{
					// record this texture and step through texture indexes
					if ( m_dwTextureCount<=31 ) m_iParamOfTexture [ m_dwTextureCount ] = iParam;
					m_dwTextureCount++;
				}
				*/
			}
		}
	}
	#else
	UINT iParametersCount = m_EffectDesc.Parameters;
	D3DXPARAMETER_DESC ParamDesc;
	GGHANDLE hParam;
	for( UINT iParam = 0; iParam < iParametersCount; iParam++ )
	{
		// temp vars
		LPCSTR pstrName = NULL;
		LPCSTR pstrFunction = NULL;
		LPCSTR pstrTarget = NULL;
		LPCSTR pstrTextureType = NULL;
		INT Width = D3DX_DEFAULT;
		INT Height= D3DX_DEFAULT;
		INT Depth = D3DX_DEFAULT;

		// get this parameter handle and description
		hParam = m_pEffect->GetParameter ( NULL, iParam );
		m_pEffect->GetParameterDesc( hParam, &ParamDesc );

		// light handles from DX9 FX files
	    GGHANDLE hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIDirectional" );
        if( hAnnot != NULL ) AssignValueHook ( "UIDirectional", hParam );
	    hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIDirectionalInv" );
        if( hAnnot != NULL ) AssignValueHook ( "UIDirectionalInv", hParam );
	    hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIPosition" );
        if( hAnnot != NULL ) AssignValueHook ( "UIPosition", hParam );

		// light handles from NVIDIA FX files
	    hAnnot = m_pEffect->GetAnnotationByName( hParam, "Object" );
	    if( hAnnot == NULL ) hAnnot = m_pEffect->GetAnnotationByName( hParam, "UIObject" );
        if( hAnnot != NULL )
		{
			// light type
			LPCSTR pstrLightType = NULL;
			if( hAnnot != NULL ) m_pEffect->GetString( hAnnot, &pstrLightType );

			// light space
			LPCSTR pstrLightSpace = NULL;
		    hAnnot = m_pEffect->GetAnnotationByName( hParam, "Space" );
			if( hAnnot != NULL ) m_pEffect->GetString( hAnnot, &pstrLightSpace );

			// assign light position hanle
			AssignValueHook ( (char*)pstrLightType, hParam );
		}

		// basic matrix semantics
		if( ParamDesc.Semantic != NULL && ( ParamDesc.Class == D3DXPC_MATRIX_ROWS || ParamDesc.Class == D3DXPC_MATRIX_COLUMNS ) )
			AssignValueHookCore ( (char*)ParamDesc.Semantic, hParam, ParamDesc.Class, false );

		// basic vector semantics
		if( ParamDesc.Semantic != NULL && ( ParamDesc.Class == D3DXPC_VECTOR ))
			AssignValueHook ( (char*)ParamDesc.Semantic, hParam );

		// basic value semantics
		if( ParamDesc.Semantic != NULL && ( ParamDesc.Class == D3DXPC_SCALAR ))
			AssignValueHook ( (char*)ParamDesc.Semantic, hParam );

		D3DXPARAMETER_DESC AnnotDesc;
		for( UINT iAnnot = 0; iAnnot < ParamDesc.Annotations; iAnnot++ )
		{
			hAnnot = m_pEffect->GetAnnotation ( hParam, iAnnot );
			m_pEffect->GetParameterDesc( hAnnot, &AnnotDesc );

			// texture name
			if ( _strcmpi( AnnotDesc.Name, "resourcename" ) == 0 )		m_pEffect->GetString( hAnnot, &pstrName );
			if ( _strcmpi( AnnotDesc.Name, "name" ) == 0 )		m_pEffect->GetString( hAnnot, &pstrName );
			if ( _strcmpi( AnnotDesc.Name, "file" ) == 0 )		m_pEffect->GetString( hAnnot, &pstrName );

			// texture type
			if ( _strcmpi( AnnotDesc.Name, "resourcetype" ) == 0 )		m_pEffect->GetString( hAnnot, &pstrTextureType );
			if ( _strcmpi( AnnotDesc.Name, "type" ) == 0 )		m_pEffect->GetString( hAnnot, &pstrTextureType );
			if ( _strcmpi( AnnotDesc.Name, "texturetype" ) == 0 )m_pEffect->GetString( hAnnot, &pstrTextureType );

			// texture details
			if ( _strcmpi( AnnotDesc.Name, "function" ) == 0 )	m_pEffect->GetString( hAnnot, &pstrFunction );
			if ( _strcmpi( AnnotDesc.Name, "target" ) == 0 )		m_pEffect->GetString( hAnnot, &pstrTarget );
			if ( _strcmpi( AnnotDesc.Name, "width" ) == 0 )		m_pEffect->GetInt( hAnnot, &Width );
			if ( _strcmpi( AnnotDesc.Name, "height" ) == 0 )		m_pEffect->GetInt( hAnnot, &Height );
			if ( _strcmpi( AnnotDesc.Name, "depth" ) == 0 )		m_pEffect->GetInt( hAnnot, &Depth );
		}
		if( pstrName != NULL )
		{
			// U75 - 200310 - detect RENDERCOLORTARGET semantic here
			bool bIsThisAnRT = false;
			if ( ParamDesc.Semantic != NULL ) if ( _strcmpi ( ParamDesc.Semantic, "RENDERCOLORTARGET" ) == 0 ) bIsThisAnRT = true;
			if ( bIsThisAnRT==true )
			{
				// this indicates an RT texture we want our shader to render to during the passes, so we need to create a render target for it
				IGGTexture* pRTTex = NULL;
				D3DSURFACE_DESC desc;
				IGGSurface *pCurrentRenderTarget = NULL;
				m_pD3D->GetRenderTarget(0,&pCurrentRenderTarget);
				if ( pCurrentRenderTarget )
				{
					pCurrentRenderTarget->GetDesc( &desc );
					if ( Width==D3DX_DEFAULT ) Width=desc.Width;
					if ( Height==D3DX_DEFAULT ) Height=desc.Height;
				}
				else
				{
					if ( Width==D3DX_DEFAULT ) Width=256;
					if ( Height==D3DX_DEFAULT ) Height=256;
				}
				D3DXCreateTexture( m_pD3D, Width, Height, 1, GGUSAGE_RENDERTARGET, GGFMT_A8R8G8B8, D3DPOOL_DEFAULT, (IGGTexture**)&pRTTex );

				// assigns RT texture directly to effect
				m_pEffect->SetTexture( m_pEffect->GetParameter( NULL, iParam ), pRTTex );

				// set flag to indicate this specialeffect object uses at least one RT (render target)
				m_bUsesAtLeastOneRT = true;

				// mark in a bitfield which textures are RT (so we can release them when this shader is deleted)
				DWORD dwCorrectBitForThisTexture = 1 << m_dwTextureCount;
				m_dwCreatedRTTextureMask = m_dwCreatedRTTextureMask | dwCorrectBitForThisTexture;

				// record this texture and step through texture indexes
				if ( m_dwTextureCount<=31 ) m_iParamOfTexture [ m_dwTextureCount ] = iParam;
				m_dwTextureCount++;

				// record this now for later release
				if ( m_dwRTTexCount<=31 ) m_pRTTex [ m_dwRTTexCount ] = pRTTex;
				m_dwRTTexCount++;
			}
			else
			{
				// texture from effect or mesh
				if ( bUseEffectTextures )
				{
					// texture holder
					LPGGBASETEXTURE pTex = NULL;

					// 2D texture is stadnard texture
					if (pstrTextureType != NULL) 
						if( _strcmpi( pstrTextureType, "2d" ) == 0 )
							pstrTextureType=NULL;

					// assign effect texture from FX file
					if (pstrTextureType != NULL) 
					{
						if( _strcmpi( pstrTextureType, "volume" ) == 0 )
						{
							// support for internal volume textures
							LPDIRECT3DVOLUMETEXTURE9 pVolumeTex = NULL;
							if( SUCCEEDED( D3DXCreateVolumeTextureFromFileEx( m_pD3D, pstrName, 
								Width, Height, Depth, 1, 0, GGFMT_UNKNOWN, D3DPOOL_MANAGED,
								D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pVolumeTex ) ) )
							{
								// iTextureStage
								pTex = pVolumeTex;
							}
						}
						else if( _strcmpi( pstrTextureType, "cube" ) == 0 )
						{
							// support for internal cube textures
							LPGGCUBETEXTURE pCubeTex = NULL;
							if( SUCCEEDED( D3DXCreateCubeTextureFromFileEx( m_pD3D, pstrName, 
								Width, D3DX_DEFAULT, 0, GGFMT_UNKNOWN, D3DPOOL_MANAGED,
								D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pCubeTex ) ) )
							{
								// iTextureStage
								pTex = pCubeTex;
							}
						}

						// record this now for later release
						if ( m_dwRTTexCount<=31 ) m_pRTTex [ m_dwRTTexCount ] = pTex;
						m_dwRTTexCount++;
					}
					else
					{
						// support for internal basic textures
						int iImageIndex = LoadOrFindTextureAsImage ( (char*)pstrName, "" );
						pTex = GetImagePointer ( iImageIndex );
					}

					// assign texture to effect
					if ( pTex )
					{
						// assigns effect texture directly to effect
						m_pEffect->SetTexture( m_pEffect->GetParameter( NULL, iParam ), pTex );
					}
					else
					{
						// u64 - 180107 - set the bit to say this texture stage should use a dynamic texture from texture object command
						DWORD dwCorrectBitForThisStage = 1 << dwCountTexturesInEffect;
						m_dwUseDynamicTextureMask = m_dwUseDynamicTextureMask | dwCorrectBitForThisStage;
					}
					dwCountTexturesInEffect++;
				}
				else
				{
					// record this texture and step through texture indexes
					if ( m_dwTextureCount<=31 ) m_iParamOfTexture [ m_dwTextureCount ] = iParam;
					m_dwTextureCount++;
				}
			}
		}
	}
	#endif

	#ifdef DX11
	#else
	// Look for default mesh
	if ( bUseEffectXFile )
	{
		D3DXPARAMETER_DESC Desc;
		if( NULL != m_pEffect->GetParameterByName( NULL, "XFile" ) &&
			SUCCEEDED( m_pEffect->GetParameterDesc( "XFile", &Desc ) ) )
		{
			// Store default xfile name
			const char* pStr = NULL;
			m_pEffect->GetString("XFile", &pStr);
			if ( pStr )
			{
				// get default xfile name
				m_pDefaultXFile = new char[strlen(pStr)+1];
				strcpy ( m_pDefaultXFile, pStr );

				// change current mesh with
				LoadRawMesh ( m_pDefaultXFile, &m_pXFileMesh );
			}
		}
	}
	#endif

	// Look for normals/diffuse/tangents/binormals semantic
	#ifdef DX11
	GGEFFECT_DESC EffectDesc;
	ID3DX11EffectTechnique* hTechnique;
	D3DX11_TECHNIQUE_DESC TechniqueDesc;
	ID3DX11EffectPass* hPass;
	m_bUsesNormals = FALSE;
	m_bUsesDiffuse = FALSE;
	m_bUsesTangents = FALSE;
	m_bUsesBinormals = FALSE;
	if ( m_BoneMatrixPaletteHandle )
		m_bUsesBoneData = TRUE;
	else
		m_bUsesBoneData = FALSE;

	m_pEffect->GetDesc( &EffectDesc );
	for( UINT iTech = 0; iTech < EffectDesc.Techniques; iTech++ )
	{
		hTechnique = m_pEffect->GetTechniqueByIndex( iTech );
		hTechnique->GetDesc ( &TechniqueDesc );
		for( UINT iPass = 0; iPass < TechniqueDesc.Passes; iPass++ )
		{
			hPass = hTechnique->GetPassByIndex ( iPass );
			D3DX11_PASS_SHADER_DESC vs_desc;
			hPass->GetVertexShaderDesc(&vs_desc);
			D3DX11_EFFECT_SHADER_DESC s_desc;
			vs_desc.pShaderVariable->GetShaderDesc(0, &s_desc);
            UINT NumVSSemanticsUsed = s_desc.NumInputSignatureEntries;
			for( UINT iSem = 0; iSem < NumVSSemanticsUsed; iSem++ )
			{
				D3D11_SIGNATURE_PARAMETER_DESC pSigParDesc;
				vs_desc.pShaderVariable->GetInputSignatureElementDesc ( 0, iSem, &pSigParDesc );
				if( stricmp ( pSigParDesc.SemanticName, "NORMAL" ) == NULL ) m_bUsesNormals = TRUE;
				if( stricmp ( pSigParDesc.SemanticName, "COLOR" ) == NULL ) m_bUsesDiffuse = TRUE;
				if( stricmp ( pSigParDesc.SemanticName, "TANGENT" ) == NULL ) m_bUsesTangents = TRUE;
				if( stricmp ( pSigParDesc.SemanticName, "BINORMAL" ) == NULL ) m_bUsesBinormals = TRUE;
			}
		}
	}
	#else
	GGEFFECT_DESC EffectDesc;
	GGHANDLE hTechnique;
	D3DXTECHNIQUE_DESC TechniqueDesc;
	GGHANDLE hPass;
	D3DXPASS_DESC PassDesc;
	m_bUsesNormals = FALSE;
	m_bUsesDiffuse = FALSE;
	m_bUsesTangents = FALSE;
	m_bUsesBinormals = FALSE;
	if ( m_BoneMatrixPaletteHandle )
		m_bUsesBoneData = TRUE;
	else
		m_bUsesBoneData = FALSE;

	m_pEffect->GetDesc( &EffectDesc );
	for( UINT iTech = 0; iTech < EffectDesc.Techniques; iTech++ )
	{
		hTechnique = m_pEffect->GetTechnique( iTech );
		m_pEffect->GetTechniqueDesc( hTechnique, &TechniqueDesc );
		for( UINT iPass = 0; iPass < TechniqueDesc.Passes; iPass++ )
		{
			hPass = m_pEffect->GetPass( hTechnique, iPass );
			m_pEffect->GetPassDesc( hPass, &PassDesc );

            UINT NumVSSemanticsUsed;
            D3DXSEMANTIC pVSSemantics[MAXD3DDECLLENGTH];

            #ifndef __GNUC__
            if( !PassDesc.pVertexShaderFunction || FAILED( D3DXGetShaderInputSemantics( PassDesc.pVertexShaderFunction, pVSSemantics, &NumVSSemanticsUsed ) ) )
                continue;

			for( UINT iSem = 0; iSem < NumVSSemanticsUsed; iSem++ )
			{
				if( pVSSemantics[iSem].Usage == GGDECLUSAGE_NORMAL ) m_bUsesNormals = TRUE;
				if( pVSSemantics[iSem].Usage == D3DDECLUSAGE_COLOR ) m_bUsesDiffuse = TRUE;
				if( pVSSemantics[iSem].Usage == D3DDECLUSAGE_TANGENT ) m_bUsesTangents = TRUE;
				if( pVSSemantics[iSem].Usage == D3DDECLUSAGE_BINORMAL ) m_bUsesBinormals = TRUE;
			}
			#endif
		}
	}
	#endif

	// complete
	return true;
}

void cSpecialEffect::ApplyEffect ( sMesh* pMesh )
{
	// Gather and calculate required constants data
    GGGetTransform( GGTS_WORLD, &g_EffectConstant.matWorld );
    GGGetTransform( GGTS_VIEW, &g_EffectConstant.matView );
    GGGetTransform( GGTS_PROJECTION, &g_EffectConstant.matProj );
    g_EffectConstant.matWorldView = g_EffectConstant.matWorld * g_EffectConstant.matView;
    g_EffectConstant.matViewProj = g_EffectConstant.matView * g_EffectConstant.matProj;
    g_EffectConstant.matWorldViewProj = g_EffectConstant.matWorld * g_EffectConstant.matView * g_EffectConstant.matProj;

	// Calculate inverse matrices
	GGMatrixInverse( &g_EffectConstant.matWorldInv, NULL, &g_EffectConstant.matWorld );
	GGMatrixInverse( &g_EffectConstant.matViewInv, NULL, &g_EffectConstant.matView );
	GGMatrixInverse( &g_EffectConstant.matProjInv, NULL, &g_EffectConstant.matProj );
	GGMatrixInverse( &g_EffectConstant.matWorldViewInv, NULL, &g_EffectConstant.matWorldView );

	// Get raw light data
	#ifdef DX11
	#else
	D3DLIGHT9 d3dLight;
	m_pD3D->GetLight(0,&d3dLight);
	if ( d3dLight.Type==GGLIGHT_DIRECTIONAL )
	{
		// get direction directly from structure
	    g_EffectConstant.vecLightDir = GGVECTOR4( d3dLight.Direction.x, d3dLight.Direction.y, d3dLight.Direction.z, 0.0f );
	    g_EffectConstant.vecLightPos = GGVECTOR4( d3dLight.Direction.x*-1000.0f, d3dLight.Direction.y*-1000.0f, d3dLight.Direction.z*-1000.0f, 1.0f );
	}
	else
	{
		// calculate direction from world position
		g_EffectConstant.vecLightDir.x = g_EffectConstant.matWorld._41-d3dLight.Position.x;
		g_EffectConstant.vecLightDir.y = g_EffectConstant.matWorld._42-d3dLight.Position.y;
		g_EffectConstant.vecLightDir.z = g_EffectConstant.matWorld._43-d3dLight.Position.z;
		g_EffectConstant.vecLightDir.w = 1.0f;

		// calculate light position (in object space)
	    g_EffectConstant.vecLightPos.x = d3dLight.Position.x;
	    g_EffectConstant.vecLightPos.y = d3dLight.Position.y;
	    g_EffectConstant.vecLightPos.z = d3dLight.Position.z;
	    g_EffectConstant.vecLightPos.w = 1.0f;
	}
	#endif

	// Calculate light and object-space light(inv)
	GGVec4Transform ( &g_EffectConstant.vecLightDirInv, &g_EffectConstant.vecLightDir, &g_EffectConstant.matWorldInv );
	GGVec4Normalize ( &g_EffectConstant.vecLightDirInv, &g_EffectConstant.vecLightDirInv );
	GGVec4Normalize ( &g_EffectConstant.vecLightDir, &g_EffectConstant.vecLightDir );

	// Get camera psition
    g_EffectConstant.vecCameraPosition = GGVECTOR4( g_EffectConstant.matViewInv._41, g_EffectConstant.matViewInv._42, g_EffectConstant.matViewInv._43, 1.0f );
	g_EffectConstant.vecEyePos = g_EffectConstant.vecCameraPosition;

	// Alpha override component
	if ( m_AlphaOverrideHandle )
	{
		float fPercentage = 1.0f;
		if ( pMesh->bAlphaOverride==true )
			fPercentage = (float)(pMesh->dwAlphaOverride>>24)/255.0f;

		GGSetEffectFloat( m_AlphaOverrideHandle, fPercentage );
	}

	// prepare tranposed matrices for column major matrices
	if ( m_bTranposeToggle )
	{
		GGMatrixTranspose( &g_EffectConstant.matWorld, &g_EffectConstant.matWorld );
		GGMatrixTranspose( &g_EffectConstant.matView, &g_EffectConstant.matView );
		GGMatrixTranspose( &g_EffectConstant.matProj, &g_EffectConstant.matProj );
		GGMatrixTranspose( &g_EffectConstant.matWorldView, &g_EffectConstant.matWorldView );
		GGMatrixTranspose( &g_EffectConstant.matViewProj, &g_EffectConstant.matViewProj );
		GGMatrixTranspose( &g_EffectConstant.matWorldViewProj, &g_EffectConstant.matWorldViewProj );
		GGMatrixTranspose( &g_EffectConstant.matWorldInv, &g_EffectConstant.matWorldInv );
		GGMatrixTranspose( &g_EffectConstant.matViewInv, &g_EffectConstant.matViewInv );
		GGMatrixTranspose( &g_EffectConstant.matProjInv, &g_EffectConstant.matProjInv );
		GGMatrixTranspose( &g_EffectConstant.matWorldViewInv, &g_EffectConstant.matWorldViewInv );
	}

	// leeadd - 290104 - addition of tranposed matrices for effects that use them
	GGMatrixTranspose( &g_EffectConstant.matWorldT, &g_EffectConstant.matWorld );
	GGMatrixTranspose( &g_EffectConstant.matViewT, &g_EffectConstant.matView );
	GGMatrixTranspose( &g_EffectConstant.matProjT, &g_EffectConstant.matProj );
	GGMatrixTranspose( &g_EffectConstant.matWorldInvT, &g_EffectConstant.matWorldInv );
	GGMatrixTranspose( &g_EffectConstant.matViewInvT, &g_EffectConstant.matViewInv );
	GGMatrixTranspose( &g_EffectConstant.matWorldViewInvT, &g_EffectConstant.matWorldViewInv );

	// 270515 - for depth texture motion blur we need the previous worldviewproj
	// and related matrices (from end of frame NOT from this post process camera!)
	g_EffectConstant.matWorldViewT = g_matThisViewProj;
	g_EffectConstant.matViewProjT = g_matThisCameraView;
	g_EffectConstant.matWorldViewProjT = g_matPreviousViewProj;

	// apply latest data to effect
    if( m_pEffect != NULL )
    {
		// main matrices (row major)
        if( m_MatWorldEffectHandle != NULL )
		{
            GGSetEffectMatrix( m_MatWorldEffectHandle, &g_EffectConstant.matWorld );
        }
        if( m_MatViewEffectHandle != NULL )
		{
            GGSetEffectMatrix( m_MatViewEffectHandle, &g_EffectConstant.matView );
        }
        if( m_MatProjEffectHandle != NULL )
		{
            GGSetEffectMatrix( m_MatProjEffectHandle, &g_EffectConstant.matProj );
        }
        if( m_MatWorldViewEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldViewEffectHandle, &g_EffectConstant.matWorldView );
        }
        if( m_MatViewProjEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatViewProjEffectHandle, &g_EffectConstant.matViewProj );
        }
        if( m_MatWorldViewProjEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldViewProjEffectHandle, &g_EffectConstant.matWorldViewProj );
        }
        if( m_MatWorldInverseEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldInverseEffectHandle, &g_EffectConstant.matWorldInv );
        }		
        if( m_MatViewInverseEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatViewInverseEffectHandle, &g_EffectConstant.matViewInv );
        }
        if( m_MatProjectionInverseEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatProjectionInverseEffectHandle, &g_EffectConstant.matProjInv );
        }
        if( m_MatWorldViewInverseEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldViewInverseEffectHandle, &g_EffectConstant.matWorldViewInv );
        }

		// tranposed matrices (column major)
        if( m_MatWorldTEffectHandle != NULL )
		{
            GGSetEffectMatrix( m_MatWorldTEffectHandle, &g_EffectConstant.matWorldT );
        }
        if( m_MatViewTEffectHandle != NULL )
		{
            GGSetEffectMatrix( m_MatViewTEffectHandle, &g_EffectConstant.matViewT );
        }
        if( m_MatProjTEffectHandle != NULL )
		{
            GGSetEffectMatrix( m_MatProjTEffectHandle, &g_EffectConstant.matProjT );
        }
        if( m_MatWorldViewTEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldViewTEffectHandle, &g_EffectConstant.matWorldViewT );
        }
        if( m_MatViewProjTEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatViewProjTEffectHandle, &g_EffectConstant.matViewProjT );
        }
        if( m_MatWorldViewProjTEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldViewProjTEffectHandle, &g_EffectConstant.matWorldViewProjT );
        }
        if( m_MatWorldInverseTEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldInverseTEffectHandle, &g_EffectConstant.matWorldInvT );
        }		
        if( m_MatViewInverseTEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatViewInverseTEffectHandle, &g_EffectConstant.matViewInvT );
        }		
        if( m_MatWorldViewInverseTEffectHandle != NULL )
        {
            GGSetEffectMatrix( m_MatWorldViewInverseTEffectHandle, &g_EffectConstant.matWorldViewInvT );
        }

		// main vectors
		if ( m_LightDirHandle != NULL )
		{
            GGSetEffectVector( m_LightDirHandle, &g_EffectConstant.vecLightDir );
		}
		if ( m_LightDirInvHandle != NULL )
		{
            GGSetEffectVector( m_LightDirInvHandle, &g_EffectConstant.vecLightDirInv );
		}
		if ( m_LightPosHandle != NULL )
		{
            GGSetEffectVector( m_LightPosHandle, &g_EffectConstant.vecLightPos );
		}
        if( m_VecCameraPosEffectHandle != NULL )
        {
            GGSetEffectVector( m_VecCameraPosEffectHandle, &g_EffectConstant.vecCameraPosition );
        }
		if ( m_VecEyePosEffectHandle != NULL )
		{
            GGSetEffectVector( m_VecEyePosEffectHandle, &g_EffectConstant.vecEyePos );
		}

		// misclanious values
		if( m_TimeEffectHandle != NULL )
		{
			float fTime = timeGetSecond();
			GGSetEffectFloat( m_TimeEffectHandle, fTime );
		}
		if( m_SinTimeEffectHandle != NULL )
		{
			// TIME DATA IN SECONDS
			//float fSinTime = ((float)sin(timeGetTime())) / 1000.0f;
			float fSinTime = sin(timeGetSecond());
			GGSetEffectFloat( m_SinTimeEffectHandle, fSinTime );
		}
		if( m_DeltaTimeEffectHandle != NULL )
		{
			// DELTA TIME DATA IN SECONDS
			float fTimeNow = timeGetSecond();
			float fDeltaTime = fTimeNow - g_fLastDeltaTime;
			GGSetEffectFloat( m_DeltaTimeEffectHandle, fDeltaTime );
			g_fLastDeltaTime = fTimeNow;
		}
		if ( m_UVScalingHandle != NULL )
		{
			g_EffectConstant.vecUVScaling = GGVECTOR4 ( pMesh->fUVScalingU, pMesh->fUVScalingV, 0, 0 );
            GGSetEffectVector( m_UVScalingHandle, &g_EffectConstant.vecUVScaling );
		}

		// set bone matrix palette if required
		if ( m_BoneMatrixPaletteHandle )
		{
			// update all bone matrices
			DWORD dwBoneMax = pMesh->dwBoneCount;
			if ( dwBoneMax > 170 ) dwBoneMax = 170; // 121018 - was 60 from old Shader Model 3.0 days
			// send bone count to shader (so can skip bone anim if nothing in palette)
			///g_EffectConstant.fBoneCount = (float)dwBoneMax;
			///GGSetEffectFloat( m_BoneCountHandle, g_EffectConstant.fBoneCount );

			// update matrix palette if any
			if ( pMesh->dwForceCPUAnimationMode==1 )
			{
				// CPU does animation (or no anim transforms sent to shader)
				for ( DWORD dwMatrixIndex = 0; dwMatrixIndex < dwBoneMax; dwMatrixIndex++ )
					GGMatrixIdentity ( &g_EffectConstant.matBoneMatrixPalette [ dwMatrixIndex ] );
			}
			else
			{
				// GPU needs matrices to do animation
				for ( DWORD dwMatrixIndex = 0; dwMatrixIndex < dwBoneMax; dwMatrixIndex++ )
					if ( pMesh->pFrameMatrices [ dwMatrixIndex ] )
						GGMatrixMultiply ( &g_EffectConstant.matBoneMatrixPalette [ dwMatrixIndex ], &pMesh->pBones [ dwMatrixIndex ].matTranslation, pMesh->pFrameMatrices [ dwMatrixIndex ] );
			}

			// send matrix array to effect (column-based is default by FX compiler)
            GGSetEffectMatrixTransposeArray ( m_BoneMatrixPaletteHandle, g_EffectConstant.matBoneMatrixPalette, dwBoneMax );
		}
    }
}

DARKSDK_DLL bool CreateMesh ( sObject** pObject, LPSTR pName )
{
	// create a new, empty mesh

	// the pointer must be valid
	SAFE_MEMORY ( pObject );

	// create a new object and check allocation
	*pObject = new sObject;
	SAFE_MEMORY ( pObject );

	// create a new frame and check allocation
	pObject [ 0 ]->pFrame = new sFrame;
	SAFE_MEMORY ( pObject [ 0 ]->pFrame );

	// finally create the mesh object
	pObject [ 0 ]->pFrame->pMesh = new sMesh;
	SAFE_MEMORY ( pObject [ 0 ]->pFrame->pMesh );

	// give it a name for reference
	if ( pName )
	{
		if ( strlen(pName) < MAX_STRING )
			strcpy(pObject [ 0 ]->pFrame->szName, pName);
	}

	// all went okay
	return true;
}

DARKSDK_DLL bool DeleteMesh ( sObject** pObject )
{
	// Before we delete object, remove from any temp lists
    if ( !m_ObjectManager.m_vVisibleObjectList.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < m_ObjectManager.m_vVisibleObjectList.size(); ++iIndex )
        {
            sObject* pThisObject = m_ObjectManager.m_vVisibleObjectList [ iIndex ];
			if ( pThisObject==pThisObject )
			{
				//m_ObjectManager.m_vVisibleObjectList [ iIndex ] = NULL;
				m_ObjectManager.m_vVisibleObjectList.erase(m_ObjectManager.m_vVisibleObjectList.begin() + iIndex);
			}
		}
	}
    if ( !m_ObjectManager.m_vVisibleObjectEarly.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < m_ObjectManager.m_vVisibleObjectEarly.size(); ++iIndex )
        {
            sObject* pThisObject = m_ObjectManager.m_vVisibleObjectEarly [ iIndex ];
			if ( pThisObject==pThisObject )
			{
				//m_ObjectManager.m_vVisibleObjectEarly [ iIndex ] = NULL;
				m_ObjectManager.m_vVisibleObjectEarly.erase(m_ObjectManager.m_vVisibleObjectEarly.begin() + iIndex);
			}
		}
	}
    if ( !m_ObjectManager.m_vVisibleObjectTransparent.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < m_ObjectManager.m_vVisibleObjectTransparent.size(); ++iIndex )
        {
            sObject* pThisObject = m_ObjectManager.m_vVisibleObjectTransparent [ iIndex ];
			if ( pThisObject==pThisObject )
			{
				//m_ObjectManager.m_vVisibleObjectTransparent [ iIndex ] = NULL;
				m_ObjectManager.m_vVisibleObjectTransparent.erase(m_ObjectManager.m_vVisibleObjectTransparent.begin() + iIndex);
			}
		}
	}
    if ( !m_ObjectManager.m_vVisibleObjectNoZDepth.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < m_ObjectManager.m_vVisibleObjectNoZDepth.size(); ++iIndex )
        {
            sObject* pThisObject = m_ObjectManager.m_vVisibleObjectNoZDepth [ iIndex ];
			if ( pThisObject==pThisObject )
			{
				//m_ObjectManager.m_vVisibleObjectNoZDepth [ iIndex ] = NULL;
				m_ObjectManager.m_vVisibleObjectNoZDepth.erase(m_ObjectManager.m_vVisibleObjectNoZDepth.begin() + iIndex);
			}
		}
	}
    if ( !m_ObjectManager.m_vVisibleObjectStandard.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < m_ObjectManager.m_vVisibleObjectStandard.size(); ++iIndex )
        {
            sObject* pThisObject = m_ObjectManager.m_vVisibleObjectStandard [ iIndex ];
			if ( pThisObject==pThisObject )
			{
				//m_ObjectManager.m_vVisibleObjectStandard [ iIndex ] = NULL;
				m_ObjectManager.m_vVisibleObjectStandard.erase(m_ObjectManager.m_vVisibleObjectStandard.begin() + iIndex);
			}
		}
	}

	// Delete allocations
	SAFE_DELETE( *pObject );

	// all went okay
	return true;
}

DARKSDK_DLL void UpdateEulerRotation ( sObject* pObject )
{
	// euler rotation 
	GGMatrixRotationX ( &pObject->position.matRotateX, GGToRadian ( pObject->position.vecRotate.x ) );
	GGMatrixRotationY ( &pObject->position.matRotateY, GGToRadian ( pObject->position.vecRotate.y ) );
	GGMatrixRotationZ ( &pObject->position.matRotateZ, GGToRadian ( pObject->position.vecRotate.z ) );

	// choose rotation order for euler matrix
	switch ( pObject->position.dwRotationOrder )
	{
		case ROTORDER_XYZ :	
				pObject->position.matRotation =	pObject->position.matRotateX *
												pObject->position.matRotateY *
												pObject->position.matRotateZ;
				break;

		case ROTORDER_ZYX :	
				pObject->position.matRotation =	pObject->position.matRotateZ *
												pObject->position.matRotateY *
												pObject->position.matRotateX;
				break;

		case ROTORDER_ZXY :
				pObject->position.matRotation = pObject->position.matRotateZ *
												pObject->position.matRotateX *
												pObject->position.matRotateY;
				break;
	}
}

DARKSDK_DLL void UpdateObjectRotation ( sObject* pObject )
{
	if ( pObject->position.bFreeFlightRotation==false )
		UpdateEulerRotation ( pObject );
	else
		pObject->position.matRotation = pObject->position.matFreeFlightRotate;
}

DARKSDK_DLL bool CalcObjectWorld ( sObject* pObject )
{
	// special handling if the object is glued to something (need abs world pos in same cycle so need calc)
	sFrame* pGluedToFramePtr = NULL;
	if ( pObject->position.iGluedToObj )
	{
		if ( g_ObjectList [ pObject->position.iGluedToObj ] != NULL )
		{
			sObject* pOriginalGlueToObj = g_ObjectList [ pObject->position.iGluedToObj ];
			sObject* pChildObject = pOriginalGlueToObj;
			if ( pChildObject )
			{
				if ( pChildObject->ppFrameList == NULL ) 
				{
					pChildObject = pChildObject->pInstanceOfObject;
				}
				if ( pChildObject && pChildObject->ppFrameList != NULL )
				{
					// Must update any object glued to (for current absolute world data) -recurse!
					CalcObjectWorld ( pChildObject );

					// 051205 - if mode 1, wipe out frame orient, leaving position only
					int iFrame = pObject->position.iGluedToMesh;
					int iMode = 0; if ( iFrame < 0 ) { iFrame *= -1; iMode=1; }

					// Identify and extra frame ptr
					if ( iFrame < pChildObject->iFrameCount )
					{
						// get actual frame ptr
						pGluedToFramePtr = pChildObject->ppFrameList[ iFrame ];

						// Mode 1 is set by issuing a negative mesh id
						if ( iMode==1 )
						{
							GGVECTOR3 vecPos = GGVECTOR3 ( pGluedToFramePtr->matCombined._41, pGluedToFramePtr->matCombined._42, pGluedToFramePtr->matCombined._43 );
							GGMatrixIdentity ( &pGluedToFramePtr->matCombined );
							pGluedToFramePtr->matCombined._41 = vecPos.x;
							pGluedToFramePtr->matCombined._42 = vecPos.y;
							pGluedToFramePtr->matCombined._43 = vecPos.z;
						}

						// leefix - 100303 - Calculate correct absolute world matrix
						CalculateAbsoluteWorldMatrix ( pOriginalGlueToObj, pGluedToFramePtr, pGluedToFramePtr->pMesh );
					}
				}
			}
		}
	}

	// setup the world matrix for the object (and ensures matWorld (used by LimbPosition) respects a glued status
	CalculateObjectWorld ( pObject, pGluedToFramePtr );
	
	// success
	return true;
}

DARKSDK_DLL bool CalculateObjectWorld ( sObject* pObject, sFrame* pGluedToFramePtr )
{
	if ( pObject->position.bCustomWorldMatrix == true )
	{
		// return with success
		return true;
	}

	// create a scaling and position matrix
	GGMatrixScaling ( &pObject->position.matScale, pObject->position.vecScale.x, pObject->position.vecScale.y, pObject->position.vecScale.z );
	GGMatrixTranslation ( &pObject->position.matTranslation, pObject->position.vecPosition.x, pObject->position.vecPosition.y, pObject->position.vecPosition.z );

	// GLobal setting to shrink each object (to defeat shadow map self shadowing effect)
	if ( g_fShrinkObjectsTo > 0.0f )
	{
		GGMATRIX matShrinkScale;
		GGMatrixScaling ( &matShrinkScale, g_fShrinkObjectsTo, g_fShrinkObjectsTo, g_fShrinkObjectsTo );
		GGMatrixMultiply ( &pObject->position.matScale, &pObject->position.matScale, &matShrinkScale );
	}
 
	LPVOID pWhenThisCHanges = (LPVOID)&pObject->position.vecPosition.x;

	// handle rotation as either euler or freeflight
	UpdateObjectRotation ( pObject );

	// Apply pivot if any
	if ( pObject->position.bApplyPivot )
	{
		// modify current rotation
		pObject->position.matRotation = pObject->position.matPivot * pObject->position.matRotation;
	}

	// build up final rotation and world matrix
	pObject->position.matObjectNoTran = pObject->position.matScale * pObject->position.matRotation;
	pObject->position.matWorld = pObject->position.matObjectNoTran * pObject->position.matTranslation;

	// Apply glue world-matrix if any
	if ( pObject->position.bGlued && pGluedToFramePtr )
	{
		// find target object:mesh
		sFrame* pTargetFrame = pGluedToFramePtr;
		if ( pTargetFrame )
		{
			// apply object world then limb world
			pObject->position.matWorld *= pTargetFrame->matAbsoluteWorld;

			// no trans taken from target, then clear translation data
			pObject->position.matObjectNoTran = pObject->position.matWorld;
			pObject->position.matObjectNoTran._41 = 0.0f;
			pObject->position.matObjectNoTran._42 = 0.0f;
			pObject->position.matObjectNoTran._43 = 0.0f;
		}
	}

	// all frames should be flagged for recalc of vectors (need data prior to sync!)
	if ( pObject->ppFrameList )
	{
		for ( int iCurrentFrame = 0; iCurrentFrame < pObject->iFrameCount; iCurrentFrame++ )
		{
			sFrame* pFrame = pObject->ppFrameList [ iCurrentFrame ];
			if ( pFrame ) pFrame->bVectorsCalculated = false;
		}
	}

	// success
	return true;
}

DARKSDK_DLL void CalculateAbsoluteWorldMatrix ( sObject* pObject, sFrame* pFrame, sMesh* pMesh )
{
	// bone or frame animation (bone anim includes frame matrix adjustment)
	bool bBoneAnimation = false;
	if ( pMesh )
	{
		// leefix - 110303 - even static bone models must apply the combined factor
		if ( pMesh->dwBoneCount )//&& (pObject->bAnimPlaying || pObject->bAnimManualSlerp) )
		{
			bBoneAnimation = true;
		}
	}

	// set the absolute matrix for the frame
	// 151003 - i have now added this to 'UpdateRealtimeFrameVectors' in dboframe.cpp
	if ( bBoneAnimation )
		pFrame->matAbsoluteWorld = pObject->position.matWorld;
	else
		pFrame->matAbsoluteWorld = pFrame->matCombined * pObject->position.matWorld;

	// 090416 - apply original transform of this frame as FBX models needed transforms applying uniformally
	if ( pObject->dwApplyOriginalScaling==1 || (pObject->pInstanceOfObject && pObject->pInstanceOfObject->dwApplyOriginalScaling==1) )
	{
		pFrame->matAbsoluteWorld = pFrame->matOriginal * pFrame->matAbsoluteWorld;
	}
}

DARKSDK_DLL bool CreateSingleMeshFromObjectCore ( sMesh** ppMesh, sObject* pObject, int iLimbNumberOptional, int iIgnoreMode )
{
	// get untranslated world matrix of object (includes rotation and scale)
	CalculateObjectWorld ( pObject, NULL );

	// create a new mesh
	*ppMesh = new sMesh;

	// count total number of vertices and indexes in object
	DWORD dwTotalVertices=0;
	DWORD dwTotalIndices=0;

	// leefix - 210703 - any FVF
	bool bGotFVFFromFirstMesh=false;
	bool bVertexOnlyBuffer=false;
	DWORD dwNewMeshFVF=0;

	// leeadd - 151008 - u70 - meshes should all be trilist from this process
	int iNewPrimitiveType=GGPT_TRIANGLELIST;

	// leefix - 081208 - U71 - ensure if ANY mesh has no indices, we go for vertex only	
	for ( int iCurrentFrame = 0; iCurrentFrame < pObject->iFrameCount; iCurrentFrame++ )
	{
		sFrame* pFrame = pObject->ppFrameList [ iCurrentFrame ];
		if ( pFrame && (iLimbNumberOptional==-1 || iLimbNumberOptional==iCurrentFrame) )
		{
			sMesh* pFrameMesh = pFrame->pMesh;
			if ( pFrameMesh )
			{
				if ( pFrameMesh->dwIndexCount==0 )
					bVertexOnlyBuffer=true;

				// U75 - 010410 - moved here from belo as both pFrameMesh->iPrimitiveType and iNewPrimitiveType known here
				if ( pFrameMesh->iPrimitiveType != iNewPrimitiveType ) 
				{
					// leeadd - 151008 - u70 - use verts if not trilist
					bVertexOnlyBuffer=true;
				}
			}
		}
	}

	// run through all of the frames within the object
	for ( int iPass = 1; iPass <= 2; iPass++ )
	{
		for ( int iCurrentFrame = 0; iCurrentFrame < pObject->iFrameCount; iCurrentFrame++ )
		{
			// find frame within object
			sFrame* pFrame = pObject->ppFrameList [ iCurrentFrame ];
			if ( pFrame && (iLimbNumberOptional==-1 || iLimbNumberOptional==iCurrentFrame) )
			{
				// mesh within frame
				sMesh* pFrameMesh = pFrame->pMesh;
				if ( pFrameMesh )
				{
					// ignore modes
					if ( iIgnoreMode==1 )
					{
						// 1 - ignore all meshes that have a NO-CULL status (typically leaves, grass, etc)
						if ( pFrameMesh->bCull==false )
							continue;
					}
					if ( iIgnoreMode==2 )
					{
						// 2 - ignore all meshes that have been HIDDEN
						if ( pFrameMesh->bVisible==false )
							continue;
					}

					// get new mesh fvf
					if ( bGotFVFFromFirstMesh==false )
					{
						dwNewMeshFVF = pFrameMesh->dwFVF;
						if ( dwNewMeshFVF==0 )
						{
							dwNewMeshFVF = GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1;
						}
						bGotFVFFromFirstMesh = true;
					}

					// calculate world matrix of frame
					GGMATRIX matWorld = pFrame->matCombined * pObject->position.matObjectNoTran;

					// convert mesh to standard temp mesh-format
					sMesh* pStandardMesh = new sMesh;
					MakeLocalMeshFromOtherLocalMesh ( pStandardMesh, pFrameMesh );
					ConvertLocalMeshToFVF ( pStandardMesh, dwNewMeshFVF );
					
					// U75 - 010410 - this is redundant as the check is performed earlier in this function
					// leeadd - 081208 - U71 - if mesh has NO index data, all mesh must have NO index data, so switch to vertex only
					//if ( pFrameMesh->dwIndexCount==0 )
					//	bVertexOnlyBuffer=true;

					// just verts
					if ( bVertexOnlyBuffer==true ) ConvertLocalMeshToVertsOnly ( pStandardMesh, false ); 

					// pass one - count all verts/indexes
					if ( iPass==1 )
					{
						// total size of data in this mesh
						dwTotalVertices += pStandardMesh->dwVertexCount;
						dwTotalIndices += pStandardMesh->dwIndexCount;
					}

					// pass two - copy data to single mesh
					if ( iPass==2 )
					{
						// copy vertex data from mesh to single-mesh
						BYTE* pDestVertexData = (BYTE*)((*ppMesh)->pVertexData+(dwTotalVertices*pStandardMesh->dwFVFSize));
						BYTE* pDestNormalData = (BYTE*)((*ppMesh)->pVertexData+(dwTotalVertices*pStandardMesh->dwFVFSize)) + ( sizeof(float)*3 );
						if ( dwNewMeshFVF & GGFVF_XYZ )
							memcpy ( pDestVertexData, pStandardMesh->pVertexData, pStandardMesh->dwVertexCount * pStandardMesh->dwFVFSize );

						// copy index data from mesh to single-mesh
						WORD* pDestIndexData = NULL;
						if ( (*ppMesh)->pIndices )
						{
							pDestIndexData = (WORD*)((*ppMesh)->pIndices + dwTotalIndices);
							memcpy ( pDestIndexData, pStandardMesh->pIndices, pStandardMesh->dwIndexCount * sizeof(WORD) );
						}

						// transform vertex data by world matrix of frame
						for ( DWORD v=0; v<pStandardMesh->dwVertexCount; v++ )
						{
							// get pointer to vertex and normal
							GGVECTOR3* pVertex = (GGVECTOR3*)(pDestVertexData+(v*pStandardMesh->dwFVFSize));
							GGVECTOR3* pNormal = (GGVECTOR3*)(pDestNormalData+(v*pStandardMesh->dwFVFSize));

							// transform with current combined frame matrix
							if ( dwNewMeshFVF & GGFVF_XYZ )
							{
								GGVec3TransformCoord ( pVertex, pVertex, &matWorld );
							}
							if ( dwNewMeshFVF & GGFVF_NORMAL )
							{
								GGVec3TransformNormal ( pNormal, pNormal, &matWorld );
								GGVec3Normalize ( pNormal, pNormal );
							}
						}

						// increment index data to reference correct vertex area
						if ( pDestIndexData )
						{
							for ( DWORD i=0; i<pStandardMesh->dwIndexCount; i++ )
							{
								pDestIndexData[i] += (WORD)dwTotalVertices;
							}
						}

						// advance counters
						dwTotalVertices += pStandardMesh->dwVertexCount;
						dwTotalIndices += pStandardMesh->dwIndexCount;
					}

					// delete standard temp mesh
					SAFE_DELETE ( pStandardMesh );
				}
			}
		}

		// end of passes
		if ( iPass==1 )
		{
			// leefix - 280305 - if index list too big, switch to using pre vertex buffer
			if ( dwTotalIndices > 0x0000FFFF )
			{
				bVertexOnlyBuffer = true;
				dwTotalVertices = dwTotalIndices * 3;
				dwTotalIndices = 0;
				iPass--; //redo count!
			}

			// make vertex and index buffers for single mesh
			SetupMeshFVFData ( *ppMesh, dwNewMeshFVF, dwTotalVertices, dwTotalIndices, false );

			// reset counters to fill single mesh
			dwTotalVertices = 0;
			dwTotalIndices = 0;
		}
		if ( iPass==2 )
		{
			// setup mesh drawing properties when single mesh data transfered
			(*ppMesh)->iPrimitiveType   = iNewPrimitiveType;
			(*ppMesh)->iDrawVertexCount = dwTotalVertices;
			if ( dwTotalIndices==0 )
				(*ppMesh)->iDrawPrimitives  = dwTotalVertices/3;
			else
				(*ppMesh)->iDrawPrimitives  = dwTotalIndices/3;
		}
	}

	if ( iLimbNumberOptional!=-1 )
	{
		// for limb specific creations, also copy texture info over
		sFrame* pFrame = pObject->ppFrameList [ iLimbNumberOptional ];
		if ( pFrame )
		{
			sMesh* pMesh = pFrame->pMesh;
			if ( pMesh )
			{
				(*ppMesh)->dwTextureCount = pMesh->dwTextureCount; 
				(*ppMesh)->pTextures = new sTexture [ (*ppMesh)->dwTextureCount ]; 
				CloneInternalTextures ( (*ppMesh), pMesh );
			}
		}
	}
	else
	{
		// for whole object conversions, use first texture of master object
		if ( pObject->ppMeshList )
		{
			sMesh* pMesh = pObject->ppMeshList [ 0 ];
			if ( pMesh )
			{
				(*ppMesh)->dwTextureCount = pMesh->dwTextureCount; 
				(*ppMesh)->pTextures = new sTexture [ (*ppMesh)->dwTextureCount ]; 
				CloneInternalTextures ( (*ppMesh), pMesh );
			}
		}
	}

	// all went okay
	return true;
}

DARKSDK_DLL bool CreateSingleMeshFromObject ( sMesh** ppMesh, sObject* pObject, int iIgnoreMode )
{
	return CreateSingleMeshFromObjectCore ( ppMesh, pObject, -1, iIgnoreMode );
}

DARKSDK_DLL bool CreateSingleMeshFromLimb ( sMesh** ppMesh, sObject* pObject, int iLimbNumber, int iIgnoreMode )
{
	return CreateSingleMeshFromObjectCore ( ppMesh, pObject, iLimbNumber, iIgnoreMode );
}

/* new version but messes with FPSC assumtpions of vertex/index layout

  bool CreateSingleMeshFromObject ( sMesh** ppMesh, sObject* pObject )
{
	// get untranslated world matrix of object (includes rotation and scale)
	CalculateObjectWorld ( pObject, NULL );

	// create a new mesh
	*ppMesh = new sMesh;

	// count total number of vertices and indexes in object
	DWORD dwTotalVertices=0;

	// leefix - 210703 - any FVF
	bool bGotFVFFromFirstMesh=false;
	DWORD dwNewMeshFVF=0;

	// run through all of the frames within the object
	for ( int iPass = 1; iPass <= 2; iPass++ )
	{
		for ( int iCurrentFrame = 0; iCurrentFrame < pObject->iFrameCount; iCurrentFrame++ )
		{
			// find frame within object
			sFrame* pFrame = pObject->ppFrameList [ iCurrentFrame ];
			if ( pFrame )
			{
				// mesh within frame
				sMesh* pFrameMesh = pFrame->pMesh;
				if ( pFrameMesh )
				{
					// get new mesh fvf
					if ( bGotFVFFromFirstMesh==false )
					{
						dwNewMeshFVF = pFrameMesh->dwFVF;
						if ( dwNewMeshFVF==0 )
						{
							// cannot create nonFVF mesh at the moment
							SAFE_DELETE(*ppMesh);
							return false;
						}
						bGotFVFFromFirstMesh = true;
					}

					// calculate world matrix of frame
					GGMATRIX matWorld = pFrame->matCombined * pObject->position.matObjectNoTran;

					// convert mesh to standard temp mesh-format
					sMesh* pStandardMesh = new sMesh;
					MakeLocalMeshFromOtherLocalMesh ( pStandardMesh, pFrameMesh );
					ConvertLocalMeshToFVF ( pStandardMesh, dwNewMeshFVF );
					ConvertLocalMeshToVertsOnly ( pStandardMesh );

					// pass one - count all verts/indexes
					if ( iPass==1 )
					{
						// total size of data in this mesh
						dwTotalVertices += pStandardMesh->dwVertexCount;
					}

					// pass two - copy data to single mesh
					if ( iPass==2 )
					{
						// copy vertex data from mesh to single-mesh
						BYTE* pDestVertexData = (BYTE*)((*ppMesh)->pVertexData+(dwTotalVertices*pStandardMesh->dwFVFSize));
						BYTE* pDestNormalData = (BYTE*)((*ppMesh)->pVertexData+(dwTotalVertices*pStandardMesh->dwFVFSize)) + ( sizeof(float)*3 );
						if ( dwNewMeshFVF & GGFVF_XYZ )
							memcpy ( pDestVertexData, pStandardMesh->pVertexData, pStandardMesh->dwVertexCount * pStandardMesh->dwFVFSize );

						// transform vertex data by world matrix of frame
						for ( DWORD v=0; v<pStandardMesh->dwVertexCount; v++ )
						{
							// get pointer to vertex and normal
							GGVECTOR3* pVertex = (GGVECTOR3*)(pDestVertexData+(v*pStandardMesh->dwFVFSize));
							GGVECTOR3* pNormal = (GGVECTOR3*)(pDestNormalData+(v*pStandardMesh->dwFVFSize));

							// transform with current combined frame matrix
							if ( dwNewMeshFVF & GGFVF_XYZ )
							{
								GGVec3TransformCoord ( pVertex, pVertex, &matWorld );
							}
							if ( dwNewMeshFVF & GGFVF_NORMAL )
							{
								GGVec3TransformNormal ( pNormal, pNormal, &matWorld );
								GGVec3Normalize ( pNormal, pNormal );
							}
						}

						// advance counters
						dwTotalVertices += pStandardMesh->dwVertexCount;
					}

					// delete standard temp mesh
					SAFE_DELETE ( pStandardMesh );
				}
			}
		}

		// end of passes
		if ( iPass==1 )
		{
			// 010804 - verts only due to size
			SetupMeshFVFData ( *ppMesh, dwNewMeshFVF, dwTotalVertices, 0, false );

			// reset counters to fill single mesh
			dwTotalVertices = 0;
		}
		if ( iPass==2 )
		{
			// setup mesh drawing properties when single mesh data transfered
			(*ppMesh)->iPrimitiveType   = GGPT_TRIANGLELIST;
			(*ppMesh)->iDrawVertexCount = dwTotalVertices;
			(*ppMesh)->iDrawPrimitives  = dwTotalVertices/3;
		}
	}

	// all went okay
	return true;
}
*/

// DXMesh implementations

DARKSDK_DLL LPGGMESH LocalMeshToDXMesh ( sMesh* pMesh, CONST LPGGVERTEXELEMENT pDeclarationOverride, DWORD dwFVFOverride )
{
	// result var
	LPGGMESH pNewMesh = NULL;

	#ifdef DX11
	// No mesh functions in DX11!
	#else
	// create a mesh
	DWORD dwFaces = pMesh->iDrawPrimitives;
	DWORD dwVertices = pMesh->dwVertexCount;
	DWORD dwFVF = pMesh->dwFVF;
	LPGGMESH pOriginalMesh = NULL;
	IGGIndexBuffer* pIB = NULL;
	IGGVertexBuffer* pVB = NULL;

	// if too big, create a 32bit index buffer XMesh
	if ( dwFaces*3 > 0x0000FFFF )
	{
		// create dx mesh
		if ( dwFVF==0 )
			D3DXCreateMesh ( dwFaces, dwVertices, D3DXMESH_MANAGED | D3DXMESH_32BIT, pMesh->pVertexDeclaration, m_pD3D, &pOriginalMesh );
		else
			D3DXCreateMeshFVF ( dwFaces, dwVertices, D3DXMESH_MANAGED | D3DXMESH_32BIT, dwFVF, m_pD3D, &pOriginalMesh );

		// if failed to create dx mesh
		if ( pOriginalMesh==NULL)
			return NULL;

		// must prepare mesh index data as basic triangle list indices, not strips
		ConvertLocalMeshToTriList ( pMesh );

		// if original input mesh has no index data, create some for process
		if ( pMesh->dwIndexCount==0 )
		{
			pMesh->dwIndexCount = dwFaces * 3;
			DWORD* pDWORDPtr = new DWORD [ pMesh->dwIndexCount ];
			pMesh->pIndices = (WORD*)pDWORDPtr;
			for ( DWORD dwI=0; dwI<pMesh->dwIndexCount; dwI++ )
				pDWORDPtr [ dwI ] = dwI;
		}
		else
		{
			// if it does have data, it's in WORDs so need to re-create them as DWORDs
			pMesh->dwIndexCount = dwFaces * 3;
			DWORD* pDWORDPtr = new DWORD [ pMesh->dwIndexCount ];
			memset(pDWORDPtr,0,pMesh->dwIndexCount*sizeof(DWORD));
			for ( DWORD dwI=0; dwI<pMesh->dwIndexCount; dwI++ )
				pDWORDPtr [ dwI ] = pMesh->pIndices[dwI];
			// 281114 - changed to SAFE_DELETE_ARRAY
			SAFE_DELETE_ARRAY(pMesh->pIndices);
			pMesh->pIndices = (WORD*)pDWORDPtr;
		}

		// fill mesh with input mesh data
		pOriginalMesh->GetIndexBuffer(&pIB);
		pOriginalMesh->GetVertexBuffer(&pVB);
		if(pIB) CopyDWORDMeshDataToDWORDIndexBuffer ( pMesh, pIB, 0 );
		if(pVB) CopyMeshDataToVertexBufferSameFVF ( pMesh, pVB, 0 );
	}
	else
	{
		// support for 32bit meshes
		if ( dwVertices > 65535 )
		{
			// create LARGE dx mesh
			if ( dwFVF==0 )
				D3DXCreateMesh ( dwFaces, dwVertices, D3DXMESH_MANAGED | D3DXMESH_32BIT, pMesh->pVertexDeclaration, m_pD3D, &pOriginalMesh );
			else
				D3DXCreateMeshFVF ( dwFaces, dwVertices, D3DXMESH_MANAGED | D3DXMESH_32BIT, dwFVF, m_pD3D, &pOriginalMesh );
		}
		else
		{
			// create dx mesh
			if ( dwFVF==0 )
				D3DXCreateMesh ( dwFaces, dwVertices, D3DXMESH_MANAGED, pMesh->pVertexDeclaration, m_pD3D, &pOriginalMesh );
			else
				D3DXCreateMeshFVF ( dwFaces, dwVertices, D3DXMESH_MANAGED, dwFVF, m_pD3D, &pOriginalMesh );
		}

		// if failed to create dx mesh
		if ( pOriginalMesh==NULL)
			return NULL;

		// must prepare mesh index data as basic triangle list indices, not strips
		ConvertLocalMeshToTriList ( pMesh );

		// if original input mesh has no index data, create some for process
		if ( pMesh->dwIndexCount==0 )
		{
			pMesh->dwIndexCount = dwFaces * 3;
			pMesh->pIndices = new WORD [ pMesh->dwIndexCount ];
			for ( WORD dwI=0; dwI<(WORD)pMesh->dwIndexCount; dwI++ )
				pMesh->pIndices [ dwI ] = dwI;
		}

		// fill mesh with input mesh data
		pOriginalMesh->GetIndexBuffer(&pIB);
		pOriginalMesh->GetVertexBuffer(&pVB);
		if(pIB) CopyMeshDataToIndexBuffer ( pMesh, pIB, 0 );
		if(pVB) CopyMeshDataToVertexBufferSameFVF ( pMesh, pVB, 0 );
	}

	// modify mesh if vertex format overwrite in place
	if ( dwFVFOverride > 0 )
	{
		// When converting mesh to FVF standard
		pOriginalMesh->CloneMeshFVF ( 0, dwFVFOverride, m_pD3D, &pNewMesh );
		SAFE_RELEASE ( pOriginalMesh );
	}
	else if ( pDeclarationOverride!=NULL )
	{
		// When converting mesh to shader Declaration
		pOriginalMesh->CloneMesh ( 0, pDeclarationOverride, m_pD3D, &pNewMesh );
		SAFE_RELEASE ( pOriginalMesh );
	}
	else
	{
		// keep mesh - it is okay
		pNewMesh = pOriginalMesh;
	}

	// release buffers
	SAFE_RELEASE(pIB);
	SAFE_RELEASE(pVB);
	#endif

	// return DX mesh
	return pNewMesh;
}

DARKSDK_DLL void UpdateLocalMeshWithDXMesh ( sMesh* pMesh, LPGGMESH pDXMesh )
{
	// do not update if dxptr invalid
	if ( pDXMesh==NULL )
		return;

	#ifdef DX11
	// this mesh vx dxmesh - any way to avoid for DX11
	#else
	// If index count exceeds 16bit..
	bool bConvertIndexedDataToVertexOnly=false;
	DWORD dwVertexCount = pDXMesh->GetNumVertices();
	DWORD dwIndexCount = pDXMesh->GetNumFaces ( ) * 3;
	if ( dwIndexCount > 0 )
	{
		if ( dwIndexCount > 0x0000FFFF )
		{
			// create a vertex only mesh
			bConvertIndexedDataToVertexOnly=true;
			dwVertexCount = dwIndexCount;
			dwIndexCount = 0;
		}
	}

	// mesh can hold regular FVF and custom declarations
	if ( pDXMesh->GetFVF()==0 )
	{
		// now create new mesh data from new Declaration
		GGVERTEXELEMENT Declaration[MAX_FVF_DECL_SIZE];
		pDXMesh->GetDeclaration( Declaration );
		if ( SetupMeshDeclarationData ( pMesh, Declaration, pDXMesh->GetNumBytesPerVertex(), dwVertexCount, dwIndexCount )==false )
		{
			// need to fully restore the object mid-conversion
			return;
		}
	}
	else
	{
		// now create new mesh data from new FVF
		SetupMeshFVFData ( pMesh, pDXMesh->GetFVF(), dwVertexCount, dwIndexCount, false );
	}

	// get vertex and index buffer
	LPDIRECT3DVERTEXBUFFER9 m_pMeshVertexBuffer;
	LPDIRECT3DINDEXBUFFER9  m_pMeshIndexBuffer;
	pDXMesh->GetVertexBuffer ( &m_pMeshVertexBuffer );
	pDXMesh->GetIndexBuffer  ( &m_pMeshIndexBuffer );

	// copy dx mesh to dbpro mesh
	if ( bConvertIndexedDataToVertexOnly==true )
	{	
		// destination data
		BYTE* pDestVertPtr = (BYTE*)pMesh->pVertexData;

		// source data
		WORD* pIndices = NULL;
		if ( SUCCEEDED ( m_pMeshIndexBuffer->Lock ( 0, 0, ( VOID** ) &pIndices, 0 ) ) )
		{
			BYTE* pSrcVertexData = NULL;
			if ( SUCCEEDED ( m_pMeshVertexBuffer->Lock ( 0, 0, ( VOID** ) &pSrcVertexData, 0 ) ) )
			{
				// go through all faces data of dx mesh
				DWORD dwIndex = 0;
				for ( DWORD f=0; f<pDXMesh->GetNumFaces ( ); f++ )
				{
					// three vertx per face
					for ( int n=0; n<3; n++ )
					{
						// source data - index data gives us src vertex position
						DWORD dwBufferOffset = pIndices[ dwIndex++ ];

						// copy across to the dest buffer and advance to next vertex
						memcpy ( pDestVertPtr, pSrcVertexData+(dwBufferOffset*pMesh->dwFVFSize), pMesh->dwFVFSize );
						pDestVertPtr+=pMesh->dwFVFSize;
					}
				}

				// unlock the vertex buffer
				m_pMeshVertexBuffer->Unlock ( );
			}

			// unlock the index buffer
			m_pMeshIndexBuffer->Unlock ( );
		}
	}
	else
	{
		// fill mesh data from vertex and index buffers
		if(m_pMeshIndexBuffer) CopyIndexBufferToMeshData ( pMesh, m_pMeshIndexBuffer, 0 );
		if(m_pMeshVertexBuffer) CopyVertexBufferToMeshDataSameFVF ( pMesh, m_pMeshVertexBuffer, 0 );
	}

	// release buffers
	SAFE_RELEASE(m_pMeshVertexBuffer);
	SAFE_RELEASE(m_pMeshIndexBuffer);

	// when mesh changes, must flag it
	pMesh->bMeshHasBeenReplaced = true;
	#endif
}

DARKSDK_DLL void RestoreLocalMesh ( sMesh* pMesh )
{
	#ifdef DX11
	#else
	LPGGMESH pDXMesh = LocalMeshToDXMesh ( pMesh, NULL, pMesh->dwFVFOriginal );
	UpdateLocalMeshWithDXMesh ( pMesh, pDXMesh );
	// lee - 240306 - u6b5 - must not hold onto this if restored mesh
	SAFE_DELETE ( pMesh->pOriginalVertexData );
	SAFE_RELEASE(pDXMesh);
	#endif
}

DARKSDK_DLL void ConvertLocalMeshToFVF ( sMesh* pMesh, DWORD dwFVF )
{
	#ifdef DX11
	if ( pMesh->dwFVF != dwFVF )
	{
		// convert current mesh VB data to new dwFVF
		sOffsetMap offsetMap, offsetMapNew;
		GetFVFOffsetMap ( pMesh, &offsetMap );
		pMesh->dwFVF = dwFVF;
		GetFVFOffsetMap ( pMesh, &offsetMapNew );
		DWORD dwNewFVFSize = offsetMapNew.dwByteSize;
		DWORD dwNumberOfVertices = pMesh->dwVertexCount;
		DWORD dwNewVertexDataSize = dwNumberOfVertices * dwNewFVFSize;
		LPSTR pNewVertexData = new char[dwNewVertexDataSize];
		for ( int iCurrentVertex = 0; iCurrentVertex < (int)dwNumberOfVertices; iCurrentVertex++ )
		{
			if ( dwFVF & GGFVF_XYZ )
			{
				GGVECTOR3 vecPos = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iCurrentVertex ) );
				*(GGVECTOR3*)( ( float* ) pNewVertexData + offsetMapNew.dwX + ( offsetMapNew.dwSize * iCurrentVertex ) ) = vecPos;
			}
			if ( dwFVF & GGFVF_NORMAL )
			{
				GGVECTOR3 vecNorm = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) );
				*(GGVECTOR3*)( ( float* ) pNewVertexData + offsetMapNew.dwNX + ( offsetMapNew.dwSize * iCurrentVertex ) ) = vecNorm;
			}
			if ( dwFVF & GGFVF_DIFFUSE )
			{
				DWORD dwDiffuseColour = *(DWORD*)( ( float* ) pMesh->pVertexData + offsetMap.dwDiffuse + ( offsetMap.dwSize * iCurrentVertex ) );
				*(DWORD*)( ( float* ) pNewVertexData + offsetMapNew.dwDiffuse + ( offsetMapNew.dwSize * iCurrentVertex ) ) = dwDiffuseColour;
			}
			/* oops! GGFVF_TEX1 actually means ONE texture set, GGFVF_TEX2 means two texture sets (i.e. lightmapping)
			if ( dwFVF & GGFVF_TEX1 )
			{
				float fTex = *(float*)( ( float* ) pMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * iCurrentVertex ) );
				*(float*)( ( float* ) pNewVertexData + offsetMapNew.dwTU[0] + ( offsetMapNew.dwSize * iCurrentVertex ) ) = fTex;
			}
			if ( dwFVF & GGFVF_TEX2 )
			*/
			if ( dwFVF & GGFVF_TEX1 || dwFVF & GGFVF_TEX2 )
			{
				GGVECTOR2 vecTex = *(GGVECTOR2*)( ( float* ) pMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * iCurrentVertex ) );
				*(GGVECTOR2*)( ( float* ) pNewVertexData + offsetMapNew.dwTU[0] + ( offsetMapNew.dwSize * iCurrentVertex ) ) = vecTex;
			}
			if ( dwFVF & GGFVF_TEX2 )
			{
				GGVECTOR2 vecTex = *(GGVECTOR2*)( ( float* ) pMesh->pVertexData + offsetMap.dwTU[1] + ( offsetMap.dwSize * iCurrentVertex ) );
				*(GGVECTOR2*)( ( float* ) pNewVertexData + offsetMapNew.dwTU[1] + ( offsetMapNew.dwSize * iCurrentVertex ) ) = vecTex;
			}
		}
		SAFE_DELETE ( pMesh->pVertexData );
		pMesh->pVertexData = (BYTE*)pNewVertexData;
		pMesh->dwFVF = dwFVF;
		pMesh->dwFVFSize = dwNewFVFSize;
	}
	#else
	if ( pMesh->dwFVF != dwFVF )
	{
		LPGGMESH pDXMesh = LocalMeshToDXMesh ( pMesh, NULL, dwFVF );
		UpdateLocalMeshWithDXMesh ( pMesh, pDXMesh );
		SAFE_RELEASE(pDXMesh);
	}
	#endif
}

DARKSDK_DLL void ConvertLocalMeshToVertsOnly ( sMesh* pMesh, bool bIs32BitIndexData )
{
	// mesh 'can' store 32bit index data temporarily for later conversion to vertex only.
	// and should only use 32bit if going to do a vertex expanding (due to >16bit verts)
	bool b32BITIndexData=false;
	if ( pMesh->dwVertexCount > 0xFFFF || bIs32BitIndexData == true )
		b32BITIndexData=true;

	// ensure it is a trilist first
	if ( b32BITIndexData==false )
		ConvertLocalMeshToTriList ( pMesh );

	// convert from index to vertex only
	if ( pMesh->iPrimitiveType==GGPT_TRIANGLELIST)
	{
		if ( pMesh->pIndices )
		{
			// new vertex data
			DWORD dwVertSize = pMesh->dwFVFSize;
			DWORD dwNewVertexCount = pMesh->dwIndexCount;
			BYTE* pNewVertexData = (BYTE*)new char [ dwNewVertexCount * dwVertSize ];

			// recreate conversion map
			SAFE_DELETE_ARRAY ( g_pConversionMap );
			if ( g_pConversionMap==NULL )
			{
				// size of old vertex buffer (where old face data referenced)
				g_pConversionMap = new DWORD[pMesh->dwVertexCount];
			}

			// go through all indices (leefix-161003-fixed wFaceIndex to dwFaceIndex as this function used to expand vertex data so no index data is required (16bit+)
			DWORD dwFaceIndex = 0;
			BYTE* pBase = pMesh->pVertexData;
			DWORD* pDWORDIndexPtr = (DWORD*)pMesh->pIndices;
			for ( DWORD i=0; i<pMesh->dwIndexCount; i+=3 )
			{
				// read face
				DWORD dwFace0, dwFace1, dwFace2;
				if ( b32BITIndexData )
				{
					// special temporary DWORD index data
					dwFace0 = pDWORDIndexPtr[i+0];
					dwFace1 = pDWORDIndexPtr[i+1];
					dwFace2 = pDWORDIndexPtr[i+2];
				}
				else
				{
					// normal WORD index data
					dwFace0 = pMesh->pIndices[i+0];
					dwFace1 = pMesh->pIndices[i+1];
					dwFace2 = pMesh->pIndices[i+2];
				}

				// get vert data
				float* pFromVert1 = (float*)((BYTE*)pBase+(dwFace0*dwVertSize));
				float* pFromVert2 = (float*)((BYTE*)pBase+(dwFace1*dwVertSize));
				float* pFromVert3 = (float*)((BYTE*)pBase+(dwFace2*dwVertSize));
				float* pToVert1 = (float*)((BYTE*)pNewVertexData+((dwFaceIndex+0)*dwVertSize));
				float* pToVert2 = (float*)((BYTE*)pNewVertexData+((dwFaceIndex+1)*dwVertSize));
				float* pToVert3 = (float*)((BYTE*)pNewVertexData+((dwFaceIndex+2)*dwVertSize));

				// record destination indexes in conversion map
				g_pConversionMap[dwFace0]=dwFaceIndex+0;
				g_pConversionMap[dwFace1]=dwFaceIndex+1;
				g_pConversionMap[dwFace2]=dwFaceIndex+2;
				dwFaceIndex+=3;

				// write to new vert data
				memcpy ( pToVert1, pFromVert1, dwVertSize );
				memcpy ( pToVert2, pFromVert2, dwVertSize );
				memcpy ( pToVert3, pFromVert3, dwVertSize );
			}

			// delete index data and old vertex data
			// 281114 - changed to SAFE_DELETE_ARRAY
			SAFE_DELETE_ARRAY(pMesh->pIndices);
			SAFE_DELETE_ARRAY(pMesh->pVertexData);

			// replace mesh ptrs
			pMesh->dwIndexCount = 0;
			pMesh->dwVertexCount = dwNewVertexCount;
			pMesh->pVertexData = pNewVertexData;
			pMesh->iDrawVertexCount = dwNewVertexCount;
			pMesh->iDrawPrimitives  = dwNewVertexCount/3;
		}
	}
}

DARKSDK_DLL bool ConvertLocalMeshToTriList ( sMesh* pMesh )
{
	// was action taken
	bool bActionTaken=false;

	// convert from tristrip
	if ( pMesh->iPrimitiveType==GGPT_TRIANGLESTRIP )
	{
		WORD* pNewIndex = NULL;
		DWORD dwNewIndexCount = 0;
		if ( pMesh->pIndices==NULL )
		{
			// generate new mesh without indices
			WORD wIndexSeq = 0;
			dwNewIndexCount = (pMesh->dwVertexCount-2) * 3;
			pNewIndex = new WORD [ dwNewIndexCount ];
			WORD wFace0 = wIndexSeq; wIndexSeq++;
			WORD wFace1 = 0;
			WORD wFace2 = wIndexSeq; wIndexSeq++;
			int iToggle = 0;
			DWORD dwIndex = 0;
			for ( DWORD i = 2; i < pMesh->dwVertexCount; i++ )
			{
				// face assignments
				if ( iToggle==0 )
				{
					wFace0 = wFace0;
					wFace1 = wFace2;
					wFace2 = wIndexSeq; wIndexSeq++;
				}
				else
				{
					wFace0 = wFace2;
					wFace1 = wFace1;
					wFace2 = wIndexSeq; wIndexSeq++;
				}
				iToggle=1-iToggle;

				// get face vectors
				pNewIndex [ dwIndex++ ] = wFace0;
				pNewIndex [ dwIndex++ ] = wFace1;
				pNewIndex [ dwIndex++ ] = wFace2;
			}
		}
		else
		{
			// generate new mesh from indices
			dwNewIndexCount = (pMesh->dwIndexCount-2) * 3;
			pNewIndex = new WORD [ dwNewIndexCount ];
			WORD wFace0 = pMesh->pIndices [ 0 ];
			WORD wFace1 = 0;
			WORD wFace2 = pMesh->pIndices [ 1 ];
			int iToggle = 0;
			DWORD dwIndex = 0;
			for ( DWORD i = 2; i < pMesh->dwIndexCount; i++ )
			{
				// face assignments
				if ( iToggle==0 )
				{
					wFace0 = wFace0;
					wFace1 = wFace2;
					wFace2 = pMesh->pIndices [ i ];
				}
				else
				{
					wFace0 = wFace2;
					wFace1 = wFace1;
					wFace2 = pMesh->pIndices [ i ];
				}
				iToggle=1-iToggle;

				// get face vectors
				pNewIndex [ dwIndex++ ] = wFace0;
				pNewIndex [ dwIndex++ ] = wFace1;
				pNewIndex [ dwIndex++ ] = wFace2;
			}
		}

		// delete old index data
		// 281114 - changed to SAFE_DELETE_ARRAY
		SAFE_DELETE_ARRAY(pMesh->pIndices);

		// replace mesh ptrs
		pMesh->iPrimitiveType = 4;
		pMesh->dwIndexCount = dwNewIndexCount;
		pMesh->pIndices = pNewIndex;

		// complete mesh replace - slow
		pMesh->bMeshHasBeenReplaced = true;
		bActionTaken = true;
	}

	// convert from trifan
	if ( pMesh->iPrimitiveType==GGPT_TRIANGLEFAN )
	{
		WORD* pNewIndex = NULL;
		DWORD dwNewIndexCount = 0;
		if ( pMesh->pIndices==NULL )
		{
			// generate new indices
			WORD wIndexSeq = 0;
			dwNewIndexCount = (pMesh->dwVertexCount-2) * 3;
			pNewIndex = new WORD [ dwNewIndexCount ];
			WORD wFace0 = wIndexSeq; wIndexSeq++;
			WORD wFace1 = 0;
			WORD wFace2 = wIndexSeq; wIndexSeq++;
			DWORD dwIndex = 0;
			for ( DWORD i = 2; i < pMesh->dwVertexCount; i++ )
			{
				// face assignments
				wFace0 = wFace0;
				wFace1 = wFace2;
				wFace2 = wIndexSeq; wIndexSeq++;

				// get face vectors
				pNewIndex [ dwIndex++ ] = wFace0;
				pNewIndex [ dwIndex++ ] = wFace1;
				pNewIndex [ dwIndex++ ] = wFace2;
			}
		}

		// delete old index data
		// 281114 - changed to SAFE_DELETE_ARRAY
		SAFE_DELETE_ARRAY(pMesh->pIndices);

		// replace mesh ptrs
		pMesh->iPrimitiveType = 4;
		pMesh->dwIndexCount = dwNewIndexCount;
		pMesh->pIndices = pNewIndex;

		// complete mesh replace - slow
		pMesh->bMeshHasBeenReplaced = true;
		bActionTaken = true;
	}

	// return action state
	return bActionTaken;
}

DARKSDK_DLL void ConvertToSharedVerts ( sMesh* pMesh, float fEpsilon )
{
	if ( pMesh->pIndices )
	{
		DWORD dwFaceIndex = 0;
		BYTE* pBase = pMesh->pVertexData;
		DWORD dwVertSize = pMesh->dwFVFSize;
		for ( DWORD i=0; i<pMesh->dwIndexCount; i+=3 )
		{
			// read face
			DWORD dwFace0, dwFace1, dwFace2;
			dwFace0 = pMesh->pIndices[i+0];
			dwFace1 = pMesh->pIndices[i+1];
			dwFace2 = pMesh->pIndices[i+2];

			// get vert data
			GGVECTOR3* pV0 = (GGVECTOR3*)((BYTE*)pBase+(dwFace0*dwVertSize));
			GGVECTOR3* pV1 = (GGVECTOR3*)((BYTE*)pBase+(dwFace1*dwVertSize));
			GGVECTOR3* pV2 = (GGVECTOR3*)((BYTE*)pBase+(dwFace2*dwVertSize));

			// find any previous instance of each vert position
			for ( DWORD facedone=0; facedone<i; facedone++ )
			{
				// read previous faces
				DWORD dwFaceD0 = pMesh->pIndices[facedone+0];
				DWORD dwFaceD1 = pMesh->pIndices[facedone+1];
				DWORD dwFaceD2 = pMesh->pIndices[facedone+2];
				GGVECTOR3* pVD0 = (GGVECTOR3*)((BYTE*)pBase+(dwFaceD0*dwVertSize));
				GGVECTOR3* pVD1 = (GGVECTOR3*)((BYTE*)pBase+(dwFaceD1*dwVertSize));
				GGVECTOR3* pVD2 = (GGVECTOR3*)((BYTE*)pBase+(dwFaceD2*dwVertSize));

				// check against current face, and re-use older face index if found
				GGVECTOR3 vec0 = *pV0 - *pVD0;
				GGVECTOR3 vec1 = *pV0 - *pVD1;
				GGVECTOR3 vec2 = *pV0 - *pVD2;
				if ( GGVec3Length ( &vec0 ) < fEpsilon ) pMesh->pIndices[i+0] = (WORD)dwFaceD0;
				if ( GGVec3Length ( &vec1 ) < fEpsilon ) pMesh->pIndices[i+0] = (WORD)dwFaceD1;
				if ( GGVec3Length ( &vec2 ) < fEpsilon ) pMesh->pIndices[i+0] = (WORD)dwFaceD2;
				vec0 = *pV1 - *pVD0;
				vec1 = *pV1 - *pVD1;
				vec2 = *pV1 - *pVD2;
				if ( GGVec3Length ( &vec0 ) < fEpsilon ) pMesh->pIndices[i+1] = (WORD)dwFaceD0;
				if ( GGVec3Length ( &vec1 ) < fEpsilon ) pMesh->pIndices[i+1] = (WORD)dwFaceD1;
				if ( GGVec3Length ( &vec2 ) < fEpsilon ) pMesh->pIndices[i+1] = (WORD)dwFaceD2;
				vec0 = *pV2 - *pVD0;
				vec1 = *pV2 - *pVD1;
				vec2 = *pV2 - *pVD2;
				if ( GGVec3Length ( &vec0 ) < fEpsilon ) pMesh->pIndices[i+2] = (WORD)dwFaceD0;
				if ( GGVec3Length ( &vec1 ) < fEpsilon ) pMesh->pIndices[i+2] = (WORD)dwFaceD1;
				if ( GGVec3Length ( &vec2 ) < fEpsilon ) pMesh->pIndices[i+2] = (WORD)dwFaceD2;
			}
		}
	}
}

DARKSDK_DLL bool MakeLocalMeshFromOtherLocalMesh ( sMesh* pMesh, sMesh* pOtherMesh, DWORD dwIndexCount, DWORD dwVertexCount )
{
	// get details from other mesh
	DWORD dwFVF				= pOtherMesh->dwFVF;
	DWORD dwFVFSize			= pOtherMesh->dwFVFSize;

	// mesh can hold regular FVF and custom declarations
	bool bTempAllow32BitIndexSoCanProduceVertOnlyMesh = false;
	if ( dwFVF==0 )
	{
		// now create new mesh data from declaration
		if ( !SetupMeshDeclarationData ( pMesh, pOtherMesh->pVertexDeclaration, dwFVFSize, dwVertexCount, dwIndexCount ) )
			return false;
	}
	else
	{
		// 310819 - if the indexcount is over 16bit, we know the next call will fail, so convert mesh to vertex only
		if ( dwIndexCount > 0 )
			if ( dwVertexCount > 0xFFFF )//if ( dwIndexCount > 0x0000FFFF )
				bTempAllow32BitIndexSoCanProduceVertOnlyMesh = true;

		// create new mesh from FVF
		if ( !SetupMeshFVFData ( pMesh, dwFVF, dwVertexCount, dwIndexCount, bTempAllow32BitIndexSoCanProduceVertOnlyMesh ) )
			return false;
	}

	// copy vertex data
	DWORD dwVertexDataSize = pOtherMesh->dwFVFSize * pOtherMesh->dwVertexCount;
	memcpy ( pMesh->pVertexData, pOtherMesh->pVertexData, dwVertexDataSize );

	// copy index data
	DWORD dwIndiceDataSize = sizeof(WORD) * pOtherMesh->dwIndexCount;
	if ( bTempAllow32BitIndexSoCanProduceVertOnlyMesh == true ) dwIndiceDataSize = sizeof(DWORD) * pOtherMesh->dwIndexCount;
	if ( pMesh->pIndices ) memcpy ( pMesh->pIndices, pOtherMesh->pIndices, dwIndiceDataSize );

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = pOtherMesh->iPrimitiveType;
	pMesh->iDrawVertexCount = pOtherMesh->iDrawVertexCount;
	pMesh->iDrawPrimitives  = pOtherMesh->iDrawPrimitives;

	// leeadd - 030306 - u60 - if mesh from rawmesh, no prim to draw from
	if ( pMesh->iDrawPrimitives==0 )
	{
		// calculate a value that makes most sense
		pMesh->iDrawPrimitives=pMesh->iDrawVertexCount/3;
	}

	// we 'still' do not support 32bit indices (ouch), so convert this mesh to vert only so it works with everything else
	if ( bTempAllow32BitIndexSoCanProduceVertOnlyMesh == true )
		ConvertLocalMeshToVertsOnly ( pMesh, bTempAllow32BitIndexSoCanProduceVertOnlyMesh );

	// okay
	return true;
}

DARKSDK_DLL bool MakeLocalMeshFromOtherLocalMesh ( sMesh* pMesh, sMesh* pOtherMesh )
{
	DWORD dwIndexCount = pOtherMesh->dwIndexCount;
	DWORD dwVertexCount = pOtherMesh->dwVertexCount;
	return MakeLocalMeshFromOtherLocalMesh ( pMesh, pOtherMesh, dwIndexCount, dwVertexCount );
}

DARKSDK_DLL bool MakeLocalMeshFromPureMeshData ( sMesh* pMesh, DWORD dwFVF, DWORD dwFVFSize, float* pMeshData, DWORD dwVertexCount, DWORD dwPrimType )
{
	// create new mesh
	if ( !SetupMeshFVFData ( pMesh, dwFVF, dwVertexCount, 0, false ) )
		return false;

	// copy vertex data
	DWORD dwVertexDataSize = dwFVFSize * dwVertexCount;
	memcpy ( pMesh->pVertexData, pMeshData, dwVertexDataSize );

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = dwPrimType;
	pMesh->iDrawVertexCount = dwVertexCount;
	pMesh->iDrawPrimitives  = dwVertexCount/3;


	// okay
	return true;
}

DARKSDK_DLL LPGGMESH ComputeTangentBasisEx ( LPGGMESH gMasterMesh, bool bMakeNormals, bool bMakeTangents, bool bMakeBinormals, bool bFixTangents, bool bCylTexGen, bool bWeightNormalsByFace )
{
	#ifdef DX11
	// define raw input data type for this computation
	typedef struct 
	{
		GGVECTOR3 position;
		GGVECTOR3 normal;
		GGVECTOR2 texCoord;
	} MeshVertex;

	// input/output ptrs for conversion
	std::vector<float> position;
	std::vector<float> normal;
	std::vector<float> texCoord;
	std::vector<float> texCoord2;
	std::vector<float> binormal;
	std::vector<float> tangent;

	// Retrieve data from the temp mesh, put in input/output ptrs
	sOffsetMap offsetMap;
	bool bRetainSecondaryUVData = false;
	GetFVFValueOffsetMap ( gMasterMesh->dwFVF, &offsetMap );
	if ( offsetMap.dwTU[1] > 0 ) bRetainSecondaryUVData = true;
	DWORD numVertices = gMasterMesh->dwVertexCount;
	for (unsigned int i = 0; i < numVertices; ++i) 
	{
		float fX = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * i ) );
		float fY = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * i ) );
		float fZ = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * i ) );
		float fNX = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * i ) );
		float fNY = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwNY + ( offsetMap.dwSize * i ) );
		float fNZ = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwNZ + ( offsetMap.dwSize * i ) );
		float fU = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * i ) );
		float fV = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwTV[0] + ( offsetMap.dwSize * i ) );
		float fU2 = 0.0f;
		float fV2 = 0.0f;
		if ( bRetainSecondaryUVData == true )
		{
			fU2 = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwTU[1] + ( offsetMap.dwSize * i ) );
			fV2 = *( ( float* ) gMasterMesh->pVertexData + offsetMap.dwTV[1] + ( offsetMap.dwSize * i ) );
		}
		position.push_back(fX);
		position.push_back(fY);
		position.push_back(fZ);
		normal.push_back(fNX);
		normal.push_back(fNY);
		normal.push_back(fNZ);
		texCoord.push_back(fU);
		texCoord.push_back(fV);
		texCoord.push_back(0);
		texCoord2.push_back(fU2);
		texCoord2.push_back(fV2);
		texCoord2.push_back(0);
	}

	// Retrieve triangle indices from the temp mesh, put in input/output ptr
	std::vector<int> index;
	if ( gMasterMesh->pIndices )
	{
		DWORD numTriangles = gMasterMesh->iDrawPrimitives;
		for (unsigned int i = 0; i < numTriangles; ++i) 
		{
			int i0 = *( ( WORD* ) gMasterMesh->pIndices + ( 3 * i ) + 0 );
			int i1 = *( ( WORD* ) gMasterMesh->pIndices + ( 3 * i ) + 1 );
			int i2 = *( ( WORD* ) gMasterMesh->pIndices + ( 3 * i ) + 2 );
			index.push_back(i0);
			index.push_back(i1);
			index.push_back(i2);
		}
	}
	else
	{
		// no indices, no conversion - sky ried to convert, but had no indices
		// return NULL;
		if ( gMasterMesh->dwFVF == 530 )
		{
			// 290618 - so can add tangents and binormals to lightmapped objects, add indices if not present
			int iPolygonCount = 0;
			DWORD numTriangles = gMasterMesh->iDrawPrimitives;
			for (unsigned int i = 0; i < numTriangles; ++i) 
			{
				int i0 = iPolygonCount + 0;
				int i1 = iPolygonCount + 1;
				int i2 = iPolygonCount + 2;
				index.push_back(i0);
				index.push_back(i1);
				index.push_back(i2);
				iPolygonCount+=3;
			}
		}
		else
		{
			// all other objects are not converted to retain backwards compatibility with engine elements (shadow floor)
			return NULL;
		}
	}

	// Specify conversion options from flags
	NVMeshMender::Option _FixTangents = NVMeshMender::FixTangents;
	NVMeshMender::Option _FixCylindricalTexGen = NVMeshMender::FixCylindricalTexGen;
	NVMeshMender::Option _WeightNormalsByFaceSize = NVMeshMender::WeightNormalsByFaceSize;
	if ( bFixTangents==false ) _FixTangents = NVMeshMender::DontFixTangents;
	if ( bCylTexGen==false ) _FixCylindricalTexGen = NVMeshMender::DontFixCylindricalTexGen;
	if ( bWeightNormalsByFace==false ) _WeightNormalsByFaceSize = NVMeshMender::DontWeightNormalsByFaceSize;

	// Setup INPUT Components
	NVMeshMender::VertexAttribute positionAtt;
	positionAtt.Name_ = "position";
	positionAtt.floatVector_ = position;
	NVMeshMender::VertexAttribute normalAtt;
	normalAtt.Name_ = "normal";
	normalAtt.floatVector_ = normal;
	NVMeshMender::VertexAttribute indexAtt;
	indexAtt.Name_ = "indices";
	indexAtt.intVector_ = index;
	NVMeshMender::VertexAttribute texCoordAtt;
	texCoordAtt.Name_ = "tex0";
	texCoordAtt.floatVector_ = texCoord;
	NVMeshMender::VertexAttribute texCoordAtt2;
	texCoordAtt2.Name_ = "tex1";
	texCoordAtt2.floatVector_ = texCoord2;

	// Create INPUT Attribute
	std::vector<NVMeshMender::VertexAttribute> inputAtts;
	inputAtts.push_back(positionAtt);
	inputAtts.push_back(indexAtt);
	inputAtts.push_back(texCoordAtt);
	if ( bRetainSecondaryUVData == true ) inputAtts.push_back(texCoordAtt2);
	inputAtts.push_back(normalAtt);
	
	// Add In OUTPUT Components
	NVMeshMender::VertexAttribute tangentAtt;
	tangentAtt.Name_ = "tangent";
	NVMeshMender::VertexAttribute binormalAtt;
	binormalAtt.Name_ = "binormal";

	// Create OUTPUT Attribute
	unsigned int n = 0;
	std::vector<NVMeshMender::VertexAttribute> outputAtts;
	outputAtts.push_back(positionAtt); ++n;
	outputAtts.push_back(indexAtt); ++n;
	outputAtts.push_back(texCoordAtt); ++n;
	if ( bRetainSecondaryUVData == true ) 
	{
		outputAtts.push_back(texCoordAtt2); ++n;
	}
	outputAtts.push_back(normalAtt); ++n;
	outputAtts.push_back(tangentAtt); ++n;
	outputAtts.push_back(binormalAtt); ++n;

	//PE: (note) tangent is sometimes calculated wrong ? perhaps for missing fixtangent, that we are not able to use.
	// Uses MeshMenderD3DX from NVIDIA
	NVMeshMender mender;
	if (!mender.MungeD3DX(
					inputAtts,								// input attributes
					outputAtts,								// outputs attributes
					3.141592654f / 3.0f,					// tangent space smooth angle
					0,										// no texture matrix applied to my texture coordinates
					_FixTangents,							// fix degenerate bases & texture mirroring
					_FixCylindricalTexGen,					// low poly and quad meshes
					_WeightNormalsByFaceSize				// weigh vertex normals by the triangle's size
					))
	{
		// Failed to convert mesh over
		return gMasterMesh;
	}

	// Get output ptrs after conversion
	--n; binormal = outputAtts[n].floatVector_; 
	--n; tangent = outputAtts[n].floatVector_; 
	--n; normal = outputAtts[n].floatVector_; 
	if ( bRetainSecondaryUVData == true ) 
	{
		--n; texCoord2 = outputAtts[n].floatVector_;
	}
	--n; texCoord = outputAtts[n].floatVector_;
	--n; index = outputAtts[n].intVector_;
	--n; position = outputAtts[n].floatVector_;

	// ensure vertex data and index data size is unchanged
	DWORD dwNewVertexCount = position.size()/3;
	DWORD dwNewFaceCount = index.size()/3;

	// create mesh from new declaration
	SAFE_DELETE(gMasterMesh->pVertexData);
	SAFE_DELETE(gMasterMesh->pIndices);
	gMasterMesh->dwFVFOriginal = gMasterMesh->dwFVF;
	gMasterMesh->dwFVF = 0;
	gMasterMesh->dwFVFSize = 12+12+8+12+12;
	if ( bRetainSecondaryUVData == true ) 
	{
		gMasterMesh->dwFVFSize += 8;
		gMasterMesh->dwFVF = 530;
	}
	DWORD dwVSize = gMasterMesh->dwFVFSize;
	gMasterMesh->pVertexData = new BYTE[dwNewVertexCount*dwVSize];
	gMasterMesh->pIndices = new WORD[dwNewFaceCount*3];
	gMasterMesh->dwIndexCount = dwNewFaceCount*3;

	// Copy data into new mesh
	int iPosOffset = -1;
	int iDiffuseOffset = -1;
	int iTexOffset = -1;
	int iTexOffset2 = -1;
	int iNormalOffset = -1;
	int iTangentOffset = -1;
	int iBinormalOffset = -1;
	GGEFFECT_DESC EffectDesc;
	ID3DX11EffectTechnique* hTechnique;
	D3DX11_TECHNIQUE_DESC TechniqueDesc;
	ID3DX11EffectPass* hPass;
	cSpecialEffect* pEffect = gMasterMesh->pVertexShaderEffect;
	pEffect->m_pEffect->GetDesc( &EffectDesc );
	for( UINT iTech = 0; iTech < EffectDesc.Techniques; iTech++ )
	{
		hTechnique = pEffect->m_pEffect->GetTechniqueByIndex( iTech );
		hTechnique->GetDesc ( &TechniqueDesc );
		for( UINT iPass = 0; iPass < TechniqueDesc.Passes; iPass++ )
		{
			hPass = hTechnique->GetPassByIndex ( iPass );
			D3DX11_PASS_SHADER_DESC vs_desc;
			hPass->GetVertexShaderDesc(&vs_desc);
			D3DX11_EFFECT_SHADER_DESC s_desc;
			vs_desc.pShaderVariable->GetShaderDesc(0, &s_desc);
            UINT NumVSSemanticsUsed = s_desc.NumInputSignatureEntries;
			int iByteOffset = 0;
			for( UINT iSem = 0; iSem < NumVSSemanticsUsed; iSem++ )
			{
				D3D11_SIGNATURE_PARAMETER_DESC pSigParDesc;
				vs_desc.pShaderVariable->GetInputSignatureElementDesc ( 0, iSem, &pSigParDesc );
				if( stricmp ( pSigParDesc.SemanticName, "POSITION" ) == NULL ) { iPosOffset = iByteOffset; iByteOffset += 12; }
				if( stricmp ( pSigParDesc.SemanticName, "NORMAL" ) == NULL ) { iNormalOffset = iByteOffset; iByteOffset += 12; }
				if( stricmp ( pSigParDesc.SemanticName, "COLOR" ) == NULL ) { iDiffuseOffset = iByteOffset; iByteOffset += 4; }
				if( stricmp ( pSigParDesc.SemanticName, "TEXCOORD" ) == NULL && pSigParDesc.SemanticIndex == 0 ) { iTexOffset = iByteOffset; iByteOffset += 8; }
				if( stricmp ( pSigParDesc.SemanticName, "TEXCOORD" ) == NULL && pSigParDesc.SemanticIndex == 1 ) { iTexOffset2 = iByteOffset; iByteOffset += 8; }
				if( stricmp ( pSigParDesc.SemanticName, "TANGENT" ) == NULL ) { iTangentOffset = iByteOffset; iByteOffset += 12; }
				if( stricmp ( pSigParDesc.SemanticName, "BINORMAL" ) == NULL ) { iBinormalOffset = iByteOffset; iByteOffset += 12; }
			}
		}
	}

	// Binormal makers
	BYTE* pPtr = gMasterMesh->pVertexData;
	for ( DWORD v=0; v<dwNewVertexCount; ++v)
	{
		// obtain component ptrs
		GGVECTOR3* vecPos = (GGVECTOR3*)(pPtr+iPosOffset);
		GGVECTOR3* vecNormal = (GGVECTOR3*)(pPtr+iNormalOffset);
		GGCOLOR*   colDiffuse = (GGCOLOR*)(pPtr+iDiffuseOffset);
		GGVECTOR2* vecTex = (GGVECTOR2*)(pPtr+iTexOffset);
		GGVECTOR3* vecTangent = (GGVECTOR3*)(pPtr+iTangentOffset);
		GGVECTOR3* vecBinormal = (GGVECTOR3*)(pPtr+iBinormalOffset);

		// fill data of components in output mesh
		if ( iPosOffset!=-1 )
		{
			vecPos->x = position[3 * v + 0];
			vecPos->y = position[3 * v + 1];
			vecPos->z = position[3 * v + 2];
		}
		if ( iNormalOffset!=-1 )
		{
			vecNormal->x = normal[3 * v + 0];
			vecNormal->y = normal[3 * v + 1];
			vecNormal->z = normal[3 * v + 2];
		}
		if ( iTexOffset!=-1 )
		{
			vecTex->x = texCoord[3 * v + 0];
			vecTex->y = texCoord[3 * v + 1];
		}
		if ( bRetainSecondaryUVData == true ) 
		{
			GGVECTOR2* vecTex2 = (GGVECTOR2*)(pPtr+iTexOffset2);
			if ( iTexOffset2!=-1 )
			{
				vecTex2->x = texCoord2[3 * v + 0];
				vecTex2->y = texCoord2[3 * v + 1];
			}
		}
		if ( iDiffuseOffset!=-1 )
		{
			*colDiffuse = GGCOLOR(255,255,255,255);
		}
		if ( iTangentOffset!=-1 )
		{
			vecTangent->x = tangent[3 * v + 0];
			vecTangent->y = tangent[3 * v + 1];
			vecTangent->z = tangent[3 * v + 2];
		}
		if ( iBinormalOffset!=-1 )
		{
			vecBinormal->x = binormal[3 * v + 0];
			vecBinormal->y = binormal[3 * v + 1];
			vecBinormal->z = binormal[3 * v + 2];
		}

		// next vertex
		pPtr+=dwVSize;
	}

	// index buffer data
	for (DWORD i = 0; i < dwNewFaceCount; ++i)
	{
		gMasterMesh->pIndices[(i*3)+0] = index[3 * i + 0];
		gMasterMesh->pIndices[(i*3)+1] = index[3 * i + 1];
		gMasterMesh->pIndices[(i*3)+2] = index[3 * i + 2];
	}
	return NULL;
	#else
	// define raw input data type for this computation
	LPGGMESH pOutputMesh = NULL;
	typedef struct 
	{
		GGVECTOR3 position;
		GGVECTOR3 normal;
		GGVECTOR2 texCoord;
	} MeshVertex;

	// input/output ptrs for conversion
	std::vector<float> position;
	std::vector<float> normal;
	std::vector<float> texCoord;
	std::vector<float> binormal;
	std::vector<float> tangent;

	// Get declaration of mesh to convert
	GGVERTEXELEMENT End = GDECL_END();
	GGVERTEXELEMENT Declaration[MAX_FVF_DECL_SIZE];
	gMasterMesh->GetDeclaration( Declaration );

	// Create temporary mesh to hold required input data XYZ,NORMAL,TEX
	// leefix - 050906 - changed 'gMesh' to 'gMasterMesh' and 'pNewMesh' to 'pInputDataMesh'
	LPGGMESH pInputDataMesh = NULL;
	DWORD dwSysteMemFlag = D3DXMESH_SYSTEMMEM;
	if ( gMasterMesh->GetNumVertices() > 65535 ) dwSysteMemFlag = D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT;
	gMasterMesh->CloneMeshFVF ( dwSysteMemFlag, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, m_pD3D, &pInputDataMesh );
	if ( pInputDataMesh==NULL ) return gMasterMesh;

	// Retrieve data from the temp mesh, put in input/output ptrs
	MeshVertex* vertexBuffer = NULL;
	DWORD numVertices = pInputDataMesh->GetNumVertices();
	pInputDataMesh->LockVertexBuffer(D3DLOCK_READONLY, (VOID**)&vertexBuffer);
	if ( vertexBuffer==NULL ) return gMasterMesh;
	unsigned int i;
	for (i = 0; i < numVertices; ++i) {
		position.push_back(vertexBuffer[i].position.x);
		position.push_back(vertexBuffer[i].position.y);
		position.push_back(vertexBuffer[i].position.z);
		normal.push_back(vertexBuffer[i].normal.x);
		normal.push_back(vertexBuffer[i].normal.y);
		normal.push_back(vertexBuffer[i].normal.z);
		texCoord.push_back(vertexBuffer[i].texCoord.x);
		texCoord.push_back(vertexBuffer[i].texCoord.y);
		texCoord.push_back(0);
	}
	pInputDataMesh->UnlockVertexBuffer();

	// Retrieve triangle indices from the temp mesh, put in input/output ptr
	WORD (*indexBuffer)[3];
	std::vector<int> index;
	DWORD numTriangles = pInputDataMesh->GetNumFaces();
	pInputDataMesh->LockIndexBuffer(D3DLOCK_READONLY, (VOID**)&indexBuffer);
	if ( indexBuffer==NULL ) return gMasterMesh;
	for (i = 0; i < numTriangles; ++i) 
	{
		index.push_back(indexBuffer[i][0]);
		index.push_back(indexBuffer[i][1]);
		index.push_back(indexBuffer[i][2]);
	}
	pInputDataMesh->UnlockIndexBuffer();

	// finished with input mesh, release this temp mesh now
	SAFE_RELEASE ( pInputDataMesh );

	// Specify conversion options from flags
	NVMeshMender::Option _FixTangents = NVMeshMender::FixTangents;
	NVMeshMender::Option _FixCylindricalTexGen = NVMeshMender::FixCylindricalTexGen;
	NVMeshMender::Option _WeightNormalsByFaceSize = NVMeshMender::WeightNormalsByFaceSize;
	if ( bFixTangents==false ) _FixTangents = NVMeshMender::DontFixTangents;
	if ( bCylTexGen==false ) _FixCylindricalTexGen = NVMeshMender::DontFixCylindricalTexGen;
	if ( bWeightNormalsByFace==false ) _WeightNormalsByFaceSize = NVMeshMender::DontWeightNormalsByFaceSize;

	// Setup INPUT Components
	NVMeshMender::VertexAttribute positionAtt;
	positionAtt.Name_ = "position";
	positionAtt.floatVector_ = position;
	NVMeshMender::VertexAttribute normalAtt;
	normalAtt.Name_ = "normal";
	normalAtt.floatVector_ = normal;
	NVMeshMender::VertexAttribute indexAtt;
	indexAtt.Name_ = "indices";
	indexAtt.intVector_ = index;
	NVMeshMender::VertexAttribute texCoordAtt;
	texCoordAtt.Name_ = "tex0";
	texCoordAtt.floatVector_ = texCoord;

	// Create INPUT Attribute
	std::vector<NVMeshMender::VertexAttribute> inputAtts;
	inputAtts.push_back(positionAtt);
	inputAtts.push_back(indexAtt);
	inputAtts.push_back(texCoordAtt);
	inputAtts.push_back(normalAtt);
	
	// Add In OUTPUT Components
	NVMeshMender::VertexAttribute tangentAtt;
	tangentAtt.Name_ = "tangent";
	NVMeshMender::VertexAttribute binormalAtt;
	binormalAtt.Name_ = "binormal";

	// Create OUTPUT Attribute
	unsigned int n = 0;
	std::vector<NVMeshMender::VertexAttribute> outputAtts;
	outputAtts.push_back(positionAtt); ++n;
	outputAtts.push_back(indexAtt); ++n;
	outputAtts.push_back(texCoordAtt); ++n;
	outputAtts.push_back(normalAtt); ++n;
	outputAtts.push_back(tangentAtt); ++n;
	outputAtts.push_back(binormalAtt); ++n;

	// Uses MeshMenderD3DX from NVIDIA
	NVMeshMender mender;
	if (!mender.MungeD3DX(
					inputAtts,								// input attributes
					outputAtts,								// outputs attributes
					3.141592654f / 3.0f,					// tangent space smooth angle
					0,										// no texture matrix applied to my texture coordinates
					_FixTangents,							// fix degenerate bases & texture mirroring
					_FixCylindricalTexGen,					// low poly and quad meshes
					_WeightNormalsByFaceSize				// weigh vertex normals by the triangle's size
					))
	{
		// Failed to convert mesh over
		return gMasterMesh;
	}

	// Get output ptrs after conversion
	--n; binormal = outputAtts[n].floatVector_; 
	--n; tangent = outputAtts[n].floatVector_; 
	--n; normal = outputAtts[n].floatVector_; 
	--n; texCoord = outputAtts[n].floatVector_;
	--n; index = outputAtts[n].intVector_;
	--n; position = outputAtts[n].floatVector_;

	///* SOMEONE is corrupting the heap (near here, is it the code below?)

	// ensure vertex data and index data size is unchanged
	DWORD dwNewVertexCount = position.size()/3;
	DWORD dwNewFaceCount = index.size()/3;
	if ( dwNewVertexCount==numVertices && dwNewFaceCount==numTriangles )
	{
		// create mesh from new declaration
		gMasterMesh->CloneMesh( dwSysteMemFlag, Declaration, m_pD3D, &pOutputMesh );
	}
	else
	{
		// create new mesh resized for new output data
		D3DXCreateMesh ( dwNewFaceCount, dwNewVertexCount, dwSysteMemFlag, Declaration, m_pD3D, &pOutputMesh);
	}
	if ( pOutputMesh==NULL ) return gMasterMesh;

	// Lock the vertex buffer
	LPDIRECT3DVERTEXBUFFER9 pVB = NULL;
	DWORD dwVSize = pOutputMesh->GetNumBytesPerVertex();
	DWORD dwVertexCount = pOutputMesh->GetNumVertices();
	DWORD dwFaceCount = pOutputMesh->GetNumFaces();
	HRESULT hr = pOutputMesh->GetVertexBuffer( &pVB );
	if( SUCCEEDED(hr) )
	{
		BYTE* pVertices = NULL;
		hr = pVB->Lock( 0, 0, (VOID**)&pVertices, 0 );
		if( SUCCEEDED(hr) )
		{
			// Find Offsets
			int iPosOffset = -1;
			int iDiffuseOffset = -1;
			int iTexOffset = -1;
			int iNormalOffset = -1;
			int iTangentOffset = -1;
			int iBinormalOffset = -1;
			for( int iElem=0; Declaration[iElem].Stream != End.Stream; iElem++ )
			{   
				if( Declaration[iElem].Usage == GGDECLUSAGE_POSITION ) iPosOffset = Declaration[iElem].Offset;
				if( Declaration[iElem].Usage == GGDECLUSAGE_NORMAL ) iNormalOffset = Declaration[iElem].Offset;
				if( Declaration[iElem].Usage == D3DDECLUSAGE_COLOR ) iDiffuseOffset = Declaration[iElem].Offset;
				if( Declaration[iElem].Usage == GGDECLUSAGE_TEXCOORD && Declaration[iElem].UsageIndex==0 ) iTexOffset = Declaration[iElem].Offset; //leefix - 050906 - only take first stage TEX UV data
				if( Declaration[iElem].Usage == D3DDECLUSAGE_TANGENT ) iTangentOffset = Declaration[iElem].Offset;
				if( Declaration[iElem].Usage == D3DDECLUSAGE_BINORMAL ) iBinormalOffset = Declaration[iElem].Offset;
			}

			// Binormal makers
			BYTE* pPtr = pVertices;
			for ( DWORD v=0; v<dwVertexCount; ++v)
			{
				// obtain component ptrs
				GGVECTOR3* vecPos = (GGVECTOR3*)(pPtr+iPosOffset);
				GGVECTOR3* vecNormal = (GGVECTOR3*)(pPtr+iNormalOffset);
				GGCOLOR*   colDiffuse = (GGCOLOR*)(pPtr+iDiffuseOffset);
				GGVECTOR2* vecTex = (GGVECTOR2*)(pPtr+iTexOffset);
				GGVECTOR3* vecTangent = (GGVECTOR3*)(pPtr+iTangentOffset);
				GGVECTOR3* vecBinormal = (GGVECTOR3*)(pPtr+iBinormalOffset);

				// fill data of components in output mesh
				if ( iPosOffset!=-1 )
				{
					vecPos->x = position[3 * v + 0];
					vecPos->y = position[3 * v + 1];
					vecPos->z = position[3 * v + 2];
				}
				if ( iNormalOffset!=-1 )
				{
					vecNormal->x = normal[3 * v + 0];
					vecNormal->y = normal[3 * v + 1];
					vecNormal->z = normal[3 * v + 2];
				}
				if ( iTexOffset!=-1 )
				{
					vecTex->x = texCoord[3 * v + 0];
					vecTex->y = texCoord[3 * v + 1];
				}
				if ( iDiffuseOffset!=-1 )
				{
					*colDiffuse = GGCOLOR(255,255,255,255);
				}
				if ( iTangentOffset!=-1 )
				{
					vecTangent->x = tangent[3 * v + 0];
					vecTangent->y = tangent[3 * v + 1];
					vecTangent->z = tangent[3 * v + 2];
				}
				if ( iBinormalOffset!=-1 )
				{
					vecBinormal->x = binormal[3 * v + 0];
					vecBinormal->y = binormal[3 * v + 1];
					vecBinormal->z = binormal[3 * v + 2];
				}

				// next vertex
				pPtr+=dwVSize;
			}

			// unlock buffer
			pVB->Unlock();
		}

		// release buffer
		SAFE_RELEASE( pVB );
	}

	// Lock the index buffer
	LPDIRECT3DINDEXBUFFER9 pIB = NULL;
	hr = pOutputMesh->GetIndexBuffer( &pIB );
	if( SUCCEEDED(hr) )
	{
		hr = pIB->Lock( 0, 0, (VOID**)&indexBuffer, 0 );
		if( SUCCEEDED(hr) )
		{
			for (DWORD i = 0; i < dwFaceCount; ++i)
			{
				indexBuffer[i][0] = index[3 * i + 0];
				indexBuffer[i][1] = index[3 * i + 1];
				indexBuffer[i][2] = index[3 * i + 2];
			}

			// unlock buffer
			pIB->Unlock();
		}

		// release buffer
		SAFE_RELEASE( pIB );
	}
	
	// free old mesh
	SAFE_RELEASE ( gMasterMesh );

	// complete
	return pOutputMesh;
	#endif
}

DARKSDK_DLL LPGGMESH ComputeTangentBasis( LPGGMESH gMesh, bool bFixTangents, bool bCylTexGen, bool bWeightNormalsByFace )
{
	// leeadd - 050906 - added extra boolean controls for normal, tangent and binormal generation (darkshader control)
	return ComputeTangentBasisEx ( gMesh, true, true, true, bFixTangents, bCylTexGen, bWeightNormalsByFace );
}

DARKSDK_DLL bool CheckIfNeedExtraBonesPerVertices ( sMesh* pMesh )
{
	if ( pMesh->dwBoneCount==0 ) return false;
	DWORD dwNumVertices = pMesh->dwVertexCount;
	DWORD dwNumBones = pMesh->dwBoneCount;
	float* pfWorkWeight = new float [ dwNumVertices * dwNumBones ];
	memset ( pfWorkWeight, 0, sizeof( float ) * dwNumVertices * dwNumBones );
	for ( int iBone = 0; iBone < ( int ) dwNumBones; iBone++ )
	{
		for ( int iVert = 0; iVert < ( int ) pMesh->pBones [ iBone ].dwNumInfluences; iVert++ )
		{
			int iIndexToVertex = pMesh->pBones [ iBone ].pVertices [ iVert ];
			float fWeight = pMesh->pBones [ iBone ].pWeights [ iVert ];
			pfWorkWeight [ (iIndexToVertex*dwNumBones)+iBone ] = fWeight;
		}
	}
	int maxBonesAttachedToVertexOfMesh = 0;
	for ( DWORD iIndexToVertex = 0; iIndexToVertex < dwNumVertices; iIndexToVertex++ )
	{
		int countBonesAttachedToVertex = 0;
		for (int iTryEightBones = 0; iTryEightBones < 8; iTryEightBones++ )
		{
			int iBestBone = -1;
			float fBest = 0.0f;
			for ( int iBone = 0; iBone < ( int ) dwNumBones; iBone++ )
			{
				float fWeight = pfWorkWeight [ (iIndexToVertex*dwNumBones)+iBone ];
				if ( fWeight>fBest ) { fBest=fWeight; iBestBone=iBone; }
			}
			if ( iBestBone!=-1)
			{
				pfWorkWeight [ (iIndexToVertex*dwNumBones)+iBestBone ]=0.0f;
				countBonesAttachedToVertex++;
			}
			else
				break;
		}
		if ( countBonesAttachedToVertex > maxBonesAttachedToVertexOfMesh ) 
			maxBonesAttachedToVertexOfMesh = countBonesAttachedToVertex;
	}
	SAFE_DELETE(pfWorkWeight);
	if ( maxBonesAttachedToVertexOfMesh > 4 )
		return true;
	else
		return false;
}

DARKSDK_DLL void ComputeBoneDataInsideVertex ( sMesh* pMesh, LPGGMESH pVertexShaderMesh, DWORD dwWeightOffsetInBytes, DWORD dwIndicesOffsetInBytes, DWORD dwExtraWeightOffsetInBytes, DWORD dwExtraIndicesOffsetInBytes, DWORD dwVertSizeInBytes )
{
	// do not do if no bones (catched earlier when making bonedata in declaration)
	if ( pMesh->dwBoneCount==0 )
		return;

	#ifdef DX11
	// put animation bone data ( weights and indices ) into the vertex data (for VS skinning)
	DWORD dwNumVertices = pMesh->dwVertexCount;
	DWORD dwNumBones = pMesh->dwBoneCount;

	// create maximum of bones-per-vertex for collection purposes
	struct sBoneForVertInfo
	{
		float fWorkWeight;
		float fFinalWeight;
	};
	sBoneForVertInfo* pBFVI = new sBoneForVertInfo [ dwNumVertices * dwNumBones ];
	memset ( pBFVI, 0, sizeof( sBoneForVertInfo ) * dwNumVertices * dwNumBones );

	// collect bonesdata for all verts
	for ( int iBone = 0; iBone < ( int ) dwNumBones; iBone++ )
	{
		// go through all influences in bone
		for ( int iVert = 0; iVert < ( int ) pMesh->pBones [ iBone ].dwNumInfluences; iVert++ )
		{
			// get the vertex and weight
			int iIndexToVertex = pMesh->pBones [ iBone ].pVertices [ iVert ];
			float fWeight = pMesh->pBones [ iBone ].pWeights [ iVert ];

			// add this influence of weight to the pool under the correct bone index
			pBFVI [ (iIndexToVertex*dwNumBones)+iBone ].fWorkWeight = fWeight;
		}
	}

	// prepare an array to hold the best four
	int* piUseBone = new int [ dwNumVertices * 4 ];
	memset ( piUseBone, 0, sizeof( int ) * dwNumVertices * 4 );

	// 121018 - also prepare an array for next best four (for eight bone-per-vertex models)
	int* piUseBoneExtra = new int [ dwNumVertices * 4 ];
	memset ( piUseBoneExtra, 0, sizeof( int ) * dwNumVertices * 4 );

	// work out the best four per vertex
	int maxBonesAttachedToVertex = 0;
	for ( DWORD iIndexToVertex = 0; iIndexToVertex < dwNumVertices; iIndexToVertex++ )
	{
		// fill with minus ones to indicate no bone at all
		for ( int iFillFlag = 0; iFillFlag<4; iFillFlag++ )
			piUseBone [ (iIndexToVertex*4)+iFillFlag ] = -1;
		for ( int iFillFlag = 0; iFillFlag<4; iFillFlag++ )
			piUseBoneExtra [ (iIndexToVertex*4)+iFillFlag ] = -1;

		// mark out best four
		int iBestFour = 0;
		for (; iBestFour<4; iBestFour++ )
		{
			// find best
			int iBestBone = -1;
			float fBest = 0.0f;
			for ( int iBone = 0; iBone < ( int ) dwNumBones; iBone++ )
			{
				// get weight and compare for best one
				float fWeight = pBFVI [ (iIndexToVertex*dwNumBones)+iBone ].fWorkWeight;
				if ( fWeight>fBest ) { fBest=fWeight; iBestBone=iBone; }
			}
			if ( iBestBone!=-1)
			{
				// confirm bone as best (in order)
				pBFVI [ (iIndexToVertex*dwNumBones)+iBestBone ].fFinalWeight=fBest;

				// record its position so we can refer to the correct bone
				piUseBone [ (iIndexToVertex*4)+iBestFour ] = iBestBone;

				// clear so not included in next iteration
				pBFVI [ (iIndexToVertex*dwNumBones)+iBestBone ].fWorkWeight=0.0f;
			}
			else
			{
				// no more weights, can break out
				break;
			}
		}
		if ( iBestFour == 4 )
		{
			// and add extra four one influences if flagged
			int iNextFour = 0;
			for (; iNextFour<4; iNextFour++ )
			{
				int iExtraBestBone = -1;
				float fExtraBest = 0.0f;
				for ( int iBone = 0; iBone < ( int ) dwNumBones; iBone++ )
				{
					// get weight and compare for best one
					float fWeight = pBFVI [ (iIndexToVertex*dwNumBones)+iBone ].fWorkWeight;
					if ( fWeight>fExtraBest ) { fExtraBest=fWeight; iExtraBestBone=iBone; }
				}
				if ( iExtraBestBone!=-1)
				{
					pBFVI [ (iIndexToVertex*dwNumBones)+iExtraBestBone ].fFinalWeight=fExtraBest;
					piUseBoneExtra [ (iIndexToVertex*4)+iNextFour ] = iExtraBestBone;
					pBFVI [ (iIndexToVertex*dwNumBones)+iExtraBestBone ].fWorkWeight=0.0f;
				}
				else
				{
					// no more extra weights, can break out
					break;
				}
			}
		}
	}

	// create new vertex data to include bone data
	DWORD dwNewFVFSize = pMesh->dwFVFSize+16+16;
	if ( dwExtraWeightOffsetInBytes > 0 ) dwNewFVFSize = dwNewFVFSize+16+16;
	DWORD dwNewSize = dwNumVertices * dwNewFVFSize;
	BYTE* pNewVertexData = new BYTE[dwNewSize];
	memset ( pNewVertexData, 0, sizeof(pNewVertexData) );
	BYTE* pWritePtr = pNewVertexData;
	BYTE* pReadPtr = pMesh->pVertexData;
	for ( DWORD v = 0; v < dwNumVertices; v++ )
	{
		memcpy ( pWritePtr, pReadPtr, pMesh->dwFVFSize );
		pWritePtr += pMesh->dwFVFSize + 16 + 16;
		if ( dwExtraWeightOffsetInBytes > 0 ) pWritePtr += 16 + 16;
		pReadPtr += pMesh->dwFVFSize;
	}

	// vertex data - populate with best four
	if ( pNewVertexData )
	{
		// get actual DWORD offsets
		DWORD dwWeightOffset = dwWeightOffsetInBytes/4;
		DWORD dwIndicesOffset = dwIndicesOffsetInBytes/4;
		DWORD dwExtraWeightOffset = dwExtraWeightOffsetInBytes/4;
		DWORD dwExtraIndicesOffset = dwExtraIndicesOffsetInBytes/4;

		// go through vertex data
		for ( DWORD v = 0; v < dwNumVertices; v++ )
		{
			// base ptr for vertex
			DWORD* pVertexPtr = (DWORD*)(pNewVertexData+(v*dwVertSizeInBytes));

			// clear weight and indices memarea
			memset ( pVertexPtr+dwWeightOffset, 0, sizeof ( float ) * 8 );

			// indexes to the bones we will use
			int iBoneA = piUseBone [ (v*4)+0 ];
			int iBoneB = piUseBone [ (v*4)+1 ];
			int iBoneC = piUseBone [ (v*4)+2 ];
			int iBoneD = piUseBone [ (v*4)+3 ];

			// indices for this vertex (references to bone index)
			if ( iBoneA>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+0) = (float)iBoneA;
			if ( iBoneB>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+1) = (float)iBoneB;
			if ( iBoneC>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+2) = (float)iBoneC;
			if ( iBoneD>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+3) = (float)iBoneD;

			// weights for this vertex (the weight to use against the bone)
			if ( iBoneA>=0 ) *(float*)(pVertexPtr+dwWeightOffset+0) = pBFVI [ (v*dwNumBones)+iBoneA ].fFinalWeight;
			if ( iBoneB>=0 ) *(float*)(pVertexPtr+dwWeightOffset+1) = pBFVI [ (v*dwNumBones)+iBoneB ].fFinalWeight;
			if ( iBoneC>=0 ) *(float*)(pVertexPtr+dwWeightOffset+2) = pBFVI [ (v*dwNumBones)+iBoneC ].fFinalWeight;
			if ( iBoneD>=0 ) *(float*)(pVertexPtr+dwWeightOffset+3) = pBFVI [ (v*dwNumBones)+iBoneD ].fFinalWeight;

			// 121018 - and also add extra indices and weights if present
			if ( dwExtraWeightOffset > 0 )
			{
				memset ( pVertexPtr+dwExtraWeightOffset, 0, sizeof ( float ) * 8 );
				int iBoneE = piUseBoneExtra [ (v*4)+0 ];
				int iBoneF = piUseBoneExtra [ (v*4)+1 ];
				int iBoneG = piUseBoneExtra [ (v*4)+2 ];
				int iBoneH = piUseBoneExtra [ (v*4)+3 ];
				if ( iBoneE>=0 ) *(float*)(pVertexPtr+dwExtraIndicesOffset+0) = (float)iBoneE;
				if ( iBoneF>=0 ) *(float*)(pVertexPtr+dwExtraIndicesOffset+1) = (float)iBoneF;
				if ( iBoneG>=0 ) *(float*)(pVertexPtr+dwExtraIndicesOffset+2) = (float)iBoneG;
				if ( iBoneH>=0 ) *(float*)(pVertexPtr+dwExtraIndicesOffset+3) = (float)iBoneH;
				if ( iBoneE>=0 ) *(float*)(pVertexPtr+dwExtraWeightOffset+0) = pBFVI [ (v*dwNumBones)+iBoneE ].fFinalWeight;
				if ( iBoneF>=0 ) *(float*)(pVertexPtr+dwExtraWeightOffset+1) = pBFVI [ (v*dwNumBones)+iBoneF ].fFinalWeight;
				if ( iBoneG>=0 ) *(float*)(pVertexPtr+dwExtraWeightOffset+2) = pBFVI [ (v*dwNumBones)+iBoneG ].fFinalWeight;
				if ( iBoneH>=0 ) *(float*)(pVertexPtr+dwExtraWeightOffset+3) = pBFVI [ (v*dwNumBones)+iBoneH ].fFinalWeight;
			}
		}
	}

	// now copy new vertex data to mesh
	SAFE_DELETE(pMesh->pVertexData);
	pMesh->dwFVFSize = dwNewFVFSize;
	pMesh->pVertexData = pNewVertexData;

	// free usages
	SAFE_DELETE ( pBFVI );
	SAFE_DELETE ( piUseBone );
	SAFE_DELETE ( piUseBoneExtra );
	#else
	// leeadd - 200204 - put animation bone data ( weights and indices ) into the vertex data (for VS skinning)
	DWORD dwNumVertices = pVertexShaderMesh->GetNumVertices();
	DWORD dwNumBones = pMesh->dwBoneCount;

	// create maximum of bones-per-vertex for collection purposes
	struct sBoneForVertInfo
	{
		float fWorkWeight;
		float fFinalWeight;
	};
	sBoneForVertInfo* pBFVI = new sBoneForVertInfo [ dwNumVertices * dwNumBones ];
	memset ( pBFVI, 0, sizeof( sBoneForVertInfo ) * dwNumVertices * dwNumBones );

	// collect bonesdata for all verts
	for ( int iBone = 0; iBone < ( int ) dwNumBones; iBone++ )
	{
		// go through all influenced bones
		for ( int iVert = 0; iVert < ( int ) pMesh->pBones [ iBone ].dwNumInfluences; iVert++ )
		{
			// get the vertex and weight
			int iIndexToVertex = pMesh->pBones [ iBone ].pVertices [ iVert ];
			float fWeight = pMesh->pBones [ iBone ].pWeights [ iVert ];

			// add this influence of weight to the pool under the correct bone index
			pBFVI [ (iIndexToVertex*dwNumBones)+iBone ].fWorkWeight = fWeight;
		}
	}

	// prepare an array to hold the best four
	int* piUseBone = new int [ dwNumVertices * 4 ];
	memset ( piUseBone, 0, sizeof( int ) * dwNumVertices * 4 );

	// work out the best four per vertex
	for ( DWORD iIndexToVertex = 0; iIndexToVertex < dwNumVertices; iIndexToVertex++ )
	{
		// fill with minus ones to indicate no bone at all
		for ( int iFillFlag = 0; iFillFlag<4; iFillFlag++ )
			piUseBone [ (iIndexToVertex*4)+iFillFlag ] = -1;

		// mark out best four
		for ( int iBestFour = 0; iBestFour<4; iBestFour++ )
		{
			// find best
			int iBestBone = -1;
			float fBest = 0.0f;
			for ( int iBone = 0; iBone < ( int ) dwNumBones; iBone++ )
			{
				// get weight and compare for best one
				float fWeight = pBFVI [ (iIndexToVertex*dwNumBones)+iBone ].fWorkWeight;
				if ( fWeight>fBest ) { fBest=fWeight; iBestBone=iBone; }
			}
			if ( iBestBone!=-1)
			{
				// confirm bone as best (in order)
				pBFVI [ (iIndexToVertex*dwNumBones)+iBestBone ].fFinalWeight=fBest;

				// record its position so we can refer to the correct bone
				piUseBone [ (iIndexToVertex*4)+iBestFour ] = iBestBone;

				// clear so not included in next iteration
				pBFVI [ (iIndexToVertex*dwNumBones)+iBestBone ].fWorkWeight=0.0f;
			}
			else
			{
				// no more weights, can break out
				break;
			}
		}
	}

	// lock vertex data and populate with best four
	BYTE* pVertexDataPtr = NULL;
	pVertexShaderMesh->LockVertexBuffer(D3DLOCK_READONLY, (VOID**)&pVertexDataPtr);
	if ( pVertexDataPtr )
	{
		// get actual DWORD offsets
		DWORD dwWeightOffset = dwWeightOffsetInBytes/4;
		DWORD dwIndicesOffset = dwIndicesOffsetInBytes/4;

		// go through vertex data
		for ( DWORD v = 0; v < dwNumVertices; v++ )
		{
			// base ptr for vertex
			DWORD* pVertexPtr = (DWORD*)(pVertexDataPtr+(v*dwVertSizeInBytes));

			// clear weight and indices memarea
			memset ( pVertexPtr+dwWeightOffset, 0, sizeof ( float ) * 8 );

			// indexes to the bones we will use
			int iBoneA = piUseBone [ (v*4)+0 ];
			int iBoneB = piUseBone [ (v*4)+1 ];
			int iBoneC = piUseBone [ (v*4)+2 ];
			int iBoneD = piUseBone [ (v*4)+3 ];

			// indices for this vertex (references to bone index)
			if ( iBoneA>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+0) = (float)iBoneA;
			if ( iBoneB>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+1) = (float)iBoneB;
			if ( iBoneC>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+2) = (float)iBoneC;
			if ( iBoneD>=0 ) *(float*)(pVertexPtr+dwIndicesOffset+3) = (float)iBoneD;

			// weights for this vertex (the weight to use against the bone)
			if ( iBoneA>=0 ) *(float*)(pVertexPtr+dwWeightOffset+0) = pBFVI [ (v*dwNumBones)+iBoneA ].fFinalWeight;
			if ( iBoneB>=0 ) *(float*)(pVertexPtr+dwWeightOffset+1) = pBFVI [ (v*dwNumBones)+iBoneB ].fFinalWeight;
			if ( iBoneC>=0 ) *(float*)(pVertexPtr+dwWeightOffset+2) = pBFVI [ (v*dwNumBones)+iBoneC ].fFinalWeight;
			if ( iBoneD>=0 ) *(float*)(pVertexPtr+dwWeightOffset+3) = pBFVI [ (v*dwNumBones)+iBoneD ].fFinalWeight;
		}

		// unlock vertex data
		pVertexShaderMesh->UnlockVertexBuffer();
	}

	// free usages
	SAFE_DELETE ( pBFVI );
	SAFE_DELETE ( piUseBone );
	#endif

	// complete
	return;
}

DARKSDK_DLL void FlipNormals ( sMesh* pMesh, int iFlipMode )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have normals in the vertices
	if ( offsetMap.dwNZ>0 )
	{
		// go through all of the vertices
		DWORD dwNumberOfVertices=pMesh->dwVertexCount;
		for ( int iCurrentVertex = 0; iCurrentVertex < (int)dwNumberOfVertices; iCurrentVertex++ )
		{
			GGVECTOR3 vecNorm = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) );
			if ( iFlipMode == 0 )
			{
				vecNorm.x *= -1;
				vecNorm.y *= -1;
				vecNorm.z *= -1;
			}
			*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) ) = vecNorm;
		}
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void GenerateNewNormalsForMesh	( sMesh* pMesh, int iMode )
{
	#ifdef DX11
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have normals in the vertices
	if ( offsetMap.dwNZ>0 )
	{
		// go through index buffer or raw vertice list
		bool bUsingIndices = true;
		DWORD iCount = pMesh->dwIndexCount;
		if ( iCount == 0 ) { iCount = pMesh->dwVertexCount; bUsingIndices = false; }

		// go through all polys, work out normal, then apply to normal vectors
		for ( DWORD i=0; i<iCount; i+=3 )
		{
			// read face
			DWORD dwFace0, dwFace1, dwFace2;
			if ( bUsingIndices == true )
			{
				dwFace0 = pMesh->pIndices[i+0];
				dwFace1 = pMesh->pIndices[i+1];
				dwFace2 = pMesh->pIndices[i+2];
			}
			else
			{
				dwFace0 = i+0;
				dwFace1 = i+1;
				dwFace2 = i+2;
			}

			// get vertex
			GGVECTOR3 vecVert0 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwFace0 ) );
			GGVECTOR3 vecVert1 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwFace1 ) );
			GGVECTOR3 vecVert2 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwFace2 ) );

			// get normal
			GGVECTOR3 vecNorm0 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace0 ) );
			GGVECTOR3 vecNorm1 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace1 ) );
			GGVECTOR3 vecNorm2 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace2 ) );

			// calculate normal from vertices
			GGVECTOR3 vNormal;
			GGVec3Cross ( &vNormal, &( vecVert2 - vecVert1 ), &( vecVert0 - vecVert1 ) );
			GGVec3Normalize ( &vNormal, &vNormal );

			// apply new normal to geometry for all normals associated with the poly
			*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace0 ) ) = vNormal;
			*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace1 ) ) = vNormal;
			*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace2 ) ) = vNormal;
		}
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );

	#else
	if ( pMesh )
	{
		// use DX to generate new normals for this mesh
		if ( iMode == 0 )
		{
			LPGGMESH pDXMesh = LocalMeshToDXMesh ( pMesh, NULL, 0 );
			if ( pDXMesh )
			{
				D3DXComputeNormals ( pDXMesh, NULL );
				UpdateLocalMeshWithDXMesh ( pMesh, pDXMesh );
				SAFE_RELEASE(pDXMesh);
			}
		}

		// special mode can flip all normals in mesh (corrects bad exports)
		if ( iMode == 1 )
		{
			FlipNormals ( pMesh, 0 );
		}
	}
	#endif
}

DARKSDK_DLL void GenerateNormalsForMesh ( sMesh* pMesh, int iMode )
{
	#ifdef DX11
	#else
	// work vars
	GGVERTEXELEMENT Declaration[MAX_FVF_DECL_SIZE];
	GGVERTEXELEMENT End = GDECL_END();
	int iElem;

	// 16 or 32 bit mesh size
	DWORD dwSysteMemFlag = D3DXMESH_SYSTEMMEM;
	if ( pMesh->dwVertexCount > 65535 ) dwSysteMemFlag = D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT;

	// get DX mesh from sMesh
	LPGGMESH pDXMesh = LocalMeshToDXMesh ( pMesh, NULL, 0 );
	if ( pDXMesh==NULL )
		return;

	// extract declaration from mesh
	pDXMesh->GetDeclaration( Declaration );

	// check if mesh already has a normal component
	BOOL bHasNormals = FALSE;
	for( iElem=0; Declaration[iElem].Stream != End.Stream; iElem++ )
	{   
		if( Declaration[iElem].Usage == GGDECLUSAGE_NORMAL )
		{
			bHasNormals = TRUE;
			break;
		}
	}

	// Update Mesh Semantics if does not have normals
	if( !bHasNormals ) 
	{
		Declaration[iElem].Stream = 0;
		Declaration[iElem].Offset = (WORD)pDXMesh->GetNumBytesPerVertex();
		Declaration[iElem].Type = GGDECLTYPE_FLOAT3;
		Declaration[iElem].Method = GGDECLMETHOD_DEFAULT;
		Declaration[iElem].Usage = GGDECLUSAGE_NORMAL;
		Declaration[iElem].UsageIndex = 0;
		Declaration[iElem+1] = End;
		LPGGMESH pTempMesh;
		HRESULT hr = pDXMesh->CloneMesh( dwSysteMemFlag, Declaration, m_pD3D, &pTempMesh );
		if( SUCCEEDED( hr ) )
		{
			SAFE_RELEASE( pDXMesh );
			pDXMesh = pTempMesh;
			D3DXComputeNormals ( pDXMesh, NULL );
		}
	}

	// update sMesh with new DX mesh
	UpdateLocalMeshWithDXMesh ( pMesh, pDXMesh );

	// free usages
	SAFE_RELEASE(pDXMesh);
	#endif
}

DARKSDK_DLL void GenerateExtraDataForMeshEx ( sMesh* pMesh, BOOL bNormals, BOOL bTangents, BOOL bBinormals, BOOL bDiffuse, BOOL bBones, DWORD dwGenerateMode )
{
	#ifdef DX11

	// get FVF details
	sOffsetMap offsetMap;
	GetFVFValueOffsetMap ( pMesh->dwFVF, &offsetMap );

	// deactivate bone flag if no bones in source mesh
	if ( pMesh->dwBoneCount==0 ) bBones=FALSE;

	// valid mesh (no longer using DXMESH)
	if ( pMesh->dwFVF > 0 )
	{
		// extract vertex size from mesh
		WORD wNumBytesPerVertex = (WORD)pMesh->dwFVFSize;

		// Starting declaration
		int iDeclarationIndex = 0;
		D3D11_INPUT_ELEMENT_DESC pDeclaration[12];

		// check if mesh already has a component (and build declaration)
		BOOL bHasNormals = FALSE;
		BOOL bHasDiffuse = FALSE;
		BOOL bHasTangents = FALSE;
		BOOL bHasBinormals = FALSE;
		BOOL bHasBlendWeights = FALSE;
		BOOL bHasBlendIndices = FALSE;
		BOOL bHasSecondaryUVs = FALSE;
		if ( pMesh->dwFVF & GGFVF_XYZ )
		{
			pDeclaration[iDeclarationIndex].SemanticName = "POSITION";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = 0;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
		}
		if ( pMesh->dwFVF & GGFVF_NORMAL ) 
		{
			pDeclaration[iDeclarationIndex].SemanticName = "NORMAL";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			bHasNormals = TRUE;
		}
		if ( pMesh->dwFVF & GGFVF_TEX1 || offsetMap.dwTU[0] > 0 ) // lightmapped objects failed this test!
		{
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
		}
		if ( pMesh->dwFVF & GGFVF_DIFFUSE ) 
		{
			pDeclaration[iDeclarationIndex].SemanticName = "COLOR";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			bHasDiffuse = TRUE;
		}
		//if ( pMesh->dwFVF & dsadadsa ) bHasTangents = TRUE; // No FVF specifies TANGENT/BINORMALS
		//if ( pMesh->dwFVF & dsadadsa ) bHasBinormals = TRUE;
		//if ( pMesh->dwFVF & offsetMap.dwTU[1] > 0 ) 
		if ( offsetMap.dwTU[1] > 0 ) // 290618 - small fix removed the & operation?!
		{
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 1;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			bHasSecondaryUVs = TRUE;
		}

		// 290618 - allow secondary UV objects to have tangent and binormals calculated (for lightmapped PBR objects)
		// objects that have TWO UV channels cause declaration problems when generating data
		// so we strip out any secondary UV channels from declaration (if generate bit two is one)
		//if ( bHasSecondaryUVs==TRUE )
		//{
		//	MessageBox ( NULL, "Secondary UV data not accepted", "GenerateExtraData", MB_OK );
		//	return;
		//}

		// Update Mesh Semantics if does not have components
		bool bGiveMeNormals = false;
		bool bGiveMeDiffuse = false;
		bool bGiveMeTangents = false;
		bool bGiveMeBinormals = false;
		bool bGiveMeBlendWeights = false;
		bool bGiveMeBlendIndices = false;
		if( !bHasNormals && bNormals ) 
		{
			bGiveMeNormals = true;
			pDeclaration[iDeclarationIndex].SemanticName = "NORMAL";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex+=12;
		}
		if( !bHasDiffuse && bDiffuse ) 
		{
			bGiveMeDiffuse = true;
			pDeclaration[iDeclarationIndex].SemanticName = "COLOR";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex+=4;
		}
		if( !bHasTangents && bTangents ) 
		{
			bGiveMeTangents = true;
			pDeclaration[iDeclarationIndex].SemanticName = "TANGENT";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex+=12;
		}
		if( !bHasBinormals && bBinormals ) 
		{
			bGiveMeBinormals = true;
			pDeclaration[iDeclarationIndex].SemanticName = "BINORMAL";
			pDeclaration[iDeclarationIndex].SemanticIndex = 0;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex+=12;
		}
		DWORD dwOffsetToWeights = wNumBytesPerVertex;
		if( !bHasBlendWeights && bBones ) 
		{
			bGiveMeBlendWeights = true;
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 1;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex+=16;
		}
		DWORD dwOffsetToIndices = wNumBytesPerVertex;
		if( !bHasBlendIndices && bBones ) 
		{
			bGiveMeBlendIndices = true;
			pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
			pDeclaration[iDeclarationIndex].SemanticIndex = 2;
			pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			pDeclaration[iDeclarationIndex].InputSlot = 0;
			pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
			iDeclarationIndex++;
			wNumBytesPerVertex+=16;
		}

		// 121018 - add extra weights if required
		DWORD dwOffsetToExtraWeights = 0;
		DWORD dwOffsetToExtraIndices = 0;
		if ( pMesh->pVertexShaderEffect )
		{
			if ( pMesh->pVertexShaderEffect->m_bNeed8BonesPerVertex == true )
			{
				dwOffsetToExtraWeights = wNumBytesPerVertex;
				if( !bHasBlendWeights && bBones ) 
				{
					pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
					pDeclaration[iDeclarationIndex].SemanticIndex = 3;
					pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					pDeclaration[iDeclarationIndex].InputSlot = 0;
					pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
					pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
					iDeclarationIndex++;
					wNumBytesPerVertex+=16;
				}
				dwOffsetToExtraIndices = wNumBytesPerVertex;
				if( !bHasBlendIndices && bBones ) 
				{
					pDeclaration[iDeclarationIndex].SemanticName = "TEXCOORD";
					pDeclaration[iDeclarationIndex].SemanticIndex = 4;
					pDeclaration[iDeclarationIndex].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					pDeclaration[iDeclarationIndex].InputSlot = 0;
					pDeclaration[iDeclarationIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
					pDeclaration[iDeclarationIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					pDeclaration[iDeclarationIndex].InstanceDataStepRate = 0;
					iDeclarationIndex++;
					wNumBytesPerVertex+=16;
				}
			}
		}

		// Compute any tangent basis data (no not attempt to correct mesh too much)
		if ( bGiveMeNormals || bGiveMeTangents || bGiveMeBinormals )
		{
			// cannot use FixTangents on bone based model, it adds verts to mess up bone skin (and does not work well)
			//PE: Tangent is not calculated correct, this calculation will be moved.
			//PE: Tangent calculation added to vertex shaders.
			//PE: Lee: We can consider to remove tangent,binormal all together should give a nice boost as we do not need to sent it to the GPU.
			//PE: in the dx9 version i was also calculating it in the shaders , so this is an old bug.
			//PE: issue https://github.com/TheGameCreators/GameGuruRepo/issues/85
			ComputeTangentBasisEx ( pMesh, bGiveMeNormals, bGiveMeTangents, bGiveMeBinormals, false, false, true );
		}

		// Fill blend data (weight and indices) if required 
		if ( bGiveMeBlendWeights || bGiveMeBlendIndices )
		{
			// fills mesh with additional data
			ComputeBoneDataInsideVertex ( pMesh, pMesh, dwOffsetToWeights, dwOffsetToIndices, dwOffsetToExtraWeights, dwOffsetToExtraIndices, wNumBytesPerVertex );

			// flag mesh as now being vertex skinned by a shader
			pMesh->bShaderBoneSkinning = true;
		}

		// copy declaration into old D3D9 format (as DBO relies on this data in the binary!)
		int iDecIndex = 0;
		int iByteOffset = 0;
		for (; iDecIndex < iDeclarationIndex; iDecIndex++ )
		{
			int iEntryByteSize = 0;
			if ( stricmp ( pDeclaration[iDecIndex].SemanticName, "POSITION" ) == NULL )
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_POSITION;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}
			if ( stricmp ( pDeclaration[iDecIndex].SemanticName, "NORMAL" ) == NULL )
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_NORMAL;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}
			if ( stricmp ( pDeclaration[iDecIndex].SemanticName, "COLOR" ) == NULL )
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_COLOR;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT2;
				iEntryByteSize = 4;
			}
			if ( stricmp ( pDeclaration[iDecIndex].SemanticName, "TANGENT" ) == NULL )
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_TANGENT;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}			
			if ( stricmp ( pDeclaration[iDecIndex].SemanticName, "BINORMAL" ) == NULL )
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_BINORMAL;
				pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT3;
				iEntryByteSize = 12;
			}			
			if ( stricmp ( pDeclaration[iDecIndex].SemanticName, "TEXCOORD" ) == NULL )
			{
				pMesh->pVertexDeclaration[iDecIndex].Usage = GGDECLUSAGE_TEXCOORD;
				if ( pDeclaration[iDecIndex].Format == DXGI_FORMAT_R32G32B32A32_FLOAT )
				{
					pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT4;
					iEntryByteSize = 16;
				}
				else
				{
					pMesh->pVertexDeclaration[iDecIndex].Type = GGDECLTYPE_FLOAT2;
					iEntryByteSize = 8;
				}
			}			
			pMesh->pVertexDeclaration[iDecIndex].Stream = 0;
			pMesh->pVertexDeclaration[iDecIndex].Method = GGDECLMETHOD_DEFAULT;
			pMesh->pVertexDeclaration[iDecIndex].UsageIndex = pDeclaration[iDecIndex].SemanticIndex;
			pMesh->pVertexDeclaration[iDecIndex].Offset = iByteOffset;
			iByteOffset += iEntryByteSize;
		}
		pMesh->pVertexDeclaration[iDecIndex] = GGDECLEND;
		#ifdef DX11
		pMesh->pVertexDeclaration[iDecIndex].Stream = 255;
		#endif
	}
	#else
	// work vars
	GGVERTEXELEMENT Declaration[MAX_FVF_DECL_SIZE];
	GGVERTEXELEMENT End = GDECL_END();
	int iElem;

	// deactivate bone flag if no bones in source mesh
	if ( pMesh->dwBoneCount==0 ) bBones=FALSE;

	// get DX mesh from sMesh
	LPGGMESH pDXMesh = LocalMeshToDXMesh ( pMesh, NULL, 0 );
	if ( pDXMesh )
	{
		// extract declaration and vertex size from mesh
		pDXMesh->GetDeclaration( Declaration );
		WORD wNumBytesPerVertex = (WORD)pDXMesh->GetNumBytesPerVertex();

		// check if mesh already has a component
		BOOL bHasNormals = FALSE;
		BOOL bHasDiffuse = FALSE;
		BOOL bHasTangents = FALSE;
		BOOL bHasBinormals = FALSE;
		BOOL bHasBlendWeights = FALSE;
		BOOL bHasBlendIndices = FALSE;
		BOOL bHasSecondaryUVs = FALSE;
		for( iElem=0; Declaration[iElem].Stream != End.Stream; iElem++ )
		{   
			if( Declaration[iElem].Usage == GGDECLUSAGE_NORMAL ) bHasNormals = TRUE;
			if( Declaration[iElem].Usage == D3DDECLUSAGE_COLOR ) bHasDiffuse = TRUE;
			if( Declaration[iElem].Usage == D3DDECLUSAGE_TANGENT ) bHasTangents = TRUE;
			if( Declaration[iElem].Usage == D3DDECLUSAGE_BINORMAL ) bHasBinormals = TRUE;
			if( Declaration[iElem].Usage == GGDECLUSAGE_TEXCOORD && Declaration[iElem].UsageIndex>0 ) bHasSecondaryUVs = TRUE;
		}

		// leefix - 071208 - U71 - objects that have TWO UV channels cause declaration problems when generating data
		// so we strip out any secondary UV channels from declaration (if generate bit two is one)
		if ( bHasSecondaryUVs==TRUE && dwGenerateMode==2 )
		{
			for(; iElem>=0; iElem-- )
			{
				if( Declaration[iElem].Usage == GGDECLUSAGE_TEXCOORD && Declaration[iElem].UsageIndex>0 )
				{
					Declaration[iElem] = Declaration[iElem+1];
					wNumBytesPerVertex -= 8; // assumes 8 bytes = texture coord
				}
			}
			for( iElem=0; Declaration[iElem].Stream != End.Stream; iElem++ ) {}
		}

		// Update Mesh Semantics if does not have components
		bool bGiveMeNormals = false;
		bool bGiveMeDiffuse = false;
		bool bGiveMeTangents = false;
		bool bGiveMeBinormals = false;
		bool bGiveMeBlendWeights = false;
		bool bGiveMeBlendIndices = false;
		if( !bHasNormals && bNormals ) 
		{
			bGiveMeNormals = true;
			Declaration[iElem].Stream = 0;
			Declaration[iElem].Offset = wNumBytesPerVertex;
			Declaration[iElem].Type = GGDECLTYPE_FLOAT3;
			Declaration[iElem].Method = GGDECLMETHOD_DEFAULT;
			Declaration[iElem].Usage = GGDECLUSAGE_NORMAL;
			Declaration[iElem].UsageIndex = 0;
			Declaration[iElem+1] = End;
			wNumBytesPerVertex+=12;
			iElem++;
		}
		if( !bHasDiffuse && bDiffuse ) 
		{
			bGiveMeDiffuse = true;
			Declaration[iElem].Stream = 0;
			Declaration[iElem].Offset = wNumBytesPerVertex;
			Declaration[iElem].Type = D3DDECLTYPE_FLOAT1;
			Declaration[iElem].Method = GGDECLMETHOD_DEFAULT;
			Declaration[iElem].Usage = D3DDECLUSAGE_COLOR;
			Declaration[iElem].UsageIndex = 0;
			Declaration[iElem+1] = End;
			wNumBytesPerVertex+=4;
			iElem++;
		}
		if( !bHasTangents && bTangents ) 
		{
			bGiveMeTangents = true;
			Declaration[iElem].Stream = 0;
			Declaration[iElem].Offset = wNumBytesPerVertex;
			Declaration[iElem].Type = GGDECLTYPE_FLOAT3;
			Declaration[iElem].Method = GGDECLMETHOD_DEFAULT;
			Declaration[iElem].Usage = D3DDECLUSAGE_TANGENT;
			Declaration[iElem].UsageIndex = 0;
			Declaration[iElem+1] = End;
			wNumBytesPerVertex+=12;
			iElem++;
		}
		//if( !bHasBinormals && bTangents ) // leefix - 050906 - now more specific for darkshader corrections
		if( !bHasBinormals && bBinormals ) 
		{
			bGiveMeBinormals = true;
			Declaration[iElem].Stream = 0;
			Declaration[iElem].Offset = wNumBytesPerVertex;
			Declaration[iElem].Type = GGDECLTYPE_FLOAT3;
			Declaration[iElem].Method = GGDECLMETHOD_DEFAULT;
			Declaration[iElem].Usage = D3DDECLUSAGE_BINORMAL;
			Declaration[iElem].UsageIndex = 0;
			Declaration[iElem+1] = End;
			wNumBytesPerVertex+=12;
			iElem++;
		}
		DWORD dwOffsetToWeights = wNumBytesPerVertex;
		if( !bHasBlendWeights && bBones ) 
		{
			bGiveMeBlendWeights = true;
			Declaration[iElem].Stream = 0;
			Declaration[iElem].Offset = wNumBytesPerVertex;
			Declaration[iElem].Type = D3DDECLTYPE_FLOAT4;
			Declaration[iElem].Method = GGDECLMETHOD_DEFAULT;
			Declaration[iElem].Usage = GGDECLUSAGE_TEXCOORD;
			Declaration[iElem].UsageIndex = 1;
			Declaration[iElem+1] = End;
			wNumBytesPerVertex+=16;
			iElem++;
		}
		DWORD dwOffsetToIndices = wNumBytesPerVertex;
		if( !bHasBlendIndices && bBones ) 
		{
			bGiveMeBlendIndices = true;
			Declaration[iElem].Stream = 0;
			Declaration[iElem].Offset = wNumBytesPerVertex;
			Declaration[iElem].Type = D3DDECLTYPE_FLOAT4;
			Declaration[iElem].Method = GGDECLMETHOD_DEFAULT;
			Declaration[iElem].Usage = GGDECLUSAGE_TEXCOORD;
			Declaration[iElem].UsageIndex = 2;
			Declaration[iElem+1] = End;
			wNumBytesPerVertex+=16;
			iElem++;
		}

		// create mesh from new declaration
		LPGGMESH pTempMesh = NULL;
		HRESULT hr;
		if ( pMesh->dwVertexCount>65535 )
			hr = pDXMesh->CloneMesh( D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT, Declaration, m_pD3D, &pTempMesh );
		else
			hr = pDXMesh->CloneMesh( D3DXMESH_SYSTEMMEM, Declaration, m_pD3D, &pTempMesh );
		if( SUCCEEDED( hr ) )
		{
			// free old mesh and switch to new cloned mesh
			SAFE_RELEASE( pDXMesh );
			pDXMesh = pTempMesh;

			// Compute any tangent basis data (no not attempt to correct mesh too much)
			if ( bGiveMeNormals || bGiveMeTangents || bGiveMeBinormals )
			{
				// leefix - 050906 - this 'used' to EAT UV1 & other data, giving back a basic POS+NORM+TEX+TANGENT mesh
				// leenote - 090217 - cannot use FixTangents on bone based model, it adds verts to mess up bone skin (and does not work well)
				//pDXMesh = ComputeTangentBasisEx ( pTempMesh, bGiveMeNormals, bGiveMeTangents, bGiveMeBinormals, true, false, true );
				pDXMesh = ComputeTangentBasisEx ( pTempMesh, bGiveMeNormals, bGiveMeTangents, bGiveMeBinormals, false, false, true );
			}

			// Fill blend data (weight and indices) if required 
			if ( bGiveMeBlendWeights || bGiveMeBlendIndices )
			{
				// fills mesh with additional data
				ComputeBoneDataInsideVertex ( pMesh, pDXMesh, dwOffsetToWeights, dwOffsetToIndices, wNumBytesPerVertex );

				// flag mesh as now being vertex skinned by a shader
				pMesh->bShaderBoneSkinning = true;
			}
		}

		// update sMesh with new DX mesh
		UpdateLocalMeshWithDXMesh ( pMesh, pDXMesh );

		// free usages
		SAFE_RELEASE(pDXMesh);
	}
	#endif
}

DARKSDK_DLL void GenerateExtraDataForMeshEx ( sMesh* pMesh, BOOL bNormals, BOOL bTangents, BOOL bBinormals, BOOL bDiffuse, BOOL bBones )
{
	GenerateExtraDataForMeshEx ( pMesh, bNormals, bTangents, bBinormals, bDiffuse, bBones, 0 );
}

DARKSDK_DLL void GenerateExtraDataForMesh ( sMesh* pMesh, BOOL bNormals, BOOL bTangents, BOOL bDiffuse, BOOL bBones )
{
	// leeadd - 050906 - added binormal seperation for U63 (for new darkshader)
	GenerateExtraDataForMeshEx ( pMesh, bNormals, bTangents, FALSE, bDiffuse, bBones );
}

DARKSDK_DLL void CopyReferencesToShaderEffects ( sMesh* pNewMesh, sMesh* pMesh )
{
	pNewMesh->bUseVertexShader = pMesh->bUseVertexShader;
	pNewMesh->pVertexShader = pMesh->pVertexShader;
	pNewMesh->pVertexDec = pMesh->pVertexDec;
	pNewMesh->bOverridePixelShader = pMesh->bOverridePixelShader;
	pNewMesh->pPixelShader = pMesh->pPixelShader;
	pNewMesh->pVertexShaderEffect = pMesh->pVertexShaderEffect;
	pNewMesh->bVertexShaderEffectRefOnly = true;
	pNewMesh->bShaderBoneSkinning = pMesh->bShaderBoneSkinning;
	pNewMesh->dwForceCPUAnimationMode = pMesh->dwForceCPUAnimationMode;
	strcpy ( pNewMesh->pEffectName, pMesh->pEffectName );
}

DARKSDK_DLL void CloneShaderEffects ( sMesh* pNewMesh, sMesh* pMesh )
{
	// Copy references as normal
	CopyReferencesToShaderEffects ( pNewMesh, pMesh );

	// Also make source a reference so effect cannot be deleted 9as now ised twice..)
	pMesh->bVertexShaderEffectRefOnly = true;
}

// buffer functions

DARKSDK_DLL bool CopyMeshDataToIndexBuffer ( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset )
{
	#ifdef DX11
	//if(0) //pObject->bDynamic )
	//{
	//	// if VB created with USAGE_DYNAMIC
	//	D3D11_MAPPED_SUBRESOURCE resource;
	//	memset ( &resource, 0, sizeof ( resource ) );
	//	if ( FAILED ( m_pImmediateContext->Map ( pIndexBufferRef, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &resource ) ) ) return false;
	//	unsigned char* pIndices = (unsigned char*)resource.pData;
	//	memcpy ( &pIndices[ dwBufferOffset ], pMesh->pIndices, sizeof ( WORD ) * pMesh->dwIndexCount );
	//	m_pImmediateContext->Unmap ( pIndexBufferRef, 0 );
	//}
	//else
	{
		// if VB created with USAGE_DEFAULT (fast GPU)
		D3D11_BOX box;
		box.left = dwBufferOffset;
		box.right = dwBufferOffset + ( sizeof ( WORD ) * pMesh->dwIndexCount );
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		m_pImmediateContext->UpdateSubresource ( pIndexBufferRef, 0, &box, pMesh->pIndices, 0, 0 );
	}
	#else
	// copy index data from mesh to buffer
	WORD* pIndices = NULL;
	pIndexBufferRef->Lock ( 0, 0, ( VOID** ) &pIndices, 0 );

	// copy across to the buffer
	memcpy ( &pIndices[ dwBufferOffset ], pMesh->pIndices, sizeof ( WORD ) * pMesh->dwIndexCount );

	// unlock the index buffer
	pIndexBufferRef->Unlock ( );
	#endif

	// all went okay
	return true;
}

DARKSDK_DLL bool CopyMeshDataToDWORDIndexBuffer ( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset )
{
	#ifdef DX11
	//if(0) //pObject->bDynamic )
	//{
	//	D3D11_MAPPED_SUBRESOURCE resource;
	//	memset ( &resource, 0, sizeof ( resource ) );
	//	if ( FAILED ( m_pImmediateContext->Map ( pIndexBufferRef, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &resource ) ) ) 
	//		return false;
	//
	//	// copy across WORD source data to the DWORD buffer
	//	unsigned char* pIndices = (unsigned char*)resource.pData;
	//	DWORD dwIter = dwBufferOffset;
	//	for ( DWORD i=0; i<pMesh->dwIndexCount; i++ )
	//		pIndices[ dwIter++ ] = pMesh->pIndices [ i ];
	//
	//	m_pImmediateContext->Unmap ( pIndexBufferRef, 0 );
	//}
	#else
	// copy index data from mesh to buffer
	DWORD* pIndices = NULL;
	pIndexBufferRef->Lock ( 0, 0, ( VOID** ) &pIndices, 0 );

	// copy across WORD source data to the DWORD buffer
	DWORD dwIter = dwBufferOffset;
	for ( DWORD i=0; i<pMesh->dwIndexCount; i++ )
		pIndices[ dwIter++ ] = pMesh->pIndices [ i ];

	// unlock the index buffer
	pIndexBufferRef->Unlock ( );
	#endif

	// all went okay
	return true;
}

DARKSDK_DLL bool CopyDWORDMeshDataToDWORDIndexBuffer ( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset )
{
	#ifdef DX11
	#else
	// copy index data from mesh to buffer
	DWORD* pIndices = NULL;
	pIndexBufferRef->Lock ( 0, 0, ( VOID** ) &pIndices, 0 );

	// copy across DWORD source data to the DWORD buffer
	DWORD dwIter = dwBufferOffset;
	DWORD* pDWORDPtr = (DWORD*)pMesh->pIndices;
	for ( DWORD i=0; i<pMesh->dwIndexCount; i++ )
		pIndices[ dwIter++ ] = pDWORDPtr [ i ];

	// unlock the index buffer
	pIndexBufferRef->Unlock ( );
	#endif

	// all went okay
	return true;
}

DARKSDK_DLL bool CopyMeshDataToVertexBufferSameFVF ( sMesh* pMesh, IGGVertexBuffer* pVertexBufferRef, DWORD dwBufferOffset )
{
	#ifdef DX11
	// lock the vertex buffer
	//if(0) //pObject->bDynamic )
	//{
	//	// for VB with USAGE_DYNAMIC
	//	D3D11_MAPPED_SUBRESOURCE resource;
	//	memset ( &resource, 0, sizeof ( resource ) );
	//	if ( FAILED ( m_pImmediateContext->Map ( pVertexBufferRef, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &resource ) ) ) return false;
	//	unsigned char* pfData = (unsigned char*)resource.pData;
	//	// copy across to the buffer
	//	memcpy ( &pfData[ dwBufferOffset ], pMesh->pVertexData, pMesh->dwFVFSize * pMesh->dwVertexCount );
	//	m_pImmediateContext->Unmap ( pVertexBufferRef, 0 );
	//}
	//else
	{
		// for VB with USAGE_DEFAULT
		D3D11_BOX box;
		box.left = dwBufferOffset;
		box.right = dwBufferOffset + ( pMesh->dwFVFSize * pMesh->dwVertexCount );
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		m_pImmediateContext->UpdateSubresource ( pVertexBufferRef, 0, &box, pMesh->pVertexData, 0, 0 );
	}
	#else
	// lock the vertex buffer
	float* pfData = NULL;
	if ( FAILED ( pVertexBufferRef->Lock ( 0, 0, ( VOID** ) &pfData, 0 ) ) )
		return false;

	// copy across to the buffer
	memcpy ( &pfData[ dwBufferOffset ], pMesh->pVertexData, pMesh->dwFVFSize * pMesh->dwVertexCount );

	// unlock the vertex buffer
	pVertexBufferRef->Unlock ( );
	#endif

	// all went okay
	return true;
}

DARKSDK_DLL bool CopyIndexBufferToMeshData ( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset )
{
	#ifdef DX11
	#else
	// copy index data from mesh to buffer
	WORD* pIndices = NULL;
	pIndexBufferRef->Lock ( 0, 0, ( VOID** ) &pIndices, 0 );

	// copy across to the buffer
	memcpy ( pMesh->pIndices, &pIndices[ dwBufferOffset ], sizeof ( WORD ) * pMesh->dwIndexCount );

	// unlock the index buffer
	pIndexBufferRef->Unlock ( );
	#endif

	// all went okay
	return true;
}

DARKSDK_DLL bool CopyVertexBufferToMeshDataSameFVF ( sMesh* pMesh, IGGVertexBuffer* pVertexBufferRef, DWORD dwBufferOffset )
{
	#ifdef DX11
	#else
	// lock the vertex buffer
	float* pfData = NULL;
	if ( FAILED ( pVertexBufferRef->Lock ( 0, 0, ( VOID** ) &pfData, 0 ) ) )
		return false;

	// copy across to the buffer
	memcpy ( pMesh->pVertexData, &pfData[ dwBufferOffset ], pMesh->dwFVFSize * pMesh->dwVertexCount );

	// unlock the vertex buffer
	pVertexBufferRef->Unlock ( );
	#endif

	// all went okay
	return true;
}

DARKSDK_DLL bool CopyIndexMeshData ( sMesh* pDstMesh, sMesh* pSrcMesh, DWORD dwOffset, DWORD dwIndexCount )
{
	// copy across
	memcpy ( pDstMesh->pIndices + dwOffset, pSrcMesh->pIndices, sizeof(WORD) * dwIndexCount );
	return true;
}

DARKSDK_DLL bool IncrementIndexMeshData ( sMesh* pDstMesh, DWORD dwOffset, DWORD dwIndexCount, DWORD dwIncrement )
{
	// increment index data (used when adding a mesh to end of another mesh)
	WORD* pIndexData = pDstMesh->pIndices + dwOffset;
	for ( int iIndex=0; iIndex<(int)dwIndexCount; iIndex++ )
	{
		*pIndexData = (WORD)(*pIndexData + dwIncrement);
		pIndexData++;
	}
	return true;
}

DARKSDK_DLL bool CopyVertexMeshDataSameFVF ( sMesh* pDstMesh, sMesh* pSrcMesh, DWORD dwOffset, DWORD dwVertexCount )
{
	// copy across
	memcpy ( pDstMesh->pVertexData + ( dwOffset * pSrcMesh->dwFVFSize ), pSrcMesh->pVertexData, pSrcMesh->dwFVFSize * dwVertexCount );
	return true;
}

DARKSDK_DLL void SplitMeshSide ( int iSide, sMesh* pMesh, sMesh* pSplitMesh )
{
	// determine direction of side
	GGVECTOR3 vecDirection;
	switch ( iSide )
	{
		case 0 : vecDirection = GGVECTOR3 ( 0.0f, 0.0f, 1.0f );
		case 1 : vecDirection = GGVECTOR3 ( 0.0f, 0.0f,-1.0f );
		case 2 : vecDirection = GGVECTOR3 ( 0.0f,-1.0f, 0.0f );
		case 3 : vecDirection = GGVECTOR3 ( 0.0f, 1.0f, 0.0f );
		case 4 : vecDirection = GGVECTOR3 (-1.0f, 0.0f, 0.0f );
		case 5 : vecDirection = GGVECTOR3 ( 1.0f, 0.0f, 0.0f );
	}

	// copy index data from mesh to single-mesh
	BYTE* pVertexData = (BYTE*)pMesh->pVertexData;
	BYTE* pNormalData = pVertexData + ( sizeof(float)*3 );
	WORD* pIndexData = (WORD*)pMesh->pIndices;

	// transform vertex data by world matrix of frame
	for ( DWORD i=0; i<pMesh->dwIndexCount; i+=3 )
	{
		// get poly vertex normals
		DWORD v0 = pIndexData[i+0];
		DWORD v1 = pIndexData[i+1];
		DWORD v2 = pIndexData[i+2];
		GGVECTOR3* pNormal0 = (GGVECTOR3*)(pNormalData+(v0*pMesh->dwFVFSize));
		GGVECTOR3* pNormal1 = (GGVECTOR3*)(pNormalData+(v1*pMesh->dwFVFSize));
		GGVECTOR3* pNormal2 = (GGVECTOR3*)(pNormalData+(v2*pMesh->dwFVFSize));
		
		// work out poly normal
		GGVECTOR3 vecPolyNormal = *pNormal0 + *pNormal1 + *pNormal2;
		GGVec3Normalize ( &vecPolyNormal, &vecPolyNormal );

		// if facing node side, split it
		if ( GGVec3Dot ( &vecPolyNormal, &vecDirection ) > 0.0f )
		{
			// hide polygon fpr test
			*((GGVECTOR3*)(pVertexData+(v1*pMesh->dwFVFSize))) = *((GGVECTOR3*)(pVertexData+(v0*pMesh->dwFVFSize)));
			*((GGVECTOR3*)(pVertexData+(v2*pMesh->dwFVFSize))) = *((GGVECTOR3*)(pVertexData+(v0*pMesh->dwFVFSize)));
		}
	}
}

DARKSDK_DLL bool GetFrameCount ( sFrame* pFrame, int* piCount )
{
	// check if the frame and counter is okay is ok
	SAFE_MEMORY ( pFrame );
	SAFE_MEMORY ( piCount );

	// increment the count
	//*piCount += 1;
	// get frames for the sibling and child
	//GetFrameCount ( pFrame->pChild,   piCount );
	//GetFrameCount ( pFrame->pSibling, piCount );

	// avoids recirsion issue
	sFrame* pThis = pFrame;
	while ( pThis )
	{
		*piCount += 1;
		GetFrameCount ( pThis->pChild, piCount );
		pThis = pThis->pSibling;
	}

	return true;
}

DARKSDK_DLL bool BuildFrameList ( sFrame** pFrameList, sFrame* pFrame, int* iStart )
{
	// check if the pointers are valid
	SAFE_MEMORY ( pFrameList );
	SAFE_MEMORY ( pFrame );
	SAFE_MEMORY ( iStart );

	//pFrameList [ *iStart ] = pFrame;
	//pFrame->iID = *iStart;
	//*iStart += 1;
	// keep on calling build, we need to run through all items in the list
	//BuildFrameList ( pFrameList, pFrame->pChild, iStart );
	//BuildFrameList ( pFrameList, pFrame->pSibling, iStart );

	// avoids recirsion issue
	sFrame* pThis = pFrame;
	while ( pThis )
	{
		pFrameList [ *iStart ] = pThis;
		pThis->iID = *iStart;
		*iStart += 1;

		// and next down to tackle children (if any)
		BuildFrameList ( pFrameList, pThis->pChild, iStart );

		// next one
		pThis = pThis->pSibling;
	}

	return true;
}

DARKSDK_DLL bool GetMeshCount ( sFrame* pFrame, int* piCount )
{
	// check if the pointers are valids
	SAFE_MEMORY ( pFrame );
	SAFE_MEMORY ( piCount );

	// see if we have a mesh
	//if ( pFrame->pMesh ) *piCount += 1;
	//GetMeshCount ( pFrame->pChild,   piCount );
	//GetMeshCount ( pFrame->pSibling, piCount );

	// avoids recirsion issue
	sFrame* pThis = pFrame;
	while ( pThis )
	{
		if ( pThis->pMesh ) *piCount += 1;
		GetMeshCount ( pThis->pChild, piCount );
		pThis = pThis->pSibling;
	}

	return true;
}

DARKSDK_DLL bool BuildMeshList ( sMesh** pMeshList, sFrame* pFrame, int* iStart )
{
	// check if the pointers are valid
	SAFE_MEMORY ( pMeshList );
	SAFE_MEMORY ( pFrame );
	SAFE_MEMORY ( iStart );

	// see if we have a mesh
	//if ( pFrame->pMesh )
	//{
	//	pMeshList [ *iStart ] = pFrame->pMesh;
	//	pFrame->iID = *iStart;
	//	*iStart += 1;
	//}
	//BuildMeshList ( pMeshList, pFrame->pChild, iStart );
	//BuildMeshList ( pMeshList, pFrame->pSibling, iStart );

	// recursive causes stack overflow on large linklists, do old fashioned way
	// delete all pointers which will in turn delete all sub frames
	// keep on calling build, we need to run through all items in the list
	//BuildMeshList ( pMeshList, pFrame->pChild, iStart );
	//BuildMeshList ( pMeshList, pFrame->pSibling, iStart );
	sFrame* pThis = pFrame;
	while ( pThis )
	{
		// see if we have a mesh
		if ( pThis->pMesh )
		{
			pMeshList [ *iStart ] = pThis->pMesh;
			pThis->iID = *iStart;
			*iStart += 1;
		}

		// and next down to tackle children (if any)
		BuildMeshList ( pMeshList, pThis->pChild, iStart );

		// next one
		pThis = pThis->pSibling;
	}

	return true;
}

DARKSDK_DLL sFrame* FindFrame ( char* szName, sFrame* pFrame )
{
	// finds a frame with a given name, this function will
	// search through any sub frames of the frame passed in

	// make sure the frame is valid
	if ( !pFrame )
		return NULL;

	// if no name is specified return this frame
	if ( !szName )
		return pFrame;

	// set up a sub frame for more searches
	sFrame* pSubFrame = NULL;

	// compare names and return if exact match
	if ( pFrame->szName != NULL && !strcmp ( szName, pFrame->szName ) )
		return pFrame;

	// see if there is a child frame we can check
	if ( pFrame->pChild )
	{
		// see if we have a match with the child frame
		if ( ( pSubFrame = FindFrame ( szName, pFrame->pChild ) ) != NULL )
			return pSubFrame;
	}

	// see if there is a sibling frame
	if ( pFrame->pSibling )
	{
		// see if we have a match with the sibling frame
		if ( ( pSubFrame = FindFrame ( szName, pFrame->pSibling ) ) != NULL )
			return pSubFrame;
	}

	// no match found in the hiearachy
	return NULL;
}

DARKSDK_DLL sAnimation* FindAnimation ( sObject* pObject, sFrame* pOriginalFrame )
{
	// use original frame name to find pAnim and duplicate for new frame
	if ( pObject->pAnimationSet )
	{
		sAnimation* pCurrentAnim = pObject->pAnimationSet->pAnimation;
		if ( pCurrentAnim )
		{
			// go through all anims
			while ( pCurrentAnim )
			{
				if ( _stricmp ( pCurrentAnim->szName, pOriginalFrame->szName )==NULL )
				{
					// Found animation
					return pCurrentAnim;
				}
				pCurrentAnim = pCurrentAnim->pNext;
			}
		}
	}

	// could not find
	return NULL;
}

DARKSDK_DLL sAnimation* CopyAnimation ( sAnimation* pCurrentAnim, LPSTR szNewName )
{
	// found original anim data - copy it
	sAnimation* pNewAnim = new sAnimation;

	// copy over values
	strcpy ( pNewAnim->szName,		  szNewName );
	pNewAnim->bLoop					= pCurrentAnim->bLoop;
	pNewAnim->bLinear				= pCurrentAnim->bLinear;
	pNewAnim->dwNumPositionKeys		= pCurrentAnim->dwNumPositionKeys;
	pNewAnim->dwNumRotateKeys		= pCurrentAnim->dwNumRotateKeys;
	pNewAnim->dwNumScaleKeys		= pCurrentAnim->dwNumScaleKeys;
	pNewAnim->dwNumMatrixKeys		= pCurrentAnim->dwNumMatrixKeys;

	// create arrays and copy data
	pNewAnim->pPositionKeys = new sPositionKey[pCurrentAnim->dwNumPositionKeys];
	memcpy ( pNewAnim->pPositionKeys, pCurrentAnim->pPositionKeys, sizeof(sPositionKey)*pCurrentAnim->dwNumPositionKeys );
	pNewAnim->pRotateKeys = new sRotateKey[pCurrentAnim->dwNumRotateKeys];
	memcpy ( pNewAnim->pRotateKeys, pCurrentAnim->pRotateKeys, sizeof(sRotateKey)*pCurrentAnim->dwNumRotateKeys );
	pNewAnim->pScaleKeys = new sScaleKey[pCurrentAnim->dwNumScaleKeys];
	memcpy ( pNewAnim->pScaleKeys, pCurrentAnim->pScaleKeys, sizeof(sScaleKey)*pCurrentAnim->dwNumScaleKeys );
	pNewAnim->pMatrixKeys = new sMatrixKey[pCurrentAnim->dwNumMatrixKeys];
	memcpy ( pNewAnim->pMatrixKeys, pCurrentAnim->pMatrixKeys, sizeof(sMatrixKey)*pCurrentAnim->dwNumMatrixKeys );

	// add new anim to existing list
	while ( pCurrentAnim->pNext ) pCurrentAnim = pCurrentAnim->pNext;
	pCurrentAnim->pNext = pNewAnim;

	return pNewAnim;
}

DARKSDK_DLL bool SortAnimationPositionByTime ( sAnimation* pAnim, bool bDoTheCostlySort )
{
	// leefix - 270203 - some animation data is not time sorted (required for keyframe finder)
	SAFE_MEMORY ( pAnim->pPositionKeys );

	// store the number of keys
	DWORD dwNumKeys = pAnim->dwNumPositionKeys;

	if ( bDoTheCostlySort == true )
	{
		// bubble sort into time ascending order (or key-frame select gets messed up)
		for ( int iKeyA = 0; iKeyA < ( int ) dwNumKeys; iKeyA++ )
		{
			for ( int iKeyB = iKeyA; iKeyB < ( int ) dwNumKeys; iKeyB++ )
			{
				if ( iKeyA!=iKeyB )
				{
					if ( pAnim->pPositionKeys [ iKeyB ].dwTime < pAnim->pPositionKeys [ iKeyA ].dwTime ) 
					{
						// swap A and B
						sPositionKey sStoreA = pAnim->pPositionKeys [ iKeyA ];
						pAnim->pPositionKeys [ iKeyA ] = pAnim->pPositionKeys [ iKeyB ];
						pAnim->pPositionKeys [ iKeyB ] = sStoreA;
					}
				}
			}
		}
	}

	// work out interpolation data after keyframes sorted
	if ( dwNumKeys > 1 )
	{
		for ( int iKey = 0; iKey < ( int ) dwNumKeys - 1; iKey++ )
		{
			DWORD dwTime;

			pAnim->pPositionKeys [ iKey ].vecPosInterpolation = pAnim->pPositionKeys [ iKey + 1 ].vecPos - pAnim->pPositionKeys [ iKey ].vecPos;
			dwTime                                            = pAnim->pPositionKeys [ iKey + 1 ].dwTime - pAnim->pPositionKeys [ iKey ].dwTime;
			
			if ( !dwTime )
				dwTime = 1;

			pAnim->pPositionKeys [ iKey ].vecPosInterpolation /= ( float ) dwTime;
		}
	}

	// success
	return true;
}

DARKSDK_DLL bool SortAnimationRotationByTime ( sAnimation* pAnim, bool bDoTheCostlySort )
{
	// leefix - 270203 - some animation data is not time sorted (required for keyframe finder)
	SAFE_MEMORY ( pAnim->pRotateKeys );

	// store the number of keys
	DWORD dwNumKeys = pAnim->dwNumRotateKeys;
	
	if ( bDoTheCostlySort == true )
	{
		// bubble sort into time ascending order (or key-frame select gets messed up)
		for ( int iKeyA = 0; iKeyA < ( int ) dwNumKeys; iKeyA++ )
		{
			for ( int iKeyB = iKeyA; iKeyB < ( int ) dwNumKeys; iKeyB++ )
			{
				if ( iKeyA!=iKeyB )
				{
					if ( pAnim->pRotateKeys [ iKeyB ].dwTime < pAnim->pRotateKeys [ iKeyA ].dwTime ) 
					{
						// swap A and B
						sRotateKey sStoreA = pAnim->pRotateKeys [ iKeyA ];
						pAnim->pRotateKeys [ iKeyA ] = pAnim->pRotateKeys [ iKeyB ];
						pAnim->pRotateKeys [ iKeyB ] = sStoreA;
					}
				}
			}
		}
	}

	// success
	return true;
}

DARKSDK_DLL bool SortAnimationScaleByTime ( sAnimation* pAnim, bool bDoTheCostlySort )
{
	// leefix - 270203 - some animation data is not time sorted (required for keyframe finder)
	SAFE_MEMORY ( pAnim->pScaleKeys );

	// store the number of keys
	DWORD dwNumKeys = pAnim->dwNumScaleKeys;
	
	if ( bDoTheCostlySort == true )
	{
		// bubble sort into time ascending order (or key-frame select gets messed up)
		for ( int iKeyA = 0; iKeyA < ( int ) dwNumKeys; iKeyA++ )
		{
			for ( int iKeyB = iKeyA; iKeyB < ( int ) dwNumKeys; iKeyB++ )
			{
				if ( iKeyA!=iKeyB )
				{
					if ( pAnim->pScaleKeys [ iKeyB ].dwTime < pAnim->pScaleKeys [ iKeyA ].dwTime ) 
					{
						// swap A and B
						sScaleKey sStoreA = pAnim->pScaleKeys [ iKeyA ];
						pAnim->pScaleKeys [ iKeyA ] = pAnim->pScaleKeys [ iKeyB ];
						pAnim->pScaleKeys [ iKeyB ] = sStoreA;
					}
				}
			}
		}
	}

	// work out interpolation data after keyframes sorted
	if ( dwNumKeys > 1 )
	{
		for ( int iKey = 0; iKey < ( int ) dwNumKeys - 1; iKey++ )
		{
			DWORD dwTime = 0;

			pAnim->pScaleKeys [ iKey ].vecScaleInterpolation = pAnim->pScaleKeys [ iKey + 1 ].vecScale - pAnim->pScaleKeys [ iKey ].vecScale;
			dwTime                                           = pAnim->pScaleKeys [ iKey + 1 ].dwTime   - pAnim->pScaleKeys [ iKey ].dwTime;
			
			if ( !dwTime )
				dwTime = 1;

			pAnim->pScaleKeys [ iKey ].vecScaleInterpolation /= ( float ) dwTime;
		}
	}

	// success
	return true;
}

DARKSDK_DLL bool SortAnimationMatrixByTime ( sAnimation* pAnim, bool bDoTheCostlySort )
{
	// store the number of keys
	DWORD dwNumKeys = pAnim->dwNumMatrixKeys;
	
	// bubble sort into time ascending order (or key-frame select gets messed up)
	if ( bDoTheCostlySort == true )
	{
		for ( int iKeyA = 0; iKeyA < ( int ) dwNumKeys; iKeyA++ )
		{
			for ( int iKeyB = iKeyA; iKeyB < ( int ) dwNumKeys; iKeyB++ )
			{
				if ( iKeyA!=iKeyB )
				{
					if ( pAnim->pMatrixKeys [ iKeyB ].dwTime < pAnim->pMatrixKeys [ iKeyA ].dwTime ) 
					{
						// swap A and B
						sMatrixKey sStoreA = pAnim->pMatrixKeys [ iKeyA ];
						pAnim->pMatrixKeys [ iKeyA ] = pAnim->pMatrixKeys [ iKeyB ];
						pAnim->pMatrixKeys [ iKeyB ] = sStoreA;
					}
				}
			}
		}
	}

	// work out interpolation data after keyframes sorted
	if ( dwNumKeys > 1 )
	{
		for ( int iKey = 0; iKey < ( int ) dwNumKeys - 1; iKey++ )
		{
			DWORD dwTime = 0;
			pAnim->pMatrixKeys [ iKey ].matInterpolation = pAnim->pMatrixKeys [ iKey + 1 ].matMatrix - pAnim->pMatrixKeys [ iKey ].matMatrix;
			dwTime = pAnim->pMatrixKeys [ iKey + 1 ].dwTime - pAnim->pMatrixKeys [ iKey ].dwTime;
			if ( !dwTime ) dwTime = 1;
			pAnim->pMatrixKeys [ iKey ].matInterpolation /= ( float ) dwTime;
		}
	}

	// success
	return true;
}

DARKSDK_DLL bool SortAnimationDataByTime ( sAnimation* pAnim, bool bCostlySort )
{
	// sort position, rotation and scale data and calc interpolations
	SortAnimationPositionByTime	( pAnim, bCostlySort );
	SortAnimationRotationByTime	( pAnim, bCostlySort );
	SortAnimationScaleByTime	( pAnim, bCostlySort );

	// 060718 - need to organise matrix keyframes too
	SortAnimationMatrixByTime	( pAnim, bCostlySort );

	// success
	return true;
}

bool MapFramesToAnimations ( sObject* pObject, bool bCostlySort )
{
	// go through the animation and find the frames which are used
	// we then store a pointer to the frame in the anim structure

	// leefix - 171203 - for MD*, we can first for sublist count
	pObject->fAnimTotalFrames = 0.0f;
	for ( int iMesh=0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		if ( pMesh )
		{
			float fNewMax = ( float ) pMesh->dwSubMeshListCount;
			if ( fNewMax > pObject->fAnimTotalFrames )
			{
				pObject->fAnimTotalFrames = fNewMax;
				pObject->fAnimFrameEnd = fNewMax;
			}
		}
	}

	// check the memory we need to access
	SAFE_MEMORY ( pObject );
	SAFE_MEMORY ( pObject->pAnimationSet );
	SAFE_MEMORY ( pObject->pAnimationSet->pAnimation );

	// setup some local variables
	sAnimationSet* pAnimSet = NULL;
	sAnimation*    pAnim    = NULL;

	// LEEFIX - 171203 - this cannot work as some MD* do not have 'pAnimationSet'
	if ( !pObject->pAnimationSet->pAnimation->bBoneType )
	{
		pObject->fAnimTotalFrames = ( float ) pObject->pAnimationSet->pAnimation->iBoneFrameA - 1;
		pObject->fAnimFrameEnd    = pObject->fAnimTotalFrames;
		return true;
	}

	// get a pointer to the animation set
	pAnimSet = pObject->pAnimationSet;

	// get total length of animation
	DWORD dwTotalLength=0;

	// we need to run through all of the set
	LONG lStartPerf = timeGetTime();
	while ( pAnimSet != NULL )
	{
		// get a pointer to the animation data
		pAnim = pAnimSet->pAnimation;

		// run through all animation sequences
		int iCountMe = 0;
		LONG lStartPerf2 = timeGetTime();
		while ( pAnim != NULL )
		{
			// scans all animation data and creates the interpolation vectors between all keyframes (vital)
			if ( pAnim ) SortAnimationDataByTime ( pAnim, bCostlySort );

			// find the frame which matches the animation
			pAnim->pFrame = FindFrame ( pAnim->szName, pObject->pFrame );	// find matching frame

			// and also associate frame with anim, so can find internal animation structure
			if ( pAnim->pFrame ) pAnim->pFrame->pAnimRef = pAnim;

			// next
			pAnim         = pAnim->pNext;									// move to next animation sequence
			iCountMe++;
		}
		lStartPerf2 = timeGetTime() - lStartPerf2;

		// update total length
		if ( pAnimSet->ulLength > dwTotalLength ) dwTotalLength = pAnimSet->ulLength;

		// move to next animation set
		pAnimSet = pAnimSet->pNext;
	}
	lStartPerf = timeGetTime() - lStartPerf;

	// store total frames in object
	pObject->fAnimTotalFrames = (float)dwTotalLength;
	pObject->fAnimFrameEnd = pObject->fAnimTotalFrames;

	// return back to caller
	return true;
}

void InitOneMeshFramesToBones ( sMesh* pMesh )
{
	// create the frame mapping matrix array and clear out
	pMesh->pFrameRef = new sFrame* [ pMesh->dwBoneCount ];
	pMesh->pFrameMatrices = new GGMATRIX* [ pMesh->dwBoneCount ];

	// set all matrix pointers to null
	for ( int i = 0; i < ( int ) pMesh->dwBoneCount; i++ )
	{
		pMesh->pFrameRef [ i ] = NULL;
		pMesh->pFrameMatrices [ i ] = NULL;
	}
}

DARKSDK_DLL bool InitFramesToBones ( sMesh** pMeshList, int iMeshCount )
{
	// check mesh list
	SAFE_MEMORY ( *pMeshList );

	// get first mesh
	for ( int iMesh=0; iMesh < iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pMeshList [ iMesh ];
		if ( pMesh )
		{
			// create the frame mapping matrix array and clear out
			InitOneMeshFramesToBones ( pMesh );
		}
	}

	// okay
	return true;
}

bool MapFramesToBones ( sMesh** pMesh, sFrame *pFrame, int iCount )
{
	// check mesh and frame
	SAFE_MEMORY ( *pMesh );
	SAFE_MEMORY ( pFrame );

	// current position
	int iPos = 0;

	// get first mesh
	sMesh* pMain = pMesh [ iPos ];

	// scan through meshes looking for bone matches
	if ( pMain != NULL )
	{
		// run through all meshes in list
		while ( 1 )
		{
			// get mesh and advance
			pMain = pMesh [ iPos++ ];

			// only update if we are dealing with bones
			if ( pMain->dwBoneCount )
			{
				// get list of bone names
				for ( DWORD i = 0; i < pMain->dwBoneCount; i++ )
				{
					// find the frame which matches the bone
					if ( strcmp ( pFrame->szName, pMain->pBones [ i ].szName ) == 0 )
					{
						// leeadd - 180204 - also store reference to frame (for bone-anim model limb based collision)
						pMain->pFrameRef [ i ] = pFrame;

						// lee - 021114 - record WHICH bone is referenced so UpdateFrame can use SkinOffset local transform matrix from bone
						pFrame->pmatBoneLocalTransform = &pMain->pBones [ i ].matTranslation;

						// store the matrix
						pMain->pFrameMatrices [ i ] = &pFrame->matCombined;

						// done
						break;
					}
				}
			}

			if ( iPos == iCount )
				break;
		}
	}

	// scan through child frames
	MapFramesToBones ( pMesh, pFrame->pChild, iCount );
	
	// scan through sibling frames
	MapFramesToBones ( pMesh, pFrame->pSibling, iCount );

	return true;
}

bool MapOneMeshFramesToBones ( sMesh* pMain, sFrame* pFrame )
{
	// only update if we are dealing with bones
	if ( pMain->dwBoneCount )
	{
		// get list of bone names
		for ( DWORD i = 0; i < pMain->dwBoneCount; i++ )
		{
			// find the frame which matches the bone
			if ( strcmp ( pFrame->szName, pMain->pBones [ i ].szName ) == 0 )
			{
				pMain->pFrameRef [ i ] = pFrame;
				pMain->pFrameMatrices [ i ] = &pFrame->matCombined;
				break;
			}
		}
	}

	// scan through child frames
	if ( pFrame->pChild ) MapOneMeshFramesToBones ( pMain, pFrame->pChild );
	
	// scan through sibling frames
	if ( pFrame->pSibling ) MapOneMeshFramesToBones ( pMain, pFrame->pSibling );

	return true;
}

DARKSDK_DLL void UpdateObjectCamDistance ( sObject* pObject )
{
	// using mesh LOD style ONLY
	if ( pObject->iUsingWhichLOD!=-1000 )
	{
		// store current LOD index
		int iStoreLOD = pObject->iUsingWhichLOD;

		// handle any LOD assigning (only if not in transition)
		if ( pObject->iUsingOldLOD==-1 )
		{
			if ( pObject->position.fCamDistance < pObject->fLODDistance [ 0 ] || pObject->fLODDistance [ 0 ]==0 )
				pObject->iUsingWhichLOD = 0;
			else
			{
				if ( pObject->position.fCamDistance < pObject->fLODDistance [ 1 ] || pObject->fLODDistance [ 1 ]==0 )
					pObject->iUsingWhichLOD = 1;
				else
				{
					if ( pObject->position.fCamDistance < pObject->fLODDistanceQUAD || pObject->fLODDistanceQUAD==0 )
						pObject->iUsingWhichLOD = 2;
					else
						pObject->iUsingWhichLOD = 3;
				}
			}

			// leeadd - U71 - add transitional code
			if ( iStoreLOD != pObject->iUsingWhichLOD )
			{
				// LOD level has changed due to distance, so initiate transition effect (handled in objectmanager)
				pObject->iUsingOldLOD = iStoreLOD;
				pObject->fLODTransition = 0.0f;
			}
		}
	}
}

DARKSDK_DLL void UpdateObjectAnimation ( sObject* pObject )
{
	// handle vertex level animation (even if not animating)
	if ( pObject->pAnimationSet )
	{
		// only need to go for first frame for MDL models
		if ( !pObject->pAnimationSet->pAnimation->bBoneType )
		{
			// just animate the first mesh MDL
			AnimateBoneMesh ( pObject, pObject->ppFrameList [ 0 ] );
			pObject->fAnimLastFrame = pObject->fAnimFrame;
			pObject->fAnimSlerpLastTime = pObject->fAnimSlerpTime;
		}
	}

	// U74 - 120409 - not NECESSARILY animation data uses below (remove if ( pObject->pAnimationSet ))
	if ( 1 )
	{		
		// all meshes in object
		for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
		{
			// get a pointer to the frame
			sFrame* pFrame = pObject->ppFrameList [ iFrame ];
			if ( pFrame==NULL ) continue;

			// determine if we need to animate
			bool bProceedToAnimate = false;
			if ( pObject->pAnimationSet && pObject->fAnimTotalFrames > 0.0f ) bProceedToAnimate = true;
			if ( pObject->bAnimUpdateOnce ) bProceedToAnimate = true;

			// 130513 - no need to animate limbs we have hidden (useful when hiding LOD meshes)
			if ( pFrame->pMesh )
				if ( pFrame->pMesh->bVisible==false )
					bProceedToAnimate = false;

			// if object is parent of instance, must animate ALL (main mesh and LOD versions)
			if ( pObject->position.bParentOfInstance )
			{
				if ( bProceedToAnimate )
				{
					// animate all meshes
					sMesh* pMesh = NULL;
					pMesh = pFrame->pMesh;
					if ( pMesh && pMesh->dwBoneCount > 0 ) AnimateBoneMesh ( pObject, pFrame, pMesh );
					pMesh = pFrame->pLOD[0];
					if ( pMesh && pMesh->dwBoneCount > 0 ) AnimateBoneMesh ( pObject, pFrame, pMesh );
					pMesh = pFrame->pLOD[1];
					if ( pMesh && pMesh->dwBoneCount > 0 ) AnimateBoneMesh ( pObject, pFrame, pMesh );
					pMesh = pFrame->pLODForQUAD;
					if ( pMesh && pMesh->dwBoneCount > 0 ) AnimateBoneMesh ( pObject, pFrame, pMesh );

					// Update regular bounds
					UpdateBoundBoxMesh ( pFrame );
					UpdateBoundSphereMesh ( pFrame );
				}
			}
			else
			{
				// choose mesh based on any LOD distance
				sMesh* pMesh = NULL;
				if ( pObject->iUsingWhichLOD==-1000 )
				{
					// uses built-in distance based LOD mesh selector (only animate LOD being used/visible)
					if ( pFrame->pMesh )
					{
						if ( pFrame->pMesh->bVisible==true )
						{
							pMesh = pFrame->pMesh;
						}
					}
				}
				else
				{
					if ( pObject->iUsingWhichLOD==0 )
					{
						// normal object-mesh rendering
						pMesh = pFrame->pMesh;
					}
					else
					{
						if ( pObject->iUsingWhichLOD==1 )
						{
							// LOD2
							pMesh = pFrame->pLOD[0];
						}
						else
						{
							if ( pObject->iUsingWhichLOD==2 )
							{
								// LOD3
								pMesh = pFrame->pLOD[1];
							}
							else
							{
								// LODQUAD
								pMesh = pFrame->pLODForQUAD;
							}
						}
					}
				}

				// for each mesh
				if ( pMesh )
				{
					// use mesh bone animation
					if ( bProceedToAnimate )
					{
						// anim can have matrix data and NO bones
						if ( pMesh->dwBoneCount > 0 )
						{
							// animate bones with matrix animation data
							AnimateBoneMesh ( pObject, pFrame, pMesh );

							// Update regular bounds
							UpdateBoundBoxMesh ( pFrame );
							UpdateBoundSphereMesh ( pFrame );
						}
					}
				}

				// also animate any old LOD mesh in transition
				sMesh* pOldLODMesh = NULL;
				if ( pObject->iUsingOldLOD==0 )
					pOldLODMesh = pFrame->pMesh;
				else
				{
					if ( pObject->iUsingOldLOD==1 )
						pOldLODMesh = pFrame->pLOD[0];
					else
					{
						if ( pObject->iUsingOldLOD==2 )
							pOldLODMesh = pFrame->pLOD[1];
						else
							if ( pObject->iUsingOldLOD==3 )
								pOldLODMesh = pFrame->pLODForQUAD;
					}
				}
				if ( pOldLODMesh )
					if ( pObject->fAnimTotalFrames > 0.0f || pObject->bAnimUpdateOnce )
						if ( pOldLODMesh->dwBoneCount > 0 )
							AnimateBoneMesh ( pObject, pFrame, pOldLODMesh );
			}
		}
		
		// reset value to here
		// pObject->bAnimUpdateOnce = false;	// U75 - 051209 - caused DarkPHYSICS ragdoll rope demo to fail!

		// Store frame for next quick reject check
		pObject->fAnimLastFrame = pObject->fAnimFrame;
		pObject->fAnimSlerpLastTime = pObject->fAnimSlerpTime;
	}
}

DARKSDK_DLL bool SetupShortVertex ( DWORD dwFVF, BYTE* pVertex, int iOffset, float x, float y, float z, float tu, float tv )
{
	// check the memory pointer is valid
	SAFE_MEMORY ( pVertex );

	// get the offset map
	sOffsetMap	offsetMap;
	GetFVFValueOffsetMap ( dwFVF, &offsetMap );

	// we can only work with any valid formats
	if ( dwFVF & GGFVF_XYZ )
	{
		*( ( float* ) pVertex + offsetMap.dwX       + ( offsetMap.dwSize * iOffset ) ) = x;
		*( ( float* ) pVertex + offsetMap.dwY       + ( offsetMap.dwSize * iOffset ) ) = y;
		*( ( float* ) pVertex + offsetMap.dwZ       + ( offsetMap.dwSize * iOffset ) ) = z;
	}
	if ( dwFVF & GGFVF_TEX1 )
	{
		*( ( float* ) pVertex + offsetMap.dwTU[0]      + ( offsetMap.dwSize * iOffset ) ) = tu;
		*( ( float* ) pVertex + offsetMap.dwTV[0]      + ( offsetMap.dwSize * iOffset ) ) = tv;
	}

	// okay
	return true;
}

DARKSDK_DLL bool SetupStandardVertex ( DWORD dwFVF, BYTE* pVertex, int iOffset, float x, float y, float z, float nx, float ny, float nz, DWORD dwDiffuseColour, float tu, float tv )
{
	// setup a standard lit vertex

	// check the memory pointer is valid
	SAFE_MEMORY ( pVertex );

	// get the offset map
	sOffsetMap	offsetMap;
	GetFVFValueOffsetMap ( dwFVF, &offsetMap );

	// we can only work with any valid formats
	if ( dwFVF & GGFVF_XYZ )
	{
		*( ( float* ) pVertex + offsetMap.dwX       + ( offsetMap.dwSize * iOffset ) ) = x;
		*( ( float* ) pVertex + offsetMap.dwY       + ( offsetMap.dwSize * iOffset ) ) = y;
		*( ( float* ) pVertex + offsetMap.dwZ       + ( offsetMap.dwSize * iOffset ) ) = z;
	}
	if ( dwFVF & GGFVF_NORMAL )
	{
		*( ( float* ) pVertex + offsetMap.dwNX      + ( offsetMap.dwSize * iOffset ) ) = nx;
		*( ( float* ) pVertex + offsetMap.dwNY      + ( offsetMap.dwSize * iOffset ) ) = ny;
		*( ( float* ) pVertex + offsetMap.dwNZ      + ( offsetMap.dwSize * iOffset ) ) = nz;
	}
	if ( dwFVF & GGFVF_DIFFUSE )
	{
		*( ( DWORD* ) pVertex + offsetMap.dwDiffuse + ( offsetMap.dwSize * iOffset ) ) = dwDiffuseColour;
	}
	if ( dwFVF & GGFVF_TEX1 )
	{
		*( ( float* ) pVertex + offsetMap.dwTU[0]      + ( offsetMap.dwSize * iOffset ) ) = tu;
		*( ( float* ) pVertex + offsetMap.dwTV[0]      + ( offsetMap.dwSize * iOffset ) ) = tv;
	}

	// mike - 160903 - point size support, set to 1.0f by default
	if ( dwFVF & GGFVF_PSIZE )
	{
		*( ( float* ) pVertex + offsetMap.dwPointSize + ( offsetMap.dwSize * iOffset ) ) = 1.0f;
	}

	// okay
	return true;
}

DARKSDK_DLL bool SetupStandardVertexDec ( sMesh* pMesh, BYTE* pVertex, int iOffset, float x, float y, float z, float nx, float ny, float nz, DWORD dwDiffuseColour, float tu, float tv )
{
	// setup a standard lit vertex

	// check the memory pointer is valid
	SAFE_MEMORY ( pVertex );

	// get the offset map
	sOffsetMap	offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// we can only work with any valid formats
	if ( offsetMap.dwZ>0 )
	{
		*( ( float* ) pVertex + offsetMap.dwX       + ( offsetMap.dwSize * iOffset ) ) = x;
		*( ( float* ) pVertex + offsetMap.dwY       + ( offsetMap.dwSize * iOffset ) ) = y;
		*( ( float* ) pVertex + offsetMap.dwZ       + ( offsetMap.dwSize * iOffset ) ) = z;
	}
	if ( offsetMap.dwNZ>0 )
	{
		*( ( float* ) pVertex + offsetMap.dwNX      + ( offsetMap.dwSize * iOffset ) ) = nx;
		*( ( float* ) pVertex + offsetMap.dwNY      + ( offsetMap.dwSize * iOffset ) ) = ny;
		*( ( float* ) pVertex + offsetMap.dwNZ      + ( offsetMap.dwSize * iOffset ) ) = nz;
	}
	if ( offsetMap.dwDiffuse>0 )
	{
		*( ( DWORD* ) pVertex + offsetMap.dwDiffuse + ( offsetMap.dwSize * iOffset ) ) = dwDiffuseColour;
	}
	if ( offsetMap.dwTV[0]>0 )
	{
		*( ( float* ) pVertex + offsetMap.dwTU[0]      + ( offsetMap.dwSize * iOffset ) ) = tu;
		*( ( float* ) pVertex + offsetMap.dwTV[0]      + ( offsetMap.dwSize * iOffset ) ) = tv;
	}

	// mike - 160903 - point size support, set to 1.0f by default
	if ( offsetMap.dwPointSize>0 )
	{
		*( ( float* ) pVertex + offsetMap.dwPointSize + ( offsetMap.dwSize * iOffset ) ) = 1.0f;
	}

	// okay
	return true;
}

DARKSDK_DLL bool CreateFrameAndMeshList ( sObject* pObject )
{
	// 130213 - used to traverse 10,000 nests (no more) (avoids recursive stack overflow)
	int iNestCountMax = 9999;
	sFrame* pFrameBeforeNest[9999];
	sFrame* pThisFrame = NULL;
	int iNestCount = 0;

	// leeadd - 221105 - can make count negative efore obj call to protect mesh list
	if ( pObject->iMeshCount>=0 )
	{
		// find the number of meshes
		pObject->iMeshCount = 0;
		if ( 1 ) 
		{
			if ( !GetMeshCount ( pObject->pFrame, &pObject->iMeshCount ) )
				return false;
		}
		else
		{
			// does NOT produce same lists!!
			pThisFrame = pObject->pFrame;
			iNestCount = 0;
			while ( pThisFrame && iNestCount==0 )
			{	
				// act on frames
				if ( pThisFrame )
				{
					// count mesh
					if ( pThisFrame->pMesh ) pObject->iMeshCount++;

					// next item
					if ( pThisFrame->pChild )
					{
						pFrameBeforeNest[iNestCount] = pThisFrame;
						pThisFrame = pThisFrame->pChild;
						iNestCount++;
					}
					else
					{
						pThisFrame = pThisFrame->pSibling;
					}
				}
				if ( pThisFrame==NULL && iNestCount>0 )
				{
					iNestCount--;
					pThisFrame = pFrameBeforeNest[iNestCount];
					pThisFrame = pThisFrame->pSibling;
				}
			}
		}

		// allocate a list of frame which matches the number of meshes
		SAFE_DELETE_ARRAY ( pObject->ppMeshList );
		pObject->ppMeshList = new sMesh* [ pObject->iMeshCount ];
		SAFE_MEMORY ( pObject->ppMeshList );
	
		// get a list of frames which have meshes
		pObject->iMeshCount = 0;
		if ( 1 ) 
		{
			if ( !BuildMeshList ( pObject->ppMeshList, pObject->pFrame, &pObject->iMeshCount ) )
				return false;
		}
		else
		{
			// does NOT produce same lists!!
			pThisFrame = pObject->pFrame;
			iNestCount = 0;
			while ( pThisFrame && iNestCount==0 )
			{	
				// act on frames
				if ( pThisFrame )
				{
					// create mesh list entry
					if ( pThisFrame->pMesh )
					{
						pObject->ppMeshList [ pObject->iMeshCount ] = pThisFrame->pMesh;
						pThisFrame->iID = pObject->iMeshCount;
						pObject->iMeshCount++;
					}

					// next item
					if ( pThisFrame->pChild )
					{
						pFrameBeforeNest[iNestCount] = pThisFrame;
						pThisFrame = pThisFrame->pChild;
						iNestCount++;
					}
					else
					{
						pThisFrame = pThisFrame->pSibling;
					}
				}
				if ( pThisFrame==NULL && iNestCount>0 )
				{
					iNestCount--;
					pThisFrame = pFrameBeforeNest[iNestCount];
					pThisFrame = pThisFrame->pSibling;
				}
			}
		}
	}
	else
	{
		// negate count back - shows we have skipped this (kept old)
		pObject->iMeshCount *= -1;
	}

	// leeadd - 221105 - can make count negative efore obj call to protect frame list
	if ( pObject->iFrameCount>=0 )
	{
		pObject->iFrameCount = 0;
		if ( 1 ) 
		{
			if ( !GetFrameCount ( pObject->pFrame, &pObject->iFrameCount ) )
				return false;
		}
		else
		{
			// does NOT produce same lists!!
			pThisFrame = pObject->pFrame;
			iNestCount = 0;
			while ( pThisFrame && iNestCount==0 )
			{	
				// act on frames
				if ( pThisFrame )
				{
					// count frame
					pObject->iFrameCount++;

					// next item
					if ( pThisFrame->pChild )
					{
						pFrameBeforeNest[iNestCount] = pThisFrame;
						pThisFrame = pThisFrame->pChild;
						iNestCount++;
					}
					else
					{
						pThisFrame = pThisFrame->pSibling;
					}
				}
				if ( pThisFrame==NULL && iNestCount>0 )
				{
					iNestCount--;
					pThisFrame = pFrameBeforeNest[iNestCount];
					pThisFrame = pThisFrame->pSibling;
				}
			}
		}

		// allocate a list of frame which matches the number of frames
		SAFE_DELETE_ARRAY ( pObject->ppFrameList );
		pObject->ppFrameList = new sFrame* [ pObject->iFrameCount ];
		SAFE_MEMORY ( pObject->ppFrameList );

		// build up a list of frames
		pObject->iFrameCount = 0;
		if ( 1 )
		{
			if ( !BuildFrameList ( pObject->ppFrameList, pObject->pFrame, &pObject->iFrameCount ) )
				return false;
		}
		else
		{
			// does NOT produce same lists!!
			pThisFrame = pObject->pFrame;
			iNestCount = 0;
			while ( pThisFrame && iNestCount==0 )
			{	
				// act on frames
				if ( pThisFrame )
				{
					// create mesh list entry
					pObject->ppFrameList [ pObject->iFrameCount ] = pThisFrame;
					pThisFrame->iID = pObject->iFrameCount;
					pObject->iFrameCount++;

					// next item
					if ( pThisFrame->pChild )
					{
						pFrameBeforeNest[iNestCount] = pThisFrame;
						pThisFrame = pThisFrame->pChild;
						iNestCount++;
					}
					else
					{
						pThisFrame = pThisFrame->pSibling;
					}
				}
				if ( pThisFrame==NULL && iNestCount>0 )
				{
					iNestCount--;
					pThisFrame = pFrameBeforeNest[iNestCount];
					pThisFrame = pThisFrame->pSibling;
				}
			}
		}
	}
	else
	{
		// negate count back - shows we have skipped this (kept old)
		pObject->iFrameCount *= -1;
	}

	// if object has no meshes
	if ( pObject->iMeshCount==0 )
	{
		// do not attempt to draw it
		SAFE_DELETE ( pObject->ppMeshList );
		pObject->bNoMeshesInObject=true;
	}
	else
	{
		// default object does have meshes
		pObject->bNoMeshesInObject=false;
	}

	// okay
	return true;
}

DARKSDK_DLL bool SetupObjectsGenericProperties ( sObject* pObject )
{
	// check the object is valid
	SAFE_MEMORY ( pObject );
	SAFE_MEMORY ( pObject->pFrame );

	// create frame and mesh lists for object
	CreateFrameAndMeshList ( pObject );

	// calculate any user matrices from limb offset/rotate/scale data
	for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
	{
		sFrame* pFrame = pObject->ppFrameList [ iFrame ];
		if ( pFrame ) UpdateUserMatrix(pFrame);
	}

	// reset hierarchy and calculate combined frame matrix data
	GGMATRIX matrix;
	GGMatrixIdentity ( &matrix );
	ResetFrameMatrices ( pObject->pFrame );
	UpdateFrame ( pObject->pFrame, &matrix );

	// success
	return true;
}

DARKSDK_DLL void ComputeBoundValues ( int iPass, GGVECTOR3 vecXYZ, GGVECTOR3* pvecMin, GGVECTOR3* pvecMax, GGVECTOR3* pvecCenter, float* pfRadius )
{
	// passes
	// leeadd - 080305 - added pass one as the animated boundbox needs bounds calc
	if ( iPass==0 || iPass==1 )
	{
		// check to see if the vertices are within the bounds and set the appriopriate values
		if ( vecXYZ.x < pvecMin->x ) pvecMin->x = vecXYZ.x;
		if ( vecXYZ.y < pvecMin->y ) pvecMin->y = vecXYZ.y;
		if ( vecXYZ.z < pvecMin->z ) pvecMin->z = vecXYZ.z;
		if ( vecXYZ.x > pvecMax->x ) pvecMax->x = vecXYZ.x;
		if ( vecXYZ.y > pvecMax->y ) pvecMax->y = vecXYZ.y;
		if ( vecXYZ.z > pvecMax->z ) pvecMax->z = vecXYZ.z;
	}
	if ( iPass==0 )
	{
		// add to the centre vector
		*pvecCenter += vecXYZ;
	}
	if ( iPass!=0 )
	{
		// relative to center of sphere
		vecXYZ = vecXYZ - *pvecCenter;

		// get the square length of the vector
		float fDistSq = GGVec3LengthSq ( &vecXYZ );

		// see if it's larger than the current radius
		if ( fDistSq > *pfRadius ) *pfRadius = fDistSq;
	}
}

DARKSDK_DLL bool CalculateMeshBounds ( sMesh* pMesh )
{
	// ensure that the pMesh is valid
	SAFE_MEMORY ( pMesh );

	// set the initial min and max vectors to defaults
	pMesh->Collision.vecMin = GGVECTOR3 (  1000000.0f,  1000000.0f,  1000000.0f );
	pMesh->Collision.vecMax = GGVECTOR3 ( -1000000.0f, -1000000.0f, -1000000.0f );

	// set the initial values of the bounding sphere
	pMesh->Collision.vecCentre = GGVECTOR3 ( 0.0f, 0.0f, 0.0f );
	pMesh->Collision.fRadius   = 0.0f;

	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// for each mesh go through all of the vertices
	for ( int iPass=0; iPass < 2; iPass++ )
	{
		for ( int iCurrentVertex = 0; iCurrentVertex < ( int ) pMesh->dwVertexCount; iCurrentVertex++ )
		{
			// make sure we have position data in the vertices
			if ( offsetMap.dwZ>0 )
			{
				// get the x, y and z components
				GGVECTOR3 vecXYZ;
				vecXYZ.x = *( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iCurrentVertex ) );
				vecXYZ.y = *( ( float* ) pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iCurrentVertex ) );
				vecXYZ.z = *( ( float* ) pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iCurrentVertex ) );

				// compute bound box, center and radius
				ComputeBoundValues ( iPass, vecXYZ, &pMesh->Collision.vecMin,
													&pMesh->Collision.vecMax,
													&pMesh->Collision.vecCentre,
													&pMesh->Collision.fRadius		);
			}
		}
		if ( iPass==0 )
		{
			// divide the centre radius by the number of vertices and we'll get our centre position
			pMesh->Collision.vecCentre = pMesh->Collision.vecMin + ((pMesh->Collision.vecMax - pMesh->Collision.vecMin)/2.0f); 
		}
		else
		{
			// set the final radius value which is the square root of the current radius
			pMesh->Collision.fRadius = ( float ) sqrt ( pMesh->Collision.fRadius );
		}
	}

	// okay
	return true;
}

DARKSDK_DLL bool CalculateObjectBounds ( int iPass, sMesh* pMesh, GGMATRIX* pMatrix, GGVECTOR3* pvecMin, GGVECTOR3* pvecMax, GGVECTOR3* pvecCenter, float* pfRadius )
{
	// ensure that the pMesh is valid
	SAFE_MEMORY ( pMesh );

	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// minimum data for quick mesh test - better system is calculate and store offsetmap in sMesh (fvf or declaration based)
	if ( pMesh->dwFVF==0 )
	{
		offsetMap.dwX = 0;
		offsetMap.dwY = 1;
		offsetMap.dwZ = 2;
		offsetMap.dwSize = pMesh->dwFVFSize/4;
	}

	// for each mesh go through all of the vertices
	for ( int iCurrentVertex = 0; iCurrentVertex < ( int ) pMesh->dwVertexCount; iCurrentVertex++ )
	{
		// make sure we have position data in the vertices
		if ( (offsetMap.dwZ>0) || pMesh->dwFVF==0 )
		{
			// get the x, y and z components
			GGVECTOR3 vecXYZ;
			vecXYZ.x = *( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iCurrentVertex ) );
			vecXYZ.y = *( ( float* ) pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iCurrentVertex ) );
			vecXYZ.z = *( ( float* ) pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iCurrentVertex ) );

			// transform the vector to world coords
			GGVec3TransformCoord( &vecXYZ, &vecXYZ, pMatrix );

			// compute bound box, center and radius
			ComputeBoundValues ( iPass, vecXYZ, pvecMin, pvecMax, pvecCenter, pfRadius );
		}
	}

	// okay
	return true;
}

DARKSDK_DLL bool CalculateAllBounds ( sObject* pObject, bool bNotUsed )
{
	// ensure that the object is valid
	SAFE_MEMORY ( pObject );

	// u74b7 - get the largest scale
	float fBiggestScale = max (
		pObject->position.vecScale.x, max (
			pObject->position.vecScale.y,
			pObject->position.vecScale.z)
		);

	// set the initial min and max vectors to defaults
	pObject->collision.vecMin = GGVECTOR3 (  1000000.0f,  1000000.0f,  1000000.0f );
	pObject->collision.vecMax = GGVECTOR3 ( -1000000.0f, -1000000.0f, -1000000.0f );

	//if ( pObject->dwObjectNumber == 70002 )
	//{
	//	int lee=42;
	//}

	// set the initial values of the bounding sphere
	GGVECTOR3 vecRealObjectCenter;
	pObject->collision.vecCentre = GGVECTOR3 ( 0.0f, 0.0f, 0.0f );
	pObject->collision.fRadius = 0.0f;

	// leeadd - 080305 - array to store bounboxes per frame
	if ( pObject->pAnimationSet )
	{
		// lee - 280306 - u6rc3 - can have animset tag but no frames data
		if ( pObject->pAnimationSet->ulLength>0 )
		{
			// lee - 150206 - u60 - always have at least one slot for bounds vectors
			DWORD dwTotalFrames = pObject->pAnimationSet->ulLength;
			if ( dwTotalFrames==0 ) dwTotalFrames=1;

			// create dynamic boundbox arrays
			SAFE_DELETE(pObject->pAnimationSet->pvecBoundMin);
			pObject->pAnimationSet->pvecBoundMin = new GGVECTOR3 [ dwTotalFrames ];
			SAFE_DELETE(pObject->pAnimationSet->pvecBoundMax);
			pObject->pAnimationSet->pvecBoundMax = new GGVECTOR3 [ dwTotalFrames ];
			SAFE_DELETE(pObject->pAnimationSet->pvecBoundCenter);
			pObject->pAnimationSet->pvecBoundCenter = new GGVECTOR3 [ dwTotalFrames ];
			SAFE_DELETE(pObject->pAnimationSet->pfBoundRadius);
			pObject->pAnimationSet->pfBoundRadius = new float [ dwTotalFrames ];

			// reset boundboxes
			for ( int iKeyframe=0; iKeyframe<(int)dwTotalFrames; iKeyframe+=1 )
			{
				pObject->pAnimationSet->pvecBoundMin [ iKeyframe ] = GGVECTOR3 (  1000000.0f,  1000000.0f,  1000000.0f );
				pObject->pAnimationSet->pvecBoundMax [ iKeyframe ] = GGVECTOR3 ( -1000000.0f, -1000000.0f, -1000000.0f );
				pObject->pAnimationSet->pvecBoundCenter [ iKeyframe ] = GGVECTOR3 ( 0.0f, 0.0f, 0.0f );
				pObject->pAnimationSet->pfBoundRadius [ iKeyframe ] = 0.0f;
			}

			// U74 - 210409 - FPSCV115 - must animate before first pass or 'center' calc wrong (as animation can SCALE vertex data)
			if ( 1 )
			{
				// store anim flag
				bool bStoreAnimFlag = pObject->bAnimPlaying;
				float fStoreFrame = pObject->fAnimFrame;

				// perform animation processing of first keyframe (in case anim scales vertex data)
				for ( int iCurrentFrame = 0; iCurrentFrame < pObject->iFrameCount; iCurrentFrame++ )
				{
					// find frame within object
					sFrame* pFrame = pObject->ppFrameList [ iCurrentFrame ];
					if ( pFrame )
					{
						sMesh* pMesh = pFrame->pMesh;
						if ( pMesh )
						{
							pObject->bAnimPlaying=false;
							pObject->fAnimFrame=0.0f;
							GGMATRIX matrix;
							GGMatrixIdentity ( &matrix );
							UpdateAllFrameData ( pObject, pObject->fAnimFrame );
							UpdateFrame ( pObject->pFrame, &matrix );
							AnimateBoneMesh ( pObject, pFrame );
						}
					}
				}

				// restore animation flag
				pObject->bAnimPlaying = bStoreAnimFlag;
				pObject->fAnimFrame = fStoreFrame;
			}
		}
	}

	// bound vectir coutn var
	DWORD dwOverallObjBoundVectorCount = 0;

	// run through two passes (center calc, largest radius calc and radius calc)
	for ( int iMainPass = 0; iMainPass < 3; iMainPass++ )
	{
		// run through all of the frames within the object
		for ( int iCurrentFrame = 0; iCurrentFrame < pObject->iFrameCount; iCurrentFrame++ )
		{
			// find frame within object
			sFrame* pFrame = pObject->ppFrameList [ iCurrentFrame ];
			if ( pFrame )
			{
				sMesh* pMesh = pFrame->pMesh;
				if ( pMesh )
				{
					// calculate bounds for mesh (and animate on third pass)
					if ( iMainPass==0 ) CalculateMeshBounds ( pMesh );

					// world matrix (includes any scaling or orienting from frame data)
					// for example DarkMATTER models that have a 0.2 scaling at frame 1
					GGMATRIX WorldMatrix = pFrame->matCombined;

					// U74 - 210409 - FPSCV115 - animated vertex data earlier which applied scale so can use an identity
					// matrix here so the pFrameCombined does not interfere with vertex data MIN MAX boundbox
					// combined matrix required by many unanimated models so ODE physics can align box with
					// geometry so if animation is in effect, reset matrix, otherwise leave be, and ensure
					// BoneCount check so only vertexdata animated do not use frame combined but other animating
					// models such as chests which rotate/scale/translate can use combined for scale adjust
					if ( pObject->pAnimationSet )
					{
						if ( pObject->pAnimationSet->ulLength>0 )
							if ( pMesh->dwBoneCount>0 )
								GGMatrixIdentity ( &WorldMatrix );
					}
					else
					{
						// 241113 - discovered can have skinweight matrix on objects with no anim data but pBones data!
						// and will throw out proper position of object if not accounted for
						// 070917 - avoid corrupt data by also checking bone count
						if ( pMesh->pBones )
							if ( pMesh->dwBoneCount>0 )
								GGMatrixMultiply ( &WorldMatrix, &pMesh->pBones[0].matTranslation, &WorldMatrix );
					}

					// accumilate mesh data for object bounds calc
					if ( iMainPass==1 )
					{
						// object is animatable or static
						GGMATRIX matrix;
						if ( pObject->pAnimationSet )
						{
							// store anim flag
							bool bStoreAnimFlag = pObject->bAnimPlaying;
							float fStoreFrame = pObject->fAnimFrame;

							// calculate from all keyframes of animated mesh
							pObject->bAnimPlaying=true;
							int iLength = pObject->pAnimationSet->ulLength;
							sAnimation* pAnim = pObject->pAnimationSet->pAnimation;

							// keyframe ised to balance bounds of animation with speed code takes
							int iKeyStep = iLength/1000;

							// leefix - 170605 - this was added from a years old suggestion (animating all frames to get true bounds)
							// but it is slow (ie 13 seconds to load a character model in FPSC-V1), so do the quick version
							// BE AWARE that this will mean the keyframe-based bounbox data is not useful!
							if ( g_bFastBoundsCalculation==true )
								iKeyStep = iLength - 1;
							
							// go through all animations to get total bounds
							if ( iKeyStep<1 ) iKeyStep=1;
							for ( int iKeyframe=0; iKeyframe<iLength; iKeyframe+=iKeyStep )
							{
								// need combined for all non-bone-animating models
								matrix = pFrame->matCombined;
								if ( pObject->pAnimationSet->ulLength>0 )
									if ( pMesh->dwBoneCount>0 )
										GGMatrixIdentity ( &matrix );

								// set frame, animate and use mesh to get largest bounds
								pObject->fAnimFrame=(float)iKeyframe;
								UpdateAllFrameData ( pObject, pObject->fAnimFrame );
								UpdateFrame ( pObject->pFrame, &matrix );
								AnimateBoneMesh ( pObject, pFrame );

								// Calculate object bounds that animate
								if ( pObject->pAnimationSet )
								{
									// Calculate object bounds and store in bounds array
									CalculateObjectBounds ( iMainPass, pMesh, &matrix,
															&pObject->pAnimationSet->pvecBoundMin [ iKeyframe ], &pObject->pAnimationSet->pvecBoundMax [ iKeyframe ],
															&pObject->pAnimationSet->pvecBoundCenter [ iKeyframe ], &pObject->pAnimationSet->pfBoundRadius [ iKeyframe ] );

									// U74 - 210409 - FPSCV115 - store boundbox locally in case animationset bounds data not used
									if ( iKeyframe==0 )
									{
										pObject->collision.vecMin = pObject->pAnimationSet->pvecBoundMin [ iKeyframe ];
										pObject->collision.vecMax = pObject->pAnimationSet->pvecBoundMax [ iKeyframe ];
										pObject->collision.vecCentre = pObject->pAnimationSet->pvecBoundCenter [ iKeyframe ];
										pObject->collision.fRadius = pObject->pAnimationSet->pfBoundRadius [ iKeyframe ];
									}
								}
							}

							// restore mesh from animation parse
							pObject->bAnimPlaying=false;
							pObject->fAnimFrame=0.0f;
							GGMatrixIdentity ( &matrix );
							UpdateAllFrameData ( pObject, pObject->fAnimFrame );
							UpdateFrame ( pObject->pFrame, &matrix );
							AnimateBoneMesh ( pObject, pFrame );

							// restore animation flag
							pObject->bAnimPlaying = bStoreAnimFlag;
							pObject->fAnimFrame = fStoreFrame;
						}
						else
						{
							// update static model if has bones
							GGMatrixIdentity ( &matrix );
							UpdateAllFrameData ( pObject, pObject->fAnimFrame );
							UpdateFrame ( pObject->pFrame, &matrix );
							AnimateBoneMesh ( pObject, pFrame );

							// leefix - 280403 - calculate bound from non-animated mesh (ie cloned objects, etc)
							CalculateObjectBounds ( iMainPass, pMesh, &WorldMatrix,
													&pObject->collision.vecMin,	&pObject->collision.vecMax,
													&pObject->collision.vecCentre, &pObject->collision.fRadius	);
						}
					}
					else
					{
						// calculate from static mesh
						CalculateObjectBounds ( iMainPass, pMesh, &WorldMatrix,
												&pObject->collision.vecMin,	&pObject->collision.vecMax,
												&pObject->collision.vecCentre, &pObject->collision.fRadius	);
					}

					// increment bound vector count
					dwOverallObjBoundVectorCount+=pMesh->dwVertexCount;
				}
			}
		}
		if ( iMainPass==0 )
		{
			// finalise object center
			pObject->collision.vecCentre = pObject->collision.vecMin + ((pObject->collision.vecMax - pObject->collision.vecMin)/2.0f);
			vecRealObjectCenter = pObject->collision.vecCentre;
		}
		if ( iMainPass==1 )
		{
			// work out largest object radius from object bound box
			pObject->collision.fLargestRadius = ( float ) sqrt ( pObject->collision.fRadius );
			pObject->collision.vecCentre = vecRealObjectCenter;
			pObject->collision.fRadius = 0.0f;

			// lee - 140307 - I think this was added without a date, but it stops scaledradius from being set as it is zero by default
			// and this condition allows the computebounds to be called while keeping scaledradius at zero if it was zero (so the no-culling feature remains viable)
			// so to combat this, we set the fScaledLargestRadius field to one before we call the object create common functions
			// u74b7 - Use the scaled radius
			if ( pObject->collision.fScaledLargestRadius > 0.0f ) pObject->collision.fScaledLargestRadius = pObject->collision.fLargestRadius * fBiggestScale;

			// lee - 060406 - u6rc6 - shadow casters have a larger visual cull radius
			if ( pObject->bCastsAShadow==true )
			{
				// increase largest range to encompass possible shadow
				if ( pObject->collision.fScaledLargestRadius > 0.0f )
				{
					pObject->collision.fScaledLargestRadius = pObject->collision.fLargestRadius;
					pObject->collision.fScaledLargestRadius += 3000.0f;
				}
			}
		}
		if ( iMainPass==2 )
		{
			// work out final object radius from object bound box
			pObject->collision.fRadius = ( float ) sqrt ( pObject->collision.fRadius );
			// u74b7 - Use the scaled radius
			pObject->collision.fScaledRadius = pObject->collision.fRadius * fBiggestScale;
		}
	}

	// leefix - 140306 - u60b3 - of course by reducing the keyframe scan, the array is not filled
	if ( pObject->pAnimationSet )
	{
		// lee - 280306 - u6rc3 - can have animset tag but no frames data
		if ( pObject->pAnimationSet->ulLength>0 )
		{
			// fill the array elements that have not been filled
			for ( int iKeyframe=0; iKeyframe<(int)pObject->pAnimationSet->ulLength; iKeyframe++ )
			{
				if ( pObject->pAnimationSet->pfBoundRadius [ iKeyframe ]==0.0f )
				{
					// fill from master collision shape
					pObject->pAnimationSet->pvecBoundMin [ iKeyframe ] = pObject->collision.vecMin;
					pObject->pAnimationSet->pvecBoundMax [ iKeyframe ] = pObject->collision.vecMax;
					pObject->pAnimationSet->pfBoundRadius [ iKeyframe ] = pObject->collision.fRadius;
					pObject->pAnimationSet->pvecBoundCenter [ iKeyframe ] = pObject->collision.vecCentre;
				}
			}
		}
	}

	// leadd - 080305 - apply boundbox from current frame in animation
	if ( pObject->pAnimationSet )
	{
		// lee - 280306 - u6rc3 - can have animset tag but no frames data
		if ( pObject->pAnimationSet->ulLength>0 )
		{
			if ( pObject->bUpdateOnlyCurrentFrameBounds==false )
			{
				// work out center and radius
				for ( int iKeyframe=0; iKeyframe<(int)pObject->pAnimationSet->ulLength; iKeyframe+=1 )
				{
					pObject->pAnimationSet->pvecBoundCenter [ iKeyframe ] = pObject->pAnimationSet->pvecBoundMin [ iKeyframe ] + ((pObject->pAnimationSet->pvecBoundMax [ iKeyframe ] - pObject->pAnimationSet->pvecBoundMin [ iKeyframe ])/2.0f);
					GGVECTOR3 vecXYZ1 = pObject->pAnimationSet->pvecBoundMin [ iKeyframe ] - pObject->pAnimationSet->pvecBoundCenter [ iKeyframe ];
					GGVECTOR3 vecXYZ2 = pObject->pAnimationSet->pvecBoundMax [ iKeyframe ] - pObject->pAnimationSet->pvecBoundCenter [ iKeyframe ];
					float fDistSq1 = GGVec3Length ( &vecXYZ1 );
					float fDistSq2 = GGVec3Length ( &vecXYZ2 );
					if ( fDistSq2>fDistSq1 ) fDistSq1=fDistSq2;
					pObject->pAnimationSet->pfBoundRadius [ iKeyframe ] = fDistSq1;
				}

				// object bounds updated, and copied to first mesh collision bounds (for visual tweak)
				pObject->collision.vecMin = pObject->pAnimationSet->pvecBoundMin [ (int)pObject->fAnimFrame ];
				pObject->collision.vecMax = pObject->pAnimationSet->pvecBoundMax [ (int)pObject->fAnimFrame ];
				pObject->collision.vecCentre = pObject->pAnimationSet->pvecBoundCenter [ (int)pObject->fAnimFrame ];
				pObject->collision.fRadius = pObject->pAnimationSet->pfBoundRadius [ (int)pObject->fAnimFrame ];
				pObject->ppMeshList [ 0 ]->Collision.vecMin = pObject->collision.vecMin;
				pObject->ppMeshList [ 0 ]->Collision.vecMax = pObject->collision.vecMax;
				pObject->ppMeshList [ 0 ]->Collision.vecCentre = pObject->collision.vecCentre;
				pObject->ppMeshList [ 0 ]->Collision.fRadius = pObject->collision.fRadius;
			}
		}
	}

	// okay
	return true;
}

DARKSDK_DLL bool SetupMeshData ( sMesh* pMesh, DWORD dwVertexCount, DWORD dwIndexCount, bool bTempAllow32BitIndexBuffer )
{
	// if index size exceeds 16bit, cannot allow index buffer (except when temporarily allowing 32bit indices to copy in other mesh, then convert to vert only, done elsewhere)
	if ( bTempAllow32BitIndexBuffer == false )
		if ( dwIndexCount > 0 )
			if ( dwVertexCount > 0xFFFF ) //if ( dwIndexCount > 0x0000FFFF )
				return false;

	// ensure the mesh is valid
	SAFE_MEMORY ( pMesh );

	// ensure we free old data
	// 281114 - changed to SAFE_DELETE_ARRAY
	SAFE_DELETE_ARRAY(pMesh->pVertexData);
	SAFE_DELETE_ARRAY(pMesh->pIndices);

	// setup mesh properties
	pMesh->dwVertexCount	= dwVertexCount;									// vertex count assigned
	pMesh->pVertexData		= new BYTE [ pMesh->dwFVFSize * dwVertexCount ];	// allocate vertex memory

	// create new index mesh data
	if ( dwIndexCount>0 )
	{
		if ( bTempAllow32BitIndexBuffer == false )
		{
			pMesh->dwIndexCount		= dwIndexCount;
			pMesh->pIndices			= new WORD [ pMesh->dwIndexCount ];
		}
		else
		{
			pMesh->dwIndexCount		= dwIndexCount;
			pMesh->pIndices			= (WORD*)new DWORD [ pMesh->dwIndexCount ];
		}
	}
	else
	{
		pMesh->dwIndexCount		= 0;
		pMesh->pIndices			= NULL;
	}

	// check the memory was allocated correctly
	SAFE_MEMORY ( pMesh->pVertexData );

	// okay
	return true;
}

DARKSDK_DLL bool SetupMeshDeclarationData ( sMesh* pMesh, CONST GGVERTEXELEMENT* pDeclaration, DWORD dwVertexSize, DWORD dwVertexCount, DWORD dwIndexCount )
{
	LPGGVERTEXLAYOUT pNewVertexDec = NULL;	
	#ifdef DX11
	#else
	// create a new vertex declaration object
	if ( FAILED ( m_pD3D->CreateVertexDeclaration ( pDeclaration, &pNewVertexDec ) ) )
		return false;
	#endif

	// setup mesh properties
	pMesh->dwFVF			= 0;
	pMesh->dwFVFSize		= dwVertexSize;

	// store declaration for later reversal
	memcpy ( pMesh->pVertexDeclaration, pDeclaration, sizeof(pMesh->pVertexDeclaration) );

	// free any previous association with vertex dec handle
	/// 151015 - pVertexDec is created then copied as reference, then original released, leaving these (leak known from this!!)
	/// SAFE_RELEASE ( pMesh->pVertexDec );
	pMesh->pVertexDec = pNewVertexDec;

	// now setup the data
	if ( !SetupMeshData ( pMesh, dwVertexCount, dwIndexCount, false ) )
		return false;

	// complete
	return true;
}

DARKSDK_DLL bool SetupMeshFVFData ( sMesh* pMesh, DWORD dwFVF, DWORD dwVertexCount, DWORD dwIndexCount, bool bTempAllow32BitIndexBuffer )
{
	// set up mesh properties for the given FVF
	pMesh->dwFVF = dwFVF;
	sOffsetMap	offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// setup mesh properties
	pMesh->dwFVFSize = offsetMap.dwByteSize;

	// now setup the data
	if ( !SetupMeshData ( pMesh, dwVertexCount, dwIndexCount, bTempAllow32BitIndexBuffer ) )
		return false;

	// complete
	return true;
}

DARKSDK_DLL bool SetupFrustum ( float fZDistance )
{
	// setup the planes for the viewing frustum
	// variable declarations
	GGMATRIX Matrix;
	float ZMin,	Q;
	GGMATRIX matProj;
	GGMATRIX matView;

	// check d3d is ok
	SAFE_MEMORY ( m_pD3D );
	
	// get the projection matrix
	GGGetTransform ( GGTS_PROJECTION, &matProj );
	
	// get the view matrix
	GGGetTransform ( GGTS_VIEW, &matView );

	if ( fZDistance != 0.0f )
	{
		// calculate new projection matrix based on distance provided
		ZMin        = -matProj._43 / matProj._33;
		Q           = fZDistance / ( fZDistance - ZMin );
		matProj._33 = Q;
		matProj._43 = -Q * ZMin;
	}

	// multiply with the projection matrix
	GGMatrixMultiply ( &Matrix, &matView, &matProj );

	// and now calculate the planes
	// near plane
	g_Planes [ 0 ][ 0 ].a = Matrix._14 + Matrix._13;
	g_Planes [ 0 ][ 0 ].b = Matrix._24 + Matrix._23;
	g_Planes [ 0 ][ 0 ].c = Matrix._34 + Matrix._33;
	g_Planes [ 0 ][ 0 ].d = Matrix._44 + Matrix._43;
	GGPlaneNormalize ( &g_Planes [ 0 ][ 0 ], &g_Planes [ 0 ][ 0 ] );

	// far plane
	g_Planes [ 0 ][ 1 ].a = Matrix._14 - Matrix._13;
	g_Planes [ 0 ][ 1 ].b = Matrix._24 - Matrix._23;
	g_Planes [ 0 ][ 1 ].c = Matrix._34 - Matrix._33;
	g_Planes [ 0 ][ 1 ].d = Matrix._44 - Matrix._43;
	GGPlaneNormalize ( &g_Planes [ 0 ][ 1 ], &g_Planes [ 0 ][ 1 ] );

	// left plane
	g_Planes [ 0 ][ 2 ].a = Matrix._14 + Matrix._11;
	g_Planes [ 0 ][ 2 ].b = Matrix._24 + Matrix._21;
	g_Planes [ 0 ][ 2 ].c = Matrix._34 + Matrix._31;
	g_Planes [ 0 ][ 2 ].d = Matrix._44 + Matrix._41;
	GGPlaneNormalize ( &g_Planes [ 0 ][ 2 ], &g_Planes [ 0 ][ 2 ] );

	// right plane
	g_Planes [ 0 ][ 3 ].a = Matrix._14 - Matrix._11;
	g_Planes [ 0 ][ 3 ].b = Matrix._24 - Matrix._21;
	g_Planes [ 0 ][ 3 ].c = Matrix._34 - Matrix._31;
	g_Planes [ 0 ][ 3 ].d = Matrix._44 - Matrix._41;
	GGPlaneNormalize ( &g_Planes [ 0 ][ 3 ], &g_Planes [ 0 ][ 3 ] );

	// top plane
	g_Planes [ 0 ][ 4 ].a = Matrix._14 - Matrix._12;
	g_Planes [ 0 ][ 4 ].b = Matrix._24 - Matrix._22;
	g_Planes [ 0 ][ 4 ].c = Matrix._34 - Matrix._32;
	g_Planes [ 0 ][ 4 ].d = Matrix._44 - Matrix._42;
	GGPlaneNormalize ( &g_Planes [ 0 ][ 4 ], &g_Planes [ 0 ][ 4 ] );

	// bottom plane
	g_Planes [ 0 ][ 5 ].a = Matrix._14 + Matrix._12;
	g_Planes [ 0 ][ 5 ].b = Matrix._24 + Matrix._22;
	g_Planes [ 0 ][ 5 ].c = Matrix._34 + Matrix._32;
	g_Planes [ 0 ][ 5 ].d = Matrix._44 + Matrix._42;
	GGPlaneNormalize ( &g_Planes [ 0 ][ 5 ], &g_Planes [ 0 ][ 5 ] );

	// complete
	return true;
}

DARKSDK_DLL bool SetupPortalFrustum ( DWORD dwFrustumCount, GGVECTOR3* pvecStart, GGVECTOR3* pvecA, GGVECTOR3* pvecB, GGVECTOR3* pvecC, GGVECTOR3* pvecD, bool bFrustrumZeroIsValid )
{
	// near plane
	GGPlaneFromPoints ( &g_Planes [ dwFrustumCount ][ 0 ], pvecA, pvecB, pvecC );
	g_PlaneVector [ dwFrustumCount ][ 0 ] = *pvecA;

	// far plane
	g_Planes [ dwFrustumCount ][ 1 ] = g_Planes [ 0 ][ 1 ];

	// left plane
	GGPlaneFromPoints ( &g_Planes [ dwFrustumCount ][ 2 ], pvecStart, pvecB, pvecC );
	g_PlaneVector [ dwFrustumCount ][ 2 ] = *pvecB;

	// right plane
	GGPlaneFromPoints ( &g_Planes [ dwFrustumCount ][ 3 ], pvecStart, pvecD, pvecA );
	g_PlaneVector [ dwFrustumCount ][ 3 ] = *pvecD;

	// top plane
	GGPlaneFromPoints ( &g_Planes [ dwFrustumCount ][ 4 ], pvecStart, pvecA, pvecB );
	g_PlaneVector [ dwFrustumCount ][ 4 ] = *pvecA;

	// bottom plane
	GGPlaneFromPoints ( &g_Planes [ dwFrustumCount ][ 5 ], pvecStart, pvecC, pvecD );
	g_PlaneVector [ dwFrustumCount ][ 5 ] = *pvecC;

	// make sure child is clipped by parent frustrum
	if ( (dwFrustumCount > 0 && bFrustrumZeroIsValid) || dwFrustumCount > 1 )
	{
		for ( int iP=0; iP<NUM_CULLPLANES; iP++ )
		{
			// not back plane
			if ( iP!=1 )
			{
				// if vector of child is outside parent plane, must clip child
				GGVECTOR3 vecChildVector = g_PlaneVector [ dwFrustumCount ][ iP ];
				float fChildOutside = GGPlaneDotCoord ( &g_Planes [ dwFrustumCount-1 ][ iP ], &vecChildVector );
				if ( (int)fChildOutside < 0.0f )
				{
					// child is outside parent frustrum at this plane, adjust child plane
					g_Planes [ dwFrustumCount ][ iP ] = g_Planes [ dwFrustumCount-1 ][ iP ];
				}
			}
		}
	}

	// complete
	return true;
}

DARKSDK_DLL bool SetupCastFrustum ( DWORD dwFrustumCount, GGVECTOR3* pvecStart, GGVECTOR3* pvecFinish )
{
	// useful to create a frustrum that immitates a ray cast for finding nodes from a line

	// complete
	return true;
}

DARKSDK_DLL bool CheckPoint ( float fX, float fY, float fZ )
{
	// make sure point is in frustum
	for ( int iPlaneIndex = 0; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ ) 
	{
		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX, fY, fZ ) ) < 0.0f )
			return false;
	}
	return true;
}

DARKSDK_DLL bool CheckPoint ( GGPLANE* pPlanes, GGVECTOR3* pvecPoint )
{
	// is point in frustum?
//	for ( int iPlaneIndex = 0; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ ) 
	for ( int iPlaneIndex = 2; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ ) 
		if ( GGPlaneDotCoord ( &pPlanes [ iPlaneIndex ], pvecPoint ) < 0.0f )
			return false;

	// yes
	return true;
}

DARKSDK_DLL bool CheckCube ( float fX, float fY, float fZ, float fSize )
{
	// this does not always work when using very long ranges or small plane data
	for ( int iPlaneIndex = 0; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ )
	{
		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX - fSize, fY - fSize, fZ - fSize ) ) >= 0.0f )
			continue;

		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX + fSize, fY - fSize, fZ - fSize ) ) >= 0.0f )
			continue;

		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX - fSize, fY + fSize, fZ - fSize ) ) >= 0.0f )
			continue;

		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX + fSize, fY + fSize, fZ - fSize ) ) >= 0.0f )
			continue;

		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX - fSize, fY - fSize, fZ + fSize ) ) >= 0.0f )
			continue;

		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX + fSize, fY - fSize, fZ + fSize ) ) >= 0.0f )
			continue;

		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX - fSize, fY + fSize, fZ + fSize ) ) >= 0.0f )
			continue;

		if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX + fSize, fY + fSize, fZ + fSize ) ) >= 0.0f )
			continue;

		return false;
	}
	return true;
}

DARKSDK_DLL bool CheckRectangleEx ( DWORD iFrustumIndex, float fX, float fY, float fZ, float fXSize, float fYSize, float fZSize )
{
	bool bBoxFoundInsideOneOfTheFrustrums=false;
	bool bBoxOutsideFrustrum=false;
	for ( int iPlaneIndex = 0; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ )
	{
		// if ALL eight points are outside the plane, it is not inside
		int iCountPointsOutsidePlane=0;

		// FPGC - 180909 - disregard near plane clip on frustrum zero checks (player camera can get nose right upto a portal)
		// if ( iFrustumIndex!=0 ) if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX - fXSize, fY - fYSize, fZ - fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		// 18113 - the above line would NEVER reject a boundbox for camera zero frustum (not intended behaviour!)
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX - fXSize, fY - fYSize, fZ - fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX + fXSize, fY - fYSize, fZ - fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX - fXSize, fY + fYSize, fZ - fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX + fXSize, fY + fYSize, fZ - fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX - fXSize, fY - fYSize, fZ + fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX + fXSize, fY - fYSize, fZ + fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX - fXSize, fY + fYSize, fZ + fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX + fXSize, fY + fYSize, fZ + fZSize ) ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( iCountPointsOutsidePlane==8 ) 
		{
			bBoxOutsideFrustrum=true;
			break;
		}
	}
	if ( bBoxOutsideFrustrum==false )
	{
		// box was inside one of the frustrums
		bBoxFoundInsideOneOfTheFrustrums=true;
	}

	// compare if box was outside ALL frustrums
	if ( bBoxFoundInsideOneOfTheFrustrums )
	{
		// at least one corner striding one of the frustrums
		return true;
	}
	else
	{
		// all corners outside all frustrums
		return false;
	}
}

DARKSDK_DLL bool CheckRectangle ( float fX, float fY, float fZ, float fXSize, float fYSize, float fZSize )
{
	return CheckRectangleEx ( 0, fX, fY, fZ, fXSize, fYSize, fZSize );
}

DARKSDK_DLL bool CheckSphere ( float fX, float fY, float fZ, float fRadius )
{
	// make sure radius is in frustum
	fRadius *= 1.25f; // leefix - 190307 - added an epsilon so object does not disappear too early
	for ( int iPlaneIndex = 0; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ )
	{
		//if ( GGPlaneDotCoord ( &g_Planes [ 0 ][ iPlaneIndex ], &GGVECTOR3 ( fX, fY, fZ ) ) < -fRadius )
		//Dave Performance
		if ( g_Planes [ 0 ] [ iPlaneIndex ].a * fX + g_Planes [ 0 ] [ iPlaneIndex ].b * fY + g_Planes [ 0 ] [ iPlaneIndex ].c * fZ + g_Planes [ 0 ] [ iPlaneIndex ].d < -fRadius )
			return false;
	}

	return true;
}

DARKSDK_DLL bool CheckSphere ( DWORD dwFrustumMax, float fX, float fY, float fZ, float fRadius )
{
	// make sure radius is in frustum
	//for ( int iFrustumIndex = 0; iFrustumIndex <= (int)dwFrustumMax; iFrustumIndex++ )
	{	
		for ( int iPlaneIndex = 0; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ )
		{
			//if ( GGPlaneDotCoord ( &g_Planes [ iFrustumIndex ][ iPlaneIndex ], &GGVECTOR3 ( fX, fY, fZ ) ) < -fRadius )
			//Dave Performance
			if ( g_Planes [ 0 ] [ iPlaneIndex ].a * fX + g_Planes [ 0 ] [ iPlaneIndex ].b * fY + g_Planes [ 0 ] [ iPlaneIndex ].c * fZ + g_Planes [ 0 ] [ iPlaneIndex ].d < -fRadius )
				return false;
		}
	}
	return true;
}

DARKSDK_DLL bool CheckPolygon ( GGPLANE* pPlanes, GGVECTOR3* pvec0, GGVECTOR3* pvec1, GGVECTOR3* pvec2 )
{
	// check all six planes of frustrum
	bool bBoxOutsideFrustrum=false;
	for ( int iPlaneIndex = 0; iPlaneIndex < NUM_CULLPLANES; iPlaneIndex++ )
	{
		// if polygon completely outside one of the planes, absolutely not in frustrum!
		int iCountPointsOutsidePlane=0;
		if ( GGPlaneDotCoord ( &pPlanes [ iPlaneIndex ], pvec0 ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &pPlanes [ iPlaneIndex ], pvec1 ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( GGPlaneDotCoord ( &pPlanes [ iPlaneIndex ], pvec2 ) < -0.01f ) iCountPointsOutsidePlane++;
		if ( iCountPointsOutsidePlane==3 ) 
		{
			bBoxOutsideFrustrum=true;
			break;
		}
	}
	if ( bBoxOutsideFrustrum )
		return false;
	else
		return true;
}

DARKSDK_DLL bool QuickSortArray ( int* array, int low, int high )
{
	// this function takes an array of any size
	// and will resort it into a correct order
	// e.g. sending the function a list of 4,2,3,1
	// will result in a list of 1,2,3,4
	// the low and high values are used to specify
	// the start and end point for the search

	if ( !array )
		return false;

	// initialize pointers
	int top    = low;
	int	bottom = high - 1;
	int part_index;
	int part_value;

	// do nothing if low >= high
	if ( low < high )
	{
		// check if elements are sequential
		if ( high == ( low + 1 ) )
		{
			if ( array [ low ] > array [ high ] )
				SwapInts ( array, high, low );
		}
		else
		{
			// choose a partition element and swap 
			// it with the last value in the array
			part_index = ( int ) ( ( low + high ) / 2 );
			part_value = array [ part_index ];

			SwapInts ( array, high, part_index );
		
			do
			{
				// increment the top pointer
				while ( ( array [ top ] <= part_value ) && ( top <= bottom ) )
					top++;
				
				// decrement the bottom pointer
				while ( ( array [ bottom ] > part_value ) && ( top <= bottom ) )
					bottom--;
				
				// swap elements if pointers have not met
				if ( top < bottom )
					SwapInts ( array, top, bottom );

			} while ( top < bottom );

			// put the partition element back where it belongs
			SwapInts ( array, top, high );

			// recursive calls
			QuickSortArray ( array, low,     top - 1 );
			QuickSortArray ( array, top + 1, high    );
		}
	}

	return true;
}

// swaps array n1 with array n2
DARKSDK_DLL bool SwapInts ( int* array, int n1, int n2 )
{
	if ( !array )
		return false;

	int temp;

	temp         = array [ n1 ];
	array [ n1 ] = array [ n2 ];
	array [ n2 ] = temp;

	return true;
}

DARKSDK_DLL bool GetFVFValueOffsetMap ( DWORD dwFVF, sOffsetMap* psOffsetMap )
{
	SAFE_MEMORY ( psOffsetMap );

	memset ( psOffsetMap, 0, sizeof ( sOffsetMap ) );

	int iOffset   = 0;
	int iPosition = 0;
	DWORD dwFVFSize = 0;

	if ( dwFVF & GGFVF_XYZ )
	{
		psOffsetMap->dwX         = iOffset + 0;
		psOffsetMap->dwY         = iOffset + 1;
		psOffsetMap->dwZ         = iOffset + 2;
		iOffset += 3;
	}

	if ( dwFVF & GGFVF_XYZRHW )
	{
		psOffsetMap->dwRWH = iOffset + 0;
		iOffset += 1;
	}

	if ( dwFVF & GGFVF_NORMAL )
	{
		psOffsetMap->dwNX        = iOffset + 0;
		psOffsetMap->dwNY        = iOffset + 1;
		psOffsetMap->dwNZ        = iOffset + 2;
		iOffset += 3;
	}

	if ( dwFVF & GGFVF_PSIZE )
	{
		psOffsetMap->dwPointSize = iOffset + 0;
		iOffset += 1;
	}

	if ( dwFVF & GGFVF_DIFFUSE )
	{
		psOffsetMap->dwDiffuse   = iOffset + 0;
		iOffset += 1;
	}

	if ( dwFVF & GGFVF_SPECULAR )
	{
		psOffsetMap->dwSpecular   = iOffset + 0;
		iOffset += 1;
	}

	DWORD dwTexCount = 0;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX1 ) dwTexCount=1;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX2 ) dwTexCount=2;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX3 ) dwTexCount=3;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX4 ) dwTexCount=4;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX5 ) dwTexCount=5;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX6 ) dwTexCount=6;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX7 ) dwTexCount=7;
	if ( (dwFVF & GGFVF_TEXCOUNT_MASK) == GGFVF_TEX8 ) dwTexCount=8;
	for ( DWORD dwTexCoordSet=0; dwTexCoordSet<dwTexCount; dwTexCoordSet++ )
	{
		DWORD dwTexCoord = dwFVF & GGFVF_TEXCOORDSIZE1(dwTexCoordSet);
		if ( dwTexCoord==(DWORD)GGFVF_TEXCOORDSIZE1(dwTexCoordSet) )
		{
			psOffsetMap->dwTU[dwTexCoordSet] = iOffset + 0;
			iOffset += 1;
		}
		if ( dwTexCoord==(DWORD)GGFVF_TEXCOORDSIZE2(dwTexCoordSet) )
		{
			psOffsetMap->dwTU[dwTexCoordSet] = iOffset + 0;
			psOffsetMap->dwTV[dwTexCoordSet] = iOffset + 1;
			iOffset += 2;
		}
		if ( dwTexCoord==(DWORD)GGFVF_TEXCOORDSIZE3(dwTexCoordSet) )
		{
			psOffsetMap->dwTU[dwTexCoordSet] = iOffset + 0;
			psOffsetMap->dwTV[dwTexCoordSet] = iOffset + 1;
			psOffsetMap->dwTZ[dwTexCoordSet] = iOffset + 2;
			iOffset += 3;
		}
		if ( dwTexCoord==(DWORD)GGFVF_TEXCOORDSIZE4(dwTexCoordSet) )
		{
			psOffsetMap->dwTU[dwTexCoordSet] = iOffset + 0;
			psOffsetMap->dwTV[dwTexCoordSet] = iOffset + 1;
			psOffsetMap->dwTZ[dwTexCoordSet] = iOffset + 2;
			psOffsetMap->dwTW[dwTexCoordSet] = iOffset + 3;
			iOffset += 4;
		}
	}
	
	// calculate byte offset
	psOffsetMap->dwByteSize = sizeof ( DWORD ) * iOffset;

	// store number of offsets
	psOffsetMap->dwSize = iOffset;

	// check if matches byte size of actual FVF
	#ifdef DX11
	#else
	dwFVFSize = D3DXGetFVFVertexSize ( dwFVF );
	if ( dwFVFSize != psOffsetMap->dwByteSize )
	{
		// Offsets not being calculated correctly!
		return false;
	}
	#endif

	// complete
	return true;
}

DARKSDK_DLL bool GetFVFOffsetMap ( sMesh* pMesh, sOffsetMap* psOffsetMap )
{
	// clear to begin with
	memset ( psOffsetMap, 0, sizeof(sOffsetMap) );

	// FVF or declaration
	if ( pMesh->dwFVF==0 )
	{
		// Define end declaration token
		GGVERTEXELEMENT End = GGDECLEND;
		#ifdef DX11
		End.Stream = 255;
		#endif

		//PE: We get a exception here , pMesh->pVertexDeclaration[iElem].Stream has no 255 (end) entry.

		// Find Offsets
		for( int iElem=0; pMesh->pVertexDeclaration[iElem].Stream != End.Stream; iElem++ )
		{   
			if (iElem >= MAX_FVF_DECL_SIZE - 1) break; //PE: Make sure we dont crash.

			int iIndex = pMesh->pVertexDeclaration[iElem].UsageIndex;
			int iElementOffset = pMesh->pVertexDeclaration[iElem].Offset / sizeof(DWORD);
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_POSITION )
			{
				psOffsetMap->dwX = iElementOffset + 0;
				psOffsetMap->dwY = iElementOffset + 1;
				psOffsetMap->dwZ = iElementOffset + 2;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_POSITIONT )
			{
				psOffsetMap->dwX = iElementOffset + 0;
				psOffsetMap->dwY = iElementOffset + 1;
				psOffsetMap->dwZ = iElementOffset + 2;
				psOffsetMap->dwRWH = iElementOffset + 3;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_PSIZE )
			{
				psOffsetMap->dwPointSize = iElementOffset + 0;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_NORMAL )
			{
				psOffsetMap->dwNX = iElementOffset + 0;
				psOffsetMap->dwNY = iElementOffset + 1;
				psOffsetMap->dwNZ = iElementOffset + 2;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_COLOR && iIndex==0 )
			{
				psOffsetMap->dwDiffuse = iElementOffset + 0;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_COLOR && iIndex==1 )
			{
				psOffsetMap->dwSpecular = iElementOffset + 0;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_TEXCOORD )
			{
				psOffsetMap->dwTU[iIndex] = iElementOffset + 0;
				if ( pMesh->pVertexDeclaration[iElem].Type==GGDECLTYPE_FLOAT2)
				{
					psOffsetMap->dwTV[iIndex] = iElementOffset + 1;
				}
				if ( pMesh->pVertexDeclaration[iElem].Type==GGDECLTYPE_FLOAT3)
				{
					psOffsetMap->dwTV[iIndex] = iElementOffset + 1;
					psOffsetMap->dwTZ[iIndex] = iElementOffset + 2;
				}
				if ( pMesh->pVertexDeclaration[iElem].Type==GGDECLTYPE_FLOAT4)
				{
					psOffsetMap->dwTV[iIndex] = iElementOffset + 1;
					psOffsetMap->dwTZ[iIndex] = iElementOffset + 2;
					psOffsetMap->dwTW[iIndex] = iElementOffset + 3;
				}
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_TANGENT )
			{
				psOffsetMap->dwTU[1] = iElementOffset + 0;
				psOffsetMap->dwTV[1] = iElementOffset + 1;
				psOffsetMap->dwTZ[1] = iElementOffset + 2;
				psOffsetMap->dwTW[1] = iElementOffset + 3;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_BINORMAL )
			{
				psOffsetMap->dwTU[2] = iElementOffset + 0;
				psOffsetMap->dwTV[2] = iElementOffset + 1;
				psOffsetMap->dwTZ[2] = iElementOffset + 2;
				psOffsetMap->dwTW[2] = iElementOffset + 3;
			}
		}

		// calculate byte offset
		psOffsetMap->dwByteSize = pMesh->dwFVFSize;

		// store number of offsets
		psOffsetMap->dwSize = pMesh->dwFVFSize/sizeof(DWORD);

		// complete
		return true;
	}
	else
	{
		return GetFVFValueOffsetMap ( pMesh->dwFVF, psOffsetMap );
	}
}

DARKSDK_DLL bool GetFVFOffsetMapFixedForBones ( sMesh* pMesh, sOffsetMap* psOffsetMap )
{
	// The above "GetFVFOffsetMap" function overwrites the Binormals and Tangents offsets
	// when bone indices and weights are also in the DBO vertex declaration (ouch!)
	// we deliberately place BONEINDICES in TUVZW[3] and BONEWEIGHTS in TUVZW[4]
	int iBoneDataOffset = 3;

	// clear to begin with
	memset ( psOffsetMap, 0, sizeof(sOffsetMap) );

	// FVF or declaration
	if ( pMesh->dwFVF==0 )
	{
		// Define end declaration token
		GGVERTEXELEMENT End = GGDECLEND;
		#ifdef DX11
		End.Stream = 255;
		#endif

		// Find Offsets
		for( int iElem=0; pMesh->pVertexDeclaration[iElem].Stream != End.Stream; iElem++ )
		{   
			if (iElem >= MAX_FVF_DECL_SIZE - 1) break;
			int iIndex = pMesh->pVertexDeclaration[iElem].UsageIndex;
			int iElementOffset = pMesh->pVertexDeclaration[iElem].Offset / sizeof(DWORD);
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_POSITION )
			{
				psOffsetMap->dwX = iElementOffset + 0;
				psOffsetMap->dwY = iElementOffset + 1;
				psOffsetMap->dwZ = iElementOffset + 2;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_POSITIONT )
			{
				psOffsetMap->dwX = iElementOffset + 0;
				psOffsetMap->dwY = iElementOffset + 1;
				psOffsetMap->dwZ = iElementOffset + 2;
				psOffsetMap->dwRWH = iElementOffset + 3;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_PSIZE )
			{
				psOffsetMap->dwPointSize = iElementOffset + 0;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_NORMAL )
			{
				psOffsetMap->dwNX = iElementOffset + 0;
				psOffsetMap->dwNY = iElementOffset + 1;
				psOffsetMap->dwNZ = iElementOffset + 2;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_COLOR && iIndex==0 )
			{
				psOffsetMap->dwDiffuse = iElementOffset + 0;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_COLOR && iIndex==1 )
			{
				psOffsetMap->dwSpecular = iElementOffset + 0;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_TEXCOORD )
			{
				if (pMesh->pVertexDeclaration[iElem].Type == GGDECLTYPE_FLOAT4)
				{
					// bone data comes in from this one, so expect INDICES then WEIGHTS
					if (iBoneDataOffset == 3 || iBoneDataOffset == 4)
					{
						psOffsetMap->dwTU[iBoneDataOffset] = iElementOffset + 0;
						psOffsetMap->dwTV[iBoneDataOffset] = iElementOffset + 1;
						psOffsetMap->dwTZ[iBoneDataOffset] = iElementOffset + 2;
						psOffsetMap->dwTW[iBoneDataOffset] = iElementOffset + 3;
						iBoneDataOffset++;
					}
				}
				else
				{
					psOffsetMap->dwTU[iIndex] = iElementOffset + 0;
					if (pMesh->pVertexDeclaration[iElem].Type == GGDECLTYPE_FLOAT2)
					{
						psOffsetMap->dwTV[iIndex] = iElementOffset + 1;
					}
					if (pMesh->pVertexDeclaration[iElem].Type == GGDECLTYPE_FLOAT3)
					{
						psOffsetMap->dwTV[iIndex] = iElementOffset + 1;
						psOffsetMap->dwTZ[iIndex] = iElementOffset + 2;
					}
				}
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_TANGENT )
			{
				psOffsetMap->dwTU[1] = iElementOffset + 0;
				psOffsetMap->dwTV[1] = iElementOffset + 1;
				psOffsetMap->dwTZ[1] = iElementOffset + 2;
				psOffsetMap->dwTW[1] = iElementOffset + 3;
			}
			if( pMesh->pVertexDeclaration[iElem].Usage == GGDECLUSAGE_BINORMAL )
			{
				psOffsetMap->dwTU[2] = iElementOffset + 0;
				psOffsetMap->dwTV[2] = iElementOffset + 1;
				psOffsetMap->dwTZ[2] = iElementOffset + 2;
				psOffsetMap->dwTW[2] = iElementOffset + 3;
			}
		}

		// calculate byte offset
		psOffsetMap->dwByteSize = pMesh->dwFVFSize;

		// store number of offsets
		psOffsetMap->dwSize = pMesh->dwFVFSize/sizeof(DWORD);

		// complete
		return true;
	}
	else
	{
		return GetFVFValueOffsetMap ( pMesh->dwFVF, psOffsetMap );
	}
}

DARKSDK_DLL bool CreateVertexShaderFromFVF ( DWORD dwFVF, DWORD* pdwShader )
{
	// create the vertex shader handle

	// check all of the memory
	SAFE_MEMORY ( m_pD3D );

	// all okay
	return true;
}

DARKSDK_DLL void DBOCalculateLoaderTempFolder ( void )
{
	if ( g_pGlob )
	{
		// Add the DBPDATA folder to temp
		strcpy ( g_WindowsTempDirectory, g_pGlob->pEXEUnpackDirectory );
		if ( g_WindowsTempDirectory [ strlen(g_WindowsTempDirectory)-1 ]!='\\' )
 			strcat ( g_WindowsTempDirectory, "\\" );
	}
	else
	{
		// Current directory
		char CurrentDirectory[_MAX_PATH];
		_getcwd(CurrentDirectory, _MAX_PATH);

		// Find temporary directory (C:\WINDOWS\Temp)
		GetTempPath(_MAX_PATH, g_WindowsTempDirectory);
		if(_strcmpi(g_WindowsTempDirectory, CurrentDirectory)==NULL)
		{
			// Pre-XP Temp Folder
			GetWindowsDirectory(g_WindowsTempDirectory, _MAX_PATH);
			strcat(g_WindowsTempDirectory, "\\temp\\");
		}

		// Create DBPDATA folder in any event
		_chdir(g_WindowsTempDirectory);
		_mkdir("dbpdata");
		_chdir(CurrentDirectory);

		// Add the DBPDATA folder to temp
		strcat ( g_WindowsTempDirectory, "\\dbpdata\\" );
	}
}

DARKSDK_DLL bool DBOFileExist ( LPSTR pFilename )
{
	HANDLE hfile = GG_CreateFile ( pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hfile != INVALID_HANDLE_VALUE )
	{
		CloseHandle ( hfile );
		return true;
	}
	return false;
}

DARKSDK_DLL bool ConvertToDBOBlock ( LPSTR pFilename, LPSTR pExtension, void** ppDBOBlock, DWORD* pdwBlockSize )
{
	// result var
	bool bResult=false;

	// LEELEE, there is, but its an updated loadde for X files, maybe look at it AFTER ASSIMP!

	// clear return block
	*ppDBOBlock = NULL;
	*pdwBlockSize = 0;

	// Set LEGACY ON
	if ( g_bSwitchLegacyOn==true ) SetLegacyModeOn();

	// 061115 - Choose X or XYZ to load
	bool bSuccess = false;
	int iModelFormatMode = 0;
	void* pTempBlock=0;
	DWORD dwTempSize=0;
	if ( strnicmp ( pExtension, "xyz", 3 ) == 0 ) iModelFormatMode = 1;
	switch ( iModelFormatMode )
	{
		case 1 : bSuccess = false; break;
		default : bSuccess = ConvXConvert ( pFilename, &pTempBlock, &dwTempSize ); break;
	}

	// Call Convert Function
	if ( bSuccess==true )
	{
		// Create local memory for block
		*pdwBlockSize = dwTempSize;
		*ppDBOBlock = (void*) new char [ dwTempSize ];
		memcpy ( (LPSTR)*ppDBOBlock, (LPSTR)pTempBlock, dwTempSize );

		// Call Free Function			
		switch ( iModelFormatMode )
		{
			case 1 : break;
			default : ConvXFree( (LPSTR)pTempBlock ); break;
		}

		// success
		bResult=true;
	}
	else
	{
		// could not convert
		bResult=false;
	}

	// okay
	return bResult;
}

// DBO Import/Export Functions

DARKSDK_DLL bool LoadDBODataBlock ( LPSTR pFilename, DWORD* pdwBlockSize, void** ppDBOBlock )
{
	// Obtain extension
	char pExtension[256];
	strcpy(pExtension, "");
	for ( int n=strlen(pFilename); n>0; n-- )
	{
		if ( pFilename[n]=='.' )
		{
			strcpy(pExtension, pFilename+n+1);
			break;
		}
	}

	/* never do MD3 at this level again
	// if file MD3 format - permit filecheck skip
	if ( _stricmp ( pExtension, "MD3" )!=NULL )
	{
		// does file exist
		if ( !DBOFileExist ( pFilename ) )
		{
			RunTimeError ( RUNTIMEERROR_FILENOTEXIST, pFilename );
			return false;
		}
	}
	*/

	// if file native DBO format
	if ( _stricmp ( pExtension, "DBO" )==NULL )
	{
		// load DBO object directly
		if ( !DBOLoadBlockFile ( pFilename, ppDBOBlock, pdwBlockSize ) )
		{
			RunTimeError ( RUNTIMEERROR_B3DOBJECTLOADFAILED );
			return false;
		}
	}
	else
	{
		// call converter DLL (ConvX.dll)
		if ( !ConvertToDBOBlock ( pFilename, pExtension, ppDBOBlock, pdwBlockSize ) )
		{
			RunTimeError ( RUNTIMEERROR_B3DOBJECTLOADFAILED );
			return false;
		}
	}

	// success
	return true;
}

enumScalingMode g_eLoadScalingMode = eScalingMode_Off;

DARKSDK_DLL void SetLoadScale ( enumScalingMode eScaleMode )
{
	// when loading (importing) an object, have the option of affecting the
	// scale as it loads to avoid creating huge transforms via the secondary scale
	// feature. Ideal if models come in as 1 unit = 1 meter vs GameGuru 1 unit = 1 inch
	g_eLoadScalingMode = eScaleMode;
}

DARKSDK_DLL bool LoadDBO ( LPSTR pPassedInFilename, sObject** ppObject, char* pOrgFilename)
{
	// DBOBlock pointer
	DWORD dwBlockSize = 0;
	void* pDBOBlock = NULL;
	char pFilename[MAX_PATH];
	strcpy(pFilename, pPassedInFilename);

	// No object to start with
	*ppObject = NULL;

	// switch to use new AssImp Importer if not a DBO file
	// Obtain extension
	char pExtension[256];
	strcpy(pExtension, "");
	for ( int n=strlen(pFilename); n>0; n-- ) { if ( pFilename[n]=='.' ) { strcpy(pExtension, pFilename+n+1); break; } }

	if ( _stricmp ( pExtension, "DBO" ) == NULL )
	{
		// load data from DBO file or multi-threaded-pre-loaded DBO data
		// and we have access to the loaded textures via g_object_outputv
		DWORD* pDataBlockFromPreload = NULL;
		for (int n = 0; n < g_object_outputv.size(); n++)
		{
			int iSearchStrLen = strlen(pFilename);
			if (strnicmp(g_object_outputv[n].pFilename + strlen(g_object_outputv[n].pFilename) - iSearchStrLen, pFilename, iSearchStrLen) == NULL)
			{
				if (g_object_outputv[n].pData)
				{
					pDataBlockFromPreload = g_object_outputv[n].pData;
					dwBlockSize = g_object_outputv[n].dwDataSize;
					g_object_outputv[n].pData = NULL;
					g_object_outputv[n].dwDataSize = 0;
					pDBOBlock = pDataBlockFromPreload;
					break;
				}
			}
		}
		if (pDataBlockFromPreload == NULL)
		{
			if (LoadDBODataBlock(pFilename, &dwBlockSize, &pDBOBlock) == false)
				return false;
		}

		// construct the object
		if (!DBOConvertBlockToObject((void*)pDBOBlock, dwBlockSize, ppObject))
		{
			RunTimeError(RUNTIMEERROR_B3DOBJECTLOADFAILED);
			return false;
		}

		// free block when done
		SAFE_DELETE_ARRAY(pDBOBlock);
	}
	else
	{

		//PE: Check if we got a cached version.
		void CreateCacheXFile(char *pFilename, void* pBlock, DWORD dwBlockSize);
		bool bCheckCacheXFile(LPSTR pFilename, DWORD* pdwBlockSize, void** ppDBOBlock);
		bool bCached = false;
		
		if (pOrgFilename != NULL && strlen(pOrgFilename) > 1 && pOrgFilename[1] != ':' && bCheckCacheXFile(pOrgFilename, &dwBlockSize, &pDBOBlock))
		{
			//PE: cached version found and loaded.
			bCached = true;
		}
		else
		{
			// call converter DLL (ConvX.dll)
			if (!ConvertToDBOBlock(pFilename, pExtension, &pDBOBlock, &dwBlockSize))
			{
				RunTimeError(RUNTIMEERROR_B3DOBJECTLOADFAILED);
				return false;
			}
		}
		// construct the object
		if (!DBOConvertBlockToObject((void*)pDBOBlock, dwBlockSize, ppObject))
		{
			RunTimeError(RUNTIMEERROR_B3DOBJECTLOADFAILED);
			return false;
		}

		if (!bCached && pOrgFilename != NULL && strlen(pOrgFilename) > 1 && pOrgFilename[1] != ':')
		{
			//PE: In standalone create a cached version of this, for faster loading can also be included in final standalone for faster loading.
			CreateCacheXFile(pOrgFilename, pDBOBlock,dwBlockSize);
		}

		// free block when done
		SAFE_DELETE_ARRAY(pDBOBlock);

	}

	// okay
	return true;
}

DARKSDK_DLL bool SaveDBO ( LPSTR pFilename, sObject* pObject )
{
	// DBOBlock ptr
	DWORD dwBlockSize = 0;
	void* pDBOBlock = NULL;

	// does file exist
	if ( DBOFileExist ( pFilename ) )
	{
		RunTimeError ( RUNTIMEERROR_FILEEXISTS, pFilename );
		return false;
	}

	// leefix - 171203 - before save a DBO, ensure vertex data is original (and not modified by animation activity)
	ResetVertexDataInMesh ( pObject );

	// convert pObject to DBOBlock
	if ( !DBOConvertObjectToBlock ( pObject, (void**)&pDBOBlock, &dwBlockSize ) )
		return false;
		
	// save DBOBlock to file
	if ( !DBOSaveBlockFile ( pFilename, (void*)pDBOBlock, dwBlockSize ) )
		return false;

	// free block when done
	SAFE_DELETE(pDBOBlock);

	// okay
	return true;
}

DARKSDK_DLL bool CloneDBO ( sObject** ppDestObject, sObject* pSrcObject )
{
	// DBOBlock ptr
	DWORD dwBlockSize = 0;
	void* pDBOBlock = NULL; // was DWORD*

	// convert pObject to DBOBlock
	if ( !DBOConvertObjectToBlock ( pSrcObject, (void**)&pDBOBlock, &dwBlockSize ) )
		return false;
		
	// construct the new destination object
	if ( !DBOConvertBlockToObject ( pDBOBlock, dwBlockSize, ppDestObject ) )
		return false;

	// free block when done
	SAFE_DELETE_ARRAY(pDBOBlock);

	// okay
	return true;
}

// 310305 - mike - new function for allocating custom memory
DARKSDK_DLL void CreateCustomDataArrayForObject	( sObject* pObject, int iSize )
{
	pObject->pCustomData = new BYTE [ iSize ];
}

