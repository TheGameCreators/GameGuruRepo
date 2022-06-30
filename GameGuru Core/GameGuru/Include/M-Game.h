//----------------------------------------------------
//--- GAMEGURU - M-Game
//----------------------------------------------------

#include "cstr.h"

void gameexecutable_init(void);
void gameexecutable_loop(void);
void gameexecutable_finish(void);

void game_masterroot_gameloop_initcode(int iUseVRTest);
bool game_masterroot_gameloop_loopcode(int iUseVRTest);
void game_masterroot_gameloop_afterloopcode(int iUseVRTest);
bool game_masterroot_levelloop_initcode(int iUseVRTest);
void game_masterroot_levelloop_initcode_aftertitleloop(void);
bool game_masterroot_levelloop_loopcode(int iUseVRTest);
void game_masterroot_levelloop_afterloopcode(int iUseVRTest);
void game_masterroot_initcode ( int iUseVRTest );
bool game_masterroot_loopcode ( int iUseVRTest );
void game_masterroot_afterloopcode ( int iUseVRTest );
void game_masterroot ( int iUseVRTest );
void game_setresolution ( void );
void game_postresolutionchange ( void );
void game_oneoff_nongraphics ( void );
void game_loadinentitiesdatainlevel ( void );
void game_loadinleveldata ( void );
void game_preparelevel ( void );
void game_preparelevel_forplayer ( void );
void game_preparelevel_finally ( void );
void game_stopallsounds ( void );
void game_freelevel ( void );
void game_init ( void );
void game_freegame ( void );
void game_hidemouse ( void );
void game_showmouse ( void );
void game_showmouse_restore_mouse(void);
void game_timeelapsed_init ( void );
void game_timeelapsed ( void );
void game_main_snapshotsoundloopcheckpoint(void);
void game_main_snapshotsound ( bool bPauseForGameMenu );
void game_main_snapshotsoundresume ( void );
void game_main_loop ( void );
void game_sync ( void );
void game_main_stop ( void );
void game_jump_to_level_from_lua ( void );
void game_finish_level_from_lua ( void );
void game_end_of_level_check ( void );
void game_setup_character_shader_entities ( bool bMode );
void game_check_character_shader_entities ( void );
