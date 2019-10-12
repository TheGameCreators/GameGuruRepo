#include "windows.h"

#define DLLEXPORT __declspec ( dllexport )

extern "C" 
{
	DLLEXPORT void GGWMR_GetHolographicSpace ( HWND hWnd );
	DLLEXPORT void GGWMR_InitHolographicSpace ( void** LEyeImage, void** REyeImage );
}

/*
//*********************************************************************************
// GG_OpenVR (version 1.0):
//*********************************************************************************
//developed by: Ron Erickson

//Info:
//The purpose of this library is to make VR possible in Game Guru

//*********************************
//	Setup Commands
//*********************************
// Quick check to see if HMD is found which can be done prior to intialization
DLLEXPORT int	GGVR_IsHmdPresent();
// Check to see if the runtime is installed
DLLEXPORT int	GGVR_IsRuntimeInstalled();


//Setup the VR System.
// The return value states the success or error value
// 0 = Success
// 1 = Init Error
// 2 = Compositor Error
DLLEXPORT int GGVR_Init(int RImageID, int LImageID, int RCamID, int LCamID, int ObjBase, int ObjHead, int ObjOrigin, int ObjRightHand, int ObjLeftHand);

DLLEXPORT GGMATRIX GGVR_GetRightEyeProjectionMatrix();
DLLEXPORT GGMATRIX GGVR_GetLeftEyeProjectionMatrix();

//Set this to true to receive error message boxes for any errors that occur
DLLEXPORT void GGVR_ErrorMessagesOn(int state );

//Tracking State: 
//Seated (default) = 0; Standing = 1
DLLEXPORT void GGVR_SetTrackingSpace( int space );
DLLEXPORT int GGVR_GetTrackingSpace( int space );

//Reset the seated pose
DLLEXPORT void GGVR_ResetSeatedZeroPose();

//Set the Camera Range (use this instead of the standard AGK command: 
DLLEXPORT void GGVR_SetCameraRange( float Near, float Far );

// Scales
DLLEXPORT void GGVR_SetWorldScale( float scale );
DLLEXPORT float GGVR_GetWorldScale(  );

//Render
DLLEXPORT void GGVR_Submit( void );

//2017-11-02: Added to allow rendering to individual eyes
DLLEXPORT void GGVR_Submit_RightEye();
DLLEXPORT void GGVR_Submit_LeftEye();

//2017-11-02: Added to support rendering depth textures
//void	GGVR_SetDepthTextures(int RDepthImageID, int LDepthImageID);

//2017-11-02: Added for custom skybox support
DLLEXPORT void	GGVR_CreateSkyboxOveride(int Front, int Back, int Left, int Right, int Top, int Bottom);

//2017-11-02: Added for custom skybox support
DLLEXPORT void	GGVR_ClearSkyboxOveride();

//2017-11-02: Command added to cleanly exit VR
DLLEXPORT void	GGVR_Shutdown();


//*********************************
//	Player Control
//*********************************
DLLEXPORT void GGVR_UpdatePlayer( );
DLLEXPORT void GGVR_SetPlayerPosition( float X, float Y, float Z );
DLLEXPORT void GGVR_SetPlayerRotation( float X, float Y, float Z );
DLLEXPORT void GGVR_SetOriginPosition(float X, float Y, float Z);
DLLEXPORT void GGVR_SetOriginRotation(float X, float Y, float Z);
DLLEXPORT void GGVR_MovePlayerLocalX( float valx );
DLLEXPORT void GGVR_MovePlayerLocalY( float valy );
DLLEXPORT void GGVR_MovePlayerLocalZ( float valz );
DLLEXPORT void GGVR_RotatePlayerLocalX( float valx );
DLLEXPORT void GGVR_RotatePlayerLocalY( float valy );
DLLEXPORT void GGVR_RotatePlayerLocalZ( float valz );
DLLEXPORT void GGVR_LockPlayerTurn( int Lock );
DLLEXPORT void GGVR_LockPlayerPitch( int Lock );
DLLEXPORT float GGVR_GetPlayerX( );
DLLEXPORT float GGVR_GetPlayerY( );
DLLEXPORT float GGVR_GetPlayerZ( );
DLLEXPORT float GGVR_GetPlayerAngleX( );
DLLEXPORT float GGVR_GetPlayerAngleY( );
DLLEXPORT float GGVR_GetPlayerAngleZ( );
DLLEXPORT float GGVR_GetOriginX();
DLLEXPORT float GGVR_GetOriginY();
DLLEXPORT float GGVR_GetOriginZ();
DLLEXPORT float GGVR_GetHMDX( );
DLLEXPORT float GGVR_GetHMDY( );
DLLEXPORT float GGVR_GetHMDZ( );
DLLEXPORT float GGVR_GetHMDYaw();
DLLEXPORT float GGVR_GetHMDPitch();
DLLEXPORT float GGVR_GetHMDRoll();
DLLEXPORT float GGVR_GetHMDOffsetX();
DLLEXPORT float GGVR_GetHMDOffsetY();
DLLEXPORT float GGVR_GetHMDOffsetZ();
DLLEXPORT float GGVR_GetHMDAngleX( );
DLLEXPORT float GGVR_GetHMDAngleY( );
DLLEXPORT float GGVR_GetHMDAngleZ( );
DLLEXPORT float GGVR_GetRightHandX( );
DLLEXPORT float GGVR_GetRightHandY( );
DLLEXPORT float GGVR_GetRightHandZ( );
DLLEXPORT float GGVR_GetRightHandAngleX( );
DLLEXPORT float GGVR_GetRightHandAngleY( );
DLLEXPORT float GGVR_GetRightHandAngleZ( );
DLLEXPORT float GGVR_GetLeftHandX( );
DLLEXPORT float GGVR_GetLeftHandY( );
DLLEXPORT float GGVR_GetLeftHandZ( );
DLLEXPORT float GGVR_GetLeftHandAngleX( );
DLLEXPORT float GGVR_GetLeftHandAngleY( );
DLLEXPORT float GGVR_GetLeftHandAngleZ( );

//*********************************
//	Controller Commands
//*********************************
DLLEXPORT int GGVR_RightControllerFound( void );
DLLEXPORT int GGVR_LeftControllerFound( void );
DLLEXPORT float GGVR_RightController_JoyX( void );
DLLEXPORT float GGVR_RightController_JoyY( void );
DLLEXPORT float GGVR_LeftController_JoyX( void );
DLLEXPORT float GGVR_LeftController_JoyY( void );
DLLEXPORT float GGVR_RightController_Trigger( void );
DLLEXPORT float GGVR_LeftController_Trigger( void );
DLLEXPORT int GGVR_RightController_Grip( void );
DLLEXPORT int GGVR_LeftController_Grip( void );
DLLEXPORT int GGVR_RightController_Button1( void );
DLLEXPORT int GGVR_LeftController_Button1( void );
DLLEXPORT int GGVR_RightController_Button2( void );
DLLEXPORT int GGVR_LeftController_Button2( void );
DLLEXPORT void GGVR_LeftController_TriggerPulse( int axis, int duration_ms);
DLLEXPORT void GGVR_RightController_TriggerPulse( int axis, int duration_ms);
DLLEXPORT int GGVR_RightController_GetFingerPointed( void );
DLLEXPORT int GGVR_LeftController_GetFingerPointed( void );
DLLEXPORT int GGVR_RightController_GetThumbUp( void );
DLLEXPORT int GGVR_LeftController_GetThumbUp( void );

//2017-11-02: Command so other buttons can be detected
DLLEXPORT int GGVR_RightController_ButtonPressed(int Button);
DLLEXPORT int GGVR_LeftController_ButtonPressed(int Button);

//2017-11-02: Command so grip triggers can be accesses on Oculus Touch controllers
DLLEXPORT float GGVR_RightController_AxisTriggerX(int axis);
DLLEXPORT float GGVR_RightController_AxisTriggerY(int axis);
DLLEXPORT float GGVR_LeftController_AxisTriggerX(int axis);
DLLEXPORT float GGVR_LeftController_AxisTriggerY(int axis);

//*********************************
//	Chaperone Commands
//*********************************
//2017-11-02: Added chaperone info commands
DLLEXPORT float GGVR_Chaperone_GetPlayAreaSizeX();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaSizeZ();
DLLEXPORT int GGVR_Chaperone_AreBoundsVisible();
DLLEXPORT void GGVR_Chaperone_ForceBoundsVisible(int flag);
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint1X();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint1Y();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint1Z();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint2X();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint2Y();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint2Z();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint3X();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint3Y();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint3Z();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint4X();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint4Y();
DLLEXPORT float GGVR_Chaperone_GetPlayAreaPoint4Z();

//*********************************
//	Device Info Commands
//*********************************
//2017-11-02: Added device info commands
DLLEXPORT char* GGVR_HMD_GetSerialNumber();
DLLEXPORT char* GGVR_HMD_GetTrackingSysName();
DLLEXPORT char* GGVR_HMD_GetModelNumber();
DLLEXPORT char* GGVR_HMD_GetManufacturer();
DLLEXPORT char* GGVR_RCntrl_GetSerialNumber();
DLLEXPORT char* GGVR_RCntrl_GetTrackingSysName();
DLLEXPORT char* GGVR_RCntrl_GetModelNumber();
DLLEXPORT char* GGVR_RCntrl_GetManufacturer();
DLLEXPORT char* GGVR_LCntrl_GetSerialNumber();
DLLEXPORT char* GGVR_LCntrl_GetTrackingSysName();
DLLEXPORT char* GGVR_LCntrl_GetModelNumber();
DLLEXPORT char* GGVR_LCntrl_GetManufacturer();

*/