//----------------------------------------------------
//--- GAMEGURU - M-Titles
//----------------------------------------------------

#include "cstr.h"

void titles_init ( void );
void titles_getstyle ( void );
void titles_save ( void );
void titles_load ( void );
void titles_immediateupdatesound ( void );
void titles_immediateupdategraphics ( void );
void titles_titlepage ( void );
void titles_aboutpage ( void );
void titles_loadingpage ( void );
void titles_loadingpageinit ( void );
void titles_loadingpageupdate ( void );
void titles_loadingpagefree ( void );
void titles_completepage ( void );
void titles_gamewonpage ( void );
void titles_gamelostpage ( void );
void titles_steampage ( void );
void titles_steamCreateLobby ( void );
void titles_steamSearchLobbies ( void );
void titles_steamInLobbyGuest ( void );
void titles_optionspage ( void );
void titles_graphicssettings ( void );
void titles_soundlevels ( void );
void titles_gamewonpage ( void );
void titles_steamchoosetypeoflevel ( void );
void titles_steamchoosefpmtouse ( void );
void titles_steamdoyouwanttocreateorupdateaworkshopitem ( void );
void titles_steamcreatingworkshopitem ( void );
void titles_steamdoyouwanttosubscribetoworkshopitem ( void );
void titles_subscribetoworkshopitemwaitingforresult ( void );
void titles_steamdTellingToRestart ( void );
void titles_base ( void );
void loadingpageprogress ( int delta );
void handletitlesmusic ( void );
void fadetitlesmusic ( int tFadeVol );

void titleslua_init ( void );
void titleslua_main ( LPSTR pPageName );
void titleslua_free ( void );