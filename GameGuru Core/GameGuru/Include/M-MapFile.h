//----------------------------------------------------
//--- GAMEGURU - M-MapFile
//----------------------------------------------------

#include "cstr.h"

void mapfile_emptyebesfromtestmapfolder ( bool bIgnoreValidTextureFiles );
void mapfile_emptyterrainfilesfromtestmapfolder(void);
void mapfile_saveproject_fpm ( void );
void mapfile_loadproject_fpm ( void );
void mapfile_newmap ( void );
void mapfile_loadmap ( void );
void mapfile_savemap ( void );
void mapfile_loadplayerconfig ( void );
void mapfile_saveplayerconfig ( void );
void mapfile_savestandalone_start ( void );
int mapfile_savestandalone_continue ( void );
float mapfile_savestandalone_getprogress ( void );
void mapfile_savestandalone_finish ( void );
void mapfile_savestandalone_restoreandclose ( void );
//void mapfile_savestandalone ( void );
void scanscriptfileandaddtocollection ( char* tfile_s );
bool addtocollection ( char* file_s );
void removefromcollection ( char* file_s );
void removeanymatchingfromcollection ( char* folderorfile_s );
void addfoldertocollection ( char* path_s );
void addallinfoldertocollection ( cstr subThisFolder_s, cstr subFolder_s );
void createallfoldersincollection ( void );
void findalltexturesinmodelfile ( char* file_s, char* folder_s, char* texpath_s );

void CreateItineraryFile ( void );
void scanallfolder ( cstr subThisFolder_s, cstr subFolder_s );
bool IsFileAStockAsset ( LPSTR pCheckThisFile );
void ScanLevelForCustomContent ( LPSTR pFPMBeingSaved );

