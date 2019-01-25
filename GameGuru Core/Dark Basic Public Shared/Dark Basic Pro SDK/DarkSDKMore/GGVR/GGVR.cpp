#include "globstruct.h"
#include "GGVR.h"
#include "stdafx.h"
#include <string>


// Link to OpenVR library
#include "openvr.h"
#include "CCameraC.h"
#include "CImageC.h"
#include "CObjectsC.h"
#include "dxdiag.h"
#include "D3dx9math.h"


// Externs
extern GlobStruct*				g_pGlob;
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;
extern LPGGSURFACE				g_pBackBuffer;
extern LPGGRENDERTARGETVIEW		m_pRenderTargetView;

//Constants
const float GGVR_SINGULARITYRADIUS = 0.0000001f; // Use for Gimbal lock numerical problems
const float GGVR_180DIVPI = 57.2957795130823f;
const float GGVR_PI = 3.14159265358979f;
const float GGVR_PIOVER2 = 0.5f * GGVR_PI;
const float GGVR_radian = 0.0174533f;

const int GGVR_LEye = 0;
const int GGVR_REye = 1; 
const int GGVR_LEye_Depth = 2;
const int GGVR_REye_Depth = 3;

//Error Reporting
bool							GGVR_ErrorMessageOn = true;

//System
vr::IVRSystem					*ivr_system;
vr::EVRInitError				GGVR_init_error;
vr::EVRApplicationType			GGVR_ApplicationType = vr::VRApplication_Scene;
//Compositor
vr::IVRCompositor				*ivr_compositor;
vr::EVRCompositorError			GGVR_CompositorError;

//Chaperone
vr::IVRChaperone				*Chaperone;

//Info
std::string						GGVR_HMD_TrackingSysName = "NULL";
std::string						GGVR_HMD_ModelNo = "NULL";
std::string						GGVR_HMD_SerialNo = "NULL";
std::string						GGVR_HMD_Manufacturer = "NULL";
std::string						GGVR_RCntrl_TrackingSysName = "NULL";
std::string						GGVR_RCntrl_ModelNo = "NULL";
std::string						GGVR_RCntrl_SerialNo = "NULL";
std::string						GGVR_RCntrl_Manufacturer = "NULL";
std::string						GGVR_LCntrl_TrackingSysName = "NULL";
std::string						GGVR_LCntrl_ModelNo = "NULL";
std::string						GGVR_LCntrl_SerialNo = "NULL";
std::string						GGVR_LCntrl_Manufacturer = "NULL";

//Tracking Space
vr::ETrackingUniverseOrigin		GGVR_TrackingSpace = vr::TrackingUniverseSeated;

//Reccomended Texture Sizes
uint32_t						GGVR_EyeW;
uint32_t						GGVR_EyeH;

//SubmitFlags (if distortion is already applied or not)
vr::EVRSubmitFlags				GGVR_SubmitFlags = vr::Submit_Default;

//Pointers to Render Images
ID3D11Resource*					GGVR_LEyeImage_Res;
ID3D11Resource*					GGVR_REyeImage_Res;
ID3D11Resource*					GGVR_Skybox_Res[6];

ID3D11Texture2D*				GGVR_LEyeImage;
ID3D11Texture2D*				GGVR_REyeImage;
ID3D11Texture2D*				GGVR_Skybox[6];

int								GGVR_REyeImageID;
int								GGVR_LEyeImageID;
int								GGVR_REyeImageID_Depth = -1;
int								GGVR_LEyeImageID_Depth = -1;
int								GGVR_RCamID;
int								GGVR_LCamID;

//Raw Projection Matrix Values and camera clipping ranges
float							GGVR_LeyeProj_Top, GGVR_LeyeProj_Bot, GGVR_LeyeProj_Left, GGVR_LeyeProj_Right;
float							GGVR_ReyeProj_Top, GGVR_ReyeProj_Bot, GGVR_ReyeProj_Left, GGVR_ReyeProj_Right;
float							GGVR_NearClip = 1.0f;
float							GGVR_FarClip = 70000.0f;
float							ProjectionRaw[2][4];


//Tracking Arrays
vr::TrackedDevicePose_t			pGamePoseArray[vr::k_unMaxTrackedDeviceCount];
vr::HmdVector3_t				pGamePoseArray_YPR[vr::k_unMaxTrackedDeviceCount];
vr::HmdVector3_t				pGamePoseArray_Pos[vr::k_unMaxTrackedDeviceCount];
vr::TrackedDevicePose_t			pRenderPoseArray[vr::k_unMaxTrackedDeviceCount];
vr::HmdVector3_t				pRenderPoseArray_YPR[vr::k_unMaxTrackedDeviceCount];
vr::HmdVector3_t				pRenderPoseArray_Pos[vr::k_unMaxTrackedDeviceCount];

//Index for Devices
uint32_t						GGVR_HMD = 0;
uint32_t						GGVR_RHandIndex = 0;
uint32_t						GGVR_LHandIndex = 0;
//Controller States
vr::VRControllerState001_t		GGVR_ControllerState[2];
//Axis info: JoySticks, DPads and Triggers
int								GGVR_AxisType[2][5];
int								GGVR_RTrigger = -1;
int								GGVR_LTrigger = -1;
int								GGVR_RJoystick = -1;
int								GGVR_LJoystick = -1;
//Controller Button Presses
bool							GGVR_RCntrlBut_Press[64];
bool							GGVR_RCntrlBut_Touch[64];
bool							GGVR_LCntrlBut_Press[64];
bool							GGVR_LCntrlBut_Touch[64];

//Texture Info 
vr::Texture_t					GGVR_tex_info[2];
vr::VRTextureBounds_t			GGVR_texture_bounds[2];

//Skybox Texture
vr::Texture_t					GGVR_CustomSkybox[6];

//Camera Reference
//cCamera*						pCamera;
float							GGVR_WorldScale = 39.3700787;
float							GGVR_ipdscale = 1.0;

GGMATRIX						GGVR_LeftEyeProjection;
GGMATRIX						GGVR_RightEyeProjection;


class GGVR_PlayerData
{
public:
	int ObjBase;
	int ObjHead;
	int ObjOrigin;
	int ObjRightHand;
	int ObjLeftHand;
	void Create();
	void Destroy();

	int GetBaseObjID();
	int GetHeadObjID();
	int GetOriginObjID();
	int GetRHandObjID();
	int GetLHandObjID();

	bool PitchLock;
	bool TurnLock;
};
GGVR_PlayerData				GGVR_Player;


//Internal Functions
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError);
void GGVR_Mat34toYPR(vr::HmdMatrix34_t *Mat, vr::HmdVector3_t *YPR, vr::HmdVector3_t *Pos);
void GGVR_UpdatePoses(void);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{

	return TRUE;
}

//*********************************
//	Setup Commands
//*********************************

//DARKSDK void ReceiveCoreDataPtr(LPVOID pCore)
//{
//	g_pGlob = (GlobStruct*)pCore;
//}

DARKSDK int	GGVR_IsHmdPresent()
{
	int result = 0;
	result = vr::VR_IsHmdPresent();
	return result;
}

int	GGVR_IsRuntimeInstalled()
{
	bool result = 0;
	result = vr::VR_IsRuntimeInstalled();
	return (int)result;
}

int	GGVR_Init(int RImageID, int LImageID, int RCamID, int LCamID, int ObjBase, int ObjHead, int ObjOrigin, int ObjRightHand, int ObjLeftHand)
{

	//Set Onject ID's for Player dummy objects
	GGVR_Player.ObjBase = ObjBase;
	GGVR_Player.ObjHead = ObjHead;
	GGVR_Player.ObjOrigin = ObjOrigin;
	GGVR_Player.ObjRightHand = ObjRightHand;
	GGVR_Player.ObjLeftHand = ObjLeftHand;

	//Create Cameras
	GGVR_RCamID = RCamID;
	GGVR_LCamID = LCamID;
	if (CameraExist(RCamID) == 0)
	{
		CreateCamera(RCamID);
	}
	SetCameraRange(RCamID, GGVR_NearClip, GGVR_FarClip);
	if (CameraExist(LCamID) == 0)
	{
		CreateCamera(LCamID);
	}
	SetCameraRange(LCamID, GGVR_NearClip, GGVR_FarClip);


	GGVR_init_error = vr::VRInitError_None;

	//Initialize OpenVR
	ivr_system = vr::VR_Init(&GGVR_init_error, GGVR_ApplicationType);
	//Could add ability to change application type, but probably no reason.
	// Default is: vr::VRApplication_Scene
	//Other options:
	//	vr::VRApplication_Other
	//	vr::VRApplication_Overlay
	//	vr::VRApplication_Background
	//	vr::VRApplication_Utility
	if (GGVR_init_error != vr::VRInitError_None)
	{
		if (GGVR_ErrorMessageOn == true)
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(GGVR_init_error));
			MessageBox(GetForegroundWindow(), buf, "VR Error", MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);
		}
		return 1;
	}

	//Initialize the Compositor
	ivr_compositor = vr::VRCompositor();
	if (ivr_compositor == NULL)
	{
		ivr_system = NULL;
		vr::VR_Shutdown();
		if (GGVR_ErrorMessageOn == true)
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "Compositor initialization failed.");
			MessageBox(GetForegroundWindow(), buf, "VR Error", MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);
		}
		return 2;
	}

	//Chaperone
	Chaperone = vr::VRChaperone();

	//Info Strings for the VR Hardware
	//-----------------------------------
	GGVR_HMD_TrackingSysName = GetTrackedDeviceString(ivr_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String, NULL);
	GGVR_HMD_ModelNo = GetTrackedDeviceString(ivr_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String, NULL);
	GGVR_HMD_SerialNo = GetTrackedDeviceString(ivr_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String, NULL);
	GGVR_HMD_Manufacturer = GetTrackedDeviceString(ivr_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ManufacturerName_String, NULL);


	//Texture Bounds
	//-------------------------------------
	GGVR_texture_bounds[GGVR_REye].uMin = 0;
	GGVR_texture_bounds[GGVR_REye].uMax = 1;
	GGVR_texture_bounds[GGVR_REye].vMin = 0;
	GGVR_texture_bounds[GGVR_REye].vMax = 1;
	GGVR_texture_bounds[GGVR_LEye].uMin = 0;
	GGVR_texture_bounds[GGVR_LEye].uMax = 1;
	GGVR_texture_bounds[GGVR_LEye].vMin = 0;
	GGVR_texture_bounds[GGVR_LEye].vMax = 1;

	//Reccomended Texture Size
	ivr_system->GetRecommendedRenderTargetSize(&GGVR_EyeW, &GGVR_EyeH);

	//Set Submit Flags
	GGVR_SubmitFlags = vr::Submit_Default;

	//Create Render Images
	GGVR_REyeImageID = RImageID;
	GGVR_LEyeImageID = LImageID;
	//SetCameraToImage(GGVR_LCamID, GGVR_LEyeImageID, GGVR_EyeW, GGVR_EyeH,3,28);
	//SetCameraToImage(GGVR_RCamID, GGVR_REyeImageID, GGVR_EyeW, GGVR_EyeH,3,28);
	SetCameraToImage(GGVR_RCamID, GGVR_REyeImageID, GGVR_EyeW, GGVR_EyeH);
	SetCameraToImage(GGVR_LCamID, GGVR_LEyeImageID, GGVR_EyeW, GGVR_EyeH);

	GGVR_REyeImage_Res = GetImagePointer(GGVR_REyeImageID);
	GGVR_LEyeImage_Res = GetImagePointer(GGVR_LEyeImageID);

	GGVR_REyeImage_Res->QueryInterface(IID_ID3D11Texture2D, (void **)&GGVR_REyeImage);
	GGVR_LEyeImage_Res->QueryInterface(IID_ID3D11Texture2D, (void **)&GGVR_LEyeImage);

	GGVR_tex_info[GGVR_LEye].eType = vr::TextureType_DirectX;
	GGVR_tex_info[GGVR_REye].eType = vr::TextureType_DirectX;
	GGVR_tex_info[GGVR_LEye].eColorSpace = vr::ColorSpace_Auto;
	GGVR_tex_info[GGVR_REye].eColorSpace = vr::ColorSpace_Auto;
	GGVR_tex_info[GGVR_LEye].handle = (void**)GGVR_LEyeImage;
	GGVR_tex_info[GGVR_REye].handle = (void**)GGVR_REyeImage;
	
	//Get the Projection Matrix values
	//ivr_system->GetProjectionRaw( vr::Eye_Left, &GGVR_LeyeProj_Left, &GGVR_LeyeProj_Right, &GGVR_LeyeProj_Top, &GGVR_LeyeProj_Bot );
	//ivr_system->GetProjectionRaw( vr::Eye_Right, &GGVR_ReyeProj_Left, &GGVR_ReyeProj_Right, &GGVR_ReyeProj_Top, &GGVR_ReyeProj_Bot );

	vr::HmdMatrix44_t HMDMat_Left;
	vr::HmdMatrix44_t HMDMat_Right;
	HMDMat_Left = ivr_system->GetProjectionMatrix(vr::Eye_Left, GGVR_NearClip, GGVR_FarClip);
	HMDMat_Right = ivr_system->GetProjectionMatrix(vr::Eye_Right, GGVR_NearClip, GGVR_FarClip);

	GGVR_LeftEyeProjection._11 = HMDMat_Left.m[0][0]; GGVR_LeftEyeProjection._12 = HMDMat_Left.m[0][1]; GGVR_LeftEyeProjection._13 = HMDMat_Left.m[0][2]; GGVR_LeftEyeProjection._14 = HMDMat_Left.m[0][3];
	GGVR_LeftEyeProjection._21 = HMDMat_Left.m[1][0]; GGVR_LeftEyeProjection._22 = HMDMat_Left.m[1][1]; GGVR_LeftEyeProjection._23 = HMDMat_Left.m[1][2]; GGVR_LeftEyeProjection._24 = HMDMat_Left.m[1][3];
	GGVR_LeftEyeProjection._31 = HMDMat_Left.m[2][0]; GGVR_LeftEyeProjection._32 = HMDMat_Left.m[2][1]; GGVR_LeftEyeProjection._33 = HMDMat_Left.m[2][2]; GGVR_LeftEyeProjection._34 = HMDMat_Left.m[2][3];
	GGVR_LeftEyeProjection._41 = HMDMat_Left.m[3][0]; GGVR_LeftEyeProjection._42 = HMDMat_Left.m[3][1]; GGVR_LeftEyeProjection._43 = HMDMat_Left.m[3][2]; GGVR_LeftEyeProjection._44 = HMDMat_Left.m[3][3];

	GGVR_RightEyeProjection._11 = HMDMat_Right.m[0][0]; GGVR_RightEyeProjection._12 = HMDMat_Right.m[0][1]; GGVR_RightEyeProjection._13 = HMDMat_Right.m[0][2]; GGVR_RightEyeProjection._14 = HMDMat_Right.m[0][3];
	GGVR_RightEyeProjection._21 = HMDMat_Right.m[1][0]; GGVR_RightEyeProjection._22 = HMDMat_Right.m[1][1]; GGVR_RightEyeProjection._23 = HMDMat_Right.m[1][2]; GGVR_RightEyeProjection._24 = HMDMat_Right.m[1][3];
	GGVR_RightEyeProjection._31 = HMDMat_Right.m[2][0]; GGVR_RightEyeProjection._32 = HMDMat_Right.m[2][1]; GGVR_RightEyeProjection._33 = HMDMat_Right.m[2][2]; GGVR_RightEyeProjection._34 = HMDMat_Right.m[2][3];
	GGVR_RightEyeProjection._41 = HMDMat_Right.m[3][0]; GGVR_RightEyeProjection._42 = HMDMat_Right.m[3][1]; GGVR_RightEyeProjection._43 = HMDMat_Right.m[3][2]; GGVR_RightEyeProjection._44 = HMDMat_Right.m[3][3];
	
	GGVR_Player.Create();

	return 0;
}

void GGVR_Shutdown()
{
	// Free any playerdata object resources
	GGVR_Player.Destroy();

	// Free camera images associated with VR rendering
	SetCameraToImage(GGVR_RCamID, -1, 0, 0);
	SetCameraToImage(GGVR_LCamID, -1, 0, 0);

	// final shutdown of VR system
	vr::VR_Shutdown();
}

GGMATRIX GGVR_GetRightEyeProjectionMatrix()
{
	return GGVR_RightEyeProjection;
}

GGMATRIX GGVR_GetLeftEyeProjectionMatrix()
{
	return GGVR_LeftEyeProjection;
}

void	GGVR_CreateSkyboxOveride(int Front, int Back, int Left, int Right, int Top, int Bottom)
{
	GGVR_Skybox_Res[0] = GetImagePointer(Front);
	GGVR_Skybox_Res[1] = GetImagePointer(Back);
	GGVR_Skybox_Res[2] = GetImagePointer(Left);
	GGVR_Skybox_Res[3] = GetImagePointer(Right);
	GGVR_Skybox_Res[4] = GetImagePointer(Top);
	GGVR_Skybox_Res[5] = GetImagePointer(Bottom);
	for (int i = 0; i < 6; i++)
	{
		GGVR_CustomSkybox[i].eType = vr::TextureType_DirectX;
		GGVR_CustomSkybox[i].eColorSpace = vr::ColorSpace_Auto;
	}
	GGVR_CustomSkybox[0].handle = GetImagePointer(Front);
	GGVR_CustomSkybox[1].handle = GetImagePointer(Back);
	GGVR_CustomSkybox[2].handle = GetImagePointer(Left);
	GGVR_CustomSkybox[3].handle = GetImagePointer(Right);
	GGVR_CustomSkybox[4].handle = GetImagePointer(Top);
	GGVR_CustomSkybox[5].handle = GetImagePointer(Bottom);
	ivr_compositor->SetSkyboxOverride(GGVR_CustomSkybox, 6);
}

void	GGVR_ClearSkyboxOveride()
{
	ivr_compositor->ClearSkyboxOverride();
}

void GGVR_ErrorMessagesOn(int state )
{
	if (state == 0)
	{
		GGVR_ErrorMessageOn = false;
	}
	else { GGVR_ErrorMessageOn = true; }
}
void GGVR_SetTrackingSpace( int space )
{
	if (space == 0)
	{
		GGVR_TrackingSpace = vr::TrackingUniverseSeated;
	}
	else { GGVR_TrackingSpace = vr::TrackingUniverseStanding; }
}
int GGVR_GetTrackingSpace( int space )
{
	if (GGVR_TrackingSpace == vr::TrackingUniverseSeated)
	{
		return 0;
	}
	else { return 1; }
}
void GGVR_ResetSeatedZeroPose()
{
	if (GGVR_init_error == vr::VRInitError_None && ivr_compositor != NULL)
	{
		ivr_system->ResetSeatedZeroPose();
	}
}
void GGVR_SetCameraRange( float Near, float Far )
{
	GGVR_NearClip = Near;
	GGVR_FarClip = Far;

	SetCameraRange(GGVR_RCamID, GGVR_NearClip, GGVR_FarClip);
	SetCameraRange(GGVR_LCamID, GGVR_NearClip, GGVR_FarClip);

	vr::HmdMatrix44_t HMDMat_Left;
	vr::HmdMatrix44_t HMDMat_Right;
	HMDMat_Left = ivr_system->GetProjectionMatrix(vr::Eye_Left, GGVR_NearClip, GGVR_FarClip);
	HMDMat_Right = ivr_system->GetProjectionMatrix(vr::Eye_Right, GGVR_NearClip, GGVR_FarClip);

	GGVR_LeftEyeProjection._11 = HMDMat_Left.m[0][0]; GGVR_LeftEyeProjection._12 = HMDMat_Left.m[0][1]; GGVR_LeftEyeProjection._13 = HMDMat_Left.m[0][2]; GGVR_LeftEyeProjection._14 = HMDMat_Left.m[0][3];
	GGVR_LeftEyeProjection._21 = HMDMat_Left.m[1][0]; GGVR_LeftEyeProjection._22 = HMDMat_Left.m[1][1]; GGVR_LeftEyeProjection._23 = HMDMat_Left.m[1][2]; GGVR_LeftEyeProjection._24 = HMDMat_Left.m[1][3];
	GGVR_LeftEyeProjection._31 = HMDMat_Left.m[2][0]; GGVR_LeftEyeProjection._32 = HMDMat_Left.m[2][1]; GGVR_LeftEyeProjection._33 = HMDMat_Left.m[2][2]; GGVR_LeftEyeProjection._34 = HMDMat_Left.m[2][3];
	GGVR_LeftEyeProjection._41 = HMDMat_Left.m[3][0]; GGVR_LeftEyeProjection._42 = HMDMat_Left.m[3][1]; GGVR_LeftEyeProjection._43 = HMDMat_Left.m[3][2]; GGVR_LeftEyeProjection._44 = HMDMat_Left.m[3][3];

	GGVR_RightEyeProjection._11 = HMDMat_Right.m[0][0]; GGVR_RightEyeProjection._12 = HMDMat_Right.m[0][1]; GGVR_RightEyeProjection._13 = HMDMat_Right.m[0][2]; GGVR_RightEyeProjection._14 = HMDMat_Right.m[0][3];
	GGVR_RightEyeProjection._21 = HMDMat_Right.m[1][0]; GGVR_RightEyeProjection._22 = HMDMat_Right.m[1][1]; GGVR_RightEyeProjection._23 = HMDMat_Right.m[1][2]; GGVR_RightEyeProjection._24 = HMDMat_Right.m[1][3];
	GGVR_RightEyeProjection._31 = HMDMat_Right.m[2][0]; GGVR_RightEyeProjection._32 = HMDMat_Right.m[2][1]; GGVR_RightEyeProjection._33 = HMDMat_Right.m[2][2]; GGVR_RightEyeProjection._34 = HMDMat_Right.m[2][3];
	GGVR_RightEyeProjection._41 = HMDMat_Right.m[3][0]; GGVR_RightEyeProjection._42 = HMDMat_Right.m[3][1]; GGVR_RightEyeProjection._43 = HMDMat_Right.m[3][2]; GGVR_RightEyeProjection._44 = HMDMat_Right.m[3][3];
	
}
void GGVR_SetWorldScale( float scale )
{
	GGVR_WorldScale = scale;
}
float GGVR_GetWorldScale(  )
{
	return GGVR_WorldScale;
}
void GGVR_SuspendRendering(int flag)
{
	if (flag == 1)
	{
		ivr_compositor->SuspendRendering(true);
	}
	else
	{
		ivr_compositor->SuspendRendering(false);
	}
}

void GGVR_Submit()
{
	if (GGVR_init_error == vr::VRInitError_None && ivr_compositor != NULL)
	{

		GGVR_CompositorError = ivr_compositor->Submit(vr::Eye_Left, &GGVR_tex_info[GGVR_LEye], &GGVR_texture_bounds[GGVR_LEye], GGVR_SubmitFlags);
		GGVR_CompositorError = ivr_compositor->Submit(vr::Eye_Right, &GGVR_tex_info[GGVR_REye], &GGVR_texture_bounds[GGVR_REye], GGVR_SubmitFlags);

	}
}

//2017-11-02: Added to allow rendering to individual eyes
void GGVR_Submit_RightEye()
{
	if (GGVR_init_error == vr::VRInitError_None && ivr_compositor != NULL)
	{		
		GGVR_CompositorError = ivr_compositor->Submit(vr::Eye_Right, &GGVR_tex_info[GGVR_REye], &GGVR_texture_bounds[GGVR_REye], GGVR_SubmitFlags);
	}
}
//2017-11-02: Added to allow rendering to individual eyes
void GGVR_Submit_LeftEye()
{
	if (GGVR_init_error == vr::VRInitError_None && ivr_compositor != NULL)
	{
		GGVR_CompositorError = ivr_compositor->Submit(vr::Eye_Left, &GGVR_tex_info[GGVR_LEye], &GGVR_texture_bounds[GGVR_LEye], GGVR_SubmitFlags);
	}
}


//*********************************
//	Player Control
//*********************************

int GGVR_GetPlayerOriginObjID( )
{
	return GGVR_Player.GetOriginObjID( );
}
int GGVR_GetPlayerHeadObjID( )
{
	return GGVR_Player.GetHeadObjID( );
}
int GGVR_GetPlayerBaseObjID( )
{
	return GGVR_Player.GetBaseObjID( );
}
int GGVR_GetPlayerRightHandObjID( )
{
	return GGVR_Player.GetRHandObjID( );
}
int GGVR_GetPlayerLeftHandObjID( )
{
	return GGVR_Player.GetLHandObjID( );
}
void GGVR_UpdatePlayer( )
{
	//Update the HMD and controller feedbacks=
	GGVR_UpdatePoses();

	//Position the Base Object offset from the origin
	//-------------------------------------------------------
	//Set the base to the orientation of the origin
	RotateObject(GGVR_Player.ObjBase, ObjectAngleX(GGVR_Player.ObjOrigin), ObjectAngleY(GGVR_Player.ObjOrigin), ObjectAngleZ(GGVR_Player.ObjOrigin));
	//Set the base to the position of the origin
	PositionObject(GGVR_Player.ObjBase, ObjectPositionX(GGVR_Player.ObjOrigin), ObjectPositionY(GGVR_Player.ObjOrigin), ObjectPositionZ(GGVR_Player.ObjOrigin));
	//Move the base on the local X and Z axis of the origin based on the X and Z HMD offset distances
	MoveObject(GGVR_Player.ObjBase, pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
	MoveObjectRight(GGVR_Player.ObjBase, pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	//Rotate the base's Y axis to match the turn angle offset of the HMD 
	if (GGVR_Player.TurnLock == 1)
	{
		TurnObjectRight(GGVR_Player.ObjBase, pGamePoseArray_YPR[GGVR_HMD].v[1]);
	}
	//Rotate the base's X axis to match the pitch angle offset of the HMD 
	if (GGVR_Player.PitchLock == 1)
	{
		PitchObjectDown(GGVR_Player.ObjBase, pGamePoseArray_YPR[GGVR_HMD].v[0]);
	}


	//Position the Head Object based on the origin
	//-------------------------------------------------------
	//Set the head to the orientation of the origin
	RotateObject(GGVR_Player.ObjHead, ObjectAngleX(GGVR_Player.ObjOrigin), ObjectAngleY(GGVR_Player.ObjOrigin), ObjectAngleZ(GGVR_Player.ObjOrigin));
	//Set the head to the position of the origin
	PositionObject(GGVR_Player.ObjHead, ObjectPositionX(GGVR_Player.ObjOrigin), ObjectPositionY(GGVR_Player.ObjOrigin), ObjectPositionZ(GGVR_Player.ObjOrigin));
	//Move the head on the local X, Y and Z axis of the origin based on the X, Y and Z HMD offset distances
	MoveObjectUp(GGVR_Player.ObjHead, pGamePoseArray_Pos[GGVR_HMD].v[1] * GGVR_WorldScale);
	MoveObject(GGVR_Player.ObjHead, pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
	MoveObjectRight(GGVR_Player.ObjHead, pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	//Rotate the head based on the Turn, Pitch and Roll offsest of the HMD
	TurnObjectRight(GGVR_Player.ObjHead, pGamePoseArray_YPR[GGVR_HMD].v[1]);
	PitchObjectDown(GGVR_Player.ObjHead, pGamePoseArray_YPR[GGVR_HMD].v[0]);
	RollObjectRight(GGVR_Player.ObjHead, -pGamePoseArray_YPR[GGVR_HMD].v[2]);

	//Set Cameras based on head orientation
	PositionCamera(GGVR_RCamID, ObjectPositionX(GGVR_Player.ObjHead), ObjectPositionY(GGVR_Player.ObjHead), ObjectPositionZ(GGVR_Player.ObjHead));
	PositionCamera(GGVR_LCamID, ObjectPositionX(GGVR_Player.ObjHead), ObjectPositionY(GGVR_Player.ObjHead), ObjectPositionZ(GGVR_Player.ObjHead));
	RotateCamera(GGVR_RCamID, ObjectAngleX(GGVR_Player.ObjHead), ObjectAngleY(GGVR_Player.ObjHead), ObjectAngleZ(GGVR_Player.ObjHead));
	RotateCamera(GGVR_LCamID, ObjectAngleX(GGVR_Player.ObjHead), ObjectAngleY(GGVR_Player.ObjHead), ObjectAngleZ(GGVR_Player.ObjHead));
	//Move Cameras for IPD Distance
	MoveCameraRight(GGVR_RCamID, (ivr_system->GetFloatTrackedDeviceProperty(0, vr::Prop_UserIpdMeters_Float) / 2.0f)*GGVR_WorldScale*GGVR_ipdscale);
	MoveCameraRight(GGVR_LCamID, -(ivr_system->GetFloatTrackedDeviceProperty(0, vr::Prop_UserIpdMeters_Float) / 2.0f)*GGVR_WorldScale*GGVR_ipdscale);

	//Set Right Hand Position
	//-------------------------------------------------------
	//Set the Right Hand to the orientation of the origin
	RotateObject(GGVR_Player.ObjRightHand, ObjectAngleX(GGVR_Player.ObjOrigin), ObjectAngleY(GGVR_Player.ObjOrigin), ObjectAngleZ(GGVR_Player.ObjOrigin));
	//Set the Right Hand to the position of the origin
	PositionObject(GGVR_Player.ObjRightHand, ObjectPositionX(GGVR_Player.ObjOrigin), ObjectPositionY(GGVR_Player.ObjOrigin), ObjectPositionZ(GGVR_Player.ObjOrigin));
	if (GGVR_RHandIndex != 0)
	{
		//Move the Right Hand on the local X, Y and Z axis of the origin based on the X, Y and Z controller tracking feedback
		MoveObject(GGVR_Player.ObjRightHand, pGamePoseArray_Pos[GGVR_RHandIndex].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjRightHand, pGamePoseArray_Pos[GGVR_RHandIndex].v[0] * GGVR_WorldScale);
		MoveObjectUp(GGVR_Player.ObjRightHand, pGamePoseArray_Pos[GGVR_RHandIndex].v[1] * GGVR_WorldScale);

		//Rotate the Right Hand based on the Turn, Pitch and Roll offsest of the controller tracking feedback
		TurnObjectRight(GGVR_Player.ObjRightHand, pGamePoseArray_YPR[GGVR_RHandIndex].v[1]);
		PitchObjectDown(GGVR_Player.ObjRightHand, pGamePoseArray_YPR[GGVR_RHandIndex].v[0]);
		RollObjectRight(GGVR_Player.ObjRightHand, -pGamePoseArray_YPR[GGVR_RHandIndex].v[2]);
	}

	//Set Left Hand Position
	//-------------------------------------------------------
	//Set the Left Hand to the orientation of the origin
	RotateObject(GGVR_Player.ObjLeftHand, ObjectAngleX(GGVR_Player.ObjOrigin), ObjectAngleY(GGVR_Player.ObjOrigin), ObjectAngleZ(GGVR_Player.ObjOrigin));
	//Set the Left Hand to the position of the origin
	PositionObject(GGVR_Player.ObjLeftHand, ObjectPositionX(GGVR_Player.ObjOrigin), ObjectPositionY(GGVR_Player.ObjOrigin), ObjectPositionZ(GGVR_Player.ObjOrigin));
	if (GGVR_LHandIndex != 0)
	{
		//Move the Left Hand on the local X, Y and Z axis of the origin based on the X, Y and Z controller tracking feedback
		MoveObject(GGVR_Player.ObjLeftHand, pGamePoseArray_Pos[GGVR_LHandIndex].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjLeftHand, pGamePoseArray_Pos[GGVR_LHandIndex].v[0] * GGVR_WorldScale);
		MoveObjectUp(GGVR_Player.ObjLeftHand, pGamePoseArray_Pos[GGVR_LHandIndex].v[1] * GGVR_WorldScale);
		//Rotate the Left Hand based on the Turn, Pitch and Roll offsest of the controller tracking feedback
		TurnObjectRight(GGVR_Player.ObjLeftHand, pGamePoseArray_YPR[GGVR_LHandIndex].v[1]);
		PitchObjectDown(GGVR_Player.ObjLeftHand, pGamePoseArray_YPR[GGVR_LHandIndex].v[0]);
		RollObjectRight(GGVR_Player.ObjLeftHand, -pGamePoseArray_YPR[GGVR_LHandIndex].v[2]);
	}

}

void GGVR_SetHMDPosition(float X, float Y, float Z)
{
	//Set the player base at the specified location
	PositionObject(GGVR_Player.ObjHead, X, Y, Z);

	//Move the base into the proper location based on it's X and Z offsets so the new location is not lost on the next player update
	PositionObject(GGVR_Player.ObjBase, X, Y, Z);
	MoveObjectUp(GGVR_Player.ObjBase, -pGamePoseArray_Pos[GGVR_HMD].v[1] * GGVR_WorldScale);

	//Move the origin into the proper location based on it's X and Z offsets so the new location is not lost on the next player update
	PositionObject(GGVR_Player.ObjOrigin, X, Y, Z);
	MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
	MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
}
//2017-11-02: Added this command to globally set the HMD rotation
void GGVR_SetHMDRotation(float X, float Y, float Z)
{
	//Set the player base at the specified location
	RotateObject(GGVR_Player.ObjOrigin, X, Y, Z);
	TurnObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_YPR[GGVR_HMD].v[1]);
	PitchObjectDown(GGVR_Player.ObjOrigin, -pGamePoseArray_YPR[GGVR_HMD].v[0]);
	RollObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_YPR[GGVR_HMD].v[2]);
}

void GGVR_SetPlayerPosition( float X, float Y, float Z )
{	
	//Set the player base at the specified location
	PositionObject(GGVR_Player.ObjBase, X, Y, Z);

	//Move the origin into the proper location based on it's X and Z offsets so the new location is not lost on the next player update
	PositionObject(GGVR_Player.ObjOrigin, X, Y, Z);
	MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
	MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
}
void GGVR_SetPlayerRotation( float X, float Y, float Z )
{
	RotateObject(GGVR_Player.ObjOrigin, X, Y, Z);
}
void GGVR_SetOriginPosition(float X, float Y, float Z)
{
	//Set the player base at the specified location
	PositionObject(GGVR_Player.ObjOrigin, X, Y, Z);

}
void GGVR_SetOriginRotation(float X, float Y, float Z)
{
	//Set the player base at the specified location
	RotateObject(GGVR_Player.ObjOrigin, X, Y, Z);

}
void GGVR_MovePlayerLocalX( float valx )
{
	if (valx != 0)
	{
		//Move the base object based on it's Z direction
		MoveObjectRight(GGVR_Player.ObjBase, valx);
		//Reposition the origin object
		PositionObject(GGVR_Player.ObjOrigin, ObjectPositionX(GGVR_Player.ObjBase), ObjectPositionY(GGVR_Player.ObjBase), ObjectPositionZ(GGVR_Player.ObjBase));
		MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	}
}
void GGVR_MovePlayerLocalY( float valy )
{
	if (valy != 0)
	{
		//Move the base object based on it's Z direction
		MoveObjectUp(GGVR_Player.ObjBase, valy);
		//Reposition the origin object
		PositionObject(GGVR_Player.ObjOrigin, ObjectPositionX(GGVR_Player.ObjBase), ObjectPositionY(GGVR_Player.ObjBase), ObjectPositionZ(GGVR_Player.ObjBase));
		MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	}
}
void GGVR_MovePlayerLocalZ( float valz )
{
	if (valz != 0)
	{
		//Move the base object based on it's Z direction
		MoveObject(GGVR_Player.ObjBase, valz);
		//Reposition the origin object
		PositionObject(GGVR_Player.ObjOrigin, ObjectPositionX(GGVR_Player.ObjBase), ObjectPositionY(GGVR_Player.ObjBase), ObjectPositionZ(GGVR_Player.ObjBase));
		MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	}
}
void GGVR_RotatePlayerLocalX( float valx )
{
	if (valx != 0)
	{
		//First Rotate the orgin object
		PitchObjectDown(GGVR_Player.ObjOrigin, valx);
		//Position the origin object at the base location, then push it to it's proper location
		PositionObject(GGVR_Player.ObjOrigin, ObjectPositionX(GGVR_Player.ObjBase), ObjectPositionY(GGVR_Player.ObjBase), ObjectPositionZ(GGVR_Player.ObjBase));
		MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	}
}
void GGVR_RotatePlayerLocalY( float valy )
{
	if (valy != 0)
	{
		//First Rotate the orgin object
		TurnObjectRight(GGVR_Player.ObjOrigin, valy);
		//Position the origin object at the base location, then push it to it's proper location
		PositionObject(GGVR_Player.ObjOrigin, ObjectPositionX(GGVR_Player.ObjBase), ObjectPositionY(GGVR_Player.ObjBase), ObjectPositionZ(GGVR_Player.ObjBase));
		MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	}
}
void GGVR_RotatePlayerLocalZ( float valz )
{
	if (valz != 0)
	{
		//First Rotate the orgin object
		RollObjectRight(GGVR_Player.ObjOrigin, valz);
		//Position the origin object at the base location, then push it to it's proper location
		PositionObject(GGVR_Player.ObjOrigin, ObjectPositionX(GGVR_Player.ObjBase), ObjectPositionY(GGVR_Player.ObjBase), ObjectPositionZ(GGVR_Player.ObjBase));
		MoveObject(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale);
		MoveObjectRight(GGVR_Player.ObjOrigin, -pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale);
	}
}

void GGVR_LockPlayerTurn( int Lock )
{
	if (Lock == 1)
	{ GGVR_Player.TurnLock = 1;}
	else { GGVR_Player.TurnLock = 0;}
}
void GGVR_LockPlayerPitch( int Lock )
{
	if (Lock == 1)
	{ GGVR_Player.PitchLock = 1;}
	else { GGVR_Player.PitchLock = 0;}
}

float GGVR_GetPlayerX( )
{
	if (ObjectExist(GGVR_Player.ObjBase) == 1)
	{ 
		return ObjectPositionX(GGVR_Player.ObjBase);
	}
	else 
	{ return 0; }
}
float GGVR_GetPlayerY( )
{
	if (ObjectExist(GGVR_Player.ObjBase) == 1)
	{
		return ObjectPositionY(GGVR_Player.ObjBase);
	}
	else { return 0; }
}
float GGVR_GetPlayerZ( )
{
	if (ObjectExist(GGVR_Player.ObjBase) == 1)
	{
		return ObjectPositionZ(GGVR_Player.ObjBase);
	}
	else { return 0; }
}
float GGVR_GetPlayerAngleX( )
{
	if (ObjectExist(GGVR_Player.ObjBase) == 1)
	{
		return ObjectAngleX(GGVR_Player.ObjBase);
	}
	else { return 0; }
}
float GGVR_GetPlayerAngleY( )
{
	if (ObjectExist(GGVR_Player.ObjBase) == 1)
	{
		return ObjectAngleY(GGVR_Player.ObjBase);
	}
	else { return 0; }
}
float GGVR_GetPlayerAngleZ( )
{
	if (ObjectExist(GGVR_Player.ObjBase) == 1)
	{
		return ObjectAngleZ(GGVR_Player.ObjBase);
	}
	else { return 0; }
}
float GGVR_GetOriginX()
{
	if (ObjectExist(GGVR_Player.ObjOrigin) == 1)
	{
		return ObjectPositionX(GGVR_Player.ObjOrigin);
	}
	else
	{
		return 0;
	}
}
float GGVR_GetOriginY()
{
	if (ObjectExist(GGVR_Player.ObjOrigin) == 1)
	{
		return ObjectPositionY(GGVR_Player.ObjOrigin);
	}
	else { return 0; }
}
float GGVR_GetOriginZ()
{
	if (ObjectExist(GGVR_Player.ObjOrigin) == 1)
	{
		return ObjectPositionZ(GGVR_Player.ObjOrigin);
	}
	else { return 0; }
}

float GGVR_GetHMDX( )
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return ObjectPositionX(GGVR_Player.ObjHead);
	}
	else { return 0; }
}
float GGVR_GetHMDY( )
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return ObjectPositionY(GGVR_Player.ObjHead);
	}
	else { return 0; }
}
float GGVR_GetHMDZ( )
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return ObjectPositionZ(GGVR_Player.ObjHead);
	}
	else { return 0; }
}
float GGVR_GetHMDYaw()
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return pGamePoseArray_YPR[GGVR_HMD].v[1];
	}
	else { return 0; }
}
float GGVR_GetHMDPitch()
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return pGamePoseArray_YPR[GGVR_HMD].v[0];
	}
	else { return 0; }
}
float GGVR_GetHMDRoll()
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return pGamePoseArray_YPR[GGVR_HMD].v[2];
	}
	else { return 0; }
}

float GGVR_GetHMDOffsetX()
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return pGamePoseArray_Pos[GGVR_HMD].v[0] * GGVR_WorldScale;
	}
	else { return 0; }
}
float GGVR_GetHMDOffsetY()
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return pGamePoseArray_Pos[GGVR_HMD].v[1] * GGVR_WorldScale;
	}
	else { return 0; }
}
float GGVR_GetHMDOffsetZ()
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return pGamePoseArray_Pos[GGVR_HMD].v[2] * GGVR_WorldScale;
	}
	else { return 0; }
}
float GGVR_GetHMDAngleX( )
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return ObjectAngleX(GGVR_Player.ObjHead);
	}
	else { return 0; }
}
float GGVR_GetHMDAngleY( )
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return ObjectAngleY(GGVR_Player.ObjHead);
	}
	else { return 0; }
}
float GGVR_GetHMDAngleZ( )
{
	if (ObjectExist(GGVR_Player.ObjHead) == 1)
	{
		return ObjectAngleZ(GGVR_Player.ObjHead);
	}
	else { return 0; }
}
float GGVR_GetRightHandX( )
{
	if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
	{
		return ObjectPositionX(GGVR_Player.ObjRightHand);
	}
	else { return 0; }
}
float GGVR_GetRightHandY( )
{
	if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
	{
		return ObjectPositionY(GGVR_Player.ObjRightHand);
	}
	else { return 0; }
}
float GGVR_GetRightHandZ( )
{
	if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
	{
		return ObjectPositionZ(GGVR_Player.ObjRightHand);
	}
	else { return 0; }
}
float GGVR_GetRightHandAngleX( )
{
	if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
	{
		return ObjectAngleX(GGVR_Player.ObjRightHand);
	}
	else { return 0; }
}
float GGVR_GetRightHandAngleY( )
{
	if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
	{
		return ObjectAngleY(GGVR_Player.ObjRightHand);
	}
	else { return 0; }
}
float GGVR_GetRightHandAngleZ( )
{
	if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
	{
		return ObjectAngleZ(GGVR_Player.ObjRightHand);
	}
	else { return 0; }
}
float GGVR_GetLeftHandX( )
{
	if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
	{
		return ObjectPositionX(GGVR_Player.ObjLeftHand);
	}
	else { return 0; }
}
float GGVR_GetLeftHandY( )
{
	if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
	{
		return ObjectPositionY(GGVR_Player.ObjLeftHand);
	}
	else { return 0; }
}
float GGVR_GetLeftHandZ( )
{
	if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
	{
		return ObjectPositionZ(GGVR_Player.ObjLeftHand);
	}
	else { return 0; }
}
float GGVR_GetLeftHandAngleX( )
{
	if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
	{
		return ObjectAngleX(GGVR_Player.ObjLeftHand);
	}
	else { return 0; }
}
float GGVR_GetLeftHandAngleY( )
{
	if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
	{
		return ObjectAngleY(GGVR_Player.ObjLeftHand);
	}
	else { return 0; }
}
float GGVR_GetLeftHandAngleZ( )
{
	if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
	{
		return ObjectAngleZ(GGVR_Player.ObjLeftHand);
	}
	else { return 0; }
}

//*********************************
//	Update Commands
//*********************************

void GGVR_UpdatePoses(void)
{
	if (GGVR_init_error == vr::VRInitError_None && ivr_compositor != NULL)
	{
		

		//Update the current orientation of the HMD's and Controllers
		GGVR_CompositorError = ivr_compositor->WaitGetPoses(pGamePoseArray, vr::k_unMaxTrackedDeviceCount, pRenderPoseArray, vr::k_unMaxTrackedDeviceCount);
		for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
		{
			if (pGamePoseArray[i].bPoseIsValid == true)
			{

				GGVR_Mat34toYPR(&pGamePoseArray[i].mDeviceToAbsoluteTracking, &pGamePoseArray_YPR[i], &pGamePoseArray_Pos[i]);
			}
			if (pRenderPoseArray[i].bPoseIsValid == true)
			{
				GGVR_Mat34toYPR(&pRenderPoseArray[i].mDeviceToAbsoluteTracking, &pRenderPoseArray_YPR[i], &pRenderPoseArray_Pos[i]);
			}
		}

		//Search for hand controllers if not already found, otherwise update the button presses
		//Right Hand
		if (GGVR_RHandIndex < 1 || GGVR_RHandIndex > vr::k_unMaxTrackedDeviceCount)
		{
			GGVR_RHandIndex = ivr_system->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
			vr::ETrackedPropertyError pError;
			if (GGVR_RHandIndex != 0) // Index for controller was found, so get info about controller
			{

				//Info Strings for the Right Controller - 2017-11-02: Added device info
				//--------------------------------------
				GGVR_RCntrl_TrackingSysName = GetTrackedDeviceString(ivr_system, GGVR_RHandIndex, vr::Prop_TrackingSystemName_String, NULL);
				GGVR_RCntrl_ModelNo = GetTrackedDeviceString(ivr_system, GGVR_RHandIndex, vr::Prop_ModelNumber_String, NULL);
				GGVR_RCntrl_SerialNo = GetTrackedDeviceString(ivr_system, GGVR_RHandIndex, vr::Prop_SerialNumber_String, NULL);
				GGVR_RCntrl_Manufacturer = GetTrackedDeviceString(ivr_system, GGVR_RHandIndex, vr::Prop_ManufacturerName_String, NULL);

				GGVR_AxisType[0][0] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_RHandIndex, vr::Prop_Axis0Type_Int32, &pError);
				GGVR_AxisType[0][1] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_RHandIndex, vr::Prop_Axis1Type_Int32, &pError);
				GGVR_AxisType[0][2] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_RHandIndex, vr::Prop_Axis2Type_Int32, &pError);
				GGVR_AxisType[0][3] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_RHandIndex, vr::Prop_Axis3Type_Int32, &pError);
				GGVR_AxisType[0][4] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_RHandIndex, vr::Prop_Axis4Type_Int32, &pError);

				//Now find specific Axis info:
				for (int i = 0; i < 5; ++i)
				{
					//Look for trigger
					if (GGVR_AxisType[0][i] == vr::k_eControllerAxis_Trigger && GGVR_RTrigger == -1)
					{
						GGVR_RTrigger = i;
					}

					//Look for Joystick
					if (GGVR_AxisType[0][i] == vr::k_eControllerAxis_TrackPad)
					{
						GGVR_RJoystick = i;
					}
					if (GGVR_AxisType[0][i] == vr::k_eControllerAxis_Joystick)
					{
						GGVR_RJoystick = i;
					}
				}
			}
		}
		else
		{ 
			ivr_system->GetControllerState(GGVR_RHandIndex, &GGVR_ControllerState[0], sizeof(vr::VRControllerState001_t));
		}
	
		//Left Hand
		if (GGVR_LHandIndex < 1 || GGVR_LHandIndex > vr::k_unMaxTrackedDeviceCount)
		{
			GGVR_LHandIndex = ivr_system->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
			vr::ETrackedPropertyError pError;
			if (GGVR_LHandIndex != 0) // Index for controller was found, so get info about controller
			{
				//Info Strings for the Right Controller - 2017-11-02: Added device info
				//--------------------------------------
				GGVR_LCntrl_TrackingSysName = GetTrackedDeviceString(ivr_system, GGVR_LHandIndex, vr::Prop_TrackingSystemName_String, NULL);
				GGVR_LCntrl_ModelNo = GetTrackedDeviceString(ivr_system, GGVR_LHandIndex, vr::Prop_ModelNumber_String, NULL);
				GGVR_LCntrl_SerialNo = GetTrackedDeviceString(ivr_system, GGVR_LHandIndex, vr::Prop_SerialNumber_String, NULL);
				GGVR_LCntrl_Manufacturer = GetTrackedDeviceString(ivr_system, GGVR_LHandIndex, vr::Prop_ManufacturerName_String, NULL);

				GGVR_AxisType[1][0] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_LHandIndex, vr::Prop_Axis0Type_Int32, &pError);
				GGVR_AxisType[1][1] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_LHandIndex, vr::Prop_Axis1Type_Int32, &pError);
				GGVR_AxisType[1][2] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_LHandIndex, vr::Prop_Axis2Type_Int32, &pError);
				GGVR_AxisType[1][3] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_LHandIndex, vr::Prop_Axis3Type_Int32, &pError);
				GGVR_AxisType[1][4] = ivr_system->GetInt32TrackedDeviceProperty(GGVR_LHandIndex, vr::Prop_Axis4Type_Int32, &pError);

				//Now find specific Axis info:
				for (int i = 0; i < 5; ++i)
				{
					//Look for trigger
					if (GGVR_AxisType[1][i] == vr::k_eControllerAxis_Trigger && GGVR_LTrigger == -1)
					{
						GGVR_LTrigger = i;
					}

					//Look for Joystick
					if (GGVR_AxisType[1][i] == vr::k_eControllerAxis_TrackPad)
					{
						GGVR_LJoystick = i;
					}
					if (GGVR_AxisType[1][i] == vr::k_eControllerAxis_Joystick)
					{
						GGVR_LJoystick = i;
					}
				}
			}
		}
		else
		{
			ivr_system->GetControllerState(GGVR_LHandIndex, &GGVR_ControllerState[1], sizeof(vr::VRControllerState001_t));
		}
	}
}

//*********************************
//	Controller Commands
//*********************************

int GGVR_RightControllerFound( )
{
	if (GGVR_RHandIndex < 1 || GGVR_RHandIndex > vr::k_unMaxTrackedDeviceCount)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
int GGVR_LeftControllerFound( )
{
	if (GGVR_LHandIndex < 1 || GGVR_LHandIndex > vr::k_unMaxTrackedDeviceCount)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
float GGVR_RightController_JoyX( void )
{
	return GGVR_ControllerState[0].rAxis[GGVR_RJoystick].x;
}
float GGVR_RightController_JoyY( void )
{
	return GGVR_ControllerState[0].rAxis[GGVR_RJoystick].y;
}
float GGVR_LeftController_JoyX( void )
{
	return GGVR_ControllerState[1].rAxis[GGVR_LJoystick].x;
}
float GGVR_LeftController_JoyY( void )
{
	return GGVR_ControllerState[1].rAxis[GGVR_LJoystick].y;
}
float GGVR_RightController_Trigger( void )
{
	return GGVR_ControllerState[0].rAxis[GGVR_RTrigger].x;
}
float GGVR_LeftController_Trigger( void )
{
	return GGVR_ControllerState[1].rAxis[GGVR_LTrigger].x;
}
int GGVR_RightController_Grip(void)
{
	if (GGVR_ControllerState[0].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Grip))
	{
		return 1;
	}
	return 0;
}
int GGVR_LeftController_Grip(void)
{
	if (GGVR_ControllerState[1].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Grip))
	{
		return 1;
	}
	return 0;
}
int GGVR_RightController_Button1(void)
{
	if (GGVR_ControllerState[0].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu))
	{
		return 1;
	}
	return 0;
}
int GGVR_LeftController_Button1(void)
{
	if (GGVR_ControllerState[1].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu))
	{
		return 1;
	}
	return 0;
}
int GGVR_RightController_Button2(void)
{
	if (GGVR_ControllerState[0].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Axis0) || GGVR_ControllerState[0].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_A))
	{
		return 1;
	}
	return 0;
}
int GGVR_LeftController_Button2(void)
{
	if (GGVR_ControllerState[1].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Axis0) || GGVR_ControllerState[1].ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_A))
	{
		return 1;
	}
	return 0;
}


//2017-11-02: Command so other buttons can be detected
int GGVR_RightController_ButtonPressed(int Button)
{
	if (Button < vr::k_EButton_Max)
	{
		if (GGVR_ControllerState[0].ulButtonPressed & vr::ButtonMaskFromId((vr::EVRButtonId)Button))
		{
			return 1;
		}
	}
	return 0;
}
//2017-11-02: Command so other buttons can be detected
int GGVR_LeftController_ButtonPressed(int Button)
{
	if (Button < vr::k_EButton_Max)
	{
		if (GGVR_ControllerState[1].ulButtonPressed & vr::ButtonMaskFromId((vr::EVRButtonId)Button))
		{
			return 1;
		}
	}
	return 0;
}

void GGVR_LeftController_TriggerPulse( int axis, int duration_ms)
{
	if (GGVR_init_error == vr::VRInitError_None && ivr_compositor != NULL)
	{
		//uint32_t axis = 0;
		ivr_system->TriggerHapticPulse(GGVR_LHandIndex, axis, duration_ms);
	}
}
void GGVR_RightController_TriggerPulse( int axis, int duration_ms)
{
	if (GGVR_init_error == vr::VRInitError_None && ivr_compositor != NULL)
	{
		//uint32_t axis = 0;
		ivr_system->TriggerHapticPulse(GGVR_RHandIndex, axis, duration_ms);
	}
}

int GGVR_RightController_GetButtonPressed( int button )
{
	if (GGVR_ControllerState[0].ulButtonPressed & vr::ButtonMaskFromId((vr::EVRButtonId)button))
	{
		return 1;
	}
	return 0;
}
int GGVR_LeftController_GetButtonPressed( int button )
{
	if (GGVR_ControllerState[1].ulButtonPressed & vr::ButtonMaskFromId((vr::EVRButtonId)button))
	{
		return 1;
	}
	return 0;
}
int GGVR_RightController_GetButtonTouched( int button )
{
	if (GGVR_ControllerState[0].ulButtonTouched & vr::ButtonMaskFromId((vr::EVRButtonId)button))
	{
		return 1;
	}
	return 0;
}
int GGVR_LeftController_GetButtonTouched( int button )
{
	if (GGVR_ControllerState[1].ulButtonTouched & vr::ButtonMaskFromId((vr::EVRButtonId)button))
	{
		return 1;
	}
	return 0;
}

int GGVR_RightController_GetFingerPointed()
{
	if (GGVR_ControllerState[0].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_Axis1))
	{
		return 0;
	}
	return 1;
}
int GGVR_LeftController_GetFingerPointed()
{
	if (GGVR_ControllerState[1].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_Axis1))
	{
		return 0;
	}
	return 1;
}
int GGVR_RightController_GetThumbUp()
{
	if (GGVR_ControllerState[0].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_Axis0) || GGVR_ControllerState[0].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_A) || GGVR_ControllerState[0].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu))
	{
		return 0;
	}
	return 1;
}
int GGVR_LeftController_GetThumbUp()
{
	if (GGVR_ControllerState[1].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_Axis0) || GGVR_ControllerState[1].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_A) || GGVR_ControllerState[1].ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu))
	{
		return 0;
	}
	return 1;
}
//2017-11-02: Command so grip triggers can be accesses on Oculus Touch controllers
float GGVR_RightController_AxisTriggerX(int axis)
{
	if (axis >= 0 && axis < vr::k_unControllerStateAxisCount)
	{
		return GGVR_ControllerState[0].rAxis[axis].x;
	}
	return 0.0f;
}
//2017-11-02: Command so grip triggers can be accesses on Oculus Touch controllers
float GGVR_RightController_AxisTriggerY(int axis)
{
	if (axis >= 0 && axis < vr::k_unControllerStateAxisCount)
	{
		return GGVR_ControllerState[0].rAxis[axis].y;
	}
	return 0.0f;
}
//2017-11-02: Command so grip triggers can be accesses on Oculus Touch controllers
float GGVR_LeftController_AxisTriggerX(int axis)
{
	if (axis >= 0 && axis < vr::k_unControllerStateAxisCount)
	{
		return GGVR_ControllerState[1].rAxis[axis].x;
	}
	return 0.0f;
}
//2017-11-02: Command so grip triggers can be accesses on Oculus Touch controllers
float GGVR_LeftController_AxisTriggerY(int axis)
{
	if (axis >= 0 && axis < vr::k_unControllerStateAxisCount)
	{
		return GGVR_ControllerState[1].rAxis[axis].y;
	}
	return 0.0f;
}
//*********************************
//	Player Class
//*********************************
void GGVR_PlayerData::Create( )
{
	//ObjBase
	MakeObjectCone(GGVR_Player.ObjBase, 0.1f*GGVR_WorldScale);
	RotateObject(GGVR_Player.ObjBase, 90.0, 0.0, 0.0);
	FixObjectPivot(GGVR_Player.ObjBase);
	SetObjectCollisionOff(GGVR_Player.ObjBase);
	//HideObject(GGVR_Player.ObjBase);

	//ObjHead
	MakeObjectCone(GGVR_Player.ObjHead, 0.1f*GGVR_WorldScale);
	RotateObject(GGVR_Player.ObjHead, 90.0, 0.0, 0.0);
	FixObjectPivot(GGVR_Player.ObjHead);
	SetObjectCollisionOff(GGVR_Player.ObjHead);
	//HideObject(GGVR_Player.ObjHead);

	//ObjOrigin
	MakeObjectCone(GGVR_Player.ObjOrigin, 0.1f*GGVR_WorldScale);
	RotateObject(GGVR_Player.ObjOrigin, 90.0, 0.0, 0.0);
	FixObjectPivot(GGVR_Player.ObjOrigin);
	SetObjectCollisionOff(GGVR_Player.ObjOrigin);
	//HideObject(GGVR_Player.ObjOrigin);

	//ObjRightHand
	MakeObjectCone(GGVR_Player.ObjRightHand, 0.1f*GGVR_WorldScale);
	RotateObject(GGVR_Player.ObjRightHand, 90.0, 0.0, 0.0);
	FixObjectPivot(GGVR_Player.ObjRightHand);
	SetObjectCollisionOff(GGVR_Player.ObjRightHand);
	//HideObject(GGVR_Player.ObjRightHand);

	//ObjLeftHand
	MakeObjectCone(GGVR_Player.ObjLeftHand, 0.1f*GGVR_WorldScale);
	RotateObject(GGVR_Player.ObjLeftHand, 90.0, 0.0, 0.0);
	FixObjectPivot(GGVR_Player.ObjLeftHand);
	SetObjectCollisionOff(GGVR_Player.ObjLeftHand);
	//HideObject(GGVR_Player.ObjLeftHand);
	PitchLock = 0;
	TurnLock = 1;
}
void GGVR_PlayerData::Destroy( )
{
	// Delete playerdata object resources
	if ( ObjectExist(GGVR_Player.ObjBase)==1 ) DeleteObject ( GGVR_Player.ObjBase );
	if ( ObjectExist(GGVR_Player.ObjHead)==1 ) DeleteObject ( GGVR_Player.ObjHead );
	if ( ObjectExist(GGVR_Player.ObjOrigin)==1 ) DeleteObject ( GGVR_Player.ObjOrigin );
	if ( ObjectExist(GGVR_Player.ObjRightHand)==1 ) DeleteObject ( GGVR_Player.ObjRightHand );
	if ( ObjectExist(GGVR_Player.ObjLeftHand)==1 ) DeleteObject ( GGVR_Player.ObjLeftHand );
}
int GGVR_PlayerData::GetOriginObjID()
{
	return ObjOrigin;
}
int GGVR_PlayerData::GetHeadObjID()
{
	return ObjHead;
}
int GGVR_PlayerData::GetBaseObjID()
{
	return ObjBase;
}
int GGVR_PlayerData::GetRHandObjID()
{
	return ObjRightHand;
}
int GGVR_PlayerData::GetLHandObjID()
{
	return ObjLeftHand;
}


//*********************************
//	Chaperone Commands
//*********************************
//2017-11-02: Added chaperone info commands
float GGVR_Chaperone_GetPlayAreaSizeX()
{
	float SizeX;
	float SizeZ;
	Chaperone->GetPlayAreaSize(&SizeX, &SizeZ);
	return SizeX * GGVR_WorldScale;
}
float GGVR_Chaperone_GetPlayAreaSizeZ()
{
	float SizeX;
	float SizeZ;
	Chaperone->GetPlayAreaSize(&SizeX, &SizeZ);
	return SizeZ * GGVR_WorldScale;
}
int GGVR_Chaperone_AreBoundsVisible()
{
	return Chaperone->AreBoundsVisible();
}
void GGVR_Chaperone_ForceBoundsVisible(int flag)
{
	if (flag == 0)
	{
		Chaperone->ForceBoundsVisible(false);
	}
	else
	{
		Chaperone->ForceBoundsVisible(true);
	}
}
float GGVR_Chaperone_GetPlayAreaPoint1X()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[0].v[0];
}
float GGVR_Chaperone_GetPlayAreaPoint1Y()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[0].v[1];
}
float GGVR_Chaperone_GetPlayAreaPoint1Z()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[0].v[2];
}
float GGVR_Chaperone_GetPlayAreaPoint2X()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[1].v[0];
}
float GGVR_Chaperone_GetPlayAreaPoint2Y()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[1].v[1];
}
float GGVR_Chaperone_GetPlayAreaPoint2Z()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[1].v[2];
}
float GGVR_Chaperone_GetPlayAreaPoint3X()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[2].v[0];
}
float GGVR_Chaperone_GetPlayAreaPoint3Y()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[2].v[1];
}
float GGVR_Chaperone_GetPlayAreaPoint3Z()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[2].v[2];
}
float GGVR_Chaperone_GetPlayAreaPoint4X()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[3].v[0];
}
float GGVR_Chaperone_GetPlayAreaPoint4Y()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[3].v[1];
}
float GGVR_Chaperone_GetPlayAreaPoint4Z()
{
	vr::HmdQuad_t Rect;
	Chaperone->GetPlayAreaRect(&Rect);
	return Rect.vCorners[3].v[2];
}

//*********************************
//	Device Info Commands
//*********************************
//2017-11-02: Added device info commands
char* GGVR_HMD_GetSerialNumber()
{
	char *cstr = new char[GGVR_HMD_SerialNo.length() + 1];
	strcpy(cstr, GGVR_HMD_SerialNo.c_str());
	return cstr;
}
char* GGVR_HMD_GetTrackingSysName()
{
	char *cstr = new char[GGVR_HMD_TrackingSysName.length() + 1];
	strcpy(cstr, GGVR_HMD_TrackingSysName.c_str());
	return cstr;
}
char* GGVR_HMD_GetModelNumber()
{
	char *cstr = new char[GGVR_HMD_ModelNo.length() + 1];
	strcpy(cstr, GGVR_HMD_ModelNo.c_str());
	return cstr;
}
char* GGVR_HMD_GetManufacturer()
{
	char *cstr = new char[GGVR_HMD_Manufacturer.length() + 1];
	strcpy(cstr, GGVR_HMD_Manufacturer.c_str());
	return cstr;
}
char* GGVR_RCntrl_GetSerialNumber()
{
	char *cstr = new char[GGVR_RCntrl_SerialNo.length() + 1];
	strcpy(cstr, GGVR_RCntrl_SerialNo.c_str());
	return cstr;
}
char* GGVR_RCntrl_GetTrackingSysName()
{
	char *cstr = new char[GGVR_RCntrl_TrackingSysName.length() + 1];
	strcpy(cstr, GGVR_RCntrl_TrackingSysName.c_str());
	return cstr;
}
char* GGVR_RCntrl_GetModelNumber()
{
	char *cstr = new char[GGVR_RCntrl_ModelNo.length() + 1];
	strcpy(cstr, GGVR_RCntrl_ModelNo.c_str());
	return cstr;
}
char* GGVR_RCntrl_GetManufacturer()
{
	char *cstr = new char[GGVR_RCntrl_Manufacturer.length() + 1];
	strcpy(cstr, GGVR_RCntrl_Manufacturer.c_str());
	return cstr;
}
char* GGVR_LCntrl_GetSerialNumber()
{
	char *cstr = new char[GGVR_LCntrl_SerialNo.length() + 1];
	strcpy(cstr, GGVR_LCntrl_SerialNo.c_str());
	return cstr;
}
char* GGVR_LCntrl_GetTrackingSysName()
{
	char *cstr = new char[GGVR_LCntrl_TrackingSysName.length() + 1];
	strcpy(cstr, GGVR_LCntrl_TrackingSysName.c_str());
	return cstr;
}
char* GGVR_LCntrl_GetModelNumber()
{
	char *cstr = new char[GGVR_LCntrl_ModelNo.length() + 1];
	strcpy(cstr, GGVR_LCntrl_ModelNo.c_str());
	return cstr;
}
char* GGVR_LCntrl_GetManufacturer()
{
	char *cstr = new char[GGVR_LCntrl_Manufacturer.length() + 1];
	strcpy(cstr, GGVR_LCntrl_Manufacturer.c_str());
	return cstr;
}

//--------------------------------------------------------------------------------------------------
// String Helper Function
//--------------------------------------------------------------------------------------------------

std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

//--------------------------------------------------------------------------------------------------
// Rotational Conversions: Convert an OpenVR matrix into Yaw, Pitch, Roll values
//--------------------------------------------------------------------------------------------------

void GGVR_Mat34toYPR(vr::HmdMatrix34_t *Mat, vr::HmdVector3_t *YPR, vr::HmdVector3_t *Pos)
{
	vr::HmdVector3_t r;
	vr::HmdVector3_t f;
	vr::HmdVector3_t u;

	r.v[0] = Mat->m[0][0]; r.v[1] = -Mat->m[2][0]; r.v[2] = Mat->m[1][0];
	f.v[0] = -Mat->m[0][2]; f.v[1] = Mat->m[2][2]; f.v[2] = -Mat->m[1][2];
	u.v[0] = Mat->m[0][1]; u.v[1] = -Mat->m[2][1]; u.v[2] = Mat->m[1][1];


	if (f.v[2] < -1.0f + GGVR_SINGULARITYRADIUS) // down
	{
		YPR->v[1] = 0.0f;
		YPR->v[0] = -GGVR_PIOVER2;
		YPR->v[2] = atan2f(u.v[0], u.v[1]);
	}
	else if (f.v[2] > 1.0f - GGVR_SINGULARITYRADIUS) // up
	{
		YPR->v[1] = 0.0f;
		YPR->v[0] = GGVR_PIOVER2;
		YPR->v[2] = GGVR_PI - atan2f(u.v[0], u.v[1]);
	}
	else // usual case
	{
		YPR->v[1] = atan2f(-f.v[0], f.v[1]);
		YPR->v[0] = asinf(f.v[2]);
		YPR->v[2] = -atan2f(r.v[2], u.v[2]);
	}

	YPR->v[1] = -YPR->v[1] * GGVR_180DIVPI;
	YPR->v[0] = -YPR->v[0] * GGVR_180DIVPI;
	YPR->v[2] = -YPR->v[2] * GGVR_180DIVPI;
	Pos->v[0] = Mat->m[0][3];
	Pos->v[1] = Mat->m[1][3];
	Pos->v[2] = -Mat->m[2][3];
}





