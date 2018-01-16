// Media List Builder.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"

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
	strcpy ( folderToCheck , "F:\\TGCSHARED\\GameGuru\\GameGuru Steam\\Steamworks SDK\\tools\\ContentBuilder\\content" );

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
					// its a file, let's add it
					char* p = new char [ 256 ];
					
					sprintf ( p, "%s\\%s", szCurrentDirectory, data.cFileName );		
					if ( strstr ( p , "showonstartup.ini") == NULL && strstr ( p , "tutorialonstartup.ini") == NULL && strstr ( p , "visuals.ini") == NULL )
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

	char *dest;
	char source[MAX_PATH];

	for ( unsigned int c = 0 ; c < fileList.size(); c++ )
	{
		dest = strstr ( fileList[c] , "F:\\TGCSHARED\\GameGuru\\GameGuru Steam\\Steamworks SDK\\tools\\ContentBuilder\\content" );
		if ( dest )
		{
			sprintf ( source , "F:\\TGCSHARED\\GameGuru\\GameGuru%s" , dest+80 );
			printf ( "Copying %d of %d\n", c, fileList.size() );

			CopyFile( source, dest , false );
		}
	}


	// wipe the directory listing pointers
	for ( unsigned int c = 0; c < directoryList.size ( ); c++ )
		delete [ ] directoryList [ c ];

	// wipe file listing pointers
	for ( unsigned int c = 0; c < fileList.size ( ); c++ )
		delete [ ] fileList [ c ];

	return 0;
}

