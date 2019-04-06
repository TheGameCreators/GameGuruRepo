//
// DarkDLLCore
//

// Standard Includes
#define _CRT_SECURE_NO_DEPRECATE
#define WINVER 0x0601
#define _USING_V110_SDK71_
#include "windows.h"
#include "math.h"
#include "time.h"

// External Includes
#include "..\error\cerror.h"
#include "..\..\DarkSDK\Core\resource.h"
#include "..\..\..\..\Guru-MapEditor\Encryptor.h"
#include ".\..\Core\SteamCheckForWorkshop.h"
#include "SteamCommands.h"
#include "DarkLUA.h"

// Internal Includes
#include "DBDLLCore.h"
#include "DBDLLDisplay.h"
#include "DBDLLCoreInternal.h"
#include "DBDLLArray.h"
#include "RenderList.h"

// Vectors and stack
#include <vector>
#include <stack>

// DBP functions
#include "CGfxC.h"
#include "CObjectsC.h"
#include "CCameraC.h"
#include "CImageC.h"
#include "cVectorC.h"
#include "CLightC.h"
#include "CSpritesC.h"
#include "ConvX.h"
#include "CSoundC.h"
#include "CBasic2DC.h"
#include "CParticleC.h"
#include "CBitmapC.h"
#include "CAnimation.h"
#include "CFileC.h"
#include "CMemblocks.h"
#include "CFTPC.h"
#include "CInputC.h"
#include "CTextC.h"
#include "CSystemC.h"
#include "BulletPhysics.H"
#include "BlitzTerrain.H"
#include "DarkAI.H"
#include "SoftwareCulling.h"
#include "DarkLUA.h"
#include "SimonReloaded.h"
#include "SteamCommands.h"
#include "LightMapper.h"
#include "Enchancements.h"

//Windows Mixed Reality Support
#include "GGVR.h"

#include <iostream>
#include <fstream>

DB_ENTER_NS()
DB_LEAVE_NS()

// External Pointers (for cores own error handling)
extern CRuntimeErrorHandler* g_pErrorHandler;
extern LPGG m_pDX;

// Prototypes
LPSTR GetTypePatternCore ( LPSTR dwTypeName, DWORD dwTypeIndex );
DWORD GetNextSyncDelay();

// Touch System works under XP and Win7 now
bool bDetectAndActivateWindows7TouchSystem = false;

// Global Core Vars
DBPRO_GLOBAL LPSTR			g_pVarSpace					= NULL;
DBPRO_GLOBAL LPSTR			g_pDataSpace				= NULL;

// Global Stack Store Vars
DBPRO_GLOBAL DWORD			g_dwStackStoreSize			= 0;
DBPRO_GLOBAL DWORD*			g_pStackStore				= NULL;

// Global Performance Switches
DBPRO_GLOBAL bool			g_bAlwaysActiveOff			= false;
DBPRO_GLOBAL bool			g_bProcessorFriendly		= false; // leefix - 070403 - patch 4 slowdown bug
DBPRO_GLOBAL bool			g_bAlwaysActiveOneOff		= false; // leeadd - 201204 - flag to draw just once (typically when PAINT refreshes)
DBPRO_GLOBAL bool			g_bSyncOff					= true;
DBPRO_GLOBAL bool			g_bSceneStarted				= false;
DBPRO_GLOBAL bool			g_bCanRenderNow				= true;
DBPRO_GLOBAL DWORD			g_dwSyncMask				= 0xFFFFFFFF;
DBPRO_GLOBAL DWORD			g_dwSyncMaskOverride		= 0xFFFFFFFF;

// Global Sync Settings
DBPRO_GLOBAL DWORD			g_dwManualSuperStepSetting	= 0;
DBPRO_GLOBAL DWORD*         g_pdwSyncRateSetting        = NULL;
DBPRO_GLOBAL DWORD          g_dwSyncRateSettingSize     = 0;
DBPRO_GLOBAL DWORD          g_dwSyncRateCurrent         = 0;

// Global Performance Flags used Internally
DBPRO_GLOBAL bool			g_bCascadeQuitFlag			= false;
DBPRO_GLOBAL DWORD			g_dwRecordedTimer			= 0;

// Global Error Handling and Pointers
DBPRO_GLOBAL LPSTR			g_pCommandLineString		= NULL;
DBPRO_GLOBAL LPVOID			g_ErrorHandler				= NULL;
DBPRO_GLOBAL LPVOID			g_EscapeValue				= NULL;
DBPRO_GLOBAL LPVOID			g_BreakOutPosition			= NULL;

// U71 - added to store structure patterns in core (passed in from EXEBlock)
DBPRO_GLOBAL DWORD			g_dwStructPatternQty		= 0;
DBPRO_GLOBAL LPSTR			g_pStructPatternsPtr		= NULL;

// Global Display Vars
DBPRO_GLOBAL HBITMAP		g_hDisplayBitmap			= NULL;
DBPRO_GLOBAL HDC			g_hdcDisplay				= NULL;
DBPRO_GLOBAL COLORREF		g_colFore					= RGB(255,255,255);
DBPRO_GLOBAL COLORREF		g_colBack					= RGB(0,0,0);
DBPRO_GLOBAL HBRUSH			g_hBrush					= NULL;
DBPRO_GLOBAL DWORD			g_dwScreenWidth				= 0;
DBPRO_GLOBAL DWORD			g_dwScreenHeight			= 0;

DBPRO_GLOBAL HICON			g_hUseIcon					= NULL;
DBPRO_GLOBAL HCURSOR		g_hUseArrow 				= NULL;
DBPRO_GLOBAL HCURSOR		g_hUseHourglass 			= NULL;
DBPRO_GLOBAL HCURSOR		g_hCustomCursors[30];
DBPRO_GLOBAL HCURSOR		g_ActiveCursor 				= NULL;
DBPRO_GLOBAL HCURSOR		g_OldCursor 				= NULL;

// Global Draw Order Flags
DBPRO_GLOBAL bool			g_bDrawAutoStuffFirst		= true;
DBPRO_GLOBAL bool			g_bDrawSpritesFirst			= false;
DBPRO_GLOBAL bool			g_bDrawEntirelyToCamera		= false;

// Global Input Vars
DBPRO_GLOBAL DWORD			g_dwWindowsTextEntrySize	= 0;
DBPRO_GLOBAL DWORD			g_dwWindowsTextEntryPos		= 0;
DBPRO_GLOBAL unsigned char	g_cKeyPressed				= 0;
DBPRO_GLOBAL unsigned char	g_cInkeyCodeKey				= 0;
DBPRO_GLOBAL int			g_iEntryCursorState			= 0;
DBPRO_GLOBAL WORD			g_wWinKey					= 0;

// Global Data Vars
DBPRO_GLOBAL LPSTR			g_pDataLabelStart			= NULL;
DBPRO_GLOBAL LPSTR			g_pDataLabelPtr				= NULL;
DBPRO_GLOBAL LPSTR			g_pDataLabelEnd				= NULL;

// Global Security Data
DBPRO_GLOBAL int			g_iSecurityCode				= 0;

// Global Error Helper Clue String
DBPRO_GLOBAL char			g_strErrorClue[512];

// 291116 - allows a much better timeGetTime() using performance counter precision
DBPRO_GLOBAL LONGLONG		g_lFirstPerfTime			= 0;

// Prototype
DARKSDK void CallEncryptDecrypt( DWORD dwStringAddress, bool bEncryptIfTrue, bool bDoNotUseTempFolder );

// Small helper function to get more accurate timings
class AccurateTimer
{
private:
	UINT  Period;
	DWORD LastTime;

	// Disable copying
	AccurateTimer(const AccurateTimer&);
	AccurateTimer& operator=(const AccurateTimer&);
public:
	AccurateTimer() 
	{
		TIMECAPS caps;
		timeGetDevCaps(&caps, sizeof(caps));
		Period = caps.wPeriodMin;
		timeBeginPeriod(Period);
	}
	~AccurateTimer()
	{
		timeEndPeriod(Period);
	}
	DWORD Get()
	{
		LastTime = timeGetTime();
		return LastTime;
	}
	DWORD Last() const
	{
		return LastTime;
	}
};

bool IsArraySingleDim ( DWORD dwArrayPtr )
{
	// Detect if array has single dimension only, false if multi or irregular array
	DWORD* pOldHeader = (DWORD*)(((LPSTR)dwArrayPtr)-HEADERSIZEINBYTES);
	DWORD dwSizeOfOneDataItem = pOldHeader[11];
	if ( dwSizeOfOneDataItem > 1024000 ) return false;
	if ( pOldHeader [ 1 ] > 0 ) return false;
	return true;
}

DARKSDK DWORD ProcessMessagesOnly(void)
{
	// U76 - Windows 7 touch has no 'touch-release' via WM_MOUSE commands
	// so create an artificial persistence so MOUSECLICK(DX) can detect it
	if ( g_pGlob->dwWindowsMouseLeftTouchPersist > 0 )
		if ( timeGetTime() > g_pGlob->dwWindowsMouseLeftTouchPersist )
			g_pGlob->dwWindowsMouseLeftTouchPersist=0;

	// Vars
	MSG msg;

	// Cascade means it will continue to quit (for rapid exit)
	if(g_bCascadeQuitFlag==true)
		return 1;

	// Message Pump
	while(TRUE)
	{
		// Standard Windows Processing
		if(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			if(msg.message!=WM_QUIT)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				g_bCascadeQuitFlag=true;
				return 1;
			}
		}
		else
		{
			// Processor Friendly
			if(g_bProcessorFriendly) Sleep(1);
			break;
		}
	}

	// Complete
	return 0;
}

DARKSDK void ConstantNonDisplayUpdate(void)
{
	// Update All NonVisuals (this gets called about six times because of processmessage calls..)
	UpdateSound();
	UpdateAllAnimation();
}

DARKSDK void ExternalDisplaySync ( int iSkipSyncRateCodeAkaFastSync )
{
	// Skip this phase if app has been shut down (always active off)
	if ( g_bAlwaysActiveOneOff ) 
		return;

	// If display not ready, can skip this
	#ifdef DX11
	if ( m_pImmediateContext == NULL )
		return;
	#endif

	// V111 - 110608 - FASTSYNC should not use sync delay!
	if ( iSkipSyncRateCodeAkaFastSync==0 )
	{
		AccurateTimer Timer;

		// Skip refreshes causing even faster FPS rates!
		if(g_dwManualSuperStepSetting>0)
		{
			if(Timer.Get()-g_dwRecordedTimer<g_dwManualSuperStepSetting)
				return;
		}

		// Force FPS
		if(g_dwSyncRateSettingSize>0)
		{
			DWORD dwDifference = GetNextSyncDelay();
			while(Timer.Get()-g_dwRecordedTimer < dwDifference)
				if(ProcessMessagesOnly()==1) return;
		}
		else
		{
			// Need to ad least process these monitors
			ConstantNonDisplayUpdate();
		}

		// u74b7 - Only update the sync timer if not using fastsync
		// Record time of update
		g_dwRecordedTimer = Timer.Last();
	}

	// leefix - 260604 - u54 - in case input wants single data-grab functionality
	ClearData();

	// 270515 - calls this to grab latest viewproj and record previous viewproj
	if ( iSkipSyncRateCodeAkaFastSync==0 )
		UpdateViewProjForMotionBlur();

	// If using External Graphics API
	// camera zero off suspends normal operations
	bool bSuspendScreenOperations = false;
	if ( !(g_dwSyncMask & 1) )
	{
		// flag the suspension of regular screen zero activity
		bSuspendScreenOperations = true;
	}
	if ( bSuspendScreenOperations==false )
	{
		// If BSP used, compute responses
		AutomaticEnd();

		// Draw Phase : Store backbuffer before any 3D is drawn..
		SaveSpritesBack();

		// Draw Phase : Draw Sprites Last
		if(g_bDrawSpritesFirst==false)
			UpdateSprites();
			
		// Ensures AutoStuff is first to be rendered
		if(g_bDrawAutoStuffFirst==true)
		{
			if(g_bSceneStarted)
			{
				End();
				if ( g_bCanRenderNow )
					Render();
			}
			g_bSceneStarted=true;
			Begin();

			// restore before-Sprites-drawn on new screen render
			RestoreSpritesBack();
		}

		// Draw Phase : Draw Sprites First
		if(g_bDrawSpritesFirst==true)
			UpdateSprites();
	}

	// Draw Phase : Draw 3D Gempoetry
	// Reset polycount and drawprim count
	if ( g_pGlob ) g_pGlob->dwNumberOfPolygonsDrawn=0;
	if ( g_pGlob ) g_pGlob->dwNumberOfPrimCalls=0;

	// Disable backdrop if camera zero disabled
	int iMode = 0; if ( bSuspendScreenOperations ) iMode = 1;

	// U75 - 080410 - ensure animation in scene only calculated once (on SYNC)
	if ( iSkipSyncRateCodeAkaFastSync==0 )
		UpdateAnimationCycle();

	// Draw all 3D - all cameras loop
	StartSceneEx ( iMode );
	do 
	{
		int iThisCamera = 1 + GetRenderCamera();
		if ( iThisCamera <= 32 )
		{
			// camera 0 - 31 can be masked
			DWORD dwCamBit = 1;
			if ( iThisCamera > 1 ) dwCamBit = dwCamBit << (DWORD)(iThisCamera-1);
			dwCamBit = dwCamBit & g_dwSyncMask;
			if ( dwCamBit==0 ) iThisCamera = 0;
		}
		if ( iThisCamera > 0 )
		{
			// Push all polygons for 3D components
			ExecuteRenderList();
		}
		// Next camera or finish..
	} while (FinishSceneEx(false)==0);

	// After 3D operations, direct whether SPRITES/2D/IMAGE
	// drawing is to take place by default (bitmap or camera zero)
	if ( g_bDrawEntirelyToCamera==true ) RunCode ( 1 );

	// not suspended
	if ( bSuspendScreenOperations==false )
	{
		// Ensures AutoStuff is last to be rendered
		if(g_bDrawAutoStuffFirst==false)
		{
			if(g_bSceneStarted)
			{
				End();
				if ( g_bCanRenderNow )
					Render();
			}
			g_bSceneStarted=true;
			Begin();

			// restore before-Sprites-drawn on new screen render
			RestoreSpritesBack();
		}

		// If BSP used, set response check
		AutomaticStart();
	}
}

DARKSDK void ExternalDisplayUpdate(void)
{
	// Call external sync if automatic
	if(g_bSyncOff) ExternalDisplaySync(0);
}

LRESULT CALLBACK EmptyWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	// Default Action
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_SETTEXT:
		{
		}

		case WM_ACTIVATE:
		{
			// 20/7/11 - Win7 - ensure we register for TOUCH over GESTURE (also allows LBUTTONDOWN to happen instantly!)
			HWND hwndPrevious = (HWND) lParam;
			if ( bDetectAndActivateWindows7TouchSystem==false )
			{
				bDetectAndActivateWindows7TouchSystem = true;
				OSVERSIONINFO osvi;
				BOOL bIsWindows7orLater;
				ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
				osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
				GetVersionEx(&osvi);
				bIsWindows7orLater = ( (osvi.dwMajorVersion > 6) || ( (osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 1) ));
				if ( bIsWindows7orLater==TRUE )
				{
					// must dynamically find the user32.dll function and call it IF Windows 7 (allows Windows XP to run)
					typedef UINT (CALLBACK* sRegisterTouchWindowFnc)(HWND,ULONG);
					HMODULE hWinUserDLL = LoadLibrary ( "user32.dll" );
					if ( hWinUserDLL )
					{
						sRegisterTouchWindowFnc pRegTouchWin = (sRegisterTouchWindowFnc) GetProcAddress ( hWinUserDLL, "RegisterTouchWindow" );
						if ( pRegTouchWin ) BOOL bRes = pRegTouchWin ( g_pGlob->hWnd, 0 );
						FreeLibrary ( hWinUserDLL );
					}
				}
			}

			break;
		}

		case WM_CLOSE:
		{
			#ifdef DARKSDK_COMPILE
			g_iDarkGameSDKQuit = 1;
			#endif
			PostQuitMessage(0);
			return TRUE;
		}

		case WM_DESTROY:
		case WM_NCDESTROY:
		{
			PostQuitMessage(0);
			break;
		}

		case WM_ERASEBKGND:
			return TRUE;

		case WM_SIZE:
		case WM_SIZING:
		case WM_MOVE:
		case WM_MOVING:
		case WM_PAINT:
			{
				// 180214 - record new size in glob struct
				RECT rc;

				// GDI Paint
				PAINTSTRUCT ps;
				HDC hdcClient = BeginPaint(hWnd, &ps);
				if(hdcClient)
				{
					if(g_hdcDisplay)
					{
						GetClientRect(hWnd, &rc);
						HGDIOBJ hdcOld = SelectObject(g_hdcDisplay, g_hDisplayBitmap);
						BitBlt(hdcClient, rc.left, rc.top, rc.right, rc.bottom, g_hdcDisplay, 0, 0, SRCCOPY);
						SelectObject(g_hdcDisplay, hdcOld);
					}
					else
					{
						// 210203 - if array of protected boxes setup (from controls requiring primary surface)
						if ( g_pGlob->dwSafeRectMax>0 )
						{
							// Clear Device
							GetClientRect(hWnd, &rc);
							HBRUSH bGrey = GetSysColorBrush ( COLOR_3DFACE );
							HBRUSH bOld = (HBRUSH)SelectObject(hdcClient, bGrey ); 
							Rectangle(hdcClient, -5, -5, rc.right+5, rc.bottom+5);
							SelectObject(hdcClient, bOld ); 
						}
					}
					EndPaint(hWnd, &ps);
				}

				// Ensures rendered areas are retained (when moving window or menu refreshing)
				if ( g_pGlob->dwAppDisplayModeUsing==1 )
				{
					// only dwDisplayMode=1 (window) should do this (otherwise render several times!!)
					// ensure refresh is not done in middle of draw-phase
					End(); Render(); Begin();
				}
			}
			return TRUE;

		case WM_MOUSEMOVE:
			{
				// Get Client Raw Mouse Position
				g_pGlob->iWindowsMouseX = LOWORD(lParam);  // horizontal position of cursor 
				g_pGlob->iWindowsMouseY = HIWORD(lParam);  // vertical position of cursor 
				
				// Special Scale for When Windows Stretch Beyond Physical Size of Backbuffer
				RECT rc;
				GetClientRect(hWnd, &rc);
				float xRatio = (float)g_pGlob->dwWindowWidth/(float)rc.right;
				float yRatio = (float)g_pGlob->dwWindowHeight/(float)rc.bottom;
				g_pGlob->iWindowsMouseX = (int)((float)g_pGlob->iWindowsMouseX * xRatio);
				g_pGlob->iWindowsMouseY = (int)((float)g_pGlob->iWindowsMouseY * yRatio);

				// Restore cursor when move mouse
				if ( g_ActiveCursor != NULL ) SetCursor ( g_ActiveCursor );

			}
			break;

		case WM_LBUTTONDOWN:
			g_pGlob->iWindowsMouseClick|=1;
			g_pGlob->dwWindowsMouseLeftTouchPersist=timeGetTime()+250; // U76 - many cycles
			if ( GetFocus()!=hWnd ) 
			{
				SetFocus ( hWnd );
			}
			break;

		case WM_RBUTTONDOWN:
			g_pGlob->iWindowsMouseClick|=2;
			if ( GetFocus()!=hWnd ) SetFocus ( hWnd );
			break;

		// aaron - 20120811 - Potential issues when using xor depending on obscure and rare window interaction
		case WM_LBUTTONUP:
			g_pGlob->iWindowsMouseClick &= ~1UL;
			break;

		case WM_RBUTTONUP:
			g_pGlob->iWindowsMouseClick &= ~2UL;
			break;

		case WM_SYSKEYDOWN:
			g_wWinKey = wParam;
			break;

		case WM_KEYDOWN:
			g_wWinKey = wParam;
			if((int)wParam==VK_ESCAPE)
			{
				if(g_EscapeValue) *(DWORD*)g_EscapeValue=1;
				if(g_pGlob->bEscapeKeyEnabled)
				{
					#ifdef DARKSDK_COMPILE
					g_iDarkGameSDKQuit = 1;
					#endif
					PostQuitMessage(0);
				}
			}
			return TRUE;

		case WM_SYSKEYUP:
			g_wWinKey=0; 
			return TRUE;

		case WM_KEYUP:
			g_cInkeyCodeKey=0;
			g_wWinKey=0;
			return TRUE;

		case WM_CHAR:

			// If win string cleared externally (InputDLL)
			if(g_pGlob->pWindowsTextEntry)
				if(g_pGlob->pWindowsTextEntry[0]==0)
					g_dwWindowsTextEntryPos=0;

			// Key that was pressed
			g_cKeyPressed = (unsigned char)wParam;
			g_cInkeyCodeKey = g_cKeyPressed;
			return TRUE;

			// windows text entry is handled in Entry() now as it gets the info from IDE
			// Ensure string is always big enough
			if(g_pGlob->pWindowsTextEntry==NULL)
			{
				g_dwWindowsTextEntrySize = 32;
				g_pGlob->pWindowsTextEntry = new char[g_dwWindowsTextEntrySize];
				g_dwWindowsTextEntryPos = 0;
			}
			if(g_dwWindowsTextEntryPos>g_dwWindowsTextEntrySize-4)
			{
				g_dwWindowsTextEntrySize = g_dwWindowsTextEntrySize * 2;
				LPSTR pNewString = new char[g_dwWindowsTextEntrySize];
				strcpy(pNewString, g_pGlob->pWindowsTextEntry);
				delete[] g_pGlob->pWindowsTextEntry;
				g_pGlob->pWindowsTextEntry=pNewString;
			}

			// Add character to entry string
			g_pGlob->pWindowsTextEntry[g_dwWindowsTextEntryPos]=g_cKeyPressed;
			g_dwWindowsTextEntryPos++;
			g_pGlob->pWindowsTextEntry[g_dwWindowsTextEntryPos]=0;

			return TRUE;

		case WM_USER+1: // Show/Hide Cursor
			if(wParam==0) ShowCursor(FALSE);
			if(wParam==1) ShowCursor(TRUE);
			return TRUE;
	}
	
	// Default Action
	return DefWindowProc(hWnd, message, wParam, lParam);
}

DARKSDK void InternalClearWindowsEntry(void)
{
	if(g_pGlob->pWindowsTextEntry)
	{
		strcpy(g_pGlob->pWindowsTextEntry,"");
		g_dwWindowsTextEntryPos		= 0;
		g_cKeyPressed				= 0;
	}
}

DARKSDK DWORD InternalProcessMessages(void)
{
	DWORD dwResult = ProcessMessagesOnly();
	ExternalDisplayUpdate();
	return dwResult;
}

DARKSDK DWORD ProcessMessages(DWORD dwPositionInMachineCode)
{
	// Process Messages from a program in debug mode
	DWORD dwReturnValue=0;

	// When breakout position filled, leave immediately
	if(g_BreakOutPosition)
	{
		// If Exit requested, store position before leave
		if(*(DWORD*)g_BreakOutPosition==1)
		{
			*(DWORD*)g_BreakOutPosition=dwPositionInMachineCode;
			return 1;
		}
	}

	// Process Internal Message Loop
	dwReturnValue = InternalProcessMessages();

	// Return Value
	return dwReturnValue;
}

DARKSDK DWORD ProcessMessages(void)
{
	// Process Messages from a program in fullspeed mode
	return InternalProcessMessages();
}

DARKSDK DWORD Quit(void)
{
	// Initate Cascade Quit
	g_bCascadeQuitFlag=true;
	if(g_EscapeValue) *(DWORD*)g_EscapeValue=2;

	// Process any other tasks during Final QUIT
	if(g_pGlob->pExitPromptString)
	{
		// Produce an Exit Window with Strings
		MessageBox(NULL, g_pGlob->pExitPromptString, g_pGlob->pExitPromptString2, MB_OK);

		// Free Strings
		SAFE_DELETE(g_pGlob->pExitPromptString);
		SAFE_DELETE(g_pGlob->pExitPromptString2);
	}

	// Complete
	return 0;
}

DARKSDK void StackSnapshotStore(DWORD dwStackPositionNow)
{
	// No Stack save - data would be useless when new m/c executed
}

DARKSDK DWORD StackSnapshotRestore(void)
{
	// No Stack save - data would be useless when new m/c executed
	return 0;
}

// aaron - 20120811 - more flexible memory management routine
// p: in pointer (nullptr: alloc; else: realloc)
// n: size to have (0: free)
DB_EXPORT void *ManageMemory(void *p, size_t n) {
	void *q;

	if (n) {
		if (p)
			q = realloc(p, n);
		else
			q = malloc(n);
	} else {
		if (p)
			free(p);

		return nullptr;
	}

	return q;
}

DARKSDK int TestMemory ( int iSizeInBytes )
{
	try
	{
		void* pMem = new char[iSizeInBytes];
		if ( pMem )
		{
			// can still reserve memory chunk
			delete pMem;
			return 1;
		}
		else
			return 0;
	}
	catch(...)
	{
		return 0;
	}
	return 0;
}

DARKSDK void CreateSingleString(DWORD* dwVariableSpaceAddress, DWORD dwSize)
{
	if(dwSize>0)
	{
		// Create a core string
		*dwVariableSpaceAddress = (DWORD)new char[dwSize];
	}
	else
	{
		// Delete a core string
		delete[] (LPSTR)*dwVariableSpaceAddress;
	}
}

DARKSDK void Break(void)
{
	// Set Escape Value to Break Into Debugger
	if(g_EscapeValue) *(DWORD*)g_EscapeValue=1;
}

DARKSDK LRESULT SendDataToDebugger(int iType, LPSTR pData, DWORD dwDataSize)
{
	LRESULT lResult=0;

	// Create Virtual File for Transfer
	HANDLE hFileMap = CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,dwDataSize,"DBPROEDITORMESSAGE");
	if(hFileMap)
	{
		LPVOID lpVoid = MapViewOfFile(hFileMap,FILE_MAP_WRITE,0,0,dwDataSize+4);
		if(lpVoid)
		{
			// Copy to Virtual File
			*(DWORD*)lpVoid = dwDataSize;
			memcpy((LPSTR)lpVoid+4, pData, dwDataSize);

			// Find Debugger to send to
			HWND hWnd = FindWindow(NULL,"DBProDebugger");
			if(hWnd)
			{
				// Found - transmit
				lResult = SendMessage(hWnd, WM_USER+10, iType, 0);
			}

			// Release virtual file
			UnmapViewOfFile(lpVoid);
		}
		CloseHandle(hFileMap);
	}

	// May have result
	return lResult;
}

DARKSDK void BreakS(DWORD pString)
{
	// Send String to CLI Debug Console
	LPSTR lpReturnError = new char[1024];
	wsprintf(lpReturnError, "%s", pString);
	SendDataToDebugger(31, lpReturnError, strlen(lpReturnError));
	delete[] lpReturnError;
	lpReturnError=NULL;

	// Set Escape Value to Break Into Debugger
	if(g_EscapeValue) *(DWORD*)g_EscapeValue=1;
}

DARKSDK bool COREDoesFileExist(LPSTR Filename)
{
	// success or failure
	bool bSuccess = true;

	// open File To See If Exist
	HANDLE hfile = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hfile==INVALID_HANDLE_VALUE)
		bSuccess=false;
	else
		CloseHandle(hfile);

	// return result
	return bSuccess;
}

DARKSDK void UpdateFilenameFromVirtualTable( DWORD dwStringAddress )
{
	// String is input with external filename
	if(dwStringAddress==0)
		return;
	
	// If Virtual Table area available
	if(g_pGlob->pEXEUnpackDirectory==NULL)
		return;

	// Construct path to virtual file (if it is there or not) leefix - 200704 - can get very big!
	LPSTR pFilename = new char[_MAX_PATH*3];
	strcpy(pFilename, g_pGlob->pEXEUnpackDirectory);
	strcat(pFilename, "\\media\\");
	strcat(pFilename, (LPSTR)dwStringAddress);

	// If File exists, use that instead of external file
	if(COREDoesFileExist(pFilename)==true)
	{
		// Virtual Table File better than local external file
		strcpy((LPSTR)dwStringAddress, pFilename);
	}

	// Free usages
	delete[] pFilename;
}

DARKSDK LPSTR ReadFileData(LPSTR FilenameString, DWORD* dwDataSize)
{
	// Read File Data
	HANDLE hreadfile = CreateFile(FilenameString, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hreadfile==INVALID_HANDLE_VALUE)
		return NULL;

	// Read readout file into memory
	DWORD bytesread=0;
	DWORD filebuffersize = GetFileSize(hreadfile, NULL);	
	LPSTR filebuffer = (char*)GlobalAlloc(GMEM_FIXED, filebuffersize);
	ReadFile(hreadfile, filebuffer, filebuffersize, &bytesread, NULL); 
	CloseHandle(hreadfile);

	*dwDataSize = filebuffersize;
	return filebuffer;
}

DARKSDK void WriteFileData(LPSTR pFilename, LPSTR pData, DWORD dwDataSize)
{
	// Delete existing file
	DeleteFile(pFilename);

	// Write New File with new data
	DWORD byteswritten=0;
	HANDLE hfile = CreateFile(pFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hfile!=INVALID_HANDLE_VALUE)
	{
		WriteFile(hfile, pData, dwDataSize, &byteswritten, NULL); 
		CloseHandle(hfile);
	}
}

DARKSDK void EncryptDecrypt( DWORD dwStringAddress, bool bEncryptIfTrue, bool bDoNotUseTempFolder )
{
	CallEncryptDecrypt ( dwStringAddress, bEncryptIfTrue, bDoNotUseTempFolder );
}

DARKSDK void Decrypt( DWORD dwStringAddress )
{
	// lee - 230306 - u6b4 - only encrypt/decrypt if from DBPDATA temp folder (should not touch local files!)
	// Dave - 28/03/2014 - commented this out because we will check if a file needs decrypting from now on
	EncryptDecrypt ( dwStringAddress, false, false );
}

DARKSDK void Encrypt( DWORD dwStringAddress )
{
	// lee - 230306 - u6b4 - only encrypt/decrypt if from DBPDATA temp folder (should not touch local files!)
	// Dave - 28/03/2014 - commented this out because we will check if a file needs decrypting from now on
	EncryptDecrypt ( dwStringAddress, true, false );
}

DARKSDK void EncryptDBPro ( DWORD dwStringAddress )
{
	LPSTR pFilename = new char[_MAX_PATH];
	strcpy(pFilename, (LPSTR)dwStringAddress);
	if(!COREDoesFileExist(pFilename))
		return;

	char newFileName[_MAX_PATH];
	sprintf ( newFileName , "_e_%s" , (LPSTR)dwStringAddress );

	char buf[BUFSIZ];
	size_t size;

	FILE* source = fopen( (LPSTR)dwStringAddress , "rb");
	FILE* dest = fopen(newFileName, "wb");

	// clean and more secure
	// feof(FILE* stream) returns non-zero if the end of file indicator for stream is set

	while (size = fread(buf, 1, BUFSIZ, source))
	{
		fwrite(buf, 1, size, dest);
	}

	fclose(source);
	fclose(dest);

	EncryptDecrypt ( (DWORD)newFileName, true, true );
}

DARKSDK void EncryptWorkshopDBPro ( char* dwStringAddress )
{
	LPSTR pFilename = new char[_MAX_PATH];
	strcpy(pFilename, (LPSTR)dwStringAddress);
	if(!COREDoesFileExist(pFilename))
		return;

	char originalPath[MAX_PATH];
	GetCurrentDirectory ( MAX_PATH, originalPath );

	char* pLocalFile = NULL;
	char filePath[MAX_PATH];
	strcpy( filePath, pFilename );
	pLocalFile = strrchr ( pFilename , '\\' );
	if ( pLocalFile )
	{	
		strcpy ( pFilename, pLocalFile+1 );
		pLocalFile = strrchr ( filePath , '\\' );
		pLocalFile[0] = '\0';
		SetCurrentDirectory ( filePath );
	}

	char newFileName[_MAX_PATH];
	sprintf ( newFileName , "_w_%s" , pFilename );

	char buf[BUFSIZ];
	size_t size;

	FILE* source = fopen( pFilename , "rb");
	FILE* dest = fopen(newFileName, "wb");

	// clean and more secure
	while (size = fread(buf, 1, BUFSIZ, source))
	{
		fwrite(buf, 1, size, dest);
	}

	fclose(source);
	fclose(dest);

	EncryptDecrypt ( (DWORD)newFileName, true, true );

	SetCurrentDirectory(originalPath);
}

DARKSDK bool EncryptNewFile ( DWORD dwStringAddress )
{
	// do not encrypt any sky models (as they use internal image loads which are also encrypted 
	// and I cannot load encryped files from within the temp folder where the decrypted.x is)
	LPSTR pScanFilename = (LPSTR)dwStringAddress;
	char pThisDirAndFile[MAX_PATH];
	GetCurrentDirectory ( MAX_PATH, pThisDirAndFile );
	strcat ( pThisDirAndFile, "\\" );
	strcat ( pThisDirAndFile, pScanFilename );
	int iScanMax = strlen(pThisDirAndFile)-8;
	if ( iScanMax < 0 ) iScanMax = 0;
	if ( strlen ( pThisDirAndFile ) > 8 )
	{
		for ( int n=0; n<iScanMax; n++ )
		{
			if ( strnicmp ( pThisDirAndFile + n, "skybank\\", 8 )==NULL || strnicmp ( pThisDirAndFile + n, "skybank/", 8 )==NULL )
			{
				if ( strnicmp ( pThisDirAndFile + n, "skybank\\", 8 )==NULL || strnicmp ( pThisDirAndFile + n, "skybank/", 8 )==NULL )
				{
					if ( strnicmp ( pThisDirAndFile + strlen(pThisDirAndFile) - 2, ".x", 2 )==NULL )
					{
						return false;
					}
				}
			}
		}
	}

	char newFileName[_MAX_PATH];
	sprintf ( newFileName , "_e_%s" , (LPSTR)dwStringAddress );

	char buf[BUFSIZ];
	size_t size;

	FILE* source = fopen( (LPSTR)dwStringAddress , "rb");
	FILE* dest = fopen(newFileName, "wb");

	// clean and more secure
	while (size = fread(buf, 1, BUFSIZ, source))
	{
		fwrite(buf, 1, size, dest);
	}

	fclose(source);
	fclose(dest);

	EncryptDecrypt ( (DWORD)newFileName, true, true );

	return true;
}

// Delete any empty folders
DARKSDK void EncryptAllFiles(char* dwStringAddress)
{
	LPSTR pFilename = new char[_MAX_PATH];
	strcpy(pFilename, dwStringAddress);

	HANDLE			hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA data  = { 0 };
    
	std::stack  < char* > directoryListStack;

	char folderToCheck[MAX_PATH];
	sprintf ( folderToCheck , pFilename );

	// add first directory into the listing
	directoryListStack.push ( folderToCheck );

	// Added Code to pre-compile Lua scripts
	LoadLua("ggprecompile.lua");

	// keep going until we have emptied the directory stack
	while ( !directoryListStack.empty ( ) )
	{
		// get the first directory
		char  szLocation [ 256 ] = "";
		char* szCurrentDirectory = directoryListStack.top ( );
		
		// now add this to the location to check plus no mask so we search for everything
		sprintf ( szLocation, "%s\\*.*", szCurrentDirectory );

		// pop this directory off the stack
		directoryListStack.pop ( );

		// find the first file in the location
		hFind = FindFirstFile ( szLocation, &data );

		// break if nothing is there
		if ( hFind == INVALID_HANDLE_VALUE )
			break;

		// cycle through all files
		do 
		{
			// only proceed if it's not . or ..
			if ( strcmp ( data.cFileName, "." ) != 0 && strcmp ( data.cFileName, ".." ) != 0 )
			{
				// deal with a directory
				if ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					// add this directory onto the stack
					char* p = new char [ 256 ];
					
					sprintf ( p, "%s\\%s", szCurrentDirectory, data.cFileName );
					
					directoryListStack.push ( p );
				}
				else
				{
					if ( strstr(data.cFileName, ".fpe") != NULL || 
						 strstr(data.cFileName, ".dds") != NULL ||  
						 strstr(data.cFileName, ".png") != NULL || 
						 strstr(data.cFileName, ".jpg") != NULL ||
						 strstr(data.cFileName, ".x")   != NULL ||
						 strstr(data.cFileName, ".dbo") != NULL ||
						 strstr(data.cFileName, ".wav") != NULL ||
						 strstr(data.cFileName, ".mp3") != NULL )
					{
						// dont encrypt a file if it already is
						if ( strstr ( data.cFileName, "_e_" )  !=  data.cFileName )
						{
							// encrypt the file
							char p[ MAX_PATH ];
							char f[ MAX_PATH ];
					
							sprintf ( p, "%s\\", szCurrentDirectory );
							strcpy ( f , data.cFileName );

							char originalFolder[MAX_PATH];
							GetCurrentDirectory ( MAX_PATH, originalFolder );						
							SetCurrentDirectory ( szCurrentDirectory );
							bool bEncryptedOkay = EncryptNewFile( (DWORD)f );
							SetCurrentDirectory ( originalFolder );
							UpdateWindow ( NULL );
							sprintf ( p, "%s\\%s", szCurrentDirectory , f );
							if ( bEncryptedOkay==true ) DeleteFile ( p );
						}
					}
					else 
					{
						if ( strstr(data.cFileName, ".lua") != NULL &&
							 strstr(data.cFileName, "multiplayer") == NULL )
						{
							// Precompile lua script, note: overwrites file, in theory if the call
							// fails for any reason the file should remain uncompiled.
							char p[MAX_PATH];

							sprintf(p, "%s\\%s", szCurrentDirectory, data.cFileName);

							LuaSetFunction("ggprecompile", 1, 0);
							LuaPushString(p);
							LuaCall();
						}
					}
				}
			}
		}
		while ( FindNextFile ( hFind, &data ) != 0 );

		// now break out if needed
		if ( GetLastError ( ) != ERROR_NO_MORE_FILES )
		{
			FindClose ( hFind );
			break;
		}

		FindClose ( hFind );
		hFind = INVALID_HANDLE_VALUE;
	}
}

DARKSDK void ConstructPostDisplayItems(HINSTANCE hInstance)
{
	SETUPPassCoreData((LPVOID)g_pGlob, 1);
	Basic2DConstructor();
	TextConstructor();
	ImageConstructor();
	SpritesConstructor();
	InfiniteVegetationConstructor();
}

DARKSDK void ConstructPostDLLItems(HINSTANCE hInstance)
{
	InputConstructor();
	SystemConstructor();
	SoundConstructor();
	FileConstructor();
	FTPConstructor();
	MemblocksConstructor();
	AnimationConstructor();
	BitmapConstructor();
	CameraConstructor();
	LightConstructor();
	Basic3DConstructor();
	VectorConstructor();
	BULLETReceiveCoreDataPtr();
	OccluderConstructor();
	LuaConstructor();
	BTConstructor();
}

DARKSDK void PassCmdLineHandlerPtr(LPVOID pCmdLinePtr)
{
	// Store pointer to command line string passed into EXE
	g_pCommandLineString = (LPSTR)pCmdLinePtr;
}

DARKSDK void PassErrorHandlerPtr(LPVOID pErrorPtr)
{
	// Store position of runtime error DWORD (held in executable dataspace)
	g_ErrorHandler = pErrorPtr;
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorPtr;

	// LEEMOD - 150803 - Also store reference in GLOBSTRUCT for ThirdPartyDLLs
	g_pGlob->g_pErrorHandlerRef = pErrorPtr;

	// Clear error clue
	strcpy ( g_strErrorClue, "" );
}

DARKSDK void ChangeMouse( DWORD dwCursorID )
{
	// Set Cursor Shape (0-31)
	if ( dwCursorID==0 ) g_ActiveCursor=g_hUseArrow;
	if ( dwCursorID==1 ) g_ActiveCursor=g_hUseHourglass;
	if ( dwCursorID>=2 && dwCursorID<=31 ) g_ActiveCursor=g_hCustomCursors[dwCursorID-2];
	if ( dwCursorID==32 ) g_ActiveCursor=NULL;
	if ( dwCursorID<=31 )
	{
		// change cursor
		SetCursor ( g_ActiveCursor );
	}
}

DARKSDK DWORD InitDisplayEx(DWORD dwDisplayType, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, HINSTANCE hInstance, LPSTR pApplicationName, HWND pParentHWND, DWORD dwInExStyle, DWORD dwInStyle)
{
	// dwDisplayType
	// =============
	// 0=Hidden Mode
	// 1=Window Mode
	// 2=Desktop Fullscreen Mode
	// 3=Exclusive Fullscreen Mode
	// 4=Desktop Fullscreen Mode (No Taskbar)
	// 5=Use EX and STYLE from values passed in

	// System Settings
	g_dwScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	g_dwScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Window Default Settings
	bool bWindowIsDisplayable=true;
	g_pGlob->dwWindowX = 0;
	g_pGlob->dwWindowY = 0;

	// Apply size of screen to global data
	g_pGlob->dwWindowWidth = dwWidth;
	g_pGlob->dwWindowHeight = dwHeight;
	g_pGlob->iScreenWidth = dwWidth;
	g_pGlob->iScreenHeight = dwHeight;
	g_pGlob->iScreenDepth = dwDepth;

	if(g_pGlob->g_GFX==NULL)
	{
		// Using GDI for Display
		if(dwDisplayType==2)
		{
			// Fullscreen Mode - With Taskbar
			RECT rc;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
			g_pGlob->dwWindowWidth = rc.right-rc.left;
			g_pGlob->dwWindowHeight = rc.bottom-rc.top;
		}
		if(dwDisplayType>=3)
		{
			// Fullscreen Mode - Simply Resize Window
			g_pGlob->dwWindowWidth = g_dwScreenWidth;
			g_pGlob->dwWindowHeight = g_dwScreenHeight;
		}
	}
	
	// Window Settings
	DWORD dwWindowStyle=0;
	DWORD dwWindowExStyle=0;
	switch(dwDisplayType)
	{
		// leechange - 101004 - should be FULL DESKTOP FULLSCREEN if made visible
		case 0 :	dwWindowStyle = WS_POPUP; // HIDDEN APP   // was WS_MINIMIZE;
					bWindowIsDisplayable=false;
					break;

		case 1 :	dwWindowStyle = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU; // WINDOW APP
					break;

		case 2 :	dwWindowStyle = WS_POPUP; // DESKTOP FULLSCREEN (see taskbar)
					break;

		case 3 :	dwWindowStyle = WS_POPUP; // EXCLUSIVE FULLSCREEN
					break;

		case 4 :	dwWindowStyle = WS_POPUP; // FULL DESKTOP FULLSCREEN (no taskbar)
					break;

		case 5 :	dwWindowStyle = dwInStyle;	// PASSED IN USING DARKGDKINIT
					dwWindowExStyle = dwInExStyle;
					break;

		case 6 :	dwWindowStyle = dwInStyle;	// PASSED IN USING DARKGDKINIT (HIDDEN)
					dwWindowExStyle = dwInExStyle;
					bWindowIsDisplayable=false;
					break;
	}

	// Icons and Cursors
	g_hUseIcon = (HICON)LoadImageA(hInstance, "icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	
	// Load Custom Cursors (first slot is ARROW, second is WAIT and third onwards is own)
	HCURSOR hCursor = NULL;
	hCursor = (HCURSOR)LoadImageA(hInstance, "arrow.cur", IMAGE_CURSOR, 32, 32, LR_LOADFROMFILE);
	if (hCursor) g_hUseArrow=hCursor;
	hCursor = (HCURSOR)LoadImageA(hInstance, "hourglass.cur", IMAGE_CURSOR, 32, 32, LR_LOADFROMFILE);
	if (hCursor) g_hUseHourglass=hCursor;
	for ( DWORD c=2; c<32; c++)
	{
		char str[_MAX_PATH];
		wsprintf(str, "pointer%d.cur", c);
		hCursor = (HCURSOR)LoadImageA(hInstance, str, IMAGE_CURSOR, 32, 32, LR_LOADFROMFILE);
		g_hCustomCursors[c-2]=hCursor;
	}

	// Use Default Cursor otherwise
	if(g_hUseArrow==NULL) g_hUseArrow = LoadCursor(NULL, IDC_ARROW);
	if(g_hUseHourglass==NULL) g_hUseHourglass = LoadCursor(NULL, IDC_WAIT);

	// Vars
	WNDCLASS wc;

	// Appname
	char pAppName[256];
	char pAppNameUnique[256];

	//PE: Use the exe filenane as the title in the game.
	//PE: So if you use Test-my-Game_name.exe as the standalone
	//PE: the windows title will be "Test my Game name".
	char workstring[1024];
	GetModuleFileName(NULL, workstring, 1024);
	if (strcmp(Lower(Right(workstring, 18)), "guru-mapeditor.exe") == 0 )
	{
		strcpy(pAppName, "Game Guru");
	}
	else 
	{
		strcpy(pAppName, "Game Guru");
		TCHAR * out;
		out = PathFindFileName(workstring);
		if (out != NULL) 
		{
			*(PathFindExtension(out)) = 0;
			for (int i = strlen(out); i > 0; i--) {
				if (out[i] == '-') out[i] = ' ';
				if (out[i] == '_') out[i] = ' ';
			}
			if (strlen(out) > 0)
				strcpy(pAppName, out);
		}
	}

	// Extract path from ModuleFileName so can locate any DLLs in GG Root Folder
	char pRootPath[1024];
	strcpy ( pRootPath, workstring );
	for ( int n = strlen(pRootPath); n > 0; n-- )
	{
		if ( pRootPath[n] == '\\' || pRootPath[n] == '/' )
		{
			pRootPath[n] = 0;
			break;
		}
	}

	// this ensures no conflict between window class name and application class name
	strcpy ( pAppNameUnique, pAppName );
	strcat ( pAppNameUnique, "12345" );

	// Register window
	if ( g_pGlob->hWnd==NULL )
	{
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = g_hUseIcon;
		wc.hCursor = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = pAppNameUnique;
		RegisterClass( &wc );
	}

	// Icon Set Manually (also in winproc too - for cursor restore control)
	g_ActiveCursor = g_hUseArrow;
	g_OldCursor = SetCursor ( g_ActiveCursor );

	// If running in window mode, start in center of screen
	if ( dwDisplayType==1 )
	{
		g_pGlob->dwWindowX=(GetSystemMetrics(SM_CXSCREEN)-g_pGlob->dwWindowWidth)/2;
		g_pGlob->dwWindowY=(GetSystemMetrics(SM_CYSCREEN)-g_pGlob->dwWindowHeight)/2;
	}

	// Create Window (if one not already created)
	g_pGlob->hInstance = hInstance;
	if ( g_pGlob->hWnd )
	{
		// override window handle with new winproc
		SetWindowLong ( g_pGlob->hWnd, GWL_WNDPROC, (LONG)WindowProc );
	}
	else
	{
		/* from sample
		g_pGlob->hWnd = CreateWindow(pAppNameUnique,
									pAppName,
									WS_OVERLAPPEDWINDOW,
									CW_USEDEFAULT,
									0,
									CW_USEDEFAULT,
									0,
									nullptr,
									nullptr,
									hInstance, 
									nullptr);
		*/
		g_pGlob->hWnd = CreateWindow( pAppNameUnique, pAppName, dwWindowStyle, g_pGlob->dwWindowX, g_pGlob->dwWindowY, g_pGlob->dwWindowWidth, g_pGlob->dwWindowHeight, NULL, NULL, hInstance, NULL);
	}

	// Init Steam API
	SteamInit();

	// Main Setup init
	g_pGlob->hOriginalhWnd = g_pGlob->hWnd;
	bool bDXFailed=false;
	if ( SETUPConstructor() == true)
	{
		SETUPPassCoreData(g_pGlob, 0);
		if(m_pDX==NULL) bDXFailed=true;
	}
	else
		bDXFailed=true;

	// Show Window
	ShowWindow(g_pGlob->hWnd, SW_SHOW);

	// Initialise DisplayDLL
	OverrideHWND(g_pGlob->hWnd);

	// Activate COM
	//CoInitialize(NULL);

	// Create Display (dwAppDisplayModeUsing controls window handler)
	g_pGlob->dwAppDisplayModeUsing=dwDisplayType;
	CreateDisplay(dwDisplayType);

	// Can fail to create starter resolution
	if(*(DWORD*)g_ErrorHandler>0) return 1;

	// Assign Function Ptrs to Glob (for other DLLs to use)
	g_pGlob->CreateDeleteString = CreateSingleString;
	g_pGlob->ProcessMessageFunction = ProcessMessagesOnly;
	g_pGlob->PrintStringFunction = PrintString;
	g_pGlob->UpdateFilenameFromVirtualTable = UpdateFilenameFromVirtualTable;
	g_pGlob->Decrypt = Decrypt;
	g_pGlob->Encrypt = Encrypt;
	g_pGlob->ChangeMouseFunction = ChangeMouse;

	// Load External DLL Displayer
	ConstructPostDisplayItems(hInstance);

	// Prepare Other DLLs
	ConstructPostDLLItems(hInstance);

	// Visible Window
	if(bWindowIsDisplayable)
	{
		// Clear Display Area
		ClearPrintArea();

		// Clear Screen
		InvalidateRect(g_pGlob->hWnd, NULL, TRUE);
		UpdateWindow(g_pGlob->hWnd);

		// Reveal Window
		ShowWindow(g_pGlob->hWnd, SW_SHOW);
	}

	// Set Any After Display Properties (ink, font, etc)
	SetDefaultDisplayProperties();

	// Process any messages prior to program start (also for begin scene call)
	InternalProcessMessages();

	// complete
	return 0;
}

DARKSDK DWORD InitDisplay(DWORD dwDisplayType, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, HINSTANCE hInstance, LPSTR pApplicationName)
{
	return InitDisplayEx(dwDisplayType, dwWidth, dwHeight, dwDepth, hInstance, pApplicationName, NULL, 0, 0);
}

DARKSDK void SetRenderOrderList(void)
{
	CreateRenderOrderList();
}

DARKSDK void ConstructDLLs(void)
{
	// Prepare Other DLLs
	ConstructPostDisplayItems(g_pGlob->hInstance);
	ConstructPostDLLItems(g_pGlob->hInstance);
}

DARKSDK int GetSecurityCode(void)
{	
	// gewnerate once
	srand((int)timeGetTime());
	if ( g_iSecurityCode!=-1 )
	{
		int iSecurityCode = rand()%1000000;
		if ( g_iSecurityCode==0 ) g_iSecurityCode = iSecurityCode;
	}
	return g_iSecurityCode;
}

DARKSDK void WipeSecurityCode(void)
{
	// clear forever
	g_iSecurityCode=-1;
}

DARKSDK DWORD GetGlobPtr(void)
{
	return (DWORD)g_pGlob;
}

DARKSDK void FreeChecklistStrings(void)
{
	// Free checklist strings
	for(DWORD c=0; c<g_pGlob->dwChecklistArraySize; c++)
		if(g_pGlob->checklist[c].string)
			SAFE_DELETE(g_pGlob->checklist[c].string);

	// Free main block
	if(g_pGlob->checklist)
	{
		g_pGlob->CreateDeleteString((DWORD*)&g_pGlob->checklist, 0);
		g_pGlob->checklist=NULL;
	}
}

DARKSDK DWORD CloseDisplay(void)
{
	// Free checklist strings
	FreeChecklistStrings();

	// Restore Display to Windowed Mode
	DeleteDisplay();

	// Free default input resources
	if(g_pGlob->pWindowsTextEntry)
	{
		delete[] g_pGlob->pWindowsTextEntry;
		g_pGlob->pWindowsTextEntry=NULL;
	}

	// Free safe rects arrays
	SAFE_DELETE ( g_pGlob->pSafeRects );

	// Close Window
	if(g_pGlob->hWnd)
	{
		ShowWindow ( g_pGlob->hWnd, SW_HIDE );
		CloseWindow(g_pGlob->hWnd);
		g_pGlob->hWnd=NULL;
	}

	// Free Cursors and Icons
	if(g_hUseIcon) DestroyIcon(g_hUseIcon);
	if(g_hUseArrow) DestroyCursor(g_hUseArrow);
	if(g_hUseHourglass) DestroyCursor(g_hUseHourglass);

	// Free COM
	CoUninitialize();

	// Complete
	return 0;
}

// ABSOLUTE BASIC COMMANDS (PRINT and INPUT)

DARKSDK void Cls(void)
{
	ClearPrintArea();
	SetPrintCursor(0,0);
}
DARKSDK void SetCursor(int iX, int iY)
{
	SetPrintCursor(iX,iY);
}
DARKSDK void Print(LONGLONG lValue)
{
	PrintInteger(lValue, true);
}
DARKSDK void Print(double dValue)
{
	PrintFloat(dValue, true);
}
DARKSDK void Print(LPSTR pString)
{
	PrintString(pString, true);
}
DARKSDK void Print(void)
{
	PrintNothing();
}
DARKSDK void PrintConcat(LONGLONG lValue)
{
	PrintInteger(lValue, false);
}
DARKSDK void PrintConcat(double dValue)
{
	PrintFloat(dValue, false);
}
DARKSDK void PrintConcat(LPSTR pString)
{
	PrintString(pString, false);
}

DARKSDK LONGLONG PerformanceTimer ( void )
{
	LARGE_INTEGER large;
	if (!QueryPerformanceCounter ( &large ))
	{
		large.QuadPart = 0;
	};
	return large.QuadPart;
}

DARKSDK LONGLONG PerformanceFrequency ( void )
{
	LARGE_INTEGER large;
	if (! QueryPerformanceFrequency( &large ))
	{
		large.QuadPart = 0;
	}
	return large.QuadPart;
}

DARKSDK float timeGetSecond(void)
{
	LARGE_INTEGER large;
	if (!QueryPerformanceCounter ( &large ))
	{
		large.QuadPart = 0;
	};
	if ( g_lFirstPerfTime == 0 ) g_lFirstPerfTime = large.QuadPart;
	LONGLONG lTimer = large.QuadPart;
	if (! QueryPerformanceFrequency( &large ))
	{
		large.QuadPart = 0;
	}
	float fTime = (float)(lTimer-g_lFirstPerfTime) / large.QuadPart;
	return fTime;
}

DARKSDK LONGLONG InputR(void)
{
	LONGLONG lValue;
	InputInteger(&lValue);
	return lValue;
}
DARKSDK double InputO(void)
{
	double dValue;
	InputFloat(&dValue);
	return dValue;
}
DARKSDK DWORD InputS(DWORD pDestStr)
{
	if(pDestStr) delete (LPSTR)pDestStr;

	LPSTR pString=NULL;
	InputString(&pString);
	return (DWORD)pString;
}

// MEMORY MANAGEMENT FUNCTIONS

DARKSDK DWORD CreateVariableSpace(DWORD VariableSpaceSize)
{
	// Create Variable Space
	g_pVarSpace = (LPSTR)GlobalAlloc(GMEM_FIXED, VariableSpaceSize);
	g_pGlob->g_pVariableSpace = (LPVOID)g_pVarSpace;
	return (DWORD)g_pVarSpace;
}

DARKSDK DWORD CreateDataSpace(DWORD DataSpaceSize)
{
	// Create Data Space
	g_pDataSpace = (LPSTR)GlobalAlloc(GMEM_FIXED, DataSpaceSize);
	return (DWORD)g_pDataSpace;
}

DARKSDK void DeleteVariableSpace(void)
{
	// Delete Variable Space Itself
	SAFE_FREE(g_pVarSpace);
}

DARKSDK void DeleteDataSpace(void)
{
	// Delete Data Space Itself
	SAFE_FREE(g_pDataSpace);
}

DARKSDK void DeleteSingleVariableAllocation(DWORD* dwVariableSpaceAddress)
{
	// Delete Actual Allocation within Variable Space (no need to clear)
	if(dwVariableSpaceAddress)
	{
		delete dwVariableSpaceAddress;
	}
}

DARKSDK DWORD CreateArray(DWORD dwSizeOfArray, DWORD dwSizeOfOneDataItem, DWORD dwTypeValueOfOneDataItem)
{
	// Calculate Total Size of Array
	DWORD dwHeaderSizeInBytes = HEADERSIZEINBYTES;
	DWORD dwDimSizeBytes = 40;
	DWORD dwRefSizeInBytes = dwSizeOfArray * 4;
	DWORD dwFlagSizeInBytes = dwSizeOfArray * 1;
	DWORD dwDataSizeInBytes = dwSizeOfArray * dwSizeOfOneDataItem;

	// Total Size
	DWORD dwTotalSize = dwHeaderSizeInBytes + dwRefSizeInBytes + dwFlagSizeInBytes + dwDataSizeInBytes;

	// Error Trap for debug to discover larger chunk allocations mid-app activity (fragmentation danger)
	if ( dwTotalSize > 1024*1000*4 )
	{
		// can put breakpoint here when checking for large allocations mid-flow
		int iMB = dwTotalSize / 1024 / 1000;
		int stopwhen4megallocated = 42;
	}

	// Create Array Memory
	LPSTR pArrayPtr = new char[dwTotalSize];
	memset(pArrayPtr, 0, sizeof(pArrayPtr));

	// Derive Pointers into Array
	DWORD* pHeader	= (DWORD*)(pArrayPtr);
	DWORD* pRef		= (DWORD*)(pArrayPtr+dwHeaderSizeInBytes);
	LPSTR  pFlag	= (LPSTR )(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes);
	LPSTR  pData	= (LPSTR )(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes+dwFlagSizeInBytes);

	// Create Header
	for(DWORD d=0; d<=9; d++) pHeader[0]=0;
	pHeader[10]=dwSizeOfArray;
	pHeader[11]=dwSizeOfOneDataItem;
	pHeader[12]=dwTypeValueOfOneDataItem;
	pHeader[13]=0;

	// Create Ref Table
	LPSTR pDataPointer = pData;
	for(DWORD r=0; r<dwSizeOfArray; r++)
	{
		pRef[r] = (DWORD)pDataPointer;
		pDataPointer+=dwSizeOfOneDataItem;
	}

	// Create DataBlockFlag Table (all flags to 1)
	memset(pFlag, 1, dwSizeOfArray);

	// Clear DataBlock Memory
	DWORD dwTotalDataSize = dwSizeOfArray * dwSizeOfOneDataItem;
	memset(pData, 0, dwTotalDataSize);

	// Advance ArrayPtr to First Byte in RefTable
	pArrayPtr+=dwHeaderSizeInBytes;

	// Return ArrayPtr
	return (DWORD)pArrayPtr;
}

DARKSDK void FreeStringsFromArray(DWORD dwArrayPtr)
{
	// Get Array Information
	if ( dwArrayPtr )
	{
		DWORD dwTypeValueOfOneDataItem = *((DWORD*)dwArrayPtr-2);
		if ( dwTypeValueOfOneDataItem == 2 ) 
		{
			// only free strings if array holds string items
			DWORD dwSizeOfTable = *((DWORD*)dwArrayPtr-4);
			DWORD dwDataItemSize = *((DWORD*)dwArrayPtr-3);
			DWORD dwRefSizeInBytes = dwSizeOfTable * 4;
			DWORD dwFlagSizeInBytes = dwSizeOfTable * 1;
			LPSTR* pData = (LPSTR*)(((LPSTR)dwArrayPtr)+dwRefSizeInBytes+dwFlagSizeInBytes);
			for ( DWORD dwDataOffset=0; dwDataOffset<dwSizeOfTable; dwDataOffset++)
			{
				if ( pData [ dwDataOffset ] )
				{
					delete[] pData [ dwDataOffset ];
				}
			}
		}
		// Clear strings from UDT's
		else if (dwTypeValueOfOneDataItem >= 9)
		{
			// Grab a copy of the arrays format string
			LPSTR UdtFormat = GetTypePatternCore( NULL, dwTypeValueOfOneDataItem );

			// Search the format string to see if the UDT contains any strings
			bool ContainsString = false;
			for ( LPSTR CurrentItem = UdtFormat; *CurrentItem; ++CurrentItem )
			{
				if (*CurrentItem == 'S')
				{
					ContainsString = true;
					break;
				}
			}

			// If it does, loop through every UDT and release those strings
			if (ContainsString)
			{
				DWORD* ArrayPtr = (DWORD*)dwArrayPtr;
				DWORD  ArraySize = ArrayPtr[-4];

				for ( DWORD Position = 0; Position < ArraySize; ++Position )
				{
					DWORD ItemOffset = 0;
					for ( LPSTR CurrentItem = UdtFormat; *CurrentItem; ++CurrentItem )
					{
						if (*CurrentItem == 'S')
						{
							DWORD P = ArrayPtr[ Position ] + ItemOffset;
							delete[] *(LPSTR*)P;
							ItemOffset += 4;            // Strings are 4 bytes
						}
						else if (*CurrentItem == 'O' || *CurrentItem == 'R')
						{
							ItemOffset += 8;            // Double float/integer are 8 bytes
						}
						else
						{
							ItemOffset += 4;            // Everything else is 4 bytes
						}
					}
				}
			}

			// Release the copy of the arrays format string
			delete[] UdtFormat;
		}
	}
}

DARKSDK void DeleteArray(DWORD dwArrayPtr)
{
	// If Array exists
	if(dwArrayPtr)
	{
		// Array Ptr Skips Header
		dwArrayPtr-=HEADERSIZEINBYTES;

		// Delete Array Memory
		delete[] (DWORD*)dwArrayPtr;
	}
}

DARKSDK DWORD ExpandArray(DWORD dwOldArrayPtr, DWORD dwAddElements)
{
	// Get Old ArrayPtr
	LPSTR pOldArrayPtr = ((LPSTR)dwOldArrayPtr)-HEADERSIZEINBYTES;

	// Old Array Pointers and Data
	DWORD* pHeader	= (DWORD*)(pOldArrayPtr);
	DWORD dwHeaderSizeInBytes = HEADERSIZEINBYTES;

	// Extract header info
	DWORD dwOldSizeOfArray = pHeader[10];
	DWORD dwOldSizeOfOneDataItem = pHeader[11];
	DWORD dwOldTypeValueOfOneDataItem = pHeader[12];
	DWORD dwOldInternalIndex = pHeader[13];

	DWORD dwOldRefSizeInBytes = dwOldSizeOfArray * 4;
	DWORD dwOldFlagSizeInBytes = dwOldSizeOfArray * 1;
	DWORD dwOldDataSizeInBytes = dwOldSizeOfArray * dwOldSizeOfOneDataItem;
	DWORD* pOldRef = (DWORD*)(pOldArrayPtr+dwHeaderSizeInBytes);
	LPSTR pOldFlag = (LPSTR)(pOldArrayPtr+dwHeaderSizeInBytes+dwOldRefSizeInBytes);
	LPSTR pOldData = (LPSTR)(pOldArrayPtr+dwHeaderSizeInBytes+dwOldRefSizeInBytes+dwOldFlagSizeInBytes);

	// Create New Size of Array
	DWORD dwSizeOfArray = dwOldSizeOfArray + dwAddElements;
	LPSTR pArrayPtr = (LPSTR)CreateArray(dwSizeOfArray, dwOldSizeOfOneDataItem, dwOldTypeValueOfOneDataItem);

	// Return ptr to beginning of memory
	pArrayPtr = pArrayPtr - HEADERSIZEINBYTES;

	// Copy dimension-size block over (10xDWORD values)
	memcpy(pArrayPtr, pOldArrayPtr, 40);

	// Calculate Sizes of New Array
	DWORD dwRefSizeInBytes = dwSizeOfArray * 4;
	DWORD dwFlagSizeInBytes = dwSizeOfArray * 1;
	DWORD dwDataSizeInBytes = dwSizeOfArray * dwOldSizeOfOneDataItem;

	// Derive Pointers into New Array
	DWORD* pNewRef		= (DWORD*)(pArrayPtr+dwHeaderSizeInBytes);
	LPSTR  pNewFlag		= (LPSTR )(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes);
	LPSTR  pNewData		= (LPSTR )(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes+dwFlagSizeInBytes);

	// Clear new data and copy old data to it
	memset(pNewData, 0, dwDataSizeInBytes);
	memcpy(pNewData, pOldData, dwOldDataSizeInBytes);

	// Update New Array Refs from Old Array Refs
	for(DWORD i=0; i<dwOldSizeOfArray; i++)
	{
		DWORD dwOffset = (DWORD)(pOldRef[i]) - (DWORD)pOldData;
		pNewRef[i] = (DWORD)(pNewData + dwOffset);
	}

	// Copy flag states from old to new
	memcpy(pNewFlag, pOldFlag, dwOldFlagSizeInBytes);

	// Create flags for new part of array
	memset(pNewFlag+dwOldFlagSizeInBytes, 1, dwFlagSizeInBytes-dwOldFlagSizeInBytes);

	// Destroy old array
	DeleteArray(dwOldArrayPtr);

	// Advance ArrayPtr to First Byte in RefTable
	pArrayPtr+=dwHeaderSizeInBytes;

	// Return ArrayPtr
	return (DWORD)pArrayPtr;
}

DARKSDK void ClearDataBlock(DWORD dwArrayPtr, DWORD dwIndex, DWORD dwQuantity)
{
	DWORD dwSizeOfTable = *((DWORD*)dwArrayPtr-4);
	DWORD dwDataItemSize = *((DWORD*)dwArrayPtr-3);
	DWORD dwRefSizeInBytes = dwSizeOfTable * 4;
	DWORD dwFlagSizeInBytes = dwSizeOfTable * 1;
	LPSTR pData = (LPSTR)(((LPSTR)dwArrayPtr)+dwRefSizeInBytes+dwFlagSizeInBytes);
	DWORD dwDataOffset = dwIndex * dwDataItemSize;
	memset(pData+dwDataOffset, 0, dwQuantity * dwDataItemSize);
}

// ARRAY COMMANDS

DARKSDK DWORD DimCore(DWORD dwOldArrayPtr, DWORD dwTypeAndSizeOfElement, DWORD dwD1, DWORD dwD2, DWORD dwD3, DWORD dwD4, DWORD dwD5, DWORD dwD6, DWORD dwD7, DWORD dwD8, DWORD dwD9)
{
	// Increment all DBPro dimensions (+1 based)
	dwD1+=1;
	if(dwD2>0) dwD2+=1;
	if(dwD3>0) dwD3+=1;
	if(dwD4>0) dwD4+=1;
	if(dwD5>0) dwD5+=1;
	if(dwD6>0) dwD6+=1;
	if(dwD7>0) dwD7+=1;
	if(dwD8>0) dwD8+=1;
	if(dwD9>0) dwD9+=1;

	// Work out array size (can be no bigger than DWORD)
	__int64 iiSize = dwD1;
	if(dwD2>0) iiSize *= dwD2;
	if(dwD3>0) iiSize *= dwD3;
	if(dwD4>0) iiSize *= dwD4;
	if(dwD5>0) iiSize *= dwD5;
	if(dwD6>0) iiSize *= dwD6;
	if(dwD7>0) iiSize *= dwD7;
	if(dwD8>0) iiSize *= dwD8;
	if(dwD9>0) iiSize *= dwD9;
	DWORD dwSizeOfArray = (DWORD)iiSize;
	if(dwSizeOfArray!=iiSize)
		return NULL;

	// new idea for dwTypeAndSizeOfElement
	// where the first 0-4095 specify a type index (>9 = user types)
	// and then a multiple of 4096 controls the size of the datatype
	// Type Value and Size as one DWORD value
	DWORD dwSizeOfOneDataItem = dwTypeAndSizeOfElement;
	dwSizeOfOneDataItem = dwSizeOfOneDataItem/4096;
	DWORD dwTypeValueOfOneDataItem = dwTypeAndSizeOfElement-(dwSizeOfOneDataItem*4096);

	// Create New Array
	DWORD dwArrayPtr =  CreateArray(dwSizeOfArray, dwSizeOfOneDataItem, dwTypeValueOfOneDataItem);

	// Fill array with dimension size data (D1-D9)
	DWORD* pHeader = (DWORD*)(((LPSTR)dwArrayPtr)-HEADERSIZEINBYTES);
	DWORD dwDimOverallSize=dwD1;
	for(DWORD h=0; h<=8; h++)
	{
		pHeader[h]=dwDimOverallSize;
		if(h==0) dwDimOverallSize=dwDimOverallSize*dwD2;
		if(h==1) dwDimOverallSize=dwDimOverallSize*dwD3;
		if(h==2) dwDimOverallSize=dwDimOverallSize*dwD4;
		if(h==3) dwDimOverallSize=dwDimOverallSize*dwD5;
		if(h==4) dwDimOverallSize=dwDimOverallSize*dwD6;
		if(h==5) dwDimOverallSize=dwDimOverallSize*dwD7;
		if(h==6) dwDimOverallSize=dwDimOverallSize*dwD8;
		if(h==7) dwDimOverallSize=dwDimOverallSize*dwD9;
	}

	// Return ArrayPtr
	return dwArrayPtr;
}

// ADDED DUE TO POPULAR DEMAND ON 040304

DARKSDK DWORD ReDimCore(DWORD dwOldArrayPtr, DWORD dwNewTypeAndSizeOfElement, DWORD dwOD1, DWORD dwOD2, DWORD dwOD3, DWORD dwOD4, DWORD dwOD5, DWORD dwOD6, DWORD dwOD7, DWORD dwOD8, DWORD dwOD9)
{
	// Increment all DBPro dimensions (+1 based) (as done is DimCore)
	DWORD dwD1=dwOD1, dwD2=dwOD2, dwD3=dwOD3, dwD4=dwOD4, dwD5=dwOD5, dwD6=dwOD6, dwD7=dwOD7, dwD8=dwOD8, dwD9=dwOD9;
	dwD1+=1;
	if(dwD2>0) dwD2+=1;
	if(dwD3>0) dwD3+=1;
	if(dwD4>0) dwD4+=1;
	if(dwD5>0) dwD5+=1;
	if(dwD6>0) dwD6+=1;
	if(dwD7>0) dwD7+=1;
	if(dwD8>0) dwD8+=1;
	if(dwD9>0) dwD9+=1;

	// Old Header Info
	DWORD dwHeaderSizeInBytes = HEADERSIZEINBYTES;
	DWORD* pOldHeader = (DWORD*)(((LPSTR)dwOldArrayPtr)-HEADERSIZEINBYTES);
	DWORD dwSizeOfOneDataItem = pOldHeader[11];

	// lee - 130206 - can detect if LOCAL DIM ARRAY attempte a REDIM with corrupt data
	// prevent bug by ignoring a REDIM and starting with a brand new Array
	if ( dwSizeOfOneDataItem > 1024000 ) // a data item over 1MB is a little extreme
		return NULL;

	// continue as the REDIM appears to be valid..
	DWORD dwTypeValueOfOneDataItem = pOldHeader[12];

	// Work out size and type of new array early
	DWORD dwNewSizeOfOneDataItem = dwNewTypeAndSizeOfElement;
	dwNewSizeOfOneDataItem = dwNewSizeOfOneDataItem/4096;
	DWORD dwNewTypeValueOfOneDataItem = dwNewTypeAndSizeOfElement-(dwNewSizeOfOneDataItem*4096);

	// Leave if core datachunk size (type) different
	if ( dwSizeOfOneDataItem!=dwNewSizeOfOneDataItem
	||   dwTypeValueOfOneDataItem!=dwNewTypeValueOfOneDataItem )
		return dwOldArrayPtr;

	// Create a New Array of new size
	DWORD dwNewArrayPtr = DimCore ( dwOldArrayPtr, dwNewTypeAndSizeOfElement, dwOD1, dwOD2, dwOD3, dwOD4, dwOD5, dwOD6, dwOD7, dwOD8, dwOD9 );
	DWORD* pNewHeader = (DWORD*)(((LPSTR)dwNewArrayPtr)-HEADERSIZEINBYTES);

	// Old Array Offsets
	DWORD dwOld[9];	for ( int i=0; i<9; i++ ) dwOld[i] = pOldHeader[i];

	// New Array Offsets
	DWORD dwNew[9];	for ( int i=0; i<9; i++ ) dwNew[i] = pNewHeader[i];

	// Find old and new ptrs to reference tables of the arrays
	DWORD* pOldRef = (DWORD*)dwOldArrayPtr;
	DWORD* pNewRef = (DWORD*)dwNewArrayPtr;

	// Work out old dim values from data chunk sizes
	DWORD dwOldDims [ 9 ];
	for(DWORD h=0; h<=8; h++)
	{
		DWORD dwDataChunkSize;
		if(h==0) dwDataChunkSize=1;
		if(h==1) dwDataChunkSize=dwOld[0];
		if(h==2) dwDataChunkSize=dwOld[0]*dwOld[1];
		if(h==3) dwDataChunkSize=dwOld[0]*dwOld[1];
		if(h==4) dwDataChunkSize=dwOld[0]*dwOld[1]*dwOld[2];
		if(h==5) dwDataChunkSize=dwOld[0]*dwOld[1]*dwOld[2]*dwOld[3];
		if(h==6) dwDataChunkSize=dwOld[0]*dwOld[1]*dwOld[2]*dwOld[3]*dwOld[4];
		if(h==7) dwDataChunkSize=dwOld[0]*dwOld[1]*dwOld[2]*dwOld[3]*dwOld[4]*dwOld[5];
		if(h==8) dwDataChunkSize=dwOld[0]*dwOld[1]*dwOld[2]*dwOld[3]*dwOld[4]*dwOld[5]*dwOld[6];
		DWORD dwActualDimValue=0;
		if ( dwDataChunkSize>0 ) dwActualDimValue=dwOld[h]/dwDataChunkSize;
		dwOldDims[h]=dwActualDimValue;
	}

	// Trim if new array is smaller than old array (odd redim but possible)
	if ( dwOldDims[0] > dwD1 ) dwOldDims[0]=dwD1;
	if ( dwOldDims[1] > dwD2 ) dwOldDims[1]=dwD2;
	if ( dwOldDims[2] > dwD3 ) dwOldDims[2]=dwD3;
	if ( dwOldDims[3] > dwD4 ) dwOldDims[3]=dwD4;
	if ( dwOldDims[4] > dwD5 ) dwOldDims[4]=dwD5;
	if ( dwOldDims[5] > dwD6 ) dwOldDims[5]=dwD6;
	if ( dwOldDims[6] > dwD7 ) dwOldDims[6]=dwD7;
	if ( dwOldDims[7] > dwD8 ) dwOldDims[7]=dwD8;
	if ( dwOldDims[8] > dwD9 ) dwOldDims[8]=dwD9;

	// make sure can get through all fornext conditions at least once (to get to code)
	for(int h=0; h<=8; h++)
	{
		DWORD dwActualDimValue=dwOldDims[h];
		if ( dwActualDimValue==0 ) dwActualDimValue=1;
		dwOldDims[h]=dwActualDimValue;
	}

	// u55 - 080704 - do not copy if old or new array is EMPTY
	if ( pOldHeader[10]>0 && pNewHeader[10]>0 )
	{
		// Copy Old array data to new array
		for ( DWORD dwI1=0; dwI1<dwOldDims[0]; dwI1++ )
		for ( DWORD dwI2=0; dwI2<dwOldDims[1]; dwI2++ )
		for ( DWORD dwI3=0; dwI3<dwOldDims[2]; dwI3++ )
		for ( DWORD dwI4=0; dwI4<dwOldDims[3]; dwI4++ )
		for ( DWORD dwI5=0; dwI5<dwOldDims[4]; dwI5++ )
		for ( DWORD dwI6=0; dwI6<dwOldDims[5]; dwI6++ )
		for ( DWORD dwI7=0; dwI7<dwOldDims[6]; dwI7++ )
		for ( DWORD dwI8=0; dwI8<dwOldDims[7]; dwI8++ )
		for ( DWORD dwI9=0; dwI9<dwOldDims[8]; dwI9++ )
		{
			// copy old block of data to new array 
			DWORD dwOldIndex = (dwI1)+(dwI2*dwOld[0])+(dwI3*dwOld[1])+(dwI4*dwOld[2])+(dwI5*dwOld[3])+(dwI6*dwOld[4])+(dwI7*dwOld[5])+(dwI8*dwOld[6])+(dwI9*dwOld[7]);
			DWORD dwNewIndex = (dwI1)+(dwI2*dwNew[0])+(dwI3*dwNew[1])+(dwI4*dwNew[2])+(dwI5*dwNew[3])+(dwI6*dwNew[4])+(dwI7*dwNew[5])+(dwI8*dwNew[6])+(dwI9*dwNew[7]);
			LPSTR pOldPtr = (LPSTR)pOldRef[dwOldIndex];
			LPSTR pNewPtr = (LPSTR)pNewRef[dwNewIndex];
			memcpy ( pNewPtr, pOldPtr, dwSizeOfOneDataItem );
		}
	}

	// Free Old Array (if any)
	DeleteArray ( dwOldArrayPtr );

	// return new sized arrau
	return dwNewArrayPtr;
}

DARKSDK DWORD DimDDD(DWORD dwOldArrayPtr, DWORD dwTypeAndSizeOfElement, DWORD dwD1, DWORD dwD2, DWORD dwD3, DWORD dwD4, DWORD dwD5, DWORD dwD6, DWORD dwD7, DWORD dwD8, DWORD dwD9)
{
	try
	{
		// leechange - 050304 - now REDIMs if array already exists
		if ( dwOldArrayPtr )
		{
			// Change Size Of Array (and retain contents)
			DWORD dwNewArrPtr = ReDimCore ( dwOldArrayPtr, dwTypeAndSizeOfElement, dwD1, dwD2, dwD3, dwD4, dwD5, dwD6, dwD7, dwD8, dwD9 );

			// If corruption detected, can resort to a new array as follows..
			if ( dwNewArrPtr!=NULL ) return dwNewArrPtr;
		}

		// Create a New Array
		return DimCore ( dwOldArrayPtr, dwTypeAndSizeOfElement, dwD1, dwD2, dwD3, dwD4, dwD5, dwD6, dwD7, dwD8, dwD9 );
	}
	catch (...)
	{
		RunTimeError(RUNTIMEERROR_NOTENOUGHMEMORY);
		return dwOldArrayPtr;
	}
}

DARKSDK DWORD UnDimDD(DWORD dwAllocation)
{
	// leefix - 070308 - U6.7 - will free strings if string array (fixes string leak)
	FreeStringsFromArray(dwAllocation);

	DeleteArray(dwAllocation);
	return NULL;
}

// ADVANCED UNIFIED ARRAY HANLDING

DARKSDK void ArrayIndexToBottom(DWORD dwArrayPtr)
{
	// set index to last item in array
	if(dwArrayPtr) *((DWORD*)dwArrayPtr-1) = *((DWORD*)dwArrayPtr-4)-1;
}
DARKSDK void ArrayIndexToTop(DWORD dwArrayPtr)
{
	// set index to first item in array
	if(dwArrayPtr) *((DWORD*)dwArrayPtr-1) = 0;
}
DARKSDK void NextArrayIndex(DWORD dwArrayPtr)
{
	// inc array index
	if(dwArrayPtr)
	{
		*((DWORD*)dwArrayPtr-1) = *((DWORD*)dwArrayPtr-1) + 1;
		if(*((DWORD*)dwArrayPtr-1) > *((DWORD*)dwArrayPtr-4))
		{
			// Last index reachable just just outside range >N
			*((DWORD*)dwArrayPtr-1) = *((DWORD*)dwArrayPtr-4);
		}
	}
}
DARKSDK void PreviousArrayIndex(DWORD dwArrayPtr)
{
	// dec array index
	if(dwArrayPtr)
	{
		if(*((DWORD*)dwArrayPtr-1)>0)
		{
			*((DWORD*)dwArrayPtr-1)=(*((DWORD*)dwArrayPtr-1))-1;
		}
		else
		{
			// First index reachable just just outside range <0
			*((DWORD*)dwArrayPtr-1)=(DWORD)-1;
		}
	}
}
DARKSDK DWORD ArrayIndexValid(DWORD dwArrayPtr)
{
	// check if index is valid (pointing to valid item)
	if(dwArrayPtr)
	{
		if(*((DWORD*)dwArrayPtr-1)<*((DWORD*)dwArrayPtr-4))
			return 1;
		else
			return 0;
	}
	else
		return 0;
}
DARKSDK DWORD ArrayCount(DWORD dwArrayPtr)
{
	// return array size
	if(dwArrayPtr) 
		return (*((DWORD*)dwArrayPtr-4))-1;
	else
		return -1;
}
DARKSDK DWORD ArrayInsertAtBottom(DWORD dwArrayPtr)
{
	try
	{
		// If no array, leave now
		if(dwArrayPtr==NULL) return dwArrayPtr;

		// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
		if ( IsArraySingleDim ( dwArrayPtr )==false )
		{
			RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM);
			return dwArrayPtr;
		}

		// Adjust Size Of Entire Array
		DWORD dwAllocation = ExpandArray(dwArrayPtr, 1);

		// Determine index
		int iIndex = (*((DWORD*)dwAllocation-4)) - 1;
		if(iIndex<0) iIndex=0;

		// Update array index to last in list
		*((DWORD*)dwAllocation-1) = iIndex;

		// Overwrites current array ptr
		return dwAllocation;
	}
	catch (...)
	{
		RunTimeError(RUNTIMEERROR_NOTENOUGHMEMORY);
		return dwArrayPtr;
	}
}
DARKSDK DWORD ArrayInsertAtBottom(DWORD dwArrayPtr, int iQuantity)
{
	try
	{
		// If no array, leave now
		if(dwArrayPtr==NULL) return dwArrayPtr;

		// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
		if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return dwArrayPtr; }

		// Autohandler
		if(iQuantity<1) iQuantity=1;

		// Adjust Size Of Entire Array
		DWORD dwAllocation = ExpandArray(dwArrayPtr, iQuantity);

		// Determine index
		int iIndex = (*((DWORD*)dwAllocation-4)) - iQuantity;
		if(iIndex<0) iIndex=0;

		// Update array index to fisrt new item at end of list
		*((DWORD*)dwAllocation-1) = iIndex;

		// Overwrites current array ptr
		return dwAllocation;
	}
	catch (...)
	{
		RunTimeError(RUNTIMEERROR_NOTENOUGHMEMORY);
		return dwArrayPtr;
	}
}

DARKSDK DWORD ArrayInsertAtTop(DWORD dwArrayPtr)
{
	try
	{
		// If no array, leave now
		if(dwArrayPtr==NULL) return dwArrayPtr;

		// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
		if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return dwArrayPtr; }

		// Adjust Size Of Entire Array
		DWORD dwAllocation = ExpandArray(dwArrayPtr, 1);

		// Store Ref located at end of list
		DWORD dwSizeOfTable = *((DWORD*)dwAllocation-4);
		DWORD dwIndex = dwSizeOfTable - 1;
		DWORD* pRef = (DWORD*)dwAllocation;
		DWORD dwRefItem = pRef[dwIndex];

		// Shuffle ref table to make space at top
		if(dwSizeOfTable>0) memcpy(pRef+1, pRef, (dwSizeOfTable-1)*4);

		// Copy refitem to top position
		pRef[0] = dwRefItem;

		// Update array index to new item
		*((DWORD*)dwAllocation-1) = 0;

		// Overwrites current array ptr
		return dwAllocation;
	}
	catch (...)
	{
		RunTimeError(RUNTIMEERROR_NOTENOUGHMEMORY);
		return dwArrayPtr;
	}
}

DARKSDK DWORD ArrayInsertAtTop(DWORD dwArrayPtr, int iQuantity)
{
	try
	{
		// If no array, leave now
		if(dwArrayPtr==NULL) return dwArrayPtr;

		// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
		if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return dwArrayPtr; }

		// Autohandler
		if(iQuantity<1) iQuantity=1;

		// Adjust Size Of Entire Array
		DWORD dwAllocation = ExpandArray(dwArrayPtr, iQuantity);

		// Store RefItems(iQuantity) located at end of list
		DWORD* pStoreRefs = (DWORD*)new DWORD[iQuantity];
		DWORD dwSizeOfTable = *((DWORD*)dwAllocation-4);
		DWORD dwIndexOfFirstRef = dwSizeOfTable-iQuantity;
		DWORD* pRef = (DWORD*)dwAllocation;
		memcpy(pStoreRefs, (DWORD*)pRef + dwIndexOfFirstRef, iQuantity*4);

		// Shuffle ref table to make space at top
		DWORD dwAmountToShuffle = 0;
		if(dwSizeOfTable>(DWORD)iQuantity) dwAmountToShuffle=(dwSizeOfTable-iQuantity)*4;
		if(dwAmountToShuffle>0) memcpy(pRef+iQuantity, pRef, dwAmountToShuffle);

		// Copy refitem to top position
		memcpy(pRef, pStoreRefs, iQuantity*4);

		// Update array index to new item
		*((DWORD*)dwAllocation-1) = 0;

		// Overwrites current array ptr
		return dwAllocation;
	}
	catch (...)
	{
		RunTimeError(RUNTIMEERROR_NOTENOUGHMEMORY);
		return dwArrayPtr;
	}
}
DARKSDK DWORD ArrayInsertAtElement(DWORD dwArrayPtr, int iIndex)
{
	try
	{
		// If no array, leave now
		if(dwArrayPtr==NULL) return dwArrayPtr;

		// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
		if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return dwArrayPtr; }

		DWORD dwSizeOfTable = *((DWORD*)dwArrayPtr-4);
		if(iIndex<0 || iIndex>=(int)dwSizeOfTable)
		{
			RunTimeError(RUNTIMEERROR_ARRAYINDEXINVALID);
			return dwArrayPtr;
		}
		
		// Size of insert
		int iQuantity=1;

		// Adjust Size Of Entire Array
		DWORD dwAllocation = ExpandArray(dwArrayPtr, iQuantity);

		// Store RefItems(iQuantity) located at end of list
		DWORD* pStoreRefs = (DWORD*)new DWORD[iQuantity];
		DWORD dwIndexOfFirstRef = dwSizeOfTable-(iQuantity-1);  //leefix-230603-corrected ptr to new item-ref in expanded array
		DWORD* pRef = (DWORD*)dwAllocation;
		memcpy(pStoreRefs, (DWORD*)pRef + dwIndexOfFirstRef, iQuantity*4);

		// Shuffle iIndex to End onwards
		DWORD dwSizeOfLaterChunk = 0;
		if(dwSizeOfTable>(DWORD)iIndex) dwSizeOfLaterChunk = dwSizeOfTable-iIndex;
		if(dwSizeOfLaterChunk>0) memcpy(pRef+iIndex+iQuantity, pRef+iIndex, dwSizeOfLaterChunk*4);

		// Copy RefItems into space created inside table
		memcpy(pRef+iIndex, pStoreRefs, iQuantity*4);
		delete[] pStoreRefs;    // Remove memory leak

		// Update array index to new item
		*((DWORD*)dwAllocation-1) = iIndex;

		// Overwrites current array ptr
		return dwAllocation;
	}
	catch (...)
	{
		RunTimeError(RUNTIMEERROR_NOTENOUGHMEMORY);
		return dwArrayPtr;
	}
}
DARKSDK void ArrayDeleteElement(DWORD dwArrayPtr, int iIndex)
{
	// If no array, leave now
	if(dwArrayPtr==NULL) return;

	// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
	if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return; }

	DWORD dwSizeOfTable = *((DWORD*)dwArrayPtr-4);
	if(dwSizeOfTable==0)
	{
		// already empty - silent failure
		return;
	}
	if(iIndex<0 || iIndex>=(int)dwSizeOfTable)
	{
		RunTimeError(RUNTIMEERROR_ARRAYINDEXINVALID);
		return;
	}

	// Prepare pointers
	DWORD dwDataItemSize = *((DWORD*)dwArrayPtr-3);
	DWORD dwRefSizeInBytes = dwSizeOfTable * 4;
	DWORD dwFlagSizeInBytes = dwSizeOfTable * 1;
	DWORD* pRef = (DWORD*)dwArrayPtr;
	LPSTR pFlag = (LPSTR)(((LPSTR)dwArrayPtr)+dwRefSizeInBytes);
	LPSTR pData = (LPSTR)(((LPSTR)dwArrayPtr)+dwRefSizeInBytes+dwFlagSizeInBytes);
	DWORD dwOffset = ((DWORD)pRef[iIndex] - (DWORD)pData);// / dwDataItemSize;

	// leeadd - 211008 - u71 - check for strings before remove this element
	DWORD dwInternalTypeIndex = *((DWORD*)dwArrayPtr-2);
	LPSTR pPattern = GetTypePatternCore ( NULL, dwInternalTypeIndex );
	if ( pPattern )
	{
		// go through pattern which matches basic types
		DWORD dwTypeInternalOffset = 0;
		for ( DWORD n=0; n<strlen(pPattern); n++ )
		{
			// delete any strings in the user type
			if ( pPattern[n]=='S' )
			{
				// U74 - 050509 - delete CORRECT part of block!
				LPSTR* pStringData = (LPSTR*)(pData+dwOffset+dwTypeInternalOffset);
				if ( *pStringData )
				{
					delete[] *pStringData;
					*pStringData=NULL;
				}
			}

			// next one..
			switch ( pPattern[n] )
			{
				case 'B'	: dwTypeInternalOffset+=4; break;//1
				case 'Y'	: dwTypeInternalOffset+=4; break;//1
				case 'W'	: dwTypeInternalOffset+=4; break;//2
				case 'O'	: dwTypeInternalOffset+=8; break;
				case 'R'	: dwTypeInternalOffset+=8; break;
				default		: dwTypeInternalOffset+=4; break;
			}
		}
		delete[] pPattern;
	}

	// Shuffle to remove item from ref table
	DWORD dwAmountToShuffle = 0;
	if((dwSizeOfTable-iIndex-1)>0) dwAmountToShuffle = (dwSizeOfTable-iIndex-1)*4;
	if(dwAmountToShuffle>0) memcpy(pRef+iIndex, pRef+iIndex+1, dwAmountToShuffle);

	// Store ref data
	DWORD** pStoreRef = new DWORD* [ dwSizeOfTable ];
	memcpy ( pStoreRef, pRef, dwSizeOfTable * sizeof(DWORD*) );

	// First shuffle out deleted data
	dwAmountToShuffle = 0;
	DWORD dwTotalSizeOfData = dwSizeOfTable * dwDataItemSize;
	if((dwTotalSizeOfData-dwOffset-dwDataItemSize)>0) dwAmountToShuffle = (dwTotalSizeOfData-dwOffset-dwDataItemSize);
	if(dwAmountToShuffle>0) memcpy(pData+dwOffset, pData+dwOffset+dwDataItemSize, dwAmountToShuffle);

	// Reduce size of array
	dwSizeOfTable = dwSizeOfTable - 1;
	*((DWORD*)dwArrayPtr-4) = dwSizeOfTable;

	// Get new sizes and pointers
	DWORD dwNewRefSizeInBytes = dwSizeOfTable * 4;
	DWORD dwNewFlagSizeInBytes = dwSizeOfTable * 1;
	LPSTR pNewFlag = (LPSTR)(((LPSTR)dwArrayPtr)+dwNewRefSizeInBytes);
	LPSTR pNewData = (LPSTR)(((LPSTR)dwArrayPtr)+dwNewRefSizeInBytes+dwNewFlagSizeInBytes);

	// Generate new ref data
	for(DWORD i=0; i<dwSizeOfTable; i++)
	{
		// leefix - 210604 - retain pattern of ref data / 260604-u54-dwDataItemSize not 1
		DWORD dwRedirectOffset = (DWORD)pStoreRef[i] - (DWORD)pData;
		if ( dwRedirectOffset >= dwOffset ) dwRedirectOffset-=dwDataItemSize;
		pRef[i] = (DWORD)(pNewData + dwRedirectOffset);
	}

	// free stored ref data
	delete[] pStoreRef;

	// Set Flag Data with ones
	memset(pNewFlag, 1, dwNewFlagSizeInBytes);

	// Then shuffle all data to new position
	DWORD dwNewTotalSizeOfData = dwSizeOfTable * dwDataItemSize;
	memcpy(pNewData, pData, dwNewTotalSizeOfData);

	// Ensure internal index is still valid
	if( *((DWORD*)dwArrayPtr-1) >= dwSizeOfTable )
		*((DWORD*)dwArrayPtr-1) = dwSizeOfTable - 1;
}
DARKSDK void ArrayDeleteElement(DWORD dwArrayPtr)
{
	// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
	if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return; }

	DWORD dwSizeOfTable = *((DWORD*)dwArrayPtr-4);
	if(dwSizeOfTable==0)
	{
		// already empty - silent failure
		return;
	}

	int iCurrentIndex = *((DWORD*)dwArrayPtr-1);
	ArrayDeleteElement ( dwArrayPtr, iCurrentIndex );
}
DARKSDK void EmptyArray(DWORD dwAllocation)
{
	// If no array, leave now
	if(dwAllocation==NULL) return;

	DWORD dwSizeOfTable = *((DWORD*)dwAllocation-4);
	if(dwSizeOfTable==0)
	{
		// already empty - silent failure
		return;
	}

	// Get Array Information
	LPSTR pArrayPtr = ((LPSTR)dwAllocation)-HEADERSIZEINBYTES;
	DWORD* pHeader	= (DWORD*)(pArrayPtr);
	DWORD dwHeaderSizeInBytes = HEADERSIZEINBYTES;
	// should this not be 10,11,12,13??  - 140104
	DWORD dwSizeOfArray = pHeader[0];
	DWORD dwSizeOfOneDataItem = pHeader[1];
	DWORD dwTypeValueOfOneDataItem = pHeader[2];
	DWORD dwInternalIndex = pHeader[3];
	DWORD dwRefSizeInBytes = dwSizeOfArray * 4;
	DWORD dwFlagSizeInBytes = dwSizeOfArray * 1;
	DWORD dwDataSizeInBytes = dwSizeOfArray * dwSizeOfOneDataItem;
	DWORD* pRef = (DWORD*)(pArrayPtr+dwHeaderSizeInBytes);
	LPSTR pFlag = (LPSTR)(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes);
	LPSTR pData = (LPSTR)(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes+dwFlagSizeInBytes);

	// Clear all data from array
	FreeStringsFromArray( dwAllocation );
	memset(pRef, 0, dwRefSizeInBytes);
	memset(pFlag, 0, dwFlagSizeInBytes);
	memset(pData, 0, dwDataSizeInBytes);

	// Reset size of array to empty
	*((DWORD*)dwAllocation-4) = 0;
	*((DWORD*)dwAllocation-1) = 0;
}
DARKSDK DWORD PushToStack(DWORD dwArrayPtr)
{
	// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
	if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return dwArrayPtr; }

	// add item to bottom of list
	dwArrayPtr = ArrayInsertAtBottom(dwArrayPtr);

	// place index to end
	int iIndexAtEnd = *((DWORD*)dwArrayPtr-4) - 1;
	*((DWORD*)dwArrayPtr-1) = iIndexAtEnd;

	// return array ptr
	return dwArrayPtr;
}
DARKSDK void PopFromStack(DWORD dwArrayPtr)
{
	// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
	if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return; }

	// remove from bottom if list
	int iIndexAtEnd = *((DWORD*)dwArrayPtr-4) - 1;
	ArrayDeleteElement(dwArrayPtr, iIndexAtEnd);

	// place index to end
	iIndexAtEnd = *((DWORD*)dwArrayPtr-4) - 1;
	*((DWORD*)dwArrayPtr-1) = iIndexAtEnd;
}
DARKSDK DWORD AddToQueue(DWORD dwArrayPtr)
{
	// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
	if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return dwArrayPtr; }

	// add to top of list
	dwArrayPtr = ArrayInsertAtBottom(dwArrayPtr);

	// place index to end
	int iIndexAtEnd = *((DWORD*)dwArrayPtr-4) - 1;
	*((DWORD*)dwArrayPtr-1) = iIndexAtEnd;

	// return array ptr
	return dwArrayPtr;
}
DARKSDK void RemoveFromQueue(DWORD dwArrayPtr)
{
	// lee - 140306 - u60b3 - Do not allow multi-dimensional arrays
	if ( IsArraySingleDim ( dwArrayPtr )==false ) { RunTimeError(RUNTIMEERROR_ARRAYMUSTBESINGLEDIM); return; }

	// remove from top of list
	ArrayDeleteElement(dwArrayPtr, 0);

	// place index to zero
	*((DWORD*)dwArrayPtr-1) = 0;
}
DARKSDK void ArrayIndexToStack(DWORD dwArrayPtr)
{
	// set index to last item in array
	if(dwArrayPtr) *((DWORD*)dwArrayPtr-1) = *((DWORD*)dwArrayPtr-4)-1;
}
DARKSDK void ArrayIndexToQueue(DWORD dwArrayPtr)
{
	// set index to first item in array
	if(dwArrayPtr) *((DWORD*)dwArrayPtr-1) = 0;
}

// HARDCODE COMMANDS

DARKSDK DWORD MakeMemory(int iSize)
{
	LPSTR pMem = new char[iSize];
	return (DWORD)pMem;
}

DARKSDK void DeleteByteMemory(DWORD dwMem)
{
	if(dwMem) delete[] (LPSTR)dwMem;
}

DARKSDK void FillByteMemory(DWORD dwDest, int iValue, int iSize)
{
	memset((LPSTR)dwDest, iValue, iSize);
}

DARKSDK void CopyMemory(DWORD dwDest, DWORD dwSrc, int iSize)
{
	if ( iSize > 0 )
		memcpy((LPSTR)dwDest, (LPSTR)dwSrc, iSize);
}

// DATA STATEMENT COMMAND FUNCTIONS
DARKSDK void Restore(void)
{
	g_pDataLabelPtr = g_pDataLabelStart;		
}
DARKSDK void RestoreD(DWORD dwDataLabel)
{
	if ( dwDataLabel==0 )
		g_pDataLabelPtr = g_pDataLabelStart;
	else
		g_pDataLabelPtr = (LPSTR)dwDataLabel;
		
}
DARKSDK DWORD ReadL(void)
{
	double dData=0;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==1)
			dData = *(double*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	int iValue = (int)dData;

	return *(DWORD*)&iValue;
}
DARKSDK DWORD ReadF(void)
{
	double dData=0;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==1)
			dData = *(double*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	float fValue = (float)dData;

	return *(DWORD*)&fValue;
}
DARKSDK DWORD ReadS(DWORD pDestStr)
{
	if(pDestStr) delete[] (LPSTR)pDestStr;

	LPSTR pDatStr = NULL;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==2)
			pDatStr = (LPSTR)*(DWORD*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	LPSTR pString;
	if (pDatStr)
	{
		DWORD dwLength = strlen(pDatStr);
		pString=new char[dwLength+1];
		strcpy(pString, pDatStr);
	}
	else
	{
		pString = new char[1];
		*pString = 0;
	}

	return (DWORD)pString;
}
DARKSDK BYTE ReadB(void)
{
	double dData=0;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==1)
			dData = *(double*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	BYTE dwValue = (BYTE)dData;

	return dwValue;
}
DARKSDK WORD ReadW(void)
{
	double dData=0;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==1)
			dData = *(double*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	WORD dwValue = (WORD)dData;

	return dwValue;
}
DARKSDK DWORD ReadD(void)
{
	double dData=0;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==1)
			dData = *(double*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	DWORD dwValue = (DWORD)dData;

	return dwValue;
}
DARKSDK LONGLONG ReadR(void)
{
	double dData=0;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==1)
			dData = *(double*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	LONGLONG lValue = (LONGLONG)dData;

	return lValue;
}
DARKSDK double ReadO(void)
{
	double dValue=0;
	if(g_pDataLabelPtr && g_pDataLabelPtr<g_pDataLabelEnd)
	{
		if(*(g_pDataLabelPtr+0)==1)
			dValue = *(double*)(g_pDataLabelPtr+2);

		// Advance After Read, but only to end of data
		g_pDataLabelPtr+=10;
	}

	return dValue;
}

DARKSDK DWORD EqualDDD(DWORD dwValueA, DWORD dwValueB)
{
	int result = dwValueA==dwValueB;
	return result;
}
DARKSDK DWORD GreaterDDD(DWORD dwValueA, DWORD dwValueB)
{
	int result = dwValueA>dwValueB;
	return result;
}
DARKSDK DWORD LessDDD(DWORD dwValueA, DWORD dwValueB)
{
	int result = dwValueA<dwValueB;
	return result;
}
DARKSDK DWORD NotEqualDDD(DWORD dwValueA, DWORD dwValueB)
{
	int result = dwValueA!=dwValueB;
	return result;
}
DARKSDK DWORD GreaterEqualDDD(DWORD dwValueA, DWORD dwValueB)
{
	int result = dwValueA>=dwValueB;
	return result;
}
DARKSDK DWORD LessEqualDDD(DWORD dwValueA, DWORD dwValueB)
{
	int result = dwValueA<=dwValueB;
	return result;
}

// EXTERNAL SUPPORT MATHS

DARKSDK DWORD PowerLLL(int iValueA, int iValueB)
{
	// do not know the ASM version of this
	int result = (int)pow((long double)iValueA,(long double)iValueB);
	return *((DWORD*)&result);
}
DARKSDK DWORD PowerBBB(DWORD dwValueA, DWORD dwValueB)
{
	DWORD result = (unsigned char)pow((long double)dwValueA,(long double)dwValueB);
	return result;
}
DARKSDK DWORD PowerWWW(DWORD dwValueA, DWORD dwValueB)
{
	DWORD result = (WORD)pow((long double)dwValueA,(long double)dwValueB);
	return result;
}
DARKSDK DWORD PowerDDD(DWORD dwValueA, DWORD dwValueB)
{
	DWORD result = (DWORD)pow(( double ) dwValueA,( double ) dwValueB);
	return result;
}

// FLOAT MATHS

DARKSDK DWORD PowerFFF(float fValueA, float fValueB)
{
	float result = (float)pow(fValueA,fValueB);
	return *((DWORD*)&result);
}
DARKSDK DWORD MulFFF(float fValueA, float fValueB)
{
	float result = fValueA*fValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD DivFFF(float fValueA, float fValueB)
{
	if(fValueB==0) return 0;
	float result = fValueA/fValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD AddFFF(float fValueA, float fValueB)
{
	float result = fValueA+fValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD SubFFF(float fValueA, float fValueB)
{
	float result = fValueA-fValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD ModFFF(float fValueA, float fValueB)
{
	// lee - 150206 - u60 -floating point MOD added
	if(fValueB==0) return 0;
	double w = (double)fValueA;
	double x = (double)fValueB;
	double z = fmod( w, x );
	float result = (float)z;
	return *((DWORD*)&result);
}

// FLOAT COMPARE MATHS

DARKSDK DWORD EqualLFF(float fValueA, float fValueB)
{
	return fValueA==fValueB;
}
DARKSDK DWORD GreaterLFF(float fValueA, float fValueB)
{
	return fValueA>fValueB;
}
DARKSDK DWORD LessLFF(float fValueA, float fValueB)
{
	return fValueA<fValueB;
}
DARKSDK DWORD NotEqualLFF(float fValueA, float fValueB)
{
	return fValueA!=fValueB;
}
DARKSDK DWORD GreaterEqualLFF(float fValueA, float fValueB)
{
	return fValueA>=fValueB;
}
DARKSDK DWORD LessEqualLFF(float fValueA, float fValueB)
{
	return fValueA<=fValueB;
}

// STRING COMPARE MATHS

DARKSDK DWORD EqualLSS(DWORD dwSrcStr,DWORD dwDestStr)
{
	if(dwSrcStr && dwDestStr)
	{
		if(strcmp((LPSTR)dwSrcStr, (LPSTR)dwDestStr)==NULL)
			return 1;
		else
			return 0;
	}
	else
	{
		if(dwSrcStr==dwDestStr)
		{
			return 1;
		}
		else
		{
			if(dwSrcStr && dwDestStr==NULL)
			{
				if(strcmp((LPSTR)dwSrcStr, "")==NULL)
					return 1;
				else
					return 0;
			}
			if(dwDestStr && dwSrcStr==NULL)
			{
				if(strcmp((LPSTR)dwDestStr, "")==NULL)
					return 1;
				else
					return 0;
			}
			return 0;
		}
	}
}
DARKSDK DWORD GreaterLSS(DWORD dwSrcStr,DWORD dwDestStr)
{
	if(dwSrcStr && dwDestStr)
	{
		if(strcmp((LPSTR)dwSrcStr, (LPSTR)dwDestStr)>0)
			return 1;
		else
			return 0;
	}
	else
	{
		if(dwSrcStr==dwDestStr)
			return 1;
		else
			return 0;
	}
}
DARKSDK DWORD LessLSS(DWORD dwSrcStr,DWORD dwDestStr)
{
	if(dwSrcStr && dwDestStr)
	{
		if(strcmp((LPSTR)dwSrcStr, (LPSTR)dwDestStr)<0)
			return 1;
		else
			return 0;
	}
	else
	{
		if(dwSrcStr==dwDestStr)
			return 1;
		else
			return 0;
	}
}
DARKSDK DWORD NotEqualLSS(DWORD dwSrcStr,DWORD dwDestStr)
{
	if(dwSrcStr && dwDestStr)
	{
		if(strcmp((LPSTR)dwSrcStr, (LPSTR)dwDestStr)!=0)
			return 1;
		else
			return 0;
	}
	else
	{
		// leefix - 060405 - inverse of equal
		if(dwSrcStr==dwDestStr)
		{
			return 0;
		}
		else
		{
			if(dwSrcStr && dwDestStr==NULL)
			{
				if(strcmp((LPSTR)dwSrcStr, "")==NULL)
					return 0;
				else
					return 1;
			}
			if(dwDestStr && dwSrcStr==NULL)
			{
				if(strcmp((LPSTR)dwDestStr, "")==NULL)
					return 0;
				else
					return 1;
			}
			return 1;
		}
	}
}
DARKSDK DWORD GreaterEqualLSS(DWORD dwSrcStr,DWORD dwDestStr)
{
	if(dwSrcStr && dwDestStr)
	{
		if(strcmp((LPSTR)dwSrcStr, (LPSTR)dwDestStr)>=0)
			return 1;
		else
			return 0;
	}
	else
	{
		if(dwSrcStr==dwDestStr)
			return 1;
		else
			return 0;
	}
}
DARKSDK DWORD LessEqualLSS(DWORD dwSrcStr,DWORD dwDestStr)
{
	if(dwSrcStr && dwDestStr)
	{
		if(strcmp((LPSTR)dwSrcStr, (LPSTR)dwDestStr)<=0)
			return 1;
		else
			return 0;
	}
	else
	{
		if(dwSrcStr==dwDestStr)
			return 1;
		else
			return 0;
	}
}


// STRING MATHS

DARKSDK DWORD AddSSS(DWORD dwRetStr, DWORD dwSrcStrA, DWORD dwSrcStrB)
{
	DWORD length=1;
	if((DWORD*)dwSrcStrA) length=strlen((LPSTR)(DWORD*)dwSrcStrA);
	if((DWORD*)dwSrcStrB) length+=strlen((LPSTR)(DWORD*)dwSrcStrB);
	LPSTR lpNewStr = new char[length+1];
	strcpy(lpNewStr,"");
	if((DWORD*)dwSrcStrA) strcat(lpNewStr, (LPSTR)dwSrcStrA);
	if((DWORD*)dwSrcStrB) strcat(lpNewStr, (LPSTR)dwSrcStrB);
	if((DWORD*)dwRetStr) delete[] (DWORD*)dwRetStr;
	return (DWORD)lpNewStr;
}
DARKSDK DWORD EquateSS(DWORD dwDestStr,DWORD dwSrcStr)
{
	DWORD length=1;
	if((DWORD*)dwSrcStr) length=strlen((LPSTR)(DWORD*)dwSrcStr);
	LPSTR lpNewStr = NULL;
	if ( dwSrcStr!=0 )
	{
		lpNewStr = new char[length+1];
		if((DWORD*)dwSrcStr) strcpy(lpNewStr, (LPSTR)dwSrcStr); else strcpy(lpNewStr,"");
	}
	if((DWORD*)dwDestStr) delete[] (DWORD*)dwDestStr;
	return (DWORD)lpNewStr;
}
DARKSDK DWORD FreeSS(DWORD dwDestStr)
{
	if(dwDestStr) delete[] (DWORD*)dwDestStr;
	return 0;
}
DARKSDK DWORD FreeStringSS(DWORD dwDestStr)
{
	if(dwDestStr) delete[] (DWORD*)dwDestStr;
	return 0;
}

// DOUBLE FLOAT MATHS

DARKSDK double PowerOOO(double dValueA, double dValueB)
{
	double result = (float)pow(dValueA,dValueB);
	return result;
}
DARKSDK double MulOOO(double dValueA, double dValueB)
{
	double result = dValueA*dValueB;
	return result;
}
DARKSDK double DivOOO(double dValueA, double dValueB)
{
	if(dValueB==0) return 0;
	double result = dValueA/dValueB;
	return result;
}
DARKSDK double AddOOO(double dValueA, double dValueB)
{
	double result = dValueA+dValueB;
	return result;
}
DARKSDK double SubOOO(double dValueA, double dValueB)
{
	double result = dValueA-dValueB;
	return result;
}

// DOUBLE FLOAT COMPARISONS

DARKSDK DWORD EqualLOO(double dValueA, double dValueB)
{
	int result = dValueA==dValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD GreaterLOO(double dValueA, double dValueB)
{
	int result = dValueA>dValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD LessLOO(double dValueA, double dValueB)
{
	int result = dValueA<dValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD NotEqualLOO(double dValueA, double dValueB)
{
	int result = dValueA!=dValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD GreaterEqualLOO(double dValueA, double dValueB)
{
	int result = dValueA>=dValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD LessEqualLOO(double dValueA, double dValueB)
{
	int result = dValueA<=dValueB;
	return *((DWORD*)&result);
}

// DOUBLE INTEGER MATHS

DARKSDK LONGLONG PowerRRR(LONGLONG dValueA, LONGLONG dValueB)
{
	LONGLONG result = (LONGLONG)pow((double)dValueA,(double)dValueB);
	return result;
}
DARKSDK LONGLONG MulRRR(LONGLONG dValueA, LONGLONG dValueB)
{
	LONGLONG result = dValueA*dValueB;
	return result;
}
DARKSDK LONGLONG DivRRR(LONGLONG dValueA, LONGLONG dValueB)
{
	if(dValueB==0) return 0;
	LONGLONG result = dValueA/dValueB;
	return result;
}
DARKSDK LONGLONG AddRRR(LONGLONG dValueA, LONGLONG dValueB)
{
	LONGLONG result = dValueA+dValueB;
	return result;
}
DARKSDK LONGLONG SubRRR(LONGLONG dValueA, LONGLONG dValueB)
{
	LONGLONG result = dValueA-dValueB;
	return result;
}

// DOUBLE INTEGER COMPARISONS

DARKSDK DWORD EqualLRR(LONGLONG lValueA, LONGLONG lValueB)
{
	int result = lValueA==lValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD GreaterLRR(LONGLONG lValueA, LONGLONG lValueB)
{
	int result = lValueA>lValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD LessLRR(LONGLONG lValueA, LONGLONG lValueB)
{
	int result = lValueA<lValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD NotEqualLRR(LONGLONG lValueA, LONGLONG lValueB)
{
	int result = lValueA!=lValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD GreaterEqualLRR(LONGLONG lValueA, LONGLONG lValueB)
{
	int result = lValueA>=lValueB;
	return *((DWORD*)&result);
}
DARKSDK DWORD LessEqualLRR(LONGLONG lValueA, LONGLONG lValueB)
{
	int result = lValueA<=lValueB;
	return *((DWORD*)&result);
}

// CASTING MATHS

DARKSDK DWORD CastLtoF(int iValue)
{
	float result = (float)iValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastLtoB(int iValue)
{
	unsigned char result = (unsigned char)iValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastLtoY(int iValue)
{
	unsigned char result = (unsigned char)iValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastLtoW(int iValue)
{
	WORD result = (WORD)iValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastLtoD(int iValue)
{
	DWORD result = (DWORD)iValue;
	return *((DWORD*)&result);
}
DARKSDK double CastLtoO(int iValue)
{
	return (double)iValue;
}
DARKSDK LONGLONG CastLtoR(int iValue)
{
	return (LONGLONG)iValue;
}
DARKSDK DWORD CastFtoL(float fValue)
{
	int result = (int)fValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastFtoB(float fValue)
{
	unsigned char result = (unsigned char)fValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastFtoW(float fValue)
{
	WORD result = (WORD)fValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastFtoD(float fValue)
{
	// a maxed out DWORD produces wrong float, so keep it within 4bytes 
	LONGLONG Long = (LONGLONG)fValue;
	if(Long>4294967295) Long=4294967295;

	DWORD result = (DWORD)Long;
	return *((DWORD*)&result);
}
DARKSDK double CastFtoO(float fValue)
{
	// LEEFIX - 141102 - FLD is different on AMD processprs, so truncate..
	return (double)fValue;
}
DARKSDK LONGLONG CastFtoR(float fValue)
{
	return (LONGLONG)fValue;
}
DARKSDK DWORD CastBtoL(unsigned char cValue)
{
	int result = (int)cValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastBtoF(unsigned char cValue)
{
	float result = (float)cValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastBtoW(unsigned char cValue)
{
	WORD result = (WORD)cValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastBtoD(unsigned char cValue)
{
	DWORD result = (DWORD)cValue;
	return *((DWORD*)&result);
}
DARKSDK double CastBtoO(unsigned char cValue)
{
	return (double)cValue;
}
DARKSDK LONGLONG CastBtoR(unsigned char cValue)
{
	return (LONGLONG)cValue;
}
DARKSDK DWORD CastWtoL(WORD wValue)
{
	int result = (int)wValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastWtoF(WORD wValue)
{
	float result = (float)wValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastWtoB(WORD wValue)
{
	unsigned char result = (unsigned char)wValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastWtoD(WORD wValue)
{
	DWORD result = (DWORD)wValue;
	return *((DWORD*)&result);
}
DARKSDK double CastWtoO(WORD wValue)
{
	return (double)wValue;
}
DARKSDK LONGLONG CastWtoR(WORD wValue)
{
	return (LONGLONG)wValue;
}
DARKSDK DWORD CastDtoL(DWORD dwValue)
{
	int result = (int)dwValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastDtoF(DWORD dwValue)
{
	float result = (float)dwValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastDtoB(DWORD dwValue)
{
	unsigned char result = (unsigned char)dwValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastDtoW(DWORD dwValue)
{
	WORD result = (WORD)dwValue;
	return *((DWORD*)&result);
}
DARKSDK double CastDtoO(DWORD dwValue)
{
	return (double)dwValue;
}
DARKSDK LONGLONG CastDtoR(DWORD dwValue)
{
	return (LONGLONG)dwValue;
}
DARKSDK DWORD CastOtoL(double dValue)
{
	int result = (int)dValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastOtoF(double dValue)
{
	float result = (float)dValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastOtoB(double dValue)
{
	unsigned char result = (unsigned char)dValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastOtoW(double dValue)
{
	WORD result = (WORD)dValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastOtoD(double dValue)
{
	DWORD result = (DWORD)dValue;
	return *((DWORD*)&result);
}
DARKSDK LONGLONG CastOtoR(double dValue)
{
	LONGLONG result = (LONGLONG)dValue;
	return result;
}
DARKSDK DWORD CastRtoL(LONGLONG lValue)
{
	int result = (int)lValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastRtoF(LONGLONG lValue)
{
	float result = (float)lValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastRtoB(LONGLONG lValue)
{
	unsigned char result = (unsigned char)lValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastRtoW(LONGLONG lValue)
{
	WORD result = (WORD)lValue;
	return *((DWORD*)&result);
}
DARKSDK DWORD CastRtoD(LONGLONG lValue)
{
	DWORD result = (DWORD)lValue;
	return *((DWORD*)&result);
}
DARKSDK double CastRtoO(LONGLONG lValue)
{
	return (double)lValue;
}

// MATHEMATICAL COMMANDS
DBPRO_GLOBAL double gDegToRad = 3.141592654f/180.0f;
DBPRO_GLOBAL double gRadToDeg = 180.0f/3.141592654f;

DB_EXPORT dbReturnFloat_t AbsFF(float fValue)
{
	return dbReturnFloat( db3::Abs( fValue ) );
}

DARKSDK DWORD IntLF(float fValue)
{
	int result = (int)fValue;
	return *((DWORD*)&result);
}

DARKSDK DWORD AcosFF(float fValue)
{
	float result = (float)(acos(fValue)*gRadToDeg);
	return *((DWORD*)&result);
}

DARKSDK float Asin(float fValue)
{
	float result = (float)(asin(fValue)*gRadToDeg);
	return result;
}

DARKSDK DWORD AtanFF(float fValue)
{
	float result = (float)(atan(fValue)*gRadToDeg);
	return *((DWORD*)&result);
}

DARKSDK float Atan2(float fA, float fB)
{
	float result = (float)(atan2(fA, fB)*gRadToDeg);
	return result;
}

DARKSDK float Cos(float fAngle)
{
	return db3::Cos(fAngle);
}

DARKSDK float Sin(float fAngle)
{
	return db3::Sin(fAngle);
}

DARKSDK float Tan(float fAngle)
{
	return db3::Tan(fAngle);
}

DARKSDK float HcosFF(float fAngle)
{
	float result = (float)cosh(fAngle*gDegToRad);
	return result;
}

DARKSDK float HsinFF(float fAngle)
{
	float result = (float)sinh(fAngle*gDegToRad);
	return result;
}

DARKSDK float HtanFF(float fAngle)
{
	float result = (float)tanh(fAngle*gDegToRad);
	return result;
}

DARKSDK float Sqrt(float fValue)
{
	float result = (float)sqrt(fValue);
	return result;
}

DARKSDK float ExpFF(float fExp)
{
	float result = (float)exp(fExp);
	return result;
}

DB_EXPORT float SignF(float a)
{
	return db3::Sign( a );
}
DB_EXPORT float CopySign(float a, float b)
{
	return db3::CopySign( a, b );
}

DB_EXPORT int FloatToIntFast(float x) {
	return db3::FloatToIntFast( x );
}
DB_EXPORT DWORD FloatToDwordFast(float x) {
	return db3::FloatToUIntFast( x );
}

DB_EXPORT dbReturnFloat_t SqrtFast(float x) {
	return dbReturnFloat( db3::SqrtFast( x ) );
}
DB_EXPORT dbReturnFloat_t InvSqrtFast(float x) {
	return dbReturnFloat( db3::InvSqrtFast( x ) );
}

DB_EXPORT dbReturnFloat_t Lerp(float x, float y, float t) {
	return dbReturnFloat( db3::Lerp(x, y, t) );
}
	
DARKSDK void Randomize(int iSeed)
{
	srand(iSeed);
}

DARKSDK int Rnd(int r)
{
	int result=0;
	if(r>0)
	{
		// leefix - 250604 - u54 - 0 to 22 million now
		if ( r>1000 )  result += (rand()*1000);
		if ( r>100 ) result += (rand()*100);
		if ( r>10 ) result += (rand()*10);
		result += rand();
		result %= r+1;
	}
	return result;
}

// New MATH FUNCTIONS

DARKSDK float Ceil(float x)
{
	float value = ceil ( x );
	return value;
}

DARKSDK float Floor(float x)
{
	float value = floor ( x );
	return value;
}

// 3D MATH EXPRESSIONS

DARKSDK float wrapangleoffset(float da)
{
	// aaron - 20120811 - Faster version from NormalizeAngle360
	return db3::NormalizeAngle360(da);
}

DARKSDK float CurveValue(float a, float da, float sp)
{
	if(sp<1.0f) sp=1.0f;
	float diff = a-da;
	da=da+(diff/sp);
	return da;
}

DARKSDK float WrapValue(float da)
{
	return wrapangleoffset(da);
}

DARKSDK float NewXValue(float x, float a, float b)
{
	float da = x + ((float)sin(GGToRadian(a))*b);
	return da;
}

DARKSDK float NewZValue(float z, float a, float b)
{
	float da = z + ((float)cos(GGToRadian(a))*b);
	return da;
}

DARKSDK float NewYValueFFFF(float y, float a, float b)
{
	float da = y - ((float)sin(GGToRadian(a))*b);
	return da;
}

DARKSDK float CurveAngle(float a, float da, float sp)
{
	if(sp<1.0f) sp=1.0f;
	a = wrapangleoffset(a);
	da = wrapangleoffset(da);
	float diff = a-da;
	if(diff<-180.0f) diff=(a+360.0f)-da;
	if(diff>180.0f) diff=a-(da+360.0f);
	da=da+(diff/sp);
	da = wrapangleoffset(da);
	return da;
}

DB_EXPORT int NextPowerOfTwo1(int x) {
	return db3::NextPowerOfTwo(x);
}
DB_EXPORT int NextPowerOfTwo2(int x, int y) {
	return db3::NextSquarePowerOfTwo(x, y);
}

DB_EXPORT dbReturnFloat_t Clamp(float x, float l, float h) {
	return dbReturnFloat( db3::Clamp(x, l,h) );
}
DB_EXPORT dbReturnFloat_t ClampSNorm(float x) {
	return dbReturnFloat( db3::ClampSNorm(x) );
}
DB_EXPORT dbReturnFloat_t ClampUNorm(float x) {
	return dbReturnFloat( db3::ClampUNorm(x) );
}

DB_EXPORT dbReturnFloat_t Min(float x, float y) {
	return dbReturnFloat( db3::Min(x, y) );
}
DB_EXPORT dbReturnFloat_t Max(float x, float y) {
	return dbReturnFloat( db3::Max(x, y) );
}

// MISCLANIOUS CORE COMMANDS

DWORD g_dwAppLocalTimeStart = 0;

DARKSDK void SetLocalTimerReset(void)
{
	g_dwAppLocalTimeStart = timeGetTime();
}

DARKSDK int Timer(void)
{
	// leefix - 230606 - u62 - timeBeginPeriod/timeEndPeriod added
	timeBeginPeriod(1);
	DWORD dwTimer = timeGetTime() - g_dwAppLocalTimeStart;
	timeEndPeriod(1);
	int iTimer = (int)dwTimer;
	if ( iTimer < 0 ) 
	{
		// if computer running over 24 days solid, need to restart counter to keep inside INT bounds
		g_dwAppLocalTimeStart = timeGetTime();
		iTimer = 0;
	}
	return iTimer;
}

DARKSDK void SleepNow(int iDelay)
{
	DWORD dwTimeNow=timeGetTime();
	while(timeGetTime()<=dwTimeNow+iDelay)
	{
		if(InternalProcessMessages()==1) break;
	}
}

DARKSDK void WaitL(int iDelay)
{
	DWORD dwTimeNow=timeGetTime();
	while(timeGetTime()<=dwTimeNow+iDelay)
	{
		if(InternalProcessMessages()==1) break;
	}
}

DARKSDK void MemorySnapshot(int iMode)
{
	// also go through all DLLs in DBP and ask them to make a snapshot prior to this report
	for ( int iDLL=0; iDLL<1; iDLL++ )
	{
		HINSTANCE hThis = NULL;
		if ( iDLL==0 ) hThis = g_pGlob->g_Basic3D;
		if ( hThis )
		{
			typedef void ( *MM_SNAPSHOT ) ( void );
			MM_SNAPSHOT pSnapShotFunc;
			pSnapShotFunc = ( MM_SNAPSHOT ) GetProcAddress ( hThis, "?mm_SnapShot@@YAXXZ" );
			if ( pSnapShotFunc ) pSnapShotFunc();
		}
	}
}

DARKSDK void WaitForKey(void)
{
	while(g_wWinKey!=0)
	{
		if(InternalProcessMessages()==1) break;
	}
	while(g_wWinKey==0)
	{
		if(InternalProcessMessages()==1) break;
	}
}

DARKSDK void WaitForMouse(void)
{
	while(g_pGlob->iWindowsMouseClick!=0)
	{
		if(InternalProcessMessages()==1) break;
	}
	while(g_pGlob->iWindowsMouseClick==0)
	{
		if(InternalProcessMessages()==1) break;
	}
}

DARKSDK LPSTR Cl(void)
{
	// get command line from main program...
	LPSTR lpNewStr = NULL;
	if(g_pCommandLineString)
	{
		lpNewStr = new char[strlen(g_pCommandLineString)+1];
		strcpy(lpNewStr, g_pCommandLineString);
	}
	else
	{
		lpNewStr = new char[2];
		strcpy(lpNewStr, "");
	}
	return lpNewStr;
}

DARKSDK DWORD GetDate$(DWORD dwDestStr)
{
	char buf[256];
	_strdate(buf);
	return reinterpret_cast<DWORD>(dbReturnString(reinterpret_cast<char *>(dwDestStr), buf));
}

DARKSDK DWORD GetTime$(DWORD dwDestStr)
{
	char buf[256];
	_strtime(buf);
	return reinterpret_cast<DWORD>(dbReturnString(reinterpret_cast<char *>(dwDestStr), buf));
}

#include "..\..\GameGuru Core\GameGuru\Include\gameguru.h"

DARKSDK LPSTR Inkey(void)
{
	// Changed to use game guru t
	t.pInkeyString[0] = g_cInkeyCodeKey;
	t.pInkeyString[1] = 0;
	return t.pInkeyString;
}

DARKSDK void SyncOn(void)
{
	g_bSyncOff = false;
	g_bProcessorFriendly = false;
	g_bCanRenderNow = false;
}

DARKSDK void SyncOff(void)
{
	g_bSyncOff = true;
	g_bProcessorFriendly = true;
	g_bCanRenderNow = true;
}

DARKSDK void Sync(void)
{
	ExternalDisplaySync(0);
	ProcessMessagesOnly();
	ConstantNonDisplayUpdate();// this is DUPLICATED in the ExternalDisplaySync(0) call
	g_bCanRenderNow = true;
}

DARKSDK void Sync(int iProcessMessages)
{
	ExternalDisplaySync(0);
	if ( iProcessMessages==1 ) ProcessMessagesOnly();
	ConstantNonDisplayUpdate();
	g_bCanRenderNow = true;
}

DARKSDK void FastSync(void)
{
	ExternalDisplaySync(1);
	g_bCanRenderNow = true;
}

DARKSDK void FastSyncInputOnly(void)
{
	ProcessMessagesOnly();
}

DARKSDK void FastSync ( int iNonDisplayUpdates )
{
	ExternalDisplaySync(1);
	if ( iNonDisplayUpdates==1 )
	{
		// leeadd - 061108 - reinstated for U71 by request under parameter
		ConstantNonDisplayUpdate();
	}
	g_bCanRenderNow = true;
}

DARKSDK void SyncRate(int iRate)
{
	// Reset everything to run full speed
	SAFE_DELETE_ARRAY( g_pdwSyncRateSetting);
	g_dwSyncRateSettingSize = 0;
	g_dwManualSuperStepSetting = 0;

	// Zero is full speed
	// Anything over 1000 can't be measured, so treat that as full speed too
	if (iRate == 0 || iRate > 1000)
		return;

	// Negative is super stepping mode
	if (iRate < 0)
	{
		g_dwManualSuperStepSetting = abs(iRate);
		return;
	}

	// What's left can be dealt with.
	// Generate a table that covers 1 second of frames and fill it out with
	// the basic MS-per-frame value. Any milliseconds dropped using the calculation
	// will be evenly distributed within the table.
	g_dwSyncRateSettingSize = iRate;
	g_pdwSyncRateSetting = new DWORD[ iRate ];

	DWORD RoundedMS                 =   1000 / iRate;
	DWORD DroppedTotalMS            =   1000 - (RoundedMS * iRate);
	float DroppedPerFrameMS         =   (float)(DroppedTotalMS) / iRate;
	float AccumulatedDroppedMS      =   0.0;

	for (int i = 0; i < iRate; ++i)
	{
		if (AccumulatedDroppedMS >= 1.0)
		{
			g_pdwSyncRateSetting[i] = RoundedMS + 1;
			AccumulatedDroppedMS -= 1.0;
			--DroppedTotalMS;
		}
		else
		{
			g_pdwSyncRateSetting[i] = RoundedMS;
		}
		AccumulatedDroppedMS += DroppedPerFrameMS;
	}

	// Any further dropped milliseconds, just use them against entries that haven't
	// already had them added previously until they are all used up.
	// This needed because of float (in)accuracy.
	for (int i = 0; i < iRate && DroppedTotalMS > 0; ++i)
	{
		if (g_pdwSyncRateSetting[i] == RoundedMS)
		{
			++g_pdwSyncRateSetting[i];
			--DroppedTotalMS;
		}
	}
}

DWORD GetNextSyncDelay()
{
	// If there is no table, then run return a 'no delay'
	if (g_dwSyncRateSettingSize == 0)
		return 0;

	// Advance the index, reset to start if gone beyond the end of the table
	++g_dwSyncRateCurrent;
	if (g_dwSyncRateCurrent >= g_dwSyncRateSettingSize)
		g_dwSyncRateCurrent = 0;

	// Return the current delay
	return g_pdwSyncRateSetting[ g_dwSyncRateCurrent ];
}

DARKSDK void DrawToBack(void)
{
	g_bDrawAutoStuffFirst = false;
}

DARKSDK void DrawToFront(void)
{
	g_bDrawAutoStuffFirst = true;
}

DARKSDK void DrawToCamera(void)
{
	g_bDrawEntirelyToCamera = true;
}

DARKSDK void DrawToScreen(void)
{
	g_bDrawEntirelyToCamera = false;
}

DARKSDK void DrawSpritesFirst(void)
{
	g_bDrawSpritesFirst=true;
}

DARKSDK void DrawSpritesLast(void)
{
	g_bDrawSpritesFirst=false;
}

DARKSDK void SaveArray(LPSTR pFilename, DWORD dwAllocation)
{
	// Temp vars
	DWORD written;

	// If Array Exists
	if(dwAllocation)
	{
		// Header Info
		DWORD dwHeaderSizeInBytes = HEADERSIZEINBYTES;
		DWORD dwSizeOfArray = *((DWORD*)dwAllocation-4);
		DWORD dwElementSize = *((DWORD*)dwAllocation-3);
		DWORD dwExistingElementType = *((DWORD*)dwAllocation-2);
		DWORD dwTableSizeInBytes = dwSizeOfArray * 4;

		// Can only save pure types
		if(dwExistingElementType<9)
		{
			// Create File for array
			HANDLE hFile = CreateFile(pFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile!=INVALID_HANDLE_VALUE)
			{
				// String arrays can be text file dumps
				if(dwExistingElementType==2)
				{
					// Save Out Array (of x size)
					DWORD dwDataPointer=dwAllocation+dwTableSizeInBytes;
					for(DWORD n=0; n<dwSizeOfArray; n++)
					{
						DWORD* pEntry = *((DWORD**)dwAllocation+n);
						if(pEntry)
						{
							// String data
							DWORD dwStringSize=0;
							LPSTR pStr = (LPSTR)*pEntry;
							if(*pEntry) dwStringSize = strlen(pStr);
							if(dwStringSize>0) WriteFile(hFile, pStr, dwStringSize, &written, FALSE);

							// carriage return
							char CR[2]; CR[0]=13; CR[1]=10;
							WriteFile(hFile, &CR, 2, &written, FALSE);
						}
					}
				}
				else
				{
					// Write Type of Array (element type 2=string)
					WriteFile(hFile, &dwExistingElementType, 4, &written, FALSE);

					// Write Size of Array (elements)
					WriteFile(hFile, &dwSizeOfArray, 4, &written, FALSE);

					// Save Out Array (of x size)
					DWORD dwDataPointer=dwAllocation+dwTableSizeInBytes;
					for(DWORD n=0; n<dwSizeOfArray; n++)
					{
						DWORD* pEntry = *((DWORD**)dwAllocation+n);
						if(pEntry)
						{
							// Write Index + Datablock
							int indexn=(int)n;
							WriteFile(hFile, &indexn, 4, &written, FALSE);

							// Value
							WriteFile(hFile, pEntry, dwElementSize, &written, FALSE);
						}
					}

					// Write Index of -1 to end
					int endn=-1;
					WriteFile(hFile, &endn, 4, &written, FALSE);
				}

				// Close file
				CloseHandle(hFile);
			}
			else
			{
				// runtime - could not create array file
				char pErrStr[1024];
				sprintf ( pErrStr, "Failed to CreateFile with: %s", pFilename );
				Message ( 0, pErrStr, "" );
				RunTimeError(RUNTIMEERROR_INVALIDFILE);
			}
		}
		else
		{
			//runtime not right type
			RunTimeError(RUNTIMEERROR_ARRAYTYPEINVALID);
		}
	}
}

DARKSDK void LoadArrayCore(LPSTR pFilename, DWORD dwAllocation)
{
	// Temp vars
	DWORD readen;

	// If Array Exists
	if(dwAllocation)
	{
		// Header Info
		DWORD dwHeaderSizeInBytes = HEADERSIZEINBYTES;
		DWORD dwExistingSizeOfArray = *((DWORD*)dwAllocation-4);
		DWORD dwElementSize = *((DWORD*)dwAllocation-3);
		DWORD dwExistingElementType = *((DWORD*)dwAllocation-2);
		DWORD dwTableSizeInBytes = dwExistingSizeOfArray * 4;

		// Can only save pure types
		if(dwExistingElementType<9)
		{
			// Load File for array
			HANDLE hFile = CreateFile(pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile!=INVALID_HANDLE_VALUE)
			{
				// If array string, load in as string table
				if(dwExistingElementType==2)
				{
					// Read Type of Array (element type 2=string)
					DWORD dwDataSize=GetFileSize(hFile, 0);
					LPSTR pData=new char[dwDataSize+2];
					ReadFile(hFile, pData, dwDataSize, &readen, FALSE);
					pData[dwDataSize]=0;
					pData[dwDataSize+1]=0;

					// Scan all lines into array
					int arrindex = 0;
					LPSTR pPtr = pData;
					LPSTR pStart = pPtr;
					LPSTR pPtrEnd = pData + dwDataSize;
					while(pPtr<=pPtrEnd && arrindex<(int)dwExistingSizeOfArray)
					{
						if( (*(pPtr+0)==13 && *(pPtr+1)==10) || *(pPtr+0)==0 )
						{
							DWORD* pEntry = *((DWORD**)dwAllocation+arrindex);
							if(pEntry)
							{
								// Free any existing string
								if(*pEntry) delete[] (LPSTR)(*pEntry);

								// Make string
								LPSTR pNewStr = NULL;
								DWORD dwStringSize=pPtr-pStart;
								pNewStr = new char[dwStringSize+1];
								memcpy(pNewStr, pStart, dwStringSize);
								pNewStr[dwStringSize]=0;

								// New string
								*pEntry = (DWORD)pNewStr;
								arrindex++;
							}

							// Next line
							pStart = pPtr+2;
							pPtr++;
						}
						pPtr++;
					}

					// Free data
					delete[] pData;
				}
				else
				{
					// Read Type of Array (element type 2=string)
					DWORD dwElementType=0;
					ReadFile(hFile, &dwElementType, 4, &readen, FALSE);

					// Read Size of Array (elements)
					DWORD dwSizeOfArray=0;
					ReadFile(hFile, &dwSizeOfArray, 4, &readen, FALSE);

					// Verify corect array loaded into
					if(dwElementType==dwExistingElementType && dwSizeOfArray==dwExistingSizeOfArray)
					{
						// Clear Array of old data
						DWORD dwDataPointer=dwAllocation+dwTableSizeInBytes;
						DWORD dwDataBlockSizeInBytes = dwSizeOfArray * dwElementSize;
						ZeroMemory((LPSTR)dwDataPointer, dwDataBlockSizeInBytes);

						// Load In Array (of x size)
						int arrindex = 0;
						ReadFile(hFile, &arrindex, 4, &readen, FALSE);
						while(arrindex!=-1)
						{
							DWORD* pEntry = *((DWORD**)dwAllocation+arrindex);
							if(pEntry)
							{
								// Value
								ReadFile(hFile, pEntry, dwElementSize, &readen, FALSE);
							}

							// Read ext index
							ReadFile(hFile, &arrindex, 4, &readen, FALSE);
						}
					}
					else
					{
						// runtime not same aray
					}
				}
			
				// Close file
				CloseHandle(hFile);
			}
			else
			{
				// runtime - could not read array file
				RunTimeError(RUNTIMEERROR_FILENOTEXIST,pFilename);
			}
		}
		else
		{
			//runtime not right type
			RunTimeError(RUNTIMEERROR_ARRAYTYPEINVALID);
		}
	}
}

DARKSDK void LoadArray( LPSTR szFilename, DWORD dwAllocation )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	CheckForWorkshopFile ( VirtualFilename );

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( (DWORD)VirtualFilename );
	LoadArrayCore ( VirtualFilename, dwAllocation );
	g_pGlob->Encrypt( (DWORD)VirtualFilename );
}

//
// DX Detect Check (from globstruct filled in DarkEXE)
//

DARKSDK DWORD GetDXVer$(DWORD dwDestStr)
{
	char buf[256];
	buf[0] = '\0';
	return reinterpret_cast<DWORD>(dbReturnString(reinterpret_cast<char *>(dwDestStr), buf));
}

//
// Suspend App - used when multiple apps running, want to shut one down
//

DARKSDK void AlwaysActiveOff ( void )
{
	// Will shut down all 3D, sound and music processing (plus any secondary monitoring)
	// Will keep input and general program execution naturally
	g_bAlwaysActiveOff = true;
	g_bAlwaysActiveOneOff = false;
}

DARKSDK void AlwaysActiveOn ( void )
{
	// Restores systems previously shutdown with AlwaysActiveOff
	g_bAlwaysActiveOff = false;
}

DARKSDK void EarlyEnd ( void )
{
	// Report an error
	MessageBox ( NULL, "You have hit a FUNCTION declaration mid-program!", "Early Exit Error", MB_OK );
}

DARKSDK void SyncSleep ( int iFlag )
{
	// controls process friendly flag
	if ( iFlag==1 ) 
		g_bProcessorFriendly = true;
	else
		g_bProcessorFriendly = false;
}

DARKSDK void SyncMask ( DWORD dwMask )
{
	// copy to master sync mask
	g_dwSyncMask = dwMask;
}

DARKSDK void SyncMaskOverride ( DWORD dwMask )
{
	// used to override ALL camera rendering (for a loading sequence)
	g_dwSyncMaskOverride = dwMask;
}

DARKSDK DWORD GetArrayType(DWORD dwArrayPtr)
{
	// return array size
	if(dwArrayPtr) 
	{
		DWORD dwTypeIndex = (*((DWORD*)dwArrayPtr-2));
		return dwTypeIndex;
	}
	else
		return 0;
}

LPSTR GetTypePatternCore ( LPSTR dwTypeName, DWORD dwTypeIndex )
{
	// U73 - 210309 - if basic string, return simple STRING pattern
	if ( dwTypeIndex==2 )
	{
		LPSTR pSimplePattern = new char[2];
		strcpy ( pSimplePattern, "S" );
		return pSimplePattern;
	}

	// U73 - 210309 - if no structures, exit now as rest is structure type stuff only
	if ( g_dwStructPatternQty==0 )
		return NULL;

	// look for type that matches name
	DWORD dwPatternDataBeginsAt = 0;
	if ( g_pStructPatternsPtr )
	{
		if ( dwTypeName )
		{
			LPSTR pFindName = new char[strlen((LPSTR)dwTypeName)+2];
			strcpy ( pFindName, (LPSTR)dwTypeName );
			strcat ( pFindName, ":" );
			DWORD dwFindLength = strlen ( pFindName );
			for ( DWORD dwI=0; dwI<g_dwStructPatternQty-dwFindLength; dwI++ )
			{
				if ( strnicmp ( g_pStructPatternsPtr+dwI, pFindName, dwFindLength )==NULL )
				{
					dwPatternDataBeginsAt = dwI+dwFindLength;
					break;
				}
			}
			delete[] pFindName;
		}
		if ( dwTypeIndex>0 )
		{
			LPSTR pFindName = new char[g_dwStructPatternQty+1];
			wsprintf ( pFindName, ":%d:", dwTypeIndex );
			DWORD dwFindLength = strlen ( pFindName );
			for ( DWORD dwI=0; dwI<g_dwStructPatternQty-dwFindLength; dwI++ )
			{
				if ( strnicmp ( g_pStructPatternsPtr+dwI, pFindName, dwFindLength )==NULL )
				{
					dwPatternDataBeginsAt = dwI+dwFindLength;
					break;
				}
			}
			delete[] pFindName;
		}
	}

	// copy pattern to return string, or null
	LPSTR lpNewStr = new char[(strlen(g_pStructPatternsPtr)-dwPatternDataBeginsAt)+1];
	if ( dwPatternDataBeginsAt > 0 )
	{
		// get type index, then go to get pattern
		if ( dwTypeName )
		{
			LPSTR lpNum = new char[(strlen(g_pStructPatternsPtr)-dwPatternDataBeginsAt)+1];
			LPSTR pSourceStr = g_pStructPatternsPtr + dwPatternDataBeginsAt;
			strcpy ( lpNum, pSourceStr );
			DWORD dwI = 0;
			for (; dwI<strlen(pSourceStr); dwI++ )
			{
				if ( lpNum[dwI]==':' )
				{
					lpNum[dwI]=0;
					break;
				}
			}
			delete[] lpNum;
			dwPatternDataBeginsAt += dwI + 1;
		}

		// get pattern, then cut off at : colon
		LPSTR pSourceStr = g_pStructPatternsPtr + dwPatternDataBeginsAt;
		strcpy ( lpNewStr, pSourceStr );
		for ( DWORD dwI=0; dwI<strlen(pSourceStr); dwI++ )
		{
			if ( lpNewStr[dwI]==':' )
			{
				lpNewStr[dwI]=0;
				break;
			}
		}
	}
	else
		strcpy ( lpNewStr, "" );

	// return pattern from type found
	return lpNewStr;
}

DARKSDK DWORD GetTypePattern$(DWORD dwDestStr,DWORD dwTypeName,DWORD dwTypeIndex)
{
	DWORD r;

	// determine if type name string passed in has contents
	LPSTR pTypeName = NULL;
	if ( dwTypeName )
		if ( strlen ( (LPSTR)dwTypeName ) > 0 )
			pTypeName = (LPSTR)dwTypeName;

	// get pattern from type name
	LPSTR lpNewStr = GetTypePatternCore( pTypeName, dwTypeIndex );

	r = reinterpret_cast<DWORD>(dbReturnString(reinterpret_cast<char *>(dwDestStr), lpNewStr));
	delete [] lpNewStr;

	return r;
}

// Get/Set data pointers
DARKSDK void GetDataPointers(LPSTR* Start, LPSTR* End, LPSTR* Current)
{
	if (Start)      *Start      = g_pDataLabelStart;
	if (End)        *End        = g_pDataLabelEnd;
	if (Current)    *Current    = g_pDataLabelPtr;
}

DARKSDK void SetDataPointer(LPSTR Current)
{
	if (Current < g_pDataLabelStart)
		Current = g_pDataLabelStart;
	if (Current > g_pDataLabelEnd)
		Current = g_pDataLabelEnd;
	g_pDataLabelPtr = Current;
}
