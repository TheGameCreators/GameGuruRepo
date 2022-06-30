// 4D homogenous vector, point with w ////////////////////
#ifndef __CPU3D_H__
#define __CPU3D_H__

#include "preprocessor-flags.h"
#include "malloc.h"
#include <vector>

//=======================================================================================
//=======================================================================================

#define METHOD_TWO

extern int cpu3dMaxPolys;

// how much will be allocated and be allowed to be set
#define CPU_3D_MAX_POLY_SETTING 60000

// 256 was the original res, now 1024 which is 4x
#define RESOLUTION_MULTI	1

#define HIDE_SMALL_OBJECTS
#define TINY_OBJECT_REJECT_SIZE 4 * RESOLUTION_MULTI
#define TINY_CHARACTER_REJECT_SIZE 2 * RESOLUTION_MULTI

//#define SCREEN_WIDTH		256
//#define SCREEN_HEIGHT     144

#define SCREEN_WIDTH		1024
#define SCREEN_HEIGHT       1024

#define DRAW_ZBUFFER

//=======================================================================================

#define EPSILON_E3 (float)(1E-3) 
#define EPSILON_E5 (float)(1E-5)

// states of polygons and faces
#define POLY4DV2_STATE_NULL               0x0000
#define POLY4DV2_STATE_ACTIVE             0x0001  
#define POLY4DV2_STATE_CLIPPED            0x0002  
#define POLY4DV2_STATE_BACKFACE           0x0004  
#define POLY4DV2_STATE_LIT                0x0008
#define POLY4DV2_ATTR_2SIDED              0x0001

// general clipping flags for polygons
#define CLIP_POLY_X_PLANE           0x0001 // cull on the x clipping planes
#define CLIP_POLY_Y_PLANE           0x0002 // cull on the y clipping planes
#define CLIP_POLY_Z_PLANE           0x0004 // cull on the z clipping planes
#define CLIP_OBJECT_XYZ_PLANES      (CULL_OBJECT_X_PLANE | CULL_OBJECT_Y_PLANE | CULL_OBJECT_Z_PLANE)

// fixed point mathematics constants
#define FIXP16_SHIFT     16
#define FIXP16_MAG       65536
#define FIXP16_DP_MASK   0x0000ffff
#define FIXP16_WP_MASK   0xffff0000
#define FIXP16_ROUND_UP  0x00008000

// defines for camera rotation sequences
#define CAM_ROT_SEQ_XYZ  0
#define CAM_ROT_SEQ_YXZ  1
#define CAM_ROT_SEQ_XZY  2
#define CAM_ROT_SEQ_YZX  3
#define CAM_ROT_SEQ_ZYX  4
#define CAM_ROT_SEQ_ZXY  5

#define CAM_MODEL_EULER            0x0008

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
typedef unsigned int   QUAD;
typedef unsigned int   UINT;

// pi defines
#define PI         ((float)3.141592654f)
#define PI2        ((float)6.283185307f)
#define PI_DIV_2   ((float)1.570796327f)
#define PI_DIV_4   ((float)0.785398163f) 
#define PI_INV     ((float)0.318309886f) 

#define TRI_TYPE_NONE           0
#define TRI_TYPE_FLAT_TOP       1 
#define TRI_TYPE_FLAT_BOTTOM	2
#define TRI_TYPE_FLAT_MASK      3
#define TRI_TYPE_GENERAL        4
#define INTERP_LHS              0
#define INTERP_RHS              1
#define MAX_VERTICES_PER_POLY   6

typedef struct VECTOR4D_TYP
{
union
    {
    float M[4]; // array indexed storage

    // explicit names
    struct
         {
         float x,y,z,w;
         }; // end struct
    }; // end union

} VECTOR4D, POINT4D, *VECTOR4D_PTR, *POINT4D_PTR;



// 3D vector, point without the w
typedef struct VECTOR3D_TYP
{
union
    {
    float M[3]; // array indexed storage

    // explicit names
    struct
         {
         float x,y,z;
         }; // end struct

    }; // end union

} VECTOR3D, POINT3D, *VECTOR3D_PTR, *POINT3D_PTR;



// 3D plane ///////////////////////////////////////////////////
typedef struct PLANE3D_TYP
{
	POINT3D p0; // point on the plane
	VECTOR3D n; // normal to the plane
} PLANE3D, *PLANE3D_PTR;



// 4x4 matrix /////////////////////////////////////////////
typedef struct MATRIX4X4_TYP
{
union
    {
    float M[4][4]; // array indexed data storage

    // storage in row major form with explicit names
    struct
         {
         float M00, M01, M02, M03;
         float M10, M11, M12, M13;
         float M20, M21, M22, M23;
         float M30, M31, M32, M33;
         }; // end explicit names

    }; // end union

} MATRIX4X4, *MATRIX4X4_PTR;


// camera
typedef struct CAMERA_TYP
{
	int state;      // state of camera
	int attr;       // camera attributes

	POINT4D pos;    // world position of camera used by both camera models

	VECTOR4D dir;   // angles or look at direction of camera for simple 
					// euler camera models, elevation and heading for
					// uvn model

	VECTOR4D u;     // extra vectors to track the camera orientation
	VECTOR4D v;     // for more complex UVN camera model
	VECTOR4D n;        

	VECTOR4D target; // look at target

	float view_dist;

	float fov;      

	// 3d clipping planes
	float near_clip_z;     // near z=constant clipping plane
	float far_clip_z;      // far z=constant clipping plane

	PLANE3D rt_clip_plane;  // the right clipping plane
	PLANE3D lt_clip_plane;  // the left clipping plane
	PLANE3D tp_clip_plane;  // the top clipping plane
	PLANE3D bt_clip_plane;  // the bottom clipping plane                        

	float viewplane_width;    
	float viewplane_height;    
							  

	float viewport_width;     // size of screen/viewport
	float viewport_height;
	float viewport_center_x;  // center of view port
	float viewport_center_y;

	// aspect ratio
	float aspect_ratio;   

	MATRIX4X4 mcam;   // storage for the world to camera transform matrix
	MATRIX4X4 mper;   // storage for the camera to perspective transform matrix
	MATRIX4X4 mscr;   // storage for the perspective to screen transform matrix

} CAMERA, *CAMERA_PTR;


// 2D vector, point without the w ////////////////////////
typedef struct VECTOR2D_TYP
{
union
    {
    float M[2]; // array indexed storage

    // explicit names
    struct
         {
         float x,y;
         }; // end struct

    }; // end union

} VECTOR2D, POINT2D, *VECTOR2D_PTR, *POINT2D_PTR;


// 4D homogeneous vertex with 2 texture coordinates, and vertex normal
// normal can be interpreted as vector or point
typedef struct VERTEX4DTV1_TYP
{
union
    {
    float M[12];            // array indexed storage

    // explicit names
    struct
         {
         float x,y,z,w;     // point
         float nx,ny,nz,nw; // normal (vector or point)
  	     float u0,v0;       // texture coordinates 

         float i;           // final vertex intensity after lighting
         int   attr;        // attributes/ extra texture coordinates
         };                 // end struct
     
     // high level types
     struct 
          {
          POINT4D  v;       // the vertex
          VECTOR4D n;       // the normal
          POINT2D  t;       // texture coordinates
          };

    }; // end union

} VERTEX4DTV1, *VERTEX4DTV1_PTR;




// a self contained polygon used for the render list
typedef struct POLYF4DV2_TYP
{
int      state;           // state information
int      attr;            // physical attributes of polygon

float    nlength; // length of the polygon normal if not normalized (new)
VECTOR4D normal;  // the general polygon normal (new)

VERTEX4DTV1 vertexList[3]; // the vertices after transformation if needed 

POLYF4DV2_TYP *next;   // pointer to next polygon in list??
POLYF4DV2_TYP *prev;   // pointer to previous polygon in list??

} POLYF4DV2, *POLYF4DV2_PTR;



typedef struct RENDERLIST_TYP
{
	int state; // state of renderlist ???
	int attr;  // attributes of renderlist ???

	// the render list is an array of pointers each pointing to 
	// a self contained "renderable" polygon face POLYF4DV2
	POLYF4DV2_PTR poly_ptrs[CPU_3D_MAX_POLY_SETTING];

	// additionally to cut down on allocatation, de-allocation
	// of polygons each frame, here's where the actual polygon
	// faces will be stored
	POLYF4DV2 poly_data[CPU_3D_MAX_POLY_SETTING];

	int num_polys; // number of polys in render list

} RENDERLIST, *RENDERLIST_PTR;


// structure for zbuffer
typedef struct ZBUFFERV1_TYP
{
int attr;       // attributes of zbuffer
UCHAR *zbuffer; // ptr to storage
int width;      // width in zpixels
int height;     // height in zpixels
int sizeq;      // total size in QUADs
                // of zbuffer
} ZBUFFERV1, *ZBUFFERV1_PTR;


//=======================================================================================
//=======================================================================================

void BuildSinCosTables(void);

// some math macros
#define DEG_TO_RAD(ang) ((ang)*PI/180.0)
#define RAD_TO_DEG(rads) ((rads)*180.0/PI)

void CameraInit(CAMERA_PTR cam, int attr, POINT4D_PTR cam_pos, 
                  VECTOR4D_PTR cam_dir, VECTOR4D_PTR cam_target,
                  float near_clip_z, float far_clip_z, float fov, 
                  float viewport_width,  float viewport_height);

inline void Vector3DInitXYZ(VECTOR3D_PTR v, float x, float y, float z) 
{(v)->x = (x); (v)->y = (y); (v)->z = (z);}

inline void Vector4DCopy(VECTOR4D_PTR vdst, VECTOR4D_PTR vsrc) 
{(vdst)->x = (vsrc)->x; (vdst)->y = (vsrc)->y;  
(vdst)->z = (vsrc)->z; (vdst)->w = (vsrc)->w;  }

inline void Vector4DInitXYZ(VECTOR4D_PTR v, float x,float y,float z) 
{(v)->x = (x); (v)->y = (y); (v)->z = (z); (v)->w = 1.0;}

inline void Vector4DZero(VECTOR4D_PTR v) 
{(v)->x = (v)->y = (v)->z = 0.0; (v)->w = 1.0;}


// 4x4 identity matrix
const MATRIX4X4 IdentMatrix4x4 = {1,0,0,0, 
                            0,1,0,0, 
                            0,0,1,0, 
                            0,0,0,1};

//Identity Matrix
#define MatrixIdent4X4(m) {memcpy((void *)(m), (void *)&IdentMatrix4x4, sizeof(MATRIX4X4));}

// bit manipulation macros
#define SetBit(word,bit_flag)   ((word)=((word) | (bit_flag)))
#define ResetBit(word,bit_flag) ((word)=((word) & (~bit_flag)))

// used for swapping algorithm
#define _swap(a,b,t) {t=a; a=b; b=t;}

// floating point comparison
#define FloatCompare(a,b) ( (fabs(a-b) < EPSILON_E3) ? 1 : 0)

inline void Point3DCopy(POINT3D_PTR vdst, POINT3D_PTR vsrc) 
{(vdst)->x = (vsrc)->x; (vdst)->y = (vsrc)->y;  (vdst)->z = (vsrc)->z; }

inline void Vector3DCopy(VECTOR3D_PTR vdst, VECTOR3D_PTR vsrc) 
{(vdst)->x = (vsrc)->x; (vdst)->y = (vsrc)->y;  (vdst)->z = (vsrc)->z; }

inline void Vector3DZero(VECTOR3D_PTR v) 
{(v)->x = (v)->y = (v)->z = 0.0;}

// memory manipulation functions
inline void MemSetWord(void *dest, USHORT data, int count);

inline void MemSetQuad(void *dest, UINT data, int count)
{
// this function fills or sets unsigned 32-bit aligned memory
// count is number of quads

	_asm 
		{ 
		mov edi, dest   ; edi points to destination memory
		mov ecx, count  ; number of 32-bit words to move
		mov eax, data   ; 32-bit data
		rep stosd       ; move data
		} // end asm

} // end MemSetQuad

// 3D parametric line /////////////////////////////////////////
typedef struct PARMLINE3D_TYP
{
POINT3D  p0; // start point of parametric line
POINT3D  p1; // end point of parametric line
VECTOR3D v;  // direction vector of line segment
             // |v|=|p0->p1|
} PARMLINE3D, *PARMLINE3D_PTR;

void Vector3DNormalize(VECTOR3D_PTR va);
void Vector3DNormalize(VECTOR3D_PTR va, VECTOR3D_PTR vn);

// 3d plane functions
void Plane3DInit(PLANE3D_PTR plane, POINT3D_PTR p0, 
                         VECTOR3D_PTR normal, int normalize);
float ComputePointInPlane3D(POINT3D_PTR pt, PLANE3D_PTR plane);
int IntersectParmLine3DPlane3D(PARMLINE3D_PTR pline, PLANE3D_PTR plane, 
                                         float *t, POINT3D_PTR pt);

float Vector3DLength(VECTOR3D_PTR va);

int ZBufferCreate(ZBUFFERV1_PTR zb, // pointer to a zbuffer object
                   int width,      // width 
                   int height);     // height

int ZBufferDelete(ZBUFFERV1_PTR zb);

void RenderListRemoveBackfaces(RENDERLIST_PTR rend_list, CAMERA_PTR cam);

void Vector4DBuild(VECTOR4D_PTR init, VECTOR4D_PTR term, VECTOR4D_PTR result);

void Vector4DCross(VECTOR4D_PTR va, 
                    VECTOR4D_PTR vb,
                    VECTOR4D_PTR vn);

float Vector4DDot(VECTOR4D_PTR va, VECTOR4D_PTR vb);

void RenderListWorldToCamera(RENDERLIST_PTR rend_list, 
                                   CAMERA_PTR cam);

void MatMulVector4D4X4(VECTOR4D_PTR  va, 
                          MATRIX4X4_PTR mb,
                          VECTOR4D_PTR  vprod);

void RenderListClipPolys(RENDERLIST_PTR rend_list, CAMERA_PTR cam, int clip_flags);

float Vector4DLengthFast(VECTOR4D_PTR va);
float FastDistance3D(float fx, float fy, float fz);

int RenderListInsert(RENDERLIST_PTR rend_list, 
                                     POLYF4DV2_PTR poly);

void RenderListCameraToPerspective(RENDERLIST_PTR rend_list, 
                                          CAMERA_PTR cam);

void RenderListPerspectiveToScreen(RENDERLIST_PTR rend_list, 
                                          CAMERA_PTR cam);

void ZBufferClear(ZBUFFERV1_PTR zb, UINT data);

void RenderListDrawZBuffer(RENDERLIST_PTR rend_list, 
                                   UCHAR *video_buffer, 
	   						       int lpitch,
                                   UCHAR *zbuffer,
                                   int zpitch);

void DrawTriangle(POLYF4DV2_PTR face,   // ptr to face
                           UCHAR *_dest_buffer,   // pointer to video buffer
                           int mem_pitch,         // bytes per line, 320, 640 etc.
                           UCHAR *_zbuffer,       // pointer to z-buffer
                           int zpitch);            // bytes per line of zbuffer

void CameraBuildMatrix(CAMERA_PTR cam, int cam_rot_seq);

void MatInit4X4(MATRIX4X4_PTR ma, 
                 float m00, float m01, float m02, float m03,
                 float m10, float m11, float m12, float m13,
                 float m20, float m21, float m22, float m23,
                 float m30, float m31, float m32, float m33);

float FastSin(float theta);
float FastCos(float theta);

void MatMul4X4(MATRIX4X4_PTR ma, MATRIX4X4_PTR mb, MATRIX4X4_PTR mprod);

//=======================================================================================
//=======================================================================================

#endif
