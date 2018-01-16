// defines for texture mapper triangular analysis
#define TRI_TYPE_NONE           0
#define TRI_TYPE_FLAT_TOP       1 
#define TRI_TYPE_FLAT_BOTTOM	2
#define TRI_TYPE_FLAT_MASK      3
#define TRI_TYPE_GENERAL        4
#define INTERP_LHS              0
#define INTERP_RHS              1
#define MAX_VERTICES_PER_POLY   6


// defines for polygons and faces version 2

// attributes of polygons and polygon faces
#define POLY4DV2_ATTR_2SIDED                0x0001
#define POLY4DV2_ATTR_TRANSPARENT           0x0002
#define POLY4DV2_ATTR_8BITCOLOR             0x0004
#define POLY4DV2_ATTR_RGB16                 0x0008
#define POLY4DV2_ATTR_RGB24                 0x0010

#define POLY4DV2_ATTR_SHADE_MODE_PURE       0x0020
#define POLY4DV2_ATTR_SHADE_MODE_CONSTANT   0x0020 // (alias)
#define POLY4DV2_ATTR_SHADE_MODE_EMISSIVE   0x0020 // (alias)

#define POLY4DV2_ATTR_SHADE_MODE_FLAT       0x0040
#define POLY4DV2_ATTR_SHADE_MODE_GOURAUD    0x0080
#define POLY4DV2_ATTR_SHADE_MODE_PHONG      0x0100
#define POLY4DV2_ATTR_SHADE_MODE_FASTPHONG  0x0100 // (alias)
#define POLY4DV2_ATTR_SHADE_MODE_TEXTURE    0x0200 

// new
#define POLY4DV2_ATTR_ENABLE_MATERIAL       0x0800 // use a real material for lighting
#define POLY4DV2_ATTR_DISABLE_MATERIAL      0x1000 // use basic color only for lighting (emulate version 1.0)

// states of polygons and faces
#define POLY4DV2_STATE_NULL               0x0000
#define POLY4DV2_STATE_ACTIVE             0x0001  
#define POLY4DV2_STATE_CLIPPED            0x0002  
#define POLY4DV2_STATE_BACKFACE           0x0004  
#define POLY4DV2_STATE_LIT                0x0008

// (new) used for simple model formats to override/control the lighting
#define VERTEX_FLAGS_OVERRIDE_MASK          0xf000 // this masks these bits to extract them
#define VERTEX_FLAGS_OVERRIDE_CONSTANT      0x1000
#define VERTEX_FLAGS_OVERRIDE_EMISSIVE      0x1000 //(alias)
#define VERTEX_FLAGS_OVERRIDE_PURE          0x1000
#define VERTEX_FLAGS_OVERRIDE_FLAT          0x2000
#define VERTEX_FLAGS_OVERRIDE_GOURAUD       0x4000
#define VERTEX_FLAGS_OVERRIDE_TEXTURE       0x8000

#define VERTEX_FLAGS_INVERT_TEXTURE_U       0x0080   // invert u texture coordinate 
#define VERTEX_FLAGS_INVERT_TEXTURE_V       0x0100   // invert v texture coordinate
#define VERTEX_FLAGS_INVERT_SWAP_UV         0x0800   // swap u and v texture coordinates



// defines for objects version 2
// objects use dynamic allocation now, but keep as max values
#define OBJECT4DV2_MAX_VERTICES           4096  // 64
#define OBJECT4DV2_MAX_POLYS              8192 // 128

// states for objects
#define OBJECT4DV2_STATE_NULL             0x0000
#define OBJECT4DV2_STATE_ACTIVE           0x0001
#define OBJECT4DV2_STATE_VISIBLE          0x0002 
#define OBJECT4DV2_STATE_CULLED           0x0004

// new
#define OBJECT4DV2_ATTR_SINGLE_FRAME      0x0001 // single frame object (emulates ver 1.0)
#define OBJECT4DV2_ATTR_MULTI_FRAME       0x0002 // multi frame object for .md2 support etc.
#define OBJECT4DV2_ATTR_TEXTURES          0x0004 // flags if object contains textured polys?


// render list defines ver 2.0
#define RENDERLIST4DV2_MAX_POLYS          32768

// defines for vertices, these are "hints" to the transform and
// lighting systems to help determine if a particular vertex has
// a valid normal that must be rotated, or a texture coordinate
// that must be clipped etc., this helps us minmize load during lighting
// and rendering since we can determine exactly what kind of vertex we
// are dealing with, something like a (direct3d) flexible vertex format in 
// as much as it can hold:
// point
// point + normal
// point + normal + texture coordinates
#define VERTEX4DTV1_ATTR_NULL             0x0000 // this vertex is empty
#define VERTEX4DTV1_ATTR_POINT            0x0001
#define VERTEX4DTV1_ATTR_NORMAL           0x0002
#define VERTEX4DTV1_ATTR_TEXTURE          0x0004

// these are some defines for conditional compilation of the new rasterizers
// I don't want 80 million different functions, so I have decided to 
// use some conditionals to change some of the logic in each
// these names aren't necessarily the most accurate, but 3 should be enough
#define RASTERIZER_ACCURATE    0 // sub-pixel accurate with fill convention
#define RASTERIZER_FAST        1 // 
#define RASTERIZER_FASTEST     2

// set this to the mode you want the engine to use
#define RASTERIZER_MODE        RASTERIZER_ACCURATE

// TYPES ///////////////////////////////////////////////////////////////////

// integer 2D vector, point without the w ////////////////////////
typedef struct VECTOR2DI_TYP
{
union
    {
    int M[2]; // array indexed storage

    // explicit names
    struct
         {
         int x,y;
         }; // end struct

    }; // end union

} VECTOR2DI, POINT2DI, *VECTOR2DI_PTR, *POINT2DI_PTR;

// integer 3D vector, point without the w ////////////////////////
typedef struct VECTOR3DI_TYP
{
union
    {
    int M[3]; // array indexed storage

    // explicit names
    struct
         {
         int x,y,z;
         }; // end struct

    }; // end union

} VECTOR3DI, POINT3DI, *VECTOR3DI_PTR, *POINT3DI_PTR;

// integer 4D homogenous vector, point with w ////////////////////
typedef struct VECTOR4DI_TYP
{
union
    {
    int M[4]; // array indexed storage

    // explicit names
    struct
         {
         int x,y,z,w;
         }; // end struct
    }; // end union

} VECTOR4DI, POINT4DI, *VECTOR4DI_PTR, *POINT4DI_PTR;


// 4D homogeneous vertex with 2 texture coordinates, and vertex normal ////////////////
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

// a self contained polygon used for the render list version 2 /////////////////////////
typedef struct POLYF4DV2_TYP
{
int      state;           // state information
int      attr;            // physical attributes of polygon
int      color;           // color of polygon
int      lit_color[3];    // holds colors after lighting, 0 for flat shading
                          // 0,1,2 for vertex colors after vertex lighting
BITMAP_IMAGE_PTR texture; // pointer to the texture information for simple texture mapping

int      mati;    // material index (-1) for no material  (new)

float    nlength; // length of the polygon normal if not normalized (new)
VECTOR4D normal;  // the general polygon normal (new)

float    avg_z;   // average z of vertices, used for simple sorting (new)

VERTEX4DTV1 vlist[3];  // the vertices of this triangle 
VERTEX4DTV1 tvlist[3]; // the vertices after transformation if needed 

POLYF4DV2_TYP *next;   // pointer to next polygon in list??
POLYF4DV2_TYP *prev;   // pointer to previous polygon in list??

} POLYF4DV2, *POLYF4DV2_PTR;

// a polygon ver 2.0 based on an external vertex list  //////////////////////////////////
typedef struct POLY4DV2_TYP
{
int state;           // state information
int attr;            // physical attributes of polygon
int color;           // color of polygon
int lit_color[3];    // holds colors after lighting, 0 for flat shading
                     // 0,1,2 for vertex colors after vertex lighting

BITMAP_IMAGE_PTR texture; // pointer to the texture information for simple texture mapping

int mati;              // material index (-1) no material (new)

VERTEX4DTV1_PTR vlist; // the vertex list itself 
POINT2D_PTR     tlist; // the texture list itself (new)
int vert[3];           // the indices into the vertex list
int text[3];           // the indices into the texture coordinate list (new)
float nlength;         // length of normal (new)

} POLY4DV2, *POLY4DV2_PTR;

// an object ver 2.0 based on a vertex list and list of polygons //////////////////////////
// this new object has a lot more flexibility and it supports "framed" animation
// that is this object can hold hundreds of frames of an animated mesh as long as
// the mesh has the same polygons and geometry, but with changing vertex positions
// similar to the Quake II .md2 format
typedef struct OBJECT4DV2_TYP
{
int   id;           // numeric id of this object
char  name[64];     // ASCII name of object just for kicks
int   state;        // state of object
int   attr;         // attributes of object
int   mati;         // material index overide (-1) - no material (new)
float *avg_radius;  // [OBJECT4DV2_MAX_FRAMES];   // average radius of object used for collision detection
float *max_radius;  // [OBJECT4DV2_MAX_FRAMES];   // maximum radius of object

POINT4D world_pos;  // position of object in world

VECTOR4D dir;       // rotation angles of object in local
                    // cords or unit direction vector user defined???

VECTOR4D ux,uy,uz;  // local axes to track full orientation
                    // this is updated automatically during
                    // rotation calls

int num_vertices;   // number of vertices per frame of this object
int num_frames;     // number of frames
int total_vertices; // total vertices, redudant, but it saves a multiply in a lot of places
int curr_frame;     // current animation frame (0) if single frame

VERTEX4DTV1_PTR vlist_local; // [OBJECT4DV1_MAX_VERTICES]; // array of local vertices
VERTEX4DTV1_PTR vlist_trans; // [OBJECT4DV1_MAX_VERTICES]; // array of transformed vertices

// these are needed to track the "head" of the vertex list for mult-frame objects
VERTEX4DTV1_PTR head_vlist_local;
VERTEX4DTV1_PTR head_vlist_trans;

// texture coordinates list (new)
POINT2D_PTR tlist;       // 3*num polys at max

BITMAP_IMAGE_PTR texture; // pointer to the texture information for simple texture mapping (new)

int num_polys;           // number of polygons in object mesh
POLY4DV2_PTR plist;      // ptr to polygons (new)

int   ivar1, ivar2;      // auxiliary vars
float fvar1, fvar2;      // auxiliary vars

// METHODS //////////////////////////////////////////////////

// setting the frame is so important that it should be a member function
// calling functions without doing this can wreak havok!
int Set_Frame(int frame);

} OBJECT4DV2, *OBJECT4DV2_PTR;

// object to hold the render list version 2.0, this way we can have more
// than one render list at a time
typedef struct RENDERLIST4DV2_TYP
{
int state; // state of renderlist ???
int attr;  // attributes of renderlist ???

// the render list is an array of pointers each pointing to 
// a self contained "renderable" polygon face POLYF4DV2
POLYF4DV2_PTR poly_ptrs[RENDERLIST4DV2_MAX_POLYS];

// additionally to cut down on allocatation, de-allocation
// of polygons each frame, here's where the actual polygon
// faces will be stored
POLYF4DV2 poly_data[RENDERLIST4DV2_MAX_POLYS];

int num_polys; // number of polys in render list

} RENDERLIST4DV2, *RENDERLIST4DV2_PTR;

// CLASSES /////////////////////////////////////////////////////////////////


// MACROS ///////////////////////////////////////////////////////////////////

// floating point comparison
#define FCMP(a,b) ( (fabs(a-b) < EPSILON_E3) ? 1 : 0)

inline void VERTEX4DTV1_COPY(VERTEX4DTV1_PTR vdst, VERTEX4DTV1_PTR vsrc)
{ *vdst = *vsrc; }

inline void VERTEX4DTV1_INIT(VERTEX4DTV1_PTR vdst, VERTEX4DTV1_PTR vsrc)
{ *vdst = *vsrc; }

inline float VECTOR4D_Length_Fast2(VECTOR4D_PTR va)
{
// this function computes the distance from the origin to x,y,z

int temp;  // used for swaping
int x,y,z; // used for algorithm

// make sure values are all positive
x = fabs(va->x) * 1024;
y = fabs(va->y) * 1024;
z = fabs(va->z) * 1024;

// sort values
if (y < x) SWAP(x,y,temp)
if (z < y) SWAP(y,z,temp)
if (y < x) SWAP(x,y,temp)

int dist = (z + 11 * (y >> 5) + (x >> 2) );

// compute distance with 8% error
return((float)(dist >> 10));

} // end VECTOR4D_Length_Fast2

// TYPES ///////////////////////////////////////////////////////////////////

// EXTERNALS ///////////////////////////////////////////////////////////////

extern UCHAR logbase2ofx[513];

extern UCHAR rgblightlookup[4096][256]; // rgb 8.12 lighting table lookup

extern char texture_path[80]; // root path to ALL textures, make current directory for now

// PROTOTYPES //////////////////////////////////////////////////////////////

char *Extract_Filename_From_Path(char *filepath, char *filename);

int Set_OBJECT4DV2_Frame(OBJECT4DV2_PTR obj, int frame);

int Destroy_OBJECT4DV2(OBJECT4DV2_PTR obj);

int Init_OBJECT4DV2(OBJECT4DV2_PTR obj,   // object to allocate
                     int _num_vertices, 
                     int _num_polys, 
                     int _num_frames,
                     int destroy=0);

void Translate_OBJECT4DV2(OBJECT4DV2_PTR obj, VECTOR4D_PTR vt);

void Scale_OBJECT4DV2(OBJECT4DV2_PTR obj, VECTOR4D_PTR vs, int all_frames=0);

void Transform_OBJECT4DV2(OBJECT4DV2_PTR obj, MATRIX4X4_PTR mt,   
                          int coord_select, int transform_basis, int all_frames=0);
                                               
void Rotate_XYZ_OBJECT4DV2(OBJECT4DV2_PTR obj, 
                          float theta_x,     
                          float theta_y, 
                          float theta_z,
                          int all_frames);

void Model_To_World_OBJECT4DV2(OBJECT4DV2_PTR obj, int coord_select = TRANSFORM_LOCAL_TO_TRANS, int all_frames=0);

int Cull_OBJECT4DV2(OBJECT4DV2_PTR obj, CAM4DV1_PTR cam, int cull_flags);

void Remove_Backfaces_OBJECT4DV2(OBJECT4DV2_PTR obj, CAM4DV1_PTR cam);

void Remove_Backfaces_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, CAM4DV1_PTR cam);

void World_To_Camera_OBJECT4DV2(OBJECT4DV2_PTR obj, CAM4DV1_PTR cam);

void Camera_To_Perspective_OBJECT4DV2(OBJECT4DV2_PTR obj, CAM4DV1_PTR cam);

void Perspective_To_Screen_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, 
                                          CAM4DV1_PTR cam);

void Camera_To_Perspective_Screen_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, 
                                                 CAM4DV1_PTR cam);

void Camera_To_Perspective_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, 
                                          CAM4DV1_PTR cam);

void Camera_To_Perspective_Screen_OBJECT4DV2(OBJECT4DV2_PTR obj, CAM4DV1_PTR cam);

void Perspective_To_Screen_OBJECT4DV2(OBJECT4DV2_PTR obj, CAM4DV1_PTR cam);

void Convert_From_Homogeneous4D_OBJECT4DV1(OBJECT4DV1_PTR obj);

int Insert_POLY4DV2_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, 
                                   POLY4DV2_PTR poly);

int Insert_POLYF4DV2_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, 
                                     POLYF4DV2_PTR poly);

int Insert_OBJECT4DV2_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, 
                                      OBJECT4DV2_PTR obj,
                                      int insert_local);

void Reset_OBJECT4DV2(OBJECT4DV2_PTR obj);

int Compute_OBJECT4DV2_Poly_Normals(OBJECT4DV2_PTR obj);

void Draw_OBJECT4DV2_Wire(OBJECT4DV2_PTR obj, UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV2_Wire16(OBJECT4DV2_PTR obj, UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV2_Wire(RENDERLIST4DV2_PTR rend_list, 
                              UCHAR *video_buffer, int lpitch);


void Draw_RENDERLIST4DV2_Wire16(RENDERLIST4DV2_PTR rend_list, 
                              UCHAR *video_buffer, int lpitch);


void Draw_RENDERLIST4DV2_Solid(RENDERLIST4DV2_PTR rend_list, 
                              UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV2_Solid16(RENDERLIST4DV2_PTR rend_list, 
                              UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV2_Textured(OBJECT4DV2_PTR obj, UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV2_Textured(RENDERLIST4DV2_PTR rend_list, 
                                  UCHAR *video_buffer, int lpitch, BITMAP_IMAGE_PTR texture);

void Draw_RENDERLIST4DV2_Textured16(RENDERLIST4DV2_PTR rend_list, 
                                    UCHAR *video_buffer, int lpitch, BITMAP_IMAGE_PTR texture);

void Draw_OBJECT4DV1_Solid2_16(OBJECT4DV1_PTR obj, 
                            UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV1_Solid2(RENDERLIST4DV1_PTR rend_list, 
                              UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV1_Solid2(OBJECT4DV1_PTR obj, 
                          UCHAR *video_buffer, int lpitch);

void World_To_Camera_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, 
                                   CAM4DV1_PTR cam);

void Draw_OBJECT4DV1_Solid2_16(OBJECT4DV1_PTR obj, 
                            UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV1_Solid2(RENDERLIST4DV1_PTR rend_list, 
                              UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV1_Solid2(OBJECT4DV1_PTR obj, 
                          UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV1_Textured(OBJECT4DV1_PTR obj, UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV1_Textured(RENDERLIST4DV1_PTR rend_list, 
                                  UCHAR *video_buffer, int lpitch, BITMAP_IMAGE_PTR texture);

void Draw_RENDERLIST4DV1_Textured16(RENDERLIST4DV1_PTR rend_list, 
                                    UCHAR *video_buffer, int lpitch, BITMAP_IMAGE_PTR texture);

void Draw_RENDERLIST4DV2_Gouraud16(RENDERLIST4DV2_PTR rend_list, 
                                    UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV1_Solid2_16(RENDERLIST4DV1_PTR rend_list, 
                                UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV1_Solid2_16(OBJECT4DV1_PTR obj, 
                            UCHAR *video_buffer, int lpitch);

void Draw_RENDERLIST4DV1_Solid2_16(RENDERLIST4DV1_PTR rend_list, 
                                UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV1_Solid2_16(OBJECT4DV1_PTR obj, 
                            UCHAR *video_buffer, int lpitch);


void Draw_RENDERLIST4DV1_Solid2(RENDERLIST4DV1_PTR rend_list, 
                                UCHAR *video_buffer, int lpitch);

void Draw_OBJECT4DV1_Solid2(OBJECT4DV1_PTR obj, 
                            UCHAR *video_buffer, int lpitch);

float Compute_OBJECT4DV2_Radius(OBJECT4DV2_PTR obj);

int Compute_OBJECT4DV2_Vertex_Normals(OBJECT4DV2_PTR obj);

int Load_OBJECT4DV2_PLG(OBJECT4DV2_PTR obj, // pointer to object
                    char *filename,         // filename of plg file
                    VECTOR4D_PTR scale,     // initial scaling factors
                    VECTOR4D_PTR pos,       // initial position
                    VECTOR4D_PTR rot,       // initial rotations
                    int vertex_flags=0);    // flags to re-order vertices
 

int Load_OBJECT4DV2_3DSASC(OBJECT4DV2_PTR obj,   // pointer to object
                           char *filename,       // filename of ASC file
                           VECTOR4D_PTR scale,   // initial scaling factors
                           VECTOR4D_PTR pos,     // initial position
                           VECTOR4D_PTR rot,     // initial rotations
                           int vertex_flags=0);  // flags to re-order vertices


int Load_OBJECT4DV2_COB(OBJECT4DV2_PTR obj,   // pointer to object
                        char *filename,       // filename of Caligari COB file
                        VECTOR4D_PTR scale,   // initial scaling factors
                        VECTOR4D_PTR pos,     // initial position
                        VECTOR4D_PTR rot,     // initial rotations
                        int vertex_flags=0);  // flags to re-order vertices 
                                              // and perform transforms

void Reset_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list);

int RGB_12_8_Lighting_Table_Builder(LPPALETTEENTRY src_palette,   // source palette
                                    UCHAR rgblookup[4096][256]);  // lookup table


int Light_OBJECT4DV2_World16(OBJECT4DV2_PTR obj,  // object to process
                             CAM4DV1_PTR cam,     // camera position
                             LIGHTV1_PTR lights,  // light list (might have more than one)
                             int max_lights);     // maximum lights in list


int Light_OBJECT4DV2_World(OBJECT4DV2_PTR obj,  // object to process
                           CAM4DV1_PTR cam,     // camera position
                           LIGHTV1_PTR lights,  // light list (might have more than one)
                           int max_lights);      // maximum lights in list

int Light_RENDERLIST4DV2_World(RENDERLIST4DV2_PTR rend_list,  // list to process
                                 CAM4DV1_PTR cam,     // camera position
                                 LIGHTV1_PTR lights,  // light list (might have more than one)
                                 int max_lights);     // maximum lights in list


int Light_RENDERLIST4DV2_World16(RENDERLIST4DV2_PTR rend_list,  // list to process
                                 CAM4DV1_PTR cam,     // camera position
                                 LIGHTV1_PTR lights,  // light list (might have more than one)
                                 int max_lights);     // maximum lights in list

// z-sort algorithm (simple painters algorithm)
void Sort_RENDERLIST4DV2(RENDERLIST4DV2_PTR rend_list, int sort_method);

// avg z-compare
int Compare_AvgZ_POLYF4DV2(const void *arg1, const void *arg2);

// near z-compare
int Compare_NearZ_POLYF4DV2(const void *arg1, const void *arg2);

// far z-compare
int Compare_FarZ_POLYF4DV2(const void *arg1, const void *arg2);

void Draw_Textured_Triangle(POLYF4DV2_PTR face, UCHAR *dest_buffer, int mem_pitch);

void Draw_Textured_TriangleFS(POLYF4DV2_PTR face,   // ptr to face
                              UCHAR *_dest_buffer,    // pointer to video buffer
                              int mem_pitch);         // bytes per line, 320, 640 etc.

void Draw_Textured_Triangle16(POLYF4DV2_PTR face, UCHAR *dest_buffer, int mem_pitch);

void Draw_Textured_TriangleFS16(POLYF4DV2_PTR face,   // ptr to face
                              UCHAR *_dest_buffer,    // pointer to video buffer
                              int mem_pitch);         // bytes per line, 320, 640 etc.

void Draw_Gouraud_Triangle16(POLYF4DV2_PTR face,   // ptr to face
                             UCHAR *_dest_buffer,  // pointer to video buffer
                             int mem_pitch);       // bytes per line, 320, 640 etc.

void Draw_Gouraud_Triangle(POLYF4DV2_PTR face,     // ptr to face
                             UCHAR *dest_buffer,   // pointer to video buffer
                             int mem_pitch);       // bytes per line, 320, 640 etc.


void Draw_Top_Tri2_16(float x1, float y1, 
                      float x2, float y2, 
                      float x3, float y3,
                      int color, 
                      UCHAR *_dest_buffer, int mempitch);

void Draw_Bottom_Tri2_16(float x1, float y1, 
                         float x2, float y2, 
                         float x3, float y3,
                         int color,
                         UCHAR *_dest_buffer, int mempitch);

void Draw_Triangle_2D2_16(float x1, float y1,
                          float x2, float y2,
                          float x3, float y3,
                          int color,
                          UCHAR *dest_buffer, int mempitch);


void Draw_Top_Tri2(float x1, float y1, 
                      float x2, float y2, 
                      float x3, float y3,
                      int color, 
                      UCHAR *_dest_buffer, int mempitch);

void Draw_Bottom_Tri2(float x1, float y1, 
                         float x2, float y2, 
                         float x3, float y3,
                         int color,
                         UCHAR *_dest_buffer, int mempitch);

void Draw_Triangle_2D2(float x1, float y1,
                          float x2, float y2,
                          float x3, float y3,
                          int color,
                          UCHAR *dest_buffer, int mempitch);

int Load_Bitmap_File2(BITMAP_FILE_PTR bitmap, char *filename);

int Load_Bitmap_PCX_File(BITMAP_FILE_PTR bitmap, char *filename);

     

