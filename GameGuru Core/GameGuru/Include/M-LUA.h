//----------------------------------------------------
//--- GAMEGURU - M-LUA
//----------------------------------------------------

#include "cstr.h"

void lua_init ( void );
void lua_loadscriptin ( void );
void lua_scanandloadactivescripts ( void );
void lua_free ( void );
void lua_ensureentityglobalarrayisinitialised ( void );
void lua_initscript ( void );
void lua_launchallinitscripts ( void );
void lua_execute_properties_variable(char *string);
void lua_quitting();
void lua_loop_begin ( void );
void lua_loop_finish ( void );
void lua_loop ( void );
