
#include "stdafx.h"
#include "DeviceResources.h"
#include "DirectXHelper.h"

using namespace D2D1;
using namespace Microsoft::WRL;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::Graphics::Holographic;

// Constructor for DeviceResources.
DX::DeviceResources::DeviceResources()
{
    CreateDeviceIndependentResources();
}

DX::DeviceResources::~DeviceResources()
{
	// free anything created by device resources from createdeviceresources
	m_d3dInteropDevice.Close();
}

// Configures resources that don't depend on the Direct3D device.
void DX::DeviceResources::CreateDeviceIndependentResources()
{
}

void DX::DeviceResources::SetHolographicSpace(HolographicSpace holographicSpace,ID3D11Device* pDevice,ID3D11DeviceContext* pContext)
{
    // Cache the holographic space. Used to re-initalize during device-lost scenarios.
    m_holographicSpace = holographicSpace;

    InitializeUsingHolographicSpace(pDevice,pContext);
}

void DX::DeviceResources::InitializeUsingHolographicSpace(ID3D11Device* pDevice,ID3D11DeviceContext* pContext)
{
    CreateDeviceResources(pDevice,pContext);
    m_holographicSpace.SetDirect3D11Device(m_d3dInteropDevice);
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DX::DeviceResources::CreateDeviceResources(ID3D11Device* pDevice,ID3D11DeviceContext* pContext)
{
    // Create the Direct3D 11 API device object and a corresponding context.
    ComPtr<ID3D11Device> device = pDevice;
    ComPtr<ID3D11DeviceContext> context = pContext;

    // Store pointers to the Direct3D device and immediate context.
    winrt::check_hresult(device.As(&m_d3dDevice));
    winrt::check_hresult(context.As(&m_d3dContext));

    // Acquire the DXGI interface for the Direct3D device.
    ComPtr<IDXGIDevice3> dxgiDevice;
    winrt::check_hresult(m_d3dDevice.As(&dxgiDevice));

    // Wrap the native device using a WinRT interop object.
    winrt::com_ptr<::IInspectable> object;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(
        dxgiDevice.Get(),
        reinterpret_cast<IInspectable**>(winrt::put_abi(object))));
    m_d3dInteropDevice = object.as<IDirect3DDevice>();

    // Check for device support for the optional feature that allows setting the render target array index from the vertex shader stage.
    D3D11_FEATURE_DATA_D3D11_OPTIONS3 options;
    m_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &options, sizeof(options));
    if (options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer)
    {
        m_supportsVprt = true;
    }
}

// Validates the back buffer for each HolographicCamera and recreates
// resources for back buffers that have changed.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::EnsureCameraResources(
    HolographicFrame frame,
    HolographicFramePrediction prediction)
{
    UseHolographicCameraResources<void>([this, frame, prediction](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
		try
		{
			for (HolographicCameraPose const& cameraPose : prediction.CameraPoses())
			{
				HolographicCameraRenderingParameters renderingParameters = frame.GetRenderingParameters(cameraPose);
				CameraResources* pCameraResources = cameraResourceMap[cameraPose.HolographicCamera().Id()].get();
				pCameraResources->CreateResourcesForBackBuffer(this, renderingParameters);
			}
		}
		catch(...)
		{
			// maybe next time (this will exception if HMD not generating a valid cameraPose/frame/prediction..)
		}
    });
}

// Prepares to allocate resources and adds resource views for a camera.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::AddHolographicCamera(HolographicCamera camera)
{
    UseHolographicCameraResources<void>([this, camera](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        cameraResourceMap[camera.Id()] = std::make_unique<CameraResources>(camera);
    });
}

// Deallocates resources for a camera and removes the camera from the set.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::RemoveHolographicCamera(HolographicCamera camera)
{
    UseHolographicCameraResources<void>([this, camera](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        CameraResources* pCameraResources = cameraResourceMap[camera.Id()].get();

        if (pCameraResources != nullptr)
        {
            pCameraResources->ReleaseResourcesForBackBuffer(this);
            cameraResourceMap.erase(camera.Id());
        }
    });
}

// Recreate all device resources and set them back to the current state.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::HandleDeviceLost()
{
    if (m_deviceNotify != nullptr)
    {
        m_deviceNotify->OnDeviceLost();
    }

    UseHolographicCameraResources<void>([this](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        for (auto& pair : cameraResourceMap)
        {
            CameraResources* pCameraResources = pair.second.get();
            pCameraResources->ReleaseResourcesForBackBuffer(this);
        }
    });

    if (m_deviceNotify != nullptr)
    {
        m_deviceNotify->OnDeviceRestored();
    }
}

// Register our DeviceNotify to be informed on device lost and creation.
void DX::DeviceResources::RegisterDeviceNotify(DX::IDeviceNotify* deviceNotify)
{
    m_deviceNotify = deviceNotify;
}

// Call this method when the app suspends. It provides a hint to the driver that the app
// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
void DX::DeviceResources::Trim()
{
    m_d3dContext->ClearState();

    ComPtr<IDXGIDevice3> dxgiDevice;
    winrt::check_hresult(m_d3dDevice.As(&dxgiDevice));
    dxgiDevice->Trim();
}

// Present the contents of the swap chain to the screen.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::Present(HolographicFrame frame)
{
    // By default, this API waits for the frame to finish before it returns.
    // Holographic apps should wait for the previous frame to finish before
    // starting work on a new frame. This allows for better results from
    // holographic frame predictions.
    // NOTE: When using one of the WaitForNextFrameReady APIs, the HolographicFrame 
    //       must be presented with the DoNotWaitForFrameToFinish flag.
    HolographicFramePresentResult presentResult = frame.PresentUsingCurrentPrediction(HolographicFramePresentWaitBehavior::DoNotWaitForFrameToFinish);

    // The PresentUsingCurrentPrediction API will detect when the graphics device
    // changes or becomes invalid. When this happens, it is considered a Direct3D
    // device lost scenario.
    if (presentResult == HolographicFramePresentResult::DeviceRemoved)
    {
        // The Direct3D device, context, and resources should be recreated.
        HandleDeviceLost();
    }
}
