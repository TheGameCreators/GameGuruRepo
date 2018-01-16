#include "stdafx.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "afxwin.h"

#define SAFE_CLOSE(x)			if(x) { CloseHandle(x); x=NULL; }

bool FileExists(LPSTR pFilename)
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