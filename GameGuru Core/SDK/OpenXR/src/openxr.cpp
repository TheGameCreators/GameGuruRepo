#include "openxr.h"

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <vector>

#include "d3d11.h"

#define XR_USE_GRAPHICS_API_D3D11
#define XR_USE_PLATFORM_WIN32
#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"

ID3D11Device *xrD3DDevice = 0;

#define PI 3.14159265f

struct AppSwapChain
{
	XrSwapchain swapchain;
	uint32_t numImages;
	XrSwapchainImageD3D11KHR* images;
	uint32_t width;
	uint32_t height;
	ID3D11RenderTargetView** imageViews;
};

XrInstance xrInstance = 0;
XrSession xrSession = 0;
XrSpace xrAppSpace = 0;
XrSystemId xrSystemId = 0;
XrViewConfigurationView xrViewConfigs[ 2 ] = { {XR_TYPE_VIEW_CONFIGURATION_VIEW}, {XR_TYPE_VIEW_CONFIGURATION_VIEW} }; // OpenXR currently only supports 2 views
XrView xrViews[ 2 ] = { {XR_TYPE_VIEW}, {XR_TYPE_VIEW} };
int64_t xrSwapchainFormat = 0;
AppSwapChain appSwapChains[ 2 ] = { {0}, {0} };
XrSessionState xrSessionState = XR_SESSION_STATE_UNKNOWN;
bool xrSessionRunning = false;

char* xrRuntimeName = 0;

// OpenXR input
XrActionSet xrActionSet;
XrPath xrPathHands[2];

// actions cover both hands
XrAction xrActionHandPose;
XrAction xrActionHandAimPose;
XrAction xrActionMoveX;
XrAction xrActionMoveY;
XrAction xrActionStickClick;
XrAction xrActionStickTouch;
XrAction xrActionTouchPadX;
XrAction xrActionTouchPadY;
XrAction xrActionTouchPadClick;
XrAction xrActionTouchPadTouch;
XrAction xrActionTrigger;
XrAction xrActionSqueeze;
XrAction xrActionA;
XrAction xrActionB;

XrSpace xrSpaceLeftHand;
XrSpace xrSpaceRightHand;
XrSpace xrSpaceLeftHandAim;
XrSpace xrSpaceRightHandAim;

#define APP_LEFT_HAND 0
#define APP_RIGHT_HAND 1

XrVector3f appControllerPosLeft = { 0, 0, 0 };
XrVector3f appControllerPosRight = { 0, 0, 0 };
XrQuaternionf appControllerRotLeft = { 0, 0, 0, 1 };
XrQuaternionf appControllerRotRight = { 0, 0, 0, 1 };
XrVector3f appControllerAimPosLeft = { 0, 0, 0 };
XrVector3f appControllerAimPosRight = { 0, 0, 0 };
XrQuaternionf appControllerAimRotLeft = { 0, 0, 0, 1 };
XrQuaternionf appControllerAimRotRight = { 0, 0, 0, 1 };

// left hand
bool  appHasLeftAim = false;
bool  appHasLeftStick = false;
bool  appHasLeftTouchPad = false;
bool  appHasLeftTrigger = false;
bool  appHasLeftSqueeze = false;
bool  appHasLeftButtonA = false;
bool  appHasLeftButtonB = false;

int   appLeftHandActive = 0;
int   appLeftControllerType = OPENXR_CONTROLLER_UNKNOWN;
float appLeftStickX = 0;
float appLeftStickY = 0;
bool  appLeftStickClick = false;
bool  appLeftStickTouch = false;
float appLeftTouchPadX = 0;
float appLeftTouchPadY = 0;
bool  appLeftTouchPadClick = false;
bool  appLeftTouchPadTouch = false;
float appLeftTrigger = 0;
float appLeftSqueeze = 0;
bool  appLeftA = false;
bool  appLeftB = false;

// right hand
bool  appHasRightAim = false;
bool  appHasRightStick = false;
bool  appHasRightTouchPad = false;
bool  appHasRightTrigger = false;
bool  appHasRightSqueeze = false;
bool  appHasRightButtonA = false;
bool  appHasRightButtonB = false;

int   appRightHandActive = 0;
int   appRightControllerType = OPENXR_CONTROLLER_UNKNOWN;
float appRightStickX = 0;
float appRightStickY = 0;
bool  appRightStickClick = false;
bool  appRightStickTouch = false;
float appRightTouchPadX = 0;
float appRightTouchPadY = 0;
bool  appRightTouchPadClick = false;
bool  appRightTouchPadTouch = false;
float appRightTrigger = 0;
float appRightSqueeze = 0;
bool  appRightA = false;
bool  appRightB = false;

void AppError( const char* msg )
{
	char msg2[256];
	sprintf_s( msg2, 256, "    > OpenXR Error: %s\n", msg );
	OutputDebugStringA( msg2 );
	//printf( "%s", msg );
	//MessageBoxA( 0, msg, "Error", 0 );
	//exit(0);
}

XrPosef PoseIdentity() { XrPosef t{}; t.orientation.w = 1; return t; }

void CreateProjMatrix( XrFovf fovs, float fNear, float fFar, float outMat[16], int invertedDepth )
{
	float fLeft = tanf(fovs.angleLeft);
	float fRight = tanf(fovs.angleRight);
	float fTop = tanf(fovs.angleUp);
	float fBottom = tanf(fovs.angleDown);
    	 
	float modifier = 1.0f; // left hand
	outMat[0] = 2 / (fRight-fLeft);
	outMat[1] = 0.0f;
	outMat[2] = 0.0f;
	outMat[3] = 0.0f;
	outMat[4] = 0.0f;
	outMat[5] = 2 / (fTop-fBottom);
	outMat[6] = 0.0f;
	outMat[7] = 0.0f;
	outMat[8] = (fRight + fLeft)/(fRight - fLeft) * -modifier; // negative for left hand
	outMat[9] = (fTop + fBottom)/(fTop-fBottom) * -modifier; // negative for left hand
	outMat[10] = (fFar/(fFar - fNear)) * modifier;
	outMat[11] = modifier;
	outMat[12] = 0.0f;
	outMat[13] = 0.0f;
	outMat[14] = -(fFar * fNear)/(fFar - fNear);
	outMat[15] = 0.0f;

	if ( invertedDepth )
	{
		outMat[10] = (fNear/(fNear - fFar)) * modifier;
		outMat[14] = -(fFar * fNear)/(fNear - fFar);
	}
}

void CreateInvViewMatrix( XrVector3f pos, XrQuaternionf rot, float outMat[16] )
{
	float xx = rot.x*rot.x;
	float yy = rot.y*rot.y;
	float zz = rot.z*rot.z;
	float xy = rot.x*rot.y;
	float wz = rot.w*rot.z;
	float xz = rot.x*rot.z;
	float wy = rot.w*rot.y;
	float yz = rot.y*rot.z;
	float wx = rot.w*rot.x;

	outMat[0] = (1 - 2*(yy + zz));
	outMat[1] = (2*(xy - wz));
	outMat[2] = (2*(xz + wy));
	outMat[3] = 0;

	outMat[4] = (2*(xy + wz));
	outMat[5] = (1 - 2*(xx + zz));
	outMat[6] = (2*(yz - wx));
	outMat[7] = 0;
	
	outMat[8] = (2*(xz - wy));
	outMat[9] = (2*(yz + wx));
	outMat[10] = (1 - 2*(xx + yy));
	outMat[11] = 0;

	float x = -pos.x;
	float y = -pos.y;
	float z = -pos.z;

	outMat[12] = x*outMat[0] + y*outMat[4] + z*outMat[8];
	outMat[13] = x*outMat[1] + y*outMat[5] + z*outMat[9];
	outMat[14] = x*outMat[2] + y*outMat[6] + z*outMat[10];
	outMat[15] = 1;
}

void CreateViewMatrix( XrVector3f pos, XrQuaternionf rot, float outMat[16] )
{
	float xx = rot.x*rot.x;
	float yy = rot.y*rot.y;
	float zz = rot.z*rot.z;
	float xy = rot.x*rot.y;
	float wz = rot.w*rot.z;
	float xz = rot.x*rot.z;
	float wy = rot.w*rot.y;
	float yz = rot.y*rot.z;
	float wx = rot.w*rot.x;

	outMat[0]  = 1 - 2*(yy + zz);
	outMat[1]  = 2*(xy + wz);
	outMat[2]  = 2*(xz - wy);
	outMat[3]  = 0;

	outMat[4]  = 2*(xy - wz);
	outMat[5]  = 1 - 2*(xx + zz);
	outMat[6]  = 2*(yz + wx);
	outMat[7]  = 0;

	outMat[8]  = 2*(xz + wy);
	outMat[9]  = 2*(yz - wx);
	outMat[10] = 1 - 2*(xx + yy);
	outMat[11] = 0;
	
	outMat[12] = pos.x;
	outMat[13] = pos.y;
	outMat[14] = pos.z;
	outMat[15] = 1;
}

bool OpenXRIsInitialised() { return (xrInstance != 0); }
bool OpenXRIsSessionSetup() { return (xrSession != 0); }
bool OpenXRIsSessionActive() { return xrSessionRunning; }

void OpenXRCreateAction( XrActionType type, const char* name, XrAction *action )
{
	XrActionCreateInfo actionInfo = { XR_TYPE_ACTION_CREATE_INFO };
    actionInfo.actionType = type;
    strcpy_s(actionInfo.actionName, name);
    strcpy_s(actionInfo.localizedActionName, name);
    actionInfo.countSubactionPaths = 2;
    actionInfo.subactionPaths = xrPathHands;
    XrResult xrRes = xrCreateAction( xrActionSet, &actionInfo, action );
	if ( xrRes < 0 ) AppError( "xrCreateAction failed" );
}

int OpenXRInit( ID3D11Device* device )
{
	xrD3DDevice = device;
	XrResult xrRes;

	const char* extensions[] = { XR_KHR_D3D11_ENABLE_EXTENSION_NAME };

	// create instance
	XrInstanceCreateInfo instanceCreateInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
    instanceCreateInfo.next = 0;
    instanceCreateInfo.enabledExtensionCount = sizeof(extensions) / sizeof(const char*);
	instanceCreateInfo.enabledExtensionNames = extensions;
	strcpy_s( instanceCreateInfo.applicationInfo.applicationName, 128, "MyXR App" );
	instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    xrRes = xrCreateInstance(&instanceCreateInfo, &xrInstance); 
	if ( xrRes < 0 ) { return -1; }//AppError( "xrCreateInstance failed" ); }

	// get runtime name
	if ( xrRuntimeName ) delete [] xrRuntimeName;
	xrRuntimeName = 0;
	XrInstanceProperties instanceProperties = { XR_TYPE_INSTANCE_PROPERTIES };
    xrRes = xrGetInstanceProperties( xrInstance, &instanceProperties );
	if ( xrRes >= 0 )
	{
		int length = strlen(instanceProperties.runtimeName) + 1;
		xrRuntimeName = new char[ length ];
		strcpy_s( xrRuntimeName, length, instanceProperties.runtimeName );
	}

	// get system ID
	XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	xrRes = xrGetSystem(xrInstance, &systemInfo, &xrSystemId);
	if ( xrRes < 0 ) { return -1; }//AppError( "xrGetSystem failed" ); }
	if ( xrSystemId == 0 ) { return -1; }//AppError( "SystemId is zero" ); }
	
	// get graphics requirements
	PFN_xrGetD3D11GraphicsRequirementsKHR pfnGetD3D11GraphicsRequirementsKHR = nullptr;
    xrRes = xrGetInstanceProcAddr( xrInstance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*) &pfnGetD3D11GraphicsRequirementsKHR );
	if ( xrRes < 0 ) { AppError( "xrGetInstanceProcAddr failed" ); }
	if ( pfnGetD3D11GraphicsRequirementsKHR == nullptr ) { AppError( "pfnGetD3D11GraphicsRequirementsKHR is null" ); }

    XrGraphicsRequirementsD3D11KHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
    xrRes = pfnGetD3D11GraphicsRequirementsKHR( xrInstance, xrSystemId, &graphicsRequirements );
	if ( xrRes < 0 ) { AppError( "pfnGetD3D11GraphicsRequirementsKHR failed" ); }
	/*
	// find DX11 adapter that matches requirements
	IDXGIFactory1 *dxgiFactory;
    HRESULT hr = CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**) &dxgiFactory );
	if ( FAILED(hr) ) { DXError( hr ); }

    for (UINT adapterIndex = 0;; adapterIndex++) 
	{
        // EnumAdapters1 will fail with DXGI_ERROR_NOT_FOUND when there are no more adapters to enumerate.
        IDXGIAdapter1* dxgiAdapter;
        dxgiFactory->EnumAdapters1( adapterIndex, &dxgiAdapter );

        DXGI_ADAPTER_DESC1 adapterDesc;
        dxgiAdapter->GetDesc1( &adapterDesc );
        if (memcmp( &adapterDesc.AdapterLuid, &graphicsRequirements.adapterLuid, sizeof(graphicsRequirements.adapterLuid) ) == 0) {
            dx11Adapter = dxgiAdapter;
			break;
        }
    }*/

	return 0; // success
}

const char* OpenXRGetRuntimeName()
{
	return xrRuntimeName ? xrRuntimeName : "";
}

void OpenXRStartSession()
{
	// if xrSystemId is 0 then no headset is present, check for one before trying to start a session
	if ( xrSystemId == 0 )
	{
		// xrGetSystem is very slow, so only do it every so often
		static int check_count = 0;
		check_count++;
		if ( check_count >= 100 )
		{
			check_count = 0;

			// get system ID
			XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
			systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
			XrResult xrRes = xrGetSystem(xrInstance, &systemInfo, &xrSystemId);
			if ( xrRes < 0 ) return; //AppError( "xrGetSystem failed" ); }
			if ( xrSystemId == 0 ) return; //AppError( "SystemId is zero" ); }
		
			// get graphics requirements
			PFN_xrGetD3D11GraphicsRequirementsKHR pfnGetD3D11GraphicsRequirementsKHR = nullptr;
			xrRes = xrGetInstanceProcAddr( xrInstance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*) &pfnGetD3D11GraphicsRequirementsKHR );
			if ( xrRes < 0 ) { AppError( "xrGetInstanceProcAddr failed" ); }
			if ( pfnGetD3D11GraphicsRequirementsKHR == nullptr ) { AppError( "pfnGetD3D11GraphicsRequirementsKHR is null" ); }

			XrGraphicsRequirementsD3D11KHR graphicsRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
			xrRes = pfnGetD3D11GraphicsRequirementsKHR( xrInstance, xrSystemId, &graphicsRequirements );
			if ( xrRes < 0 ) { AppError( "pfnGetD3D11GraphicsRequirementsKHR failed" ); }
		}
		else return;
	}

	XrGraphicsBindingD3D11KHR xrGraphicsBinding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
	xrGraphicsBinding.device = xrD3DDevice;

	// create session
	XrSessionCreateInfo sessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
    sessionCreateInfo.next = (void*) &xrGraphicsBinding;
    sessionCreateInfo.systemId = xrSystemId;
    XrResult xrRes = xrCreateSession(xrInstance, &sessionCreateInfo, &xrSession);
	if ( xrRes < 0 ) { /*AppError( "xrCreateSession failed" );*/ return; }

	// create app space
	XrReferenceSpaceCreateInfo referenceSpaceCreateInfo = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	referenceSpaceCreateInfo.poseInReferenceSpace = PoseIdentity();
	xrRes = xrCreateReferenceSpace( xrSession, &referenceSpaceCreateInfo, &xrAppSpace);
	if ( xrRes < 0 ) { AppError( "xrCreateReferenceSpace failed" ); }

	// Query and cache view configuration views.
    uint32_t viewCount, viewCountOut;
    xrRes = xrEnumerateViewConfigurationViews(xrInstance, xrSystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
	if ( xrRes < 0 ) { AppError( "xrEnumerateViewConfigurationViews failed" ); }
    if ( viewCount != 2 ) { AppError( "xrEnumerateViewConfigurationViews must return 2 views" ); }
    xrRes = xrEnumerateViewConfigurationViews(xrInstance, xrSystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCountOut, xrViewConfigs);
	if ( xrRes < 0 ) { AppError( "Second xrEnumerateViewConfigurationViews failed" ); }
	
	// get swapchain format
	uint32_t swapchainFormatCount;
    xrRes = xrEnumerateSwapchainFormats(xrSession, 0, &swapchainFormatCount, nullptr);
	if ( xrRes < 0 ) { AppError( "xrEnumerateSwapchainFormats failed" ); }
    std::vector<int64_t> swapchainFormats(swapchainFormatCount);
    xrRes = xrEnumerateSwapchainFormats(xrSession, (uint32_t)swapchainFormats.size(), &swapchainFormatCount, swapchainFormats.data());
	if ( xrRes < 0 ) { AppError( "Second xrEnumerateSwapchainFormats failed" ); }
	xrSwapchainFormat = 0;
	for( uint32_t i = 0; i < swapchainFormatCount; i++ )
	{
		if ( swapchainFormats[i] == DXGI_FORMAT_R8G8B8A8_UNORM )
		{
			xrSwapchainFormat = swapchainFormats[i];
			break;
		}
		if ( swapchainFormats[i] == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB )
		{
			xrSwapchainFormat = swapchainFormats[i];
			break;
		}
	}
    if ( xrSwapchainFormat == 0 ) { AppError( "Failed to find suitable swapchain format" ); }

	// create a swapchain for each view
    for (uint32_t i = 0; i < viewCount; i++) {
        const XrViewConfigurationView& vp = xrViewConfigs[i];
        
        // Create the swapchain.
        XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.format = xrSwapchainFormat;
        swapchainCreateInfo.width = vp.recommendedImageRectWidth;
        swapchainCreateInfo.height = vp.recommendedImageRectHeight;
        swapchainCreateInfo.mipCount = 1;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.sampleCount = vp.recommendedSwapchainSampleCount;
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        xrRes = xrCreateSwapchain(xrSession, &swapchainCreateInfo, &appSwapChains[i].swapchain);
		if ( xrRes < 0 ) { AppError( "xrCreateSwapchain failed" ); }

		appSwapChains[i].width = swapchainCreateInfo.width;
		appSwapChains[i].height = swapchainCreateInfo.height;
		
		// get swap chain images
        appSwapChains[i].numImages = 0;
        xrRes = xrEnumerateSwapchainImages( appSwapChains[i].swapchain, 0, &appSwapChains[i].numImages, nullptr );
		if ( xrRes < 0 ) { AppError( "xrEnumerateSwapchainImages failed" ); }
		appSwapChains[i].images = new XrSwapchainImageD3D11KHR[ appSwapChains[i].numImages ];
		appSwapChains[i].imageViews = new ID3D11RenderTargetView*[ appSwapChains[i].numImages ];
		for( uint32_t j = 0; j < appSwapChains[i].numImages; j++ ) 
		{
			appSwapChains[i].images[j] = { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR };
			appSwapChains[i].imageViews[j] = 0;
		}
		XrSwapchainImageBaseHeader* baseImages = (XrSwapchainImageBaseHeader*) appSwapChains[i].images;
        xrRes = xrEnumerateSwapchainImages( appSwapChains[i].swapchain, appSwapChains[i].numImages, &appSwapChains[i].numImages, baseImages);
		if ( xrRes < 0 ) { AppError( "Second xrEnumerateSwapchainImages failed" ); }
    }

	// create an action set
    XrActionSetCreateInfo actionSetInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
    strcpy_s(actionSetInfo.actionSetName, "gameplay");
    strcpy_s(actionSetInfo.localizedActionSetName, "Gameplay");
    actionSetInfo.priority = 0;
    xrRes = xrCreateActionSet( xrInstance, &actionSetInfo, &xrActionSet );
	if ( xrRes < 0 ) AppError( "xrCreateActionSet failed" );
    
    // create subactions for left and right hands
	xrRes = xrStringToPath( xrInstance, "/user/hand/left", &xrPathHands[APP_LEFT_HAND] );
	if ( xrRes < 0 ) AppError( "xrStringToPath /user/hand/left failed" );
    xrRes = xrStringToPath( xrInstance, "/user/hand/right", &xrPathHands[APP_RIGHT_HAND] );
	if ( xrRes < 0 ) AppError( "xrStringToPath /user/hand/right failed" );

    // Create an input actions
	OpenXRCreateAction( XR_ACTION_TYPE_POSE_INPUT,    "hand_pose",      &xrActionHandPose );
	OpenXRCreateAction( XR_ACTION_TYPE_POSE_INPUT,    "hand_aim_pose",  &xrActionHandAimPose );
	OpenXRCreateAction( XR_ACTION_TYPE_FLOAT_INPUT,   "move_x",         &xrActionMoveX );
	OpenXRCreateAction( XR_ACTION_TYPE_FLOAT_INPUT,   "move_y",         &xrActionMoveY );
	OpenXRCreateAction( XR_ACTION_TYPE_BOOLEAN_INPUT, "stick_click",    &xrActionStickClick );
	OpenXRCreateAction( XR_ACTION_TYPE_BOOLEAN_INPUT, "stick_touch",    &xrActionStickTouch );
	OpenXRCreateAction( XR_ACTION_TYPE_FLOAT_INPUT,   "touchpad_x",     &xrActionTouchPadX );
	OpenXRCreateAction( XR_ACTION_TYPE_FLOAT_INPUT,   "touchpad_y",     &xrActionTouchPadY );
	OpenXRCreateAction( XR_ACTION_TYPE_BOOLEAN_INPUT, "touchpad_click", &xrActionTouchPadClick );
	OpenXRCreateAction( XR_ACTION_TYPE_BOOLEAN_INPUT, "touchpad_touch", &xrActionTouchPadTouch );
	OpenXRCreateAction( XR_ACTION_TYPE_FLOAT_INPUT,   "trigger",        &xrActionTrigger );
	OpenXRCreateAction( XR_ACTION_TYPE_FLOAT_INPUT,   "squeeze",        &xrActionSqueeze );
	OpenXRCreateAction( XR_ACTION_TYPE_BOOLEAN_INPUT, "button_a",       &xrActionA );
	OpenXRCreateAction( XR_ACTION_TYPE_BOOLEAN_INPUT, "button_b",       &xrActionB );

	{
		// Oculus Touch bindings
		XrPath pathInteractionProfile; xrStringToPath( xrInstance, "/interaction_profiles/oculus/touch_controller", &pathInteractionProfile );
		XrPath pathLeftPose;        xrStringToPath( xrInstance, "/user/hand/left/input/grip/pose", &pathLeftPose );
		XrPath pathLeftAimPose;     xrStringToPath( xrInstance, "/user/hand/left/input/aim/pose", &pathLeftAimPose );
		XrPath pathLeftMoveX;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/x", &pathLeftMoveX );
		XrPath pathLeftMoveY;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/y", &pathLeftMoveY );
		XrPath pathLeftStick;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/click", &pathLeftStick );
		XrPath pathLeftStickTouch;  xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/touch", &pathLeftStickTouch );
		XrPath pathLeftTrigger;     xrStringToPath( xrInstance, "/user/hand/left/input/trigger/value", &pathLeftTrigger );
		XrPath pathLeftSqueeze;     xrStringToPath( xrInstance, "/user/hand/left/input/squeeze/value", &pathLeftSqueeze );
		XrPath pathLeftA;           xrStringToPath( xrInstance, "/user/hand/left/input/x/click", &pathLeftA );
		XrPath pathLeftB;           xrStringToPath( xrInstance, "/user/hand/left/input/y/click", &pathLeftB );
		XrPath pathRightPose;       xrStringToPath( xrInstance, "/user/hand/right/input/grip/pose", &pathRightPose );
		XrPath pathRightAimPose;    xrStringToPath( xrInstance, "/user/hand/right/input/aim/pose", &pathRightAimPose );
		XrPath pathRightMoveX;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/x", &pathRightMoveX );
		XrPath pathRightMoveY;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/y", &pathRightMoveY );
		XrPath pathRightStick;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/click", &pathRightStick );
		XrPath pathRightStickTouch; xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/touch", &pathRightStickTouch );
		XrPath pathRightTrigger;    xrStringToPath( xrInstance, "/user/hand/right/input/trigger/value", &pathRightTrigger );
		XrPath pathRightSqueeze;    xrStringToPath( xrInstance, "/user/hand/right/input/squeeze/value", &pathRightSqueeze );
		XrPath pathRightA;          xrStringToPath( xrInstance, "/user/hand/right/input/a/click", &pathRightA );
		XrPath pathRightB;          xrStringToPath( xrInstance, "/user/hand/right/input/b/click", &pathRightB );
	
		XrActionSuggestedBinding bindings[20] = { {xrActionHandPose,    pathLeftPose},       {xrActionHandPose,    pathRightPose},
			                                      {xrActionHandAimPose, pathLeftAimPose},    {xrActionHandAimPose, pathRightAimPose},
		                                          {xrActionMoveX,       pathLeftMoveX},      {xrActionMoveX,       pathRightMoveX},
		                                          {xrActionMoveY,       pathLeftMoveY},      {xrActionMoveY,       pathRightMoveY},
		                                          {xrActionStickClick,  pathLeftStick},      {xrActionStickClick,  pathRightStick},
			                                      {xrActionStickTouch,  pathLeftStickTouch}, {xrActionStickTouch,  pathRightStickTouch},
		                                          {xrActionTrigger,     pathLeftTrigger},    {xrActionTrigger,     pathRightTrigger},
		                                          {xrActionSqueeze,     pathLeftSqueeze},    {xrActionSqueeze,     pathRightSqueeze},
		                                          {xrActionA,           pathLeftA},          {xrActionA,           pathRightA},
		                                          {xrActionB,           pathLeftB},          {xrActionB,           pathRightB} };
		XrInteractionProfileSuggestedBinding suggestedBindings = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		suggestedBindings.interactionProfile = pathInteractionProfile;
		suggestedBindings.suggestedBindings = bindings;
		suggestedBindings.countSuggestedBindings = 20;
		xrRes = xrSuggestInteractionProfileBindings( xrInstance, &suggestedBindings );
		if ( xrRes < 0 ) AppError( "Oculus xrSuggestInteractionProfileBindings failed" );
	}
	
	{
		// HTC Vive bindings
		XrPath pathInteractionProfile; xrStringToPath( xrInstance, "/interaction_profiles/htc/vive_controller", &pathInteractionProfile );
		XrPath pathLeftPose;        xrStringToPath( xrInstance, "/user/hand/left/input/grip/pose", &pathLeftPose );
		XrPath pathLeftAimPose;     xrStringToPath( xrInstance, "/user/hand/left/input/aim/pose", &pathLeftAimPose );
		XrPath pathLeftTouchX;      xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/x", &pathLeftTouchX );
		XrPath pathLeftTouchY;      xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/y", &pathLeftTouchY );
		XrPath pathLeftTouchClick;  xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/click", &pathLeftTouchClick );
		XrPath pathLeftTouchTouch;  xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/touch", &pathLeftTouchTouch );
		XrPath pathLeftTrigger;     xrStringToPath( xrInstance, "/user/hand/left/input/trigger/value", &pathLeftTrigger );
		XrPath pathLeftSqueeze;     xrStringToPath( xrInstance, "/user/hand/left/input/squeeze/click", &pathLeftSqueeze );
		XrPath pathRightPose;       xrStringToPath( xrInstance, "/user/hand/right/input/grip/pose", &pathRightPose );
		XrPath pathRightAimPose;    xrStringToPath( xrInstance, "/user/hand/right/input/aim/pose", &pathRightAimPose );
		XrPath pathRightTouchX;     xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/x", &pathRightTouchX );
		XrPath pathRightTouchY;     xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/y", &pathRightTouchY );
		XrPath pathRightTouchClick; xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/click", &pathRightTouchClick );
		XrPath pathRightTouchTouch; xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/touch", &pathRightTouchTouch );
		XrPath pathRightTrigger;    xrStringToPath( xrInstance, "/user/hand/right/input/trigger/value", &pathRightTrigger );
		XrPath pathRightSqueeze;    xrStringToPath( xrInstance, "/user/hand/right/input/squeeze/click", &pathRightSqueeze );
	
		XrActionSuggestedBinding bindings[16] = { {xrActionHandPose,      pathLeftPose},       {xrActionHandPose,      pathRightPose},
			                                      {xrActionHandAimPose,   pathLeftAimPose},    {xrActionHandAimPose,   pathRightAimPose},
		                                          {xrActionTouchPadX,     pathLeftTouchX},     {xrActionTouchPadX,     pathRightTouchX},
		                                          {xrActionTouchPadY,     pathLeftTouchY},     {xrActionTouchPadY,     pathRightTouchY},
		                                          {xrActionTouchPadClick, pathLeftTouchClick}, {xrActionTouchPadClick, pathRightTouchClick},
			                                      {xrActionTouchPadTouch, pathLeftTouchTouch}, {xrActionTouchPadTouch, pathRightTouchTouch},
		                                          {xrActionTrigger,       pathLeftTrigger},    {xrActionTrigger,       pathRightTrigger},
		                                          {xrActionSqueeze,       pathLeftSqueeze},    {xrActionSqueeze,       pathRightSqueeze} };
		XrInteractionProfileSuggestedBinding suggestedBindings = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		suggestedBindings.interactionProfile = pathInteractionProfile;
		suggestedBindings.suggestedBindings = bindings;
		suggestedBindings.countSuggestedBindings = 16;
		xrRes = xrSuggestInteractionProfileBindings( xrInstance, &suggestedBindings );
		if ( xrRes < 0 ) AppError( "HTC Vive xrSuggestInteractionProfileBindings failed" );
	}

	{
		// Vive Index bindings
		XrPath pathInteractionProfile; xrStringToPath( xrInstance, "/interaction_profiles/valve/index_controller", &pathInteractionProfile );
		XrPath pathLeftPose;        xrStringToPath( xrInstance, "/user/hand/left/input/grip/pose", &pathLeftPose );
		XrPath pathLeftAimPose;     xrStringToPath( xrInstance, "/user/hand/left/input/aim/pose", &pathLeftAimPose );
		XrPath pathLeftMoveX;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/x", &pathLeftMoveX );
		XrPath pathLeftMoveY;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/y", &pathLeftMoveY );
		XrPath pathLeftStick;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/click", &pathLeftStick );
		XrPath pathLeftStickTouch;  xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/touch", &pathLeftStickTouch );
		XrPath pathLeftTouchX;      xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/x", &pathLeftTouchX );
		XrPath pathLeftTouchY;      xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/y", &pathLeftTouchY );
		XrPath pathLeftTouchTouch;  xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/touch", &pathLeftTouchTouch );
		XrPath pathLeftTrigger;     xrStringToPath( xrInstance, "/user/hand/left/input/trigger/value", &pathLeftTrigger );
		XrPath pathLeftSqueeze;     xrStringToPath( xrInstance, "/user/hand/left/input/squeeze/value", &pathLeftSqueeze );
		XrPath pathLeftA;           xrStringToPath( xrInstance, "/user/hand/left/input/a/click", &pathLeftA );
		XrPath pathLeftB;           xrStringToPath( xrInstance, "/user/hand/left/input/b/click", &pathLeftB );
		XrPath pathRightPose;       xrStringToPath( xrInstance, "/user/hand/right/input/grip/pose", &pathRightPose );
		XrPath pathRightAimPose;    xrStringToPath( xrInstance, "/user/hand/right/input/aim/pose", &pathRightAimPose );
		XrPath pathRightMoveX;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/x", &pathRightMoveX );
		XrPath pathRightMoveY;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/y", &pathRightMoveY );
		XrPath pathRightStick;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/click", &pathRightStick );
		XrPath pathRightStickTouch; xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/touch", &pathRightStickTouch );
		XrPath pathRightTouchX;     xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/x", &pathRightTouchX );
		XrPath pathRightTouchY;     xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/y", &pathRightTouchY );
		XrPath pathRightTouchTouch; xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/touch", &pathRightTouchTouch );
		XrPath pathRightTrigger;    xrStringToPath( xrInstance, "/user/hand/right/input/trigger/value", &pathRightTrigger );
		XrPath pathRightSqueeze;    xrStringToPath( xrInstance, "/user/hand/right/input/squeeze/value", &pathRightSqueeze );
		XrPath pathRightA;          xrStringToPath( xrInstance, "/user/hand/right/input/a/click", &pathRightA );
		XrPath pathRightB;          xrStringToPath( xrInstance, "/user/hand/right/input/b/click", &pathRightB );
	
		XrActionSuggestedBinding bindings[26] = { {xrActionHandPose,      pathLeftPose},       {xrActionHandPose,      pathRightPose},
			                                      {xrActionHandAimPose,   pathLeftAimPose},    {xrActionHandAimPose,   pathRightAimPose},
		                                          {xrActionMoveX,         pathLeftMoveX},      {xrActionMoveX,         pathRightMoveX},
		                                          {xrActionMoveY,         pathLeftMoveY},      {xrActionMoveY,         pathRightMoveY},
		                                          {xrActionStickClick,    pathLeftStick},      {xrActionStickClick,    pathRightStick},
			                                      {xrActionStickTouch,    pathLeftStickTouch}, {xrActionStickTouch,    pathRightStickTouch},
			                                      {xrActionTouchPadX,     pathLeftTouchX},     {xrActionTouchPadX,     pathRightTouchX},
		                                          {xrActionTouchPadY,     pathLeftTouchY},     {xrActionTouchPadY,     pathRightTouchY},
			                                      {xrActionTouchPadTouch, pathLeftTouchTouch}, {xrActionTouchPadTouch, pathRightTouchTouch},
		                                          {xrActionTrigger,       pathLeftTrigger},    {xrActionTrigger,       pathRightTrigger},
		                                          {xrActionSqueeze,       pathLeftSqueeze},    {xrActionSqueeze,       pathRightSqueeze},
		                                          {xrActionA,             pathLeftA},          {xrActionA,             pathRightA},
		                                          {xrActionB,             pathLeftB},          {xrActionB,             pathRightB} };
		XrInteractionProfileSuggestedBinding suggestedBindings = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		suggestedBindings.interactionProfile = pathInteractionProfile;
		suggestedBindings.suggestedBindings = bindings;
		suggestedBindings.countSuggestedBindings = 26;
		xrRes = xrSuggestInteractionProfileBindings( xrInstance, &suggestedBindings );
		if ( xrRes < 0 ) AppError( "Vive Index xrSuggestInteractionProfileBindings failed" );
	}

	{
		// WMR bindings
		XrPath pathInteractionProfile; xrStringToPath( xrInstance, "/interaction_profiles/microsoft/motion_controller", &pathInteractionProfile );
		XrPath pathLeftPose;        xrStringToPath( xrInstance, "/user/hand/left/input/grip/pose", &pathLeftPose );
		XrPath pathLeftAimPose;     xrStringToPath( xrInstance, "/user/hand/left/input/aim/pose", &pathLeftAimPose );
		XrPath pathLeftMoveX;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/x", &pathLeftMoveX );
		XrPath pathLeftMoveY;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/y", &pathLeftMoveY );
		XrPath pathLeftStick;       xrStringToPath( xrInstance, "/user/hand/left/input/thumbstick/click", &pathLeftStick );
		XrPath pathLeftTouchX;      xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/x", &pathLeftTouchX );
		XrPath pathLeftTouchY;      xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/y", &pathLeftTouchY );
		XrPath pathLeftTouchClick;  xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/click", &pathLeftTouchClick );
		XrPath pathLeftTouchTouch;  xrStringToPath( xrInstance, "/user/hand/left/input/trackpad/touch", &pathLeftTouchTouch );
		XrPath pathLeftTrigger;     xrStringToPath( xrInstance, "/user/hand/left/input/trigger/value", &pathLeftTrigger );
		XrPath pathLeftSqueeze;     xrStringToPath( xrInstance, "/user/hand/left/input/squeeze/click", &pathLeftSqueeze );
		XrPath pathRightPose;       xrStringToPath( xrInstance, "/user/hand/right/input/grip/pose", &pathRightPose );
		XrPath pathRightAimPose;    xrStringToPath( xrInstance, "/user/hand/right/input/aim/pose", &pathRightAimPose );
		XrPath pathRightMoveX;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/x", &pathRightMoveX );
		XrPath pathRightMoveY;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/y", &pathRightMoveY );
		XrPath pathRightStick;      xrStringToPath( xrInstance, "/user/hand/right/input/thumbstick/click", &pathRightStick );
		XrPath pathRightTouchX;     xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/x", &pathRightTouchX );
		XrPath pathRightTouchY;     xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/y", &pathRightTouchY );
		XrPath pathRightTouchClick; xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/click", &pathRightTouchClick );
		XrPath pathRightTouchTouch; xrStringToPath( xrInstance, "/user/hand/right/input/trackpad/touch", &pathRightTouchTouch );
		XrPath pathRightTrigger;    xrStringToPath( xrInstance, "/user/hand/right/input/trigger/value", &pathRightTrigger );
		XrPath pathRightSqueeze;    xrStringToPath( xrInstance, "/user/hand/right/input/squeeze/click", &pathRightSqueeze );
	
		XrActionSuggestedBinding bindings[22] = { {xrActionHandPose,      pathLeftPose},       {xrActionHandPose,      pathRightPose},
			                                      {xrActionHandAimPose,   pathLeftAimPose},    {xrActionHandAimPose,   pathRightAimPose},
		                                          {xrActionMoveX,         pathLeftMoveX},      {xrActionMoveX,         pathRightMoveX},
		                                          {xrActionMoveY,         pathLeftMoveY},      {xrActionMoveY,         pathRightMoveY},
		                                          {xrActionStickClick,    pathLeftStick},      {xrActionStickClick,    pathRightStick},
			                                      {xrActionTouchPadX,     pathLeftTouchX},     {xrActionTouchPadX,     pathRightTouchX},
		                                          {xrActionTouchPadY,     pathLeftTouchY},     {xrActionTouchPadY,     pathRightTouchY},
		                                          {xrActionTouchPadClick, pathLeftTouchClick}, {xrActionTouchPadClick, pathRightTouchClick},
			                                      {xrActionTouchPadTouch, pathLeftTouchTouch}, {xrActionTouchPadTouch, pathRightTouchTouch},
		                                          {xrActionTrigger,       pathLeftTrigger},    {xrActionTrigger,       pathRightTrigger},
		                                          {xrActionSqueeze,       pathLeftSqueeze},    {xrActionSqueeze,       pathRightSqueeze} };
		XrInteractionProfileSuggestedBinding suggestedBindings = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		suggestedBindings.interactionProfile = pathInteractionProfile;
		suggestedBindings.suggestedBindings = bindings;
		suggestedBindings.countSuggestedBindings = 22;
		xrRes = xrSuggestInteractionProfileBindings( xrInstance, &suggestedBindings );
		if ( xrRes < 0 ) AppError( "Vive Index xrSuggestInteractionProfileBindings failed" );
	}
	
    XrActionSpaceCreateInfo actionSpaceInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
    actionSpaceInfo.action = xrActionHandPose;
    actionSpaceInfo.poseInActionSpace.orientation.w = 1.0f;
    actionSpaceInfo.subactionPath = xrPathHands[APP_LEFT_HAND];
    xrRes = xrCreateActionSpace( xrSession, &actionSpaceInfo, &xrSpaceLeftHand );
	if ( xrRes < 0 ) AppError( "xrCreateActionSpace left hand failed" );
    actionSpaceInfo.subactionPath = xrPathHands[APP_RIGHT_HAND];
    xrRes = xrCreateActionSpace( xrSession, &actionSpaceInfo, &xrSpaceRightHand );
	if ( xrRes < 0 ) AppError( "xrCreateActionSpace right hand failed" );

	actionSpaceInfo.action = xrActionHandAimPose;
    actionSpaceInfo.subactionPath = xrPathHands[APP_LEFT_HAND];
    xrRes = xrCreateActionSpace( xrSession, &actionSpaceInfo, &xrSpaceLeftHandAim );
	if ( xrRes < 0 ) AppError( "xrCreateActionSpace left hand failed" );
    actionSpaceInfo.subactionPath = xrPathHands[APP_RIGHT_HAND];
    xrRes = xrCreateActionSpace( xrSession, &actionSpaceInfo, &xrSpaceRightHandAim );
	if ( xrRes < 0 ) AppError( "xrCreateActionSpace right hand failed" );

    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &xrActionSet;
    xrRes = xrAttachSessionActionSets( xrSession, &attachInfo );
	if ( xrRes < 0 ) AppError( "xrCreateActionSpace right hand failed" );
}

void OpenXREndSession()
{
	if ( xrSession == 0 ) return;

	xrSessionRunning = false;

	xrDestroySpace( xrSpaceLeftHand );     xrSpaceLeftHand = 0;
	xrDestroySpace( xrSpaceRightHand );    xrSpaceRightHand = 0;
	xrDestroySpace( xrSpaceLeftHandAim );  xrSpaceLeftHandAim = 0;
	xrDestroySpace( xrSpaceRightHandAim ); xrSpaceRightHandAim = 0;

	xrDestroyActionSet( xrActionSet ); xrActionSet = 0;

	xrDestroySwapchain( appSwapChains[0].swapchain ); appSwapChains[0] = { 0 };
	xrDestroySwapchain( appSwapChains[1].swapchain ); appSwapChains[1] = { 0 };

	xrDestroySpace( xrAppSpace );  xrAppSpace = 0;
	xrDestroySession( xrSession ); xrSession = 0;
}

void OpenXRDestroy()
{
	if ( xrInstance == 0 ) return;

	OpenXREndSession();
	xrSystemId = 0;
	xrDestroyInstance( xrInstance );
	xrInstance = 0;
}

void OpenXRGetActionFloat( XrAction action, XrPath path, float* outFloat )
{
	if ( !outFloat ) return;
	XrActionStateFloat floatValue = { XR_TYPE_ACTION_STATE_FLOAT };
	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = action;
	getInfo.subactionPath = path;
	XrResult xrRes = xrGetActionStateFloat( xrSession, &getInfo, &floatValue );
    if ( xrRes >= 0 && floatValue.isActive == XR_TRUE) *outFloat = floatValue.currentState;
}

void OpenXRGetActionBool( XrAction action, XrPath path, bool* outBool )
{
	if ( !outBool ) return;
	XrActionStateBoolean boolValue = { XR_TYPE_ACTION_STATE_BOOLEAN };
	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = action;
	getInfo.subactionPath = path;
	XrResult xrRes = xrGetActionStateBoolean( xrSession, &getInfo, &boolValue );
    if ( xrRes >= 0 && boolValue.isActive == XR_TRUE ) *outBool = (boolValue.currentState == XR_TRUE);
}

void OpenXRHandleEvents()
{
	XrEventDataBuffer event = { XR_TYPE_EVENT_DATA_BUFFER };
	XrResult xr;
	while( (xr = xrPollEvent(xrInstance, &event)) == XR_SUCCESS)
	{
		switch (event.type) 
		{
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: return;

			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: 
			{
				XrEventDataSessionStateChanged* sessionEvent = (XrEventDataSessionStateChanged*) &event;
				xrSessionState = sessionEvent->state;
				switch (xrSessionState) 
				{
					case XR_SESSION_STATE_READY: {
						XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
						sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
						XrResult xrRes = xrBeginSession(xrSession, &sessionBeginInfo);
						if ( xrRes < 0 ) { AppError( "xrBeginSession failed" ); }
						xrSessionRunning = true;
						break;
					}
					case XR_SESSION_STATE_STOPPING: {
						xrSessionRunning = false;
						XrResult xrRes = xrEndSession(xrSession);
						if ( xrRes < 0 ) { AppError( "xrEndSession failed" ); }
						break;
					}
					case XR_SESSION_STATE_EXITING: {
						xrSessionRunning = false;
						return;
					}
					case XR_SESSION_STATE_LOSS_PENDING: {
						OpenXREndSession();
						break;
					}
					default:
						break;
				}
				break;
			}

			case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: 
			{
				// get left hand profile
				XrInteractionProfileState profileState = { XR_TYPE_INTERACTION_PROFILE_STATE };
				XrPath topLevelPath; xrStringToPath( xrInstance, "/user/hand/left", &topLevelPath );
				xrGetCurrentInteractionProfile( xrSession, topLevelPath, &profileState );
				char profile[ 256 ];
				uint32_t length = 256;
				xrPathToString( xrInstance, profileState.interactionProfile, length, &length, profile );

				if ( strcmp( profile, "/interaction_profiles/oculus/touch_controller" ) == 0 )
				{
					// Oculus Touch controller
					appHasLeftAim = true;
					appHasLeftStick = true;
					appHasLeftTouchPad = false;
					appHasLeftTrigger = true;
					appHasLeftSqueeze = true;
					appHasLeftButtonA = true;
					appHasLeftButtonB = true;
					appLeftControllerType = OPENXR_CONTROLLER_OCULUS_TOUCH;
				}
				else if ( strcmp( profile, "/interaction_profiles/htc/vive_controller" ) == 0 )
				{
					// HTC Vive controller
					appHasLeftAim = true;
					appHasLeftStick = false;
					appHasLeftTouchPad = true;
					appHasLeftTrigger = true;
					appHasLeftSqueeze = true;
					appHasLeftButtonA = false;
					appHasLeftButtonB = false;
					appLeftControllerType = OPENXR_CONTROLLER_HTC_VIVE;
				}
				else if ( strcmp( profile, "/interaction_profiles/valve/index_controller" ) == 0 )
				{
					// Vive Index controller
					appHasLeftAim = true;
					appHasLeftStick = true;
					appHasLeftTouchPad = true;
					appHasLeftTrigger = true;
					appHasLeftSqueeze = true;
					appHasLeftButtonA = true;
					appHasLeftButtonB = true;
					appLeftControllerType = OPENXR_CONTROLLER_VALVE_INDEX;
				}
				else if ( strcmp( profile, "/interaction_profiles/microsoft/motion_controller" ) == 0 )
				{
					// WMR controller
					appHasLeftAim = true;
					appHasLeftStick = true;
					appHasLeftTouchPad = true;
					appHasLeftTrigger = true;
					appHasLeftSqueeze = true;
					appHasLeftButtonA = false;
					appHasLeftButtonB = false;
					appLeftControllerType = OPENXR_CONTROLLER_WMR;
				}

				// get right hand profile
				xrStringToPath( xrInstance, "/user/hand/right", &topLevelPath );
				xrGetCurrentInteractionProfile( xrSession, topLevelPath, &profileState );
				length = 256;
				xrPathToString( xrInstance, profileState.interactionProfile, length, &length, profile );

				if ( strcmp( profile, "/interaction_profiles/oculus/touch_controller" ) == 0 )
				{
					// Oculus Touch controller
					appHasRightAim = true;
					appHasRightStick = true;
					appHasRightTouchPad = false;
					appHasRightTrigger = true;
					appHasRightSqueeze = true;
					appHasRightButtonA = true;
					appHasRightButtonB = true;
					appRightControllerType = OPENXR_CONTROLLER_OCULUS_TOUCH;
				}
				else if ( strcmp( profile, "/interaction_profiles/htc/vive_controller" ) == 0 )
				{
					// HTC Vive controller
					appHasRightAim = true;
					appHasRightStick = false;
					appHasRightTouchPad = true;
					appHasRightTrigger = true;
					appHasRightSqueeze = true;
					appHasRightButtonA = false;
					appHasRightButtonB = false;
					appRightControllerType = OPENXR_CONTROLLER_HTC_VIVE;
				}
				else if ( strcmp( profile, "/interaction_profiles/valve/index_controller" ) == 0 )
				{
					// Vive Index controller
					appHasRightAim = true;
					appHasRightStick = true;
					appHasRightTouchPad = true;
					appHasRightTrigger = true;
					appHasRightSqueeze = true;
					appHasRightButtonA = true;
					appHasRightButtonB = true;
					appRightControllerType = OPENXR_CONTROLLER_VALVE_INDEX;
				}
				else if ( strcmp( profile, "/interaction_profiles/microsoft/motion_controller" ) == 0 )
				{
					// WMR controller
					appHasRightAim = true;
					appHasRightStick = true;
					appHasRightTouchPad = true;
					appHasRightTrigger = true;
					appHasRightSqueeze = true;
					appHasRightButtonA = false;
					appHasRightButtonB = false;
					appRightControllerType = OPENXR_CONTROLLER_WMR;
				}
				
				break;
			}

			case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
			default: {
				//printf("Ignoring event type %d", event.type);
				char msg[256];
				sprintf_s( msg, 256, "Ignoring event: %d", event.type );
				OutputDebugStringA( "    > " );
				OutputDebugStringA( msg );
				OutputDebugStringA( "\n" );
				break;
			}
		}

		event.type = XR_TYPE_EVENT_DATA_BUFFER;
	}

	if (xr != XR_EVENT_UNAVAILABLE) 
	{
		AppError( "xrPollEvent error" );
	}
	
	if ( OpenXRIsSessionSetup() )
	{
		// handle actions
		XrActionStatePose poseState = { XR_TYPE_ACTION_STATE_POSE };
		XrActionStateFloat floatValue = { XR_TYPE_ACTION_STATE_FLOAT };
		XrActionStateBoolean boolValue = { XR_TYPE_ACTION_STATE_BOOLEAN };

		// Sync actions
		const XrActiveActionSet activeActionSet = { xrActionSet, XR_NULL_PATH };
		XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
		syncInfo.countActiveActionSets = 1;
		syncInfo.activeActionSets = &activeActionSet;
		XrResult xrRes = xrSyncActions( xrSession, &syncInfo );
		if ( xrRes < 0 ) { AppError( "xrSyncActions failed" ); }

		// get left hand pose state
		XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
		getInfo.action = xrActionHandPose;
		getInfo.subactionPath = xrPathHands[ APP_LEFT_HAND ];
		xrRes = xrGetActionStatePose( xrSession, &getInfo, &poseState );
		appLeftHandActive = poseState.isActive ? 1 : 0;

		// get right hand pose state
		getInfo.subactionPath = xrPathHands[ APP_RIGHT_HAND ];
		xrRes = xrGetActionStatePose( xrSession, &getInfo, &poseState );
		appRightHandActive = poseState.isActive ? 1 : 0;

		appLeftStickX = 0;
		appLeftStickY = 0;
		appLeftStickClick = false;
		appLeftStickTouch = false;
		appLeftTouchPadX = 0;
		appLeftTouchPadY = 0;
		appLeftTouchPadClick = false;
		appLeftTouchPadTouch = false;
		appLeftTrigger = 0;
		appLeftSqueeze = 0;
		appLeftA = false;
		appLeftB = false;
		    
		OpenXRGetActionFloat( xrActionMoveX,         xrPathHands[ APP_LEFT_HAND ], &appLeftStickX );
		OpenXRGetActionFloat( xrActionMoveY,         xrPathHands[ APP_LEFT_HAND ], &appLeftStickY );
		OpenXRGetActionBool(  xrActionStickClick,    xrPathHands[ APP_LEFT_HAND ], &appLeftStickClick );
		OpenXRGetActionBool(  xrActionStickTouch,    xrPathHands[ APP_LEFT_HAND ], &appLeftStickTouch );
		OpenXRGetActionFloat( xrActionTouchPadX,     xrPathHands[ APP_LEFT_HAND ], &appLeftTouchPadX );
		OpenXRGetActionFloat( xrActionTouchPadY,     xrPathHands[ APP_LEFT_HAND ], &appLeftTouchPadY );
		OpenXRGetActionBool(  xrActionTouchPadClick, xrPathHands[ APP_LEFT_HAND ], &appLeftTouchPadClick );
		OpenXRGetActionBool(  xrActionTouchPadTouch, xrPathHands[ APP_LEFT_HAND ], &appLeftTouchPadTouch );
		OpenXRGetActionFloat( xrActionTrigger,       xrPathHands[ APP_LEFT_HAND ], &appLeftTrigger );
		OpenXRGetActionFloat( xrActionSqueeze,       xrPathHands[ APP_LEFT_HAND ], &appLeftSqueeze );
		OpenXRGetActionBool(  xrActionA,             xrPathHands[ APP_LEFT_HAND ], &appLeftA );
		OpenXRGetActionBool(  xrActionB,             xrPathHands[ APP_LEFT_HAND ], &appLeftB );

		appRightStickX = 0;
		appRightStickY = 0;
		appRightStickClick = false;
		appRightStickTouch = false;
		appRightTouchPadX = 0;
		appRightTouchPadY = 0;
		appRightTouchPadClick = false;
		appRightTouchPadTouch = false;
		appRightTrigger = 0;
		appRightSqueeze = 0;
		appRightA = false;
		appRightB = false;
	
		OpenXRGetActionFloat( xrActionMoveX,         xrPathHands[ APP_RIGHT_HAND ], &appRightStickX );
		OpenXRGetActionFloat( xrActionMoveY,         xrPathHands[ APP_RIGHT_HAND ], &appRightStickY );
		OpenXRGetActionBool(  xrActionStickClick,    xrPathHands[ APP_RIGHT_HAND ], &appRightStickClick );
		OpenXRGetActionBool(  xrActionStickTouch,    xrPathHands[ APP_RIGHT_HAND ], &appRightStickTouch );
		OpenXRGetActionFloat( xrActionTouchPadX,     xrPathHands[ APP_RIGHT_HAND ], &appRightTouchPadX );
		OpenXRGetActionFloat( xrActionTouchPadY,     xrPathHands[ APP_RIGHT_HAND ], &appRightTouchPadY );
		OpenXRGetActionBool(  xrActionTouchPadClick, xrPathHands[ APP_RIGHT_HAND ], &appRightTouchPadClick );
		OpenXRGetActionBool(  xrActionTouchPadTouch, xrPathHands[ APP_RIGHT_HAND ], &appRightTouchPadTouch );
		OpenXRGetActionFloat( xrActionTrigger,       xrPathHands[ APP_RIGHT_HAND ], &appRightTrigger );
		OpenXRGetActionFloat( xrActionSqueeze,       xrPathHands[ APP_RIGHT_HAND ], &appRightSqueeze );
		OpenXRGetActionBool(  xrActionA,             xrPathHands[ APP_RIGHT_HAND ], &appRightA );
		OpenXRGetActionBool(  xrActionB,             xrPathHands[ APP_RIGHT_HAND ], &appRightB );
	}
}

// left hand
bool OpenXRHasLeftAim() { return appHasLeftAim; }
bool OpenXRHasLeftStick() { return appHasLeftStick; }
bool OpenXRHasLeftTouchPad() { return appHasLeftTouchPad; }
bool OpenXRHasLeftTrigger() { return appHasLeftTrigger; }
bool OpenXRHasLeftSqueeze() { return appHasLeftSqueeze; }
bool OpenXRHasLeftButtonA() { return appHasLeftButtonA; }
bool OpenXRHasLeftButtonB() { return appHasLeftButtonB; }

int OpenXRGetLeftHandActive() { return appLeftHandActive; }
int OpenXRGetLeftHandControllerType() { return appLeftControllerType; }
void  OpenXRGetLeftHandPos( float* x, float* y, float* z ) 
{ 
	*x = appControllerPosLeft.x; 
	*y = appControllerPosLeft.y; 
	*z = appControllerPosLeft.z; 
}
void  OpenXRGetLeftHandQuat( float* w, float* x, float* y, float* z ) 
{ 
	*w = appControllerRotLeft.w; 
	*x = appControllerRotLeft.x; 
	*y = appControllerRotLeft.y; 
	*z = appControllerRotLeft.z; 
}
void  OpenXRGetLeftHandMatrix( float* m, float scale ) 
{ 
	CreateViewMatrix( appControllerPosLeft, appControllerRotLeft, m );
	m[0] *= scale;
	m[1] *= scale;
	m[2] *= scale;

	m[4] *= scale;
	m[5] *= scale;
	m[6] *= scale;

	m[8] *= scale;
	m[9] *= scale;
	m[10] *= scale;
}
void  OpenXRGetLeftHandAimPos( float* x, float* y, float* z ) 
{ 
	*x = appControllerAimPosLeft.x; 
	*y = appControllerAimPosLeft.y; 
	*z = appControllerAimPosLeft.z; 
}
void  OpenXRGetLeftHandAimQuat( float* w, float* x, float* y, float* z ) 
{ 
	*w = appControllerAimRotLeft.w; 
	*x = appControllerAimRotLeft.x; 
	*y = appControllerAimRotLeft.y; 
	*z = appControllerAimRotLeft.z; 
}
void  OpenXRGetLeftHandAimMatrix( float* m, float scale ) 
{ 
	CreateViewMatrix( appControllerAimPosLeft, appControllerAimRotLeft, m );
	m[0] *= scale;
	m[1] *= scale;
	m[2] *= scale;

	m[4] *= scale;
	m[5] *= scale;
	m[6] *= scale;

	m[8] *= scale;
	m[9] *= scale;
	m[10] *= scale;
}
float OpenXRGetLeftStickX()     { return appLeftStickX; }
float OpenXRGetLeftStickY()     { return appLeftStickY; }
bool  OpenXRGetLeftStickClick() { return appLeftStickClick; }
bool  OpenXRGetLeftStickTouch() { return appLeftStickTouch; }
float OpenXRGetLeftTouchPadX() { return appLeftTouchPadX; }
float OpenXRGetLeftTouchPadY() { return appLeftTouchPadY; }
bool  OpenXRGetLeftTouchPadClick() { return appLeftTouchPadClick; }
bool  OpenXRGetLeftTouchPadTouch() { return appLeftTouchPadTouch; }
float OpenXRGetLeftTrigger()    { return appLeftTrigger; }
float OpenXRGetLeftSqueeze()    { return appLeftSqueeze; }
bool  OpenXRGetLeftButtonA()    { return appLeftA; }
bool  OpenXRGetLeftButtonB()    { return appLeftB; }

// Right hand
bool OpenXRHasRightAim() { return appHasRightAim; }
bool OpenXRHasRightStick() { return appHasRightStick; }
bool OpenXRHasRightTouchPad() { return appHasRightTouchPad; }
bool OpenXRHasRightTrigger() { return appHasRightTrigger; }
bool OpenXRHasRightSqueeze() { return appHasRightSqueeze; }
bool OpenXRHasRightButtonA() { return appHasRightButtonA; }
bool OpenXRHasRightButtonB() { return appHasRightButtonB; }

int OpenXRGetRightHandActive() { return appRightHandActive; }
int OpenXRGetRightHandControllerType() { return appRightControllerType; }
void  OpenXRGetRightHandPos( float* x, float* y, float* z ) 
{ 
	*x = appControllerPosRight.x; 
	*y = appControllerPosRight.y; 
	*z = appControllerPosRight.z; 
}
void  OpenXRGetRightHandQuat( float* w, float* x, float* y, float* z ) 
{ 
	*w = appControllerRotRight.w; 
	*x = appControllerRotRight.x; 
	*y = appControllerRotRight.y; 
	*z = appControllerRotRight.z; 
}
void  OpenXRGetRightHandMatrix( float* m, float scale ) 
{ 
	CreateViewMatrix( appControllerPosRight, appControllerRotRight, m );
	m[0] *= scale;
	m[1] *= scale;
	m[2] *= scale;

	m[4] *= scale;
	m[5] *= scale;
	m[6] *= scale;

	m[8] *= scale;
	m[9] *= scale;
	m[10] *= scale;
}
void  OpenXRGetRightHandAimPos( float* x, float* y, float* z ) 
{ 
	*x = appControllerAimPosRight.x; 
	*y = appControllerAimPosRight.y; 
	*z = appControllerAimPosRight.z; 
}
void  OpenXRGetRightHandAimQuat( float* w, float* x, float* y, float* z ) 
{ 
	*w = appControllerAimRotRight.w; 
	*x = appControllerAimRotRight.x; 
	*y = appControllerAimRotRight.y; 
	*z = appControllerAimRotRight.z; 
}
void  OpenXRGetRightHandAimMatrix( float* m, float scale ) 
{ 
	CreateViewMatrix( appControllerAimPosRight, appControllerAimRotRight, m );
	m[0] *= scale;
	m[1] *= scale;
	m[2] *= scale;

	m[4] *= scale;
	m[5] *= scale;
	m[6] *= scale;

	m[8] *= scale;
	m[9] *= scale;
	m[10] *= scale;
}
float OpenXRGetRightStickX()     { return appRightStickX; }
float OpenXRGetRightStickY()     { return appRightStickY; }
bool  OpenXRGetRightStickClick() { return appRightStickClick; }
bool  OpenXRGetRightStickTouch() { return appRightStickTouch; }
float OpenXRGetRightTouchPadX()  { return appRightTouchPadX; }
float OpenXRGetRightTouchPadY()  { return appRightTouchPadY; }
bool  OpenXRGetRightTouchPadClick() { return appRightTouchPadClick; }
bool  OpenXRGetRightTouchPadTouch() { return appRightTouchPadTouch; }
float OpenXRGetRightTrigger()    { return appRightTrigger; }
float OpenXRGetRightSqueeze()    { return appRightSqueeze; }
bool  OpenXRGetRightButtonA()    { return appRightA; }
bool  OpenXRGetRightButtonB()    { return appRightB; }


// rendering functions, must be called in this order
// OpenXRStartFrame
// ImageView = OpenXRStartRender( OPENXR_RENDER_LEFT )
// Draw to ImageView
// OpenXREndRender
// ImageView = OpenXRStartRender( OPENXR_RENDER_RIGHT )
// Draw to ImageView
// OpenXREndRender
// OpenXREndFrame

typedef enum OpenXRFrameState
{
	OPENXR_RENDER_STATE_START_FRAME = 0,
	OPENXR_RENDER_STATE_RENDERING,
	OPENXR_RENDER_STATE_END_FRAME
} OpenXRFrameState;

OpenXRFrameState xrRenderState = OPENXR_RENDER_STATE_START_FRAME;
XrFrameState frameState;
XrCompositionLayerProjection layer = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
XrCompositionLayerProjectionView projectionLayerViews[ 2 ] = { {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}, {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW} };
int xrNumLayers = 0;

OpenXRRenderSide xrCurrentRenderSide = OPENXR_RENDER_UNDEFINED;

bool OpenXRStartFrame()
{
	if ( xrRenderState != OPENXR_RENDER_STATE_START_FRAME ) { AppError( "OpenXR functions must be called in the correct order" ); }

	XrFrameWaitInfo frameWaitInfo = { XR_TYPE_FRAME_WAIT_INFO };
	frameState = { XR_TYPE_FRAME_STATE };
	XrResult xrRes = xrWaitFrame( xrSession, &frameWaitInfo, &frameState );
	if ( xrRes < 0 ) { AppError( "xrWaitFrame failed" ); }

	XrFrameBeginInfo frameBeginInfo = { XR_TYPE_FRAME_BEGIN_INFO };
	xrRes = xrBeginFrame( xrSession, &frameBeginInfo );
	if ( xrRes < 0 ) { AppError( "xrBeginFrame failed" ); }

	xrRenderState = OPENXR_RENDER_STATE_END_FRAME;
	xrNumLayers = 0;

	if ( frameState.shouldRender != XR_TRUE ) return false; // skip rendering this loop

	xrNumLayers = 1;
	layer.viewCount = 2;
	layer.views = projectionLayerViews;
	layer.space = xrAppSpace;

	// locate views
	XrViewState viewState = { XR_TYPE_VIEW_STATE };

	XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
	viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	viewLocateInfo.displayTime = frameState.predictedDisplayTime;
	viewLocateInfo.space = xrAppSpace;

	uint32_t viewCountOutput;
	xrRes = xrLocateViews( xrSession, &viewLocateInfo, &viewState, 2, &viewCountOutput, xrViews );
	if ( xrRes != XR_SUCCESS ) { AppError( "xrLocateViews failed" ); }
	if ( viewCountOutput != 2 ) { AppError( "Located views must be equal to 2" ); }

	// locate hand spaces
	XrSpaceLocation spaceLocation = { XR_TYPE_SPACE_LOCATION };
	xrRes = xrLocateSpace( xrSpaceLeftHand, xrAppSpace, frameState.predictedDisplayTime, &spaceLocation );
	if ( xrRes < XR_SUCCESS ) { AppError( "xrLocateSpace failed" ); }
	if ( xrRes == XR_SUCCESS )
	{
		appControllerPosLeft = spaceLocation.pose.position;
		appControllerPosLeft.z = -appControllerPosLeft.z;
		appControllerRotLeft = spaceLocation.pose.orientation;
		appControllerRotLeft.y = -appControllerRotLeft.y;
		appControllerRotLeft.x = -appControllerRotLeft.x;
	}

	xrRes = xrLocateSpace( xrSpaceRightHand, xrAppSpace, frameState.predictedDisplayTime, &spaceLocation );
	if ( xrRes < XR_SUCCESS ) { AppError( "xrLocateSpace failed" ); }
	if ( xrRes == XR_SUCCESS )
	{
		appControllerPosRight = spaceLocation.pose.position;
		appControllerPosRight.z = -appControllerPosRight.z;
		appControllerRotRight = spaceLocation.pose.orientation;
		appControllerRotRight.y = -appControllerRotRight.y;
		appControllerRotRight.x = -appControllerRotRight.x;
	}

	// locate aim spaces
	xrRes = xrLocateSpace( xrSpaceLeftHandAim, xrAppSpace, frameState.predictedDisplayTime, &spaceLocation );
	if ( xrRes < XR_SUCCESS ) { AppError( "xrLocateSpace failed" ); }
	if ( xrRes == XR_SUCCESS )
	{
		appControllerAimPosLeft = spaceLocation.pose.position;
		appControllerAimPosLeft.z = -appControllerAimPosLeft.z;
		appControllerAimRotLeft = spaceLocation.pose.orientation;
		appControllerAimRotLeft.y = -appControllerAimRotLeft.y;
		appControllerAimRotLeft.x = -appControllerAimRotLeft.x;
	}

	xrRes = xrLocateSpace( xrSpaceRightHandAim, xrAppSpace, frameState.predictedDisplayTime, &spaceLocation );
	if ( xrRes < XR_SUCCESS ) { AppError( "xrLocateSpace failed" ); }
	if ( xrRes == XR_SUCCESS )
	{
		appControllerAimPosRight = spaceLocation.pose.position;
		appControllerAimPosRight.z = -appControllerAimPosRight.z;
		appControllerAimRotRight = spaceLocation.pose.orientation;
		appControllerAimRotRight.y = -appControllerAimRotRight.y;
		appControllerAimRotRight.x = -appControllerAimRotRight.x;
	}

	return true;
}

void OpenXRGetViewMat( OpenXRRenderSide side, float outMat[16] )
{
	if ( side < 0 || side > 1 ) { AppError( "Invalid render side" ); }

	XrVector3f pos = xrViews[side].pose.position;
	pos.z = -pos.z;
	XrQuaternionf rot = xrViews[side].pose.orientation;
	rot.x = -rot.x;
	rot.y = -rot.y;
	CreateViewMatrix( pos, rot, outMat );
}

void OpenXRGetInvViewMat( OpenXRRenderSide side, float outMat[16] )
{
	if ( side < 0 || side > 1 ) { AppError( "Invalid render side" ); }

	XrVector3f pos = xrViews[side].pose.position;
	pos.z = -pos.z;
	XrQuaternionf rot = xrViews[side].pose.orientation;
	rot.x = -rot.x;
	rot.y = -rot.y;
	CreateInvViewMatrix( pos, rot, outMat );
}

void OpenXRGetProjMat( OpenXRRenderSide side, float fNear, float fFar, float outMat[16], int invertedDepth )
{
	if ( side < 0 || side > 1 ) { AppError( "Invalid render side" ); }

	CreateProjMatrix( xrViews[side].fov, fNear, fFar, outMat, invertedDepth );
}

void OpenXRGetResolution( OpenXRRenderSide side, int* width, int* height )
{
	if ( side < 0 || side > 1 ) { AppError( "Invalid render side" ); }

	if ( width ) *width = appSwapChains[side].width;
	if ( height ) *height = appSwapChains[side].height;
}

unsigned int OpenXRGetSurfaceFormat()
{
	return (unsigned int) xrSwapchainFormat;
}

ID3D11RenderTargetView* OpenXRStartRender( OpenXRRenderSide side, ID3D11Texture2D** renderTextureOut )
{
	if ( xrRenderState != OPENXR_RENDER_STATE_END_FRAME ) { AppError( "OpenXR functions must be called in the correct order" ); }
	if ( side < 0 || side > 1 ) { AppError( "Invalid render side" ); }
	xrCurrentRenderSide = side;

	// acquire swap chain
	XrSwapchainImageAcquireInfo acquireInfo = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t imageIndex;
	XrResult xrRes = xrAcquireSwapchainImage( appSwapChains[side].swapchain, &acquireInfo, &imageIndex );
	if ( xrRes < 0 ) { AppError( "xrAcquireSwapchainImage failed" ); }

	XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	waitInfo.timeout = XR_INFINITE_DURATION;
	xrRes = xrWaitSwapchainImage( appSwapChains[side].swapchain, &waitInfo );
	if ( xrRes < 0 ) { AppError( "xrWaitSwapchainImage failed" ); }

	projectionLayerViews[side].pose = xrViews[side].pose;
	projectionLayerViews[side].fov = xrViews[side].fov;
	projectionLayerViews[side].subImage.swapchain = appSwapChains[side].swapchain;
	projectionLayerViews[side].subImage.imageRect.offset = { 0, 0 };
	projectionLayerViews[side].subImage.imageRect.extent = { (int)appSwapChains[side].width, (int)appSwapChains[side].height };

	if ( appSwapChains[side].imageViews[imageIndex] == 0 )
	{
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc( D3D11_RTV_DIMENSION_TEXTURE2D, (DXGI_FORMAT)xrSwapchainFormat );
		HRESULT hr = xrD3DDevice->CreateRenderTargetView( appSwapChains[side].images[imageIndex].texture, &renderTargetViewDesc, &appSwapChains[side].imageViews[imageIndex] );
		if ( FAILED(hr) ) { AppError( "CreateRenderTargetView failed" ); }
	}

	xrRenderState = OPENXR_RENDER_STATE_RENDERING;

	if ( renderTextureOut ) *renderTextureOut = appSwapChains[side].images[ imageIndex ].texture;

	return appSwapChains[side].imageViews[imageIndex];
}

void OpenXREndRender()
{
	if ( xrRenderState != OPENXR_RENDER_STATE_RENDERING ) { AppError( "OpenXR functions must be called in the correct order" ); }
	if ( xrCurrentRenderSide < 0 || xrCurrentRenderSide > 1 ) { AppError( "Invalid render side, must call OpenXRStartRender first" ); }

	XrSwapchainImageReleaseInfo releaseInfo = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	XrResult xrRes = xrReleaseSwapchainImage( appSwapChains[xrCurrentRenderSide].swapchain, &releaseInfo );
	if ( xrRes < 0 ) { AppError( "xrReleaseSwapchainImage failed" ); }

	xrCurrentRenderSide = OPENXR_RENDER_UNDEFINED;
	xrRenderState = OPENXR_RENDER_STATE_END_FRAME;
}

void OpenXREndFrame()
{
	if ( xrRenderState != OPENXR_RENDER_STATE_END_FRAME ) { AppError( "OpenXR functions must be called in the correct order" ); }

	XrCompositionLayerBaseHeader* layers[ 1 ];
	layers[ 0 ] = (XrCompositionLayerBaseHeader*) &layer;

	XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
	frameEndInfo.displayTime = frameState.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.layerCount = xrNumLayers;
	frameEndInfo.layers = layers;
	XrResult xrRes = xrEndFrame( xrSession, &frameEndInfo );
	if ( xrRes < 0 ) { AppError( "xrEndFrame failed" ); }

	xrRenderState = OPENXR_RENDER_STATE_START_FRAME;
}