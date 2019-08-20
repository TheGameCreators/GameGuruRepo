//----------------------------------------------------
//--- GAMEGURU - M-Particles
//----------------------------------------------------

#include "cstr.h"

void ravey_particles_init                ( void );
void ravey_particles_load_images         ( void );
int  ravey_particles_load_image          ( LPSTR szFilename, int iID );
int  ravey_particles_load_effect         ( LPSTR szFilename, int iID );
void ravey_particles_update              ( void );
void ravey_particles_update_emitters     ( void );
bool ravey_particles_update_particles    ( void );
void ravey_particles_generate_particle   ( int iID, float fPosX, float fPosY, float fPosZ );
void ravey_particles_free                ( void );
void ravey_particles_add_emitter         ( void );
void ravey_particles_delete_emitter      ( void );
void ravey_particles_delete_all_emitters ( void );
void ravey_particles_get_free_emitter    ( void );
void ravey_particles_set_frames          ( int iID, int iAnimSpeed, int iStartFrame, int iEndFrame );
void ravey_particles_set_no_wind         ( int iID );
void ravey_particles_set_speed           ( int iID, float fMinX, float fMinY, float fMinZ, float fMaxX, float fMaxY, float fMaxZ );
void ravey_particles_set_gravity         ( int iID, float fStartG, float fEndG );
void ravey_particles_set_offset          ( int iID, float fMinX, float fMinY, float fMinZ, float fMaxX, float fMaxY, float fMaxZ );
void ravey_particles_set_angle           ( int iID, float fAngX, float fAngY, float fAngZ );
void ravey_particles_set_rotate          ( int iID, float fMinXRot, float fMaxXRot, float fMinYRot, float fMaxYRot, float fMinZRot, float fMaxZRot, int bStartsOffRandom );
void ravey_particles_set_scale           ( int iID, float fStartMin, float fStartMax, float fEndMin, float fEndMax );
void ravey_particles_set_alpha           ( int iID, float fStartMin, float fStartMax, float fEndMin, float fEndMax );
void ravey_particles_set_life            ( int iID, int iSpawnRate, float iLifeMin, float iLifeMax, int iMaxParticles, int iOnDeathAction, int iMaxPerFrame );
void ravey_particles_set_wind_vector     ( float fwindX, float fwindZ );