//----------------------------------------------------
//--- GAMEGURU - M-Importer
//----------------------------------------------------

#include "cstr.h"

void importer_init ( void );
void importer_free ( void );
void importer_changeshader ( LPSTR pNewShaderFilename );
void importer_loadmodel ( void );
void importer_loop ( void );
void importer_update_selection_markers ( void );
void importer_extract_collision ( void );
void importer_ShowCollisionOnly ( void );
void importer_ShowCollisionOnlyOff ( void );
void importer_snapLeft ( void );
void importer_snapUp ( void );
void importer_snapforward ( void );
void importer_check_for_physics_changes ( void );
void importer_update_textures ( void );
void importer_load_textures_finish ( int tCount, bool bCubeMapOnly );
void importer_load_textures ( void );
void importer_load_fpe ( void );
void importer_apply_fpe ( void );
void importer_save_fpe ( void );
void importer_handleScale ( void );
void importer_draw ( void );
void importer_quit ( void );
void importer_save_entity ( void );
void importer_tabs_update ( void );
void importer_tabs_draw ( void );
void importer_add_collision_box ( void );
void importer_dupe_collision_box ( void );
void importer_add_collision_box_loaded ( void );
void importer_delete_collision_box ( void );
void importer_checkForShaderFiles ( void );
void importer_checkForScriptFiles ( void );
void importer_help ( void );
void importer_screenSwitch ( void );
void importer_debug ( char* debugText_s );
void importer_debug_num ( char* title_s, int  debugNumber );
int findFreeDll ( void );
int findFreeMemblock ( void );
char* openFileBox ( char* filter, char* initdir, char* dtitle, char* defext, unsigned char open );
int _get_str_ptr ( char* pstr );
char* _get_str ( DWORD strptr, int strsize );
char* importerPadString ( char* tString );
int findFreeObject ( void );
float GetDistance ( float x1, float  y1, float  z1, float  x2, float  y2, float  z2 );
int importer_check_if_protected ( char* timporterfilecheck_s );
void importer_sort_names ( void );
void importer_find_object_name_from_fpe ( void );
void importer_hide_mouse ( void );
void importer_show_mouse ( void );
void importer_fade_out ( void );
void importer_fade_in ( void );
void importer_check_script_token_exists ( void );
void importer_update_scale ( void );
