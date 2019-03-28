//
// GGWMR - Windows Mixed Reality DLL (WinRT hooked)
//

// Includes for Windows Mixed Reality
#include "stdafx.h"
#include "GGWMR.h"
#include <HolographicSpaceInterop.h>
#include <windows.graphics.holographic.h>
#include <winrt\Windows.Graphics.Holographic.h>

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

/*
// Includes for Windows Mixed Reality
#include <thread>
#include <memory>
#include "Common\DeviceResources.h"

#include <..\winrt\WinRTBase.h>
#include <windows.graphics.holographic.h>
#include <windows.ui.input.spatial.h>
#include <..\um\HolographicSpaceInterop.h>
#include <..\um\SpatialInteractionManagerInterop.h>
#include <wrl.h>

// Namespaces for Windows Mixed Reality
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Graphics::Holographic;
using namespace ABI::Windows::UI::Input::Spatial;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Perception::Spatial;
*/

// Globals
//Windows::Graphics::Holographic::HolographicSpace^ m_holographicSpace;
//Windows::UI::Input::Spatial::SpatialInteractionManager^ m_spatialInteractionManager;
App app;

// DLL Entry Function
BOOL APIENTRY GGWMRMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
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

    // Main message loop:
    ///app.Run(); moved below to be called in loop

    // Perform application teardown.
    ///app.Uninitialize(); for end

	/*
	// Initialise COM
	CoInitializeEx(nullptr, 0);

	// Create Holographic Space 
    ComPtr<IHolographicSpaceStatics> spHolographicSpaceFactory;
    HRESULT hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Holographic_HolographicSpace).Get(), &spHolographicSpaceFactory);
    ComPtr<IHolographicSpaceInterop> spHolographicSpaceInterop;
    if (SUCCEEDED(hr))
    {
        hr = spHolographicSpaceFactory.As(&spHolographicSpaceInterop);
    }
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicSpace> spHolographicSpace;
    if (SUCCEEDED(hr))
    {
        hr = spHolographicSpaceInterop->CreateForWindow(hWnd, IID_PPV_ARGS(&spHolographicSpace));
        if (SUCCEEDED(hr))
        {
            m_holographicSpace = reinterpret_cast<Windows::Graphics::Holographic::HolographicSpace^>(spHolographicSpace.Get());
        }
    }

	// Create Spatial Interaction Manager
	ComPtr<ISpatialInteractionManagerStatics> spSpatialInteractionFactory;
    hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Input_Spatial_SpatialInteractionManager).Get(), &spSpatialInteractionFactory);
    ComPtr<ISpatialInteractionManagerInterop> spSpatialInterop;
    if (SUCCEEDED(hr))
    {
        hr = spSpatialInteractionFactory.As(&spSpatialInterop);
    }
    ComPtr<ISpatialInteractionManager> spSpatialInteractionManager;
    if (SUCCEEDED(hr))
    {
        hr = spSpatialInterop->GetForWindow(hWnd, IID_PPV_ARGS(&spSpatialInteractionManager));
        if (SUCCEEDED(hr))
        {
            m_spatialInteractionManager = reinterpret_cast<Windows::UI::Input::Spatial::SpatialInteractionManager^>(spSpatialInteractionManager.Get());
        }
    }
	*/
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

DLLEXPORT void GGWMR_InitHolographicSpace ( void* LEyeImage, void* REyeImage )
{
    app.Run(LEyeImage, REyeImage);
}

DLLEXPORT void GGWMR_DestroyHolographicSpace ( void )
{
	app.Uninitialize();
	app.SetInitialised(false);
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

	// LEE: On second pass, get error "'CreateForWindow requires a window (HWND) which has not been used in a previous call to CreateForWindow.'."
	// suggesting there is a way to KEEp the hologpahic space, just need to find a way to BRING IT BACK as it stays hidden once we use device and devicecontext for GG rendering!?!
    winrt::check_hresult(holographicSpaceInterop->CreateForWindow(hWnd, __uuidof(ABI::Windows::Graphics::Holographic::IHolographicSpace), winrt::put_abi(spHolographicSpace)));

    if (!spHolographicSpace)
    {
        winrt::check_hresult(E_FAIL);
    }

    // Store the holographic space.
    m_holographicSpace = spHolographicSpace.as<HolographicSpace>();
}

void App::CreateHolographicSpaceB(ID3D11Device* pDevice,ID3D11DeviceContext* pContext)
{
    // The DeviceResources class uses the preferred DXGI adapter ID from the holographic
    // space (when available) to create a Direct3D device. The HolographicSpace
    // uses this ID3D11Device to create and manage device-based resources such as
    // swap chains.
    m_deviceResources->SetHolographicSpace ( m_holographicSpace, pDevice, pContext );

    // The main class uses the holographic space for updates and rendering.
    m_main->SetHolographicSpace(hWnd, m_holographicSpace);
}

void App::Run(void* pLeftCamTex, void* pRightCamTex)
{
	// message pump code when idle
    if (m_windowVisible && (m_holographicSpace != nullptr))
    {
		// Update
        HolographicFrame holographicFrame = m_main->Update();

		// Render
        if (m_main->Render(holographicFrame, pLeftCamTex, pRightCamTex))
        {
            // The holographic frame has an API that presents the swap chain for each HolographicCamera
            m_deviceResources->Present(holographicFrame);
        }
        else
        {
            Sleep(10);
        }
    }
}

void App::Uninitialize()
{
	// completely free Holographic Space    
	m_main.reset();
	m_deviceResources.reset();
}
