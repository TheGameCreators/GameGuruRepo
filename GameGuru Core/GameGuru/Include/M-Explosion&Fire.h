//----------------------------------------------------
//--- GAMEGURU - M-Explosion&Fire
//----------------------------------------------------

#include "cstr.h"

void explosion_init ( void );
void explosion_cleanup ( void );
void explosion_free ( void );
void draw_debris ( void );
void draw_particles ( void );
int Create_Emitter ( int x, int y, int z, int etype, int part, int textureid, int delay, int scale, int damage, int sound, int volume, int loopsound );
void Set_Object_Frame ( int tempobj, int currentframe, int height_f, int width_f );
void Set_Object_Frame_Update ( int tempobj, int currentframe, int height_f, int width_f );
void make_debris ( void );
void make_particles ( void );
int find_free_debris ( void );
int find_free_particle ( int emitter, int start, int endpart );
int find_free_emitter ( void );
void make_large_fire ( int x, int y, int z );
void make_medium_fire ( int x, int y, int z );
void make_small_fire ( int x, int y, int z );
void explosion_rocket ( int x, int y, int z );
void explosion_grenade ( int x, int y, int z );
void explosion_fireball ( int x, int y, int z );
void reset_current_particle ( int emitter, int use );
