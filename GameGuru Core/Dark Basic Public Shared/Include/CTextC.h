#ifndef _CTEXT_H_
#define _CTEXT_H_

void InitCTextC(void);
char* GetReturnStringFromTEXTWorkString(char*);

int	  Asc					( char* dwSrcStr);
char* Bin					( char* pDestStr, int iValue );
char* Chr					( int iValue );
char* Hex					( char* pDestStr, int iValue );
char* Left					( char* szText, int iValue );
int	  Len					( char* dwSrcStr );
char* Lower					( char* szText );
char* Mid					( char* szText, int iValue );
char* Right					( char* szText, int iValue );
char* Str					( float fValue );
char* StrEx					( float fValue, int iDecPlaces );
char* Str					( int iValue );
char* Upper					( char* szText );
float ValF					( char* dwSrcStr );
char* StrDouble				( char* pDestStr, double dValue );
char* StrDoubleInt			( char* pDestStr, long long lValue );
long long ValR				( char* dwSrcStr );
void  Reverse				( char* dwA );
char* Spaces				( int iSpaces );

int   FindFirstChar			( char* dwSource, char* dwChar );
int   FindLastChar			( char* dwSource, char* dwChar );
int   FindSubString			( char* dwSource, char* dwString );
char* FirstToken			( char* dwSource, char* dwDelim );
char* NextToken				( char* dwDelim );

#endif _CTEXT_H_
