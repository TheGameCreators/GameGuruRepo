#ifndef _CGFX_H_
#define _CGFX_H_

// Defines
#define VR920SUPPORT

// Includes
#include "directx-macros.h"
#pragma comment ( lib, "winmm.lib" )	// include windows multimedia extensions ( timers etc )
#include <stdio.h>
#include ".\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Error\cerror.h"
#include "globstruct.h"

#undef DARKSDK
#define DARKSDK
#define DEBUG_MODE 0

enum TIMER_COMMAND	{
						// timer values
						TIMER_RESET,				// reset the timer
						TIMER_START,				// start the timer
						TIMER_STOP,					// stop the timer
						TIMER_ADVANCE,				// advance the timer
						TIMER_GETABSOLUTETIME,		// absolute time of the timer
						TIMER_GETAPPTIME,			// app time
						TIMER_GETELAPSEDTIME		// elapsed time
					};

enum CGfx_Return
					{
						// return values only useful for DarkSDK
						OK,							// everything went ok
						CANNOTCREATEINTERFACE,		// failed to setup interface
						CANNOTGETDISPLAYINFO,		// failed to get display information
						CANNOTCREATEDEVICE,			// failed to create device
						INVALIDMODE,				// invalid more specified
					};

#ifdef DX11
enum CGfx_Flags {		// flags only useful for DarkSDK
						FULLSCREEN,										// fullscreen mode
						WINDOWED,										// windowed mode
						DEFAULT,										// default goes to windowed mode
						HARDWARE = 1,//D3DCREATE_HARDWARE_VERTEXPROCESSING,	// hardware vertex processing
						SOFTWARE = 2,//D3DCREATE_SOFTWARE_VERTEXPROCESSING,	// software vertex processing
						MIXED    = 3,//D3DCREATE_MIXED_VERTEXPROCESSING,	// mixed vertex processing
				};
struct tagInfo
{
	// graphics card information structure
	char*			szName;				// name of graphics card
	DWORD			D3DCaps;			// capabilities
	DWORD			D3DAdapter;			// adapter identifier
	DWORD			D3DMode;			// current display mode
	DWORD*			D3DDisplay;			// display mode info
	int				iDisplayCount;		// num of available display modes
};
#else
enum CGfx_Flags {		// flags only useful for DarkSDK
						FULLSCREEN,										// fullscreen mode
						WINDOWED,										// windowed mode
						DEFAULT,										// default goes to windowed mode
						HARDWARE = D3DCREATE_HARDWARE_VERTEXPROCESSING,	// hardware vertex processing
						SOFTWARE = D3DCREATE_SOFTWARE_VERTEXPROCESSING,	// software vertex processing
						MIXED    = D3DCREATE_MIXED_VERTEXPROCESSING,	// mixed vertex processing
				};
struct tagInfo
{
	// graphics card information structure
	char*					szName;				// name of graphics card
	GGCAPS				D3DCaps;			// capabilities
	D3DADAPTER_IDENTIFIER9	D3DAdapter;			// adapter identifier
	GGDISPLAYMODE			D3DMode;			// current display mode
	GGDISPLAYMODE*			D3DDisplay;			// display mode info
	int						iDisplayCount;		// num of available display modes
};
#endif

#ifndef VERTEX2DSTRUCT
#define VERTEX2DSTRUCT
struct VERTEX2D
{
	float	x;
	float	y;
	float	z;

	float	rhw;

	DWORD	color;

	float	tu;
	float	tv;
};
#endif

// GameGuru Shader Globals
#ifdef DX11
#define SHADERSARRAYMAX 2000
#define SHADERSTERRAINBASIC 1
#define SHADERSSPRITEBASIC 601
#define SHADERSSTATICBASIC 602
#define SHADERSSHADOWBASIC 603
struct sShaderGlobalsType
{
	char pName[512];
	ID3DBlob* pBlob;
	ID3DX11Effect* pEffect;
	ID3D11InputLayout* pInputLayout;
};
extern sShaderGlobalsType g_sShaders[SHADERSARRAYMAX];

void GetD3DExtraInfo ( int *piAdapterOrdinal, LPSTR pAdapterName, int* piFeatureLevel );

DARKSDK ID3DX11Effect*		SETUPLoadShader ( LPSTR szFile, LPSTR szBlobFile, int iShaderIndex );
#endif

#ifdef VR920SUPPORT
DARKSDK bool				SetupGetTracking					( float* pfYaw, float* pfPitch, float* pfRoll, float fSensitivity );
DARKSDK void				SetupResetTracking					( void );
#endif

DARKSDK bool				GetBackBufferAndDepthBuffer			( void );
DARKSDK bool				Setup                       		( void );
DARKSDK void				SetupSetViewport					( int iCameraID, D3D11_VIEWPORT* vp, LPGGSURFACE pSurface );
DARKSDK void				GetValidBackBufferFormat    		( void );
DARKSDK void				GetStencilDepth						( void );
DARKSDK int					CreateDX9                      		( HWND hWnd, GGPRESENT_PARAMETERS* d3dpp );
DARKSDK float __stdcall		Timer								( TIMER_COMMAND command );
DARKSDK void				Begin								( void );
DARKSDK void				End									( void );
DARKSDK void				Render								( void );
DARKSDK LPGG				GetDirect3D							( void );

DARKSDK bool				SETUPConstructor 					( void );
DARKSDK void				SETUPDestructor  					( void );
DARKSDK void 				SETUPSetErrorHandler 				( LPVOID pErrorHandlerPtr );
DARKSDK void 				SETUPPassCoreData					( LPVOID pGlobPtr, int iStage );

#ifdef DX11
DARKSDK ID3DX11Effect*		SETUPLoadShader						( LPSTR szFile, LPSTR szBlobFile, int iShaderIndex );
DARKSDK bool				SETUPFreeShader						( int iShaderIndex );
#endif

DARKSDK bool 				SetDisplayDebugMode					( void );
DARKSDK bool 				SetDisplayMode						( int iWidth, int iHeight );
DARKSDK bool 				SetDisplayMode						( int iWidth, int iHeight, int iDepth );
DARKSDK void 				SetDisplayModeVSYNC					( int iWidth, int iHeight, int iDepth, int iVSyncOn );
DARKSDK void 				SetDisplayModeANTIALIAS				( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode );
DARKSDK void 				SetDisplayModeMODBACKBUFFER			( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode, int iBackbufferWidth, int iBackbufferHeight );
DARKSDK void				SetDisplayModeVR					( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode, int iBackbufferWidth, int iBackbufferHeight, int iActivateVRMode );
DARKSDK bool 				SetDisplayMode						( int iWidth, int iHeight, int iDepth, int iMode );
DARKSDK bool 				SetDisplayMode						( int iWidth, int iHeight, int iDepth, int iMode, int iVertexProcessing );
DARKSDK bool 				SetDisplayMode						( int iWidth, int iHeight, int iDepth, int iMode, int iVertexProcessing, int iLockable );
DARKSDK void 				RestoreDisplayMode  				( void );
DARKSDK void 				RestoreLostDevice	 				( void );

DARKSDK void 				AddSwapChain    					( HWND hwnd );
DARKSDK void 				StartSwapChain	 					( int iID );
DARKSDK void 				EndSwapChain    					( int iID );
DARKSDK void 				UpdateSwapChain 					( int iID );

DARKSDK void 				SETUPClear 							( int   iR, int   iG, int   iB );
DARKSDK void 				GetGamma 							( int* piR, int* piG, int* piB );

DARKSDK bool 				CheckDisplayMode 					( int iWidth, int iHeight );
DARKSDK bool 				CheckDisplayMode 					( int iWidth, int iHeight, int iDepth, int iMode );
DARKSDK bool 				CheckDisplayMode 					( int iWidth, int iHeight, int iDepth, int iMode, int iVertexProcessing );

DARKSDK bool 				GetWindowedMode						( void );
DARKSDK int  				GetNumberOfDisplayModes				( void );
DARKSDK void 				GetDisplayMode						( int iID, char* szMode );
DARKSDK int  				GetNumberOfDisplayDevices			( void );
DARKSDK void 				SetDisplayDevice					( int iID );
DARKSDK void 				GetDeviceName						( int iID, char* szDevice );
DARKSDK void 				GetDeviceDriverName					( int iID, char* szDriver );

DARKSDK void 				SetDitherMode   					( int iMode );
DARKSDK void 				SetShadeMode    					( int iMode );
DARKSDK void 				SetLightMode    					( int iMode );
DARKSDK void 				SetCullMode     					( int iMode );
DARKSDK void 				SetSpecularMode 					( int iMode );
DARKSDK void 				SetRenderState  					( int iState, int iValue );

DARKSDK void 				OverrideHWND						( HWND hWnd );
DARKSDK void 				DisableWindowResize					( void );
DARKSDK void 				EnableWindowResize  				( void );
DARKSDK void 				UpdateWindowSize 					( int  iWidth,  int  iHeight  );
DARKSDK void 				GetWindowSize    					( int* piWidth, int* piHeight );

DARKSDK int   				CheckDisplayMode					( int iWidth, int iHeight, int iDepth );
DARKSDK int 				CheckDisplayModeANTIALIAS			( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode );

DARKSDK LPSTR 				CurrentGraphicsCard					( void );
DARKSDK int   				EmulationMode 						( void );
DARKSDK void  				PerformChecklistForDisplayModes		( void );
DARKSDK void  				PerformChecklistForGraphicsCards	( void );
DARKSDK int   				GetDisplayType   					( void );
DARKSDK int   				GetDisplayWidth  					( void );
DARKSDK int   				GetDisplayHeight 					( void );
DARKSDK int   				GetWindowWidth  					( void );
DARKSDK int   				GetWindowHeight 					( void );
DARKSDK int   				GetDesktopWidth  					( void );
DARKSDK int   				GetDesktopHeight 					( void );
DARKSDK int   				GetDisplayDepth  					( void );
DARKSDK int   				GetDisplayFPS    					( void );
DARKSDK int   				GetDisplayInvalid 					( void );
DARKSDK void  				SetGamma		  					( int   iR, int   iG, int   iB );
DARKSDK void  				SetDisplayModeEx 					( int iWidth, int iHeight, int iDepth );
DARKSDK void  				SetEmulationOn   					( void );
DARKSDK void  				SetEmulationOff   					( void );
DARKSDK void  				SetGraphicsCard 					( DWORD dwCardname );

DARKSDK void  				SetWindowModeOn						( void );
DARKSDK void  				SetWindowModeOff					( void );
DARKSDK void  				SetWindowSettings					( int iStyle, int iCaption, int iIcon );
DARKSDK void 				SetWindowPosition					( int posx, int posy );
DARKSDK void  				SetWindowSize						( int sizex, int sizey );
DARKSDK void  				HideWindow							( void );
DARKSDK void				ShowWindow							( void );
DARKSDK void  				MinimiseWindow						( void );
DARKSDK void  				MaximiseWindow						( void );
DARKSDK void  				RestoreWindow						( void );
DARKSDK void  				SetWindowTitle						( LPSTR pTitleString );
DARKSDK int   				WindowExist							( LPSTR pTitleString );
DARKSDK void  				WindowToBack						( void );
DARKSDK void  				WindowToFront						( void );
DARKSDK void  				WindowToFront						( LPSTR pTitleString );
DARKSDK void  				WindowToBack						( LPSTR pTitleString );
DARKSDK HWND				GetWindowHandle						( void );

DARKSDK void  				LockBackbuffer						( void );
DARKSDK void  				UnlockBackbuffer					( void );
DARKSDK DWORD 				GetBackbufferPtr					( void );
DARKSDK int   				GetBackbufferWidth					( void );
DARKSDK int   				GetBackbufferHeight					( void );
DARKSDK int   				GetBackbufferDepth					( void );
DARKSDK int   				GetBackbufferPitch					( void );

// lee - 130108 - added for X10 compat.
DARKSDK void				SetNvPerfHUD						( int iUsePerfHUD );
DARKSDK void				ForceAdapterOrdinal					( int iForceOrdinal );
DARKSDK void				ForceAdapterD3D11ONLY				( int iForceD3D11ONLY );
DARKSDK void				SetCaptureName						( DWORD pFilename );
DARKSDK void				SetCaptureCodec						( DWORD pFilename );
DARKSDK void				SetCaptureSettings					( int iCompressed, int iFPS, int iWidth, int iHeight, int iThreaded, int iWait );
DARKSDK void				SetCaptureMode						( int iRecordVideo );
DARKSDK void				SaveScreenshot						( DWORD pFilename );
DARKSDK void				StartPlayback						( DWORD pFilename, float fSpeed );
DARKSDK void				UpdatePlayback						( void );
DARKSDK void				StopPlayback						( void );
DARKSDK int					PlaybackPlaying						( void );
DARKSDK int					GetSessionTerminate					( void );
DARKSDK LPSTR				GetDXName							( void );

// extra functions
DARKSDK int ResetLeftEye ( void );
DARKSDK void ResetTracking ( void );
DARKSDK int GetChildWindowWidth(int iDPIAware);
DARKSDK int GetChildWindowHeight(int iDPIAware);
DARKSDK int GetChildWindowWidth();
DARKSDK int GetChildWindowHeight();
DARKSDK void SetChildWindowTruePixel ( int mode );
DARKSDK LPSTR GetDirectRefreshRate ( void );

DARKSDK void SETUPLoadAllCoreShadersFIRST ( int iShowDetailedShaderErrorMessage );
DARKSDK void SETUPLoadAllCoreShadersREST ( int iShowDetailedShaderErrorMessage, int iPBRMode );

void SetRenderAndDepthTarget ( LPGGRENDERTARGETVIEW render, LPGGDEPTHSTENCILVIEW depth );

#endif _CGFX_H_
