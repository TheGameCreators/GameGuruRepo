//
// GGWMR - Windows Mixed Reality DLL (WinRT hooked)
//

// Includes for Windows Mixed Reality
#include "GGWMR.h"
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

// Constants
const float GGWMR_SINGULARITYRADIUS = 0.0000001f; // Use for Gimbal lock numerical problems
const float GGWMR_180DIVPI = 57.2957795130823f;
const float GGWMR_PI = 3.14159265358979f;
const float GGWMR_PIOVER2 = 0.5f * GGWMR_PI;
const float GGWMR_radian = 0.0174533f;

// Globals
Windows::Graphics::Holographic::HolographicSpace^ m_holographicSpace;
Windows::UI::Input::Spatial::SpatialInteractionManager^ m_spatialInteractionManager;

// DLL Entry Function
BOOL APIENTRY GGWMRMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

DLLEXPORT void GGWMR_GetHolographicSpace ( HWND hWnd )
{
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
	Windows::Perception::Spatial::SpatialLocator^ m_locator = SpatialLocator::GetDefault();
	Windows::Perception::Spatial::SpatialStationaryFrameOfReference^ m_referenceFrame = m_locator->CreateStationaryFrameOfReferenceAtCurrentLocation();

	// LOOP UPDATE - Manage Holographic Space (see MRWin32 project for more details) [note the namespaces point to a DIFFERENT Holographic which is a ref class C++/CX)
	// get current coordinate system
    // SpatialCoordinateSystem^ currentCoordinateSystem = m_referenceFrame->CoordinateSystem;
	// get next frame and from it get current prediction
    // Windows::Graphics::Holographic::HolographicFrame^ holographicFrame = m_holographicSpace->CreateNextFrame();
    // Windows::Graphics::Holographic::HolographicFramePrediction^ prediction = holographicFrame->CurrentPrediction;

	// loop
	//HolographicFrame holographicFrame = m_main->Update();
    //if (m_main->Render(holographicFrame))
    //    m_deviceResources->Present(holographicFrame);
    //else
    //    Sleep(10);

	// finish VR
	m_main.reset();
	m_deviceResources.reset();
}

DLLEXPORT void GGWMR_InitHolographicSpace ( void** LEyeImage, void** REyeImage )
{
	// test to see if we can read the surface desc from this DLL
	//ID3D11Texture2D* pLeftEye = (ID3D11Texture2D*)LEyeImage;
	//ID3D11Texture2D* pRightEye = (ID3D11Texture2D*)REyeImage;
	//D3D11_TEXTURE2D_DESC imageddsd;
	//pLeftEye->GetDesc(&imageddsd);
	//imageddsd.Width = imageddsd.Height;

	//
	// HolographicSpace requires a Direct3D11 device, create one and call SetDirect3D11Device
    std::shared_ptr<DX::DeviceResources> m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetHolographicSpace(m_holographicSpace);

	// HMM, Graft simple WinRT sample into this DLL and get it rendering the cube, this way we know its possible
	// to have VR working from the GG test game button press, then we can migrate over real content..

}
