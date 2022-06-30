//
// GGVR - VR System Handling
//

// Common Includes
#include "globstruct.h"
#include "GGVR.h"
#include "stdafx.h"
#include <string>
#include "openvr.h"
#include "CCameraC.h"
#include "CImageC.h"
#include "CObjectsC.h"
#include "dxdiag.h"
#include "D3dx9math.h"
#include "BlitzTerrain.h"
#include "CGfxC.h"
#include "CSystemC.h"

#include "CFileC.h"

#include "openxr.h"

#include <iostream>
#include <exception>
using namespace std;

// Externs
extern GlobStruct*				g_pGlob;
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;
extern LPGGSURFACE				g_pBackBuffer;
extern LPGGRENDERTARGETVIEW		m_pRenderTargetView;

// Constants
const float GGVR_SINGULARITYRADIUS = 0.0000001f;
const float GGVR_180DIVPI = 57.2957795130823f;
const float GGVR_PI = 3.14159265358979f;
const float GGVR_PIOVER2 = 0.5f * GGVR_PI;
const float GGVR_radian = 0.0174533f;
const int GGVR_LEye = 0;
const int GGVR_REye = 1; 
const int GGVR_LEye_Depth = 2;
const int GGVR_REye_Depth = 3;

// Reccomended Texture Sizes
uint32_t						GGVR_EyeW;
uint32_t						GGVR_EyeH;

// Pointers to Render Images
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
bool							GGVR_bLeftIsBest = false;

// Raw Projection Matrix Values and camera clipping ranges
float							GGVR_NearClip = 1.0f;
float							GGVR_FarClip = 300000.0f;

#define GGVR_TELEPORT_INACTIVE	0
#define GGVR_TELEPORT_ACTIVE	1
#define GGVR_TELEPORT_FINISHED	2
#define GGVR_TELEPORT_WAITFORNOCLICKRELEASE 3

#define GGVR_TELEPORT_HAND_LEFT  0
#define GGVR_TELEPORT_HAND_RIGHT 1

// Controller Button Presses
int								GGVR_bPrevTurnStateLeft = 0;
int								GGVR_bPrevTurnStateRight = 0;
int								GGVR_TeleportHand = GGVR_TELEPORT_HAND_LEFT;
int								GGVR_TeleportState = GGVR_TELEPORT_INACTIVE;
bool                            GGVR_bTeleportInvalid = false;
float							GGVR_fTelePortDestinationX = 0.0f;
float							GGVR_fTelePortDestinationY = 0.0f;
float							GGVR_fTelePortDestinationZ = 0.0f;
bool							GGVR_RCntrlBut_Press[64];
bool							GGVR_RCntrlBut_Touch[64];
bool							GGVR_LCntrlBut_Press[64];
bool							GGVR_LCntrlBut_Touch[64];

// Camera Reference
float							GGVR_WorldScale = 39.3700787f;

GGVR_PlayerData					GGVR_Player;

// VR System
int								GGVR_EnabledMode = 0;
int								GGVR_EnabledState = 0;
int								GGVR_TrackingSpace = 0;

// OpenXR variables
#define PI 3.14159265f
float player_pos_x, player_pos_y, player_pos_z;
float player_rot_x, player_rot_y, player_rot_z; // Euler angles
float generic_offset_ang_x = 0;
float wmr_offset_ang_x = 0;
KMaths::Matrix matPlayer;
float left_eye_pos_x, left_eye_pos_y, left_eye_pos_z;
float right_eye_pos_x, right_eye_pos_y, right_eye_pos_z;
float hmd_rot_x, hmd_rot_y, hmd_rot_z;
float hmd_normal_x, hmd_normal_y, hmd_normal_z;
int eye_width, eye_height, curr_side;
ID3D11Texture2D* depthTexture;
ID3D11DepthStencilView* depthView;
int left_controller_type = OPENXR_CONTROLLER_UNKNOWN;
int right_controller_type = OPENXR_CONTROLLER_UNKNOWN;
char ggvr_rootdir[ MAX_PATH ];

// reuse these data structures
vr::TrackedDevicePose_t			pGamePoseArray[vr::k_unMaxTrackedDeviceCount];
vr::HmdVector3_t				pGamePoseArray_YPR[vr::k_unMaxTrackedDeviceCount];
vr::HmdVector3_t				pGamePoseArray_Pos[vr::k_unMaxTrackedDeviceCount];
vr::VRControllerState001_t		GGVR_ControllerState[2];

// DLL Entry (no DLL though!)
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

// Debugging

int g_iDebuggingActive = 0;
char* g_pGGVRDebugLog[10000];
char g_pDebugAbsPath[1024];
extern void timestampactivity ( int i, char* desc_s );

int DebugGGVRlog ( const char* pReportLog )
{
	// dont use this for now - is it crashing istefl?
	return 0;

	// Log File
	if ( pReportLog == NULL )
	{
		// Reset at VERY start
		for ( int iFind = 0; iFind < 10000; iFind++ ) g_pGGVRDebugLog[iFind] = NULL; 
		strcpy ( g_pDebugAbsPath, "" );
		return 0;
	}
	if ( g_iDebuggingActive == 1 )
	{
		// the debug file
		const char* pFilename = "GGVRDebugLog.log";

		// First use of debug, get absolute path to log file
		if ( strcmp ( g_pDebugAbsPath, "" ) == NULL )
		{
			GetCurrentDirectoryA ( 1024, g_pDebugAbsPath );
		}

		// New entry 
		char pWithTime[2048];
		sprintf_s ( pWithTime, "%d : %s\r\n", timeGetTime(), pReportLog );
		DWORD dwNewEntry = strlen(pWithTime);

		// Find new slot
		for ( int iFind = 0; iFind < 10000; iFind++ )
		{
			if ( g_pGGVRDebugLog[iFind] == NULL ) 
			{
				g_pGGVRDebugLog[iFind] = new char[2048];
				memset ( g_pGGVRDebugLog[iFind], 0, 2048 );
				strcpy_s ( g_pGGVRDebugLog[iFind], 2048, pWithTime );
				break;
			}
		}

		// save new log file
		DWORD bytesdone = 0;
		char pAbsFilename[1024];
		strcpy ( pAbsFilename, g_pDebugAbsPath );
		strcat ( pAbsFilename, "\\" );
		strcat ( pAbsFilename, pFilename );
		HANDLE hFile = GG_CreateFile(pAbsFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			for ( int iFind = 0; iFind < 10000; iFind++ )
			{
				if ( g_pGGVRDebugLog[iFind] != NULL ) 
				{
					WriteFile(hFile, g_pGGVRDebugLog[iFind], strlen(g_pGGVRDebugLog[iFind]), &bytesdone, FALSE);
				}
			}
			CloseHandle ( hFile );
		}
		else
		{
			// error - cannot get write lock on this file!
			return 1;
		}
	}

	// success
	return 0;
}

// Support Functions

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

void GGVR_MatrixToEuler( KMaths::Matrix* mat, float* x, float* y, float* z )
{
	float ax, ay, az;
	if ( mat->m[0][2] > 0.99999f )
	{
		ax = 0;
		ay = PI / 2.0f;
		az = 0;
	}
	else if ( mat->m[0][2] < -0.99999f )
	{
		ax = 0;
		ay = -PI / 2.0f;
		az = 0;
	}
	else
	{
		ax = atan2f( mat->m[1][2], mat->m[2][2] );
		ay = asin( -mat->m[0][2] );
		az = atan2f( mat->m[0][1], mat->m[0][0] );
	}

	if ( x ) *x = GGToDegree(ax);
	if ( y ) *y = GGToDegree(ay);
	if ( z ) *z = GGToDegree(az);
}

// Generic
int GGVR_ChooseVRSystem ( int iGGVREnabledMode, int iDebuggingActive, LPSTR pAbsPathToDLL )
{
	// Debug flag can only be zero or one
	if ( iDebuggingActive > 1 ) iDebuggingActive = 1;

	// Assign VR System Mode to Use
	g_iDebuggingActive = iDebuggingActive;
	//DebugGGVRlog ( NULL );

	if ( !OpenXRIsInitialised() ) 
	{
		if ( OpenXRInit( m_pD3D ) < 0 ) return 1;
	}
	
	// no error code
	return 0;
}

int	GGVR_IsHmdPresent()
{
	// Return OpenXR (and be inclusion WMR) as Type 2
	return OpenXRIsSessionActive() ? 2 : 0;
}

int	GGVR_IsRuntimeFound()
{
	return OpenXRIsInitialised() ? 1 : 0;
}

void GGVR_SetTrackingSpace( int space )
{
	// 0 = seated
	// 1 = standing
	GGVR_TrackingSpace = space;
}

int GGVR_GetTrackingSpace ( void ) 
{
	return GGVR_TrackingSpace;
}

int	GGVR_Init ( LPSTR pRootDir, int RImageID, int LImageID, int RCamID, int LCamID, int ObjBase, int ObjHead, int ObjOrigin, int ObjRightHand, int ObjLeftHand, int iObjTeleportStart, int iObjTeleportFinish, int iShaderID, int iTextureID, int iCShaderID, int iCTextureID0, int iCTextureID1, int iCTextureID2, int iCTextureID3, int iCTextureID4, int iCTextureID5, int iCTextureID6, int oculusTex0)
{
	strcpy( ggvr_rootdir, pRootDir );

	// Set Object ID's for Player objects
	GGVR_Player.ObjRightHand = ObjRightHand;
	GGVR_Player.ObjLeftHand = ObjLeftHand;
	GGVR_Player.ObjTeleportStart = iObjTeleportStart;
	GGVR_Player.ObjTeleportFinish = iObjTeleportFinish;

	GGVR_Player.ShaderID = iShaderID;//g.guishadereffectindex
	GGVR_Player.TextureID = iTextureID;//g.editorimagesoffset+14

	GGVR_Player.ControllerShaderID = iCShaderID;
	GGVR_Player.ControllerTextureID0 = iCTextureID0;
	GGVR_Player.ControllerTextureID1 = iCTextureID1;
	GGVR_Player.ControllerTextureID2 = iCTextureID2;
	GGVR_Player.ControllerTextureID3 = iCTextureID3;
	GGVR_Player.ControllerTextureID4 = iCTextureID4;
	GGVR_Player.ControllerTextureID5 = iCTextureID5;
	GGVR_Player.ControllerTextureID6 = iCTextureID6;
	GGVR_Player.OculusTex0 = oculusTex0;

	// this needs to be zero if regular mouse control in script is to work (i.e title page)
	GGVR_Player.LaserGuideActive = 0;

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

	GGVR_Player.Create(pRootDir);

	if ( !OpenXRIsInitialised() ) 
	{
		if ( OpenXRInit( m_pD3D ) < 0 ) return -1;
	}

	// default is 100.0f but can be changed if VRweapon used
	GGVR_SetLaserForwardDistance(100.0f);

	return 0;
}

void GGVR_ReInit()
{
	if ( !OpenXRIsInitialised() ) 
	{
		OpenXRInit( m_pD3D );
	}
}

void GGVR_Shutdown()
{
	// this needs to be zero if regular mouse control in script is to work (i.e title page)
	GGVR_Player.LaserGuideActive = 0;

	// Free any playerdata object resources
	GGVR_Player.Destroy();

	// Free camera images associated with VR rendering
	SetCameraToImage(GGVR_RCamID, -1, 0, 0);
	SetCameraToImage(GGVR_LCamID, -1, 0, 0);

	OpenXREndSession();
}

void GGVR_SetWorldScale( float scale )
{
	GGVR_WorldScale = scale;
}

float GGVR_GetWorldScale(  )
{
	return GGVR_WorldScale;
}

int GGVR_GetPlayerRightHandObjID( )
{
	return GGVR_Player.GetRHandObjID( );
}

int GGVR_GetPlayerLeftHandObjID( )
{
	return GGVR_Player.GetLHandObjID( );
}
														
void GGVR_UpdatePlayer ( bool bPlayerDucking, int iTerrainID, int iLMObjStart, int iLMObjFinish, int iEntObjStart, int iEndObjEnd, int iBatchEntObjStart, int iBatchEndObjEnd )
{
	// Update the HMD and controller feedbacks
	timestampactivity ( 0, "TS: Calling OpenXRHandleEvents" );

	// check and update controller models
	int new_type = OpenXRGetLeftHandControllerType();
	if ( left_controller_type != new_type )
	{
		DeleteObject( GGVR_Player.ObjLeftHand );

		switch( new_type )
		{
			case OPENXR_CONTROLLER_OCULUS_TOUCH:
			{
				char pControllerModel[1024];
				strcpy ( pControllerModel, ggvr_rootdir );
				strcat ( pControllerModel, "\\gamecore\\vrcontroller\\oculus\\controller_l.dbo" );
				LoadObject ( pControllerModel, GGVR_Player.ObjLeftHand );
				ScaleObject ( GGVR_Player.ObjLeftHand, 40.0f, 40.0f, 40.0f );
				TextureObject ( GGVR_Player.ObjLeftHand, 0, GGVR_Player.OculusTex0 );
			} break;

			default:
			{
				char pControllerModel[1024];
				strcpy ( pControllerModel, ggvr_rootdir );
				strcat ( pControllerModel, "\\gamecore\\vrcontroller\\vrcontrollerleft.dbo" );
				LoadObject ( pControllerModel, GGVR_Player.ObjLeftHand );
				ScaleObject ( GGVR_Player.ObjLeftHand, 100.0f, 100.0f, 100.0f );
				TextureObject ( GGVR_Player.ObjLeftHand, 0, GGVR_Player.ControllerTextureID0 );
			}
		}
		left_controller_type = new_type;

		SetObjectCollisionOff( GGVR_Player.ObjLeftHand );
		SetSphereRadius ( GGVR_Player.ObjLeftHand, 0 );
		SetObjectMask ( GGVR_Player.ObjLeftHand, (1<<6) + (1<<7) + 1 );
		TextureObject ( GGVR_Player.ObjLeftHand, 7, GGVR_Player.ControllerTextureID1 );
		TextureObject ( GGVR_Player.ObjLeftHand, 1, GGVR_Player.ControllerTextureID1 );
		TextureObject ( GGVR_Player.ObjLeftHand, 2, GGVR_Player.ControllerTextureID2 );
		TextureObject ( GGVR_Player.ObjLeftHand, 3, GGVR_Player.ControllerTextureID3 );
		TextureObject ( GGVR_Player.ObjLeftHand, 4, GGVR_Player.ControllerTextureID4 );
		TextureObject ( GGVR_Player.ObjLeftHand, 5, GGVR_Player.ControllerTextureID5 );
		TextureObject ( GGVR_Player.ObjLeftHand, 6, GGVR_Player.ControllerTextureID6 );
		SetObjectEffect ( GGVR_Player.ObjLeftHand, GGVR_Player.ControllerShaderID );
		DisableObjectZDepth(GGVR_Player.ObjLeftHand);
	}

	new_type = OpenXRGetRightHandControllerType();
	if ( right_controller_type != new_type )
	{
		DeleteObject( GGVR_Player.ObjRightHand );

		switch( new_type )
		{
			case OPENXR_CONTROLLER_OCULUS_TOUCH:
			{
				char pControllerModel[1024];
				strcpy ( pControllerModel, ggvr_rootdir );
				strcat ( pControllerModel, "\\gamecore\\vrcontroller\\oculus\\controller_r.dbo" );
				LoadObject ( pControllerModel, GGVR_Player.ObjRightHand );
				ScaleObject ( GGVR_Player.ObjRightHand, 40.0f, 40.0f, 40.0f );
				TextureObject ( GGVR_Player.ObjRightHand, 0, GGVR_Player.OculusTex0 );
			} break;

			default:
			{
				char pControllerModel[1024];
				strcpy ( pControllerModel, ggvr_rootdir );
				strcat ( pControllerModel, "\\gamecore\\vrcontroller\\vrcontrollerright.dbo" );
				LoadObject ( pControllerModel, GGVR_Player.ObjRightHand );
				ScaleObject ( GGVR_Player.ObjRightHand, 100.0f, 100.0f, 100.0f );
				TextureObject ( GGVR_Player.ObjRightHand, 0, GGVR_Player.ControllerTextureID0 );
			}
		}
		right_controller_type = new_type;

		SetObjectCollisionOff( GGVR_Player.ObjRightHand );
		SetSphereRadius ( GGVR_Player.ObjRightHand, 0 );
		SetObjectMask ( GGVR_Player.ObjRightHand, (1<<6) + (1<<7) + 1 );
		TextureObject ( GGVR_Player.ObjRightHand, 7, GGVR_Player.ControllerTextureID1 );
		TextureObject ( GGVR_Player.ObjRightHand, 1, GGVR_Player.ControllerTextureID1 );
		TextureObject ( GGVR_Player.ObjRightHand, 2, GGVR_Player.ControllerTextureID2 );
		TextureObject ( GGVR_Player.ObjRightHand, 3, GGVR_Player.ControllerTextureID3 );
		TextureObject ( GGVR_Player.ObjRightHand, 4, GGVR_Player.ControllerTextureID4 );
		TextureObject ( GGVR_Player.ObjRightHand, 5, GGVR_Player.ControllerTextureID5 );
		TextureObject ( GGVR_Player.ObjRightHand, 6, GGVR_Player.ControllerTextureID6 );
		SetObjectEffect ( GGVR_Player.ObjRightHand, GGVR_Player.ControllerShaderID );
		DisableObjectZDepth(GGVR_Player.ObjRightHand);
	}

	// show controller if data coming through
	if (OpenXRGetLeftHandActive())
	{
		if ( GGVR_Player.bMakeLeftHandInvisible == false )
			ShowObject ( GGVR_Player.ObjLeftHand );
		else
			HideObject ( GGVR_Player.ObjLeftHand );
	}
	else
		HideObject ( GGVR_Player.ObjLeftHand );

	if (OpenXRGetRightHandActive())
	{
		if ( GGVR_Player.bMakeRightHandInvisible == false )
			ShowObject ( GGVR_Player.ObjRightHand );
		else
			HideObject ( GGVR_Player.ObjRightHand );
	}
	else
		HideObject ( GGVR_Player.ObjRightHand );

	// prefer right hand, but allow left if touch pad touched (for motion controller action)
	// this also keeps the laser right on the right side where the gun is
	float x = ObjectPositionX ( GGVR_Player.ObjRightHand );
	float y = ObjectPositionY ( GGVR_Player.ObjRightHand );
	float z = ObjectPositionZ ( GGVR_Player.ObjRightHand );
	MoveObject ( GGVR_Player.ObjRightHand, 10.0f );
	float nx = ObjectPositionX ( GGVR_Player.ObjRightHand ) - x;
	float ny = ObjectPositionY ( GGVR_Player.ObjRightHand ) - y;
	float nz = ObjectPositionZ ( GGVR_Player.ObjRightHand ) - z;
	MoveObject ( GGVR_Player.ObjRightHand, -10.0f );
	bool bPreferLeftHand = false;
	if (OpenXRGetLeftHandActive() == true) 
		if(OpenXRGetLeftTouchPadTouch() == true )
			bPreferLeftHand = true;
	if ( bPreferLeftHand==true )
	{
		x = ObjectPositionX ( GGVR_Player.ObjLeftHand );
		y = ObjectPositionY ( GGVR_Player.ObjLeftHand );
		z = ObjectPositionZ ( GGVR_Player.ObjLeftHand );
		MoveObject ( GGVR_Player.ObjLeftHand, 10.0f );
		nx = ObjectPositionX ( GGVR_Player.ObjLeftHand ) - x;
		ny = ObjectPositionY ( GGVR_Player.ObjLeftHand ) - y;
		nz = ObjectPositionZ ( GGVR_Player.ObjLeftHand ) - z;
		MoveObject ( GGVR_Player.ObjLeftHand, -10.0f );
	}
	/*
	// determine hand position and angle
	float x = ObjectPositionX ( GGVR_Player.ObjLeftHand );
	float y = ObjectPositionY ( GGVR_Player.ObjLeftHand );
	float z = ObjectPositionZ ( GGVR_Player.ObjLeftHand );
	MoveObject ( GGVR_Player.ObjLeftHand, 10.0f );
	float nx = ObjectPositionX ( GGVR_Player.ObjLeftHand ) - x;
	float ny = ObjectPositionY ( GGVR_Player.ObjLeftHand ) - y;
	float nz = ObjectPositionZ ( GGVR_Player.ObjLeftHand ) - z;
	MoveObject ( GGVR_Player.ObjLeftHand, -10.0f );
	bool bPreferRightHand = false;
	if (OpenXRGetLeftHandActive() == false) bPreferRightHand = true;
	if ( GGVR_TeleportHand == GGVR_TELEPORT_HAND_RIGHT || bPreferRightHand==true )
	{
		x = ObjectPositionX ( GGVR_Player.ObjRightHand );
		y = ObjectPositionY ( GGVR_Player.ObjRightHand );
		z = ObjectPositionZ ( GGVR_Player.ObjRightHand );
		MoveObject ( GGVR_Player.ObjRightHand, 10.0f );
		nx = ObjectPositionX ( GGVR_Player.ObjRightHand ) - x;
		ny = ObjectPositionY ( GGVR_Player.ObjRightHand ) - y;
		nz = ObjectPositionZ ( GGVR_Player.ObjRightHand ) - z;
		MoveObject ( GGVR_Player.ObjRightHand, -10.0f );
	}
	*/

	// create teleport arc control points or laser
	GGVR_Player.LaserGuideActive = 0;
	static int g_iControllerVisualMode = 0;
	if ( GGVR_TeleportState != GGVR_TELEPORT_INACTIVE )
	{
		GGVR_bTeleportInvalid = false;

		// only intersect scan a few times a second
		float fIntersectChecksEveryUnits = 0.0f;
		bool bDoAScanAndUpdateStatus = false;
		static DWORD g_dwTeleportScanTimer = 0;
		if ( timeGetTime() > g_dwTeleportScanTimer )
		{
			bDoAScanAndUpdateStatus = true;
			g_dwTeleportScanTimer = timeGetTime() + 200;
		}

		std::vector <GGVECTOR3> vecControlPoints;
		vecControlPoints.clear();
		bool bAboveGround = true;
		float fLastX = x;
		float fLastY = y;
		float fLastZ = z;
		while ( bAboveGround == true )
		{
			// shift normal to fall with gravity
			nx *= 0.99f;
			ny -= 0.1f; if ( ny < -10.0f ) ny = -10.0f;
			nz *= 0.99f;

			// move control point
			x += nx;
			y += ny;
			z += nz;
			GGVECTOR3 pnt = GGVECTOR3 ( x, y, z );
			vecControlPoints.push_back ( pnt );

			// when drop below ground
			float fActualTerrainGroundY = BT_GetGroundHeight ( iTerrainID, x, z );
			if ( y < fActualTerrainGroundY )
			{
				GGVR_fTelePortDestinationX = x;
				GGVR_fTelePortDestinationY = fActualTerrainGroundY;
				GGVR_fTelePortDestinationZ = z;
				bAboveGround = false;
			}

			// when cut through an entity in the scene
			if ( bDoAScanAndUpdateStatus == true )
			{
				bool bCheckedRay = false;
				if ( fIntersectChecksEveryUnits == 0.0f || bAboveGround == false )
				{
					// check ray between last and current
					IntersectAll(iLMObjStart,iLMObjFinish,0,0,0,0,0,0,-123);
					int tHitObj = IntersectAll(iEntObjStart,iEndObjEnd,x,y,z,fLastX,fLastY,fLastZ,0);
					if ( tHitObj == 0 ) tHitObj = IntersectAll(iBatchEntObjStart,iBatchEndObjEnd,x,y,z,fLastX,fLastY,fLastZ,0);
					if ( tHitObj > 0 )
					{
						GGVR_fTelePortDestinationX = ChecklistFValueA(6);
						GGVR_fTelePortDestinationY = ChecklistFValueB(6);
						GGVR_fTelePortDestinationZ = ChecklistFValueC(6);
						bAboveGround = false;
						if ( ny >= 0.0f ) GGVR_bTeleportInvalid = true;
					}
					bCheckedRay = true;
				}
				float fDistX = x - fLastX;
				float fDistY = y - fLastY;
				float fDistZ = z - fLastZ;
				if ( bCheckedRay == true )
				{
					fLastX = x; fLastY = y; fLastZ = z;
				}
				fIntersectChecksEveryUnits = sqrt ( fabs(fDistX*fDistX)+fabs(fDistY*fDistY)+fabs(fDistZ*fDistZ));
				if ( fIntersectChecksEveryUnits >= 100.0f ) fIntersectChecksEveryUnits = 0.0f;
			}
		}

		// only check now and again
		static float g_iLastScanHitGGVR_fTelePortDestinationX = 0;
		static float g_iLastScanHitGGVR_fTelePortDestinationY = 0;
		static float g_iLastScanHitGGVR_fTelePortDestinationZ = 0;
		static bool g_iLastScanHitbAboveGround = 0;
		static bool g_iLastScanHitInvalid = 0;
		if ( bDoAScanAndUpdateStatus == true )
		{
			// store last results of scan
			g_iLastScanHitGGVR_fTelePortDestinationX = GGVR_fTelePortDestinationX;
			g_iLastScanHitGGVR_fTelePortDestinationY = GGVR_fTelePortDestinationY;
			g_iLastScanHitGGVR_fTelePortDestinationZ = GGVR_fTelePortDestinationZ;
			g_iLastScanHitbAboveGround = bAboveGround;
			g_iLastScanHitInvalid = GGVR_bTeleportInvalid;
		}
		else
		{
			// rest of time simply remember last result
			GGVR_fTelePortDestinationX = g_iLastScanHitGGVR_fTelePortDestinationX;
			GGVR_fTelePortDestinationY = g_iLastScanHitGGVR_fTelePortDestinationY;
			GGVR_fTelePortDestinationZ = g_iLastScanHitGGVR_fTelePortDestinationZ;
			bAboveGround = g_iLastScanHitbAboveGround;
			GGVR_bTeleportInvalid = g_iLastScanHitInvalid;
		}

		// work out spread from perfect arc data to available dots below
		int iControlPointCount = vecControlPoints.size();
		float fCountPointIndex = 0;
		int iMaxObjects = (GGVR_Player.ObjTeleportFinish-GGVR_Player.ObjTeleportStart)-1;
		float fControlPointStep = (float)iControlPointCount/(float)(iMaxObjects);

		// project arc to show teleport destination
		int iLastControlPoint = -1;
		if ( GGVR_TeleportHand == GGVR_TELEPORT_HAND_RIGHT )
		{
			x = ObjectPositionX ( GGVR_Player.ObjRightHand );
			y = ObjectPositionY ( GGVR_Player.ObjRightHand );
			z = ObjectPositionZ ( GGVR_Player.ObjRightHand );
		}
		else
		{
			x = ObjectPositionX ( GGVR_Player.ObjLeftHand );
			y = ObjectPositionY ( GGVR_Player.ObjLeftHand );
			z = ObjectPositionZ ( GGVR_Player.ObjLeftHand );
		}

		// before we settle on final teleport dest, ensure its not too close to a surface (so dont get trapped inside it)
		float fDirX = GGVR_fTelePortDestinationX - x;
		float fDirZ = GGVR_fTelePortDestinationZ - z;
		float fDirDist = sqrt ( fabs(fDirX*fDirX)+fabs(fDirZ*fDirZ) );
		fDirX /= fDirDist;
		fDirZ /= fDirDist;
		fDirX *= 25.0f;
		fDirZ *= 25.0f;
		float fPosNearerPlrX = GGVR_fTelePortDestinationX - fDirX;
		float fPosNearerPlrZ = GGVR_fTelePortDestinationZ - fDirZ;
		float fPosEndOfCircleX = GGVR_fTelePortDestinationX + fDirX;
		float fPosEndOfCircleZ = GGVR_fTelePortDestinationZ + fDirZ;
		float fYOffsetAboveDest = 5.0f;
		IntersectAll(iLMObjStart,iLMObjFinish,0,0,0,0,0,0,-123);
		int tDoesCircleIntersect = IntersectAll(iEntObjStart,iEndObjEnd,fPosNearerPlrX,GGVR_fTelePortDestinationY+fYOffsetAboveDest,fPosNearerPlrZ,fPosEndOfCircleX,GGVR_fTelePortDestinationY+fYOffsetAboveDest,fPosEndOfCircleZ,0);
		if ( tDoesCircleIntersect == 0 ) tDoesCircleIntersect = IntersectAll(iBatchEntObjStart,iBatchEndObjEnd,fPosNearerPlrX,GGVR_fTelePortDestinationY+fYOffsetAboveDest,fPosNearerPlrZ,fPosEndOfCircleX,GGVR_fTelePortDestinationY+fYOffsetAboveDest,fPosEndOfCircleZ,0);
		if ( tDoesCircleIntersect > 0 )
		{
			// the circle is intersecting something, cannot land here
			GGVR_bTeleportInvalid = true;
		}
		IntersectAll(iLMObjStart,iLMObjFinish,0,0,0,0,0,0,-123);
		tDoesCircleIntersect = IntersectAll(iEntObjStart,iEndObjEnd,GGVR_fTelePortDestinationX,GGVR_fTelePortDestinationY+fYOffsetAboveDest,GGVR_fTelePortDestinationZ,GGVR_fTelePortDestinationX,GGVR_fTelePortDestinationY+85.0f,GGVR_fTelePortDestinationZ,0);
		if ( tDoesCircleIntersect == 0 ) tDoesCircleIntersect = IntersectAll(iBatchEntObjStart,iBatchEndObjEnd,GGVR_fTelePortDestinationX,GGVR_fTelePortDestinationY+fYOffsetAboveDest,GGVR_fTelePortDestinationZ,GGVR_fTelePortDestinationX,GGVR_fTelePortDestinationY+85.0f,GGVR_fTelePortDestinationZ,0);
		if ( tDoesCircleIntersect > 0 )
		{
			// the circle is intersecting above (so player cannot fit in), cannot land here
			GGVR_bTeleportInvalid = true;
		}

		// create objects for the arc
		if ( g_iControllerVisualMode != 1 && ObjectExist(GGVR_Player.ObjTeleportStart) == 1 ) DeleteObject ( GGVR_Player.ObjTeleportStart );
		for ( int o = GGVR_Player.ObjTeleportStart; o < GGVR_Player.ObjTeleportFinish; o++ )
		{
			int iThisUniqueControlPoint = (int)fCountPointIndex;
			if ( iThisUniqueControlPoint >= iControlPointCount ) iThisUniqueControlPoint = iControlPointCount - 1;
			if ( !ObjectExist ( o ) )
			{
				// create object slice
				MakeObjectBox ( o, 0.3f, 0.1f, 5.0f );	
				SetObjectEffect ( o, GGVR_Player.ShaderID );
				SetSphereRadius ( o, 0 );
				SetObjectMask ( o, (1<<6) + (1<<7) + 1 );
				// from editimage+14 (also have +16 for red and +17 for green)
				if ( GGVR_bTeleportInvalid )
					TextureObject ( o, 0, GGVR_Player.TextureID+2 );
				else
					TextureObject ( o, 0, GGVR_Player.TextureID+3 );
				SetObjectTransparency ( o, 2 );
				SetObjectLight ( o, 0 );
				SetAlphaMappingOn ( o, 50 );
				g_iControllerVisualMode = 1;
			}
			else
			{
				// from editimage+14 (also have +16 for red and +17 for green)
				if ( GGVR_bTeleportInvalid )
					TextureObject ( o, 0, GGVR_Player.TextureID+2 );
				else
					TextureObject ( o, 0, GGVR_Player.TextureID+3 );
			}
			HideObject ( o );
			if ( ObjectExist ( o ) && iLastControlPoint != iThisUniqueControlPoint )
			{
				// use this object
				ShowObject ( o );

				// position and rotate object
				float lx = x;
				float ly = y;
				float lz = z;
				x = vecControlPoints[iThisUniqueControlPoint].x;
				y = vecControlPoints[iThisUniqueControlPoint].y;
				z = vecControlPoints[iThisUniqueControlPoint].z;
				PositionObject ( o, x, y, z );

				// rotate to follow curve
				PointObject ( o, lx, ly, lz );
			}

			// next control point
			iLastControlPoint = (int)fCountPointIndex;
			fCountPointIndex += fControlPointStep;
		}

		// last object is a circle (squashed sphere)
		if ( !ObjectExist ( GGVR_Player.ObjTeleportFinish ) )
		{
			MakeObjectSphere ( GGVR_Player.ObjTeleportFinish, 50.0f, 6, 30 );
			ScaleObject ( GGVR_Player.ObjTeleportFinish, 100, 10, 100 );
			SetObjectEffect ( GGVR_Player.ObjTeleportFinish, GGVR_Player.ShaderID );
			SetSphereRadius ( GGVR_Player.ObjTeleportFinish, 0 );
			SetObjectMask ( GGVR_Player.ObjTeleportFinish, (1<<6) + (1<<7) + 1 );
			SetObjectTransparency ( GGVR_Player.ObjTeleportFinish, 2 );
			SetObjectLight ( GGVR_Player.ObjTeleportFinish, 0 );
			SetAlphaMappingOn ( GGVR_Player.ObjTeleportFinish, 50 );
		}
		if ( GGVR_bTeleportInvalid )
			TextureObject ( GGVR_Player.ObjTeleportFinish, 0, GGVR_Player.TextureID+2 );
		else
			TextureObject ( GGVR_Player.ObjTeleportFinish, 0, GGVR_Player.TextureID+3 );
		PositionObject ( GGVR_Player.ObjTeleportFinish, GGVR_fTelePortDestinationX, GGVR_fTelePortDestinationY+5.0f, GGVR_fTelePortDestinationZ );
		RotateObject ( GGVR_Player.ObjTeleportFinish, 0, 0, 0 );
		if ( !GGVR_bTeleportInvalid )
			ShowObject ( GGVR_Player.ObjTeleportFinish );
		else
			HideObject ( GGVR_Player.ObjTeleportFinish );
	}
	else
	{
		if ( g_iControllerVisualMode != 2 && ObjectExist(GGVR_Player.ObjTeleportStart) == 1 ) DeleteObject ( GGVR_Player.ObjTeleportStart );
		int o = GGVR_Player.ObjTeleportStart;
		if ( !ObjectExist ( o ) )
		{
			MakeObjectBox ( o, 0.1f, 0.1f, 200.0f );	
			SetObjectEffect ( o, GGVR_Player.ShaderID );
			SetSphereRadius ( o, 0 );
			SetObjectMask ( o, (1<<6) + (1<<7) + 1 );
			TextureObject ( o, 0, GGVR_Player.TextureID+3 );
			SetObjectTransparency ( o, 2 );
			SetObjectLight ( o, 0 );
			SetAlphaMappingOn ( o, 50 );
			g_iControllerVisualMode = 2;
		}
		if ( ObjectExist ( o ) )
		{
			// position and rotate object
			ShowObject ( o );
			PositionObject ( o, x, y, z );
			PointObject ( o, x+nx, y+ny, z+nz );
			MoveObject ( o, GGVR_Player.fLaserForwardDistance );
			GGVR_Player.LaserGuideActive = o;
		}
		for ( int o = GGVR_Player.ObjTeleportStart+1; o <= GGVR_Player.ObjTeleportFinish; o++ )
			if ( ObjectExist(o) )
				HideObject ( o );
	}
}

int GGVR_GetTurnDirection()
{
	if ( GGVR_TeleportState != GGVR_TELEPORT_INACTIVE )
	{
		GGVR_bPrevTurnStateLeft = 0;
		GGVR_bPrevTurnStateRight = 0;
		return 0;
	}

	int resultL = 0;
	int resultR = 0;

	int turnDir = GGVR_bPrevTurnStateLeft;
	if ( OpenXRGetLeftStickX() > 0.4f ) turnDir = 1;
	else if ( OpenXRGetLeftStickX() < -0.4f ) turnDir = -1;
	else if ( abs(OpenXRGetLeftStickX()) < 0.3f ) turnDir = 0;
	if ( turnDir != GGVR_bPrevTurnStateLeft && GGVR_bPrevTurnStateLeft == 0 )
	{
		resultL = turnDir;
	}
	GGVR_bPrevTurnStateLeft = turnDir;

	turnDir = GGVR_bPrevTurnStateRight;
	if ( OpenXRGetRightStickX() > 0.4f ) turnDir = 1;
	else if ( OpenXRGetRightStickX() < -0.4f ) turnDir = -1;
	else if ( abs(OpenXRGetRightStickX()) < 0.3f ) turnDir = 0;
	if ( turnDir != GGVR_bPrevTurnStateRight && GGVR_bPrevTurnStateRight == 0 )
	{
		resultR = turnDir;
	}
	GGVR_bPrevTurnStateRight = turnDir;

	if ( resultR != 0 ) resultL = resultR;
	return resultL;
}

bool GGVR_HandlePlayerTeleport ( float* pNewPosX, float* pNewPosY, float* pNewPosZ, float* pNewAngleY )
{
	bool lTouchPadClick = OpenXRGetLeftTouchPadClick();
	bool rTouchPadClick = OpenXRGetRightTouchPadClick();
	bool lTouchPadTouch = OpenXRGetLeftTouchPadTouch();
	bool rTouchPadTouch = OpenXRGetRightTouchPadTouch();
	bool hasLeftTouchPad = OpenXRHasLeftTouchPad();
	bool hasRightTouchPad = OpenXRHasRightTouchPad();
	bool lButton = OpenXRGetLeftButtonA();
	bool rButton = OpenXRGetRightButtonA();

	switch( GGVR_TeleportState )
	{
		case GGVR_TELEPORT_INACTIVE:
		{
			bool bActive = false;
			if ( hasRightTouchPad )
			{
				if ( rTouchPadTouch ) bActive = true;
			}
			else
			{
				if ( rButton ) bActive = true;
			}

			if ( bActive )
			{
				GGVR_TeleportHand = GGVR_TELEPORT_HAND_RIGHT;
				GGVR_TeleportState = GGVR_TELEPORT_ACTIVE;
			}

			bActive = false;
			if ( hasLeftTouchPad )
			{
				if ( lTouchPadTouch ) bActive = true;
			}
			else
			{
				if ( lButton ) bActive = true;
			}

			if ( bActive )
			{
				GGVR_TeleportHand = GGVR_TELEPORT_HAND_LEFT;
				GGVR_TeleportState = GGVR_TELEPORT_ACTIVE;
			}			
		} break;

		case GGVR_TELEPORT_ACTIVE:
		{
			if ( GGVR_TeleportHand == GGVR_TELEPORT_HAND_LEFT )
			{
				if ( hasLeftTouchPad )
				{
					if ( lTouchPadClick ) GGVR_TeleportState = GGVR_TELEPORT_FINISHED;
					else if ( !lTouchPadTouch ) GGVR_TeleportState = GGVR_TELEPORT_INACTIVE;
				}
				else
				{
					if ( !lButton ) GGVR_TeleportState = GGVR_TELEPORT_FINISHED;
				}
			}
			else
			{
				if ( hasRightTouchPad )
				{
					if ( rTouchPadClick ) GGVR_TeleportState = GGVR_TELEPORT_FINISHED;
					else if ( !rTouchPadTouch ) GGVR_TeleportState = GGVR_TELEPORT_INACTIVE;
				}
				else
				{
					if ( !rButton ) GGVR_TeleportState = GGVR_TELEPORT_FINISHED;
				}
			}
		} break;

		case GGVR_TELEPORT_FINISHED:
		{
			GGVR_TeleportState = GGVR_TELEPORT_WAITFORNOCLICKRELEASE;
			if ( !GGVR_bTeleportInvalid ) 
			{
				*pNewPosX = GGVR_fTelePortDestinationX;
				*pNewPosY = GGVR_fTelePortDestinationY+30;
				*pNewPosZ = GGVR_fTelePortDestinationZ;
				*pNewAngleY = CameraAngleY(0);
				return true;
			}
		}

		case GGVR_TELEPORT_WAITFORNOCLICKRELEASE:
		{
			if (lTouchPadClick == false && rTouchPadClick == false)
			{
				GGVR_TeleportState = GGVR_TELEPORT_INACTIVE;
			}
		}
	}

	return false;
}

void GGVR_SetPlayerPosition( float X, float Y, float Z )
{	
	player_pos_x = X;
	player_pos_y = Y;
	player_pos_z = Z;
}

void GGVR_SetPlayerRotation( float X, float Y, float Z )
{
	player_rot_x = X;
	player_rot_y = Y;
	player_rot_z = Z;
}

void GGVR_SetPlayerAngleY( float valy )
{
	player_rot_y = valy;
}

void GGVR_SetGenericOffsetAngX( float angx )
{
	generic_offset_ang_x = angx;
}

void GGVR_SetWMROffsetAngX( float angx )
{
	wmr_offset_ang_x = angx;
}

float GGVR_GetPlayerX( ) { return player_pos_x; }
float GGVR_GetPlayerY( ) { return player_pos_y; }
float GGVR_GetPlayerZ( ) { return player_pos_z; }

float GGVR_GetPlayerAngleX( ) { return player_rot_x; }
float GGVR_GetPlayerAngleY( ) { return player_rot_y; }
float GGVR_GetPlayerAngleZ( ) { return player_rot_z; }

void GGVR_SetHMDDirectly(float fX, float fY, float fZ, float fNX, float fNY, float fNZ)
{
	// called from heart of left eye render to get HMD orientation from source
	hmd_rot_x = fX; // not working great in LUA script, which is why I added normal values
	hmd_rot_y = fY;
	hmd_rot_z = fZ;
	hmd_normal_x = fNX;
	hmd_normal_y = fNY;
	hmd_normal_z = fNZ;
}

float GGVR_GetHMDX( )
{
	return (left_eye_pos_x + right_eye_pos_x) / 2.0f;
}

float GGVR_GetHMDY( )
{
	return (left_eye_pos_y + right_eye_pos_y) / 2.0f;
}

float GGVR_GetHMDZ( )
{
	return (left_eye_pos_z + right_eye_pos_z) / 2.0f;
}

float GGVR_GetHMDYaw()
{
	return hmd_rot_y - player_rot_y;
}

float GGVR_GetHMDPitch()
{
	return hmd_rot_x - player_rot_x;
}

float GGVR_GetHMDRoll()
{
	return hmd_rot_z - player_rot_z;
}

float GGVR_GetHMDRNormalX()
{
	return hmd_normal_x;
}

float GGVR_GetHMDRNormalY()
{
	return hmd_normal_y;
}

float GGVR_GetHMDRNormalZ()
{
	return hmd_normal_z;
}

float GGVR_GetHMDYawWorld()
{
	return hmd_rot_y;
}

float GGVR_GetHMDPitchWorld()
{
	return hmd_rot_x;
}

float GGVR_GetHMDRollWorld()
{
	return hmd_rot_z;
}

float GGVR_GetHMDOffsetX()
{
	// not sure what this value is meant to be, return the difference between the eye position and player position
	return GGVR_GetHMDX() - GGVR_GetPlayerX();
}

float GGVR_GetHMDOffsetY()
{
	// not sure what this value is meant to be, return the difference between the eye position and player position
	return GGVR_GetHMDY() - GGVR_GetPlayerY();
}

float GGVR_GetHMDOffsetZ()
{
	// not sure what this value is meant to be, return the difference between the eye position and player position
	return GGVR_GetHMDZ() - GGVR_GetPlayerZ();
}

float GGVR_GetHMDAngleX( )
{
	return hmd_rot_x;
}

float GGVR_GetHMDAngleY( )
{
	return hmd_rot_y;
}

float GGVR_GetHMDAngleZ( )
{
	return hmd_rot_z;
}

int GGVR_GetRightHandObject()
{
	return GGVR_Player.ObjRightHand;
}

int GGVR_GetLeftHandObject()
{
	return GGVR_Player.ObjLeftHand;
}

int GGVR_GetLaserGuideObject()
{
	return GGVR_Player.LaserGuideActive;
}

void GGVR_SetRightHandInvisible(bool bInvisible)
{
	GGVR_Player.bMakeRightHandInvisible = bInvisible;
}

void GGVR_SetLeftHandInvisible(bool bInvisible)
{
	GGVR_Player.bMakeLeftHandInvisible = bInvisible;
}

float GGVR_GetRightHandX( )
{
	if ( GGVR_Player.ObjRightHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
		{
			return ObjectPositionX(GGVR_Player.ObjRightHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetRightHandY( )
{
	if ( GGVR_Player.ObjRightHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
		{
			return ObjectPositionY(GGVR_Player.ObjRightHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetRightHandZ( )
{
	if ( GGVR_Player.ObjRightHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
		{
			return ObjectPositionZ(GGVR_Player.ObjRightHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetRightHandAngleX( )
{
	if ( GGVR_Player.ObjRightHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
		{
			return ObjectAngleX(GGVR_Player.ObjRightHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetRightHandAngleY( )
{
	if ( GGVR_Player.ObjRightHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
		{
			return ObjectAngleY(GGVR_Player.ObjRightHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetRightHandAngleZ( )
{
	if ( GGVR_Player.ObjRightHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjRightHand) == 1)
		{
			return ObjectAngleZ(GGVR_Player.ObjRightHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetLeftHandX( )
{
	if ( GGVR_Player.ObjLeftHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
		{
			return ObjectPositionX(GGVR_Player.ObjLeftHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetLeftHandY( )
{
	if ( GGVR_Player.ObjLeftHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
		{
			return ObjectPositionY(GGVR_Player.ObjLeftHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetLeftHandZ( )
{
	if ( GGVR_Player.ObjLeftHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
		{
			return ObjectPositionZ(GGVR_Player.ObjLeftHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetLeftHandAngleX( )
{
	if ( GGVR_Player.ObjLeftHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
		{
			return ObjectAngleX(GGVR_Player.ObjLeftHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetLeftHandAngleY( )
{
	if ( GGVR_Player.ObjLeftHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
		{
			return ObjectAngleY(GGVR_Player.ObjLeftHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetLeftHandAngleZ( )
{
	if ( GGVR_Player.ObjLeftHand > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjLeftHand) == 1)
		{
			return ObjectAngleZ(GGVR_Player.ObjLeftHand);
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetBestHandX()
{
	if ( GGVR_Player.ObjTeleportStart > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjTeleportStart) == 1)
		{
			float fValue = ObjectPositionX ( GGVR_Player.ObjTeleportStart );
			return fValue;
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetBestHandY()
{
	//float fValue = GGVR_GetLeftHandY();
	//if ( fabs(GGVR_GetRightHandX()) > 0 ) fValue = GGVR_GetRightHandY();
	if ( GGVR_Player.ObjTeleportStart > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjTeleportStart) == 1)
		{
			float fValue = ObjectPositionY ( GGVR_Player.ObjTeleportStart );
			return fValue;
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetBestHandZ()
{
	//float fValue = GGVR_GetLeftHandZ();
	//if ( fabs(GGVR_GetRightHandX()) > 0 ) fValue = GGVR_GetRightHandZ();
	if ( GGVR_Player.ObjTeleportStart > 0 )
	{
		if (ObjectExist(GGVR_Player.ObjTeleportStart) == 1)
		{
			float fValue = ObjectPositionZ ( GGVR_Player.ObjTeleportStart );
			return fValue;
		}
		else { return 0; }
	}
	else { return 0; }
}

float GGVR_GetBestHandAngleX()
{
	float fValue = GGVR_GetLeftHandAngleX();
	if ( fabs(GGVR_GetRightHandX()) > 0 ) fValue = GGVR_GetRightHandAngleX();
	return fValue;
}

float GGVR_GetBestHandAngleY()
{
	float fValue = GGVR_GetLeftHandAngleY();
	if ( fabs(GGVR_GetRightHandX()) > 0 ) fValue = GGVR_GetRightHandAngleY();
	return fValue;
}

float GGVR_GetBestHandAngleZ()
{
	float fValue = GGVR_GetLeftHandAngleZ();
	if ( fabs(GGVR_GetRightHandX()) > 0 ) fValue = GGVR_GetRightHandAngleZ();
	return fValue;
}

int GGVR_GetLaserGuidedEntityObj ( int entityviewstartobj, int entityviewendobj )
{
	int iEntObjectNumber = 0;
	if ( GGVR_Player.LaserGuideActive > 0 )
	{
		int o = GGVR_Player.LaserGuideActive;
		if ( ObjectExist ( o ) == 1 )
		{
			// work out ray cast from laser object
			MoveObject ( o, -100.0f );
			float fX = ObjectPositionX ( o );
			float fY = ObjectPositionY ( o );
			float fZ = ObjectPositionZ ( o );
			MoveObject ( o, 200.0f );
			float fNewX = ObjectPositionX ( o );
			float fNewY = ObjectPositionY ( o );
			float fNewZ = ObjectPositionZ ( o );
			MoveObject ( o, -100.0f );

			// use this laser object to trace line and detect any entity object in its path
			int iIgnoreObjNo = GGVR_Player.LaserGuideActive;
			int ttt = IntersectAll(0,0,0,0,0,0,0,0,-123);
			int tthitvalue = IntersectAll(entityviewstartobj,entityviewendobj,fX, fY, fZ, fNewX, fNewY, fNewZ, iIgnoreObjNo);
			if ( tthitvalue > 0 )
			{
				// this entity was found when tracing the laser line
				iEntObjectNumber = tthitvalue;
			}
		}
	}
	return iEntObjectNumber;
}

void GGVR_SetLaserForwardDistance(float fDist)
{
	GGVR_Player.fLaserForwardDistance = fDist;
}

// Generic Controller

int GGVR_RightControllerFound( )
{
	return 1;
}

int GGVR_LeftControllerFound( )
{
	return 1;
}

void GGVR_LeftIsBest(bool bEnabled)
{
	GGVR_bLeftIsBest = bEnabled;
}

float GGVR_BestController_JoyX(void)
{
	float fValue = OpenXRGetLeftStickX();
	if (GGVR_bLeftIsBest == false)
	{
		float fValueR = OpenXRGetRightStickX();
		if (fabs(fValueR) > fabs(fValue)) fValue = fValueR;
	}
	return fValue;
}

float GGVR_BestController_JoyY(void)
{
	float fValue = OpenXRGetLeftStickY();
	if (GGVR_bLeftIsBest == false)
	{
		float fValueR = OpenXRGetRightStickY();
		if (fabs(fValueR) > fabs(fValue)) fValue = fValueR;
	}
	return fValue;
}

float GGVR_RightController_JoyX( void )
{
	return OpenXRGetRightStickX();
}

float GGVR_RightController_JoyY( void )
{
	return OpenXRGetRightStickY();
}

float GGVR_LeftController_JoyX( void )
{
	return OpenXRGetLeftStickX();
}

float GGVR_LeftController_JoyY( void )
{
	return OpenXRGetLeftStickY();
}

float GGVR_RightController_Trigger( void )
{
	return OpenXRGetRightTrigger();
}

float GGVR_LeftController_Trigger( void )
{
	return OpenXRGetLeftTrigger();
}

int GGVR_RightController_Grip(void)
{
	if ( OpenXRGetRightSqueeze() > 0.9f )
		return 1;
	else
		return 0;
}

int GGVR_LeftController_Grip(void)
{
	if ( OpenXRGetLeftSqueeze() > 0.9f )
		return 1;
	else
		return 0;
}

int GGVR_RightController_Button1(void)
{
	return OpenXRGetRightButtonA();
}

int GGVR_LeftController_Button1(void)
{
	return OpenXRGetLeftButtonA();
}

int GGVR_RightController_Button2(void)
{
	return OpenXRGetRightButtonB();
}

int GGVR_LeftController_Button2(void)
{
	return OpenXRGetLeftButtonB();
}

int GGVR_PlayerData::Create( LPSTR pRootDir )
{
	// Clear teleport work objects
	for ( int o = GGVR_Player.ObjTeleportStart; o <= GGVR_Player.ObjTeleportFinish; o++ )
	{
		if ( ObjectExist ( o ) ) DeleteObject ( o );
	}

	//ObjRightHand
	char pControllerModel[1024];
	strcpy ( pControllerModel, pRootDir );
	strcat ( pControllerModel, "\\gamecore\\vrcontroller\\vrcontrollerright.dbo" );
	LoadObject ( pControllerModel, GGVR_Player.ObjRightHand );
	SetObjectCollisionOff(GGVR_Player.ObjRightHand);

	//ObjLeftHand
	strcpy ( pControllerModel, pRootDir );
	strcat ( pControllerModel, "\\gamecore\\vrcontroller\\vrcontrollerleft.dbo" );
	LoadObject ( pControllerModel, GGVR_Player.ObjLeftHand );
	SetObjectCollisionOff(GGVR_Player.ObjLeftHand);

	// make controller objects renderable
	for ( int iM = 0; iM < 2; iM++ )
	{
		int iMObj = 0;
		if ( iM == 0 ) iMObj = GGVR_Player.ObjRightHand;
		if ( iM == 1 ) iMObj = GGVR_Player.ObjLeftHand;
		//DisableObjectZDepth ( iMObj );
		//DisableObjectZRead ( iMObj );
		SetSphereRadius ( iMObj, 0 );
		SetObjectMask ( iMObj, (1<<6) + (1<<7) + 1 );
		TextureObject ( iMObj, 7, GGVR_Player.ControllerTextureID1 );
		TextureObject ( iMObj, 0, GGVR_Player.ControllerTextureID0 );
		TextureObject ( iMObj, 1, GGVR_Player.ControllerTextureID1 );
		TextureObject ( iMObj, 2, GGVR_Player.ControllerTextureID2 );
		TextureObject ( iMObj, 3, GGVR_Player.ControllerTextureID3 );
		TextureObject ( iMObj, 4, GGVR_Player.ControllerTextureID4 );
		TextureObject ( iMObj, 5, GGVR_Player.ControllerTextureID5 );
		TextureObject ( iMObj, 6, GGVR_Player.ControllerTextureID6 );
		SetObjectEffect ( iMObj, GGVR_Player.ControllerShaderID );
		DisableObjectZDepth(iMObj);
	}

	//PlayerVars
	PitchLock = 0;
	TurnLock = 1;

	// success
	return 0;
}

void GGVR_PlayerData::Destroy( )
{
	// Delete playerdata object resources
	if ( ObjectExist(GGVR_Player.ObjRightHand)==1 ) DeleteObject ( GGVR_Player.ObjRightHand );
	if ( ObjectExist(GGVR_Player.ObjLeftHand)==1 ) DeleteObject ( GGVR_Player.ObjLeftHand );

	// Delete any teleport/laser objects
	for ( int o = GGVR_Player.ObjTeleportStart; o <= GGVR_Player.ObjTeleportFinish; o++ )
		if ( ObjectExist(o) ) 
			DeleteObject ( o );
}

int GGVR_PlayerData::GetRHandObjID()
{
	return ObjRightHand;
}

int GGVR_PlayerData::GetLHandObjID()
{
	return ObjLeftHand;
}

int GGVR_PreSubmit ( int iDebugMode )
{
	OpenXRHandleEvents();

	if ( !OpenXRIsSessionSetup() ) 
	{
		OpenXRStartSession();

		if ( !OpenXRIsSessionSetup() ) 
		{
			return -3; // headset not found
		}
		else
		{
			OpenXRGetResolution( OPENXR_RENDER_LEFT, &eye_width, &eye_height );

			// setup depth buffer
			D3D11_TEXTURE2D_DESC depthStencilBufferDesc = { 0 };
			depthStencilBufferDesc.ArraySize = 1;
			depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required.
			depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthStencilBufferDesc.Width = eye_width;
			depthStencilBufferDesc.Height = eye_height;
			depthStencilBufferDesc.MipLevels = 1;
			depthStencilBufferDesc.SampleDesc.Count = 1;
			depthStencilBufferDesc.SampleDesc.Quality = 0;
			depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
 			HRESULT hr = m_pD3D->CreateTexture2D( &depthStencilBufferDesc, nullptr, &depthTexture );
			if ( FAILED(hr) ) 
			{ 
				//MessageBox( NULL, "Failed to create depth texture", "GGVR Error", 0 );
				return 1;
			}

			hr = m_pD3D->CreateDepthStencilView( depthTexture, nullptr, &depthView );
			if ( FAILED(hr) ) 
			{ 
				//MessageBox( NULL, "Failed to create depth view", "GGVR Error", 0 );
				return 1;
			}
		}
	}

	if ( !OpenXRIsSessionActive() ) return -2;

	bool shouldRender = OpenXRStartFrame();
	if ( shouldRender )
	{
		// create non-inverse player body matrix
		KMaths::Matrix matPlayer;

		float sx = sin( player_rot_x * PI / 180.0f );
		float sy = sin( player_rot_y * PI / 180.0f );
		float sz = sin( player_rot_z * PI / 180.0f );

		float cx = cos( player_rot_x * PI / 180.0f );
		float cy = cos( player_rot_y * PI / 180.0f );
		float cz = cos( player_rot_z * PI / 180.0f );

		matPlayer.m[0][0] = cz*cy + sx*sy*sz;
		matPlayer.m[0][1] = sz*cx;
		matPlayer.m[0][2] = sz*sx*cy - cz*sy;
		matPlayer.m[0][3] = 0;
	
		matPlayer.m[1][0] = sy*cz*sx - sz*cy;
		matPlayer.m[1][1] = cx*cz;
		matPlayer.m[1][2] = sz*sy + cy*cz*sx;
		matPlayer.m[1][3] = 0;
	
		matPlayer.m[2][0] = sy*cx;
		matPlayer.m[2][1] = -sx;
		matPlayer.m[2][2] = cx*cy;
		matPlayer.m[2][3] = 0;

		matPlayer.m[3][0] = player_pos_x;
		matPlayer.m[3][1] = player_pos_y;
		matPlayer.m[3][2] = player_pos_z;
		matPlayer.m[3][3] = 1;

		// get left hand pose matrix
		KMaths::Matrix matHand;
		OpenXRGetLeftHandMatrix( (float*)(matHand.m), 1 );
		matHand.m[3][0] *= GGVR_WorldScale; // convert from meters to inches
		matHand.m[3][1] *= GGVR_WorldScale;
		matHand.m[3][2] *= GGVR_WorldScale;

		// multiply by the player body matrix
		matHand *= matPlayer;

		// set position and rotation instead of the matrix directly as we need to retrieve the position and rotation in other places
		PositionObject( GGVR_Player.ObjLeftHand, matHand.m[3][0], matHand.m[3][1], matHand.m[3][2] );
		float x,y,z;
		GGVR_MatrixToEuler( &matHand, &x, &y, &z );
		if ( left_controller_type == OPENXR_CONTROLLER_WMR ) 
			x += wmr_offset_ang_x;
		else
			x += generic_offset_ang_x;
		RotateObject( GGVR_Player.ObjLeftHand, x, y, z );
				
		// get right hand pose matrix
		OpenXRGetRightHandMatrix( (float*)(matHand.m), 1 );
		matHand.m[3][0] *= GGVR_WorldScale; // convert from meters to inches
		matHand.m[3][1] *= GGVR_WorldScale;
		matHand.m[3][2] *= GGVR_WorldScale;

		// multiply by the player body matrix
		matHand *= matPlayer;

		PositionObject( GGVR_Player.ObjRightHand, matHand.m[3][0], matHand.m[3][1], matHand.m[3][2] );
		GGVR_MatrixToEuler( &matHand, &x, &y, &z );
		if ( right_controller_type == OPENXR_CONTROLLER_WMR ) 
			x += wmr_offset_ang_x;
		else
			x += generic_offset_ang_x;
		RotateObject( GGVR_Player.ObjRightHand, x, y, z );
	}

	return shouldRender ? 0 : -1;
}

void GGVR_StartRender( int side )
{
	curr_side = side;

	int camID = GGVR_RCamID;
	OpenXRRenderSide renderSide = OPENXR_RENDER_RIGHT;
	if ( side == 0 ) 
	{
		camID = GGVR_LCamID;
		renderSide = OPENXR_RENDER_LEFT;
	}

	tagCameraData* pCameraPtr = (tagCameraData*)GetCameraInternalData ( camID );

	// set render target
	ID3D11Texture2D* renderTexture = 0;
	ID3D11RenderTargetView* renderView = OpenXRStartRender( renderSide, &renderTexture );
	static ID3D11ShaderResourceView* shaderView = 0;
	ID3D11ShaderResourceView* currShaderView = 0;
	if ( camID == 6 )
	{
		if ( shaderView ) shaderView->Release();
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		memset( &viewDesc, 0, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC) );
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Format = (DXGI_FORMAT) OpenXRGetSurfaceFormat();
		viewDesc.Texture2D.MipLevels = 1;
		m_pD3D->CreateShaderResourceView( renderTexture, &viewDesc, &shaderView );
		currShaderView = shaderView;
	}
	SetCameraToView( camID, renderView, depthView, eye_width, eye_height, currShaderView );

	// get VR view matrix
	KMaths::Matrix view;
	OpenXRGetInvViewMat( renderSide, (float*)(view.m) );
	view.m[3][0] *= GGVR_WorldScale; // convert from meters to inches
	view.m[3][1] *= GGVR_WorldScale;
	view.m[3][2] *= GGVR_WorldScale;
		
	// create inverse matrix for player body
	KMaths::Matrix matPlayer;

	float sx = sin( player_rot_x * PI / 180.0f );
	float sy = sin( player_rot_y * PI / 180.0f );
	float sz = sin( player_rot_z * PI / 180.0f );

	float cx = cos( player_rot_x * PI / 180.0f );
	float cy = cos( player_rot_y * PI / 180.0f );
	float cz = cos( player_rot_z * PI / 180.0f );

	matPlayer.m[0][0] = cz*cy + sx*sy*sz;
	matPlayer.m[1][0] = sz*cx;
	matPlayer.m[2][0] = sz*sx*cy - cz*sy;
	
	matPlayer.m[0][1] = sy*cz*sx - sz*cy;
	matPlayer.m[1][1] = cx*cz;
	matPlayer.m[2][1] = sz*sy + cy*cz*sx;
	
	matPlayer.m[0][2] = sy*cx;
	matPlayer.m[1][2] = -sx;
	matPlayer.m[2][2] = cx*cy;

	matPlayer.m[0][3] = 0;
	matPlayer.m[1][3] = 0;
	matPlayer.m[2][3] = 0;

	float x = -player_pos_x;
	float y = -player_pos_y;
	float z = -player_pos_z;

	matPlayer.m[3][0] = x*matPlayer.m[0][0] + y*matPlayer.m[1][0] + z*matPlayer.m[2][0];
	matPlayer.m[3][1] = x*matPlayer.m[0][1] + y*matPlayer.m[1][1] + z*matPlayer.m[2][1];
	matPlayer.m[3][2] = x*matPlayer.m[0][2] + y*matPlayer.m[1][2] + z*matPlayer.m[2][2];
	matPlayer.m[3][3] = 1;
	
	// combine VR view ,atrix with player body matrix
	matPlayer *= view;

	// must call SetCameraMatrix, setting the override manually doesn't seem to work
	SetCameraMatrix( camID, &matPlayer );

	KMaths::Vector3 pos = GetCameraPosition();
	if ( renderSide == OPENXR_RENDER_LEFT )
	{
		left_eye_pos_x = pos.x;
		left_eye_pos_y = pos.y;
		left_eye_pos_z = pos.z;
	}
	else
	{
		right_eye_pos_x = pos.x;
		right_eye_pos_y = pos.y;
		right_eye_pos_z = pos.z;
	}

	hmd_rot_x = CameraAngleX( camID );
	hmd_rot_y = CameraAngleY( camID );
	hmd_rot_z = CameraAngleZ( camID );

	// set projection matrix
	float proj[16];
	OpenXRGetProjMat( renderSide, GGVR_NearClip, GGVR_FarClip, proj, 0 );
	memcpy( pCameraPtr->matProjection, proj, sizeof(float)*16 );
	memcpy( pCameraPtr->matProjOverride, proj, sizeof(float)*16 );
}

void GGVR_EndRender()
{
	OpenXREndRender();

	int camID = GGVR_RCamID;
	if ( curr_side == 0 ) camID = GGVR_LCamID;

	tagCameraData* pCameraPtr = (tagCameraData*)GetCameraInternalData ( camID );
	pCameraPtr->bOverride = false;
}

void GGVR_SetOpenXRValuesForMAX(void)
{
	// this was missed out of the manual coding of VR rendering for MAX (would be nice to unify them one day)
	// these values used by player control script to determine if HMD being used instead of mouse/keyboard

	// now done wth new GGVR_SetHMDDirectly function called from actual render code
	//hmd_rot_x = CameraAngleX( 0 );
	//hmd_rot_y = CameraAngleY( 0 );
	//hmd_rot_z = CameraAngleZ( 0 );

	// for now do not walk in direction looking, requires an element of the code snipped below, but its not
	// 100% there, something else is impacting the old style behavior, so come at it with fresh head!
	/*
	// get VR view matrix
	KMaths::Matrix view;
	OpenXRGetInvViewMat( OPENXR_RENDER_LEFT, (float*)(view.m) );
	view.m[3][0] *= GGVR_WorldScale; // convert from meters to inches
	view.m[3][1] *= GGVR_WorldScale;
	view.m[3][2] *= GGVR_WorldScale;
		
	// create inverse matrix for player body
	KMaths::Matrix matPlayer;

	float sx = sin( player_rot_x * PI / 180.0f );
	float sy = sin( player_rot_y * PI / 180.0f );
	float sz = sin( player_rot_z * PI / 180.0f );

	float cx = cos( player_rot_x * PI / 180.0f );
	float cy = cos( player_rot_y * PI / 180.0f );
	float cz = cos( player_rot_z * PI / 180.0f );

	matPlayer.m[0][0] = cz*cy + sx*sy*sz;
	matPlayer.m[1][0] = sz*cx;
	matPlayer.m[2][0] = sz*sx*cy - cz*sy;
	
	matPlayer.m[0][1] = sy*cz*sx - sz*cy;
	matPlayer.m[1][1] = cx*cz;
	matPlayer.m[2][1] = sz*sy + cy*cz*sx;
	
	matPlayer.m[0][2] = sy*cx;
	matPlayer.m[1][2] = -sx;
	matPlayer.m[2][2] = cx*cy;

	matPlayer.m[0][3] = 0;
	matPlayer.m[1][3] = 0;
	matPlayer.m[2][3] = 0;

	float x = -player_pos_x;
	float y = -player_pos_y;
	float z = -player_pos_z;

	matPlayer.m[3][0] = x*matPlayer.m[0][0] + y*matPlayer.m[1][0] + z*matPlayer.m[2][0];
	matPlayer.m[3][1] = x*matPlayer.m[0][1] + y*matPlayer.m[1][1] + z*matPlayer.m[2][1];
	matPlayer.m[3][2] = x*matPlayer.m[0][2] + y*matPlayer.m[1][2] + z*matPlayer.m[2][2];
	matPlayer.m[3][3] = 1;
	
	// combine VR view ,atrix with player body matrix
	matPlayer *= view;

	// must call SetCameraMatrix so we can get the true eulers
	if (CameraExist(6) == 0) CreateCamera(6);
	SetCameraMatrix( 6, &matPlayer );
	hmd_rot_x = CameraAngleX( 6 );
	hmd_rot_y = CameraAngleY( 6 );
	hmd_rot_z = CameraAngleZ( 6 );
	*/
}

void GGVR_SetHandObjectForMAX(int iLeftOrRightHand, float fHandX, float fHandY, float fHandZ, float fHandAngX, float fHandAngY, float fHandAngZ)
{
	int iObjectHand = GGVR_Player.ObjLeftHand;
	if (iLeftOrRightHand == 1)
	{
		iObjectHand = GGVR_Player.ObjRightHand;
		if (right_controller_type == OPENXR_CONTROLLER_WMR) 
			fHandAngX += wmr_offset_ang_x;
		else
			fHandAngX += generic_offset_ang_x;
	}
	else
	{
		if (left_controller_type == OPENXR_CONTROLLER_WMR) 
			fHandAngX += wmr_offset_ang_x;
		else
			fHandAngX += generic_offset_ang_x;
	}
	PositionObject(iObjectHand, fHandX, fHandY, fHandZ);
	RotateObject(iObjectHand, fHandAngX, fHandAngY, fHandAngZ);
}

void GGVR_SetCameraRange( float Near, float Far )
{
	GGVR_NearClip = Near;
	GGVR_FarClip = Far;
}

void GGVR_Submit ( void )
{
	if ( !OpenXRIsSessionActive() ) return;

	OpenXREndFrame();
}