//----------------------------------------------------
//--- GAMEGURU - Common-Images
//----------------------------------------------------

#include "cstr.h"

void deleteallinternalimages ( void );
void refreshallinternalimages ( void );
void refreshinternalimage ( int t );
void removeinternalimage ( int img );
void loadinternalimageexcompressquality ( char* tfile_s, int imgid, int compressmode, int quality, int iLoaderMode );
void loadinternalimageexcompress ( char* tfile_s, int imgid, int compressmode );
void loadinternalimageex ( char* tfile_s, int imgid );
int loadinternalimagecompressquality ( char* tfile_s, int compressmode, int quality );
int loadinternalimagecompress ( char* tfile_s, int compressmode );
int loadinternalimage ( char* tfile_s );
int loadinternalimagequality ( char* tfile_s );
int loadinternaltexturemode ( char* tfile_s, int iImageLoaderMode );
int addinternaltexture(char* tfile_s);
int findinternaltexture(char* tfile_s);
int loadinternaltexture ( char* tfile_s );
void removeinternaltexture ( int teximg );
void deleteinternaltexture(char* tfile_s);
int loadinternaltextureex ( char* tfile_s, int compressmode, int quality );
int loadinternaleffectunique ( char* tfile_s, int makeunique );
void deleteinternaleffect ( int effectid );
void filleffectparamarray ( int effectid );
int loadinternaleffect ( char* tfile_s );

void cubemap_generateimage ( int iImageID, float fX, float fY, float fZ, LPSTR pCacheCubeMapFile );
void cubemap_generateglobalenvmap ( void );
