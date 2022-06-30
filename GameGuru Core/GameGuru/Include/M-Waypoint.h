//----------------------------------------------------
//--- GAMEGURU - M-Waypoint
//----------------------------------------------------

#include "cstr.h"

void waypoint_savedata ( void );
void waypoint_loaddata ( void );
void waypoint_recreateobjs ( void );
void waypoint_createnew ( void );
void waypoint_createallobjs ( void );
void waypoint_deleteall ( void );
void waypoint_hideall ( void );
void waypoint_hideallpaths ( void );
void waypoint_showall ( void );
void waypoint_showallpaths ( void );
void waypoint_mousemanage ( void );
void waypoint_delete ( void );
void waypoint_findcenter ( void );
void waypoint_movetogrideleprof ( void );
void waypoint_movetothiscoordinate ( void );
void waypoint_moveentitycenter ( void );
void createwaypointobj ( int obj, int waypointindex );

void makepolymesh ( int obj, int polycount );
void addverttomesh ( int obj, int vertindex, unsigned int color, float fX, float fY, float fZ);
void addpolytomesh ( int obj, int polyindex, int diffuse1, int diffuse2, int diffuse3, int x1_f, int y1_f, int z1_f, int x2_f, int y2_f, int z2_f, int x3_f, int y3_f, int z3_f );
void finalisepolymesh ( int obj );

void waypoint_reset ( void );
void waypoint_restore ( void );
void waypoint_hide ( void );
void waypoint_ispointinzone ( void );
int waypoint_getmax ( void );
int waypoint_ispointinzoneex ( int iWPIndex, float fX, float fY, float fZ, int iStyleFloorZoneOnly );
void waypoint_ispointinzone ( void );
int ispointinzone ( int waypointindex, int tpointx_f, int tpointz_f );

void waypoint_imgui_loop(void);
void waypoint_drawmode_loop(void);
