// pragma
#pragma comment ( lib, "GGVR.lib"     )

// defines
//#define USINGOPENVR

// Generic
int GGVR_ChooseVRSystem ( int iGGVREnabledMode, int iDebuggingActive, LPSTR pAbsPathToDLL );
int GGVR_IsHmdPresent();
void GGVR_SetTrackingSpace( int space );
int GGVR_GetTrackingSpace ( void );
int GGVR_Init ( LPSTR pRootDir, int RImageID, int LImageID, int RCamID, int LCamID, int ObjBase, int ObjHead, int ObjOrigin, int ObjRightHand, int ObjLeftHand, int iObjTeleportStart, int iObjTeleportFinish, int iShaderID, int iTextureID, int iCShaderID, int iCTextureID0, int iCTextureID1, int iCTextureID2, int iCTextureID3, int iCTextureID4, int iCTextureID5, int iCTextureID6);
GGMATRIX GGVR_GetRightEyeProjectionMatrix();
GGMATRIX GGVR_GetLeftEyeProjectionMatrix();
void GGVR_SetWorldScale( float scale );
float GGVR_GetWorldScale(  );
void GGVR_Shutdown();
void GGVR_UpdatePlayer(bool bPlayerDucking, int iTerrainID, int iLMObjStart, int iLMObjFinish, int iEntObjStart, int iEndObjEnd, int iBatchStart, int iBatchEnd );
bool GGVR_HandlePlayerTeleport ( float* pNewPosX, float* pNewPosY, float* pNewPosZ, float* pNewAngleY );
void GGVR_SetPlayerPosition( float X, float Y, float Z );
void GGVR_SetPlayerRotation( float X, float Y, float Z );
void GGVR_SetOriginPosition(float X, float Y, float Z);
void GGVR_SetOriginRotation(float X, float Y, float Z);
void GGVR_MovePlayerLocalX( float valx );
void GGVR_MovePlayerLocalY( float valy );
void GGVR_MovePlayerLocalZ( float valz );
void GGVR_RotatePlayerLocalX( float valx );
void GGVR_SetPlayerAngleY( float valy );
void GGVR_RotatePlayerLocalY( float valy );
void GGVR_RotatePlayerLocalZ( float valz );
void GGVR_LockPlayerTurn( int Lock );
void GGVR_LockPlayerPitch( int Lock );
float GGVR_GetPlayerX( );
float GGVR_GetPlayerY( );
float GGVR_GetPlayerZ( );
float GGVR_GetPlayerAngleX( );
float GGVR_GetPlayerAngleY( );
float GGVR_GetPlayerAngleZ( );
float GGVR_GetOriginX();
float GGVR_GetOriginY();
float GGVR_GetOriginZ();
float GGVR_GetHMDX( );
float GGVR_GetHMDY( );
float GGVR_GetHMDZ( );
float GGVR_GetHMDYaw();
float GGVR_GetHMDPitch();
float GGVR_GetHMDRoll();
float GGVR_GetHMDOffsetX();
float GGVR_GetHMDOffsetY();
float GGVR_GetHMDOffsetZ();
float GGVR_GetHMDAngleX( );
float GGVR_GetHMDAngleY( );
float GGVR_GetHMDAngleZ( );
float GGVR_GetRightHandX( );
float GGVR_GetRightHandY( );
float GGVR_GetRightHandZ( );
float GGVR_GetRightHandAngleX( );
float GGVR_GetRightHandAngleY( );
float GGVR_GetRightHandAngleZ( );
float GGVR_GetLeftHandX( );
float GGVR_GetLeftHandY( );
float GGVR_GetLeftHandZ( );
float GGVR_GetLeftHandAngleX( );
float GGVR_GetLeftHandAngleY( );
float GGVR_GetLeftHandAngleZ( );
float GGVR_GetBestHandX( );
float GGVR_GetBestHandY( );
float GGVR_GetBestHandZ( );
float GGVR_GetBestHandAngleX( );
float GGVR_GetBestHandAngleY( );
float GGVR_GetBestHandAngleZ( );
int GGVR_GetLaserGuidedEntityObj( int entityviewstartobj, int entityviewendobj );

// Generic Controller
int GGVR_RightControllerFound( void );
int GGVR_LeftControllerFound( void );
float GGVR_RightController_JoyX( void );
float GGVR_RightController_JoyY( void );
float GGVR_LeftController_JoyX( void );
float GGVR_LeftController_JoyY( void );
float GGVR_RightController_Trigger( void );
float GGVR_LeftController_Trigger( void );
int GGVR_RightController_Grip( void );
int GGVR_LeftController_Grip( void );
int GGVR_RightController_Button1( void );
int GGVR_LeftController_Button1( void );
int GGVR_RightController_Button2( void );
int GGVR_LeftController_Button2( void );
void GGVR_LeftController_TriggerPulse( int axis, int duration_ms);
void GGVR_RightController_TriggerPulse( int axis, int duration_ms);
int GGVR_RightController_GetFingerPointed( void );
int GGVR_LeftController_GetFingerPointed( void );
int GGVR_RightController_GetThumbUp( void );
int GGVR_LeftController_GetThumbUp( void );
int GGVR_RightController_ButtonPressed(int Button);
int GGVR_LeftController_ButtonPressed(int Button);
float GGVR_RightController_AxisTriggerX(int axis);
float GGVR_RightController_AxisTriggerY(int axis);
float GGVR_LeftController_AxisTriggerX(int axis);
float GGVR_LeftController_AxisTriggerY(int axis);

// WMR Specific
int GGVR_CreateHolographicSpace1 ( HWND hWnd, LPSTR pRootPath );
void GGVR_ReconnectWithHolographicSpaceControllers ( void );
int GGVR_CreateHolographicSpace2 ( void* pDevice, void* pContext );
int GGVR_PreSubmit( int iDebugMode );
void GGVR_SetCameraRange( float Near, float Far );
void GGVR_Submit( void );

// OpenVR Specific
#ifdef USINGOPENVR
int GGVR_IsRuntimeInstalled();
void GGVR_SuspendRendering(int flag);
void GGVR_ErrorMessagesOn(int state );
void GGVR_ResetSeatedZeroPose();
void GGVR_Submit_RightEye();
void GGVR_Submit_LeftEye();
void GGVR_CreateSkyboxOveride(int Front, int Back, int Left, int Right, int Top, int Bottom);
void GGVR_ClearSkyboxOveride();
float GGVR_Chaperone_GetPlayAreaSizeX();
float GGVR_Chaperone_GetPlayAreaSizeZ();
int GGVR_Chaperone_AreBoundsVisible();
void GGVR_Chaperone_ForceBoundsVisible(int flag);
float GGVR_Chaperone_GetPlayAreaPoint1X();
float GGVR_Chaperone_GetPlayAreaPoint1Y();
float GGVR_Chaperone_GetPlayAreaPoint1Z();
float GGVR_Chaperone_GetPlayAreaPoint2X();
float GGVR_Chaperone_GetPlayAreaPoint2Y();
float GGVR_Chaperone_GetPlayAreaPoint2Z();
float GGVR_Chaperone_GetPlayAreaPoint3X();
float GGVR_Chaperone_GetPlayAreaPoint3Y();
float GGVR_Chaperone_GetPlayAreaPoint3Z();
float GGVR_Chaperone_GetPlayAreaPoint4X();
float GGVR_Chaperone_GetPlayAreaPoint4Y();
float GGVR_Chaperone_GetPlayAreaPoint4Z();
char* GGVR_HMD_GetSerialNumber();
char* GGVR_HMD_GetTrackingSysName();
char* GGVR_HMD_GetModelNumber();
char* GGVR_HMD_GetManufacturer();
char* GGVR_RCntrl_GetSerialNumber();
char* GGVR_RCntrl_GetTrackingSysName();
char* GGVR_RCntrl_GetModelNumber();
char* GGVR_RCntrl_GetManufacturer();
char* GGVR_LCntrl_GetSerialNumber();
char* GGVR_LCntrl_GetTrackingSysName();
char* GGVR_LCntrl_GetModelNumber();
char* GGVR_LCntrl_GetManufacturer();
#endif
