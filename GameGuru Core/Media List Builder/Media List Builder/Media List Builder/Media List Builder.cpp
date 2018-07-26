// Media List Builder.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"

void makeDevVersion();

int _tmain(int argc, _TCHAR* argv[])
{

	using namespace std;

	char originalLocation[MAX_PATH];
	GetCurrentDirectory( MAX_PATH , originalLocation );

	HANDLE			hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA data  = { 0 };
    
	stack  < char* > directoryListStack;
	vector < char* > directoryList;
	vector < char* > fileList;

	char folderToCheck[MAX_PATH];
	//sprintf ( folderToCheck , "%s\\Files" , originalLocation );
	//strcpy ( folderToCheck , "F:\\TGCSHARED\\GameGuru\\GameGuru Steam\\Steamworks SDK\\tools\\ContentBuilder\\content\\Files" );
	strcpy ( folderToCheck , "F:\\Dropbox\\Dropbox\\GameGuru Builds\\Steam\\Steamworks SDK\\tools\\ContentBuilder\\content\\Files" );

	// add first directory into the listing
	directoryListStack.push ( folderToCheck );

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
					directoryList.push_back ( p );
				}
				else
				{
					// its a file, let's see if it is an fpe
					char* p = new char [ 256 ];
					
					sprintf ( p, "%s\\%s", szCurrentDirectory, data.cFileName );
					
					if ( strstr ( p , ".fpe" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "scriptbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "effectbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "skybank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "terrainbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "vegbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "gamecore\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "audiobank\\" ) )
						fileList.push_back ( p );					

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

	// make the log file
	FILE *f;
	//f = fopen ( "F:\\TGCSHARED\\GameGuru\\GameGuru Steam\\Steamworks SDK\\tools\\ContentBuilder\\content\\Files\\editors\\baseList.dat"  , "w" );
	f = fopen ( "F:\\Dropbox\\Dropbox\\GameGuru Builds\\Steam\\Steamworks SDK\\tools\\ContentBuilder\\content\\Files\\editors\\baseList.dat"  , "w" );

	if ( f )
	{
		char *p;

		for ( unsigned int c = 0 ; c < fileList.size(); c++ )
		{
			if ( c ) fputs ( "\n" , f );

			//p = strstr ( fileList[c] , "F:\\TGCSHARED\\GameGuru\\GameGuru Steam\\Steamworks SDK\\tools\\ContentBuilder\\content\\Files\\" );
			p = strstr ( fileList[c] , "F:\\Dropbox\\Dropbox\\GameGuru Builds\\Steam\\Steamworks SDK\\tools\\ContentBuilder\\content\\Files\\" );
			if ( p )
				strcpy ( fileList[c] , p+87 );

			fputs ( fileList[c] , f );
		}

		fclose (f);
	}

	// wipe the directory listing pointers
	for ( unsigned int c = 0; c < directoryList.size ( ); c++ )
		delete [ ] directoryList [ c ];

	// wipe file listing pointers
	for ( unsigned int c = 0; c < fileList.size ( ); c++ )
		delete [ ] fileList [ c ];

	makeDevVersion();


	return 0;
}

void makeDevVersion()
{
	using namespace std;

	char originalLocation[MAX_PATH];
	GetCurrentDirectory( MAX_PATH , originalLocation );

	HANDLE			hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA data  = { 0 };
    
	stack  < char* > directoryListStack;
	vector < char* > directoryList;
	vector < char* > fileList;

	char folderToCheck[MAX_PATH];
	//sprintf ( folderToCheck , "%s\\Files" , originalLocation );
	//strcpy ( folderToCheck , "F:\\TGCSHARED\\GameGuru\\GameGuru\\Files" );
	strcpy ( folderToCheck , "F:\\GameGuruRepo\\GameGuru\\Files" );

	// add first directory into the listing
	directoryListStack.push ( folderToCheck );

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
					directoryList.push_back ( p );
				}
				else
				{
					// its a file, let's see if it is an fpe
					char* p = new char [ 256 ];
					
					sprintf ( p, "%s\\%s", szCurrentDirectory, data.cFileName );
					
					if ( strstr ( p , ".fpe" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "scriptbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "effectbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "skybank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "terrainbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "vegbank\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "gamecore\\" ) )
						fileList.push_back ( p );
					else if ( strstr ( p , "audiobank\\" ) )
						fileList.push_back ( p );					

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

	// make the log file
	FILE *f;
	//f = fopen ( "F:\\TGCSHARED\\GameGuru\\GameGuru\\Files\\editors\\baseList.dat"  , "w" );
	f = fopen ( "F:\\GameGuruRepo\\GameGuru\\Files\\editors\\baseList.dat"  , "w" );

	if ( f )
	{
		char *p;

		for ( unsigned int c = 0 ; c < fileList.size(); c++ )
		{
			if ( c ) fputs ( "\n" , f );

			//p = strstr ( fileList[c] , "F:\\TGCSHARED\\GameGuru\\GameGuru\\Files\\" );
			p = strstr ( fileList[c] , "F:\\GameGuruRepo\\GameGuru\\Files\\" );
			if ( p )
				strcpy ( fileList[c] , p+37 );

			fputs ( fileList[c] , f );
		}

		fclose (f);
	}

	// wipe the directory listing pointers
	for ( unsigned int c = 0; c < directoryList.size ( ); c++ )
		delete [ ] directoryList [ c ];

	// wipe file listing pointers
	for ( unsigned int c = 0; c < fileList.size ( ); c++ )
		delete [ ] fileList [ c ];

}

