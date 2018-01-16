
#ifndef _CZIP_H_
#define _CZIP_H_

#ifdef WIN32
#define ISWINDOWS
#endif

#ifdef WIN64
#define ISWINDOWS
#endif

#ifdef ISWINDOWS
#include <windows.h>

#include "..\Zlib\zlib.h"
#include "..\Zlib\zip.h"
#include "..\Zlib\unzip.h"
#endif

#ifdef MACOSX
#define BYTE unsigned char
#include "zlib.h"
#include "zip.h"
#include "unzip.h"
#endif

#include <vector>

class cZip
{
	public:
		 cZip ( );
		~cZip ( );

		bool			Open         ( char* szFile, char* szPassword );
		bool			Add          ( char* szFilename, char* szFile, BYTE* pData = NULL, int iSize = 0 );
		BYTE*			Extract      ( char* szFilename, char* szOutputFile, int* piSize = NULL );
		unsigned int	GetFileCount ( void );
		char*			GetFilename  ( unsigned int iIndex );
		void			Close		 ( void );

	private:
		zipFile					m_ZipFile;
		unzFile					m_UnzipFile;
		unz_global_info			m_Info;
		std::vector < char* >	m_pszFiles;
		char					m_szPassword [ 256 ];
};

#endif