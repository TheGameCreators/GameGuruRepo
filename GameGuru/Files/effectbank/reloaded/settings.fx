//#####################################################################################################
//#### Many of these shaders have been improved by Preben Eriksen. For more information visit
//#### http://www.daily3d.com, or e-mail plemsoft@plemsoft.com.  If you want to use these
//#### shaders to create cross-platform 3D games, feel free to check out "GameGuru Loader" DLC
//#### for AppGameKit which enables you to load GameGuru levels and code your game from scratch.
//#### Visit www.appgamekit.com or the main Steam page here: http://store.steampowered.com/app/325180/
//#####################################################################################################

//#define LAPTOP // Make everything faster for laptops, will also remove DOF,MB,SAO. Lower shadow, remove grass normals, distance improvements.

//#define USEPBRTERRAIN // USE PBR terrain even if you have pbroverride=0 ( medium, only work with one cascade (3) using pbroverride=0 ).
//#define USEPBRVEGETATION // USE PBR for vegetation even if you have pbroverride=0
#define FASTROCKTEXTURE // disable tri-planar rock for faster terrain. 

#define IMPROVEDISTANCE // Improve distance terrain.
#define MEDIUMIMPROVEDISTANCE // PE: missing in PBR, Improve medium distance terrain.
#define WATERREFLECTIONMEDIUM // water reflection use medium.
//#define USEDISTBASEMAP // PE: ( not used in PBR ), Use basemap in distance faster.
#define LENSFLARE // enable / disable lens flare.
#define DISABLELENSFLAREHALO // diable the lens flare halo.
#define FXAA // enable / disable FXAA

//#define CARTOON // enable / disable cartoon (cel) shader.
//#define RETRODITHER // make a retro dither look
//#define RETRODITHER64 // make a retro C64 dither look.
//#define CHROMATICABBERATION (7.0,9.0,3.0)
//#define CHROMATICABBERATIONLUA // Define this if you want to use SetShaderVariable(0,"ChromaticAbberation", R split , G split, B split, 0=Disable,1=Enable) from lua.
#define DOF // enable / disable depth of field.
#define MOTIONBLUR // enable / disable motion blur.
//#define NOSAO

//NOTE: Below settings can be used to finetune a level.
//If you dont know what the delow settings is, there no need to change them.
//#define SPECULARCAMERA
//#define SPECULARCAMERAINTENSITY (0.4)

#define TERRAINROCKSLOPE (0.5)
//#define NOCOLOREFFECT // Disable color effect, like: screen going red when you die, or you have little health.

// HQLENSFLARE , lens flare use own render target, and you can set when it trigger.
// use TRIGGERLENSFLARE to define when to trigger lens flare , higher means it dont trigger as often. use values from 0.0 - 1.0
#define HQLENSFLARE
#define TRIGGERLENSFLARE (0.97,0.97,0.97)

#define BETTERSHADOWS

// About LENSFLAREINTENSITY: 3.0 Normal intensity, 0.5 very low, 2.0 Low , 5.0 High intensity.
#define LENSFLAREINTENSITY (3.0)

// CARTOONIMPACT 4.2=Normal , can be any number from. 0 - ??
#define CARTOONIMPACT (4.2)

//#define ADDSKYBOXFOG // Add some fog to the skybox. keep fog colors below 94 if you use lens flare.

//#define DEBUGSHADOW // Use this to see the levels of cascade on shadow mapping
#define SKIPIFNODYNAMICLIGHTS // Skip dynamic lights calc if no lights
//#define REFLECTIVEFLASHLIGHT // PE: Show reflection of flashlight on metallic objects, wip.
#define MAXFLASHLIGHT (0.975) // PE: max intensity of flashlight 1.0 = 100% , 0.5 = 50%.
//#define PEROJECTLIGHT // Change per object specular to per object light.
//#define BOOSTILLUM // Add more light from the illumination texture.

//Settings used by FXAA-CV-LS
#define COLORVIBRANCE (0.40)
#define LUMASHARPEN (0.65)
//#define SPLITTESTFXAACVLS // Run the new FXAA-CV-LS in a split screen test so you can compare it.

// Below you can enable/disable the features you like in your LAPTOP setup.
// The more you disable the more speed you get.
#ifdef LAPTOP
#ifdef DOF
 #undef DOF
#endif
#ifdef MOTIONBLUR
 #undef MOTIONBLUR
#endif
#ifndef FASTROCKTEXTURE
 #define FASTROCKTEXTURE
#endif
#ifdef WATERREFLECTIONMEDIUM
 #undef WATERREFLECTIONMEDIUM
#endif
#ifdef MEDIUMIMPROVEDISTANCE
 #undef MEDIUMIMPROVEDISTANCE
#endif
//#ifdef BETTERSHADOWS
// #undef BETTERSHADOWS  
//#endif
#ifndef REMOVEGRASSNORMALS
 #define REMOVEGRASSNORMALS
#endif
#ifdef IMPROVEDISTANCE
 #undef IMPROVEDISTANCE
#endif
#ifndef NOSAO
 #define NOSAO
#endif
#endif
