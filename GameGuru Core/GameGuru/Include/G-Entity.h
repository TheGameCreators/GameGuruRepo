//----------------------------------------------------
//--- GAMEGURU - G-Entity
//----------------------------------------------------

#include "cstr.h"

void entity_init ( void );
void entity_bringnewentitiestolife ( void );
void entity_initafterphysics ( void );
void entity_configueelementforuse ( void );
void entity_freeragdoll ( void );
void entity_free ( void );
void entity_delete ( void );
void entity_pauseanimations ( void );
void entity_resumeanimations ( void );
void entity_loop ( void );
void entity_loopanim ( void );
void entity_controlrecalcdist ( void );
void entity_getmaxfreezedistance ( void );
void entity_updatepos ( void );
void entity_determinedamagemultiplier ( void );
void entity_determinegunforce ( void );
void entity_find_charanimindex_fromttte ( void );
void entity_applydamage ( void );
void entity_gettruecamera ( void );
void entity_hasbulletrayhit ( void );
void entity_hitentity ( int e, int obj );
void entity_triggerdecalatimpact ( float fX, float fY, float fZ );
void entity_createattachment ( void );
void entity_freeattachment ( void );
void entity_controlattachments ( void );
void entity_monitorattachments ( void );
void entity_converttoclone ( void );
void entity_converttoclonetransparent ( void );
void entity_converttoinstance ( void );
void entity_createobj ( void );
void entity_prepareobj ( void );
void entity_calculateentityLODdistances ( int tentid, int tobj, int iModifier );
void entity_setupcharobjsettings ( void );
void entity_resettodefaultanimation ( void );
void entity_positionandscale ( void );
void entity_updateentityobj ( void );
