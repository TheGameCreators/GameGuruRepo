//----------------------------------------------------
//--- GAMEGURU - M-Ragdoll
//----------------------------------------------------

#include "cstr.h"

void ragdoll_init ( void );
void ragdoll_free ( void );
void ragdoll_setcollisionmask ( int iMode );
void ragdoll_create ( void );
void ragdoll_destroy ( void );
int newvector3 ( int x_f, int y_f, int z_f );
int newvector2 ( int x_f, int y_f );
int vector3exist ( int vec );
int deletevector3 ( int vec );
int getlimbbyname ( int objid, char* limbname_s );
