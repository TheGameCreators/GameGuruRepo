#define _CRT_SECURE_NO_WARNINGS

#define PHOTONMP

#include "SteamCheckForWorkshop.h"
#include <stdio.h>

// New include
#include "SteamCommands.h"

bool CheckForWorkshopFile ( LPSTR VirtualFilename)
{
	// actually checks for existence of _e_ file in place of original!

	// some quick rejections
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
	for ( int c = 0 ; c < (int)strlen(szEncryptedFilenameFolder); c++ )
	{
		if ( szEncryptedFilenameFolder[c] == '/' ) 
			szEncryptedFilenameFolder[c] = '\\';
	}

	// assemble _e_ filename
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

	// check it exists
	FILE* tempFile = NULL;
	tempFile = fopen ( szEncryptedFilename ,"r" );
	if ( tempFile )
	{
		fclose ( tempFile );
		strcpy ( VirtualFilename , szEncryptedFilename );
		return true;
	}
	// end of encrypted file check

	#ifdef PHOTONMP
		// Photon has no workshop support
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

				if ( szWorkshopFilenameFolder[c] == '_' )
					szWorkshopFilenameFolder[c] = '@';
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
			tempFile = fopen ( szTempName ,"r" );
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
				tempFile = fopen ( szWorkshopFilename ,"r" );
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

bool bCanIUse_E_ = false;

bool CanIUse_E_()
{
	return bCanIUse_E_;
}

void SetCanUse_e_ ( int flag )
{
	if ( flag == 1 ) 
		bCanIUse_E_ = true;
	else
		bCanIUse_E_ = false;
}

bool Steam_CanIUse_W_()
{

	// Check if the functions pointers exist, if they don't - jolly well make them!
	/*if ( Steam_SteamIsWorkshopLoadingOnDLL==NULL )
	{
		// Setup pointers to Steam functions
		SteamMultiplayerModule = LoadLibrary ( "SteamMultiplayer.dll" );

		if ( !SteamMultiplayerModule )
		{
			//MessageBox(NULL, "Unable to find SteamMultiplayer", "SteamMultiplayer Error", NULL);
			return false;
		}

		Steam_SteamGetWorkshopItemPathDLL=(t_SteamGetWorkshopItemPathDLL)GetProcAddress( SteamMultiplayerModule , "?SteamGetWorkshopItemPathDLL@@YAXPAD@Z" );
		Steam_SteamIsWorkshopLoadingOnDLL=(t_SteamIsWorkshopLoadingOnDLL)GetProcAddress ( SteamMultiplayerModule , "?SteamIsWorkshopLoadingOnDLL@@YAHXZ" );	
		// End of setup pointers to steam functions
	}

	if ( !Steam_SteamIsWorkshopLoadingOnDLL || !Steam_SteamGetWorkshopItemPathDLL ) 
		return false;*/	

	if ( SteamIsWorkshopLoadingOnDLL() == 1 )
		return true;

	return false;
}