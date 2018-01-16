#ifndef _CTEXT_H_
#define _CTEXT_H_

#include "directx-macros.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Text\CPositionC.h"

#define DARKSDK 
#define MAX_NUM_VERTICES 50 * 6

struct FONT2DVERTEX 
{ 
	GGVECTOR4 p;
	DWORD		color;
	float		tu;
	float		tv;
};

struct FONT3DVERTEX 
{ 
	GGVECTOR3 p;
	GGVECTOR3 n;
	float		tu;
	float		tv;
};

inline FONT2DVERTEX InitFont2DVertex ( const GGVECTOR4& p, GGCOLOR color, FLOAT tu, FLOAT tv )
{
    FONT2DVERTEX v;   
	
	v.p     = p;
	v.color = color;
	v.tu    = tu;
	v.tv    = tv;

    return v;
}

inline FONT3DVERTEX InitFont3DVertex ( const GGVECTOR3& p, const GGVECTOR3& n, FLOAT tu, FLOAT tv )
{
    FONT3DVERTEX v;
	
	v.p  = p;
	v.n  = n;
	v.tu = tu;
	v.tv = tv;

    return v;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK void TextConstructor ( void );
DARKSDK void TextDestructor  ( void );
DARKSDK void TextSetErrorHandler ( LPVOID pErrorHandlerPtr );
DARKSDK void TextPassCoreData( LPVOID pGlobPtr );
DARKSDK void TextRefreshGRAFIX ( int iMode );

#ifdef DARKSDK_COMPILE
	void ConstructorText ( HINSTANCE );
	void DestructorText  ( void );
	void SetErrorHandlerText ( LPVOID pErrorHandlerPtr );
	void PassCoreDataText( LPVOID pGlobPtr );
	void RefreshGRAFIXText ( int iMode );
#endif

DARKSDK void SetupFont   ( void );
DARKSDK void TextSetupStates ( void );
DARKSDK void TextRecreate    ( void );

DARKSDK void SetTextColor ( int iAlpha, int iRed, int iGreen, int iBlue );
//DARKSDK void SetTextFont          ( char* szTypeface, int iCharacterSet );		// set the text font and character set
DARKSDK void Text ( int iX, int iY, char* szText );
DARKSDK int  GetTextWidth          ( char* szString );
DARKSDK int  GetTextHeight         ( char* szString );

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// COMMAND FUNCTIONS /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK int	  Asc	( LPSTR dwSrcStr);
DARKSDK DWORD Bin	( DWORD pDestStr, int iValue );
DARKSDK LPSTR Chr	( int iValue );
DARKSDK DWORD Hex	( DWORD pDestStr, int iValue );
DARKSDK LPSTR Left	( LPSTR szText, int iValue );
DARKSDK int	  Len	( LPSTR dwSrcStr );
DARKSDK LPSTR Lower ( LPSTR szText );
DARKSDK LPSTR Mid	( LPSTR szText, int iValue );
DARKSDK LPSTR Right ( LPSTR szText, int iValue );
DARKSDK LPSTR Str	( float fValue );
DARKSDK LPSTR StrEx	( float fValue, int iDecPlaces );
DARKSDK LPSTR Str	( int iValue );
DARKSDK LPSTR Upper ( LPSTR szText );

DARKSDK float ValF	( LPSTR dwSrcStr );
DARKSDK DWORD StrDouble ( DWORD pDestStr, double dValue );
DARKSDK DWORD StrDoubleInt ( DWORD pDestStr, LONGLONG lValue );
DARKSDK LONGLONG ValR ( DWORD dwSrcStr );

DARKSDK void PerformChecklistForFonts ( void );
DARKSDK void Text					( int iX, int iY, LPSTR szText );
DARKSDK void CenterText				( int iX, int iY, LPSTR szText );						// centre text at x and y coordinates
DARKSDK void SetTextFont		  ( LPSTR szTypeface );							// set the text font
DARKSDK void SetTextFont		  ( LPSTR szTypeface, int iCharacterSet );		// set the text font and character set
DARKSDK void SetTextSize          ( int iSize );								// set the size
DARKSDK void SetTextToNormal      ( void );										// set text to normal style
DARKSDK void SetTextToItalic      ( void );										// set text to italic
DARKSDK void SetTextToBold        ( void );										// set text to bold
DARKSDK void SetTextToBoldItalic  ( void );										// set text to bold italic
DARKSDK void SetTextToOpaque      ( void );										// set text to opaque
DARKSDK void SetTextToTransparent ( void );										// set text to transparent

DARKSDK int   TextBackgroundType ( void );
DARKSDK DWORD TextFont			 ( DWORD pDestStr );
DARKSDK int   TextSize           ( void );
DARKSDK int   TextStyle          ( void );
DARKSDK int   TextWidth          ( DWORD szString );
DARKSDK int   TextHeight         ( DWORD szString );

DARKSDK void Text3D     ( char* szText );										// print 3D text at x and y coordinates

DARKSDK LPSTR	  Spaces	( int iSpaces );

// u74b7 - removed append statement as not fixable
//DARKSDK void  Append        ( DWORD dwA, DWORD dwB );
DARKSDK void  Reverse       ( LPSTR dwA );
DARKSDK int   FindFirstChar ( LPSTR dwSource, LPSTR dwChar );
DARKSDK int   FindLastChar  ( LPSTR dwSource, LPSTR dwChar );
DARKSDK int   FindSubString ( LPSTR dwSource, LPSTR dwString );
DARKSDK int   CompareCase   ( LPSTR dwA, LPSTR dwB );
DARKSDK LPSTR FirstToken    ( LPSTR dwSource, LPSTR dwDelim );
DARKSDK LPSTR NextToken     ( LPSTR dwDelim );

DARKSDK LPSTR GetReturnStringFromTEXTWorkString(char* WorkString);

#ifdef DARKSDK_COMPILE
		int			dbAsc						( char* dwSrcStr);
		char* 		dbBin						( int iValue );
		char* 		dbChr						( int iValue );
		char* 		dbHex						( int iValue );
		char* 		dbLeft						( char* szText, int iValue );
		int			dbLen						( char* dwSrcStr );
		char* 		dbLower 					( char* szText );
		char* 		dbMid						( char* szText, int iValue );
		char* 		dbRight 					( char* szText, int iValue );
		char* 		dbStr						( float fValue );
		char* 		dbStr						( int iValue );
		char* 		dbUpper 					( char* szText );
		float 		dbValF						( char* dwSrcStr );
		double		dbStrDouble					( double dValue );
		LONGLONG	dbValR						( char* dwSrcStr );

		void		dbPerformChecklistForFonts	( void );
		void		dbText						( int iX, int iY, char* szText );
		void 		dbCenterText				( int iX, int iY, char* szText );
		void 		dbSetTextFont				( char* szTypeface );
		void 		dbSetTextFont		  		( char* szTypeface, int iCharacterSet );	
		void 		dbSetTextSize          		( int iSize );							
		void 		dbSetTextToNormal      		( void );								
		void 		dbSetTextToItalic      		( void );								
		void 		dbSetTextToBold        		( void );								
		void 		dbSetTextToBoldItalic  		( void );								
		void 		dbSetTextToOpaque      		( void );								
		void 		dbSetTextToTransparent 		( void );								
		
		int   		dbTextBackgroundType 		( void );
		char* 		dbTextFont			 		( void );
		int   		dbTextSize           		( void );
		int   		dbTextStyle          		( void );
		int			dbTextWidth          		( char* szString );
		int			dbTextHeight         		( char* szString );

		void		dbText3D     				( char* szText );									

		char*		dbSpaces					( int iSpaces );

		void		dbAppend        			( char* dwA, char* dwB );
		void		dbReverse       			( char* dwA );
		int			dbFindFirstChar 			( char* dwSource, char* dwChar );
		int			dbFindLastChar  			( char* dwSource, char* dwChar );
		int			dbFindSubString 			( char* dwSource, char* dwString );
		// mike - 220107 - make second param char*
		int			dbCompareCase   			( char* dwA, char* dwB );
		char*		dbFirstToken    			( char* dwSource, char* dwDelim );
		char*		dbNextToken     			( char* dwDelim );

		void		dbSetTextColor				( int iAlpha, int iRed, int iGreen, int iBlue );
		void		dbSetTextFont				( char* szTypeface, int iCharacterSet );
		void		dbText						( int iX, int iY, char* szText );
		int			dbGetTextWidth				( char* szString );
		int			dbGetTextHeight				( char* szString );

		// lee - 300706 - GDK fixes
		void 		dbSetTextOpaque      		( void );								
		void 		dbSetTextTransparent 		( void );								
		float 		dbVal						( char* dwSrcStr );

#endif

//////////////////////////////////////////////////////////////////////////////////
#endif _CTEXT_H_