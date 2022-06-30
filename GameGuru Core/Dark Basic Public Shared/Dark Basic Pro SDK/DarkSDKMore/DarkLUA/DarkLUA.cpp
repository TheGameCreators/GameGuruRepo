// DarkLUA.cpp : Defines the exported functions for the DLL application.
//

//#define FASTBULLETPHYSICS

#define _USING_V110_SDK71_
#include "stdafx.h"
#include "DarkLUA.h"
#include "globstruct.h"
#include "CGfxC.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

//new includes (now donein gameguru.h)
//#include "PhotonCommands.h"
//#include "SteamCommands.h"
//#include "DarkAI.h"
//#include "CTextC.h"
//#include "CImageC.h"
//#include "CFileC.h"
//#include "CSoundC.h"

// DarkLUA needs access to the T global (but could be in two locations)
#include "..\..\..\..\GameGuru\Include\gameguru.h"


// Prototypes
extern void DrawSpritesFirst(void);
extern void DrawSpritesLast(void);

using namespace std;

//#define LUA_DO_DEBUG

#ifdef LUA_DO_DEBUG
void WriteToDebugLog ( char* szMessage, bool bNewLine );
void WriteToDebugLog ( char* szMessage, int i );
void WriteToDebugLog ( char* szMessage, float f );
void WriteToDebugLog ( char* szMessage, char* s );
#endif

#define lua_c

char errorString[256];
char functionName[256];
int functionParams = 0;
int functionResults = 0;
int functionStateID = 0;
int defaultState = 1;

DARKLUA_API int LoadLua( LPSTR pString );
bool LuaCheckForWorkshopFile ( LPSTR VirtualFilename);

 struct StringList
 {
	 int  stateID;
	 char fileName[MAX_PATH];
 };

 std::vector <StringList> ScriptsLoaded;
 std::vector <StringList> FunctionsWithErrors;

// Prototype function
float wrapangleoffset(float da);

// DarkAI Commands =======================================================================
HMODULE DarkAIModule = NULL;
HMODULE MultiplayerModule = NULL;

// externals to get tracking info from VR920 (if any)
extern bool g_VR920AdapterAvailable;
extern float g_fVR920TrackingYaw;
extern float g_fVR920TrackingPitch;
extern float g_fVR920TrackingRoll;
extern float g_fDriverCompensationYaw;
extern float g_fDriverCompensationPitch;
extern float g_fDriverCompensationRoll;

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

 extern GlobStruct* g_pGlob;

 void LuaConstructor ( void )
 {
 }

 void LuaDestructor ( void )
 {
 }

 void LuaReceiveCoreDataPtr ( LPVOID pCore )
 {	
 }


struct luaState
{
	lua_State	*state;
};

lua_State *lua = NULL;

int maxLuaStates = 0;
luaState** ppLuaStates = NULL;

//=============

struct luaMessage
{
	luaMessage() { strcpy ( msgDesc , "" ); msgIndex = 0; msgInt = 0; msgFloat = 0; strcpy ( msgString , "" ); }
	char msgDesc[256];
	int msgIndex;
	int msgInt;
	float msgFloat;
	char msgString[256];
};

luaMessage currentMessage;

int luaMessageCount = 0;
int maxLuaMessages = 0;
luaMessage** ppLuaMessages = NULL;

//=============

 int LuaSendMessage(lua_State *L)
 {
	 lua = L;

	/* get number of arguments */
	int n = lua_gettop(L);
	int i;

	/* loop through each argument */
	for (i = 1; i <= n; i++)
	{
		luaMessageCount++;

		if ( maxLuaMessages == 0 )
		{
			strcpy ( currentMessage.msgDesc, "" );
			currentMessage.msgFloat = 0.0f;
			currentMessage.msgIndex = 0;
			currentMessage.msgInt = 0;
			strcpy ( currentMessage.msgString, "" );

			maxLuaMessages = 100;
			ppLuaMessages = new luaMessage*[maxLuaMessages];

			for ( int c = 0 ; c < maxLuaMessages ; c++ )
				ppLuaMessages[c] = NULL;
		}

		if ( luaMessageCount > maxLuaMessages )
		{
			luaMessage** ppBigger = NULL;
			ppBigger = new luaMessage*[luaMessageCount+100];

			for ( int c = 0; c < luaMessageCount; c++ )
			 ppBigger [ c ] = ppLuaMessages [ c ];

			delete [ ] ppLuaMessages;

			ppLuaMessages = ppBigger;

			for ( int c = maxLuaMessages; c < maxLuaMessages+100; c++ )
				ppLuaMessages[c] = NULL;

			maxLuaMessages += 100;
		}

		if ( ppLuaMessages[luaMessageCount-1] == NULL )
		{
	  
			luaMessage* msg = new luaMessage();

			strcpy ( msg->msgDesc , lua_tostring(L, i) );
			msg->msgIndex = 0;
			msg->msgFloat = 0;
			msg->msgInt = 0;
			strcpy ( msg->msgString , "" );
			ppLuaMessages[luaMessageCount-1] = msg;
		}
	}

	 return 0;
 }

 int LuaSendMessageI(lua_State *L)
 {
	 lua = L;

	/* get number of arguments */
	int n = lua_gettop(L);
	int i;

	if ( n != 2 && n != 3 )
	{
		//MessageBox(NULL, "SendMessageI takes 2 or 3 params", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	luaMessageCount++;

	/* loop through each argument */
	for (i = 1; i <= n; i++)
	{
		if ( maxLuaMessages == 0 )
		{
			strcpy ( currentMessage.msgDesc, "" );
			currentMessage.msgIndex = 0;
			currentMessage.msgFloat = 0.0f;
			currentMessage.msgInt = 0;
			strcpy ( currentMessage.msgString, "" );

			maxLuaMessages = 100;
			ppLuaMessages = new luaMessage*[maxLuaMessages];

			for ( int c = 0 ; c < maxLuaMessages ; c++ )
				ppLuaMessages[c] = NULL;
		}

		if ( luaMessageCount > maxLuaMessages )
		{
			luaMessage** ppBigger = NULL;
			ppBigger = new luaMessage*[luaMessageCount+100];

			for ( int c = 0; c < luaMessageCount; c++ )
			 ppBigger [ c ] = ppLuaMessages [ c ];

			delete [ ] ppLuaMessages;

			ppLuaMessages = ppBigger;

			for ( int c = maxLuaMessages; c < maxLuaMessages+100; c++ )
				ppLuaMessages[c] = NULL;

			maxLuaMessages += 100;
		}

		if ( ppLuaMessages[luaMessageCount-1] == NULL )
		{
	  
			luaMessage* msg = new luaMessage();

			strcpy ( msg->msgDesc , lua_tostring(L, i) );
			msg->msgFloat = 0;
			msg->msgInt = 0;
			msg->msgIndex = 0;
			strcpy ( msg->msgString , "" );
			ppLuaMessages[luaMessageCount-1] = msg;

			/*if ( strcmp ( msg->msgDesc , "setentityhealth" ) == 0 )
			{
				int dave = 1;
			}*/
		}
		else
		{
			if ( n == 3 && i == 2 )
				ppLuaMessages[luaMessageCount-1]->msgIndex = (int)lua_tonumber( L , i );
			else
				ppLuaMessages[luaMessageCount-1]->msgInt = (int)lua_tonumber( L , i );
		}
	}

	 return 0;
 }

  int LuaSendMessageF(lua_State *L)
 {
	 lua = L;

	/* get number of arguments */
	int n = lua_gettop(L);
	int i;

	if ( n != 2 && n != 3 )
	{
		//MessageBox(NULL, "SendMessageI takes 2 or 3 params", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	luaMessageCount++;

	/* loop through each argument */
	for (i = 1; i <= n; i++)
	{

		if ( maxLuaMessages == 0 )
		{
			strcpy ( currentMessage.msgDesc, "" );
			currentMessage.msgFloat = 0.0f;
			currentMessage.msgInt = 0;
			currentMessage.msgIndex = 0;
			strcpy ( currentMessage.msgString, "" );

			maxLuaMessages = 100;
			ppLuaMessages = new luaMessage*[maxLuaMessages];

			for ( int c = 0 ; c < maxLuaMessages ; c++ )
				ppLuaMessages[c] = NULL;
		}

		if ( luaMessageCount > maxLuaMessages )
		{
			luaMessage** ppBigger = NULL;
			ppBigger = new luaMessage*[luaMessageCount+100];

			for ( int c = 0; c < luaMessageCount; c++ )
			 ppBigger [ c ] = ppLuaMessages [ c ];

			delete [ ] ppLuaMessages;

			ppLuaMessages = ppBigger;

			for ( int c = maxLuaMessages; c < maxLuaMessages+100; c++ )
				ppLuaMessages[c] = NULL;

			maxLuaMessages += 100;
		}

		if ( ppLuaMessages[luaMessageCount-1] == NULL )
		{
	  
			luaMessage* msg = new luaMessage();

			strcpy ( msg->msgDesc , lua_tostring(L, i) );
			msg->msgFloat = 0;
			msg->msgInt = 0;
			msg->msgIndex = 0;
			strcpy ( msg->msgString , "" );
			ppLuaMessages[luaMessageCount-1] = msg;
		}
		else
		{

			if ( n == 3 && i == 2 )
				ppLuaMessages[luaMessageCount-1]->msgIndex = (int)lua_tonumber( L , i );
			else
				ppLuaMessages[luaMessageCount-1]->msgFloat = (float)lua_tonumber( L , i );
		}
	}

	 return 0;
 }

 int LuaSendMessageS(lua_State *L)
 {
	 lua = L;

	/* get number of arguments */
	int n = lua_gettop(L);
	int i;

	if ( n != 2 && n != 3 )
	{
		//MessageBox(NULL, "SendMessageI takes 2 or 3 params", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	luaMessageCount++;

	/* loop through each argument */
	for (i = 1; i <= n; i++)
	{

		if ( maxLuaMessages == 0 )
		{
			strcpy ( currentMessage.msgDesc, "" );
			currentMessage.msgFloat = 0.0f;
			currentMessage.msgInt = 0;
			currentMessage.msgIndex = 0;
			strcpy ( currentMessage.msgString, "" );

			maxLuaMessages = 100;
			ppLuaMessages = new luaMessage*[maxLuaMessages];

			for ( int c = 0 ; c < maxLuaMessages ; c++ )
				ppLuaMessages[c] = NULL;
		}

		if ( luaMessageCount > maxLuaMessages )
		{
			luaMessage** ppBigger = NULL;
			ppBigger = new luaMessage*[luaMessageCount+100];

			for ( int c = 0; c < luaMessageCount; c++ )
			 ppBigger [ c ] = ppLuaMessages [ c ];

			delete [ ] ppLuaMessages;

			ppLuaMessages = ppBigger;

			for ( int c = maxLuaMessages; c < maxLuaMessages+100; c++ )
				ppLuaMessages[c] = NULL;

			maxLuaMessages += 100;
		}

		if ( ppLuaMessages[luaMessageCount-1] == NULL )
		{
	  
			luaMessage* msg = new luaMessage();

			strcpy ( msg->msgDesc , lua_tostring(L, i) );
			msg->msgFloat = 0;
			msg->msgInt = 0;
			msg->msgIndex = 0;
			strcpy ( msg->msgString , "" );
			ppLuaMessages[luaMessageCount-1] = msg;
		}
		else
		{
			if ( n == 3 && i == 2 )
				ppLuaMessages[luaMessageCount-1]->msgIndex = (int)lua_tonumber( L , i );
			else
				strcpy ( ppLuaMessages[luaMessageCount-1]->msgString , lua_tostring(L, i) );
		}
	}

	 return 0;
 }

 // Direct Calls
 void lua_updateweaponstats ( void );

 int RestoreGameFromSlot(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	t.luaglobal.gamestatechange = lua_tonumber(L, 1);
	if ( t.luaglobal.gamestatechange==0 )
	{
		// if successfully reset a game-load-state, also ensure advance level loader resets
		strcpy ( t.game.pAdvanceWarningOfLevelFilename, "" );
	}
	return 0;
 }
 int ResetFade(lua_State *L)
 {
	lua = L;
	if ( t.game.gameloop == 1 )
	{
		// only blank if in the game menu (not main menu load page)
		postprocess_reset_fade();
		DisableAllSprites();
		Sync();
		Sync();
	}
	return 0;
 }

 int GetInternalSoundState(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	if (iIndex >= 0 && iIndex < 65535)
	{
		int iState = t.soundloopcheckpoint[iIndex];
		lua_pushinteger ( L, iState );
	}
	return 1;
 }
 int SetInternalSoundState(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	if (iIndex >= 0 && iIndex < 65535)
	{
		int iState = lua_tonumber(L, 2);
		t.soundloopcheckpoint[iIndex] = iState;
		if (t.soundloopcheckpoint[iIndex] != 0)
		{
			if (iIndex > 0 && SoundExist(iIndex) == 1)
			{
				if (t.soundloopcheckpoint[iIndex] == 3)
					LoopSound(iIndex);
				else if (t.soundloopcheckpoint[iIndex] == 1)
					PlaySound(iIndex);
			}
		}
		return 1;
	}
	return 0;
 }
 int SetCheckpoint(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	t.playercheckpoint.x=lua_tonumber(L, 1);
	t.playercheckpoint.y=lua_tonumber(L, 2);
	t.playercheckpoint.z=lua_tonumber(L, 3);
	t.playercheckpoint.a=lua_tonumber(L, 4);
	return 1;
 }

 int UpdateWeaponStats(lua_State *L)
 {
	lua = L;
	lua_updateweaponstats();
	return 0;
 }
  
 int ResetWeaponSystems(lua_State *L)
 {
	weapon_projectile_free();
	return 0;
 }
 int GetWeaponSlotGot(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	lua_pushinteger ( L, t.weaponslot[iIndex].got );
	return 1;
 }
 int GetWeaponSlotNoSelect(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	lua_pushinteger ( L, t.weaponslot[iIndex].noselect );
	return 1;
 }
 int SetWeaponSlot(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	t.weaponslot[iIndex].got = lua_tonumber(L, 2);
	t.weaponslot[iIndex].pref = lua_tonumber(L, 3);
	return 0;
 }
 int GetWeaponAmmo(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	lua_pushinteger ( L, t.weaponammo[iIndex] );
	return 1;
 }
 int SetWeaponAmmo(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	t.weaponammo[iIndex] = lua_tonumber(L, 2);
	return 0;
 }
 int GetWeaponClipAmmo(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	lua_pushinteger ( L, t.weaponclipammo[iIndex] );
	return 1;
 }
 int SetWeaponClipAmmo(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	t.weaponclipammo[iIndex] = lua_tonumber(L, 2);
	return 0;
 }
 int GetWeaponPoolAmmo(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	lua_pushinteger ( L, t.ammopool[iIndex].ammo );
	return 1;
 }
 int SetWeaponPoolAmmo(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	t.ammopool[iIndex].ammo = lua_tonumber(L, 2);
	return 0;
 }
 int GetWeaponSlot(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;

	// returns the gunID
	int iReturnValue = 0;

	// find gun name specicied to get gunindex
	int iSlotIndex = lua_tonumber(L, 1);
	iReturnValue = t.weaponslot[iSlotIndex].got;

	// return true GunID found in slot
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }

 // Weapon Modding Commands
 int GetPlayerWeaponID(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n > 0 ) return 0;

	// returns the playres current gun ID
	int iReturnValue = t.gunid;
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetWeaponID(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;

	// returns the gun
	int iReturnValue = 0;

	// find gun name specicied to get gunindex
	char pGunName[512];
	strcpy ( pGunName, lua_tostring(L, 1) );
	for ( int tgunid = 1; tgunid < ArrayCount(t.gun); tgunid++ )
	{
		if ( stricmp ( t.gun[tgunid].name_s.Get()+(strlen(t.gun[tgunid].name_s.Get())-strlen(pGunName)), pGunName ) == NULL )
		{
			iReturnValue = tgunid;
			break;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetEntityWeaponID(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	if ( iEntityIndex > 0 )
	{
		int iEntityBankIndex = t.entityelement[iEntityIndex].bankindex;
		if ( iEntityBankIndex > 0 )
		{
			iReturnValue = t.entityprofile[iEntityBankIndex].isweapon;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int RawSetWeaponData ( lua_State *L, int iDataMode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	int tgunid = lua_tonumber(L, 1);
	int tfiremode = lua_tonumber(L, 2);
	int newvalue = lua_tonumber(L, 3);
	if ( tgunid > 0 && tgunid <= ArrayCount(t.gun) )
	{
		switch ( iDataMode )
		{
			case 1 : g.firemodes[tgunid][tfiremode].settings.damage = newvalue; break;
			case 2 : g.firemodes[tgunid][tfiremode].settings.accuracy = newvalue; break;
			case 3 : g.firemodes[tgunid][tfiremode].settings.reloadqty = newvalue; break;
			case 4 : g.firemodes[tgunid][tfiremode].settings.iterate = newvalue; break;
			case 5 : g.firemodes[tgunid][tfiremode].settings.range = newvalue; break;
			case 6 : g.firemodes[tgunid][tfiremode].settings.dropoff = newvalue; break;
			case 7 : g.firemodes[tgunid][tfiremode].settings.usespotlighting = newvalue; break;
		}
	}
	return 0;
 }
 int RawGetWeaponData( lua_State *L, int iDataMode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;

	// specify weaponID and firemode index
	int tgunid = lua_tonumber(L, 1);
	int tfiremode = lua_tonumber(L, 2);

	// returns the field data
	int iReturnValue = 0;

	// use datamode to determine which data is returned
	if ( tgunid > 0 && tgunid <= ArrayCount(t.gun) )
	{
		switch ( iDataMode )
		{
			case 1 : iReturnValue = g.firemodes[tgunid][tfiremode].settings.damage; break;
			case 2 : iReturnValue = g.firemodes[tgunid][tfiremode].settings.accuracy; break;
			case 3 : iReturnValue = g.firemodes[tgunid][tfiremode].settings.reloadqty; break;
			case 4 : iReturnValue = g.firemodes[tgunid][tfiremode].settings.iterate; break;
			case 5 : iReturnValue = g.firemodes[tgunid][tfiremode].settings.range; break;
			case 6 : iReturnValue = g.firemodes[tgunid][tfiremode].settings.dropoff; break;
			case 7 : iReturnValue = g.firemodes[tgunid][tfiremode].settings.usespotlighting; break;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int SetWeaponDamage(lua_State *L) { return RawSetWeaponData ( L, 1 ); }
 int SetWeaponAccuracy(lua_State *L) { return RawSetWeaponData ( L, 2 ); }
 int SetWeaponReloadQuantity(lua_State *L) { return RawSetWeaponData ( L, 3 ); }
 int SetWeaponFireIterations(lua_State *L) { return RawSetWeaponData ( L, 4 ); }
 int SetWeaponRange(lua_State *L) { return RawSetWeaponData ( L, 5 ); }
 int SetWeaponDropoff(lua_State *L) { return RawSetWeaponData ( L, 6 ); }
 int SetWeaponSpotLighting(lua_State *L) { return RawSetWeaponData ( L, 7 ); }
 int GetWeaponDamage(lua_State *L) { return RawGetWeaponData ( L, 1 ); }
 int GetWeaponAccuracy(lua_State *L) { return RawGetWeaponData ( L, 2 ); }
 int GetWeaponReloadQuantity(lua_State *L) { return RawGetWeaponData ( L, 3 ); }
 int GetWeaponFireIterations(lua_State *L) { return RawGetWeaponData ( L, 4 ); }
 int GetWeaponRange(lua_State *L) { return RawGetWeaponData ( L, 5 ); }
 int GetWeaponDropoff(lua_State *L) { return RawGetWeaponData ( L, 6 ); }
 int GetWeaponSpotLighting(lua_State *L) { return RawGetWeaponData ( L, 7 ); }

 //
 // Player Camera Overrides
 //
 int RawSetCameraData ( lua_State *L, int iDataMode )
 {
	lua = L;
	int n = lua_gettop(L);
	int tcameraid=0, tvalue=0;
	float fX=0, fY=0, fZ=0;
	if ( iDataMode < 11 )
	{
		if ( n < 1 ) return 0;
		tvalue = lua_tonumber(L, 1);
	}
	else
	{
		if ( n < 4 ) return 0;
		tcameraid = lua_tonumber(L, 1);
		fX = lua_tonumber(L, 2);
		fY = lua_tonumber(L, 3);
		fZ = lua_tonumber(L, 4);
	}
	switch ( iDataMode )
	{
		case 1 : g.luacameraoverride = tvalue; break;
		case 11 : if ( tcameraid == 0 ) { PositionCamera ( tcameraid, fX, fY, fZ ); } break;
		case 12 : if ( tcameraid == 0 ) 
		{ 
			RotateCamera ( tcameraid, fX, fY, fZ ); 
		} 
		break;
		case 13 : if ( tcameraid == 0 ) 
		{
			RotateCamera ( tcameraid, 0, 0, 0 );
			RollCameraRight ( tcameraid, fZ );
			TurnCameraRight ( tcameraid, fY );
			PitchCameraUp ( tcameraid, fX );
		} 
		break;
	}
	return 0;
 }
 int RawGetCameraData( lua_State *L, int iDataMode )
 {
	lua = L;
	int n = lua_gettop(L);
	int tcameraid = 0;
	if ( iDataMode < 500 )
	{
		if ( n < 1 ) return 0;
		tcameraid = lua_tonumber(L, 1);
	}
	float fReturnValue = 0;
	int iReturnValue = 0;
	if ( tcameraid == 0 )
	{
		switch ( iDataMode )
		{
			case 1 : fReturnValue = CameraPositionX ( tcameraid ); break;
			case 2 : fReturnValue = CameraPositionY ( tcameraid ); break;
			case 3 : fReturnValue = CameraPositionZ ( tcameraid ); break;
			case 4 : fReturnValue = CameraAngleX ( tcameraid ); break;
			case 5 : fReturnValue = CameraAngleY ( tcameraid ); break;
			case 6 : fReturnValue = CameraAngleZ ( tcameraid ); break;
			case 501 : iReturnValue = g.luacameraoverride; break;
		}
	}
	if ( iDataMode < 500 )
		lua_pushnumber ( L, fReturnValue );
	else
		lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int WrapAngle(lua_State *L) 
 { 
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	float fAngle = wrapangleoffset(lua_tonumber(L, 1));
	float fDestAngle = wrapangleoffset(lua_tonumber(L, 2));
	float fSmoothFactor = lua_tonumber(L, 3);
	float fReturnValue = fAngle;
	float fDiff = (fDestAngle-fAngle);
	if ( fDiff > 180.0f ) fDiff -= 360.0f;
	if ( fDiff < -180.0f ) fDiff += 360.0f;
	fReturnValue += fDiff*fSmoothFactor;
	lua_pushinteger ( L, fReturnValue );
	return 1;
 }
 int GetCameraOverride(lua_State *L) { return RawGetCameraData ( L, 501 ); }
 int SetCameraOverride(lua_State *L) { return RawSetCameraData ( L, 1 ); }
 int SetCameraPosition(lua_State *L) { return RawSetCameraData ( L, 11 ); }
 int SetCameraAngle(lua_State *L) { return RawSetCameraData ( L, 12 ); }
 int SetCameraFreeFlight(lua_State *L) { return RawSetCameraData ( L, 13 ); }
 int GetCameraPositionX(lua_State *L) { return RawGetCameraData ( L, 1 ); }
 int GetCameraPositionY(lua_State *L) { return RawGetCameraData ( L, 2 ); }
 int GetCameraPositionZ(lua_State *L) { return RawGetCameraData ( L, 3 ); }
 int GetCameraAngleX(lua_State *L) { return RawGetCameraData ( L, 4 ); }
 int GetCameraAngleY(lua_State *L) { return RawGetCameraData ( L, 5 ); }
 int GetCameraAngleZ(lua_State *L) { return RawGetCameraData ( L, 6 ); }

 int SetCameraFOV ( lua_State *L )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iCameraIndex = lua_tonumber(L, 1);
	float fCameraFOV = lua_tonumber(L, 2);
	SetCameraFOV ( iCameraIndex, fCameraFOV );
	return 0;
 }

 //
 // Player Direct commands
 //
 int RawSetPlayerData ( lua_State *L, int iDataMode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( iDataMode == 1 )
	{
 		//  apply force to push player
		if ( n < 2 ) return 0;
		float fAngle = lua_tonumber(L, 1);
		float fForce = lua_tonumber(L, 2);
		t.playercontrol.pushangle_f = fAngle;
		t.playercontrol.pushforce_f = fForce;
	}
	return 0;
 }
 int ForcePlayer(lua_State *L) { return RawSetPlayerData ( L, 1 ); }

 //
 // All SET & GET Functions which replaces g_Entity[e] tables
 //
 /*
 int SetEntityLUACore ( lua_State *L, int iCode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	switch ( iCode )
	{
		case 1 : t.entityelement[iIndex].luadata.x = lua_tonumber(L, 2); break;
		case 2 : t.entityelement[iIndex].luadata.y = lua_tonumber(L, 2); break;
		case 3 : t.entityelement[iIndex].luadata.z = lua_tonumber(L, 2); break;
		case 4 : t.entityelement[iIndex].luadata.anglex = lua_tonumber(L, 2); break;
		case 5 : t.entityelement[iIndex].luadata.angley = lua_tonumber(L, 2); break;
		case 6 : t.entityelement[iIndex].luadata.anglez = lua_tonumber(L, 2); break;
		case 7 : t.entityelement[iIndex].luadata.obj = lua_tonumber(L, 2); break;
		case 8 : t.entityelement[iIndex].luadata.active = lua_tonumber(L, 2); break;
		case 9 : t.entityelement[iIndex].luadata.activated = lua_tonumber(L, 2); break;
		case 10 : t.entityelement[iIndex].luadata.collected = lua_tonumber(L, 2); break;
		case 11 : t.entityelement[iIndex].luadata.haskey = lua_tonumber(L, 2); break;
		case 12 : t.entityelement[iIndex].luadata.plrinzone = lua_tonumber(L, 2); break;
		case 13 : t.entityelement[iIndex].luadata.entityinzone = lua_tonumber(L, 2); break;
		case 14 : t.entityelement[iIndex].luadata.plrvisible = lua_tonumber(L, 2); break;
		case 15 : t.entityelement[iIndex].luadata.health = lua_tonumber(L, 2); break;
		case 16 : t.entityelement[iIndex].luadata.frame = lua_tonumber(L, 2); break;
		case 17 : t.entityelement[iIndex].luadata.timer = lua_tonumber(L, 2); break;
		case 18 : t.entityelement[iIndex].luadata.plrdist = lua_tonumber(L, 2); break;
		case 19 : t.entityelement[iIndex].luadata.avoid = lua_tonumber(L, 2); break;
		case 20 : t.entityelement[iIndex].luadata.limbhit = lua_tostring(L, 2); break;
		case 21 : t.entityelement[iIndex].luadata.limbhitindex = lua_tonumber(L, 2); break;
		case 22 : t.entityelement[iIndex].luadata.animating = lua_tonumber(L, 2); break;
	}
	return 0;
 }
 int GetEntityLUACore ( lua_State *L, int iCode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	if ( iIndex > 0 )
	{
		switch ( iCode )
		{
			case 1 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.x ); break;
			case 2 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.y ); break;
			case 3 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.z ); break;
			case 4 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.anglex ); break;
			case 5 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.angley ); break;
			case 6 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.anglez ); break;
			case 7 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.obj ); break;
			case 8 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.active ); break;
			case 9 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.activated ); break;
			case 10 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.collected ); break;
			case 11 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.haskey ); break;
			case 12 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.plrinzone ); break;
			case 13 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.entityinzone ); break;
			case 14 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.plrvisible ); break;
			case 15 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.health ); break;
			case 16 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.frame ); break;
			case 17 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.timer ); break;
			case 18 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.plrdist ); break;
			case 19 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.avoid ); break;
			case 20 : lua_pushstring ( L, t.entityelement[iIndex].luadata.limbhit.Get() ); break;
			case 21 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.limbhitindex ); break;
			case 22 : lua_pushinteger ( L, t.entityelement[iIndex].luadata.animating ); break;
		}
		return 1;
	}
	else
	{
		// return zero if no index specified (error)
		return 0;
	}
 }

 int SetEntityLUAX(lua_State *L) { return SetEntityLUACore ( L, 1 ); }
 int GetEntityLUAX(lua_State *L) { return GetEntityLUACore ( L, 1 ); }
 int SetEntityLUAY(lua_State *L) { return SetEntityLUACore ( L, 2 ); }
 int GetEntityLUAY(lua_State *L) { return GetEntityLUACore ( L, 2 ); }
 int SetEntityLUAZ(lua_State *L) { return SetEntityLUACore ( L, 3 ); }
 int GetEntityLUAZ(lua_State *L) { return GetEntityLUACore ( L, 3 ); }
 int SetEntityLUAAngleX(lua_State *L) { return SetEntityLUACore ( L, 4 ); }
 int GetEntityLUAAngleX(lua_State *L) { return GetEntityLUACore ( L, 4 ); }
 int SetEntityLUAAngleY(lua_State *L) { return SetEntityLUACore ( L, 5 ); }
 int GetEntityLUAAngleY(lua_State *L) { return GetEntityLUACore ( L, 5 ); }
 int SetEntityLUAAngleZ(lua_State *L) { return SetEntityLUACore ( L, 6 ); }
 int GetEntityLUAAngleZ(lua_State *L) { return GetEntityLUACore ( L, 6 ); }
 int SetEntityLUAObj(lua_State *L) { return SetEntityLUACore ( L, 7 ); }
 int GetEntityLUAObj(lua_State *L) { return GetEntityLUACore ( L, 7 ); }
 int SetEntityLUAActive(lua_State *L) { return SetEntityLUACore ( L, 8 ); }
 int GetEntityLUAActive(lua_State *L) { return GetEntityLUACore ( L, 8 ); }
 int SetEntityLUAActivated(lua_State *L) { return SetEntityLUACore ( L, 9 ); }
 int GetEntityLUAActivated(lua_State *L) { return GetEntityLUACore ( L, 9 ); }
 int SetEntityLUACollected(lua_State *L) { return SetEntityLUACore ( L, 10 ); }
 int GetEntityLUACollected(lua_State *L) { return GetEntityLUACore ( L, 10 ); }
 int SetEntityLUAHasKey(lua_State *L) { return SetEntityLUACore ( L, 11 ); }
 int GetEntityLUAHasKey(lua_State *L) { return GetEntityLUACore ( L, 11 ); }
 int SetEntityLUAPlrInZone(lua_State *L) { return SetEntityLUACore ( L, 12 ); }
 int GetEntityLUAPlrInZone(lua_State *L) { return GetEntityLUACore ( L, 12 ); }
 int SetEntityLUAEntityInZone(lua_State *L) { return SetEntityLUACore ( L, 13 ); }
 int GetEntityLUAEntityInZone(lua_State *L) { return GetEntityLUACore ( L, 13 ); }
 int SetEntityLUAPlrVisible(lua_State *L) { return SetEntityLUACore ( L, 14 ); }
 int GetEntityLUAPlrVisible(lua_State *L) { return GetEntityLUACore ( L, 14 ); }
 int SetEntityLUAAnimating(lua_State *L) { return SetEntityLUACore ( L, 22 ); }
 int GetEntityLUAAnimating(lua_State *L) { return GetEntityLUACore ( L, 22 ); }
 int SetEntityLUAHealth(lua_State *L) { return SetEntityLUACore ( L, 15 ); }
 int GetEntityLUAHealth(lua_State *L) { return GetEntityLUACore ( L, 15 ); }
 int SetEntityLUAFrame(lua_State *L) { return SetEntityLUACore ( L, 16 ); }
 int GetEntityLUAFrame(lua_State *L) { return GetEntityLUACore ( L, 16 ); }
 int SetEntityLUATimer(lua_State *L) { return SetEntityLUACore ( L, 17 ); }
 int GetEntityLUATimer(lua_State *L) { return GetEntityLUACore ( L, 17 ); }
 int SetEntityLUAPlrDist(lua_State *L) { return SetEntityLUACore ( L, 18 ); }
 int GetEntityLUAPlrDist(lua_State *L) { return GetEntityLUACore ( L, 18 ); }
 int SetEntityLUAAvoid(lua_State *L) { return SetEntityLUACore ( L, 19 ); }
 int GetEntityLUAAvoid(lua_State *L) { return GetEntityLUACore ( L, 19 ); }
 int SetEntityLUALimbHit(lua_State *L) { return SetEntityLUACore ( L, 20 ); }
 int GetEntityLUALimbHit(lua_State *L) { return GetEntityLUACore ( L, 20 ); }
 int SetEntityLUALimbHitIndex(lua_State *L) { return SetEntityLUACore ( L, 21 ); }
 int GetEntityLUALimbHitIndex(lua_State *L) { return GetEntityLUACore ( L, 21 ); }
 */

 // Direct Entity Element Instructions (different from LUA DATA instructions above)

 int SetEntityActive(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	int iSetThisValue = lua_tonumber(L, 2);
	t.entityelement[iIndex].active = iSetThisValue;
	return 0;
 }
 int SetEntityActivated(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	t.entityelement[iIndex].activated = lua_tonumber(L, 2);
	return 0;
 }
 int SetEntityCollected(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	t.entityelement[iIndex].collected = lua_tonumber(L, 2);
	return 0;
 }
 int SetEntityHasKey(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	t.entityelement[iIndex].lua.haskey = lua_tonumber(L, 2);
	return 0;
 }
 int GetEntityActive(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iIndex = lua_tonumber(L, 1);
	int iReturnValue = 0;
	if ( iIndex > 0 )
	{
		iReturnValue = t.entityelement[iIndex].active;
		if ( Len(t.entityelement[iIndex].eleprof.aimainname_s.Get())>1 ) 
			if ( t.entityelement[iIndex].eleprof.aimain == 0 ) 
				iReturnValue = 0;
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetEntityVisibility(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	if ( iEntityIndex > 0 )
	{
		int iObjectNumber = t.entityelement[iEntityIndex].obj;
		if ( iObjectNumber > 0 )
		{
			sObject* pObject = GetObjectData ( iObjectNumber );
			if ( pObject && pObject->bVisible == true )
				iReturnValue = 1;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int SetEntitySpawnAtStart(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iEntityIndex = lua_tonumber(L, 1);
	t.entityelement[iEntityIndex].eleprof.spawnatstart = lua_tonumber(L, 2);
	return 0;
 }
 int GetEntitySpawnAtStart(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	if ( iEntityIndex > 0 ) iReturnValue = t.entityelement[iEntityIndex].eleprof.spawnatstart;
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetEntityFilePath(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	char pReturnValue[1024];
	strcpy ( pReturnValue, "" );
	int iEntityIndex = lua_tonumber(L, 1);
	if ( iEntityIndex > 0 ) 
	{
		int iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 )
		{
			strcpy ( pReturnValue, t.entitybank_s[iEntID].Get() );
		}
	}
	lua_pushstring ( L, pReturnValue );
	return 1;
 }

 int SetPreExitValue(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iEntityIndex = lua_tonumber(L, 1);
	t.entityelement[iEntityIndex].eleprof.aipreexit = lua_tonumber(L, 2);
	return 0;
 }

 int RawSetEntityData ( lua_State *L, int iDataMode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iEntityIndex = lua_tonumber(L, 1);
	float fValue = lua_tonumber(L, 2);
	switch ( iDataMode )
	{
		case 12 : // 120417 - change anim speed mod, and character speed if a character
		{
			if ( t.entityelement[iEntityIndex].animspeedmod != fValue )
			{
				t.entityelement[iEntityIndex].animspeedmod = fValue; 
				float fFinalAnimSpeed = t.entityelement[iEntityIndex].eleprof.animspeed * t.entityelement[iEntityIndex].animspeedmod;
				t.e = iEntityIndex;
				entity_lua_findcharanimstate ( );
				if ( t.tcharanimindex != -1 ) t.charanimstates[t.tcharanimindex].animationspeed_f = (65.0f/100.0f) * fFinalAnimSpeed;
			}
			break;
		}
	}
	return 0;
 }

 int GetEntityData ( lua_State *L, int iDataMode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( iDataMode == 19 )
	{
		if ( n < 4 ) return 0;
	}
	else
	{
		if ( n < 1 ) return 0;
	}
	int iEntityIndex = lua_tonumber(L, 1);
	if ( iEntityIndex > 0 )
	{
		if ( iDataMode < 101 )
		{
			float fReturnValue = 0;
			int iObjectNumber = t.entityelement[iEntityIndex].obj;
			if ( iObjectNumber > 0 )
			{
				sObject* pObject = GetObjectData ( iObjectNumber );
				if ( pObject )
				{
					switch ( iDataMode )
					{
						case 1 : fReturnValue = pObject->position.vecPosition.x; break;
						case 2 : fReturnValue = pObject->position.vecPosition.y; break;
						case 3 : fReturnValue = pObject->position.vecPosition.z; break;
						case 4 : fReturnValue = pObject->position.vecRotate.x; break;
						case 5 : fReturnValue = pObject->position.vecRotate.y; break;
						case 6 : fReturnValue = pObject->position.vecRotate.z; break;
						case 11 : fReturnValue = t.entityelement[iEntityIndex].eleprof.animspeed/100.0f; break;
						case 12 : fReturnValue = t.entityelement[iEntityIndex].animspeedmod; break;
						case 13 : 
						{
							// work out delta from last position and current position
							float fDX = t.entityelement[iEntityIndex].x - t.entityelement[iEntityIndex].lastx;
							float fDY = t.entityelement[iEntityIndex].y - t.entityelement[iEntityIndex].lasty;
							float fDZ = t.entityelement[iEntityIndex].z - t.entityelement[iEntityIndex].lastz;
							fReturnValue = sqrt ( fabs(fDX*fDX)+fabs(fDY*fDY)+fabs(fDZ*fDZ) );
							break;
						}
						case 14 : 
						{
							// Return collision box coordinates (6 values )
							lua_pushnumber( L, pObject->collision.vecMin.x );
							lua_pushnumber( L, pObject->collision.vecMin.y );
							lua_pushnumber( L, pObject->collision.vecMin.x );
							lua_pushnumber( L, pObject->collision.vecMax.x );
							lua_pushnumber( L, pObject->collision.vecMax.y );
							lua_pushnumber( L, pObject->collision.vecMax.z );
							return 6;
						}
						case 15 :
						{
							// Position and Angle together (6 values )
							lua_pushnumber( L, pObject->position.vecPosition.x );
							lua_pushnumber( L, pObject->position.vecPosition.y );
							lua_pushnumber( L, pObject->position.vecPosition.z );
							lua_pushnumber( L, pObject->position.vecRotate.x );
							lua_pushnumber( L, pObject->position.vecRotate.y );
							lua_pushnumber( L, pObject->position.vecRotate.z );
							return 6;
						}
						case 16 : fReturnValue = t.entityelement[iEntityIndex].eleprof.phyweight; break;
						case 17 : 
						{
							// Scale factors 3 values
							lua_pushnumber( L, pObject->position.vecScale.x );
							lua_pushnumber( L, pObject->position.vecScale.y );
							lua_pushnumber( L, pObject->position.vecScale.z );
							return 3;
						}
						case 18 : 
						{
							lua_pushstring(L, t.entityelement[iEntityIndex].eleprof.name_s.Get() );
							return 1;
						}
						case 19 : 
						{
							// as above, but done manually with no outside assistance from neighboring systems
							float fThisPosX = lua_tonumber(L, 2);
							float fThisPosY = lua_tonumber(L, 3);
							float fThisPosZ = lua_tonumber(L, 4);
							float fDX = fThisPosX - t.entityelement[iEntityIndex].customlastx;
							float fDY = fThisPosY - t.entityelement[iEntityIndex].customlasty;
							float fDZ = fThisPosZ - t.entityelement[iEntityIndex].customlastz;
							t.entityelement[iEntityIndex].customlastx = fThisPosX;
							t.entityelement[iEntityIndex].customlasty = fThisPosY;
							t.entityelement[iEntityIndex].customlastz = fThisPosZ;
							fReturnValue = sqrt ( fabs(fDX*fDX)+fabs(fDY*fDY)+fabs(fDZ*fDZ) );
							break;
						}
						case 20: fReturnValue = (float)t.entityelement[iEntityIndex].eleprof.speed / 100.0f; break;
						case 21: 
						{
							int entid = t.entityelement[iEntityIndex].bankindex;
							fReturnValue = (float)t.entityprofile[entid].ismarker;
							break;
						}
					}
				}
			}
			lua_pushnumber ( L, fReturnValue );
		}
		else
		{
			int iReturnValue = 0;
			switch ( iDataMode )
			{
				case 101: 
				{
					break;
				}
			}
			lua_pushnumber ( L, iReturnValue );
		}
	}
	else
	{
		lua_pushnumber ( L, 0 );
	}
	return 1;
 }

 int GetEntityPositionX(lua_State *L) { return GetEntityData ( L, 1 ); }
 int GetEntityPositionY(lua_State *L) { return GetEntityData ( L, 2 ); }
 int GetEntityPositionZ(lua_State *L) { return GetEntityData ( L, 3 ); }
 int GetEntityAngleX(lua_State *L) { return GetEntityData ( L, 4 ); }
 int GetEntityAngleY(lua_State *L) { return GetEntityData ( L, 5 ); }
 int GetEntityAngleZ(lua_State *L) { return GetEntityData ( L, 6 ); }
 int GetMovementSpeed(lua_State *L) { return GetEntityData(L, 20); }
 int GetAnimationSpeed(lua_State *L) { return GetEntityData ( L, 11 ); }
 int SetAnimationSpeedModulation(lua_State *L) { return RawSetEntityData ( L, 12 ); }
 int GetAnimationSpeedModulation(lua_State *L) { return GetEntityData ( L, 12 ); }
 int GetMovementDelta(lua_State *L) { return GetEntityData ( L, 13 ); }
 int GetEntityCollBox(lua_State *L) { return GetEntityData ( L, 14 ); }
 int GetEntityPosAng(lua_State *L)  { return GetEntityData ( L, 15 ); }
 int GetEntityWeight(lua_State *L)  { return GetEntityData ( L, 16 ); }
 int GetEntityScales(lua_State *L)  { return GetEntityData ( L, 17 ); }
 int GetEntityName(lua_State *L)    { return GetEntityData ( L, 18 ); }
 int GetMovementDeltaManually(lua_State *L) { return GetEntityData ( L, 19 ); }
 int GetEntityMarkerMode(lua_State *L) { return GetEntityData (L, 21); }




 int SetEntityString(lua_State *L)
 {
	lua = L;
	int n = lua_gettop( L );
	if ( n < 3 ) return 0;
	bool setSound = ( n == 4 ) && lua_tonumber( L, 4 ) == 1;
	int iReturnValue = 0;
	int iEntityIndex    = lua_tonumber( L, 1 );
	int iSlotIndex      = lua_tonumber( L, 2 );
	const char* pString = lua_tostring( L, 3 );
	if ( iEntityIndex > 0 )
	{
		if ( iSlotIndex == 0 ) {
			t.entityelement[ iEntityIndex ].eleprof.soundset_s = pString;
			if ( setSound )
			{
				if ( t.entityelement[ iEntityIndex ].soundset > 0 ) deleteinternalsound( t.entityelement[ iEntityIndex ].soundset );
				t.entityelement[ iEntityIndex ].soundset =
					loadinternalsoundcore( t.entityelement[ iEntityIndex ].eleprof.soundset_s.Get(), 1 );
			}
		}
		if ( iSlotIndex == 1 )
		{
			t.entityelement[ iEntityIndex ].eleprof.soundset1_s = pString;
			if ( setSound )
			{
				if ( t.entityelement[ iEntityIndex ].soundset1 > 0 ) deleteinternalsound( t.entityelement[ iEntityIndex ].soundset1 );
				t.entityelement[ iEntityIndex ].soundset1 =
					loadinternalsoundcore( t.entityelement[ iEntityIndex ].eleprof.soundset1_s.Get(), 1 );
			}
		}
		if ( iSlotIndex == 2 ) 
		{ 
			t.entityelement[ iEntityIndex ].eleprof.soundset2_s = pString; 
			if ( setSound )
			{
				if ( t.entityelement[ iEntityIndex ].soundset2 > 0 ) deleteinternalsound( t.entityelement[ iEntityIndex ].soundset2 );
				t.entityelement[ iEntityIndex ].soundset2 =
					loadinternalsoundcore( t.entityelement[ iEntityIndex ].eleprof.soundset2_s.Get(), 1 );
			}
		}
		if ( iSlotIndex == 3 ) 
		{ 
			t.entityelement[ iEntityIndex ].eleprof.soundset3_s = pString; 
			if ( setSound )
			{
				if ( t.entityelement[ iEntityIndex ].soundset3 > 0 ) deleteinternalsound( t.entityelement[ iEntityIndex ].soundset3 );
				t.entityelement[ iEntityIndex ].soundset3 =
					loadinternalsoundcore( t.entityelement[ iEntityIndex ].eleprof.soundset3_s.Get(), 1 );
			}
		}
		t.entityelement[iEntityIndex].soundset4 = 0;
		/* no longer using this slot for sounds, now for DLUA!
		if ( iSlotIndex == 4 )
		{
			t.entityelement[ iEntityIndex ].eleprof.soundset4_s = pString; 
			if ( setSound )
			{
				if ( t.entityelement[ iEntityIndex ].soundset4 > 0 ) deleteinternalsound( t.entityelement[ iEntityIndex ].soundset4 );
				t.entityelement[ iEntityIndex ].soundset4 =
					loadinternalsoundcore( t.entityelement[ iEntityIndex ].eleprof.soundset4_s.Get(), 1 );
			}
		}
		*/
	}
	return 0;
 }
 int GetEntityString(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	int iSlotIndex = lua_tonumber(L, 2);
	LPSTR pString = "";
	if ( iEntityIndex > 0 )
	{
		if ( iSlotIndex == 0 ) pString = t.entityelement[iEntityIndex].eleprof.soundset_s.Get();
		if ( iSlotIndex == 1 ) pString = t.entityelement[iEntityIndex].eleprof.soundset1_s.Get();
		if ( iSlotIndex == 2 ) pString = t.entityelement[iEntityIndex].eleprof.soundset2_s.Get();
		if ( iSlotIndex == 3 ) pString = t.entityelement[iEntityIndex].eleprof.soundset3_s.Get();
		if ( iSlotIndex == 4 ) pString = t.entityelement[iEntityIndex].eleprof.soundset4_s.Get();
	}
	lua_pushstring ( L, pString );
	return 1;
 }
 int GetLimbName(lua_State *L)
 {
	 lua = L;
	 int n = lua_gettop(L);
	 if (n < 2) return 0;
	 int iID = lua_tonumber(L, 1);
	 LPSTR pString = "";
	 if (ObjectExist(iID))
	 {
		 int iLimbNum = lua_tonumber(L, 2);
		 if (iLimbNum < 1 || iLimbNum >= g_ObjectList[iID]->iFrameCount) return 0;

		 // check the object exists
		 if (ConfirmObjectAndLimb(iID, iLimbNum))
		 {
			 // get name of frame
			 sObject* pObject = g_ObjectList[iID];
			 LPSTR pLimbName = pObject->ppFrameList[iLimbNum]->szName;
			 pString = pLimbName;
		 }
		 //LPSTR pString = LimbName(lua_tonumber(L, 1), lua_tonumber(L, 2)); //PE: Leak mem.
	 }
	 lua_pushstring(L, pString);
	 return 1;
 }

 // Entity Animation
 int SetEntityAnimation(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	int iEntityIndex = lua_tonumber(L, 1);
	int iAnimationSetIndex = lua_tonumber(L, 2);
	int iAnimationSetStart = lua_tonumber(L, 3);
	int iAnimationSetFinish = lua_tonumber(L, 4);
	if ( iEntityIndex > 0 )
	{
		int iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 )
		{
			t.entityanim[iEntID][iAnimationSetIndex].start = iAnimationSetStart;
			t.entityanim[iEntID][iAnimationSetIndex].finish = iAnimationSetFinish;
			if ( iAnimationSetStart == -1 && iAnimationSetFinish == -1 )
				t.entityanim[iEntID][iAnimationSetIndex].found = 0;
			else
				t.entityanim[iEntID][iAnimationSetIndex].found = 1;
		}
	}
	return 1;
 }
 int GetEntityAnimationStart(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	int iAnimationSetIndex = lua_tonumber(L, 2);
	if ( iEntityIndex > 0 )
	{
		int iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 )
		{
			iReturnValue = t.entityanim[iEntID][iAnimationSetIndex].start;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetEntityAnimationFinish(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	int iAnimationSetIndex = lua_tonumber(L, 2);
	if ( iEntityIndex > 0 )
	{
		int iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 )
		{
			iReturnValue = t.entityanim[iEntID][iAnimationSetIndex].finish;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetEntityAnimationFound(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	int iAnimationSetIndex = lua_tonumber(L, 2);
	if ( iEntityIndex > 0 )
	{
		int iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 )
		{
			iReturnValue = t.entityanim[iEntID][iAnimationSetIndex].found;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }

 int GetObjectAnimationFinished(lua_State *L)
 {
	 lua = L;
	 int n = lua_gettop(L);
	 if ( n < 1 ) return 0;
	 int iReturnValue = 0;
	 int iEntityIndex = lua_tonumber(L, 1);
	 if (iEntityIndex > 0)
	 {
		 int iObjID = t.entityelement[iEntityIndex].obj;
		 if (iObjID)
		 {
			 sObject* pObject = GetObjectData(iObjID);
			 if (pObject)
			 {
				// detects when a played animation comes to an end
				if (pObject->fAnimFrame >= t.luaglobal.setanimfinish)
				{
					iReturnValue = 1;
				}
			 }
		 }
	 }
	 lua_pushinteger (L, iReturnValue);
	 return 1;
 }

 int GetEntityFootfallMax(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	if ( iEntityIndex > 0 )
	{
		int iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 )
		{
			iReturnValue = t.entityprofile[iEntID].footfallmax;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetEntityFootfallKeyframe(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	int iReturnValue = 0;
	int iEntityIndex = lua_tonumber(L, 1);
	int iFootfallIndex = lua_tonumber(L, 2);
	int iLeftOrRight = lua_tonumber(L, 3);
	if ( iEntityIndex > 0 )
	{
		int iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 )
		{
			if ( iLeftOrRight == 0 )
				iReturnValue = t.entityfootfall[iEntID][iFootfallIndex].leftfootkeyframe;
			else
				iReturnValue = t.entityfootfall[iEntID][iFootfallIndex].rightfootkeyframe;
		}
	}
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }

 int GetAmmoClipMax(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	t.e = lua_tonumber(L, 1);
	entity_lua_findcharanimstate();
	int iReturnValue = -1;
	if ( t.tcharanimindex != - 1 ) iReturnValue = t.charanimstates[t.tcharanimindex].ammoinclipmax;
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int GetAmmoClip(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	t.e = lua_tonumber(L, 1);
	entity_lua_findcharanimstate();
	int iReturnValue = -1;
	if ( t.tcharanimindex != - 1 ) iReturnValue = t.charanimstates[t.tcharanimindex].ammoinclip;
	lua_pushinteger ( L, iReturnValue );
	return 1;
 }
 int SetAmmoClip(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	t.e = lua_tonumber(L, 1);
	entity_lua_findcharanimstate();
	if ( t.tcharanimindex != - 1 ) t.charanimstates[t.tcharanimindex].ammoinclip = lua_tonumber(L, 2);
	return 0;
 }

 //
 // Entity Physics Commands
 //
 int FreezeEntityCore ( lua_State *L, int iCoreMode )
 {
	lua = L;
	int n = lua_gettop(L);
	if ( iCoreMode == 0 && n < 1 ) return 0;
	if ( iCoreMode == 1 && n < 2 ) return 0;
	int iEntityIndex = lua_tonumber(L, 1);
	int iObjectNumber = t.entityelement[iEntityIndex].obj;
	if ( iObjectNumber > 0 )
	{
		sObject* pObject = GetObjectData ( iObjectNumber );
		if ( pObject )
		{
			if ( iCoreMode == 1 )
			{
				// force a freeze mode onto physics object
				int iFreezeMode = lua_tonumber(L, 2);
				ODESetBodyResponse ( iObjectNumber, 1 + iFreezeMode );
			}
			else
			{
				// restore response to time of creation
				ODESetBodyResponse ( iObjectNumber, 0 );
			}
		}
	}
	return 0;
 }
 int FreezeEntity(lua_State *L) { return FreezeEntityCore ( L, 1 ); }
 int UnFreezeEntity(lua_State *L) { return FreezeEntityCore ( L, 0 ); }

 // Terrain
 float GetLUATerrainHeightEx ( float fX, float fZ )
 {
	float fReturnHeight = g.gdefaultterrainheight;
	if (  t.terrain.TerrainID>0 ) 
	{
		fReturnHeight = BT_GetGroundHeight ( t.terrain.TerrainID, fX, fZ );
	}
	return fReturnHeight;
 }
 int GetTerrainHeight(lua_State *L)
 {
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	float fReturnHeight = 0.0f;
	float fX = lua_tonumber(L, 1);
	float fZ = lua_tonumber(L, 2);
	fReturnHeight = GetLUATerrainHeightEx(fX,fZ);
	lua_pushinteger ( L, fReturnHeight );
	return 1;
 }

 // DarkAI
 int AISetEntityControl(lua_State *L)
 {
	 lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 2 ) return 0;

	AISetEntityControl ( lua_tonumber(L, 1), lua_tonumber(L, 2) );

	// TO DO: Dave - need to check if coop mode is on, need a command to let lua know not to send these messages if they are not needed
	SteamSendLua ( 30 , lua_tointeger(L, 1), lua_tonumber(L, 2) );

	return 0;
 }

 int AIEntityAssignPatrolPath(lua_State *L)
 {
	 lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 2 ) return 0;

	AIEntityAssignPatrolPath ( lua_tonumber(L, 1), lua_tonumber(L, 2) );

	return 0;
 }

 int AIEntityStop(lua_State *L)
 {
	 lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 1 ) return 0;

	AIEntityStop ( lua_tonumber(L, 1) );

	return 0;
 }

int AIEntityAddTarget(lua_State *L)
{
	 lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 2 ) return 0;

	AIEntityAddTarget ( lua_tonumber(L, 1), lua_tonumber(L, 2) );

	return 0;
}

int AIEntityRemoveTarget(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 2 ) return 0;

	AIEntityRemoveTarget ( lua_tonumber(L, 1), lua_tonumber(L, 2) );

	return 0;
}

int AIEntityMoveToCover(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 3 ) return 0;

	lua_pushinteger ( L , AIEntityMoveToCover ( lua_tonumber(L, 1), lua_tonumber(L, 2) , lua_tonumber(L, 3) ) );

	return 1;
}

int AIGetEntityCanSee(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 5 ) return 0;

	lua_pushinteger ( L , AIGetEntityCanSee ( lua_tonumber(L, 1), lua_tonumber(L, 2) , lua_tonumber(L, 3) , lua_tonumber(L, 4) , lua_tonumber(L, 5) ) );

	return 1;
}

int AIGetEntityCanFire(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 1 ) return 0;

	lua_pushinteger ( L , AIGetEntityCanFire ( lua_tonumber(L, 1) ) );

	return 1;
}

int AIGetEntityViewRange(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L , AIGetEntityViewRange ( lua_tonumber(L, 1) ) );
	return 1;
}
int AIGetEntitySpeed(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L , AIGetEntitySpeed ( lua_tonumber(L, 1) ) );
	return 1;
}

int AIGetTotalPaths(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n != 0 ) return 0;
	lua_pushinteger ( L , AIGetTotalPaths () );
	return 1;
}
int AIGetPathCountPoints(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushinteger ( L , AIGetPathCountPoints ( lua_tonumber(L, 1) ) );
	return 1;
}
int AIPathGetPointX(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	lua_pushnumber ( L , AIPathGetPointX ( lua_tonumber(L, 1) , lua_tonumber(L, 2) ) );
	return 1;
}
int AIPathGetPointY(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	lua_pushnumber ( L , AIPathGetPointY ( lua_tonumber(L, 1) , lua_tonumber(L, 2) ) );
	return 1;
}
int AIPathGetPointZ(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	lua_pushnumber ( L , AIPathGetPointZ ( lua_tonumber(L, 1) , lua_tonumber(L, 2) ) );
	return 1;
}

int AIGetTotalCover(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n != 0 ) return 0;
	lua_pushinteger ( L , AIGetTotalCover () );
	return 1;
}
int AICoverGetPointX(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L , AICoverGetPointX ( lua_tonumber(L, 1) ) );
	return 1;
}
int AICoverGetPointY(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L , AICoverGetPointY ( lua_tonumber(L, 1) ) );
	return 1;
}
int AICoverGetPointZ(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L , AICoverGetPointZ ( lua_tonumber(L, 1) ) );
	return 1;
}
int AICoverGetAngle(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L , AICoverGetAngle ( lua_tonumber(L, 1) ) );
	return 1;
}
int AICoverGetIfUsed(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushstring ( L , AICoverGetIfUsed ( lua_tonumber(L, 1) ) );
	return 1;
}

int MsgBox(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 1 ) return 0;

	MessageBox(NULL, lua_tostring(L, 1), "LUA MESSAGE", MB_TOPMOST | MB_OK);

	return 0;
}

int AISetEntityMoveBoostPriority(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	int iObj = lua_tointeger(L, 1);
	AISetEntityMoveBoostPriority ( iObj );
}

int AIEntityGoToPosition(lua_State *L)
{
	// can pass in 3 or 4 params
	// (3) obj,x,z 
	// (4) obj,x,y,z
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	int iObj = lua_tointeger(L, 1);
	float fGoToX = lua_tonumber(L, 2);
	if ( n == 3 )
	{
		// legacy X and Z within current container
		float fGoToZ = lua_tonumber(L, 3);
		AIEntityGoToPosition ( iObj, fGoToX, fGoToZ );
	}
	if ( n == 4 )
	{
		// search for container based on true X, Y, Z position
		float fGoToY = lua_tonumber(L, 3);
		float fGoToZ = lua_tonumber(L, 4);
		int iDestinationContainerIndex = 0;
		t.tpointx_f = fGoToX;
		t.tpointz_f = fGoToZ;
		for ( t.waypointindex = 1; t.waypointindex <= g.waypointmax; t.waypointindex++ )
		{
			// zone - confined containers
			if ( t.waypoint[t.waypointindex].style == 3 )
			{
				t.tokay = 0; waypoint_ispointinzone ( );
				if ( t.tokay == 1 ) 
				{
					int e = t.waypoint[t.waypointindex].linkedtoentityindex;
					if ( fGoToY > t.entityelement[e].y - 25.0f && fGoToY < t.entityelement[e].y + 65.0f )
					{
						// only if Y position above zone entity position and below cap of this layer
						iDestinationContainerIndex = t.waypointindex;
					}
				}
			}
		}
		AIEntityGoToPosition ( iObj, fGoToX, fGoToZ, iDestinationContainerIndex );
	}
	return 0;
}


int AIGetEntityHeardSound(lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L , AIGetEntityHeardSound ( lua_tonumber(L, 1) ) );
	return 1;
}

int AISetData ( lua_State *L, int iDataMode )
{
	lua = L;
	int iParamNum = 0;
	switch ( iDataMode )
	{
		case 1 : iParamNum = 4;	break;
		case 2 : iParamNum = 2;	break;
	}
	int n = lua_gettop(L);
	if ( n < iParamNum ) return 0;
	switch ( iDataMode )
	{
		case 1 : AISetEntityPosition ( lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) ); break;
		case 2 : AISetEntityTurnSpeed ( lua_tonumber(L, 1), lua_tonumber(L, 2) ); break;
	}
	return 0;
}
int AIGetData ( lua_State *L, int iDataMode )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	switch ( iDataMode )
	{
		case 1 : lua_pushnumber ( L , AIGetEntityAngleY ( lua_tonumber(L, 1) ) ); break;
		case 2 : if ( t.aisystem.processlogic == 0 )
					lua_pushnumber ( L , 0 ); 
				 else
					lua_pushnumber ( L , AIGetEntityIsMoving ( lua_tonumber(L, 1) ) ); 
				 break;
	}
	return 1;
}
int AISetEntityPosition ( lua_State *L ) { return AISetData ( L, 1 ); }
int AISetEntityTurnSpeed ( lua_State *L ) { return AISetData ( L, 2 ); }
int AIGetEntityAngleY ( lua_State *L ) { return AIGetData ( L, 1 ); }
int AIGetEntityIsMoving ( lua_State *L ) { return AIGetData ( L, 2 ); }

//
// Visual Attribute Settings
//

int AIGetVisualSetting ( lua_State *L, int iMode )
{
	lua = L;
	//int n = lua_gettop(L);
	//if ( n < 1 ) return 0;
	switch ( iMode )
	{
		case 1 : lua_pushnumber ( L, t.visuals.FogNearest_f ); break;
		case 2 : lua_pushnumber ( L, t.visuals.FogDistance_f ); break;
		case 3 : lua_pushnumber ( L, t.visuals.FogR_f ); break;
		case 4 : lua_pushnumber ( L, t.visuals.FogG_f ); break;
		case 5 : lua_pushnumber ( L, t.visuals.FogB_f ); break;
		case 6 : lua_pushnumber ( L, t.visuals.FogA_f ); break;
		case 7 : lua_pushnumber ( L, t.visuals.AmbienceIntensity_f ); break;
		case 8 : lua_pushnumber ( L, t.visuals.AmbienceRed_f ); break;
		case 9 : lua_pushnumber ( L, t.visuals.AmbienceGreen_f ); break;
		case 10 : lua_pushnumber ( L, t.visuals.AmbienceBlue_f ); break;
		case 11 : lua_pushnumber ( L, t.visuals.SurfaceRed_f ); break;
		case 12 : lua_pushnumber ( L, t.visuals.SurfaceGreen_f ); break;
		case 13 : lua_pushnumber ( L, t.visuals.SurfaceBlue_f ); break;
		case 14 : lua_pushnumber ( L, t.visuals.SurfaceIntensity_f ); break;
		case 15 : lua_pushnumber ( L, t.visuals.VignetteRadius_f ); break;
		case 16 : lua_pushnumber ( L, t.visuals.VignetteIntensity_f ); break;
		case 17 : lua_pushnumber ( L, t.visuals.MotionDistance_f ); break;
		case 18 : lua_pushnumber ( L, t.visuals.MotionIntensity_f ); break;
		case 19 : lua_pushnumber ( L, t.visuals.DepthOfFieldDistance_f ); break;
		case 20 : lua_pushnumber ( L, t.visuals.DepthOfFieldIntensity_f ); break;
		default : lua_pushnumber ( L, 0 ); break;
	}
	return 1;
}

int GetFogNearest(lua_State *L) { return AIGetVisualSetting ( L, 1 ); }
int GetFogDistance(lua_State *L) { return AIGetVisualSetting ( L, 2 ); }
int GetFogRed(lua_State *L) { return AIGetVisualSetting ( L, 3 ); }
int GetFogGreen(lua_State *L) { return AIGetVisualSetting ( L, 4 ); }
int GetFogBlue(lua_State *L) { return AIGetVisualSetting ( L, 5 ); }
int GetFogIntensity(lua_State *L) { return AIGetVisualSetting ( L, 6 ); }
int GetAmbienceIntensity(lua_State *L) { return AIGetVisualSetting ( L, 7 ); }
int GetAmbienceRed(lua_State *L) { return AIGetVisualSetting ( L, 8 ); }
int GetAmbienceGreen(lua_State *L) { return AIGetVisualSetting ( L, 9 ); }
int GetAmbienceBlue(lua_State *L) { return AIGetVisualSetting ( L, 10 ); }
int GetSurfaceRed(lua_State *L) { return AIGetVisualSetting ( L, 11 ); }
int GetSurfaceGreen(lua_State *L) { return AIGetVisualSetting ( L, 12 ); }
int GetSurfaceBlue(lua_State *L) { return AIGetVisualSetting ( L, 13 ); }
int GetSurfaceIntensity(lua_State *L) { return AIGetVisualSetting ( L, 14 ); }
int GetPostVignetteRadius(lua_State *L) { return AIGetVisualSetting ( L, 15 ); }
int GetPostVignetteIntensity(lua_State *L) { return AIGetVisualSetting ( L, 16 ); }
int GetPostMotionDistance(lua_State *L) { return AIGetVisualSetting ( L, 17 ); }
int GetPostMotionIntensity(lua_State *L) { return AIGetVisualSetting ( L, 18 ); }
int GetPostDepthOfFieldDistance(lua_State *L) { return AIGetVisualSetting ( L, 19 ); }
int GetPostDepthOfFieldIntensity(lua_State *L) { return AIGetVisualSetting ( L, 20 ); }

int AICouldSee(lua_State *L )
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 4 ) return 0;

	lua_pushinteger ( L , AICouldSee ( lua_tonumber(L, 1) , lua_tonumber(L, 2) , lua_tonumber(L, 3) , lua_tonumber(L, 4) ) );

	return 1;

}

//
//
//


//
//
//


//
// New RecastDetour(RD) AI Commands
//


int GetDeviceWidth(lua_State *L)
{
	lua = L;

	lua_pushinteger ( L , GetDisplayWidth() );

	return 1;
}

int GetDeviceHeight(lua_State *L)
{
	lua = L;

	lua_pushinteger ( L , GetDisplayHeight() );

	return 1;
}

int GetFirstEntitySpawn(lua_State *L)
{
	lua = L;

	int id = 0;

	if ( t.delayOneFrameForActivatedLua == 0 )
	{
		if ( t.entitiesActivatedForLua.size() > 0 )
		{
			id = t.entitiesActivatedForLua.back();
			t.entitiesActivatedForLua.pop_back();
		}
	}
	else 
		t.delayOneFrameForActivatedLua = 0;

	lua_pushinteger ( L , id );

	return 1;
}

// VR and Head Tracking

int GetHeadTracker(lua_State *L)
{
	lua = L;
	int id = 0;
	#ifdef VRTECH
	 if ( GGVR_IsHmdPresent() > 0 && g.vrglobals.GGVRUsingVRSystem == 1 ) id = 1;
	#else
	 if ( SetupGetTracking(NULL,NULL,NULL,1.0f) == true ) id = 1;
	#endif
	lua_pushinteger ( L , id );
	return 1;
}
int ResetHeadTracker(lua_State *L)
{
	lua = L;
	int id = 0;
	#ifdef VRTECH
	#else
	 SetupResetTracking();
	#endif
	lua_pushinteger ( L , id );
	return 1;
}
int GetHeadTrackerYaw(lua_State *L)
{
	lua = L;
	#ifdef VRTECH
	 float fValue = GGVR_GetHMDYaw();// + g_fDriverCompensationYaw;
	#else
	 float fValue = g_fVR920TrackingYaw + g_fDriverCompensationYaw;
	 if ( g_VR920AdapterAvailable == false ) fValue = 0.0f;
	#endif
	lua_pushnumber ( L , fValue );
	return 1;
}
int GetHeadTrackerPitch(lua_State *L)
{
	lua = L;
	#ifdef VRTECH
	 float fValue = GGVR_GetHMDPitch();// + g_fDriverCompensationYaw;
	#else
	 float fValue = g_fVR920TrackingPitch + g_fDriverCompensationPitch;
	 if ( g_VR920AdapterAvailable == false ) fValue = 0.0f;
	#endif
	lua_pushnumber ( L , fValue );
	return 1;
}
int GetHeadTrackerRoll(lua_State *L)
{
	lua = L;
	#ifdef VRTECH
	 float fValue = GGVR_GetHMDRoll();// + g_fDriverCompensationYaw;
	#else
	 float fValue = g_fVR920TrackingRoll + g_fDriverCompensationRoll;
	 if ( g_VR920AdapterAvailable == false ) fValue = 0.0f;
	#endif
	lua_pushnumber ( L , fValue );
	return 1;
}

int GetHeadTrackerNormalX(lua_State *L)
{
	lua = L;
	float fValue = 0.0f;
	lua_pushnumber ( L , fValue );
	return 1;
}

int GetHeadTrackerNormalY(lua_State *L)
{
	lua = L;
	float fValue = 0.0f;
	lua_pushnumber ( L , fValue );
	return 1;
}

int GetHeadTrackerNormalZ(lua_State *L)
{
	lua = L;
	float fValue = 0.0f;
	lua_pushnumber ( L , fValue );
	return 1;
}


// PROMPT 3D

int Prompt3D(lua_State *L)
{
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	char pTextToRender[1024];
	strcpy ( pTextToRender, lua_tostring(L, 1));
	DWORD dwPrompt3DTime = lua_tonumber(L, 2);
	lua_prompt3d(pTextToRender, Timer() + dwPrompt3DTime , 0 );
	return 1;
}

int PositionPrompt3D(lua_State *L)
{
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	float fX = lua_tonumber(L, 1);
	float fY = lua_tonumber(L, 2);
	float fZ = lua_tonumber(L, 3);
	float fAY = lua_tonumber(L, 4);
	lua_positionprompt3d(0, fX, fY, fZ, fAY, false );
	return 1;
}

// AGK IMAGE AND SPRITE COMMANDS

int LoadImage(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, send 0 result back
	if ( n < 1 )
	{
		lua_pushnumber ( L , 0 );
		return 1;
	}

	int iID = 0;

	char filename[MAX_PATH];
	strcpy ( filename , cstr(g.fpscrootdir_s+"\\Files\\").Get());
	strcat ( filename , lua_tostring(L, 1) );

	iID = GetFreeLUAImageID();
	SetMipmapNum(1); //PE: mipmaps not needed for lua loaded images.
	image_setlegacyimageloading(true);
	if ( FileExist ( filename ) == 1 && iID > 0 )
		LoadImage ( filename , iID );		
	image_setlegacyimageloading(false);
	SetMipmapNum(-1);
	lua_pushnumber ( L , iID );

	return 1;
}

int GetImageWidth(lua_State *L)
{
	// get LUA param
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) { lua_pushnumber ( L , 0 ); return 1; }

	// get image width
	int iImageID = lua_tointeger(L, 1);
	float fImageWidth = ( ((float)ImageWidth(iImageID)/(float)g_dwScreenWidth) * 100.0f);

	// push return value
	lua_pushnumber ( L , fImageWidth );

	// success
	return 1;
}

int GetImageHeight(lua_State *L)
{
	// get LUA param
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) { lua_pushnumber ( L , 0 ); return 1; }

	// get image width
	int iImageID = lua_tointeger(L, 1);
	float fImageHeight = ( ((float)ImageHeight(iImageID)/(float)g_dwScreenHeight) * 100.0f);

	// push return value
	lua_pushnumber ( L , fImageHeight );

	// success
	return 1;
}

int DeleteSpriteImage(lua_State *L)
{
	// get LUA param
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) { lua_pushnumber ( L , 0 ); return 1; }

	// get image width
	int iImageID = lua_tointeger(L, 1);
	if ( ImageExist(iImageID)==1 )
		DeleteImage(iImageID);

	// push return value
	lua_pushnumber ( L , 1 );

	// success
	return 1;
}

int CreateSprite(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, send 0 result back
	if ( n < 1 )
	{
		lua_pushnumber ( L , 0 );
		return 1;
	}

	int iID = 0;
	iID = GetFreeLUASpriteID();

	Sprite ( iID , 0 , 0 , lua_tointeger(L, 1) );
	SetSpritePriority ( iID , 90 ); // which is 10 in agk

	lua_pushnumber ( L , iID );

	return 1;
}

int PasteSprite(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) { lua_pushnumber ( L , 0 ); return 1; }
	int iID = lua_tointeger(L, 1);
	PasteSprite ( iID, SpriteX( iID ), SpriteY( iID ) );
	return 0;
}

int PasteSpritePosition(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) { lua_pushnumber ( L , 0 ); return 1; }
	int iID = lua_tointeger(L, 1);

//	float fX = lua_tointeger(L, 2);
//	float fY = lua_tointeger(L, 3);

	//PE: Thanks AmenMoses for this fix :)
	float fX = lua_tonumber(L, 2);
	float fY = lua_tonumber(L, 3);

	fX = ( fX * g_dwScreenWidth ) / 100.0f;
	fY = ( fY * g_dwScreenHeight ) / 100.0f;
	PasteSprite ( iID, fX, fY );
	return 0;
}

int SetSpriteImage(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 2 )
		return 0;

	int iID = lua_tointeger(L, 1);
	float image = lua_tointeger(L, 2);

	if ( SpriteExist ( iID ) == 1 )
	{
		Sprite ( iID , SpriteX( iID ) , SpriteY( iID ) , image );
	}

	return 0;

}

int SetSpritePosition(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 3 )
		return 0;

	int iID = lua_tointeger(L, 1);
	float x = lua_tonumber(L, 2);
	float y = lua_tonumber(L, 3);

	x = ( x * g_dwScreenWidth ) / 100.0f;
	y = ( y * g_dwScreenHeight ) / 100.0f;

	if ( SpriteExist ( iID ) == 1 )
	{
		Sprite ( iID , x , y , GetSpriteImage(iID) );
	}

	return 0;
}

int SetSpriteDepth(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 2 )
		return 0;

	int iID = lua_tointeger(L, 1);
	// need to flip it as agk does the order reversed
	float depth = 100 - lua_tointeger(L, 2);


	if ( SpriteExist ( iID ) == 1 )
	{
		SetSpritePriority ( iID , depth );
	}

	return 0;
}

int SetSpriteColor(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 5 )
		return 0;

	int iID = lua_tointeger(L, 1);
	int red = lua_tointeger(L, 2);
	int green = lua_tointeger(L, 3);
	int blue = lua_tointeger(L, 4);
	int alpha = lua_tointeger(L, 5);


	if ( SpriteExist ( iID ) == 1 )
	{
		SetSpriteAlpha ( iID , alpha );
		SetSpriteDiffuse ( iID , red , green , blue );
	}

	return 0;
}

int SetSpriteAngle(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 2 )
		return 0;

	int iID = lua_tointeger(L, 1);
	int angle = lua_tonumber(L, 2);


	if ( SpriteExist ( iID ) == 1 )
	{
		RotateSprite ( iID , angle );
	}

	return 0;
}

int DeleteSprite(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 1 )
		return 0;

	int iID = lua_tointeger(L, 1);

	if ( SpriteExist ( iID ) == 1 )
	{
		DeleteSprite ( iID );
	}

	return 0;
}

int SetSpriteOffset(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 3 )
		return 0;

	int iID = lua_tointeger(L, 1);
	float x = lua_tonumber(L, 2);
	float y = lua_tonumber(L, 3);

	if ( x == -1 && y == -1 ) return 0;

	if ( x != -1 )
	{
		x = ( x * g_dwScreenWidth ) / 100.0f;
	}

	if ( y != -1 )
	{
		y = ( y * g_dwScreenHeight ) / 100.0f;

		if ( x == -1 )
		{
			float perc = ( y / ImageHeight(GetSpriteImage(iID)) ) * 100.0f;
			x = (( perc * ImageWidth(GetSpriteImage(iID)) ) / 100.0f) * ( g_dwScreenWidth / g_dwScreenHeight );
		}
	}
	else
	{
		float perc = ( x / ImageWidth(GetSpriteImage(iID)) ) * 100.0f;
		y = (( perc * ImageHeight(GetSpriteImage(iID)) ) / 100.0f) * ( g_dwScreenWidth / g_dwScreenHeight );
	}

	if ( SpriteExist ( iID ) == 1 )
	{
		OffsetSprite ( iID , x , y );
	}

	return 0;
}

int SetSpriteSize ( lua_State *L )
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 3 )
		return 0;

	int iID = lua_tointeger(L, 1);
	float sizeX = lua_tonumber(L, 2);
	float sizeY = lua_tonumber(L, 3);


	//PE: vertex data use iXOffset)+iWidth-0.5f, (-0.5f) soo add a bit.
	//PE: mainly visible when using 100 percent
	//PE: https://github.com/TheGameCreators/GameGuruRepo/issues/423
	//PE: So 100 percent do not fill the entire screen.
	//PE: Did not want to change vertex data as it might change how sprites display on other level.
	//PE: So for now just do this to fill the hole screen. (should be changed in vertex at some point)

	if (sizeX == 100 ) {
		sizeX += 0.1;
	}
	if (sizeY == 100) {
		sizeY += 0.1;
	}

	if ( sizeX == -1 && sizeY == -1 ) return 0;

	if ( sizeX != -1 )
	{
		sizeX = ( sizeX * g_dwScreenWidth ) / 100.0f;
	}
	
	if ( sizeY != -1 )
	{
		sizeY = ( sizeY * g_dwScreenHeight ) / 100.0f;

		if ( sizeX == -1 )
		{
			float perc = ( sizeY / ImageHeight(GetSpriteImage(iID)) ) * 100.0f;
			sizeX = (( perc * ImageWidth(GetSpriteImage(iID)) ) / 100.0f) * ( g_dwScreenWidth / g_dwScreenHeight );
		}
	}
	else
	{
		//PE: 11-06-19 issue: https://github.com/TheGameCreators/GameGuruRepo/issues/504
		//PE: I cant test this on my system , but assume we could always use the backbuffer size g_pGlob->iScreenWidth instead of the screenwidth g_dwScreenWidth
		//PE: Can someone with a similar screen setup do this, test the return of these 2 MessageBox.
		//PE: They should always be the same , but issue indicate they are not.
		//PE: Just add rader.lua and enable the below 4 lines to test :)
//		char tmp[80]; sprintf(tmp, "g_pGlob->iScreenWidth: %d", g_pGlob->iScreenWidth); // 1920
//		MessageBox(NULL, tmp, "g_pGlob->iScreenWidth", MB_TOPMOST | MB_OK);
//		sprintf(tmp, "g_dwScreenWidth: %d", g_dwScreenWidth); // 1920
//		MessageBox(NULL, tmp, "g_dwScreenWidth", MB_TOPMOST | MB_OK);
		float perc = ( sizeX / ImageWidth(GetSpriteImage(iID)) ) * 100.0f;
		sizeY = (( perc * ImageHeight(GetSpriteImage(iID)) ) / 100.0f) * ( g_dwScreenWidth / g_dwScreenHeight );
	}

	if ( SpriteExist ( iID ) == 1 )
	{
		SizeSprite ( iID , sizeX , sizeY );
	}

	return 0;
}

int DrawSpritesFirstForLUA ( lua_State *L )
{
	DrawSpritesFirst();
	return 0;
}

int DrawSpritesLastForLUA ( lua_State *L )
{
	DrawSpritesLast();
	return 0;
}

int BackdropOffForLUA ( lua_State *L )
{
	BackdropOff();
	return 0;
}

int BackdropOnForLUA ( lua_State *L )
{
	BackdropOn();
	return 0;
}

int LoadGlobalSound ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	const char* pFilename = lua_tostring(L, 1);
	int iID = g.globalsoundoffset + lua_tointeger(L, 2);
	if ( SoundExist(iID)==1 ) DeleteSound(iID);
	LoadSound((LPSTR)pFilename,iID);
	return 0;
}
int PlayGlobalSound ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	if ( SoundExist(iID)==1 )
	{
		PlaySound(iID);
	}
	return 0;
}
int LoopGlobalSound ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	if ( SoundExist(iID)==1 )
	{
		LoopSound(iID);
	}
	return 0;
}
int StopGlobalSound ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	if ( SoundExist(iID)==1 )
	{
		StopSound(iID);
	}
	return 0;
}
int DeleteGlobalSound ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	if ( SoundExist(iID)==1 )
	{
		DeleteSound(iID);
	}
	return 0;
}
int SetGlobalSoundSpeed ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	int iSpeed = lua_tointeger(L, 2);
	if ( SoundExist(iID)==1 )
	{
		SetSoundSpeed(iID,iSpeed);
	}
	return 0;
}
int SetGlobalSoundVolume ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	int iVolume = lua_tointeger(L, 2);
	if ( SoundExist(iID)==1 )
	{
		SetSoundVolume(iID,iVolume);
	}
	return 0;
}
int GetGlobalSoundExist(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	lua_pushinteger ( L , SoundExist ( iID ) );
	return 1;
}
int GetGlobalSoundPlaying(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	lua_pushinteger ( L , SoundPlaying ( iID ) );
	return 1;
}
int GetGlobalSoundLooping(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iID = g.globalsoundoffset + lua_tointeger(L, 1);
	lua_pushinteger ( L , SoundLooping ( iID ) );
	return 1;
}

int SetRawSoundData ( lua_State *L, int iDataMode )
{
	lua = L;
	int iParamNum = 0;
	switch ( iDataMode )
	{
		case 1 : iParamNum = 1;	break;
		case 2 : iParamNum = 1;	break;
		case 3 : iParamNum = 1;	break;
		case 4 : iParamNum = 2;	break;
	}
	int n = lua_gettop(L);
	if ( n < iParamNum ) return 0;
	int iSoundID = lua_tonumber(L, 1);
	if (iSoundID > 0 && SoundExist(iSoundID) == 1)
	{
		switch (iDataMode)
		{
		case 1: PlaySound(lua_tonumber(L, 1)); break;
		case 2: LoopSound(lua_tonumber(L, 1)); break;
		case 3: StopSound(lua_tonumber(L, 1)); break;
		case 4: SetSoundVolume(lua_tonumber(L, 1), soundtruevolume(lua_tonumber(L, 2))); break;
		case 5: SetSoundSpeed(lua_tonumber(L, 1), lua_tonumber(L, 2)); break;
		}
	}
	return 0;
}
int GetRawSoundData ( lua_State *L, int iDataMode )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	switch ( iDataMode )
	{
		case 1 : lua_pushnumber ( L , SoundExist ( lua_tonumber(L, 1) ) ); break;
		case 2 : lua_pushnumber ( L , SoundPlaying ( lua_tonumber(L, 1) ) ); break;
		case 3 : lua_pushnumber ( L , t.entityelement[lua_tonumber(L, 1)].soundset ); break;
	}
	return 1;
}
int PlayRawSound ( lua_State *L ) { return SetRawSoundData ( L, 1 ); }
int LoopRawSound ( lua_State *L ) { return SetRawSoundData ( L, 2 ); }
int StopRawSound ( lua_State *L ) { return SetRawSoundData ( L, 3 ); }
int SetRawSoundVolume ( lua_State *L ) { return SetRawSoundData ( L, 4 ); }
int SetRawSoundSpeed ( lua_State *L ) { return SetRawSoundData ( L, 5 ); }
int RawSoundExist ( lua_State *L ) { return GetRawSoundData ( L, 1 ); }
int RawSoundPlaying ( lua_State *L ) { return GetRawSoundData ( L, 2 ); }

int GetEntityRawSound(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	int iE = lua_tonumber(L, 1);
	int iSoundSlot = lua_tonumber(L, 2);
	int iRawSoundIndex = 0;
	if (iSoundSlot == 0) iRawSoundIndex = t.entityelement[iE].soundset;
	if (iSoundSlot == 1) iRawSoundIndex = t.entityelement[iE].soundset1;
	if (iSoundSlot == 2) iRawSoundIndex = t.entityelement[iE].soundset2;
	if (iSoundSlot == 3) iRawSoundIndex = t.entityelement[iE].soundset3;
	if (iSoundSlot == 4) iRawSoundIndex = t.entityelement[iE].soundset4;
	lua_pushnumber ( L , iRawSoundIndex );
	return 1;
}

// Voice

#ifdef VRTECH
int GetSpeech(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iRunning = 0;
	int iE = lua_tonumber(L, 1);
	int iCharAnimIndex = -1;
	for ( int tcharanimindex = 1 ; tcharanimindex <= g.charanimindexmax; tcharanimindex++ )
	{
		if (  t.charanimstates[tcharanimindex].e == iE ) { iCharAnimIndex = tcharanimindex  ; break; }
	}
	if ( iCharAnimIndex != -1 )
	{
		if ( t.charanimstates[iCharAnimIndex].ccpo.speak.fMouthTimeStamp > 0.0f ) iRunning = 1;
	}
	lua_pushinteger ( L, iRunning );
	return 1;
}
#endif

// Generic

int GetTimeElapsed ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, g.timeelapsed_f );
	return 1;
}

int GetKeyState ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	int iKeyValue = lua_tonumber(L, 1);
	lua_pushnumber ( L, KeyState(g.keymap[iKeyValue]) );
	return 1;
}

int GetGlobalTimer ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, Timer() );
	return 1;
}

int MouseMoveX ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, MouseMoveX() );
	return 1;
}
int MouseMoveY ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, MouseMoveY() );
	return 1;
}
int GetDesktopWidth ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, GetDesktopWidth() );
	return 1;
}
int GetDesktopHeight ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, GetDesktopHeight() );
	return 1;
}
int CurveValue ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	float a = lua_tonumber(L, 1);
	float b = lua_tonumber(L, 2);
	float c = lua_tonumber(L, 3);
	lua_pushnumber ( L, CurveValue(a, b, c) );
	return 1;
}
int CurveAngle ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	float a = lua_tonumber(L, 1);
	float b = lua_tonumber(L, 2);
	float c = lua_tonumber(L, 3);
	lua_pushnumber ( L, CurveAngle(a, b, c) );
	return 1;
}
int PositionMouse ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	float fScreenX = lua_tonumber(L, 1);
	float fScreenY = lua_tonumber(L, 2);
	PositionMouse ( fScreenX, fScreenY );
	g.LUAMouseX = fScreenX;
	g.LUAMouseY = fScreenY;
	return 0;
}

int GetDynamicCharacterControllerDidJump ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ODEGetDynamicCharacterControllerDidJump() );
	return 1;
}
int GetCharacterControllerDucking ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ODEGetCharacterControllerDucking(t.aisystem.objectstartindex) );
	return 1;
}

int WrapValue ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	float a = lua_tonumber(L, 1);
	lua_pushnumber ( L, WrapValue(a) );
	return 1;
}
int GetElapsedTime ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, t.ElapsedTime_f );
	return 1;
}
int GetPlrObjectPositionX ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ObjectPositionX(t.aisystem.objectstartindex) );
	return 1;
}
int GetPlrObjectPositionY ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ObjectPositionY(t.aisystem.objectstartindex) );
	return 1;
}
int GetPlrObjectPositionZ ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ObjectPositionZ(t.aisystem.objectstartindex) );
	return 1;
}
int GetPlrObjectAngleX ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ObjectAngleX(t.aisystem.objectstartindex) );
	return 1;
}
int GetPlrObjectAngleY ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ObjectAngleY(t.aisystem.objectstartindex) );
	return 1;
}
int GetPlrObjectAngleZ ( lua_State *L )
{
	lua = L;
	lua_pushnumber ( L, ObjectAngleZ(t.aisystem.objectstartindex) );
	return 1;
}
int GetGroundHeight ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	float x = lua_tonumber(L, 1);
	float z = lua_tonumber(L, 2);
	lua_pushnumber ( L, BT_GetGroundHeight(t.terrain.TerrainID,x,z) );
	return 1;
}
int NewXValue ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	float a = lua_tonumber(L, 1);
	float b = lua_tonumber(L, 2);
	float c = lua_tonumber(L, 3);
	lua_pushnumber ( L, NewXValue(a, b, c) );
	return 1;
}
int NewZValue ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	float a = lua_tonumber(L, 1);
	float b = lua_tonumber(L, 2);
	float c = lua_tonumber(L, 3);
	lua_pushnumber ( L, NewZValue(a, b, c) );
	return 1;
}

int ControlDynamicCharacterController ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 8 ) return 0;
	float fAngleY = lua_tonumber(L, 1);
	float fAngleX = lua_tonumber(L, 2);
	float fSpeed = lua_tonumber(L, 3);
	float fJump = lua_tonumber(L, 4);
	float fDucking = lua_tonumber(L, 5);
	float fPushAngle = lua_tonumber(L, 6);
	float fPushForce = lua_tonumber(L, 7);
	float fThrustUpwards = lua_tonumber(L, 8);
	#ifdef VRTECH
	if ( g.vrglobals.GGVREnabled == 0 )
	{
		// no VR control
		ODEControlDynamicCharacterController ( t.aisystem.objectstartindex, fAngleY, fAngleX, fSpeed, fJump, fDucking, fPushAngle, fPushForce, fThrustUpwards );
	}
	else
	{
		// VR Control (standing or seated)
		double Modified_fAngleY = 0.0;
		double norm = 0.0;
		if ( g.vrglobals.GGVRStandingMode == 1 )
		{
			// VR Controlled player capsule
			double norm_XOffset = 0.0;
			double norm_ZOffset = 0.0;

			// Rotate the offset around the Yaw of the HMD to make it relative to the HMD facing
			double radian = 0.0174532988888;
			double modifiedX = 0.0;
			double modifiedZ = 0.0;
			double HMDYaw = GGVR_GetHMDYaw();
			double camL = t.playercontrol.finalcameraangley_f;

			if (HMDYaw < 0.0)
			{
				HMDYaw = 360.0 + HMDYaw;
			}
			if (HMDYaw > 360.0)
			{
				HMDYaw = HMDYaw - 360.0;
			}
			if (camL < 0.0)
			{
				camL = 360.0 + camL;
			}
			if (camL > 360.0)
			{
				camL = camL - 360.0;
			}

			double yl = (camL - HMDYaw);

			if (yl < 0.0f)
			{
				yl = 360.0f + yl;
			}
			if (yl > 360.0f)
			{
				yl = yl - 360.0f;
			}
		
			yl = yl *  radian;
			double cosYL = cos(yl);  double sinYL = sin(yl); double nsinYL = -sin(yl);

			// move player along X and Z if in standing mode
			modifiedX = (sin(fAngleY*radian)*fSpeed) + ((g.vrglobals.GGVR_XposOffsetChange*cosYL) + (g.vrglobals.GGVR_ZposOffsetChange*sinYL));
			modifiedZ = (cos(fAngleY*radian)*fSpeed) + ((g.vrglobals.GGVR_XposOffsetChange*nsinYL) + (g.vrglobals.GGVR_ZposOffsetChange*cosYL));
		
			norm_XOffset = 0.0;
			norm_ZOffset = 0.0;
			Modified_fAngleY = 0.0;
		
			//Work out the motion angle of the HMD in the play area
			norm = sqrt((modifiedX*modifiedX) + (modifiedZ*modifiedZ));
			if (norm != 0.0)
			{
				double XOffset = modifiedX / norm;
				double ZOffset = modifiedZ / norm;
				double MovementAngle = 0.0f;

				if (XOffset == 0.0)
				{
					if (ZOffset > 0.0f)
					{
						MovementAngle = 0.0f;
					}
					else
					{
						MovementAngle = 180.0f;
					}
				}
				if (XOffset > 0.0)
				{
					if (ZOffset >= 0.0f)
					{
						MovementAngle = Asin(XOffset);
					}
					else
					{
						MovementAngle = 180.0f - Asin(XOffset);
					}
				}
				if (XOffset < 0.0)
				{
					if (ZOffset >= 0.0)
					{
						MovementAngle = 360.0f + Asin(XOffset);
					}
					else
					{
						MovementAngle = 180.0f - Asin(XOffset);
					}
				}

				Modified_fAngleY = MovementAngle;

			}
			else
			{
				norm_XOffset = 0.0;
				norm_ZOffset = 0.0;
				norm = 0.0;
				Modified_fAngleY = fAngleY;
			}
		}
		else
		{
			norm = fSpeed;
			Modified_fAngleY = fAngleY;
		}
		ODEControlDynamicCharacterController(t.aisystem.objectstartindex, Modified_fAngleY, fAngleX, norm, fJump, fDucking, fPushAngle, fPushForce, fThrustUpwards);
	}
	#else
		// no VR control
		ODEControlDynamicCharacterController ( t.aisystem.objectstartindex, fAngleY, fAngleX, fSpeed, fJump, fDucking, fPushAngle, fPushForce, fThrustUpwards );
	#endif
	return 0;
}
int GetCharacterHitFloor ( lua_State *L )
{
	lua_pushnumber ( L, ODEGetCharacterHitFloor() );
	return 1;
}
int GetCharacterFallDistance ( lua_State *L )
{
	lua_pushnumber ( L, ODEGetCharacterFallDistance() );
	return 1;
}
int RayTerrain ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 6 ) return 0;
	float fX = lua_tonumber(L, 1);
	float fY = lua_tonumber(L, 2);
	float fZ = lua_tonumber(L, 3);
	float fToX = lua_tonumber(L, 4);
	float fToY = lua_tonumber(L, 5);
	float fToZ = lua_tonumber(L, 6);
	lua_pushnumber ( L, ODERayTerrain(fX, fY, fZ, fToX, fToY, fToZ) );
	return 1;
}
int GetRayCollisionX ( lua_State *L )
{
	lua_pushnumber ( L, ODEGetRayCollisionX() );
	return 1;
}
int GetRayCollisionY ( lua_State *L )
{
	lua_pushnumber ( L, ODEGetRayCollisionY() );
	return 1;
}
int GetRayCollisionZ ( lua_State *L )
{
	lua_pushnumber ( L, ODEGetRayCollisionZ() );
	return 1;
}
int IntersectCore ( lua_State *L, int iStaticOnly )
{
	int n = lua_gettop(L);
	if ( n < 7 ) return 0;
	float fX = lua_tonumber(L, 1);
	float fY = lua_tonumber(L, 2);
	float fZ = lua_tonumber(L, 3);
	float fNewX = lua_tonumber(L, 4);
	float fNewY = lua_tonumber(L, 5);
	float fNewZ = lua_tonumber(L, 6);
	int iIgnoreObjNo = lua_tonumber(L, 7);
	if (g.lightmappedobjectoffset >= g.lightmappedobjectoffsetfinish)
		int ttt = IntersectAll(87000, 87000 + g.merged_new_objects - 1, 0, 0, 0, 0, 0, 0, -123);
	else
		int ttt=IntersectAll(g.lightmappedobjectoffset,g.lightmappedobjectoffsetfinish,0,0,0,0,0,0,-123);
	int tthitvalue=IntersectAllEx(g.entityviewstartobj,g.entityviewendobj,fX, fY, fZ, fNewX, fNewY, fNewZ, iIgnoreObjNo, iStaticOnly);
	lua_pushnumber ( L, tthitvalue );
	return 1;
}
int IntersectAll ( lua_State *L )
{
	return IntersectCore ( L, 0 );
}
int IntersectStatic ( lua_State *L )
{
	return IntersectCore ( L, 1 );
}
int GetIntersectCollisionX ( lua_State *L )
{
	lua_pushnumber ( L, ChecklistFValueA(6) );
	return 1;
}
int GetIntersectCollisionY ( lua_State *L )
{
	lua_pushnumber ( L, ChecklistFValueB(6) );
	return 1;
}
int GetIntersectCollisionZ ( lua_State *L )
{
	lua_pushnumber ( L, ChecklistFValueC(6) );
	return 1;
}
int GetIntersectCollisionNX ( lua_State *L )
{
	lua_pushnumber ( L, ChecklistFValueA(7) );
	return 1;
}
int GetIntersectCollisionNY ( lua_State *L )
{
	lua_pushnumber ( L, ChecklistFValueB(7) );
	return 1;
}
int GetIntersectCollisionNZ ( lua_State *L )
{
	lua_pushnumber ( L, ChecklistFValueC(7) );
	return 1;
}
int PositionCamera ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	PositionCamera ( lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}
int PointCamera ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	PointCamera ( lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}
int MoveCamera ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	MoveCamera ( lua_tonumber(L, 1), lua_tonumber(L, 2) );
	return 0;
}
int GetObjectExist ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L, ObjectExist(lua_tonumber(L, 1)) );
	return 1;
}
int SetObjectFrame ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	SetObjectFrame ( lua_tonumber(L, 1), lua_tonumber(L, 2) );
	return 0;
}
int GetObjectFrame ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L, GetFrame(lua_tonumber(L, 1)) );
	return 1;
}
int SetObjectSpeed ( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 2 ) return 0;
	SetObjectSpeed ( lua_tonumber(L, 1), lua_tonumber(L, 2) );
	return 0;
}
int GetObjectSpeed ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L, GetSpeed(lua_tonumber(L, 1)) );
	return 1;
}
int PositionObject ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	PositionObject ( lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}
int ScaleObjectXYZ(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 4) return 0;
	ScaleObject(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}
// Add Fast Quaternion functions
int QuatMultiply(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 8) return 0;

	GGQUATERNION q1( lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	GGQUATERNION q2( lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8) );
	
	float A = ( q1.w + q1.x ) * ( q2.w + q2.x );
	float B = ( q1.z - q1.y ) * ( q2.y - q2.z );
	float C = ( q1.w - q1.x ) * ( q2.y + q2.z );
	float D = ( q1.y + q1.z ) * ( q2.w - q2.x );
	float E = ( q1.x + q1.z ) * ( q2.x + q2.y );
	float F = ( q1.x - q1.z ) * ( q2.x - q2.y );
	float G = ( q1.w + q1.y ) * ( q2.w - q2.z );
	float H = ( q1.w - q1.y ) * ( q2.w + q2.z );

	q1.w = B + (-E - F + G + H ) / 2;
	q1.x = A - ( E + F + G + H ) / 2;
	q1.y = C + ( E - F + G - H ) / 2;
	q1.z = D + ( E - F - G + H ) / 2;

	lua_pushnumber( L, q1.x );
	lua_pushnumber( L, q1.y );
	lua_pushnumber( L, q1.z );
	lua_pushnumber( L, q1.w );
	return 4;
}
int QuatToEuler(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 4) return 0;

	GGQUATERNION q( lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );

	float sqw = q.w * q.w;
	float sqx = q.x * q.x;
	float sqy = q.y * q.y;
	float sqz = q.z * q.z;

	float h = -2.0 * ( q.x * q.z - q.y * q.w );

	float x, y, z;

	x = -atan2( 2.0 * ( q.y * q.z + q.x * q.w ), ( -sqx - sqy + sqz + sqw ) );
	z = -atan2( 2.0 * ( q.x * q.y + q.z * q.w ), (  sqx - sqy - sqz + sqw ) );

	if ( abs( h ) < 0.99999 )
	{		
		y =  asin( -2.0 * ( q.x * q.z - q.y * q.w ) );
	}
	else
	{
		y = ( PI / 2 ) * h;
	}

	lua_pushnumber( L, x );
	lua_pushnumber( L, y );
	lua_pushnumber( L, z );
	return 3;
}
int EulerToQuat(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 3) return 0;
	float pitch = lua_tonumber( L, 1 );
	float yaw   = lua_tonumber( L, 2 );
	float roll  = lua_tonumber( L, 3 );

	float sr = sin( roll  / 2.0 );
	float sp = sin( pitch / 2.0 );
	float sy = sin( yaw   / 2.0 );
	float cr = cos( roll  / 2.0 );
	float cp = cos( pitch / 2.0 );
	float cy = cos( yaw   / 2.0 );

	float cycp = cy * cp;
	float sysp = sy * sp;
	float sycp = sy * cp;
	float cysp = cy * sp;

	lua_pushnumber( L, ( sr * sycp ) - ( cr * cysp ) );  // q.x
	lua_pushnumber( L, ( sr * cysp ) + ( cr * sycp ) );  // q.y
	lua_pushnumber( L, ( cr * sysp ) - ( sr * cycp ) );  // q.z
	lua_pushnumber( L, ( sr * sysp ) + ( cr * cycp ) );  // q.w
	return 4;
}
int QuatSLERP(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 9) return 0;
	const GGQUATERNION qa(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	const GGQUATERNION qb(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

	GGQUATERNION qOut;

	QuaternionSlerp( &qOut, &qa, &qb, lua_tonumber(L, 9) );

	lua_pushnumber(L, qOut.x);
	lua_pushnumber(L, qOut.y);
	lua_pushnumber(L, qOut.z);
	lua_pushnumber(L, qOut.w);
	return 4;
}
int QuatLERP(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 9) return 0;

	const GGQUATERNION qa(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	const GGQUATERNION qb(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));
	float t = lua_tonumber(L, 9);

	float at = 1.0 - t;
	float bt = t;
	if (qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w < 0) 
	{
		bt = -t;
	}

	lua_pushnumber(L, qa.x * at + qb.x * bt );
	lua_pushnumber(L, qa.y * at + qb.y * bt );
	lua_pushnumber(L, qa.z * at + qb.z * bt );
	lua_pushnumber(L, qa.w * at + qb.w * bt );
	return 4;
}
// end of Fast Quaternion functions
int RotateObject ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	RotateObject ( lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}
int GetObjectAngleX ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L, ObjectAngleX(lua_tonumber(L, 1)) );
	return 1;
}
int GetObjectAngleY ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L, ObjectAngleY(lua_tonumber(L, 1)) );
	return 1;
}
int GetObjectAngleZ ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	lua_pushnumber ( L, ObjectAngleZ(lua_tonumber(L, 1)) );
	return 1;
}
int GetObjectPosAng( lua_State *L )
{
	int n = lua_gettop( L );
	if (n < 1) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance(iID) )
	{
		// seems can be called in LUA when object not exist, so return zeros
		lua_pushnumber ( L, 0 );
		lua_pushnumber ( L, 0 );
		lua_pushnumber ( L, 0 );
		lua_pushnumber ( L, 0 );
		lua_pushnumber ( L, 0 );
		lua_pushnumber ( L, 0 );
		return 6;
	}
	else
	{
		// object information
		sObject* pObject = g_ObjectList[iID];
		lua_pushnumber ( L, pObject->position.vecPosition.x );
		lua_pushnumber ( L, pObject->position.vecPosition.y );
		lua_pushnumber ( L, pObject->position.vecPosition.z );
		lua_pushnumber ( L, pObject->position.vecRotate.x );
		lua_pushnumber ( L, pObject->position.vecRotate.y );
		lua_pushnumber ( L, pObject->position.vecRotate.z );
	}
	return 6;
}
int GetObjectColBox( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iID = lua_tonumber( L, 1 );
	if (!ConfirmObjectInstance( iID ) )
		return 0;
	// object information
	sObject* pObject = g_ObjectList[iID];

	lua_pushnumber( L, pObject->collision.vecMin.x );
	lua_pushnumber( L, pObject->collision.vecMin.y );
	lua_pushnumber( L, pObject->collision.vecMin.z );
	lua_pushnumber( L, pObject->collision.vecMax.x );
	lua_pushnumber( L, pObject->collision.vecMax.y );
	lua_pushnumber( L, pObject->collision.vecMax.z );
	return 6;
}
int GetObjectCentre( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iID = lua_tonumber(L, 1);
	if (!ConfirmObjectInstance(iID))
		return 0;
	// object information
	sObject* pObject = g_ObjectList[iID];

	lua_pushnumber(L, pObject->collision.vecCentre.x);
	lua_pushnumber(L, pObject->collision.vecCentre.y);
	lua_pushnumber(L, pObject->collision.vecCentre.z);
	return 3;
}
int GetObjectScales( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iID = lua_tonumber(L, 1);
	if (!ConfirmObjectInstance(iID))
		return 0;
	// object information
	sObject* pObject = g_ObjectList[iID];

	lua_pushnumber(L, pObject->position.vecScale.x);
	lua_pushnumber(L, pObject->position.vecScale.y);
	lua_pushnumber(L, pObject->position.vecScale.z);
	return 3;
}
int PushObject( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 4) return 0;
	int iID = lua_tonumber(L, 1);
	if (!ConfirmObjectInstance(iID))
		return 0;
	if (n == 7)
	{
		ODEAddBodyForce( iID, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
						 	  lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7));
	}
	else if (n == 4)
	{
		ODEAddBodyForce( iID, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), 0, 0, 0 );
	}
	return 0;
}
int ConstrainObjMotion( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 4) return 0;
	int iID = lua_tonumber(L, 1);
	if (!ConfirmObjectInstance(iID))
		return 0; 
	ODEConstrainBodyMotion( iID, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}
int ConstrainObjRotation( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 4 ) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance(iID) )
		return 0;
	ODEConstrainBodyRotation( iID, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}
int CreateSingleHinge( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 7 ) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance(iID) )
		return 0;
	int iC = ODECreateHingeSingle( iID, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
		                                lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7) );
	lua_pushnumber( L, iC );
	return 1;
}
int CreateDoubleHinge( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 11 ) return 0;
	int iIDa = lua_tonumber(L, 1);
	int iIDb = lua_tonumber(L, 2);
	if (!ConfirmObjectInstance(iIDa) || !ConfirmObjectInstance(iIDb))
		return 0;

	int iC = ODECreateHingeDouble( iIDa, iIDb, lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5),
		                                       lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8),
		                                       lua_tonumber(L, 9), lua_tonumber(L, 10), lua_tonumber(L, 11) );
	lua_pushnumber( L, iC );
	return 1;
}
int CreateSingleJoint( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 4 ) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance( iID ) )
		return 0;

	int iC = ODECreateJointSingle( iID, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	lua_pushnumber( L, iC );
	return 1;
}
int CreateDoubleJoint( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 9 ) return 0;
	int iIDa = lua_tonumber( L, 1 );
	int iIDb = lua_tonumber( L, 2 );
	if ( !ConfirmObjectInstance( iIDa ) || !ConfirmObjectInstance( iIDb ) )
		return 0;

	int iC = ODECreateJointDouble( iIDa, iIDb, lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5),
		                                       lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8),
		                                       lua_tonumber(L, 9) );
	lua_pushnumber( L, iC );
	return 1;
}
int CreateSliderDouble( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 17 ) return 0;
	int iIDa = lua_tonumber( L, 1 );
	int iIDb = lua_tonumber( L, 2 );
	if ( !ConfirmObjectInstance( iIDa ) || !ConfirmObjectInstance( iIDb ) )
		return 0;

	int iC = ODECreateSliderDouble( iIDa, iIDb, lua_tonumber(L,  3), lua_tonumber(L,  4), lua_tonumber(L,  5),
		                                        lua_tonumber(L,  6), lua_tonumber(L,  7), lua_tonumber(L,  8),
		                                        lua_tonumber(L,  9), lua_tonumber(L, 10), lua_tonumber(L, 11),
		                                        lua_tonumber(L, 12), lua_tonumber(L, 13), lua_tonumber(L, 14),
		                                        lua_tonumber(L, 15), lua_tonumber(L, 16), lua_tonumber(L, 17) == 1 );
	lua_pushnumber( L, iC );
	return 1;
}
int SetSliderLimits(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 5) return 0;
	int iC = lua_tonumber(L, 1);

	ODESetSliderLimits( iC, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5) );
	return 0;
}

int RemoveObjectConstraints( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 1 ) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance( iID ) )
		return 0;

	ODERemoveBodyConstraints( iID );
	return 0;
}
int RemoveConstraint( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iC = lua_tonumber( L, 1 );

	ODERemoveConstraint( iC );
	return 0;
}
int SetObjectDamping( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 3) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance( iID ) )
		return 0;

	ODESetBodyDamping( iID, lua_tonumber( L, 2 ), lua_tonumber( L, 3 ) );

	return 0;
}
int SetHingeLimits(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 3) return 0;
	int iC = lua_tonumber(L, 1);

	if (n < 4)
	{
		// Set angle min/max
		ODESetHingeLimits(iC, lua_tonumber(L, 2), lua_tonumber(L, 3), 0.9f, 0.3f, 1.0f);
	}
	else if (n < 5)
	{
		// also set softness
		ODESetHingeLimits(iC, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), 0.3f, 1.0f);
	}
	else if (n < 6)
	{
		// also set bias
		ODESetHingeLimits(iC, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
			lua_tonumber(L, 5), 1.0f);
	}
	else
	{
		// also set relaxation
		ODESetHingeLimits(iC, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
			lua_tonumber(L, 5), lua_tonumber(L, 6));
	}
	return 0;
}
int SetHingeMotor( lua_State *L )
{
	int n = lua_gettop(L);
	if (n < 4) return 0;
	int iC = lua_tonumber(L, 1);

	ODESetHingeMotor(iC, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}
int SetSliderMotor(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 4) return 0;
	int iC = lua_tonumber(L, 1);

	ODESetSliderMotor( iC, lua_tonumber(L, 2) == 1, lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}
int GetHingeAngle(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iC = lua_tonumber(L, 1);

	lua_pushnumber(L, ODEGetHingeAngle(iC));
	return 1;
}
int GetSliderPosition(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iC = lua_tonumber(L, 1);

	lua_pushnumber( L, ODEGetSliderPosition( iC ) );
	return 1;
}
int SetBodyScaling(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 4) return 0;
	int iC = lua_tonumber(L, 1);

	ODESetBodyScaling( iC, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4) );
	return 0;
}

int PhysicsRayCast( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 7 ) return 0;
	float fFromX = lua_tonumber( L, 1 );
	float fFromY = lua_tonumber( L, 2 );
	float fFromZ = lua_tonumber( L, 3 );
	float fToX   = lua_tonumber( L, 4 );
	float fToY   = lua_tonumber( L, 5 );
	float fToZ   = lua_tonumber( L, 6 );
	float fForce = lua_tonumber( L, 7 );
	if ( ODERayForce( fFromX, fFromY, fFromZ, fToX, fToY, fToZ, fForce ) == 1 )
	{
		int iObjHit = ODEGetRayObjectHit();
		// only return dynamic objects
		if ( ODEGetBodyIsDynamic( iObjHit ) )
		{
			lua_pushnumber( L, iObjHit );
			lua_pushnumber( L, ODEGetRayCollisionX() );
			lua_pushnumber( L, ODEGetRayCollisionY() );
			lua_pushnumber( L, ODEGetRayCollisionZ() );
			return 4;
		}
	}
	lua_pushnumber( L, 0 );
	return 1;
}
int GetObjectNumCollisions(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iID = lua_tonumber(L, 1);
	if (!ConfirmObjectInstance(iID))
		return 0;
	lua_pushnumber(L, ODEGetBodyNumCollisions(iID));
	return 1;
}
int GetObjectCollisionDetails( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 1 ) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance( iID ) )
		return 0;
	int colNum = 0;
	int iColObj = 0;
	float fX, fY, fZ, fF;
	if (n == 2) colNum = lua_tonumber(L, 2);
	ODEGetBodyCollisionDetails( iID, colNum, iColObj, fX, fY, fZ, fF );
	lua_pushnumber( L, iColObj );
	lua_pushnumber( L, fX );
	lua_pushnumber( L, fY );
	lua_pushnumber( L, fZ );
	lua_pushnumber( L, fF );

	return 5;
}
int GetTerrainNumCollisions( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 1 ) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance( iID ) )
		return 0;
	lua_pushnumber( L, ODEGetTerrainNumCollisions( iID ) );
	return 1;
}
int GetTerrainCollisionDetails( lua_State *L )
{
	int n = lua_gettop( L );
	if ( n < 1 ) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance( iID ) )
		return 0;
	int colNum = 0;
	int iLatest = 0;
	float fX, fY, fZ;
	if (n == 2) colNum = lua_tonumber( L, 2 );
	ODEGetTerrainCollisionDetails( iID, colNum, iLatest, fX, fY, fZ );
	lua_pushnumber( L, iLatest );
	lua_pushnumber( L, fX );
	lua_pushnumber( L, fY );
	lua_pushnumber( L, fZ );

	return 4;
}
int AddObjectCollisionCheck( lua_State *L )
{
	int n = lua_gettop( L );
	if (n < 1) return 0;
	int iID = lua_tonumber( L, 1 );
	if ( !ConfirmObjectInstance( iID ) )
		return 0;
	ODEAddBodyCollisionCheck( iID );
	return 0;
}
int RemoveObjectCollisionCheck(lua_State *L)
{
	int n = lua_gettop(L);
	if (n < 1) return 0;
	int iID = lua_tonumber(L, 1);
	if (!ConfirmObjectInstance(iID))
		return 0;
	ODERemoveBodyCollisionCheck(iID);
	return 0;
}

// Lua control of dynamic light
// get the light number using entity e number 
// then use that in the other light functions
int GetEntityLightNumber( lua_State *L )
{
	lua = L;
	int n = lua_gettop( L );
	if ( n < 1 ) return 0;

	// get lightentity e number
	int iID = lua_tonumber( L, 1 );
	if ( iID <= 0 ) return 0;

	for ( int i = 1; i <= g.infinilightmax; i++)
	{
		if ( t.infinilight[ i ].used == 1 && t.infinilight[ i ].e == iID )
		{
			lua_pushinteger( L, i );
			return 1;
		}
	}
	return 0;
}
int GetLightPosition( lua_State *L )
{
	lua = L;
	int n = lua_gettop( L );
	if ( n < 1 )
		return 0;

	// get light number
	int i = lua_tointeger( L, 1 );

	if ( i > 0 && i <= g.infinilightmax && t.infinilight[ i ].used == 1 )
	{
		lua_pushnumber( L, t.infinilight[i].x );
		lua_pushnumber( L, t.infinilight[i].y );
		lua_pushnumber( L, t.infinilight[i].z );
		return 3;
	}
	return 0;
}

//PE: Needed function to make light rotate, now that we have strange Z int eular angles.
//RotateGlobalAngleY(ax,ay,az,NewAngleY)
int RotateGlobalAngleY(lua_State *L)
{
	lua = L;
	// get number of arguments
	int n = lua_gettop(L);
	// Not enough params, return out
	if (n < 4)
		return 0;

	float fAngleX = lua_tonumber(L, 1);
	float fAngleY = lua_tonumber(L, 2);
	float fAngleZ = lua_tonumber(L, 3);
	
	//Matrix
	float fNewAngleY = lua_tonumber(L, 4);

	//PE: Rotate Matrix.

	//New eular.
	lua_pushnumber(L, fAngleX);
	lua_pushnumber(L, fAngleY);
	lua_pushnumber(L, fAngleZ);
	return 3;
}


int GetLightAngle(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 1)
		return 0;

	// get light number
	int i = lua_tointeger(L, 1);

	if (i > 0 && i <= g.infinilightmax && t.infinilight[i].used == 1)
	{
		float fAngleX = t.infinilight[i].f_angle_x;
		float fAngleY = t.infinilight[i].f_angle_y;
		float fAngleZ = t.infinilight[i].f_angle_z;

		void FixEulerZInverted(float &ax, float &ay, float &az);
		FixEulerZInverted(fAngleX, fAngleY, fAngleZ);


		lua_pushnumber(L, fAngleX);
		lua_pushnumber(L, fAngleY);
		lua_pushnumber(L, fAngleZ);
		return 3;
	}
	return 0;
}
int GetLightRGB( lua_State *L )
{
	lua = L;
	int n = lua_gettop( L );
	if ( n < 1 )
		return 0;

	// get light number
	int i = lua_tointeger( L, 1 );

	if (i > 0 && i <= g.infinilightmax && t.infinilight[i].used == 1)
	{
		lua_pushnumber( L, t.infinilight[i].colrgb.r );
		lua_pushnumber( L, t.infinilight[i].colrgb.g );
		lua_pushnumber( L, t.infinilight[i].colrgb.b );
		return 3;
	}
	return 0;
}
int GetLightRange(lua_State *L)
{
	lua = L;
	int n = lua_gettop( L );
	if ( n < 1 )
		return 0;

	// get light number
	int i = lua_tointeger( L, 1 );

	if ( i > 0 && i <= g.infinilightmax && t.infinilight[ i ].used == 1 )
	{
		lua_pushnumber( L, t.infinilight[ i ].range );
		return 1;
	}
	return 0;
}
// uses light number from above
int SetLightPosition( lua_State *L )
{
	lua = L;
	// get number of arguments
	int n = lua_gettop( L );
	// Not enough params, return out
	if ( n < 4 )
		return 0;

	// get light number
	int i = lua_tonumber( L, 1 );

	if ( i > 0 && i <= g.infinilightmax && t.infinilight[ i ].used == 1 )
	{
		t.infinilight[ i ].x = lua_tonumber( L, 2 );
		t.infinilight[ i ].y = lua_tonumber( L, 3 );
		t.infinilight[ i ].z = lua_tonumber( L, 4 );
	}
	return 0;
}

float QuickEulerWrapAngle(float Angle)
{
	float NewAngle = fmod(Angle, 360.0f);
	if (NewAngle < 0.0f)
		NewAngle += 360.0f;
	return NewAngle;
}

int SetLightAngle(lua_State *L)
{
	lua = L;
	// get number of arguments
	int n = lua_gettop(L);
	// Not enough params, return out
	if (n < 4)
		return 0;

	// get light number
	int i = lua_tonumber(L, 1);

	if (i > 0 && i <= g.infinilightmax && t.infinilight[i].used == 1)
	{
		t.infinilight[i].f_angle_x = lua_tonumber(L, 2);
		t.infinilight[i].f_angle_y = lua_tonumber(L, 3);
		t.infinilight[i].f_angle_z = lua_tonumber(L, 4);

	}
	return 0;
}
int SetLightRGB( lua_State *L ) 
{
	lua = L;
	// get number of arguments
	int n = lua_gettop( L );
	// Not enough params, return out
	if ( n < 4 )
		return 0;

	// get light number
	int i = lua_tonumber( L, 1 );

	if ( i > 0 && i <= g.infinilightmax && t.infinilight[ i ].used == 1 )
	{
		t.infinilight[ i ].colrgb.r = lua_tonumber( L, 2 );
		t.infinilight[ i ].colrgb.g = lua_tonumber( L, 3 );
		t.infinilight[ i ].colrgb.b = lua_tonumber( L, 4 );
	}
	return 0;
}

int SetLightRange( lua_State *L )
{
	lua = L;
	// get number of arguments
	int n = lua_gettop(L);
	// Not enough params, return out
	if (n < 2)
		return 0;

	// get light number
	int i = lua_tointeger(L, 1);

	if ( i > 0 && i <= g.infinilightmax && t.infinilight[i].used == 1 )
	{
		float rng = lua_tonumber(L, 2);
		if ( rng < 1.0f )
		{
			rng = 1.0f;
		}
		else if ( rng > 10000.0f )
		{
			rng = 10000.0f;
		}
		t.infinilight[ i ].range = rng;
	}
	return 0;
}

int RunCharLoop ( lua_State *L )
{
	// run character animation system
	char_loop ( );
	return 0;
}
int TriggerWaterRipple ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	g.decalx=lua_tonumber(L, 1);
	g.decaly=lua_tonumber(L, 2);
	g.decalz=lua_tonumber(L, 3);
	decal_triggerwaterripple ( );
	return 0;
}
int PlayFootfallSound ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 5 ) return 0;
	int footfalltype = lua_tonumber(L, 1);
	float fX = lua_tonumber(L, 2);
	float fY = lua_tonumber(L, 3);
	float fZ = lua_tonumber(L, 4);
	int lastfootfallsound = lua_tonumber(L, 5);
	t.trndsnd = Rnd(3);
	if ( t.trndsnd == 0 ) t.tsnd = t.material[footfalltype].tred0id;
	if ( t.trndsnd == 1 ) t.tsnd = t.material[footfalltype].tred1id;
	if ( t.trndsnd == 2 ) t.tsnd = t.material[footfalltype].tred2id;
	if ( t.trndsnd == 3 ) t.tsnd = t.material[footfalltype].tred3id;
	if ( t.tsnd>0 ) 
	{
		if ( t.trndsnd == lastfootfallsound ) 
		{
			t.trndsnd = t.trndsnd+1 ; if ( t.trndsnd>3 ) t.trndsnd = 0;
			if ( t.trndsnd == 0 ) t.tsnd = t.material[footfalltype].tred0id;
			if ( t.trndsnd == 1 ) t.tsnd = t.material[footfalltype].tred1id;
			if ( t.trndsnd == 2 ) t.tsnd = t.material[footfalltype].tred2id;
			if ( t.trndsnd == 3 ) t.tsnd = t.material[footfalltype].tred3id;
		}
		lastfootfallsound = t.trndsnd;

		//  play this material sound (will play tsnd+0 through tsnd+4)
		float fSoundVolPerc = t.playercontrol.footfallvolume_f * 100.0f;
		if (fSoundVolPerc < 0) fSoundVolPerc = 0;
		if (fSoundVolPerc > 100) fSoundVolPerc = 100;
		t.tsoundtrigger = t.tsnd; t.tvol_f = fSoundVolPerc;
		t.tspd_f = t.material[footfalltype].freq;
		t.tsx_f = fX;
		t.tsy_f = fY;
		t.tsz_f = fZ;
		material_triggersound ( 1 );
	}
	lua_pushnumber ( L, lastfootfallsound );
	return 1;
}
int ResetUnderwaterState ( lua_State *L )
{
	physics_player_reset_underwaterstate();
	return 0;
}
int SetUnderwaterOn ( lua_State *L )
{
	visuals_underwater_on();
	return 0;
}
int SetUnderwaterOff ( lua_State *L )
{
	visuals_underwater_off();
	return 0;
}

// Set Shader Values

int SetShaderVariable ( lua_State *L )
{
	int n = lua_gettop(L);
	if ( n < 6 ) return 0;
	int iShaderIndex = lua_tonumber(L, 1);
	char pConstantName[512];
	strcpy ( pConstantName, lua_tostring(L, 2) );
	float fValue1 = lua_tonumber(L, 3);
	float fValue2 = lua_tonumber(L, 4);
	float fValue3 = lua_tonumber(L, 5);
	float fValue4 = lua_tonumber(L, 6);
	int iShaderIndexStart = 1;
	int iShaderIndexFinish = 2000;
	if ( iShaderIndex > 0 ) { iShaderIndexStart = iShaderIndex; iShaderIndexFinish = iShaderIndex; }
	SetVector4 ( g.terrainvectorindex1, fValue1, fValue2, fValue3, fValue4 );
	for ( int iSI = iShaderIndexStart; iSI <= iShaderIndexFinish; iSI++ )
	{
		if ( GetEffectExist ( iSI ) == 1 ) 
		{
			DWORD pConstantPtr = GetEffectParameterIndex ( iSI, pConstantName );
			if ( pConstantPtr ) 
			{
				SetEffectConstantVEx( iSI, pConstantPtr, g.terrainvectorindex1 );
			}
		}
	}
	return 0;
}

//Control Water Shader
//setter
int SetWaterHeight(lua_State *L) {
	t.terrain.waterliney_f = lua_tonumber(L, 1);
	terrain_updatewaterphysics();
	return 0;
}
int SetWaterShaderColor(lua_State *L) {
	t.visuals.WaterRed_f = lua_tonumber(L, 1);
	t.visuals.WaterGreen_f = lua_tonumber(L, 2);
	t.visuals.WaterBlue_f = lua_tonumber(L, 3);
	SetVector4(g.terrainvectorindex, t.visuals.WaterRed_f / 256, t.visuals.WaterGreen_f / 256, t.visuals.WaterBlue_f / 256, 0);
	SetEffectConstantV(t.terrain.effectstartindex + 1, "WaterCol", g.terrainvectorindex);
	return 0;
}
int SetWaterWaveIntensity(lua_State *L){
	t.visuals.WaterWaveIntensity_f = lua_tonumber(L, 1);
	SetVector4(g.terrainvectorindex, t.visuals.WaterWaveIntensity_f, t.visuals.WaterWaveIntensity_f, 0, 0);
	SetEffectConstantV(t.terrain.effectstartindex + 1, "nWaterScale", g.terrainvectorindex);
	return 0;
}
int SetWaterTransparancy(lua_State *L){
	t.visuals.WaterTransparancy_f = lua_tonumber(L, 1);
	SetEffectConstantF(t.terrain.effectstartindex + 1, "WaterTransparancy", t.visuals.WaterTransparancy_f);
	return 0;
}
int SetWaterReflection(lua_State *L){
	t.visuals.WaterReflection_f = lua_tonumber(L, 1);
	SetEffectConstantF(t.terrain.effectstartindex + 1, "WaterReflection", t.visuals.WaterReflection_f);
	return 0;
}
int SetWaterReflectionSparkleIntensity(lua_State *L){
	t.visuals.WaterReflectionSparkleIntensity = lua_tonumber(L, 1);
	SetEffectConstantF(t.terrain.effectstartindex + 1, "reflectionSparkleIntensity", t.visuals.WaterReflectionSparkleIntensity);
	return 0;
}
int SetWaterFlowDirection(lua_State *L){
	t.visuals.WaterFlowDirectionX = lua_tonumber(L, 1);
	t.visuals.WaterFlowDirectionY = lua_tonumber(L, 2);
	t.visuals.WaterFlowSpeed = lua_tonumber(L, 3);
	SetVector4(g.terrainvectorindex, t.visuals.WaterFlowDirectionX*t.visuals.WaterFlowSpeed, t.visuals.WaterFlowDirectionY*t.visuals.WaterFlowSpeed, 0, 0);
	SetEffectConstantV(t.terrain.effectstartindex + 1, "flowdirection", g.terrainvectorindex);
	return 0;
}
int SetWaterDistortionWaves(lua_State *L){
	t.visuals.WaterDistortionWaves = lua_tonumber(L, 1);
	SetEffectConstantF(t.terrain.effectstartindex + 1, "distortion2", t.visuals.WaterDistortionWaves);
	return 0;
}
int SetRippleWaterSpeed(lua_State *L){
	t.visuals.WaterSpeed1 = lua_tonumber(L, 1);
	SetEffectConstantF(t.terrain.effectstartindex + 1, "WaterSpeed1", t.visuals.WaterSpeed1);
	return 0;
}
//getter
int GetWaterHeight(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.terrain.waterliney_f);
	return 1;
}
int GetWaterWaveIntensity(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterWaveIntensity_f);
	return 1;
}
int GetWaterShaderColorRed(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterRed_f);
	return 1;
}
int GetWaterShaderColorGreen(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterGreen_f);
	return 1;
}
int GetWaterShaderColorBlue(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterBlue_f);
	return 1;
}
int GetWaterTransparancy(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterTransparancy_f);
	return 1;
}
int GetWaterReflection(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterReflection_f);
	return 1;
}
int GetWaterReflectionSparkleIntensity(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterReflectionSparkleIntensity);
	return 1;
}
int GetWaterFlowDirectionX(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterFlowDirectionX);
	return 1;
}
int GetWaterFlowDirectionY(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterFlowDirectionY);
	return 1;
}
int GetWaterFlowSpeed(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterFlowSpeed);
	return 1;
}
int GetWaterDistortionWaves(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterDistortionWaves);
	return 1;
}
int GetRippleWaterSpeed(lua_State *L)
{
	lua = L;
	lua_pushnumber(L, t.visuals.WaterSpeed1);
	return 1;
}

int GetIsTestgame(lua_State *L) {
	lua = L;
	if ((t.game.gameisexe == 0 || g.gprofileinstandalone == 1) && t.game.runasmultiplayer == 0) 
	{
		lua_pushnumber(L, 1);
	}
	else 
	{
		lua_pushnumber(L, 0);
	}
	return 1;
}

//Dynamic sun.

int SetSunDirection(lua_State *L) {
	//Set sun direction -1 to 1 in all directions.

	//Sun use a position and then points to 0,0,0: so just expand in the direction set.
	float fDirX = lua_tonumber(L, 1) * 1000.0f;
	float fDirY = lua_tonumber(L, 2) * 1000.0f;
	float fDirZ = lua_tonumber(L, 3) * 1000.0f;
	t.terrain.sundirectionx_f = fDirX;
	t.terrain.sundirectiony_f = fDirY;
	t.terrain.sundirectionz_f = fDirZ;

	t.tav_f = abs(t.terrain.sundirectionx_f) + abs(t.terrain.sundirectiony_f) + abs(t.terrain.sundirectionz_f);
	t.terrain.sundirectionx_f = t.terrain.sundirectionx_f / t.tav_f;
	t.terrain.sundirectiony_f = t.terrain.sundirectiony_f / t.tav_f;
	t.terrain.sundirectionz_f = t.terrain.sundirectionz_f / t.tav_f;

	//PE: When moving the sun we need to disable speedshadows.
	g.globals.speedshadows = 0;
	return 0;
}


// Game Player Control/State Set/Get commands

int SetGamePlayerControlData ( lua_State *L, int iDataMode )
{
	lua = L;
	int iSrc = 0;
	int iDest = 0;
	int n = lua_gettop(L);
	if ( iDataMode < 500 )
	{
		if ( n < 1 ) return 0;
	}
	else
	{
		if ( n < 2 ) return 0;
	}
	int gunId = t.gunid;
	int fireMode = g.firemode;
	int param = 1;
	if ( n > 1 && iDataMode > 200 && iDataMode < 500 ) 
	{
		gunId = lua_tonumber( L, 1 );
		if ( n == 2 )
		{
			fireMode = 0;
			param = 2;
		}
		else
		{
			fireMode = lua_tonumber( L, 2 );
			param = 3;
		}
	}
	switch ( iDataMode )
	{
		case 1 : t.playercontrol.jetpackmode = lua_tonumber(L, 1); break;
		case 2 : t.playercontrol.jetpackfuel_f = lua_tonumber(L, 1); break;
		case 3 : t.playercontrol.jetpackhidden = lua_tonumber(L, 1); break;
		case 4 : t.playercontrol.jetpackcollected = lua_tonumber(L, 1); break;
		case 5 : t.playercontrol.soundstartindex = lua_tonumber(L, 1); break;
		case 6 : t.playercontrol.jetpackparticleemitterindex = lua_tonumber(L, 1); break;
		case 7 : t.playercontrol.jetpackthrust_f = lua_tonumber(L, 1); break;
		case 8 : t.playercontrol.startstrength = lua_tonumber(L, 1); break;

		case 9 : t.playercontrol.isrunning = lua_tonumber(L, 1); break;
		case 10 : break;
		case 11 : t.playercontrol.cx_f = lua_tonumber(L, 1); break;
		case 12 : t.playercontrol.cy_f = lua_tonumber(L, 1); break;
		case 13 : t.playercontrol.cz_f = lua_tonumber(L, 1); break;
		case 14 : 
		{
#ifdef FASTBULLETPHYSICS
			t.playercontrol.basespeed_f *= 2.0; //FASTBULLETPHYSICS need to move faster.
#else
			t.playercontrol.basespeed_f = lua_tonumber(L, 1);
			extern bool bPhysicsRunningAt120FPS;
			if (!bPhysicsRunningAt120FPS)
				t.playercontrol.basespeed_f *= 2.0;
#endif
			break;
		}
		case 15 : t.playercontrol.canrun = lua_tonumber(L, 1); break;
		case 16 : t.playercontrol.maxspeed_f = lua_tonumber(L, 1); break;
		case 17 : t.playercontrol.topspeed_f = lua_tonumber(L, 1); break;
		case 18 : t.playercontrol.movement = lua_tonumber(L, 1); break;
		case 19 : t.playercontrol.movey_f = lua_tonumber(L, 1); break;
		case 20 : t.playercontrol.lastmovement = lua_tonumber(L, 1); break;
		case 21 : t.playercontrol.footfallcount = lua_tonumber(L, 1); break;
		case 22 : break;
		case 23 : t.playercontrol.gravityactive = lua_tonumber(L, 1); break;
		case 24 : t.playercontrol.plrhitfloormaterial = lua_tonumber(L, 1); break;
		case 25 : t.playercontrol.underwater = lua_tonumber(L, 1); break;
		case 26 : t.playercontrol.jumpmode = lua_tonumber(L, 1); break;
		case 27 : t.playercontrol.jumpmodecanaffectvelocitycountdown_f = lua_tonumber(L, 1); break;
		case 28 : t.playercontrol.speed_f = lua_tonumber(L, 1); break;
		case 29 : t.playercontrol.accel_f = lua_tonumber(L, 1); break;
		case 30 : t.playercontrol.speedratio_f = lua_tonumber(L, 1); break;
		case 31 : t.playercontrol.wobble_f = lua_tonumber(L, 1); break;
		case 32 : t.playercontrol.wobblespeed_f = lua_tonumber(L, 1); break;
		case 33 : t.playercontrol.wobbleheight_f = lua_tonumber(L, 1); break;
		case 34 : t.playercontrol.jumpmax_f = lua_tonumber(L, 1); break;
		case 35 : t.playercontrol.pushangle_f = lua_tonumber(L, 1); break;
		case 36 : t.playercontrol.pushforce_f = lua_tonumber(L, 1); break;
		case 37 : t.playercontrol.footfallpace_f = lua_tonumber(L, 1); break;
		case 38 : t.playercontrol.lockatheight = lua_tonumber(L, 1); break;
		case 39 : t.playercontrol.controlheight = lua_tonumber(L, 1); break;
		case 40 : t.playercontrol.controlheightcooldown = lua_tonumber(L, 1); break;
		case 41 : t.playercontrol.storemovey = lua_tonumber(L, 1); break;
		case 42 : break;
		case 43 : t.playercontrol.hurtfall = lua_tonumber(L, 1); break;
		case 44 : t.playercontrol.leanoverangle_f = lua_tonumber(L, 1); break;
		case 45 : t.playercontrol.leanover_f = lua_tonumber(L, 1); break;
		case 46 : t.playercontrol.camerashake_f = lua_tonumber(L, 1); break;
		case 47 : t.playercontrol.finalcameraanglex_f = lua_tonumber(L, 1); break;
		case 48 : t.playercontrol.finalcameraangley_f = lua_tonumber(L, 1); break;
		case 49 : t.playercontrol.finalcameraanglez_f = lua_tonumber(L, 1); break;
		case 50 : t.playercontrol.camrightmousemode = lua_tonumber(L, 1); break;
		case 51 : t.playercontrol.camcollisionsmooth = lua_tonumber(L, 1); break;
		case 52 : t.playercontrol.camcurrentdistance = lua_tonumber(L, 1); break;
		case 53 : t.playercontrol.camdofullraycheck = lua_tonumber(L, 1); break;
		case 54 : t.playercontrol.lastgoodcx_f = lua_tonumber(L, 1); break;
		case 55 : t.playercontrol.lastgoodcy_f = lua_tonumber(L, 1); break;
		case 56 : t.playercontrol.lastgoodcz_f = lua_tonumber(L, 1); break;
		case 57 : break;
		case 58 : t.playercontrol.flinchx_f = lua_tonumber(L, 1); break;
		case 59 : t.playercontrol.flinchy_f = lua_tonumber(L, 1); break;
		case 60 : t.playercontrol.flinchz_f = lua_tonumber(L, 1); break;
		case 61 : t.playercontrol.flinchcurrentx_f = lua_tonumber(L, 1); break;
		case 62 : t.playercontrol.flinchcurrenty_f = lua_tonumber(L, 1); break;
		case 63 : t.playercontrol.flinchcurrentz_f = lua_tonumber(L, 1); break;
		case 64 : t.playercontrol.footfalltype = lua_tonumber(L, 1); break;
		case 65 : t.playercontrol.ripplecount_f = lua_tonumber(L, 1); break;
		case 66 : t.playercontrol.lastfootfallsound = lua_tonumber(L, 1); break;
		case 67 : t.playercontrol.inwaterstate = lua_tonumber(L, 1); break;
		case 68 : t.playercontrol.drowntimestamp = lua_tonumber(L, 1); break;
		case 69 : t.playercontrol.deadtime = lua_tonumber(L, 1); break;
		case 70 : t.playercontrol.swimtimestamp = lua_tonumber(L, 1); break;
		case 71 : t.playercontrol.redDeathFog_f = lua_tonumber(L, 1); break;
		case 72 : t.playercontrol.heartbeatTimeStamp = lua_tonumber(L, 1); break;
		case 81 : t.playercontrol.thirdperson.enabled = lua_tonumber(L, 1); break;
		case 82 : t.playercontrol.thirdperson.characterindex = lua_tonumber(L, 1); break;
		case 83 : t.playercontrol.thirdperson.camerafollow = lua_tonumber(L, 1); break;
		case 84 : t.playercontrol.thirdperson.camerafocus = lua_tonumber(L, 1); break;
		case 85 : t.playercontrol.thirdperson.charactere = lua_tonumber(L, 1); break;
		case 86 : break;
		case 87 : t.playercontrol.thirdperson.shotfired = lua_tonumber(L, 1); break;
		case 88 : t.playercontrol.thirdperson.cameradistance = lua_tonumber(L, 1); break;
		case 89 : t.playercontrol.thirdperson.cameraspeed = lua_tonumber(L, 1); break;
		case 90 : t.playercontrol.thirdperson.cameralocked = lua_tonumber(L, 1); break;
		case 91 : t.playercontrol.thirdperson.cameraheight = lua_tonumber(L, 1); break;
		case 92 : t.playercontrol.thirdperson.camerashoulder = lua_tonumber(L, 1); break;

		case 99 : break;
		case 101 : t.gunmode = lua_tonumber(L, 1); break;
		case 102 : t.player[t.plrid].state.firingmode = lua_tonumber(L, 1); break;
		case 103 : g.weaponammoindex = lua_tonumber(L, 1); break;
		case 104 : g.ammooffset = lua_tonumber(L, 1); break;
		case 105 : g.ggunmeleekey = lua_tonumber(L, 1); break;
		case 106 : t.player[t.plrid].state.blockingaction = lua_tonumber(L, 1); break;
		case 107 : t.gunshootnoammo = lua_tonumber(L, 1); break;
		case 108 : g.playerunderwater = lua_tonumber(L, 1); break;
		case 109 : g.gdisablerightmousehold = lua_tonumber(L, 1); break;
		case 110 : g.gxbox = lua_tonumber(L, 1); break;
		case 111 : break;
		case 112 : break;
		case 113 : break;
		case 114 : t.gunzoommode = lua_tonumber(L, 1); break;
		case 115 : t.gunzoommag_f = lua_tonumber(L, 1); break;
		case 116 : t.gunreloadnoammo = lua_tonumber(L, 1); break;
		case 117 : g.plrreloading = lua_tonumber(L, 1); break;
		case 118 : g.ggunaltswapkey1 = lua_tonumber(L, 1); break;
		case 119 : g.ggunaltswapkey2 = lua_tonumber(L, 1); break;
		case 120 : t.weaponkeyselection = lua_tonumber(L, 1); break;
		case 121 : t.weaponindex = lua_tonumber(L, 1); break;
		case 122 : t.player[t.plrid].command.newweapon = lua_tonumber(L, 1); break;
		case 123 : t.gunid = lua_tonumber(L, 1); break;
		case 124 : t.gunselectionafterhide = lua_tonumber(L, 1); break;
		case 125 : t.gunburst = lua_tonumber(L, 1); break;
		case 126 : break;
		case 127 : break;
		case 128 : break;
		case 129 : break;
		case 130 : t.plrzoominchange = lua_tonumber(L, 1); break;
		case 131 : t.plrzoomin_f = lua_tonumber(L, 1); break;
		case 132 : g.luaactivatemouse = lua_tonumber(L, 1); break;
		case 133 : g.realfov_f = lua_tonumber(L, 1); break;
		case 134 : g.gdisablepeeking = lua_tonumber(L, 1); break;
		case 135 : t.plrhasfocus = lua_tonumber(L, 1); break;
		case 136 : t.game.runasmultiplayer = lua_tonumber(L, 1); break;
		case 137 : g.mp.respawnLeft = lua_tonumber(L, 1); break;
		case 138 : g.tabmode = lua_tonumber(L, 1); break;
		case 139 : g.lowfpswarning = lua_tonumber(L, 1); break;
		case 140 : t.visuals.CameraFOV_f = lua_tonumber(L, 1); break;
		case 141 : t.visuals.CameraFOVZoomed_f = lua_tonumber(L, 1); break;
		case 142 : g.gminvert = lua_tonumber(L, 1); break;
		case 143 : t.plrkeySLOWMOTION = lua_tonumber(L, 1); break;
		case 144 : g.globals.smoothcamerakeys = lua_tonumber(L, 1); break;
		case 145 : t.cammousemovex_f = lua_tonumber(L, 1); break;
		case 146 : t.cammousemovey_f = lua_tonumber(L, 1); break;
		case 147 : g.gunRecoilX_f = lua_tonumber(L, 1); break;
		case 148 : g.gunRecoilY_f = lua_tonumber(L, 1); break;
		case 149 : g.gunRecoilAngleX_f = lua_tonumber(L, 1); break;
		case 150 : g.gunRecoilAngleY_f = lua_tonumber(L, 1); break;
		case 151 : t.gunRecoilCorrectY_f = lua_tonumber(L, 1); break;
		case 152 : g.gunRecoilCorrectX_f = lua_tonumber(L, 1); break;
		case 153 : g.gunRecoilCorrectAngleY_f = lua_tonumber(L, 1); break;
		case 154 : t.gunRecoilCorrectAngleX_f = lua_tonumber(L, 1); break;
		case 155 : t.camangx_f = lua_tonumber(L, 1); break;
		case 156 : t.camangy_f = lua_tonumber(L, 1); break;
		case 157 : t.aisystem.playerducking = lua_tonumber(L, 1); break;
		case 158 : t.conkit.editmodeactive = lua_tonumber(L, 1); break;
		case 159 : t.plrkeySHIFT = lua_tonumber(L, 1); break;
		case 160 : t.plrkeySHIFT2 = lua_tonumber(L, 1); break;
		case 161 : t.inputsys.keycontrol = lua_tonumber(L, 1); break;
		case 162 : t.hardwareinfoglobals.nowater = lua_tonumber(L, 1); break;
		case 163 : t.terrain.waterliney_f = lua_tonumber(L, 1); break;
		case 164 : g.flashLightKeyEnabled = lua_tonumber(L, 1); break;
		case 165 : t.playerlight.flashlightcontrol_f = lua_tonumber(L, 1); break;
		case 166 : t.player[t.plrid].state.moving = lua_tonumber(L, 1); break;
		case 167 : t.tplayerterrainheight_f = lua_tonumber(L, 1); break;
		case 168 : t.tjetpackverticalmove_f = lua_tonumber(L, 1); break;
		case 169 : t.terrain.TerrainID = lua_tonumber(L, 1); break;
		case 170 : g.globals.enableplrspeedmods = lua_tonumber(L, 1); break;
		case 171 : g.globals.riftmode = lua_tonumber(L, 1); break;
		case 172 : t.tplayerx_f = lua_tonumber(L, 1); break;
		case 173 : t.tplayery_f = lua_tonumber(L, 1); break;
		case 174 : t.tplayerz_f = lua_tonumber(L, 1); break;
		case 175 : t.terrain.playerx_f = lua_tonumber(L, 1); break;
		case 176 : t.terrain.playery_f = lua_tonumber(L, 1); break;
		case 177 : t.terrain.playerz_f = lua_tonumber(L, 1); break;
		case 178 : t.terrain.playerax_f = lua_tonumber(L, 1); break;
		case 179 : t.terrain.playeray_f = lua_tonumber(L, 1); break;
		case 180 : t.terrain.playeraz_f = lua_tonumber(L, 1); break;
		case 181 : t.tadjustbasedonwobbley_f = lua_tonumber(L, 1); break;
		case 182 : t.tFinalCamX_f = lua_tonumber(L, 1); break;
		case 183 : t.tFinalCamY_f = lua_tonumber(L, 1); break;
		case 184 : t.tFinalCamZ_f = lua_tonumber(L, 1); break;
		case 185 : t.tshakex_f = lua_tonumber(L, 1); break;
		case 186 : t.tshakey_f = lua_tonumber(L, 1); break;
		case 187 : t.tshakez_f = lua_tonumber(L, 1); break;		
		case 188 : t.huddamage.immunity = lua_tonumber(L, 1); break;		
		case 189 : g.charanimindex = lua_tonumber(L, 1); break;	

		// 190-200 reserved for MOTION CONTROLLER actions
	
		case 201 : t.gun[gunId].settings.ismelee         = lua_tonumber( L, param ); break;
		case 202 : t.gun[gunId].settings.alternate       = lua_tonumber( L, param ); break;
		case 203 : t.gun[gunId].settings.modessharemags  = lua_tonumber( L, param ); break;
		case 204 : t.gun[gunId].settings.alternateisflak = lua_tonumber( L, param ); break;
		case 205 : t.gun[gunId].settings.alternateisray  = lua_tonumber( L, param ); break;
		case 301 : g.firemodes[gunId][fireMode].settings.reloadqty         = lua_tonumber( L, param ); break;
		case 302 : g.firemodes[gunId][fireMode].settings.isempty           = lua_tonumber( L, param ); break;
		case 303 : g.firemodes[gunId][fireMode].settings.jammed            = lua_tonumber( L, param ); break;
		case 304 : g.firemodes[gunId][fireMode].settings.jamchance         = lua_tonumber( L, param ); break;
		case 305 : g.firemodes[gunId][fireMode].settings.mintimer          = lua_tonumber( L, param ); break;
		case 306 : g.firemodes[gunId][fireMode].settings.addtimer          = lua_tonumber( L, param ); break;
		case 307 : g.firemodes[gunId][fireMode].settings.shotsfired        = lua_tonumber( L, param ); break;
		case 308 : g.firemodes[gunId][fireMode].settings.cooltimer         = lua_tonumber( L, param ); break;
		case 309 : g.firemodes[gunId][fireMode].settings.overheatafter     = lua_tonumber( L, param ); break;
		case 310 : g.firemodes[gunId][fireMode].settings.jamchancetime     = lua_tonumber( L, param ); break;
		case 311 : g.firemodes[gunId][fireMode].settings.cooldown          = lua_tonumber( L, param ); break;
		case 312 : g.firemodes[gunId][fireMode].settings.nosubmergedfire   = lua_tonumber( L, param ); break;
		case 313 : g.firemodes[gunId][fireMode].settings.simplezoom        = lua_tonumber( L, param ); break;
		case 314 : g.firemodes[gunId][fireMode].settings.forcezoomout      = lua_tonumber( L, param ); break;
		case 315 : g.firemodes[gunId][fireMode].settings.zoommode          = lua_tonumber( L, param ); break;
		case 316 : g.firemodes[gunId][fireMode].settings.simplezoomanim    = lua_tonumber( L, param ); break;
		case 317 : g.firemodes[gunId][fireMode].settings.poolindex         = lua_tonumber( L, param ); break;
		case 318 : g.firemodes[gunId][fireMode].settings.plrturnspeedmod   = lua_tonumber( L, param ); break;
		case 319 : g.firemodes[gunId][fireMode].settings.zoomturnspeed     = lua_tonumber( L, param ); break;
		case 320 : g.firemodes[gunId][fireMode].settings.plrjumpspeedmod   = lua_tonumber( L, param ); break;
		case 321 : g.firemodes[gunId][fireMode].settings.plremptyspeedmod  = lua_tonumber( L, param ); break;
		case 322 : g.firemodes[gunId][fireMode].settings.plrmovespeedmod   = lua_tonumber( L, param ); break;
		case 323 : g.firemodes[gunId][fireMode].settings.zoomwalkspeed     = lua_tonumber( L, param ); break;
		case 324 : g.firemodes[gunId][fireMode].settings.plrreloadspeedmod = lua_tonumber( L, param ); break;
		case 325 : g.firemodes[gunId][fireMode].settings.hasempty          = lua_tonumber( L, param ); break;
		case 326 : g.firemodes[gunId][fireMode].action.block.s             = lua_tonumber( L, param ); break;

		case 501 : t.gunsound[t.gunid][lua_tonumber(L, 1)].soundid1 = lua_tonumber(L, 2); break;
		case 502 : t.gunsound[t.gunid][lua_tonumber(L, 1)].altsoundid = lua_tonumber(L, 2); break;
		case 503 : break;
		case 504 : break;

		case 700 :  iSrc = lua_tonumber(L, 1);
					iDest = lua_tonumber(L, 2);
					if ( iDest == 0 ) 
						t.charanimstate = t.charanimstates[lua_tonumber(L, 1)]; 
					else
						if ( iSrc == 0 ) 
							t.charanimstates[iDest] = t.charanimstate;
						else
							t.charanimstates[iDest] = t.charanimstates[iSrc]; 
					break;
		case 701 :	iDest = lua_tonumber(L, 1);
					if ( iDest == 0 ) 
						t.charanimstate.playcsi = lua_tonumber(L, 2); 
					else
						t.charanimstates[iDest].playcsi = lua_tonumber(L, 2); 
					break;
		case 702 :	iDest = lua_tonumber(L, 1);
					if ( iDest == 0 ) 
						t.charanimstate.originale = lua_tonumber(L, 2); 
					else
						t.charanimstates[iDest].originale = lua_tonumber(L, 2); 
					break;
		case 703 :	iDest = lua_tonumber(L, 1);
					if ( iDest == 0 ) 
						t.charanimstate.obj = lua_tonumber(L, 2); 
					else
						t.charanimstates[iDest].obj = lua_tonumber(L, 2); 
					break;
		case 704 :	iDest = lua_tonumber(L, 1);
					if ( iDest == 0 ) 
						t.charanimstate.animationspeed_f = lua_tonumber(L, 2); 
					else
						t.charanimstates[iDest].animationspeed_f = lua_tonumber(L, 2); 
					break;
		case 705 :	iDest = lua_tonumber(L, 1);
					if ( iDest == 0 ) 
						t.charanimstate.e = lua_tonumber(L, 2); 
					else
						t.charanimstates[iDest].e = lua_tonumber(L, 2); 
					break;

		case 801 : t.charanimcontrols[lua_tonumber(L, 1)].leaping = lua_tonumber(L, 2); 
			break;
		case 802 : t.charanimcontrols[lua_tonumber(L, 1)].moving = lua_tonumber(L, 2); 
			break;

		case 1001 : break;
		case 1002 : break;
	}
	return 0;
}
int GetGamePlayerControlData ( lua_State *L, int iDataMode )
{
	lua = L;
	int iSrc = 0;
	int n = lua_gettop(L);
	if ( iDataMode >= 500 )
	{
		if ( iDataMode >= 1001 )
			if ( n < 2 ) return 0;
		else
			if ( n < 1 ) return 0;
	}
	int gunId = t.gunid;
	int fireMode = t.tfiremode;

	if ( n > 0 && iDataMode > 200 && iDataMode < 500 )
	{
		gunId = lua_tonumber(L, 1);
		if (n > 1)
		{
			fireMode = lua_tonumber(L, 2);
		}
		else
		{
			fireMode = 0;
		}
	}

	switch ( iDataMode )
	{
		case 1 : lua_pushnumber ( L, t.playercontrol.jetpackmode ); break;
		case 2 : lua_pushnumber ( L, t.playercontrol.jetpackfuel_f ); break;
		case 3 : lua_pushnumber ( L, t.playercontrol.jetpackhidden ); break;
		case 4 : lua_pushnumber ( L, t.playercontrol.jetpackcollected ); break;
		case 5 : lua_pushnumber ( L, t.playercontrol.soundstartindex ); break;
		case 6 : lua_pushnumber ( L, t.playercontrol.jetpackparticleemitterindex ); break;
		case 7 : lua_pushnumber ( L, t.playercontrol.jetpackthrust_f ); break;
		case 8 : lua_pushnumber ( L, t.playercontrol.startstrength ); break;
		case 9 : lua_pushnumber ( L, t.playercontrol.isrunning ); break;
		case 10 : break;
		case 11 : lua_pushnumber ( L, t.playercontrol.cx_f ); break;
		case 12 : lua_pushnumber ( L, t.playercontrol.cy_f ); break;
		case 13 : lua_pushnumber ( L, t.playercontrol.cz_f ); break;
		case 14 : lua_pushnumber ( L, t.playercontrol.basespeed_f ); break;
		case 15 : lua_pushnumber ( L, t.playercontrol.canrun ); break;
		case 16 : lua_pushnumber ( L, t.playercontrol.maxspeed_f ); break;
		case 17 : lua_pushnumber ( L, t.playercontrol.topspeed_f ); break;
		case 18 : lua_pushnumber ( L, t.playercontrol.movement ); break;
		case 19 : lua_pushnumber ( L, t.playercontrol.movey_f ); break;
		case 20 : lua_pushnumber ( L, t.playercontrol.lastmovement ); break;
		case 21 : lua_pushnumber ( L, t.playercontrol.footfallcount ); break;
		case 22 : break;
		case 23 : lua_pushnumber ( L, t.playercontrol.gravityactive ); break;
		case 24 : lua_pushnumber ( L, t.playercontrol.plrhitfloormaterial ); break;
		case 25 : lua_pushnumber ( L, t.playercontrol.underwater ); break;
		case 26 : lua_pushnumber ( L, t.playercontrol.jumpmode ); break;
		case 27 : lua_pushnumber ( L, t.playercontrol.jumpmodecanaffectvelocitycountdown_f ); break;
		case 28 : lua_pushnumber ( L, t.playercontrol.speed_f ); break;
		case 29 : lua_pushnumber ( L, t.playercontrol.accel_f ); break;
		case 30 : lua_pushnumber ( L, t.playercontrol.speedratio_f ); break;
		case 31 : lua_pushnumber ( L, t.playercontrol.wobble_f ); break;
		case 32 :
		{
#ifdef FASTBULLETPHYSICS
			lua_pushnumber(L, t.playercontrol.wobblespeed_f*0.5);
#else
			extern bool bPhysicsRunningAt120FPS;
			if (!bPhysicsRunningAt120FPS)
				lua_pushnumber(L, t.playercontrol.wobblespeed_f*0.5);
			else
				lua_pushnumber(L, t.playercontrol.wobblespeed_f);
#endif
			break;
		}
		case 33 : lua_pushnumber ( L, t.playercontrol.wobbleheight_f ); break;
		case 34 : lua_pushnumber ( L, t.playercontrol.jumpmax_f ); break;
		case 35 : lua_pushnumber ( L, t.playercontrol.pushangle_f ); break;
		case 36 : lua_pushnumber ( L, t.playercontrol.pushforce_f ); break;
		case 37 : lua_pushnumber ( L, t.playercontrol.footfallpace_f ); break;
		case 38 : lua_pushnumber ( L, t.playercontrol.lockatheight ); break;
		case 39 : lua_pushnumber ( L, t.playercontrol.controlheight ); break;
		case 40 : lua_pushnumber ( L, t.playercontrol.controlheightcooldown ); break;
		case 41 : lua_pushnumber ( L, t.playercontrol.storemovey ); break;
		case 42 : break;
		case 43 : lua_pushnumber ( L, t.playercontrol.hurtfall ); break;
		case 44 : lua_pushnumber ( L, t.playercontrol.leanoverangle_f ); break;
		case 45 : lua_pushnumber ( L, t.playercontrol.leanover_f ); break;
		case 46 : lua_pushnumber ( L, t.playercontrol.camerashake_f ); break;
		case 47 : lua_pushnumber ( L, t.playercontrol.finalcameraanglex_f ); break;
		case 48 : lua_pushnumber ( L, t.playercontrol.finalcameraangley_f ); break;
		case 49 : lua_pushnumber ( L, t.playercontrol.finalcameraanglez_f ); break;
		case 50 : lua_pushnumber ( L, t.playercontrol.camrightmousemode ); break;
		case 51 : lua_pushnumber ( L, t.playercontrol.camcollisionsmooth ); break;
		case 52 : lua_pushnumber ( L, t.playercontrol.camcurrentdistance ); break;
		case 53 : lua_pushnumber ( L, t.playercontrol.camdofullraycheck ); break;
		case 54 : lua_pushnumber ( L, t.playercontrol.lastgoodcx_f ); break;
		case 55 : lua_pushnumber ( L, t.playercontrol.lastgoodcy_f ); break;
		case 56 : lua_pushnumber ( L, t.playercontrol.lastgoodcz_f ); break;
		case 57 : break;
		case 58 : lua_pushnumber ( L, t.playercontrol.flinchx_f ); break;
		case 59 : lua_pushnumber ( L, t.playercontrol.flinchy_f ); break;
		case 60 : lua_pushnumber ( L, t.playercontrol.flinchz_f ); break;
		case 61 : lua_pushnumber ( L, t.playercontrol.flinchcurrentx_f ); break;
		case 62 : lua_pushnumber ( L, t.playercontrol.flinchcurrenty_f ); break;
		case 63 : lua_pushnumber ( L, t.playercontrol.flinchcurrentz_f ); break;
		case 64 : lua_pushnumber ( L, t.playercontrol.footfalltype ); break;
		case 65 : lua_pushnumber ( L, t.playercontrol.ripplecount_f ); break;
		case 66 : lua_pushnumber ( L, t.playercontrol.lastfootfallsound ); break;
		case 67 : lua_pushnumber ( L, t.playercontrol.inwaterstate ); break;
		case 68 : lua_pushnumber ( L, t.playercontrol.drowntimestamp ); break;
		case 69 : lua_pushnumber ( L, t.playercontrol.deadtime ); break;
		case 70 : lua_pushnumber ( L, t.playercontrol.swimtimestamp ); break;
		case 71 : lua_pushnumber ( L, t.playercontrol.redDeathFog_f ); break;
		case 72 : lua_pushnumber ( L, t.playercontrol.heartbeatTimeStamp ); break;
		case 81 : lua_pushnumber ( L, t.playercontrol.thirdperson.enabled ); break;
		case 82 : lua_pushnumber ( L, t.playercontrol.thirdperson.characterindex ); break;
		case 83 : lua_pushnumber ( L, t.playercontrol.thirdperson.camerafollow ); break;
		case 84 : lua_pushnumber ( L, t.playercontrol.thirdperson.camerafocus ); break;
		case 85 : lua_pushnumber ( L, t.playercontrol.thirdperson.charactere ); break;
		case 86 : break;
		case 87 : lua_pushnumber ( L, t.playercontrol.thirdperson.shotfired ); break;
		case 88 : lua_pushnumber ( L, t.playercontrol.thirdperson.cameradistance ); break;
		case 89 : lua_pushnumber ( L, t.playercontrol.thirdperson.cameraspeed ); break;
		case 90 : lua_pushnumber ( L, t.playercontrol.thirdperson.cameralocked ); break;
		case 91 : lua_pushnumber ( L, t.playercontrol.thirdperson.cameraheight ); break;
		case 92 : lua_pushnumber ( L, t.playercontrol.thirdperson.camerashoulder ); break;

		case 99 : lua_pushnumber ( L, g.gxboxcontrollertype ); break;		
		case 101 : lua_pushnumber ( L, t.gunmode ); break;
		case 102 : lua_pushnumber ( L, t.player[t.plrid].state.firingmode ); break;
		case 103 : lua_pushnumber ( L, g.weaponammoindex ); break;
		case 104 : lua_pushnumber ( L, g.ammooffset ); break;
		case 105 : lua_pushnumber ( L, g.ggunmeleekey ); break;
		case 106 : lua_pushnumber ( L, t.player[t.plrid].state.blockingaction ); break;
		case 107 : lua_pushnumber ( L, t.gunshootnoammo ); break;		
		case 108 : lua_pushnumber ( L, g.playerunderwater ); break;		
		case 109 : lua_pushnumber ( L, g.gdisablerightmousehold ); break;		
		case 110 : lua_pushnumber ( L, g.gxbox ); break;		
		case 111 : lua_pushnumber ( L, JoystickX() ); break;
		case 112 : lua_pushnumber ( L, JoystickY() ); break;
		case 113 : lua_pushnumber ( L, JoystickZ() ); break;
		case 114 : lua_pushnumber ( L, t.gunzoommode ); break;		
		case 115 : lua_pushnumber ( L, t.gunzoommag_f ); break;		
		case 116 : lua_pushnumber ( L, t.gunreloadnoammo ); break;	
		case 117 : lua_pushnumber ( L, g.plrreloading ); break;	
		case 118 : lua_pushnumber ( L, g.ggunaltswapkey1 ); break;	
		case 119 : lua_pushnumber ( L, g.ggunaltswapkey2 ); break;	
		case 120 : lua_pushnumber ( L, t.weaponkeyselection ); break;	
		case 121 : lua_pushnumber ( L, t.weaponindex ); break;	
		case 122 : lua_pushnumber ( L, t.player[t.plrid].command.newweapon ); break;	
		case 123 : lua_pushnumber ( L, t.gunid ); break;	
		case 124 : lua_pushnumber ( L, t.gunselectionafterhide ); break;	
		case 125 : lua_pushnumber ( L, t.gunburst ); break;	
		case 126 : break;
		case 127 : lua_pushnumber ( L, JoystickTwistX() ); break;
		case 128 : lua_pushnumber ( L, JoystickTwistY() ); break;
		case 129 : lua_pushnumber ( L, JoystickTwistZ() ); break;
		case 130 : lua_pushnumber ( L, t.plrzoominchange ); break;	
		case 131 : lua_pushnumber ( L, t.plrzoomin_f ); break;	
		case 132 : lua_pushnumber ( L, g.luaactivatemouse ); break;	
		case 133 : lua_pushnumber ( L, g.realfov_f ); break;	
		case 134 : lua_pushnumber ( L, g.gdisablepeeking ); break;	
		case 135 : lua_pushnumber ( L, t.plrhasfocus ); break;	
		case 136 : lua_pushnumber ( L, t.game.runasmultiplayer ); break;	
		case 137 : lua_pushnumber ( L, g.mp.respawnLeft ); break;	
		case 138 : lua_pushnumber ( L, g.tabmode ); break;	
		case 139 : lua_pushnumber ( L, g.lowfpswarning ); break;	
		case 140 : lua_pushnumber ( L, t.visuals.CameraFOV_f ); break;	
		case 141 : lua_pushnumber ( L, t.visuals.CameraFOVZoomed_f ); break;	
		case 142 : lua_pushnumber ( L, g.gminvert ); break;	
		case 143 : lua_pushnumber ( L, t.plrkeySLOWMOTION ); break;	
		case 144 : lua_pushnumber ( L, g.globals.smoothcamerakeys ); break;	
		case 145 : lua_pushnumber ( L, t.cammousemovex_f ); break;	
		case 146 : lua_pushnumber ( L, t.cammousemovey_f ); break;	
		case 147 : lua_pushnumber ( L, g.gunRecoilX_f ); break;	
		case 148 : lua_pushnumber ( L, g.gunRecoilY_f ); break;	
		case 149 : lua_pushnumber ( L, g.gunRecoilAngleX_f ); break;	
		case 150 : lua_pushnumber ( L, g.gunRecoilAngleY_f ); break;	
		case 151 : lua_pushnumber ( L, t.gunRecoilCorrectY_f ); break;	
		case 152 : lua_pushnumber ( L, g.gunRecoilCorrectX_f ); break;	
		case 153 : lua_pushnumber ( L, g.gunRecoilCorrectAngleY_f ); break;	
		case 154 : lua_pushnumber ( L, t.gunRecoilCorrectAngleX_f ); break;	
		case 155 : lua_pushnumber ( L, t.camangx_f ); break;	
		case 156 : lua_pushnumber ( L, t.camangy_f ); break;	
		case 157 : lua_pushnumber ( L, t.aisystem.playerducking ); break;	
		case 158 : lua_pushnumber ( L, t.conkit.editmodeactive ); break;	
		case 159 : lua_pushnumber ( L, t.plrkeySHIFT ); break;	
		case 160 : lua_pushnumber ( L, t.plrkeySHIFT2 ); break;	
		case 161 : lua_pushnumber ( L, t.inputsys.keycontrol ); break;	
		case 162 : lua_pushnumber ( L, t.hardwareinfoglobals.nowater ); break;	
		case 163 : lua_pushnumber ( L, t.terrain.waterliney_f ); break;	
		case 164 : lua_pushnumber ( L, g.flashLightKeyEnabled ); break;	
		case 165 : lua_pushnumber ( L, t.playerlight.flashlightcontrol_f ); break;	
		case 166 : lua_pushnumber ( L, t.player[t.plrid].state.moving ); break;	
		case 167 : lua_pushnumber ( L, t.tplayerterrainheight_f ); break;	
		case 168 : lua_pushnumber ( L, t.tjetpackverticalmove_f ); break;	
		case 169 : lua_pushnumber ( L, t.terrain.TerrainID ); break;	
		case 170 : lua_pushnumber ( L, g.globals.enableplrspeedmods ); break;	
		case 171 : lua_pushnumber ( L, g.globals.riftmode ); break;	
		case 172 : lua_pushnumber ( L, t.tplayerx_f ); break;	
		case 173 : lua_pushnumber ( L, t.tplayery_f ); break;	
		case 174 : lua_pushnumber ( L, t.tplayerz_f ); break;	
		case 175 : lua_pushnumber ( L, t.terrain.playerx_f ); break;	
		case 176 : lua_pushnumber ( L, t.terrain.playery_f ); break;	
		case 177 : lua_pushnumber ( L, t.terrain.playerz_f ); break;	
		case 178 : lua_pushnumber ( L, t.terrain.playerax_f ); break;	
		case 179 : lua_pushnumber ( L, t.terrain.playeray_f ); break;	
		case 180 : lua_pushnumber ( L, t.terrain.playeraz_f ); break;	
		case 181 : lua_pushnumber ( L, t.tadjustbasedonwobbley_f ); break;	
		case 182 : lua_pushnumber ( L, t.tFinalCamX_f ); break;	
		case 183 : lua_pushnumber ( L, t.tFinalCamY_f ); break;	
		case 184 : lua_pushnumber ( L, t.tFinalCamZ_f ); break;	
		case 185 : lua_pushnumber ( L, t.tshakex_f ); break;	
		case 186 : lua_pushnumber ( L, t.tshakey_f); break;	
		case 187 : lua_pushnumber ( L, t.tshakez_f ); break;	
		case 188 : lua_pushnumber ( L, t.huddamage.immunity ); break;	
		case 189 : lua_pushnumber ( L, g.charanimindex ); break;				

		#ifdef VRTECH
		case 190 : if ( GGVR_IsHmdPresent() > 0 ) { lua_pushnumber ( L, 1 ); } else { lua_pushnumber ( L, 0 ); } break;
		case 191 : lua_pushnumber ( L, GGVR_IsHmdPresent() ); break;				
		case 192 : lua_pushnumber ( L, GGVR_BestController_JoyX() ); break;
		case 193 : lua_pushnumber ( L, GGVR_BestController_JoyY() ); break;
		case 194 : lua_pushnumber ( L, GGVR_RightController_Trigger() ); break;
		case 195 : lua_pushnumber ( L, GGVR_RightController_Grip() ); break;
		case 196 : lua_pushnumber ( L, GGVR_RightController_JoyX() ); break;
		case 197 : lua_pushnumber ( L, GGVR_RightController_JoyX() ); break;
		case 251 : lua_pushnumber ( L, GGVR_GetBestHandX() ); break;
		case 252 : lua_pushnumber ( L, GGVR_GetBestHandY() ); break;
		case 253 : lua_pushnumber ( L, GGVR_GetBestHandZ() ); break;
		case 254 : lua_pushnumber ( L, GGVR_GetBestHandAngleX() ); break;
		case 255 : lua_pushnumber ( L, GGVR_GetBestHandAngleY() ); break;
		case 256 : lua_pushnumber ( L, GGVR_GetBestHandAngleZ() ); break;
		case 257: lua_pushnumber ( L, GGVR_GetLaserGuidedEntityObj(g.entityviewstartobj,g.entityviewendobj) ); break;
		#else
		case 190 : 
		case 191 : 
		case 192 : 
		case 193 : 
		case 194 : 
		case 195 : 
		case 196 : 
		case 197 : 
		case 198 : 
		case 199 : 
		case 200 : 
		case 251 : 
		case 252 : 
		case 253 : 
		case 254 : 
		case 255 : 
		case 256 : 
		case 257 : 
			lua_pushnumber ( L, 0 ); 
			break;
		#endif

		case 201 : lua_pushnumber ( L, t.gun[gunId].settings.ismelee         ); break;
		case 202 : lua_pushnumber ( L, t.gun[gunId].settings.alternate       ); break;
		case 203 : lua_pushnumber ( L, t.gun[gunId].settings.modessharemags  ); break;
		case 204 : lua_pushnumber ( L, t.gun[gunId].settings.alternateisflak ); break;
		case 205 : lua_pushnumber ( L, t.gun[gunId].settings.alternateisray  ); break;
		
		// 251-260 used above

		case 301 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.reloadqty         ); break;
		case 302 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.isempty           ); break;
		case 303 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.jammed            ); break;
		case 304 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.jamchance         ); break;
		case 305 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.mintimer          ); break;
		case 306 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.addtimer          ); break;
		case 307 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.shotsfired        ); break;
		case 308 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.cooltimer         ); break;
		case 309 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.overheatafter     ); break;
		case 310 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.jamchancetime     ); break;
		case 311 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.cooldown          ); break;
		case 312 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.nosubmergedfire   ); break;
		case 313 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.simplezoom        ); break;
		case 314 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.forcezoomout      ); break;
		case 315 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.zoommode          ); break;
		case 316 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.simplezoomanim    ); break;
		case 317 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.poolindex         ); break;
		case 318 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.plrturnspeedmod   ); break;
		case 319 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.zoomturnspeed     ); break;
		case 320 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.plrjumpspeedmod   ); break;
		case 321 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.plremptyspeedmod  ); break;
		case 322 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.plrmovespeedmod   ); break;
		case 323 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.zoomwalkspeed     ); break;
		case 324 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.plrreloadspeedmod ); break;
		case 325 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].settings.hasempty          ); break;
		case 326 : lua_pushnumber ( L, g.firemodes[gunId][fireMode].action.block.s             ); break;

		case 501 : lua_pushnumber ( L, t.gunsound[t.gunid][lua_tonumber(L, 1)].soundid1 ); break;
		case 502 : lua_pushnumber ( L, t.gunsound[t.gunid][lua_tonumber(L, 1)].altsoundid ); break;		
		case 503 : lua_pushnumber ( L, JoystickHatAngle(lua_tonumber(L, 1)) ); break;
		case 504 : lua_pushnumber ( L, JoystickFireXL(lua_tonumber(L, 1)) ); break;

		case 701 :	iSrc = lua_tonumber(L, 1);
					if ( iSrc == 0 )
						lua_pushnumber ( L, t.charanimstate.playcsi ); 
					else
						lua_pushnumber ( L, t.charanimstates[iSrc].playcsi ); 
					break;
		case 702 :	iSrc = lua_tonumber(L, 1);
					if ( iSrc == 0 )
						lua_pushnumber ( L, t.charanimstate.originale ); 
					else
						lua_pushnumber ( L, t.charanimstates[iSrc].originale ); 
					break;
		case 703 :	iSrc = lua_tonumber(L, 1);
					if ( iSrc == 0 )
						lua_pushnumber ( L, t.charanimstate.obj ); 
					else
						lua_pushnumber ( L, t.charanimstates[iSrc].obj ); 
					break;
		case 704 :	iSrc = lua_tonumber(L, 1);
					if ( iSrc == 0 )
						lua_pushnumber ( L, t.charanimstate.animationspeed_f ); 
					else
						lua_pushnumber ( L, t.charanimstates[iSrc].animationspeed_f ); 
					break;
		case 705 :	iSrc = lua_tonumber(L, 1);
					if ( iSrc == 0 )
						lua_pushnumber ( L, t.charanimstate.e ); 
					else
						lua_pushnumber ( L, t.charanimstates[iSrc].e ); 
					break;

		case 741 : lua_pushnumber ( L, t.csi_stoodvault[lua_tonumber(L, 1)] ); break;
		case 751 : lua_pushnumber ( L, t.charseq[lua_tonumber(L, 1)].trigger ); break;
		case 761 : lua_pushnumber ( L, t.entityelement[lua_tonumber(L, 1)].bankindex ); break;
		case 762 : lua_pushnumber ( L, t.entityelement[lua_tonumber(L, 1)].obj ); break;
		case 763 : lua_pushnumber ( L, t.entityelement[lua_tonumber(L, 1)].ragdollified ); break;
		case 764 : lua_pushnumber ( L, t.entityelement[lua_tonumber(L, 1)].speedmodulator_f ); break;
		case 801 : lua_pushnumber ( L, t.charanimcontrols[lua_tonumber(L, 1)].leaping ); break;
		case 802 : lua_pushnumber ( L, t.charanimcontrols[lua_tonumber(L, 1)].moving ); break;
		case 851 : lua_pushnumber ( L, t.entityprofile[lua_tonumber(L, 1)].fJumpModifier ); break;
		case 852 : lua_pushnumber ( L, t.entityprofile[lua_tonumber(L, 1)].startofaianim ); break;			
		case 853 : lua_pushnumber ( L, t.entityprofile[lua_tonumber(L, 1)].jumphold ); break;			
		case 854 : lua_pushnumber ( L, t.entityprofile[lua_tonumber(L, 1)].jumpresume ); break;			

		case 1001 : lua_pushnumber ( L, t.entityanim[lua_tonumber(L, 1)][lua_tonumber(L, 2)].start ); break;
		case 1002 : lua_pushnumber ( L, t.entityanim[lua_tonumber(L, 1)][lua_tonumber(L, 2)].finish ); break;
	}
	return 1;
}
int GetGamePlayerControlJetpackMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 1 ); }
int GetGamePlayerControlJetpackFuel ( lua_State *L ) { return GetGamePlayerControlData ( L, 2 ); }
int GetGamePlayerControlJetpackHidden ( lua_State *L ) { return GetGamePlayerControlData ( L, 3 ); }
int GetGamePlayerControlJetpackCollected ( lua_State *L ) { return GetGamePlayerControlData ( L, 4 ); }
int GetGamePlayerControlSoundStartIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 5 ); }
int GetGamePlayerControlJetpackParticleEmitterIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 6 ); }
int GetGamePlayerControlJetpackThrust ( lua_State *L ) { return GetGamePlayerControlData ( L, 7 ); }
int GetGamePlayerControlStartStrength ( lua_State *L ) { return GetGamePlayerControlData ( L, 8 ); }
int GetGamePlayerControlIsRunning ( lua_State *L ) { return GetGamePlayerControlData ( L, 9 ); }
int GetGamePlayerControlX6 ( lua_State *L ) { return GetGamePlayerControlData ( L, 10 ); }
int GetGamePlayerControlCx ( lua_State *L ) { return GetGamePlayerControlData ( L, 11 ); }
int GetGamePlayerControlCy ( lua_State *L ) { return GetGamePlayerControlData ( L, 12 ); }
int GetGamePlayerControlCz ( lua_State *L ) { return GetGamePlayerControlData ( L, 13 ); }
int GetGamePlayerControlBasespeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 14 ); }
int GetGamePlayerControlCanRun ( lua_State *L ) { return GetGamePlayerControlData ( L, 15 ); }
int GetGamePlayerControlMaxspeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 16 ); }
int GetGamePlayerControlTopspeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 17 ); }
int GetGamePlayerControlMovement ( lua_State *L ) { return GetGamePlayerControlData ( L, 18 ); }
int GetGamePlayerControlMovey ( lua_State *L ) { return GetGamePlayerControlData ( L, 19 ); }
int GetGamePlayerControlLastMovement ( lua_State *L ) { return GetGamePlayerControlData ( L, 20 ); }
int GetGamePlayerControlFootfallCount ( lua_State *L ) { return GetGamePlayerControlData ( L, 21 ); }
int GetGamePlayerControlX1 ( lua_State *L ) { return GetGamePlayerControlData ( L, 22 ); }
int GetGamePlayerControlGravityActive ( lua_State *L ) { return GetGamePlayerControlData ( L, 23 ); }
int GetGamePlayerControlPlrHitFloorMaterial ( lua_State *L ) { return GetGamePlayerControlData ( L, 24 ); }
int GetGamePlayerControlUnderwater ( lua_State *L ) { return GetGamePlayerControlData ( L, 25 ); }
int GetGamePlayerControlJumpMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 26 ); }
int GetGamePlayerControlJumpModeCanAffectVelocityCountdown ( lua_State *L ) { return GetGamePlayerControlData ( L, 27 ); }
int GetGamePlayerControlSpeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 28 ); }
int GetGamePlayerControlAccel ( lua_State *L ) { return GetGamePlayerControlData ( L, 29 ); }
int GetGamePlayerControlSpeedRatio ( lua_State *L ) { return GetGamePlayerControlData ( L, 30 ); }
int GetGamePlayerControlWobble ( lua_State *L ) { return GetGamePlayerControlData ( L, 31 ); }
int GetGamePlayerControlWobbleSpeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 32 ); }
int GetGamePlayerControlWobbleHeight ( lua_State *L ) { return GetGamePlayerControlData ( L, 33 ); }
int GetGamePlayerControlJumpmax ( lua_State *L ) { return GetGamePlayerControlData ( L, 34 ); }
int GetGamePlayerControlPushangle ( lua_State *L ) { return GetGamePlayerControlData ( L, 35 ); }
int GetGamePlayerControlPushforce ( lua_State *L ) { return GetGamePlayerControlData ( L, 36 ); }
int GetGamePlayerControlFootfallPace ( lua_State *L ) { return GetGamePlayerControlData ( L, 37 ); }
int GetGamePlayerControlLockAtHeight ( lua_State *L ) { return GetGamePlayerControlData ( L, 38 ); }
int GetGamePlayerControlControlHeight ( lua_State *L ) { return GetGamePlayerControlData ( L, 39 ); }
int GetGamePlayerControlControlHeightCooldown ( lua_State *L ) { return GetGamePlayerControlData ( L, 40 ); }
int GetGamePlayerControlStoreMovey ( lua_State *L ) { return GetGamePlayerControlData ( L, 41 ); }
int GetGamePlayerControlX3 ( lua_State *L ) { return GetGamePlayerControlData ( L, 42 ); }
int GetGamePlayerControlHurtFall ( lua_State *L ) { return GetGamePlayerControlData ( L, 43 ); }
int GetGamePlayerControlLeanoverAngle ( lua_State *L ) { return GetGamePlayerControlData ( L, 44 ); }
int GetGamePlayerControlLeanover ( lua_State *L ) { return GetGamePlayerControlData ( L, 45 ); }
int GetGamePlayerControlCameraShake ( lua_State *L ) { return GetGamePlayerControlData ( L, 46 ); }
int GetGamePlayerControlFinalCameraAnglex ( lua_State *L ) { return GetGamePlayerControlData ( L, 47 ); }
int GetGamePlayerControlFinalCameraAngley ( lua_State *L ) { return GetGamePlayerControlData ( L, 48 ); }
int GetGamePlayerControlFinalCameraAnglez ( lua_State *L ) { return GetGamePlayerControlData ( L, 49 ); }
int GetGamePlayerControlCamRightMouseMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 50 ); }
int GetGamePlayerControlCamCollisionSmooth ( lua_State *L ) { return GetGamePlayerControlData ( L, 51 ); }
int GetGamePlayerControlCamCurrentDistance ( lua_State *L ) { return GetGamePlayerControlData ( L, 52 ); }
int GetGamePlayerControlCamDoFullRayCheck ( lua_State *L ) { return GetGamePlayerControlData ( L, 53 ); }
int GetGamePlayerControlLastGoodcx ( lua_State *L ) { return GetGamePlayerControlData ( L, 54 ); }
int GetGamePlayerControlLastGoodcy ( lua_State *L ) { return GetGamePlayerControlData ( L, 55 ); }
int GetGamePlayerControlLastGoodcz ( lua_State *L ) { return GetGamePlayerControlData ( L, 56 ); }
int GetGamePlayerControlX4 ( lua_State *L ) { return GetGamePlayerControlData ( L, 57 ); }
int GetGamePlayerControlFlinchx ( lua_State *L ) { return GetGamePlayerControlData ( L, 58 ); }
int GetGamePlayerControlFlinchy ( lua_State *L ) { return GetGamePlayerControlData ( L, 59 ); }
int GetGamePlayerControlFlinchz ( lua_State *L ) { return GetGamePlayerControlData ( L, 60 ); }
int GetGamePlayerControlFlinchCurrentx ( lua_State *L ) { return GetGamePlayerControlData ( L, 61 ); }
int GetGamePlayerControlFlinchCurrenty ( lua_State *L ) { return GetGamePlayerControlData ( L, 62 ); }
int GetGamePlayerControlFlinchCurrentz ( lua_State *L ) { return GetGamePlayerControlData ( L, 63 ); }
int GetGamePlayerControlFootfallType ( lua_State *L ) { return GetGamePlayerControlData ( L, 64 ); }
int GetGamePlayerControlRippleCount ( lua_State *L ) { return GetGamePlayerControlData ( L, 65 ); }
int GetGamePlayerControlLastFootfallSound ( lua_State *L ) { return GetGamePlayerControlData ( L, 66 ); }
int GetGamePlayerControlInWaterState ( lua_State *L ) { return GetGamePlayerControlData ( L, 67 ); }
int GetGamePlayerControlDrownTimestamp ( lua_State *L ) { return GetGamePlayerControlData ( L, 68 ); }
int GetGamePlayerControlDeadTime ( lua_State *L ) { return GetGamePlayerControlData ( L, 69 ); }
int GetGamePlayerControlSwimTimestamp ( lua_State *L ) { return GetGamePlayerControlData ( L, 70 ); }
int GetGamePlayerControlRedDeathFog ( lua_State *L ) { return GetGamePlayerControlData ( L, 71 ); }
int GetGamePlayerControlHeartbeatTimeStamp ( lua_State *L ) { return GetGamePlayerControlData ( L, 72 ); }
int GetGamePlayerControlThirdpersonEnabled ( lua_State *L ) { return GetGamePlayerControlData ( L, 81 ); }
int GetGamePlayerControlThirdpersonCharacterIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 82 ); }
int GetGamePlayerControlThirdpersonCameraFollow ( lua_State *L ) { return GetGamePlayerControlData ( L, 83 ); }
int GetGamePlayerControlThirdpersonCameraFocus ( lua_State *L ) { return GetGamePlayerControlData ( L, 84 ); }
int GetGamePlayerControlThirdpersonCharactere ( lua_State *L ) { return GetGamePlayerControlData ( L, 85 ); }
int GetGamePlayerControlX7 ( lua_State *L ) { return GetGamePlayerControlData ( L, 86 ); }
int GetGamePlayerControlThirdpersonShotFired ( lua_State *L ) { return GetGamePlayerControlData ( L, 87 ); }
int GetGamePlayerControlThirdpersonCameraDistance ( lua_State *L ) { return GetGamePlayerControlData ( L, 88 ); }
int GetGamePlayerControlThirdpersonCameraSpeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 89 ); }
int GetGamePlayerControlThirdpersonCameraLocked ( lua_State *L ) { return GetGamePlayerControlData ( L, 90 ); }
int GetGamePlayerControlThirdpersonCameraHeight ( lua_State *L ) { return GetGamePlayerControlData ( L, 91 ); }
int GetGamePlayerControlThirdpersonCameraShoulder ( lua_State *L ) { return GetGamePlayerControlData ( L, 92 ); }
int SetGamePlayerControlJetpackMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 1 ); }
int SetGamePlayerControlJetpackFuel ( lua_State *L ) { return SetGamePlayerControlData ( L, 2 ); }
int SetGamePlayerControlJetpackHidden ( lua_State *L ) { return SetGamePlayerControlData ( L, 3 ); }
int SetGamePlayerControlJetpackCollected ( lua_State *L ) { return SetGamePlayerControlData ( L, 4 ); }
int SetGamePlayerControlSoundStartIndex ( lua_State *L ) { return SetGamePlayerControlData ( L, 5 ); }
int SetGamePlayerControlJetpackParticleEmitterIndex ( lua_State *L ) { return SetGamePlayerControlData ( L, 6 ); }
int SetGamePlayerControlJetpackThrust ( lua_State *L ) { return SetGamePlayerControlData ( L, 7 ); }
int SetGamePlayerControlStartStrength ( lua_State *L ) { return SetGamePlayerControlData ( L, 8 ); }
int SetGamePlayerControlIsRunning ( lua_State *L ) { return SetGamePlayerControlData ( L, 9 ); }
int SetGamePlayerControlX6 ( lua_State *L ) { return SetGamePlayerControlData ( L, 10 ); }
int SetGamePlayerControlCx ( lua_State *L ) { return SetGamePlayerControlData ( L, 11 ); }
int SetGamePlayerControlCy ( lua_State *L ) { return SetGamePlayerControlData ( L, 12 ); }
int SetGamePlayerControlCz ( lua_State *L ) { return SetGamePlayerControlData ( L, 13 ); }
int SetGamePlayerControlBasespeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 14 ); }
int SetGamePlayerControlCanRun ( lua_State *L ) { return SetGamePlayerControlData ( L, 15 ); }
int SetGamePlayerControlMaxspeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 16 ); }
int SetGamePlayerControlTopspeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 17 ); }
int SetGamePlayerControlMovement ( lua_State *L ) { return SetGamePlayerControlData ( L, 18 ); }
int SetGamePlayerControlMovey ( lua_State *L ) { return SetGamePlayerControlData ( L, 19 ); }
int SetGamePlayerControlLastMovement ( lua_State *L ) { return SetGamePlayerControlData ( L, 20 ); }
int SetGamePlayerControlFootfallCount ( lua_State *L ) { return SetGamePlayerControlData ( L, 21 ); }
int SetGamePlayerControlX1 ( lua_State *L ) { return SetGamePlayerControlData ( L, 22 ); }
int SetGamePlayerControlGravityActive ( lua_State *L ) { return SetGamePlayerControlData ( L, 23 ); }
int SetGamePlayerControlPlrHitFloorMaterial ( lua_State *L ) { return SetGamePlayerControlData ( L, 24 ); }
int SetGamePlayerControlUnderwater ( lua_State *L ) { return SetGamePlayerControlData ( L, 25 ); }
int SetGamePlayerControlJumpMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 26 ); }
int SetGamePlayerControlJumpModeCanAffectVelocityCountdown ( lua_State *L ) { return SetGamePlayerControlData ( L, 27 ); }
int SetGamePlayerControlSpeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 28 ); }
int SetGamePlayerControlAccel ( lua_State *L ) { return SetGamePlayerControlData ( L, 29 ); }
int SetGamePlayerControlSpeedRatio ( lua_State *L ) { return SetGamePlayerControlData ( L, 30 ); }
int SetGamePlayerControlWobble ( lua_State *L ) { return SetGamePlayerControlData ( L, 31 ); }
int SetGamePlayerControlWobbleSpeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 32 ); }
int SetGamePlayerControlWobbleHeight ( lua_State *L ) { return SetGamePlayerControlData ( L, 33 ); }
int SetGamePlayerControlJumpmax ( lua_State *L ) { return SetGamePlayerControlData ( L, 34 ); }
int SetGamePlayerControlPushangle ( lua_State *L ) { return SetGamePlayerControlData ( L, 35 ); }
int SetGamePlayerControlPushforce ( lua_State *L ) { return SetGamePlayerControlData ( L, 36 ); }
int SetGamePlayerControlFootfallPace ( lua_State *L ) { return SetGamePlayerControlData ( L, 37 ); }
int SetGamePlayerControlLockAtHeight ( lua_State *L ) { return SetGamePlayerControlData ( L, 38 ); }
int SetGamePlayerControlControlHeight ( lua_State *L ) { return SetGamePlayerControlData ( L, 39 ); }
int SetGamePlayerControlControlHeightCooldown ( lua_State *L ) { return SetGamePlayerControlData ( L, 40 ); }
int SetGamePlayerControlStoreMovey ( lua_State *L ) { return SetGamePlayerControlData ( L, 41 ); }
int SetGamePlayerControlX3 ( lua_State *L ) { return SetGamePlayerControlData ( L, 42 ); }
int SetGamePlayerControlHurtFall ( lua_State *L ) { return SetGamePlayerControlData ( L, 43 ); }
int SetGamePlayerControlLeanoverAngle ( lua_State *L ) { return SetGamePlayerControlData ( L, 44 ); }
int SetGamePlayerControlLeanover ( lua_State *L ) { return SetGamePlayerControlData ( L, 45 ); }
int SetGamePlayerControlCameraShake ( lua_State *L ) { return SetGamePlayerControlData ( L, 46 ); }
int SetGamePlayerControlFinalCameraAnglex ( lua_State *L ) { return SetGamePlayerControlData ( L, 47 ); }
int SetGamePlayerControlFinalCameraAngley ( lua_State *L ) { return SetGamePlayerControlData ( L, 48 ); }
int SetGamePlayerControlFinalCameraAnglez ( lua_State *L ) { return SetGamePlayerControlData ( L, 49 ); }
int SetGamePlayerControlCamRightMouseMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 50 ); }
int SetGamePlayerControlCamCollisionSmooth ( lua_State *L ) { return SetGamePlayerControlData ( L, 51 ); }
int SetGamePlayerControlCamCurrentDistance ( lua_State *L ) { return SetGamePlayerControlData ( L, 52 ); }
int SetGamePlayerControlCamDoFullRayCheck ( lua_State *L ) { return SetGamePlayerControlData ( L, 53 ); }
int SetGamePlayerControlLastGoodcx ( lua_State *L ) { return SetGamePlayerControlData ( L, 54 ); }
int SetGamePlayerControlLastGoodcy ( lua_State *L ) { return SetGamePlayerControlData ( L, 55 ); }
int SetGamePlayerControlLastGoodcz ( lua_State *L ) { return SetGamePlayerControlData ( L, 56 ); }
int SetGamePlayerControlX4 ( lua_State *L ) { return SetGamePlayerControlData ( L, 57 ); }
int SetGamePlayerControlFlinchx ( lua_State *L ) { return SetGamePlayerControlData ( L, 58 ); }
int SetGamePlayerControlFlinchy ( lua_State *L ) { return SetGamePlayerControlData ( L, 59 ); }
int SetGamePlayerControlFlinchz ( lua_State *L ) { return SetGamePlayerControlData ( L, 60 ); }
int SetGamePlayerControlFlinchCurrentx ( lua_State *L ) { return SetGamePlayerControlData ( L, 61 ); }
int SetGamePlayerControlFlinchCurrenty ( lua_State *L ) { return SetGamePlayerControlData ( L, 62 ); }
int SetGamePlayerControlFlinchCurrentz ( lua_State *L ) { return SetGamePlayerControlData ( L, 63 ); }
int SetGamePlayerControlFootfallType ( lua_State *L ) { return SetGamePlayerControlData ( L, 64 ); }
int SetGamePlayerControlRippleCount ( lua_State *L ) { return SetGamePlayerControlData ( L, 65 ); }
int SetGamePlayerControlLastFootfallSound ( lua_State *L ) { return SetGamePlayerControlData ( L, 66 ); }
int SetGamePlayerControlInWaterState ( lua_State *L ) { return SetGamePlayerControlData ( L, 67 ); }
int SetGamePlayerControlDrownTimestamp ( lua_State *L ) { return SetGamePlayerControlData ( L, 68 ); }
int SetGamePlayerControlDeadTime ( lua_State *L ) { return SetGamePlayerControlData ( L, 69 ); }
int SetGamePlayerControlSwimTimestamp ( lua_State *L ) { return SetGamePlayerControlData ( L, 70 ); }
int SetGamePlayerControlRedDeathFog ( lua_State *L ) { return SetGamePlayerControlData ( L, 71 ); }
int SetGamePlayerControlHeartbeatTimeStamp ( lua_State *L ) { return SetGamePlayerControlData ( L, 72 ); }
int SetGamePlayerControlThirdpersonEnabled ( lua_State *L ) { return SetGamePlayerControlData ( L, 81 ); }
int SetGamePlayerControlThirdpersonCharacterIndex ( lua_State *L ) { return SetGamePlayerControlData ( L, 82 ); }
int SetGamePlayerControlThirdpersonCameraFollow ( lua_State *L ) { return SetGamePlayerControlData ( L, 83 ); }
int SetGamePlayerControlThirdpersonCameraFocus ( lua_State *L ) { return SetGamePlayerControlData ( L, 84 ); }
int SetGamePlayerControlThirdpersonCharactere ( lua_State *L ) { return SetGamePlayerControlData ( L, 85 ); }
int SetGamePlayerControlX7 ( lua_State *L ) { return SetGamePlayerControlData ( L, 86 ); }
int SetGamePlayerControlThirdpersonShotFired ( lua_State *L ) { return SetGamePlayerControlData ( L, 87 ); }
int SetGamePlayerControlThirdpersonCameraDistance ( lua_State *L ) { return SetGamePlayerControlData ( L, 88 ); }
int SetGamePlayerControlThirdpersonCameraSpeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 89 ); }
int SetGamePlayerControlThirdpersonCameraLocked ( lua_State *L ) { return SetGamePlayerControlData ( L, 90 ); }
int SetGamePlayerControlThirdpersonCameraHeight ( lua_State *L ) { return SetGamePlayerControlData ( L, 91 ); }
int SetGamePlayerControlThirdpersonCameraShoulder ( lua_State *L ) { return SetGamePlayerControlData ( L, 92 ); }

// 99 used
int SetGamePlayerStateGunMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 101 ); }
int GetGamePlayerStateGunMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 101 ); }
int SetGamePlayerStateFiringMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 102 ); }
int GetGamePlayerStateFiringMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 102 ); }
int GetGamePlayerStateWeaponAmmoIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 103 ); }
int GetGamePlayerStateAmmoOffset ( lua_State *L ) { return GetGamePlayerControlData ( L, 104 ); }
int GetGamePlayerStateGunMeleeKey ( lua_State *L ) { return GetGamePlayerControlData ( L, 105 ); }
int GetGamePlayerStateBlockingAction ( lua_State *L ) { return GetGamePlayerControlData ( L, 106 ); }
int SetGamePlayerStateGunShootNoAmmo ( lua_State *L ) { return SetGamePlayerControlData ( L, 107 ); }
int GetGamePlayerStateGunShootNoAmmo ( lua_State *L ) { return GetGamePlayerControlData ( L, 107 ); }
int SetGamePlayerStateUnderwater ( lua_State *L ) { return SetGamePlayerControlData ( L, 108 ); }
int GetGamePlayerStateUnderwater ( lua_State *L ) { return GetGamePlayerControlData ( L, 108 ); }
int SetGamePlayerStateRightMouseHold ( lua_State *L ) { return SetGamePlayerControlData ( L, 109 ); }
int GetGamePlayerStateRightMouseHold ( lua_State *L ) { return GetGamePlayerControlData ( L, 109 ); }
int SetGamePlayerStateXBOX ( lua_State *L ) { return SetGamePlayerControlData ( L, 110 ); }
int GetGamePlayerStateXBOX ( lua_State *L ) { return GetGamePlayerControlData ( L, 110 ); }
int SetGamePlayerStateXBOXControllerType ( lua_State *L ) { return SetGamePlayerControlData ( L, 99 ); }
int GetGamePlayerStateXBOXControllerType ( lua_State *L ) { return GetGamePlayerControlData ( L, 99 ); }
int JoystickX ( lua_State *L ) { return GetGamePlayerControlData ( L, 111 ); }
int JoystickY ( lua_State *L ) { return GetGamePlayerControlData ( L, 112 ); }
int JoystickZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 113 ); }
int SetGamePlayerStateGunZoomMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 114 ); }
int GetGamePlayerStateGunZoomMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 114 ); }
int SetGamePlayerStateGunZoomMag ( lua_State *L ) { return SetGamePlayerControlData ( L, 115 ); }
int GetGamePlayerStateGunZoomMag ( lua_State *L ) { return GetGamePlayerControlData ( L, 115 ); }
int SetGamePlayerStateGunReloadNoAmmo ( lua_State *L ) { return SetGamePlayerControlData ( L, 116 ); }
int GetGamePlayerStateGunReloadNoAmmo ( lua_State *L ) { return GetGamePlayerControlData ( L, 116 ); }
int SetGamePlayerStatePlrReloading ( lua_State *L ) { return SetGamePlayerControlData ( L, 117 ); }
int GetGamePlayerStatePlrReloading ( lua_State *L ) { return GetGamePlayerControlData ( L, 117 ); }
int SetGamePlayerStateGunAltSwapKey1 ( lua_State *L ) { return SetGamePlayerControlData ( L, 118 ); }
int GetGamePlayerStateGunAltSwapKey1 ( lua_State *L ) { return GetGamePlayerControlData ( L, 118 ); }
int SetGamePlayerStateGunAltSwapKey2 ( lua_State *L ) { return SetGamePlayerControlData ( L, 119 ); }
int GetGamePlayerStateGunAltSwapKey2 ( lua_State *L ) { return GetGamePlayerControlData ( L, 119 ); }
int SetGamePlayerStateWeaponKeySelection ( lua_State *L ) { return SetGamePlayerControlData ( L, 120 ); }
int GetGamePlayerStateWeaponKeySelection ( lua_State *L ) { return GetGamePlayerControlData ( L, 120 ); }
int SetGamePlayerStateWeaponIndex ( lua_State *L ) { return SetGamePlayerControlData ( L, 121 ); }
int GetGamePlayerStateWeaponIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 121 ); }
int SetGamePlayerStateCommandNewWeapon ( lua_State *L ) { return SetGamePlayerControlData ( L, 122 ); }
int GetGamePlayerStateCommandNewWeapon ( lua_State *L ) { return GetGamePlayerControlData ( L, 122 ); }
int SetGamePlayerStateGunID ( lua_State *L ) { return SetGamePlayerControlData ( L, 123 ); }
int GetGamePlayerStateGunID ( lua_State *L ) { return GetGamePlayerControlData ( L, 123 ); }
int SetGamePlayerStateGunSelectionAfterHide ( lua_State *L ) { return SetGamePlayerControlData ( L, 124 ); }
int GetGamePlayerStateGunSelectionAfterHide ( lua_State *L ) { return GetGamePlayerControlData ( L, 124 ); }
int SetGamePlayerStateGunBurst ( lua_State *L ) { return SetGamePlayerControlData ( L, 125 ); }
int GetGamePlayerStateGunBurst ( lua_State *L ) { return GetGamePlayerControlData ( L, 125 ); }
int JoystickHatAngle ( lua_State *L ) { return GetGamePlayerControlData ( L, 503 ); }
int JoystickFireXL ( lua_State *L ) { return GetGamePlayerControlData ( L, 504 ); }
int JoystickTwistX ( lua_State *L ) { return GetGamePlayerControlData ( L, 127 ); }
int JoystickTwistY ( lua_State *L ) { return GetGamePlayerControlData ( L, 128 ); }
int JoystickTwistZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 129 ); }
int SetGamePlayerStatePlrZoomInChange ( lua_State *L ) { return SetGamePlayerControlData ( L, 130 ); }
int GetGamePlayerStatePlrZoomInChange ( lua_State *L ) { return GetGamePlayerControlData ( L, 130 ); }
int SetGamePlayerStatePlrZoomIn ( lua_State *L ) { return SetGamePlayerControlData ( L, 131 ); }
int GetGamePlayerStatePlrZoomIn ( lua_State *L ) { return GetGamePlayerControlData ( L, 131 ); }
int SetGamePlayerStateLuaActiveMouse ( lua_State *L ) { return SetGamePlayerControlData ( L, 132 ); }
int GetGamePlayerStateLuaActiveMouse ( lua_State *L ) { return GetGamePlayerControlData ( L, 132 ); }
int SetGamePlayerStateRealFov ( lua_State *L ) { return SetGamePlayerControlData ( L, 133 ); }
int GetGamePlayerStateRealFov ( lua_State *L ) { return GetGamePlayerControlData ( L, 133 ); }
int SetGamePlayerStateDisablePeeking ( lua_State *L ) { return SetGamePlayerControlData ( L, 134 ); }
int GetGamePlayerStateDisablePeeking ( lua_State *L ) { return GetGamePlayerControlData ( L, 134 ); }
int SetGamePlayerStatePlrHasFocus ( lua_State *L ) { return SetGamePlayerControlData ( L, 135 ); }
int GetGamePlayerStatePlrHasFocus ( lua_State *L ) { return GetGamePlayerControlData ( L, 135 ); }
int SetGamePlayerStateGameRunAsMultiplayer ( lua_State *L ) { return SetGamePlayerControlData ( L, 136 ); }
int GetGamePlayerStateGameRunAsMultiplayer ( lua_State *L ) { return GetGamePlayerControlData ( L, 136 ); }
int SetGamePlayerStateSteamWorksRespawnLeft ( lua_State *L ) { return SetGamePlayerControlData ( L, 137 ); }
int GetGamePlayerStateSteamWorksRespawnLeft ( lua_State *L ) { return GetGamePlayerControlData ( L, 137 ); }
int SetGamePlayerStateMPRespawnLeft ( lua_State *L ) { return SetGamePlayerControlData ( L, 137 ); }
int GetGamePlayerStateMPRespawnLeft ( lua_State *L ) { return GetGamePlayerControlData ( L, 137 ); }
int SetGamePlayerStateTabMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 138 ); }
int GetGamePlayerStateTabMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 138 ); }
int SetGamePlayerStateLowFpsWarning ( lua_State *L ) { return SetGamePlayerControlData ( L, 139 ); }
int GetGamePlayerStateLowFpsWarning ( lua_State *L ) { return GetGamePlayerControlData ( L, 139 ); }
int SetGamePlayerStateCameraFov ( lua_State *L ) { return SetGamePlayerControlData ( L, 140 ); }
int GetGamePlayerStateCameraFov ( lua_State *L ) { return GetGamePlayerControlData ( L, 140 ); }
int SetGamePlayerStateCameraFovZoomed ( lua_State *L ) { return SetGamePlayerControlData ( L, 141 ); }
int GetGamePlayerStateCameraFovZoomed ( lua_State *L ) { return GetGamePlayerControlData ( L, 141 ); }
int SetGamePlayerStateMouseInvert ( lua_State *L ) { return SetGamePlayerControlData ( L, 142 ); }
int GetGamePlayerStateMouseInvert ( lua_State *L ) { return GetGamePlayerControlData ( L, 142 ); }
int SetGamePlayerStateSlowMotion ( lua_State *L ) { return SetGamePlayerControlData ( L, 143 ); }
int GetGamePlayerStateSlowMotion ( lua_State *L ) { return GetGamePlayerControlData ( L, 143 ); }
int SetGamePlayerStateSmoothCameraKeys ( lua_State *L ) { return SetGamePlayerControlData ( L, 144 ); }
int GetGamePlayerStateSmoothCameraKeys ( lua_State *L ) { return GetGamePlayerControlData ( L, 144 ); }
int SetGamePlayerStateCamMouseMoveX ( lua_State *L ) { return SetGamePlayerControlData ( L, 145 ); }
int GetGamePlayerStateCamMouseMoveX ( lua_State *L ) { return GetGamePlayerControlData ( L, 145 ); }
int SetGamePlayerStateCamMouseMoveY ( lua_State *L ) { return SetGamePlayerControlData ( L, 146 ); }
int GetGamePlayerStateCamMouseMoveY ( lua_State *L ) { return GetGamePlayerControlData ( L, 146 ); }
int SetGamePlayerStateGunRecoilX ( lua_State *L ) { return SetGamePlayerControlData ( L, 147 ); }
int GetGamePlayerStateGunRecoilX ( lua_State *L ) { return GetGamePlayerControlData ( L, 147 ); }
int SetGamePlayerStateGunRecoilY ( lua_State *L ) { return SetGamePlayerControlData ( L, 148 ); }
int GetGamePlayerStateGunRecoilY ( lua_State *L ) { return GetGamePlayerControlData ( L, 148 ); }
int SetGamePlayerStateGunRecoilAngleX ( lua_State *L ) { return SetGamePlayerControlData ( L, 149 ); }
int GetGamePlayerStateGunRecoilAngleX ( lua_State *L ) { return GetGamePlayerControlData ( L, 149 ); }
int SetGamePlayerStateGunRecoilAngleY ( lua_State *L ) { return SetGamePlayerControlData ( L, 150 ); }
int GetGamePlayerStateGunRecoilAngleY ( lua_State *L ) { return GetGamePlayerControlData ( L, 150 ); }
int SetGamePlayerStateGunRecoilCorrectY ( lua_State *L ) { return SetGamePlayerControlData ( L, 151 ); }
int GetGamePlayerStateGunRecoilCorrectY ( lua_State *L ) { return GetGamePlayerControlData ( L, 151 ); }
int SetGamePlayerStateGunRecoilCorrectX ( lua_State *L ) { return SetGamePlayerControlData ( L, 152 ); }
int GetGamePlayerStateGunRecoilCorrectX ( lua_State *L ) { return GetGamePlayerControlData ( L, 152 ); }
int SetGamePlayerStateGunRecoilCorrectAngleY ( lua_State *L ) { return SetGamePlayerControlData ( L, 153 ); }
int GetGamePlayerStateGunRecoilCorrectAngleY ( lua_State *L ) { return GetGamePlayerControlData ( L, 153 ); }
int SetGamePlayerStateGunRecoilCorrectAngleX ( lua_State *L ) { return SetGamePlayerControlData ( L, 154 ); }
int GetGamePlayerStateGunRecoilCorrectAngleX ( lua_State *L ) { return GetGamePlayerControlData ( L, 154 ); }
int SetGamePlayerStateCamAngleX ( lua_State *L ) { return SetGamePlayerControlData ( L, 155 ); }
int GetGamePlayerStateCamAngleX ( lua_State *L ) { return GetGamePlayerControlData ( L, 155 ); }
int SetGamePlayerStateCamAngleY ( lua_State *L ) { return SetGamePlayerControlData ( L, 156 ); }
int GetGamePlayerStateCamAngleY ( lua_State *L ) { return GetGamePlayerControlData ( L, 156 ); }
int SetGamePlayerStatePlayerDucking ( lua_State *L ) { return SetGamePlayerControlData ( L, 157 ); }
int GetGamePlayerStatePlayerDucking ( lua_State *L ) { return GetGamePlayerControlData ( L, 157 ); }
int SetGamePlayerStateEditModeActive ( lua_State *L ) { return SetGamePlayerControlData ( L, 158 ); }
int GetGamePlayerStateEditModeActive ( lua_State *L ) { return GetGamePlayerControlData ( L, 158 ); }
int SetGamePlayerStatePlrKeyShift ( lua_State *L ) { return SetGamePlayerControlData ( L, 159 ); }
int GetGamePlayerStatePlrKeyShift ( lua_State *L ) { return GetGamePlayerControlData ( L, 159 ); }
int SetGamePlayerStatePlrKeyShift2 ( lua_State *L ) { return SetGamePlayerControlData ( L, 160 ); }
int GetGamePlayerStatePlrKeyShift2 ( lua_State *L ) { return GetGamePlayerControlData ( L, 160 ); }
int SetGamePlayerStatePlrKeyControl ( lua_State *L ) { return SetGamePlayerControlData ( L, 161 ); }
int GetGamePlayerStatePlrKeyControl ( lua_State *L ) { return GetGamePlayerControlData ( L, 161 ); }
int SetGamePlayerStateNoWater ( lua_State *L ) { return SetGamePlayerControlData ( L, 162 ); }
int GetGamePlayerStateNoWater ( lua_State *L ) { return GetGamePlayerControlData ( L, 162 ); }
int SetGamePlayerStateWaterlineY ( lua_State *L ) { return SetGamePlayerControlData ( L, 163 ); }
int GetGamePlayerStateWaterlineY ( lua_State *L ) { return GetGamePlayerControlData ( L, 163 ); }
int SetGamePlayerStateFlashlightKeyEnabled ( lua_State *L ) { return SetGamePlayerControlData ( L, 164 ); }
int GetGamePlayerStateFlashlightKeyEnabled ( lua_State *L ) { return GetGamePlayerControlData ( L, 164 ); }
int SetGamePlayerStateFlashlightControl ( lua_State *L ) { return SetGamePlayerControlData ( L, 165 ); }
int GetGamePlayerStateFlashlightControl ( lua_State *L ) { return GetGamePlayerControlData ( L, 165 ); }
int SetGamePlayerStateMoving ( lua_State *L ) { return SetGamePlayerControlData ( L, 166 ); }
int GetGamePlayerStateMoving ( lua_State *L ) { return GetGamePlayerControlData ( L, 166 ); }
int SetGamePlayerStateTerrainHeight ( lua_State *L ) { return SetGamePlayerControlData ( L, 167 ); }
int GetGamePlayerStateTerrainHeight ( lua_State *L ) { return GetGamePlayerControlData ( L, 167 ); }
int SetGamePlayerStateJetpackVerticalMove ( lua_State *L ) { return SetGamePlayerControlData ( L, 168 ); }
int GetGamePlayerStateJetpackVerticalMove ( lua_State *L ) { return GetGamePlayerControlData ( L, 168 ); }
int SetGamePlayerStateTerrainID ( lua_State *L ) { return SetGamePlayerControlData ( L, 169 ); }
int GetGamePlayerStateTerrainID ( lua_State *L ) { return GetGamePlayerControlData ( L, 169 ); }
int SetGamePlayerStateEnablePlrSpeedMods ( lua_State *L ) { return SetGamePlayerControlData ( L, 170 ); }
int GetGamePlayerStateEnablePlrSpeedMods ( lua_State *L ) { return GetGamePlayerControlData ( L, 170 ); }
int SetGamePlayerStateRiftMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 171 ); }
int GetGamePlayerStateRiftMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 171 ); }
int SetGamePlayerStatePlayerX ( lua_State *L ) { return SetGamePlayerControlData ( L, 172 ); }
int GetGamePlayerStatePlayerX ( lua_State *L ) { return GetGamePlayerControlData ( L, 172 ); }
int SetGamePlayerStatePlayerY ( lua_State *L ) { return SetGamePlayerControlData ( L, 173 ); }
int GetGamePlayerStatePlayerY ( lua_State *L ) { return GetGamePlayerControlData ( L, 173 ); }
int SetGamePlayerStatePlayerZ ( lua_State *L ) { return SetGamePlayerControlData ( L, 174 ); }
int GetGamePlayerStatePlayerZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 174 ); }
int SetGamePlayerStateTerrainPlayerX ( lua_State *L ) { return SetGamePlayerControlData ( L, 175 ); }
int GetGamePlayerStateTerrainPlayerX ( lua_State *L ) { return GetGamePlayerControlData ( L, 175 ); }
int SetGamePlayerStateTerrainPlayerY ( lua_State *L ) { return SetGamePlayerControlData ( L, 176 ); }
int GetGamePlayerStateTerrainPlayerY ( lua_State *L ) { return GetGamePlayerControlData ( L, 176 ); }
int SetGamePlayerStateTerrainPlayerZ ( lua_State *L ) { return SetGamePlayerControlData ( L, 177 ); }
int GetGamePlayerStateTerrainPlayerZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 177 ); }
int SetGamePlayerStateTerrainPlayerAX ( lua_State *L ) { return SetGamePlayerControlData ( L, 178 ); }
int GetGamePlayerStateTerrainPlayerAX ( lua_State *L ) { return GetGamePlayerControlData ( L, 178 ); }
int SetGamePlayerStateTerrainPlayerAY ( lua_State *L ) { return SetGamePlayerControlData ( L, 179 ); }
int GetGamePlayerStateTerrainPlayerAY ( lua_State *L ) { return GetGamePlayerControlData ( L, 179 ); }
int SetGamePlayerStateTerrainPlayerAZ ( lua_State *L ) { return SetGamePlayerControlData ( L, 180 ); }
int GetGamePlayerStateTerrainPlayerAZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 180 ); }
int SetGamePlayerStateAdjustBasedOnWobbleY ( lua_State *L ) { return SetGamePlayerControlData ( L, 181 ); }
int GetGamePlayerStateAdjustBasedOnWobbleY ( lua_State *L ) { return GetGamePlayerControlData ( L, 181 ); }
int SetGamePlayerStateFinalCamX ( lua_State *L ) { return SetGamePlayerControlData ( L, 182 ); }
int GetGamePlayerStateFinalCamX ( lua_State *L ) { return GetGamePlayerControlData ( L, 182 ); }
int SetGamePlayerStateFinalCamY ( lua_State *L ) { return SetGamePlayerControlData ( L, 183 ); }
int GetGamePlayerStateFinalCamY ( lua_State *L ) { return GetGamePlayerControlData ( L, 183 ); }
int SetGamePlayerStateFinalCamZ ( lua_State *L ) { return SetGamePlayerControlData ( L, 184 ); }
int GetGamePlayerStateFinalCamZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 184 ); }
int SetGamePlayerStateShakeX ( lua_State *L ) { return SetGamePlayerControlData ( L, 185 ); }
int GetGamePlayerStateShakeX ( lua_State *L ) { return GetGamePlayerControlData ( L, 185 ); }
int SetGamePlayerStateShakeY ( lua_State *L ) { return SetGamePlayerControlData ( L, 186 ); }
int GetGamePlayerStateShakeY ( lua_State *L ) { return GetGamePlayerControlData ( L, 186 ); }
int SetGamePlayerStateShakeZ ( lua_State *L ) { return SetGamePlayerControlData ( L, 187 ); }
int GetGamePlayerStateShakeZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 187 ); }
int SetGamePlayerStateImmunity ( lua_State *L ) { return SetGamePlayerControlData ( L, 188 ); }
int GetGamePlayerStateImmunity ( lua_State *L ) { return GetGamePlayerControlData ( L, 188 ); }
int SetGamePlayerStateCharAnimIndex ( lua_State *L ) { return SetGamePlayerControlData ( L, 189 ); }
int GetGamePlayerStateCharAnimIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 189 ); }

int GetGamePlayerStateMotionController ( lua_State *L ) { return GetGamePlayerControlData ( L, 190 ); }
int GetGamePlayerStateMotionControllerType ( lua_State *L ) { return GetGamePlayerControlData ( L, 191 ); }
int MotionControllerThumbstickX ( lua_State *L ) { return GetGamePlayerControlData ( L, 192 ); }
int MotionControllerThumbstickY ( lua_State *L ) { return GetGamePlayerControlData ( L, 193 ); }
int CombatControllerTrigger ( lua_State *L ) { return GetGamePlayerControlData ( L, 194 ); }
int CombatControllerGrip ( lua_State *L ) { return GetGamePlayerControlData ( L, 195 ); }
int CombatControllerThumbstickX ( lua_State *L ) { return GetGamePlayerControlData ( L, 196 ); }
int CombatControllerThumbstickY ( lua_State *L ) { return GetGamePlayerControlData ( L, 197 ); }
int MotionControllerBestX ( lua_State *L ) { return GetGamePlayerControlData ( L, 251 ); }
int MotionControllerBestY ( lua_State *L ) { return GetGamePlayerControlData ( L, 252 ); }
int MotionControllerBestZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 253 ); }
int MotionControllerBestAngleX ( lua_State *L ) { return GetGamePlayerControlData ( L, 254 ); }
int MotionControllerBestAngleY ( lua_State *L ) { return GetGamePlayerControlData ( L, 255 ); }
int MotionControllerBestAngleZ ( lua_State *L ) { return GetGamePlayerControlData ( L, 256 ); }
int MotionControllerLaserGuidedEntityObj ( lua_State *L ) { return GetGamePlayerControlData ( L, 257 ); }

int SetGamePlayerStateIsMelee ( lua_State *L ) { return SetGamePlayerControlData ( L, 201 ); }
int GetGamePlayerStateIsMelee ( lua_State *L ) { return GetGamePlayerControlData ( L, 201 ); }
int SetGamePlayerStateAlternate ( lua_State *L ) { return SetGamePlayerControlData ( L, 202 ); }
int GetGamePlayerStateAlternate ( lua_State *L ) { return GetGamePlayerControlData ( L, 202 ); }
int SetGamePlayerStateModeShareMags ( lua_State *L ) { return SetGamePlayerControlData ( L, 203 ); }
int GetGamePlayerStateModeShareMags ( lua_State *L ) { return GetGamePlayerControlData ( L, 203 ); }
int SetGamePlayerStateAlternateIsFlak ( lua_State *L ) { return SetGamePlayerControlData ( L, 204 ); }
int GetGamePlayerStateAlternateIsFlak ( lua_State *L ) { return GetGamePlayerControlData ( L, 204 ); }
int SetGamePlayerStateAlternateIsRay ( lua_State *L ) { return SetGamePlayerControlData ( L, 205 ); }
int GetGamePlayerStateAlternateIsRay ( lua_State *L ) { return GetGamePlayerControlData ( L, 205 ); }
int SetFireModeSettingsReloadQty ( lua_State *L ) { return SetGamePlayerControlData ( L, 301 ); }
int GetFireModeSettingsReloadQty ( lua_State *L ) { return GetGamePlayerControlData ( L, 301 ); }
int SetFireModeSettingsIsEmpty ( lua_State *L ) { return SetGamePlayerControlData ( L, 302 ); }
int GetFireModeSettingsIsEmpty ( lua_State *L ) { return GetGamePlayerControlData ( L, 302 ); }
int SetFireModeSettingsJammed ( lua_State *L ) { return SetGamePlayerControlData ( L, 303 ); }
int GetFireModeSettingsJammed ( lua_State *L ) { return GetGamePlayerControlData ( L, 303 ); }
int SetFireModeSettingsJamChance ( lua_State *L ) { return SetGamePlayerControlData ( L, 304 ); }
int GetFireModeSettingsJamChance ( lua_State *L ) { return GetGamePlayerControlData ( L, 304 ); }
int SetFireModeSettingsMinTimer ( lua_State *L ) { return SetGamePlayerControlData ( L, 305 ); }
int GetFireModeSettingsMinTimer ( lua_State *L ) { return GetGamePlayerControlData ( L, 305 ); }
int SetFireModeSettingsAddTimer ( lua_State *L ) { return SetGamePlayerControlData ( L, 306 ); }
int GetFireModeSettingsAddTimer ( lua_State *L ) { return GetGamePlayerControlData ( L, 306 ); }
int SetFireModeSettingsShotsFired ( lua_State *L ) { return SetGamePlayerControlData ( L, 307 ); }
int GetFireModeSettingsShotsFired ( lua_State *L ) { return GetGamePlayerControlData ( L, 307 ); }
int SetFireModeSettingsCoolTimer ( lua_State *L ) { return SetGamePlayerControlData ( L, 308 ); }
int GetFireModeSettingsCoolTimer ( lua_State *L ) { return GetGamePlayerControlData ( L, 308 ); }
int SetFireModeSettingsOverheatAfter ( lua_State *L ) { return SetGamePlayerControlData ( L, 309 ); }
int GetFireModeSettingsOverheatAfter ( lua_State *L ) { return GetGamePlayerControlData ( L, 309 ); }
int SetFireModeSettingsJamChanceTime ( lua_State *L ) { return SetGamePlayerControlData ( L, 310 ); }
int GetFireModeSettingsJamChanceTime ( lua_State *L ) { return GetGamePlayerControlData ( L, 310 ); }
int SetFireModeSettingsCoolDown ( lua_State *L ) { return SetGamePlayerControlData ( L, 311 ); }
int GetFireModeSettingsCoolDown ( lua_State *L ) { return GetGamePlayerControlData ( L, 311 ); }
int SetFireModeSettingsNoSubmergedFire ( lua_State *L ) { return SetGamePlayerControlData ( L, 312 ); }
int GetFireModeSettingsNoSubmergedFire ( lua_State *L ) { return GetGamePlayerControlData ( L, 312 ); }
int SetFireModeSettingsSimpleZoom ( lua_State *L ) { return SetGamePlayerControlData ( L, 313 ); }
int GetFireModeSettingsSimpleZoom ( lua_State *L ) { return GetGamePlayerControlData ( L, 313 ); }
int SetFireModeSettingsForceZoomOut ( lua_State *L ) { return SetGamePlayerControlData ( L, 314 ); }
int GetFireModeSettingsForceZoomOut ( lua_State *L ) { return GetGamePlayerControlData ( L, 314 ); }
int SetFireModeSettingsZoomMode ( lua_State *L ) { return SetGamePlayerControlData ( L, 315 ); }
int GetFireModeSettingsZoomMode ( lua_State *L ) { return GetGamePlayerControlData ( L, 315 ); }
int SetFireModeSettingsSimpleZoomAnim ( lua_State *L ) { return SetGamePlayerControlData ( L, 316 ); }
int GetFireModeSettingsSimpleZoomAnim ( lua_State *L ) { return GetGamePlayerControlData ( L, 316 ); }
int SetFireModeSettingsPoolIndex ( lua_State *L ) { return SetGamePlayerControlData ( L, 317 ); }
int GetFireModeSettingsPoolIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 317 ); }
int SetFireModeSettingsPlrTurnSpeedMod ( lua_State *L ) { return SetGamePlayerControlData ( L, 318 ); }
int GetFireModeSettingsPlrTurnSpeedMod ( lua_State *L ) { return GetGamePlayerControlData ( L, 318 ); }
int SetFireModeSettingsZoomTurnSpeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 319 ); }
int GetFireModeSettingsZoomTurnSpeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 319 ); }
int SetFireModeSettingsPlrJumpSpeedMod ( lua_State *L ) { return SetGamePlayerControlData ( L, 320 ); }
int GetFireModeSettingsPlrJumpSpeedMod ( lua_State *L ) { return GetGamePlayerControlData ( L, 320 ); }
int SetFireModeSettingsPlrEmptySpeedMod ( lua_State *L ) { return SetGamePlayerControlData ( L, 321 ); }
int GetFireModeSettingsPlrEmptySpeedMod ( lua_State *L ) { return GetGamePlayerControlData ( L, 321 ); }
int SetFireModeSettingsPlrMoveSpeedMod ( lua_State *L ) { return SetGamePlayerControlData ( L, 322 ); }
int GetFireModeSettingsPlrMoveSpeedMod ( lua_State *L ) { return GetGamePlayerControlData ( L, 322 ); }
int SetFireModeSettingsZoomWalkSpeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 323 ); }
int GetFireModeSettingsZoomWalkSpeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 323 ); }
int SetFireModeSettingsPlrReloadSpeedMod ( lua_State *L ) { return SetGamePlayerControlData ( L, 324 ); }
int GetFireModeSettingsPlrReloadSpeedMod ( lua_State *L ) { return GetGamePlayerControlData ( L, 324 ); }
int SetFireModeSettingsHasEmpty ( lua_State *L ) { return SetGamePlayerControlData ( L, 325 ); }
int GetFireModeSettingsHasEmpty ( lua_State *L ) { return GetGamePlayerControlData ( L, 325 ); }
int GetFireModeSettingsActionBlockStart ( lua_State *L ) { return GetGamePlayerControlData ( L, 326 ); }

int SetGamePlayerStateGunSound ( lua_State *L ) { return SetGamePlayerControlData ( L, 501 ); }
int GetGamePlayerStateGunSound ( lua_State *L ) { return GetGamePlayerControlData ( L, 501 ); }
int SetGamePlayerStateGunAltSound ( lua_State *L ) { return SetGamePlayerControlData ( L, 502 ); }
int GetGamePlayerStateGunAltSound ( lua_State *L ) { return GetGamePlayerControlData ( L, 502 ); }

int CopyCharAnimState ( lua_State *L ) { return SetGamePlayerControlData ( L, 700 ); }
int SetCharAnimStatePlayCsi ( lua_State *L ) { return SetGamePlayerControlData ( L, 701 ); }
int GetCharAnimStatePlayCsi ( lua_State *L ) { return GetGamePlayerControlData ( L, 701 ); }
int SetCharAnimStateOriginalE ( lua_State *L ) { return SetGamePlayerControlData ( L, 702 ); }
int GetCharAnimStateOriginalE ( lua_State *L ) { return GetGamePlayerControlData ( L, 702 ); }
int SetCharAnimStateObj ( lua_State *L ) { return SetGamePlayerControlData ( L, 703 ); }
int GetCharAnimStateObj ( lua_State *L ) { return GetGamePlayerControlData ( L, 703 ); }
int SetCharAnimStateAnimationSpeed ( lua_State *L ) { return SetGamePlayerControlData ( L, 704 ); }
int GetCharAnimStateAnimationSpeed ( lua_State *L ) { return GetGamePlayerControlData ( L, 704 ); }
int SetCharAnimStateE ( lua_State *L ) { return SetGamePlayerControlData ( L, 705 ); }
int GetCharAnimStateE ( lua_State *L ) { return GetGamePlayerControlData ( L, 705 ); }
int GetCsiStoodVault ( lua_State *L ) { return GetGamePlayerControlData ( L, 741 ); }
int GetCharSeqTrigger ( lua_State *L ) { return GetGamePlayerControlData ( L, 751 ); }
int GetEntityElementBankIndex ( lua_State *L ) { return GetGamePlayerControlData ( L, 761 ); }
int GetEntityElementObj ( lua_State *L ) { return GetGamePlayerControlData ( L, 762 ); }
int GetEntityElementRagdollified ( lua_State *L ) { return GetGamePlayerControlData ( L, 763 ); }
int GetEntityElementSpeedModulator ( lua_State *L ) { return GetGamePlayerControlData ( L, 764 ); }
int SetCharAnimControlsLeaping ( lua_State *L ) { return SetGamePlayerControlData ( L, 801 ); }
int GetCharAnimControlsLeaping ( lua_State *L ) { return GetGamePlayerControlData ( L, 801 ); }
int SetCharAnimControlsMoving ( lua_State *L ) { return SetGamePlayerControlData ( L, 802 ); }
int GetCharAnimControlsMoving ( lua_State *L ) { return GetGamePlayerControlData ( L, 802 ); }
int GetEntityProfileJumpModifier ( lua_State *L ) { return GetGamePlayerControlData ( L, 851 ); }
int GetEntityProfileStartOfAIAnim ( lua_State *L ) { return GetGamePlayerControlData ( L, 852 ); }
int GetEntityProfileJumpHold ( lua_State *L ) { return GetGamePlayerControlData ( L, 853 ); }
int GetEntityProfileJumpResume ( lua_State *L ) { return GetGamePlayerControlData ( L, 854 ); }

int GetEntityAnimStart ( lua_State *L ) { return GetGamePlayerControlData ( L, 1001 ); }
int GetEntityAnimFinish ( lua_State *L ) { return GetGamePlayerControlData ( L, 1002 ); }

int SetRotationYSlowly ( lua_State *L ) 
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 3 ) return 0;
	int iEntityID = lua_tonumber( L, 1 );
	float fDestAngle = lua_tonumber( L, 2 );
	float fSlowlyRate = lua_tonumber( L, 3 );
	int iStoreE = t.e; t.e = iEntityID;
	float fStoreV = t.v; t.v = fSlowlyRate;
	entity_lua_rotatetoanglecore ( fDestAngle, 0.0f );
	t.e = iStoreE;
	t.v = fStoreV;
	return 1;
}


// Particle commands

int ParticlesGetFreeEmitter ( lua_State *L )
{
	lua = L;
	ravey_particles_get_free_emitter ( );
	lua_pushnumber ( L, t.tResult );
	return 1;
}

int ParticlesLoadImage(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 1) return 0;

	int iID = 0;
	if (n == 2) iID = lua_tonumber(L, 2);

	char pFileName[256];
	strcpy(pFileName, lua_tostring(L, 1));

	iID = ravey_particles_load_image(pFileName, iID);

	if (iID == -1) return 0;

	lua_pushnumber(L, iID);
	return 1;
}

int ParticlesLoadEffect(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 2) return 0;

	char pFileName[256];
	strcpy(pFileName, lua_tostring(L, 1));

	lua_pushnumber(L, ravey_particles_load_effect(pFileName, lua_tonumber(L, 2)));
	return 1;
}

int ParticlesSetFrames(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 4) return 0;

	ravey_particles_set_frames(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}

int ParticlesSetSpeed(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 7) return 0;

	ravey_particles_set_speed(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
		lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7));
	return 0;
}

int ParticlesSetGravity(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 3) return 0;

	ravey_particles_set_gravity(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
	return 0;
}

int ParticlesSetOffset(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 7) return 0;

	ravey_particles_set_offset(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
		lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7));
	return 0;
}

int ParticlesSetAngle(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 4) return 0;

	ravey_particles_set_angle(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}

int ParticlesSetRotation(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 8) return 0;

	ravey_particles_set_rotate(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
		lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));
	return 0;
}

int ParticlesSetScale(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 5) return 0;

	ravey_particles_set_scale(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
		lua_tonumber(L, 5));
	return 0;
}

int ParticlesSetAlpha(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 5) return 0;

	ravey_particles_set_alpha(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4),
		lua_tonumber(L, 5));
	return 0;
}

int ParticlesSetLife( lua_State *L )
{
	lua = L;
	int n = lua_gettop( L );
	if ( n < 6 ) return 0;

	if ( n == 6 )
	{
		ravey_particles_set_life( lua_tonumber( L, 1 ), lua_tonumber( L, 2 ), lua_tonumber( L, 3 ), lua_tonumber( L, 4 ),
			                      lua_tonumber( L, 5 ), lua_tonumber( L, 6 ), RAVEY_PARTICLES_MAX_SPAWNED_AT_ONCE_BY_AN_EMITTER );
	}
	else
	{
		ravey_particles_set_life( lua_tonumber( L, 1 ), lua_tonumber( L, 2 ), lua_tonumber( L, 3 ), lua_tonumber( L, 4 ),
			                      lua_tonumber( L, 5 ), lua_tonumber( L, 6 ), lua_tonumber( L, 7 ) );
	}
	return 0;
}

int ParticlesSetWindVector(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 2) return 0;

	ravey_particles_set_wind_vector(lua_tonumber(L, 1), lua_tonumber(L, 2));
	return 0;
}

int ParticlesSetNoWind(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 1) return 0;

	ravey_particles_set_no_wind(lua_tonumber(L, 1));
	return 0;
}

int ParticlesSpawnParticle(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 1) return 0;

	if (n < 4)
	{
		ravey_particles_generate_particle(lua_tonumber(L, 1), 0, 0, 0);
	}
	else
	{
		ravey_particles_generate_particle(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	}
	return 0;
}

int ParticlesAddEmitterCore(lua_State *L, int iExtended)
{
	lua = L;
	int n = lua_gettop(L);
	if (iExtended == 0)
	{
		if (n < 28) return 0;
	}
	else
	{
		if (n < 32) return 0;
	}

	// populate emitter data
	t.tResult = lua_tonumber(L, 1);
	float animationSpeed = lua_tonumber(L, 2);
	float startsOffRandomAngle = lua_tonumber(L, 4);
	float offsetMinX = lua_tonumber(L, 4);
	float offsetMinY = lua_tonumber(L, 5);
	float offsetMinZ = lua_tonumber(L, 6);
	float offsetMaxX = lua_tonumber(L, 7);
	float offsetMaxY = lua_tonumber(L, 8);
	float offsetMaxZ = lua_tonumber(L, 9);
	float scaleStartMin = lua_tonumber(L, 10);
	float scaleStartMax = lua_tonumber(L, 11);
	float scaleEndMin = lua_tonumber(L, 12);
	float scaleEndMax = lua_tonumber(L, 13);
	float movementSpeedMinX = lua_tonumber(L, 14);
	float movementSpeedMinY = lua_tonumber(L, 15);
	float movementSpeedMinZ = lua_tonumber(L, 16);
	float movementSpeedMaxX = lua_tonumber(L, 17);
	float movementSpeedMaxY = lua_tonumber(L, 18);
	float movementSpeedMaxZ = lua_tonumber(L, 19);
	float rotateSpeedMinZ = lua_tonumber(L, 20);
	float rotateSpeedMaxZ = lua_tonumber(L, 21);
	float lifeMin = lua_tonumber(L, 22);
	float lifeMax = lua_tonumber(L, 23);
	float alphaStartMin = lua_tonumber(L, 24);
	float alphaStartMax = lua_tonumber(L, 25);
	float alphaEndMin = lua_tonumber(L, 26);
	float alphaEndMax = lua_tonumber(L, 27);
	float frequency = lua_tonumber(L, 28);
	int entID = -1;
	int entLimbIndex = -1;
	int particleImage = RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset;
	int particleFrameCount = 64;
	if (iExtended == 1)
	{
		entID = lua_tonumber(L, 29);
		entLimbIndex = lua_tonumber(L, 30);
		int tCheckParticleImage = lua_tonumber(L, 31);
		if (tCheckParticleImage > 0)
		{
			particleImage = tCheckParticleImage + g.particlesimageoffset;
			particleFrameCount = lua_tonumber(L, 32);
		}
	}
	g.tEmitter.id = t.tResult;
	g.tEmitter.emitterLife = 0;
	if (entID == -1)
	{
		g.tEmitter.parentObject = t.aisystem.objectstartindex;
		g.tEmitter.parentLimb = 0;
	}
	else if (entID > 0)
	{
		g.tEmitter.parentObject = t.entityelement[entID].obj;
		g.tEmitter.parentLimb = entLimbIndex;
	}
	else
	{
		g.tEmitter.parentObject = 0;
	}
	g.tEmitter.isAnObjectEmitter = 0;
	g.tEmitter.startsOffRandomAngle = startsOffRandomAngle;
	g.tEmitter.offsetMinX = offsetMinX;
	g.tEmitter.offsetMinY = offsetMinY;
	g.tEmitter.offsetMinZ = offsetMinZ;
	g.tEmitter.offsetMaxX = offsetMaxX;
	g.tEmitter.offsetMaxY = offsetMaxY;
	g.tEmitter.offsetMaxZ = offsetMaxZ;
	g.tEmitter.scaleStartMin = scaleStartMin;
	g.tEmitter.scaleStartMax = scaleStartMax;
	g.tEmitter.scaleEndMin = scaleEndMin;
	g.tEmitter.scaleEndMax = scaleEndMax;
	g.tEmitter.movementSpeedMinX = movementSpeedMinX;
	g.tEmitter.movementSpeedMinY = movementSpeedMinY;
	g.tEmitter.movementSpeedMinZ = movementSpeedMinZ;
	g.tEmitter.movementSpeedMaxX = movementSpeedMaxX;
	g.tEmitter.movementSpeedMaxY = movementSpeedMaxY;
	g.tEmitter.movementSpeedMaxZ = movementSpeedMaxZ;
	g.tEmitter.rotateSpeedMinZ = rotateSpeedMinZ;
	g.tEmitter.rotateSpeedMaxZ = rotateSpeedMaxZ;
	g.tEmitter.startGravity = 0;
	g.tEmitter.endGravity = 0;
	g.tEmitter.lifeMin = lifeMin;
	g.tEmitter.lifeMax = lifeMax;
	g.tEmitter.alphaStartMin = alphaStartMin;
	g.tEmitter.alphaStartMax = alphaStartMax;
	g.tEmitter.alphaEndMin = alphaEndMin;
	g.tEmitter.alphaEndMax = alphaEndMax;
	g.tEmitter.frequency = frequency;

	// fixed animation for smoke
	g.tEmitter.imageNumber = particleImage;//RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset;
	g.tEmitter.isAnimated = 1;
	g.tEmitter.frameCount = particleFrameCount;

	if (animationSpeed > 0)
	{
		g.tEmitter.animationSpeed = animationSpeed;
		g.tEmitter.isLooping = 1;
		g.tEmitter.startFrame = 0;
		g.tEmitter.endFrame = g.tEmitter.frameCount - 1;
	}
	else
	{
		g.tEmitter.animationSpeed = 0;
		g.tEmitter.isLooping = 0;
	}

	// create emitter
	ravey_particles_add_emitter();

	return 0;
}

int ParticlesAddEmitter( lua_State *L )
{
	return ParticlesAddEmitterCore ( L, 0 );
}

int ParticlesAddEmitterEx( lua_State *L )
{
	return ParticlesAddEmitterCore ( L, 1 );
}

int ParticlesDeleteEmitter( lua_State *L )
{
	lua = L;
	int n = lua_gettop(L);
	if ( n < 1 ) return 0;
	t.tRaveyParticlesEmitterID = lua_tonumber(L, 1);
	ravey_particles_delete_emitter ( );
	return 0;
}

int GetBulletHit(lua_State *L)
{
	if (t.tdamagesource == 1)
	{
		lua_pushnumber(L, g.decalx);
		lua_pushnumber(L, g.decaly);
		lua_pushnumber(L, g.decalz);
		lua_pushnumber(L, t.tttriggerdecalimpact);
		lua_pushnumber(L, t.playercontrol.thirdperson.charactere);
		t.tdamagesource = 0;
		return 5;
	}
	return 0;
}

// Image commands

int GetFreeLUAImageID ( void )
{
	for ( int c = g.LUAImageoffset ; c <= g.LUAImageoffsetMax ; c++ )
	{
		if ( ImageExist ( c ) == 0 )
			return c;
	}

	return 0;
}

int GetFreeLUASpriteID ( void )
{
	for ( int c = g.LUASpriteoffset ; c <= g.LUASpriteoffsetMax ; c++ )
	{
		if ( SpriteExist ( c ) == 0 )
			return c;
	}
	return 0;
}

void FreeLUASpritesAndImages ( void )
{
	for ( int c = g.LUASpriteoffset ; c <= g.LUASpriteoffsetMax ; c++ )
	{
		if ( SpriteExist ( c ) == 1 )
			DeleteSprite ( c );
	}

	for ( int c = g.LUAImageoffset ; c <= g.LUAImageoffsetMax ; c++ )
	{
		if ( ImageExist ( c ) == 1 )
			DeleteImage ( c );
	}
}

void HideOrShowLUASprites ( bool hide )
{
	for ( int c = g.LUASpriteoffset ; c <= g.LUASpriteoffsetMax ; c++ )
	{
		if ( SpriteExist ( c ) == 1 )
		{
			if ( hide ) 
				HideSprite ( c );
			else
				ShowSprite ( c );
		}
	}
}

int SetFlashLight ( lua_State *L )
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 1 )
		return 0;

	int mode = lua_tointeger(L, 1);

	if ( mode == 1 )
		t.playerlight.flashlightcontrol_f = 1.0f;
	else
		t.playerlight.flashlightcontrol_f = 0.0f;

		return 0;
}

int SetPlayerRun ( lua_State *L )
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 1 )
		return 0;

	int mode = lua_tointeger(L, 1);

	t.playercontrol.canrun = mode;

	return 0;
}

int SetFont ( lua_State *L )
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 2 )
		return 0;

	char fileName[MAX_PATH];
	char path[MAX_PATH];
	char finalPath[MAX_PATH];
	char fpspath[MAX_PATH];

	strcpy ( fpspath , g.fpscrootdir_s.Get() );
	strcat ( fpspath , "\\Files\\" );

	strcpy ( finalPath , "" );

	strcpy ( fileName , lua_tostring(L,1) );
	strcpy ( path , fpspath );
	strcat ( path , "fontbank\\FPSCR-Font-" );
	strcat ( path , fileName ); 
	if ( FileExist ( path ) == 1 )
		strcpy ( finalPath , path );

	strcpy ( fileName , lua_tostring(L,1) );
	strcpy ( path , fpspath );
	strcat ( path , "fontbank\\FPSCR-" );
	strcat ( path , fileName ); 
	if ( FileExist ( path ) == 1 )
		strcpy ( finalPath , path );

	strcpy ( fileName , lua_tostring(L,1) );
	strcpy ( path , fpspath );
	strcat ( path , "fontbank\\" );
	strcat ( path , fileName ); 
	if ( FileExist ( path ) == 1 )
		strcpy ( finalPath , path );

	if ( strcmp ( finalPath , "" ) == 0 ) return 0;

	if (finalPath[strlen(finalPath) - 4] == '.')
		// this is a c style string terminator, so alternative to fix custom fonts for Cogwheel
		//finalPath[strlen(finalPath)-4] = '\0';
	{
		finalPath[strlen(finalPath) - 4] = 0;
	}

	changebitmapfont(finalPath, lua_tointeger(L, 2));

	return 0;
}

int SetOcclusion ( lua_State *L )
{

	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 1 )
		return 0;

	t.slidersmenuvalue[t.slidersmenunames.graphicoptions][5].value = lua_tointeger(L, 1);
	CPU3DSetPolyCount ( t.slidersmenuvalue[t.slidersmenunames.graphicoptions][5].value );

	return 0;
}

int SetFlashLightKeyEnabled ( lua_State *L )
{

	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 1 )
		return 0;

	int value = lua_tointeger(L, 1);

	if ( value != 0 && value != 1 ) return 0;

	g.flashLightKeyEnabled = value;

	return 0;
}

int SetPlayerWeapons ( lua_State *L )
{

	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 1 )
		return 0;

	int mode = lua_tointeger(L, 1);
	
	if ( mode == 0 )
	{
		g.noPlayerGuns = true;
		g.remembergunid = g.autoloadgun;
		if ( g.autoloadgun != 0 ) 
		{ 
			g.autoloadgun=0 ;
			gun_change ( );
		}
		physics_no_gun_zoom ( );
	}
	else
	{	
		g.noPlayerGuns = false;
		
		if ( g.remembergunid != 0 ) 
		{ 
			g.autoloadgun = g.remembergunid;			
			gun_change ( );
			physics_no_gun_zoom ( );
			g.remembergunid = 0;
		}		
	}

	return 0;
}

int FirePlayerWeapon(lua_State *L)
{
	lua = L;
	int n = lua_gettop(L);
	if (n < 1) return 0;

	int firingmode = lua_tointeger(L, 1);
	//t.player[1].state.firingmode = firingmode; // 121215 - seems when in game, this gets ignored so no gunshoot happens..
	if ( firingmode == 1 )
	{
		if ( t.gunmode < 101 )
			t.gunmode = 101;
	}
	if ( firingmode >= 2 )
	{
		t.player[1].state.firingmode = firingmode;
	}

	return 0;
}

int SetAttachmentVisible ( lua_State *L )
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	// Not enough params, return out
	if ( n < 2 )
		return 0;

	int e = lua_tointeger(L, 1);
	int v = lua_tointeger(L, 2);

	if ( v == 1 )
	{
		if ( t.entityelement[e].attachmentobj > 0 )
		{
			if ( ObjectExist ( t.entityelement[e].attachmentobj ) == 1 )
				ShowObject ( t.entityelement[e].attachmentobj );
		}
	}
	else
	{
		if ( t.entityelement[e].attachmentobj > 0 )
		{
			if ( ObjectExist ( t.entityelement[e].attachmentobj ) == 1 )
				HideObject ( t.entityelement[e].attachmentobj );
		}
	}

		return 0;
}

int Include(lua_State *L)
{
	lua = L;

	// get number of arguments
	int n = lua_gettop(L);

	if ( n < 1 ) return 0;

	char str[MAX_PATH];
	strcpy ( str , "scriptbank\\" );
	strcat ( str , lua_tostring(L, 1) );

	LoadLua ( str );

	return 0;
}

void addFunctions()
{
	lua_register(lua, "SendMessage", LuaSendMessage);
	lua_register(lua, "SendMessageI", LuaSendMessageI);
	lua_register(lua, "SendMessageF", LuaSendMessageF);
	lua_register(lua, "SendMessageS", LuaSendMessageS);
	lua_register(lua, "Include", Include);

	// Direct Calls
	lua_register(lua, "RestoreGameFromSlot", RestoreGameFromSlot);
	lua_register(lua, "ResetFade", ResetFade);

	lua_register(lua, "GetInternalSoundState", GetInternalSoundState);
	lua_register(lua, "SetInternalSoundState", SetInternalSoundState);
	lua_register(lua, "SetCheckpoint", SetCheckpoint);

	lua_register(lua, "UpdateWeaponStats", UpdateWeaponStats);
	lua_register(lua, "ResetWeaponSystems", ResetWeaponSystems);
	lua_register(lua, "GetWeaponSlotGot", GetWeaponSlotGot);
	lua_register(lua, "GetWeaponSlotNoSelect", GetWeaponSlotNoSelect);
	lua_register(lua, "SetWeaponSlot", SetWeaponSlot);

	lua_register(lua, "GetWeaponAmmo", GetWeaponAmmo);
	lua_register(lua, "SetWeaponAmmo", SetWeaponAmmo);
	lua_register(lua, "GetWeaponClipAmmo", GetWeaponClipAmmo);
	lua_register(lua, "SetWeaponClipAmmo", SetWeaponClipAmmo);
	lua_register(lua, "GetWeaponPoolAmmo", GetWeaponPoolAmmo);
	lua_register(lua, "SetWeaponPoolAmmo", SetWeaponPoolAmmo);

	lua_register(lua, "GetWeaponSlot", GetWeaponSlot);
	lua_register(lua, "GetPlayerWeaponID", GetPlayerWeaponID);
	lua_register(lua, "GetWeaponID", GetWeaponID);
	lua_register(lua, "GetEntityWeaponID", GetEntityWeaponID);
	lua_register(lua, "SetWeaponDamage", SetWeaponDamage);
	lua_register(lua, "SetWeaponAccuracy", SetWeaponAccuracy);
	lua_register(lua, "SetWeaponReloadQuantity", SetWeaponReloadQuantity);
	lua_register(lua, "SetWeaponFireIterations", SetWeaponFireIterations);
	lua_register(lua, "SetWeaponRange", SetWeaponRange);
	lua_register(lua, "SetWeaponDropoff", SetWeaponDropoff);
	lua_register(lua, "SetWeaponSpotLighting", SetWeaponSpotLighting);
	lua_register(lua, "GetWeaponDamage", GetWeaponDamage);
	lua_register(lua, "GetWeaponAccuracy", GetWeaponAccuracy);
	lua_register(lua, "GetWeaponReloadQuantity", GetWeaponReloadQuantity);
	lua_register(lua, "GetWeaponFireIterations", GetWeaponFireIterations);
	lua_register(lua, "GetWeaponRange", GetWeaponRange);
	lua_register(lua, "GetWeaponDropoff", GetWeaponDropoff);
	lua_register(lua, "GetWeaponSpotLighting", GetWeaponSpotLighting);

	lua_register(lua, "WrapAngle", WrapAngle);
	lua_register(lua, "GetCameraOverride", GetCameraOverride);
	lua_register(lua, "SetCameraOverride", SetCameraOverride);
	lua_register(lua, "SetCameraFOV", SetCameraFOV);
	lua_register(lua, "SetCameraPosition", SetCameraPosition);
	lua_register(lua, "SetCameraAngle", SetCameraAngle);
	lua_register(lua, "SetCameraFreeFlight", SetCameraFreeFlight);
	lua_register(lua, "GetCameraPositionX", GetCameraPositionX);
	lua_register(lua, "GetCameraPositionY", GetCameraPositionY);
	lua_register(lua, "GetCameraPositionZ", GetCameraPositionZ);
	lua_register(lua, "GetCameraAngleX", GetCameraAngleX);
	lua_register(lua, "GetCameraAngleY", GetCameraAngleY);
	lua_register(lua, "GetCameraAngleZ", GetCameraAngleZ);

	lua_register(lua, "ForcePlayer", ForcePlayer);

	lua_register(lua, "SetEntityActive", SetEntityActive);
	lua_register(lua, "SetEntityActivated", SetEntityActivated);
	lua_register(lua, "SetEntityCollected", SetEntityCollected);
	lua_register(lua, "SetEntityHasKey", SetEntityHasKey);
	lua_register(lua, "GetEntityActive", GetEntityActive);
	lua_register(lua, "GetEntityVisibility", GetEntityVisibility);
	lua_register(lua, "GetEntityPositionX", GetEntityPositionX);
	lua_register(lua, "GetEntityPositionY", GetEntityPositionY);
	lua_register(lua, "GetEntityPositionZ", GetEntityPositionZ);
	lua_register(lua, "GetEntityCollBox", GetEntityCollBox);
	lua_register(lua, "GetEntityPosAng", GetEntityPosAng);
	lua_register(lua, "GetEntityWeight", GetEntityWeight);
	lua_register(lua, "GetEntityScales", GetEntityScales);
	lua_register(lua, "GetEntityName", GetEntityName);
	lua_register(lua, "GetEntityAngleX", GetEntityAngleX);
	lua_register(lua, "GetEntityAngleY", GetEntityAngleY);
	lua_register(lua, "GetEntityAngleZ", GetEntityAngleZ);
	lua_register(lua, "GetMovementSpeed", GetMovementSpeed);
	lua_register(lua, "GetAnimationSpeed", GetAnimationSpeed);
	lua_register(lua, "SetAnimationSpeedModulation", SetAnimationSpeedModulation);
	lua_register(lua, "GetAnimationSpeedModulation", GetAnimationSpeedModulation);
	lua_register(lua, "GetMovementDelta", GetMovementDelta);
	lua_register(lua, "GetMovementDeltaManually", GetMovementDeltaManually);
	lua_register(lua, "GetEntityMarkerMode", GetEntityMarkerMode);


	lua_register(lua, "SetEntityString", SetEntityString);
	lua_register(lua, "GetEntityString", GetEntityString);

	lua_register(lua, "GetLimbName", GetLimbName);

	lua_register(lua, "SetEntitySpawnAtStart", SetEntitySpawnAtStart);
	lua_register(lua, "GetEntitySpawnAtStart", GetEntitySpawnAtStart);
	lua_register(lua, "GetEntityFilePath", GetEntityFilePath);
	lua_register(lua, "SetPreExitValue", SetPreExitValue);

	lua_register(lua, "SetEntityAnimation", SetEntityAnimation);
	lua_register(lua, "GetEntityAnimationStart", GetEntityAnimationStart);
	lua_register(lua, "GetEntityAnimationFinish", GetEntityAnimationFinish);
	lua_register(lua, "GetEntityAnimationFound", GetEntityAnimationFound);
	lua_register(lua, "GetObjectAnimationFinished", GetObjectAnimationFinished);

	lua_register(lua, "GetEntityFootfallMax", GetEntityFootfallMax);
	lua_register(lua, "GetEntityFootfallKeyframe", GetEntityFootfallKeyframe);

	lua_register(lua, "GetAmmoClipMax", GetAmmoClipMax);
	lua_register(lua, "GetAmmoClip", GetAmmoClip);
	lua_register(lua, "SetAmmoClip", SetAmmoClip);

	// Entity Physics
	lua_register(lua, "FreezeEntity", FreezeEntity);
	lua_register(lua, "UnFreezeEntity", UnFreezeEntity);

	// Terrain
	lua_register(lua, "GetTerrainHeight", GetTerrainHeight);
	
	// DarkAI - Legacy Automatic Mode
	lua_register(lua, "AIEntityAssignPatrolPath" , AIEntityAssignPatrolPath );
	lua_register(lua, "AIEntityAddTarget" , AIEntityAddTarget );
	lua_register(lua, "AIEntityRemoveTarget" , AIEntityRemoveTarget );
	lua_register(lua, "AIEntityMoveToCover" , AIEntityMoveToCover );
	lua_register(lua, "AIGetEntityCanFire" , AIGetEntityCanFire );

	// DarkAI - New Manual Mode Control
	lua_register(lua, "AISetEntityControl", AISetEntityControl);
	lua_register(lua, "AIEntityStop" , AIEntityStop );
	lua_register(lua, "AIGetEntityCanSee" , AIGetEntityCanSee );
	lua_register(lua, "AIGetEntityViewRange" , AIGetEntityViewRange );
	lua_register(lua, "AIGetEntitySpeed" , AIGetEntitySpeed );
	lua_register(lua, "AISetEntityMoveBoostPriority" , AISetEntityMoveBoostPriority );
	lua_register(lua, "AIEntityGoToPosition" , AIEntityGoToPosition );
	lua_register(lua, "AIGetEntityHeardSound" , AIGetEntityHeardSound );
	lua_register(lua, "AISetEntityPosition" , AISetEntityPosition );
	lua_register(lua, "AISetEntityTurnSpeed" , AISetEntityTurnSpeed );
	lua_register(lua, "AIGetEntityAngleY" , AIGetEntityAngleY );
	lua_register(lua, "AIGetEntityIsMoving" , AIGetEntityIsMoving );

	lua_register(lua, "AIGetTotalPaths" , AIGetTotalPaths );
	lua_register(lua, "AIGetPathCountPoints" , AIGetPathCountPoints );
	lua_register(lua, "AIPathGetPointX" , AIPathGetPointX );
	lua_register(lua, "AIPathGetPointY" , AIPathGetPointY );
	lua_register(lua, "AIPathGetPointZ" , AIPathGetPointZ );

	lua_register(lua, "AIGetTotalCover" , AIGetTotalCover );
	lua_register(lua, "AICoverGetPointX" , AICoverGetPointX );
	lua_register(lua, "AICoverGetPointY" , AICoverGetPointY );
	lua_register(lua, "AICoverGetPointZ" , AICoverGetPointZ );
	lua_register(lua, "AICoverGetAngle" , AICoverGetAngle );
	lua_register(lua, "AICoverGetIfUsed" , AICoverGetIfUsed );
	lua_register(lua, "AICouldSee", AICouldSee);

	// New Attachment Commands

	// New Debugging Commands
		
	// New RecastDetour (RD) AI Commands
	
	// Visual Attribs
	lua_register(lua, "GetFogNearest" , GetFogNearest );
	lua_register(lua, "GetFogDistance" , GetFogDistance );
	lua_register(lua, "GetFogRed" , GetFogRed );
	lua_register(lua, "GetFogGreen" , GetFogGreen );
	lua_register(lua, "GetFogBlue" , GetFogBlue );
	lua_register(lua, "GetFogIntensity" , GetFogIntensity );
	lua_register(lua, "GetAmbienceIntensity" , GetAmbienceIntensity );
	lua_register(lua, "GetAmbienceRed" , GetAmbienceRed );
	lua_register(lua, "GetAmbienceGreen" , GetAmbienceGreen );
	lua_register(lua, "GetAmbienceBlue" , GetAmbienceBlue );
	lua_register(lua, "GetSurfaceRed" , GetSurfaceRed );
	lua_register(lua, "GetSurfaceGreen" , GetSurfaceGreen );
	lua_register(lua, "GetSurfaceBlue" , GetSurfaceBlue );
	lua_register(lua, "GetSurfaceIntensity" , GetSurfaceIntensity );
	lua_register(lua, "GetPostVignetteRadius" , GetPostVignetteRadius );
	lua_register(lua, "GetPostVignetteIntensity" , GetPostVignetteIntensity );
	lua_register(lua, "GetPostMotionDistance" , GetPostMotionDistance );
	lua_register(lua, "GetPostMotionIntensity" , GetPostMotionIntensity );
	lua_register(lua, "GetPostDepthOfFieldDistance" , GetPostDepthOfFieldDistance );
	lua_register(lua, "GetPostDepthOfFieldIntensity" , GetPostDepthOfFieldIntensity );

	lua_register(lua, "LoadImage" , LoadImage );
	lua_register(lua, "DeleteImage" , DeleteSpriteImage );
	lua_register(lua, "GetImageWidth" , GetImageWidth );
	lua_register(lua, "GetImageHeight" , GetImageHeight );
	lua_register(lua, "CreateSprite" , CreateSprite );
	lua_register(lua, "PasteSprite" , PasteSprite );
	lua_register(lua, "PasteSpritePosition" , PasteSpritePosition );
	lua_register(lua, "SetSpritePosition" , SetSpritePosition );
	lua_register(lua, "SetSpriteSize" , SetSpriteSize );
	lua_register(lua, "SetSpriteDepth" , SetSpriteDepth );
	lua_register(lua, "SetSpriteColor" , SetSpriteColor );	
	lua_register(lua, "SetSpriteAngle" , SetSpriteAngle );	
	lua_register(lua, "SetSpriteOffset" , SetSpriteOffset );
	lua_register(lua, "DeleteSprite" , DeleteSprite );
	lua_register(lua, "SetSpriteImage" , SetSpriteImage );	
	lua_register(lua, "DrawSpritesFirst" , DrawSpritesFirstForLUA );	
	lua_register(lua, "DrawSpritesLast" , DrawSpritesLastForLUA );	
	lua_register(lua, "BackdropOff" , BackdropOffForLUA );	
	lua_register(lua, "BackdropOn" , BackdropOnForLUA );	
	
	lua_register(lua, "LoadGlobalSound" , LoadGlobalSound );	
	lua_register(lua, "PlayGlobalSound" , PlayGlobalSound );	
	lua_register(lua, "LoopGlobalSound" , LoopGlobalSound );	
	lua_register(lua, "StopGlobalSound" , StopGlobalSound );	
	lua_register(lua, "DeleteGlobalSound" , DeleteGlobalSound );	
	lua_register(lua, "SetGlobalSoundSpeed" , SetGlobalSoundSpeed );	
	lua_register(lua, "SetGlobalSoundVolume" , SetGlobalSoundVolume );	
	lua_register(lua, "GetGlobalSoundExist" , GetGlobalSoundExist );
	lua_register(lua, "GetGlobalSoundPlaying" , GetGlobalSoundPlaying );
	lua_register(lua, "GetGlobalSoundLooping" , GetGlobalSoundLooping );

	lua_register(lua, "PlayRawSound" , PlayRawSound );
	lua_register(lua, "LoopRawSound" , LoopRawSound );
	lua_register(lua, "StopRawSound" , StopRawSound );
	lua_register(lua, "SetRawSoundVolume" , SetRawSoundVolume );
	lua_register(lua, "SetRawSoundSpeed" , SetRawSoundSpeed );
	lua_register(lua, "RawSoundExist" , RawSoundExist );
	lua_register(lua, "RawSoundPlaying" , RawSoundPlaying );
	lua_register(lua, "GetEntityRawSound" , GetEntityRawSound );

	#ifdef VRTECH
	lua_register(lua, "GetSpeech" , GetSpeech );
	#endif

	lua_register(lua, "GetTimeElapsed" , GetTimeElapsed );
	lua_register(lua, "GetKeyState" , GetKeyState );
	lua_register(lua, "Timer" , GetGlobalTimer );
	lua_register(lua, "MouseMoveX" , MouseMoveX );
	lua_register(lua, "MouseMoveY" , MouseMoveY );
	lua_register(lua, "GetDesktopWidth" , GetDesktopWidth );
	lua_register(lua, "GetDesktopHeight" , GetDesktopHeight );
	lua_register(lua, "CurveValue" , CurveValue );
	lua_register(lua, "CurveAngle" , CurveAngle );
	lua_register(lua, "PositionMouse" , PositionMouse );
	lua_register(lua, "GetDynamicCharacterControllerDidJump" , GetDynamicCharacterControllerDidJump );
	lua_register(lua, "GetCharacterControllerDucking" , GetCharacterControllerDucking );
	lua_register(lua, "WrapValue" , WrapValue );
	lua_register(lua, "GetElapsedTime" , GetElapsedTime );
	lua_register(lua, "GetPlrObjectPositionX" , GetPlrObjectPositionX );
	lua_register(lua, "GetPlrObjectPositionY" , GetPlrObjectPositionY );
	lua_register(lua, "GetPlrObjectPositionZ" , GetPlrObjectPositionZ );
	lua_register(lua, "GetPlrObjectAngleX" , GetPlrObjectAngleX );
	lua_register(lua, "GetPlrObjectAngleY" , GetPlrObjectAngleY );
	lua_register(lua, "GetPlrObjectAngleZ" , GetPlrObjectAngleZ );
	lua_register(lua, "GetGroundHeight" , GetGroundHeight );
	lua_register(lua, "NewXValue" , NewXValue );
	lua_register(lua, "NewZValue" , NewZValue );
	lua_register(lua, "ControlDynamicCharacterController" , ControlDynamicCharacterController );
	lua_register(lua, "GetCharacterHitFloor" , GetCharacterHitFloor );
	lua_register(lua, "GetCharacterFallDistance" , GetCharacterFallDistance );
	lua_register(lua, "RayTerrain" , RayTerrain );
	lua_register(lua, "GetRayCollisionX" , GetRayCollisionX );
	lua_register(lua, "GetRayCollisionY" , GetRayCollisionY );
	lua_register(lua, "GetRayCollisionZ" , GetRayCollisionZ );
	lua_register(lua, "IntersectAll" , IntersectAll );
	lua_register(lua, "IntersectStatic" , IntersectStatic );
	lua_register(lua, "GetIntersectCollisionX" , GetIntersectCollisionX );
	lua_register(lua, "GetIntersectCollisionY" , GetIntersectCollisionY );
	lua_register(lua, "GetIntersectCollisionZ" , GetIntersectCollisionZ );
	lua_register(lua, "GetIntersectCollisionNX" , GetIntersectCollisionNX );
	lua_register(lua, "GetIntersectCollisionNY" , GetIntersectCollisionNY );
	lua_register(lua, "GetIntersectCollisionNZ" , GetIntersectCollisionNZ );
	lua_register(lua, "PositionCamera" , PositionCamera );
	lua_register(lua, "PointCamera" , PointCamera );
	lua_register(lua, "MoveCamera" , MoveCamera );
	lua_register(lua, "GetObjectExist" , GetObjectExist );
	lua_register(lua, "SetObjectFrame" , SetObjectFrame );
	lua_register(lua, "GetObjectFrame" , GetObjectFrame );
	lua_register(lua, "SetObjectSpeed" , SetObjectSpeed );
	lua_register(lua, "GetObjectSpeed" , GetObjectSpeed );
	lua_register(lua, "PositionObject" , PositionObject );
	lua_register(lua, "RotateObject" , RotateObject );
	lua_register(lua, "GetObjectAngleX" , GetObjectAngleX );
	lua_register(lua, "GetObjectAngleY" , GetObjectAngleY );
	lua_register(lua, "GetObjectAngleZ" , GetObjectAngleZ );
	lua_register(lua, "GetObjectPosAng",  GetObjectPosAng );
	lua_register(lua, "GetObjectColBox",  GetObjectColBox );
	lua_register(lua, "GetObjectCentre",  GetObjectCentre );
	lua_register(lua, "GetObjectScales",  GetObjectScales );
	lua_register(lua, "ScaleObject",   ScaleObjectXYZ );

	// Physics related functions
	lua_register(lua, "PushObject",              PushObject );
	lua_register(lua, "ConstrainObjMotion",      ConstrainObjMotion );
	lua_register(lua, "ConstrainObjRotation",    ConstrainObjRotation );
	lua_register(lua, "CreateSingleHinge",       CreateSingleHinge );
	lua_register(lua, "CreateSingleJoint",       CreateSingleJoint );
	lua_register(lua, "CreateDoubleHinge",       CreateDoubleHinge );
	lua_register(lua, "CreateDoubleJoint",       CreateDoubleJoint );
	lua_register(lua, "CreateSliderDouble",      CreateSliderDouble );
	lua_register(lua, "RemoveObjectConstraints", RemoveObjectConstraints );
	lua_register(lua, "RemoveConstraint",        RemoveConstraint );
	lua_register(lua, "PhysicsRayCast",          PhysicsRayCast );
	lua_register(lua, "SetObjectDamping",        SetObjectDamping );
	lua_register(lua, "SetHingeLimits",          SetHingeLimits );
	lua_register(lua, "GetHingeAngle",           GetHingeAngle );
	lua_register(lua, "SetHingeMotor",           SetHingeMotor );
	lua_register(lua, "SetSliderMotor",          SetSliderMotor );
	lua_register(lua, "SetBodyScaling",          SetBodyScaling );
	lua_register(lua, "SetSliderLimits",         SetSliderLimits );
	lua_register(lua, "GetSliderPosition",       GetSliderPosition );

	// Collision detection functions 
	lua_register(lua, "GetObjectNumCollisions",     GetObjectNumCollisions );
	lua_register(lua, "GetObjectCollisionDetails",  GetObjectCollisionDetails );
	lua_register(lua, "GetTerrainNumCollisions",    GetTerrainNumCollisions );
	lua_register(lua, "GetTerrainCollisionDetails", GetTerrainCollisionDetails );
	lua_register(lua, "AddObjectCollisionCheck",    AddObjectCollisionCheck );
	lua_register(lua, "RemoveObjectCollisionCheck", RemoveObjectCollisionCheck );

	// quaternion library functions
	lua_register(lua, "QuatToEuler",  QuatToEuler );
	lua_register(lua, "EulerToQuat",  EulerToQuat );
	lua_register(lua, "QuatMultiply", QuatMultiply );
	lua_register(lua, "QuatSLERP",    QuatSLERP );
	lua_register(lua, "QuatLERP",     QuatLERP );

	// Lua control of dynamic light
	lua_register(lua, "GetEntityLightNumber", GetEntityLightNumber );
	lua_register(lua, "GetLightPosition",     GetLightPosition );
	lua_register(lua, "GetLightAngle",        GetLightAngle );
	lua_register(lua, "GetLightRGB",          GetLightRGB );
	lua_register(lua, "GetLightRange",        GetLightRange );
	lua_register(lua, "SetLightPosition",     SetLightPosition );
	lua_register(lua, "SetLightAngle",        SetLightAngle );
	lua_register(lua, "SetLightRGB",          SetLightRGB );
	lua_register(lua, "SetLightRange",        SetLightRange );
	
	lua_register(lua, "RunCharLoop" , RunCharLoop );
	lua_register(lua, "TriggerWaterRipple" , TriggerWaterRipple );
	lua_register(lua, "PlayFootfallSound" , PlayFootfallSound );
	lua_register(lua, "ResetUnderwaterState" , ResetUnderwaterState );
	lua_register(lua, "SetUnderwaterOn" , SetUnderwaterOn );
	lua_register(lua, "SetUnderwaterOff" , SetUnderwaterOff );

	lua_register(lua, "SetShaderVariable" , SetShaderVariable );
	
	lua_register(lua, "GetGamePlayerControlJetpackMode" , GetGamePlayerControlJetpackMode );
	lua_register(lua, "GetGamePlayerControlJetpackFuel" , GetGamePlayerControlJetpackFuel );
	lua_register(lua, "GetGamePlayerControlJetpackHidden" , GetGamePlayerControlJetpackHidden );
	lua_register(lua, "GetGamePlayerControlJetpackCollected" , GetGamePlayerControlJetpackCollected );
	lua_register(lua, "GetGamePlayerControlSoundStartIndex" , GetGamePlayerControlSoundStartIndex );
	lua_register(lua, "GetGamePlayerControlJetpackParticleEmitterIndex" , GetGamePlayerControlJetpackParticleEmitterIndex );
	lua_register(lua, "GetGamePlayerControlJetpackThrust" , GetGamePlayerControlJetpackThrust );
	lua_register(lua, "GetGamePlayerControlStartStrength" , GetGamePlayerControlStartStrength );
	lua_register(lua, "GetGamePlayerControlIsRunning" , GetGamePlayerControlIsRunning );
	lua_register(lua, "GetGamePlayerControlFinalCameraAngley" , GetGamePlayerControlFinalCameraAngley );
	lua_register(lua, "GetGamePlayerControlCx" , GetGamePlayerControlCx );
	lua_register(lua, "GetGamePlayerControlCy" , GetGamePlayerControlCy );
	lua_register(lua, "GetGamePlayerControlCz" , GetGamePlayerControlCz );
	lua_register(lua, "GetGamePlayerControlBasespeed" , GetGamePlayerControlBasespeed );
	lua_register(lua, "GetGamePlayerControlCanRun" , GetGamePlayerControlCanRun );
	lua_register(lua, "GetGamePlayerControlMaxspeed" , GetGamePlayerControlMaxspeed );
	lua_register(lua, "GetGamePlayerControlTopspeed" , GetGamePlayerControlTopspeed );
	lua_register(lua, "GetGamePlayerControlMovement" , GetGamePlayerControlMovement );
	lua_register(lua, "GetGamePlayerControlMovey" , GetGamePlayerControlMovey );
	lua_register(lua, "GetGamePlayerControlLastMovement" , GetGamePlayerControlLastMovement );
	lua_register(lua, "GetGamePlayerControlFootfallCount" , GetGamePlayerControlFootfallCount );
	lua_register(lua, "GetGamePlayerControlLastMovement" , GetGamePlayerControlLastMovement );
	lua_register(lua, "GetGamePlayerControlGravityActive" , GetGamePlayerControlGravityActive );
	lua_register(lua, "GetGamePlayerControlPlrHitFloorMaterial" , GetGamePlayerControlPlrHitFloorMaterial );
	lua_register(lua, "GetGamePlayerControlUnderwater" , GetGamePlayerControlUnderwater );
	lua_register(lua, "GetGamePlayerControlJumpMode" , GetGamePlayerControlJumpMode );
	lua_register(lua, "GetGamePlayerControlJumpModeCanAffectVelocityCountdown" , GetGamePlayerControlJumpModeCanAffectVelocityCountdown );
	lua_register(lua, "GetGamePlayerControlSpeed" , GetGamePlayerControlSpeed );
	lua_register(lua, "GetGamePlayerControlAccel" , GetGamePlayerControlAccel );
	lua_register(lua, "GetGamePlayerControlSpeedRatio" , GetGamePlayerControlSpeedRatio );
	lua_register(lua, "GetGamePlayerControlWobble" , GetGamePlayerControlWobble );
	lua_register(lua, "GetGamePlayerControlWobbleSpeed" , GetGamePlayerControlWobbleSpeed );
	lua_register(lua, "GetGamePlayerControlWobbleHeight" , GetGamePlayerControlWobbleHeight );
	lua_register(lua, "GetGamePlayerControlJumpmax" , GetGamePlayerControlJumpmax );
	lua_register(lua, "GetGamePlayerControlPushangle" , GetGamePlayerControlPushangle );
	lua_register(lua, "GetGamePlayerControlPushforce" , GetGamePlayerControlPushforce );
	lua_register(lua, "GetGamePlayerControlFootfallPace" , GetGamePlayerControlFootfallPace );
	lua_register(lua, "GetGamePlayerControlFinalCameraAngley" , GetGamePlayerControlFinalCameraAngley );
	lua_register(lua, "GetGamePlayerControlLockAtHeight" , GetGamePlayerControlLockAtHeight );
	lua_register(lua, "GetGamePlayerControlControlHeight" , GetGamePlayerControlControlHeight );
	lua_register(lua, "GetGamePlayerControlControlHeightCooldown" , GetGamePlayerControlControlHeightCooldown );
	lua_register(lua, "GetGamePlayerControlStoreMovey" , GetGamePlayerControlStoreMovey );
	lua_register(lua, "GetGamePlayerControlPlrHitFloorMaterial" , GetGamePlayerControlPlrHitFloorMaterial );
	lua_register(lua, "GetGamePlayerControlHurtFall" , GetGamePlayerControlHurtFall );
	lua_register(lua, "GetGamePlayerControlLeanoverAngle" , GetGamePlayerControlLeanoverAngle );
	lua_register(lua, "GetGamePlayerControlLeanover" , GetGamePlayerControlLeanover );
	lua_register(lua, "GetGamePlayerControlCameraShake" , GetGamePlayerControlCameraShake );
	lua_register(lua, "GetGamePlayerControlFinalCameraAnglex" , GetGamePlayerControlFinalCameraAnglex );
	lua_register(lua, "GetGamePlayerControlFinalCameraAngley" , GetGamePlayerControlFinalCameraAngley );
	lua_register(lua, "GetGamePlayerControlFinalCameraAnglez" , GetGamePlayerControlFinalCameraAnglez );
	lua_register(lua, "GetGamePlayerControlCamRightMouseMode" , GetGamePlayerControlCamRightMouseMode );
	lua_register(lua, "GetGamePlayerControlCamCollisionSmooth" , GetGamePlayerControlCamCollisionSmooth );
	lua_register(lua, "GetGamePlayerControlCamCurrentDistance" , GetGamePlayerControlCamCurrentDistance );
	lua_register(lua, "GetGamePlayerControlCamDoFullRayCheck" , GetGamePlayerControlCamDoFullRayCheck );
	lua_register(lua, "GetGamePlayerControlLastGoodcx" , GetGamePlayerControlLastGoodcx );
	lua_register(lua, "GetGamePlayerControlLastGoodcy" , GetGamePlayerControlLastGoodcy );
	lua_register(lua, "GetGamePlayerControlLastGoodcz" , GetGamePlayerControlLastGoodcz );
	lua_register(lua, "GetGamePlayerControlCamDoFullRayCheck" , GetGamePlayerControlCamDoFullRayCheck );
	lua_register(lua, "GetGamePlayerControlFlinchx" , GetGamePlayerControlFlinchx );
	lua_register(lua, "GetGamePlayerControlFlinchy" , GetGamePlayerControlFlinchy );
	lua_register(lua, "GetGamePlayerControlFlinchz" , GetGamePlayerControlFlinchz );
	lua_register(lua, "GetGamePlayerControlFlinchCurrentx" , GetGamePlayerControlFlinchCurrentx );
	lua_register(lua, "GetGamePlayerControlFlinchCurrenty" , GetGamePlayerControlFlinchCurrenty );
	lua_register(lua, "GetGamePlayerControlFlinchCurrentz" , GetGamePlayerControlFlinchCurrentz );
	lua_register(lua, "GetGamePlayerControlFootfallType" , GetGamePlayerControlFootfallType );
	lua_register(lua, "GetGamePlayerControlRippleCount" , GetGamePlayerControlRippleCount );
	lua_register(lua, "GetGamePlayerControlLastFootfallSound" , GetGamePlayerControlLastFootfallSound );
	lua_register(lua, "GetGamePlayerControlInWaterState" , GetGamePlayerControlInWaterState );
	lua_register(lua, "GetGamePlayerControlDrownTimestamp" , GetGamePlayerControlDrownTimestamp );
	lua_register(lua, "GetGamePlayerControlDeadTime" , GetGamePlayerControlDeadTime );
	lua_register(lua, "GetGamePlayerControlSwimTimestamp" , GetGamePlayerControlSwimTimestamp );
	lua_register(lua, "GetGamePlayerControlRedDeathFog" , GetGamePlayerControlRedDeathFog );
	lua_register(lua, "GetGamePlayerControlHeartbeatTimeStamp" , GetGamePlayerControlHeartbeatTimeStamp );
	lua_register(lua, "GetGamePlayerControlThirdpersonEnabled" , GetGamePlayerControlThirdpersonEnabled );
	lua_register(lua, "GetGamePlayerControlThirdpersonCharacterIndex" , GetGamePlayerControlThirdpersonCharacterIndex );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraFollow" , GetGamePlayerControlThirdpersonCameraFollow );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraFocus" , GetGamePlayerControlThirdpersonCameraFocus );
	lua_register(lua, "GetGamePlayerControlThirdpersonCharactere" , GetGamePlayerControlThirdpersonCharactere );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraFollow" , GetGamePlayerControlThirdpersonCameraFollow );
	lua_register(lua, "GetGamePlayerControlThirdpersonShotFired" , GetGamePlayerControlThirdpersonShotFired );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraDistance" , GetGamePlayerControlThirdpersonCameraDistance );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraSpeed" , GetGamePlayerControlThirdpersonCameraSpeed );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraLocked" , GetGamePlayerControlThirdpersonCameraLocked );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraHeight" , GetGamePlayerControlThirdpersonCameraHeight );
	lua_register(lua, "GetGamePlayerControlThirdpersonCameraShoulder" , GetGamePlayerControlThirdpersonCameraShoulder );
	lua_register(lua, "SetGamePlayerControlJetpackMode" , SetGamePlayerControlJetpackMode );
	lua_register(lua, "SetGamePlayerControlJetpackFuel" , SetGamePlayerControlJetpackFuel );
	lua_register(lua, "SetGamePlayerControlJetpackHidden" , SetGamePlayerControlJetpackHidden );
	lua_register(lua, "SetGamePlayerControlJetpackCollected" , SetGamePlayerControlJetpackCollected );
	lua_register(lua, "SetGamePlayerControlSoundStartIndex" , SetGamePlayerControlSoundStartIndex );
	lua_register(lua, "SetGamePlayerControlJetpackParticleEmitterIndex" , SetGamePlayerControlJetpackParticleEmitterIndex );
	lua_register(lua, "SetGamePlayerControlJetpackThrust" , SetGamePlayerControlJetpackThrust );
	lua_register(lua, "SetGamePlayerControlStartStrength" , SetGamePlayerControlStartStrength );
	lua_register(lua, "SetGamePlayerControlIsRunning" , SetGamePlayerControlIsRunning );
	lua_register(lua, "SetGamePlayerControlFinalCameraAngley" , SetGamePlayerControlFinalCameraAngley );
	lua_register(lua, "SetGamePlayerControlCx" , SetGamePlayerControlCx );
	lua_register(lua, "SetGamePlayerControlCy" , SetGamePlayerControlCy );
	lua_register(lua, "SetGamePlayerControlCz" , SetGamePlayerControlCz );
	lua_register(lua, "SetGamePlayerControlBasespeed" , SetGamePlayerControlBasespeed );
	lua_register(lua, "SetGamePlayerControlCanRun" , SetGamePlayerControlCanRun );
	lua_register(lua, "SetGamePlayerControlMaxspeed" , SetGamePlayerControlMaxspeed );
	lua_register(lua, "SetGamePlayerControlTopspeed" , SetGamePlayerControlTopspeed );
	lua_register(lua, "SetGamePlayerControlMovement" , SetGamePlayerControlMovement );
	lua_register(lua, "SetGamePlayerControlMovey" , SetGamePlayerControlMovey );
	lua_register(lua, "SetGamePlayerControlLastMovement" , SetGamePlayerControlLastMovement );
	lua_register(lua, "SetGamePlayerControlFootfallCount" , SetGamePlayerControlFootfallCount );
	lua_register(lua, "SetGamePlayerControlLastMovement" , SetGamePlayerControlLastMovement );
	lua_register(lua, "SetGamePlayerControlGravityActive" , SetGamePlayerControlGravityActive );
	lua_register(lua, "SetGamePlayerControlPlrHitFloorMaterial" , SetGamePlayerControlPlrHitFloorMaterial );
	lua_register(lua, "SetGamePlayerControlUnderwater" , SetGamePlayerControlUnderwater );
	lua_register(lua, "SetGamePlayerControlJumpMode" , SetGamePlayerControlJumpMode );
	lua_register(lua, "SetGamePlayerControlJumpModeCanAffectVelocityCountdown" , SetGamePlayerControlJumpModeCanAffectVelocityCountdown );
	lua_register(lua, "SetGamePlayerControlSpeed" , SetGamePlayerControlSpeed );
	lua_register(lua, "SetGamePlayerControlAccel" , SetGamePlayerControlAccel );
	lua_register(lua, "SetGamePlayerControlSpeedRatio" , SetGamePlayerControlSpeedRatio );
	lua_register(lua, "SetGamePlayerControlWobble" , SetGamePlayerControlWobble );
	lua_register(lua, "SetGamePlayerControlWobbleSpeed" , SetGamePlayerControlWobbleSpeed );
	lua_register(lua, "SetGamePlayerControlWobbleHeight" , SetGamePlayerControlWobbleHeight );
	lua_register(lua, "SetGamePlayerControlJumpmax" , SetGamePlayerControlJumpmax );
	lua_register(lua, "SetGamePlayerControlPushangle" , SetGamePlayerControlPushangle );
	lua_register(lua, "SetGamePlayerControlPushforce" , SetGamePlayerControlPushforce );
	lua_register(lua, "SetGamePlayerControlFootfallPace" , SetGamePlayerControlFootfallPace );
	lua_register(lua, "SetGamePlayerControlFinalCameraAngley" , SetGamePlayerControlFinalCameraAngley );
	lua_register(lua, "SetGamePlayerControlLockAtHeight" , SetGamePlayerControlLockAtHeight );
	lua_register(lua, "SetGamePlayerControlControlHeight" , SetGamePlayerControlControlHeight );
	lua_register(lua, "SetGamePlayerControlControlHeightCooldown" , SetGamePlayerControlControlHeightCooldown );
	lua_register(lua, "SetGamePlayerControlStoreMovey" , SetGamePlayerControlStoreMovey );
	lua_register(lua, "SetGamePlayerControlPlrHitFloorMaterial" , SetGamePlayerControlPlrHitFloorMaterial );
	lua_register(lua, "SetGamePlayerControlHurtFall" , SetGamePlayerControlHurtFall );
	lua_register(lua, "SetGamePlayerControlLeanoverAngle" , SetGamePlayerControlLeanoverAngle );
	lua_register(lua, "SetGamePlayerControlLeanover" , SetGamePlayerControlLeanover );
	lua_register(lua, "SetGamePlayerControlCameraShake" , SetGamePlayerControlCameraShake );
	lua_register(lua, "SetGamePlayerControlFinalCameraAnglex" , SetGamePlayerControlFinalCameraAnglex );
	lua_register(lua, "SetGamePlayerControlFinalCameraAngley" , SetGamePlayerControlFinalCameraAngley );
	lua_register(lua, "SetGamePlayerControlFinalCameraAnglez" , SetGamePlayerControlFinalCameraAnglez );
	lua_register(lua, "SetGamePlayerControlCamRightMouseMode" , SetGamePlayerControlCamRightMouseMode );
	lua_register(lua, "SetGamePlayerControlCamCollisionSmooth" , SetGamePlayerControlCamCollisionSmooth );
	lua_register(lua, "SetGamePlayerControlCamCurrentDistance" , SetGamePlayerControlCamCurrentDistance );
	lua_register(lua, "SetGamePlayerControlCamDoFullRayCheck" , SetGamePlayerControlCamDoFullRayCheck );
	lua_register(lua, "SetGamePlayerControlLastGoodcx" , SetGamePlayerControlLastGoodcx );
	lua_register(lua, "SetGamePlayerControlLastGoodcy" , SetGamePlayerControlLastGoodcy );
	lua_register(lua, "SetGamePlayerControlLastGoodcz" , SetGamePlayerControlLastGoodcz );
	lua_register(lua, "SetGamePlayerControlCamDoFullRayCheck" , SetGamePlayerControlCamDoFullRayCheck );
	lua_register(lua, "SetGamePlayerControlFlinchx" , SetGamePlayerControlFlinchx );
	lua_register(lua, "SetGamePlayerControlFlinchy" , SetGamePlayerControlFlinchy );
	lua_register(lua, "SetGamePlayerControlFlinchz" , SetGamePlayerControlFlinchz );
	lua_register(lua, "SetGamePlayerControlFlinchCurrentx" , SetGamePlayerControlFlinchCurrentx );
	lua_register(lua, "SetGamePlayerControlFlinchCurrenty" , SetGamePlayerControlFlinchCurrenty );
	lua_register(lua, "SetGamePlayerControlFlinchCurrentz" , SetGamePlayerControlFlinchCurrentz );
	lua_register(lua, "SetGamePlayerControlFootfallType" , SetGamePlayerControlFootfallType );
	lua_register(lua, "SetGamePlayerControlRippleCount" , SetGamePlayerControlRippleCount );
	lua_register(lua, "SetGamePlayerControlLastFootfallSound" , SetGamePlayerControlLastFootfallSound );
	lua_register(lua, "SetGamePlayerControlInWaterState" , SetGamePlayerControlInWaterState );
	lua_register(lua, "SetGamePlayerControlDrownTimestamp" , SetGamePlayerControlDrownTimestamp );
	lua_register(lua, "SetGamePlayerControlDeadTime" , SetGamePlayerControlDeadTime );
	lua_register(lua, "SetGamePlayerControlSwimTimestamp" , SetGamePlayerControlSwimTimestamp );
	lua_register(lua, "SetGamePlayerControlRedDeathFog" , SetGamePlayerControlRedDeathFog );
	lua_register(lua, "SetGamePlayerControlHeartbeatTimeStamp" , SetGamePlayerControlHeartbeatTimeStamp );
	lua_register(lua, "SetGamePlayerControlThirdpersonEnabled" , SetGamePlayerControlThirdpersonEnabled );
	lua_register(lua, "SetGamePlayerControlThirdpersonCharacterIndex" , SetGamePlayerControlThirdpersonCharacterIndex );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraFollow" , SetGamePlayerControlThirdpersonCameraFollow );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraFocus" , SetGamePlayerControlThirdpersonCameraFocus );
	lua_register(lua, "SetGamePlayerControlThirdpersonCharactere" , SetGamePlayerControlThirdpersonCharactere );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraFollow" , SetGamePlayerControlThirdpersonCameraFollow );
	lua_register(lua, "SetGamePlayerControlThirdpersonShotFired" , SetGamePlayerControlThirdpersonShotFired );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraDistance" , SetGamePlayerControlThirdpersonCameraDistance );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraSpeed" , SetGamePlayerControlThirdpersonCameraSpeed );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraLocked" , SetGamePlayerControlThirdpersonCameraLocked );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraHeight" , SetGamePlayerControlThirdpersonCameraHeight );
	lua_register(lua, "SetGamePlayerControlThirdpersonCameraShoulder" , SetGamePlayerControlThirdpersonCameraShoulder );

	lua_register(lua, "SetGamePlayerStateGunMode" , SetGamePlayerStateGunMode );
	lua_register(lua, "GetGamePlayerStateGunMode" , GetGamePlayerStateGunMode );
	lua_register(lua, "SetGamePlayerStateFiringMode" , SetGamePlayerStateFiringMode );
	lua_register(lua, "GetGamePlayerStateFiringMode" , GetGamePlayerStateFiringMode );
	lua_register(lua, "GetGamePlayerStateWeaponAmmoIndex" , GetGamePlayerStateWeaponAmmoIndex );
	lua_register(lua, "GetGamePlayerStateAmmoOffset" , GetGamePlayerStateAmmoOffset );
	lua_register(lua, "GetGamePlayerStateGunMeleeKey" , GetGamePlayerStateGunMeleeKey );
	lua_register(lua, "GetGamePlayerStateBlockingAction" , GetGamePlayerStateBlockingAction );
	lua_register(lua, "SetGamePlayerStateGunShootNoAmmo" , SetGamePlayerStateGunShootNoAmmo );
	lua_register(lua, "GetGamePlayerStateGunShootNoAmmo" , GetGamePlayerStateGunShootNoAmmo );
	lua_register(lua, "SetGamePlayerStateUnderwater" , SetGamePlayerStateUnderwater );
	lua_register(lua, "GetGamePlayerStateUnderwater" , GetGamePlayerStateUnderwater );
	lua_register(lua, "SetGamePlayerStateRightMouseHold" , SetGamePlayerStateRightMouseHold );
	lua_register(lua, "GetGamePlayerStateRightMouseHold" , GetGamePlayerStateRightMouseHold );
	lua_register(lua, "SetGamePlayerStateXBOX" , SetGamePlayerStateXBOX );
	lua_register(lua, "GetGamePlayerStateXBOX" , GetGamePlayerStateXBOX );
	lua_register(lua, "SetGamePlayerStateXBOXControllerType" , SetGamePlayerStateXBOXControllerType );
	lua_register(lua, "GetGamePlayerStateXBOXControllerType" , GetGamePlayerStateXBOXControllerType );
	lua_register(lua, "JoystickX" , JoystickY );
	lua_register(lua, "JoystickY" , JoystickX );
	lua_register(lua, "JoystickZ" , JoystickZ );
	lua_register(lua, "SetGamePlayerStateGunZoomMode" , SetGamePlayerStateGunZoomMode );
	lua_register(lua, "GetGamePlayerStateGunZoomMode" , GetGamePlayerStateGunZoomMode );
	lua_register(lua, "SetGamePlayerStateGunZoomMag" , SetGamePlayerStateGunZoomMag );
	lua_register(lua, "GetGamePlayerStateGunZoomMag" , GetGamePlayerStateGunZoomMag );
	lua_register(lua, "SetGamePlayerStateGunReloadNoAmmo" , SetGamePlayerStateGunReloadNoAmmo );
	lua_register(lua, "GetGamePlayerStateGunReloadNoAmmo" , GetGamePlayerStateGunReloadNoAmmo );
	lua_register(lua, "SetGamePlayerStatePlrReloading" , SetGamePlayerStatePlrReloading );
	lua_register(lua, "GetGamePlayerStatePlrReloading" , GetGamePlayerStatePlrReloading );
	lua_register(lua, "SetGamePlayerStateGunAltSwapKey1" , SetGamePlayerStateGunAltSwapKey1 );
	lua_register(lua, "GetGamePlayerStateGunAltSwapKey1" , GetGamePlayerStateGunAltSwapKey1 );
	lua_register(lua, "SetGamePlayerStateGunAltSwapKey2" , SetGamePlayerStateGunAltSwapKey2 );
	lua_register(lua, "GetGamePlayerStateGunAltSwapKey2" , GetGamePlayerStateGunAltSwapKey2 );
	lua_register(lua, "SetGamePlayerStateWeaponKeySelection" , SetGamePlayerStateWeaponKeySelection );
	lua_register(lua, "GetGamePlayerStateWeaponKeySelection" , GetGamePlayerStateWeaponKeySelection );
	lua_register(lua, "SetGamePlayerStateWeaponIndex" , SetGamePlayerStateWeaponIndex );
	lua_register(lua, "GetGamePlayerStateWeaponIndex" , GetGamePlayerStateWeaponIndex );
	lua_register(lua, "SetGamePlayerStateCommandNewWeapon" , SetGamePlayerStateCommandNewWeapon );
	lua_register(lua, "GetGamePlayerStateCommandNewWeapon" , GetGamePlayerStateCommandNewWeapon );
	lua_register(lua, "SetGamePlayerStateGunID" , SetGamePlayerStateGunID );
	lua_register(lua, "GetGamePlayerStateGunID" , GetGamePlayerStateGunID );
	lua_register(lua, "SetGamePlayerStateGunSelectionAfterHide" , SetGamePlayerStateGunSelectionAfterHide );
	lua_register(lua, "GetGamePlayerStateGunSelectionAfterHide" , GetGamePlayerStateGunSelectionAfterHide );
	lua_register(lua, "SetGamePlayerStateGunBurst" , SetGamePlayerStateGunBurst );
	lua_register(lua, "GetGamePlayerStateGunBurst" , GetGamePlayerStateGunBurst );
	lua_register(lua, "JoystickHatAngle" , JoystickHatAngle );
	lua_register(lua, "JoystickFireXL" , JoystickFireXL );
	lua_register(lua, "JoystickTwistX" , JoystickTwistX );
	lua_register(lua, "JoystickTwistY" , JoystickTwistY );
	lua_register(lua, "JoystickTwistZ" , JoystickTwistZ );
	lua_register(lua, "SetGamePlayerStatePlrZoomInChange" , SetGamePlayerStatePlrZoomInChange );
	lua_register(lua, "GetGamePlayerStatePlrZoomInChange" , GetGamePlayerStatePlrZoomInChange );
	lua_register(lua, "SetGamePlayerStatePlrZoomIn" , SetGamePlayerStatePlrZoomIn );
	lua_register(lua, "GetGamePlayerStatePlrZoomIn" , GetGamePlayerStatePlrZoomIn );
	lua_register(lua, "SetGamePlayerStateLuaActiveMouse" , SetGamePlayerStateLuaActiveMouse );
	lua_register(lua, "GetGamePlayerStateLuaActiveMouse" , GetGamePlayerStateLuaActiveMouse );
	lua_register(lua, "SetGamePlayerStateRealFov" , SetGamePlayerStateRealFov );
	lua_register(lua, "GetGamePlayerStateRealFov" , GetGamePlayerStateRealFov );
	lua_register(lua, "SetGamePlayerStateDisablePeeking" , SetGamePlayerStateDisablePeeking );
	lua_register(lua, "GetGamePlayerStateDisablePeeking" , GetGamePlayerStateDisablePeeking );
	lua_register(lua, "SetGamePlayerStatePlrHasFocus" , SetGamePlayerStatePlrHasFocus );
	lua_register(lua, "GetGamePlayerStatePlrHasFocus" , GetGamePlayerStatePlrHasFocus );
	lua_register(lua, "SetGamePlayerStateGameRunAsMultiplayer" , SetGamePlayerStateGameRunAsMultiplayer );
	lua_register(lua, "GetGamePlayerStateGameRunAsMultiplayer" , GetGamePlayerStateGameRunAsMultiplayer );
	lua_register(lua, "SetGamePlayerStateSteamWorksRespawnLeft" , SetGamePlayerStateSteamWorksRespawnLeft );
	lua_register(lua, "GetGamePlayerStateSteamWorksRespawnLeft" , GetGamePlayerStateSteamWorksRespawnLeft );
	lua_register(lua, "SetGamePlayerStateMPRespawnLeft" , SetGamePlayerStateMPRespawnLeft );
	lua_register(lua, "GetGamePlayerStateMPRespawnLeft" , GetGamePlayerStateMPRespawnLeft );
	lua_register(lua, "SetGamePlayerStateTabMode" , SetGamePlayerStateTabMode );
	lua_register(lua, "GetGamePlayerStateTabMode" , GetGamePlayerStateTabMode );
	lua_register(lua, "SetGamePlayerStateLowFpsWarning" , SetGamePlayerStateLowFpsWarning );
	lua_register(lua, "GetGamePlayerStateLowFpsWarning" , GetGamePlayerStateLowFpsWarning );
	lua_register(lua, "SetGamePlayerStateCameraFov" , SetGamePlayerStateCameraFov );
	lua_register(lua, "GetGamePlayerStateCameraFov" , GetGamePlayerStateCameraFov );
	lua_register(lua, "SetGamePlayerStateCameraFovZoomed" , SetGamePlayerStateCameraFovZoomed );
	lua_register(lua, "GetGamePlayerStateCameraFovZoomed" , GetGamePlayerStateCameraFovZoomed );
	lua_register(lua, "SetGamePlayerStateMouseInvert" , SetGamePlayerStateMouseInvert );
	lua_register(lua, "GetGamePlayerStateMouseInvert" , GetGamePlayerStateMouseInvert );

	lua_register(lua, "SetGamePlayerStateSlowMotion" , SetGamePlayerStateSlowMotion );
	lua_register(lua, "GetGamePlayerStateSlowMotion" , GetGamePlayerStateSlowMotion );
	lua_register(lua, "SetGamePlayerStateSmoothCameraKeys" , SetGamePlayerStateSmoothCameraKeys );
	lua_register(lua, "GetGamePlayerStateSmoothCameraKeys" , GetGamePlayerStateSmoothCameraKeys );
	lua_register(lua, "SetGamePlayerStateCamMouseMoveX" , SetGamePlayerStateCamMouseMoveX );
	lua_register(lua, "GetGamePlayerStateCamMouseMoveX" , GetGamePlayerStateCamMouseMoveX );
	lua_register(lua, "SetGamePlayerStateCamMouseMoveY" , SetGamePlayerStateCamMouseMoveY );
	lua_register(lua, "GetGamePlayerStateCamMouseMoveY" , GetGamePlayerStateCamMouseMoveY );
	lua_register(lua, "SetGamePlayerStateGunRecoilX" , SetGamePlayerStateGunRecoilX );
	lua_register(lua, "GetGamePlayerStateGunRecoilX" , GetGamePlayerStateGunRecoilX );
	lua_register(lua, "SetGamePlayerStateGunRecoilY" , SetGamePlayerStateGunRecoilY );
	lua_register(lua, "GetGamePlayerStateGunRecoilY" , GetGamePlayerStateGunRecoilY );
	lua_register(lua, "SetGamePlayerStateGunRecoilAngleX" , SetGamePlayerStateGunRecoilAngleX );
	lua_register(lua, "GetGamePlayerStateGunRecoilAngleX" , GetGamePlayerStateGunRecoilAngleX );
	lua_register(lua, "SetGamePlayerStateGunRecoilAngleY" , SetGamePlayerStateGunRecoilAngleY );
	lua_register(lua, "GetGamePlayerStateGunRecoilAngleY" , GetGamePlayerStateGunRecoilAngleY );
	lua_register(lua, "SetGamePlayerStateGunRecoilCorrectY" , SetGamePlayerStateGunRecoilCorrectY );
	lua_register(lua, "GetGamePlayerStateGunRecoilCorrectY" , GetGamePlayerStateGunRecoilCorrectY );
	lua_register(lua, "SetGamePlayerStateGunRecoilCorrectX" , SetGamePlayerStateGunRecoilCorrectX );
	lua_register(lua, "GetGamePlayerStateGunRecoilCorrectX" , GetGamePlayerStateGunRecoilCorrectX );
	lua_register(lua, "SetGamePlayerStateGunRecoilCorrectAngleY" , SetGamePlayerStateGunRecoilCorrectAngleY );
	lua_register(lua, "GetGamePlayerStateGunRecoilCorrectAngleY" , GetGamePlayerStateGunRecoilCorrectAngleY );
	lua_register(lua, "SetGamePlayerStateGunRecoilCorrectAngleX" , SetGamePlayerStateGunRecoilCorrectAngleX );
	lua_register(lua, "GetGamePlayerStateGunRecoilCorrectAngleX" , GetGamePlayerStateGunRecoilCorrectAngleX );
	lua_register(lua, "SetGamePlayerStateCamAngleX" , SetGamePlayerStateCamAngleX );
	lua_register(lua, "GetGamePlayerStateCamAngleX" , GetGamePlayerStateCamAngleX );
	lua_register(lua, "SetGamePlayerStateCamAngleY" , SetGamePlayerStateCamAngleY );
	lua_register(lua, "GetGamePlayerStateCamAngleY" , GetGamePlayerStateCamAngleY );
	lua_register(lua, "SetGamePlayerStatePlayerDucking" , SetGamePlayerStatePlayerDucking );
	lua_register(lua, "GetGamePlayerStatePlayerDucking" , GetGamePlayerStatePlayerDucking );
	lua_register(lua, "SetGamePlayerStateEditModeActive" , SetGamePlayerStateEditModeActive );
	lua_register(lua, "GetGamePlayerStateEditModeActive" , GetGamePlayerStateEditModeActive );
	lua_register(lua, "SetGamePlayerStatePlrKeyShift" , SetGamePlayerStatePlrKeyShift );
	lua_register(lua, "GetGamePlayerStatePlrKeyShift" , GetGamePlayerStatePlrKeyShift );
	lua_register(lua, "SetGamePlayerStatePlrKeyShift2" , SetGamePlayerStatePlrKeyShift2 );
	lua_register(lua, "GetGamePlayerStatePlrKeyShift2" , GetGamePlayerStatePlrKeyShift2 );
	lua_register(lua, "SetGamePlayerStatePlrKeyControl" , SetGamePlayerStatePlrKeyControl );
	lua_register(lua, "GetGamePlayerStatePlrKeyControl" , GetGamePlayerStatePlrKeyControl );
	lua_register(lua, "SetGamePlayerStateNoWater" , SetGamePlayerStateNoWater );
	lua_register(lua, "GetGamePlayerStateNoWater" , GetGamePlayerStateNoWater );
	lua_register(lua, "SetGamePlayerStateWaterlineY" , SetGamePlayerStateWaterlineY );
	lua_register(lua, "GetGamePlayerStateWaterlineY" , GetGamePlayerStateWaterlineY );
	lua_register(lua, "SetGamePlayerStateFlashlightKeyEnabled" , SetGamePlayerStateFlashlightKeyEnabled );
	lua_register(lua, "GetGamePlayerStateFlashlightKeyEnabled" , GetGamePlayerStateFlashlightKeyEnabled );
	lua_register(lua, "SetGamePlayerStateFlashlightControl" , SetGamePlayerStateFlashlightControl );
	lua_register(lua, "GetGamePlayerStateFlashlightControl" , GetGamePlayerStateFlashlightControl );
	lua_register(lua, "SetGamePlayerStateMoving" , SetGamePlayerStateMoving );
	lua_register(lua, "GetGamePlayerStateMoving" , GetGamePlayerStateMoving );
	lua_register(lua, "SetGamePlayerStateTerrainHeight" , SetGamePlayerStateTerrainHeight );
	lua_register(lua, "GetGamePlayerStateTerrainHeight" , GetGamePlayerStateTerrainHeight );
	lua_register(lua, "SetGamePlayerStateJetpackVerticalMove" , SetGamePlayerStateJetpackVerticalMove );
	lua_register(lua, "GetGamePlayerStateJetpackVerticalMove" , GetGamePlayerStateJetpackVerticalMove );
	lua_register(lua, "SetGamePlayerStateTerrainID" , SetGamePlayerStateTerrainID );
	lua_register(lua, "GetGamePlayerStateTerrainID" , GetGamePlayerStateTerrainID );
	lua_register(lua, "SetGamePlayerStateEnablePlrSpeedMods" , SetGamePlayerStateEnablePlrSpeedMods );
	lua_register(lua, "GetGamePlayerStateEnablePlrSpeedMods" , GetGamePlayerStateEnablePlrSpeedMods );
	lua_register(lua, "SetGamePlayerStateRiftMode" , SetGamePlayerStateRiftMode );
	lua_register(lua, "GetGamePlayerStateRiftMode" , GetGamePlayerStateRiftMode );
	lua_register(lua, "SetGamePlayerStatePlayerX" , SetGamePlayerStatePlayerX );
	lua_register(lua, "GetGamePlayerStatePlayerX" , GetGamePlayerStatePlayerX );
	lua_register(lua, "SetGamePlayerStatePlayerY" , SetGamePlayerStatePlayerY );
	lua_register(lua, "GetGamePlayerStatePlayerY" , GetGamePlayerStatePlayerY );
	lua_register(lua, "SetGamePlayerStatePlayerZ" , SetGamePlayerStatePlayerZ );
	lua_register(lua, "GetGamePlayerStatePlayerZ" , GetGamePlayerStatePlayerZ );
	lua_register(lua, "SetGamePlayerStateTerrainPlayerX" , SetGamePlayerStateTerrainPlayerX );
	lua_register(lua, "GetGamePlayerStateTerrainPlayerX" , GetGamePlayerStateTerrainPlayerX );
	lua_register(lua, "SetGamePlayerStateTerrainPlayerY" , SetGamePlayerStateTerrainPlayerY );
	lua_register(lua, "GetGamePlayerStateTerrainPlayerY" , GetGamePlayerStateTerrainPlayerY );
	lua_register(lua, "SetGamePlayerStateTerrainPlayerZ" , SetGamePlayerStateTerrainPlayerZ );
	lua_register(lua, "GetGamePlayerStateTerrainPlayerZ" , GetGamePlayerStateTerrainPlayerZ );
	lua_register(lua, "SetGamePlayerStateTerrainPlayerAX" , SetGamePlayerStateTerrainPlayerAX );
	lua_register(lua, "GetGamePlayerStateTerrainPlayerAX" , GetGamePlayerStateTerrainPlayerAX );
	lua_register(lua, "SetGamePlayerStateTerrainPlayerAY" , SetGamePlayerStateTerrainPlayerAY );
	lua_register(lua, "GetGamePlayerStateTerrainPlayerAY" , GetGamePlayerStateTerrainPlayerAY );
	lua_register(lua, "SetGamePlayerStateTerrainPlayerAZ" , SetGamePlayerStateTerrainPlayerAZ );
	lua_register(lua, "GetGamePlayerStateTerrainPlayerAZ" , GetGamePlayerStateTerrainPlayerAZ );
	lua_register(lua, "SetGamePlayerStateAdjustBasedOnWobbleY" , SetGamePlayerStateAdjustBasedOnWobbleY );
	lua_register(lua, "GetGamePlayerStateAdjustBasedOnWobbleY" , GetGamePlayerStateAdjustBasedOnWobbleY );
	lua_register(lua, "SetGamePlayerStateFinalCamX" , SetGamePlayerStateFinalCamX );
	lua_register(lua, "GetGamePlayerStateFinalCamX" , GetGamePlayerStateFinalCamX );
	lua_register(lua, "SetGamePlayerStateFinalCamY" , SetGamePlayerStateFinalCamY );
	lua_register(lua, "GetGamePlayerStateFinalCamY" , GetGamePlayerStateFinalCamY );
	lua_register(lua, "SetGamePlayerStateFinalCamZ" , SetGamePlayerStateFinalCamZ );
	lua_register(lua, "GetGamePlayerStateFinalCamZ" , GetGamePlayerStateFinalCamZ );
	lua_register(lua, "SetGamePlayerStateShakeX" , SetGamePlayerStateShakeX );
	lua_register(lua, "GetGamePlayerStateShakeX" , GetGamePlayerStateShakeX );
	lua_register(lua, "SetGamePlayerStateShakeY" , SetGamePlayerStateShakeY );
	lua_register(lua, "GetGamePlayerStateShakeY" , GetGamePlayerStateShakeY );
	lua_register(lua, "SetGamePlayerStateShakeZ" , SetGamePlayerStateShakeZ );
	lua_register(lua, "GetGamePlayerStateShakeZ" , GetGamePlayerStateShakeZ );
	lua_register(lua, "SetGamePlayerStateImmunity" , SetGamePlayerStateImmunity );
	lua_register(lua, "GetGamePlayerStateImmunity" , GetGamePlayerStateImmunity );
	lua_register(lua, "SetGamePlayerStateCharAnimIndex" , SetGamePlayerStateCharAnimIndex );
	lua_register(lua, "GetGamePlayerStateCharAnimIndex" , GetGamePlayerStateCharAnimIndex );

	lua_register(lua, "GetGamePlayerStateMotionController" , GetGamePlayerStateMotionController );
	lua_register(lua, "GetGamePlayerStateMotionControllerType" , GetGamePlayerStateMotionControllerType );
	lua_register(lua, "MotionControllerThumbstickX" , MotionControllerThumbstickX );
	lua_register(lua, "MotionControllerThumbstickY" , MotionControllerThumbstickY );
	lua_register(lua, "CombatControllerTrigger" , CombatControllerTrigger );
	lua_register(lua, "CombatControllerGrip" , CombatControllerGrip );
	lua_register(lua, "CombatControllerThumbstickX" , CombatControllerThumbstickX );
	lua_register(lua, "CombatControllerThumbstickY" , CombatControllerThumbstickY );
	lua_register(lua, "MotionControllerBestX" , MotionControllerBestX );
	lua_register(lua, "MotionControllerBestY" , MotionControllerBestY );
	lua_register(lua, "MotionControllerBestZ" , MotionControllerBestZ );
	lua_register(lua, "MotionControllerBestAngleX" , MotionControllerBestAngleX );
	lua_register(lua, "MotionControllerBestAngleY" , MotionControllerBestAngleY );
	lua_register(lua, "MotionControllerBestAngleZ" , MotionControllerBestAngleZ );
	lua_register(lua, "MotionControllerLaserGuidedEntityObj" , MotionControllerLaserGuidedEntityObj );
	
	lua_register(lua, "SetGamePlayerStateIsMelee" , SetGamePlayerStateIsMelee );
	lua_register(lua, "GetGamePlayerStateIsMelee" , GetGamePlayerStateIsMelee );
	lua_register(lua, "SetGamePlayerStateAlternate" , SetGamePlayerStateAlternate );
	lua_register(lua, "GetGamePlayerStateAlternate" , GetGamePlayerStateAlternate );
	lua_register(lua, "SetGamePlayerStateModeShareMags" , SetGamePlayerStateModeShareMags );
	lua_register(lua, "GetGamePlayerStateModeShareMags" , GetGamePlayerStateModeShareMags );
	lua_register(lua, "SetGamePlayerStateAlternateIsFlak" , SetGamePlayerStateAlternateIsFlak );
	lua_register(lua, "GetGamePlayerStateAlternateIsFlak" , GetGamePlayerStateAlternateIsFlak );
	lua_register(lua, "SetGamePlayerStateAlternateIsRay" , SetGamePlayerStateAlternateIsRay );
	lua_register(lua, "GetGamePlayerStateAlternateIsRay" , GetGamePlayerStateAlternateIsRay );
	lua_register(lua, "SetFireModeSettingsReloadQty" , SetFireModeSettingsReloadQty );
	lua_register(lua, "GetFireModeSettingsReloadQty" , GetFireModeSettingsReloadQty );
	lua_register(lua, "SetFireModeSettingsIsEmpty" , SetFireModeSettingsIsEmpty );
	lua_register(lua, "GetFireModeSettingsIsEmpty" , GetFireModeSettingsIsEmpty );
	lua_register(lua, "SetFireModeSettingsJammed" , SetFireModeSettingsJammed );
	lua_register(lua, "GetFireModeSettingsJammed" , GetFireModeSettingsJammed );
	lua_register(lua, "SetFireModeSettingsJamChance" , SetFireModeSettingsJamChance );
	lua_register(lua, "GetFireModeSettingsJamChance" , GetFireModeSettingsJamChance );
	lua_register(lua, "SetFireModeSettingsMinTimer" , SetFireModeSettingsMinTimer );
	lua_register(lua, "GetFireModeSettingsMinTimer" , GetFireModeSettingsMinTimer );
	lua_register(lua, "SetFireModeSettingsAddTimer" , SetFireModeSettingsAddTimer );
	lua_register(lua, "GetFireModeSettingsAddTimer" , GetFireModeSettingsAddTimer );
	lua_register(lua, "SetFireModeSettingsShotsFired" , SetFireModeSettingsShotsFired );
	lua_register(lua, "GetFireModeSettingsShotsFired" , GetFireModeSettingsShotsFired );
	lua_register(lua, "SetFireModeSettingsCoolTimer" , SetFireModeSettingsCoolTimer );
	lua_register(lua, "GetFireModeSettingsCoolTimer" , GetFireModeSettingsCoolTimer );
	lua_register(lua, "SetFireModeSettingsOverheatAfter" , SetFireModeSettingsOverheatAfter );
	lua_register(lua, "GetFireModeSettingsOverheatAfter" , GetFireModeSettingsOverheatAfter );
	lua_register(lua, "SetFireModeSettingsJamChanceTime" , SetFireModeSettingsJamChanceTime );
	lua_register(lua, "GetFireModeSettingsJamChanceTime" , GetFireModeSettingsJamChanceTime );
	lua_register(lua, "SetFireModeSettingsCoolDown" , SetFireModeSettingsCoolDown );
	lua_register(lua, "GetFireModeSettingsCoolDown" , GetFireModeSettingsCoolDown );
	lua_register(lua, "SetFireModeSettingsNoSubmergedFire" , SetFireModeSettingsNoSubmergedFire );
	lua_register(lua, "GetFireModeSettingsNoSubmergedFire" , GetFireModeSettingsNoSubmergedFire );
	lua_register(lua, "SetFireModeSettingsSimpleZoom" , SetFireModeSettingsSimpleZoom );
	lua_register(lua, "GetFireModeSettingsSimpleZoom" , GetFireModeSettingsSimpleZoom );
	lua_register(lua, "SetFireModeSettingsForceZoomOut" , SetFireModeSettingsForceZoomOut );
	lua_register(lua, "GetFireModeSettingsForceZoomOut" , GetFireModeSettingsForceZoomOut );
	lua_register(lua, "SetFireModeSettingsZoomMode" , SetFireModeSettingsZoomMode );
	lua_register(lua, "GetFireModeSettingsZoomMode" , GetFireModeSettingsZoomMode );
	lua_register(lua, "SetFireModeSettingsSimpleZoomAnim" , SetFireModeSettingsSimpleZoomAnim );
	lua_register(lua, "GetFireModeSettingsSimpleZoomAnim" , GetFireModeSettingsSimpleZoomAnim );
	lua_register(lua, "SetFireModeSettingsPoolIndex" , SetFireModeSettingsPoolIndex );
	lua_register(lua, "GetFireModeSettingsPoolIndex" , GetFireModeSettingsPoolIndex );
	lua_register(lua, "SetFireModeSettingsPlrTurnSpeedMod" , SetFireModeSettingsPlrTurnSpeedMod );
	lua_register(lua, "GetFireModeSettingsPlrTurnSpeedMod" , GetFireModeSettingsPlrTurnSpeedMod );
	lua_register(lua, "SetFireModeSettingsZoomTurnSpeed" , SetFireModeSettingsZoomTurnSpeed );
	lua_register(lua, "GetFireModeSettingsZoomTurnSpeed" , GetFireModeSettingsZoomTurnSpeed );
	lua_register(lua, "SetFireModeSettingsPlrJumpSpeedMod" , SetFireModeSettingsPlrJumpSpeedMod );
	lua_register(lua, "GetFireModeSettingsPlrJumpSpeedMod" , GetFireModeSettingsPlrJumpSpeedMod );
	lua_register(lua, "SetFireModeSettingsPlrEmptySpeedMod" , SetFireModeSettingsPlrEmptySpeedMod );
	lua_register(lua, "GetFireModeSettingsPlrEmptySpeedMod" , GetFireModeSettingsPlrEmptySpeedMod );
	lua_register(lua, "SetFireModeSettingsPlrMoveSpeedMod" , SetFireModeSettingsPlrMoveSpeedMod );
	lua_register(lua, "GetFireModeSettingsPlrMoveSpeedMod" , GetFireModeSettingsPlrMoveSpeedMod );
	lua_register(lua, "SetFireModeSettingsZoomWalkSpeed" , SetFireModeSettingsZoomWalkSpeed );
	lua_register(lua, "GetFireModeSettingsZoomWalkSpeed" , GetFireModeSettingsZoomWalkSpeed );
	lua_register(lua, "SetFireModeSettingsPlrReloadSpeedMod" , SetFireModeSettingsPlrReloadSpeedMod );
	lua_register(lua, "GetFireModeSettingsPlrReloadSpeedMod" , GetFireModeSettingsPlrReloadSpeedMod );
	lua_register(lua, "SetFireModeSettingsHasEmpty" , SetFireModeSettingsHasEmpty );
	lua_register(lua, "GetFireModeSettingsHasEmpty" , GetFireModeSettingsHasEmpty );
	lua_register(lua, "GetFireModeSettingsActionBlockStart" , GetFireModeSettingsActionBlockStart );
	
	lua_register(lua, "SetGamePlayerStateGunSound" , SetGamePlayerStateGunSound );
	lua_register(lua, "GetGamePlayerStateGunSound" , GetGamePlayerStateGunSound );
	lua_register(lua, "SetGamePlayerStateGunAltSound" , SetGamePlayerStateGunAltSound );
	lua_register(lua, "GetGamePlayerStateGunAltSound" , GetGamePlayerStateGunAltSound );

	lua_register(lua, "CopyCharAnimState" , CopyCharAnimState );
	lua_register(lua, "SetCharAnimStatePlayCsi" , SetCharAnimStatePlayCsi );
	lua_register(lua, "GetCharAnimStatePlayCsi" , GetCharAnimStatePlayCsi );
	lua_register(lua, "SetCharAnimStateOriginalE" , SetCharAnimStateOriginalE );
	lua_register(lua, "GetCharAnimStateOriginalE" , GetCharAnimStateOriginalE );
	lua_register(lua, "SetCharAnimStateObj" , SetCharAnimStateObj );
	lua_register(lua, "GetCharAnimStateObj" , GetCharAnimStateObj );
	lua_register(lua, "SetCharAnimStateAnimationSpeed" , SetCharAnimStateAnimationSpeed );
	lua_register(lua, "GetCharAnimStateAnimationSpeed" , GetCharAnimStateAnimationSpeed );
	lua_register(lua, "SetCharAnimStateE" , SetCharAnimStateE );
	lua_register(lua, "GetCharAnimStateE" , GetCharAnimStateE );

	lua_register(lua, "GetCsiStoodVault" , GetCsiStoodVault );
	lua_register(lua, "GetCharSeqTrigger" , GetCharSeqTrigger );
	lua_register(lua, "GetEntityElementBankIndex" , GetEntityElementBankIndex );
	lua_register(lua, "GetEntityElementObj" , GetEntityElementObj );
	lua_register(lua, "GetEntityElementRagdollified" , GetEntityElementRagdollified );
	lua_register(lua, "GetEntityElementSpeedModulator" , GetEntityElementSpeedModulator );
	lua_register(lua, "GetEntityProfileJumpModifier" , GetEntityProfileJumpModifier );
	lua_register(lua, "GetEntityProfileStartOfAIAnim" , GetEntityProfileStartOfAIAnim );
	lua_register(lua, "GetEntityProfileJumpHold" , GetEntityProfileJumpHold );
	lua_register(lua, "GetEntityProfileJumpResume" , GetEntityProfileJumpResume );

	lua_register(lua, "SetCharAnimControlsLeaping" , SetCharAnimControlsLeaping );
	lua_register(lua, "GetCharAnimControlsLeaping" , GetCharAnimControlsLeaping );
	lua_register(lua, "SetCharAnimControlsMoving" , SetCharAnimControlsMoving );
	lua_register(lua, "GetCharAnimControlsMoving" , GetCharAnimControlsMoving );
	
	lua_register(lua, "GetEntityAnimStart" , GetEntityAnimStart );
	lua_register(lua, "GetEntityAnimFinish" , GetEntityAnimFinish );

	lua_register(lua, "SetRotationYSlowly" , SetRotationYSlowly );

	lua_register(lua, "ParticlesGetFreeEmitter" , ParticlesGetFreeEmitter );
	lua_register(lua, "ParticlesAddEmitter" ,     ParticlesAddEmitter );
	lua_register(lua, "ParticlesAddEmitterEx" ,   ParticlesAddEmitterEx );
	lua_register(lua, "ParticlesDeleteEmitter" ,  ParticlesDeleteEmitter );
	lua_register(lua, "ParticlesSpawnParticle",   ParticlesSpawnParticle );
	lua_register(lua, "ParticlesLoadImage",       ParticlesLoadImage );
	lua_register(lua, "ParticlesLoadEffect",      ParticlesLoadEffect );
	lua_register(lua, "ParticlesSetFrames",       ParticlesSetFrames );
	lua_register(lua, "ParticlesSetSpeed",        ParticlesSetSpeed );
	lua_register(lua, "ParticlesSetGravity",      ParticlesSetGravity );
	lua_register(lua, "ParticlesSetOffset",       ParticlesSetOffset );
	lua_register(lua, "ParticlesSetAngle",        ParticlesSetAngle );
	lua_register(lua, "ParticlesSetRotation",     ParticlesSetRotation );
	lua_register(lua, "ParticlesSetScale",        ParticlesSetScale );
	lua_register(lua, "ParticlesSetAlpha",        ParticlesSetAlpha );
	lua_register(lua, "ParticlesSetLife",         ParticlesSetLife );
	lua_register(lua, "ParticlesSetWindVector",   ParticlesSetWindVector );
	lua_register(lua, "ParticlesSetNoWind",       ParticlesSetNoWind );

	lua_register(lua, "GetBulletHit",             GetBulletHit);

	lua_register(lua, "SetFlashLight" , SetFlashLight );	
	lua_register(lua, "SetAttachmentVisible" , SetAttachmentVisible );
	lua_register(lua, "SetOcclusion" , SetOcclusion );
	lua_register(lua, "SetPlayerWeapons", SetPlayerWeapons);
	lua_register(lua, "FirePlayerWeapon", FirePlayerWeapon);
	lua_register(lua, "SetFlashLightKeyEnabled" , SetFlashLightKeyEnabled );
	lua_register(lua, "SetPlayerRun" , SetPlayerRun );
	lua_register(lua, "SetFont" , SetFont );
	lua_register(lua, "GetDeviceWidth" , GetDeviceWidth );
	lua_register(lua, "GetDeviceHeight" , GetDeviceHeight );
	lua_register(lua, "GetFirstEntitySpawn" , GetFirstEntitySpawn );
	lua_register(lua, "GetNextEntitySpawn" , GetFirstEntitySpawn );	

	// VR and head tracking
	lua_register(lua, "GetHeadTracker" , GetHeadTracker );	
	lua_register(lua, "ResetHeadTracker" , ResetHeadTracker );	
	lua_register(lua, "GetHeadTrackerYaw" , GetHeadTrackerYaw );	
	lua_register(lua, "GetHeadTrackerPitch" , GetHeadTrackerPitch );	
	lua_register(lua, "GetHeadTrackerRoll" , GetHeadTrackerRoll );	
	lua_register(lua, "GetHeadTrackerNormalX" , GetHeadTrackerNormalX );	
	lua_register(lua, "GetHeadTrackerNormalY" , GetHeadTrackerNormalY );	
	lua_register(lua, "GetHeadTrackerNormalZ" , GetHeadTrackerNormalZ );		

	// Prompt3D
	lua_register(lua, "Prompt3D" , Prompt3D );	
	lua_register(lua, "PositionPrompt3D" , PositionPrompt3D );	
	
	// utility
	lua_register(lua, "MsgBox" , MsgBox );
	lua_register(lua, "IsTestGame", GetIsTestgame);

	//Water Shader
	//setter
	lua_register(lua, "SetWaterHeight", SetWaterHeight);
	lua_register(lua, "SetWaterColor", SetWaterShaderColor);
	lua_register(lua, "SetWaterWaveIntensity", SetWaterWaveIntensity);
	lua_register(lua, "SetWaterTransparancy", SetWaterTransparancy);
	lua_register(lua, "SetWaterReflection", SetWaterReflection);
	lua_register(lua, "SetWaterReflectionSparkleIntensity", SetWaterReflectionSparkleIntensity);
	lua_register(lua, "SetWaterFlowDirection", SetWaterFlowDirection);
	lua_register(lua, "SetWaterDistortionWaves", SetWaterDistortionWaves);
	lua_register(lua, "SetRippleWaterSpeed", SetRippleWaterSpeed);
	//getter
	lua_register(lua, "GetWaterHeight", GetWaterHeight);
	lua_register(lua, "GetWaterWaveIntensity", GetWaterWaveIntensity);
	lua_register(lua, "GetWaterShaderColorRed", GetWaterShaderColorRed);
	lua_register(lua, "GetWaterShaderColorGreen", GetWaterShaderColorGreen);
	lua_register(lua, "GetWaterShaderColorBlue", GetWaterShaderColorBlue);
	lua_register(lua, "GetWaterTransparancy", GetWaterTransparancy);
	lua_register(lua, "GetWaterReflection", GetWaterReflection);
	lua_register(lua, "GetWaterReflectionSparkleIntensity", GetWaterReflectionSparkleIntensity);
	lua_register(lua, "GetWaterFlowDirectionX", GetWaterFlowDirectionX);
	lua_register(lua, "GetWaterFlowDirectionY", GetWaterFlowDirectionY);
	lua_register(lua, "GetWaterFlowSpeed", GetWaterFlowSpeed);
	lua_register(lua, "GetWaterDistortionWaves", GetWaterDistortionWaves);
	lua_register(lua, "GetRippleWaterSpeed", GetRippleWaterSpeed);

	//Dynamic sun.
	lua_register(lua, "SetSunDirection", SetSunDirection);

}

 /*
 struct luaMessage
{
	char msgDesc[256];
	int msgInt;
	float msgFloat;
	char msgString[256];
};

luaMessage currentMessage;

int luaMessageCount = 0;
int maxLuaMessages = 0;
luaMessage** ppLuaMessages = NULL;
 */

char szLuaReturnString[1024];

 DARKLUA_API LPSTR LuaMessageDesc ( void )
 {
  	// Return string pointer	
	const char *s = currentMessage.msgDesc;

	// If input string valid
	if(s)
	{
		strcpy(szLuaReturnString, s);
	}
	else
	{
		strcpy(szLuaReturnString, "");
	}

	return GetReturnStringFromTEXTWorkString( szLuaReturnString );
 }

 DARKLUA_API int LuaMessageIndex ()
 {
	return currentMessage.msgIndex;
 }

 DARKLUA_API float LuaMessageFloat ()
 {
	return currentMessage.msgFloat;
 }

  DARKLUA_API int LuaMessageInt ()
 {
	return currentMessage.msgInt;
 }

  char szLuaMessageString[1024];

 DARKLUA_API LPSTR LuaMessageString ( void )
 {
  	// Return string pointer
	LPSTR pReturnString=NULL;
	const char *s = currentMessage.msgString;

	// If input string valid
	if(s)
	{
		strcpy(szLuaMessageString, s);
	}
	else
	{
		strcpy(szLuaMessageString, "");
	}

	return GetReturnStringFromTEXTWorkString( szLuaMessageString );
 }

 DARKLUA_API int LuaNext()
 {

	if ( luaMessageCount == 0 ) 
	{
		strcpy ( currentMessage.msgDesc, "" );
		currentMessage.msgFloat = 0.0f;
		currentMessage.msgInt = 0;
		currentMessage.msgIndex = 0;
		strcpy ( currentMessage.msgString, "" );

		return 0;
	}
	else
	{
		strcpy ( currentMessage.msgDesc, ppLuaMessages[0]->msgDesc );
		currentMessage.msgFloat = ppLuaMessages[0]->msgFloat;
		currentMessage.msgInt = ppLuaMessages[0]->msgInt;
		currentMessage.msgIndex = ppLuaMessages[0]->msgIndex;
		strcpy ( currentMessage.msgString, ppLuaMessages[0]->msgString );

		delete ppLuaMessages[0];
		ppLuaMessages[0] = NULL;

		for ( int c = 1 ; c < luaMessageCount ; c++ )
			ppLuaMessages[c-1] = ppLuaMessages[c];

		ppLuaMessages[luaMessageCount-1] = NULL;

		luaMessageCount--;
	}

	return 1;

 }

 DARKLUA_API void SetLuaState ( int id )
 {
	 defaultState = id;
 }

 bool checkScriptAlreadyLoaded ( int id , LPSTR pString )
 {

	 for ( int c = 0 ; c < ScriptsLoaded.size() ; c++ )
	 {
		 if ( strcmp ( pString , ScriptsLoaded[c].fileName ) == 0 )
		 {
			 if ( id == ScriptsLoaded[c].stateID )
			 {
				return true;
			 }
		 }
	 }

	
	 StringList tempStringList;
	 strcpy ( tempStringList.fileName , pString );
	 tempStringList.stateID = id;

	 ScriptsLoaded.push_back(tempStringList);

	 return false;

 }

 DARKLUA_API int LoadLua( LPSTR pString , int id )
 {

	 if ( checkScriptAlreadyLoaded ( id , pString ) ) return 0;

	if ( id <= 0 )
	{
		//MessageBox(NULL, "invalid Lua ID, must be 1 or above", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( maxLuaStates == 0 )
	{
		maxLuaStates = 100;
		ppLuaStates = new luaState*[maxLuaStates+1];

		for ( int c = 0 ; c < maxLuaStates+1 ; c++ )
			ppLuaStates[c] = NULL;
	}

	if ( id > maxLuaStates )
	{
		luaState** ppBigger = NULL;
		ppBigger = new luaState*[maxLuaStates+101];

		for ( int c = 0; c < maxLuaStates+1; c++ )
		 ppBigger [ c ] = ppLuaStates [ c ];

		delete [ ] ppLuaStates;

		ppLuaStates = ppBigger;

		for ( int c = maxLuaStates+1; c < maxLuaStates+101; c++ )
			ppLuaStates[c] = NULL;

		maxLuaStates += 100;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua State ID already in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		//return 0;

		// create new Lua state   
		lua = luaL_newstate();

		// load Lua libraries
		static const luaL_Reg lualibs[] =
		{
			{ "base", luaopen_base },
			{ NULL, NULL}
		};

		addFunctions();

		/*const luaL_Reg *lib = lualibs;
		for(; lib->func != NULL; lib++)
		{
			lib->func(lua);
			lua_settop(lua, 0);
		}*/

		luaL_openlibs(lua);
	}

    // run the Lua script
	int result = 0;

	char VirtualFilename[MAX_PATH];
	strcpy ( VirtualFilename , pString );
	LuaCheckForWorkshopFile ( VirtualFilename );

	result =  luaL_dofile(lua, VirtualFilename);

	if (result == 1 )
	{
		while(ShowCursor(TRUE) <= 0);
		SetCursorPos ( g_dwScreenWidth / 2 , g_dwScreenHeight / 2 );
		MessageBox( g_pGlob->hWnd , lua_tostring(lua, -1), "LUA ERROR!" , MB_OK | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND );
		lua_pop(lua, 1);
	}
	else
	{
		if ( ppLuaStates[id] == NULL )
			ppLuaStates[id] = new luaState;

		ppLuaStates[id]->state = lua;
	}

	return result;
 }

 DARKLUA_API int LoadLua( LPSTR pString )
 {
	int id = defaultState;


	if ( checkScriptAlreadyLoaded ( id , pString ) ) return 0;

	if ( id <= 0 )
	{
		//MessageBox(NULL, "invalid Lua ID, must be 1 or above", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( maxLuaStates == 0 )
	{
		maxLuaStates = 100;
		ppLuaStates = new luaState*[maxLuaStates+1];

		for ( int c = 0 ; c < maxLuaStates+1 ; c++ )
			ppLuaStates[c] = NULL;
	}

	if ( id > maxLuaStates )
	{
		luaState** ppBigger = NULL;
		ppBigger = new luaState*[maxLuaStates+101];

		for ( int c = 0; c < maxLuaStates+1; c++ )
		 ppBigger [ c ] = ppLuaStates [ c ];

		delete [ ] ppLuaStates;

		ppLuaStates = ppBigger;

		for ( int c = maxLuaStates+1; c < maxLuaStates+101; c++ )
			ppLuaStates[c] = NULL;

		maxLuaStates += 100;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua State ID already in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		//return 0;

		// create new Lua state   
		lua = luaL_newstate();

		// load Lua libraries
		static const luaL_Reg lualibs[] =
		{
			{ "base", luaopen_base },
			{ NULL, NULL}
		};

		addFunctions();

		/*const luaL_Reg *lib = lualibs;
		for(; lib->func != NULL; lib++)
		{
			lib->func(lua);
			lua_settop(lua, 0);
		}*/

		luaL_openlibs(lua);

	}

	/*lua_getglobal(lua, "package");
	lua_getfield(lua, -1, "path"); // get field "path" from table at top of stack (-1)
	std::string cur_path = lua_tostring(lua, -1); // grab path string from top of stack
	cur_path.append (";"); // do your path magic here
	cur_path.append("F:/TGCSHARED/fpsc-reloaded/FPS Creator Files/Files/?");
	lua_pop(lua, 1); // get rid of the string on the stack we just pushed on line 5
	lua_pushstring(lua, cur_path.c_str()); // push the new one
	lua_setfield(lua, -2, "path"); // set the field "path" in table at -2 with value at top of stack
	lua_pop(lua, 1); // get rid of package table from top of stack*/

    // run the Lua script
	int result = 0;

	char VirtualFilename[MAX_PATH];
	strcpy ( VirtualFilename , pString );
	LuaCheckForWorkshopFile ( VirtualFilename );

	result = luaL_dofile(lua, VirtualFilename);

	if (result == 1 )
	{
		while(ShowCursor(TRUE) <= 0);
		SetCursorPos ( g_dwScreenWidth / 2 , g_dwScreenHeight / 2 );
		MessageBox( g_pGlob->hWnd , lua_tostring(lua, -1), "LUA ERROR!" , MB_OK | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND );
		//MessageBox(NULL, lua_tostring(lua, -1), "LUA ERROR", MB_TOPMOST | MB_OK);
		lua_pop(lua, 1);
	}
	else
	{
		if ( ppLuaStates[id] == NULL )
			ppLuaStates[id] = new luaState;

		ppLuaStates[id]->state = lua;
	}

	return result;
 }

DARKLUA_API void LuaSetFunction( LPSTR pString , int id, int params, int results )
{

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

	lua = ppLuaStates[id]->state;
	
	char* pLastOccurance =  strrchr ( pString , '\\' );
	if ( pLastOccurance )
		strcpy ( functionName, pLastOccurance+1 );
	else
		strcpy ( functionName , pString );
	functionParams = params;
	functionResults = results;
	functionStateID = id;

	// the function name 
	lua_getglobal(lua, functionName );
}

DARKLUA_API void LuaSetFunction( LPSTR pString , int params, int results )
{
	int id = defaultState;

#ifdef LUA_DO_DEBUG
	WriteToDebugLog ( "-->LuaSetFunction" , true );
	WriteToDebugLog ( "ID" , id );
	WriteToDebugLog ( "pString" , pString );
	WriteToDebugLog ( "params" , params );
	WriteToDebugLog ( "results" , results );
	WriteToDebugLog ( "===========" , true );
#endif

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

	lua = ppLuaStates[id]->state;

	char* pLastOccurance =  strrchr ( pString , '\\' );
	if ( pLastOccurance )
		strcpy ( functionName, pLastOccurance+1 );
	else
		strcpy ( functionName , pString );
	functionParams = params;
	functionResults = results;
	functionStateID = id;

	// the function name 
	lua_getglobal(lua, functionName );
}

DARKLUA_API int LuaValidateEntityTable ( int iEntityIndex )
{
	int iValid = 0;
	int id = defaultState;
	if ( id > maxLuaStates+1 ) return 0;
	if ( ppLuaStates==NULL ) return 0;
	if ( ppLuaStates[id] == NULL ) return 0;
	lua = ppLuaStates[id]->state;
	int stacktopindex = lua_gettop (lua);
	lua_getglobal(lua, "g_Entity");
	lua_pushnumber(lua, iEntityIndex); 
	lua_gettable(lua, -2); // g_Entity[e] 
	lua_pushstring(lua, "x");
	if ( lua_istable(lua,-2) )
	{
		lua_gettable(lua, -2);  // g_Entity[e]["x"]
		if ( lua_isnumber ( lua, -1 ) || lua_isstring ( lua, -1 ) )
		{
			// table exists and the element within the table also exists as a number/string so its valid
			iValid = 1;
		}
	}
	lua_settop (lua, stacktopindex);
	return iValid;
}


DARKLUA_API void LuaCall()
{
	for ( int c = 0 ; c < FunctionsWithErrors.size() ; c++ )
	{
		if ( strcmp ( functionName , FunctionsWithErrors[c].fileName ) == 0 )
		{
			lua_pop(lua,functionParams+1);
			return;
		}
	}

	int id = functionStateID;

	int failedResults = 0;

	if ( id > maxLuaStates+1 )
	{

		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		lua_pop(lua,functionParams+1);

		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{

		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		lua_pop(lua,functionParams+1);

		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

#ifdef LUA_DO_DEBUG
	WriteToDebugLog ( "-->LuaCall" , true );
	WriteToDebugLog ( "ID" , functionStateID );
	WriteToDebugLog ( "functionParams" , functionParams );
	WriteToDebugLog ( "functionResults" , functionResults );
	WriteToDebugLog ( "===========" , true );
#endif

	lua = ppLuaStates[id]->state;

	//if ( functionParams == 1 )
	//	int dave = 1;

	// call the function with x arguments, return y results
	if ( lua_isfunction(lua, -(1+functionParams)) == 1 )
	{
		//lua_call(lua, functionParams, functionResults);
      if (lua_pcall(lua, functionParams, functionResults, 0) != 0)
	  {
		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		char s[256];
		sprintf ( s , "error running function: %s", lua_tostring(lua, -1));
		//MessageBox(NULL, s, "LUA ERROR", MB_TOPMOST | MB_OK);
		while(ShowCursor(TRUE) <= 0);
		SetCursorPos ( g_dwScreenWidth / 2 , g_dwScreenHeight / 2 );
		MessageBox( g_pGlob->hWnd , lua_tostring(lua, -1), "LUA ERROR" , MB_OK | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND );
		lua_pop(lua, 1);
		failedResults = 1;
	  }
	}
	else
	{

		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		// remove params from the stack
		lua_pop(lua,functionParams);
		failedResults = 1;

		char s[256];
		//sprintf ( s , "No function called %s" , functionName );
		//MessageBox(NULL, s, "LUA ERROR", MB_TOPMOST | MB_OK);
	}

	functionStateID = 0;
	if ( failedResults > 0 )
		lua_pop(lua, failedResults);
}

DARKLUA_API void LuaCallSilent()
{
	for ( int c = 0 ; c < FunctionsWithErrors.size() ; c++ )
	{
		if ( strcmp ( functionName , FunctionsWithErrors[c].fileName ) == 0 )
		{
			lua_pop(lua,functionParams+1);
			return;
		}
	}

	int id = functionStateID;

	int failedResults = 0;

	if ( id > maxLuaStates+1 )
	{

		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		lua_pop(lua,functionParams+1);

		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{

		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		lua_pop(lua,functionParams+1);

		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return ;
	}

#ifdef LUA_DO_DEBUG
	WriteToDebugLog ( "-->LuaCallSilent" , true );
	WriteToDebugLog ( "ID" , id );
	WriteToDebugLog ( "functionParams" , functionParams );
	WriteToDebugLog ( "functionResults" , functionResults );
	WriteToDebugLog ( "===========" , true );
#endif

	lua = ppLuaStates[id]->state;

	// call the function with x arguments, return y results
	if ( lua_isfunction(lua, -(1+functionParams)) == 1 )
	{
		//lua_call(lua, functionParams, functionResults);
      if (lua_pcall(lua, functionParams, functionResults, 0) != 0)
	  {
		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		char s[256];
		//sprintf ( s , "error running function: %s", lua_tostring(lua, -1));
		//MessageBox(NULL, s, "LUA ERROR", MB_TOPMOST | MB_OK);
		failedResults = 1;
	  }
	}
	else
	{

		//add to error list
		StringList item;
		strcpy ( item.fileName , functionName );
		FunctionsWithErrors.push_back(item);

		// remove params from the stack
		lua_pop(lua,functionParams);
		failedResults = 1;
	}

	functionStateID = 0;
	if ( failedResults > 0 )
		lua_pop(lua, failedResults);

}

 DARKLUA_API void CloseLua( int id )
 {
	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates[id] != NULL )
	{
		// close the Lua state
		lua_close(ppLuaStates[id]->state);
		delete ppLuaStates[id];
		ppLuaStates[id] = NULL;
	}
	//else
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);

 }

  DARKLUA_API void CloseLua()
 {
	int id = defaultState;
	
	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates[id] != NULL )
	{
		// close the Lua state
		lua_close(ppLuaStates[id]->state);
		delete ppLuaStates[id];
		ppLuaStates[id] = NULL;
	}
	//else
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);

 }

  DARKLUA_API void CloseLuaSilent( int id )
 {
	if ( id > maxLuaStates+1 )
	{
		return;
	}

	if ( ppLuaStates[id] != NULL )
	{
		// close the Lua state
		lua_close(ppLuaStates[id]->state);
		delete ppLuaStates[id];
		ppLuaStates[id] = NULL;
	}

 }

 DARKLUA_API void LuaReset ()
 {
	// Close all states, silently
	if ( ppLuaStates )
	{
		for ( int c = 0 ; c < maxLuaStates+1; c++ )
		{
			if ( ppLuaStates[c] != NULL )
			{
				CloseLuaSilent (c);
				ppLuaStates[c] = NULL;
			}
		}
		delete[] ppLuaStates;
		ppLuaStates = NULL;
		maxLuaStates = 0;
	}

	// Empty Message Queue
	if ( ppLuaMessages )
	{
		for ( int c = 0 ; c < maxLuaMessages; c++ )
		{
			if ( ppLuaMessages[c] != NULL )
			{
				delete ppLuaMessages[c];
				ppLuaMessages[c] = NULL;
			}
		}
	}

	// reset messaging
	strcpy ( currentMessage.msgDesc, "" );
	currentMessage.msgFloat = 0.0f;
	currentMessage.msgInt = 0;
	currentMessage.msgIndex = 0;
	strcpy ( currentMessage.msgString, "" );

	//Reset already loaded list
	ScriptsLoaded.clear();

	//Reset error list
	FunctionsWithErrors.clear();

	// 050416 - delete any sprites created inside LUA scripting
	for ( int c = g.LUASpriteoffset ; c <= g.LUASpriteoffsetMax ; c++ )
		if ( SpriteExist ( c ) == 1 )
			DeleteSprite ( c );

	// restore state default
	defaultState = 1;
 }

 DARKLUA_API int LuaExecute ( LPSTR pString , int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	lua = ppLuaStates[id]->state;

    // run the Lua script string
	int a = 0;

	a = luaL_loadbuffer(lua, pString, strlen(pString), pString) ||	lua_pcall(lua, 0, 0, 0);
	if (a) 
	{
	  //MessageBox(NULL, lua_tostring(lua, -1), "LUA ERROR", MB_TOPMOST | MB_OK);
	  lua_pop(lua, 1);  /* pop error message from the stack */
	}

	// Return 1 for success, like dbpro styles
	if ( a == 0 )
		a = 1;
	else
		a = 0;

	return a;
 }

 DARKLUA_API int LuaExecute ( LPSTR pString )
 {
	if ( ppLuaStates == NULL ) return 1;

	int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	lua = ppLuaStates[id]->state;

    // run the Lua script string
	int a = 0;

	a = luaL_loadbuffer(lua, pString, strlen(pString), pString) ||	lua_pcall(lua, 0, 0, 0);
	if (a) 
	{
	  //MessageBox(NULL, lua_tostring(lua, -1), "LUA ERROR", MB_TOPMOST | MB_OK);
	  lua_pop(lua, 1);  /* pop error message from the stack */
	}

	// Return 1 for success, like dbpro styles
	if ( a == 0 )
		a = 1;
	else
		a = 0;

	return a;
 }

 DARKLUA_API int LuaGetInt ( LPSTR pString , int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	lua = ppLuaStates[id]->state;
    lua_getglobal(lua, pString);

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	int ret = (int)lua_tonumber(lua, -1);
	lua_pop(lua,1);

    return ret;
 }

  DARKLUA_API int LuaGetInt ( LPSTR pString )
 {
	 int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	lua = ppLuaStates[id]->state;
    lua_getglobal(lua, pString);

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	int ret = (int)lua_tonumber(lua, -1);
	lua_pop(lua,1);

    return ret;
 }

  DARKLUA_API int LuaReturnInt ( int id )
 {
	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( lua_gettop(lua) == 0 )
	{
		return 0;
	}

	lua = ppLuaStates[id]->state;

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	int ret = (int)lua_tonumber(lua, -1);
	lua_pop(lua,1);

    return ret;
 }

 DARKLUA_API int LuaReturnInt ( void )
 {
	int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( lua_gettop(lua) == 0 )
	{
		return 0;
	}

	lua = ppLuaStates[id]->state;

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	int ret = (int)lua_tonumber(lua, -1);
	lua_pop(lua,1);

    return ret;
 }

 DARKLUA_API float LuaGetFloat ( LPSTR pString , int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	lua = ppLuaStates[id]->state;
    lua_getglobal(lua, pString);

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	float fValue = (float)lua_tonumber(lua, -1);
	lua_pop(lua,1);
	return fValue;
 }

 DARKLUA_API float LuaGetFloat ( LPSTR pString )
 {
	 int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	lua = ppLuaStates[id]->state;
    lua_getglobal(lua, pString);

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	float fValue = (float)lua_tonumber(lua, -1);
	lua_pop(lua,1);
	return fValue;
 }

 DARKLUA_API float LuaReturnFloat ( int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( lua_gettop(lua) == 0 )
	{
		return 0;
	}

	lua = ppLuaStates[id]->state;

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	float fValue = (float)lua_tonumber(lua, -1);
	lua_pop(lua,1);
	return fValue;
 }

 DARKLUA_API float LuaReturnFloat ( void )
 {
	 int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return 0;
	}

	if ( lua_gettop(lua) == 0 )
	{
		return 0;
	}

	lua = ppLuaStates[id]->state;

	/*if (!lua_isnumber(L, -1))
    {
        MessageBox(NULL, "Variable is not a number", "LUA ERROR", MB_TOPMOST | MB_OK);
        return 0;
    }*/

	float fValue = (float)lua_tonumber(lua, -1);
	lua_pop(lua,1);
	return fValue;
 }

 DARKLUA_API void LuaSetInt ( LPSTR pString , int value, int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );
	lua_setglobal( lua, pString );

 }

  DARKLUA_API void LuaSetInt ( LPSTR pString , int value )
 {
	 int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );
	lua_setglobal( lua, pString );

 }

 DARKLUA_API void LuaPushInt ( int value, int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );

 }

 DARKLUA_API void LuaPushInt ( int value )
 {
	 int id = defaultState;

#ifdef LUA_DO_DEBUG
	WriteToDebugLog ( "-->LuaPushInt" , true );
	WriteToDebugLog ( "ID" , id );
	WriteToDebugLog ( "value" , value );
	WriteToDebugLog ( "===========" , true );
#endif

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );

 }

 DARKLUA_API void LuaSetFloat ( LPSTR pString , float value, int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );
	lua_setglobal( lua, pString );
 }

 DARKLUA_API void LuaSetFloat ( LPSTR pString , float value )
 {
	int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );
	lua_setglobal( lua, pString );
 }

 DARKLUA_API void LuaPushFloat ( float value, int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );

 }

  DARKLUA_API void LuaPushFloat ( float value )
 {
	 int id = defaultState;

#ifdef LUA_DO_DEBUG
	WriteToDebugLog ( "-->LuaPushFloat" , true );
	WriteToDebugLog ( "ID" , id );
	WriteToDebugLog ( "value" , value );
	WriteToDebugLog ( "===========" , true );
#endif

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", MB_TOPMOST | MB_OK);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushnumber( lua, (lua_Number)value );

 }
	
 DARKLUA_API void LuaSetString ( LPSTR pString , LPSTR pStringValue, int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushstring( lua, pStringValue );
	lua_setglobal( lua, pString );
 }

 DARKLUA_API void LuaSetString ( LPSTR pString , LPSTR pStringValue )
 {
	int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushstring( lua, pStringValue );
	lua_setglobal( lua, pString );
 }


 DARKLUA_API void LuaPushString ( LPSTR pStringValue, int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushstring( lua, pStringValue );

 }

 DARKLUA_API void LuaPushString ( LPSTR pStringValue )
 {
	 int id = defaultState;

#ifdef LUA_DO_DEBUG
	WriteToDebugLog ( "-->LuaPushString" , true );
	WriteToDebugLog ( "ID" , id );
	WriteToDebugLog ( "pStringValue" , pStringValue );
	WriteToDebugLog ( "===========" , true );
#endif

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	if ( ppLuaStates==NULL ) return;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return;
	}

	lua = ppLuaStates[id]->state;
	lua_pushstring( lua, pStringValue );

 }

 DARKLUA_API int LuaArrayInt ( LPSTR pString , int id )
 {
	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	if ( ppLuaStates==NULL ) return 0;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	lua = ppLuaStates[id]->state;

	//
	char str[512];
	strcpy ( str , pString);
	char * pch;
	pch = strstr (str,"->");
	while (pch)
	{
		if (pch) strncpy (pch,"..",2);
		pch = strstr (str,"->");
	}
	//
	//char str[512];
	//strcpy ( str , pString);
	//char * pch;
	//MessageBox(NULL, str, "str =", NULL);
	bool foundFunction = false;	
	pch = NULL;
	int offset = -1;
	pch = strtok (str,".");
	char lastString[256];
	int lastNumber;
	int lastWas= 0;
	while (pch != NULL)
	{
		if (!foundFunction)
		{
			foundFunction = true;
			
			//sprintf ( errorString , "Array = %s" , pch );
			lua_getglobal(lua, pch );
			strcpy ( lastString, pch );
			//MessageBox(NULL, errorString , "" , NULL);
			if ( !lua_istable(lua, offset) )
			{
				lua_pop(lua,-offset);
				sprintf ( errorString , "%s is not a Lua Table" , pch );
				MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
				return 0;
			}
		}
		else
		{
			if (lastWas == 1)
			{

				lua_getfield ( lua , offset , lastString );

				if ( !lua_istable(lua, offset) )
				{
					lua_pop(lua,-offset);
					sprintf ( errorString , "previous field to %s does not exist in table" , pch );
					MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
					return 0;
				}
			}
			else if ( lastWas == 2 )
			{
				lua_rawgeti ( lua, offset , lastNumber );
			}

			lastWas = 1;

			if ( pch[0] == '#' )
			{
				char szString[256];
				strcpy ( szString , pch );
				memcpy ( &szString [ 0 ], &szString [ 1 ], strlen ( szString ) - 1 );
				szString [ strlen ( szString ) - 1 ] = 0;

				int num = atoi(szString);
				lastNumber = num;
				lastWas = 2;
			}
			else
				strcpy ( lastString, pch );
			
		}

		pch = strtok (NULL, ".");
	}

	if ( lastWas != 2 )
	{
		lua_pushstring(lua, lastString );
		offset--;
		lua_gettable(lua, offset);
	}
	else
	{
		lua_rawgeti ( lua, offset , lastNumber );
	}

	int ret = (int)lua_tonumber(lua, -1);
	lua_pop(lua,-(++offset));
	return ret;

 }

  DARKLUA_API int LuaArrayInt ( LPSTR pString )
 {
	 int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	if ( ppLuaStates==NULL ) return 0;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	lua = ppLuaStates[id]->state;

	//
	char str[512];
	strcpy ( str , pString);
	char * pch;
	pch = strstr (str,"->");
	while (pch)
	{
		if (pch) strncpy (pch,"..",2);
		pch = strstr (str,"->");
	}
	//
	//char str[512];
	//strcpy ( str , pString);
	//char * pch;
	//MessageBox(NULL, str, "str =", NULL);
	bool foundFunction = false;	
	pch = NULL;
	int offset = -1;
	pch = strtok (str,".");
	char lastString[256];
	int lastNumber;
	int lastWas= 0;
	while (pch != NULL)
	{
		if (!foundFunction)
		{
			foundFunction = true;
			
			//sprintf ( errorString , "Array = %s" , pch );
			lua_getglobal(lua, pch );
			strcpy ( lastString, pch );
			//MessageBox(NULL, errorString , "" , NULL);
			if ( !lua_istable(lua, offset) )
			{
				lua_pop(lua,-offset);
				sprintf ( errorString , "%s is not a Lua Table" , pch );
				MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
				return 0;
			}
		}
		else
		{
			if (lastWas == 1)
			{

				lua_getfield ( lua , offset , lastString );

				if ( !lua_istable(lua, offset) )
				{
					lua_pop(lua,-offset);
					sprintf ( errorString , "previous field to %s does not exist in table" , pch );
					MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
					return 0;
				}
			}
			else if ( lastWas == 2 )
			{
				lua_rawgeti ( lua, offset , lastNumber );
			}

			lastWas = 1;

			if ( pch[0] == '#' )
			{
				char szString[256];
				strcpy ( szString , pch );
				memcpy ( &szString [ 0 ], &szString [ 1 ], strlen ( szString ) - 1 );
				szString [ strlen ( szString ) - 1 ] = 0;

				int num = atoi(szString);
				lastNumber = num;
				lastWas = 2;
			}
			else
				strcpy ( lastString, pch );
			
		}

		pch = strtok (NULL, ".");
	}

	if ( lastWas != 2 )
	{
		lua_pushstring(lua, lastString );
		offset--;
		lua_gettable(lua, offset);
	}
	else
	{
		lua_rawgeti ( lua, offset , lastNumber );
	}

	int ret = (int)lua_tonumber(lua, -1);
	lua_pop(lua,-(++offset));
	return ret;

 }

 DARKLUA_API float LuaArrayFloat ( LPSTR pString , int id )
 {

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	if ( ppLuaStates==NULL ) return 0;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	lua = ppLuaStates[id]->state;

	//
	char str[512];
	strcpy ( str , pString);
	char * pch;
	pch = strstr (str,"->");
	while (pch)
	{
		if (pch) strncpy (pch,"..",2);
		pch = strstr (str,"->");
	}
	//
	//char str[512];
	//strcpy ( str , pString);
	//char * pch;
	//MessageBox(NULL, str, "str =", NULL);
	bool foundFunction = false;	
	pch = NULL;
	int offset = -1;
	pch = strtok (str,".");
	char lastString[256];
	int lastNumber;
	int lastWas= 0;
	while (pch != NULL)
	{
		if (!foundFunction)
		{
			foundFunction = true;
			
			//sprintf ( errorString , "Array = %s" , pch );
			lua_getglobal(lua, pch );
			strcpy ( lastString, pch );
			//MessageBox(NULL, errorString , "" , NULL);
			if ( !lua_istable(lua, offset) )
			{
				lua_pop(lua,-offset);
				sprintf ( errorString , "%s is not a Lua Table" , pch );
				MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
				return 0;
			}
		}
		else
		{
			if (lastWas == 1)
			{

				lua_getfield ( lua , offset , lastString );

				if ( !lua_istable(lua, offset) )
				{
					lua_pop(lua,-offset);
					sprintf ( errorString , "previous field to %s does not exist in table" , pch );
					MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
					return 0;
				}
			}
			else if ( lastWas == 2 )
			{
				lua_rawgeti ( lua, offset , lastNumber );
			}

			lastWas = 1;

			if ( pch[0] == '#' )
			{
				char szString[256];
				strcpy ( szString , pch );
				memcpy ( &szString [ 0 ], &szString [ 1 ], strlen ( szString ) - 1 );
				szString [ strlen ( szString ) - 1 ] = 0;

				int num = atoi(szString);
				lastNumber = num;
				lastWas = 2;
			}
			else
				strcpy ( lastString, pch );
			
		}

		pch = strtok (NULL, ".");
	}

	if ( lastWas != 2 )
	{
		lua_pushstring(lua, lastString );
		offset--;
		lua_gettable(lua, offset);
	}
	else
	{
		lua_rawgeti ( lua, offset , lastNumber );
	}

	float fValue = (float)lua_tonumber(lua, -1);
	lua_pop(lua,1);
	return fValue;

 }

  DARKLUA_API float LuaArrayFloat ( LPSTR pString )
 {
	 int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	if ( ppLuaStates==NULL ) return 0;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	lua = ppLuaStates[id]->state;

	//
	char str[512];
	strcpy ( str , pString);
	char * pch;
	pch = strstr (str,"->");
	while (pch)
	{
		if (pch) strncpy (pch,"..",2);
		pch = strstr (str,"->");
	}
	//
	//char str[512];
	//strcpy ( str , pString);
	//char * pch;
	//MessageBox(NULL, str, "str =", NULL);
	bool foundFunction = false;	
	pch = NULL;
	int offset = -1;
	pch = strtok (str,".");
	char lastString[256];
	int lastNumber;
	int lastWas= 0;
	while (pch != NULL)
	{
		if (!foundFunction)
		{
			foundFunction = true;
			
			//sprintf ( errorString , "Array = %s" , pch );
			lua_getglobal(lua, pch );
			strcpy ( lastString, pch );
			//MessageBox(NULL, errorString , "" , NULL);
			if ( !lua_istable(lua, offset) )
			{
				lua_pop(lua,-offset);
				sprintf ( errorString , "%s is not a Lua Table" , pch );
				MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
				return 0;
			}
		}
		else
		{
			if (lastWas == 1)
			{

				lua_getfield ( lua , offset , lastString );

				if ( !lua_istable(lua, offset) )
				{
					lua_pop(lua,-offset);
					sprintf ( errorString , "previous field to %s does not exist in table" , pch );
					MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
					return 0;
				}
			}
			else if ( lastWas == 2 )
			{
				lua_rawgeti ( lua, offset , lastNumber );
			}

			lastWas = 1;

			if ( pch[0] == '#' )
			{
				char szString[256];
				strcpy ( szString , pch );
				memcpy ( &szString [ 0 ], &szString [ 1 ], strlen ( szString ) - 1 );
				szString [ strlen ( szString ) - 1 ] = 0;

				int num = atoi(szString);
				lastNumber = num;
				lastWas = 2;
			}
			else
				strcpy ( lastString, pch );
			
		}

		pch = strtok (NULL, ".");
	}

	if ( lastWas != 2 )
	{
		lua_pushstring(lua, lastString );
		offset--;
		lua_gettable(lua, offset);
	}
	else
	{
		lua_rawgeti ( lua, offset , lastNumber );
	}

	float fValue = (float)lua_tonumber(lua, -1);
	lua_pop(lua,1);
	return fValue;

 }

 DARKLUA_API LPSTR LuaArrayString ( LPSTR pString , int id )
 {
	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	if ( ppLuaStates==NULL ) return 0;
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	lua = ppLuaStates[id]->state;

	//
	char str[512];
	strcpy ( str , pString);
	char * pch;
	pch = strstr (str,"->");
	while (pch)
	{
		if (pch) strncpy (pch,"..",2);
		pch = strstr (str,"->");
	}
	//
	//char str[512];
	//strcpy ( str , pString);
	//char * pch;
	//MessageBox(NULL, str, "str =", NULL);
	bool foundFunction = false;	
	pch = NULL;
	int offset = -1;
	pch = strtok (str,".");
	char lastString[256];
	int lastNumber;
	int lastWas= 0;
	while (pch != NULL)
	{
		if (!foundFunction)
		{
			foundFunction = true;
			
			//sprintf ( errorString , "Array = %s" , pch );
			lua_getglobal(lua, pch );
			strcpy ( lastString, pch );
			//MessageBox(NULL, errorString , "" , NULL);
			if ( !lua_istable(lua, offset) )
			{
				lua_pop(lua,-offset);
				sprintf ( errorString , "%s is not a Lua Table" , pch );
				MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
				return 0;
			}
		}
		else
		{
			if (lastWas == 1)
			{

				lua_getfield ( lua , offset , lastString );

				if ( !lua_istable(lua, offset) )
				{
					lua_pop(lua,-offset);
					sprintf ( errorString , "previous field to %s does not exist in table" , pch );
					MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
					return 0;
				}
			}
			else if ( lastWas == 2 )
			{
				lua_rawgeti ( lua, offset , lastNumber );
			}

			lastWas = 1;

			if ( pch[0] == '#' )
			{
				char szString[256];
				strcpy ( szString , pch );
				memcpy ( &szString [ 0 ], &szString [ 1 ], strlen ( szString ) - 1 );
				szString [ strlen ( szString ) - 1 ] = 0;

				int num = atoi(szString);
				lastNumber = num;
				lastWas = 2;
			}
			else
				strcpy ( lastString, pch );
			
		}

		pch = strtok (NULL, ".");
	}

	if ( lastWas != 2 )
	{
		lua_pushstring(lua, lastString );
		offset--;
		lua_gettable(lua, offset);
	}
	else
	{
		lua_rawgeti ( lua, offset , lastNumber );
	}

  	// Return string pointer
	LPSTR pReturnString=NULL;
	const char *s = lua_tostring(lua, -1);
	lua_pop(lua,1);

	// If input string valid
	if(s)
	{
		// Create a new string and copy input string to it
		DWORD dwSize=strlen( s );
		g_pGlob->CreateDeleteString ( (char**)&pReturnString, dwSize+1 );
		strcpy(pReturnString, s);
	}
	else
	{
		return NULL;
	}

	return pReturnString;

 }

 DARKLUA_API LPSTR LuaArrayString ( LPSTR pString )
 {
	 int id = defaultState;

	if ( id > maxLuaStates+1 )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	if ( ppLuaStates==NULL ) return 0; 
	if ( ppLuaStates[id] == NULL )
	{
		//MessageBox(NULL, "Lua ID not in use", "LUA ERROR", NULL);
		return 0;
	}

	lua = ppLuaStates[id]->state;

	//
	char str[512];
	strcpy ( str , pString);
	char * pch;
	pch = strstr (str,"->");
	while (pch)
	{
		if (pch) strncpy (pch,"..",2);
		pch = strstr (str,"->");
	}
	//
	//char str[512];
	//strcpy ( str , pString);
	//char * pch;
	//MessageBox(NULL, str, "str =", NULL);
	bool foundFunction = false;	
	pch = NULL;
	int offset = -1;
	pch = strtok (str,".");
	char lastString[256];
	int lastNumber;
	int lastWas= 0;
	while (pch != NULL)
	{
		if (!foundFunction)
		{
			foundFunction = true;
			
			//sprintf ( errorString , "Array = %s" , pch );
			lua_getglobal(lua, pch );
			strcpy ( lastString, pch );
			//MessageBox(NULL, errorString , "" , NULL);
			if ( !lua_istable(lua, offset) )
			{
				lua_pop(lua,-offset);
				sprintf ( errorString , "%s is not a Lua Table" , pch );
				MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
				return 0;
			}
		}
		else
		{
			if (lastWas == 1)
			{

				lua_getfield ( lua , offset , lastString );

				if ( !lua_istable(lua, offset) )
				{
					lua_pop(lua,-offset);
					sprintf ( errorString , "previous field to %s does not exist in table" , pch );
					MessageBox(NULL, errorString , "" , MB_TOPMOST | MB_OK);
					return 0;
				}
			}
			else if ( lastWas == 2 )
			{
				lua_rawgeti ( lua, offset , lastNumber );
			}

			lastWas = 1;

			if ( pch[0] == '#' )
			{
				char szString[256];
				strcpy ( szString , pch );
				memcpy ( &szString [ 0 ], &szString [ 1 ], strlen ( szString ) - 1 );
				szString [ strlen ( szString ) - 1 ] = 0;

				int num = atoi(szString);
				lastNumber = num;
				lastWas = 2;
			}
			else
				strcpy ( lastString, pch );
			
		}

		pch = strtok (NULL, ".");
	}

	if ( lastWas != 2 )
	{
		lua_pushstring(lua, lastString );
		offset--;
		lua_gettable(lua, offset);
	}
	else
	{
		lua_rawgeti ( lua, offset , lastNumber );
	}

  	// Return string pointer
	LPSTR pReturnString=NULL;
	const char *s = lua_tostring(lua, -1);
	lua_pop(lua,1);

	// If input string valid
	if(s)
	{
		// Create a new string and copy input string to it
		DWORD dwSize=strlen( s );
		g_pGlob->CreateDeleteString ( (char**)&pReturnString, dwSize+1 );
		strcpy(pReturnString, s);
	}
	else
	{
		return NULL;
	}

	return pReturnString;
 }

// This is the constructor of a class that has been exported.
// see DarkLUA.h for the class definition
CDarkLUA::CDarkLUA()
{
	return;
}

///////////////////////////////////////////////////////////

bool LuaCheckForWorkshopFile ( LPSTR VirtualFilename)
{
	if ( !VirtualFilename ) return false;
	if ( strlen ( VirtualFilename ) < 3 ) return false;

	char* tempCharPointerCheck = NULL;
	tempCharPointerCheck = strrchr( VirtualFilename, '\\' );
	if ( tempCharPointerCheck == VirtualFilename+strlen(VirtualFilename)-1 ) return false;
	if ( VirtualFilename[0] == '.' ) return false;
	if ( strstr ( VirtualFilename , ".fpm" ) ) return false;

	// encrypted file check
	char szEncryptedFilename[_MAX_PATH];
	char szEncryptedFilenameFolder[_MAX_PATH];
	char* tempCharPointer = NULL;
	strcpy ( szEncryptedFilenameFolder, VirtualFilename );

	// replace and forward slashes with backslash
	for ( int c = 0 ; c < strlen(szEncryptedFilenameFolder); c++ )
	{
		if ( szEncryptedFilenameFolder[c] == '/' ) 
			szEncryptedFilenameFolder[c] = '\\';
	}

	tempCharPointer = strrchr( szEncryptedFilenameFolder, '\\' );
	if ( tempCharPointer && tempCharPointer != szEncryptedFilenameFolder+strlen(szEncryptedFilenameFolder)-1 )
	{
		tempCharPointer[0] = 0;
		sprintf ( szEncryptedFilename , "%s\\_e_%s" , szEncryptedFilenameFolder , tempCharPointer+1 );
	}
	else
	{
		sprintf ( szEncryptedFilename , "_e_%s" , szEncryptedFilenameFolder );
	}
	FILE* tempFile = NULL;
	tempFile = GG_fopen ( szEncryptedFilename ,"r" );
	if ( tempFile )
	{
		fclose ( tempFile );
		strcpy ( VirtualFilename , szEncryptedFilename );
		return true;
	}
	// end of encrypted file check

	// Workshop handling
	#ifdef PHOTONMP
	#else
		char szWorkshopFilename[_MAX_PATH];
		char szWorkshopFilenameFolder[_MAX_PATH];
		char szWorkShopItemPath[_MAX_PATH];
		SteamGetWorkshopItemPathDLL(szWorkShopItemPath);
		//strcpy ( szWorkShopItemPath,"D:\\Games\\Steam\\steamapps\\workshop\\content\\266310\\378822626");
		// If the string is empty then there is no active workshop item, so we can return
		if ( strcmp ( szWorkShopItemPath , "" ) == 0 ) return false;
		tempCharPointer = NULL;
		strcpy ( szWorkshopFilenameFolder, VirtualFilename );

		// only check if the workshop item path isnt blank
		if ( strcmp ( szWorkShopItemPath , "" ) )
		{
			// replace and forward slashes with backslash
			for ( unsigned int c = 0 ; c < strlen(szWorkshopFilenameFolder); c++ )
			{
				if ( szWorkshopFilenameFolder[c] == '/' )
					szWorkshopFilenameFolder[c] = '\\';
			}

			// strip off any path to files folder
			bool found = true;
			while ( found )
			{
			char* stripped = strstr ( szWorkshopFilenameFolder , "Files\\" );
			if ( !stripped )
				stripped = strstr ( szWorkshopFilenameFolder , "files\\" );

			if ( stripped )
				strcpy ( szWorkshopFilenameFolder , stripped+6 );
			else
				found = false;
			}

			bool last = false;
			char tempstring[MAX_PATH];
			strcpy ( tempstring, szWorkshopFilenameFolder);
			strcpy ( szWorkshopFilenameFolder , "" );
			// replace and forward slashes with backslash
			for ( unsigned int c = 0 ; c < strlen(tempstring); c++ )
			{
				if ( tempstring[c] == '/' || tempstring[c] == '\\' ) 
				{
					if ( last == false )
					{
						strcat ( szWorkshopFilenameFolder , "_" );
						last = true;
					}
				}
				else
				{
					strcat ( szWorkshopFilenameFolder , " " );
					szWorkshopFilenameFolder[strlen(szWorkshopFilenameFolder)-1] = tempstring[c];
					last = false;
				}
			}

			//NEED TO CHECK IF THE FILE EXISTS FIRST, IF IT DOES WE COPY IT
			char szTempName[_MAX_PATH];
			strcpy ( szTempName , szWorkShopItemPath );
			strcat ( szTempName , "\\" );
			strcat ( szTempName , szWorkshopFilenameFolder );

			FILE* tempFile = NULL;
			tempFile = GG_fopen ( szTempName ,"r" );
			if ( tempFile )
			{
				fclose ( tempFile );
				int szTempNamelength = strlen(szTempName);
				int virtualfilelength = strlen(VirtualFilename);				
				strcpy ( VirtualFilename , szTempName );
				return true;
			}
			else // check for encrypted version
			{
				char* tempCharPointer = NULL;

				tempCharPointer = strrchr( szTempName, '\\' );
				if ( tempCharPointer && tempCharPointer != szTempName+strlen(szTempName)-1 )
				{
					tempCharPointer[0] = 0;
					sprintf ( szWorkshopFilename , "%s\\_w_%s" , szTempName , tempCharPointer+1 );
				}
				else
				{
					sprintf ( szWorkshopFilename , "_w_%s" , szTempName );
				}
				FILE* tempFile = NULL;
				tempFile = GG_fopen ( szWorkshopFilename ,"r" );
				if ( tempFile )
				{
					fclose ( tempFile );
					strcpy ( VirtualFilename , szWorkshopFilename );
					return true;
				}
			}
		}
	#endif
	return false;
}


#ifdef LUA_DO_DEBUG
FILE* g_fpDebug = NULL;

void OpenDebugLog ( char* szFile )
{
 char szFileOpen [ 256 ] = "";
 sprintf ( szFileOpen, "f:\\%s", szFile );
 g_fpDebug = GG_fopen ( szFileOpen, "wt" );
}

void WriteToDebugLog ( char* szMessage, bool bNewLine )
{
 if ( !g_fpDebug )
  OpenDebugLog ( "log.txt" );
 
 if ( !g_fpDebug )
  return;

 char szOut [ 256 ] = "";

 if ( bNewLine )
  sprintf ( szOut, "%s\n", szMessage );
 else
  sprintf ( szOut, "%s", szMessage );

 fwrite ( szOut, strlen ( szOut ) * sizeof ( char ), 1, g_fpDebug );
 fflush ( g_fpDebug );
}

void WriteToDebugLog ( char* szMessage, int i )
{
 if ( !g_fpDebug )
  OpenDebugLog ( "log.txt" );
 
 if ( !g_fpDebug )
  return;

 char szOut [ 256 ] = "";

 if ( 1 )
  sprintf ( szOut, "%s = %i\n", szMessage , i );


 fwrite ( szOut, strlen ( szOut ) * sizeof ( char ), 1, g_fpDebug );
 fflush ( g_fpDebug );
}

void WriteToDebugLog ( char* szMessage, float f )
{
 if ( !g_fpDebug )
  OpenDebugLog ( "log.txt" );
 
 if ( !g_fpDebug )
  return;

 char szOut [ 256 ] = "";

 if ( 1 )
  sprintf ( szOut, "%s = %f\n", szMessage , f );


 fwrite ( szOut, strlen ( szOut ) * sizeof ( char ), 1, g_fpDebug );
 fflush ( g_fpDebug );
}

void WriteToDebugLog ( char* szMessage, char* s )
{
 if ( !g_fpDebug )
  OpenDebugLog ( "log.txt" );
 
 if ( !g_fpDebug )
  return;

 char szOut [ 256 ] = "";

 if ( 1 )
  sprintf ( szOut, "%s = %s\n", s );


 fwrite ( szOut, strlen ( szOut ) * sizeof ( char ), 1, g_fpDebug );
 fflush ( g_fpDebug );
}

void CloseDebugLog ( void )
{
 fclose ( g_fpDebug );
}
#endif
