//----------------------------------------------------
//--- GAMEGURU - G-Gun
//----------------------------------------------------

#include "cstr.h"

void gun_restart ( void );
void gun_resetactivateguns ( void );
void gun_activategunsfromentities ( void );
void gun_decaldetails ( void );
void gun_loadonlypresent ( void );
void gun_resetgunsettings ( void );
void gun_manager ( void );
void gun_change ( void );
void gun_update_hud ( void );
void gun_update_hud_visibility ( void );
void gun_update_overlay ( void );
void gun_picksndvariant ( void );
void gun_getstartandfinish ( void );
void gun_getzoomstartandfinish ( void );
void gun_control ( void );
void gunmode121_cancel ( void );
void gun_actualreloadcode ( void );
void gun_flash ( void );
void gun_brass ( void );
void gun_brass_indi ( void );
void gun_smoke ( void );
void gun_updatebulletvisibility ( void );
void gun_shoot ( void );
void gun_shoot_oneray ( void );
void gun_soundcontrol ( void );
void gun_create_hud ( void );
void gun_setup ( void );
void gun_gatherslotorder ( void );
void gun_selectandorload ( void );
void gun_load ( void );
void gun_updategunshaders ( void );
void gun_freeafterlevel ( void );
void gun_freeguns ( void );
void gun_free ( void );
void gun_releaseresources ( void );
void gun_tagmpgunstolist ( void );
void gun_removempgunsfromlist ( void );
void gun_playerdead ( void );
int loadgun ( int gunid, char* tfile_s );
int createsecondgun ( void );
int loadbrass ( char* tfile_s );
int loadmuzzle ( char* tfile_s );
int loadsmoke ( char* tfile_s );
