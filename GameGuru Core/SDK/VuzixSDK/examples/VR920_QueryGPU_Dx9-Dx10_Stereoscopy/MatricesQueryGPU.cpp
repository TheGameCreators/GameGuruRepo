//-----------------------------------------------------------------------------
// File: MatricesQueryGPU.cpp
//  Date: 8.23.2008
//      Addition of Internal Driver filtering support, if driver supported.
//      Addition of Windowed mode processing.
//		Fixed hot-plugging detection for tracking.
//      Mention of "BackBuffer" locking alternate method to support GPU syncronization.
//  Date: 4.17.2008
//		Addition of Off-Axis projections for stereoscopic rendering.
//		Addition of tracker filtering, As a user option.
//		Altered Key-command settings.
//	Date: 3.06.2008 
//		Definitive method for syncronizing FreezeFrame Stereoscopy from UserMode.
//		1: Queries to access status on the GPU pipeline will provide for higher throughput 
//		   While achieving Syncronized left/right Frame updates with the VR920s video engine.
//		2: Prior to calling the Present(...) method the application must be certain the 
//		   frame will be scanned out the VR920's vga adapter on the next Vsync period 
//		   after the call to IWRSTEREO_SetLR(...) is executed.
//		
//	Stereoscopy Rules:
//	1: Never leave the VR920 in FreezeFrame stereoscopy prior to exiting the application.
//		Unless it is the intention of the developer.
//  2: During long periods of non-scene rendering times. 
//		Consider placing the VR920 into mono mode. 
//		Ex: scene loading period...
//
//	Head Tracking Rules:
//	1: Always provide for a Non-Filtered method of head tracking.
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "Resource.h"
#define IWEAR_ONETIME_DEFINITIONS
#include <iWearSDK.h>

#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#define RECCALCULATE_FPS		60 // Number of frames to elapse before recompute of FPS
#define EXPANSION_COUNT			7 // Number of V.Buffers / Triangles to increment / decrement.
#define OUTER_SIZE				8.0f	// Define size of tunnel vision.
#define TUNNEL_ROTATION			D3DX_PI / 2.0f // Lay tunnel down lengthwise.
#define SEPARATION_CHANGE		0.01f
#define MINIMUM_SEPARATION		0.01f
#define DEFAULT_SEPARATION		0.35f
#define FOCAL_LENGTH_CHANGE		0.5f
#define DEFAULT_FOCAL_LENGTH	10.0f
#define MINIMUM_FOCAL_LENGTH	4.0f
#define PRESENTATION_QUEUE_SIZE	2
#define Z_LIMIT					40.0f
#define Z_CHANGE				0.1f;
#define FONT_SIZE				16
#define YFONT_SPACING			20
#define XMISC_START				10
#define	YMISC_START				290
#define CLASS_NAME				"D3D Stereoscopy"
#define WINDOW_NAME				"D3D: VR920 Stereoscopy"
#define EXIT_STEREOSCOPIC_MODE	E_FAIL

enum						{ IWR_NOT_CONNECTED=-1, IWR_IS_CONNECTED };
enum						{ LEFT_EYE=0, RIGHT_EYE, MONO_EYES };
enum						{ NO_FILTERING=0, APPLICATION_METHOD, DRIVER_METHOD };
HRESULT						InititializeDirect3DResources( HWND hWnd );
UINT						IWRFindVR920Adapter( LPDIRECT3D9 g_pD3D );
UINT						IWRIsVR920Connected( bool *PrimaryDevice );
// Provide for a filtering mechanism on the VR920 tracker readings.
extern void					IWRFilterTracking( long *yaw, long *pitch, long *roll );

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
float					g_EyeSeparation = DEFAULT_SEPARATION;// Intraocular Distance: aka, Distance between left and right cameras.
float					g_FocalLength	= DEFAULT_FOCAL_LENGTH;	// Screen projection plane: aka, focal length(distance to front of virtual screen).

LPDIRECT3D9				g_pD3D			= NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice	= NULL;	// Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB_Tunnel	= NULL;	// Buffer to hold walls
LPDIRECT3DVERTEXBUFFER9 g_pVB			= NULL;			// Buffer to hold cylinders
LPDIRECT3DQUERY9		g_pLeftEyeQuery	= NULL;
LPDIRECT3DQUERY9		g_pRightEyeQuery= NULL;
ID3DXFont*				g_pFont			= NULL;
D3DDISPLAYMODE			g_d3ddm;
D3DPRESENT_PARAMETERS	g_d3dpp;
D3DCAPS9				g_D3Dcaps;
TCHAR					g_strFont[LF_FACESIZE];
UINT					g_VR920Adapter	= D3DADAPTER_DEFAULT;
bool					g_FullScreen	= false;
bool					g_zDirection	= false;
bool					g_BenchMarking	= false;
bool					g_RenderStats	= false;
bool					g_tracking		= false;
bool					g_Pause			= false;
bool					g_StereoEnabled	= true;
int						g_Filtering		= APPLICATION_METHOD;	
UINT					g_bottomLine	= 0;
UINT					g_ViewPortWidth;
UINT					g_ViewPortHeight;
HANDLE					g_StereoHandle	= INVALID_HANDLE_VALUE;
WORD                    g_ProductID = 0;
DWORD					g_ProductDetails = 0;
int						g_ProdSubID = 0;
int						g_ProdAspect = 0;
int						g_ProdTrackerStyle = 0;
UINT					g_iTime;
float					g_zMotion=0.0f;
D3DXVECTOR3				g_ViewVector, g_UpVector, g_RightVector;
float					g_CameraXPosition=0.0f, g_CameraYPosition=0.0f, g_CameraZPosition=-12.0f;
unsigned long			g_FrameCounter=0;
unsigned long			g_RenderingCounter=0;
long					g_VertexBuffers;
int						g_Vb_Triangles;
UINT					g_StripSize;
DWORD					g_startingPresentFrame=0;
DWORD					g_startingRenderFrame=0;
DWORD					g_StartTimer=0;
DWORD					g_StartRenderTimer=0;
float					g_fRate = 0.0f;
float					g_fRenderRate = 0.0f;
struct CUSTOMVERTEX {
    D3DXVECTOR3			position; 
    D3DCOLOR			color;    
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
//-----------------------------------------------------------------------------
// Log file for operational status and testing results. 
//-----------------------------------------------------------------------------
int		IWROut( LPCTSTR strout )
{
int		fOut;
size_t	nBytes;	
TCHAR	Outfile[MAX_PATH];
size_t	i;
	strcpy_s( Outfile, MAX_PATH, __argv[0] );
	for( i=strlen( Outfile ); i; i-- )
		if( Outfile[i] == '\\' ) break;
	strcpy_s( &Outfile[i], MAX_PATH-i, "\\VR920Stereoscopy.log" );
	if( strout == NULL ) {
		DeleteFile( Outfile );
		return 0;
		}
	if( (fOut = _open( Outfile, _O_CREAT | _O_BINARY | _O_APPEND | _O_WRONLY, _S_IREAD | _S_IWRITE )) != -1 ) {
		nBytes = strlen( strout );
		if((_write( fOut, strout, (unsigned int)nBytes )) == -1 )
			nBytes = nBytes;// Error writing to log file.
		_close( fOut );		 
	}
	else
		; // Open for log file failed.		
	return 0;
}

//-----------------------------------------------------------------------------
// Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
HRESULT		hr;
HDC			hDC = GetDC( NULL );
int			nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
int			nHeight = -FONT_SIZE * nLogPixelsY / 72;
DWORD		Behavior=0;

	ReleaseDC( NULL, hDC );

    // Cut font height in half for Wrap920 SxS
    if (g_ProductID == IWR_PROD_WRAP920) {
        nHeight *= 0.63;
    }
    // Create the D3D object.
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) ){
		IWROut("LOG: Failed to Direct3DCreate9\n");
        return E_FAIL;
		}
	// Return failure if no VR920 adapter is found.
	g_VR920Adapter = IWRFindVR920Adapter( g_pD3D );
	if( g_VR920Adapter == IWR_NOT_CONNECTED ) {
		IWROut("LOG: WARNING: Failed to find vr920 adapter, Using default\n");
		g_VR920Adapter = D3DADAPTER_DEFAULT;
		}
	// Get the Mode the VR920 is configured for.
	hr = g_pD3D->GetAdapterDisplayMode( g_VR920Adapter, &g_d3ddm );
	if( FAILED(hr)) {
		IWROut("LOG: Failed to get display mode\n");
		return hr;
		}
	hr = g_pD3D->GetDeviceCaps( g_VR920Adapter, D3DDEVTYPE_HAL, &g_D3Dcaps );
	if( FAILED(hr)) {
		IWROut("LOG: Failed to GetDeviceCaps\n");
		return hr;
		}
     // Set up the structure used to create the D3DDevice
    ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );
	if( g_FullScreen ) {
		// Full Screen mode.
		g_d3dpp.BackBufferFormat	= g_d3ddm.Format;
		// Enable triple buffering.
		g_d3dpp.BackBufferCount		= PRESENTATION_QUEUE_SIZE;
		// MUST wait for VSync to enable the presentation que.
		g_d3dpp.PresentationInterval= D3DPRESENT_INTERVAL_ONE;
		g_d3dpp.Windowed			= FALSE;
		g_d3dpp.SwapEffect			= D3DSWAPEFFECT_FLIP;
		g_d3dpp.hDeviceWindow		= hWnd;
		g_d3dpp.BackBufferWidth		= g_d3ddm.Width;
		g_d3dpp.BackBufferHeight	= g_d3ddm.Height;
		g_d3dpp.FullScreen_RefreshRateInHz= g_d3ddm.RefreshRate;
		}
	else {
		g_d3dpp.Windowed			= TRUE;
		g_d3dpp.hDeviceWindow		= hWnd;
		g_d3dpp.SwapEffect			= D3DSWAPEFFECT_COPY;
		g_d3dpp.BackBufferFormat	= g_d3ddm.Format;
		// IF Polling VSync is available:
		if( g_D3Dcaps.Caps & D3DCAPS_READ_SCANLINE )
			g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		else {
			// IF Polling VSync is NOT available:
			IWROut( "LOG: Polling v-Sync not available.\n");
			g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
			}
		}
    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW then use SWVP.
    if( (g_D3Dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 )
        Behavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    else
        Behavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    // Work on a pure device if supported.
    if ((g_D3Dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) && 
        (Behavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) )
        Behavior |= D3DCREATE_PUREDEVICE;
// Enable locking. to sync GPU with CPU and VR920 frame sync.
// Left as an option for the developer. 
// All Queries must be removed in favor of locking the backbuffer prior to present.
// g_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	// Create device with a ZBuffer.
	g_d3dpp.EnableAutoDepthStencil  = TRUE;
	g_d3dpp.AutoDepthStencilFormat  = D3DFMT_D16;    
	// Attempt to create the device.
    if( FAILED( g_pD3D->CreateDevice( g_VR920Adapter, D3DDEVTYPE_HAL, hWnd,
		Behavior, &g_d3dpp, &g_pd3dDevice ) ) ) {
		IWROut("LOG: Failed to create DX9 Device\n");
        return E_FAIL;
	    }
	// Remember Viewport, could change during window re-sizing.
	g_ViewPortWidth = g_d3ddm.Width;
	g_ViewPortHeight= g_d3ddm.Height; 
	// Initialize fields to reflect proper rendering / presenting setup.
	g_VertexBuffers		= EXPANSION_COUNT * 4;
	g_Vb_Triangles		= EXPANSION_COUNT * 5;
	g_StartTimer		= timeGetTime();
	g_StartRenderTimer  = g_StartTimer;
	// Create a font for text status output.
    hr = D3DXCreateFont( g_pd3dDevice,            // D3D device
                         nHeight,               // Height
                         0,                     // Width
                         FW_BOLD,               // Weight
                         1,                     // MipLevels, 0 = autogen mipmaps
                         FALSE,                 // Italic
                         DEFAULT_CHARSET,       // CharSet
                         OUT_DEFAULT_PRECIS,    // OutputPrecision
                         DEFAULT_QUALITY,       // Quality
                         DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
                         g_strFont,              // pFaceName
                         &g_pFont);              // ppFont
	if( FAILED( hr ) ){
		IWROut("LOG: Failed to create font\n");
        return hr;
		}
    // Turn off culling, so we see the front and back of the triangle
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    // Turn off D3D lighting, since we are providing our own vertex colors
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    // Turn on the zbuffer
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	hr = g_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &g_pLeftEyeQuery);
	if( FAILED( hr ) ) {
		IWROut("LOG: LeftEye CreateQuery failed...\n");
        return hr;
		}
	hr = g_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &g_pRightEyeQuery);
	if( FAILED( hr ) ) {
		IWROut("LOG: RightEye CreateQuery failed...\n");
        return hr;
		}
    return S_OK;
}

//-----------------------------------------------------------------------------
// Creates the scene geometry
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	// Remove old VB.
	SAFE_RELEASE( g_pVB );
	SAFE_RELEASE( g_pVB_Tunnel );
    // Create the vertex buffer.
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( g_Vb_Triangles * 2 * sizeof(CUSTOMVERTEX), 
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL ) ) ) {
		IWROut("LOG: Failed to create 1st Vertexbuffer\n");
        return E_FAIL;
	    }
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( g_Vb_Triangles * 2 * sizeof(CUSTOMVERTEX), 
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB_Tunnel, NULL ) ) ) {
		IWROut("LOG: Failed to create 2nd Vertexbuffer\n");
        return E_FAIL;
	    }
    // Fill the vertex buffer.
    CUSTOMVERTEX* pVertices;
	if( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) ){
		IWROut("LOG: Failed to lock 1st Vertexbuffer\n");
        return E_FAIL;
		}
	for( int i=0; i < g_Vb_Triangles; i++ ) {
		FLOAT theta = (2*D3DX_PI*i) / (g_Vb_Triangles-1);
		pVertices[2*i+0].position = D3DXVECTOR3( sinf(theta),-3.0f, cosf(theta) );
		pVertices[2*i+0].color    = 0xffff0000;
		pVertices[2*i+1].position = D3DXVECTOR3( sinf(theta), 3.0f, cosf(theta) );
		pVertices[2*i+1].color    = 0xff0000ff;
		}
    g_pVB->Unlock();
	g_StripSize = 2 * g_Vb_Triangles - 2;
	// Build tunnel for greater effect.
	if( FAILED( g_pVB_Tunnel->Lock( 0, 0, (void**)&pVertices, 0 ) ) ){
		IWROut("LOG: Failed to Lock 2nd Vertexbuffer\n");
        return E_FAIL;
		}
	for( int i=0; i < g_Vb_Triangles; i++ ) {
		FLOAT theta = (2*D3DX_PI*i) / (g_Vb_Triangles-1);
		pVertices[2*i+0].position = D3DXVECTOR3( sinf(theta) * OUTER_SIZE,-8.0f * OUTER_SIZE, 1.0f /*cosf(theta) * OUTER_SIZE*/  );
		pVertices[2*i+0].color    = 0xff00ff00;
		pVertices[2*i+1].position = D3DXVECTOR3( sinf(theta) * OUTER_SIZE, 32.0f * OUTER_SIZE, 1.0f /*cosf(theta) * OUTER_SIZE*/ );
		pVertices[2*i+1].color    = 0xff00ff00;
		}
    g_pVB_Tunnel->Unlock();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID	Cleanup()
{
	SAFE_RELEASE( g_pFont );
	SAFE_RELEASE( g_pVB );
	SAFE_RELEASE( g_pVB_Tunnel );
	SAFE_RELEASE( g_pLeftEyeQuery );
	SAFE_RELEASE( g_pRightEyeQuery );
    SAFE_RELEASE( g_pd3dDevice );
    SAFE_RELEASE( g_pD3D );
}

//-----------------------------------------------------------------------------
//  Polls iWear Head tracker and updates animation sequences.
//	Provides for filtering of tracker readings.
//-----------------------------------------------------------------------------
VOID	UpdateOrientationAndAnimate( void )
{
D3DXMATRIXA16	matOrient;
HRESULT			iwr_status;
float			fYaw=0.0f, fPitch=0.0f, fRoll=0.0f;
long			Roll=0,Yaw=0, Pitch=0;
	// Get iWear tracking yaw, pitch, roll
	iwr_status = IWRGetTracking( &Yaw, &Pitch, &Roll );  
	if(	g_BenchMarking || iwr_status != IWR_OK ){
		// iWear tracker could be OFFLine: just inform user or wait until plugged in...
		Yaw = Pitch = Roll = 0;
		// Attempt to re-establish communications with the VR920.
		IWROpenTracker();	
		}
	// Always provide for a means to disable filtering;
	if( g_Filtering == APPLICATION_METHOD ) 
		IWRFilterTracking( &Yaw, &Pitch, &Roll );
	fPitch =  (float)Pitch * IWR_RAWTORAD;
	fYaw   =  (float)Yaw * IWR_RAWTORAD;
	fRoll  =  (float)Roll * IWR_RAWTORAD;
	// Update animation parameters.
	if( !g_Pause ) {
		g_iTime = timeGetTime() % 10000;
		if( g_zDirection ){
			g_zMotion += Z_CHANGE;
			if( g_zMotion > Z_LIMIT )
				g_zDirection = !g_zDirection;
			}
		else {
			g_zMotion -= Z_CHANGE;
			if( g_zMotion < -Z_LIMIT )
				g_zDirection = !g_zDirection;
			}
		}
	// Recompute the orientation vectors.
	g_ViewVector	= D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	g_UpVector		= D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	g_RightVector	= D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
	// Reorient View vectors for roll, pitch and yaw.
	if( g_tracking ) {
		// Add Yaw angle.  (About the up vector)
		// Compute the new viewvector.
		// Transform the right vector.
		D3DXMatrixRotationAxis( &matOrient, &g_UpVector, -fYaw );
		D3DXVec3TransformCoord( &g_ViewVector, &g_ViewVector, &matOrient );
		D3DXVec3TransformCoord( &g_RightVector, &g_RightVector, &matOrient );
		// Add pitch angle.  (About the right vector)
		// Compute the new upvector.
		// Transform the View vector.
		D3DXMatrixRotationAxis( &matOrient, &g_RightVector, -fPitch );
		D3DXVec3TransformCoord( &g_UpVector, &g_UpVector, &matOrient );
		D3DXVec3TransformCoord( &g_ViewVector, &g_ViewVector, &matOrient );
		// Add roll angle.   (About the view vector)
		// Compute the new UpVector.
		// Transform the Right vector.
		D3DXMatrixRotationAxis( &matOrient, &g_ViewVector, -fRoll );
		D3DXVec3TransformCoord( &g_UpVector, &g_UpVector, &matOrient );
		D3DXVec3TransformCoord( &g_RightVector, &g_RightVector, &matOrient );
		}
}

//-----------------------------------------------------------------------------
// Sets up the view, and projection transform matrices.
//		Uses preferred parallel slide of camera for stereoscopy rendering.
//      Complimented with an off-axis projection.
//-----------------------------------------------------------------------------
VOID	SetViewingFrustum( LPDIRECT3DDEVICE9 pd3dDevice, int Eye )
{
D3DXVECTOR3		vEyePt, vLookatPt, vStereoAdj;
D3DXMATRIXA16	matProj, matView;
float			fovy	= 45.0;                 //field of view in y-axis
float			nearZ	= 1.0;					//near clipping plane
float			farZ	= 1000.0;				//far clipping plane
float			aspect	= float(g_ViewPortWidth) / float(g_ViewPortHeight);//screen aspect ratio
float			top		= nearZ * tan( D3DXToRadian(fovy) / 2 );//sets top of frustum based on fovy and near clipping plane
float			right	= aspect* top, left;	//sets right of frustum based on aspect ratio
float			frustumshift = (g_EyeSeparation / 2) * nearZ / g_FocalLength;
	// Construct view matrix from orientation vectors.
	vEyePt		= D3DXVECTOR3( g_CameraXPosition, g_CameraYPosition, g_CameraZPosition );
	vLookatPt	= vEyePt + g_ViewVector;
	// Slide the camera along the Right Vector for Stereoscopic generation.
	switch( Eye ) {
		case LEFT_EYE:
			left = -right + frustumshift;
			right = right + frustumshift;
			// Vector adjust:
			vStereoAdj	= g_RightVector * g_EyeSeparation;
			vEyePt		-= vStereoAdj;
			vLookatPt	-= vStereoAdj;
		break;
		case RIGHT_EYE:
			left = -right - frustumshift;
			right = right - frustumshift;
			// Vector adjust:
			vStereoAdj	= g_RightVector * g_EyeSeparation;
			vEyePt		+= vStereoAdj;
			vLookatPt	+= vStereoAdj; 
		break;
		case MONO_EYES:
			left = -right;
		break;
		}
	// Projection matrix set.
	D3DXMatrixPerspectiveOffCenterLH( &matProj, left, right, -top, top, nearZ, farZ ); 
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
	// View matrix set.
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &g_UpVector );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
}

//-----------------------------------------------------------------------------
// Move and Rotate cylinders in symetric order.
//-----------------------------------------------------------------------------
VOID	ChangeWorldMatrix( LPDIRECT3DDEVICE9 pd3dDevice, long maxObjects, UINT iTime, int object, float zMotion )
{
D3DXMATRIXA16	matWorld, matTranslation, matRotate;
float			fAngle = iTime * (2.0f * D3DX_PI) / 10000.0f;
float			x, y, z, zMax;

	zMax = (float)(maxObjects / EXPANSION_COUNT) * 20.0f;
	x = (float)((object % EXPANSION_COUNT) - (EXPANSION_COUNT/2)) * 3.0f;
	z = zMax - (float)(object / EXPANSION_COUNT) * 20.0f - zMotion;
	y = -4.0f;
	D3DXMatrixTranslation( &matTranslation, x, y, z );
    D3DXMatrixRotationX( &matRotate, fAngle );
    D3DXMatrixRotationY( &matWorld, D3DX_PI / 2.0f );
	D3DXMatrixMultiply( &matWorld, &matWorld, &matRotate );
	D3DXMatrixMultiply( &matWorld, &matWorld, &matTranslation );

	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
}

//-----------------------------------------------------------------------------
// Put up outer walls to provide better stereo prespective.
//-----------------------------------------------------------------------------
HRESULT		RenderTunnelWorldMatrix( )
{
D3DXMATRIXA16	matWorld, matTranslation;
	if( g_BenchMarking ) {
	    // Turn on wireframe
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		D3DXMatrixTranslation( &matTranslation, 0.0f, -4.0f, 0.0f );
		}
	else
		D3DXMatrixTranslation( &matTranslation, 0.0f, -6.0f, 0.0f );
	// floor
    D3DXMatrixRotationX( &matWorld, TUNNEL_ROTATION );
	D3DXMatrixMultiply( &matWorld, &matWorld, &matTranslation );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	// Render the tunnel.
	g_pd3dDevice->SetStreamSource( 0, g_pVB_Tunnel, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, g_StripSize );
	// Left wall
	D3DXMatrixTranslation( &matTranslation, -40.0f, 0.0f, 10.0f );
    D3DXMatrixRotationZ( &matWorld, TUNNEL_ROTATION );
	D3DXMatrixMultiply( &matWorld, &matWorld, &matTranslation );
    D3DXMatrixRotationY( &matTranslation, TUNNEL_ROTATION );
	D3DXMatrixMultiply( &matWorld, &matWorld, &matTranslation );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	// Render the tunnel.
	g_pd3dDevice->SetStreamSource( 0, g_pVB_Tunnel, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, g_StripSize );
	// Right wall
	D3DXMatrixTranslation( &matTranslation, 40.0f, 0.0f, 10.0f );
    D3DXMatrixRotationZ( &matWorld, -TUNNEL_ROTATION );
	D3DXMatrixMultiply( &matWorld, &matWorld, &matTranslation );
    D3DXMatrixRotationY( &matTranslation, -TUNNEL_ROTATION );
	D3DXMatrixMultiply( &matWorld, &matWorld, &matTranslation );
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	// Render the tunnel.
	g_pd3dDevice->SetStreamSource( 0, g_pVB_Tunnel, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, g_StripSize );
	if( g_BenchMarking ) 
	    // Turn on Solidfill.
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	return D3D_OK;
}

//--------------------------------------------------------------------------------------
// Supplement from DXUTmisc.cpp: see directX sdk common sources.
// Returns the string for the given D3DFORMAT.
//--------------------------------------------------------------------------------------
LPCTSTR DXUTD3DFormatToString( D3DFORMAT format )
{
TCHAR* pstr = NULL;
	switch( format ) {
	case D3DFMT_UNKNOWN:         pstr = "D3DFMT_UNKNOWN"; break;
	case D3DFMT_R8G8B8:          pstr = "D3DFMT_R8G8B8"; break;
	case D3DFMT_A8R8G8B8:        pstr = "D3DFMT_A8R8G8B8"; break;
	case D3DFMT_X8R8G8B8:        pstr = "D3DFMT_X8R8G8B8"; break;
	case D3DFMT_R5G6B5:          pstr = "D3DFMT_R5G6B5"; break;
	case D3DFMT_X1R5G5B5:        pstr = "D3DFMT_X1R5G5B5"; break;
	case D3DFMT_A1R5G5B5:        pstr = "D3DFMT_A1R5G5B5"; break;
	case D3DFMT_A4R4G4B4:        pstr = "D3DFMT_A4R4G4B4"; break;
	case D3DFMT_R3G3B2:          pstr = "D3DFMT_R3G3B2"; break;
	case D3DFMT_A8:              pstr = "D3DFMT_A8"; break;
	case D3DFMT_A8R3G3B2:        pstr = "D3DFMT_A8R3G3B2"; break;
	case D3DFMT_X4R4G4B4:        pstr = "D3DFMT_X4R4G4B4"; break;
	case D3DFMT_A2B10G10R10:     pstr = "D3DFMT_A2B10G10R10"; break;
	case D3DFMT_A8B8G8R8:        pstr = "D3DFMT_A8B8G8R8"; break;
	case D3DFMT_X8B8G8R8:        pstr = "D3DFMT_X8B8G8R8"; break;
	case D3DFMT_G16R16:          pstr = "D3DFMT_G16R16"; break;
	case D3DFMT_A2R10G10B10:     pstr = "D3DFMT_A2R10G10B10"; break;
	case D3DFMT_A16B16G16R16:    pstr = "D3DFMT_A16B16G16R16"; break;
	case D3DFMT_A8P8:            pstr = "D3DFMT_A8P8"; break;
	case D3DFMT_P8:              pstr = "D3DFMT_P8"; break;
	case D3DFMT_L8:              pstr = "D3DFMT_L8"; break;
	case D3DFMT_A8L8:            pstr = "D3DFMT_A8L8"; break;
	case D3DFMT_A4L4:            pstr = "D3DFMT_A4L4"; break;
	case D3DFMT_V8U8:            pstr = "D3DFMT_V8U8"; break;
	case D3DFMT_L6V5U5:          pstr = "D3DFMT_L6V5U5"; break;
	case D3DFMT_X8L8V8U8:        pstr = "D3DFMT_X8L8V8U8"; break;
	case D3DFMT_Q8W8V8U8:        pstr = "D3DFMT_Q8W8V8U8"; break;
	case D3DFMT_V16U16:          pstr = "D3DFMT_V16U16"; break;
	case D3DFMT_A2W10V10U10:     pstr = "D3DFMT_A2W10V10U10"; break;
	case D3DFMT_UYVY:            pstr = "D3DFMT_UYVY"; break;
	case D3DFMT_YUY2:            pstr = "D3DFMT_YUY2"; break;
	case D3DFMT_DXT1:            pstr = "D3DFMT_DXT1"; break;
	case D3DFMT_DXT2:            pstr = "D3DFMT_DXT2"; break;
	case D3DFMT_DXT3:            pstr = "D3DFMT_DXT3"; break;
	case D3DFMT_DXT4:            pstr = "D3DFMT_DXT4"; break;
	case D3DFMT_DXT5:            pstr = "D3DFMT_DXT5"; break;
	case D3DFMT_D16_LOCKABLE:    pstr = "D3DFMT_D16_LOCKABLE"; break;
	case D3DFMT_D32:             pstr = "D3DFMT_D32"; break;
	case D3DFMT_D15S1:           pstr = "D3DFMT_D15S1"; break;
	case D3DFMT_D24S8:           pstr = "D3DFMT_D24S8"; break;
	case D3DFMT_D24X8:           pstr = "D3DFMT_D24X8"; break;
	case D3DFMT_D24X4S4:         pstr = "D3DFMT_D24X4S4"; break;
	case D3DFMT_D16:             pstr = "D3DFMT_D16"; break;
	case D3DFMT_L16:             pstr = "D3DFMT_L16"; break;
	case D3DFMT_VERTEXDATA:      pstr = "D3DFMT_VERTEXDATA"; break;
	case D3DFMT_INDEX16:         pstr = "D3DFMT_INDEX16"; break;
	case D3DFMT_INDEX32:         pstr = "D3DFMT_INDEX32"; break;
	case D3DFMT_Q16W16V16U16:    pstr = "D3DFMT_Q16W16V16U16"; break;
	case D3DFMT_MULTI2_ARGB8:    pstr = "D3DFMT_MULTI2_ARGB8"; break;
	case D3DFMT_R16F:            pstr = "D3DFMT_R16F"; break;
	case D3DFMT_G16R16F:         pstr = "D3DFMT_G16R16F"; break;
	case D3DFMT_A16B16G16R16F:   pstr = "D3DFMT_A16B16G16R16F"; break;
	case D3DFMT_R32F:            pstr = "D3DFMT_R32F"; break;
	case D3DFMT_G32R32F:         pstr = "D3DFMT_G32R32F"; break;
	case D3DFMT_A32B32G32R32F:   pstr = "D3DFMT_A32B32G32R32F"; break;
	case D3DFMT_CxV8U8:          pstr = "D3DFMT_CxV8U8"; break;
	default:                     pstr = "Unknown format"; break;
	}
	return pstr;
}

//-----------------------------------------------------------------------------
// Show stats and other information.
//-----------------------------------------------------------------------------
VOID	RenderFrameRates( int Eye )
{
#define INFO_FIELDS	15
TCHAR		*str_info[INFO_FIELDS] = {
	"STATUS IN LEFT EYE ONLY? (Both eyes in mono mode)",
	"   Left/Right = +/- Eye separation",
	"   Up/Down = +/- Focal length",
	"   PgUp/PgDown = +/- # of Triangles",
	"   Ins/Del = +/- # of Objects",
	"   Enter = Toggle Stereo/Mono",
	"   ALT-Enter = Toggle FullScreen/Windowed modes",
	"   t = Toggle tracking",
	"   f = Cycle filtered tracking: (none,application,driver)",
	"   p = Pause animation",
	"   ESC,q = Exit application",
	"   b = Benchmarking mode",
	"   Space = Reset Defaults",
	"   F1 = Toggle status info",
	"   F2,F3,F4,F5 = Set camera viewpoint",
};
RECT		rc0, rc1, rc2, rc3, rc4, rc4b, rc5;
TCHAR		str[ MAX_PATH ];
DWORD		totalTime;
int			i;
D3DCOLOR	InfoColor = D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f );
D3DCOLOR	TitleColor = D3DXCOLOR( 0.0f, 0.0f, 1.0f, 1.0f );
	if( g_RenderStats ) {
		SetRect( &rc0, XMISC_START, YMISC_START + YFONT_SPACING*1, 0, 0 );        
		SetRect( &rc1, XMISC_START, YMISC_START + YFONT_SPACING*2, 0, 0 );        
		SetRect( &rc2, XMISC_START, YMISC_START + YFONT_SPACING*3, 0, 0 );        
		SetRect( &rc3, XMISC_START, YMISC_START + YFONT_SPACING*4, 0, 0 );        
		SetRect( &rc4, XMISC_START, YMISC_START + YFONT_SPACING*5, 0, 0 );        
		SetRect( &rc4b,XMISC_START, YMISC_START + YFONT_SPACING*6, 0, 0 );        
		SetRect( &rc5, XMISC_START, 0, 0, 0 );        
		// Every 60 frames recompute frame rates and restart frame rate timers.
		if( !(g_RenderingCounter % RECCALCULATE_FPS) ) {
			totalTime = timeGetTime() - g_StartRenderTimer;
			g_fRenderRate = (float)(g_RenderingCounter - g_startingRenderFrame) / (float)totalTime * 1000.0f;
			g_startingRenderFrame  = g_RenderingCounter;
			g_StartRenderTimer = timeGetTime();
			}
		for( i=0; i < INFO_FIELDS; i++ ) {
			g_pFont->DrawText( NULL, str_info[i], -1, &rc5, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
			rc5.top    += YFONT_SPACING;
			rc5.bottom += YFONT_SPACING;
			}
		if( g_StereoEnabled )
			sprintf_s( str, "STEREO Mode: Eye Separation:(%6.2f) Focal Length (%6.2f)", g_EyeSeparation, g_FocalLength );
		else
			sprintf_s( str, "MONO Mode:" );
		g_pFont->DrawText( NULL, str, -1, &rc0, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
		sprintf_s( str, "    Video Adapter: %d  Mode: (%dx%d,%s @%dHz)", g_VR920Adapter,g_d3ddm.Width,g_d3ddm.Height,DXUTD3DFormatToString(g_d3ddm.Format),g_d3ddm.RefreshRate );
		g_pFont->DrawText( NULL, str, -1, &rc1, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
		sprintf_s( str, "Number of VertexBuffers: %lu  # of Triangles: %ld", g_VertexBuffers, g_StripSize );
		g_pFont->DrawText( NULL, str, -1, &rc2, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
		sprintf_s( str, "    Presentation Frame: (%d) Presentation Rate: %6.2f",g_FrameCounter, g_fRate );
		g_pFont->DrawText( NULL, str, -1, &rc3, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
		sprintf_s( str, "    Rendering Frame: (%d)   Rendering Rate: %6.2f", g_RenderingCounter, g_fRenderRate );
		g_pFont->DrawText( NULL, str, -1, &rc4, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
		switch( g_Filtering ) {
			case APPLICATION_METHOD:
				sprintf_s( str, "    Filtering Method: (Application)");
			break;
			case DRIVER_METHOD:
				sprintf_s( str, "    Filtering Method: (Driver internal)");
			break;
			case NO_FILTERING:
				sprintf_s( str, "    Filtering Method: (None)");
			break;
			}
		g_pFont->DrawText( NULL, str, -1, &rc4b, DT_NOCLIP, D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f ));
	}
}

//-----------------------------------------------------------------------------
// Name: RenderWorld()
// Desc: Renders new mono/left/right eyes at application speed.
//		Rendering is Syncronous to the presentation process.
//-----------------------------------------------------------------------------
VOID	RenderWorld( int Eye )
{
    int			i;
    HRESULT		hr;
    // No device during shut down.
    if( g_pd3dDevice == NULL ) {
        IWROut( "LOG: g_pd3dDevice is NULL.\n" );
        return;
    }
    // Test the cooperative level to see if it's okay to render
    if( FAILED( hr = g_pd3dDevice->TestCooperativeLevel() ) )	{
        if( D3DERR_DEVICELOST == hr ) {
            // The device has been lost but cannot be reset at this time.  
            // So wait until it can be reset.
            IWROut( "LOG: Device is lost.\n" );
        }
        IWROut( "LOG: TestCooperativelevel failed.\n" );
        SendMessage( g_d3dpp.hDeviceWindow, WM_CLOSE, 0,0);
        return;
    }
    // Update rendering counter.
    g_RenderingCounter++;
    // Clear the backbuffer to a black color
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) ) {
        // Setup the view, and projection matrices for proper rendering.
        SetViewingFrustum( g_pd3dDevice, Eye );
        // Draw outer tunnel vision
        RenderTunnelWorldMatrix( );
        // Draw marching cylinders.
        for( i=0; i < g_VertexBuffers; i++ ) {
            ChangeWorldMatrix( g_pd3dDevice, g_VertexBuffers, g_iTime, i, g_zMotion );
            // Render the vertex buffer contents
            g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
            g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
            g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, g_StripSize );
        }
        // Only render status info when not rendering the left eye.
        //if (g_ProductID == IWR_PROD_VR920) {
            if( Eye != RIGHT_EYE ) 
                RenderFrameRates( Eye );
        //}
        // End the scene
        g_pd3dDevice->EndScene();
    }
}

//-----------------------------------------------------------------------------
// Window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
RECT	wRect;
DWORD	dwWndStyle;
   switch( msg ) {
		case WM_MOVE:
			// Acquire bottom line for windowed mode vSync.
			GetWindowRect( hWnd, &wRect );
			g_bottomLine = wRect.bottom;
			if( g_bottomLine >= g_d3ddm.Height )
				g_bottomLine = g_d3ddm.Height - 1;
		break;
		case WM_SIZE:
			switch( wParam ) {
				case SIZE_MAXIMIZED:
				case SIZE_RESTORED:
					// Acquire bottom line for windowed mode vSync.
					GetWindowRect( hWnd, &wRect );
					g_bottomLine = wRect.bottom;
					if( g_bottomLine >= g_d3ddm.Height )
						g_bottomLine = g_d3ddm.Height - 1;
					if( FAILED( InititializeDirect3DResources( hWnd ) ) ) {
						IWROut( "LOG: Fauled to resize D3D Resources.\n" );
						SendMessage( hWnd, WM_CLOSE, 0,0);
						}
					// Resize viewport settings.
					g_ViewPortWidth = wRect.right - wRect.left;
					if( g_ViewPortWidth > g_d3ddm.Width )
						g_ViewPortWidth = g_d3ddm.Width;
					g_ViewPortHeight = wRect.bottom - wRect.top;
					if( g_ViewPortHeight > g_d3ddm.Height )
						g_ViewPortHeight = g_d3ddm.Height;
				break;
			}
		break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
			switch( wParam ) {
				case 'T':// turn tracking on and off
					g_tracking = !g_tracking;
				break;
				case 'F': // Cycle filtering states.
					switch( g_Filtering ) {
						case NO_FILTERING:
							g_Filtering = APPLICATION_METHOD;
						break;
						case APPLICATION_METHOD:
							if( IWRSetFilterState != NULL ) {
								g_Filtering = DRIVER_METHOD;
								// Always provide for a means to disable filtering;
								IWRSetFilterState(TRUE);
								}
							else
								g_Filtering = NO_FILTERING;
						break;
						case DRIVER_METHOD:
							// Never get here if driver filtering is not available.
							IWRSetFilterState(FALSE);
							g_Filtering = NO_FILTERING;
						break;
						}
				break;
				case 'P':// Pause frame motion.
					g_Pause = !g_Pause;
				break;
				case 'B': // Provide benchmarking quick set.
#define BENCHMARK_TRIANGLES	1547
#define BENCHMARK_BUFFERS	49
					g_Pause			= true;
					g_BenchMarking	= true;
					g_tracking		= false;
					g_Filtering		= NO_FILTERING;
					// Turn off internal filtering during benchmarking.
					if( IWRSetFilterState ) 
						IWRSetFilterState(FALSE);
					g_zMotion		= Z_LIMIT / 3.0f;
					g_iTime			= 0;
					g_Vb_Triangles	= BENCHMARK_TRIANGLES;
					g_VertexBuffers	= BENCHMARK_BUFFERS;
					InitGeometry();
				break;
				case 'Q':
				case VK_ESCAPE:
					SendMessage( hWnd, WM_CLOSE, 0,0);
				break;
				case VK_F1: // Show help information
					g_RenderStats = !g_RenderStats;
				break;
				case VK_F2:
					g_CameraXPosition=0.0f, g_CameraYPosition=0.0f, g_CameraZPosition=-12.0f;
				break;
				case VK_F3:
					g_CameraXPosition=8.0f, g_CameraYPosition=-2.0f, g_CameraZPosition=-12.0f;
				break;
				case VK_F4:
					g_CameraXPosition=0.0f, g_CameraYPosition=10.0f, g_CameraZPosition=100.0f;
				break;
				case VK_F5:
					g_CameraXPosition=-10.0f, g_CameraYPosition=20.0f, g_CameraZPosition=0.0f;
				break;
				case VK_SPACE: // Reset defaults.
					g_EyeSeparation = DEFAULT_SEPARATION;
					g_FocalLength	= DEFAULT_FOCAL_LENGTH;	
					g_CameraXPosition=0.0f, g_CameraYPosition=0.0f, g_CameraZPosition=-12.0f;
					g_Pause			= false;
					g_BenchMarking	= false;
				break;
				case VK_RETURN: // transition into and out of fullscreen or stereoscopy mode. 
					if( GetAsyncKeyState( VK_MENU ) & 0x8000 ) {
						if ( g_ProductID != IWR_PROD_WRAP920 ){
							g_FullScreen = !g_FullScreen;
							if( g_FullScreen ) {
								dwWndStyle = WS_POPUP | WS_VISIBLE;
								SetWindowLong(hWnd, GWL_STYLE, dwWndStyle);
								SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW );
								ShowCursor( FALSE );
								}
							else {
								dwWndStyle = WS_OVERLAPPEDWINDOW;
								SetWindowLong(hWnd, GWL_STYLE, dwWndStyle);
								SetWindowPos( hWnd, HWND_NOTOPMOST, 160, 120, 640, 480, SWP_SHOWWINDOW );
								ShowCursor( TRUE );
								}
							if( !SUCCEEDED( InititializeDirect3DResources( hWnd ) ) ) {
								IWROut("LOG: Error re-allocating D3D Resources.\n");
								SendMessage( hWnd, WM_CLOSE, 0,0);
								}
						}
					}
					else {
						g_FrameCounter		= 0;
						g_RenderingCounter	= 0;
						g_startingPresentFrame= 0;
						g_startingRenderFrame= 0;
						g_StartTimer		= timeGetTime();
						g_StartRenderTimer  = g_StartTimer;
						if( g_StereoHandle != INVALID_HANDLE_VALUE ) {
							g_StereoEnabled	= !g_StereoEnabled;
							if( g_StereoEnabled ) 
								IWRSTEREO_SetStereo( g_StereoHandle, IWR_STEREO_MODE );
							else 
								IWRSTEREO_SetStereo( g_StereoHandle, IWR_MONO_MODE );
							}
						}
				break;
				case VK_LEFT: // Change Eye separation.
					g_EyeSeparation -= SEPARATION_CHANGE;
					if( g_EyeSeparation <= MINIMUM_SEPARATION ) 
						g_EyeSeparation = MINIMUM_SEPARATION;
				break;
				case VK_RIGHT:// Change Eye separation.
					g_EyeSeparation += SEPARATION_CHANGE;
				break;
				case VK_UP:// Change Focal length.
					g_FocalLength += FOCAL_LENGTH_CHANGE;
				break;
				case VK_DOWN:// Change Focal length.
					g_FocalLength -= FOCAL_LENGTH_CHANGE;
					if( g_FocalLength <= MINIMUM_FOCAL_LENGTH ) 
						g_FocalLength = MINIMUM_FOCAL_LENGTH;
				break;
				case VK_PRIOR:
					g_Vb_Triangles += EXPANSION_COUNT;
					InitGeometry();
				break;
				case VK_NEXT:
					g_Vb_Triangles -= EXPANSION_COUNT;
					if( g_Vb_Triangles < EXPANSION_COUNT )
						g_Vb_Triangles = EXPANSION_COUNT;
					InitGeometry();
				break;
				case VK_DELETE:
					g_VertexBuffers -= EXPANSION_COUNT;
					if( g_VertexBuffers <= EXPANSION_COUNT ) 
						g_VertexBuffers = EXPANSION_COUNT;
				break;
				case VK_INSERT:
					g_VertexBuffers += EXPANSION_COUNT;
				break;
			}
		break;
        case WM_DESTROY:
            Cleanup();
			if( g_StereoHandle != INVALID_HANDLE_VALUE ) {
				// Back to mono mode prior to exit.
				IWRSTEREO_SetStereo( g_StereoHandle, IWR_MONO_MODE );
				// Close all communications with the VR920 Stereo Driver.
				IWRSTEREO_Close( g_StereoHandle );
				}
			IWRFreeDll();
		    UnregisterClass( CLASS_NAME, GetModuleHandle(NULL) );
            PostQuitMessage( 0 );
        return 0;
		}
    return DefWindowProc( hWnd, msg, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Function will find the VR920 Adapter and return code if found.
//  IWR_NOT_CONNECTED:  VR920 Not found.
//  IWR_IS_CONNECTED:   VR920 is connected to an adapter in the system.
//-----------------------------------------------------------------------------
UINT		IWRIsVR920Connected( bool *PrimaryDevice )
{
DISPLAY_DEVICE	DisplayDevice, MonitorDevice;
	// Step 1: Find VR920 Device+Adapter
	*PrimaryDevice = false;
	DisplayDevice.cb = MonitorDevice.cb = sizeof( DISPLAY_DEVICE );
	for( int i=0; EnumDisplayDevices(NULL, i, &DisplayDevice, 0x0 ); i++ ) {
		// Ignore mirrored devices and only look at desktop attachments.
		if( !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) &&
			(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) ) {
			for( int j=0; EnumDisplayDevices(DisplayDevice.DeviceName, j, &MonitorDevice, 0x0 ); j++ ){
				if( (MonitorDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
					(strstr( MonitorDevice.DeviceID, "IWR0002" ) || // VR920 id
					 strstr( MonitorDevice.DeviceID, "IWR0149" ))) {// Wrap920 id
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

//-----------------------------------------------------------------------------
// Using directX root Direct3D object class to acquire the VR920s adapter proper.
//  -1:  VR920 Not found.
//  (0 - #): of adapters in system: Adapter VR920 is plugged into.Directx Ordinal.
//-----------------------------------------------------------------------------
UINT		IWRFindVR920Adapter( LPDIRECT3D9 g_pD3D )
{
UINT					AdapterCount;
D3DADAPTER_IDENTIFIER9	Adapter_Identify;
DISPLAY_DEVICE			MonitorDevice;
	// Find the DirectX method of adapter ordinal, that is the VR920's
	AdapterCount = g_pD3D->GetAdapterCount();
	MonitorDevice.cb = sizeof( DISPLAY_DEVICE );
	for( UINT i=0; i < AdapterCount; i++ ) {
		g_pD3D->GetAdapterIdentifier( i, 0, &Adapter_Identify );
		for( int j=0; EnumDisplayDevices(Adapter_Identify.DeviceName, j, &MonitorDevice, 0x0 ); j++ ) {
			if( (MonitorDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
				strstr( MonitorDevice.DeviceID, "IWR0002" ) ) {
				// Found the VR920 PnP id.
				return i;
				}
			}
		}
	return IWR_NOT_CONNECTED;
}

//-----------------------------------------------------------------------------
// Provide a means for entering and leaving fullscreen mode.
//-----------------------------------------------------------------------------
HRESULT InititializeDirect3DResources( HWND hWnd )
{
HRESULT		hr;
	// Release all prior resources, if any.
	Cleanup();
	// Re/Allocate new resources.
	hr = InitD3D( hWnd );
	if( SUCCEEDED( hr ) ) 
		hr = InitGeometry();
	return hr;
}

//-----------------------------------------------------------------------------
// Provide for a method in (Windowed Mode) to poll the adapters Vertical Sync function.
//  This is a requirement for the VR920s VGA to Frame syncronization process AND
//  MUST be available for highest possible performance in windowed mode.
//-----------------------------------------------------------------------------
HRESULT	D3DWindowedWaitForVsync( void )
{
D3DRASTER_STATUS	rStatus;
HRESULT				hr=E_FAIL;
	if( g_D3Dcaps.Caps & D3DCAPS_READ_SCANLINE ) {
		// IF Polling VSync is available:
		hr = g_pd3dDevice->GetRasterStatus( 0, &rStatus );
		if( hr == S_OK ) {
			// In rare case when in vblank, Wait for vblank to drop.
			while( rStatus.InVBlank )
				g_pd3dDevice->GetRasterStatus( 0,&rStatus );
			while( rStatus.ScanLine < g_bottomLine ){
				g_pd3dDevice->GetRasterStatus( 0,&rStatus );
				// if scan ever crossed vblank again; break. could be issue with window transitioning modes.
				if( rStatus.InVBlank ) break;
			}
		}
	}
	// IF Polling VSync is NOT available:
	if( hr != S_OK )
		Sleep( 10 );
	return S_OK;
}

//-----------------------------------------------------------------------------
// Provide for a process syncronize a rendered left or right eye to the VR920s frame
//  Buffers.  Common to both Windowed and FullScreen modes.
//-----------------------------------------------------------------------------
HRESULT	IWRSyncronizeEye( int Eye )
{
    HRESULT		hr=S_OK;
    TCHAR		str[ MAX_PATH ];
    // Wait for acknowledgement from previous frame, to present a new left eye.
    IWRSTEREO_WaitForAck( g_StereoHandle, Eye );
    // Windowed mode: Wait for the start of Vsync prior to presenting.
    if( !g_FullScreen ) 
        D3DWindowedWaitForVsync();
    // Ensure GPU is ready. 
    // If using the locking method nows the time to wait for a lock on the backbuffer.

    // Put the frame in the queue(full screen). or immediate copy(windowed mode).
    hr = g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
    if( FAILED(hr) ) {
        // Critical error detected.
        if( hr == D3DERR_DEVICELOST )
            sprintf_s( str, MAX_PATH, "iWear: Critical error(%x) DeviceLost.\n", hr );
        else
            sprintf_s( str, MAX_PATH, "iWear: Critical error(%x) presenting frames.\n", hr );
        IWROut( str );
        // Exit StereoScopic rendering mode
        return EXIT_STEREOSCOPIC_MODE;
    }
    // If using the Query GPU method. wait for GPU now.
    if( Eye == LEFT_EYE ) {
        // Wait for Left eye GPU status ok.
        while(S_FALSE == g_pLeftEyeQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ))
            ;//IWROut("LOG: Waiting on left eye flush\n");
    }
    else {
        // Wait for Right eye GPU status ok.
        while(S_FALSE == g_pRightEyeQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ))
            ;//IWROut("LOG: Waiting on right eye flush\n");
    }
    // Signal to the VR920 the next eyes frame is available
    //  AND: Will scan out on the NEXT Vsync interval.
    if( !IWRSTEREO_SetLR( g_StereoHandle, Eye ) ) {
        sprintf_s( str, MAX_PATH, "iWear: Critical error: Failed communications with iWearStereo Driver.\n", hr );
        IWROut( str );
        return EXIT_STEREOSCOPIC_MODE;
    }
    return S_OK;
}

//-----------------------------------------------------------------------------
// Provide for a process on how to, support the VR920's USB stereoscopic process.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    HRESULT		iwr_status;
    DWORD		totalTime;
    bool		IsPrimary;
    HWND		hWnd;
    // Protect from multiple instances.
    if( hWnd = FindWindow( CLASS_NAME, WINDOW_NAME ) ) {
        SetForegroundWindow( hWnd );
        return 0;
    }
    // Reset Log file testing.
    IWROut( NULL );
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL, CLASS_NAME, NULL };
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClassEx( &wc );

    // Open the VR920's tracker driver.
    iwr_status = IWRLoadDll();
    if( iwr_status != IWR_OK ) {
        MessageBox( NULL, "NO VR920 iwrdriver support", "VR920 DRIVER ERROR", MB_OK );
        return 0;
    }
    // Get the ID of the hardware and its capabilities
    g_ProductID = IWRGetProductID();
	g_ProductDetails = IWRGetProductDetails();

	g_ProdSubID = IWR_GET_SUBID(g_ProductDetails);
	g_ProdAspect = IWR_GET_ASPECT_RATIO(g_ProductDetails);
	g_ProdTrackerStyle = IWR_GET_TRACKER_STYLE(g_ProductDetails);

	// Open a handle to the VR920's stereo driver.
    g_StereoHandle = IWRSTEREO_Open();
    if( g_StereoHandle == INVALID_HANDLE_VALUE ) {
        if (GetLastError() == ERROR_INVALID_FUNCTION) {
            MessageBox( NULL, "Your VR920 firmware does not support stereoscopy.  Please update your firmware.", "VR920 STEREO WARNING", MB_OK );
            g_StereoEnabled	= false;
        }
        else {
            MessageBox( NULL, "NO VR920 Stereo Driver handle", "VR920 STEREO ERROR", MB_OK );
            return 0;
        }	
    }
    
    // Determine if the VR920 or Wrap920 is plugged into a Video Adapter.
    /*if( IWRIsVR920Connected( &IsPrimary ) == IWR_NOT_CONNECTED ) {
        iwr_status = MessageBox( NULL, "The VR920 does not appear to be connected to an accessable video adapter.\n\n YES= The VR920 is connected to the default adapter?\n NO= Exit, to check the VR920 is plugged into an adapter and is connected to my desktop?", "VR920 Adapter WARNING", MB_YESNO );
        if( iwr_status == IDNO ){
            DestroyWindow( hWnd );
            return 0;
        }
    }*/
    


    // Create the application's window
    if (g_ProductID == IWR_PROD_WRAP920) g_FullScreen = true;
    if( g_FullScreen ) 
        hWnd = CreateWindow( CLASS_NAME, WINDOW_NAME,
        WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, GetDesktopWindow(), NULL, wc.hInstance, NULL );
    else
        hWnd = CreateWindow( CLASS_NAME, WINDOW_NAME,
        WS_OVERLAPPEDWINDOW, 160, 120, 640, 480, GetDesktopWindow(), NULL, wc.hInstance, NULL );

    if( SUCCEEDED( InititializeDirect3DResources( hWnd ) ) ) {
        IWROut("LOG: Begin Rendering process\n");
        // Show the window in it's default state with NO CURSOR.
        ShowWindow( hWnd, SW_SHOWDEFAULT );
        UpdateWindow( hWnd );
        if( g_FullScreen ) 
            ShowCursor( FALSE );
        // Startup in stereo mode. If available.
        if( g_StereoEnabled ) 
            IWRSTEREO_SetStereo( g_StereoHandle, IWR_STEREO_MODE );
        // Enter the message loop
        MSG msg;
        ZeroMemory( &msg, sizeof(msg) );
        while( msg.message!=WM_QUIT ) {
            if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
            else {
                if( g_pd3dDevice ) {
                    // Poll devices and update animation sequences.
                    UpdateOrientationAndAnimate();
                    if( g_StereoEnabled ) {
                        if ((g_ProductID == IWR_PROD_VR920) || 
							(g_ProductDetails & IWR_FEATURE_USB_PSCAN_3D)) {
                            g_pLeftEyeQuery->Issue(D3DISSUE_BEGIN);
                            RenderWorld( LEFT_EYE );
                            // Add an end marker to the command buffer queue.
                            g_pLeftEyeQuery->Issue(D3DISSUE_END);

                            // Syncronize the frame to the left eyes frame buffer.
                            //  Ignore failures until processing on the right eye.
                            IWRSyncronizeEye( LEFT_EYE );
                            g_FrameCounter++;

                            // Start Rendering the right eye prior to waiting on the left eye rendering.
                            g_pRightEyeQuery->Issue(D3DISSUE_BEGIN);
                            RenderWorld( RIGHT_EYE );
                            g_pRightEyeQuery->Issue(D3DISSUE_END);

                            if( IWRSyncronizeEye( RIGHT_EYE ) == EXIT_STEREOSCOPIC_MODE ) {
                                // Lost connection possibly hot-plug VR920's USB connector.
                                g_StereoHandle = INVALID_HANDLE_VALUE;
                                g_StereoEnabled = false; 
                            }
                            g_FrameCounter++;
                        }
                        else if ((g_ProductID == IWR_PROD_WRAP920) ||
							(g_ProductDetails & IWR_FEATURE_SIDE_X_SIDE)) {
                            // Full Screen mode only…
                            D3DVIEWPORT9 vpl = { 0, 0, g_ViewPortWidth/2, g_ViewPortHeight, 0.0f, 1.0f };
                            if( FAILED(g_pd3dDevice->SetViewport( &vpl ) ) ) {
                                IWROut( "LOG: Unable to set Left viewport\n");
                                //ERRAddErrorStatus( E_FAIL, ErrStr );
                            }
                            RenderWorld( LEFT_EYE );

                            D3DVIEWPORT9 vpr = { g_ViewPortWidth/2, 0, g_ViewPortWidth/2, g_ViewPortHeight, 0.0f, 1.0f };
                            if( FAILED(g_pd3dDevice->SetViewport( &vpr ) ) ) {
                                IWROut( "LOG: Unable to set right viewport\n");
                                //ERRAddErrorStatus( E_FAIL, ErrStr );
                            }
                            RenderWorld( RIGHT_EYE );
                            if( FAILED( g_pd3dDevice->Present( NULL, NULL, NULL, NULL ) ) ) {
                                IWROut( "LOG: Failed to Present.\n" );
                                DestroyWindow( hWnd );
                            }
                        }
                    }
                    else { // Mono Mode rendering.
						D3DVIEWPORT9 vpm = { 0, 0, g_ViewPortWidth, g_ViewPortHeight, 0.0f, 1.0f };
                        if( FAILED(g_pd3dDevice->SetViewport( &vpm ) ) ) {
                            IWROut( "LOG: Unable to set Mono viewport\n");
                            //ERRAddErrorStatus( E_FAIL, ErrStr );
                        }
                        RenderWorld( MONO_EYES );
                        if( FAILED( g_pd3dDevice->Present( NULL, NULL, NULL, NULL ) ) ) {
                            IWROut( "LOG: Failed to Present.\n" );
                            DestroyWindow( hWnd );
                        }
                        g_FrameCounter++;
                    }
                    // Recompute Frame Rate.
                    if( !(g_FrameCounter % RECCALCULATE_FPS) ) {
                        totalTime = timeGetTime() - g_StartTimer;
                        g_fRate = (float)(g_FrameCounter - g_startingPresentFrame) / (float)totalTime * 1000.0f;
                        g_startingPresentFrame = g_FrameCounter;
                        g_StartTimer = timeGetTime();
                    }
                }
            }
        }
    }
    else {
        IWROut("LOG: Error allocating D3D Resources\n");
        DestroyWindow( hWnd );
    }
    IWROut("LOG: Exit Rendering process\n");
    return 0;
}
