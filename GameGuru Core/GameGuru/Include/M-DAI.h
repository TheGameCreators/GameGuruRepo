//----------------------------------------------------
//--- GAMEGURU - M-DAI
//----------------------------------------------------

#include "cstr.h"

void darkai_init ( void );
void darkai_resetsmoothanims ( void );
void darkai_free ( void );
void darkai_preparedata ( void );
void darkai_completeobstacles ( void );
void darkai_invalidateobstacles ( void );
void darkai_saveobstacles ( void );
void darkai_loadobstacles ( void );
int darkai_finddoorcontainer ( int iObj );
void darkai_adddoor ( void );
void darkai_removedoor ( void );
void darkai_createinternaldebugvisuals ( void );
void darkai_destroyinternaldebugvisuals ( void );
void darkai_updatedebugobjects ( void );
void darkai_obstacles_terrain ( void );
void darkai_obstacles_terrain_refresh ( void );
void darkai_setup_characters ( void );
void darkai_destroy_all_characterdata ( void );
void darkai_release_characters ( void );
void darkai_setup_tree ( void );
void darkai_setup_entity ( void );
void darkai_addobstoallneededcontainers ( int iType, int iObj, int iFullHeight, float fMinHeight, float fSliceHeight, float fSliceMinSize );
void darkai_staggerAIprocessing ( void );
void darkai_setupcharacter ( void );
void darkai_staywithzone ( int iAIObj, float fLastX, float fLastZ, float* pX, float* pZ );
void darkai_makesound ( void );
void darkai_makeexplosionsound ( void );
void darkai_shootplayer ( void );
void darkai_shooteffect ( void );
void darkai_killai ( void );
void darkai_shootcharacter ( void );
void darkai_calcplrvisible ( void );
void darkai_loop ( void );
void darkai_update ( void );
void darkai_character_remove_charpart ( void );
void darkai_character_remove ( void );
void darkai_character_loop ( void );
void darkai_finalsettingofcharacterobjects ( void );
void darkai_character_freezeall ( void );
void darkai_assignanimtofield ( void );
void char_init ( void );
void char_createseqdata ( void );
void char_getcharseqcsifromplaycsi ( void );
void char_loop ( void );
void darkai_ischaracterhit ( void );
void smoothanimtriggerrev ( int obj, float  st, float  fn, int  speedoftransition, int  rev, int  playflag );
void smoothanimtrigger ( int obj, float  st, float  fn, int  speedoftransition );
void smoothanimupdate ( int obj );
