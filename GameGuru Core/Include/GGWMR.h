//#include "windows.h"

#pragma once

//#include "resource.h"
#include "Common/DeviceResources.h"
#include "BasicHologramMain.h"

#define DLLEXPORT __declspec ( dllexport )
extern "C" 
{
	DLLEXPORT void GGWMR_CreateHolographicSpace1 ( HWND hWnd );
	DLLEXPORT void GGWMR_CreateHolographicSpace2 ( void* pD3DDevice, void* pD3DContext );
	DLLEXPORT void GGWMR_GetUpdate ( void );
	DLLEXPORT void GGWMR_GetRenderTargetAndDepthStencilView ( void** ppRenderTarget, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight );
	DLLEXPORT void GGWMR_Present ( void );
	DLLEXPORT void GGWMR_DestroyHolographicSpace ( void );
}

namespace BasicHologram
{
    // IFrameworkView class. Connects the app with the Windows shell and handles application lifecycle events.
    class App sealed
    {
    public:
        void Initialize();
        void CreateHolographicSpaceA(HWND hWnd);
        void CreateHolographicSpaceB(ID3D11Device* pDevice,ID3D11DeviceContext* pContext);
		void UpdateFrame();
		void UpdateRender();
		void GetRenderTargetAndDepthStencilView ( void** ppRenderTarget, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight );
		void Present();
        //void Run(void* pLeftCamTex, void* pRightCamTex);
        void Uninitialize();

		void SetInitialised(bool bState) { m_initialised = bState; }
		bool GetInitialised() { return m_initialised; }

    private:
		bool													m_initialised = false;
        std::unique_ptr<BasicHologramMain>						m_main;

        HWND hWnd;

        std::shared_ptr<DX::DeviceResources>                    m_deviceResources;
        bool                                                    m_windowClosed = false;
        bool                                                    m_windowVisible = true;
		bool													m_canPresentThisFrame = false;

        HINSTANCE m_hInst;                                // current instance
        LPCWSTR m_szTitle = L"Windows Mixed Reality Win32 App";
        LPCWSTR m_szWindowClass = L"Windows Mixed Reality Win32 App"; // The title bar text

        // The holographic space the app will use for rendering.
        winrt::Windows::Graphics::Holographic::HolographicSpace m_holographicSpace = nullptr;
    };
}
