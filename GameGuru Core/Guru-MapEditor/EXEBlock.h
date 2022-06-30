// EXEBlock.h: interface for the CEXEBlock class.

// Includes used for DX check
#include <windows.h>
#include <stdio.h>
#include "directx-macros.h"

// Safe Macros
#include "macros.h"

// EXEBlock Class Defenition
class CEXEBlock  
{
	public:
		CEXEBlock();
		virtual ~CEXEBlock();

		void			Clear(void);
		bool			StartInfo(LPSTR pUnpackFolderName, DWORD dwEncryptionKey);
		bool			Load(char* filename);
		bool			LoadValue(HANDLE hFile, DWORD* Value);
		bool			LoadBlock(HANDLE hFile, LPSTR* pMem, DWORD dwSize);
		bool			LoadValueArray(HANDLE hFile, DWORD** pArray, DWORD* Count);
		bool			LoadValueArrayBytes(HANDLE hFile, DWORD** pArray, DWORD* Count, DWORD dwType);
		bool			LoadValueArrayBytes(HANDLE hFile, DWORD** pArray, DWORD* Count);
		bool			LoadStringArray(HANDLE hFile, DWORD** pArray, DWORD* Count);
		bool			Save(char* filename);
		bool			SaveValue(HANDLE hFile, DWORD* Value);
		bool			SaveBlock(HANDLE hFile, LPSTR* pMem, DWORD dwSize);
		bool			SaveValueArray(HANDLE hFile, DWORD** pArray, DWORD* Count);
		bool			SaveValueArrayBytes(HANDLE hFile, DWORD** pArray, DWORD* Count);
		bool			SaveStringArray(HANDLE hFile, DWORD** pArray, DWORD* Count);
		DWORD*			CreateArray(DWORD dwCount,DWORD dwType);
		DWORD*			CreateArray(DWORD dwCount);
		bool			RecreateArray(DWORD** pArray, DWORD dwCount, DWORD NewCount);
		void			DeleteArrayContents(DWORD* pArray, DWORD dwCount);
		bool			FileExists(LPSTR pFilename);

		bool			Init(HINSTANCE hInstance, bool bResult, LPSTR pCmdLine);
		bool			Run(bool bResult);
		bool			RunFrom(bool bResult, DWORD dwOffset);
		void			Free(void);
		void			FreeUptoDisplay(void);

	public:

		// General Flags
		bool			m_bEXEBlockPresent;

		// Executable Settings
		DWORD			m_dwInitialDisplayMode;
		DWORD			m_dwInitialDisplayWidth;
		DWORD			m_dwInitialDisplayHeight;
		DWORD			m_dwInitialDisplayDepth;
		LPSTR			m_pInitialAppName;
		LPSTR			m_pOriginalFolderName;
		LPSTR			m_pUnpackFolderName;
		LPSTR			m_pAbsoluteAppFile;
		DWORD			m_dwEncryptionKey;

		// DLL Data
		DWORD			m_dwNumberOfDLLs;
		DWORD*			m_pDLLIndexArray;
		DWORD*			m_pDLLFilenameArray;
		DWORD*			m_pDLLLoadedAlreadyArray;

		// MCB Reference Data
		DWORD			m_dwNumberOfReferences;
		DWORD*			m_pRefArray;
		DWORD*			m_pRefTypeArray;
		DWORD*			m_pRefIndexArray;

		// Runtime Error and Escape Value DWORDs
		DWORD			m_dwRuntimeErrorDWORD;
		DWORD			m_dwRuntimeErrorLineDWORD;
		DWORD			m_dwEscapeValueDWORD;

		// Runtime Error String Database
		DWORD			m_dwNumberOfRuntimeErrorStrings;
		LPSTR*			m_pRuntimeErrorStringsArray;

		// Machine Code Block (MCB)
		DWORD			m_dwSizeOfMCB;
		DWORD*			m_pMachineCodeBlock;
		DWORD			m_dwStartOfMiniMC;

		// Commands Data
		DWORD			m_dwNumberOfCommands;
		DWORD*			m_pCommandDLLIdArray;
		DWORD*			m_pCommandDLLCallArray;

		// Strings Data
		DWORD			m_dwNumberOfStrings;
		DWORD*			m_pStringsArray;

		// Data Statements Data
		DWORD			m_dwNumberOfDataItems;
		LPSTR			m_pDataArray;
		DWORD*			m_pDataStringsArray;

		// Variable Space Data
		DWORD			m_dwOldVariableSpaceSize;
		DWORD			m_dwVariableSpaceSize;
		LPSTR			m_pVariableSpace;

		// Datat Statement Space Data
		DWORD			m_dwOldDataSpaceSize;
		DWORD			m_dwDataSpaceSize;
		LPSTR			m_pDataSpace;

		// Dynamic Variable Offset Array (points into VarSpace)
		DWORD			m_dwDynamicVarsQuantity;
		DWORD*			m_pDynamicVarsArray;
		DWORD*			m_pDynamicVarsArrayType;

		// New U71 - stores structure patterns
		DWORD			m_dwUsertypeStringPatternQuantity;
		LPSTR			m_pUsertypeStringPatternArray;
};
