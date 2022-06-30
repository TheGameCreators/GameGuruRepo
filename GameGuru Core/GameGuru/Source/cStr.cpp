//
// cStr code
//

// Includes 
#include "stdafx.h"
#include "cStr.h"
#include "windows.h"
#include <map>
#include <sstream>

// External Includes
#include "globstruct.h"
#include "..\..\Dark Basic Public Shared\Dark Basic Pro SDK\DarkSDK\Core\resource.h"
#include "..\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Core\SteamCheckForWorkshop.h"
#include <ctype.h>

extern "C" FILE* GG_fopen( const char* filename, const char* mode );

bool noDeleteCSTR = false;
#define STRMAXSIZE 4096
//#define STRMINSIZE 256 // some DLUA strings can easily exceed 256 characters!
#ifdef PRODUCTCLASSIC
//PE: We cant affort this in classic, when you add one object to a level:
//PE: we have around 30 cstr inside entityelement , each is constructed with s="" that preallocate STRMINSIZE.
//PE: 30*1024 = 30kb per entity.
//PE: I have set it to 128 (default) in classic, it should be fine as it will grow if needed.
//PE: Classic, everything above 100 has a very low realloc rate so use 100.
#define STRMINSIZE 100
#else
//#define STRMINSIZE 1024
//PE: The default STRMINSIZE use tons of mem, i think we fixed all bugs in here so we should be able to use a lower amount, it will increase if needed.
#define STRMINSIZE 512
#endif

//#define TESTUSEOFSTRINGS
#ifdef TESTUSEOFSTRINGS
int CheckPreSize[5000];
int CheckReSize[5000];
#endif

cStr::cStr ( const cStr& cString )
{

	m_size = strlen ( cString.m_pString );

	#ifdef TESTUSEOFSTRINGS
	if (m_size < 5000) CheckPreSize[m_size]++;
	#endif

	//PE: Should be >= if size was exactly STRMINSIZE we would get a heap error.
	if ( m_size >= STRMINSIZE )
	{
		m_pString = new char [ sizeof ( char ) * ( m_size ) + 1 ];
		m_szTemp = new char [ sizeof ( char ) * ( m_size ) + 1 ];
	}
	else
	{
		m_pString = new char [ STRMINSIZE ];
		m_szTemp = new char [ STRMINSIZE ];	
	}

	memset ( m_pString, 0, sizeof ( m_pString ) );
	memset ( m_szTemp, 0, sizeof ( m_szTemp ) );
	strcpy ( m_pString, cString.m_pString );
	
}

cStr::cStr ( char* szString )
{
	m_size = strlen ( szString );
#ifdef TESTUSEOFSTRINGS
	if (m_size < 5000) CheckPreSize[m_size]++;
#endif

	if ( m_size >= STRMINSIZE )
	{
		m_pString = new char [ sizeof ( char ) * ( m_size ) + 1 ];
		m_szTemp = new char [ sizeof ( char ) * ( m_size ) + 1 ];
	}
	else
	{
		m_pString = new char [ STRMINSIZE ];
		m_szTemp = new char [ STRMINSIZE ];
	}

	memset ( m_pString, 0, sizeof ( m_pString ) );
	memset ( m_szTemp, 0, sizeof ( m_szTemp ) );
	strcpy ( m_pString, szString );
	
}

cStr::cStr ( int iValue )
{
	m_pString = new char [ STRMINSIZE ];
	memset ( m_pString, 0, sizeof ( m_pString ) );
	sprintf ( m_pString, "%i", iValue );

	m_szTemp = new char [ STRMINSIZE ];
	memset ( m_szTemp, 0, sizeof ( m_szTemp ) );
}

cStr::cStr ( float fValue )
{
	m_pString = new char [ STRMINSIZE ];
	memset ( m_pString, 0, sizeof ( m_pString ) );
	sprintf ( m_pString, "%.2f", fValue );

	m_szTemp = new char [ STRMINSIZE ];
	memset ( m_szTemp, 0, sizeof ( m_szTemp ) );
}

cStr::cStr ( double dValue )
{
	m_pString = new char [ STRMINSIZE ];
	memset ( m_pString, 0, sizeof ( m_pString ) );
	sprintf ( m_pString, "%.2f", ( float ) dValue );

	m_szTemp = new char [ STRMINSIZE ];
	memset ( m_szTemp, 0, sizeof ( m_szTemp ) );
}

cStr::cStr ( )
{
	m_pString = new char [ STRMINSIZE ];
	memset ( m_pString, 0, sizeof ( m_pString ) );
	strcpy ( m_pString, "" );

	m_szTemp = new char [ STRMINSIZE ];
	memset ( m_szTemp, 0, sizeof ( m_szTemp ) );
}

cStr::~cStr ( )
{
	if ( ! noDeleteCSTR )
	{
		if ( m_pString )
			delete [ ] m_pString;

		if ( m_szTemp )
			delete [] m_szTemp;  //PE: Heap error here if >256 strings and using s=s+" "
	}
}

cStr cStr::operator + ( const cStr& other )
{
	//PE: We have a bug here if using >= 256 strings and using:
	// cstr s = (large string)
	// s = s + " " //This generates a heap error.
	// s += " " // This works.

	m_size = strlen ( other.m_pString ) + strlen ( m_pString );

	if ( m_size >= STRMINSIZE )
	{
#ifdef TESTUSEOFSTRINGS
		if (m_size < 5000) CheckReSize[m_size]++;
#endif

		if ( m_szTemp )
			delete [] m_szTemp;

		m_szTemp = new char [ sizeof ( char ) * ( m_size ) + 1 ];
	}

	if ( m_pString )
		strcpy ( m_szTemp, m_pString );

	if ( other.m_pString )
		strcat ( m_szTemp, other.m_pString );

	return cStr ( m_szTemp );
}

cStr& cStr::operator += ( const cStr& other )
{

	m_size = strlen ( other.m_pString ) + strlen ( m_pString );

	if ( m_size >= STRMINSIZE )
	{
#ifdef TESTUSEOFSTRINGS
		if (m_size < 5000) CheckReSize[m_size]++;
#endif

		char* newstring = new char [ sizeof ( char ) * ( m_size ) + 1 ];
		strcpy ( newstring , m_pString );
		delete[] m_pString;
		m_pString = newstring;
		delete[] m_szTemp;
		m_szTemp = new char [ sizeof ( char ) * ( m_size ) + 1 ];
	}

	if ( other.m_pString )
		strcat ( m_pString, other.m_pString );

	return *this;
}
		
cStr cStr::operator = ( const cStr& other )
{
	m_size = strlen ( other.m_pString );

	if ( m_size >= STRMINSIZE )
	{
#ifdef TESTUSEOFSTRINGS
		if (m_size < 5000) CheckReSize[m_size]++;
#endif

		char* newstring = new char [ sizeof ( char ) * ( m_size ) + 1 ];
		delete[] m_pString;
		m_pString = newstring;
		delete[] m_szTemp;
		m_szTemp = new char [ sizeof ( char ) * ( m_size ) + 1 ];
	}

	strcpy ( m_pString, other.m_pString );

	//return *this;
	return cStr ( other.m_pString );
}

cStr& cStr::operator = ( const char* other )
{

	m_size = strlen ( other );

	if ( m_size >= STRMINSIZE )
	{
#ifdef TESTUSEOFSTRINGS
		if (m_size < 5000) CheckReSize[m_size]++;
#endif
		char* newstring = new char [ sizeof ( char ) * ( m_size ) + 1 ];
		delete[] m_pString;
		m_pString = newstring;
		delete[] m_szTemp;
		m_szTemp = new char [ sizeof ( char ) * ( m_size ) + 1 ];
	}

	strcpy ( m_pString, other );

	return *this;
}

bool cStr::operator == ( const cStr& s )
{
	if ( m_pString )
		if ( strcmp ( s.m_pString , m_pString ) == 0 ) 
			return true;

	return false;
}
bool cStr::operator == ( const char* s )
{
	if ( m_pString )
		if ( strcmp ( s , m_pString ) == 0 )
			return true;

	return false;
}

bool cStr::operator != ( const cStr& s )
{
	if ( m_pString )
		if ( strcmp ( s.m_pString , m_pString ) != 0 )
			return true;
	
	return false;
}

bool cStr::operator != ( const char* s )
{
	if ( m_pString )
		if ( strcmp ( s, m_pString ) != 0 )
			return true;

	return false;
}

int cStr::Len ( void )
{
	if ( m_pString )
		return strlen ( m_pString );

	return 0;
}

char* cStr::Get ( void )
{
	return m_pString;
}

int cStr::Val ( void )
{
	if ( m_pString )
		return atoi ( m_pString );

	return 0;
}

float cStr::ValF ( void )
{
	if ( m_pString )
		return (float)atof ( m_pString );

	return 0.0f;
}

cStr cStr::Upper ( void )
{
	char szTemp [STRMAXSIZE] = "";
	
	if ( m_pString )
	{
		for ( int i = 0; i < (int)strlen ( m_pString ); i++ )
			szTemp [ i ] = toupper ( m_pString [ i ] );
	}

	return cStr ( szTemp );
}

cStr cStr::Lower ( void )
{
	char szTemp [STRMAXSIZE] = "";
	
	if ( m_pString )
	{
		for ( int i = 0; i < (int)strlen ( m_pString ); i++ )
			szTemp [ i ] = tolower ( m_pString [ i ] );
	}

	return cStr ( szTemp );
}

cStr cStr::Left ( int iCount )
{
	char szTemp [STRMAXSIZE] = "";
	
	if ( m_pString && iCount < (int)strlen ( m_pString ) )
	{
		for ( int i = 0; i <= iCount; i++ )
			szTemp [ i ] = m_pString [ i ];
	}

	return cStr ( szTemp );
}

cStr cStr::Right ( int iCount )
{
	char szTemp [STRMAXSIZE] = "";
	
	if ( m_pString && iCount >= 0 )
	{
		for ( int i = (int)strlen ( m_pString ) - iCount, j = 0; i < (int)strlen ( m_pString ); i++ )
			szTemp [ j++ ] = m_pString [ i ];
	}

	return cStr ( szTemp );
}

cStr cStr::Mid ( int iPoint )
{
	char szTemp [STRMAXSIZE] = "";
	
	if ( m_pString && iPoint >= 0 && iPoint < (int)strlen ( m_pString ) )
		szTemp [ 0 ] = m_pString [ iPoint ];

	return cStr ( szTemp );
}

#ifdef bent
char cStr::work[2048] = "";

char* cStr::Get ( void )
{
	if ( !pString )
	{
		pString = new char [ sizeof ( char ) * 1024 ];
		strcpy ( pString , "" );
		iSize = 0;
	}
	return pString;
}

cStr::cStr ( )
{
	pString = new char [ sizeof ( char ) * 1024 ];
	strcpy ( pString , "" );
	iSize = 0;
}

cStr::cStr ( char* szSet )
{

	if ( strlen ( szSet ) < 1024 )
		pString = new char [ sizeof ( char ) * 1024 ];
	else
		pString = new char [ sizeof ( char ) * strlen ( szSet ) + 1 ];

	strcpy ( pString, szSet );

	iSize = strlen ( szSet );
}

cStr::cStr ( int iSet )
{
	sprintf ( work , "%i" , iSet );
	if ( strlen ( work ) < 1024 )
		pString = new char [ sizeof ( char ) * 1024 ];
	else
		pString = new char [ sizeof ( char ) * strlen ( work ) + 1 ];
	strcpy ( pString, work );

	iSize = strlen ( work );
}

cStr::cStr ( float fSet )
{
	sprintf ( work , "%.3f" , fSet );
	if ( strlen ( work ) < 1024 )
		pString = new char [ sizeof ( char ) * 1024 ];
	else
		pString = new char [ sizeof ( char ) * strlen ( work ) + 1 ];

	strcpy ( pString, work );

	iSize = strlen ( work );
}

cStr::~cStr ( )
{
	/*if ( pString ) 
	{
		delete[] pString;
		pString = NULL;
	}*/
}

/*const cStr &cStr::operator = ( const cstr& Equal )
{
	if ( pString ) delete[] pString;

	if ( strlen ( Equal.pString ) < 1024 )
		pString = new char [ sizeof ( char ) * 1024 ];
	else
		pString = new char [ sizeof ( char ) * (strlen ( Equal.pString ) + 1) ];

	strcpy ( pString, Equal.pString );

	iSize = strlen ( pString );

	return *this;
}*/

const cStr &cStr::operator = ( const char* szEqual )
{
	if ( pString ) delete[] pString;

	if ( strlen ( szEqual ) < 1024 )
		pString = new char [ sizeof ( char ) * 1024 ];
	else
		pString = new char [ sizeof ( char ) * (strlen ( szEqual ) + 1) ];

	strcpy ( pString, szEqual );

	iSize = strlen ( pString );

	return *this;
}

const cStr &cStr::operator = ( const unsigned long pDWEqual )
{
	char* szEqual = (char*)pDWEqual;

	if ( pString ) delete[] pString;

	if ( strlen ( szEqual ) < 1024 )
		pString = new char [ sizeof ( char ) * 1024 ];
	else
		pString = new char [ sizeof ( char ) * (strlen ( szEqual ) + 1) ];

	strcpy ( pString, szEqual );

	iSize = strlen ( pString );

	delete [] szEqual;

	return *this;
}

cStr &cStr::operator + ( const cStr &add )
{
	char* ret = "";

	if ( pString && add.pString )
	{

		if ( strlen ( pString ) + strlen ( add.pString ) < 1024 )
			ret = new char [ sizeof ( char ) * 1024 ];
		else
			ret = new char[ sizeof ( char ) * ( strlen ( pString ) + strlen ( add.pString ) + 1 ) ];

		strcpy ( ret, pString );
		strcat ( ret, add.pString );

		delete [] pString;
		pString = ret;

	}

	iSize = strlen ( pString );

	return *this;
}

cStr &cStr::operator + ( const unsigned long pDWAdd )
{
	char* pAdd = (char*)pDWAdd;
	char* ret = "";

	if ( pString && pAdd )
	{
		if ( strlen ( pString ) + strlen ( pAdd ) < 1024 )
			ret = new char [ sizeof ( char ) * 1024 ];
		else
			ret = new char[ sizeof ( char ) * ( strlen ( pString ) + strlen ( pAdd ) + 1 ) ];

		strcpy ( ret, pString );
		strcat ( ret, pAdd );

		delete [] pAdd;

		delete [] pString;
		pString = ret;
	}

	iSize = strlen ( pString );

	return *this;
}

cStr &cStr::operator += ( const cStr &add )
{

	if ( pString )
	{
		char* p;

		if ( strlen ( pString ) + strlen ( add.pString ) < 1024 )
			p = new char [ sizeof ( char ) * 1024 ];
		else
			p = new char[ sizeof ( char ) * ( strlen ( pString ) + strlen ( add.pString ) ) + 1 ];

		strcpy ( p , pString );
		strcat ( p , add.pString );

		delete [] pString;
		pString = p;

		iSize = strlen ( pString );

	}
	return *this;
}

cStr &cStr::operator += ( const unsigned long pDWAdd )
{
	char* pAdd = (char*)pDWAdd;

	if ( pString && pAdd )
	{
		char* p;

		if ( strlen ( pString ) + strlen ( pAdd ) < 1024 )
			p = new char [ sizeof ( char ) * 1024 ];
		else
			p = new char[ sizeof ( char ) * ( strlen ( pString ) + strlen ( pAdd ) ) + 1 ];

		strcpy ( p , pString );
		strcat ( p , pAdd );
		
		delete pAdd;

		delete [] pString;
		pString = p;

		iSize = strlen ( pString );

	}
	return *this;
}


bool cStr::operator == ( const cStr& s ) const
{
	if ( _strnicmp ( s.pString , pString , strlen(s.pString) ) == 0 ) return true;
	return false;
}

bool cStr::operator == ( const char* s ) const
{
	if ( _strnicmp ( s , pString , strlen(s) ) == 0 ) return true;
	return false;
}

bool cStr::operator != ( const cStr& s ) const
{
	if ( _strnicmp ( s.pString , pString , strlen(s.pString) ) != 0 ) return true;
	return false;
}

bool cStr::operator != ( const char* s ) const
{
	if ( _strnicmp ( s , pString , strlen(s) ) != 0 ) return true;
	return false;
}
#endif

/*int ArrayCount ( std::vector<cstr> array )
{
	return array.size()-1;
}*/

int ArrayCount2 ( std::vector<std::vector<cstr>> array )
{
	return (array.size()*array[0].size()) - 1;
}

cstr ArrayAt ( std::vector<std::vector<cstr>>& array, int l )
{
	int a,b;
	b = (int)(floor((float)l / (float)array.size()));
	a = l - (b*array.size());
	return array[a][b];
}


void SaveArray ( char* filename , std::vector<cstr> array )
{
	FILE* file;
	char t[2048];

	DeleteFileA ( filename );

	file = GG_fopen ( filename , "w" );

	if ( file )
	{
		for ( int c = 0 ; c < (int)array.size() ; c++ )
		{
			strcpy ( t , array[c].Get() );
			strcat ( t , "\n" );
			fputs ( t , file );
		}
		fclose ( file );
	}

}

void SaveArray ( char* filename , std::vector<std::vector<cstr>> array )
{
	FILE* file;
	char t[2048];
	int size = array[0].size();

	DeleteFileA ( filename );

	file = GG_fopen ( filename , "w" );

	if ( file )
	{		

		for ( int b = 0 ; b <size ; b++ )
		{
			for ( int a = 0 ; a < (int)array.size() ; a++ )
			{
				strcpy ( t , array[a][b].Get() );
				strcat ( t , "\n" );
				fputs ( t , file );
			}
		}

		fclose ( file );
	}

}

void LoadArray ( char* filename , std::vector<cstr>& array )
{
	FILE* file;
	char t[2048];
	int lineCount = 0;

	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, filename);
	//g_pGlob->UpdateFilenameFromVirtualTable( VirtualFilename);

	CheckForWorkshopFile ( VirtualFilename );

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );

	file = GG_fopen ( VirtualFilename , "r" );

	// first we see how big the file is to ensure array is big enough
	if ( file )
	{
		while ( !feof(file) )
		{
			fgets ( t , 2047 , file );
			lineCount++;
		}

		// if the last line is blank, remove it
		if ( strcmp ( t , "" ) == 0 || strcmp ( t , "\n" ) == 0 )
			lineCount--;

		fclose (file);

		if ( lineCount >= (int)array.size() ) Dim ( array , lineCount );

		file = GG_fopen ( VirtualFilename , "r" );
		if ( file )
		{
			for ( int c = 0 ; c < lineCount ; c++ )
			{
				fgets ( t , 2047 , file );
				if ( t[strlen(t)-1] == '\n' ) t[strlen(t)-1] = '\0';
				array[c] = t;
			}
			fclose ( file );
		}
	}

	g_pGlob->Encrypt( VirtualFilename );

}

void LoadArray ( char* filename , std::vector<std::vector<cstr>>& array )
{
	FILE* file;
	char t[2048];
	int lineCount = 0;
	int size = array[0].size();

	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, filename);
	//g_pGlob->UpdateFilenameFromVirtualTable( VirtualFilename);

	CheckForWorkshopFile ( VirtualFilename );

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );

	file = GG_fopen ( VirtualFilename , "r" );

	if ( file )
	{

		file = GG_fopen ( VirtualFilename , "r" );
		if ( file )
		{
			for ( int b = 0 ; b < size ; b++ )
			{
				for ( int a = 0 ; a < (int)array.size() ; a++ )
				{
					fgets ( t , 2047 , file );
					if ( t[strlen(t)-1] == '\n' ) t[strlen(t)-1] = '\0';
					array[a][b] = t;
				}
			}
			fclose ( file );
		}
	}

	g_pGlob->Encrypt( VirtualFilename );

}

// MATHEMATICAL COMMANDS
extern double gDegToRad;
extern double gRadToDeg;
float atan2deg( float fA , float fB ) { return (float)(atan2(fA, fB)*gRadToDeg); }

// Other string functions
void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}



