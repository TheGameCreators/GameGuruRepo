//
// DBDLLCore Internal Functions
//

// Common Includes
#define _CRT_SECURE_NO_DEPRECATE
#include "stdio.h"

// Internal Includes
#include "DBDLLCore.h"
#include "DBDLLDisplay.h"
#include "DBDLLCoreInternal.h"

// External Data Pointer
extern void ExternalDisplaySync(int iSkipSyncDelay);
extern GlobStruct g_Glob;

DARKSDK void InputSomething(LPSTR* pStr)
{
	// Clear Windows Entry Field
	InternalClearWindowsEntry();

	// Set Input Data
	int iEntryX = g_Glob.iCursorX;
	int iEntryY = g_Glob.iCursorY;
	int iLeftmostDeletableX = g_Glob.iCursorX;
	int iRightmostDeletableX = g_Glob.iCursorX;
	DWORD dwLocalInputWorkSize = 32;
	LPSTR pLocalInputWorkString = new char[dwLocalInputWorkSize];
	strcpy(pLocalInputWorkString,"");
	DWORD dwLocalInputWorkPos = 0;

	// Loop to Gather Data
	int iLastStringLen=-1;
	bool bStayInInputLoop=true;
	DWORD dwTraceInputForExitChar=0;
	while(bStayInInputLoop==true)
	{
		// Check entry for exit char
		if(dwTraceInputForExitChar<g_dwWindowsTextEntryPos)
		{
			// Get current char from windows entry string
			unsigned char cThisChar = g_Glob.pWindowsTextEntry[dwTraceInputForExitChar];
			dwTraceInputForExitChar++;

			// Check char for exit value (exit on ESCAPE or RETURN)
			if(cThisChar==13 || cThisChar==27)
			{
				// Exit string entry
				g_iEntryCursorState=0;
				bStayInInputLoop=false;
			}
			else
			{
				// Assign char to work string
				if(dwLocalInputWorkPos>=4)
				{
					if(dwLocalInputWorkPos+4>dwLocalInputWorkSize)
					{
						dwLocalInputWorkSize = dwLocalInputWorkSize * 2;
						LPSTR pNewString = new char[dwLocalInputWorkSize];
						strcpy(pNewString, pLocalInputWorkString);
						SAFE_DELETE(pLocalInputWorkString);
						pLocalInputWorkString=pNewString;
					}
				}

				// Advance work string position
				if(cThisChar==8)
				{
					if(dwLocalInputWorkPos>0)
					{
						dwLocalInputWorkPos--;
						pLocalInputWorkString[dwLocalInputWorkPos]=0;
					}
				}
				else
				{
					pLocalInputWorkString[dwLocalInputWorkPos]=cThisChar;
					pLocalInputWorkString[dwLocalInputWorkPos+1]=0;
					dwLocalInputWorkPos++;
				}
			}
		}
		else
		{
			// means we have backspaced
			if(dwTraceInputForExitChar>g_dwWindowsTextEntryPos)
			{
				// remove from local input work string
				dwLocalInputWorkPos--;
				pLocalInputWorkString[dwLocalInputWorkPos]=0;
				dwTraceInputForExitChar--;
			}
		}
		// Display Input
		LPSTR pForPrint = new char[dwLocalInputWorkSize+4];
		strcpy(pForPrint, pLocalInputWorkString);
		if ( (int)strlen(pLocalInputWorkString)>iLastStringLen )
		{
			// leeadd - 300305 - no underscore when advancing
		}
		else
			if(g_iEntryCursorState==1)
				strcat(pForPrint, "_");

		if(g_Glob.iCursorX<iRightmostDeletableX)
		{
			// Clear text overrun
			iLeftmostDeletableX = g_Glob.iCursorX;
		}
		iRightmostDeletableX = g_Glob.iCursorX;
		SAFE_DELETE(pForPrint);

		// Handle Cursor Flashing
		g_iEntryCursorState=1-g_iEntryCursorState;

		// May have sync on, input overrides this
		ExternalDisplaySync(0);

		// Process message loop while taking input
		if(InternalProcessMessages()==1)
			break;
	}

	// Advance Cursor
	g_Glob.iCursorX=0; g_Glob.iCursorY+=0;

	// Create Output String
	if(pLocalInputWorkString)
	{
		DWORD length = strlen(pLocalInputWorkString);
		LPSTR pNewStr = new char[length+1];
		strcpy(pNewStr, pLocalInputWorkString);
		(*pStr)=pNewStr;
	}

	// Free usages
	SAFE_DELETE(pLocalInputWorkString);
}

DARKSDK void InputInteger(LONGLONG* plValue)
{
	LPSTR pStr = NULL;
	InputSomething(&pStr);
	if(pStr)
		*plValue = _atoi64(pStr);
	else
		*plValue = 0;

	SAFE_DELETE(pStr);
}

DARKSDK void InputFloat(double* pdValue)
{
	LPSTR pStr = NULL;
	InputSomething(&pStr);
	if(pStr)
		*pdValue = atof(pStr);
	else
		*pdValue = 0;

	SAFE_DELETE(pStr);
}

DARKSDK void InputString(LPSTR* ppStr)
{
	InputSomething(ppStr);
}
