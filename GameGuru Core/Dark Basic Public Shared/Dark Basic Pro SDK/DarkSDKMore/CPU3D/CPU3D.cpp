#include "CPU3D.h"
#include "math.h"
#include "stdio.h"
#include "memory.h"

//=======================================================================================
//=======================================================================================

// storage for our lookup tables
float cos_look[361]; // 1 extra element so we can store 0-360 inclusive
float sin_look[361];

// these defined the general clipping rectangle
int min_clip_x = 0,                             // clipping rectangle 
    max_clip_x = (SCREEN_WIDTH),
    min_clip_y = 0,
    max_clip_y = (SCREEN_HEIGHT);

//=======================================================================================
//=======================================================================================

void BuildSinCosTables(void)
{
  
	// generate the tables 0 - 360 
	for (int ang = 0; ang <= 360; ang++)
    {
    // convert ang to radians
    float theta = (float)ang*PI/(float)180;

    // insert next entry into table
    cos_look[ang] = cos(theta);
    sin_look[ang] = sin(theta);

    }

} 


//=======================================================================================
//=======================================================================================

void CameraInit(CAMERA_PTR cam,       // the camera object
                  int cam_attr,          // attributes
                  POINT4D_PTR cam_pos,   // initial camera position
                  VECTOR4D_PTR cam_dir,  // initial camera angles
                  POINT4D_PTR cam_target, // UVN target
                  float near_clip_z,     // near and far clipping planes
                  float far_clip_z,
                  float fov,             // field of view in degrees
                  float viewport_width,  // size of final screen viewport
                  float viewport_height)
{

	cam->attr = cam_attr;

	Vector4DCopy(&cam->pos, cam_pos); // positions
	Vector4DCopy(&cam->dir, cam_dir); // direction vector or angles for
									   // euler camera
	// for UVN camera
	Vector4DInitXYZ(&cam->u, 1,0,0);  // set to +x
	Vector4DInitXYZ(&cam->v, 0,1,0);  // set to +y
	Vector4DInitXYZ(&cam->n, 0,0,1);  // set to +z        

	if (cam_target!= NULL)
	   Vector4DCopy(&cam->target, cam_target); // UVN target
	else
	   Vector4DZero(&cam->target);

	cam->near_clip_z = near_clip_z;     // near z=constant clipping plane
	cam->far_clip_z  = far_clip_z;      // far z=constant clipping plane

	cam->viewport_width  = viewport_width;   // dimensions of viewport
	cam->viewport_height = viewport_height;

	cam->viewport_center_x = (viewport_width-1)/2; // center of viewport
	cam->viewport_center_y = (viewport_height-1)/2;

	cam->aspect_ratio = (float)viewport_width/(float)viewport_height;

	// set all camera matrices to identity matrix
	MatrixIdent4X4(&cam->mcam);
	MatrixIdent4X4(&cam->mper);
	MatrixIdent4X4(&cam->mscr);
	// set independent vars
	cam->fov              = fov;

	// set the viewplane dimensions up, they will be 2 x (2/ar)
	cam->viewplane_width  = 2.0;
	cam->viewplane_height = 2.0/cam->aspect_ratio;

	// now we know fov and we know the viewplane dimensions plug into formula and
	// solve for view distance parameters
	float tan_fov_div2 = tan(DEG_TO_RAD(fov/2));

	cam->view_dist = (0.5)*(cam->viewplane_width)*tan_fov_div2;

	// test for 90 fov
	if (fov == 90.0)
    {
      // set up the clipping planes -- easy for 90 degrees!
      POINT3D pt_origin; // point on the plane
      Vector3DInitXYZ(&pt_origin,0,0,0);

      VECTOR3D vn; // normal to plane

      // right clipping plane 
      Vector3DInitXYZ(&vn,1,0,-1); // x=z plane
      Plane3DInit(&cam->rt_clip_plane, &pt_origin,  &vn, 1);

      // left clipping plane
      Vector3DInitXYZ(&vn,-1,0,-1); // -x=z plane
      Plane3DInit(&cam->lt_clip_plane, &pt_origin,  &vn, 1);

      // top clipping plane
      Vector3DInitXYZ(&vn,0,1,-1); // y=z plane
      Plane3DInit(&cam->tp_clip_plane, &pt_origin,  &vn, 1);

      // bottom clipping plane
      Vector3DInitXYZ(&vn,0,-1,-1); // -y=z plane
      Plane3DInit(&cam->bt_clip_plane, &pt_origin,  &vn, 1);
    } // end if d=1
else 
    {
      // now compute clipping planes
      POINT3D pt_origin; // point on the plane
      Vector3DInitXYZ(&pt_origin,0,0,0);

      VECTOR3D vn; // normal to plane

      // right clipping plane
      Vector3DInitXYZ(&vn,cam->view_dist,0,-cam->viewplane_width/2.0); 
      Plane3DInit(&cam->rt_clip_plane, &pt_origin,  &vn, 1);

      // left clipping plane
      Vector3DInitXYZ(&vn,-cam->view_dist,0,-cam->viewplane_width/2.0); 
      Plane3DInit(&cam->lt_clip_plane, &pt_origin,  &vn, 1);

      // top clipping plane, same construction
      Vector3DInitXYZ(&vn,0,cam->view_dist,-cam->viewplane_width/2.0); 
      Plane3DInit(&cam->tp_clip_plane, &pt_origin,  &vn, 1);

      // bottom clipping plane, same inversion
      Vector3DInitXYZ(&vn,0,-cam->view_dist,-cam->viewplane_width/2.0); 
      Plane3DInit(&cam->bt_clip_plane, &pt_origin,  &vn, 1);
    } // end else

} 

void Vector3DNormalize(VECTOR3D_PTR va)
{
	// normalizes the sent vector in placew

	// compute length
	float length = sqrtf(va->x*va->x + va->y*va->y + va->z*va->z);

	// test for zero length vector
	// if found return zero vector
	if (length < EPSILON_E5) 
	   return;

	float length_inv = 1/length;

	// compute normalized version of vector
	va->x*=length_inv;
	va->y*=length_inv;
	va->z*=length_inv;

}

void Vector3DNormalize(VECTOR3D_PTR va, VECTOR3D_PTR vn)
{
	// normalizes the sent vector and returns the result in vn

	Vector3DZero(vn);

	// compute length
	float length = Vector3DLength(va);

	// test for zero length vector
	// if found return zero vector
	if (length < EPSILON_E5) 
	   return;

	float length_inv = 1.0/length;

	// compute normalized version of vector
	vn->x = va->x*length_inv;
	vn->y = va->y*length_inv;
	vn->z = va->z*length_inv;

}

void Plane3DInit(PLANE3D_PTR plane, POINT3D_PTR p0, 
                  VECTOR3D_PTR normal, int normalize=0)
{
	// this function initializes a 3d plane

	// copy the point
	Point3DCopy(&plane->p0, p0);

	// if normalize is 1 then the normal is made into a unit vector
	if (!normalize)
	   Vector3DCopy(&plane->n, normal);
	else
	   {
	   // make normal into unit vector
	   Vector3DNormalize(normal,&plane->n);
	   } 

} 

float Vector3DLength(VECTOR3D_PTR va)
{
	return( (float)sqrtf(va->x*va->x + va->y*va->y + va->z*va->z) );

} 

int ZBufferCreate(ZBUFFERV1_PTR zb, // pointer to a zbuffer object
                   int width,      // width 
                   int height)     // height
{
	if (!zb) 
	   return 0;

	// is there any memory already allocated
	if (zb->zbuffer)
	   free(zb->zbuffer);

	// set fields
	zb->width  = width;
	zb->height = height;
	zb->attr   = 32;

	// compute size in quads
	zb->sizeq = width*height;

	// allocate memory
	if ((zb->zbuffer = (UCHAR *)malloc(width * height * sizeof(int))))
		return 1;
	else
		return 0;

} 

int ZBufferDelete(ZBUFFERV1_PTR zb)
{
	// this function deletes and frees the memory of the zbuffer

	// test for valid object
	if (zb)
	   {
	   // delete memory and zero object
	   if (zb->zbuffer)
		  free(zb->zbuffer);
   
	   // clear memory
	   memset((void *)zb,0, sizeof(ZBUFFERV1));

	   return(1);

	   } // end if
	else
	   return 0;

} 


void RenderListRemoveBackfaces(RENDERLIST_PTR rend_list, CAMERA_PTR cam)
{

	static POLYF4DV2_PTR curr_poly;
	static int poly;

	for ( poly = 0; poly < rend_list->num_polys; poly++)
    {
    // get current polygon
    curr_poly = rend_list->poly_ptrs[poly];

    // is this polygon valid?
    if ((curr_poly==NULL) || !(curr_poly->state & POLY4DV2_STATE_ACTIVE) ||
        (curr_poly->state & POLY4DV2_STATE_CLIPPED ) || 
        (curr_poly->attr  & POLY4DV2_ATTR_2SIDED)    ||
        (curr_poly->state & POLY4DV2_STATE_BACKFACE) )
        continue; // move onto next poly
    
        // we need to compute the normal of this polygon face, and recall
        // that the vertices are in cw order, u = p0->p1, v=p0->p2, n=uxv
        VECTOR4D u, v, n;
 
        // build u, v
        Vector4DBuild(&curr_poly->vertexList[0].v, &curr_poly->vertexList[1].v, &u);
        Vector4DBuild(&curr_poly->vertexList[0].v, &curr_poly->vertexList[2].v, &v);

        // compute cross product
        Vector4DCross(&u, &v, &n);

		/*float fNorm = sqrtf ( n.x * n.x + n.y * n.y + n.z * n.z + n.w * n.w );

		n.x = n.x / fNorm;
		n.y = n.y / fNorm;
		n.z = n.z / fNorm;
		n.w = n.w / fNorm;*/

        // now create eye vector to viewpoint
        VECTOR4D view;
        Vector4DBuild(&curr_poly->vertexList[0].v, &cam->pos, &view); 

        // and finally, compute the dot product
        float dp = Vector4DDot(&n, &view);

        // if the sign is > 0 then visible, 0 = scathing, < 0 invisible
        if (dp <= 0.0 )
		{
            SetBit(curr_poly->state, POLY4DV2_STATE_BACKFACE);
		}
		else
		{
			// transform 
			//for (int vertex = 0; vertex < 3; vertex++)
			//{
				// transform the vertex by the mcam matrix within the camera
				static POINT4D presult; // hold result of each transformation

				// transform point
				MatMulVector4D4X4(&curr_poly->vertexList[0].v, &cam->mcam, &presult);
				// store result back
				Vector4DCopy(&curr_poly->vertexList[0].v, &presult); 

				// transform point
				MatMulVector4D4X4(&curr_poly->vertexList[1].v, &cam->mcam, &presult);
				// store result back
				Vector4DCopy(&curr_poly->vertexList[1].v, &presult); 

				// transform point
				MatMulVector4D4X4(&curr_poly->vertexList[2].v, &cam->mcam, &presult);
				// store result back
				Vector4DCopy(&curr_poly->vertexList[2].v, &presult); 
			//} 
		}

    }

}

void Vector4DBuild(VECTOR4D_PTR init, VECTOR4D_PTR term, VECTOR4D_PTR result)
{
	// build a 4d vector
	result->x = term->x - init->x;
	result->y = term->y - init->y;
	result->z = term->z - init->z;
	result->w = 1;

} 

void Vector4DCross(VECTOR4D_PTR va, 
                    VECTOR4D_PTR vb,
                    VECTOR4D_PTR vn)
{
	// this function computes the cross product between va and vb
	// and returns the vector that is perpendicular to each in vn

	vn->x =  ( (va->y * vb->z) - (va->z * vb->y) );
	vn->y = -( (va->x * vb->z) - (va->z * vb->x) );
	vn->z =  ( (va->x * vb->y) - (va->y * vb->x) ); 
	vn->w = 1;

} 

float Vector4DDot(VECTOR4D_PTR va, VECTOR4D_PTR vb)
{
	// computes the dot product between va and vb
	return( (va->x * vb->x) + (va->y * vb->y) + (va->z * vb->z) );
} 

void RenderListWorldToCamera(RENDERLIST_PTR rend_list, 
                                   CAMERA_PTR cam)
{
	for (int poly = 0; poly < rend_list->num_polys; poly++)
	{
		// acquire current polygon
		POLYF4DV2_PTR curr_poly = rend_list->poly_ptrs[poly];

		// is this polygon valid?
		if ((curr_poly==NULL) || !(curr_poly->state & POLY4DV2_STATE_ACTIVE) ||
			 (curr_poly->state & POLY4DV2_STATE_CLIPPED ) ||
			 (curr_poly->state & POLY4DV2_STATE_BACKFACE) )
			   continue; // move onto next poly

		// transform 
		for (int vertex = 0; vertex < 3; vertex++)
		{
		// transform the vertex by the mcam matrix within the camera
		POINT4D presult; // hold result of each transformation

		// transform point
		MatMulVector4D4X4(&curr_poly->vertexList[vertex].v, &cam->mcam, &presult);

		// store result back
		Vector4DCopy(&curr_poly->vertexList[vertex].v, &presult); 
		} 

	}

} 

void MatMulVector4D4X4(VECTOR4D_PTR  va, 
                          MATRIX4X4_PTR mb,
                          VECTOR4D_PTR  vprod)
{
	// this function multiplies a VECTOR4D against a 
	// 4x4 matrix - ma*mb and stores the result in mprod

    /*for (int col=0; col < 4; col++)
        {
        // compute dot product from row of ma 
        // and column of mb
        float sum = 0; // used to hold result

        for (int row=0; row<4; row++)
             {
             // add in next product pair
             sum+=(va->M[row]*mb->M[row][col]);
             } 

        // insert resulting col element
        vprod->M[col] = sum;

        } */

	static float sum;

	sum = 0;
	sum+=(va->M[0]*mb->M[0][0]);
	sum+=(va->M[1]*mb->M[1][0]);
	sum+=(va->M[2]*mb->M[2][0]);
	sum+=(va->M[3]*mb->M[3][0]);
	vprod->M[0] = sum;

	sum = 0;
	sum+=(va->M[0]*mb->M[0][1]);
	sum+=(va->M[1]*mb->M[1][1]);
	sum+=(va->M[2]*mb->M[2][1]);
	sum+=(va->M[3]*mb->M[3][1]);
	vprod->M[1] = sum;

	sum = 0;
	sum+=(va->M[0]*mb->M[0][2]);
	sum+=(va->M[1]*mb->M[1][2]);
	sum+=(va->M[2]*mb->M[2][2]);
	sum+=(va->M[3]*mb->M[3][2]);
	vprod->M[2] = sum;

	sum = 0;
	sum+=(va->M[0]*mb->M[0][3]);
	sum+=(va->M[1]*mb->M[1][3]);
	sum+=(va->M[2]*mb->M[2][3]);
	sum+=(va->M[3]*mb->M[3][3]);
	vprod->M[3] = sum;

}

void RenderListClipPolys(RENDERLIST_PTR rend_list, CAMERA_PTR cam, int clip_flags)
{
	// this function clips the polygons in the list against the requested clipping planes
	// and sets the clipped flag on the poly, so it's not rendered

	// internal clipping codes
	#define CLIP_CODE_GZ   0x0001    // z > z_max
	#define CLIP_CODE_LZ   0x0002    // z < z_min
	#define CLIP_CODE_IZ   0x0004    // z_min < z < z_max

	#define CLIP_CODE_GX   0x0001    // x > x_max
	#define CLIP_CODE_LX   0x0002    // x < x_min
	#define CLIP_CODE_IX   0x0004    // x_min < x < x_max

	#define CLIP_CODE_GY   0x0001    // y > y_max
	#define CLIP_CODE_LY   0x0002    // y < y_min
	#define CLIP_CODE_IY   0x0004    // y_min < y < y_max

	#define CLIP_CODE_NULL 0x0000

	int vertex_ccodes[3]; // used to store clipping flags
	int num_verts_in;     // number of vertices inside
	int v0, v1, v2;       // vertex indices

	float z_factor,       // used in clipping computations
		  z_test;         // used in clipping computations

	float xi, yi, x01i, y01i, x02i, y02i, // vertex intersection points
		  t1, t2,                         // parametric t values
		  ui, vi, u01i, v01i, u02i, v02i; // texture intersection points

	int last_poly_index,            // last valid polygon in polylist
		insert_poly_index;          // the current position new polygons are inserted at

	VECTOR4D u,v,n;                 // used in vector calculations

	POLYF4DV2 temp_poly;            // used when we need to split a poly into 2 polys

	// set last, current insert index to end of polygon list
	// we don't want to clip poly's two times
	insert_poly_index = last_poly_index = rend_list->num_polys;

	// traverse polygon list and clip/cull polygons
	for (int poly = 0; poly < last_poly_index; poly++)
    {
		// acquire current polygon
		POLYF4DV2_PTR curr_poly = rend_list->poly_ptrs[poly];

		// is this polygon valid?
		// test this polygon if and only if it's not clipped, not culled,
		// active, and visible and not 2 sided. Note we test for backface in the event that
		// a previous call might have already determined this, so why work
		// harder!
		if ((curr_poly==NULL) || !(curr_poly->state & POLY4DV2_STATE_ACTIVE) ||
			(curr_poly->state & POLY4DV2_STATE_CLIPPED ) || 
			(curr_poly->state & POLY4DV2_STATE_BACKFACE) )
			continue; // move onto next poly
           
		   // clip/cull to x-planes       
		   if (clip_flags & CLIP_POLY_X_PLANE)
			   {
			   // clip/cull only based on x clipping planes
			   // for each vertice determine if it's in the clipping region or beyond it and
			   // set the appropriate clipping code
			   // we do NOT clip the final triangles, we are only trying to trivally reject them 
			   // we are going to clip polygons in the rasterizer to the screen rectangle
			   // but we do want to clip/cull polys that are totally outside the viewfrustrum

			   // since we are clipping to the right/left x-planes we need to use the FOV or
			   // the plane equations to find the z value that at the current x position would
			   // be outside the plane
			   z_factor = (0.5)*cam->viewplane_width/cam->view_dist;  

			   // vertex 0

			   z_test = z_factor*curr_poly->vertexList[0].z;

			   if (curr_poly->vertexList[0].x > z_test)
				  vertex_ccodes[0] = CLIP_CODE_GX;
			   else
			   if (curr_poly->vertexList[0].x < -z_test)
				  vertex_ccodes[0] = CLIP_CODE_LX;
			   else
				  vertex_ccodes[0] = CLIP_CODE_IX;
          
			   // vertex 1

			   z_test = z_factor*curr_poly->vertexList[1].z;         

			   if (curr_poly->vertexList[1].x > z_test)
				  vertex_ccodes[1] = CLIP_CODE_GX;
			   else
			   if (curr_poly->vertexList[1].x < -z_test)
				  vertex_ccodes[1] = CLIP_CODE_LX;
			   else
				  vertex_ccodes[1] = CLIP_CODE_IX;

			   // vertex 2

			   z_test = z_factor*curr_poly->vertexList[2].z;              

			   if (curr_poly->vertexList[2].x > z_test)
				  vertex_ccodes[2] = CLIP_CODE_GX;
			   else
			   if (curr_poly->vertexList[2].x < -z_test)
				  vertex_ccodes[2] = CLIP_CODE_LX;
			   else
				  vertex_ccodes[2] = CLIP_CODE_IX;
           
			  // test for trivial rejections, polygon completely beyond right or left
			  // clipping planes
			  if ( ((vertex_ccodes[0] == CLIP_CODE_GX) && 
					(vertex_ccodes[1] == CLIP_CODE_GX) && 
					(vertex_ccodes[2] == CLIP_CODE_GX) ) ||

				   ((vertex_ccodes[0] == CLIP_CODE_LX) && 
					(vertex_ccodes[1] == CLIP_CODE_LX) && 
					(vertex_ccodes[2] == CLIP_CODE_LX) ) )

				 {
				 // clip the poly completely out of frustrum
				 SetBit(curr_poly->state, POLY4DV2_STATE_CLIPPED);
             
				 // move on to next polygon
				 continue;
				 } // end if
                           
			  } // end if x planes

		   // clip/cull to y-planes       
		   if (clip_flags & CLIP_POLY_Y_PLANE)
			   {
			   // clip/cull only based on y clipping planes
			   // for each vertice determine if it's in the clipping region or beyond it and
			   // set the appropriate clipping code
			   // we do NOT clip the final triangles, we are only trying to trivally reject them 
			   // we are going to clip polygons in the rasterizer to the screen rectangle
			   // but we do want to clip/cull polys that are totally outside the viewfrustrum

			   // since we are clipping to the top/bottom y-planes we need to use the FOV or
			   // the plane equations to find the z value that at the current y position would
			   // be outside the plane
			   z_factor = (0.5)*cam->viewplane_width/cam->view_dist;  

			   // vertex 0
			   z_test = z_factor*curr_poly->vertexList[0].z;

			   if (curr_poly->vertexList[0].y > z_test)
				  vertex_ccodes[0] = CLIP_CODE_GY;
			   else
			   if (curr_poly->vertexList[0].y < -z_test)
				  vertex_ccodes[0] = CLIP_CODE_LY;
			   else
				  vertex_ccodes[0] = CLIP_CODE_IY;
          
			   // vertex 1
			   z_test = z_factor*curr_poly->vertexList[1].z;         

			   if (curr_poly->vertexList[1].y > z_test)
				  vertex_ccodes[1] = CLIP_CODE_GY;
			   else
			   if (curr_poly->vertexList[1].y < -z_test)
				  vertex_ccodes[1] = CLIP_CODE_LY;
			   else
				  vertex_ccodes[1] = CLIP_CODE_IY;

			   // vertex 2
			   z_test = z_factor*curr_poly->vertexList[2].z;              

			   if (curr_poly->vertexList[2].y > z_test)
				  vertex_ccodes[2] = CLIP_CODE_GY;
			   else
			   if (curr_poly->vertexList[2].x < -z_test)
				  vertex_ccodes[2] = CLIP_CODE_LY;
			   else
				  vertex_ccodes[2] = CLIP_CODE_IY;
           
			  // test for trivial rejections, polygon completely beyond top or bottom
			  // clipping planes
			  if ( ((vertex_ccodes[0] == CLIP_CODE_GY) && 
					(vertex_ccodes[1] == CLIP_CODE_GY) && 
					(vertex_ccodes[2] == CLIP_CODE_GY) ) ||

				   ((vertex_ccodes[0] == CLIP_CODE_LY) && 
					(vertex_ccodes[1] == CLIP_CODE_LY) && 
					(vertex_ccodes[2] == CLIP_CODE_LY) ) )

				 {
				 // clip the poly completely out of frustrum
				 SetBit(curr_poly->state, POLY4DV2_STATE_CLIPPED);
             
				 // move on to next polygon
				 continue;
				 } // end if
                           
			  } // end if y planes

			// clip/cull to z planes
			if (clip_flags & CLIP_POLY_Z_PLANE)
			   {
			   // clip/cull only based on z clipping planes
			   // for each vertice determine if it's in the clipping region or beyond it and
			   // set the appropriate clipping code
			   // then actually clip all polygons to the near clipping plane, this will result
			   // in at most 1 additional triangle

			   // reset vertex counters, these help in classification
			   // of the final triangle 
			   num_verts_in = 0;

			   // vertex 0
			   if (curr_poly->vertexList[0].z > cam->far_clip_z)
				  {
				  vertex_ccodes[0] = CLIP_CODE_GZ;
				  } 
			   else
			   if (curr_poly->vertexList[0].z < cam->near_clip_z)
				  {
				  vertex_ccodes[0] = CLIP_CODE_LZ;
				  }
			   else
				  {
				  vertex_ccodes[0] = CLIP_CODE_IZ;
				  num_verts_in++;
				  } 
          
			   // vertex 1
			   if (curr_poly->vertexList[1].z > cam->far_clip_z)
				  {
				  vertex_ccodes[1] = CLIP_CODE_GZ;
				  } 
			   else
			   if (curr_poly->vertexList[1].z < cam->near_clip_z)
				  {
				  vertex_ccodes[1] = CLIP_CODE_LZ;
				  }
			   else
				  {
				  vertex_ccodes[1] = CLIP_CODE_IZ;
				  num_verts_in++;
				  }     

			   // vertex 2
			   if (curr_poly->vertexList[2].z > cam->far_clip_z)
				  {
				  vertex_ccodes[2] = CLIP_CODE_GZ;
				  } 
			   else
			   if (curr_poly->vertexList[2].z < cam->near_clip_z)
				  {
				  vertex_ccodes[2] = CLIP_CODE_LZ;
				  }
			   else
				  {
				  vertex_ccodes[2] = CLIP_CODE_IZ;
				  num_verts_in++;
				  } 
           
			  // test for trivial rejections, polygon completely beyond far or near
			  // z clipping planes
			  if ( ((vertex_ccodes[0] == CLIP_CODE_GZ) && 
					(vertex_ccodes[1] == CLIP_CODE_GZ) && 
					(vertex_ccodes[2] == CLIP_CODE_GZ) ) ||

				   ((vertex_ccodes[0] == CLIP_CODE_LZ) && 
					(vertex_ccodes[1] == CLIP_CODE_LZ) && 
					(vertex_ccodes[2] == CLIP_CODE_LZ) ) )

				 {
				 // clip the poly completely out of frustrum
				 SetBit(curr_poly->state, POLY4DV2_STATE_CLIPPED);
             
				 // move on to next polygon
				 continue;
				 } // end if

			  // test if any vertex has protruded beyond near clipping plane?
			  if ( ( (vertex_ccodes[0] | vertex_ccodes[1] | vertex_ccodes[2]) & CLIP_CODE_LZ) )
			  {
			  // at this point we are ready to clip the polygon to the near 
			  // clipping plane no need to clip to the far plane since it can't 
			  // possible cause problems. We have two cases: case 1: the triangle 
			  // has 1 vertex interior to the near clipping plane and 2 vertices 
			  // exterior, OR case 2: the triangle has two vertices interior of 
			  // the near clipping plane and 1 exterior

			  // step 1: classify the triangle type based on number of vertices
			  // inside/outside
			  // case 1: easy case :)
			  if (num_verts_in == 1)
				 {
				 // we need to clip the triangle against the near clipping plane
				 // the clipping procedure is done to each edge leading away from
				 // the interior vertex, to clip we need to compute the intersection
				 // with the near z plane, this is done with a parametric equation of 
				 // the edge, once the intersection is computed the old vertex position
				 // is overwritten along with re-computing the texture coordinates, if
				 // there are any, what's nice about this case, is clipping doesn't 
				 // introduce any added vertices, so we can overwrite the old poly
				 // the other case below results in 2 polys, so at very least one has
				 // to be added to the end of the rendering list -- bummer
 
				 // step 1: find vertex index for interior vertex
				 if ( vertex_ccodes[0] == CLIP_CODE_IZ)
					{ v0 = 0; v1 = 1; v2 = 2; }
				 else 
				 if (vertex_ccodes[1] == CLIP_CODE_IZ)
					{ v0 = 1; v1 = 2; v2 = 0; }
				 else
					{ v0 = 2; v1 = 0; v2 = 1; }

				// step 2: clip each edge
				// basically we are going to generate the parametric line p = v0 + v01*t
				// then solve for t when the z component is equal to near z, then plug that
				// back into to solve for x,y of the 3D line, we could do this with high
				// level code and parametric lines, but to save time, lets do it manually

				// clip edge v0->v1
				Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v1].v, &v);                          

				// the intersection occurs when z = near z, so t = 
				t1 = ( (cam->near_clip_z - curr_poly->vertexList[v0].z) / v.z );
      
				// now plug t back in and find x,y intersection with the plane
				xi = curr_poly->vertexList[v0].x + v.x * t1;
				yi = curr_poly->vertexList[v0].y + v.y * t1;

				// now overwrite vertex with new vertex
				curr_poly->vertexList[v1].x = xi;
				curr_poly->vertexList[v1].y = yi;
				curr_poly->vertexList[v1].z = cam->near_clip_z; 
         
				// clip edge v0->v2
				Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v2].v, &v);                          

				// the intersection occurs when z = near z, so t = 
				t2 = ( (cam->near_clip_z - curr_poly->vertexList[v0].z) / v.z );
      
				// now plug t back in and find x,y intersection with the plane
				xi = curr_poly->vertexList[v0].x + v.x * t2;
				yi = curr_poly->vertexList[v0].y + v.y * t2;

				// now overwrite vertex with new vertex
				curr_poly->vertexList[v2].x = xi;
				curr_poly->vertexList[v2].y = yi;
				curr_poly->vertexList[v2].z = cam->near_clip_z; 

				// finally, we have obliterated our pre-computed normal length
				// it needs to be recomputed!!!!
 
				// build u, v
				Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v1].v, &u);
				Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v2].v, &v);

				// compute cross product
				Vector4DCross(&u, &v, &n);

				// compute length of normal accurately and store in poly nlength
				// +- epsilon later to fix over/underflows
				curr_poly->nlength = Vector4DLengthFast(&n); 

				 } // end if
			  else
			  if (num_verts_in == 2)
			  { // num_verts = 2

				 // must be the case with num_verts_in = 2 
				 // we need to clip the triangle against the near clipping plane
				 // the clipping procedure is done to each edge leading away from
				 // the interior vertex, to clip we need to compute the intersection
				 // with the near z plane, this is done with a parametric equation of 
				 // the edge, however unlike case 1 above, the triangle will be split
				 // into two triangles, thus during the first clip, we will store the 
				 // results into a new triangle at the end of the rendering list, and 
				 // then on the last clip we will overwrite the triangle being clipped

				 // step 0: copy the polygon
				 memcpy(&temp_poly, curr_poly, sizeof(POLYF4DV2) );

				 // step 1: find vertex index for exterior vertex
				 if ( vertex_ccodes[0] == CLIP_CODE_LZ)
					{ v0 = 0; v1 = 1; v2 = 2; }
				 else 
				 if (vertex_ccodes[1] == CLIP_CODE_LZ)
					{ v0 = 1; v1 = 2; v2 = 0; }
				 else
					{ v0 = 2; v1 = 0; v2 = 1; }

				 // step 2: clip each edge
				 // basically we are going to generate the parametric line p = v0 + v01*t
				 // then solve for t when the z component is equal to near z, then plug that
				 // back into to solve for x,y of the 3D line, we could do this with high
				 // level code and parametric lines, but to save time, lets do it manually

				 // clip edge v0->v1
				 Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v1].v, &v);                          

				 // the intersection occurs when z = near z, so t = 
				 t1 = ( (cam->near_clip_z - curr_poly->vertexList[v0].z) / v.z );
      
				 // now plug t back in and find x,y intersection with the plane
				 x01i = curr_poly->vertexList[v0].x + v.x * t1;
				 y01i = curr_poly->vertexList[v0].y + v.y * t1;
         
				 // clip edge v0->v2
				 Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v2].v, &v);                          

				 // the intersection occurs when z = near z, so t = 
				 t2 = ( (cam->near_clip_z - curr_poly->vertexList[v0].z) / v.z );
      
				 // now plug t back in and find x,y intersection with the plane
				 x02i = curr_poly->vertexList[v0].x + v.x * t2;
				 y02i = curr_poly->vertexList[v0].y + v.y * t2; 

				 // now we have both intersection points, we must overwrite the inplace
				 // polygon's vertex 0 with the intersection point, this poly 1 of 2 from
				 // the split

				 // now overwrite vertex with new vertex
				 curr_poly->vertexList[v0].x = x01i;
				 curr_poly->vertexList[v0].y = y01i;
				 curr_poly->vertexList[v0].z = cam->near_clip_z; 

				 // create a new polygon
				 // from the 2 intersection points and v2, this polygon will be inserted
				 // at the end of the rendering list, but for now, we are building it up
				 // in  temp_poly

				 // so leave v2 alone, but overwrite v1 with v01, and overwrite v0 with v02
				 temp_poly.vertexList[v1].x = x01i;
				 temp_poly.vertexList[v1].y = y01i;
				 temp_poly.vertexList[v1].z = cam->near_clip_z;              

				 temp_poly.vertexList[v0].x = x02i;
				 temp_poly.vertexList[v0].y = y02i;
				 temp_poly.vertexList[v0].z = cam->near_clip_z;    

 
				// poly 1 first, in place
 
				// build u, v
				Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v1].v, &u);
				Vector4DBuild(&curr_poly->vertexList[v0].v, &curr_poly->vertexList[v2].v, &v);

				// compute cross product
				Vector4DCross(&u, &v, &n);

				// compute length of normal accurately and store in poly nlength
				// +- epsilon later to fix over/underflows
				curr_poly->nlength = Vector4DLengthFast(&n); 

				// now poly 2, temp_poly
				// build u, v
				Vector4DBuild(&temp_poly.vertexList[v0].v, &temp_poly.vertexList[v1].v, &u);
				Vector4DBuild(&temp_poly.vertexList[v0].v, &temp_poly.vertexList[v2].v, &v);

				// compute cross product
				Vector4DCross(&u, &v, &n);

				// compute length of normal accurately and store in poly nlength
				// +- epsilon later to fix over/underflows
				temp_poly.nlength = Vector4DLengthFast(&n); 

				// now we are good to go, insert the polygon into list
				// if the poly won't fit, it won't matter, the function will
				// just return 0
				RenderListInsert(rend_list, &temp_poly);

				} 
        
			} 
             
		} 

    } 

} 

float Vector4DLengthFast(VECTOR4D_PTR va)
{
	int temp;  // used for _swaping
	int x,y,z; // used for algorithm

	// make sure values are all positive
	x = fabs(va->x) * 1024;
	y = fabs(va->y) * 1024;
	z = fabs(va->z) * 1024;

	// sort values
	if (y < x) _swap(x,y,temp)

	if (z < y) _swap(y,z,temp)

	if (y < x) _swap(x,y,temp)

	int dist = (z + 11 * (y >> 5) + (x >> 2) );

	return((float)(dist >> 10));

} 

float FastDistance3D(float fx, float fy, float fz)
{
	// this function computes the distance from the origin to x,y,z

	int temp;  // used for _swaping
	int x,y,z; // used for algorithm

	// make sure values are all positive
	x = fabs(fx) * 1024;
	y = fabs(fy) * 1024;
	z = fabs(fz) * 1024;

	// sort values
	if (y < x) _swap(x,y,temp)

	if (z < y) _swap(y,z,temp)

	if (y < x) _swap(x,y,temp)

	int dist = (z + 11 * (y >> 5) + (x >> 2) );

	return((float)(dist >> 10));

} 

int RenderListInsert(RENDERLIST_PTR rend_list, POLYF4DV2_PTR poly)
{
	// inserts the sent polyface POLYF4DV1 into the render list

	// step 0: are we full?
	if (rend_list->num_polys >= cpu3dMaxPolys)
	   return 0;

	// step 1: copy polygon into next opening in polygon render list

	// point pointer to polygon structure
	rend_list->poly_ptrs[rend_list->num_polys] = &rend_list->poly_data[rend_list->num_polys];

	// copy face right into array, thats it!
	memcpy((void *)&rend_list->poly_data[rend_list->num_polys],(void *)poly, sizeof(POLYF4DV2));

	// now the polygon is loaded into the next free array position, but
	// we need to fix up the links
	// test if this is the first entry
	if (rend_list->num_polys == 0)
	{
	   // set pointers to null, could loop them around though to self
	   rend_list->poly_data[0].next = NULL;
	   rend_list->poly_data[0].prev = NULL;
	} 
	else
	{
	   // first set this node to point to previous node and next node (null)
	   rend_list->poly_data[rend_list->num_polys].next = NULL;
	   rend_list->poly_data[rend_list->num_polys].prev = 
			 &rend_list->poly_data[rend_list->num_polys-1];

	   // now set previous node to point to this node
	   rend_list->poly_data[rend_list->num_polys-1].next = 
			  &rend_list->poly_data[rend_list->num_polys];
	} 

	// increment number of polys in list
	rend_list->num_polys++;

	// return successful insertion
	return 1;

} 

void RenderListCameraToPerspective(RENDERLIST_PTR rend_list, CAMERA_PTR cam)
{

	/*static float alpha;
	static float beta;*/
	static POLYF4DV2_PTR curr_poly;
	static int poly;

	/*alpha = (0.5*cam->viewport_width-0.5);
	beta  = (0.5*cam->viewport_height-0.5);*/

	for (int poly = 0; poly < rend_list->num_polys; poly++)
	{
		// get current polygon
		curr_poly = rend_list->poly_ptrs[poly];

		// is this polygon valid?
		// transform this polygon if and only if it's not clipped, not culled,
		// active, and visible
		if ((curr_poly==NULL) || !(curr_poly->state & POLY4DV2_STATE_ACTIVE) ||
			 (curr_poly->state & POLY4DV2_STATE_CLIPPED ) ||
			 (curr_poly->state & POLY4DV2_STATE_BACKFACE) )
			   continue; // move onto next poly

		// transform the vertex by the view parameters in the camera
		if ( curr_poly->vertexList[0].z != 0 )
		{
			curr_poly->vertexList[0].x = cam->view_dist*curr_poly->vertexList[0].x/curr_poly->vertexList[0].z;
			curr_poly->vertexList[0].y = cam->view_dist*curr_poly->vertexList[0].y*cam->aspect_ratio/curr_poly->vertexList[0].z;
		}

		// z = z, so no change

		// transform the vertex by the view parameters in the camera
		if ( curr_poly->vertexList[1].z != 0 )
		{
			curr_poly->vertexList[1].x = cam->view_dist*curr_poly->vertexList[1].x/curr_poly->vertexList[1].z;
			curr_poly->vertexList[1].y = cam->view_dist*curr_poly->vertexList[1].y*cam->aspect_ratio/curr_poly->vertexList[1].z;
		}

		// z = z, so no change

		// transform the vertex by the view parameters in the camera
		if ( curr_poly->vertexList[2].z != 0 )
		{
			curr_poly->vertexList[2].x = cam->view_dist*curr_poly->vertexList[2].x/curr_poly->vertexList[2].z;
			curr_poly->vertexList[2].y = cam->view_dist*curr_poly->vertexList[2].y*cam->aspect_ratio/curr_poly->vertexList[2].z;
		}

		// z = z, so no change

		//==============
		// Now to screen

		// transform the vertex by the view parameters in the camera
		/*curr_poly->vertexList[0].x = alpha + alpha*curr_poly->vertexList[0].x;
		curr_poly->vertexList[0].y = beta  - beta *curr_poly->vertexList[0].y;

		curr_poly->vertexList[1].x = alpha + alpha*curr_poly->vertexList[1].x;
		curr_poly->vertexList[1].y = beta  - beta *curr_poly->vertexList[1].y;

		curr_poly->vertexList[2].x = alpha + alpha*curr_poly->vertexList[2].x;
		curr_poly->vertexList[2].y = beta  - beta *curr_poly->vertexList[2].y;*/

	} 

} 


void RenderListPerspectiveToScreen(RENDERLIST_PTR rend_list, CAMERA_PTR cam)
{

		static float alpha;
		static float beta;
		static POLYF4DV2_PTR curr_poly;
		static int poly;

		alpha = (0.5*cam->viewport_width-0.5);
		beta  = (0.5*cam->viewport_height-0.5);

	for ( poly = 0; poly < rend_list->num_polys; poly++)
	{
		// get current polygon
		curr_poly = rend_list->poly_ptrs[poly];

		// is this polygon valid?
		if ((curr_poly==NULL) || !(curr_poly->state & POLY4DV2_STATE_ACTIVE) ||
			 (curr_poly->state & POLY4DV2_STATE_CLIPPED ) ||
			 (curr_poly->state & POLY4DV2_STATE_BACKFACE) )
			   continue; // move onto next poly

		// the vertex is in perspective normalized coords from -1 to 1
		// on each axis, simple scale them and invert y axis and project
		// to screen

		// transform the vertex by the view parameters in the camera
		curr_poly->vertexList[0].x = alpha + alpha*curr_poly->vertexList[0].x;
		curr_poly->vertexList[0].y = beta  - beta *curr_poly->vertexList[0].y;

		curr_poly->vertexList[1].x = alpha + alpha*curr_poly->vertexList[1].x;
		curr_poly->vertexList[1].y = beta  - beta *curr_poly->vertexList[1].y;

		curr_poly->vertexList[2].x = alpha + alpha*curr_poly->vertexList[2].x;
		curr_poly->vertexList[2].y = beta  - beta *curr_poly->vertexList[2].y;

		/*for ( int c = 0 ; c < 3 ; c++ )
		{
			if ( curr_poly->vertexList[c].x < 0 ) 
				curr_poly->vertexList[c].x = 0;
			if ( curr_poly->vertexList[c].x > cam->viewport_width-1 ) 
				curr_poly->vertexList[c].x = cam->viewport_width-1;
			if ( curr_poly->vertexList[c].y < 0 ) 
				curr_poly->vertexList[c].y = 0;
			if ( curr_poly->vertexList[c].y > cam->viewport_height-1 ) 
				curr_poly->vertexList[c].y = cam->viewport_height-1;
		}*/
		//

		for ( int c = 0 ; c < 3 ; c++ )
		{
			if ( curr_poly->vertexList[c].x < -cam->viewport_width*16 ) 
				SetBit ( curr_poly->state , POLY4DV2_STATE_CLIPPED );
			if ( curr_poly->vertexList[c].x > cam->viewport_width*16 ) 
				SetBit ( curr_poly->state , POLY4DV2_STATE_CLIPPED );
			if ( curr_poly->vertexList[c].y < -cam->viewport_width*16 ) 
				SetBit ( curr_poly->state , POLY4DV2_STATE_CLIPPED );
			if ( curr_poly->vertexList[c].y > cam->viewport_height*16 ) 
				SetBit ( curr_poly->state , POLY4DV2_STATE_CLIPPED );
		}


	} 

} 

void ZBufferClear(ZBUFFERV1_PTR zb, UINT data)
{
	MemSetQuad((void *)zb->zbuffer, data, zb->sizeq); 
} 

void RenderListDrawZBuffer(RENDERLIST_PTR rend_list, UCHAR *video_buffer, int lpitch, UCHAR *zbuffer, int zpitch)
{
	POLYF4DV2 face; 

	for (int poly=0; poly < rend_list->num_polys; poly++)
	{
		if (!(rend_list->poly_ptrs[poly]->state & POLY4DV2_STATE_ACTIVE) ||
			 (rend_list->poly_ptrs[poly]->state & POLY4DV2_STATE_CLIPPED ) ||
			 (rend_list->poly_ptrs[poly]->state & POLY4DV2_STATE_BACKFACE) )
		   continue; // move onto next poly
      
		   // set the vertices
		   face.vertexList[0].x = (float)rend_list->poly_ptrs[poly]->vertexList[0].x;
		   face.vertexList[0].y = (float)rend_list->poly_ptrs[poly]->vertexList[0].y;
		   face.vertexList[0].z  = (float)rend_list->poly_ptrs[poly]->vertexList[0].z;

		   face.vertexList[1].x = (float)rend_list->poly_ptrs[poly]->vertexList[1].x;
		   face.vertexList[1].y = (float)rend_list->poly_ptrs[poly]->vertexList[1].y;
		   face.vertexList[1].z  = (float)rend_list->poly_ptrs[poly]->vertexList[1].z;

		   face.vertexList[2].x = (float)rend_list->poly_ptrs[poly]->vertexList[2].x;
		   face.vertexList[2].y = (float)rend_list->poly_ptrs[poly]->vertexList[2].y;
		   face.vertexList[2].z  = (float)rend_list->poly_ptrs[poly]->vertexList[2].z;

		   DrawTriangle(&face, video_buffer, lpitch,zbuffer,zpitch);
	} 

}

void DrawTriangle(POLYF4DV2_PTR face, UCHAR *_dest_buffer, int mem_pitch, UCHAR *_zbuffer, int zpitch)
{

	int v0=0,
		v1=1,
		v2=2,
		temp=0,
		tri_type = TRI_TYPE_NONE,
		irestart = INTERP_LHS;

	int dx,dy,dyl,dyr,      // general deltas
		z,
		dz,
		xi,yi,              // the current interpolated x,y
		zi,                 // the current interpolated z
		xstart,
		xend,
		ystart,
		yrestart,
		yend,
		xl,                 
		dxdyl,              
		xr,
		dxdyr,             
		dzdyl,   
		zl,
		dzdyr,
		zr;

	int x0,y0,tz0,    // cached vertices
		x1,y1,tz1,
		x2,y2,tz2;

	USHORT *screen_ptr  = NULL,
		   *screen_line = NULL,
		   *textmap     = NULL,
		   *dest_buffer = (USHORT *)_dest_buffer;

	UINT  *z_ptr = NULL,
		  *zbuffer = (UINT *)_zbuffer;

	#ifdef DEBUG_ON
		// track rendering stats
		debug_polys_rendered_per_frame++;
	#endif

	// adjust memory pitch to words, divide by 2
	mem_pitch >>=1;

	// adjust zbuffer pitch for 32 bit alignment
	zpitch >>= 2;

	// apply fill convention to coordinates
	face->vertexList[0].x = (int)(face->vertexList[0].x+0.5);
	face->vertexList[0].y = (int)(face->vertexList[0].y+0.5);

	face->vertexList[1].x = (int)(face->vertexList[1].x+0.5);
	face->vertexList[1].y = (int)(face->vertexList[1].y+0.5);

	face->vertexList[2].x = (int)(face->vertexList[2].x+0.5);
	face->vertexList[2].y = (int)(face->vertexList[2].y+0.5);


	// first trivial clipping rejection tests 
	if (((face->vertexList[0].y < min_clip_y)  && 
		 (face->vertexList[1].y < min_clip_y)  &&
		 (face->vertexList[2].y < min_clip_y)) ||

		((face->vertexList[0].y > max_clip_y)  && 
		 (face->vertexList[1].y > max_clip_y)  &&
		 (face->vertexList[2].y > max_clip_y)) ||

		((face->vertexList[0].x < min_clip_x)  && 
		 (face->vertexList[1].x < min_clip_x)  &&
		 (face->vertexList[2].x < min_clip_x)) ||

		((face->vertexList[0].x > max_clip_x)  && 
		 (face->vertexList[1].x > max_clip_x)  &&
		 (face->vertexList[2].x > max_clip_x)))
	   return;


	// sort vertices
	if (face->vertexList[v1].y < face->vertexList[v0].y) 
		{_swap(v0,v1,temp);} 

	if (face->vertexList[v2].y < face->vertexList[v0].y) 
		{_swap(v0,v2,temp);}

	if (face->vertexList[v2].y < face->vertexList[v1].y) 
		{_swap(v1,v2,temp);}

	// now test for trivial flat sided cases
	if (FloatCompare(face->vertexList[v0].y, face->vertexList[v1].y) )
	{ 
		// set triangle type
		tri_type = TRI_TYPE_FLAT_TOP;

		// sort vertices left to right
		if (face->vertexList[v1].x < face->vertexList[v0].x) 
			{_swap(v0,v1,temp);}

	}
	else
	// now test for trivial flat sided cases
	if (FloatCompare(face->vertexList[v1].y, face->vertexList[v2].y) )
	{ 
		// set triangle type
		tri_type = TRI_TYPE_FLAT_BOTTOM;

		// sort vertices left to right
		if (face->vertexList[v2].x < face->vertexList[v1].x) 
			{_swap(v1,v2,temp);}
	
	} // end if
	else
	{
		// must be a general triangle
		tri_type = TRI_TYPE_GENERAL;

	} // end else

	// extract vertices for processing, now that we have order
	x0  = (int)(face->vertexList[v0].x+0.0);
	y0  = (int)(face->vertexList[v0].y+0.0);

	tz0 = (int)(face->vertexList[v0].z+0.5);

	x1  = (int)(face->vertexList[v1].x+0.0);
	y1  = (int)(face->vertexList[v1].y+0.0);

	tz1 = (int)(face->vertexList[v1].z+0.5);
            
	x2  = (int)(face->vertexList[v2].x+0.0);
	y2  = (int)(face->vertexList[v2].y+0.0);

	tz2 = (int)(face->vertexList[v2].z+0.5);

	// degenerate triangle
	if ( ((x0 == x1) && (x1 == x2)) || ((y0 ==  y1) && (y1 == y2)))
	   return;

	// set interpolation restart value
	yrestart = y1;

	// what kind of triangle
	if (tri_type & TRI_TYPE_FLAT_MASK)
	{

		if (tri_type == TRI_TYPE_FLAT_TOP)
		{
			// compute all deltas
			dy = (y2 - y0);

			dxdyl = ((x2 - x0)   << FIXP16_SHIFT)/dy;
			dzdyl = ((tz2 - tz0) << FIXP16_SHIFT)/dy; 

			dxdyr = ((x2 - x1)   << FIXP16_SHIFT)/dy;
			dzdyr = ((tz2 - tz1) << FIXP16_SHIFT)/dy;   

			// test for y clipping
			if (y0 < min_clip_y)
				{
				// compute overclip
				dy = (min_clip_y - y0);

				// computer new LHS starting values
				xl = dxdyl*dy + (x0  << FIXP16_SHIFT);
				zl = dzdyl*dy + (tz0 << FIXP16_SHIFT);

				// compute new RHS starting values
				xr = dxdyr*dy + (x1  << FIXP16_SHIFT);
				zr = dzdyr*dy + (tz1 << FIXP16_SHIFT);

				// compute new starting y
				ystart = min_clip_y;

		} 
		else
		{
			// no clipping
			// set starting values
			xl = (x0 << FIXP16_SHIFT);
			xr = (x1 << FIXP16_SHIFT);

			zl = (tz0 << FIXP16_SHIFT);
			zr = (tz1 << FIXP16_SHIFT);

			// set starting y
			ystart = y0;

		}

	} 
	else
	{
		// must be flat bottom

		// compute all deltas
		dy = (y1 - y0);

		dxdyl = ((x1 - x0)   << FIXP16_SHIFT)/dy;
		dzdyl = ((tz1 - tz0) << FIXP16_SHIFT)/dy; 

		dxdyr = ((x2 - x0)   << FIXP16_SHIFT)/dy;
		dzdyr = ((tz2 - tz0) << FIXP16_SHIFT)/dy;   

		// test for y clipping
		if (y0 < min_clip_y)
			{
			// compute overclip
			dy = (min_clip_y - y0);

			// computer new LHS starting values
			xl = dxdyl*dy + (x0  << FIXP16_SHIFT);
			zl = dzdyl*dy + (tz0 << FIXP16_SHIFT);

			// compute new RHS starting values
			xr = dxdyr*dy + (x0  << FIXP16_SHIFT);
			zr = dzdyr*dy + (tz0 << FIXP16_SHIFT);

			// compute new starting y
			ystart = min_clip_y;

		} 
		else
		{
			// no clipping

			// set starting values
			xl = (x0 << FIXP16_SHIFT);
			xr = (x0 << FIXP16_SHIFT);

			zl = (tz0 << FIXP16_SHIFT);
			zr = (tz0 << FIXP16_SHIFT);

			// set starting y
			ystart = y0;

		} 

	} 

	// test for bottom clip, always
	if ((yend = y2) > max_clip_y)
		yend = max_clip_y;

    // test for horizontal clipping
	if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
		(x1 < min_clip_x) || (x1 > max_clip_x) ||
		(x2 < min_clip_x) || (x2 > max_clip_x))
	{
		// clip version

		// point screen ptr to starting line
		screen_ptr = dest_buffer + (ystart * mem_pitch);

		// point zbuffer to starting line
		z_ptr = zbuffer + (ystart * zpitch);

		for (yi = ystart; yi < yend; yi++)
		{
			// compute span endpoints
			xstart = ((xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT);
			xend   = ((xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT);

			// compute starting points for u,v,w interpolants
			zi = zl + FIXP16_ROUND_UP;

			// compute u,v interpolants
			if ((dx = (xend - xstart))>0)
			{
				dz = (zr - zl)/dx;
			}
			else
			{
				dz = (zr - zl);
			} 

			///////////////////////////////////////////////////////////////////////

			// test for x clipping, LHS
			if (xstart < min_clip_x)
			{
				// compute x overlap
				dx = min_clip_x - xstart;

				// slide interpolants over
				zi+=dx*dz;
			
				// reset vars
				xstart = min_clip_x;

			} // end if
		
			// test for x clipping RHS
			if (xend > max_clip_x)
				xend = max_clip_x;

			///////////////////////////////////////////////////////////////////////

			// draw span
			for (xi=xstart; xi < xend; xi++)
			{
				// test if z of current pixel is nearer than current z buffer value
				//if (UINT(zi / ZDEPTH) < z_ptr[xi])
				if (UINT(zi) < z_ptr[xi])
				{
				   // write textel assume 5.6.5
    			   //screen_ptr[xi] = color;

				   // update z-buffer
				   //z_ptr[xi] = UINT(zi  / ZDEPTH);           
				   z_ptr[xi] = UINT(zi);           
				} 

				// interpolate u,v,w,z
				zi+=dz;
			}

			// interpolate z,x along right and left edge
			xl+=dxdyl;
			zl+=dzdyl;
	
			xr+=dxdyr;
			zr+=dzdyr;
 
			// advance screen ptr
			screen_ptr+=mem_pitch;

			// advance z-buffer ptr
			z_ptr+=zpitch;

		}

	} // end if clip
	else
	{
		// non-clip version

		// point screen ptr to starting line
		screen_ptr = dest_buffer + (ystart * mem_pitch);

		// point zbuffer to starting line
		z_ptr = zbuffer + (ystart * zpitch);

		for (yi = ystart; yi < yend; yi++)
		{
			// compute span endpoints
			xstart = ((xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT);
			xend   = ((xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT);
		
			// compute starting points for u,v,w interpolants
			zi = zl + FIXP16_ROUND_UP;
	
			// compute u,v interpolants
			if ((dx = (xend - xstart))>0)
			{
				dz = (zr - zl)/dx;
			}
			else
			{
				dz = (zr - zl);
			} 

			// draw span
			for (xi=xstart; xi < xend; xi++)
			{
				// test if z of current pixel is nearer than current z buffer value
				//if (UINT(zi / ZDEPTH) < z_ptr[xi])
				if (UINT(zi) < z_ptr[xi])
				  {
				   // write textel 5.6.5
				   //screen_ptr[xi] = color;

				   // update z-buffer
				   //z_ptr[xi] = UINT(zi / ZDEPTH);           
				   z_ptr[xi] = UINT(zi);           
				  } 


				// interpolate z
				zi+=dz;
			} 

			// interpolate x,z along right and left edge
			xl+=dxdyl;
			zl+=dzdyl;
	
			xr+=dxdyr;
			zr+=dzdyr;

			// advance screen ptr
			screen_ptr+=mem_pitch;

			// advance z-buffer ptr
			z_ptr+=zpitch;

			}

		} 

	} // end if
	//potato
else
if (tri_type==TRI_TYPE_GENERAL)
	{

	// first test for bottom clip, always
	if ((yend = y2) > max_clip_y)
		yend = max_clip_y;

	// pre-test y clipping status
	if (y1 < min_clip_y)
		{
		// compute all deltas
		// LHS
		dyl = (y2 - y1);

		dxdyl = ((x2  - x1)  << FIXP16_SHIFT)/dyl;
		dzdyl = ((tz2 - tz1) << FIXP16_SHIFT)/dyl; 

		// RHS
		dyr = (y2 - y0);	

		dxdyr = ((x2  - x0)  << FIXP16_SHIFT)/dyr;
		dzdyr = ((tz2 - tz0) << FIXP16_SHIFT)/dyr;  
		
		// compute overclip
		dyr = (min_clip_y - y0);
		dyl = (min_clip_y - y1);

		// computer new LHS starting values
		xl = dxdyl*dyl + (x1  << FIXP16_SHIFT);
		zl = dzdyl*dyl + (tz1 << FIXP16_SHIFT);

		// compute new RHS starting values
		xr = dxdyr*dyr + (x0  << FIXP16_SHIFT);
		zr = dzdyr*dyr + (tz0 << FIXP16_SHIFT);

		// compute new starting y
		ystart = min_clip_y;

		// test if we need _swap to keep rendering left to right
		if (dxdyr > dxdyl)
			{
			_swap(dxdyl,dxdyr,temp);
			_swap(dzdyl,dzdyr,temp);
			_swap(xl,xr,temp);
			_swap(zl,zr,temp);
			_swap(x1,x2,temp);
			_swap(y1,y2,temp);
			_swap(tz1,tz2,temp);
		
			// set interpolation restart
			irestart = INTERP_RHS;

			} // end if

		} // end if
	else
	if (y0 < min_clip_y)
		{
		// compute all deltas
		// LHS
		dyl = (y1 - y0);

		dxdyl = ((x1  - x0)  << FIXP16_SHIFT)/dyl;
		dzdyl = ((tz1 - tz0) << FIXP16_SHIFT)/dyl; 

		// RHS
		dyr = (y2 - y0);	

		dxdyr = ((x2  - x0)  << FIXP16_SHIFT)/dyr;
		dzdyr = ((tz2 - tz0) << FIXP16_SHIFT)/dyr;  
		
		// compute overclip
		dy = (min_clip_y - y0);

		// computer new LHS starting values
		xl = dxdyl*dy + (x0  << FIXP16_SHIFT);
		zl = dzdyl*dy + (tz0 << FIXP16_SHIFT);

		// compute new RHS starting values
		xr = dxdyr*dy + (x0  << FIXP16_SHIFT);
		zr = dzdyr*dy + (tz0 << FIXP16_SHIFT);

		// compute new starting y
		ystart = min_clip_y;

		// test if we need _swap to keep rendering left to right
		if (dxdyr < dxdyl)
			{
			_swap(dxdyl,dxdyr,temp);
			_swap(dzdyl,dzdyr,temp);
			_swap(xl,xr,temp);
			_swap(zl,zr,temp);
			_swap(x1,x2,temp);
			_swap(y1,y2,temp);
			_swap(tz1,tz2,temp);
		
			// set interpolation restart
			irestart = INTERP_RHS;

			} // end if

		} // end if
	else
		{
		// no initial y clipping
	
		// compute all deltas
		// LHS
		dyl = (y1 - y0);

		dxdyl = ((x1  - x0)  << FIXP16_SHIFT)/dyl;
		dzdyl = ((tz1 - tz0) << FIXP16_SHIFT)/dyl; 

		// RHS
		dyr = (y2 - y0);	

		dxdyr = ((x2 - x0)   << FIXP16_SHIFT)/dyr;
		dzdyr = ((tz2 - tz0) << FIXP16_SHIFT)/dyr;

		// no clipping y

		// set starting values
		xl = (x0 << FIXP16_SHIFT);
		xr = (x0 << FIXP16_SHIFT);

		zl = (tz0 << FIXP16_SHIFT);

		zr = (tz0 << FIXP16_SHIFT);

		// set starting y
		ystart = y0;

		// test if we need _swap to keep rendering left to right
		if (dxdyr < dxdyl)
			{
			_swap(dxdyl,dxdyr,temp);
			_swap(dzdyl,dzdyr,temp);
			_swap(xl,xr,temp);
			_swap(zl,zr,temp);
			_swap(x1,x2,temp);
			_swap(y1,y2,temp);
			_swap(tz1,tz2,temp);
		
			// set interpolation restart
			irestart = INTERP_RHS;

			} // end if

		} // end else

    // test for horizontal clipping
	if ((x0 < min_clip_x) || (x0 > max_clip_x) ||
		(x1 < min_clip_x) || (x1 > max_clip_x) ||
		(x2 < min_clip_x) || (x2 > max_clip_x))
	{
    // clip version
	// x clipping	

	// point screen ptr to starting line
	screen_ptr = dest_buffer + (ystart * mem_pitch);

    // point zbuffer to starting line
    z_ptr = zbuffer + (ystart * zpitch);

	for (yi = ystart; yi < yend; yi++)
		{
		// compute span endpoints
		xstart = ((xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT);
		xend   = ((xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT);
		
		// compute starting points for z interpolants
		zi = zl + FIXP16_ROUND_UP;
	
		// compute z interpolants
		if ((dx = (xend - xstart))>0)
			{
			dz = (zr - zl)/dx;
			} // end if
		else
			{
			dz = (zr - zl);
			} // end else

		///////////////////////////////////////////////////////////////////////

		// test for x clipping, LHS
		if (xstart < min_clip_x)
			{
			// compute x overlap
			dx = min_clip_x - xstart;

			// slide interpolants over
			zi+=dx*dz;
			
			// set x to left clip edge
			xstart = min_clip_x;

			} // end if
		
		// test for x clipping RHS
		if (xend > max_clip_x)
			xend = max_clip_x;

		///////////////////////////////////////////////////////////////////////

		// draw span
		for (xi=xstart; xi < xend; xi++)
			{
            // test if z of current pixel is nearer than current z buffer value
            //if (UINT(zi / ZDEPTH) < z_ptr[xi])
            if (UINT(zi) < z_ptr[xi])
               {
			   // write textel assume 5.6.5
              //screen_ptr[xi] = color;

               // update z-buffer
               //z_ptr[xi] = UINT(zi / ZDEPTH);           
               z_ptr[xi] = UINT(zi);           
               } // end if

			// interpolate z
			zi+=dz;
			} // end for xi

		// interpolate z,x along right and left edge
		xl+=dxdyl;
		zl+=dzdyl;
	
		xr+=dxdyr;
		zr+=dzdyr;

		// advance screen ptr
		screen_ptr+=mem_pitch;

        // advance z-buffer ptr
        z_ptr+=zpitch;

		// test for yi hitting second region, if so change interpolant
		if (yi==yrestart)
			{
		    // test interpolation side change flag

			if (irestart == INTERP_LHS)
			{
			// LHS
			dyl = (y2 - y1);	

			dxdyl = ((x2 - x1)   << FIXP16_SHIFT)/dyl;
			dzdyl = ((tz2 - tz1) << FIXP16_SHIFT)/dyl;  

			// set starting values
			xl = (x1  << FIXP16_SHIFT);
			zl = (tz1 << FIXP16_SHIFT);

			// interpolate down on LHS to even up
			xl+=dxdyl;
			zl+=dzdyl;
			} // end if
			else
			{
			// RHS
			dyr = (y1 - y2);	

			dxdyr = ((x1 - x2)   << FIXP16_SHIFT)/dyr;
			dzdyr = ((tz1 - tz2) << FIXP16_SHIFT)/dyr;   

			// set starting values
			xr = (x2  << FIXP16_SHIFT);
			zr = (tz2 << FIXP16_SHIFT);

			// interpolate down on RHS to even up
			xr+=dxdyr;
			zr+=dzdyr;
		
			} // end else

			} // end if

		} // end for y

	} // end if
	else
	{
	// no x clipping
	// point screen ptr to starting line
	screen_ptr = dest_buffer + (ystart * mem_pitch);

    // point zbuffer to starting line
    z_ptr = zbuffer + (ystart * zpitch);

	for (yi = ystart; yi < yend; yi++)
		{
		// compute span endpoints
		xstart = ((xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT);
		xend   = ((xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT);
		
		// compute starting points for u,v,w,z interpolants
		zi = zl + FIXP16_ROUND_UP;
	
		// compute u,v interpolants
		if ((dx = (xend - xstart))>0)
			{
			dz = (zr - zl)/dx;
			} // end if
		else
			{
			dz = (zr - zl);
			} // end else

		// draw span
		for (xi=xstart; xi < xend; xi++)
			{
            // test if z of current pixel is nearer than current z buffer value
            //if (UINT(zi / ZDEPTH) < z_ptr[xi])
            if (UINT(zi) < z_ptr[xi])
               {
			   // write textel assume 5.6.5
    		   //screen_ptr[xi] = color;

               // update z-buffer
               //z_ptr[xi] = UINT(zi / ZDEPTH);           
               z_ptr[xi] = UINT(zi);           
               } // end if

			// interpolate z
			zi+=dz;
			} // end for xi

		// interpolate x,z along right and left edge
		xl+=dxdyl;
		zl+=dzdyl;
	
		xr+=dxdyr;
		zr+=dzdyr;

		// advance screen ptr
		screen_ptr+=mem_pitch;

        // advance z-buffer ptr
        z_ptr+=zpitch;

		// test for yi hitting second region, if so change interpolant
		if (yi==yrestart)
			{
			// test interpolation side change flag

			if (irestart == INTERP_LHS)
			{
			// LHS
			dyl = (y2 - y1);	

			dxdyl = ((x2 - x1)   << FIXP16_SHIFT)/dyl;
			dzdyl = ((tz2 - tz1) << FIXP16_SHIFT)/dyl;   

			// set starting values
			xl = (x1  << FIXP16_SHIFT);
			zl = (tz1 << FIXP16_SHIFT);

			// interpolate down on LHS to even up
			xl+=dxdyl;
			zl+=dzdyl;
			} // end if
			else
			{
			// RHS
			dyr = (y1 - y2);	

			dxdyr = ((x1 - x2)   << FIXP16_SHIFT)/dyr;
			dzdyr = ((tz1 - tz2) << FIXP16_SHIFT)/dyr;   

			// set starting values
			xr = (x2  << FIXP16_SHIFT);
			zr = (tz2 << FIXP16_SHIFT);

			// interpolate down on RHS to even up
			xr+=dxdyr;
			zr+=dzdyr;
			} // end else

			} // end if

		} // end for y

  	} // end else	

	} // end if

}


void CameraBuildMatrix(CAMERA_PTR cam, int cam_rot_seq)
{
	MATRIX4X4 mt_inv,  // inverse camera translation matrix
			  mx_inv,  // inverse camera x axis rotation matrix
			  my_inv,  // inverse camera y axis rotation matrix
			  mz_inv,  // inverse camera z axis rotation matrix
			  mrot,    // concatenated inverse rotation matrices
			  mtmp;    // temporary working matrix


	// step 1: create the inverse translation matrix for the camera
	// position
	MatInit4X4(&mt_inv, 1,    0,     0,     0,
						  0,    1,     0,     0,
						  0,    0,     1,     0,
						  -cam->pos.x, -cam->pos.y, -cam->pos.z, 1);

	// step 2: create the inverse rotation sequence for the camera
	// rember either the transpose of the normal rotation matrix or
	// plugging negative values into each of the rotations will result
	// in an inverse matrix

	// first compute all 3 rotation matrices

	// extract out euler angles
	float theta_x = cam->dir.x;
	float theta_y = cam->dir.y;
	float theta_z = cam->dir.z;

	// compute the sine and cosine of the angle x
	float cos_theta = FastCos(theta_x);  // no change since cos(-x) = cos(x)
	float sin_theta = -FastSin(theta_x); // sin(-x) = -sin(x)

	// set the matrix up 
	MatInit4X4(&mx_inv, 1,    0,         0,         0,
						  0,    cos_theta, sin_theta, 0,
						  0,   -sin_theta, cos_theta, 0,
						  0,    0,         0,         1);

	// compute the sine and cosine of the angle y
	cos_theta = FastCos(theta_y);  // no change since cos(-x) = cos(x)
	sin_theta = -FastSin(theta_y); // sin(-x) = -sin(x)

	// set the matrix up 
	MatInit4X4(&my_inv,cos_theta, 0, -sin_theta, 0,  
						 0,         1,  0,         0,
						 sin_theta, 0,  cos_theta,  0,
						 0,         0,  0,          1);

	// compute the sine and cosine of the angle z
	cos_theta = FastCos(theta_z);  // no change since cos(-x) = cos(x)
	sin_theta = -FastSin(theta_z); // sin(-x) = -sin(x)

	// set the matrix up 
	MatInit4X4(&mz_inv, cos_theta, sin_theta, 0, 0,  
						 -sin_theta, cos_theta, 0, 0,
						  0,         0,         1, 0,
						  0,         0,         0, 1);

	// now compute inverse camera rotation sequence
	switch(cam_rot_seq)
	{
		case CAM_ROT_SEQ_XYZ:
		{
			MatMul4X4(&mx_inv, &my_inv, &mtmp);
			MatMul4X4(&mtmp, &mz_inv, &mrot);
		} break;

		case CAM_ROT_SEQ_YXZ:
		{
			MatMul4X4(&my_inv, &mx_inv, &mtmp);
			MatMul4X4(&mtmp, &mz_inv, &mrot);
		} break;

		case CAM_ROT_SEQ_XZY:
		{
			MatMul4X4(&mx_inv, &mz_inv, &mtmp);
			MatMul4X4(&mtmp, &my_inv, &mrot);
		} break;

		case CAM_ROT_SEQ_YZX:
		{
			MatMul4X4(&my_inv, &mz_inv, &mtmp);
			MatMul4X4(&mtmp, &mx_inv, &mrot);
		} break;

		case CAM_ROT_SEQ_ZYX:
		{
			MatMul4X4(&mz_inv, &my_inv, &mtmp);
			MatMul4X4(&mtmp, &mx_inv, &mrot);
		} break;

		case CAM_ROT_SEQ_ZXY:
		{
			MatMul4X4(&mz_inv, &mx_inv, &mtmp);
			MatMul4X4(&mtmp, &my_inv, &mrot);

		} break;

		default: break;
	} 

	// now mrot holds the concatenated product of inverse rotation matrices
	// multiply the inverse translation matrix against it and store in the 
	// camera objects' camera transform matrix
	MatMul4X4(&mt_inv, &mrot, &cam->mcam);

} 

void MatInit4X4(MATRIX4X4_PTR ma, 
                 float m00, float m01, float m02, float m03,
                 float m10, float m11, float m12, float m13,
                 float m20, float m21, float m22, float m23,
                 float m30, float m31, float m32, float m33)

{
	// this function fills a 4x4 matrix with the sent data in 
	// row major form
	ma->M00 = m00; ma->M01 = m01; ma->M02 = m02; ma->M03 = m03;
	ma->M10 = m10; ma->M11 = m11; ma->M12 = m12; ma->M13 = m13;
	ma->M20 = m20; ma->M21 = m21; ma->M22 = m22; ma->M23 = m23;
	ma->M30 = m30; ma->M31 = m31; ma->M32 = m32; ma->M33 = m33;

} 

float FastSin(float theta)
{
	// convert angle to 0-359
	theta = fmodf(theta,360);

	// make angle positive
	if (theta < 0) theta+=360.0;

	// compute floor of theta and fractional part to interpolate
	int theta_int    = (int)theta;
	float theta_frac = theta - theta_int;

	// now compute the value of sin(angle) using the lookup tables
	// and interpolating the fractional part, note that if theta_int
	// is equal to 359 then theta_int+1=360, but this is fine since the
	// table was made with the entries 0-360 inclusive
	return(sin_look[theta_int] + 
		   theta_frac*(sin_look[theta_int+1] - sin_look[theta_int]));

} 

///////////////////////////////////////////////////////////////

float FastCos(float theta)
{

	// convert angle to 0-359
	theta = fmodf(theta,360);

	// make angle positive
	if (theta < 0) theta+=360.0;

	// compute floor of theta and fractional part to interpolate
	int theta_int    = (int)theta;
	float theta_frac = theta - theta_int;

	// now compute the value of sin(angle) using the lookup tables
	// and interpolating the fractional part, note that if theta_int
	// is equal to 359 then theta_int+1=360, but this is fine since the
	// table was made with the entries 0-360 inclusive
	return(cos_look[theta_int] + 
		   theta_frac*(cos_look[theta_int+1] - cos_look[theta_int]));

}

void MatMul4X4(MATRIX4X4_PTR ma, 
                 MATRIX4X4_PTR mb,
                 MATRIX4X4_PTR mprod)
{

	for (int row=0; row<4; row++)
	{
		for (int col=0; col<4; col++)
		{
			// compute dot product from row of ma 
			// and column of mb

			float sum = 0; // used to hold result

			for (int index=0; index<4; index++)
			{
				 // add in next product pair
				 sum+=(ma->M[row][index]*mb->M[index][col]);
			} 

			// insert resulting row,col element
			mprod->M[row][col] = sum;

		} 

	} 

} 

//=======================================================================================
//=======================================================================================
