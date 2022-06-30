//----------------------------------------------------
//--- GAMEGURU - M-Lightmapping
//----------------------------------------------------

#include "cstr.h"

void lm_init ( void );
void lm_removeold ( void );
void lm_deletelmobjectsfirst ( void );
void lm_createglassterrainobjects ( void );
void lm_flashprompt ( void );
void lm_onscreenprompt ( void );
void lm_process ( void );
void lm_zbias ( void );
void lm_focuscameraonoverlordxz ( void );
void lm_deleteall ( void );
void lm_restoreall ( void );
void lm_show_lmos ( void );
void lm_hide_lmos ( void );
void lm_showall ( void );
void lm_savescene ( void );
void lm_emptylightmapandttsfilesfolder ( void );
void lm_loadscene ( void );
void lm_preplmobj ( void );
void lm_handleshaders ( void );
int findlightmaptexturefilenameindex ( char* file_s );
