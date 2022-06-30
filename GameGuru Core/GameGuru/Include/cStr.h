
#include <stdio.h>
#include <vector>

#ifndef __C_STR__

#define __C_STR__

#define cstr cStr

extern bool noDeleteCSTR;

class cStr
{
	public:
		cStr	( const cStr& cString );
		cStr	( char*  szString );
		cStr	( int    iValue );
		cStr	( float  fValue );
		cStr	( double dValue );
		cStr	( );
		~cStr	( );

		cStr	operator +  ( const cStr& other );
		cStr&	operator += ( const cStr& other );
		cStr	operator  = ( const cStr& other );
		cStr&	operator  = ( const char* other );
		bool	operator == ( const cStr& s );
		bool	operator == ( const char* s );
		bool	operator != ( const cStr& s );
		bool	operator != ( const char* s );

		int		Len   ( void );
		char*	Get   ( void );
		int		Val   ( void );
		float	ValF  ( void );
		cStr	Upper ( void );
		cStr	Lower ( void );

		cStr	Left  ( int iCount );
		cStr	Mid   ( int iPoint );
		cStr	Right ( int iCount );
		
	protected:
		char*	m_pString;
		char*	m_szTemp;
		int		m_size;
};

#endif

#ifndef __DBP_LIKE_ARRAYS__

#define __DBP_LIKE_ARRAYS__

#define Dim( name , size ) { noDeleteCSTR = true; name.resize ( size+1 ); noDeleteCSTR = false; }
#define Redim Dim

#define Dim2( name , a , b ) { noDeleteCSTR = true; name.resize(a+1); for(int i = 0 ; i < a+1 ; ++i) { name[i].resize ( b+1 ); } noDeleteCSTR = false; }
#define Redim2 Dim2

#define Dim3( name , a , b , c ) { noDeleteCSTR = true; name.resize(a+1); for(int i = 0 ; i < a+1 ; ++i) { name[i].resize ( b+1 ); } for(int i = 0 ; i < a+1 ; ++i) { for(int j = 0 ; j < b+1 ; ++j) name [ i ] [ j ].resize ( c+100 ); } noDeleteCSTR = false; }
#define Redim3 Dim3

#define Undim( name ) { name.resize ( 0 ); name.shrink_to_fit(); }
#define UnDim Undim

#define Undim2( name ) { for ( int i = 0 ; i < (int)name.size() ; i++ ) { name[i].resize ( 0 ); name[i].shrink_to_fit(); } name.resize ( 0 ); name.shrink_to_fit(); }
#define UnDim2 Undim2

#define Undim3( name ) { for(int i = 0 ; i < (int)name.size() ; ++i) { for(int j = 0 ; j < (int)name[i].size ; ++j) {  name[i][j].resize ( 0 ); name[i][j].shrink_to_fit(); } } for ( int i = 0 ; i < name.size() ; i++ ) { name[i].resize ( 0 ); name[i].shrink_to_fit(); } name.resize ( 0 ); name.shrink_to_fit(); }
#define UnDim3 Undim3

// taken the place of dbpro command array count
#define ArrayCount( name ) (int)name.size()-1

//int ArrayCount ( std::vector<cstr> array );
int ArrayCount2 ( std::vector<std::vector<cstr>> array );

cstr ArrayAt ( std::vector<std::vector<cstr>>& array, int l );

void LoadArray ( char* fileName , std::vector<cstr>& array );
void SaveArray ( char* fileName , std::vector<cstr> array );

void LoadArray ( char* fileName , std::vector<std::vector<cstr>>& array );
void SaveArray ( char* fileName , std::vector<std::vector<cstr>> array );

float atan2deg( float fA , float fB );
void replaceAll(std::string& str, const std::string& from, const std::string& to);
std::string soundexall(std::string all);

#endif
