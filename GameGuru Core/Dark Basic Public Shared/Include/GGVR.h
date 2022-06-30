// pragma
#pragma comment ( lib, "GGVR.lib"     )

// defines
//#define USINGOPENVR

// GGVR Data structure
#ifndef GGVRPLAYERDATA 
#define GGVRPLAYERDATA
class GGVR_PlayerData
{
public:
	int ObjRightHand;
	int ObjLeftHand;
	bool bMakeRightHandInvisible;
	bool bMakeLeftHandInvisible;
	int ObjTeleportStart;
	int ObjTeleportFinish;
	int ShaderID;
	int TextureID;
	int ControllerShaderID;
	int ControllerTextureID0;
	int ControllerTextureID1;
	int ControllerTextureID2;
	int ControllerTextureID3;
	int ControllerTextureID4;
	int ControllerTextureID5;
	int ControllerTextureID6;
	int OculusTex0;
	int Create ( LPSTR pRootDir );
	void Destroy();

	int GetRHandObjID();
	int GetLHandObjID();

	bool PitchLock;
	bool TurnLock;
	int LaserGuideActive;
	float fLaserForwardDistance;
};
#endif

// Generic
int GGVR_ChooseVRSystem ( int iGGVREnabledMode, int iDebuggingActive, LPSTR pAbsPathToDLL );
int GGVR_IsRuntimeFound();
int GGVR_IsHmdPresent();
void GGVR_SetTrackingSpace( int space );
int GGVR_GetTrackingSpace ( void );
int GGVR_Init ( LPSTR pRootDir, int RImageID, int LImageID, int RCamID, int LCamID, int ObjBase, int ObjHead, int ObjOrigin, int ObjRightHand, int ObjLeftHand, int iObjTeleportStart, int iObjTeleportFinish, int iShaderID, int iTextureID, int iCShaderID, int iCTextureID0, int iCTextureID1, int iCTextureID2, int iCTextureID3, int iCTextureID4, int iCTextureID5, int iCTextureID6, int oculusTex0);
void GGVR_ReInit();
void GGVR_SetWorldScale( float scale );
float GGVR_GetWorldScale(  );
void GGVR_Shutdown();
void GGVR_UpdatePlayer(bool bPlayerDucking, int iTerrainID, int iLMObjStart, int iLMObjFinish, int iEntObjStart, int iEndObjEnd, int iBatchStart, int iBatchEnd );
int  GGVR_GetTurnDirection();
bool GGVR_HandlePlayerTeleport ( float* pNewPosX, float* pNewPosY, float* pNewPosZ, float* pNewAngleY );
void GGVR_SetPlayerPosition( float X, float Y, float Z );
void GGVR_SetPlayerRotation( float X, float Y, float Z );
void GGVR_SetPlayerAngleY( float valy );
void GGVR_SetGenericOffsetAngX( float angx );
void GGVR_SetWMROffsetAngX( float angx );
float GGVR_GetPlayerX( );
float GGVR_GetPlayerY( );
float GGVR_GetPlayerZ( );
float GGVR_GetPlayerAngleX( );
float GGVR_GetPlayerAngleY( );
float GGVR_GetPlayerAngleZ( );
void GGVR_SetHMDDirectly(float fX, float fY, float fZ, float fNX, float fNY, float fNZ);
float GGVR_GetHMDX( );
float GGVR_GetHMDY( );
float GGVR_GetHMDZ( );
float GGVR_GetHMDYaw();
float GGVR_GetHMDPitch();
float GGVR_GetHMDRoll();
float GGVR_GetHMDRNormalX();
float GGVR_GetHMDRNormalY();
float GGVR_GetHMDRNormalZ();
float GGVR_GetHMDOffsetX();
float GGVR_GetHMDOffsetY();
float GGVR_GetHMDOffsetZ();
float GGVR_GetHMDAngleX( );
float GGVR_GetHMDAngleY( );
float GGVR_GetHMDAngleZ( );
int GGVR_GetRightHandObject();
int GGVR_GetLeftHandObject();
int GGVR_GetLaserGuideObject();
void GGVR_SetRightHandInvisible(bool bInvisible);
void GGVR_SetLeftHandInvisible(bool bInvisible);
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
void GGVR_SetLaserForwardDistance(float fDist);

// Generic Controller
int GGVR_RightControllerFound( void );
int GGVR_LeftControllerFound( void );
void GGVR_LeftIsBest(bool bEnabled);
float GGVR_BestController_JoyX( void );
float GGVR_BestController_JoyY( void );
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

// OpenXR specific
void GGVR_StartRender( int side );
void GGVR_EndRender();
void GGVR_SetOpenXRValuesForMAX();
void GGVR_SetHandObjectForMAX(int iLeftOrRightHand, float fHandX, float fHandY, float fHandZ, float fHandAngX, float fHandAngY, float fHandAngZ);

int GGVR_PreSubmit( int iDebugMode );
void GGVR_SetCameraRange( float Near, float Far );
void GGVR_Submit( void );
