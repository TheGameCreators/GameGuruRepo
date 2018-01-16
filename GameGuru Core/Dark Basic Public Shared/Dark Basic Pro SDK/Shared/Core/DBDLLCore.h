//
// DBDLLCore Header
//

#ifndef DBDLLCORE_H
#define DBDLLCORE_H

// Common Includes
#include "windows.h"

// Override with DBP version
#undef CopyMemory

// Custom Includes
#include "globstruct.h"
#include "..\error\cerror.h"
#include "macros.h"

// Additional CoreHeader Includes
#include "DBDLLExtCalls.h"
#include "..\global.h"
#include <DB3Math.h>

#define DBPRO_GLOBAL 

// Global Core Functions
extern DARKSDK void InternalClearWindowsEntry(void);
extern DARKSDK DWORD InternalProcessMessages();

// Global Internal Data
extern HWND				g_hWnd						;
extern LPSTR			g_pVarSpace					;

// Global Performance Flag Vars
extern bool				g_bProcessorFriendly		;

// Global Display Vars
extern HBITMAP			g_hDisplayBitmap			;
extern HDC				g_hdcDisplay				;
extern int				g_iX						;
extern int				g_iY						;
extern COLORREF			g_colFore					;
extern COLORREF			g_colBack					;
extern HBRUSH			g_hBrush					;
extern DWORD			g_dwScreenWidth				;
extern DWORD			g_dwScreenHeight			;
extern DWORD			g_dwWindowWidth				;
extern DWORD			g_dwWindowHeight			;

// Global Input Vars
extern DWORD			g_dwWindowsTextEntrySize	;
extern LPSTR			g_pWindowsTextEntry			;
extern DWORD			g_dwWindowsTextEntryPos		;
extern unsigned char	g_cKeyPressed				;
extern int				g_iEntryCursorState			;

// Global Data Vars
extern LPSTR			g_pDataLabelPtr				;
extern LPSTR			g_pDataLabelEnd				;

// Internals Functions

DARKSDK void PassCmdLineHandlerPtr(LPVOID pCmdLinePtr);
DARKSDK void PassErrorHandlerPtr(LPVOID pErrorPtr);
DARKSDK DWORD InitDisplay(DWORD dwDisplayType, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, HINSTANCE hInstance, LPSTR pApplicationName);
DARKSDK void SetRenderOrderList(void);
DARKSDK void ConstructDLLs(void);
DARKSDK DWORD CloseDisplay(void);

// Actual commands

DARKSDK void Sync(void);
DARKSDK void SetCursor ( int iX, int iY );
DARKSDK void SyncOn ( void );
DARKSDK void SyncOff ( void );
DARKSDK void SyncRate(int iRate);
DARKSDK void SyncMask(DWORD dwMask);
DARKSDK void FastSync ( void );
DARKSDK void FastSync ( int iNonDisplayUpdates );
DARKSDK void FastSyncInputOnly ( void );
DARKSDK void DrawToBack ( void );
DARKSDK void DrawToFront ( void );
DARKSDK LONGLONG PerformanceTimer ( void );
DARKSDK DWORD MakeMemory(int iSize);
DARKSDK void SetLocalTimerReset(void);
DARKSDK int Timer(void);
DARKSDK void LoadArray( LPSTR szFilename, DWORD dwAllocation );
DARKSDK void SaveArray(LPSTR pFilename, DWORD dwAllocation);
DARKSDK LPSTR Cl ( void );
DARKSDK void CopyMemory(DWORD dwDest, DWORD dwSrc, int iSize);
DARKSDK void AlwaysActiveOn ( void );
DARKSDK void AlwaysActiveOff ( void );
DARKSDK void Print(LPSTR pString);
//DARKSDK void Print(LONGLONG lValue);
DARKSDK void Print(double dValue);
DARKSDK void Print(void);
//DARKSDK void PrintConcat(LONGLONG lValue);
DARKSDK void PrintConcat(double dValue);
DARKSDK void PrintConcat(LPSTR pString);
DARKSDK void WaitForKey(void);
DARKSDK void SleepNow(int iDelay);
DARKSDK float WrapValue(float da);
DARKSDK float Sqrt(float fValue);
DARKSDK float NewXValue(float x, float a, float b);
DARKSDK float NewZValue(float z, float a, float b);
DARKSDK float CurveValue(float a, float da, float sp);
DARKSDK float CurveAngle(float a, float da, float sp);
DARKSDK float Sin(float fAngle);
DARKSDK float Cos(float fAngle);
DARKSDK int Rnd(int r);
DARKSDK float Atan2(float fA, float fB);
DARKSDK void Randomize(int iSeed);
DARKSDK LONGLONG PerformanceFrequency ( void );
DARKSDK float timeGetSecond ( void );

DARKSDK LPSTR Inkey(void);
DARKSDK float Ceil(float x);
DARKSDK float Floor(float x);
DARKSDK float Tan(float fAngle);
DARKSDK float Asin(float fValue);
DARKSDK void Cls(void);

DARKSDK void EncryptWorkshopDBPro ( char* dwStringAddress );
DARKSDK void EncryptAllFiles(char* dwStringAddress);


#endif
