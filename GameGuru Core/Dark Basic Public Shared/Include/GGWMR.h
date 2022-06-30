//#include "windows.h"

#pragma once

//#include "resource.h"
#include "Common/DeviceResources.h"
#include "BasicHologramMain.h"
#include <SpatialInteractionManagerInterop.h>
#include <windows.ui.input.spatial.h>
#include <winrt\windows.ui.input.spatial.h>

#define DLLEXPORT __declspec ( dllexport )
extern "C" 
{
	DLLEXPORT int GGWMR_CreateHolographicSpace1 ( HWND hWnd, int iDebugMode );
	DLLEXPORT void GGWMR_ReconnectWithHolographicSpaceControllers ( void );
	DLLEXPORT int GGWMR_CreateHolographicSpace2 ( void* pD3DDevice, void* pD3DContext );
	DLLEXPORT void GGWMR_DeleteHolographicSpace(void);
	DLLEXPORT void GGWMR_GetUpdate ( void );
	DLLEXPORT void GGWMR_GetHeadPosAndDir ( float* pPosX, float* pPosY, float* pPosZ, float* pUpX, float* pUpY, float* pUpZ, float* pDirX, float* pDirY, float* pDirZ );
	DLLEXPORT void GGWMR_GetProjectionMatrix ( int iEyeIndex,	float* pM00, float* pM10, float* pM20, float* pM30, 
																float* pM01, float* pM11, float* pM21, float* pM31,
																float* pM02, float* pM12, float* pM22, float* pM32,
																float* pM03, float* pM13, float* pM23, float* pM33);
	DLLEXPORT void GGWMR_GetThumbAndTrigger ( float* pSideButtonValue, float* pTriggerValue, float* pThumbStickX, float* pThumbStickY );
	DLLEXPORT void GGWMR_GetTouchPadData ( bool* pbTouchedisRightHand, bool* pbTouched, bool* pbPressed, float* pfTouchPadX, float* pfTouchPadY );
	DLLEXPORT void GGWMR_GetHandPosAndOrientation ( int iLeftHandMode, float* pRHX, float* pRHY, float* pRHZ, float* pQuatW, float* pQuatX, float* pQuatY, float* pQuatZ );
	DLLEXPORT void GGWMR_GetRenderTargetAndDepthStencilView ( void** ppRenderTargetLeft, void** ppRenderTargetRight, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight, void** ppLeftShaderResourceView );
	DLLEXPORT void GGWMR_Present ( void );
}

namespace BasicHologram
{
    // IFrameworkView class. Connects the app with the Windows shell and handles application lifecycle events.
    class App sealed
    {
    public:
        void Initialize();
        int CreateHolographicSpaceA(HWND hWnd);
		void ReconnectWithHolographicSpaceControllers(void);
        int CreateHolographicSpaceB(ID3D11Device* pDevice,ID3D11DeviceContext* pContext);
		void Uninitialize();
		void DeleteHolographicSpace();

		void UpdateFrame();
		void GetHeadPosAndDir ( float* pPosX, float* pPosY, float* pPosZ, float* pUpX, float* pUpY, float* pUpZ, float* pDirX, float* pDirY, float* pDirZ );
		void GetThumbAndTrigger ( float* pSideButtonValue, float* pTriggerValue, float* pThumbStickX, float* pThumbStickY );
		void GetTouchPadData ( bool* pbTouchedisRightHand, bool* pbTouched, bool* pbPressed, float* pfTouchPadX, float* pfTouchPadY );
		void GetHandPosAndOrientation ( int iLeftHandMode, float* pRHX, float* pRHY, float* pRHZ, float* pQuatW, float* pQuatX, float* pQuatY, float* pQuatZ );
		void GetProjectionMatrix ( int iEyeIndex,	float* pM00, float* pM10, float* pM20, float* pM30, 
													float* pM01, float* pM11, float* pM21, float* pM31,
													float* pM02, float* pM12, float* pM22, float* pM32,
													float* pM03, float* pM13, float* pM23, float* pM33);
		void UpdateRender();
		void GetRenderTargetAndDepthStencilView ( void** ppRenderTargetLeft, void** ppRenderTargetRight, void** ppDepthStencil, DWORD* pdwWidth, DWORD* pdwHeight, void** ppLeftShaderResourceView );
		void Present();

		void SetInitialised(bool bState) { m_initialised = bState; }
		bool GetInitialised() { return m_initialised; }

		//winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceState CheckForInput();

    private:
		bool													m_initialised = false;
        std::unique_ptr<BasicHologramMain>						m_main;
        std::shared_ptr<DX::DeviceResources>                    m_deviceResources;
        bool                                                    m_windowClosed = false;
        bool                                                    m_windowVisible = true;
		bool													m_canPresentThisFrame = false;

        HINSTANCE m_hInst;                                // current instance
        LPCWSTR m_szTitle = L"Windows Mixed Reality Win32 App";
        LPCWSTR m_szWindowClass = L"Windows Mixed Reality Win32 App"; // The title bar text

        // The holographic space the app will use for rendering.
        winrt::Windows::Graphics::Holographic::HolographicSpace m_holographicSpace = nullptr;

	private:
        // Interaction event handler.
        //void OnSourcePressed(
        //    winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& sender,
       //     winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs const& args);

		void OnSourceLost(
			winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& sender,
			winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs const& args);

		void OnSourceUpdated(
			winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& sender,
			winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs const& args);

        //void OnSourceReleased(
        //    winrt::Windows::UI::Input::Spatial::SpatialInteractionManager const& sender,
        //    winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceEventArgs const& args);

        // API objects used to process gesture input, and generate gesture events.
        winrt::Windows::UI::Input::Spatial::SpatialInteractionManager       m_interactionManager = nullptr;

		// store controller input
		float																m_fSideButtonValue[2];
		float																m_fTriggerValue[2];
		float																m_fThumbX[2];
		float																m_fThumbY[2];

		bool																m_bTouchPadTouched[2];
		bool																m_bTouchPadPressed[2];
		float																m_fTouchPadX[2];
		float																m_fTouchPadY[2];

		float																m_fHandX[2] = {0,0};
		float																m_fHandY[2] = {0,0};
		float																m_fHandZ[2] = {0,0};
		winrt::Windows::Foundation::Numerics::quaternion					m_qHandOrientation[2];


        // Event registration token.
        //winrt::event_token												m_sourcePressedEventToken;
		winrt::event_token													m_sourceUpdatedEventToken;
        //winrt::event_token												m_sourceReleasedEventToken;

        // Used to indicate that a Pressed input event was received this frame.
        //winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceState   m_sourceState = nullptr;
	};
}
