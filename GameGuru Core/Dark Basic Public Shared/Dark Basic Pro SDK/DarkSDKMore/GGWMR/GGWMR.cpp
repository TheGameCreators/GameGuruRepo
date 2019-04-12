//
// GGWMR - Windows Mixed Reality DLL (WinRT hooked)
//

#define ACTIVATEVRDEBUGLOGGING

// Includes for Windows Mixed Reality
#include "stdafx.h"
#include <HolographicSpaceInterop.h>
#include <windows.graphics.holographic.h>
#include <winrt\Windows.Graphics.Holographic.h>
#include "GGWMR.h"

// Namespaces
using namespace BasicHologram;
using namespace concurrency;
using namespace std::placeholders;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input::Spatial;

// Globals
App app;
HolographicFrame g_holographicFrame = nullptr;
int g_iDebugLoggingActive = 0;
char* g_pVRDebugLog[10000];

// Support debug log system
int DebugVRlog ( const char* pReportLog )
{
	// Log File
	#ifdef ACTIVATEVRDEBUGLOGGING
	if ( pReportLog == NULL )
	{
		// Reset at VERY start
		for ( int iFind = 0; iFind < 10000; iFind++ ) g_pVRDebugLog[iFind] = NULL; 
		return 0;
	}
	if ( g_iDebugLoggingActive == 1 )
	{
		// the debug file
		const char* pFilename = "VRDebugLog.log";

		// New entry 
		char pWithTime[2048];
		sprintf_s ( pWithTime, "%d : %s\r\n", timeGetTime(), pReportLog );
		DWORD dwNewEntry = strlen(pWithTime);

		// Find new slot
		for ( int iFind = 0; iFind < 10000; iFind++ )
		{
			if ( g_pVRDebugLog[iFind] == NULL ) 
			{
				g_pVRDebugLog[iFind] = new char[2048];
				memset ( g_pVRDebugLog[iFind], 0, 2048 );
				strcpy_s ( g_pVRDebugLog[iFind], 2048, pWithTime );
				break;
			}
		}

		// save new log file
		DWORD bytesdone = 0;
		HANDLE hFile = CreateFile(pFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			for ( int iFind = 0; iFind < 10000; iFind++ )
			{
				if ( g_pVRDebugLog[iFind] != NULL ) 
				{
					WriteFile(hFile, g_pVRDebugLog[iFind], strlen(g_pVRDebugLog[iFind]), &bytesdone, FALSE);
				}
			}
			CloseHandle ( hFile );
		}
		else
		{
			// error - cannot get write lock on this file!
			return 1;
		}
	}
	#endif

	// success
	return 0;
}

// DLL Entry Function
BOOL APIENTRY GGWMRMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	hModule = hModule;
	ul_reason_for_call = ul_reason_for_call;
	lpReserved = lpReserved;
	DebugVRlog(NULL);
	DebugVRlog("WMR DLL Activated");
	return TRUE;
}

DLLEXPORT int GGWMR_CreateHolographicSpace1 ( HWND hWnd, int iDebugLoggingActive )
{
	// hack this in for now
	MessageBox ( NULL, "Reached GGWMR_CreateHolographicSpace1", "", MB_OK );

	// set debugging flag
	g_iDebugLoggingActive = iDebugLoggingActive;

	// test abilty to write VR debug log file
	if ( DebugVRlog("test VR debug log file writing") == 1 )
		return 51;

	// now go through VR setup
	DebugVRlog("GetInitialised");
	if ( app.GetInitialised() == false )
	{
		// COM init
		DebugVRlog("init_apartment");
		winrt::init_apartment();

		// Initialize global strings, and perform application initialization.
		DebugVRlog("Initialize");
		app.Initialize();

		// Create the HWND and the HolographicSpace.
		DebugVRlog("CreateHolographicSpaceA");
		int iErrorCode = app.CreateHolographicSpaceA(hWnd);
		if ( iErrorCode > 0 ) return iErrorCode;
	}
	else
		return 11;

	// success
	return 0;
}

DLLEXPORT int GGWMR_CreateHolographicSpace2 ( void* pD3DDevice, void* pD3DContext )
{
	if ( app.GetInitialised() == false )
	{
		// now initialised
		DebugVRlog("SetInitialised");
		app.SetInitialised(true);

		// complete holographic space creation using existing device and context from engine
		DebugVRlog("CreateHolographicSpaceB");
	    int iErrorCode = app.CreateHolographicSpaceB((ID3D11Device*)pD3DDevice, (ID3D11DeviceContext*)pD3DContext);
		if ( iErrorCode > 0 ) return iErrorCode;
	}

	// success
	return 0;
}

DLLEXPORT void GGWMR_GetUpdate ( void )
{
	app.UpdateFrame();
}

DLLEXPORT void GGWMR_GetHeadPosAndDir ( float* pPosX, float* pPosY, float* pPosZ, float* pUpX, float* pUpY, float* pUpZ, float* pDirX, float* pDirY, float* pDirZ )
{
	app.GetHeadPosAndDir ( pPosX, pPosY, pPosZ, pUpX, pUpY, pUpZ, pDirX, pDirY, pDirZ );
}

DLLEXPORT void GGWMR_GetProjectionMatrix ( int iEyeIndex,	float* pM00, float* pM10, float* pM20, float* pM30, 
															float* pM01, float* pM11, float* pM21, float* pM31,
															float* pM02, float* pM12, float* pM22, float* pM32,
															float* pM03, float* pM13, float* pM23, float* pM33)
{
	app.GetProjectionMatrix ( iEyeIndex, pM00, pM10, pM20, pM30, pM01, pM11, pM21, pM31, pM02, pM12, pM22, pM32, pM03, pM13, pM23, pM33 );
}

DLLEXPORT void GGWMR_GetThumbAndTrigger ( float* pTriggerValue, float* pThumbStickX, float* pThumbStickY )
{
	app.GetThumbAndTrigger ( pTriggerValue, pThumbStickX, pThumbStickY );
}

DLLEXPORT void GGWMR_GetTouchPadData ( bool* pbTouched, bool* pbPressed, float* pfTouchPadX, float* pfTouchPadY )
{
	app.GetTouchPadData ( pbTouched, pbPressed, pfTouchPadX, pfTouchPadY );
}

DLLEXPORT void GGWMR_GetHandPosAndOrientation ( float* pRHX, float* pRHY, float* pRHZ, float* pQuatW, float* pQuatX, float* pQuatY, float* pQuatZ )
{
	app.GetHandPosAndOrientation ( pRHX, pRHY, pRHZ, pQuatW, pQuatX, pQuatY, pQuatZ );
}

DLLEXPORT void GGWMR_GetRenderTargetAndDepthStencilView ( void** ppRenderTargetLeft, void** ppRenderTargetRight, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight)
{
	app.UpdateRender();
	app.GetRenderTargetAndDepthStencilView ( ppRenderTargetLeft, ppRenderTargetRight, ppDepthStencil, pdwWidth, pdwHeight );
}

DLLEXPORT void GGWMR_Present ( void )
{
	app.Present();
}

// Holographic Functions

void App::Initialize()
{
    // At this point we have access to the device and we can create device-dependent
	DebugVRlog("DeviceResources");
    m_deviceResources = std::make_shared<DX::DeviceResources>();
	DebugVRlog("BasicHologramMain");
    m_main = std::make_unique<BasicHologramMain>(m_deviceResources);
}

int App::CreateHolographicSpaceA(HWND hWnd)
{
    // Use WinRT factory to create the holographic space.
	//MessageBox ( NULL, "VR error1", "", MB_OK );
    using namespace winrt::Windows::Graphics::Holographic;
	//MessageBox ( NULL, "VR error2", "", MB_OK );
	DebugVRlog("IHolographicSpaceInterop");
    winrt::com_ptr<IHolographicSpaceInterop> holographicSpaceInterop = winrt::get_activation_factory<HolographicSpace, IHolographicSpaceInterop>();
	//MessageBox ( NULL, "VR error3", "", MB_OK );
	DebugVRlog("IHolographicSpace");
    winrt::com_ptr<ABI::Windows::Graphics::Holographic::IHolographicSpace> spHolographicSpace;
	//MessageBox ( NULL, "VR error4", "", MB_OK );
	DebugVRlog("CreateForWindow");
	//MessageBox ( NULL, "VR error5", "", MB_OK );
    winrt::check_hresult(holographicSpaceInterop->CreateForWindow(hWnd, __uuidof(ABI::Windows::Graphics::Holographic::IHolographicSpace), winrt::put_abi(spHolographicSpace)));
	//MessageBox ( NULL, "VR error6", "", MB_OK );
    if (!spHolographicSpace)
    {
		//MessageBox ( NULL, "VR error", "", MB_OK );
        winrt::check_hresult(E_FAIL);
		return 1;
    }

    // Store the holographic space.
	DebugVRlog("HolographicSpace");
    m_holographicSpace = spHolographicSpace.as<HolographicSpace>();
	if ( m_holographicSpace == nullptr )
		return 2;

	// Create spatial interaction manager for hand/controller input
	DebugVRlog("get_activation_factory");
	winrt::com_ptr<ISpatialInteractionManagerInterop> spatialInteractionManagerInterop = winrt::get_activation_factory<SpatialInteractionManager, ISpatialInteractionManagerInterop>();
	if (!spatialInteractionManagerInterop)
	{
		winrt::check_hresult(E_FAIL);
		return 3;
	}
	DebugVRlog("ISpatialInteractionManager");
	winrt::com_ptr<ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager> spSpatialInteractionManager;
	DebugVRlog("spatialInteractionManagerInterop");
	winrt::check_hresult(spatialInteractionManagerInterop->GetForWindow(hWnd, __uuidof(ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager), winrt::put_abi(spSpatialInteractionManager)));
	if (!spSpatialInteractionManager)
	{
		winrt::check_hresult(E_FAIL);
		return 4;
	}

	// create interaction manager
	DebugVRlog("SpatialInteractionManager");
	m_interactionManager = spSpatialInteractionManager.as<SpatialInteractionManager>();
	if ( m_interactionManager == nullptr )
		return 5;

	//m_sourcePressedEventToken = m_interactionManager.SourcePressed(bind(&App::OnSourcePressed, this, _1, _2));
	DebugVRlog("SourceUpdated");
	m_sourceUpdatedEventToken = m_interactionManager.SourceUpdated(bind(&App::OnSourceUpdated, this, _1, _2));
	//m_sourceReleasedEventToken = m_interactionManager.SourceReleased(bind(&App::OnSourceReleased, this, _1, _2));

	// success
	return 0;
}

int App::CreateHolographicSpaceB(ID3D11Device* pDevice,ID3D11DeviceContext* pContext)
{
    // The DeviceResources class uses the preferred DXGI adapter ID from the holographic
    // space (when available) to create a Direct3D device. The HolographicSpace
    // uses this ID3D11Device to create and manage device-based resources such as
    // swap chains.
	DebugVRlog("deviceResources SetHolographicSpace");
    m_deviceResources->SetHolographicSpace ( m_holographicSpace, pDevice, pContext );

    // The main class uses the holographic space for updates and rendering.
	DebugVRlog("main SetHolographicSpace");
    m_main->SetHolographicSpace(m_holographicSpace, &m_interactionManager);

	// success
	return 0;
}

void App::UpdateFrame()
{
	g_holographicFrame = nullptr;
    if (m_holographicSpace != nullptr)
    {
		winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference pSpatialStationaryFrameOfReference = nullptr;
        g_holographicFrame = m_main->Update(&pSpatialStationaryFrameOfReference);
	}
}

void App::GetHeadPosAndDir ( float* pPosX, float* pPosY, float* pPosZ, float* pUpX, float* pUpY, float* pUpZ, float* pDirX, float* pDirY, float* pDirZ )
{
    if (g_holographicFrame != nullptr)
    {
		*pPosX = m_main->GetPassOutHeadPosX();
		*pPosY = m_main->GetPassOutHeadPosY();
		*pPosZ = m_main->GetPassOutHeadPosZ();
		*pUpX = m_main->GetPassOutHeadUpX();
		*pUpY = m_main->GetPassOutHeadUpY();
		*pUpZ = m_main->GetPassOutHeadUpZ();
		*pDirX = m_main->GetPassOutHeadDirX();
		*pDirY = m_main->GetPassOutHeadDirY();
		*pDirZ = m_main->GetPassOutHeadDirZ();
	}
}

void App::GetThumbAndTrigger ( float* pTriggerValue, float* pThumbStickX, float* pThumbStickY )
{
    if (g_holographicFrame != nullptr)
    {
		*pTriggerValue = m_fTriggerValue;
		*pThumbStickX = m_fThumbX;
		*pThumbStickY = m_fThumbY;
	}
}

void App::GetTouchPadData ( bool* pbTouched, bool* pbPressed, float* pfTouchPadX, float* pfTouchPadY )
{
    if (g_holographicFrame != nullptr)
    {
		*pbTouched = m_bTouchPadTouched;
		*pbPressed = m_bTouchPadPressed;
		*pfTouchPadX = m_fTouchPadX;
		*pfTouchPadY = m_fTouchPadY;
	}
}

void App::GetHandPosAndOrientation ( float* pRHX, float* pRHY, float* pRHZ, float* pQuatW, float* pQuatX, float* pQuatY, float* pQuatZ )
{
    if (g_holographicFrame != nullptr)
    {
		*pRHX = m_fRightHandX;
		*pRHY = m_fRightHandY;
		*pRHZ = m_fRightHandZ;
		*pQuatW = m_qRightHandOrientation.w;
		*pQuatX = m_qRightHandOrientation.x;
		*pQuatY = m_qRightHandOrientation.y;
		*pQuatZ = m_qRightHandOrientation.z;
	}
}

void App::GetProjectionMatrix ( int iEyeIndex,	float* pM00, float* pM10, float* pM20, float* pM30, 
												float* pM01, float* pM11, float* pM21, float* pM31,
												float* pM02, float* pM12, float* pM22, float* pM32,
												float* pM03, float* pM13, float* pM23, float* pM33)
{
	if ( iEyeIndex == 0 ) 
	{
		*pM00 = m_main->GetPassOutProjectionLeft().m11;
		*pM10 = m_main->GetPassOutProjectionLeft().m21;
		*pM20 = m_main->GetPassOutProjectionLeft().m31;
		*pM30 = m_main->GetPassOutProjectionLeft().m41;
		*pM01 = m_main->GetPassOutProjectionLeft().m12;
		*pM11 = m_main->GetPassOutProjectionLeft().m22;
		*pM21 = m_main->GetPassOutProjectionLeft().m32;
		*pM31 = m_main->GetPassOutProjectionLeft().m42;
		*pM02 = m_main->GetPassOutProjectionLeft().m13;
		*pM12 = m_main->GetPassOutProjectionLeft().m23;
		*pM22 = m_main->GetPassOutProjectionLeft().m33;
		*pM32 = m_main->GetPassOutProjectionLeft().m43;
		*pM03 = m_main->GetPassOutProjectionLeft().m14;
		*pM13 = m_main->GetPassOutProjectionLeft().m24;
		*pM23 = m_main->GetPassOutProjectionLeft().m34;
		*pM33 = m_main->GetPassOutProjectionLeft().m41;
	}
	else
	{
		*pM00 = m_main->GetPassOutProjectionRight().m11;
		*pM10 = m_main->GetPassOutProjectionRight().m21;
		*pM20 = m_main->GetPassOutProjectionRight().m31;
		*pM30 = m_main->GetPassOutProjectionRight().m41;
		*pM01 = m_main->GetPassOutProjectionRight().m12;
		*pM11 = m_main->GetPassOutProjectionRight().m22;
		*pM21 = m_main->GetPassOutProjectionRight().m32;
		*pM31 = m_main->GetPassOutProjectionRight().m42;
		*pM02 = m_main->GetPassOutProjectionRight().m13;
		*pM12 = m_main->GetPassOutProjectionRight().m23;
		*pM22 = m_main->GetPassOutProjectionRight().m33;
		*pM32 = m_main->GetPassOutProjectionRight().m43;
		*pM03 = m_main->GetPassOutProjectionRight().m14;
		*pM13 = m_main->GetPassOutProjectionRight().m24;
		*pM23 = m_main->GetPassOutProjectionRight().m34;
		*pM33 = m_main->GetPassOutProjectionRight().m41;
	}
}

void App::UpdateRender()
{
	m_canPresentThisFrame = false;
    if (g_holographicFrame != nullptr)
    {
        if (m_main->Render(g_holographicFrame))
        {
			m_canPresentThisFrame = true;
        }
	}
}

void App::GetRenderTargetAndDepthStencilView ( void** ppRenderTargetLeft, void** ppRenderTargetRight, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight )
{
	*ppRenderTargetLeft = m_main->GetPassOutRenderTargetLeftView();
	*ppRenderTargetRight = m_main->GetPassOutRenderTargetRightView();
	*ppDepthStencil = m_main->GetPassOutDepthStencilView();
	*pdwWidth = m_main->GetPassOutRenderTargetWidth();
	*pdwHeight = m_main->GetPassOutRenderTargetHeight();
}

void App::Present()
{
    if (g_holographicFrame != nullptr)
    {
		if ( m_canPresentThisFrame == true )
		{
			if ( m_main->GetPassOutRenderTargetLeftView() != NULL )
			{
				m_deviceResources->Present(g_holographicFrame);
			}
		}
	}
}

void App::Uninitialize()
{
	// completely free Holographic Space    
	m_main.reset();
	m_deviceResources.reset();
}

void App::OnSourceUpdated(SpatialInteractionManager const&, SpatialInteractionSourceEventArgs const& args)
{
	SpatialInteractionSourceState state = args.State();
	SpatialInteractionSource source = state.Source();
	SpatialInteractionController controller = source.Controller();
	if (controller != nullptr)
    {
		// controller properties
		SpatialInteractionControllerProperties controllerState = state.ControllerProperties();

		// get trigger value and thumbstick
		m_fTriggerValue = (float)state.SelectPressedValue();
        if (controller.HasThumbstick())
        {
			m_fThumbX = (float)controllerState.ThumbstickX();
			m_fThumbY = (float)controllerState.ThumbstickY();
		}
		else
		{
			m_fThumbX = 0.0f;
			m_fThumbY = 0.0f;
		}

		// get controller touch input
        if (controller.HasTouchpad())
        {
			m_bTouchPadTouched = controllerState.IsTouchpadTouched();
			m_bTouchPadPressed = controllerState.IsTouchpadPressed();
			m_fTouchPadX = (float)controllerState.TouchpadX();
			m_fTouchPadY = (float)controllerState.TouchpadY();
		}
		else
		{
			m_bTouchPadTouched = false;
			m_bTouchPadPressed = false;
			m_fTouchPadX = 0;
			m_fTouchPadY = 0;
		}

		// get controller position and orientation
        SpatialPointerPose pose = nullptr;
		winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference stationaryReferenceFrame = m_main->GetFrameOfReference();
		pose = state.TryGetPointerPose(stationaryReferenceFrame.CoordinateSystem());
		if ( pose != nullptr )
		{
			SpatialPointerInteractionSourcePose motioncontrollerpose = nullptr;
			motioncontrollerpose = pose.TryGetInteractionSourcePose(source);
			if ( motioncontrollerpose != nullptr )
			{
				//if ( source.Handedness() == SpatialInteractionSourceHandedness::Right )
				//{
					m_fRightHandX = motioncontrollerpose.Position().x;
					m_fRightHandY = motioncontrollerpose.Position().y;
					m_fRightHandZ = motioncontrollerpose.Position().z;
					m_qRightHandOrientation = motioncontrollerpose.Orientation();
				//}
			}
		}
	}
}
