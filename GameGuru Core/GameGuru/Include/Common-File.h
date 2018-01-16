//----------------------------------------------------
//--- GAMEGURU - Common-File
//----------------------------------------------------

#include "cstr.h"

void file_createmydocsfolder ( void );
int getdatevaluefromdatestring ( char* dayofyear_s );
int isbinaryfileolderthantxtfile ( char* binaryfilefull_s, char* textfilefull_s );
int isbinaryfileolderthantxtfileforseg ( char* binaryfilefull_s, char* textfilefull_s );
int isbinaryfileolderthanglobalversion ( char* binaryfilefull_s );
int versionnumberislarger ( int tNum1, int tNum2 );
void deleteOutOfDateDBO ( char* tfile_s );
