// Includes
#include "stdafx.h"
#include "BasicHologramMain.h"
#include "Common/DirectXHelper.h"
#include <windows.graphics.directx.direct3d11.interop.h>

// Namespaces
using namespace BasicHologram;
using namespace concurrency;
using namespace Microsoft::WRL;
using namespace std::placeholders;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::UI::Input::Spatial;

int DebugVRlog ( const char* pReportLog );

// Loads and initializes application assets when the application is loaded.
BasicHologramMain::BasicHologramMain(std::shared_ptr<DX::DeviceResources> const& deviceResources) :
    m_deviceResources(deviceResources)
{
    // Register to be notified if the device is lost or recreated.
	DebugVRlog("RegisterDeviceNotify");
    m_deviceResources->RegisterDeviceNotify(this);

    // If connected, a game controller can also be used for input.
	DebugVRlog("m_gamepadAddedEventToken");
    m_gamepadAddedEventToken = Gamepad::GamepadAdded(bind(&BasicHologramMain::OnGamepadAdded, this, _1, _2));
    m_gamepadRemovedEventToken = Gamepad::GamepadRemoved(bind(&BasicHologramMain::OnGamepadRemoved, this, _1, _2));

	DebugVRlog("OnGamepadAdded");
    for (Gamepad const& gamepad : Gamepad::Gamepads())
    {
        OnGamepadAdded(nullptr, gamepad);
    }

	DebugVRlog("m_canGetHolographicDisplayForCamera");
    m_canGetHolographicDisplayForCamera = winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(L"Windows.Graphics.Holographic.HolographicCamera", L"Display");
    m_canGetDefaultHolographicDisplay = winrt::Windows::Foundation::Metadata::ApiInformation::IsMethodPresent(L"Windows.Graphics.Holographic.HolographicDisplay", L"GetDefault");
    m_canCommitDirect3D11DepthBuffer = winrt::Windows::Foundation::Metadata::ApiInformation::IsMethodPresent(L"Windows.Graphics.Holographic.HolographicCameraRenderingParameters", L"CommitDirect3D11DepthBuffer");

	DebugVRlog("IsAvailableChanged");
    if (m_canGetDefaultHolographicDisplay)
    {
        // Subscribe for notifications about changes to the state of the default HolographicDisplay 
        // and its SpatialLocator.
        m_holographicDisplayIsAvailableChangedEventToken = HolographicSpace::IsAvailableChanged(bind(&BasicHologramMain::OnHolographicDisplayIsAvailableChanged, this, _1, _2));
    }

    // Acquire the current state of the default HolographicDisplay and its SpatialLocator.
	DebugVRlog("OnHolographicDisplayIsAvailableChanged");
    OnHolographicDisplayIsAvailableChanged(nullptr, nullptr);
}

void BasicHologramMain::SetHolographicSpace(HolographicSpace const& holographicSpace, winrt::Windows::UI::Input::Spatial::SpatialInteractionManager* interactionManager)
{
	DebugVRlog("m_gamepadAddedEventToken");
    UnregisterHolographicEventHandlers();

	DebugVRlog("m_holographicSpace");
    m_holographicSpace = holographicSpace;
    m_interactionManager = interactionManager;

    // Respond to camera added events by creating any resources that are specific
    // to that camera, such as the back buffer render target view.
    // When we add an event handler for CameraAdded, the API layer will avoid putting
    // the new camera in new HolographicFrames until we complete the deferral we created
    // for that handler, or return from the handler without creating a deferral. This
    // allows the app to take more than one frame to finish creating resources and
    // loading assets for the new holographic camera.
    // This function should be registered before the app creates any HolographicFrames.
	DebugVRlog("m_cameraAddedToken");
    m_cameraAddedToken = m_holographicSpace.CameraAdded(std::bind(&BasicHologramMain::OnCameraAdded, this, _1, _2));

    // Respond to camera removed events by releasing resources that were created for that
    // camera.
    // When the app receives a CameraRemoved event, it releases all references to the back
    // buffer right away. This includes render target views, Direct2D target bitmaps, and so on.
    // The app must also ensure that the back buffer is not attached as a render target, as
    // shown in DeviceResources::ReleaseResourcesForBackBuffer.
	DebugVRlog("m_cameraRemovedToken");
    m_cameraRemovedToken = m_holographicSpace.CameraRemoved(std::bind(&BasicHologramMain::OnCameraRemoved, this, _1, _2));

    // Notes on spatial tracking APIs:
    // * Stationary reference frames are designed to provide a best-fit position relative to the
    //   overall space. Individual positions within that reference frame are allowed to drift slightly
    //   as the device learns more about the environment.
    // * When precise placement of individual holograms is required, a SpatialAnchor should be used to
    //   anchor the individual hologram to a position in the real world - for example, a point the user
    //   indicates to be of special interest. Anchor positions do not drift, but can be corrected; the
    //   anchor will use the corrected position starting in the next frame after the correction has
    //   occurred.
}

void BasicHologramMain::UnregisterHolographicEventHandlers()
{
    if (m_holographicSpace != nullptr)
    {
        // Clear previous event registrations.
        m_holographicSpace.CameraAdded(m_cameraAddedToken);
        m_cameraAddedToken = {};
        m_holographicSpace.CameraRemoved(m_cameraRemovedToken);
        m_cameraRemovedToken = {};
    }

    if (m_spatialLocator != nullptr)
    {
        m_spatialLocator.LocatabilityChanged(m_locatabilityChangedToken);
    }
}

BasicHologramMain::~BasicHologramMain()
{
    m_deviceResources->RegisterDeviceNotify(nullptr);

    UnregisterHolographicEventHandlers();

    Gamepad::GamepadAdded(m_gamepadAddedEventToken);
    Gamepad::GamepadRemoved(m_gamepadRemovedEventToken);
    HolographicSpace::IsAvailableChanged(m_holographicDisplayIsAvailableChangedEventToken);

	m_holographicSpace = nullptr;
}

// Updates the application state once per frame.
HolographicFrame BasicHologramMain::Update ( winrt::Windows::Perception::Spatial::SpatialStationaryFrameOfReference* pstationaryReferenceFrame )
{
    // Before doing the timer update, there is some work to do per-frame
    // to maintain holographic rendering. First, we will get information
    // about the current frame.
	///MessageBox ( NULL, "Hard Debug Trace", "11", MB_OK );
	if ( m_holographicSpace == nullptr )
	{
		///MessageBox ( NULL, "Hard Debug Trace", "12", MB_OK );
		DebugVRlog("m_holographicSpace not valid");
		return nullptr;
	}

    // The HolographicFrame has information that the app needs in order
    // to update and render the current frame. The app begins each new
    // frame by calling CreateNextFrame.
	///MessageBox ( NULL, "Hard Debug Trace", "13", MB_OK );
	DebugVRlog("CreateNextFrame");
	///MessageBox ( NULL, "Hard Debug Trace", "14", MB_OK );
    HolographicFrame holographicFrame = nullptr;
	try
	{
		holographicFrame = m_holographicSpace.CreateNextFrame();
	}
	catch(...)
	{
		DebugVRlog("failed m_holographicSpace.CreateNextFrame()");
	}

    // Wait for the frame to be ready before pose-dependent updates and rendering.
    // NOTE: This API call will throw an exception if a wait occurs at the wrong time, or out of sequence.
	///MessageBox ( NULL, "Hard Debug Trace", "15", MB_OK );
	DebugVRlog("WaitForNextFrameReady");
	///MessageBox ( NULL, "Hard Debug Trace", "16", MB_OK );
	try
	{
		// it will ALSO throw an exception if you are not running Windows version 1809 (as that is when this function came online!)
	    m_holographicSpace.WaitForNextFrameReady();
	}
	catch(...)
	{
		DebugVRlog("failed m_holographicSpace.WaitForNextFrameReady()");
	}

    // Get a prediction of where holographic cameras will be when this frame is presented.
	///MessageBox ( NULL, "Hard Debug Trace", "17", MB_OK );
	if ( holographicFrame == nullptr )
	{
		///MessageBox ( NULL, "Hard Debug Trace", "18", MB_OK );
		DebugVRlog("holographicFrame not valid");
		return nullptr;
	}

	///MessageBox ( NULL, "Hard Debug Trace", "19", MB_OK );
	DebugVRlog("CurrentPrediction");
	///MessageBox ( NULL, "Hard Debug Trace", "20", MB_OK );
	HolographicFramePrediction prediction = nullptr;
	try
	{
	    prediction = holographicFrame.CurrentPrediction();
	}
	catch(...)
	{
		DebugVRlog("failed holographicFrame.CurrentPrediction()");
	}
	if ( prediction == nullptr )
	{
		///MessageBox ( NULL, "Hard Debug Trace", "21", MB_OK );
		DebugVRlog("CurrentPrediction not valid");
		return nullptr;
	}

	// Back buffers can change from frame to frame. Validate each buffer, and recreate
	// resource views and depth buffers as needed.
	///MessageBox ( NULL, "Hard Debug Trace", "22", MB_OK );
	DebugVRlog("m_deviceResources");
	if ( m_deviceResources )
	{
		///MessageBox ( NULL, "Hard Debug Trace", "23", MB_OK );
		DebugVRlog("EnsureCameraResources");
		try
		{
			m_deviceResources->EnsureCameraResources(holographicFrame, prediction);
		}
		catch(...)
		{
			DebugVRlog("failed EnsureCameraResources(holographicFrame, prediction)");
		}
		if (m_stationaryReferenceFrame != nullptr)
		{
			///MessageBox ( NULL, "Hard Debug Trace", "24", MB_OK );
			// Check for new input state since the last frame.
			/*
			DebugVRlog("gamepadWithButtonState");
			for (GamepadWithButtonState& gamepadWithButtonState : m_gamepads)
			{
				bool buttonDownThisUpdate = ((gamepadWithButtonState.gamepad.GetCurrentReading().Buttons & GamepadButtons::A) == GamepadButtons::A);
				if (buttonDownThisUpdate && !gamepadWithButtonState.buttonAWasPressedLastFrame)
				{
					//m_pointerPressed = true;
				}
				gamepadWithButtonState.buttonAWasPressedLastFrame = buttonDownThisUpdate;
			}
			*/

			// get post from predicted frame
			///MessageBox ( NULL, "Hard Debug Trace", "25", MB_OK );
			SpatialPointerPose pose = nullptr;
			DebugVRlog("TryGetAtTimestamp");
			try
			{
				pose = SpatialPointerPose::TryGetAtTimestamp(m_stationaryReferenceFrame.CoordinateSystem(), prediction.Timestamp());
			}
			catch(...)
			{
				DebugVRlog("failed SpatialPointerPose::TryGetAtTimestamp");
			}

			// extract head position and direction
			if ( pose != nullptr )
			{
				///MessageBox ( NULL, "Hard Debug Trace", "26", MB_OK );
				DebugVRlog("pose.Head().Position()");
				m_fHeadPosX = pose.Head().Position().x;
				m_fHeadPosY = pose.Head().Position().y;
				m_fHeadPosZ = pose.Head().Position().z;
				m_fHeadUpX = pose.Head().UpDirection().x;
				m_fHeadUpY = pose.Head().UpDirection().y;
				m_fHeadUpZ = pose.Head().UpDirection().z;
				m_fHeadDirX = pose.Head().ForwardDirection().x;
				m_fHeadDirY = pose.Head().ForwardDirection().y;
				m_fHeadDirZ = pose.Head().ForwardDirection().z;
			}
		}
	}

    m_timer.Tick([this]()
    {
    });

	///MessageBox ( NULL, "Hard Debug Trace", "27", MB_OK );
	//DebugVRlog("m_canCommitDirect3D11DepthBuffer");
    //if (!m_canCommitDirect3D11DepthBuffer)
    //{
		///MessageBox ( NULL, "Hard Debug Trace", "28", MB_OK );
        // On versions of the platform that do not support the CommitDirect3D11DepthBuffer API, we can control
        // image stabilization by setting a focus point with optional plane normal and velocity.
		//DebugVRlog("cameraPose");
        //for (HolographicCameraPose const& cameraPose : prediction.CameraPoses())
        //{
			///MessageBox ( NULL, "Hard Debug Trace", "29", MB_OK );
            // The HolographicCameraRenderingParameters class provides access to set
            // the image stabilization parameters.
			//DebugVRlog("GetRenderingParameters");
            //HolographicCameraRenderingParameters renderingParameters = holographicFrame.GetRenderingParameters(cameraPose);

            // SetFocusPoint informs the system about a specific point in your scene to
            // prioritize for image stabilization. The focus point is set independently
                // for each holographic camera. When setting the focus point, put it on or 
                // near content that the user is looking at.
                // In this example, we put the focus point at the center of the sample hologram.
                // You can also set the relative velocity and facing of the stabilization
                // plane using overloads of this method.
            //if (m_stationaryReferenceFrame != nullptr)
            //{
            //}
        //}
    //}

    // The holographic frame will be used to get up-to-date view and projection matrices and to present the swap chain.
	///MessageBox ( NULL, "Hard Debug Trace", "", MB_OK );
	DebugVRlog("m_stationaryReferenceFrame");
	if ( pstationaryReferenceFrame ) *pstationaryReferenceFrame = m_stationaryReferenceFrame;
    return holographicFrame;
}

// Renders the current frame to each holographic camera, according to the
// current application and spatial positioning state. Returns true if the
// frame was rendered to at least one camera.
bool BasicHologramMain::Render(HolographicFrame const& holographicFrame)
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return false;
    }

     // Lock the set of holographic camera resources, then draw to each camera
    // in this frame.
    return m_deviceResources->UseHolographicCameraResources<bool>(
        [this, holographicFrame](std::map<UINT32, std::unique_ptr<DX::CameraResources>>& cameraResourceMap)
    {
        // Up-to-date frame predictions enhance the effectiveness of image stablization and
        // allow more accurate positioning of holograms.
        holographicFrame.UpdateCurrentPrediction();
        HolographicFramePrediction prediction = holographicFrame.CurrentPrediction();

        bool atLeastOneCameraRendered = false;
        for (HolographicCameraPose const& cameraPose : prediction.CameraPoses())
        {
            // This represents the device-based resources for a HolographicCamera.
            DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose.HolographicCamera().Id()].get();

            // Set render targets to the current holographic camera.
			if ( pCameraResources->GetBackBufferRenderTargetLeftView() != NULL )//targets[0] != NULL )
			{
				m_pPassOutRenderTargetLeftView = pCameraResources->GetBackBufferRenderTargetLeftView();
				m_pPassOutRenderTargetRightView = pCameraResources->GetBackBufferRenderTargetRightView();
				m_pPassOutDepthStencilView = pCameraResources->GetDepthStencilView();
				m_dwPassOutRenderTargetWidth = (DWORD)pCameraResources->GetRenderTargetSize().Width;
				m_dwPassOutRenderTargetHeight = (DWORD)pCameraResources->GetRenderTargetSize().Height;
				m_pPassOutLeftShaderResourceView = pCameraResources->GetLeftShaderResourceView();

				// The projection transform for each frame is provided by the cameraPose
			    HolographicStereoTransform cameraProjectionTransform = cameraPose.ProjectionTransform();
				m_matProjectionLeft = cameraProjectionTransform.Left;
				m_matProjectionRight = cameraProjectionTransform.Right;
			}
			atLeastOneCameraRendered = true;
        }

        return atLeastOneCameraRendered;
    });
}

void BasicHologramMain::SaveAppState()
{
}

void BasicHologramMain::LoadAppState()
{
}

void BasicHologramMain::OnPointerPressed()
{
    m_pointerPressed = true;
}

// Notifies classes that use Direct3D device resources that the device resources
// need to be released before this method returns.
void BasicHologramMain::OnDeviceLost()
{
}

// Notifies classes that use Direct3D device resources that the device resources
// may now be recreated.
void BasicHologramMain::OnDeviceRestored()
{
}

void BasicHologramMain::OnLocatabilityChanged(SpatialLocator const& sender, winrt::Windows::Foundation::IInspectable const& /*args*/)
{
    switch (sender.Locatability())
    {
    case SpatialLocatability::Unavailable:
        // Holograms cannot be rendered.
    {
        winrt::hstring message(L"Warning! Positional tracking is " + std::to_wstring(int(sender.Locatability())) + L".\n");
        OutputDebugStringW(message.data());
    }
    break;

    // In the following three cases, it is still possible to place holograms using a
    // SpatialLocatorAttachedFrameOfReference.
    case SpatialLocatability::PositionalTrackingActivating:
        // The system is preparing to use positional tracking.

    case SpatialLocatability::OrientationOnly:
        // Positional tracking has not been activated.

    case SpatialLocatability::PositionalTrackingInhibited:
        // Positional tracking is temporarily inhibited. User action may be required
        // in order to restore positional tracking.
        break;

    case SpatialLocatability::PositionalTrackingActive:
        // Positional tracking is active. World-locked content can be rendered.
        break;
    }
}

void BasicHologramMain::OnCameraAdded(
    HolographicSpace const& /*sender*/,
    HolographicSpaceCameraAddedEventArgs const& args
)
{
    winrt::Windows::Foundation::Deferral deferral = args.GetDeferral();
    HolographicCamera holographicCamera = args.Camera();
    create_task([this, deferral, holographicCamera]()
    {
        // Create device-based resources for the holographic camera and add it to the list of
        // cameras used for updates and rendering. Notes:
        //   * Since this function may be called at any time, the AddHolographicCamera function
        //     waits until it can get a lock on the set of holographic camera resources before
        //     adding the new camera. At 60 frames per second this wait should not take long.
        //   * A subsequent Update will take the back buffer from the RenderingParameters of this
        //     camera's CameraPose and use it to create the ID3D11RenderTargetView for this camera.
        //     Content can then be rendered for the HolographicCamera.
        m_deviceResources->AddHolographicCamera(holographicCamera);

        // Holographic frame predictions will not include any information about this camera until
        // the deferral is completed.
        deferral.Complete();
    });
}

void BasicHologramMain::OnCameraRemoved(
    HolographicSpace const& /*sender*/,
    HolographicSpaceCameraRemovedEventArgs const& args
)
{
    create_task([this]()
    {
    });

    // Before letting this callback return, ensure that all references to the back buffer 
    // are released.
    // Since this function may be called at any time, the RemoveHolographicCamera function
    // waits until it can get a lock on the set of holographic camera resources before
    // deallocating resources for this camera. At 60 frames per second this wait should
    // not take long.
    m_deviceResources->RemoveHolographicCamera(args.Camera());
}

void BasicHologramMain::OnGamepadAdded(winrt::Windows::Foundation::IInspectable, Gamepad const& args)
{
    for (GamepadWithButtonState const& gamepadWithButtonState : m_gamepads)
    {
        if (args == gamepadWithButtonState.gamepad)
        {
            // This gamepad is already in the list.
            return;
        }
    }

    GamepadWithButtonState newGamepad = { args, false };
    m_gamepads.push_back(newGamepad);
}

void BasicHologramMain::OnGamepadRemoved(winrt::Windows::Foundation::IInspectable, Gamepad const& args)
{
    m_gamepads.erase(std::remove_if(m_gamepads.begin(), m_gamepads.end(), [&](GamepadWithButtonState& gamepadWithState)
        {
            return gamepadWithState.gamepad == args;
        }),
        m_gamepads.end());
}

void BasicHologramMain::OnHolographicDisplayIsAvailableChanged(winrt::Windows::Foundation::IInspectable, winrt::Windows::Foundation::IInspectable)
{
    // Get the spatial locator for the default HolographicDisplay, if one is available.
    SpatialLocator spatialLocator = nullptr;
    if (m_canGetDefaultHolographicDisplay)
    {
        HolographicDisplay defaultHolographicDisplay = HolographicDisplay::GetDefault();
        if (defaultHolographicDisplay)
        {
            spatialLocator = defaultHolographicDisplay.SpatialLocator();
        }
    }
    else
    {
        spatialLocator = SpatialLocator::GetDefault();
    }

    if (m_spatialLocator != spatialLocator)
    {
        // If the spatial locator is disconnected or replaced, we should discard all state that was
        // based on it.
        if (m_spatialLocator != nullptr)
        {
            m_spatialLocator.LocatabilityChanged(m_locatabilityChangedToken);
            m_spatialLocator = nullptr;
        }

        m_stationaryReferenceFrame = nullptr;

        if (spatialLocator != nullptr)
        {
            // Use the SpatialLocator from the default HolographicDisplay to track the motion of the device.
            m_spatialLocator = spatialLocator;

            // Respond to changes in the positional tracking state.
            m_locatabilityChangedToken = m_spatialLocator.LocatabilityChanged(std::bind(&BasicHologramMain::OnLocatabilityChanged, this, _1, _2));

            // The simplest way to render world-locked holograms is to create a stationary reference frame
            // based on a SpatialLocator. This is roughly analogous to creating a "world" coordinate system
            // with the origin placed at the device's position as the app is launched.
            m_stationaryReferenceFrame = m_spatialLocator.CreateStationaryFrameOfReferenceAtCurrentLocation();
        }
    }
}
