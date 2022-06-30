#define _CRT_SECURE_NO_WARNINGS

#define MAX_PATH 260
#include "cZip.h"

extern "C" FILE* GG_fopen( const char* filename, const char* mode );

cZip::cZip ( )
{
	// default values
	m_ZipFile   = 0;
	m_UnzipFile = 0;

	strcpy ( m_szPassword, "" );
}

cZip::~cZip ( )
{
	// close zip file and free up memory
	Close ( );
}



void cZip::Close ( void )
{
	// free memory from file list
	for ( unsigned int i = 0; i < m_pszFiles.size ( ); i++ )
		delete [ ] m_pszFiles [ i ];

	// clear list
	m_pszFiles.clear ( );

	// close unzip file if needed
	if ( m_UnzipFile )
		unzClose ( m_UnzipFile );

	// close zip file if needed
	if ( m_ZipFile )
		zipClose ( m_ZipFile, NULL );

	// reset
	m_ZipFile   = 0;
	m_UnzipFile = 0;
}

unsigned int cZip::GetFileCount ( void )
{
	// get number of files in zip
	return m_pszFiles.size ( );
}

char* cZip::GetFilename ( unsigned int iIndex )
{
	// get filename for given index
	if ( iIndex < m_pszFiles.size ( ) )
		return m_pszFiles [ iIndex ];
	else
		return NULL;
}

BYTE* cZip::Extract ( char* szFilename, char* szOutputFile, int* piSize )
{
	// either extract file to hard drive or get its contents

	// return if no valid file
	if ( !m_UnzipFile )
		return NULL;

	// see if we can find the file
	if ( unzLocateFile ( m_UnzipFile, szFilename, NULL ) != UNZ_OK )
		return NULL;

	// file info and name
	unz_file_info fileInfo;
	char          szZipFilename [ MAX_PATH ];

	// get info for the current file
	if ( unzGetCurrentFileInfo ( m_UnzipFile, &fileInfo, szZipFilename, MAX_PATH, NULL, 0, NULL, 0 ) != UNZ_OK )
		return NULL;

	// check if password is used, otherwise open file normally
	if ( strlen ( m_szPassword ) > 0 )
	{
		// open file with password
		if ( unzOpenCurrentFilePassword ( m_UnzipFile, m_szPassword ) != UNZ_OK )
			return NULL;
	}
	else if ( unzOpenCurrentFile ( m_UnzipFile ) != UNZ_OK )
		return NULL;

	// allocate array to store data
	BYTE* pData = new BYTE [ fileInfo.uncompressed_size ];

	// return if invalid
	if ( !pData )
		return NULL;

	// read data from zip into array
	if ( unzReadCurrentFile ( m_UnzipFile, pData, fileInfo.uncompressed_size ) != fileInfo.uncompressed_size )
	{
		delete [ ] pData;
		return NULL;
	}

	// store size if needed
	if ( piSize )
		*piSize = fileInfo.uncompressed_size;

	// see if we're dumping everything out to a file
	if ( szOutputFile )
	{
		// open the file
		FILE* fp = GG_fopen ( szOutputFile, "wb" );

		// check file has been opened
		if ( fp )
		{
			// write contents and close file
			fwrite ( pData, sizeof ( BYTE ) * fileInfo.uncompressed_size, 1, fp );
			fclose ( fp );

			// free up data
			delete [ ] pData;
			pData = NULL;
		}
	}
		
	// close zip file
	unzCloseCurrentFile ( m_UnzipFile );

	// return data
	return pData;
}

bool cZip::Open ( char* szFile, char* szPassword )
{
	// open zip file

	// store password
	if ( szPassword )
		strcpy ( m_szPassword, szPassword );

	// open the file
	m_UnzipFile = unzOpen ( szFile );

	// if we can't open the zip then assume we're going to make a new one and add to it
	if ( m_UnzipFile == NULL )
	{
		// make a new one
		m_ZipFile = zipOpen ( szFile, APPEND_STATUS_CREATE );

		// make sure it's okay
		if ( m_ZipFile == NULL )
			return false;

		// back out
		return true;
	}

	// get info for the zip
	if ( unzGetGlobalInfo ( m_UnzipFile, &m_Info ) != UNZ_OK )
	{
		// close and return
		unzClose ( m_UnzipFile );
		return false;
	}

	// run through file entries
	for ( unsigned int i = 0; i < m_Info.number_entry; i++ )
	{
		// make new character array to hold name
		unz_file_info fileInfo;
		char*         pszFilename = new char [ MAX_PATH ];

		// blank string in file name for now
		strcpy ( pszFilename, "" );

		// get file info
		if ( unzGetCurrentFileInfo ( m_UnzipFile, &fileInfo, pszFilename, MAX_PATH, NULL, 0, NULL, 0 ) != UNZ_OK )
		{
			// close and return
			unzClose ( m_UnzipFile );
			return false;
		}

		// store filename in list
		m_pszFiles.push_back ( pszFilename );

		// close current file in zip
		unzCloseCurrentFile ( m_UnzipFile );

		// move to the next file in the zip
		if ( ( i + 1 ) < m_Info.number_entry )
		{
			if ( unzGoToNextFile ( m_UnzipFile ) != UNZ_OK )
			{
				// close and return
				unzClose ( m_UnzipFile );
				return false;
			}
		}
	}

	// restore to first file position in zip
	unzGoToFirstFile ( m_UnzipFile );

	// all done
	return true;
}

bool cZip::Add ( char* szFilename, char* szFile, BYTE* pData, int iSize )
{
	// add file to zip

	// locals
	unsigned long CRC           = crc32 ( 0L, Z_NULL, 0 );
	FILE*         pFile         = NULL;
	BYTE*         pOriginalData = NULL;
	int			  iResult       = 0;
	zip_fileinfo  zipInfo;
	
	// are we adding it from a file
	if ( szFile )
	{
		// open the file
		pFile = GG_fopen ( szFilename, "rb" );

		if ( !pFile )
			return false;

		// read its contents and copy data
		fseek ( pFile, 0, SEEK_END );
		iSize = ftell ( pFile );
		rewind ( pFile );
		pOriginalData = new BYTE [ iSize ];
		memset ( pOriginalData, 0, sizeof ( BYTE ) * iSize );
		fread ( pOriginalData, iSize * sizeof ( BYTE ), 1, pFile );
		fclose ( pFile );
	}
	else
	{
		// adding from pointer passed in
		pOriginalData = pData;
	}

	// build CRC data
	CRC = crc32 ( CRC, ( Bytef* ) pOriginalData, iSize );
	
	// clear zip info - no need to set any variables in here
	memset ( &zipInfo, 0, sizeof ( zip_fileinfo ) );

	// switching to Z_BEST_COMPRESSION makes things smaller, but could be slow on some machines, so using default for now
	if ( strlen ( m_szPassword ) > 0 )
		iResult = zipOpenNewFileInZip3 ( m_ZipFile, szFile, &zipInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0, 15, 8, Z_DEFAULT_STRATEGY, m_szPassword, CRC );
	else
		iResult = zipOpenNewFileInZip3 ( m_ZipFile, szFile, &zipInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0, 15, 8, Z_DEFAULT_STRATEGY, NULL, CRC );

	// check it was okay
	if ( iResult != ZIP_OK )
		return false;

	// now write the file
	if ( zipWriteInFileInZip ( m_ZipFile, pOriginalData, iSize ) != ZIP_OK )
		return false;

	// close file in zip
	zipCloseFileInZip ( m_ZipFile );

	// if we added to the zip from a file then wipe the data
	if ( szFile )
		delete [ ] pOriginalData;

	// all done
	return true;
}