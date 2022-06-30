// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DARKLUA_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DARKLUA_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef __DARKLUA_H__
#define __DARKLUA_H__

extern "C" {
#include ".\..\Dark Basic Pro SDK\DarkSDKMore\DarkLUA\lua\lua.h"
#include ".\..\Dark Basic Pro SDK\DarkSDKMore\DarkLUA\lua\lualib.h"
#include ".\..\Dark Basic Pro SDK\DarkSDKMore\DarkLUA\lua\lauxlib.h"
}

/*
#ifdef DARKLUA_EXPORTS
#define DARKLUA_API __declspec(dllexport)
#else
#define DARKLUA_API __declspec(dllimport)
#endif
*/
#define DARKLUA_API 

// This class is exported from the DarkLUA.dll
class DARKLUA_API CDarkLUA {
public:
	CDarkLUA(void);
	// TODO: add your methods here.
};
extern DARKLUA_API int nDarkLUA;
DARKLUA_API int fnDarkLUA(void);

/////////// Dark Lua Functions //////////////////

void LuaConstructor ( void );
void LuaDestructor ( void );
void LuaReceiveCoreDataPtr ( LPVOID pCore );
int LuaSendMessage(lua_State *L);
int LuaSendMessageI(lua_State *L);
int LuaSendMessageF(lua_State *L);
int LuaSendMessageS(lua_State *L);
int AISetEntityControl(lua_State *L);
int AIEntityAssignPatrolPath(lua_State *L);
int AIEntityStop(lua_State *L);
int AIEntityAddTarget(lua_State *L);
int AIEntityRemoveTarget(lua_State *L);
int AIEntityMoveToCover(lua_State *L);
int AIGetEntityCanSee(lua_State *L);
int AIGetEntityCanFire(lua_State *L);
int AIGetEntityViewRange(lua_State *L);
int AIGetEntitySpeed(lua_State *L);
int AIGetTotalPaths(lua_State *L);
int AIGetPathCountPoints(lua_State *L);
int AIPathGetPointX(lua_State *L);
int AIPathGetPointZ(lua_State *L);
int MsgBox(lua_State *L);
int AIEntityGoToPosition(lua_State *L);
int AIGetEntityHeardSound(lua_State *L );
int AIGetEntityIsMoving(lua_State *L );
int AICouldSee(lua_State *L );
int Include(lua_State *L);

//AGK IMAGE and SPRITE COMMANDS
#ifdef LoadImage
#undef LoadImage
#endif
int LoadImage(lua_State *L);
int CreateSprite(lua_State *L);
int SetSpritePosition(lua_State *L);
int SetSpriteSize( lua_State *L );
int GetFreeLUAImageID ( void ); // support function
int GetFreeLUASpriteID ( void ); // support function
void FreeLUASpritesAndImages ( void ); // support
void HideOrShowLUASprites ( bool hide ); // support
int SetSpriteDepth(lua_State *L);
int SetSpriteColor(lua_State *L);
int SetSpriteAngle(lua_State *L);
int SetSpriteOffset(lua_State *L);
int DeleteSprite(lua_State *L);
int SetSpriteImage ( lua_State *L );

int SetFlashLight ( lua_State *L );
int SetAttachmentVisible ( lua_State *L );
int SetOcclusion ( lua_State *L );
int SetPlayerWeapons(lua_State *L);
int FirePlayerWeapon(lua_State *L);
int SetFlashLightKeyEnabled ( lua_State *L );
int SetPlayerRun ( lua_State *L );
int SetFont ( lua_State *L );

int GetDeviceWidth(lua_State *L);
int GetDeviceHeight(lua_State *L);
int GetFirstEntitySpawn(lua_State *L);

void addFunctions();
DARKLUA_API LPSTR LuaMessageDesc ( void );
DARKLUA_API int LuaMessageIndex ();
DARKLUA_API float LuaMessageFloat ();
DARKLUA_API int LuaMessageInt ();
DARKLUA_API LPSTR LuaMessageString ( void );
DARKLUA_API int LuaNext();
DARKLUA_API void SetLuaState ( int id );
DARKLUA_API int LoadLua( LPSTR pString , int id );
DARKLUA_API int LoadLua( LPSTR pString );
DARKLUA_API void LuaSetFunction( LPSTR pString , int id, int params, int results );
DARKLUA_API void LuaSetFunction( LPSTR pString , int params, int results );
DARKLUA_API int LuaValidateEntityTable ( int EntityIndex );
DARKLUA_API void LuaCall();
DARKLUA_API void LuaCallSilent();
DARKLUA_API void CloseLua( int id );
DARKLUA_API void CloseLua();
DARKLUA_API void CloseLuaSilent( int id );
DARKLUA_API void LuaReset ();
DARKLUA_API int LuaExecute ( LPSTR pString , int id );
DARKLUA_API int LuaExecute ( LPSTR pString );
DARKLUA_API int LuaGetInt ( LPSTR pString , int id );
DARKLUA_API int LuaGetInt ( LPSTR pString );
DARKLUA_API int LuaReturnInt ( int id );
DARKLUA_API int LuaReturnInt ( void );
DARKLUA_API float LuaGetFloat ( LPSTR pString , int id );
DARKLUA_API float LuaGetFloat ( LPSTR pString );
DARKLUA_API float LuaReturnFloat ( int id );
DARKLUA_API float LuaReturnFloat ( void );
//DARKLUA_API DWORD LuaGetString ( DWORD pOldString, LPSTR pString , int id );
//DARKLUA_API DWORD LuaGetString ( DWORD pOldString, LPSTR pString );
//DARKLUA_API DWORD LuaReturnString ( DWORD pOldString, int id );
//DARKLUA_API DWORD LuaReturnString ( DWORD pOldString );
DARKLUA_API void LuaSetInt ( LPSTR pString , int value, int id );
DARKLUA_API void LuaSetInt ( LPSTR pString , int value );
DARKLUA_API void LuaPushInt ( int value, int id );
DARKLUA_API void LuaPushInt ( int value );
DARKLUA_API void LuaSetFloat ( LPSTR pString , float value, int id );
DARKLUA_API void LuaSetFloat ( LPSTR pString , float value );
DARKLUA_API void LuaPushFloat ( float value, int id );
DARKLUA_API void LuaPushFloat ( float value );
DARKLUA_API void LuaSetString ( LPSTR pString , LPSTR pStringValue, int id );
DARKLUA_API void LuaSetString ( LPSTR pString , LPSTR pStringValue );
DARKLUA_API void LuaPushString ( LPSTR pStringValue, int id );
DARKLUA_API void LuaPushString ( LPSTR pStringValue );
DARKLUA_API int LuaArrayInt ( LPSTR pString , int id );
DARKLUA_API int LuaArrayInt ( LPSTR pString );
DARKLUA_API float LuaArrayFloat ( LPSTR pString , int id );
DARKLUA_API float LuaArrayFloat ( LPSTR pString );
DARKLUA_API LPSTR LuaArrayString ( LPSTR pString , int id );
DARKLUA_API LPSTR LuaArrayString ( LPSTR pString );

#endif