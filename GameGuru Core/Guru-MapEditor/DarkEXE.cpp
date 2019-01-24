//
// DarkEXE
//

// Internal Includes
#define _CRT_SECURE_NO_DEPRECATE

#pragma warning(disable : 4099)
#pragma warning(disable : 4996)

#include "FileReader.h"
#include "windows.h"
#include "direct.h"
#include "time.h"

// Defines and Externs
#include "DarkEXE.h"
#include "resource.h"
#include "globstruct.h"
#include "EXEBlock.h"
#include "macros.h"
#include "CFileC.h"
#include "io.h"

// globals
extern struct _finddata_t	filedata;
extern long					hInternalFile;
extern int					FileReturnValue;
time_t					TodaysDay					= 0;

// Interal DarkEXE Data Variables
LPSTR								gRefCommandLineString=NULL;
char								gUnpackDirectory[_MAX_PATH];
char								gpDBPDataName[_MAX_PATH];
DWORD								gEncryptionKey;

// EXECUTABLE Class
CEXEBlock							CEXE;

// Populates a string array with all runtime errors
void FillDarkErrors ( void );

// DebugDump needs this to get at useful debug data (U5.8)
//extern GlobStruct*				g_pGlob;
GlobStruct							g_Glob;
GlobStruct*							g_pGlob = &g_Glob;

// Temporary Window while loading DLLs
HWND								g_hTempWindow = NULL;

// IGLOADER can send WM_SETTEXT to the temp window, DBP executables attempt to 'EMBED'
HWND								g_igLoader_HWND = NULL;

// LOAD EXE DATA AND RUN

bool RunProgram(HINSTANCE hInstance, LPSTR* pReturnError)
{
	// Result Var
	bool bResult = true;
	bool bSuccessfulDLLLinks = false;

	// Make program
	bResult = CEXE.Init(hInstance, bResult, gRefCommandLineString);

	// LEEADD - 221008 - U71 - EXTERNAL DEBUGGER SUPPORT
	/*
	char pUniqueMutexName[512];
	strcpy(pUniqueMutexName, CEXE.m_pAbsoluteAppFile);
	strcat(pUniqueMutexName, "(Mutex)");
	for (DWORD n = 0; n < strlen(pUniqueMutexName); n++)
	{
		if (pUniqueMutexName[n] == ':') pUniqueMutexName[n] = '_';
		if (pUniqueMutexName[n] == '\\') pUniqueMutexName[n] = '_';
		if (pUniqueMutexName[n] == '/') pUniqueMutexName[n] = '_';
	}
	HANDLE pAppMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, pUniqueMutexName);
	if (pAppMutex)
	{
		// it appears another process has already created an identical mutex
		// (which can happen if multiple programs with the same name are running)
		// so we then check a shared storage location to see whether debugging
		// has been requested by the 'external process' that may have created a mutex
		char pSharedStringStorage[512];
		strcpy(pSharedStringStorage, "");
		char pUniqueFileMapName[512];
		strcpy(pUniqueFileMapName, CEXE.m_pAbsoluteAppFile);
		strcat(pUniqueFileMapName, "(FileMap)");
		for (DWORD n = 0; n < strlen(pUniqueFileMapName); n++)
		{
			if (pUniqueFileMapName[n] == ':') pUniqueFileMapName[n] = '_';
			if (pUniqueFileMapName[n] == '\\') pUniqueFileMapName[n] = '_';
			if (pUniqueFileMapName[n] == '/') pUniqueFileMapName[n] = '_';
		}

		HANDLE hFileMap = OpenFileMapping(FILE_MAP_READ, FALSE, pUniqueFileMapName);
		if (hFileMap)
		{
			LPVOID lpVoid = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
			if (lpVoid)
			{
				DWORD dwDataSize = 0;
				dwDataSize = *((LPDWORD)lpVoid);
				if (dwDataSize > 0)
				{
					LPSTR pLoadData = (LPSTR)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwDataSize + 1);
					memcpy(pLoadData, (LPSTR)lpVoid + 4, dwDataSize);
					strcpy(pSharedStringStorage, pLoadData); // takes the shared string ehre!
					GlobalFree(pLoadData);
				}
				UnmapViewOfFile(lpVoid);
			}
			CloseHandle(hFileMap);
		}
		if (stricmp(pSharedStringStorage, "debugme") == NULL)
		{
			// a mutex for this app exists and the shared storage says it wants to
			// DEBUG this application, so we pause here until we can own the mutex
			// (which is achieved by the current owner of the mutex releasing it)
			// but before we pause, we must replace the string in the shared storage
			// so some new data which tells the debugger we are here now and waiting
			// for ownership of the mutex so we can start running the program. In 
			// order to keep this section simple, we simply pass in the memory address
			// of the GLOBSTRUCT data, which includes all the information needed
			DWORD dwWriteDataSize = 4;
			HANDLE hWriteFileMap = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, dwWriteDataSize, pUniqueFileMapName);
			if (hWriteFileMap)
			{
				LPVOID lpWriteVoid = MapViewOfFile(hWriteFileMap, FILE_MAP_WRITE, 0, 0, dwWriteDataSize);
				if (lpWriteVoid)
				{
					// Copy to Virtual File
					*(DWORD*)lpWriteVoid = (DWORD)g_pGlob;
					UnmapViewOfFile(lpWriteVoid);
				}
				CloseHandle(hWriteFileMap);
			}

			// friendly message
			MessageBox(NULL, pUniqueMutexName, "DBP App has deposited the glob struct in the filemap as a DWORD, and now wants to OWN this mutex...give it to me!", MB_OK);

			// now wait for the external debugger to release the mutex
			DWORD dwWaitResult = WaitForSingleObject(pAppMutex, 5000L);
			switch (dwWaitResult)
			{
				// The thread got mutex ownership.
				case WAIT_OBJECT_0:		dwWaitResult = dwWaitResult; break;
				case WAIT_TIMEOUT:		dwWaitResult = dwWaitResult; break;
				case WAIT_ABANDONED:	dwWaitResult = dwWaitResult; break;
			}
		}
		else
		{
			// for whatever reason, the shared string did NOT contain the text
			// which would trigger this application to seek ownership of the 
			// mutex and so can carry on without attempting to own this mutex
			// as it was probably created by another app with the same mutex name
		}

		// close the mutex handle as we are finished with it
		CloseHandle(pAppMutex);
	}
	else
	{
		// could not find a MUTEX open, so no other apps with same mutex name
		// are running, which includes any external debuggers so we continue as normal
	}
	*/

	// Run the EXE Program
	bResult = CEXE.Run(bResult);

	/* never used - 240417
	// Report Any Runtime Errors
	DWORD dwRTError=CEXE.m_dwRuntimeErrorDWORD;
	DWORD dwRTErrorLine=CEXE.m_dwRuntimeErrorLineDWORD;
	if(dwRTError>0)
	{
		// create report string and store
		*pReturnError = new char[1024];
		LPSTR pRuntimeErrorString = NULL;
		if(CEXE.m_pRuntimeErrorStringsArray) pRuntimeErrorString = (LPSTR)CEXE.m_pRuntimeErrorStringsArray[dwRTError];
		wsprintf(*pReturnError, "Runtime Error %d - %s in section %d", dwRTError, pRuntimeErrorString, dwRTErrorLine);
		bResult=false;
	}
	*/

	// Return Result
	return bResult;
}

void DeleteContentsOfDBPDATA ( bool bOnlyIfOlderThan2DAYS )
{
	// Delete all files in current folder
	FFindFirstFile();
	int iAttempts=20;
	while(FGetFileReturnValue()!=-1L && iAttempts>0)
	{
		// only if older than 2 days
		bool bGo = false;
		if ( bOnlyIfOlderThan2DAYS==false ) bGo = true;
		if ( bOnlyIfOlderThan2DAYS==true )
		{
			time_t ThisFileDays = filedata.time_write / 60 / 60 / 24;
			time_t Difference = TodaysDay - ThisFileDays; 
			if ( Difference >= 2 )
			{
				// this file is at least 2 days old
				bGo = true;
			}
		}

		// go
		if ( bGo )
		{
			if(FGetActualTypeValue(filedata.attrib)==1)
			{
				if(stricmp(filedata.name, ".")!=NULL
				&& stricmp(filedata.name, "..")!=NULL)
				{
					char file[_MAX_PATH];
					strcpy(file, filedata.name);
					char old[_MAX_PATH];
					getcwd(old, _MAX_PATH);
					BOOL bResult = RemoveDirectory(file);
					if(bResult==FALSE)
					{
						_chdir(file);
						FFindCloseFile();
						DeleteContentsOfDBPDATA ( bOnlyIfOlderThan2DAYS );
						_chdir(old);
						BOOL bResult = RemoveDirectory(file);
						FFindFirstFile();
					}
					iAttempts--;
				}
			}
			else
			{
				DeleteFile(filedata.name);
			}
		}
		FFindNextFile();
	}
	FFindCloseFile();
}

void MakeOrEnterUniqueDBPDATA(void)
{
	// Default DBPDATA string
	strcpy(gpDBPDataName,"dbpdata");

	// Prepare DBPDATA string builder
	DWORD dwBuildID=2;
	char pBuildStart[_MAX_PATH];
	strcpy(pBuildStart, gpDBPDataName);

	// Make it as unique
	mkdir(gpDBPDataName);

	// get todays day when made directory
	FFindFirstFile();
	TodaysDay = filedata.time_write / 60 / 60 / 24;
	FFindCloseFile();
}

void DeleteAllOldDBPDATAFolders(void)
{
	// Default DBPDATA string
	strcpy(gpDBPDataName,"dbpdata");

	// Prepare DBPDATA string builder
	DWORD dwBuildID=2;
	char pBuildStart[_MAX_PATH];
	strcpy(pBuildStart, gpDBPDataName);

	// Go for a million attempts
	while(dwBuildID<1000000)
	{
		// Check if DBPDATA folder exists
		if(_chdir(gpDBPDataName)!=-1 || dwBuildID==2)
		{
			// Delete contents of directory
			DeleteContentsOfDBPDATA(true);

			// Delete directory (if empty)
			_chdir("..");
			rmdir(gpDBPDataName);

			// Create new folder name from build data
			wsprintf(gpDBPDataName, "%s%d", pBuildStart, dwBuildID);
		}
		else
		{
			// Done scanning
			break;
		}

		// Will change foldername slightly
		dwBuildID++;
	}
}

// DUMP DEBUG REPORT

void DumpDebugReport ( void )
{
	// Setup Report (by date and time)
	char pReportDate [ _MAX_PATH ];
	_strdate ( pReportDate );
	for ( DWORD i=0; i<strlen(pReportDate); i++ )
		if ( pReportDate[i]=='/' )
			pReportDate[i]='_';

	// Current location
	char currentdir [ _MAX_PATH ];
	_getcwd ( currentdir, _MAX_PATH );

	// Construct local directory
	char pReportFile [ _MAX_PATH ];
	strcpy ( pReportFile, currentdir );
	strcat ( pReportFile, "\\CrashOn_" );
	strcat ( pReportFile, pReportDate );
	strcat ( pReportFile, ".txt" );

	// Create Report File (by date and time)
	char pLineToReport [ _MAX_PATH ];
	wsprintf ( pLineToReport, "%s", pReportFile );
	WritePrivateProfileString ( "COMMON", "PathToEXE", pReportFile, pReportFile );
	if ( CEXE.m_dwRuntimeErrorDWORD==0 )
	{
		// crashed out inside a function - hard crash (ie access NULL ptr)
		if ( g_pGlob )
			wsprintf ( pLineToReport, "Internal Code:%d", g_pGlob->dwInternalFunctionCode );
		else
			wsprintf ( pLineToReport, "Unknown Internal Location - email this file and TEMP\\FullSourceDump.dba to bugs@thegamecreators.com" );
	}
	else
	{
		// regular runtime error
		wsprintf ( pLineToReport, "%d", CEXE.m_dwRuntimeErrorDWORD );
	}
	WritePrivateProfileString ( "CEXE", "m_dwRuntimeErrorDWORD", pLineToReport, pReportFile );
	wsprintf ( pLineToReport, "%d", CEXE.m_dwRuntimeErrorLineDWORD );
	WritePrivateProfileString ( "CEXE", "m_dwRuntimeErrorLineDWORD", pLineToReport, pReportFile );			
}

bool FileExists(LPSTR pFilename)
{
	HANDLE hFile = CreateFile(pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile=NULL;
		return true;
	}
	return false;
}

// WINDOWS MAIN FUNCTION

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Store reference to CL$()
	#ifdef _DEBUG
	// Removed in favor of automated attachment of debugger to child process
	// buy this: http://entrian.com/attach/download.html
	// MessageBox ( NULL, "Debug Mode - You can attach VS debugger to Guru-MapEditor.exe now", "", MB_OK );
	#endif
	gRefCommandLineString=lpCmdLine;

	// Prepare Virtual Directory from Data Appended to EXE File
	char ActualEXEFilename[_MAX_PATH];
	GetModuleFileName(hInstance, ActualEXEFilename, _MAX_PATH);

	// Get current working directory (for temp folder compare)
	char CurrentDirectory[_MAX_PATH];
	getcwd(CurrentDirectory, _MAX_PATH);

	//char whereaminow[256];
	//getcwd(whereaminow, _MAX_PATH);

	// new define for an EXE which uses local exe location for extraction TEMP
	char WindowsTempDirectory[_MAX_PATH];
	#ifdef LOCALLYEXTRACTINGEXE
	// The TEMP folder is the EXE folder
	strcpy ( WindowsTempDirectory, CurrentDirectory );
	MakeOrEnterUniqueDBPDATA();
	strcat(WindowsTempDirectory, "\\");
	strcat(WindowsTempDirectory, gpDBPDataName);
	strcpy(gUnpackDirectory, WindowsTempDirectory);
	chdir(gpDBPDataName);
	DeleteContentsOfDBPDATA(false);
	chdir(CurrentDirectory);
	#else
	// Find temporary directory (C:\WINDOWS\Temp)
	GetTempPath(_MAX_PATH, WindowsTempDirectory);
	//getcwd(whereaminow, _MAX_PATH);
	if(stricmp(WindowsTempDirectory, CurrentDirectory)!=NULL)
	{
		// XP Temp Folder
		_chdir(WindowsTempDirectory);
		//getcwd(whereaminow, _MAX_PATH);
		MakeOrEnterUniqueDBPDATA();
		//getcwd(whereaminow, _MAX_PATH);
		strcat(WindowsTempDirectory, "\\");
		strcat(WindowsTempDirectory, gpDBPDataName);
		strcpy(gUnpackDirectory, WindowsTempDirectory);
		_chdir(gpDBPDataName);
		//getcwd(whereaminow, _MAX_PATH);
		DeleteContentsOfDBPDATA(false);
		//getcwd(whereaminow, _MAX_PATH);
		_chdir(CurrentDirectory);
		/*
		_chdir("C:\\");
		getcwd(whereaminow, _MAX_PATH);
		_chdir("Program Files");
		getcwd(whereaminow, _MAX_PATH);
		_chdir("WindowsApps");
		getcwd(whereaminow, _MAX_PATH);
		//_chdir("GameGuru_0.0.0.1_x86__khk7gqqk5d15p");
		SetCurrentDirectoryA("New folder");
		DWORD errorMessageID = GetLastError();
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		MessageBox(NULL,messageBuffer,messageBuffer,MB_OK );
		LocalFree(messageBuffer);
		getcwd(whereaminow, _MAX_PATH);
		*/
	}
	else
	{
		// Pre-XP Temp Folder
		GetWindowsDirectory(WindowsTempDirectory, _MAX_PATH);
		_chdir(WindowsTempDirectory);
		mkdir("temp");
		_chdir("temp");
		MakeOrEnterUniqueDBPDATA();
		strcat(WindowsTempDirectory, "\\temp\\");
		strcat(WindowsTempDirectory, gpDBPDataName);
		strcpy(gUnpackDirectory, WindowsTempDirectory);
		_chdir(gpDBPDataName);
		DeleteContentsOfDBPDATA(false);
		_chdir(CurrentDirectory);
	}
	#endif
	
	//getcwd(whereaminow, _MAX_PATH);
	//MessageBox(NULL,whereaminow,whereaminow,MB_OK );

	// Send critical start info to CEXE
	CEXE.StartInfo(gUnpackDirectory, gEncryptionKey);

	// Place absolute EXE filename in CEXE structure
	if ( CEXE.m_pAbsoluteAppFile ) strcpy ( CEXE.m_pAbsoluteAppFile, ActualEXEFilename );

	// In case of error
	LPSTR pErrorString = NULL;

	// Instead of EXE Loda, have to enter values manually
	CEXE.m_pInitialAppName = new char[32];
	#ifdef GURULIGHTMAPPER
	CEXE.m_dwInitialDisplayMode = 1;
	CEXE.m_dwInitialDisplayWidth = GetSystemMetrics(SM_CXSCREEN);
	CEXE.m_dwInitialDisplayHeight = GetSystemMetrics(SM_CYSCREEN);
	CEXE.m_dwInitialDisplayDepth = 32;
	strcpy(CEXE.m_pInitialAppName, "Guru-Lightmapper");
	#else
	CEXE.m_dwInitialDisplayMode = 0;
	CEXE.m_dwInitialDisplayWidth = 799;
	CEXE.m_dwInitialDisplayHeight = 599;
	CEXE.m_dwInitialDisplayDepth = 32;
	strcpy(CEXE.m_pInitialAppName, "Guru-MapEditor");
	#endif

	// Manually create runtime error string array
	CEXE.m_pRuntimeErrorStringsArray = new LPSTR[9999];
	for ( int n=0; n<9999; n++ )
	{
		CEXE.m_pRuntimeErrorStringsArray[n] = new char[256];
		memset ( CEXE.m_pRuntimeErrorStringsArray[n], 0, sizeof(256) );
	}
	FillDarkErrors();

	// Global Shared Data
	ZeroMemory(&g_Glob, sizeof(GlobStruct));
	g_Glob.bWindowsMouseVisible		= true;
	g_Glob.dwForeColor				= -1; // (white)
	g_Glob.dwBackColor				= 0;
	g_Glob.bEscapeKeyEnabled		= true;

	// Run EXE Block
	RunProgram(hInstance, &pErrorString);

	// Free Display First
	CEXE.FreeUptoDisplay();

	// Report any errors
	if(pErrorString)
	{
		// Report Failure to Run
		ShowCursor(TRUE);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		char pFullError[_MAX_PATH];
		strcpy ( pFullError, pErrorString );
			
		// is there any additional error info after EXEPATH string?
		DWORD dwEXEPathLength = strlen ( g_pGlob->pEXEUnpackDirectory );
		LPSTR pSecretErrorMessage = g_pGlob->pEXEUnpackDirectory + dwEXEPathLength + 1;
		if ( strlen ( pSecretErrorMessage ) > 0 && strlen ( pSecretErrorMessage ) < _MAX_PATH )
		{
			// addition error info
			strcat ( pFullError, ".\n" );
			strcat ( pFullError, pSecretErrorMessage );
		}
		char err[512];
		wsprintf ( err, "GameGuru has detected an unexpected issue and needs to restart your session. If this problem persists, please contact support with error code (%s)", pErrorString );
		MessageBox(NULL, err, "GameGuru Problem Detected", MB_TOPMOST | MB_OK);
		SAFE_DELETE(pErrorString);
	}

	// use current date to remove
	_chdir(CurrentDirectory);

	// Delete temporary unpack directory
	rmdir(gUnpackDirectory);

	// Complete
	return 0;
}
