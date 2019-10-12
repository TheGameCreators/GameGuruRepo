// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

//#define FPSCR __declspec ( dllexport )
#define FPSCR 

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "timer.h"

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

#ifdef WIN32

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 2k or later.
#define WINVER 0x0500		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows 2k or later.                   
#define _WIN32_WINNT 0x0500	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

// Windows Header Files:
#include <tchar.h>

// Winsock
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib" )

typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;

#include "steam/steam_api.h"
#include "steam/isteamuserstats.h"
#include "steam/isteamremotestorage.h"
#include "steam/isteammatchmaking.h"
#include "steam/steam_gameserver.h"

// OUT_Z_ARRAY indicates an output array that will be null-terminated.
#if _MSC_VER >= 1600
       // Include the annotation header file.
       #include <sal.h>
       #if _MSC_VER >= 1700
              // VS 2012+
              #define OUT_Z_ARRAY _Post_z_
       #else
              // VS 2010
              #define OUT_Z_ARRAY _Deref_post_z_
       #endif
#else
       // gcc, clang, old versions of VS
       #define OUT_Z_ARRAY
#endif
 
template <size_t maxLenInChars> void sprintf_safe(OUT_Z_ARRAY char (&pDest)[maxLenInChars], const char *pFormat, ... )
{
	va_list params;
	va_start( params, pFormat );
#ifdef POSIX
	vsnprintf( pDest, maxLenInChars, pFormat, params );
#else
	_vsnprintf( pDest, maxLenInChars, pFormat, params );
#endif
	pDest[maxLenInChars - 1] = '\0';
	va_end( params );
}

// XAudio2 header files
#include "xaudio new.h"

#ifdef STEAM_CEG
// Steam DRM header file
#include "cegclient.h"
#else
#define Steamworks_InitCEGLibrary() (true)
#define Steamworks_TermCEGLibrary()
#define Steamworks_TestSecret()
#define Steamworks_SelfCheck()
#endif

#endif