//
// Text Code (Module Core)
//

// Includes
#include "stdio.h"
#include "ctextc.h"
#include ".\..\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Error\CError.h"
#include "globstruct.h"

// Defines
#define RETURNSTRONGPOOLMAX 40

// Externs
//extern PTR_FuncCreateStr g_pCreateDeleteStringFunction;

// Globals
char*			m_szTokenString = NULL;
unsigned int	m_dwTokenStringSize = 0;
unsigned int	m_dwTEXTWorkStringSize = 0;
char*			m_pTEXTWorkString = NULL;
char			g_szReturnStringPool[RETURNSTRONGPOOLMAX][1024];
int				g_iReturnStringPoolCount = 0;

// Module Utility Functions

//PE: Missing constructor.
void InitCTextC( void )
{
	if (!m_szTokenString)
	{
		m_dwTokenStringSize = 100;
		m_szTokenString = new char[100];
		memset(m_szTokenString, 0, sizeof(m_szTokenString));
	}
}

char* GetReturnStringFromTEXTWorkString(char* WorkString = m_pTEXTWorkString)
{
	char* pReturnString=NULL;
	if(WorkString)
	{
		DWORD dwSize=strlen(WorkString);
		pReturnString = g_szReturnStringPool[g_iReturnStringPoolCount];
		if ( ++g_iReturnStringPoolCount >= RETURNSTRONGPOOLMAX ) g_iReturnStringPoolCount = 0;
		if(WorkString)
			strcpy(pReturnString, WorkString);
		else
			strcpy(pReturnString, "");
		return pReturnString;
	}	
	return "";
}

void ValidateWorkStringBySize ( unsigned int dwSize )
{
	if (m_pTEXTWorkString)
	{
		if (m_dwTEXTWorkStringSize < dwSize)
		{
			if (m_pTEXTWorkString)
			{
				delete[] m_pTEXTWorkString;
				m_pTEXTWorkString = NULL;
			}
		}
	}

	// create new work string of good size
	if ( m_pTEXTWorkString==NULL )
	{
		m_dwTEXTWorkStringSize = dwSize+1;
		m_pTEXTWorkString = new char[m_dwTEXTWorkStringSize];
		memset ( m_pTEXTWorkString, 0, m_dwTEXTWorkStringSize );
	}
}

void ValidateWorkString(char* pString)
{
	// Size from string
	if ( pString ) ValidateWorkStringBySize ( strlen(pString)+1 );
}

// Module Functions

int	Asc ( char* dwSrcStr )
{
	if(dwSrcStr)
		return (int)*dwSrcStr;
	else
		return 0;
}

char* Bin ( char* pDestStr, int iValue )
{
	// Work string
	char* text=m_pTEXTWorkString;
	int t=0;
	text[t++]='%';
	for(int bit=31; bit>=0; bit--)
	{
		unsigned int mask = 1 << bit;
		if(iValue & mask)
			text[t++]='1';
		else
			text[t++]='0';
	}
	text[t]=0;

	// Create and return string
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Chr ( int iValue )
{
	// Work string
	m_pTEXTWorkString[0]=iValue;
	m_pTEXTWorkString[1]=0;

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Hex ( char* pDestStr, int iValue )
{
	// Work string
	wsprintf(m_pTEXTWorkString, "%X", iValue);

	// Create and return string
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Left ( char* szText, int iValue )
{
	// Work string
	ValidateWorkString ( (char*)szText );
	char* text=m_pTEXTWorkString;
	if(szText)
		strcpy(text, (char*)szText);
	else
		strcpy(text, "");

	if(iValue>0 && iValue<=(int)strlen(text))
		text[iValue]=0;
	else
		if(iValue<=0)
			strcpy(text, "");

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

int Len ( char* dwSrcStr )
{
	if(dwSrcStr)
		return strlen(dwSrcStr);
	else
		return 0;
}

char* Lower ( char* szText )
{
	// Work string
	ValidateWorkString ( (char*)szText );
	if(szText)
		strcpy(m_pTEXTWorkString, (char*)szText);
	else
		strcpy(m_pTEXTWorkString, "");

	_strlwr(m_pTEXTWorkString);

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Mid ( char* szText, int iValue )
{
	// Work string
	ValidateWorkString ( szText );
	char character;
	char* text = m_pTEXTWorkString;
	if(szText)
		strcpy_s(text, m_dwTEXTWorkStringSize, szText);
	else
		strcpy_s(text, 2, "");

	unsigned int index = iValue;
	if(index>0 && index<=strlen(text))
	{
		character=text[index-1];	
		text[0]=character;
		text[1]=0;
	}
	else
		text[0]=0;

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Right ( char* szText, int iValue )
{
	// Work string
	ValidateWorkString ( (char*)szText );
	char* text=m_pTEXTWorkString;
	if(szText)
		strcpy(text, (char*)szText);
	else
		strcpy(text, "");

	int w = 0;
	int length = strlen(text);
	int rightmost = length-iValue;
	if(rightmost>=0 && rightmost<=length)
	{
		for(int n=rightmost; n<length; n++)
			text[w++]=text[n];
		text[w]=0;
	}
	else
		if(rightmost>length)
			strcpy(text, "");
		
	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Str ( float fValue )
{
	// Work string
	sprintf(m_pTEXTWorkString, "%.12g", fValue);

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* StrEx	( float fValue, int iDecPlaces )
{
	// Work string
	char format[32];
	sprintf(format, "%%.%df", iDecPlaces );
	sprintf(m_pTEXTWorkString, format, fValue);

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Str ( int iValue )
{
	// Work string
	if ( m_pTEXTWorkString ) sprintf(m_pTEXTWorkString, "%d", iValue);

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* Upper ( char* szText )
{
	// Work string
	ValidateWorkString ( (char*)szText );
	if(szText)
		strcpy(m_pTEXTWorkString, (char*)szText);
	else
		strcpy(m_pTEXTWorkString, "");

	_strupr(m_pTEXTWorkString);

	// Create and return string
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

float ValF ( char* dwSrcStr )
{
	float fValue = 0.0f;
	if(dwSrcStr) fValue = (float)atof(dwSrcStr);
	return fValue;
}

LONGLONG ValR ( unsigned int dwSrcStr )
{
	LONGLONG lValue = 0;
	if(dwSrcStr) lValue = _atoi64((char*)dwSrcStr);
	return lValue;
}

char* StrDouble ( char* pDestStr, double dValue )
{
	// Work string
	sprintf(m_pTEXTWorkString, "%.16g", dValue);

	// Create and return string
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* StrDoubleInt ( char* pDestStr, long long lValue )
{
	// Work string
	sprintf(m_pTEXTWorkString, "%I64d", lValue);

	// Create and return string
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
	char* pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

char* SetupString ( char* szInput )
{
	char* pReturn = NULL;
	unsigned int dwSize  = strlen ( szInput );
	g_pGlob->CreateDeleteString((char**) &pReturn, dwSize + 1 );
	if ( !pReturn ) RunTimeError ( RUNTIMEERROR_NOTENOUGHMEMORY );	
	memcpy ( pReturn, szInput, dwSize );
	pReturn [ dwSize ] = 0;
	return pReturn;
}

void Reverse ( char* dwA )
{
	strrev ( dwA );
}

char* Spaces ( int iSpaces )
{
	if ( iSpaces < 0 )
	{
		char* pReturnString=NULL;
		g_pGlob->CreateDeleteString((char**)&pReturnString, 2 );
		memset((char*)pReturnString, 32, 2);
		pReturnString [ 0 ] = 0;
		pReturnString [ 1 ] = 0;
		return pReturnString;	
	}

	// Create and return string
	char* pReturnString=NULL;
	g_pGlob->CreateDeleteString((char**)&pReturnString, iSpaces+1 );
	memset((char*)pReturnString, 32, iSpaces);
	pReturnString[iSpaces]=0;
	return pReturnString;
}

int FindFirstChar ( char* dwSource, char* dwChar )
{
	char* pInt    = dwChar;
	char  pIntA   = *pInt;
	char* pFirst  = strchr ( dwSource, ( int ) pIntA );
	int   iResult = pFirst - dwSource + 1;
	return iResult;
}

int FindLastChar ( char* dwSource, char* dwChar )
{
	char* pInt    = dwChar;
	char  pIntA   = *pInt;
	char* pFirst  = strrchr ( dwSource, ( int ) pIntA );
	int   iResult = pFirst - dwSource + 1;
	return iResult;
}

int FindSubString ( char* dwSource, char* dwString )
{
	char* pFirst  = strstr ( dwSource, dwString );
	int   iResult = pFirst - dwSource + 1;
	return iResult;
}

char* FirstToken ( char* dwSource, char* dwDelim )
{
    char* szSource = dwSource;
    char* szDelim  = dwDelim;

    // If the delimiter an empty string, use a space
    if (szDelim == 0 || szDelim[0] == 0)
    {
        szDelim = " ";
    }

    // If there's a string, copy it to the temp area
    if ( szSource && szSource[0] )
    {
        // Get length, including null terminator
        unsigned int dwLength = strlen( szSource ) + 1;

        // If the temp area isn't large enough, make it so
        if ( dwLength > m_dwTokenStringSize )
        {
            // double the allocation is not enough, must use dwLength
            m_dwTokenStringSize = dwLength + 1;

			// free old string and create larger one
            delete[] m_szTokenString;
            m_szTokenString = new char[ m_dwTokenStringSize ];
			memset ( m_szTokenString, 0, sizeof(m_szTokenString) );
        }

        // Copy the source string into the temp area
        memcpy( m_szTokenString, szSource, dwLength );
    }
    else
    {
		//PE: Make sure we have a valid token string.
		if (!m_szTokenString)
		{
			m_dwTokenStringSize = 256;
			m_szTokenString = new char[m_dwTokenStringSize+1];
			memset(m_szTokenString, 0, sizeof(m_szTokenString));
		}
        m_szTokenString[ 0 ] = 0;
		return SetupString(""); //Just empty.
    }

	// free old string and create new one
    char* szToken = strtok ( m_szTokenString, szDelim );
    if ( szToken )
    {
	    return SetupString ( szToken );
    }
    return SetupString ( "" );
}

char* NextToken ( char* dwDelim )
{
    char* szDelim  = dwDelim;

    // If the delimiter an empty string, use a space
    if (szDelim == 0 || szDelim[0] == 0)
    {
        szDelim = " ";
    }

	// free old string and create new one
	char* szToken = strtok ( NULL, szDelim );
    if ( szToken )
    {
	    return SetupString ( szToken );
    }
    return SetupString ( "" );
}
