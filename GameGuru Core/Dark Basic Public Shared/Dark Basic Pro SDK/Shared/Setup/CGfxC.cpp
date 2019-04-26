#define _CRT_SECURE_NO_DEPRECATE
#define _USING_V110_SDK71_

// For DXDIAG version extraction
//#define INITGUID

#include "cGFXc.h"	// include GFX header
#include "..\..\DarkSDK\Setup\resource.h"
#include <delayimp.h>
#include <iostream> // can use STD:: with this include
#include <vector>
#include <DXGI.h>

#include "CTextC.h"
#include "CBasic2DC.h"
#include "CSpritesC.h"
#include "CImageC.h"
#include "CInputC.h"
#include "CSystemC.h"
#include "CMemblocks.h"
#include "CBitmapC.h"
#include "CAnimation.h"
#include "CObjectsC.h"
#include "CCameraC.h"
#include "CLightC.h"
#include "CParticleC.h"
#include "cVectorC.h"
#include "ConvX.h"
#include "CSoundC.h"
#include "BlitzTerrain.h"

#ifdef VR920SUPPORT
// Includes
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#define IWEAR_ONETIME_DEFINITIONS
#include "iWearSDK.h"
// Enumerations
#define						EXIT_STEREOSCOPIC_MODE	E_FAIL
enum						{ IWR_NOT_CONNECTED=-1, IWR_IS_CONNECTED };
enum						{ LEFT_EYE=0, RIGHT_EYE, MONO_EYES };
enum						{ NO_FILTERING=0, APPLICATION_METHOD, DRIVER_METHOD };
// Prototypes and externs
HRESULT						D3DWindowedWaitForVsync( void );
// Globals
bool						g_VR920StereoMethod = false;
bool						g_VR920AdapterAvailable = false;
bool						g_StereoEnabled	= true;
HANDLE						g_StereoHandle	= INVALID_HANDLE_VALUE;
UINT						g_StereoEyeToggle = LEFT_EYE;
LPGGQUERY9					g_pLeftEyeQuery	= NULL;
LPGGQUERY9					g_pRightEyeQuery = NULL;
bool						g_VR920RenderStereoNow = false;
float						g_fVR920TrackingYaw	= 0;
float						g_fVR920TrackingPitch = 0;
float						g_fVR920TrackingRoll = 0;
float						g_fVR920Sensitivity = 1.0f;
float						g_fDriverCompensationPitch = 0;
float						g_fDriverCompensationYaw = 0;
float						g_fDriverCompensationRoll = 0;
#endif

DBPRO_GLOBAL tagInfo*						m_pInfo;			// gfx card information
DBPRO_GLOBAL int							m_iAdapterCount;	// number of graphic cars
DBPRO_GLOBAL int							m_iAdapterUsed;		// graphics card being used by number
DBPRO_GLOBAL char							m_pAdapterName[_MAX_PATH];// graphica card being used by name 
DBPRO_GLOBAL bool							m_bDraw;			// flag to start a scene
DBPRO_GLOBAL bool							m_iDisplayChange;	// set when display has been switched
DBPRO_GLOBAL HWND							m_hWnd;				// handle to main VISUAL window(Editor window in IDE eventually)

DBPRO_GLOBAL bool							m_bOverrideHWND;	// should we override the window
DBPRO_GLOBAL bool							m_bResizeWindow;	// should we resize the window

DBPRO_GLOBAL GGPRESENT_PARAMETERS*			m_D3DPP				= NULL;
DBPRO_GLOBAL int							m_iWidth;			// width of display mode
DBPRO_GLOBAL int							m_iHeight;			// height of display mode
DBPRO_GLOBAL int							m_iChopWidth;		// chop width of display mode (for nonstandard setdisplaymode sizes)
DBPRO_GLOBAL int							m_iChopHeight;		// chop height of display mode
DBPRO_GLOBAL int							m_iDisplayType;		// windowed / fullscreen
DBPRO_GLOBAL int							m_iProcess;			// vertex processing mode
DBPRO_GLOBAL int							m_iDepth;			// depth of display
DBPRO_GLOBAL int							m_iBackBufferCount;	// back buffer count
DBPRO_GLOBAL bool							m_bLockable;		// lockable mode
DBPRO_GLOBAL bool							m_bVSync = true;
DBPRO_GLOBAL int							m_iVSyncInterval = 1;
DBPRO_GLOBAL bool							m_bVSyncInitialDefault = false;
DBPRO_GLOBAL int							m_iMultisamplingFactor = 0;
DBPRO_GLOBAL int							m_iMultimonitorMode = 0;
DBPRO_GLOBAL int							m_iModBackbufferWidth = 0;
DBPRO_GLOBAL int							m_iModBackbufferHeight = 0;
DBPRO_GLOBAL UINT							m_uAdapterChoice = GGADAPTER_DEFAULT;
DBPRO_GLOBAL bool							m_bNVPERFHUD = false;
DBPRO_GLOBAL int							m_iForceAdapterOrdinal = 0;
DBPRO_GLOBAL int							m_iForceAdapterD3D11ONLY = 0;

DBPRO_GLOBAL GGFORMAT						m_Depth;			// final back bufferformat
DBPRO_GLOBAL GGFORMAT						m_StencilDepth;		// final stencil buffer format
DBPRO_GLOBAL DWORD							m_dwFlags;			// flags
DBPRO_GLOBAL GGDISPLAYMODE					m_WindowsD3DMODE;
DBPRO_GLOBAL GGFORMAT						g_GGFORMAT;		// dx9 adapter format of choice

DBPRO_GLOBAL bool							m_bZBuffer;			// ZBuffer present
DBPRO_GLOBAL GGSWAPEFFECT					m_SwapMode;			// Whether in COPY or FLIP Mode

DBPRO_GLOBAL int							m_iWindowWidth;		// width of window
DBPRO_GLOBAL int							m_iWindowHeight;	// window height

DBPRO_GLOBAL LPGGSWAPCHAIN					m_pSwapChain [ MAX_SWAP_CHAINS ];
DBPRO_GLOBAL int							m_iSwapChainCount;

DBPRO_GLOBAL int							m_iGammaRed;
DBPRO_GLOBAL int							m_iGammaBlue;
DBPRO_GLOBAL int							m_iGammaGreen;

DBPRO_GLOBAL LPGGSURFACE					g_pBackBuffer = NULL;	//globals for locking backbuffer (commands)
DBPRO_GLOBAL LPGGRENDERTARGETVIEW			m_pRenderTargetView = NULL;
DBPRO_GLOBAL LPGGSURFACE					m_pDepthStencil = NULL;
DBPRO_GLOBAL LPGGDEPTHSTENCILVIEW			m_pDepthStencilView = NULL;
DBPRO_GLOBAL LPGGSHADERRESOURCEVIEW			m_pDepthStencilResourceView = NULL;

DBPRO_GLOBAL DWORD							g_dwSurfacePtr = 0;
DBPRO_GLOBAL DWORD							g_dwSurfaceWidth = 0;
DBPRO_GLOBAL DWORD							g_dwSurfaceHeight = 0;
DBPRO_GLOBAL DWORD							g_dwSurfaceDepth = 0;
DBPRO_GLOBAL DWORD							g_dwSurfacePitch = 0;

DBPRO_GLOBAL bool							gbScreenBecomeInvalid=false;	// globals for fine windows control (commands)
DBPRO_GLOBAL bool							gbWindowMode=false;
DBPRO_GLOBAL bool							gbWindowBorderActive=true;
DBPRO_GLOBAL DWORD							gWindowSizeX=0;
DBPRO_GLOBAL DWORD							gWindowSizeY=0;
DBPRO_GLOBAL DWORD							gWindowExtraX=0;
DBPRO_GLOBAL DWORD							gWindowExtraY=0;
DBPRO_GLOBAL DWORD							gWindowExtraXForOverlap=0;
DBPRO_GLOBAL DWORD							gWindowExtraYForOverlap=0;
DBPRO_GLOBAL DWORD							gWindowVisible=SW_SHOWDEFAULT;
DBPRO_GLOBAL char							gWindowName[256];
DBPRO_GLOBAL DWORD							gWindowStyle=WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU;
DBPRO_GLOBAL HICON							gOriginalIcon=NULL;
DBPRO_GLOBAL HICON							gWindowIconHandle=NULL;
DBPRO_GLOBAL bool							gbFirstInitOfDisplayOnly=true;

// to allow true pixel representation when rendering to a child window.
DBPRO_GLOBAL DWORD							g_dwChildWindowTruePixel = 0;

static bool									g_bCreateChecklistNow=false;
static DWORD								g_dwMaxStringSizeInEnum=0;
static char									m_pWorkString[_MAX_PATH];

DBPRO_GLOBAL bool							g_bValidFPS = true;
DBPRO_GLOBAL bool							g_bSceneBegun = false;
DBPRO_GLOBAL LPGGSURFACE					g_pGFXBackBufferRenderTarget = NULL;

extern LPGG									m_pDX;				// interface to D3D
extern LPGGDEVICE							m_pD3D;				// D3D device
extern GlobStruct*							g_pGlob;
extern PTR_FuncCreateStr					g_pCreateDeleteStringFunction;
DBPRO_GLOBAL D3D_FEATURE_LEVEL				g_featureLevel;

DBPRO_GLOBAL HWND							g_OldHwnd						= NULL;
DBPRO_GLOBAL bool							g_bWindowOverride				= false;
DBPRO_GLOBAL char							g_szMainWindow  [ MAX_PATH ];
DBPRO_GLOBAL char							g_szChildWindow [ MAX_PATH ];

bool										gbAlwaysIgnoreShaderBlobFile = false;

#ifdef DX11
sShaderGlobalsType							g_sShaders [ SHADERSARRAYMAX ];
int											g_iShowDetailedShaderErrorMessage = 0;
#endif

///GGMATRIX m_worldMatrix;

#if DEBUG_MODE
	DBPRO_GLOBAL FILE*	m_fp;
#endif

// mike - 070207 - need a way of overriding the present mode to draw to custom
//			     - areas, the globals are placed in a namespace as a simple way
//			     - of avoiding conflicts between the libraries
namespace DisplayLibrary
{
	bool					g_bCustomPresentMode;
	std::vector < RECT >	g_CustomPresentRectangles;
};

DARKSDK GlobStruct* GetGlobalData ( void )
{
	return g_pGlob;
}

void GetD3DExtraInfo ( int *piAdapterOrdinal, LPSTR pAdapterName, int* piFeatureLevel )
{
	*piAdapterOrdinal = m_uAdapterChoice;
	strcpy ( pAdapterName, m_pAdapterName );
	*piFeatureLevel = g_featureLevel;
}

static LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep, std::string& strError)
{
	// Structured Exception Handler for delay loaded DLLs
	// If this is a Delay-load problem, ExceptionInformation[0] points 
	// to a DelayLoadInfo structure that has detailed error info
	//PDelayLoadInfo pdli = PDelayLoadInfo(pep->ExceptionRecord->ExceptionInformation[0]);
	//switch(pep->ExceptionRecord->ExceptionCode)
	//{
	//default:
	//	// We don't recognize this exception
  	//	return EXCEPTION_CONTINUE_SEARCH;
	//	break;
	//}
	return EXCEPTION_CONTINUE_SEARCH;
}

std::string strInitD3DXError;
static bool InitD3DX ( void )
{
	return true;
	/*
	bool bRet = true;
	__try
	{
		// Load D3DX DLL by calling one of its functions
		GGMATRIXA16 m1, m2, m3;
		memset(&m1, 0, sizeof(m1));
		memset(&m2, 0, sizeof(m2));
		GGMatrixMultiply(&m3, &m1, &m2);
	}
	__except (DelayLoadDllExceptionFilter(GetExceptionInformation(), strInitD3DXError))
	{
		bRet = false;
	}
	return bRet;
	*/
}

DARKSDK bool SETUPConstructorDX9 ( void )
{
	// Direct X 9 Creation
	#ifndef DX11
	// Load D3DX (need to provoke D3DX DLL to load in, so we call a D3DX command that does not need m_pD3D)
	bool bD3DXAvailable = false;
	if( InitD3DX() ) bD3DXAvailable = true;
	if ( FAILED ( m_pDX = Direct3DCreate9 ( D3D_SDK_VERSION ) ) || bD3DXAvailable==false )
	{
		Error ( "Unable to create Direct3D interface" );
		return false;
	}

	// obtain hardware information
	m_iAdapterCount = m_pDX->GetAdapterCount ( );

	// setup adapter storage
	m_pInfo = new tagInfo [ m_iAdapterCount ];
	memset ( m_pInfo, 0, sizeof ( m_pInfo ) * m_iAdapterCount );

	// find out graphics card information e.g.
	int	iTemp = 0;
	for ( UINT iAdapter = 0; iAdapter < (DWORD)m_iAdapterCount; iAdapter++ )
	{
		GGDISPLAYMODE			d3dmode;
		GGCAPS				d3dcaps;
		D3DADAPTER_IDENTIFIER9	d3dAdapter;
		HRESULT					hr;

		// clear out any structures
		memset ( &d3dAdapter, 0, sizeof ( d3dAdapter ) );
		memset ( &d3dmode,    0, sizeof ( d3dmode    ) );
		memset ( &d3dcaps,    0, sizeof ( d3dcaps    ) );

		// get current display mode
		if ( FAILED ( hr = m_pDX->GetAdapterDisplayMode ( iAdapter, &d3dmode ) ) )
			Error ( "Failed to get adapter display mode" );

		// get device caps
		if ( FAILED ( hr = m_pDX->GetDeviceCaps ( iAdapter, D3DDEVTYPE_HAL, &d3dcaps ) ) )
			Error ( "Failed to get device caps" );

		// get adapter info
		if ( FAILED ( hr = m_pDX->GetAdapterIdentifier ( iAdapter, 0, &d3dAdapter ) ) )
			Error ( "Failed to get adapter identifier" );

		// get the name of the device
		m_pInfo [ iAdapter ].szName = new char [ 256 ];

		if ( !m_pInfo [ iAdapter ].szName )
			Error ( "Failed to allocate adapter name storage" );

		memset ( m_pInfo [ iAdapter ].szName, 0, sizeof ( char ) * 256 );
		memcpy ( m_pInfo [ iAdapter ].szName, d3dAdapter.Description, sizeof ( char ) * 256 );

		// copy adapter info
		memset ( &m_pInfo [ iAdapter ].D3DAdapter, 0, sizeof ( m_pInfo [ iAdapter ].D3DAdapter ) );
		memcpy ( &m_pInfo [ iAdapter ].D3DAdapter, &d3dAdapter, sizeof ( d3dAdapter ) );

		// copy current display mode
		memset ( &m_pInfo [ iAdapter ].D3DMode, 0, sizeof ( m_pInfo [ iAdapter ].D3DMode ) );
		memcpy ( &m_pInfo [ iAdapter ].D3DMode, &d3dmode, sizeof ( d3dmode ) );

		// copy the device caps
		memset ( &m_pInfo [ iAdapter ].D3DCaps, 0, sizeof ( m_pInfo [ iAdapter ].D3DCaps ) );
		memcpy ( &m_pInfo [ iAdapter ].D3DCaps, &d3dcaps, sizeof ( d3dcaps ) );

		// dx8->dx9 get all available display modes
		int iModeCount=0;
		m_pInfo [ iAdapter ].iDisplayCount = 0;
		while (iModeCount>=0 && iModeCount<=5)
		{
			if ( iModeCount==0 ) g_GGFORMAT = GGFMT_X8R8G8B8;
			if ( iModeCount==1 ) g_GGFORMAT = GGFMT_A8R8G8B8;
			if ( iModeCount==2 ) g_GGFORMAT = GGFMT_X1R5G5B5;
			if ( iModeCount==3 ) g_GGFORMAT = GGFMT_A1R5G5B5;
			if ( iModeCount==4 ) g_GGFORMAT = GGFMT_R5G6B5;
			if ( iModeCount==5 ) g_GGFORMAT = GGFMT_A2R10G10B10;
			m_pInfo [ iAdapter ].iDisplayCount += m_pDX->GetAdapterModeCount ( iAdapter, g_GGFORMAT );
			iModeCount++;
		}

		// create display mode array
		m_pInfo [ iAdapter ].D3DDisplay    = new GGDISPLAYMODE [ m_pInfo [ iAdapter ].iDisplayCount ];
		if ( !m_pInfo [ iAdapter ].D3DDisplay )
			Error ( "Failed to allocate dispay list" );

		// clear display mode array
		memset ( m_pInfo [ iAdapter ].D3DDisplay, 0, sizeof ( m_pInfo [ iAdapter ].D3DDisplay ) * m_pInfo [ iAdapter ].iDisplayCount );

		// enumerate display modes
		iModeCount=0;
		int iArrayCount=0;
		GGDISPLAYMODE DisplayMode, LastDsiplayMode;
		while (iModeCount>=0 && iModeCount<=5)
		{
			// formats
			if ( iModeCount==0 ) g_GGFORMAT = GGFMT_X8R8G8B8;
			if ( iModeCount==1 ) g_GGFORMAT = GGFMT_A8R8G8B8;
			if ( iModeCount==2 ) g_GGFORMAT = GGFMT_X1R5G5B5;
			if ( iModeCount==3 ) g_GGFORMAT = GGFMT_A1R5G5B5;
			if ( iModeCount==4 ) g_GGFORMAT = GGFMT_R5G6B5;
			if ( iModeCount==5 ) g_GGFORMAT = GGFMT_A2R10G10B10;

			// get number of them
			int iLocalCount = m_pDX->GetAdapterModeCount ( iAdapter, g_GGFORMAT );

			// enumerate them into list
			LastDsiplayMode.Width=0;
			LastDsiplayMode.Height=0;
			for ( iTemp=0; iTemp<iLocalCount; iTemp++ )
			{
				// enum for display mode data
				if ( FAILED ( hr = m_pDX->EnumAdapterModes ( iAdapter, g_GGFORMAT, iTemp, &m_pInfo [ iAdapter ].D3DDisplay [ iArrayCount ] ) ) )
					Error ( "Failed to enumerate adapter display modes" );

				// leefix - 200603 - most modes repeated for refresh rate, ignore refresh differences
				DisplayMode = m_pInfo [ iAdapter ].D3DDisplay [ iArrayCount ];
				if( DisplayMode.Width==LastDsiplayMode.Width && DisplayMode.Height==LastDsiplayMode.Height
				&& GetBitDepthFromFormat(DisplayMode.Format)==GetBitDepthFromFormat(LastDsiplayMode.Format) )
				{
					// do not increment array, so we overwrite with next one..
				}
				else
				{
					// next in array
					iArrayCount++;
				}
				LastDsiplayMode = DisplayMode;
			}

			// next mode
			iModeCount++;
		}

		// adjust size to actual display modes to use
		m_pInfo [ iAdapter ].iDisplayCount = iArrayCount;
	}

	// graphics card being used by number
	m_iAdapterUsed=0;		
	strcpy(m_pAdapterName, m_pInfo [ m_iAdapterUsed ].szName);
	#endif

	// success
	return true;
}

DARKSDK bool SETUPConstructorDX11 ( void )
{
	#ifdef DX11
	// Direct X 11 Creation

	// create m_pDX using equiv. Direct3DCreate9 ( D3D_SDK_VERSION )

	// fill m_pInfo with all adapters (needed?)
	// m_iAdapterCount = m_pDX->GetAdapterCount ( );
	//m_pInfo = new tagInfo [ m_iAdapterCount ];
	//memset ( m_pInfo, 0, sizeof ( m_pInfo ) * m_iAdapterCount );
	//for ( UINT iAdapter = 0; iAdapter < (DWORD)m_iAdapterCount; iAdapter++ )
	//if ( FAILED ( hr = m_pDX->GetAdapterDisplayMode ( iAdapter, &d3dmode ) ) )
	//if ( FAILED ( hr = m_pDX->GetDeviceCaps ( iAdapter, D3DDEVTYPE_HAL, &d3dcaps ) ) )
	//if ( FAILED ( hr = m_pDX->GetAdapterIdentifier ( iAdapter, 0, &d3dAdapter ) ) )
	//m_pInfo [ iAdapter ].szName = new char [ 256 ];
	//memset ( m_pInfo [ iAdapter ].szName, 0, sizeof ( char ) * 256 );
	//memcpy ( m_pInfo [ iAdapter ].szName, d3dAdapter.Description, sizeof ( char ) * 256 );
	//memset ( &m_pInfo [ iAdapter ].D3DAdapter, 0, sizeof ( m_pInfo [ iAdapter ].D3DAdapter ) );
	//memcpy ( &m_pInfo [ iAdapter ].D3DAdapter, &d3dAdapter, sizeof ( d3dAdapter ) );
	//memset ( &m_pInfo [ iAdapter ].D3DMode, 0, sizeof ( m_pInfo [ iAdapter ].D3DMode ) );
	//memcpy ( &m_pInfo [ iAdapter ].D3DMode, &d3dmode, sizeof ( d3dmode ) );
	//memset ( &m_pInfo [ iAdapter ].D3DCaps, 0, sizeof ( m_pInfo [ iAdapter ].D3DCaps ) );
	//memcpy ( &m_pInfo [ iAdapter ].D3DCaps, &d3dcaps, sizeof ( d3dcaps ) );
	//m_pInfo [ iAdapter ].iDisplayCount = 0;
	//while (iModeCount>=0 && iModeCount<=5) {
	//if ( iModeCount==0 ) g_GGFORMAT = GGFMT_X8R8G8B8;
	//if ( iModeCount==1 ) g_GGFORMAT = GGFMT_A8R8G8B8;
	//m_pInfo [ iAdapter ].iDisplayCount += m_pDX->GetAdapterModeCount ( iAdapter, g_GGFORMAT );
	//iModeCount++;}
	//m_pInfo [ iAdapter ].D3DDisplay    = new GGDISPLAYMODE [ m_pInfo [ iAdapter ].iDisplayCount ];
	//memset ( m_pInfo [ iAdapter ].D3DDisplay, 0, sizeof ( m_pInfo [ iAdapter ].D3DDisplay ) * m_pInfo [ iAdapter ].iDisplayCount );
	//if ( iModeCount==0 ) g_GGFORMAT = GGFMT_X8R8G8B8;
	//if ( iModeCount==1 ) g_GGFORMAT = GGFMT_A8R8G8B8;
	//int iLocalCount = m_pDX->GetAdapterModeCount ( iAdapter, g_GGFORMAT );
	//for ( iTemp=0; iTemp<iLocalCount; iTemp++ )
	//if ( FAILED ( hr = m_pDX->EnumAdapterModes ( iAdapter, g_GGFORMAT, iTemp, &m_pInfo [ iAdapter ].D3DDisplay [ iArrayCount ] ) ) )
	//DisplayMode = m_pInfo [ iAdapter ].D3DDisplay [ iArrayCount ];
	//iArrayCount++;
	//m_pInfo [ iAdapter ].iDisplayCount = iArrayCount;
	//m_iAdapterUsed=0;		
	//strcpy(m_pAdapterName, m_pInfo [ m_iAdapterUsed ].szName);
	#endif

	// success
	return true;
}

DARKSDK bool SETUPConstructor ( void )
{
	// setup default values
	m_iDisplayType   = 1;
	m_iDisplayChange = false;
	m_bOverrideHWND  = false;
	m_bResizeWindow  = true;
	m_Depth = GGFMT_UNKNOWN;
	m_StencilDepth = GGFMT_UNKNOWN;
	m_hWnd = GetForegroundWindow(); // redundant line?
	m_iSwapChainCount = 0;
	m_iGammaRed   = 255;
	m_iGammaGreen = 255;
	m_iGammaBlue  = 255;

	// ensure this is false by default
	DisplayLibrary::g_bCustomPresentMode = false;

	#ifdef DX11
	 SETUPConstructorDX11();
	#else
	 SETUPConstructorDX9();
	#endif

	// Default Window Settings for Windows Control
	gbWindowMode=true;
	gbWindowBorderActive=false;
	strcpy(gWindowName,"");
	gWindowVisible=SW_SHOWDEFAULT;
	gWindowSizeX = m_iWidth;
	gWindowSizeY = m_iHeight;
	gWindowIconHandle = NULL;
	gWindowExtraX = gWindowExtraXForOverlap;
	gWindowExtraY = gWindowExtraYForOverlap;
	gWindowStyle = WS_OVERLAPPEDWINDOW;
	gbFirstInitOfDisplayOnly=true;

	// Get Window Display Mode (for later use for windows-modes)
	#ifdef DX11
	#else
	m_pDX->GetAdapterDisplayMode ( 0, &m_WindowsD3DMODE );
	#endif

	// disable screen saver at start of program
	SystemParametersInfo ( SPI_SETSCREENSAVEACTIVE, FALSE, 0, SPIF_SENDCHANGE );

	// Success
	return true;
}

#ifdef DX11
DARKSDK HRESULT SwapChainPresent ( int iID )
{
	if ( m_pSwapChain [ iID ] )
	{
		int iSyncInterval = 0;
		if ( m_bVSync )
		{
			// CAP TO MONITOR REFRESH RATE - NO TEARING
			iSyncInterval = m_iVSyncInterval;
		}
		else
		{
			// FAST AS YOU CAN - HAS HORIZ TEARING
			iSyncInterval = 0;
		}
		return m_pSwapChain [ iID ]->Present( iSyncInterval, 0 );
	}
	else
		return 0;
}
#endif

#ifdef VR920SUPPORT
/*
UINT IWRIsVR920Connected( bool *PrimaryDevice )
{
	IWRGetProductID
	DISPLAY_DEVICE	DisplayDevice, MonitorDevice;
	// Step 1: Find VR920 Device+Adapter
	*PrimaryDevice = false;
	DisplayDevice.cb = MonitorDevice.cb = sizeof( DISPLAY_DEVICE );
	for( int i=0; EnumDisplayDevices(NULL, i, &DisplayDevice, 0x0 ); i++ )
	{
		// Ignore mirrored devices and only look at desktop attachments.
		if( !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) &&
			(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) )
		{
			for( int j=0; EnumDisplayDevices(DisplayDevice.DeviceName, j, &MonitorDevice, 0x0 ); j++ )
			{
				if( (MonitorDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
					(strstr( MonitorDevice.DeviceID, "IWR0002" ) ||  // VR920 id
					 strstr( MonitorDevice.DeviceID, "IWR0149" ))) { // Wrap920 id
					// Found the VR920 PnP id.
					if( DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE ) 
						*PrimaryDevice = true; // VR920 is the primary display device.
					return IWR_IS_CONNECTED;
				}
			}
		}
	}
	// VR920 does not appear to be on any accessible adapters.
	return IWR_NOT_CONNECTED;
}
*/

//-----------------------------------------------------------------------------
// Provide for a method in (Windowed Mode) to poll the adapters Vertical Sync function.
//  This is a requirement for the VR920s VGA to Frame syncronization process AND
//  MUST be available for highest possible performance in windowed mode.
//-----------------------------------------------------------------------------
HRESULT	D3DWindowedWaitForVsync( void )
{
	/* find DX11 equiviliant
	HRESULT				hr=E_FAIL;
	D3DRASTER_STATUS	rStatus;
	if( g_d3dcaps.Caps & D3DCAPS_READ_SCANLINE )
	{
		// IF Polling VSync is available:
		hr = m_pD3D->GetRasterStatus( 0, &rStatus );
		if( hr == S_OK )
		{
			// In rare case when in vblank, Wait for vblank to drop.
			while( rStatus.InVBlank )
				m_pD3D->GetRasterStatus( 0,&rStatus );
			while( rStatus.ScanLine < (UINT)(m_iWindowHeight-1) )
			{
				m_pD3D->GetRasterStatus( 0,&rStatus );
				// if scan ever crossed vblank again; break. could be issue with window transitioning modes.
				if( rStatus.InVBlank )
					break;
			}
		}
	}
	// IF Polling VSync is NOT available:
	if( hr != S_OK ) Sleep( 10 );
	*/
	return S_OK;
}
//-----------------------------------------------------------------------------
// Provide for a process syncronize a rendered left or right eye to the VR920s frame
// Buffers.  Common to both Windowed and FullScreen modes.
//-----------------------------------------------------------------------------
HRESULT	IWRSyncronizeEye( int Eye )
{
	HRESULT		hr=S_OK;
	// Wait for acknowledgement from previous frame, to present a new left eye.
	if ( IWRSTEREO_WaitForAck ) IWRSTEREO_WaitForAck( g_StereoHandle, Eye );
	// Windowed mode: Wait for the start of Vsync prior to presenting.
	//// if( m_iDisplayType != FULLSCREEN ) D3DWindowedWaitForVsync(); MAKES IT STUTTER!!
	// Ensure GPU is ready. 
	// If using the locking method nows the time to wait for a lock on the backbuffer.
	// Put the frame in the queue(full screen). or immediate copy(windowed mode).
	//hr = m_pD3D->Present( NULL, NULL, NULL, NULL );
	if ( g_StereoEyeToggle == 1 ) SwapChainPresent(0);
	/* WRAP does side by side, VR920 is true fullscreen stereo (below)
	if ( SwapChainPresent(0) == S_OK )
	{
		//if( FAILED(hr) ) return EXIT_STEREOSCOPIC_MODE;
		// If using the Query GPU method. wait for GPU now.
		if( Eye == LEFT_EYE )
		{
			// Wait for Left eye GPU status ok.
			if ( g_pLeftEyeQuery )
			{
				//while(S_FALSE == g_pLeftEyeQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ));
				BOOL pData = FALSE;
				while ( pData == FALSE )
				{
					m_pImmediateContext->GetData ( g_pLeftEyeQuery, &pData, sizeof(pData), 0 );
				}
			}
		}
		else
		{
			// Wait for Right eye GPU status ok.
			if ( g_pRightEyeQuery )
			{
				//while(S_FALSE == g_pRightEyeQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ));
				BOOL pData = FALSE;
				while ( pData == FALSE )
				{
					m_pImmediateContext->GetData ( g_pRightEyeQuery, &pData, sizeof(pData), 0 );
				}
			}
		}
		// Signal to the VR920 the next eyes frame is available
		// AND: Will scan out on the NEXT Vsync interval.
		if( !IWRSTEREO_SetLR( g_StereoHandle, Eye ) ) 
			return EXIT_STEREOSCOPIC_MODE;
	}
	*/
	// Signal to the VR920 the next eyes frame is available
	// AND: Will scan out on the NEXT Vsync interval.
	if(IWRSTEREO_SetLR)
		if( !IWRSTEREO_SetLR( g_StereoHandle, Eye ) ) 
			return EXIT_STEREOSCOPIC_MODE;
	return S_OK;
}
bool SetupGetTracking ( float* pfYaw, float* pfPitch, float* pfRoll, float fSensitivity )
{
	// tracking status
	bool bTracking = true;
	if ( IWRGetTracking == NULL ) { return false; }

	// head tracker
	long Roll=0, Yaw=0, Pitch=0;

	// get tracking data
	HRESULT iwr_status = IWRGetTracking( &Yaw, &Pitch, &Roll );  
	if(	iwr_status != IWR_OK )
	{
		// Attempt to re-establish communications with the VR920.
		IWROpenTracker();	
		HRESULT iwr_status = IWRGetTracking( &Yaw, &Pitch, &Roll );  
		if(	iwr_status != IWR_OK )
			bTracking = false;

		// iWear tracker could be OFFLine: just inform user or wait until plugged in...
		Yaw = Pitch = Roll = 0;
	}

	// See if we cannot add sensitivity controls to head tracking
	if ( fSensitivity != 1.0f )
	{
		if ( fSensitivity < 0.75f ) fSensitivity = 0.75f;
		if ( fSensitivity > 3.0f ) fSensitivity = 3.0f;
		Yaw *= fSensitivity;
		Pitch *= fSensitivity;
		Roll *= fSensitivity;
	}

	// Always provide for a means to disable filtering;
	//if( g_Filtering == APPLICATION_METHOD ) 
	//{
	//	IWRFilterTracking( &Yaw, &Pitch, &Roll );
	//}

	// get radians from data
	if ( pfYaw )
	{
		// get angles from tracker
		float fSuggestedPitch =  (float)Pitch * IWR_RAWTORAD;
		float fSuggestedYaw =  (float)Yaw * IWR_RAWTORAD;
		float fSuggestedRoll =  (float)Roll * IWR_RAWTORAD;

		// sometimes the values can reset (caused by driver resetting at extremes)
		float fDiffYaw = fSuggestedYaw - *pfYaw;
		if ( fabs(fDiffYaw) > 6.28f/8.0f )
		{
			// if instantly switches to more than 45 degrees, driver kicked in the change! ARG!
			float fDiffPitch = fSuggestedPitch - *pfPitch;
			float fDiffRoll = fSuggestedRoll - *pfRoll;
			g_fDriverCompensationPitch -= GGToDegree(fDiffPitch);
			g_fDriverCompensationYaw -= GGToDegree(fDiffYaw);
			g_fDriverCompensationRoll -= GGToDegree(fDiffRoll);
		}

		// final return value includes a compensation offset (for when driver resets YAW!)
		*pfPitch = fSuggestedPitch;
		*pfYaw   = fSuggestedYaw;
		*pfRoll  = fSuggestedRoll;
	}

	// return if tracking is present
	return bTracking;
}

void SetupResetTracking	( void )
{
	ResetTracking();
}
#endif

DARKSDK void GetBackBufferPointersDX9(void)
{
	#ifndef DX11
	if(g_pGlob)
	{
		if(g_pGlob->pHoldBackBufferPtr) g_pGlob->pHoldBackBufferPtr->Release();
		if(g_pGlob->pHoldDepthBufferPtr) g_pGlob->pHoldDepthBufferPtr->Release();
		m_pD3D->GetRenderTarget(0, &g_pGlob->pHoldBackBufferPtr);
		m_pD3D->GetDepthStencilSurface(&g_pGlob->pHoldDepthBufferPtr);
		m_pD3D->SetRenderTarget(0, g_pGlob->pHoldBackBufferPtr);
		m_pD3D->SetDepthStencilSurface ( g_pGlob->pHoldDepthBufferPtr );
		g_pGlob->pCurrentBitmapSurface=g_pGlob->pHoldBackBufferPtr;
		g_pGlob->iCurrentBitmapNumber=0;
	}
	#endif
}

DARKSDK void GetBackBufferPointersDX11(void)
{
	#ifdef DX11
	if(g_pGlob)
	{
		g_pGlob->pHoldBackBufferPtr = m_pRenderTargetView;
		g_pGlob->pHoldDepthBufferPtr = m_pDepthStencilView;
		g_pGlob->pCurrentBitmapSurface = g_pBackBuffer;
		g_pGlob->pCurrentBitmapSurfaceView = m_pRenderTargetView;
		g_pGlob->pCurrentBitmapDepthView = m_pDepthStencilView;
		g_pGlob->pCurrentRenderView = m_pRenderTargetView;
		g_pGlob->pCurrentDepthView = m_pDepthStencilView;
		g_pGlob->iCurrentBitmapNumber = 0;
		g_pGlob->dwClientRegionWidth = 0; // can be updated after present when scan client rect size
		g_pGlob->dwClientRegionHeight = 0;
	}
	#endif
}

DARKSDK void GetBackBufferPointers(void)
{
	#ifdef DX11
	GetBackBufferPointersDX11();
	#else
	GetBackBufferPointersDX9();
	#endif
}

DARKSDK void ReleaseBackBufferPointers(void)
{
	// U74 - BETA9 - 280609 - release VR920 if used
	#ifdef VR920SUPPORT
	if ( g_VR920StereoMethod==true )
	{
		// free D3D resources
		SAFE_RELEASE( g_pLeftEyeQuery );
		SAFE_RELEASE( g_pRightEyeQuery );
	}
	#endif

	// Free refs
	#ifdef DX11
	#else
	if(g_pGlob)
	{
		if(g_pGlob->pHoldBackBufferPtr)
		{
			g_pGlob->pHoldBackBufferPtr->Release();
			g_pGlob->pHoldBackBufferPtr=NULL;
		}
		if(g_pGlob->pHoldDepthBufferPtr)
		{
			g_pGlob->pHoldDepthBufferPtr->Release();
			g_pGlob->pHoldDepthBufferPtr=NULL;
		}
	}

	// Free locked surface
	if(g_dwSurfacePtr)
	{
		g_pBackBuffer->UnlockRect();
		g_dwSurfacePtr=NULL;
	}

	// Release backbuffer
	SAFE_RELEASE(g_pBackBuffer);
	#endif
}

DARKSDK void SETUPDestructor ( void ) 
{
	// release VR920 if used
	#ifdef VR920SUPPORT
	if ( g_VR920StereoMethod==true )
	{
		// free VR920 driver resources
		if ( g_StereoHandle )
		{
			IWRSTEREO_WaitForAck( g_StereoHandle, 0 );
			IWRSTEREO_SetStereo( g_StereoHandle, IWR_MONO_MODE );
			IWRSTEREO_Close( g_StereoHandle );
			//IWRFreeDll();
			IWRCloseTracker();
			g_StereoHandle=NULL;
		}
	}
	#endif

	// Free ref to backbuffer
	ReleaseBackBufferPointers();

	// clear up D3D
	#ifdef DX11
	SAFE_RELEASE ( m_pRenderTargetView );
	SAFE_RELEASE ( g_pBackBuffer );
	SAFE_RELEASE ( m_pDepthStencilView );
	SAFE_RELEASE ( m_pDepthStencilResourceView );
	SAFE_RELEASE ( m_pDepthStencil );
	SAFE_RELEASE ( m_pSwapChain[0] );
	SAFE_RELEASE ( m_pImmediateContext );
	SAFE_RELEASE ( m_pD3D );
	#else
	// Delete global property-desc of device
	if ( m_D3DPP )
	{
		delete m_D3DPP;
		m_D3DPP=NULL;
	}
	SAFE_RELEASE ( m_pD3D );	// release the device
	SAFE_RELEASE ( m_pDX );	// release the interface
	#endif

	// remove any adapter information
	for ( int iTemp = 0; iTemp < m_iAdapterCount; iTemp++ )
	{
		SAFE_DELETE_ARRAY ( m_pInfo [ iTemp ].szName     );	// clear out name
		SAFE_DELETE_ARRAY ( m_pInfo [ iTemp ].D3DDisplay );	// clear out display structure
	}

	// finally get rid of the adapter info array
	SAFE_DELETE_ARRAY ( m_pInfo );

	// enable screen saver at end of program
	SystemParametersInfo ( SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDCHANGE );
}

DARKSDK void SETUPSetErrorHandler ( LPVOID pErrorHandlerPtr ) 
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK BOOL CALLBACK EnumChildProc ( HWND hwnd, LPARAM lParam )
{
	char szBuffer [ MAX_PATH ];
	GetWindowText ( hwnd, szBuffer, MAX_PATH );
	if ( strcmp ( szBuffer, g_szChildWindow ) == 0 )
	{
		g_pGlob->hWnd     = hwnd;
		m_hWnd            = hwnd;
		g_bWindowOverride = true;
	}
	return TRUE;
}

DARKSDK BOOL CALLBACK EnumWindowsProc ( HWND hwnd, LPARAM lParam )
{
	char szBuffer [ MAX_PATH ];
	GetWindowText ( hwnd, szBuffer, MAX_PATH );
	int iResult = strspn ( szBuffer, g_szMainWindow );
	if ( iResult )
		EnumChildWindows ( hwnd, EnumChildProc, 0 );
	
	return TRUE;
}

DARKSDK void AttachWindowToChildOfAnother ( LPSTR pAbsoluteAppFilename )
{
	if ( !pAbsoluteAppFilename )
		return;

	// find external settings file (230105 - added >4 detect as it corrupted stack before in release mode)
	char pAppExtFile[_MAX_PATH];
	if ( strlen ( pAbsoluteAppFilename ) > 4 )
	{
		// myprog.exe becomes myprog.ini in pAppExtFile
		strcpy ( pAppExtFile, pAbsoluteAppFilename );
		strcpy ( pAppExtFile + strlen(pAppExtFile) - 4, ".ini" );
	}
	else
		return;

	// clear strings
	strcpy ( g_szMainWindow, "" );
	strcpy ( g_szChildWindow, "" );

	// read data from settings file
	GetPrivateProfileString ( "External", "Main Window", "", g_szMainWindow, MAX_PATH, pAppExtFile );
	GetPrivateProfileString ( "External", "Child Window", "", g_szChildWindow, MAX_PATH, pAppExtFile );

	// determine if window should be attached to child of another
	if ( strlen ( g_szMainWindow ) > 1 )
		EnumWindows ( EnumWindowsProc, 0 );

	// if not overridden, try again after delay
	if ( g_bWindowOverride == false )
	{
		Sleep(2000);
		if ( strlen ( g_szMainWindow ) > 1 )
			EnumWindows ( EnumWindowsProc, 0 );

		if ( g_bWindowOverride == false )
		{
			Sleep(3000);
			if ( strlen ( g_szMainWindow ) > 1 )
				EnumWindows ( EnumWindowsProc, 0 );
		}
	}
}

DARKSDK void SETUPPassCoreData ( LPVOID pGlobPtr, int iStage ) 
{
	switch(iStage)
	{
		case 0 :	// Constructor Phase
					g_OldHwnd = g_pGlob->hWnd;
					// Make this window child of another (if applicable)
					AttachWindowToChildOfAnother ( (LPSTR)g_pGlob->ppEXEAbsFilename );
					// done
					break;
		case 1 :	// Post-Device-Creation Phase
					g_pGlob = (GlobStruct*)pGlobPtr;
					g_pCreateDeleteStringFunction = g_pGlob->CreateDeleteString;
					#ifdef DX11
					#else
					SETUPClear(0,0,0);
					GetBackBufferPointers();
					#endif
					break;
	}
}

DARKSDK void BuildFunctionsForDLLRefresh(void)
{
}

DARKSDK void InformDLLsOfDeviceLostOrNotReset ( int iDeviceLost )
{
	// leeadd - 020308 - inform all TPC DLLs that Device Has Been Lost
	if ( g_pGlob->pDynMemPtr )
	{
		// get local copy of DLLs
		HINSTANCE	hDLLMod[256];
		bool		bDLLTPC[256];
		memcpy ( hDLLMod, g_pGlob->pDynMemPtr+0, (sizeof(HINSTANCE)*256) );
		memcpy ( bDLLTPC, g_pGlob->pDynMemPtr+(sizeof(HINSTANCE)*256), (sizeof(bool)*256) );
	}
}

DARKSDK void InformDLLsOfD3DChange(int iMode)
{
	// Make sure altest DLLs have links to their refresh functions
	//
	// COOL TRICK : REM out refresh lines to find resource leak when SETDSPLAYMODE
	//
	BuildFunctionsForDLLRefresh();

	// D3D Change release/recreate
	if(g_pGlob)
	{
		// Release/Regrab backbuffer pointers
		if(iMode==0) ReleaseBackBufferPointers();
		if(iMode==1) GetBackBufferPointers();

		// Call Refresh Function of Any Active DLLS
		TextRefreshGRAFIX(iMode);
		Basic2DRefreshGRAFIX(iMode);
		SpritesRefreshGRAFIX(iMode);
		ImageRefreshGRAFIX(iMode);
		InputRefreshGRAFIX(iMode);
		SystemRefreshGRAFIX(iMode); 
		MemblocksRefreshGRAFIX(iMode);
		BitmapRefreshGRAFIX(iMode);
		AnimationRefreshGRAFIX(iMode);
		Basic3DRefreshGRAFIX(iMode);
		CameraRefreshGRAFIX(iMode);
		LightRefreshGRAFIX(iMode);
		VectorRefreshGRAFIX(iMode);
		SoundRefreshGRAFIX(iMode);

		// leeadd - 280305 - must also refresh all TPC DLls (u58)
		if ( g_pGlob->pDynMemPtr )
		{
			// get local copy of DLLs
			HINSTANCE	hDLLMod[256];
			bool		bDLLTPC[256];
			memcpy ( hDLLMod, g_pGlob->pDynMemPtr+0, (sizeof(HINSTANCE)*256) );
			memcpy ( bDLLTPC, g_pGlob->pDynMemPtr+(sizeof(HINSTANCE)*256), (sizeof(bool)*256) );
		}
	}
}

static LPSTR GetReturnStringFromWorkString(void)
{
	LPSTR pReturnString=NULL;
	if(m_pWorkString)
	{
		DWORD dwSize=strlen(m_pWorkString);
		g_pCreateDeleteStringFunction((DWORD*)&pReturnString, dwSize+1);
		strcpy(pReturnString, m_pWorkString);
	}
	return pReturnString;
}

static int GetBitDepthFromFormatDX9(GGFORMAT Format)
{
	#ifndef DX11
	switch(Format)
	{
		case GGFMT_R8G8B8 :		return 24;	break;
		case GGFMT_A8R8G8B8 :		return 32;	break;
		case GGFMT_X8R8G8B8 :		return 32;	break;
		case GGFMT_R5G6B5 :		return 16;	break;
		case GGFMT_X1R5G5B5 :		return 16;	break;
		case GGFMT_A1R5G5B5 :		return 16;	break;
		case GGFMT_A4R4G4B4 :		return 16;	break;
		case GGFMT_A8	:			return 8;	break;
		case GGFMT_R3G3B2 :		return 8;	break;
		case GGFMT_A8R3G3B2 :		return 16;	break;
		case GGFMT_X4R4G4B4 :		return 16;	break;
		case GGFMT_A2B10G10R10 :	return 32;	break;
		case GGFMT_G16R16 :		return 32;	break;
		case GGFMT_A8P8 :			return 8;	break;
		case GGFMT_P8 :			return 8;	break;
		case GGFMT_L8 :			return 8;	break;
		case GGFMT_A8L8 :			return 16;	break;
		case GGFMT_A4L4 :			return 8;	break;
	}
	#endif
	return 0;
}

static int GetBitDepthFromFormatDX11(GGFORMAT Format)
{
	#ifdef DX11
	#endif
	return 32;
}

static int GetBitDepthFromFormat(GGFORMAT Format)
{
	#ifdef DX11
		return GetBitDepthFromFormatDX11(Format);
	#else
		return GetBitDepthFromFormatDX9(Format);
	#endif
}

DARKSDK void DB_UpdateEntireWindow(bool bFullUpdate, bool bMovement)
{
	// regular DBP window
	if(bFullUpdate==false)
	{
		ShowWindow(m_hWnd, gWindowVisible);
	}
	else
	{
		if(strlen(gWindowName)>0) SetWindowText(m_hWnd, gWindowName);
		SetWindowLong(m_hWnd, GWL_STYLE, gWindowStyle);
		DWORD dwActualWindowWidth = gWindowSizeX+gWindowExtraX;
		DWORD dwActualWindowHeight = gWindowSizeY+gWindowExtraY;
		SetWindowPos(m_hWnd, HWND_TOP, g_pGlob->dwWindowX, g_pGlob->dwWindowY, dwActualWindowWidth, dwActualWindowHeight, SWP_SHOWWINDOW);
		ShowWindow(m_hWnd, gWindowVisible);
		SetClassLong(m_hWnd, GCL_HICON, (LONG)gWindowIconHandle);
	}

	// Paint after window switch
	if(bMovement)
		InvalidateRect(NULL, NULL, FALSE);
	else
		InvalidateRect(m_hWnd, NULL, FALSE);

	UpdateWindow(m_hWnd);
}

DARKSDK void DB_EnsureWindowRestored(void)
{
	if(gbWindowMode)
	{
		gbWindowMode=false;
		gbWindowBorderActive=true;
		gWindowVisible=SW_SHOWDEFAULT;
		gWindowSizeX = GetSystemMetrics(SM_CXFULLSCREEN);
		gWindowSizeY = GetSystemMetrics(SM_CYFULLSCREEN);
		gWindowStyle = WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU;
		gWindowIconHandle = gOriginalIcon;
		DB_UpdateEntireWindow(true, true);
	}
}

DARKSDK void UpdateWindowSize ( int iWidth, int iHeight )
{
	// updates the window size, remember that in windowed
	// mode the window could be resized at any stage, if this
	// happens we need to inform the setup library

	// we could always detect the size automatically but then
	// it's waste because we would be checking it every frame
	// and this would be inefficient, by doing it this way we
	// call this function when the window has been resized

	// store the new window size
	m_iWindowWidth  = iWidth;
	m_iWindowHeight = iHeight;
}

DARKSDK void GetWindowSize ( int* piWidth, int* piHeight )
{
	// retrieve the size of the window, several other DLL's
	// need to be able to do this, for example when clicking
	// on objects we need to determine the size of the window

	// check for valid pointers
	if ( !piWidth || !piHeight )
		Error1 ( "Invalid pointers passed to GetWindowSize for setup library" );

	// assign the pointers the saved window size
	*piWidth  = m_iWindowWidth;
	*piHeight = m_iWindowHeight;
}

DARKSDK void OverrideHWND ( HWND hWnd )
{
	// use an external window instead of the default
	// check the window handle is valid
	if ( !hWnd ) Error1 ( "Invalid window handle passed to OverrideHWND" );
	m_bOverrideHWND = true;
	m_hWnd          = hWnd;
}

DARKSDK void DisableWindowResize ( void )
{
	// disable window resizing
	m_bResizeWindow = false;
}

DARKSDK void EnableWindowResize ( void )
{
	// enable window resizing
	m_bResizeWindow = true;
}

DARKSDK void AddSwapChain ( HWND hwnd )
{
	// add to the swap chain

	// variable declarations
	GGPRESENT_PARAMETERS	d3dpp;	// setup structure
	GGDISPLAYMODE			mode;	// display mode

	// check the window handle is valid
	if ( !hwnd )
		Error1 ( "Invalid window handle for AddSwapChain" );

	// clear out structures
	memset ( &d3dpp, 0, sizeof ( d3dpp ) );
	memset ( &mode,  0, sizeof ( mode  ) );

	#ifdef DX11
	#else
	// get the current display mode
	m_pDX->GetAdapterDisplayMode ( m_uAdapterChoice, &mode );
	
	// setup some fields
	d3dpp.Windowed         = true;					// windowed mode
	d3dpp.SwapEffect       = D3DSWAPEFFECT_COPY;	// use copy
	d3dpp.BackBufferFormat = mode.Format;			// back buffer format is same as main mode
	d3dpp.hDeviceWindow    = hwnd;					// handle to device window

	// finally add the new swap chain
	HRESULT					hr;		// used for error checking
	if ( FAILED ( hr = m_pD3D->CreateAdditionalSwapChain ( &d3dpp, &m_pSwapChain [ m_iSwapChainCount++ ] ) ) )
	{
		// check the type of error, while DXTrace would do a better job than this
		// we can't use it as it's only provided for debug use
		switch ( hr )
		{
			case D3DERR_INVALIDCALL:
				Error ( "Invalid parameters when creating additional swap chain" );
			break;

			case D3DERR_OUTOFVIDEOMEMORY:
				Error ( "Unable to create additional swap chain - out of video memory" );
			break;
		}
	}
	#endif
}

DARKSDK void StartSwapChain ( int iID )
{
	// start rendering a swap chain
	#ifdef DX11
	#else
	// declare some surfaces
	LPGGSURFACE	pBack    = NULL;	// back buffer
	LPGGSURFACE	pStencil = NULL;	// stencil buffer

	// check that the device is valid
	if ( !m_pD3D )
		return;

	// check the ID is valid
	if ( iID > MAX_SWAP_CHAINS )
		Error ( "Specified invalid swap chain - overrun maximum limit" );

	// check that we could begin a scene
	if ( SUCCEEDED ( m_pD3D->BeginScene ( ) ) )
		m_bDraw = true;

	// get the backbuffer
	m_pSwapChain [ iID ]->GetBackBuffer ( 0, D3DBACKBUFFER_TYPE_MONO, &pBack );

	// now get the stencil buffer
	m_pD3D->GetDepthStencilSurface ( &pStencil );

	// check we got the buffer
	if ( !pBack || !pStencil )
		Error ( "Unable to access back buffer / stencil buffer for swap chain" );

	// now setup the new render target
	m_pD3D->SetRenderTarget ( 0, pBack );
	m_pD3D->SetDepthStencilSurface ( pStencil );

	// due to the way COM works we need to release
	// these interfaces otherwise we would end up 
	// with some resource leaks
	pBack->Release    ( );
	pStencil->Release ( );
	#endif
}

DARKSDK void EndSwapChain ( int iID )
{
	// finish rendering a swap chain
	// check the device is valid
	if ( !m_pD3D )
		return;

	// check we are ok to draw
	if ( m_bDraw )
	{
		#ifdef DX11
		#else
		m_pD3D->EndScene ( );	// finish the scene drawing
		#endif
		m_bDraw = false;		// report that we've finished drawing
	}
}

DARKSDK void UpdateSwapChain ( int iID )
{
	// update the swap chain and draw it's contents on screen

	// check the ID is valid
	if ( iID > MAX_SWAP_CHAINS )
		Error1 ( "Specified invalid swap chain - overrun maximum limit" );

	// check the swap chain is valid
	if ( !m_pSwapChain [ iID ] )
		Error1 ( "Swap chain pointer not setup correctly" );

	// update full screen
	#ifdef DX11
	SwapChainPresent(iID);
	#else
	m_pSwapChain [ iID ]->Present ( NULL, NULL, NULL, NULL, 0 );
	#endif
}

DARKSDK void Begin ( void )
{
	// being a typical rendering session
	g_bValidFPS = true;
	//if ( g_bWindowOverride ) ShowWindow ( g_OldHwnd, SW_HIDE );
	if ( !m_pD3D ) return;

	// now begin scene drawing
	if ( g_bSceneBegun==false )
	{
		g_bSceneBegun = true;
		#ifdef DX11
		#else
		if ( SUCCEEDED ( m_pD3D->BeginScene ( ) ) )
			m_bDraw = true;
		#endif
	}
}

DARKSDK void End ( void )
{
	// end the rendering session
	if ( !m_pD3D ) return;
	if ( m_bDraw && m_pD3D )
	{
		if ( g_bSceneBegun==true ) 
		{
			#ifdef DX11
			#else
			m_pD3D->EndScene ( );
			#endif
		}
		m_bDraw = false;
		g_bSceneBegun = false;
	}
}

DARKSDK HRESULT PresentRect ( RECT* pArea, RECT* pClientrc, float fX, float fY )
{
	// result
	HRESULT hRes = 0;

	// assign src and dest rects
	RECT src = { 0, 0, g_pGlob->iScreenWidth, g_pGlob->iScreenHeight };
	for(DWORD s=0; s<4; s++)
	{
		if(s==0)
		{
			src.bottom=pArea->top;
		}
		if(s==1)
		{
			src.top=pArea->top;
			src.bottom=pArea->bottom;
			src.right=pArea->left;
		}
		if(s==2)
		{
			src.left=pArea->right;
			src.right=pClientrc->right;
		}
		if(s==3)
		{
			src.top=pArea->bottom;
			src.left=0;
			src.bottom=pClientrc->bottom;
		}
		RECT dest = { (int)(src.left*fX), (int)(src.top*fY), (int)(src.right*fX), (int)(src.bottom*fY) };
		#ifdef DX11
		#else
		hRes = m_pD3D->Present ( &src, &dest, NULL, NULL );
		#endif
	}
	return hRes;
}

DARKSDK void DivideAreaByRect ( RECT* pArea, RECT* pSafeBoxes, DWORD* pdwDrawToBoxes, RECT** ppDrawBoxes )
{
	// Determine if area is clear of safe boxes
	bool bAreaIsClearOfSafeBoxes=true;
	for ( DWORD si=0; si<g_pGlob->dwSafeRectMax; si++ )
	{
		// check whether safe box intersects area
		if ( pSafeBoxes[si].right!=0 )
		{
			if( pArea->right > pSafeBoxes[si].left
			&&	pArea->bottom > pSafeBoxes[si].top
			&&	pArea->left < pSafeBoxes[si].right
			&&	pArea->top < pSafeBoxes[si].bottom	)	
			{
				// found safe box cutting into area
				bAreaIsClearOfSafeBoxes=false;

				// Store original rect
				RECT rectOrig = pSafeBoxes[si];

				// divide area into four smaller areas (all sides of safe box)
				RECT src = *pArea;
				for(DWORD s=0; s<4; s++)
				{
					if(s==0)
					{
						src.bottom=rectOrig.top;
					}
					if(s==1)
					{
						src.top=rectOrig.top;
						src.bottom=rectOrig.bottom;
						src.right=rectOrig.left;
					}
					if(s==2)
					{
						src.left=rectOrig.right;
						src.right=pArea->right;
					}
					if(s==3)
					{
						src.top=rectOrig.bottom;
						src.left=pArea->left;
						src.bottom=pArea->bottom;
					}

					// further divide this area by remaining safe boxes
					DivideAreaByRect ( &src, pSafeBoxes, pdwDrawToBoxes, ppDrawBoxes );
				}
			}
		}
	}

	// if area clear of safe boxes, add to draw box array
	if ( bAreaIsClearOfSafeBoxes )
	{
		// check if box is a valid one
		bool bBoxIsQualified=false;
		if(	pArea->right > pArea->left && pArea->bottom > pArea->top
		&&	pArea->left >= 0 && pArea->top >= 0
		&&	pArea->right <= g_pGlob->iScreenWidth
		&&	pArea->bottom <= g_pGlob->iScreenHeight	)
			bBoxIsQualified=true;

		// only add qualified boxes
		if (bBoxIsQualified )
		{
			if ( (*ppDrawBoxes) ) (*ppDrawBoxes) [ (*pdwDrawToBoxes) ] = *pArea;
			*pdwDrawToBoxes = *pdwDrawToBoxes + 1;
		}
	}
}

DARKSDK void CreateDrawBoxes ( DWORD* pdwDrawToBoxes, RECT** ppDrawBoxes )
{
	// Go through safe boxes
	if ( g_pGlob->dwSafeRectMax > 0 )
	{
		// Wipe out any boxes entirely inside other boxes
		for ( DWORD iSR=0; iSR<g_pGlob->dwSafeRectMax; iSR++ )
		{
			for ( DWORD iSR2=0; iSR2<g_pGlob->dwSafeRectMax; iSR2++ )
			{
				if ( iSR != iSR2 )
				{
					if(	g_pGlob->pSafeRects [iSR].left >= g_pGlob->pSafeRects [iSR2].left
					&&	g_pGlob->pSafeRects [iSR].right <= g_pGlob->pSafeRects [iSR2].right
					&&	g_pGlob->pSafeRects [iSR].top >= g_pGlob->pSafeRects [iSR2].top
					&&	g_pGlob->pSafeRects [iSR].bottom <= g_pGlob->pSafeRects [iSR2].bottom )
					{
						g_pGlob->pSafeRects [iSR].right=0;
					}
				}
			}
		}

		// Create a copy of the safe boxes
		RECT* pSafeBoxesCopy = new RECT [ g_pGlob->dwSafeRectMax ];

		// Initial area is entire screen
		RECT area = { 0, 0, g_pGlob->iScreenWidth, g_pGlob->iScreenHeight };

		// Count number of draw boxes
		memcpy ( pSafeBoxesCopy, g_pGlob->pSafeRects, sizeof(RECT) * g_pGlob->dwSafeRectMax );
		DivideAreaByRect ( &area, pSafeBoxesCopy, pdwDrawToBoxes, ppDrawBoxes );

		// Create draw array
		*ppDrawBoxes = new RECT [ *pdwDrawToBoxes ];
		*pdwDrawToBoxes = 0;

		// Create draw boxes
		memcpy ( pSafeBoxesCopy, g_pGlob->pSafeRects, sizeof(RECT) * g_pGlob->dwSafeRectMax );
		DivideAreaByRect ( &area, pSafeBoxesCopy, pdwDrawToBoxes, ppDrawBoxes );

		// Free copy of safe boxes
		SAFE_DELETE(pSafeBoxesCopy);
	}
}

// mike - 070207 - switch on custom present mode
void dbSetCustomPresentMode ( bool bMode )
{
	DisplayLibrary::g_bCustomPresentMode = bMode;
}

// mike - 070207 - add a rectangle region
void dbAddCustomPresentRectangle ( RECT rect )
{
	DisplayLibrary::g_CustomPresentRectangles.push_back ( rect );
}

// mike - 070207 - clear the draw list
void dbClearCustomPresentList ( void )
{
	DisplayLibrary::g_CustomPresentRectangles.clear ( );
}

// mike - 070207 - render all regions
HRESULT dbRenderUsingCustomPresentList ( void )
{
	HRESULT hr = 0;
	for ( int i = 0; i < (int)DisplayLibrary::g_CustomPresentRectangles.size ( ); i++ )
	{
		// dest is primary stretched maybe
		RECT clientrc = { 0,0,0,0 };
		GetClientRect(g_pGlob->hWnd, &clientrc);

		// calculate scaling between src and dest
		float fX = (float)clientrc.right / (float)g_pGlob->iScreenWidth;
		float fY = (float)clientrc.bottom / (float)g_pGlob->iScreenHeight;
		RECT src = DisplayLibrary::g_CustomPresentRectangles [ i ];
		RECT dest = { (int)(src.left*fX), (int)(src.top*fY), (int)(src.right*fX), (int)(src.bottom*fY) };
		#ifdef DX11
		#else
		hr = m_pD3D->Present ( &src, &dest, NULL, NULL );
		#endif
	}
	return hr;
}

int g_iTriggerResize = 0;

HRESULT StandardPresent ( void )
{
	// result
	HRESULT hRes  S_OK;

	// Standard stretched present to visual surface
	#ifdef DX11

	#ifdef VR920SUPPORT
	if( g_StereoEnabled && g_StereoHandle )
	{
		// RENDER HANDLED BY CAMERA (to set and render from viewpoint)
		//if ( g_StereoEyeToggle==LEFT_EYE )
		//	m_pImmediateContext->End(g_pLeftEyeQuery);
		//else
		//	m_pImmediateContext->End(g_pRightEyeQuery);

		// Syncronize the frame to the left eyes frame buffer.
		IWRSyncronizeEye( g_StereoEyeToggle );

		// toggle stereo eyes automatically
		if ( g_StereoEyeToggle==LEFT_EYE )
		{
			g_StereoEyeToggle=RIGHT_EYE;
		//	m_pImmediateContext->Begin(g_pRightEyeQuery);
		}
		else
		{
			g_StereoEyeToggle=LEFT_EYE;
		//	m_pImmediateContext->Begin(g_pLeftEyeQuery);
		}
	}
	else
	{
		// Mono Mode rendering - standard present
		SwapChainPresent(0);
	}
	#else
	{
		// No VR, just a simple swap chain pesent
		SwapChainPresent(0);
	}
	#endif

	if ( g_bWindowOverride && g_dwChildWindowTruePixel && m_pSwapChain[0] )
	{
		// no stretch present equivilant in DX11, so resize backbuffer instead
		// resize swapchain to suit correct child window size
		RECT src = { 0, 0, 0, 0 };
		GetClientRect ( g_pGlob->hWnd, &src );
		if ( src.right != g_pGlob->dwClientRegionWidth || src.bottom != g_pGlob->dwClientRegionHeight )
		{
			m_pImmediateContext->ClearState();
			SAFE_RELEASE ( m_pRenderTargetView );
			SAFE_RELEASE ( g_pBackBuffer );
			SAFE_RELEASE ( m_pDepthStencilView );
			SAFE_RELEASE ( m_pDepthStencilResourceView );
			SAFE_RELEASE ( m_pDepthStencil );
			HRESULT hRes = m_pSwapChain[0]->ResizeBuffers ( 0, src.right, src.bottom, DXGI_FORMAT_UNKNOWN, 0 );
			GetBackBufferAndDepthBuffer();
			GetBackBufferPointers();
			if ( hRes == S_OK )
			{
				g_pGlob->dwClientRegionWidth = src.right;
				g_pGlob->dwClientRegionHeight = src.bottom;
				g_iTriggerResize = 0;
			}
		}
	}
	#else
	if ( g_bWindowOverride && g_dwChildWindowTruePixel )
	{
		RECT src = { 0, 0, 0, 0 };
		GetClientRect ( m_hWnd , &src );
		hRes = m_pD3D->Present ( &src , NULL, NULL, NULL );
	}
	else
		hRes = m_pD3D->Present ( NULL, NULL, NULL, NULL );
	#endif

	// return if device lost during present
	return hRes;
}

DARKSDK void Render ( void )
{
	// result
	HRESULT hRes;

	// copy the buffers and show the contents
	if ( !m_pD3D ) return;

	// dest is primary stretched maybe
	RECT clientrc = { 0,0,0,0 };
	GetClientRect(g_pGlob->hWnd, &clientrc);

	// calculate scaling between src and dest
	float fX = (float)clientrc.right / (float)g_pGlob->iScreenWidth;
	float fY = (float)clientrc.bottom / (float)g_pGlob->iScreenHeight;

	// if array of protected boxes setup (from controls requiring primary surface)
	if ( g_pGlob->dwSafeRectMax>0 )
	{
		// create draw boxes from protected boxes
		DWORD dwDrawToBoxes = 0;
		RECT* pDrawBoxes = NULL;
		CreateDrawBoxes ( &dwDrawToBoxes, &pDrawBoxes );

		// go through boxes
		if ( dwDrawToBoxes > 0 )
		{
			for ( DWORD boxindex=0; boxindex<dwDrawToBoxes; boxindex++ )
			{
				// present each one to copy to primary
				RECT src = pDrawBoxes [ boxindex ];
				RECT dest = { (int)(src.left*fX), (int)(src.top*fY), (int)(src.right*fX), (int)(src.bottom*fY) };
				#ifdef DX11
				#else
				hRes = m_pD3D->Present ( &src, &dest, NULL, NULL );
				#endif
			}
		}

		// Free usages
		SAFE_DELETE(pDrawBoxes);
	}
	else
	{
		// if area is for direct primary-surface drawing (such as a DVD video window)
		if ( g_pGlob->iNoDrawRight!=0 )
		{
			// area as src-scaled rect
			RECT area = { (LONG)g_pGlob->iNoDrawLeft, (LONG)g_pGlob->iNoDrawTop, (LONG)g_pGlob->iNoDrawRight, (LONG)g_pGlob->iNoDrawBottom };

			// Present 
			hRes = PresentRect ( &area, &clientrc, fX, fY );
		}
		else
		{
			// Standard stretched present to visual surface
			hRes = StandardPresent();
		}
	}

	// Catch if present fail, device may be lost
	if(hRes==GGERR_DEVICELOST)
	{
		// Attempt to restore device
		RestoreLostDevice();
	}
}

DARKSDK bool SetDisplayMode ( int iWidth, int iHeight )
{
	// sets the display mode to the specified
	// width and height, default values are provided
	// when this function is called the program will
	// default to setting 16 bit color depth and will
	// also default to full screen mode and attempt
	// to select hardware vertex processing
	if ( !SetDisplayMode ( iWidth, iHeight, 16, FULLSCREEN, HARDWARE ) )
		return false;

	return true;
}

DARKSDK bool SetDisplayMode ( int iWidth, int iHeight, int iDepth )
{
	// sets the display mode to the specified
	// width and height, default values are provided
	// when this function is called the program will
	// default to full screen mode and attempt to
	// select hardware vertex processing, lockable
	// backbuffers are switched off
	if(!SetDisplayMode ( iWidth, iHeight, iDepth, FULLSCREEN, HARDWARE, 0 ) )
		return false;

	return true;
}

DARKSDK bool SetDisplayMode ( int iWidth, int iHeight, int iDepth, int iMode )
{
	// sets the display mode to the specified
	// width, height, depth and mode default values
	// are provided when this function is called 
	// the program will attempt to select hardware
	// vertex processing, lockable backbuffers are
	// switched off
	if ( !SetDisplayMode ( iWidth, iHeight, iDepth, iMode, HARDWARE, 0 ) )
		return false;

	return true;
}

DARKSDK bool SetDisplayMode ( int iWidth, int iHeight, int iDepth, int iMode, int iVertexProcessing )
{
	// sets the display mode
	if ( !SetDisplayMode ( iWidth, iHeight, iDepth, iMode, iVertexProcessing, 0 ) )
		return false;

	return true;
}

DARKSDK bool SetDisplayMode ( int iWidth, int iHeight, int iDepth, int iMode, int iVertexProcessing, int iLockable )
{
	#ifdef DX11
	#else
	int  iTemp;
	bool bValid = false;
	
	// check the width and height
	if ( iWidth <= 0 || iHeight <= 0 )
	{
		Error ( "Invalid display resolution" );
		RunTimeError(RUNTIMEERROR_SCREENSIZEILLEGAL);
		return false;
	}

	// if depth is zero, fill with current windows depth
	if ( iDepth == 0 )
	{
		// get windows natural depth
		GGDISPLAYMODE d3dmode;
		m_pDX->GetAdapterDisplayMode ( 0, &d3dmode );
		iDepth = GetBitDepthFromFormat(d3dmode.Format);
		g_pGlob->iScreenDepth = iDepth;
	}

	// 24bit not supported
	if ( iDepth == 24 )
	{
		Error ( "Invalid bit depth specified" );
		RunTimeError(RUNTIMEERROR_24BITNOTSUPPORTED);
		return false;
	}

	// check the depth
	if ( iDepth != 16 && iDepth != 32 && iDepth != 24 )
	{
		Error ( "Invalid bit depth specified" );
		RunTimeError(RUNTIMEERROR_SCREENDEPTHILLEGAL);
		return false;
	}

	// mode
	if ( iMode < 0 || iMode > 1 )
	{
		Error ( "Invalid display mode specified" );
		RunTimeError(RUNTIMEERROR_SCREENMODEINVALID);
		return false;
	}

	// just check that the mode does exist
	for ( iTemp = 0; iTemp < m_pInfo [ 0 ].iDisplayCount; iTemp++ )
	{
		// find the mode which which matches the selected width and height of the new display settings
		if (	m_pInfo [ 0 ].D3DDisplay [ iTemp ].Width  == (DWORD)iWidth &&
				m_pInfo [ 0 ].D3DDisplay [ iTemp ].Height  == (DWORD)iHeight &&
				GetBitDepthFromFormat(m_pInfo [ 0 ].D3DDisplay [ iTemp ].Format) == iDepth )
		{
			bValid = true;
			break;
		}
	}

	// if non standard size required, submit a valid size and setup clip data
	if ( !bValid )
	{
		// Generate a standard width
		int iStWidth=iWidth, iStHeight=iHeight;
		for ( iTemp = 0; iTemp < m_pInfo [ 0 ].iDisplayCount; iTemp++ )
		{
			if (	m_pInfo [ 0 ].D3DDisplay [ iTemp ].Width  > (DWORD)iStWidth &&
					m_pInfo [ 0 ].D3DDisplay [ iTemp ].Height  > (DWORD)iStHeight &&
					GetBitDepthFromFormat(m_pInfo [ 0 ].D3DDisplay [ iTemp ].Format) == iDepth )
			{
				// Nearest valid display mode
				iStWidth = m_pInfo [ 0 ].D3DDisplay [ iTemp ].Width;
				iStHeight = m_pInfo [ 0 ].D3DDisplay [ iTemp ].Height;
				bValid = true;
				break;
			}
		}

		// Create clip data
		m_iChopWidth = iStWidth - iWidth;
		m_iChopHeight = iStHeight - iHeight;

		// Assign new width and height
		iWidth = iStWidth;
		iHeight = iStHeight;
	}
	else
	{
		// No chopping required
		m_iChopWidth = 0;
		m_iChopHeight = 0;
	}

	// ensure that the mode exists
	if ( !bValid )
	{
		Error ( "Display mode not supported" );
		if(iDepth==16) RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAY16B);
		if(iDepth==24) RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAY24B);
		if(iDepth==32) RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAY32B);
		return false;
	}
	
	// now check that the processing mode is valid
	if ( iVertexProcessing != HARDWARE && iVertexProcessing != SOFTWARE )
	{
		if ( iVertexProcessing < 0 || iVertexProcessing > 2 )
		{
			Error ( "Unknown vertex processing mode" );
			RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAYVB);
			return false;
		}
	}

	// setup the correct vertex processing mode
	if ( iVertexProcessing == 0 )
		iVertexProcessing = HARDWARE;

	if ( iVertexProcessing == 1 )
		iVertexProcessing = SOFTWARE;

	if ( iVertexProcessing == 2 )
		iVertexProcessing = SOFTWARE;

	// last thing to do is check that the lockable flag is valid
	if ( iLockable < 0 || iLockable > 1 )
	{
		Error ( "Invalid lockable flag" );
		RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAYLOCK);
		return false;
	}

	// Delete global property-desc of device
	if ( m_D3DPP )
	{
		delete m_D3DPP;
		m_D3DPP=NULL;
	}
	#endif

	// U69 - 180508 - multimonitor mode doubles the width
	if ( m_iMultimonitorMode==1 )
	{
		// to fill primary (first half) and second monitor (second half)
		iWidth = iWidth * 2;
	}

	// setup values
	m_D3DPP			    = NULL;					// don't use debug mode
	m_iWidth		    = iWidth;				// set width
	m_iHeight		    = iHeight;				// set height
	m_iDepth		    = iDepth;				// set default color depth
	m_iDisplayType	    = iMode;				// windowed or fullscreen
	m_iProcess		    = iVertexProcessing;	// vertex processing mode
	m_bLockable		    = true;					// default to a lockable backbuffer
	m_iBackBufferCount  = 1;					// back buffer count

	// now setup the device
	if ( Setup ( ) )
	{
		// Adjust window size if desktop fullscreen mode in effect
		if(g_pGlob)
		{
			// Get Default Icon
			gOriginalIcon = (HICON)GetClassLong(g_pGlob->hWnd, GCL_HICON);
			gWindowIconHandle=gOriginalIcon;

			// 0=hidden
			if(g_pGlob->dwAppDisplayModeUsing==1)
			{
				// 1=window
				RECT clientrc;
				GetClientRect(g_pGlob->hWnd, &clientrc);
				gWindowSizeX = g_pGlob->iScreenWidth;
				gWindowSizeY = g_pGlob->iScreenHeight;
				if(gWindowExtraXForOverlap==0 && gbFirstInitOfDisplayOnly==true)
				{
					// Determine Extra for Overlapped Window Border
					gWindowExtraXForOverlap = g_pGlob->iScreenWidth-clientrc.right;
					gWindowExtraYForOverlap = g_pGlob->iScreenHeight-clientrc.bottom;
				}

				// U75 - 070909 - AA must have actual correct client size matching screen size, so ensure this here
				DWORD dwWindowStyle = GetWindowLong(g_pGlob->hWnd, GWL_STYLE);
				if ( (dwWindowStyle&WS_CAPTION) && (dwWindowStyle&WS_THICKFRAME) )
				{
					gWindowExtraX = gWindowExtraXForOverlap;
					gWindowExtraY = gWindowExtraYForOverlap;
				}
				else
				{
					gWindowExtraX = 0;
					gWindowExtraY = 0;
				}

				DWORD dwActualWindowWidth = gWindowSizeX+gWindowExtraX;
				DWORD dwActualWindowHeight = gWindowSizeY+gWindowExtraY;
				SetWindowPos ( g_pGlob->hWnd, HWND_TOP, 0, 0, dwActualWindowWidth, dwActualWindowHeight, SWP_NOMOVE | SWP_SHOWWINDOW );
			}
			if(g_pGlob->dwAppDisplayModeUsing==2)
			{
				// 2=taskbar
				RECT rc;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
				gWindowSizeX = rc.right-rc.left;
				gWindowSizeY = rc.bottom-rc.top;
				gWindowExtraXForOverlap=0;
				gWindowExtraYForOverlap=0;
				gWindowExtraX = gWindowExtraXForOverlap;
				gWindowExtraY = gWindowExtraYForOverlap;
				SetWindowPos(g_pGlob->hWnd, HWND_TOP, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOMOVE );
			}
			if(g_pGlob->dwAppDisplayModeUsing==3)
			{
				// 3=exclusive
				gWindowSizeX = iWidth;
				gWindowSizeY = iHeight;
				gWindowExtraXForOverlap=0;
				gWindowExtraYForOverlap=0;
				gWindowExtraX = gWindowExtraXForOverlap;
				gWindowExtraY = gWindowExtraYForOverlap;
				SetWindowPos(g_pGlob->hWnd, HWND_TOP, 0, 0, iWidth, iHeight, SWP_NOMOVE );
			}
			if(g_pGlob->dwAppDisplayModeUsing==0
			|| g_pGlob->dwAppDisplayModeUsing==4)
			{
				// 0=hidden (added 101004 so hidden window can become mode4)
				// 4=notaskbar
				DWORD dwWidth=GetSystemMetrics(SM_CXSCREEN);
				DWORD dwHeight=GetSystemMetrics(SM_CYSCREEN);
				gWindowSizeX = dwWidth;
				gWindowSizeY = dwHeight;
				gWindowExtraXForOverlap=0;
				gWindowExtraYForOverlap=0;
				gWindowExtraX = gWindowExtraXForOverlap;
				gWindowExtraY = gWindowExtraYForOverlap;
				// leefix - 200906 - u63 - removed to not interfere with parallel DBP related tasks
				if ( g_pGlob->dwAppDisplayModeUsing==4 )
				{
					// non-hidden windows still must set this though
					SetWindowPos(g_pGlob->hWnd, HWND_TOP, 0, 0, dwWidth, dwHeight, SWP_NOMOVE);
				}
			}

			// Set only when CreateWindow first creates window (used to calculate extra window borders)
			gbFirstInitOfDisplayOnly=false;
		}

		// in case drawing right away, open scene
		Begin();

		// display mode successful
		return true;
	}
	else
	{
		Error1 ( "Unable to setup 3D device" );
	}

	// Runs off the end as unknown direct x, else runtime picked up along the way
	return false;
}

DARKSDK bool SetDisplayDebugMode ( void )
{
	Error1 ( "SetDisplayDebug mode disabled" );
	return true;
}

DARKSDK void RestoreDisplayMode  ( void )
{
	// Restore display mode (called after a set display mode when releasing)
	#ifdef DX11
	#else
	if(m_D3DPP && m_pD3D)
	{
		if(m_D3DPP->Windowed==FALSE)
		{
			m_D3DPP->Windowed = TRUE;
			ReleaseBackBufferPointers();
			m_pD3D->Reset ( m_D3DPP );
			GetBackBufferPointers();
		}
	}
	#endif
}

BOOL Is_Win_Vista_Or_Later () 
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   int op=VER_GREATER_EQUAL;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 6;
   osvi.dwMinorVersion = 0;
   osvi.wServicePackMajor = 0;
   osvi.wServicePackMinor = 0;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, op );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi, 
      VER_MAJORVERSION | VER_MINORVERSION | 
      VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
      dwlConditionMask);
}

DARKSDK bool GetBackBufferAndDepthBuffer ( void )
{
	#ifdef DX11
	// Create backbuffer from swap chain
    HRESULT hr = m_pSwapChain[0]->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&g_pBackBuffer );
    if( FAILED( hr ) )
	{
		//Error( "Failed to GetBuffer\n" );
        return false;
	}

	// Create render target view from backbuffer
    hr = m_pD3D->CreateRenderTargetView( g_pBackBuffer, NULL, &m_pRenderTargetView );
    if( FAILED( hr ) )
    {
		Error1 ( "Failed to CreateRenderTargetView\n" );
        return false;
	}

	// Get render target size so can match with depth buffer size
	D3D11_TEXTURE2D_DESC ddsd;
	g_pBackBuffer->GetDesc ( &ddsd );

	// Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = ddsd.Width;
    descDepth.Height = ddsd.Height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_R32_TYPELESS;//DXGI_FORMAT_R24G8_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;    
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_pD3D->CreateTexture2D( &descDepth, NULL, &m_pDepthStencil );
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to CreateTexture2D\n" );
        return false;
	}

	// Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pD3D->CreateDepthStencilView( m_pDepthStencil, &descDSV, &m_pDepthStencilView );
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to CreateDepthStencilView\n" );
        return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;//DXGI_FORMAT_R24_UNORM_X8_TYPELESS; (DXGI_FORMAT_R32_FLOAT)
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = m_pD3D->CreateShaderResourceView ( m_pDepthStencil, &shaderResourceViewDesc, &m_pDepthStencilResourceView );

	// success
	return true;
	#endif
}

#define DEPTH_BIAS_D32_FLOAT(d) (d/(1/pow(2,23)))

DARKSDK bool SetupDX11 ( void )
{
	#ifdef DX11

	// use default adapter, unless force another one
	m_uAdapterChoice = GGADAPTER_DEFAULT;
	if ( m_iForceAdapterOrdinal>0 )
	{
		m_uAdapterChoice = m_iForceAdapterOrdinal;
	}
    IDXGIAdapter* pAdapter = NULL;
	strcpy ( m_pAdapterName, "Default Adapter" );
	D3D_DRIVER_TYPE adapterType = D3D_DRIVER_TYPE_HARDWARE;
    std::vector <IDXGIAdapter*> vAdapters; 
    IDXGIFactory* pFactory = NULL; 
    //if(SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory) ,(void**)&pFactory))) GGVR needs this!
    if(SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory) ,(void**)&pFactory)))
    {
		// special mode to search for a non-Intel GPU adapter (typically a dedicated higher powered one)
		if ( m_uAdapterChoice == 99 )
		{
			m_uAdapterChoice = 0; // in any event, use default adapter if cannot find a better adapter
			for ( int iAdapterIndex = 0; iAdapterIndex < 10; iAdapterIndex++ )
			{
				if ( pFactory->EnumAdapters(iAdapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND )
				{
					DXGI_ADAPTER_DESC adapterDesc;
					pAdapter->GetDesc(&adapterDesc);
					memset ( m_pAdapterName, 0, sizeof ( m_pAdapterName ) );
					const int size = ::WideCharToMultiByte( CP_UTF8, 0, adapterDesc.Description, -1, NULL, 0, 0, NULL );
					::WideCharToMultiByte( CP_UTF8, 0, adapterDesc.Description, -1, m_pAdapterName, size, 0, NULL );
					strlwr ( m_pAdapterName );
					if ( strstr ( m_pAdapterName, "intel" ) != NULL )
					{
						// this adapter is likely an integrated Intel processor, skip this one
					}
					else
					{
						// a found non-Intel adapter
						m_uAdapterChoice = iAdapterIndex;
						break;
					}
				}
			}
		}
		if ( pFactory->EnumAdapters(m_uAdapterChoice, &pAdapter) != DXGI_ERROR_NOT_FOUND )
		{
			DXGI_ADAPTER_DESC adapterDesc;
			pAdapter->GetDesc(&adapterDesc);
			memset ( m_pAdapterName, 0, sizeof ( m_pAdapterName ) );
			const int size = ::WideCharToMultiByte( CP_UTF8, 0, adapterDesc.Description, -1, NULL, 0, 0, NULL );
			::WideCharToMultiByte( CP_UTF8, 0, adapterDesc.Description, -1, m_pAdapterName, size, 0, NULL );
			adapterType = D3D_DRIVER_TYPE_UNKNOWN;
		}
		else
		{
			// cannot get adapter
			strcpy ( m_pAdapterName, "Invalid Adapter Chosen" );
			pAdapter = NULL;
		}
        pFactory->Release();
	}

	int _nTargetFrameRate = 0;
	bool bWindowed = true;
	int numerator = 0;
	int denominator = 0;
	D3D_FEATURE_LEVEL featureLevelsD3D11ONLY[] =
    {
        D3D_FEATURE_LEVEL_11_1,
	};
	D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
	};
	g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory ( &sd, sizeof( sd ) );
    sd.BufferCount							= 1;
    sd.BufferDesc.Width						= m_iWidth;
    sd.BufferDesc.Height					= m_iHeight;
    sd.BufferDesc.Format					= GGFMT_A8R8G8B8;
	if (_nTargetFrameRate <= 0) 
	{
		sd.BufferDesc.RefreshRate.Numerator		= 0;
		sd.BufferDesc.RefreshRate.Denominator	= 1;
		sd.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
	}
	else
	{
		sd.BufferDesc.RefreshRate.Numerator		= numerator;
		sd.BufferDesc.RefreshRate.Denominator	= denominator;
		sd.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
	}
	if ( bWindowed )
		sd.Flags = 0 ;
	else
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow							= m_hWnd;
	sd.SampleDesc.Count						= 1;
    sd.SampleDesc.Quality					= 0;
    sd.Windowed								= bWindowed;
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	HRESULT hr;
	m_pSwapChain[0] = NULL;
	if ( m_iForceAdapterD3D11ONLY == 1 )
	{
		hr = D3D11CreateDeviceAndSwapChain(	pAdapter, adapterType, NULL, creationFlags, featureLevelsD3D11ONLY, ARRAYSIZE(featureLevelsD3D11ONLY),
												D3D11_SDK_VERSION, &sd, &m_pSwapChain[0], &m_pD3D, 
												&g_featureLevel, &m_pImmediateContext );
	}
	else
	{
		hr = D3D11CreateDeviceAndSwapChain(	pAdapter, adapterType, NULL, creationFlags, featureLevels, ARRAYSIZE(featureLevels),
												D3D11_SDK_VERSION, &sd, &m_pSwapChain[0], &m_pD3D, 
												&g_featureLevel, &m_pImmediateContext );
	}
    if( FAILED( hr ) )
	{
		if ( hr == D3D11_ERROR_FILE_NOT_FOUND ) Error1 ( "D3D11CreateDeviceAndSwapChain = D3D11_ERROR_FILE_NOT_FOUND\n" );
		else if ( hr == D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS ) Error1 ( "D3D11CreateDeviceAndSwapChain = D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS\n" );
		else if ( hr == D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS ) Error1 ( "D3D11CreateDeviceAndSwapChain = D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS\n" );
		else if ( hr == D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD ) Error1 ( "D3D11CreateDeviceAndSwapChain = D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD\n" );
		else if ( hr == D3DERR_WASSTILLDRAWING ) Error1 ( "D3D11CreateDeviceAndSwapChain = D3DERR_WASSTILLDRAWING\n" );
		else if ( hr == D3DERR_INVALIDCALL ) Error1 ( "D3D11CreateDeviceAndSwapChain = D3DERR_INVALIDCALL\n" );
		else if ( hr == E_FAIL ) Error1 ( "D3D11CreateDeviceAndSwapChain = E_FAIL\n" );
		else if ( hr == E_INVALIDARG ) Error1 ( "D3D11CreateDeviceAndSwapChain = E_INVALIDARG\n" );
		else if ( hr == E_OUTOFMEMORY ) Error1 ( "D3D11CreateDeviceAndSwapChain = E_OUTOFMEMORY\n" );
		else if ( hr == S_FALSE ) Error1 ( "D3D11CreateDeviceAndSwapChain = S_FALSE\n" );
		else
		{
			char szOut [ 256 ] = "";
			sprintf ( szOut, "Cannot initialize DirectX 11" );
			Error1 ( szOut );
		}
		Error1 ( "Failed to D3D11CreateDeviceAndSwapChain\n" );
		return false;
	}

	// Set Full screen state (or not)
	if ( m_pSwapChain[0] ) 
	{
		m_pSwapChain[0]->SetFullscreenState ( !bWindowed, NULL );
		// Create back buffer and depth buffer (with views) from swapchain
		if ( GetBackBufferAndDepthBuffer() == false )
			return false;
	}

	// Create the depth stencil STATE for 3D rendering
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; // when did this, terrain in cubemap stopped working (suggests no depth values being used!) // PE: works now.
	depthStencilDesc.StencilEnable = false;//true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_pD3D->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState);
	if( FAILED( hr ) )
	{
		Error1 ( "Failed to CreateDepthStencilState\n" );
		return false;
	}

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = false;//true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_pD3D->CreateDepthStencilState(&depthStencilDesc, &m_pDepthNoWriteStencilState);
	if( FAILED( hr ) )
	{
		Error1 ( "Failed to CreateDepthStencilState\n" );
		return false;
	}

	// Create the depth stencil STATE for 2D rendering
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = false;//true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_pD3D->CreateDepthStencilState(&depthDisabledStencilDesc, &m_pDepthDisabledStencilState);

	// Setup the raster description which will determine how and what polygons will be drawn
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	hr = m_pD3D->CreateRasterizerState(&rasterDesc, &m_pRasterState);
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to m_pRasterState\n" );
        return false;
	}
	rasterDesc.CullMode = D3D11_CULL_NONE;
	hr = m_pD3D->CreateRasterizerState(&rasterDesc, &m_pRasterStateNoCull);
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to m_pRasterStateNoCull\n" );
        return false;
	}	
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = (int)DEPTH_BIAS_D32_FLOAT(-0.00001);//-100.0f;//-0.6f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;//-0.0000005f;
	hr = m_pD3D->CreateRasterizerState(&rasterDesc, &m_pRasterStateDepthBias);
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to m_pRasterStateDepthBias\n" );
        return false;
	}	

	// Setup the viewport for rendering
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)m_iWidth;
	viewport.Height =(float)m_iHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	//m_pImmediateContext->RSSetViewports(1, &viewport);
	SetupSetViewport ( 0, &viewport, NULL );

	// Create blend state
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_pD3D->CreateBlendState(&blendDesc, &m_pBlendStateAlpha);
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to m_pBlendStateAlpha\n" );
        return false;
	}
	blendDesc.RenderTarget[0].BlendEnable = false;
	hr = m_pD3D->CreateBlendState(&blendDesc, &m_pBlendStateNoAlpha);
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to m_pBlendStateNoAlpha\n" );
        return false;
	}
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
	hr = m_pD3D->CreateBlendState(&blendDesc, &m_pBlendStateShadowBlend);
    if( FAILED( hr ) )
	{
		Error1 ( "Failed to m_pBlendStateShadowBlend\n" );
        return false;
	}
	
	#endif
	return true;
}

DARKSDK void SetupSetViewport ( int iCameraID, D3D11_VIEWPORT* originalvp, LPGGSURFACE pSurface )
{
	D3D11_VIEWPORT vp = *originalvp;
	if ( pSurface ) 
	{
		D3D11_TEXTURE2D_DESC ddsd;
		pSurface->GetDesc ( &ddsd );
		vp.Width=ddsd.Width;
		vp.Height=ddsd.Height;
	}
	if ( g_VR920AdapterAvailable == true && iCameraID == 3 && g_VR920RenderStereoNow == true )
	{
		// VR side by side rendering (left side default)
		vp.Width /= 2;
		if ( g_StereoEyeToggle == RIGHT_EYE )
		{
			// right side shift
			vp.TopLeftX += vp.Width;
		}
	}
    m_pImmediateContext->RSSetViewports( 1, &vp );
}

DARKSDK bool SetupDX9 ( void )
{
	#ifndef DX11
	// flag for backbuffer
	m_dwFlags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// setup the display mode using the preset values, this function is only called internally
	if ( !m_pDX )
	{
		Error ( "Invalid D3D pointer for Setup ( )" );
		RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAYNODX);
		return false;
	}

	// check for other adapters (perhaps use later)
	UINT uNumberOfAdapters = m_pDX->GetAdapterCount();
	for ( UINT uI=0; uI<uNumberOfAdapters; uI++ )
	{
		D3DADAPTER_IDENTIFIER9 identifier;
		HRESULT hRes = m_pDX->GetAdapterIdentifier ( uI, 0, &identifier );
	}
	m_uAdapterChoice = GGADAPTER_DEFAULT;

	// Reduce overhead on system by discarding buffer after use (must refresh each SYNC)
	if ( m_iMultisamplingFactor > 0 )
	{
		// multisampling requires a DISCARDable backbuffer
		m_SwapMode = D3DSWAPEFFECT_DISCARD;

		// Cannot lock backbuffer for multisampled devices so switch D3DPRESENTFLAG_LOCKABLE_BACKBUFFER ''off''
		m_dwFlags = 0;
	}
	else
	{
		// default for non 3D/sprite/grabbing backbuffer
		m_SwapMode = D3DSWAPEFFECT_COPY;
	}

	// test for flip approach
	if ( g_bWindowOverride )
		m_SwapMode = D3DSWAPEFFECT_COPY;
	else
		m_SwapMode = D3DSWAPEFFECT_FLIP;
	
	// this will get switched off if we can't find a stencil buffer
	m_bZBuffer = true;

	// Free before setting properties
	if ( m_D3DPP )
	{
		delete m_D3DPP;
		m_D3DPP=NULL;
	}

	if ( !m_D3DPP ) 
	{
		m_D3DPP = new GGPRESENT_PARAMETERS;
		if ( !m_D3DPP ) Error ( "Failed to allocate display mode parameters" );
		memset ( m_D3DPP, 0, sizeof ( GGPRESENT_PARAMETERS ) );
		m_D3DPP->SwapEffect				     = m_SwapMode;
		m_D3DPP->BackBufferCount			 = m_iBackBufferCount;
		m_D3DPP->EnableAutoDepthStencil		 = m_bZBuffer;
		m_D3DPP->FullScreen_RefreshRateInHz  = D3DPRESENT_RATE_DEFAULT;
		m_D3DPP->Flags                       = m_dwFlags;

		// can have different backbuffer size if flagged
		if ( m_iModBackbufferWidth != 0 )
		{
			// use this instead of '799 m_bUseDeskTopBB resolution trick'
			m_D3DPP->BackBufferWidth             = m_iModBackbufferWidth;
			m_D3DPP->BackBufferHeight            = m_iModBackbufferHeight;
		}
		else
		{
			// default is backbuffer matches resolution
			m_D3DPP->BackBufferWidth             = m_iWidth;
			m_D3DPP->BackBufferHeight            = m_iHeight;
		}
		
		// if VSYNC flagged, allow antialiasing of device up-to device max.
		if ( m_iMultisamplingFactor > 0 )
		{
			// allow anti-aliasing of device (if supported)
			DWORD dwQualityLevels = 0;
			m_D3DPP->MultiSampleType = (D3DMULTISAMPLE_TYPE)m_iMultisamplingFactor;

			// check if multisampling is available for device
			if( SUCCEEDED(m_pDX->CheckDeviceMultiSampleType(	m_uAdapterChoice, D3DDEVTYPE_HAL, m_WindowsD3DMODE.Format,
																TRUE, m_D3DPP->MultiSampleType, &dwQualityLevels ) ) )
			{
				// set quality level of multisampling we found
				m_D3DPP->MultiSampleQuality			= dwQualityLevels-1;
			}
			else
			{
				// fall back to no multisampling
				m_D3DPP->MultiSampleType			= D3DMULTISAMPLE_NONE;
				m_D3DPP->MultiSampleQuality			= 0;
			}
		}
		else
		{
			// default behaviour
			m_D3DPP->MultiSampleType			= D3DMULTISAMPLE_NONE;
			m_D3DPP->MultiSampleQuality			= 0;
		}
		
		if ( m_iDisplayType == FULLSCREEN )
		{
			GetValidBackBufferFormat ( );
			GetStencilDepth ( );
			m_D3DPP->AutoDepthStencilFormat		= m_StencilDepth;
			m_D3DPP->Windowed					= false;
			m_D3DPP->BackBufferFormat			= m_Depth;
			m_D3DPP->FullScreen_RefreshRateInHz = 0;
			m_D3DPP->BackBufferFormat           = m_Depth;
		}
		else
		{
			m_Depth									 = m_WindowsD3DMODE.Format;
			m_D3DPP->Windowed                        = true;
			m_D3DPP->BackBufferFormat                = m_WindowsD3DMODE.Format;
			m_D3DPP->PresentationInterval			 = D3DPRESENT_INTERVAL_DEFAULT;
			GetStencilDepth ( );
			m_D3DPP->AutoDepthStencilFormat = m_StencilDepth;
			if ( m_StencilDepth == GGFMT_UNKNOWN )
				Error ( "Failed to find valid stencil buffer" );
		}

		// unified VSYNC handler
		if ( m_bVSync )
		{
			// CAP TO MONITOR REFRESH RATE - NO TEARING
			m_D3DPP->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			if ( m_iVSyncInterval==2 ) m_D3DPP->PresentationInterval = D3DPRESENT_INTERVAL_TWO;
			if ( m_iVSyncInterval==3 ) m_D3DPP->PresentationInterval = D3DPRESENT_INTERVAL_THREE;
		}
		else
		{
			// FAST AS YOU CAN - HAS HORIZ TEARING
			m_D3DPP->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}
	}

	// MAIN CREATE FUNCTION
	if ( CreateDX9 ( m_hWnd, m_D3DPP )!=1 )
	{
		// Runtime errors generated witin Create function
		return false;
	}

	// Fail if no device created
	if(m_pD3D==NULL)
	{
		RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAYINVALID);
		return false;
	}
	#endif
	return true;
}

DARKSDK bool Setup ( void )
{
	#ifdef DX11
	return SetupDX11();
	#else
	return SetupDX9();
	#endif
}

DARKSDK GGFORMAT GetDepthFormatFromDisplaySetting ( int Width, int Height, int Depth )
{
	GGFORMAT m_Depth = GGFMT_A8R8G8B8;
	#ifdef DX11
	#else
	for ( int iTemp = 0; iTemp < m_pInfo [ 0 ].iDisplayCount; iTemp++ )
	{
		if	(	m_pInfo [ 0 ].D3DDisplay [ iTemp ].Width  == (DWORD)Width &&
				m_pInfo [ 0 ].D3DDisplay [ iTemp ].Height == (DWORD)Height )
		{
			if ( Depth == GetBitDepthFromFormat ( m_pInfo [ 0 ].D3DDisplay [ iTemp ].Format ) )
			{
				m_Depth = m_pInfo [ 0 ].D3DDisplay [ iTemp ].Format;
				continue;
			}
		}
	}
	#endif
	return m_Depth;
}

DARKSDK void GetValidBackBufferFormat ( )
{
	// find a suitable format for the backbuffer, note
	// that earlier on we stored a list of all the supported
	// display modes, now we're simply running through each
	// one to find a suitable match

	// Get available depth format from settings
	m_Depth = GetDepthFormatFromDisplaySetting ( m_iWidth, m_iHeight, m_iDepth );
}

DARKSDK void GetStencilDepth ( void )
{
	// create the list in order of precedence
	#ifdef DX11
	#else
	GGFORMAT	list [ ] =
							{
								GGFMT_D24S8, //GeForce4 top choice
								GGFMT_R8G8B8,
								GGFMT_A8R8G8B8,
								GGFMT_X8R8G8B8,
								GGFMT_R5G6B5,
								GGFMT_X1R5G5B5,
								GGFMT_A1R5G5B5,
								GGFMT_A4R4G4B4,
								GGFMT_R3G3B2,
								GGFMT_A8,
								GGFMT_A8R3G3B2,
								GGFMT_X4R4G4B4,
								GGFMT_A8P8,
								GGFMT_P8,
								GGFMT_L8,
								GGFMT_A8L8,
								GGFMT_A4L4,
								GGFMT_V8U8,
								GGFMT_L6V5U5,
								GGFMT_X8L8V8U8,
								GGFMT_Q8W8V8U8,
								GGFMT_V16U16,
								GGFMT_D16_LOCKABLE,
								GGFMT_D32,
								GGFMT_D15S1,
								GGFMT_D16,
								GGFMT_D24X8,
								GGFMT_D24X4S4,
								GGFMT_D24FS8,
								GGFMT_D32F_LOCKABLE,
								GGFMT_D32_LOCKABLE,
								GGFMT_S8_LOCKABLE
							};

	for ( int iTemp = 0; iTemp < 32; iTemp++ )
	{
		// Verify that the depth format exists first
		if ( SUCCEEDED ( m_pDX->CheckDeviceFormat( m_uAdapterChoice,
													D3DDEVTYPE_HAL,
													m_Depth,
													D3DUSAGE_DEPTHSTENCIL,
													D3DRTYPE_SURFACE,
													list [ iTemp ]						) ) )
		{
			if ( SUCCEEDED ( m_pDX->CheckDepthStencilMatch	(	m_uAdapterChoice,
																D3DDEVTYPE_HAL,
																m_Depth,
																m_Depth,
																list [ iTemp ]				) ) )
			{
				m_StencilDepth = list [ iTemp ];
				break;
			}
		}
	}
	#endif
}

DARKSDK int CreateDX9 ( HWND hWnd, GGPRESENT_PARAMETERS* d3dpp )
{
	#ifndef DX11
	// 190315 - trace display start-up issues
	char pDisplayErrTrace[2048];
	strcpy ( pDisplayErrTrace, "" );

	HRESULT			hr;
	GGDISPLAYMODE	d3dmode;
	memset ( &d3dmode, 0, sizeof ( d3dmode  ) );
	m_pDX->GetAdapterDisplayMode ( 0, &d3dmode );
	wsprintf ( pDisplayErrTrace, "%sW%dH%dD%dR%d", pDisplayErrTrace, d3dmode.Width, d3dmode.Height, d3dmode.Format, d3dmode.RefreshRate );

	// End scene before any release (just in case)
	End();
	SAFE_RELEASE ( m_pD3D );
	
	// Assign new window handle to D3DPP
	d3dpp->hDeviceWindow = hWnd;

	// can setup app for viewing in PerfHUD
	D3DDEVTYPE pDevType = D3DDEVTYPE_HAL;
	if ( m_bNVPERFHUD==true )
	{
		// U74BETA9 - 010709 - can switch on PERFHUD for next SET DISPLAY MODE call
		m_uAdapterChoice = m_pDX->GetAdapterCount()-1;
		pDevType = D3DDEVTYPE_REF;
	}
	else
	{
		if ( m_iForceAdapterOrdinal>0 )
		{
			m_uAdapterChoice = m_iForceAdapterOrdinal;
		}
	}
	if ( pDevType==D3DDEVTYPE_HAL ) wsprintf ( pDisplayErrTrace, "%s HAL", pDisplayErrTrace );
	if ( pDevType==D3DDEVTYPE_REF ) wsprintf ( pDisplayErrTrace, "%s REF", pDisplayErrTrace );

	// no depth format detected, default to at GGFMT_D24S8 (standard)
	if ( d3dpp->AutoDepthStencilFormat==0 )
	{
		d3dpp->AutoDepthStencilFormat = GGFMT_D24X8;
		wsprintf ( pDisplayErrTrace, "%sGGFMT_D24X8_ADDED", pDisplayErrTrace );
	}

	// create device
	wsprintf ( pDisplayErrTrace, "%s A=%d P=%d D£DPP=%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", pDisplayErrTrace, m_uAdapterChoice, m_iProcess, d3dpp->BackBufferCount, d3dpp->BackBufferWidth, d3dpp->BackBufferHeight, d3dpp->AutoDepthStencilFormat, d3dpp->BackBufferFormat, d3dpp->EnableAutoDepthStencil, d3dpp->Flags, d3dpp->FullScreen_RefreshRateInHz, d3dpp->hDeviceWindow, d3dpp->MultiSampleQuality, d3dpp->MultiSampleType, d3dpp->PresentationInterval, d3dpp->SwapEffect, d3dpp->Windowed );
	if(g_pGlob) g_pGlob->iSoftwareVP = 0;
	if ( FAILED ( hr = m_pDX->CreateDevice (	m_uAdapterChoice,						// use default adapter
												pDevType,								// hardware mode
												hWnd,									// handle to window
												m_iProcess,
												d3dpp,									// display info
												&m_pD3D							// pointer to device
												) ) )
	{
		// try again to create device (with software processing)
		if ( FAILED ( hr = m_pDX->CreateDevice
										(
											m_uAdapterChoice,						// use default adapter
											D3DDEVTYPE_HAL,							// hardware mode
											hWnd,									// handle to window
											//D3DCREATE_SOFTWARE_VERTEXPROCESSING,	// software processing //040414 - oops!!
											D3DCREATE_HARDWARE_VERTEXPROCESSING,	// software processing
											d3dpp,									// display info
											&m_pD3D							// pointer to device
										) ) )
		{
			if ( hr==D3DERR_INVALIDCALL ) hr=1;
			if ( hr==D3DERR_NOTAVAILABLE  ) hr=2;
			if ( hr==D3DERR_OUTOFVIDEOMEMORY  ) hr=3;
			wsprintf ( pDisplayErrTrace, "%s HR=%d", pDisplayErrTrace, hr );
			MessageBox ( NULL, pDisplayErrTrace, "Display Mode Error", MB_OK | MB_TOPMOST );

			Error ( "Unable to create device" );
			if(hr==D3DERR_INVALIDCALL) RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAYINVALID);
			if(hr==D3DERR_NOTAVAILABLE) RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAYNOTAVAIL);
			if(hr==D3DERR_OUTOFVIDEOMEMORY) RunTimeError(RUNTIMEERROR_NOTSUPPORTDISPLAYNOVID);
			return 0;
		}
		else
		{
			// Device will use software VP
			if(g_pGlob) g_pGlob->iSoftwareVP = 1;
		}
	}

	// U74 - 280609 - VR920 Support
	#ifdef VR920SUPPORT

	// Use stereo switching technique (attempt to)
	// g_VR920StereoMethod is set in SET DISPLAY MODE extended command
	g_StereoEnabled = false;
	if ( g_VR920StereoMethod==true )
	{
		// Determine if the VR920 is plugged into a Video Adapter.
		bool bPrimaryInterface = false;
		if( IWRIsVR920Connected( &bPrimaryInterface ) != IWR_NOT_CONNECTED )
		{
			// VR hardware/driver found
			g_VR920AdapterAvailable = true;
			g_StereoEnabled	= g_VR920StereoMethod;

			// Create query interfaces to sync well with GPU
			m_pD3D->CreateQuery(D3DQUERYTYPE_EVENT, &g_pLeftEyeQuery);
			m_pD3D->CreateQuery(D3DQUERYTYPE_EVENT, &g_pRightEyeQuery);
			memset ( &g_d3dcaps, 0, sizeof ( g_d3dcaps ) );
			m_pD3D->GetDeviceCaps ( &g_d3dcaps );

			// Open the VR920's tracker driver.
			HRESULT	iwr_status = IWRLoadDll();
			if( iwr_status != IWR_OK )
				g_VR920AdapterAvailable = false;

			// Open a handle to the VR920's stereo driver.
			g_StereoHandle = IWRSTEREO_Open();
			if( g_StereoHandle == INVALID_HANDLE_VALUE )
				g_VR920AdapterAvailable = false;

			// Select MONO or STEREO
			if( g_StereoEnabled ) 
				IWRSTEREO_SetStereo( g_StereoHandle, IWR_STEREO_MODE );
			else 
				IWRSTEREO_SetStereo( g_StereoHandle, IWR_MONO_MODE );
		}
		else
		{
			MessageBox ( NULL, "VR920 does not appear to be on any accessible adapters", "VR Error", MB_OK );
			g_VR920StereoMethod=false;
			g_StereoEnabled	= g_VR920StereoMethod;
		}
	}
	#endif

	// HWND can be overwritten
	if ( g_bWindowOverride ) g_pGlob->hWnd = g_OldHwnd;

	// setup render states for initial device
	m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER, GGTEXF_LINEAR );	// mip mapping
	m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER, GGTEXF_LINEAR );	// mip mapping
	m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER, GGTEXF_LINEAR );	// mip mapping
	m_pD3D->SetRenderState ( D3DRS_DITHERENABLE,   TRUE             );		// enable dither
	m_pD3D->SetRenderState ( D3DRS_ZENABLE,        TRUE             );		// enable z buffer
	m_pD3D->SetRenderState ( D3DRS_SHADEMODE,      D3DSHADE_GOURAUD );		// set shade mode to gourad shading
	m_pD3D->SetRenderState ( D3DRS_SPECULARENABLE, FALSE            );		// turn off specular highlights
	m_pD3D->SetRenderState ( D3DRS_LIGHTING,       TRUE             );		// turn lighting on
	m_pD3D->SetRenderState ( D3DRS_CULLMODE,       D3DCULL_NONE     );		// set cull mode to none
	m_pD3D->SetRenderState ( D3DRS_AMBIENT, GGCOLOR_ARGB ( 255, 50, 50, 50 ) );		// set ambient light on

	// CLEAR SCREEN STRAIGHT AWAY SO NO ARTIFACTS
	SETUPClear ( 0, 0, 0 );
	#endif

	// success
	return 1;
}

DARKSDK void SETUPClear ( int iR, int iG, int iB )
{
	#ifdef DX11
	if ( g_pGlob->pCurrentRenderView )
	{
		// clear the screen to the specified colour
		float ClearColor[4];
		ClearColor[0] = iR/255.0f;
		ClearColor[1] = iG/255.0f;
		ClearColor[2] = iB/255.0f;
		ClearColor[3] = 1.0f;
		if(m_bZBuffer==false)
		{
			// clear the full screen
			m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentRenderView, ClearColor);
		}
		else
		{
			// clear the full screen and the zbuffer
			m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentRenderView, ClearColor);
			m_pImmediateContext->ClearDepthStencilView(g_pGlob->pCurrentDepthView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0 );
		}
	}
	#else
	// clear the screen to the specified colour
	if(m_bZBuffer==false)
	{
		// clear the full screen
		m_pD3D->Clear ( 0, NULL, D3DCLEAR_TARGET, GGCOLOR_XRGB ( iR, iG, iB ), 1.0f, 0 );
	}
	else
	{
		// clear the full screen and the zbuffer
		m_pD3D->Clear ( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GGCOLOR_XRGB ( iR, iG, iB ), 1.0f, 0 );
	}
	#endif
}

DARKSDK void GetGamma ( int* piR, int* piG, int* piB )
{
	// retrieve the current gamma settings

	// check all of the pointers are valid
	if ( !piR || !piG || !piB )
		Error1 ( "Invalid pointers passed to GetGamma" );

	// now assign the gamma values
	*piR = m_iGammaRed;			// copy red
	*piG = m_iGammaGreen;		// copy green
	*piB = m_iGammaBlue;		// copy blue
}

DARKSDK bool CheckDisplayMode ( int iWidth, int iHeight )
{
	#ifdef DX11
	#else
	int iTemp;
	for ( iTemp = 0; iTemp < m_pInfo [ 0 ].iDisplayCount; iTemp++ )
	{
		if ( m_pInfo [ 0 ].D3DDisplay [ iTemp ].Width == (DWORD)iWidth && m_pInfo [ 0 ].D3DDisplay [ iTemp ].Height == (DWORD)iHeight )
			return true;
	}
	#endif
	return false;
}

DARKSDK int CheckDisplayModeANTIALIAS ( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode )
{
	// extra check if VSYNC supported, and if MULTISAMPLING supported
	// no check vsync - assume all devices support D3DPRESENT_INTERVAL_ONE and D3DPRESENT_INTERVAL_IMMEDIATE
	// no multimonitor check - assume double wide resolution (typical S3D monitor)
	// check multisampling
	if ( iMultisamplingFactor > 0 )
	{
		#ifdef DX11
		#else
		BOOL bWindowModeFlag = TRUE;
		GGFORMAT d3dDeviceFormat = m_WindowsD3DMODE.Format;
		if ( m_iDisplayType == FULLSCREEN )
		{
			bWindowModeFlag = FALSE;
			d3dDeviceFormat = m_Depth;
		}
		if ( SUCCEEDED(m_pDX->CheckDeviceMultiSampleType(	m_uAdapterChoice, D3DDEVTYPE_HAL, d3dDeviceFormat,
															bWindowModeFlag, (D3DMULTISAMPLE_TYPE)iMultisamplingFactor, NULL ) ) )
			return true;
		else
			return false;
		#endif
	}

	// regular check on W,H,D
	return CheckDisplayMode ( iWidth, iHeight, iDepth );
}

DARKSDK bool CheckDisplayMode ( int iWidth, int iHeight, int iDepth, int iMode )
{
	if ( CheckDisplayMode ( iWidth, iHeight, iDepth ) )
	{
		GetValidBackBufferFormat ( );
		#ifdef DX11
		#else
		if ( m_pDX )
		{
			if ( SUCCEEDED ( m_pDX->CheckDeviceType
														( 
															0, 
															D3DDEVTYPE_HAL,
															m_Depth,
															m_Depth,
															iMode 
														) ) )
				return true;
		}
		#endif
	}

	return false;
}

DARKSDK bool CheckDisplayMode ( int iWidth, int iHeight, int iDepth, int iMode, int iVertexProcessing )
{
	if ( CheckDisplayMode ( iWidth, iHeight, iDepth, iMode ) )
	{
		#ifdef DX11
		#else
		GGCAPS	d3dCaps;
		memset ( &d3dCaps, 0, sizeof ( d3dCaps ) );
		m_pD3D->GetDeviceCaps ( &d3dCaps );
		if ( iVertexProcessing == 0 && ( d3dCaps.Caps & D3DDEVCAPS_TLVERTEXSYSTEMMEMORY ) )
			return true;
		if ( iVertexProcessing == 1 && ( d3dCaps.Caps & D3DDEVCAPS_TLVERTEXVIDEOMEMORY ) )
			return true;
		#endif
	}

	return false;
}

DARKSDK bool GetWindowedMode ( void )
{
	// returns true if the app is running in windowed mode

	// check the display mode we're in
	if ( m_iDisplayType == 1 )
		return true;
	else
		return false;
}

DARKSDK int GetNumberOfDisplayModes ( void )
{
	// return the number of display modes available
	#ifdef DX11
	return 1;
	#else
	return m_pDX->GetAdapterModeCount ( 0, g_GGFORMAT );
	#endif
}

DARKSDK void GetDisplayMode ( int iID, char* szMode )
{
	// variable declarations
	char	szResolution [ 256 ];	// used to store the main bulk of the string
	
	// check the string passed is valid
	if ( !szMode )
		return;

	// clear out the resolution string
	memset ( szResolution, 0, sizeof ( szResolution ) );
	
	// check the format of the mode
	#ifdef DX11
	#else
	int		iDepth;					// stores the resolution depth
	if ( 
			m_pInfo [ 0 ].D3DDisplay [ iID ].Format == GGFMT_X8R8G8B8 ||
			m_pInfo [ 0 ].D3DDisplay [ iID ].Format == GGFMT_A8R8G8B8 ||
			m_pInfo [ 0 ].D3DDisplay [ iID ].Format == GGFMT_R8G8B8
	   )
	{
		// we're dealing with a 32 bit mode
		iDepth = 32;
	}
	else
	{
		// we're dealing with a 16 bit mode
		iDepth = 16;
	}

	// finally build up the string
	sprintf ( szResolution, ("%ld x %ld x %ld"), m_pInfo [ 0 ].D3DDisplay [ iID ].Width, m_pInfo [ 0 ].D3DDisplay [ iID ].Height, iDepth );

	// and now copy the resolution information across
	strcpy ( szMode, szResolution );
	#endif
}

DARKSDK int GetNumberOfDisplayDevices ( void )
{
	#ifdef DX11
	return 1;
	#else
	return m_pDX->GetAdapterCount ( );
	#endif
}

DARKSDK void SetDisplayDevice ( int iID )
{
}

DARKSDK void GetDeviceName ( int iID, char* szDevice )
{
	// gets the name of the selected device
	#ifdef DX11
	#else
	// declare a structure which will contain device information
	D3DADAPTER_IDENTIFIER9 identifier;

	// check that D3D has been initialised and that the pointer
	// passed to the function is valid
	if ( !m_pDX || !szDevice )
		return;

	// clear out the structure
	memset ( &identifier, 0, sizeof ( identifier ) );

	// get info about the device
	m_pDX->GetAdapterIdentifier ( iID, 0, &identifier );

	// now check that the call succeeded, if it did
	// copy the name across to the pointer
	if ( identifier.Description )
		strcpy ( szDevice, identifier.Description );
	#endif
}

DARKSDK void GetDeviceDriverName ( int iID, char* szDriver )
{
	// gets the name of the driver for the selected device
	#ifdef DX11
	#else
	// declare a structure which will contain device information
	D3DADAPTER_IDENTIFIER9 identifier;

	// check that D3D has been initialised and that the pointer
	// passed to the function is valid
	if ( !m_pDX || !szDriver )
		return;

	// clear out the structure
	memset ( &identifier, 0, sizeof ( identifier ) );

	// now get info about the device
	m_pDX->GetAdapterIdentifier ( iID, 0, &identifier );

	// now check that the call succeeded, if it did
	// copy the driver name across to the pointer
	if ( identifier.Driver )
		strcpy ( szDriver, identifier.Driver );
	#endif
}

DARKSDK void SetDitherMode ( int iMode )
{
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( D3DRS_DITHERENABLE, iMode );
	#endif
}

DARKSDK void SetShadeMode ( int iMode )
{
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( D3DRS_SHADEMODE, iMode );
	#endif
}

DARKSDK void SetLightMode ( int iMode )
{
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( D3DRS_LIGHTING, iMode );
	#endif
}

DARKSDK void SetCullMode ( int iMode )
{
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( D3DRS_CULLMODE, iMode );
	#endif
}

DARKSDK void SetSpecularMode ( int iMode )
{
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( D3DRS_SPECULARENABLE, iMode );
	#endif
}

DARKSDK void SetRenderState ( int iState, int iValue )
{
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( ( D3DRENDERSTATETYPE ) iState, iValue );
	#endif
}

DARKSDK LPGG GetDirect3D ( void )
{
	return m_pDX;
}

DARKSDK float __stdcall Timer ( TIMER_COMMAND command )
{
	// sets up a timer, flags can be used to perform the
	// following operations -
	//          TIMER_RESET           - to reset the timer
	//          TIMER_START           - to start the timer
	//          TIMER_STOP            - to stop ( or pause ) the timer
	//          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
	//          TIMER_GETABSOLUTETIME - to get the absolute system time
	//          TIMER_GETAPPTIME      - to get the current time
	//          TIMER_GETELAPSEDTIME  - to get the time that elapsed between 
	//                                  TIMER_GETELAPSEDTIME calls

    static BOOL     m_bTimerInitialized = FALSE;
    static BOOL     m_bUsingQPF         = FALSE;
    static LONGLONG m_llQPFTicksPerSec  = 0;

    // initialize the timer
    if ( FALSE == m_bTimerInitialized )
    {
        m_bTimerInitialized = TRUE;

        // use QueryPerformanceFrequency ( ) to get frequency of timer. If QPF is
        // not supported, we will timeGetTime ( ) which returns milliseconds.
        LARGE_INTEGER qwTicksPerSec;

        m_bUsingQPF = QueryPerformanceFrequency ( &qwTicksPerSec );

        if ( m_bUsingQPF )
            m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
    }

    if ( m_bUsingQPF )
    {
        static LONGLONG m_llStopTime        = 0;
        static LONGLONG m_llLastElapsedTime = 0;
        static LONGLONG m_llBaseTime        = 0;
        double fTime;
        double fElapsedTime;
        LARGE_INTEGER qwTime;
        
        // get either the current time or the stop time, depending
        // on whether we're stopped and what command was sent
        if ( m_llStopTime != 0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME )
            qwTime.QuadPart = m_llStopTime;
        else
            QueryPerformanceCounter ( &qwTime );

        // return the elapsed time
        if ( command == TIMER_GETELAPSEDTIME )
        {
            fElapsedTime = ( double ) ( qwTime.QuadPart - m_llLastElapsedTime ) / ( double ) m_llQPFTicksPerSec;
            m_llLastElapsedTime = qwTime.QuadPart;
            return ( FLOAT ) fElapsedTime;
        }
    
        // return the current time
        if ( command == TIMER_GETAPPTIME )
        {
            double fAppTime = ( double ) ( qwTime.QuadPart - m_llBaseTime ) / ( double ) m_llQPFTicksPerSec;
            return ( FLOAT ) fAppTime;
        }
    
        // reset the timer
        if ( command == TIMER_RESET )
        {
            m_llBaseTime        = qwTime.QuadPart;
            m_llLastElapsedTime = qwTime.QuadPart;
            return 0.0f;
        }
    
        // start the timer
        if ( command == TIMER_START )
        {
            m_llBaseTime       += qwTime.QuadPart - m_llStopTime;
            m_llStopTime        = 0;
            m_llLastElapsedTime = qwTime.QuadPart;
            return 0.0f;
        }
    
        // stop the timer
        if ( command == TIMER_STOP )
        {
            m_llStopTime        = qwTime.QuadPart;
            m_llLastElapsedTime = qwTime.QuadPart;
            return 0.0f;
        }
    
        // advance the timer by 1/10th second
        if ( command == TIMER_ADVANCE )
        {
            m_llStopTime += m_llQPFTicksPerSec / 10;
            return 0.0f;
        }

        if ( command == TIMER_GETABSOLUTETIME )
        {
            fTime = qwTime.QuadPart / ( double ) m_llQPFTicksPerSec;
            return ( FLOAT ) fTime;
        }

		// invalid command specified
        return -1.0f;
    }
    else
    {
        // get the time using timeGetTime()
        static double m_fLastElapsedTime  = 0.0;
        static double m_fBaseTime         = 0.0;
        static double m_fStopTime         = 0.0;
        double fTime;
        double fElapsedTime;
        
        // get either the current time or the stop time, depending
        // on whether we're stopped and what command was sent
        if ( m_fStopTime != 0.0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME )
            fTime = m_fStopTime;
        else
            fTime = timeGetTime ( ) * 0.001;
    
        // return the elapsed time
        if ( command == TIMER_GETELAPSEDTIME )
        {   
            fElapsedTime = ( double ) ( fTime - m_fLastElapsedTime );
            m_fLastElapsedTime = fTime;
            return ( FLOAT ) fElapsedTime;
        }
    
        // return the current time
        if ( command == TIMER_GETAPPTIME )
        {
            return ( FLOAT ) ( fTime - m_fBaseTime );
        }
    
        // reset the timer
        if ( command == TIMER_RESET )
        {
            m_fBaseTime         = fTime;
            m_fLastElapsedTime  = fTime;
            return 0.0f;
        }
    
        // start the timer
        if ( command == TIMER_START )
        {
            m_fBaseTime        += fTime - m_fStopTime;
            m_fStopTime         = 0.0f;
            m_fLastElapsedTime  = fTime;
            return 0.0f;
        }
    
        // stop the timer
        if ( command == TIMER_STOP )
        {
            m_fStopTime = fTime;
            return 0.0f;
        }
    
        // advance the timer by 1/10th second
        if ( command == TIMER_ADVANCE )
        {
            m_fStopTime += 0.1f;
            return 0.0f;
        }

		// get absolute time
        if ( command == TIMER_GETABSOLUTETIME )
        {
            return ( FLOAT ) fTime;
        }

		// invalid command specified
        return -1.0f;
    }
}

//
// Display Command Functions
//

DARKSDK int CheckDisplayMode ( int iWidth, int iHeight, int iDepth )
{
	#ifdef DX11
	#else
	for ( int iTemp = 0; iTemp < m_pInfo [ 0 ].iDisplayCount; iTemp++ )
	{
		if ( m_pInfo [ 0 ].D3DDisplay [ iTemp ].Width == (DWORD)iWidth && m_pInfo [ 0 ].D3DDisplay [ iTemp ].Height == (DWORD)iHeight )
		{
			GGFORMAT GGFORMAT = m_pInfo [ 0 ].D3DDisplay [ iTemp ].Format;
			if ( iDepth == 16 )
			{
				 if (	GGFORMAT == GGFMT_R5G6B5   || 
						GGFORMAT == GGFMT_X1R5G5B5 ||
						GGFORMAT == GGFMT_A1R5G5B5  )
						return 1;
			}

			if ( iDepth == 32 )
			{
				 if (	GGFORMAT == GGFMT_X8R8G8B8 || 
						GGFORMAT == GGFMT_A8R8G8B8 ||
						GGFORMAT == GGFMT_R8G8B8    )
						return 1;
			}
		}
	}
	#endif
	return 0;
}

DARKSDK LPSTR CurrentGraphicsCard ( void )
{
	// Work string
	strcpy(m_pWorkString, m_pAdapterName);

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromWorkString();
	return pReturnString;
}

DARKSDK int EmulationMode ( void )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
	return 0;
}

DARKSDK void PerformChecklistForDisplayModes ( void )
{
	#ifdef DX11
	#else
	GGDISPLAYMODE* pMode = new GGDISPLAYMODE [ m_pInfo [ m_iAdapterUsed ].iDisplayCount ];
	memcpy ( pMode, m_pInfo [ m_iAdapterUsed ].D3DDisplay, sizeof ( GGDISPLAYMODE ) * m_pInfo [ m_iAdapterUsed ].iDisplayCount );
	for ( int iA = 0; iA < m_pInfo [ m_iAdapterUsed ].iDisplayCount; iA++ )
	{
		for ( int iB = 0; iB < m_pInfo [ m_iAdapterUsed ].iDisplayCount; iB++ )
		{
			if ( iA == iB )
				continue;

			int iWidthA   = pMode [ iA ].Width;
			int iHeightA  = pMode [ iA ].Height;
			int iRefreshA = pMode [ iA ].RefreshRate;

			int iWidthB   = pMode [ iB ].Width;
			int iHeightB  = pMode [ iB ].Height;
			int iRefreshB = pMode [ iB ].RefreshRate;

			if ( iWidthA == iWidthB && iHeightA == iHeightB )
			{
				DWORD BitDepthA = GetBitDepthFromFormat ( pMode [ iA ].Format );
				DWORD BitDepthB = GetBitDepthFromFormat ( pMode [ iB ].Format );

				if ( BitDepthA == BitDepthB )
				{
					pMode [ iA ].Width  = 0;
					pMode [ iA ].Height = 0;
				}
			}
		}
	}

	g_pGlob->checklisthasvalues=true;
	g_pGlob->checklisthasstrings=true;

	g_dwMaxStringSizeInEnum=0;
	g_bCreateChecklistNow=false;
	for(int pass=0; pass<2; pass++)
	{
		if(pass==1)
		{
			// Ensure checklist is large enough
			g_bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
				GlobExpandChecklist(c, g_dwMaxStringSizeInEnum);
		}

		// Run through total list of enumerated display modes
		g_pGlob->checklistqty=0;
		for ( int iTemp = 0; iTemp < m_pInfo [ m_iAdapterUsed ].iDisplayCount; iTemp++ )
		{
			int BitDepth=0;
			GGDISPLAYMODE displaymode = pMode [ iTemp ];

			// mike - 230604 - skip invalid modes
			if ( displaymode.Width == 0 && displaymode.Height == 0 )
				continue;

			// These are the only valid render depths
			BitDepth=GetBitDepthFromFormat(displaymode.Format);

			// Only list 16bit+ modes
			if(BitDepth>=16)
			{
				if(g_bCreateChecklistNow)
				{
					g_pGlob->checklist[g_pGlob->checklistqty].valuea = displaymode.Width;
					g_pGlob->checklist[g_pGlob->checklistqty].valueb = displaymode.Height;
					g_pGlob->checklist[g_pGlob->checklistqty].valuec = BitDepth;
					wsprintf(g_pGlob->checklist[g_pGlob->checklistqty].string, "%dx%dx%d", displaymode.Width, displaymode.Height, BitDepth);
				}
				else
				{
					DWORD dwLength=32;//00x00x00
					if(dwLength>g_dwMaxStringSizeInEnum)
						g_dwMaxStringSizeInEnum=dwLength;
				}
				g_pGlob->checklistqty++;
			}
		}
	}
 
	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
	
	SAFE_DELETE_ARRAY ( pMode );
	#endif
}

DARKSDK void PerformChecklistForGraphicsCards ( void )
{
	// Generate Checklist
	g_pGlob->checklisthasvalues=false;
	g_pGlob->checklisthasstrings=true;

	g_dwMaxStringSizeInEnum=0;
	g_bCreateChecklistNow=false;
	for(int pass=0; pass<2; pass++)
	{
		if(pass==1)
		{
			// Ensure checklist is large enough
			g_bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
				GlobExpandChecklist(c, g_dwMaxStringSizeInEnum);
		}

		// Run through total list of enumerated adapter(card) names
		g_pGlob->checklistqty=0;
		for ( int iTemp = 0; iTemp < m_iAdapterCount; iTemp++ )
		{
			if(g_bCreateChecklistNow)
			{
				wsprintf(g_pGlob->checklist[g_pGlob->checklistqty].string, "%s", m_pInfo [ iTemp ].szName);
			}
			else
			{
				DWORD dwLength=strlen(m_pInfo [ iTemp ].szName);
				if(dwLength>g_dwMaxStringSizeInEnum)
					g_dwMaxStringSizeInEnum=dwLength;
			}
			g_pGlob->checklistqty++;
		}
	}
 
	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
}

DARKSDK int GetDisplayType ( void )
{
	// find out what type of device we're dealing with
	// return true if it's a hardware device

	// variable declarations
	GGCAPS	d3dCaps;	// D3D capabilities structure

	// clear out the structure
	memset ( &d3dCaps, 0, sizeof ( d3dCaps ) );

	// get the device caps, first check the pointer is valid
	if ( !m_pD3D )
		return 0;

	#ifdef DX11
	#else
	m_pD3D->GetDeviceCaps ( &d3dCaps );
	if ( d3dCaps.DeviceType == D3DDEVTYPE_HAL )
		return 1;
	#endif
	return 0;
}

DARKSDK int GetDisplayWidth ( void )
{
	// always return screen width
	return g_pGlob->iScreenWidth;

	// get the width of the display, first off check what mode
	// we're running in as we have to deal with it a bit differently

	if ( m_iDisplayType == 0 )
	{
		// when we're running in fullscreen just return
		// the width we have stored
		return m_iWidth;
	}
	else
	{
		// when we're in windowed mode return the
		// size of the window
		RECT rect;

		GetWindowRect ( m_hWnd, &rect );
		
		return rect.right - rect.left;
	}
}

DARKSDK int GetDisplayHeight ( void )
{
	// always return screen height
	return g_pGlob->iScreenHeight;

	// get the height of the display, first off check what mode
	// we're running in as we have to deal with it a bit differently

	if ( m_iDisplayType == 0 )
	{
		// when we're running in fullscreen just return
		// the width we have stored
		return m_iWidth;
	}
	else
	{
		// when we're in windowed mode return the
		// size of the window
		RECT rect;

		GetWindowRect ( m_hWnd, &rect );

		return rect.bottom - rect.top;
	}
}

DARKSDK int GetWindowWidth ( void )
{
	return g_pGlob->dwWindowWidth;
}

DARKSDK int GetWindowHeight ( void )
{
	return g_pGlob->dwWindowHeight;
}

DARKSDK int GetDesktopWidth ( void )
{
	return (int)GetSystemMetrics(SM_CXSCREEN);
}

DARKSDK int GetDesktopHeight ( void )
{
	return (int)GetSystemMetrics(SM_CYSCREEN);
}

DARKSDK int GetDisplayDepth ( void )
{
	return m_iDepth;
}

DBPRO_GLOBAL FLOAT fLastTime = 0.0f;
DBPRO_GLOBAL DWORD dwFrames  = 0L;
DBPRO_GLOBAL float m_fFPS    = 0.0f;
DBPRO_GLOBAL char FrameStats [ 256 ];
DBPRO_GLOBAL float fTime;

DARKSDK int GetDisplayFPS ( void )
{
	// get the rate at which tha app is running, must be called once
	// each frame to work properly

	// mike - 250604 - only do fps once per frame
	if ( g_bValidFPS )
	{

		fTime = Timer ( TIMER_GETABSOLUTETIME );
		++dwFrames;

		if ( fTime - fLastTime > 1.0f )
		{
			m_fFPS    = dwFrames / ( fTime - fLastTime );
			fLastTime = fTime;
			dwFrames  = 0L;
		}

		g_bValidFPS = false;
	}

	return ( int ) m_fFPS;
}

DARKSDK int GetDisplayInvalid ( void )
{
	// is set to 1 when the focus has been remove from the window
	int iInvalidSet=0;
	if(g_pGlob->bInvalidFlag) iInvalidSet=1;
	g_pGlob->bInvalidFlag=false;
	return iInvalidSet;
}

DARKSDK void SetGamma ( int iR, int iG, int iB )
{
	// set the gamma for the screen
	#ifdef DX11
	#else
	// variable declarations
	D3DGAMMARAMP	ddgr;			// gamma structure
    
	WORD			wRed   = 0;		// red value
	WORD			wGreen = 0;		// green value
	WORD			wBlue  = 0;		// blue value

	int				iColour;		// used for loops

	// clear out the gamma structu
    memset ( &ddgr, 0, sizeof ( ddgr ) );

	// store the gamma values in case we need
	// to retrieve them later on
	m_iGammaRed   = iR;		// store red
	m_iGammaGreen = iG;		// store green
	m_iGammaBlue  = iB;		// store blue
        
	// run through all of the array and setup the colours
    for ( iColour = 0; iColour < 256; iColour++ )
    {
		// setup colours
		ddgr.red   [ iColour ] = wRed;		// set red
        ddgr.green [ iColour ] = wGreen;	// set green
        ddgr.blue  [ iColour ] = wBlue;		// set blue

		// increment colours
		wRed   += ( WORD ) iR;	// add red component
		wGreen += ( WORD ) iG;	// add green component
		wBlue  += ( WORD ) iB;	// add blue component
    }

	// now that we have setup the gamma structure we can
	// pass the info across to D3D and apply the changes
	m_pD3D->SetGammaRamp ( 0, D3DSGR_NO_CALIBRATION, &ddgr );        
	#endif
}

DARKSDK void SetDisplayModeEx ( int iWidth, int iHeight, int iDepth )
{
	// Inform all DLLs to release resources
	InformDLLsOfD3DChange(0);

	// lee - 090312 - Detect if want to force OS to change resolution (not friendly) (an alternative to full screen mode)
	if ( iWidth<0 || iHeight<0 )
	{
		// restore resolution to positive values
		iWidth=abs(iWidth);
		iHeight=abs(iHeight);

		// now force the OS to this resolution
		DEVMODE NewDevice;
		NewDevice.dmSize = sizeof(DEVMODE);
		NewDevice.dmPelsWidth = iWidth;
		NewDevice.dmPelsHeight = iHeight;
		NewDevice.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		HRESULT hRes = ChangeDisplaySettings(&NewDevice, 0);//CDS_GLOBAL | CDS_UPDATEREGISTRY);
		if ( hRes==DISP_CHANGE_SUCCESSFUL )
		{
			// successfully changed the resolution
		}
		else
		{
			LPSTR pError = "Unknown Change Display Settings Error";
			switch ( hRes )
			{
				case DISP_CHANGE_BADDUALVIEW : pError = "The settings change was unsuccessful because the system is DualView capable."; break;
				case DISP_CHANGE_BADFLAGS : pError = "An invalid set of flags was passed in."; break;
				case DISP_CHANGE_BADMODE : pError = "The graphics mode is not supported."; break;
				case DISP_CHANGE_BADPARAM : pError = "An invalid parameter was passed in. This can include an invalid flag or combination of flags."; break;
				case DISP_CHANGE_FAILED : pError = "The display driver failed the specified graphics mode."; break;
				case DISP_CHANGE_NOTUPDATED : pError = "Unable to write settings to the registry."; break;
				case DISP_CHANGE_RESTART : pError = "The computer must be restarted for the graphics mode to work."; break;
			}
			MessageBox ( NULL, pError, "Change Display Settings Error", MB_OK );
		}
	}

	// Sets window size in any event
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	// Update global data for window size
	g_pGlob->dwWindowWidth = m_iWidth;
	g_pGlob->dwWindowHeight = m_iHeight;

	// Update global data for screen size
	g_pGlob->iScreenWidth=m_iWidth;
	g_pGlob->iScreenHeight=m_iHeight;
	g_pGlob->iScreenDepth=m_iDepth;

	// Set New Display Mode (recreates D3DDevice)
	if( !SetDisplayMode ( iWidth, iHeight, iDepth, m_iDisplayType, HARDWARE, 1 ) )
		return;

	// Inform all DLLs of new Device and recreate resources
	InformDLLsOfD3DChange(1);

	// Clear area now used as backbuffer
	SETUPClear ( 0, 0, 0 );

	// VR920 Support - Use stereo switching technique
	#ifdef VR920SUPPORT
	g_StereoEnabled = false;
	if ( g_VR920StereoMethod==true )
	{
		// Determine if the VR920 is plugged in
		g_VR920AdapterAvailable = false;

		// Open the VR920's tracker driver.
		long iwr_status = IWRLoadDll();
		if( iwr_status != IWR_OK ) 
		{
			MessageBox( NULL, "NO VR920 iwrdriver support", "VR Error", MB_OK );
			return;
		}

		// Get the ID of the hardware and its capabilities
		WORD vrProdID = IWRGetProductID();
		if ( vrProdID == IWR_PROD_VR920 || vrProdID == IWR_PROD_WRAP )
		{
			// stereo render behavior left + right
			g_StereoEnabled	= g_VR920StereoMethod;

			// VR hardware/driver found
			g_VR920AdapterAvailable = true;
			if ( IWRSetFilterState ) IWRSetFilterState(TRUE);

			// And switch off autocorrection
			if ( IWRSetFilterState ) IWRSetMagAutoCorrect(TRUE);

			// Create query interfaces to sync well with GPU
			D3D11_QUERY_DESC d3dQueryDesc;
			d3dQueryDesc.Query = D3D11_QUERY_EVENT;
			d3dQueryDesc.MiscFlags = 0;
			m_pD3D->CreateQuery(&d3dQueryDesc, &g_pLeftEyeQuery);
			m_pD3D->CreateQuery(&d3dQueryDesc, &g_pRightEyeQuery);

			// Open a handle to the VR920's stereo driver
			g_StereoHandle = IWRSTEREO_Open();
			if( g_StereoHandle == INVALID_HANDLE_VALUE )
				g_VR920AdapterAvailable = false;

			// Select MONO or STEREO
			if( g_StereoEnabled ) 
				IWRSTEREO_SetStereo( g_StereoHandle, IWR_STEREO_MODE );
			else 
				IWRSTEREO_SetStereo( g_StereoHandle, IWR_MONO_MODE );
		}
		if ( g_VR920AdapterAvailable == false )
		{
			//MessageBox ( NULL, "iWear is not connected. Connect your iWear device and relaunch the software if you wish to Test Game in VR mode.", "VR Error", MB_OK );
			g_VR920StereoMethod=false;
			g_StereoEnabled	= g_VR920StereoMethod;
		}
	}
	#endif
}

DARKSDK void SetDisplayModeVSYNC ( int iWidth, int iHeight, int iDepth, int iVSyncOn )
{
	// choose new global vsync state
	if ( iVSyncOn>0 )
	{
		if ( iVSyncOn==1 ) m_iVSyncInterval=1;
		if ( iVSyncOn==2 ) m_iVSyncInterval=2;
		if ( iVSyncOn==3 ) m_iVSyncInterval=3;
		m_bVSync=true;
	}
	else
	{
		m_bVSync=false;
	}

	// call regular set display mode
	SetDisplayModeEx ( iWidth, iHeight, iDepth );
}

DARKSDK void SetDisplayModeANTIALIAS ( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode )
{
	// set new multisampling factor
	m_iMultisamplingFactor = iMultisamplingFactor;

	// set new multimpnitor mode value
	m_iMultimonitorMode = iMultimonitorMode;

	// call regular set display mode
	SetDisplayModeVSYNC ( iWidth, iHeight, iDepth, iVSyncOn );
}

DARKSDK void SetDisplayModeMODBACKBUFFER ( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode, int iBackbufferWidth, int iBackbufferHeight )
{
	// adjust real backbuffer to differ form size of resolution (render 1-2-1 pixels in window of screen)
	m_iModBackbufferWidth = iBackbufferWidth;
	m_iModBackbufferHeight = iBackbufferHeight;

	// call regular set display mode
	SetDisplayModeANTIALIAS ( iWidth, iHeight, iDepth, iVSyncOn, iMultisamplingFactor, iMultimonitorMode );
}

DARKSDK void SetDisplayModeVR ( int iWidth, int iHeight, int iDepth, int iVSyncOn, int iMultisamplingFactor, int iMultimonitorMode, int iBackbufferWidth, int iBackbufferHeight, int iActivateVRMode )
{
	// switch on special left/right present code for iActivateVRMode if flagged
	// mode 1 : VR920
	#ifdef VR920SUPPORT
	if ( iActivateVRMode==1 )
		g_VR920StereoMethod = true;
	else
		g_VR920StereoMethod = false;
	#endif

	// call regular set display mode
	SetDisplayModeMODBACKBUFFER ( iWidth, iHeight, iDepth, iVSyncOn, iMultisamplingFactor, iMultimonitorMode, iBackbufferWidth, iBackbufferHeight );
}


DARKSDK void RestoreLostDevice ( void )
{
	#ifdef DX11
	#else
	HRESULT hRes = m_pD3D->TestCooperativeLevel();
	if(hRes!=GG_OK)
	{
		// Fullscreen lost focus (maybe ALT+TAB)
		if(hRes==GGERR_DEVICELOST)
		{
			// leeadd - 020308 - signal to all TPC DLLs that device has been lost
			InformDLLsOfDeviceLostOrNotReset ( 1 );
			return;
		}

		// Attempt to restore device by testing for not reset state
		if(hRes==D3DERR_DEVICENOTRESET)
		{
			// leeadd - 020308 - signal to all TPC DLLs that device has not been reset (but is no longer lost)
			InformDLLsOfDeviceLostOrNotReset ( 2 );

			// Recreates device to restore application
			SetDisplayModeEx ( g_pGlob->iScreenWidth, g_pGlob->iScreenHeight, g_pGlob->iScreenDepth );

			// leeadd - 070308 - added an extra callback to signal when successfully recreated device
			hRes = m_pD3D->TestCooperativeLevel();
			if(hRes==GG_OK)
			{
				// allows TPC DLLs to know when the device has been reset, and objects can be recreated 
				InformDLLsOfDeviceLostOrNotReset ( 3 );
			}
		}
	}
	#endif
}

DARKSDK void SetEmulationOn   ( void )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}

DARKSDK void SetEmulationOff   ( void )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}

DARKSDK void SetGraphicsCard ( DWORD dwCardname )
{
	// lee 100206 - Not Implemented in DBPRO U6 RELEASE (was never implemented)
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}


//
// Window Command Functions
//

bool g_bDetachFromPreviousWindow = false;

DARKSDK void SetWindowModeOn(void)
{
	if ( g_bWindowOverride==false )
	{
		// Switch to window
		if ( m_iDisplayType==FULLSCREEN )
		{
			// lee - 230306 - u6b4 - only if currently fullscreen
			m_iDisplayType = WINDOWED;
			SetDisplayModeEx ( g_pGlob->iScreenWidth, g_pGlob->iScreenHeight, g_pGlob->iScreenDepth );
		}

		// lee - 290306 - u6rc3 - keep legacy behaviour by switching mode to regular window
		g_pGlob->dwAppDisplayModeUsing=1;

		gbWindowMode=true;
		gbWindowBorderActive=false;
		gWindowVisible=SW_SHOWDEFAULT;
		gWindowSizeX = g_pGlob->iScreenWidth;
		gWindowSizeY = g_pGlob->iScreenHeight;
		gWindowExtraX = gWindowExtraXForOverlap;
		gWindowExtraY = gWindowExtraYForOverlap;
		gWindowStyle = WS_OVERLAPPEDWINDOW;
		DB_UpdateEntireWindow(true,true);
	}

	// Focus on getting input from keyboard and mouse
	if ( g_bWindowOverride )
	{
		if ( g_bDetachFromPreviousWindow==false )
		{
			// direct input
			g_bDetachFromPreviousWindow = true;
			SetupKeyboardEx(1);
			SetupMouseEx(1);
		}
		else
		{
			// input via parent window
			g_bDetachFromPreviousWindow = false;
			SetupKeyboardEx(0);
			SetupMouseEx(0);
		}
	}
}

DARKSDK void SetWindowModeOff(void)
{
	gbWindowMode=false;
	gbWindowBorderActive=true;
	gWindowVisible=SW_SHOWDEFAULT;
	gWindowSizeX = GetSystemMetrics(SM_CXFULLSCREEN);
	gWindowSizeY = GetSystemMetrics(SM_CYFULLSCREEN);
	gWindowIconHandle = gOriginalIcon;
	gWindowExtraX = 0;
	gWindowExtraY = 0;
	gWindowStyle = WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU;
	DB_UpdateEntireWindow(true,true);

	// Switch to fullscreen
	if ( m_iDisplayType==WINDOWED )
	{
		// lee - 230306 - u6b4 - only if currently window
		m_iDisplayType = FULLSCREEN;
		g_pGlob->dwAppDisplayModeUsing=3;
		SetDisplayModeEx ( g_pGlob->iScreenWidth, g_pGlob->iScreenHeight, g_pGlob->iScreenDepth );
	}
}

DARKSDK void SetWindowSettings( int iStyle, int iCaption, int iIcon )
{
	gWindowStyle=0;
	if(iStyle==1)
	{
		gWindowExtraX=gWindowExtraXForOverlap;
		gWindowExtraY=gWindowExtraYForOverlap;
		gWindowStyle |= WS_OVERLAPPEDWINDOW;
	}
	else
	{
		gWindowExtraX=0;
		gWindowExtraY=0;
		gWindowStyle |= WS_POPUP; 
	}

	// lee - 200306 - u6b4 - added extra layout codes
	if ( iStyle==2 ) gWindowStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX;
	if ( iStyle==3 ) gWindowStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	if ( iStyle==4 ) gWindowStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX;

	if(iCaption==1)
	{
		gWindowExtraX=gWindowExtraXForOverlap-2;
		gWindowExtraY=gWindowExtraYForOverlap-2;
		gWindowStyle |= WS_CAPTION;
	}

	if(iIcon==1)
		gWindowIconHandle=gOriginalIcon;
	else
		gWindowIconHandle=NULL;

	// update window
	DB_UpdateEntireWindow(true,true);
}

DARKSDK void SetWindowPosition( int posx, int posy )
{
	g_pGlob->dwWindowX=posx;
	g_pGlob->dwWindowY=posy;
	DB_UpdateEntireWindow(true,true);
}

DARKSDK void SetWindowSize( int sizex, int sizey )
{
	gWindowSizeX=sizex;
	gWindowSizeY=sizey;
	DB_UpdateEntireWindow(true,true);
}

DARKSDK void HideWindow(void)
{
	gWindowVisible=SW_HIDE;
	DB_UpdateEntireWindow(false,true);
}

DARKSDK void ShowWindow(void)
{
	gWindowVisible=SW_SHOW;
	DB_UpdateEntireWindow(false,true);
}

DARKSDK void MinimiseWindow(void)
{
	gWindowVisible=SW_MINIMIZE;
	DB_UpdateEntireWindow(false,true);
}

DARKSDK void MaximiseWindow(void)
{
	gWindowVisible=SW_MAXIMIZE;
	DB_UpdateEntireWindow(false,true);
}

DARKSDK void RestoreWindow(void)
{
	gWindowVisible=SW_SHOWNORMAL;
	DB_UpdateEntireWindow(false,true);
}

DARKSDK void SetWindowTitle( LPSTR pTitleString )
{
	strcpy(gWindowName, pTitleString);
	SetWindowText(m_hWnd, gWindowName);
}

DARKSDK int WindowExist( LPSTR pTitleString )
{
	if ( FindWindow ( NULL, pTitleString )!=NULL )
		return 1;
	else
		return 0;
}

DARKSDK void WindowToBack(void)
{
	SetWindowPos(m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

DARKSDK void WindowToFront(void)
{
	// U75 - 080909 - previously commented out - SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetForegroundWindow(m_hWnd);
}

DARKSDK void WindowToFront(LPSTR pTitleString)
{
	LPSTR lpstrTitle = pTitleString;
	if ( strnicmp ( lpstrTitle, "__topmost__", 11 )==NULL )
	{
		// U75 - 051109 - special name to send window to topmost
		// makes sense in test game for FPS Creator as mouse pointer is hijacked anyway!
		SetForegroundWindow(m_hWnd);
		SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		// U&5 - 070510 - Force a click on the screen to give topmost window the focus
		MOUSEINPUT Mouse;
		memset ( &Mouse, 0, sizeof(MOUSEINPUT) );
		Mouse.dx=320;
		Mouse.dy=240;
		INPUT Input;
		memset ( &Input, 0, sizeof(INPUT) );
		Input.type = INPUT_MOUSE;
		Input.mi = Mouse;
		Mouse.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput ( 1, &Input, sizeof(INPUT) );
		Mouse.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput ( 1, &Input, sizeof(INPUT) );
		Mouse.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput ( 1, &Input, sizeof(INPUT) );
		MOUSEINPUT Mouse2;
		memset ( &Mouse2, 0, sizeof(MOUSEINPUT) );
		Mouse2.dx=320;
		Mouse2.dy=240;
		INPUT Input2;
		memset ( &Input2, 0, sizeof(INPUT) );
		Input2.type = INPUT_MOUSE;
		Input2.mi = Mouse2;
		Mouse2.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput ( 1, &Input2, sizeof(INPUT) );
	}
	else
	{
		HWND hWnd = FindWindow ( NULL, lpstrTitle );
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetForegroundWindow(hWnd);
	}
}

DARKSDK void WindowToBack(LPSTR pTitleString)
{
	HWND hWnd = FindWindow ( NULL, pTitleString );
	SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

HWND GetWindowHandle ( void )
{
	return m_hWnd;
}

//
// Backbuffer Command Functions
//

DARKSDK void LockBackbuffer(void)
{
	#ifdef DX11
	#else
	if(g_pBackBuffer==NULL)
	{
		HRESULT hRes = m_pD3D->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_pBackBuffer);
		if(g_pBackBuffer)
		{
			// get format of backbuffer
			D3DSURFACE_DESC ddsd;
			hRes = g_pBackBuffer->GetDesc(&ddsd);

			GGLOCKED_RECT d3dlockedrect;
			hRes = g_pBackBuffer->LockRect(&d3dlockedrect, NULL, D3DLOCK_NOSYSLOCK);
			if(hRes==GG_OK)
			{
				int BitDepth=GetBitDepthFromFormat(ddsd.Format);
				g_dwSurfacePtr=(DWORD)d3dlockedrect.pBits;
				g_dwSurfaceWidth=ddsd.Width;
				g_dwSurfaceHeight=ddsd.Height;
				g_dwSurfaceDepth=BitDepth;
				g_dwSurfacePitch=d3dlockedrect.Pitch;
			}
		}
	}
	#endif
}

DARKSDK void UnlockBackbuffer(void)
{
	#ifdef DX11
	#else
	if(g_pBackBuffer)
	{
		// Free locked surface
		g_pBackBuffer->UnlockRect();
		g_dwSurfacePtr=NULL;
		g_dwSurfaceWidth=0;
		g_dwSurfaceHeight=0;
		g_dwSurfaceDepth=0;
		g_dwSurfacePitch=0;

		// Release backbuffer
		SAFE_RELEASE(g_pBackBuffer);
	}
	#endif
}

DARKSDK DWORD GetBackbufferPtr()
{
	DWORD pReturnValue=-1;
	if(g_dwSurfacePtr)
		pReturnValue = g_dwSurfacePtr;

	return pReturnValue;
}

DARKSDK int GetBackbufferWidth()
{
	int pReturnValue=-1;
	if(g_dwSurfacePtr)
		pReturnValue = g_dwSurfaceWidth;

	return pReturnValue;
}

DARKSDK int GetBackbufferHeight()
{
	int pReturnValue=-1;
	if(g_dwSurfacePtr)
		pReturnValue = g_dwSurfaceHeight;

	return pReturnValue;
}

DARKSDK int GetBackbufferDepth()
{
	int pReturnValue=-1;
	if(g_dwSurfacePtr)
		pReturnValue = g_dwSurfaceDepth;

	return pReturnValue;
}

DARKSDK int GetBackbufferPitch()
{
	int pReturnValue=-1;
	if(g_dwSurfacePtr)
		pReturnValue = g_dwSurfacePitch;

	return pReturnValue;
}

// lee - 130108 - added for X10 compat.
DARKSDK void				SetNvPerfHUD						( int iUsePerfHUD )
{
	if ( iUsePerfHUD==1 )
		m_bNVPERFHUD = true;
	else
		m_bNVPERFHUD = false;
}

DARKSDK void				ForceAdapterOrdinal ( int iForceOrdinal )
{
	m_iForceAdapterOrdinal = iForceOrdinal;
}

DARKSDK void				ForceAdapterD3D11ONLY ( int iForceD3D11ONLY )
{
	m_iForceAdapterD3D11ONLY = iForceD3D11ONLY;
}

DARKSDK void				SetCaptureName						( DWORD pFilename )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void				SetCaptureCodec						( DWORD pFilename )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void				SetCaptureSettings					( int iCompressed, int iFPS, int iWidth, int iHeight, int iThreaded, int iWait )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void				SetCaptureMode						( int iRecordVideo )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void				SaveScreenshot						( DWORD pFilename )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void				StartPlayback						( DWORD pFilename, float fSpeed )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void				UpdatePlayback						( void )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void				StopPlayback						( void )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK int					PlaybackPlaying						( void )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
	return 0;
}

DARKSDK int					GetSessionTerminate					( void )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
	return 0;
}

bool g_bGrabTrackingDataOncePerTrio = false;
float g_fYaw, g_fPitch, g_fRoll;

/*
void ObtainYawPitchRoll ( int g_Filtering )
{
	#ifdef VR920SUPPORT
	// tmp vars
	long Roll=0, Yaw=0, Pitch=0;

	// Get iWear tracking yaw, pitch, roll
	HRESULT iwr_status = IWRGetTracking( &Yaw, &Pitch, &Roll );  
	if(	iwr_status != IWR_OK )
	{
		// iWear tracker could be OFFLine: just inform user or wait until plugged in...
		Yaw = Pitch = Roll = 0;

		// Attempt to re-establish communications with the VR920.
		IWROpenTracker();	
	}

	// Always provide for a means to disable filtering;
	if( g_Filtering == 1 )
	{
		// manual application filtering
		//IWRFilterTracking( &Yaw, &Pitch, &Roll );
		IWRGetTracking( &Yaw, &Pitch, &Roll );
		if( IWRSetFilterState ) IWRSetFilterState(FALSE);
	}
	if( g_Filtering == 2 )
	{
		// internal driver filtering
		if( IWRSetFilterState ) IWRSetFilterState(TRUE);
	}

	// return final values
	g_fPitch =  (float)Pitch * IWR_RAWTODEG;
	g_fYaw   =  (float)Yaw * IWR_RAWTODEG;
	g_fRoll  =  (float)Roll * IWR_RAWTODEG;
	#endif
}
*/

DARKSDK int ResetLeftEye ( void )
{
	// U75 - 150310 - extra command to detect VR920 AND reset to left eye (otherwise user cross-eyed)
	#ifdef VR920SUPPORT
	g_StereoEyeToggle=LEFT_EYE;
	if ( g_VR920AdapterAvailable==true )
		return 1;
	else
		return 0;
	#else
		return 0;
	#endif
}

DARKSDK void ResetTracking ( void )
{
	// reset to zero (forward facing set)
	#ifdef VR920SUPPORT
	// set to forward facing
	IWRZeroSet();
	// also reset left/right toggle so the eyes are not inverted
	// basically, after commands the engine presents left, then right, so reset
	g_StereoEyeToggle=LEFT_EYE;
	// also reset compensation as driver is cleared of all wrong doing
	g_fDriverCompensationPitch = 0;
	g_fDriverCompensationYaw = 0;
	g_fDriverCompensationRoll = 0;	
	#endif
}

/*
DARKSDK DWORD GetTrackingPitch ( int iFiltering )
{
	// resets grab on pitch call (X)
	g_bGrabTrackingDataOncePerTrio=false;
	if ( g_bGrabTrackingDataOncePerTrio==false )
	{
		ObtainYawPitchRoll ( iFiltering );
		g_bGrabTrackingDataOncePerTrio=true;
	}
	return *(DWORD*)&g_fPitch;
}
DARKSDK DWORD GetTrackingYaw ( int iFiltering )
{
	if ( g_bGrabTrackingDataOncePerTrio==false )
	{
		ObtainYawPitchRoll ( iFiltering );
		g_bGrabTrackingDataOncePerTrio=true;
	}
	return *(DWORD*)&g_fYaw;
}
DARKSDK DWORD GetTrackingRoll ( int iFiltering )
{
	if ( g_bGrabTrackingDataOncePerTrio==false )
	{
		ObtainYawPitchRoll ( iFiltering );
		g_bGrabTrackingDataOncePerTrio=true;
	}
	return *(DWORD*)&g_fRoll;
}
*/

DARKSDK LPSTR GetDXName ( void )
{
	#ifdef DX11
	 wsprintf ( m_pWorkString, "DX11" );
	#else
	 wsprintf ( m_pWorkString, "Classic" );
	#endif
	LPSTR pReturnString=GetReturnStringFromWorkString();
	return pReturnString;
}

DARKSDK LPSTR GetDirectRefreshRate ( void )
{
	// Create and return string
	#ifdef DX11
	wsprintf ( m_pWorkString, "DirectX 11" );
	#else
	wsprintf ( m_pWorkString, "%d", (int)m_WindowsD3DMODE.RefreshRate );
	#endif
	LPSTR pReturnString=GetReturnStringFromWorkString();
	return pReturnString;
}

//Dave - new routines to work with true pixels in client windows
DARKSDK void SetChildWindowTruePixel ( int mode )
{
	g_dwChildWindowTruePixel = mode;
}

DARKSDK int GetChildWindowX()
{
	RECT src;
	GetClientRect ( m_hWnd , &src );
	return (int)src.left;
}

DARKSDK int GetChildWindowY()
{
	RECT src;
	GetClientRect ( m_hWnd , &src );
	return (int)src.top;
}

DARKSDK int GetChildWindowWidth(int iDPIAware)
{
	RECT src;
	GetClientRect ( m_hWnd , &src );
	int iWidth=(int)src.right;
	if ( iDPIAware==1 )
	{
		HDC hdc = GetDC(NULL);
		int LogicalScreenHeight = GetDeviceCaps(hdc, (int)VERTRES);
		int PhysicalScreenHeight = GetDeviceCaps(hdc, (int)DESKTOPVERTRES); 
		float ScreenScalingFactor = (float)PhysicalScreenHeight / (float)LogicalScreenHeight;
		iWidth = (int)g_pGlob->dwWindowWidth;
		if ( ScreenScalingFactor==1.0f || ScreenScalingFactor==1.25f )
		{
			// 100% or 125% font size
			iWidth = (int)src.right; // ALL
		}
		if ( ScreenScalingFactor==1.5f )
		{
			// 150% font size
			if ( g_pGlob->dwWindowWidth==1280 ) iWidth = (int)src.right; // 1920x900
			if ( g_pGlob->dwWindowWidth==2560 ) iWidth = (int)src.right; // 3840x2160
		}
		ReleaseDC(NULL, hdc);
	}
	return iWidth;
}

DARKSDK int GetChildWindowHeight(int iDPIAware)
{
	RECT src;
	GetClientRect ( m_hWnd , &src );
	int iHeight=(int)src.bottom;
	if ( iDPIAware==1 )
	{
		HDC hdc = GetDC(NULL);
		int LogicalScreenHeight = GetDeviceCaps(hdc, (int)VERTRES);
		int PhysicalScreenHeight = GetDeviceCaps(hdc, (int)DESKTOPVERTRES); 
		float ScreenScalingFactor = (float)PhysicalScreenHeight / (float)LogicalScreenHeight;
		iHeight = (int)g_pGlob->dwWindowHeight;
		if ( ScreenScalingFactor==1.0f || ScreenScalingFactor==1.25f )
		{
			// 100% or 125% font size
			iHeight = (int)src.bottom; // ALL
		}
		if ( ScreenScalingFactor==1.5f )
		{
			// 150% font size
			if ( g_pGlob->dwWindowWidth==1280 ) iHeight = (int)src.bottom; // 1920x900
			if ( g_pGlob->dwWindowWidth==2560 ) iHeight = (int)src.bottom; // 3840x2160
		}
		ReleaseDC(NULL, hdc);
	}
	return iHeight;
}
DARKSDK int GetChildWindowWidth()
{
	return GetChildWindowWidth(0);
}
DARKSDK int GetChildWindowHeight()
{
	return GetChildWindowHeight(0);
}

#ifdef DX11
DARKSDK ID3DX11Effect* SETUPLoadShader ( LPSTR szFile, LPSTR szBlobFile, int iShaderIndex )
{
	// check if this index already has a shader inside it (zero just loads the shader to nowhere to create blob)
	if ( iShaderIndex > 0 )
	{
		if ( g_sShaders[iShaderIndex].pEffect )
		{
			// return existing shader (likely when LoadEffect command used)
			return g_sShaders[iShaderIndex].pEffect;
		}
	}

	// get wchar of blobfilename
	size_t origsize = strlen(szBlobFile) + 1;
	const size_t newsize = 1024;
	size_t convertedChars = 0;
	wchar_t wcstringBlobFilename[newsize];
	mbstowcs_s(&convertedChars, wcstringBlobFilename, origsize, szBlobFile, _TRUNCATE);

	// if special flag, delete any blob file (editing shaders mode)
	if ( gbAlwaysIgnoreShaderBlobFile == true || iShaderIndex == 0 )
		if ( DoesFileExist ( szBlobFile ) == true ) 
			DeleteFile ( szBlobFile );

	// compile if blob file does not exist
	bool bCompiledShader = false;
	ID3DBlob* pErrorBlob = NULL;
	HRESULT hRes = D3DReadFileToBlob ( wcstringBlobFilename, &g_sShaders[iShaderIndex].pBlob);
	if ( hRes != S_OK )
	{
		// proceed to compile shader blob
		D3DX11CompileFromFile(szFile, NULL, NULL, NULL, "fx_5_0", D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, 0, 0, &g_sShaders[iShaderIndex].pBlob, &pErrorBlob, NULL);
		bCompiledShader = true;
	}
	if ( pErrorBlob != NULL )
	{
		if ( g_iShowDetailedShaderErrorMessage == 0 && iShaderIndex > 0 )
		{
			char pErrorDesc[1024];
			sprintf ( pErrorDesc, "Failed to load the shader '%s'. All GameGuru shaders need to be HLSL level 5.0 or above.", szFile ); 
			MessageBox ( NULL, pErrorDesc, "DirectX", MB_OK );
		}
		else
		{
			LPSTR szError = (LPSTR)pErrorBlob->GetBufferPointer ( );
			MessageBox ( NULL, szError, "D3DX11CompileFromFile failure", MB_OK );
		}
		SAFE_RELEASE ( pErrorBlob );
		strcpy ( g_sShaders[iShaderIndex].pName, "" );
		return NULL;
	}
	else
	{
		if ( bCompiledShader == true ) 
		{
			D3DWriteBlobToFile ( g_sShaders[iShaderIndex].pBlob, wcstringBlobFilename, FALSE );
		}
	}

	// only if loading shader for real
	if ( iShaderIndex > 0 )
	{
		ID3DBlob* pFXBlob = g_sShaders[iShaderIndex].pBlob;
		if ( pFXBlob )
		{
			D3DX11CreateEffectFromMemory(pFXBlob->GetBufferPointer(), pFXBlob->GetBufferSize(), 0, m_pD3D, &g_sShaders[iShaderIndex].pEffect);
		}
		else
		{
			MessageBox ( NULL, "Effect file not found!", "D3DX11CreateEffectFromMemory failure", MB_OK );
			strcpy ( g_sShaders[iShaderIndex].pName, "" );
			return NULL;
		}
	}
	else
	{
		// iShaderIndex of zero is just to create a new shader blob file
		g_sShaders[iShaderIndex].pEffect = NULL;
	}

	// copy name into record
	strcpy ( g_sShaders[iShaderIndex].pName, szFile );

	// return effect ptr for when used by LoadEffect command
	return g_sShaders[iShaderIndex].pEffect;
}
DARKSDK bool SETUPFreeShader ( int iShaderIndex )
{
	if ( g_sShaders[iShaderIndex].pEffect )
	{
		SAFE_RELEASE ( g_sShaders[iShaderIndex].pEffect ); 
		SAFE_RELEASE ( g_sShaders[iShaderIndex].pBlob );
		return true;
	}
	else
		return false;
}
#endif

DARKSDK void SETUPLoadAllCoreShadersFIRST ( int iShowDetailedShaderErrorMessage )
{
	// For DirectX11, all core shaders loaded early for later referencing
	#ifdef DX11
	// Set Error mode
	g_iShowDetailedShaderErrorMessage = iShowDetailedShaderErrorMessage;
	// Clear shader globals
	for ( int n = 0; n < SHADERSARRAYMAX; n++ ) g_sShaders[n] = { NULL, NULL, NULL };
	// Preload common shaders (for cases where inputlayout needs creating 'prior' to shader being loaded in old system)
	SETUPLoadShader ( "effectbank\\reloaded\\sprite_basic.fx", "effectbank\\reloaded\\sprite_basic.blob", SHADERSSPRITEBASIC );
	#endif
}

DARKSDK void SETUPLoadAllCoreShadersREST ( int iShowDetailedShaderErrorMessage, int iPBRMode )
{
	// For DirectX11, all core shaders loaded early for later referencing
	#ifdef DX11
	// Preload common shaders (for cases where inputlayout needs creating 'prior' to shader being loaded in old system)
	if ( iPBRMode == 1 )
		SETUPLoadShader ( "effectbank\\reloaded\\apbr_terrain.fx", "effectbank\\reloaded\\apbr_terrain.blob", SHADERSTERRAINBASIC );
	else
		SETUPLoadShader ( "effectbank\\reloaded\\terrain_basic.fx", "effectbank\\reloaded\\terrain_basic.blob", SHADERSTERRAINBASIC );
	SETUPLoadShader ( "effectbank\\reloaded\\static_basic.fx", "effectbank\\reloaded\\static_basic.blob", SHADERSSTATICBASIC );
	SETUPLoadShader ( "effectbank\\reloaded\\shadow_basic.fx", "effectbank\\reloaded\\shadow_basic.blob", SHADERSSHADOWBASIC );
	#endif
}

DARKSDK void SETUPFreeAllCoreShaders ( void )
{
	// For DirectX11, all core shaders released here (perhaps redundant as do this end exit app)
	#ifdef DX11
	SETUPFreeShader ( SHADERSTERRAINBASIC );
	SETUPFreeShader ( SHADERSSPRITEBASIC );
	#endif
}

void SetRenderAndDepthTarget ( LPGGRENDERTARGETVIEW render, LPGGDEPTHSTENCILVIEW depth )
{
	#ifdef DX11
	if ( render ) m_pImmediateContext->OMSetRenderTargets ( 1, &render, depth );
	g_pGlob->pCurrentRenderView = render;
	g_pGlob->pCurrentDepthView = depth;
	#endif
}
