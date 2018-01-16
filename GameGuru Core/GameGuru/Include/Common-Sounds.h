//----------------------------------------------------
//--- GAMEGURU - Common-Sounds
//----------------------------------------------------

#include "cstr.h"

int soundfileexist ( char* tfile_s );
float soundtruevolume ( int tvolume_f );
int loadinternalsoundcorecloneflag ( char* tfile_s, int mode, int clonesoundindex );
int loadinternalsoundcore ( char* tfile_s, int mode );
int loadinternalsound ( char* tfile_s );
int changeplrforsound ( int tplrid, int tsnd );
void deleteinternalsound ( int soundid );
void playinternalsound ( int id );
void playinternalsoundinterupt ( int id );
void playinternal3dsound ( int id, int x_f, int y_f, int z_f );
void playinternalBC3dsound ( int id, int x_f, int y_f, int z_f, int strength_f );
void loopinternal3dsound ( int id, int x_f, int y_f, int z_f );
void posinternal3dsound ( int id, int x_f, int y_f, int z_f );
void playinternal3dsoundfactor ( int id, int x_f, int y_f, int z_f, int factor_f );

void music_init ( void );
void music_loop ( void );
void music_fadeAllMusicToNewVolume ( void );
void music_resettrack ( int m );
void music_stoptrack ( int m, int tFadeOut );
void music_stopall ( int tFadeOut );
void music_play ( int m, int tFadeTime );
int music_getplaybackvolume ( int tVol );
void music_resetall ( void );
void music_playdefault ( void );

void sound_footfallsound ( int footfalltype, float fX, float fY, float fZ, int* lastfootfallsound );