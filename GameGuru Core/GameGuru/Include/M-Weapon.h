//----------------------------------------------------
//--- GAMEGURU - M-Weapon
//----------------------------------------------------

#include "cstr.h"

void weapon_preloadfiles ( void );
void weapon_getfreeobject ( void );
void weapon_getfreeimage ( void );
void weapon_getfreesound ( void );
void weapon_loadobject ( void );
void weapon_loadtexture ( void );
void weapon_loadtextfile ( void );
void weapon_readfield ( void );
void weapon_init ( void );
void weapon_free ( void );
void weapon_loop ( void );
void weapon_load ( void );
void weapon_setstate ( void );
void weapon_addanimation ( void );
void weapon_processanimation ( void );
void weapon_projectile_init ( void );
void weapon_getprojectileid ( void );
void weapon_projectile_free ( void );
void weapon_projectile_loop ( void );
void weapon_projectile_load ( void );
void weapon_projectile_setup ( int* piSndForBaseSound, int* piSndForBaseDestroy );
void weapon_projectile_make ( bool bUsingVRForAngle );
void weapon_add_projectile_particles ( void );
void weapon_projectile_reset ( void );
void weapon_mp_projectile_reset ( void );
void weapon_projectile_setDistFromPlayer ( void );
void weapon_projectile_destroy ( void );
void weapon_projectileresult_make ( void );
void weapon_loadsound ( void );
void weapon_LUA_addWeapon ( void );
void weapon_LUA_addProjectileBase ( void );
void weapon_LUA_makeProjectile ( void );
void weapon_LUA_firePlayerWeapon ( void );
void weapon_LUA_reloadPlayerWeapon ( void );
void weapon_LUA_changePlayerWeapon ( void );
void weapon_LUA_changePlayerWeaponMode ( void );
void weapon_LUA_setWeaponROFsingle ( void );
void weapon_LUA_setWeaponROFauto ( void );
void weapon_LUA_setWeaponAmmo ( void );
void weapon_LUA_setWeaponDamageModifier ( void );
void weapon_LUA_setWeaponRangeModifier ( void );
