/*

// Add an occluder
CPU3D Add Occluder objectID

// Add an occludee
CPU3D Add Occludee objectID

// Change camera used
CPU3D Set Camera Index cameraID

// Clear Occluder and Ocludee lists
CPU3D Clear

// Do the Occluding
CPU3D Occlude

*/

#ifndef __CPU3DCULLING__
#define __CPU3DCULLING__

// Internal functions
void OccluderConstructor ( void );
void OccluderDestructor ( void );
DWORD OccludergetColor ( int r , int g , int b );
void OccluderPix( int x, int y, int col );
double OccluderLog2( double n );

// Actual commands
void CPUShiftXZ ( float x, float z );
void CPU3DAddOccluder ( int id );
void CPU3DAddOccludee ( int id , bool isCharacter );
void CPU3DSetCameraIndex ( int id );
void CPU3DClear();
void CPU3DSetPolyCount( int amount );
void CPU3DShow( int show );
void CPU3DOcclude ();
void CPU3DDoOcclude ();
void CPU3DSetCameraFar ( float f );

extern HANDLE   g_hOccluderBegin;
extern HANDLE   g_hOccluderEnd;

#endif
