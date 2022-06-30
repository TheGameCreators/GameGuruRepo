#include "stdafx.h"

#include "CameraResources.h"
#include "Common/DirectXHelper.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Perception::Spatial;

int DebugVRlog ( const char* pReportLog );

DX::CameraResources::CameraResources(HolographicCamera const& camera) :
    m_holographicCamera(camera),
    m_isStereo(camera.IsStereo()),
    m_d3dRenderTargetSize(camera.RenderTargetSize())
{
	//m_holographicCamera.SetNearPlaneDistance(1.0f);
	//m_holographicCamera.SetFarPlaneDistance(70000.0f); created big Z clash!
	m_holographicCamera.SetNearPlaneDistance(5.0f);
	m_holographicCamera.SetFarPlaneDistance(70000.0f);

    m_d3dViewport = CD3D11_VIEWPORT(
        0.f, 0.f,
        m_d3dRenderTargetSize.Width,
        m_d3dRenderTargetSize.Height
    );
};

// Updates resources associated with a holographic camera's swap chain.
// The app does not access the swap chain directly, but it does create
// resource views for the back buffer.
void DX::CameraResources::CreateResourcesForBackBuffer(
    DX::DeviceResources* pDeviceResources,
    HolographicCameraRenderingParameters const& cameraParameters
)
{
    ID3D11Device* device = NULL;
	try
	{
		device = pDeviceResources->GetD3DDevice();
	}
	catch(...)
	{
		DebugVRlog("failed pDeviceResources->GetD3DDevice()");
	}

    // Get the WinRT object representing the holographic camera's back buffer.
    IDirect3DSurface surface = nullptr;
	try
	{
		surface = cameraParameters.Direct3D11BackBuffer();
	}
	catch(...)
	{
		DebugVRlog("failed cameraParameters.Direct3D11BackBuffer()");
	}

    // Get the holographic camera's back buffer.
    // Holographic apps do not create a swap chain themselves; instead, buffers are
    // owned by the system. The Direct3D back buffer resources are provided to the
    // app using WinRT interop APIs.
    ComPtr<ID3D11Texture2D> cameraBackBuffer;
	try
	{
	    winrt::check_hresult(surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>()->GetInterface(IID_PPV_ARGS(&cameraBackBuffer)));
	}
	catch(...)
	{
		DebugVRlog("failed IDirect3DDxgiInterfaceAccess");
	}

    // Determine if the back buffer has changed. If so, ensure that the render target view
    // is for the current back buffer.
    if (m_d3dBackBuffer.Get() != cameraBackBuffer.Get())
    {
        // This can change every frame as the system moves to the next buffer in the
        // swap chain. This mode of operation will occur when certain rendering modes
        // are activated.
        m_d3dBackBuffer = cameraBackBuffer;

        // Get the DXGI format for the back buffer.
        D3D11_TEXTURE2D_DESC backBufferDesc;
		try
		{
	        m_d3dBackBuffer->GetDesc(&backBufferDesc);
		}
		catch(...)
		{
			DebugVRlog("failed m_d3dBackBuffer->GetDesc(&backBufferDesc)");
		}
        m_dxgiFormat = backBufferDesc.Format;

		// instead, split backbuffer into left and right render target views
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = m_dxgiFormat;//DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.ArraySize = 1;
		try
		{
			rtvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, 0, 1);
			winrt::check_hresult(device->CreateRenderTargetView(m_d3dBackBuffer.Get(),&rtvDesc,&m_d3dRenderTargetLeftView));
			rtvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, 1, 1);
			winrt::check_hresult(device->CreateRenderTargetView(m_d3dBackBuffer.Get(),&rtvDesc,&m_d3dRenderTargetRightView));
		}
		catch(...)
		{
			DebugVRlog("failed CreateRenderTargetView");
		}

        // Check for render target size changes.
		try
		{
			winrt::Windows::Foundation::Size currentSize = m_holographicCamera.RenderTargetSize();
			if (m_d3dRenderTargetSize != currentSize)
			{
				// Set render target size.
				m_d3dRenderTargetSize = currentSize;

				// A new depth stencil view is also needed.
				m_d3dDepthStencilView.Reset();
			}
		}
		catch(...)
		{
			DebugVRlog("failed m_holographicCamera.RenderTargetSize()");
		}

		// create shader resource view of left backbuffer (will be used to copy to camera zero for preview)
		if ( m_d3dLeftShaderResourceView == nullptr )
		{
			DebugVRlog("CreateLeftShaderResourceView");
			try
			{
				CD3D11_SHADER_RESOURCE_VIEW_DESC leftShaderResourceViewDesc(
					D3D11_SRV_DIMENSION_TEXTURE2D,
					m_dxgiFormat
				);
				winrt::check_hresult(
					device->CreateShaderResourceView(
						m_d3dBackBuffer.Get(),
						&leftShaderResourceViewDesc,
						&m_d3dLeftShaderResourceView
					));
			}
			catch(...)
			{
				DebugVRlog("failed CreateLeftShaderResourceView");
			}
		}
    }

    // Refresh depth stencil resources, if needed.
    if (m_d3dDepthStencilView == nullptr)
    {
        // Create a depth stencil view for use with 3D rendering if needed.
		DebugVRlog("depthStencilDesc");
        CD3D11_TEXTURE2D_DESC depthStencilDesc(
            DXGI_FORMAT_R32_TYPELESS,//DXGI_FORMAT_R16_TYPELESS,
            static_cast<UINT>(m_d3dRenderTargetSize.Width),
            static_cast<UINT>(m_d3dRenderTargetSize.Height),
            1, // we are handling render target views separately now - m_isStereo ? 2 : 1, // Create two textures when rendering in stereo.
            1, // Use a single mipmap level.
            D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
        );

		DebugVRlog("CreateTexture2D");
        winrt::check_hresult(
            device->CreateTexture2D(
                &depthStencilDesc,
                nullptr,
                &m_d3dDepthStencil
            ));

		DebugVRlog("CreateDepthStencilView");
		try
		{
			CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
				m_isStereo ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D,
				//DXGI_FORMAT_D16_UNORM
				DXGI_FORMAT_D32_FLOAT
			);
			winrt::check_hresult(
				device->CreateDepthStencilView(
					m_d3dDepthStencil.Get(),
					&depthStencilViewDesc,
					&m_d3dDepthStencilView
				));
		}
		catch(...)
		{
			DebugVRlog("failed CreateDepthStencilView");
		}
    }

    // Create the constant buffer, if needed.
    if (m_viewProjectionConstantBuffer == nullptr)
    {
        // Create a constant buffer to store view and projection matrices for the camera.
		DebugVRlog("CreateBuffer");
        CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		try
		{
			winrt::check_hresult(
				device->CreateBuffer(
					&constantBufferDesc,
					nullptr,
					&m_viewProjectionConstantBuffer
				));
		}
		catch(...)
		{
			DebugVRlog("failed CreateBuffer");
		}
    }
}

// Releases resources associated with a back buffer.
void DX::CameraResources::ReleaseResourcesForBackBuffer(DX::DeviceResources* pDeviceResources)
{
    ID3D11DeviceContext* context = pDeviceResources->GetD3DDeviceContext();

    // Release camera-specific resources.
    m_d3dBackBuffer.Reset();
    m_d3dDepthStencil.Reset();
    m_d3dRenderTargetLeftView.Reset();
    m_d3dRenderTargetRightView.Reset();
    m_d3dDepthStencilView.Reset();
    m_viewProjectionConstantBuffer.Reset();

    // Ensure system references to the back buffer are released by clearing the render
    // target from the graphics pipeline state, and then flushing the Direct3D context.
    ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
    context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
    context->Flush();
}

// Gets the view-projection constant buffer for the HolographicCamera and attaches it
// to the shader pipeline.
bool DX::CameraResources::AttachViewProjectionBuffer(
    std::shared_ptr<DX::DeviceResources>& deviceResources
)
{
    // This method uses Direct3D device-based resources.
    ID3D11DeviceContext* context = deviceResources->GetD3DDeviceContext();

    // Loading is asynchronous. Resources must be created before they can be updated.
    // Cameras can also be added asynchronously, in which case they must be initialized
    // before they can be used.
    if (context == nullptr || m_viewProjectionConstantBuffer == nullptr || m_framePending == false)
    {
        return false;
    }

    // Set the viewport for this camera.
    context->RSSetViewports(1, &m_d3dViewport);

    // Send the constant buffer to the vertex shader.
    context->VSSetConstantBuffers(
        1,
        1,
        m_viewProjectionConstantBuffer.GetAddressOf()
    );

    m_framePending = false;

    return true;
}
