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
	// remove for now - performance?
	return 0;

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
	//DebugVRlog(NULL);
	//DebugVRlog("WMR DLL Activated");
	return TRUE;
}

DLLEXPORT int GGWMR_CreateHolographicSpace1 ( HWND hWnd, int iDebugLoggingActive )
{
	// set debugging flag
	g_iDebugLoggingActive = iDebugLoggingActive;

	// test abilty to write VR debug log file
	//if ( //DebugVRlog("test VR debug log file writing") == 1 )
	//	return 51;

	// now go through VR setup
	//DebugVRlog("GetInitialised");
	if (app.GetInitialised() == false)
	{
		// COM init
		//DebugVRlog("init_apartment");
		winrt::init_apartment();

		// Initialize global strings, and perform application initialization.
		//DebugVRlog("Initialize");
		app.Initialize();
	}

	// Create the HWND and the HolographicSpace.
	//DebugVRlog("CreateHolographicSpaceA");
	int iErrorCode = app.CreateHolographicSpaceA(hWnd);
	if ( iErrorCode > 0 ) return iErrorCode;

	// separated Initialize and removed error code 11 so can re-call CreateHolographicSpaceA
	//}
	//else
	//	return 11;

	// success
	return 0;
}

DLLEXPORT void GGWMR_ReconnectWithHolographicSpaceControllers ( void )
{
	app.ReconnectWithHolographicSpaceControllers();
}

DLLEXPORT int GGWMR_CreateHolographicSpace2 ( void* pD3DDevice, void* pD3DContext )
{
	if ( app.GetInitialised() == false )
	{
		// now initialised
		//DebugVRlog("SetInitialised");
		app.SetInitialised(true);

		// complete holographic space creation using existing device and context from engine
		//DebugVRlog("CreateHolographicSpaceB");
	    int iSuccess = app.CreateHolographicSpaceB((ID3D11Device*)pD3DDevice, (ID3D11DeviceContext*)pD3DContext);
		if ( iSuccess == 0 ) 
		{
			// failed to complete CreateHolographicSpaceB - suspect inappropriate D3D device used (adapter choice?)
			return 456;
		}
	}

	// no error code
	return 0;
}

DLLEXPORT void GGWMR_GetUpdate ( void )
{
	///MessageBox ( NULL, "Inside GGWMR_GetUpdate", "0.B", MB_OK );
	try
	{
		app.UpdateFrame();
	}
	catch (...)
	{
		//DebugVRlog("failed app.UpdateFrame");
	}
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

DLLEXPORT void GGWMR_GetThumbAndTrigger ( float* pSideButtonValue, float* pTriggerValue, float* pThumbStickX, float* pThumbStickY )
{
	app.GetThumbAndTrigger ( pSideButtonValue, pTriggerValue, pThumbStickX, pThumbStickY );
}

DLLEXPORT void GGWMR_GetTouchPadData ( bool* pbTouchedisRightHand, bool* pbTouched, bool* pbPressed, float* pfTouchPadX, float* pfTouchPadY )
{
	app.GetTouchPadData ( pbTouchedisRightHand, pbTouched, pbPressed, pfTouchPadX, pfTouchPadY );
}

DLLEXPORT void GGWMR_GetHandPosAndOrientation ( int iLeftHandMode, float* pRHX, float* pRHY, float* pRHZ, float* pQuatW, float* pQuatX, float* pQuatY, float* pQuatZ )
{
	//DebugVRlog("GetHandPosAndOrientation");
	app.GetHandPosAndOrientation ( iLeftHandMode, pRHX, pRHY, pRHZ, pQuatW, pQuatX, pQuatY, pQuatZ );
}

DLLEXPORT void GGWMR_GetRenderTargetAndDepthStencilView ( void** ppRenderTargetLeft, void** ppRenderTargetRight, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight, void** ppLeftShaderResourceView )
{
	app.UpdateRender();
	app.GetRenderTargetAndDepthStencilView ( ppRenderTargetLeft, ppRenderTargetRight, ppDepthStencil, pdwWidth, pdwHeight, ppLeftShaderResourceView );
}

DLLEXPORT void GGWMR_Present ( void )
{
	app.Present();
}

DLLEXPORT void GGWMR_DeleteHolographicSpace(void)
{
	app.Uninitialize();
	app.DeleteHolographicSpace();
	app.SetInitialised(false);
}

// Holographic Functions

void App::Initialize()
{
    // At this point we have access to the device and we can create device-dependent
	//DebugVRlog("DeviceResources");
    m_deviceResources = std::make_shared<DX::DeviceResources>();
	//DebugVRlog("BasicHologramMain");
    m_main = std::make_unique<BasicHologramMain>(m_deviceResources);
}

int App::CreateHolographicSpaceA(HWND hWnd)
{
    // Use WinRT factory to create the holographic space.
	//MessageBox ( NULL, "VR error1", "", MB_OK );
    using namespace winrt::Windows::Graphics::Holographic;
	//MessageBox ( NULL, "VR error2", "", MB_OK );
	//DebugVRlog("IHolographicSpaceInterop");
    winrt::com_ptr<IHolographicSpaceInterop> holographicSpaceInterop = winrt::get_activation_factory<HolographicSpace, IHolographicSpaceInterop>();
	//MessageBox ( NULL, "VR error3", "", MB_OK );
	//DebugVRlog("IHolographicSpace");
    winrt::com_ptr<ABI::Windows::Graphics::Holographic::IHolographicSpace> spHolographicSpace;
	//MessageBox ( NULL, "VR error4", "", MB_OK );
	//DebugVRlog("CreateForWindow");
	//MessageBox ( NULL, "VR error5", "", MB_OK );
    //winrt::check_hresult(holographicSpaceInterop->CreateForWindow(hWnd, __uuidof(ABI::Windows::Graphics::Holographic::IHolographicSpace), winrt::put_abi(spHolographicSpace)));
	holographicSpaceInterop->CreateForWindow(hWnd, __uuidof(ABI::Windows::Graphics::Holographic::IHolographicSpace), winrt::put_abi(spHolographicSpace));
	//MessageBox ( NULL, "VR error6", "", MB_OK );
    if (!spHolographicSpace)
    {
		//MessageBox ( NULL, "VR error", "", MB_OK );
        //winrt::check_hresult(E_FAIL);
		return 1;
    }

    // Store the holographic space.
	//DebugVRlog("HolographicSpace");
    m_holographicSpace = spHolographicSpace.as<HolographicSpace>();
	if ( m_holographicSpace == nullptr )
		return 2;

	// Create spatial interaction manager for hand/controller input
	//DebugVRlog("get_activation_factory");
	winrt::com_ptr<ISpatialInteractionManagerInterop> spatialInteractionManagerInterop = winrt::get_activation_factory<SpatialInteractionManager, ISpatialInteractionManagerInterop>();
	if (!spatialInteractionManagerInterop)
	{
		winrt::check_hresult(E_FAIL);
		return 3;
	}
	//DebugVRlog("ISpatialInteractionManager");
	winrt::com_ptr<ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager> spSpatialInteractionManager;
	//DebugVRlog("spatialInteractionManagerInterop");
	winrt::check_hresult(spatialInteractionManagerInterop->GetForWindow(hWnd, __uuidof(ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager), winrt::put_abi(spSpatialInteractionManager)));
	if (!spSpatialInteractionManager)
	{
		winrt::check_hresult(E_FAIL);
		return 4;
	}

	// create interaction manager
	//DebugVRlog("SpatialInteractionManager");
	m_interactionManager = spSpatialInteractionManager.as<SpatialInteractionManager>();
	if ( m_interactionManager == nullptr )
		return 5;

	//DebugVRlog("SourceUpdated");
	m_sourceUpdatedEventToken = m_interactionManager.SourceUpdated(bind(&App::OnSourceUpdated, this, _1, _2));

	//DebugVRlog("SourceLost");
	m_sourceUpdatedEventToken = m_interactionManager.SourceLost(bind(&App::OnSourceLost, this, _1, _2));

	// success
	return 0;
}

void App::ReconnectWithHolographicSpaceControllers(void)
{
    if (g_holographicFrame != nullptr)
    {
		//DebugVRlog("SourceUpdated");
		m_sourceUpdatedEventToken = m_interactionManager.SourceUpdated(bind(&App::OnSourceUpdated, this, _1, _2));
	}
}

int App::CreateHolographicSpaceB(ID3D11Device* pDevice,ID3D11DeviceContext* pContext)
{
    // The DeviceResources class uses the preferred DXGI adapter ID from the holographic
    // space (when available) to create a Direct3D device. The HolographicSpace
    // uses this ID3D11Device to create and manage device-based resources such as
    // swap chains.
	//DebugVRlog("deviceResources SetHolographicSpace");
    int iSuccess = m_deviceResources->SetHolographicSpace ( m_holographicSpace, pDevice, pContext );
	if ( iSuccess == 0 ) return 0;

    // The main class uses the holographic space for updates and rendering.
	//DebugVRlog("main SetHolographicSpace");
    m_main->SetHolographicSpace(m_holographicSpace, &m_interactionManager);

	// success
	return 1;
}

void App::DeleteHolographicSpace()
{
	// completely free initial creations
	// from createA
	m_interactionManager = nullptr;
	m_holographicSpace = nullptr;
}

void App::Uninitialize()
{
	// completely free Holographic Space    
	m_main.reset();
	m_deviceResources.reset();
}

void App::UpdateFrame()
{
	///MessageBox ( NULL, "Inside UpdateFrame", "0.C", MB_OK );
	///MessageBox ( NULL, "Hard Debug Trace", "1", MB_OK );
	g_holographicFrame = nullptr;
	///MessageBox ( NULL, "Hard Debug Trace", "2", MB_OK );
    if (m_holographicSpace != nullptr)
    {
		///MessageBox ( NULL, "Hard Debug Trace", "3", MB_OK );
		winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference pSpatialStationaryFrameOfReference = nullptr;
		///MessageBox ( NULL, "Hard Debug Trace", "4", MB_OK );
		try
		{
	        g_holographicFrame = m_main->Update(&pSpatialStationaryFrameOfReference);
		}
		catch(...)
		{
			//DebugVRlog("failed m_main->Update(&pSpatialStationaryFrameOfReference)");
			///MessageBox ( NULL, "FAILED g_holographicFrame = m_main->Update(&pSpatialStationaryFrameOfReference)", "4", MB_OK );
		}
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

void App::GetThumbAndTrigger ( float* pSideButtonValue, float* pTriggerValue, float* pThumbStickX, float* pThumbStickY )
{
    if (g_holographicFrame != nullptr)
    {
		if ( fabs(m_fSideButtonValue[1]) > fabs(m_fSideButtonValue[0]) )
			*pSideButtonValue = m_fSideButtonValue[1];
		else
			*pSideButtonValue = m_fSideButtonValue[0];

		if ( fabs(m_fTriggerValue[1]) > fabs(m_fTriggerValue[0]) )
			*pTriggerValue = m_fTriggerValue[1];
		else
			*pTriggerValue = m_fTriggerValue[0];

		if ( fabs(m_fThumbX[1])+fabs(m_fThumbY[1]) > fabs(m_fThumbX[0])+fabs(m_fThumbY[0]) )
		{
			*pThumbStickX = m_fThumbX[1];
			*pThumbStickY = m_fThumbY[1];
		}
		else
		{
			*pThumbStickX = m_fThumbX[0];
			*pThumbStickY = m_fThumbY[0];
		}
	}
}

void App::GetTouchPadData ( bool* pbTouchedisRightHand, bool* pbTouched, bool* pbPressed, float* pfTouchPadX, float* pfTouchPadY )
{
    if (g_holographicFrame != nullptr)
    {
		if ( m_bTouchPadTouched[1] == true )
		{
			*pbTouched = m_bTouchPadTouched[1];
			*pbPressed = m_bTouchPadPressed[1];
			*pfTouchPadX = m_fTouchPadX[1];
			*pfTouchPadY = m_fTouchPadY[1];
			*pbTouchedisRightHand = true;
		}
		else
		{
			*pbTouched = m_bTouchPadTouched[0];
			*pbPressed = m_bTouchPadPressed[0];
			*pfTouchPadX = m_fTouchPadX[0];
			*pfTouchPadY = m_fTouchPadY[0];
			*pbTouchedisRightHand = false;
		}
	}
}

void App::GetHandPosAndOrientation ( int iLeftHandMode, float* pRHX, float* pRHY, float* pRHZ, float* pQuatW, float* pQuatX, float* pQuatY, float* pQuatZ )
{
	if ( iLeftHandMode == -1 )
	{
		// called to reset before next session (to hide controllers if they are not used in this session)
		m_fHandX[0] = 0.0f;
		m_fHandY[0] = 0.0f;
		m_fHandZ[0] = 0.0f;	
		m_fHandX[1] = 0.0f;
		m_fHandY[1] = 0.0f;
		m_fHandZ[1] = 0.0f;	
	}
	else
	{
		if (g_holographicFrame != nullptr)
		{
			if ( iLeftHandMode == 0 )
			{
				*pRHX = m_fHandX[1];
				*pRHY = m_fHandY[1];
				*pRHZ = m_fHandZ[1];
				*pQuatW = m_qHandOrientation[1].w;
				*pQuatX = m_qHandOrientation[1].x;
				*pQuatY = m_qHandOrientation[1].y;
				*pQuatZ = m_qHandOrientation[1].z;
			}
			else
			{
				*pRHX = m_fHandX[0];
				*pRHY = m_fHandY[0];
				*pRHZ = m_fHandZ[0];
				*pQuatW = m_qHandOrientation[0].w;
				*pQuatX = m_qHandOrientation[0].x;
				*pQuatY = m_qHandOrientation[0].y;
				*pQuatZ = m_qHandOrientation[0].z;
			}
		}
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

void App::GetRenderTargetAndDepthStencilView ( void** ppRenderTargetLeft, void** ppRenderTargetRight, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight, void** ppLeftShaderResourceView )
{
	*ppRenderTargetLeft = m_main->GetPassOutRenderTargetLeftView();
	*ppRenderTargetRight = m_main->GetPassOutRenderTargetRightView();
	*ppDepthStencil = m_main->GetPassOutDepthStencilView();
	*pdwWidth = m_main->GetPassOutRenderTargetWidth();
	*pdwHeight = m_main->GetPassOutRenderTargetHeight();
	*ppLeftShaderResourceView = m_main->GetPassOutLeftShaderResourceView();
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

void App::OnSourceLost(SpatialInteractionManager const&, SpatialInteractionSourceEventArgs const& args)
{
	SpatialInteractionSourceState state = args.State();
	SpatialInteractionSource source = state.Source();
	SpatialInteractionController controller = source.Controller();
	if (controller != nullptr)
    {
	}
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

		// left/right index
		int iLeftRightIndex = 0;
		if ( source.Handedness() == SpatialInteractionSourceHandedness::Right )
			iLeftRightIndex = 1;

		// get side button (grasp), trigger value and thumbstick
		m_fSideButtonValue[iLeftRightIndex] = 0.0f;
		if ( state.IsGrasped() == true ) m_fSideButtonValue[iLeftRightIndex] = 1.0f;
		m_fTriggerValue[iLeftRightIndex] = (float)state.SelectPressedValue();
        if (controller.HasThumbstick())
        {
			m_fThumbX[iLeftRightIndex] = (float)controllerState.ThumbstickX();
			m_fThumbY[iLeftRightIndex] = (float)controllerState.ThumbstickY();
		}
		else
		{
			m_fThumbX[iLeftRightIndex] = 0.0f;
			m_fThumbY[iLeftRightIndex] = 0.0f;
		}

		// get controller touch input
        if (controller.HasTouchpad())
        {
			m_bTouchPadTouched[iLeftRightIndex] = controllerState.IsTouchpadTouched();
			m_bTouchPadPressed[iLeftRightIndex] = controllerState.IsTouchpadPressed();
			m_fTouchPadX[iLeftRightIndex] = (float)controllerState.TouchpadX();
			m_fTouchPadY[iLeftRightIndex] = (float)controllerState.TouchpadY();
		}
		else
		{
			m_bTouchPadTouched[iLeftRightIndex] = false;
			m_bTouchPadPressed[iLeftRightIndex] = false;
			m_fTouchPadX[iLeftRightIndex] = 0;
			m_fTouchPadY[iLeftRightIndex] = 0;
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
				m_fHandX[iLeftRightIndex] = motioncontrollerpose.Position().x;
				m_fHandY[iLeftRightIndex] = motioncontrollerpose.Position().y;
				m_fHandZ[iLeftRightIndex] = motioncontrollerpose.Position().z;
				m_qHandOrientation[iLeftRightIndex] = motioncontrollerpose.Orientation();
			}
		}
	}
}
