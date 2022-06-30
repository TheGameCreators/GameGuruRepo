//-----------------------------------------------------------------------------
// File: OpenGLExample.cpp
//  Date: 3.15.2008
//      The Opengl workspace is compiled and built from:
//			The "Glut" SDK version 3.7.6
//			http://www.xmission.com/~nate/glut.html
// glext.h: http://www.opengl.org/registry/api/glext.h
// wglext.h: http://www.opengl.org/registry/api/wglext.h
//
//	Date: 9.3.2010
//      1: Workspace now offers an example of SidexSide stereoscopy.
//		2: VGA adapter detection is still implemented, but no longer used by the program
//		3: Rendering method separated into distinct Wrap and VR sections for demonstrating
//		   their distinct techniques.
//
//		Wrap920:
//		1: Set viewport to left half of the screen when rendering left eye, right side 
//         when rendering right eye
//		2: Only swap buffers after drawing both eyes
//		3: No need to synchronize with the eyewear through code
//		4: Application will be forced to fullscreen when SidexSide stereo is being used
//			
//  Date: 10.24.2008
//      1: Now demonstrates user to hot-swap without losing tracking, but stereo cannot be 
//      reenabled after swap
//		2: Addition of Internal driver filtering.
//		3: Addition of Windowed mode stereoscopy.
//
//	Date: 3.31.2008 
//		Definitive method for syncronizing FreezeFrame Stereoscopy from UserMode.
//		1: glFinish() will provide for a mechanism to achieve Syncronized left/right 
//         Frame updates with the VR920s video engine.
//		2: The application calls glutSwapBuffers() followed by glFinish() to be certain 
//         the frame will be scanned out the VR920's vga adapter on the next 
//         Vsync period after the call to IWRSTEREO_SetLR(...) is executed.
//      3: In order to achieve 30-60FPS you must enable triple buffering in your video
//         card's control panel.
//		
//	Stereoscopy Rules:
//	1: Never leave the VR920 in FreezeFrame stereoscopy prior to exiting the application.
//		Unless it is the intention of the developer.
//  2: During long periods of non-scene rendering times. 
//		Consider placing the VR920 into mono mode. 
//		Ex: scene loading period...
//
//	Head Tracking Rules:
//	1: Always provide for a Non-Filtered method of head tracking.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#define IWEAR_ONETIME_DEFINITIONS
#include <iWearSDK.h>
//#include "IWRsdk.h"
#include <GL/glut.h> // glut library includes standard gl and glu libraries as well
#include <GL/glext.h> 
#include <GL/wglext.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
// Block the console window at program start
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#define OPENGL_WINDOW_NAME		"Vuzix iWear SDK OpenGL Example"
#define SEPARATION_CHANGE		0.01f
#define MINIMUM_SEPARATION		0.01f
#define DEFAULT_SEPARATION		0.25f
#define FOCAL_LENGTH_CHANGE		0.5f
#define DEFAULT_FOCAL_LENGTH	10.0f
#define MINIMUM_FOCAL_LENGTH	4.0f
#define DEGREESTORADIANS		0.0174532925f

PFNGLGENQUERIESARBPROC			glGenQueriesARB        = NULL;
PFNGLDELETEQUERIESARBPROC		glDeleteQueriesARB     = NULL;
PFNGLISQUERYARBPROC				glIsQueryARB           = NULL;
PFNGLBEGINQUERYARBPROC			glBeginQueryARB        = NULL;
PFNGLENDQUERYARBPROC			glEndQueryARB          = NULL;
PFNGLGETQUERYIVARBPROC			glGetQueryivARB        = NULL;
PFNGLGETQUERYOBJECTIVARBPROC	glGetQueryObjectivARB  = NULL;
PFNGLGETQUERYOBJECTUIVARBPROC	glGetQueryObjectuivARB = NULL;

enum							{ IWR_NOT_CONNECTED=-1, IWR_IS_CONNECTED };
enum							{ LEFT_EYE=0, RIGHT_EYE, MONO_EYES };
enum							{ NO_FILTER=0, APPLICATION_FILTER, INTERNAL_FILTER };
enum							{ MONO, PROGRESSIVE_SCAN, SIDE_X_SIDE };
// Provide for a filtering mechanism on the VR920 tracker reading.
extern void	IWRFilterTracking( long *yaw, long *pitch, long *roll );
void		createGLWindow();
void		SetStereoViewport(int eye);
void		UpdateTracking();

#pragma region Global Variables
//------------------------------------
// Globals
//------------------------------------
float		g_EyeSeparation = DEFAULT_SEPARATION;// Intraocular Distance: aka, Distance between left and right cameras.
float		g_FocalLength	= DEFAULT_FOCAL_LENGTH;	// Screen projection plane: aka, focal length(distance to front of virtual screen).

GLint		g_screenWidth	= 640, g_screenHeight = 480, g_bpp = 32;
int			g_winID = 0;
HANDLE		g_StereoHandle	= INVALID_HANDLE_VALUE;
bool		g_StereoEnabled = false;
bool		g_tracking		= false;		// True = enable head tracking
int			g_Filtering		= 0;			// Provide very primitive filtering process.
bool		g_RenderStats	= true;			// Show help status.
bool		g_benchmark		= false;		// True, when running benchmark capturing.
bool		g_fullscreen    = true;
bool		g_noQuery		= true;
GLfloat		g_rotationH		= 0.0f;		// horizontal rotation for the spheres
GLfloat		g_rotationV		= 0.0f;		// vertical rotation for the spheres
GLfloat		g_rotationRed	= 0.0f;
GLfloat		g_rotationBlue	= 0.0f;
GLint		g_numSpheres	= 0;
GLint		g_framecounter	= 0, g_timebase = 0;
GLfloat		g_fps			= 0.0;
GLfloat		g_fYaw = 0.0f,  g_fPitch = 0.0f, g_fRoll = 0.0f;
GLdouble	g_CameraXPosition=0.0f, g_CameraYPosition=0.0f, g_CameraZPosition=0.0f;
GLuint		g_leftQuery		= -1;
GLuint		g_rightQuery	= -1;
int         g_IWRStereoscopyMode;
int			Pid				= 0;
DWORD		PDetails		= 0;
bool		usingWrap		= false;
int			iwr_status		= IWR_OK;
int			g_ProdSubID		= 0;
int			g_ProdAspect	= 0;
int			g_ProdTrackerStyle	= 0;

#pragma endregion

#pragma region Rendering
/*
* Function for drawing to the screen; must set glRasterPos and color prior 
* to calling this function
*/
void print_bitmap_string(void* font, char* s)
{
   if (s && strlen(s)) {
      while (*s) {
         glutBitmapCharacter(font, *s);
         s++;
      }
   }
}

/*
* Function to handle resizing the window
*/
void reshape(int w, int h)
{
    if (h==0)
        h=1; //prevent divide by 0
	g_screenWidth  = w;
	g_screenHeight = h;
    glViewport(0, 0, w, h);
}

/*
* Function to handle keyboard input
*/
void keyboard(GLubyte key, GLint xMouse, GLint yMouse)
{
	switch(key){
		case 'q': // exit with q or Escape
		case VK_ESCAPE:
			exit(0);
			break;
		case 'w':
			g_rotationV -= 0.5;
			break;
		case 's':
			g_rotationV += 0.5;
			break;
		case 'a':
			g_rotationH -= 0.5;
			break;
		case 'd':
			g_rotationH += 0.5;
			break;
		case 'b': // Start/stop benchmark scene
			if (!g_benchmark){
				// Benchmarking on.
				// Stereo on, Center the view, Remove the rotations, Reset to default view position.
				g_numSpheres	= 100;
				g_StereoEnabled = true;
				g_RenderStats	= true;
				g_tracking		= false; 
				g_Filtering		= false;
				g_EyeSeparation = DEFAULT_SEPARATION;
				g_FocalLength	= DEFAULT_FOCAL_LENGTH;	
				g_rotationH		= g_rotationV = g_rotationRed = g_rotationBlue = 0.0;
				g_CameraXPosition=0.0f, g_CameraYPosition=0.0f, g_CameraZPosition=0.0f;
				g_benchmark		= true;
				if( g_StereoHandle != INVALID_HANDLE_VALUE )
					IWRSTEREO_SetStereo( g_StereoHandle, IWR_STEREO_MODE );
			} else {
				// Benchmarking off.
				g_numSpheres	= 0;
				g_benchmark		= false;
			}
			break;
		case 'f': // switch between filtering and not filtering
			if (IWRSetFilterState != NULL )
				IWRSetFilterState(FALSE);
			g_Filtering++;

			if (g_Filtering > INTERNAL_FILTER)
				g_Filtering = NO_FILTER;

			if (g_Filtering == INTERNAL_FILTER){ 
				if( IWRSetFilterState != NULL ){
					IWRSetFilterState(TRUE);						
				} else {
					g_Filtering = NO_FILTER;
				}
			}
			break;
		case 't': // switch between tracking and not tracking
			g_tracking = !g_tracking; 
			break;
		case VK_SPACE: // defaults
				g_numSpheres	= 0;
				g_EyeSeparation = DEFAULT_SEPARATION;
				g_FocalLength	= DEFAULT_FOCAL_LENGTH;	
				g_rotationH     = 0.0f;
				g_rotationV     = 0.0f;
				g_CameraXPosition=0.0f, g_CameraYPosition=0.0f, g_CameraZPosition=0.0f;
				g_benchmark		= false;
			break;
		case VK_RETURN: // enter, switched between stereo and mono viewing
			g_timebase = 0;
			g_StereoEnabled = !g_StereoEnabled;
			if( g_StereoHandle == INVALID_HANDLE_VALUE ) {
				g_StereoEnabled = false;
			}else{
				if (g_StereoEnabled)
					IWRSTEREO_SetStereo( g_StereoHandle, IWR_STEREO_MODE );
				else 
					IWRSTEREO_SetStereo( g_StereoHandle, IWR_MONO_MODE );				
			}
			glutPostRedisplay();
			break;
		}
}

/*
* Handle special keyboard functions like arrow keys, control, alt, etc.
*/
void specKeyboard(int key, int x, int y){
	switch(key){
		case GLUT_KEY_PAGE_UP: // "page up" increases the nuber of purple spheres
			if (!g_benchmark)
				g_numSpheres++;
			break;
		case GLUT_KEY_PAGE_DOWN: // "page down" decreases the number of spheres
			if (!g_benchmark) {
				g_numSpheres--;
				if( g_numSpheres < 0 )
					g_numSpheres = 0;
			}
			break;
		case GLUT_KEY_LEFT: // left decreases the g_EyeSeparation
			if (g_EyeSeparation > MINIMUM_SEPARATION)
				g_EyeSeparation -= SEPARATION_CHANGE;
			break;
		case GLUT_KEY_RIGHT: // right increases the g_EyeSeparation
			g_EyeSeparation += SEPARATION_CHANGE;
			break;
		case GLUT_KEY_DOWN: // down decreases the g_FocalLength
			if (g_FocalLength > MINIMUM_FOCAL_LENGTH)
				g_FocalLength -= FOCAL_LENGTH_CHANGE;
			break;
		case GLUT_KEY_UP: // up increases the g_FocalLength
			g_FocalLength += FOCAL_LENGTH_CHANGE;
			break;
		case GLUT_KEY_F1: // toggle renderstats display
			g_RenderStats = !g_RenderStats;
			break;
		case GLUT_KEY_F2: // center of yellow sphere.
			g_CameraXPosition=0.0f; g_CameraYPosition=0.0f; g_CameraZPosition=7.0f;
			break;
		case GLUT_KEY_F3:
			g_CameraXPosition=-7.0f; g_CameraYPosition=7.0f; g_CameraZPosition=-3.0f;
			break;
		case GLUT_KEY_F4:
			g_CameraXPosition=0.0f; g_CameraYPosition=0.0f; g_CameraZPosition=15.0f;
			break;
		case GLUT_KEY_F5: // origin; default.
			g_CameraXPosition=0.0f; g_CameraYPosition=0.0f; g_CameraZPosition=0.0f;
			break;
		case GLUT_KEY_F11: // toggle fullscreen
			if (g_fullscreen && g_IWRStereoscopyMode == PROGRESSIVE_SCAN){
				g_fullscreen = false;
				createGLWindow();
			} else if (!g_fullscreen){
				g_fullscreen = true;
				createGLWindow();
				glutFullScreen();
			}
			break;
	}
}

//
// Set up the perspective frustum based on window properties
// StereoScopic Mode: 
//		Sets up an off-axis projection considering Eye separation and focal length.
// MonoScopic Mode:
//		Uses the same projection method with parameters to center the monoscopic 
//      view along the viewing axis.
//
void SetViewingFrustum( int Eye )
{
GLfloat		fovy	= 45.0;                                          //field of view in y-axis
GLfloat		aspect	= float(g_screenWidth)/float(g_screenHeight);    //screen aspect ratio
GLfloat		nearZ	= 1.0;                                        //near clipping plane
GLfloat		farZ	= 1000.0;                                      //far clipping plane
GLfloat		top		= nearZ * tan( DEGREESTORADIANS * fovy / 2);     
GLdouble	right	= aspect * top;                             
GLdouble	frustumshift= (g_EyeSeparation / 2) * nearZ / g_FocalLength;
GLdouble	leftfrustum, rightfrustum, bottomfrustum, topfrustum;
GLfloat		modeltranslation;

	switch( Eye ) {
		case LEFT_EYE:
			// Setup left viewing frustum.
			topfrustum		= top;
			bottomfrustum	= -top;
			leftfrustum		= -right + frustumshift;
			rightfrustum	= right + frustumshift;
			modeltranslation= g_EyeSeparation / 2;
		break;
		case RIGHT_EYE:
			// Setup right viewing frustum.
			topfrustum		= top;
			bottomfrustum	= -top;
			leftfrustum		= -right - frustumshift;
			rightfrustum	= right - frustumshift;
			modeltranslation= -g_EyeSeparation / 2;
		break;
		case MONO_EYES:
			// Setup mono viewing frustum.
			topfrustum		= top;
			bottomfrustum	= -top;
			leftfrustum		= -right;
			rightfrustum	= right;
			modeltranslation= 0.0f;
		break;
	}
	// Initialize off-axis projection if in stereoscopy mode.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();                  
	glFrustum(leftfrustum, rightfrustum, bottomfrustum, topfrustum,	nearZ, farZ);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Translate Camera to cancel parallax.
	glTranslated(modeltranslation, 0.0f, 0.0f);
	// Add headtracking from VR920 tracker.
	if (g_tracking){		// if tracking is enabled, apply pitch, roll, and yaw.
		glRotated(-g_fPitch, 1.0, 0.0, 0.0);
		glRotated(g_fRoll, 0.0, 0.0, 1.0);
		glRotated(-g_fYaw, 0.0, 1.0, 0.0);
		}
	// Translate camera to global viewing position. 
	glTranslated(g_CameraXPosition, g_CameraYPosition, g_CameraZPosition);
}

/*
* Render the geometry of the world, for this Eye.
*  Including status output, if enabled.
*/
void RenderWorld(int Eye)
{
#define INFO_FIELDS	14
TCHAR	*str_info[INFO_FIELDS] = {
	"STATUS IN RIGHT EYE ONLY? (Both eyes in mono mode)",
	"   Right/Left = +/- Eye separation",
	"   Up/Down = +/- Focal length",
	"   PgUp/PgDown = +/- # of Spheres",
	"   Enter = Toggle Stereo/Mono",
	"   t = Toggle tracking",
	"   f = Toggle filtered tracking",
	"   W,S,A,D = Change orientations",
	"   ESC,q = Exit application",
	"   b = Benchmarking mode",
	"   Space = Reset Defaults",
	"   F1 = Toggle status info",
	"   F2,F3,F4,F5= Set camera viewpoint",
	"   F11 = Toggle fullscreen",
};
TCHAR	str[MAX_PATH];
	// Select appropriate back buffer.
	if( g_IWRStereoscopyMode != SIDE_X_SIDE ){
		switch( Eye ) {
			case LEFT_EYE:
				glDrawBuffer(GL_BACK_LEFT);                              
			break;
			case RIGHT_EYE:
				glDrawBuffer(GL_BACK_RIGHT);                             
			break;
			case MONO_EYES:
				glDrawBuffer(GL_BACK);                             
			break;
		}
	} else {
		glDrawBuffer(GL_BACK);
	}
	      
	// Apply a global translation at entry; translate to screenplane
	glPushMatrix();
	// Move all ojects in the scene to same zDepth.
	glTranslatef( 0.0, 0.0, -7.0f );                      
	// start applying wasd rotation matrices
	glPushMatrix(); 
	glRotatef(g_rotationH, 0.0, 1.0, 0.0); 
	glRotatef(g_rotationV, 1.0, 0.0, 0.0); 
	glColor3f( 1.0f, 1.0f, 0.0f);
	glutWireSphere(2, 10, 10); // draw the main yellow sphere

	glPushMatrix();
		glTranslatef(2.0, 0.0, -5.0);
		glRotatef(-g_rotationRed, 0.0, 1.0, 0.0);
		glColor3f(1.0, 0.0, 0.0);
		glutWireSphere(2, 10, 10); // draw a red sphere further away
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-2.0, 2.0, -10.0);
		glRotatef(g_rotationBlue, 0.0, 1.0, 0.0);
		glColor3f(0.0, 0.0, 1.0);
		glutWireSphere(2, 10, 10); // draw a blue sphere further away and higher up
	glPopMatrix();
	// stop applying wasd rotation matrices
	glPopMatrix(); 

	glColor3f( 0.0, 0.4, 0.0); // set color to a darker green
	glBegin(GL_QUADS); // regular OGL primitives need these glBegin() and glEnd() calls
		glVertex3f(-9.0, -10.0, 50.0); // floor
		glVertex3f(-9.0, -10.0, -50.0);
		glVertex3f(9.0, -10.0, -50.0);
		glVertex3f(9.0, -10.0, 50.0);

		glVertex3f(-10.0, -9.0, 50.0); // left side
		glVertex3f(-10.0, -9.0, -50.0);
		glVertex3f(-10.0, 9.0, -50.0);
		glVertex3f(-10.0, 9.0, 50.0);

		glVertex3f(10.0, -9.0, 50.0); // right side
		glVertex3f(10.0, -9.0, -50.0);
		glVertex3f(10.0, 9.0, -50.0);
		glVertex3f(10.0, 9.0, 50.0);
	glEnd();
	
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_STRIP); // left lines
		glVertex3f( -9.99, -8.0, -50.0); 
		glVertex3f( -9.99, -8.0, 50.0);
		glVertex3f( -9.99, -6.0, 50.0);
		glVertex3f( -9.99, -6.0, -50.0);
		glVertex3f( -9.99, -4.0, -50.0);
		glVertex3f( -9.99, -4.0, 50.0);
		glVertex3f( -9.99, -2.0, 50.0);
		glVertex3f( -9.99, -2.0, -50.0);
		glVertex3f( -9.99, 0.0, -50.0);
		glVertex3f( -9.99, 0.0, 50.0);

		glVertex3f( -9.99, 2.0, 50.0);
		glVertex3f( -9.99, 2.0, -50.0);
		glVertex3f( -9.99, 4.0, -50.0);
		glVertex3f( -9.99, 4.0, 50.0);
		glVertex3f( -9.99, 6.0, 50.0);
		glVertex3f( -9.99, 6.0, -50.0);
		glVertex3f( -9.99, 8.0, -50.0);
		glVertex3f( -9.99, 8.0, 50.0);
	glEnd();

	glBegin(GL_LINE_STRIP); // right lines
		glVertex3f( 9.99, -8.0, -50.0); 
		glVertex3f( 9.99, -8.0, 50.0);
		glVertex3f( 9.99, -6.0, 50.0);
		glVertex3f( 9.99, -6.0, -50.0);
		glVertex3f( 9.99, -4.0, -50.0);
		glVertex3f( 9.99, -4.0, 50.0);
		glVertex3f( 9.99, -2.0, 50.0);
		glVertex3f( 9.99, -2.0, -50.0);
		glVertex3f( 9.99, 0.0, -50.0);
		glVertex3f( 9.99, 0.0, 50.0);

		glVertex3f( 9.99, 2.0, 50.0);
		glVertex3f( 9.99, 2.0, -50.0);
		glVertex3f( 9.99, 4.0, -50.0);
		glVertex3f( 9.99, 4.0, 50.0);
		glVertex3f( 9.99, 6.0, 50.0);
		glVertex3f( 9.99, 6.0, -50.0);
		glVertex3f( 9.99, 8.0, -50.0);
		glVertex3f( 9.99, 8.0, 50.0);
	glEnd();

	glBegin(GL_LINE_STRIP); // bottom lines
		glVertex3f( -8.0, -9.99, -50.0); 
		glVertex3f( -8.0, -9.99, 50.0);
		glVertex3f( -6.0, -9.99, 50.0);
		glVertex3f( -6.0, -9.99, -50.0);
		glVertex3f( -4.0, -9.99, -50.0);
		glVertex3f( -4.0, -9.99, 50.0);
		glVertex3f( -2.0, -9.99, 50.0);
		glVertex3f( -2.0, -9.99, -50.0);
		glVertex3f( 0.0, -9.99, -50.0);
		glVertex3f( 0.0, -9.99, 50.0);

		glVertex3f( 2.0, -9.99, 50.0);
		glVertex3f( 2.0, -9.99, -50.0);
		glVertex3f( 4.0, -9.99, -50.0);
		glVertex3f( 4.0, -9.99, 50.0);
		glVertex3f( 6.0, -9.99, 50.0);
		glVertex3f( 6.0, -9.99, -50.0);
		glVertex3f( 8.0, -9.99, -50.0);
		glVertex3f( 8.0, -9.99, 50.0);
	glEnd();
	// Purple sphere loop, adds 10 spheres for each g_numSpheres
	for (int i = 1; i <= g_numSpheres; i++){
		int yVal = -8.0;
		while (yVal <= 8.0){
			glPushMatrix();
				glColor3f( .5, 0.0, 1.0 );
				glTranslatef((-8.0), yVal, (i * -5.0));
				glutSolidSphere(2.0, 10, 10);
			glPopMatrix();
			glPushMatrix();
				glTranslatef((8.0), yVal, (i * -5.0));
				glutSolidSphere(2.0, 10, 10);
			glPopMatrix();

			yVal += 4.0;
		}
	}
	// Finished with global translation from entry.
	glPopMatrix();
	// Output text information.
	if( g_RenderStats && (Eye != LEFT_EYE) ) {
		// Restore view matrix for status output in fixed screen location.
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslated(0.0f, 0.0f, -8.0f);

		glColor3f(1.0, 1.0, 1.0);
		glRasterPos2f(0.0, 2.75);
		sprintf_s(str, "%d x %d @ %dbits/pixel", g_screenWidth, g_screenHeight, g_bpp);
		if (g_fullscreen && g_IWRStereoscopyMode == PROGRESSIVE_SCAN)
			print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, str);
		else 
			print_bitmap_string(GLUT_BITMAP_HELVETICA_12, str);

		glRasterPos2f(-0.6, 3.0);
		sprintf_s(str, "FPS =  %7.2f", g_fps);
		if (g_fullscreen && g_IWRStereoscopyMode == PROGRESSIVE_SCAN)
			print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, str);
		else 
			print_bitmap_string(GLUT_BITMAP_HELVETICA_12, str);

		glRasterPos2f(1.0, 3.0);
		sprintf_s(str, "Sphere Count = %d", g_numSpheres * 10 + 3 );
		if (g_fullscreen && g_IWRStereoscopyMode == PROGRESSIVE_SCAN)
			print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, str);
		else 
			print_bitmap_string(GLUT_BITMAP_HELVETICA_12, str);

		if( Eye == RIGHT_EYE ) {
			glRasterPos2f(-3.0, 3.0);
			sprintf_s(str, "Eye Separation = %4.2f", g_EyeSeparation);
			if (g_fullscreen && g_IWRStereoscopyMode == PROGRESSIVE_SCAN)
				print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, str);
			else 
				print_bitmap_string(GLUT_BITMAP_HELVETICA_12, str);

			glRasterPos2f(-3.0, 2.75);
			sprintf_s(str, "Focal Length = %4.2f", g_FocalLength);
			if (g_fullscreen && g_IWRStereoscopyMode == PROGRESSIVE_SCAN)
				print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, str);
			else 
				print_bitmap_string(GLUT_BITMAP_HELVETICA_12, str);
			}
		for( int i=0; i < INFO_FIELDS; i++ ) {
			glRasterPos2f(-3.0, 2.45 - ((float)i * 0.2f) );
			if (g_fullscreen)
				print_bitmap_string(GLUT_BITMAP_TIMES_ROMAN_24, str_info[i]);
			else 
				print_bitmap_string(GLUT_BITMAP_HELVETICA_12, str_info[i]);
		}
	}
}

/*
* Primary rendering process.
* Defines the behavior for when no input is received, usually used for animation
* Defines the default display / rendering function.
*  Render a scene, either a Left and a Right eye frame or a monoscopic frame.
*/
void idleRenderScene()
{
	int		time = 0;
	GLuint	canDisplay = 0;

	//calculate g_fps; based on timebase.
	if (((time = glutGet(GLUT_ELAPSED_TIME)) - g_timebase) > 1000) { 
		g_fps = g_framecounter * 1000.0 / (time - g_timebase);
		g_timebase = time;
		g_framecounter = 0;
	}

	//update rotation of the red and blue spheres
	if (!g_benchmark){
		g_rotationRed += 0.2;
		g_rotationBlue += 0.2;
	}

	UpdateTracking();

	//clear color and depth buffers before 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Stereo mode enabled?
	if( g_StereoEnabled ) { 
		if (g_IWRStereoscopyMode == SIDE_X_SIDE){
			// Render left eye frame
			g_framecounter++;

			SetStereoViewport( LEFT_EYE );
			SetViewingFrustum( LEFT_EYE );
			RenderWorld( LEFT_EYE );
				
			glFinish();

			// Render MATCHING right eye frame; (No tracking or animation movement from the left eyes frame.)	
			SetStereoViewport( RIGHT_EYE );
			SetViewingFrustum( RIGHT_EYE );
			RenderWorld( RIGHT_EYE );

			glFinish();
			// Force GPU to finish, the rendered frame must scan out on the next VSYNC.
			glutSwapBuffers();

		}else if (g_IWRStereoscopyMode == PROGRESSIVE_SCAN){
			// Render left eye frame
			g_framecounter++;
			if (!g_noQuery)
				glBeginQueryARB( GL_QUERY_COUNTER_BITS_ARB, g_leftQuery );

			SetViewingFrustum( LEFT_EYE );
			RenderWorld( LEFT_EYE );

			if (!g_noQuery)
				glEndQueryARB( GL_QUERY_COUNTER_BITS_ARB );

			//Wait for the VR920 to acknowldege previous right eye frame is scanning into frame buffer.
			IWRSTEREO_WaitForAck( g_StereoHandle,LEFT_EYE );
			glFinish();

			if (!g_noQuery){
				glGetQueryObjectuivARB( g_leftQuery, GL_QUERY_RESULT_AVAILABLE_ARB, &canDisplay);
				while (canDisplay != 0) // Stall until query says it's finished
					glGetQueryObjectuivARB( g_leftQuery, GL_QUERY_RESULT_AVAILABLE_ARB, &canDisplay);
			}
			// Force GPU to finish, the rendered frame must scan out on the next VSYNC.
			glutSwapBuffers(); 
			// Inform VR920 to begin scanning on next vSync, a new left eye frame.
			IWRSTEREO_SetLR( g_StereoHandle, LEFT_EYE );

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render MATCHING right eye frame; (No tracking or animation movement from the left eyes frame.)
			g_framecounter++;
			if (!g_noQuery)
				glBeginQueryARB( GL_QUERY_COUNTER_BITS_ARB, g_rightQuery );

			SetViewingFrustum( RIGHT_EYE );
			RenderWorld( RIGHT_EYE );

			if (!g_noQuery)
				glEndQueryARB( GL_QUERY_COUNTER_BITS_ARB );

			//Wait for the VR920 to acknowldege previous left eye frame is scanning into frame buffer.
			IWRSTEREO_WaitForAck( g_StereoHandle,RIGHT_EYE );

			glFinish();

			if (!g_noQuery){
				glGetQueryObjectuivARB( g_rightQuery, GL_QUERY_RESULT_AVAILABLE_ARB, &canDisplay);
				while (canDisplay != 0) // Stall until query says it's finished
					glGetQueryObjectuivARB( g_rightQuery, GL_QUERY_RESULT_AVAILABLE_ARB, &canDisplay);
			}

			// Force GPU to finish, the rendered frame must scan out on the next VSYNC.
			glutSwapBuffers();
			// Inform VR920 to begin scanning on next vSync, a new right eye frame.
			IWRSTEREO_SetLR( g_StereoHandle, RIGHT_EYE);
		}
	}else { 
		// VR920 is in mono mode
		g_framecounter++;
		SetStereoViewport( MONO_EYES );
		SetViewingFrustum( MONO_EYES );
		RenderWorld( MONO_EYES );
		glFinish();
		glutSwapBuffers();
	}
}

/*
* Initialize the states of the program (lighting, depth rendering, etc)
* Sets viewport - In this case, the whole screen
*/
void init()
{
	glEnable( GL_DEPTH_TEST ); // turn on hidden surface removal
	glClearColor( 0.0, 0.0, 0.0, 0.0); // set clear color to black
    glViewport( 0, 0, g_screenWidth, g_screenHeight );
}

void SetStereoViewport(int eye){
	if (eye == LEFT_EYE){
		glViewport( 0, 0, g_screenWidth/2, g_screenHeight );
	} else if (eye == RIGHT_EYE) {
		glViewport( g_screenWidth/2, 0, g_screenWidth/2, g_screenHeight );
	} else {
		glViewport( 0, 0, g_screenWidth, g_screenHeight );
	}
}

void createGLWindow(){
	if( g_winID )
		glutDestroyWindow(g_winID);	
	g_winID = glutCreateWindow(OPENGL_WINDOW_NAME);
	init ();						// call initialization function
	glutDisplayFunc(idleRenderScene);// assign the main drawing function
	glutIdleFunc(idleRenderScene);	// assign the function to run while not receiving input
	glutKeyboardFunc(keyboard);		// assign the function to handle keyboard input
	glutSpecialFunc(specKeyboard);	// handle special keyboard input (arrows, ctrl, etc.)
	glutReshapeFunc(reshape);		// Hook reshape calls for proper perspective.
	// Do not show cursor, in virtual world. FullScreen only.
	if (g_fullscreen)
		ShowCursor(FALSE);
	else
		ShowCursor(TRUE);
}

//bool WGLExtensionSupported(const char *extension_name)
//{
//    // this is pointer to function which returns pointer to string with list of all wgl extensions
//    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;
//    // determine pointer to wglGetExtensionsStringEXT function
//    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");
//    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL) {
//        // string was not found
//        return false;
//    }
//    // extension is supported
//    return true;
//}

// Hook the exit process to properly shut down the iwear drivers.
void OpenGlExitprocess( void );
void OpenGlExitprocess()
{
	if( g_winID )
		glutDestroyWindow( g_winID );
	if( g_StereoHandle != INVALID_HANDLE_VALUE ) {
		IWRSTEREO_SetStereo( g_StereoHandle, IWR_MONO_MODE );
		IWRSTEREO_Close( g_StereoHandle );
	}
	IWRFreeDll();
}

void UpdateTracking(){
	// Poll input devices.
	long	Roll=0,Yaw=0, Pitch=0;
	iwr_status = IWRGetTracking( &Yaw, &Pitch, &Roll );
	if(	iwr_status != IWR_OK ){
		// iWear tracker could be OFFLine: just inform user, continue to poll until its plugged in...
		g_tracking = false;
		Yaw = Pitch = Roll = 0;
		IWROpenTracker();
	}
	// Always provide for a means to disable filtering;
	if( g_Filtering == APPLICATION_FILTER){ 
		IWRFilterTracking( &Yaw, &Pitch, &Roll );
	}
	// Convert the tracker's inputs to angles
	g_fPitch =  (float)Pitch * IWR_RAWTODEG; 
	g_fYaw   =  (float)Yaw * IWR_RAWTODEG;
	g_fRoll  =  (float)Roll * IWR_RAWTODEG;
}
#pragma endregion

#pragma region IWR Stuff

//-----------------------------------------------------------------------------
// Log file for operational status and testing results. 
//-----------------------------------------------------------------------------
int		IWROut( LPCTSTR strout )
{
int		fOut;
size_t	nBytes;	
TCHAR	Outfile[MAX_PATH];
size_t	i;
	strcpy_s( Outfile, MAX_PATH, __argv[0] );
	for( i=strlen( Outfile ); i; i-- )
		if( Outfile[i] == '\\' ) break;
	strcpy_s( &Outfile[i], MAX_PATH-i, "\\VR920Stereoscopy.log" );
	if( strout == NULL ) {
		DeleteFile( Outfile );
		return 0;
		}
	if( (fOut = _open( Outfile, _O_CREAT | _O_BINARY | _O_APPEND | _O_WRONLY, _S_IREAD | _S_IWRITE )) != -1 ) {
		nBytes = strlen( strout );
		if((_write( fOut, strout, (unsigned int)nBytes )) == -1 )
			;// Error writing to log file.
		_close( fOut );		 
	}
	else
		; // Open for log file failed.		
	return 0;
}

// 
// Function will find the VR920 Adapter and return code if found.
//  IWR_NOT_CONNECTED:  VR920 Not found.
//  IWR_IS_CONNECTED:   VR920 is connected to an adapter in the system.
//
UINT		IWRIsVR920Connected( bool *PrimaryDevice )
{
DISPLAY_DEVICE	DisplayDevice, MonitorDevice;
	// Step 1: Find VR920 Device+Adapter
	*PrimaryDevice = false;
	DisplayDevice.cb = MonitorDevice.cb = sizeof( DISPLAY_DEVICE );
	for( int i=0; EnumDisplayDevices(NULL, i, &DisplayDevice, 0x0 ); i++ ) {
		// Ignore mirrored devices and only look at desktop attachments.
		if( !(DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) &&
			(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) ) {
			for( int j=0; EnumDisplayDevices(DisplayDevice.DeviceName, j, &MonitorDevice, 0x0 ); j++ ){
				if( (MonitorDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
					(strstr( MonitorDevice.DeviceID, "IWR0002" ) ||  // VR920 id
					 strstr( MonitorDevice.DeviceID, "IWR0149" ))) { // Wrap920 id
					// Found the VR920 PnP id.
					if( DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE ) 
						*PrimaryDevice = true; // VR920 is the primary display device.
					return IWR_IS_CONNECTED;
					}
				}
			}
		}
	// VR920 does not appear to be on any accessible adapters.
	return IWR_NOT_CONNECTED;
}
#pragma endregion

#pragma region Program Entry / Main

int main(int argc, char** argv)
{
long	iwr_status;
bool	IsPrimary;
HWND	hWnd;
HMODULE hMod;
	// Determine if process is allready running.
	hWnd = FindWindow( NULL, OPENGL_WINDOW_NAME );
	if( hWnd ) {
		SetForegroundWindow( hWnd );
		return 0;
	}
	hMod = LoadLibrary("opengl32.dll");
	// Determine if the VR920 is plugged into a Video Adapter.
	/*if( IWRIsVR920Connected( &IsPrimary ) == IWR_NOT_CONNECTED ) {
		iwr_status = MessageBox( NULL, "The VR920 does not appear to be connected to an accessable video adapter.\n\n YES= The VR920 is connected to the default adapter?\n NO= Exit, to check the VR920 is plugged into an adapter and is connected to my desktop?", "VR920 Adapter WARNING", MB_YESNO );
		if( iwr_status == IDNO )
			return 0;
	}*/

	// Open the VR920's tracker driver.
	iwr_status = IWRLoadDll();
	if( iwr_status != IWR_OK ) {
		MessageBox( NULL, "NO VR920 iwrdriver support", "VR920 Driver ERROR", MB_OK );
		IWRFreeDll();
		return 0;
	}
	// Handle stereo setup
	g_StereoHandle = IWRSTEREO_Open();
	if( g_StereoHandle == INVALID_HANDLE_VALUE ) {
		if (GetLastError() == ERROR_INVALID_FUNCTION) 
			MessageBox( NULL, "Your VR920 firmware does not support stereoscopy.  Please update your firmware.", "VR920 Stereo ERROR", MB_OK );
		else 
			MessageBox( NULL, "NO VR920 Stereo Driver handle", "VR920 STEREO ERROR", MB_OK );
		IWRFreeDll();
		return 0;
	}
	// Look for proc address...Ask for device installed on users system, set/clear sidexside
	if( IWRGetProductID ) 
		Pid = IWRGetProductID();
	// Check the capabilities of the connected hardware
	if( IWRGetProductDetails ) 
		PDetails = IWRGetProductDetails();
	
	// If the hardware can do SxS stereo or Progressive Scan, set them here
	if (PDetails & IWR_FEATURE_SIDE_X_SIDE)
		g_IWRStereoscopyMode = SIDE_X_SIDE;
	else if (PDetails & IWR_FEATURE_USB_PSCAN_3D)
		g_IWRStereoscopyMode = PROGRESSIVE_SCAN;

	g_ProdSubID = IWR_GET_SUBID(PDetails);
	g_ProdAspect = IWR_GET_ASPECT_RATIO(PDetails);
	g_ProdTrackerStyle = IWR_GET_TRACKER_STYLE(PDetails);

	// Initialize Opengl
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB); // Double Buffered and RGB color model
	glutInitWindowSize( g_screenWidth, g_screenHeight );
	
	glutInitWindowPosition( 0, 0 );
	createGLWindow();
	if (g_fullscreen)
		glutFullScreen();
	// Hook an exit procedure for proper driver shutdown.
	atexit( OpenGlExitprocess );
	//PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
	//PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;
	//if (WGLExtensionSupported("WGL_EXT_swap_control")) {
	//	// Extension is supported, init pointers.
	//	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
	//	// this is another function from WGL_EXT_swap_control extension
	//	wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");
	//	wglSwapIntervalEXT(1);
	//}else 
	//	// Application can't force vsync on; 
	//	// inform the user and continue as this is a sterescopy requirement only.
	//	MessageBox( NULL, "Unable to Force Waiting for Vertical blanking syncronisation\n  This is required for VR920 stereoscopy support\n  Check your display driver options to 'Force Vsync' on, if tearing is noticable.", "WGL EXTENSION WARNING...", MB_OK );
	char *ext = (char*)glGetString( GL_EXTENSIONS );
	if( strstr( ext, "GL_ARB_occlusion_query" ) == NULL ) {
		MessageBox(NULL,"GL_ARB_occlusion_query extension was not found, QueryGPU vsync is unavailable.  This method requires OpenGL version 1.5.",
		   "ERROR",MB_OK|MB_ICONEXCLAMATION);
		g_noQuery = true;
	}
	else {
		glGenQueriesARB        = (PFNGLGENQUERIESARBPROC)wglGetProcAddress("glGenQueriesARB");
		glDeleteQueriesARB     = (PFNGLDELETEQUERIESARBPROC)wglGetProcAddress("glDeleteQueriesARB");
		glIsQueryARB           = (PFNGLISQUERYARBPROC)wglGetProcAddress("glIsQueryARB");
		glBeginQueryARB        = (PFNGLBEGINQUERYARBPROC)wglGetProcAddress("glBeginQueryARB");
		glEndQueryARB          = (PFNGLENDQUERYARBPROC)wglGetProcAddress("glEndQueryARB");
		glGetQueryivARB        = (PFNGLGETQUERYIVARBPROC)wglGetProcAddress("glGetQueryivARB");
		glGetQueryObjectivARB  = (PFNGLGETQUERYOBJECTIVARBPROC)wglGetProcAddress("glGetQueryObjectivARB");
		glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)wglGetProcAddress("glGetQueryObjectuivARB");

	   if( !glGenQueriesARB  || !glDeleteQueriesARB || !glIsQueryARB || 
		   !glBeginQueryARB || !glEndQueryARB || !glGetQueryivARB ||
		   !glGetQueryObjectivARB || !glGetQueryObjectuivARB ) {
		   MessageBox(NULL,"One or more GL_ARB_occlusion_query functions were not found",
			   "ERROR",MB_OK|MB_ICONEXCLAMATION);
		   MessageBox(NULL,"GL_ARB_occlusion_query extension was not found, QueryGPU vsync is unavailable.  This method requires OpenGL version 1.5.",
		   "ERROR",MB_OK|MB_ICONEXCLAMATION);
			g_noQuery = true;
		} else {
			glGenQueriesARB( 1, &g_leftQuery );
			glGenQueriesARB( 1, &g_rightQuery );
			g_noQuery = false;
		}
	}
	// Run through the callback functions each frame
	glutMainLoop(); 
    return 0;
}
#pragma endregion