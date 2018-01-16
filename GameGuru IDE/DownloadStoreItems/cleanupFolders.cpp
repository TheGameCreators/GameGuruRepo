#include "stdafx.h"
#include "..\Editor.h"
#include "..\resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "afxwin.h"
#include <vector>
#include <stack>
#include "cleanupFolders.h"

using namespace std;

// Delete any empty folders
void cleanupFolders()
{
	HANDLE			hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA data  = { 0 };
    
	stack  < char* > directoryListStack;
	vector < char* > directoryList;

	char folderToCheck[MAX_PATH];
	sprintf ( folderToCheck , "%s\\Files" , theApp.m_szDirectory );

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

	// lee - 240715 - avoid crash when directoryList.size() is empty
	if ( directoryList.size() > 0 )
	{
		for ( unsigned int c = directoryList.size()-1; c > 0; c-- )
		{
			RemoveDirectory ( directoryList [ c ] );
		}
	}

	// wipe the directory listing pointers
	for ( unsigned int i = 0; i < directoryList.size ( ); i++ )
		delete [ ] directoryList [ i ];


}