//----------------------------------------------------
//--- GAMEGURU - M-EBE
//----------------------------------------------------

#include "cstr.h"

void ebe_init ( int BuildObj );
void ebe_free(void);
void ebe_init_newbuild ( int iBuildObj, int entid );
void ebe_updateparent ( int entityelementindex );
void ebe_freecubedata ( int entitybankindex );
void ebe_refreshmesh ( int iBuildObj, int x1, int y1, int z1, int x2, int y2, int z2 );
void ebe_loadpattern ( LPSTR pEBEFilename );
void ebe_updatepatternwithrotation ( void );
void ebe_loop ( void );
void imgui_ebe_loop(void);
void ebe_snapshottobuffer ( void );
void ebe_undo ( void );
void ebe_redo ( void );
void ebe_physics_setupebestructure ( int tphyobj, int entityelementindex );
void ebe_optimize_e ( void );
void ebe_optimize_object ( int iObj, int iEntID );
void ebe_reset ( void );
void ebe_hardreset ( void );
void ebe_hide ( void );
void ebe_show ( void );
int  ebe_createnewentityID ( int e, int newsiteentitybankindex );
void ebe_newsite ( int iEntityIndex );
int  ebe_save ( int iEntityIndex );
void ebe_save_ebefile ( cStr tSaveFile, int iEntID );
void ebe_load_ebefile ( cStr tLoadFile, int iEntID );
int ebe_loadcustomtexture ( int iEntityProfileIndex, int iWhichTextureOver );
void ebe_finishsite ( void );
void ebe_packsite ( DWORD* pdwRLEPos, DWORD** ppRLEData );
void ebe_unpacksite ( DWORD dwRLESize, DWORD* pRLEData );
void ebe_loadtxp ( LPSTR pTXPFilename );
cstr ebe_constructlongTXPname ( LPSTR pExt );
void ebe_savetxp ( LPSTR pTXPFilename );
void ebe_restoreebedefaulttextures(void);
int ebe_createnewstructuretexture(LPSTR pDestTerrainTextureFile, int iWhichTextureOver, LPSTR pTexFileToLoad, int iSeamlessMode, int iCompressIt);
