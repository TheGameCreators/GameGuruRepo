#ifndef _DBPROGLOBAL_H_
#define _DBPROGLOBAL_H_

#pragma once

#undef SAFE_DELETE
#undef SAFE_RELEASE
#undef SAFE_DELETE_ARRAY
#undef SAFE_MEMORY
#undef SAFE_CLOSE
#undef SAFE_FREE

#define SAFE_DELETE( p )		delete( p ), ( p ) = nullptr
#define SAFE_RELEASE( p )		{ if ( p ) { ( p )->Release ( ); ( p ) = NULL; } }
#define SAFE_DELETE_ARRAY( p )	delete[]( p ), ( p ) = nullptr
#define SAFE_MEMORY( p )		{ if ( p == NULL ) return false; }
#define SAFE_CLOSE(x)			if(x) { CloseHandle(x); x=NULL; }
#define SAFE_FREE(x)			if(x) { GlobalFree(x); x=NULL; }

#ifndef MAX_STRING
# define MAX_STRING				256 
#endif

#undef DARKSDK 
#define DARKSDK
#define DB_EXPORT 
#define DBPRO_GLOBAL 	
#define DARKSDK_DLL 

#endif
