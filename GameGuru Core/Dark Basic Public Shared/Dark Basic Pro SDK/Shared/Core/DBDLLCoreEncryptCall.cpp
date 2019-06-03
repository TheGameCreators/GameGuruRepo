#include "windows.h"
#include "globstruct.h"
#include "DBDLLCore.h"
#include "..\..\..\..\Guru-MapEditor\Encryptor.h"
#include ".\..\Core\SteamCheckForWorkshop.h"

void CallEncryptDecrypt( DWORD dwStringAddress, bool bEncryptIfTrue, bool bDoNotUseTempFolder )
{
	// Dave - for safety
	g_pGlob->dwEncryptionUniqueKey = 0;

	// String is input with external filename
	if(dwStringAddress==0)
		return;

	// If re-encrypting, and filename is in temp, just delete it and finish
	LPSTR pLocalTRemp = g_pGlob->pEXEUnpackDirectory;
	if ( bDoNotUseTempFolder==false && bEncryptIfTrue==true )
	{
		if ( strnicmp ( (LPSTR)dwStringAddress, pLocalTRemp, strlen(pLocalTRemp) )==NULL )
		{
			// Delete temp decrypted file
			if(COREDoesFileExist((LPSTR)dwStringAddress)) DeleteFile((LPSTR)dwStringAddress);

			// Finish early
			return;
		}
	}

	// length of six... e.g. "_e_Z.x" is the min size file (1 letter name)
	if ( strlen((LPSTR)dwStringAddress) < 6 ) return;

	char checkForEncryptName[_MAX_PATH];
	char keyName[10] = "7yFkC30a";
		
	strcpy( checkForEncryptName , (LPSTR)dwStringAddress );

	// a file that needs decrypting will start with "_e_"
	bool okayToProceed = false;
	if ( ( checkForEncryptName[0] == '_' && checkForEncryptName[1] == 'e' && checkForEncryptName[2] == '_' ) || strstr ( checkForEncryptName , "\\_e_" )  )
	{
		if ( CanIUse_E_() || bEncryptIfTrue )
			okayToProceed = true;
		else
		{
			char message[1024];
			sprintf ( message , "This file is encrypted and cannot be loaded into GameGuru editor:\n%s", (LPSTR)dwStringAddress );
			MessageBox( NULL, message, "GAME GURU Error", MB_ICONEXCLAMATION );
		}
	}
	else
	{
		if ( Steam_CanIUse_W_() || bEncryptIfTrue )
		{
			if ( ( checkForEncryptName[0] == '_' && checkForEncryptName[1] == 'w' && checkForEncryptName[2] == '_' ) || strstr ( checkForEncryptName , "\\_w_" )  )
			{
				okayToProceed = true;
				strcpy ( keyName , "iG72VL8q" );
			}
		}
	}

	if ( okayToProceed )
	{
		// Dave - set key to 1 to show this file is encrypted
		g_pGlob->dwEncryptionUniqueKey = 1;

		char stringToMakeKey[_MAX_PATH];
		strcpy ( stringToMakeKey , "" );
		int tLength = 0;
		for ( DWORD c = 0 ; c < strlen(checkForEncryptName) ; c++ )
		{
			if ( checkForEncryptName[c] != '.' )
				stringToMakeKey[tLength++] = checkForEncryptName[c];
				stringToMakeKey[tLength] = 0;
		}

		int len = strlen(stringToMakeKey)-1;
		if ( len <= 0 ) return;

		for ( int c = len; c >= 0 && c > len-9 ; c-- )
		{
			keyName[len - c] = stringToMakeKey[c];
			if ( keyName[len - c] >= 65 && keyName[len - c] < 90 ) keyName[len - c]++;
			else if ( keyName[len - c] >= 97 && keyName[len - c] < 122 ) keyName[len - c]++;
			else if ( keyName[len - c] == 90 ) keyName[len - c] = 65;
			else if ( keyName[len - c] == 122 ) keyName[len - c] = 97;
		}
	}
	else
		return;

	// If exe not setup for encryption, dont use it
	if(g_pGlob->dwEncryptionUniqueKey==0)
		return;

	// Check if file exists
	LPSTR pFilename = new char[_MAX_PATH];
	strcpy(pFilename, (LPSTR)dwStringAddress);
	if(COREDoesFileExist(pFilename))
	{
		// Open File Data
		DWORD dwDataSize = 0;
		LPSTR pData = ReadFileData(pFilename, &dwDataSize);

		// Decrypt File Data
		CEncryptor Encryptor(0);
		Encryptor.SetUniqueKey(1);
		Encryptor.SetUniqueKeyName(keyName);
		Encryptor.EncryptFileData(pData, dwDataSize, bEncryptIfTrue);

		// temp or real
		if ( bDoNotUseTempFolder==false )
		{
			// get file ext
			char pExt[32];
			strcpy ( pExt, "" );
			for ( int n=strlen(pFilename)-1; n>0; n-- )
			{
				if ( pFilename[n]=='.' )
				{
					strcpy ( pExt, pFilename+n );
					break;
				}
			}

			// work out write file to local write-safe area
			strcpy ( pFilename, pLocalTRemp );
			strcat ( pFilename, "\\decrypted" );
			strcat ( pFilename, pExt );

			// if decrypting file, write decrypted file to safe-write area
			// if encrypting, simply delete the temp writted file in safe-write area
			if ( bEncryptIfTrue==false )
			{
				// Write New File
				WriteFileData(pFilename, pData, dwDataSize);

				// And point module using this function to the new filename
				strcpy ( (LPSTR)dwStringAddress, pFilename );
			}
		}
		else
		{
			// Write New File and overrite what is there
			WriteFileData(pFilename, pData, dwDataSize);
		}

		// Free FileData
		if(pData)
		{
			GlobalFree(pData);
			pData=NULL;
		}
	}

	// Free usages
	delete[] pFilename;

	//Dave - set key back to 0 after
	g_pGlob->dwEncryptionUniqueKey = 0;
}
