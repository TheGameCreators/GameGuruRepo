#pragma once

//
// Comment out this preprocessor definition to disable all of the
// sample content.
//
// To remove the content after disabling it:
//     * Remove the unused code from your app's Main class.
//     * Delete the Content folder provided with this template.
//
#define DRAW_SAMPLE_CONTENT

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

// Updates, renders, and presents holographic content using Direct3D.
namespace BasicHologram
{
    class BasicHologramMain : public DX::IDeviceNotify
    {
    public:
        BasicHologramMain(std::shared_ptr<DX::DeviceResources> const& deviceResources);
        ~BasicHologramMain();

        // Sets the holographic space. This is our closest analogue to setting a new window
        // for the app.
        void SetHolographicSpace(winrt::Windows::Graphics::Holographic::HolographicSpace const& holographicSpace, winrt::Windows::UI::Input::Spatial::SpatialInteractionManager* interactionManager);

        // Starts the holographic frame and updates the content.
        winrt::Windows::Graphics::Holographic::HolographicFrame Update(winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference* pstationaryReferenceFrame);

        // Renders holograms, including world-locked content.
        bool Render(winrt::Windows::Graphics::Holographic::HolographicFrame const& holographicFrame);

        // Handle saving and loading of app state owned by AppMain.
        void SaveAppState();
        void LoadAppState();

        // Handle mouse input.
        void OnPointerPressed();

        // IDeviceNotify
        void OnDeviceLost() override;
        void OnDeviceRestored() override;

		// Handle extraction of render views
		ID3D11RenderTargetView*	GetPassOutRenderTargetLeftView(void) { return m_pPassOutRenderTargetLeftView; }
		ID3D11RenderTargetView*	GetPassOutRenderTargetRightView(void) { return m_pPassOutRenderTargetRightView; }
		ID3D11DepthStencilView*	GetPassOutDepthStencilView(void) { return m_pPassOutDepthStencilView; }
		DWORD GetPassOutRenderTargetWidth(void) { return m_dwPassOutRenderTargetWidth; }
		DWORD GetPassOutRenderTargetHeight(void) { return m_dwPassOutRenderTargetHeight; }
		ID3D11ShaderResourceView* GetPassOutLeftShaderResourceView(void) { return m_pPassOutLeftShaderResourceView; }
		winrt::Windows::Foundation::Numerics::float4x4 GetPassOutProjectionLeft(void) { return m_matProjectionLeft; }
		winrt::Windows::Foundation::Numerics::float4x4 GetPassOutProjectionRight(void) { return m_matProjectionRight; }
		float GetPassOutHeadPosX(void) { return m_fHeadPosX; }
		float GetPassOutHeadPosY(void) { return m_fHeadPosY; }
		float GetPassOutHeadPosZ(void) { return m_fHeadPosZ; }
		float GetPassOutHeadUpX(void) { return m_fHeadUpX; }
		float GetPassOutHeadUpY(void) { return m_fHeadUpY; }
		float GetPassOutHeadUpZ(void) { return m_fHeadUpZ; }		
		float GetPassOutHeadDirX(void) { return m_fHeadDirX; }
		float GetPassOutHeadDirY(void) { return m_fHeadDirY; }
		float GetPassOutHeadDirZ(void) { return m_fHeadDirZ; }		

		// utility member functions
		winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference GetFrameOfReference ( void ) { return m_stationaryReferenceFrame; }

    private:
        // Asynchronously creates resources for new holographic cameras.
        void OnCameraAdded(
            winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
            winrt::Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs const& args);

        // Synchronously releases resources for holographic cameras that are no longer
        // attached to the system.
        void OnCameraRemoved(
            winrt::Windows::Graphics::Holographic::HolographicSpace const& sender,
            winrt::Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs const& args);

        // Used to notify the app when the positional tracking state changes.
        void OnLocatabilityChanged(
            winrt::Windows::Perception::Spatial::SpatialLocator const& sender,
            winrt::Windows::Foundation::IInspectable const& args);

        // Used to be aware of gamepads that are plugged in after the app starts.
        void OnGamepadAdded(winrt::Windows::Foundation::IInspectable, winrt::Windows::Gaming::Input::Gamepad const& args);

        // Used to stop looking for gamepads that are removed while the app is running.
        void OnGamepadRemoved(winrt::Windows::Foundation::IInspectable, winrt::Windows::Gaming::Input::Gamepad const& args);

        // Used to respond to changes to the default spatial locator.
        void OnHolographicDisplayIsAvailableChanged(winrt::Windows::Foundation::IInspectable, winrt::Windows::Foundation::IInspectable);

        // Clears event registration state. Used when changing to a new HolographicSpace
        // and when tearing down AppMain.
        void UnregisterHolographicEventHandlers();

        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources>                        m_deviceResources;

        // Render loop timer.
        DX::StepTimer                                               m_timer;

        // Represents the holographic space around the user.
        winrt::Windows::Graphics::Holographic::HolographicSpace     m_holographicSpace = nullptr;
		winrt::Windows::UI::Input::Spatial::SpatialInteractionManager* m_interactionManager = NULL;

        // SpatialLocator that is attached to the default HolographicDisplay.
        winrt::Windows::Perception::Spatial::SpatialLocator         m_spatialLocator = nullptr;

        // A stationary reference frame based on m_spatialLocator.
        winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference m_stationaryReferenceFrame = nullptr;

        // Event registration tokens.
        winrt::event_token                                          m_cameraAddedToken;
        winrt::event_token                                          m_cameraRemovedToken;
        winrt::event_token                                          m_locatabilityChangedToken;
        winrt::event_token                                          m_gamepadAddedEventToken;
        winrt::event_token                                          m_gamepadRemovedEventToken;
        winrt::event_token                                          m_holographicDisplayIsAvailableChangedEventToken;

        // Keep track of gamepads.
        struct GamepadWithButtonState
        {
            winrt::Windows::Gaming::Input::Gamepad gamepad;
            bool buttonAWasPressedLastFrame = false;
        };
        std::vector<GamepadWithButtonState>                         m_gamepads;

        // Keep track of mouse input.
        bool                                                        m_pointerPressed = false;

        // Cache whether or not the HolographicCamera.Display property can be accessed.
        bool                                                        m_canGetHolographicDisplayForCamera = false;

        // Cache whether or not the HolographicDisplay.GetDefault() method can be called.
        bool                                                        m_canGetDefaultHolographicDisplay = false;

        // Cache whether or not the HolographicCameraRenderingParameters.CommitDirect3D11DepthBuffer() method can be called.
        bool                                                        m_canCommitDirect3D11DepthBuffer = false;

		// Extra members
		ID3D11RenderTargetView*										m_pPassOutRenderTargetLeftView = NULL;
		ID3D11RenderTargetView*										m_pPassOutRenderTargetRightView = NULL;
		ID3D11DepthStencilView*										m_pPassOutDepthStencilView = NULL;
		DWORD														m_dwPassOutRenderTargetWidth = 0;
		DWORD														m_dwPassOutRenderTargetHeight = 0;
		ID3D11ShaderResourceView*									m_pPassOutLeftShaderResourceView = NULL;
		
		winrt::Windows::Foundation::Numerics::float4x4				m_matProjectionLeft;
		winrt::Windows::Foundation::Numerics::float4x4				m_matProjectionRight;
		float														m_fHeadPosX = 0;
		float														m_fHeadPosY = 0;
		float														m_fHeadPosZ = 0;
		float														m_fHeadUpX = 0;
		float														m_fHeadUpY = 0;
		float														m_fHeadUpZ = 0;
		float														m_fHeadDirX = 0;
		float														m_fHeadDirY = 0;
		float														m_fHeadDirZ = 0;
    };
}
