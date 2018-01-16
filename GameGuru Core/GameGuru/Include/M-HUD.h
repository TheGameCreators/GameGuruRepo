//----------------------------------------------------
//--- GAMEGURU - M-HUD
//----------------------------------------------------

#include "cstr.h"

void hud_init ( void );
void hud_scanforhudlayers ( void );
void hud_free ( void );
void hud_updatehudlayerobjects ( void );
void blood_damage_init ( void );
void placeblood ( int damage, int x, int y, int z, int howfar );
int controlblood ( void );
void resetblood ( void );
void new_damage_marker ( int entity, int x, int z, int y, int tempdamage );
int controldamagemarker ( void );
void resetdamagemarker ( void );
