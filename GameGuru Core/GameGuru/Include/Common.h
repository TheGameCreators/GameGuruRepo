//----------------------------------------------------
//--- GAMEGURU - Common
//----------------------------------------------------

#include "cstr.h"

void common_init_globals ( void );
void common_init ( void );
int common_isserialcodevalid ( LPSTR pCode );
void common_writeserialcode ( LPSTR pCode );
void FPSC_SetDefaults ( void );
void FPSC_LoadSETUPINI ( void );
void FPSC_Full_Data_Init ( void );
void FPSC_Setup ( void );
void common_loadfonts ( void );
void common_loadcommonassets ( int iShowScreenPrompts );
void common_hide_mouse ( void );
void common_show_mouse ( void );
void common_vectorsinit ( void );
void common_wipeeffectifnotexist ( void );
void common_makeeffecttextureset ( void );
void common_justbeforeend ( void );
void version_commandlineprompt ( void );
void version_endofinit ( void );
void version_splashtext_statusupdate ( void );
void version_splashtext ( void );
void version_onscreenlogos ( void );
void version_permittestgame ( void );
void version_resourcewarning ( void );
void version_universe_saveELEandLGT ( void );
void version_universe_construct ( void );
void version_buildgame ( void );
void version_main_game_buildexe ( void );
void common_refreshDisplaySize ( void );
void popup_text_close ( void );
void loadresource ( void );
void saveresource ( void );
int openresource ( char* name_s );
int closeresource ( void );
void resetfilecollection ( void );
void addfiletocollection ( char* filename_s );
void addallfilestocollection ( char* folder_s );
void hide3d ( void );
void show3d ( void );
void debugfilename ( char* tfile_s, char* desc_s );
void debugstring ( char* tfile_s, char* desc_s );
void debugseevar ( int var );
void debugpeek ( char* desc_s );
void debugviewactivate ( int mode );
void debugviewprogressmax ( int progressmax );
void debugviewtext ( int progress, char* gamedebugviewtext_s );
void debugviewupdate ( int doisync );
void printvalue ( int x, int y, int value );
int loadinternalmesh ( char* tfile_s );
//cstr findmaterialtexturesinmodelfile ( char* file_s, char* texpath_s );
char* browseropen_s ( int browsemode );
void loadscreenpromptassets ( void );
void printscreenprompt ( char* screenprompt_s );
int mod ( int num, int modulus );
void GGBoxGradient ( int iLeft, int iTop, int iRight, int iBottom, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4 );

void InkEx ( int r, int g, int b );
void BoxEx ( int x1, int y1, int x2, int y2 );
void LineEx ( int x1, int y1, int x2, int y2 );
