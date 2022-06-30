//
// Use PRODUCTx flag to switch rest of flags on
//

// Common flags
#define DISABLETUTORIALS
#define DISABLEMULTIPLAYERFORMAX

// default of 260 some long paths when Asset Store item paths added to Docs path! 
#undef MAX_PATH
#define MAX_PATH 1050

// Determine compile flags for each product
#ifdef PRODUCTV3
 // Flags to compile the VR QUEST V3 version of GameGuru
 #define FPSEXCHANGE
 #define ENABLEIMGUI
 #define PHOTONMP
 #define VRTECH
 #define CLOUDKEYSYSTEM
 #define USERENDERTARGET
 #define WMFVIDEO
#else
 #ifdef PRODUCTMAX
  // Flags to compile the MAX version of GameGuru
  #define FPSEXCHANGE
  #define ENABLEIMGUI
  #define PHOTONMP
  #define VRTECH
  #define ALPHAEXPIRESYSTEM
  #define USERENDERTARGET
  #define WMFVIDEO
 #else
  #ifdef PRODUCTWICKEDMAX
   // Flags to compile the MAX version of GameGuru
   #define FPSEXCHANGE
   #define ENABLEIMGUI
   #define PHOTONMP
   #define VRTECH
   #define ALPHAEXPIRESYSTEM
   #define USERENDERTARGET
   #define WMFVIDEO
   #define WICKEDENGINE
   
   //#define GGREDUCED was used to include fog color extras but seemed to cause crashes

   // Need a common header we can use the ACTIVATE and DEACTIVATE
   // features to allow in-development vs final completed section releases
   //#define PETESTING
   //#define ADVANCEDCOLORS // Team decided not to go this route.
   //#define PEOVERLAPMODE
   #define GROUPINGFEATURE
   //#define LBBUGTRACKING
   #define HIDEOBJECTMODES
   #define PEWORKINGONPROPERTIES
   #define NEWOBJECTTOOLSLAYOUTV5
   #define USENEWLIBRARY
   ///#define DISPLAYBOUNDINGBOXIN_PROPERTIES
   ///#define DIRECTIONALCAMERASCROLLING (team decided they prefer drag camera approach - was XZ pan)
   #define ALLOWSELECTINGLOCKEDOBJECTS
   #define PRELOAD_OBJECTS_ON_HOVER
   #define USEWELCOMESCREEN

	// Paul's New Terrain!
   //#define GGTERRAIN_USE_NEW_TERRAIN
   //#define PROCEDURALTERRAINWINDOW
 
	#define USENEWMEDIASELECTWINDOWS

	//PE: @lee Polygon sort is really not precise, when you have animations ... give it a try and include here if you think.
	//#define INCLUDEPOLYGONSORT
	//Enable Phonetic results in end of results, direct matches always display at the top.
	//#ifdef PHONETICSEARCH

	//PE: The new wide thumbs to be used everywhere, this is only for wicked version.
	#define USEWIDEICONSEVERYWHERE

	// LB: New way to drag in elements and new light properties
	//PE: Include the new left panel entity tool icons.
	//PE: Include the new predefined light setup.
	#define USENEWLIGHTBEHAVIOUR
	#define INCLUDELEFTENTITYTOOLICONS
	#define INCLUDEPREDEFINEDLIGHTS

	// Gameplay work started (not for public until August)
	#define SHOOTERGAMEBUTTON
	#define NEWMAXAISYSTEM

	//PE: This will create a fixed backbuffer used for thumbs, so there is never any differences no matter what resolution/ratio the user have.
	#define USEFIXEDBACKBUFFERSIZE

	#define USENEWPARTICLESETUP
	//#define USEFULLSCREENPARTICLETRANSITIONS

	#define ADD_DETAIL_LEFT_PANEL_ENTITY_LIST


  #else
   // Flags to compile the Classic version of GameGuru
   #define FPSEXCHANGE
   #define ENABLECUSTOMTERRAIN
   #ifdef PRODUCTCONVERTER
    #define NOSTEAMORVIDEO
   #endif
  #endif
 #endif
#endif

