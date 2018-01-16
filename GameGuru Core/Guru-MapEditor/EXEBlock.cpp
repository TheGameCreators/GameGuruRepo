//#define INITGUID
#include ".\..\Dark Basic Public Shared\Dark basic Pro SDK\Shared\Error\CError.h"
#include "EXEBlock.h"
#include "direct.h"
#include "resource.h"
#include "globstruct.h"
#include ".\..\Dark Basic Public Shared\Dark basic Pro SDK\Shared\Core\DBDLLCore.h"
#include "GameGuruMain.h"

// Direct X stuff
LPGG								m_pDX = NULL;
LPGGDEVICE							m_pD3D = NULL;

// Specific DirectX 11 stuff
#ifdef DX11
LPGGIMMEDIATECONTEXT				m_pImmediateContext = NULL;
LPGGDEPTHSTENCILSTATE				m_pDepthStencilState = NULL;
LPGGDEPTHSTENCILSTATE				m_pDepthNoWriteStencilState = NULL;
LPGGDEPTHSTENCILSTATE				m_pDepthDisabledStencilState = NULL;
LPGGRASTERIZERSTATE					m_pRasterState = NULL;
LPGGRASTERIZERSTATE					m_pRasterStateNoCull = NULL;
LPGGRASTERIZERSTATE					m_pRasterStateDepthBias = NULL;
LPGGBLENDSTATE						m_pBlendStateAlpha = NULL;
LPGGBLENDSTATE						m_pBlendStateNoAlpha = NULL;
LPGGBLENDSTATE						m_pBlendStateShadowBlend = NULL;

#endif

DBPRO_GLOBAL HWND					GlobalHwndCopy = NULL;

// Global temp string
DBPRO_GLOBAL char					m_pWorkString[_MAX_PATH];

// Global checklist
DBPRO_GLOBAL bool					g_bCreateChecklistNow;
DBPRO_GLOBAL DWORD					g_dwMaxStringSizeInEnum;

// Global DLL Storage
HINSTANCE							hDLLMod[256];
bool								bDLLTPC[256];

// Global Variables
bool								g_bSuccessfulDLLLinks			= false;
DWORD								g_dwEscapeValueMem				= 0;
DWORD								g_dwBreakOutPosition			= 0;
LPSTR								g_pVarSpaceAddressInUse			= NULL;
DWORD								g_dwVarSpaceSizeInUse			= 0;
bool								g_bIsInternalDebugger			= false;
PROCESS_INFORMATION					g_InternalDebuggerProcessInfo;
HANDLE								g_hLastGFXPointer				= NULL;

// If linked from DarkEXE (temp loading... window)
extern HWND g_hTempWindow;
extern HWND g_igLoader_HWND;

#ifdef DEMOPROTECTEDMODE
bool WriteStringToRegistry(char* PerfmonNamesKey, char* valuekey, char* string)
{
	HKEY hKeyNames = 0;
	DWORD Status;
	DWORD dwDisposition;
	char ObjectType[256];
	strcpy(ObjectType,"Num");
	Status = RegCreateKeyEx(HKEY_CURRENT_USER, PerfmonNamesKey, 0L, ObjectType, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WRITE, NULL, &hKeyNames, &dwDisposition);
	if(dwDisposition==REG_OPENED_EXISTING_KEY)
	{
		RegCloseKey(hKeyNames);
		Status = RegOpenKeyEx(HKEY_CURRENT_USER, PerfmonNamesKey, 0L, KEY_WRITE, &hKeyNames);
	}
    if(Status==ERROR_SUCCESS)
	{
        Status = RegSetValueEx(hKeyNames, valuekey, 0, REG_SZ, (LPBYTE)string, (strlen(string)+1)*sizeof(char));
	}
	RegCloseKey(hKeyNames);
	hKeyNames=0;
	return true;
}
void ReadStringFromRegistry(char* PerfmonNamesKey, char* valuekey, char* string)
{
	HKEY hKeyNames = 0;
	DWORD Status;
	char ObjectType[256];
	DWORD Datavalue = 0;

	strcpy(string,"");
	strcpy(ObjectType,"Num");
	Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PerfmonNamesKey, 0L, KEY_READ, &hKeyNames);
    if(Status==ERROR_SUCCESS)
	{
		DWORD Type=REG_SZ;
		DWORD Size=256;
		Status = RegQueryValueEx(hKeyNames, valuekey, NULL, &Type, NULL, &Size);
		if(Size<255)
			RegQueryValueEx(hKeyNames, valuekey, NULL, &Type, (LPBYTE)string, &Size);

		RegCloseKey(hKeyNames);
	}
}
#endif

CEXEBlock::CEXEBlock()
{
	// Reset flags
	m_bEXEBlockPresent=false;

	// Settings
	m_dwInitialDisplayMode=1;
	m_dwInitialDisplayWidth=0;
	m_dwInitialDisplayHeight=0;
	m_dwInitialDisplayDepth=0;
	m_pInitialAppName=NULL;

	m_pOriginalFolderName=new char[_MAX_PATH+1];
	strcpy(m_pOriginalFolderName,"");
	m_pUnpackFolderName=new char[_MAX_PATH+1];
	strcpy(m_pUnpackFolderName,"");
	m_pAbsoluteAppFile=new char[_MAX_PATH+1];
	strcpy(m_pAbsoluteAppFile,"");

	// DLL Data
	m_dwNumberOfDLLs=0;
	m_pDLLIndexArray=NULL;
	m_pDLLFilenameArray=NULL;
	m_pDLLLoadedAlreadyArray=NULL;

	// MCB Reference Data
	m_dwNumberOfReferences=0;
	m_pRefArray=NULL;
	m_pRefTypeArray=NULL;
	m_pRefIndexArray=NULL;

	// Clear runtime error dword
	m_dwRuntimeErrorDWORD=0;
	m_dwRuntimeErrorLineDWORD=0;

	// Runtime string array database
	m_dwNumberOfRuntimeErrorStrings=0;
	m_pRuntimeErrorStringsArray=NULL;

	// Machine Code Block (MCB)
	m_dwSizeOfMCB=0;
	m_pMachineCodeBlock=NULL;
	m_dwStartOfMiniMC=0;

	// Commands Data
	m_dwNumberOfCommands=0;
	m_pCommandDLLIdArray=NULL;
	m_pCommandDLLCallArray=NULL;

	// Strings Data
	m_dwNumberOfStrings=0;
	m_pStringsArray=NULL;

	// Data Statements Data
	m_dwNumberOfDataItems=0;
	m_pDataArray=NULL;
	m_pDataStringsArray=NULL;

	// Variable Space Data
	m_dwVariableSpaceSize=0;
	m_pVariableSpace=NULL;

	// Data Space Data
	m_dwDataSpaceSize=0;
	m_pDataSpace=NULL;

	// Record Dynamic Variables for auto-freeing
	m_dwDynamicVarsQuantity=0;
	m_pDynamicVarsArray=NULL;
	m_pDynamicVarsArrayType=NULL;

	// Record UserTypeStringPatterns - reactivated for U71 (store structure types)
	m_dwUsertypeStringPatternQuantity=0;
	m_pUsertypeStringPatternArray=NULL;
}

CEXEBlock::~CEXEBlock()
{
	Clear();
}

void CEXEBlock::Clear(void)
{
	// Release appname
	SAFE_DELETE(m_pInitialAppName);

	// Release exefile ptrs
	SAFE_DELETE(m_pOriginalFolderName);
	SAFE_DELETE(m_pUnpackFolderName);
	SAFE_DELETE(m_pAbsoluteAppFile);

	// Release DLLs Data
	if ( m_pDLLFilenameArray ) DeleteArrayContents(m_pDLLFilenameArray,m_dwNumberOfDLLs);
	SAFE_DELETE(m_pDLLFilenameArray);
	SAFE_DELETE(m_pDLLIndexArray);
	SAFE_DELETE(m_pDLLLoadedAlreadyArray);

	// Release Ref Data
	SAFE_DELETE(m_pRefArray);
	SAFE_DELETE(m_pRefIndexArray);
	SAFE_DELETE(m_pRefTypeArray);

	// Release MCB Data 9leeadd - 090305 - DEP release)
	VirtualFree ( m_pMachineCodeBlock, 0, MEM_DECOMMIT | MEM_RELEASE );
	m_pMachineCodeBlock = NULL;

	// Release Runtime Error Strings Database
	if ( m_pRuntimeErrorStringsArray ) DeleteArrayContents((DWORD*)m_pRuntimeErrorStringsArray,m_dwNumberOfRuntimeErrorStrings);
	SAFE_DELETE(m_pRuntimeErrorStringsArray);

	// Release Commands Data
	if ( m_pCommandDLLCallArray ) DeleteArrayContents(m_pCommandDLLCallArray,m_dwNumberOfCommands);
	SAFE_DELETE(m_pCommandDLLCallArray);
	SAFE_DELETE(m_pCommandDLLIdArray);

	// Release Strings Data
	if ( m_pStringsArray ) DeleteArrayContents(m_pStringsArray,m_dwNumberOfStrings);
	SAFE_DELETE(m_pStringsArray);

	// Release Data Data
	SAFE_DELETE(m_pDataArray);
	if ( m_pDataStringsArray) DeleteArrayContents(m_pDataStringsArray,m_dwNumberOfDataItems);
	SAFE_DELETE(m_pDataStringsArray);

	// Release Dynamic Variable Offset Array
	SAFE_DELETE(m_pDynamicVarsArray);
	SAFE_DELETE(m_pDynamicVarsArrayType);

	// Release Structure Pattern Array
	SAFE_DELETE(m_pUsertypeStringPatternArray);

	// Reset all values
	m_bEXEBlockPresent=false;
	m_dwNumberOfDLLs=0;
	m_dwNumberOfReferences=0;
	m_dwSizeOfMCB=0;
	m_dwNumberOfCommands=0;
	m_dwNumberOfStrings=0;
	m_dwVariableSpaceSize=0;
	m_dwDataSpaceSize=0;
}

DWORD* CEXEBlock::CreateArray(DWORD dwCount,DWORD dwType)
{
	// leeadd - 090305 - DEP or regular flavour
	DWORD* pArray = NULL;
	if ( dwType==PAGE_EXECUTE_READWRITE )
	{
		// data block used to execute code
		pArray = (DWORD*) VirtualAlloc ( NULL, dwCount*sizeof(DWORD), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
	}
	else
	{
		// regular data block
		pArray = new DWORD[dwCount];
		for(DWORD i=0; i<dwCount; i++)
			*(pArray+i)=NULL;
	}

	// return ptr
	return pArray;
}

DWORD* CEXEBlock::CreateArray(DWORD dwCount)
{
	return CreateArray(dwCount,0);
}

bool CEXEBlock::RecreateArray(DWORD** pArray, DWORD dwCount, DWORD NewCount)
{
	if(pArray)
	{
		DWORD* pTempArray = new DWORD[NewCount];
		if(pTempArray)
		{
			// Clear New
			for(DWORD i=0; i<NewCount; i++)
				*(pTempArray+i)=NULL;

			// Copy Old to New
			memcpy(pTempArray, *pArray, dwCount*sizeof(DWORD));

			// Delete Old (pointer array only)
			delete *pArray;

			// Switch Pointers
			*pArray=pTempArray;

			return true;
		}
		else
		{
			// EXEBlock shared - silent fail
			return false;
		}
	}

	// Soft fail
	return false;
}

void CEXEBlock::DeleteArrayContents(DWORD* pArray, DWORD dwCount)
{
	if(pArray)
	{
		for(DWORD i=0; i<dwCount; i++)
		{
			if(*(pArray+i))
			{
				delete (DWORD*)*(pArray+i);
				*(pArray+i)=NULL;
			}
		}
	}
}

bool CEXEBlock::FileExists(LPSTR pFilename)
{
	HANDLE hFile = CreateFile(pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		// Close File
		SAFE_CLOSE(hFile);
		return true;
	}
	// soft fail
	return false;
}

bool CEXEBlock::Save(char* lpFilename)
{
	HANDLE hFile = CreateFile(lpFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		// Settings
		SaveValue(hFile, &m_dwInitialDisplayMode);
		SaveValue(hFile, &m_dwInitialDisplayWidth);
		SaveValue(hFile, &m_dwInitialDisplayHeight);
		SaveValue(hFile, &m_dwInitialDisplayDepth);

		// Save AppName String
		DWORD dwLength=256;
		SaveValueArrayBytes(hFile, (DWORD**)&m_pInitialAppName, &dwLength);

		// DLL Data
		SaveValue(hFile, &m_dwNumberOfDLLs);
		SaveValueArray(hFile, &m_pDLLIndexArray, &m_dwNumberOfDLLs);
		SaveStringArray(hFile, &m_pDLLFilenameArray, &m_dwNumberOfDLLs);
		SaveValueArray(hFile, &m_pDLLLoadedAlreadyArray, &m_dwNumberOfDLLs);

		// MCB Reference Data
		SaveValue(hFile, &m_dwNumberOfReferences);
		SaveValueArray(hFile, &m_pRefArray, &m_dwNumberOfReferences);
		SaveValueArray(hFile, &m_pRefTypeArray, &m_dwNumberOfReferences);
		SaveValueArray(hFile, &m_pRefIndexArray, &m_dwNumberOfReferences);

		// Runtime Error String Database
		SaveValue(hFile, &m_dwNumberOfRuntimeErrorStrings);
		SaveStringArray(hFile, (DWORD**)&m_pRuntimeErrorStringsArray, &m_dwNumberOfRuntimeErrorStrings);

		// Machine Code Block (MCB)
		SaveValue(hFile, &m_dwSizeOfMCB);
		SaveValueArrayBytes(hFile, &m_pMachineCodeBlock, &m_dwSizeOfMCB);

		// Commands Data
		SaveValue(hFile, &m_dwNumberOfCommands);
		SaveValueArray(hFile, &m_pCommandDLLIdArray, &m_dwNumberOfCommands);
		SaveStringArray(hFile, &m_pCommandDLLCallArray, &m_dwNumberOfCommands);

		// Strings Data
		SaveValue(hFile, &m_dwNumberOfStrings);
		SaveStringArray(hFile, &m_pStringsArray, &m_dwNumberOfStrings);

		// Data Data
		SaveValue(hFile, &m_dwNumberOfDataItems);
		SaveBlock(hFile, &m_pDataArray, m_dwNumberOfDataItems*10);
		SaveStringArray(hFile, &m_pDataStringsArray, &m_dwNumberOfDataItems);

		// Variable Space Data
		SaveValue(hFile, &m_dwVariableSpaceSize);

		// Data Space Data
		SaveValue(hFile, &m_dwDataSpaceSize);

		// Dynamic Variable Offset Data
		SaveValue(hFile, &m_dwDynamicVarsQuantity);
		SaveValueArray(hFile, &m_pDynamicVarsArray, &m_dwDynamicVarsQuantity);
		SaveValueArray(hFile, &m_pDynamicVarsArrayType, &m_dwDynamicVarsQuantity);

		// Usertype String Patterns - reactivated for U71 (store structure types)
		SaveValue(hFile, &m_dwUsertypeStringPatternQuantity);
		SaveBlock(hFile, &m_pUsertypeStringPatternArray, m_dwUsertypeStringPatternQuantity);

		// Close file
		CloseHandle(hFile);
		return true;
	}
	else
	{
		// EXEBlock shared - silent fail
		return false;
	}
}

bool CEXEBlock::SaveValue(HANDLE hFile, DWORD* Value)
{
	DWORD bytes=0;
	WriteFile(hFile, Value, 4, &bytes, NULL);
	if(bytes==0)
	{
		// EXEBlock shared - silent fail
		return false;
	}
	else
		return true;
}

bool CEXEBlock::SaveBlock(HANDLE hFile, LPSTR* pMem, DWORD dwSize)
{
	DWORD bytes=0;
	WriteFile(hFile, *pMem, dwSize, &bytes, NULL);
	if(bytes==0)
	{
		// EXEBlock shared - silent fail
		return false;
	}
	else
		return true;
}

bool CEXEBlock::SaveValueArray(HANDLE hFile, DWORD** pArray, DWORD* Count)
{
	DWORD bytes=0;
	bool bResult=true;
	if(*pArray && *Count>0)
	{
		WriteFile(hFile, *pArray, (*Count)*sizeof(DWORD), &bytes, NULL);
		if(bytes==0) bResult=false;
	}
	else
		bResult=false;

	return bResult;
}

bool CEXEBlock::SaveValueArrayBytes(HANDLE hFile, DWORD** pArray, DWORD* Count)
{
	DWORD bytes=0;
	bool bResult=true;
	if(*pArray && *Count>0)
	{
		WriteFile(hFile, *pArray, (*Count), &bytes, NULL);
		if(bytes==0) bResult=false;
	}
	else
		bResult=false;

	return bResult;
}

bool CEXEBlock::SaveStringArray(HANDLE hFile, DWORD** pArray, DWORD* Count)
{
	DWORD bytes=0;
	bool bResult=true;
	if(*pArray && *Count>0)
	{
		for(DWORD index=0; index<*Count; index++)
		{
			char* pStr = (char*)*(*pArray+index);
			DWORD length = 0;
			if(pStr) length = strlen(pStr);
			WriteFile(hFile, &length, 4, &bytes, NULL);
			if(bytes==0) bResult=false;
			if(pStr)
			{
				// Write number of bytes in string
				if(length>0)
				{
					// Write string if has a length
					WriteFile(hFile, pStr, length, &bytes, NULL);
					if(bytes==0) bResult=false;
				}
			}
		}
	}
	else
		bResult=false;

	return bResult;
}

bool CEXEBlock::StartInfo(LPSTR pUnpackFolderName, DWORD dwEncryptionKey)
{
	// Set Unpack Folder (and copy to global data)
	strcpy(m_pUnpackFolderName, pUnpackFolderName);
	m_dwEncryptionKey = dwEncryptionKey;

	// Complete
	return true;
}

bool CEXEBlock::Load(char* lpFilename)
{
	// Load EXE Filedata
	HANDLE hFile = CreateFile(lpFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		// Settings
		LoadValue(hFile, &m_dwInitialDisplayMode);
		LoadValue(hFile, &m_dwInitialDisplayWidth);
		LoadValue(hFile, &m_dwInitialDisplayHeight);
		LoadValue(hFile, &m_dwInitialDisplayDepth);

		// AppName String
		DWORD dwLength=256;
		LoadValueArrayBytes(hFile, (DWORD**)&m_pInitialAppName, &dwLength);

		// DLL Data
		LoadValue(hFile, &m_dwNumberOfDLLs);
		LoadValueArray(hFile, &m_pDLLIndexArray, &m_dwNumberOfDLLs);
		LoadStringArray(hFile, &m_pDLLFilenameArray, &m_dwNumberOfDLLs);
		LoadValueArray(hFile, &m_pDLLLoadedAlreadyArray, &m_dwNumberOfDLLs);
		ZeroMemory(m_pDLLLoadedAlreadyArray, sizeof(DWORD)*m_dwNumberOfDLLs);
		
		// MCB Reference Data
		LoadValue(hFile, &m_dwNumberOfReferences);
		LoadValueArray(hFile, &m_pRefArray, &m_dwNumberOfReferences);
		LoadValueArray(hFile, &m_pRefTypeArray, &m_dwNumberOfReferences);
		LoadValueArray(hFile, &m_pRefIndexArray, &m_dwNumberOfReferences);

		// Runtime Error String Database
		LoadValue(hFile, &m_dwNumberOfRuntimeErrorStrings);
		LoadStringArray(hFile, (DWORD**)&m_pRuntimeErrorStringsArray, &m_dwNumberOfRuntimeErrorStrings);

		// Machine Code Block (MCB)
		// leeadd - 090305 - added flag for DEP protection allowance
		LoadValue(hFile, &m_dwSizeOfMCB);
		LoadValueArrayBytes(hFile, &m_pMachineCodeBlock, &m_dwSizeOfMCB, PAGE_EXECUTE_READWRITE);

		// Commands Data
		LoadValue(hFile, &m_dwNumberOfCommands);
		LoadValueArray(hFile, &m_pCommandDLLIdArray, &m_dwNumberOfCommands);
		LoadStringArray(hFile, &m_pCommandDLLCallArray, &m_dwNumberOfCommands);

		// Strings Data
		LoadValue(hFile, &m_dwNumberOfStrings);
		LoadStringArray(hFile, &m_pStringsArray, &m_dwNumberOfStrings);

		// Data Data
		LoadValue(hFile, &m_dwNumberOfDataItems);
		LoadBlock(hFile, &m_pDataArray, m_dwNumberOfDataItems*10);
		LoadStringArray(hFile, &m_pDataStringsArray, &m_dwNumberOfDataItems);

		// Variable Space Data
		LoadValue(hFile, &m_dwVariableSpaceSize);

		// Data Space Data
		LoadValue(hFile, &m_dwDataSpaceSize);

		// Dynamic Variable Offset Data
		LoadValue(hFile, &m_dwDynamicVarsQuantity);
		LoadValueArray(hFile, &m_pDynamicVarsArray, &m_dwDynamicVarsQuantity);
		LoadValueArray(hFile, &m_pDynamicVarsArrayType, &m_dwDynamicVarsQuantity);

		// Usertype String Patterns - reactivated for U71 (store structure types)
		LoadValue(hFile, &m_dwUsertypeStringPatternQuantity);
		LoadBlock(hFile, &m_pUsertypeStringPatternArray, m_dwUsertypeStringPatternQuantity);

		// Close file
		CloseHandle(hFile);
		return true;
	}
	else
	{
		// EXEBlock shared - silent fail
		return false;
	}
}

bool CEXEBlock::LoadValue(HANDLE hFile, DWORD* Value)
{
	DWORD bytes=0;
	ReadFile(hFile, Value, 4, &bytes, NULL);
	if(bytes==0)
	{
		// EXEBlock shared - silent fail
		return false;
	}
	else
		return true;
}

bool CEXEBlock::LoadBlock(HANDLE hFile, LPSTR* pMem, DWORD dwSize)
{
	DWORD bytes=0;
	*pMem = new char[dwSize+1];
	ReadFile(hFile, *pMem, dwSize, &bytes, NULL);
	if(bytes==0)
	{
		// EXEBlock shared - silent fail
		return false;
	}
	else
		return true;
}

bool CEXEBlock::LoadValueArray(HANDLE hFile, DWORD** pArray, DWORD* Count)
{
	DWORD bytes=0;
	bool bResult=true;
	if(*Count>0)
	{
		// Create Array 
		*pArray = CreateArray(*Count,0);

		// Read data into Array
		ReadFile(hFile, *pArray, (*Count)*sizeof(DWORD), &bytes, NULL);
		if(bytes==0) bResult=false;
	}
	else
		bResult=false;

	return bResult;
}

bool CEXEBlock::LoadValueArrayBytes(HANDLE hFile, DWORD** pArray, DWORD* Count, DWORD dwType)
{
	DWORD bytes=0;
	bool bResult=true;
	if(*Count>0)
	{
		// Create Array 
		*pArray = CreateArray(*Count,dwType);

		// Read data into Array
		ReadFile(hFile, *pArray, (*Count), &bytes, NULL);
		if(bytes==0) bResult=false;
	}
	else
		bResult=false;

	return bResult;
}

bool CEXEBlock::LoadValueArrayBytes(HANDLE hFile, DWORD** pArray, DWORD* Count)
{
	return LoadValueArrayBytes(hFile, pArray, Count, 0);
}


bool CEXEBlock::LoadStringArray(HANDLE hFile, DWORD** pArray, DWORD* Count)
{
	DWORD bytes=0;
	bool bResult=true;
	if(*Count>0)
	{
		// Create Array 
		*pArray = CreateArray(*Count,0);

		// Read strings into Array of strings
		for(DWORD index=0; index<*Count; index++)
		{
			// Read length of string
			DWORD length = 0;
			ReadFile(hFile, &length, 4, &bytes, NULL);
			char* pStr = new char[length+1];
			if(length>0)
			{
				ReadFile(hFile, pStr, length, &bytes, NULL);
				if(bytes==0) bResult=false;
			}
			pStr[length]=0;
			*(*pArray+index) = (DWORD)pStr;
		}
	}
	else
		bResult=false;

	return bResult;
}

bool CEXEBlock::Init ( HINSTANCE hInstance, bool bResult, LPSTR pCmdLine )
{
	// Direct X Check - not used any more

	// [EXE] Switch to TEMP Folder (that holds all exe-linked files)
	getcwd(m_pOriginalFolderName, _MAX_PATH);
	_chdir(m_pUnpackFolderName);

	// [EXE] Copy vital data for all DLLs to access to Glob Structure
	if(g_pGlob)
	{
		memset ( g_pGlob->pEXEUnpackDirectory, 0, _MAX_PATH );
		strcpy(g_pGlob->pEXEUnpackDirectory, m_pUnpackFolderName);
		g_pGlob->ppEXEAbsFilename = (DWORD)m_pAbsoluteAppFile;
		g_pGlob->dwEncryptionUniqueKey = m_dwEncryptionKey;
	}

	// [EXE] Load Icons into Glob for use by Core
	if(g_pGlob)
	{
		// Seems this command not recognised now!?! LoadIcon(g_pGlob->hInstance, MAKEINTRESOURCE(IDI_ICON1));
		g_pGlob->hAppIcon = NULL;
	}

	// [EXE] Set global states and initialise display
	if ( bResult==true )
	{
		// Prepare Display (GDI or EXT)
		PassCmdLineHandlerPtr((LPVOID)pCmdLine);
		PassErrorHandlerPtr((LPVOID)&m_dwRuntimeErrorDWORD);
		SetRenderOrderList();

		// Dynamic memory alloc
		if ( g_pGlob->pDynMemPtr==NULL )
		{
			g_pGlob->dwDynMemSize = (sizeof(HINSTANCE)*256)+(sizeof(bool)*256);
			g_pGlob->pDynMemPtr = new char [ g_pGlob->dwDynMemSize ];
			memcpy ( g_pGlob->pDynMemPtr+0, hDLLMod, (sizeof(HINSTANCE)*256) );
			memcpy ( g_pGlob->pDynMemPtr+(sizeof(HINSTANCE)*256), bDLLTPC, (sizeof(bool)*256) );
		}
		
		// Initialise Display (and DX check)
		if (InitDisplay(m_dwInitialDisplayMode, m_dwInitialDisplayWidth, m_dwInitialDisplayHeight, m_dwInitialDisplayDepth, hInstance, m_pInitialAppName) == 1)
		{
			// Failed to DXSetup - Exit now
			bResult = false;
		}
	}

	// [EXE] Switch out of TEMP Folder
	_chdir(m_pOriginalFolderName);

	// Success result
	return bResult;
}

bool CEXEBlock::Run(bool bResult)
{
	// [EXE] Run Machine Code Program
	if (bResult != true)
		return bResult;

	// GG Engine (uses ExitProcess)
	GuruMain();

	// Never gets here
	return bResult;
}

bool CEXEBlock::RunFrom(bool bResult, DWORD dwOffset)
{
	return bResult;
}

void CEXEBlock::FreeUptoDisplay(void)
{
	// [EXE] Delete All Allocations Within Data Space
	if(m_pDataSpace)
	{
		// Data Item Format [Type][Reserved Byte][8byte for data]=10 bytes each
		for(DWORD dv=0; dv<m_dwDataSpaceSize; dv+=10)
		{
			if(m_pDataSpace[dv]==3)
			{
				DWORD* pMemoryAllocation = (DWORD*)((LPSTR)m_pDataSpace[dv]+2);
			}
		}
	}

	// [EXE] Delete All Allocations Within Var Space
	if(m_pVariableSpace)
	{
		// Scan variables for all dynamic allocations (use dynamicvaroffsetarray)
		for(DWORD dv=0; dv<m_dwDynamicVarsQuantity; dv++)
		{
			DWORD* pMemoryAllocation = (DWORD*)*((DWORD*)(m_pVariableSpace+m_pDynamicVarsArray[dv]));
		}
	}

	// [EXE] Close Display
	CloseDisplay();
}

void CEXEBlock::Free(void)
{
	// [CORE] Close any memory created for glob
	if ( g_pGlob ) if ( g_pGlob->pDynMemPtr ) SAFE_DELETE ( g_pGlob->pDynMemPtr );

	// [EXE] Free Up Data Allocations (from Load)
	Clear();

	// [EXE] Freeing Up
	if(!g_bSuccessfulDLLLinks)
		return;
}
