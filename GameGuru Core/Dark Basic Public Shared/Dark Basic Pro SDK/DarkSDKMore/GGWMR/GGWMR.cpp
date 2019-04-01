//
// GGWMR - Windows Mixed Reality DLL (WinRT hooked)
//

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

// DLL Entry Function
BOOL APIENTRY GGWMRMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	hModule = hModule;
	ul_reason_for_call = ul_reason_for_call;
	lpReserved = lpReserved;
	return TRUE;
}

DLLEXPORT void GGWMR_CreateHolographicSpace1 ( HWND hWnd )
{
	if ( app.GetInitialised() == false )
	{
		// COM init
		winrt::init_apartment();

		// Initialize global strings, and perform application initialization.
		app.Initialize();

		// Create the HWND and the HolographicSpace.
		app.CreateHolographicSpaceA(hWnd);
	}
}

DLLEXPORT void GGWMR_CreateHolographicSpace2 ( void* pD3DDevice, void* pD3DContext )
{
	if ( app.GetInitialised() == false )
	{
		// now initialised
		app.SetInitialised(true);

		// complete holographic space creation using existing device and context from engine
	    app.CreateHolographicSpaceB((ID3D11Device*)pD3DDevice, (ID3D11DeviceContext*)pD3DContext);
	}
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
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    m_main = std::make_unique<BasicHologramMain>(m_deviceResources);
}

void App::CreateHolographicSpaceA(HWND hWnd)
{
    // Use WinRT factory to create the holographic space.
    using namespace winrt::Windows::Graphics::Holographic;
    winrt::com_ptr<IHolographicSpaceInterop> holographicSpaceInterop = winrt::get_activation_factory<HolographicSpace, IHolographicSpaceInterop>();
    winrt::com_ptr<ABI::Windows::Graphics::Holographic::IHolographicSpace> spHolographicSpace;
    winrt::check_hresult(holographicSpaceInterop->CreateForWindow(hWnd, __uuidof(ABI::Windows::Graphics::Holographic::IHolographicSpace), winrt::put_abi(spHolographicSpace)));
    if (!spHolographicSpace)
    {
        winrt::check_hresult(E_FAIL);
    }

    // Store the holographic space.
    m_holographicSpace = spHolographicSpace.as<HolographicSpace>();

	// Create spatial interaction manager for hand/controller input
	winrt::com_ptr<ISpatialInteractionManagerInterop> spatialInteractionManagerInterop = winrt::get_activation_factory<SpatialInteractionManager, ISpatialInteractionManagerInterop>();
	if (!spatialInteractionManagerInterop)
	{
		winrt::check_hresult(E_FAIL);
	}
	winrt::com_ptr<ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager> spSpatialInteractionManager;
	winrt::check_hresult(spatialInteractionManagerInterop->GetForWindow(hWnd, __uuidof(ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager), winrt::put_abi(spSpatialInteractionManager)));
	if (!spSpatialInteractionManager)
	{
		winrt::check_hresult(E_FAIL);
	}

	m_interactionManager = spSpatialInteractionManager.as<SpatialInteractionManager>();

	m_sourcePressedEventToken = m_interactionManager.SourcePressed(bind(&App::OnSourcePressed, this, _1, _2));
	m_sourceUpdatedEventToken = m_interactionManager.SourceUpdated(bind(&App::OnSourceUpdated, this, _1, _2));
	m_sourceReleasedEventToken = m_interactionManager.SourceReleased(bind(&App::OnSourceReleased, this, _1, _2));
}

void App::CreateHolographicSpaceB(ID3D11Device* pDevice,ID3D11DeviceContext* pContext)
{
    // The DeviceResources class uses the preferred DXGI adapter ID from the holographic
    // space (when available) to create a Direct3D device. The HolographicSpace
    // uses this ID3D11Device to create and manage device-based resources such as
    // swap chains.
    m_deviceResources->SetHolographicSpace ( m_holographicSpace, pDevice, pContext );

    // The main class uses the holographic space for updates and rendering.
    m_main->SetHolographicSpace(m_holographicSpace, &m_interactionManager);
}

void App::UpdateFrame()
{
	g_holographicFrame = nullptr;
    if (m_holographicSpace != nullptr)
    {
        g_holographicFrame = m_main->Update();

		// direct access to controller input
		if ( m_interactionManager != nullptr && g_holographicFrame != nullptr )
		{
			if ( g_holographicFrame.CurrentPrediction() != nullptr )
			{
				auto states = m_interactionManager.GetDetectedSourcesAtTimestamp(g_holographicFrame.CurrentPrediction().Timestamp());
				for (const auto& state : states)
				{
					bool bThumbStickpressed = state.ControllerProperties().IsThumbstickPressed();
					if ( bThumbStickpressed )
					{
						MessageBox ( NULL, "can detect controller input now", "", MB_OK );
					}
					double fThumbX = state.ControllerProperties().ThumbstickX();
					fThumbX=fThumbX;
				}
			}
		}

		/* not using event based for now - though would pickup input on SLOWER systems!!
		// not working (see inside Update)
		SpatialInteractionSourceState pointerState = CheckForInput();
		if ( pointerState != nullptr )
		{
			bool bPressed = pointerState.IsSelectPressed();
			if ( bPressed == true )
			{
				MessageBox ( NULL, "hello", "", MB_OK );
			}
		}
		*/
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

SpatialInteractionSourceState App::CheckForInput()
{
    SpatialInteractionSourceState sourceState = m_sourceState;
    m_sourceState = nullptr;
    return sourceState;
}

void App::OnSourcePressed(SpatialInteractionManager const& /*sender*/, SpatialInteractionSourceEventArgs const& args)
{
	// see above - not using this way for now
    m_sourceState = args.State();
}

void App::OnSourceReleased(SpatialInteractionManager const& /*sender*/, SpatialInteractionSourceEventArgs const& args)
{
	// see above
}

void App::OnSourceUpdated(SpatialInteractionManager const& /*sender*/, SpatialInteractionSourceEventArgs const& args)
{
	/* see above
	winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceState state = args.State;
	SpatialInteractionSource source = state.Source;
	SpatialInteractionController controller = source.Controller;
	if (controller != nullptr)
    {
        // Give Thumbstuck priority over Touchpad for tool selection.
        if (controller.HasThumbstick)
        {
			SpatialInteractionControllerProperties controllerState = args.State.ControllerProperties;
			double x = controllerState.ThumbstickX;
			double y = controllerState.ThumbstickY;
		}
	}
	*/
}
