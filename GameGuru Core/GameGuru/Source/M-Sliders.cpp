//----------------------------------------------------
//--- GAMEGURU - M-Sliders
//----------------------------------------------------

// Globals
#include "stdafx.h"
#include "gameguru.h"

#ifdef VRTECH
#include "..\Imgui\imgui.h"
#include "..\Imgui\imgui_impl_win32.h"
#include "..\Imgui\imgui_gg_dx11.h"
#endif

// Prototypes
void set_inputsys_mclick(int value);

// Externals
extern UINT g_StereoEyeToggle;

// 
//  Menu, Display & Sliders Module
// 

void sliders_init ( void )
{
	image_setlegacyimageloading(true);
	//  load images for slider resources
	t.timgbase=g.slidersmenuimageoffset;
	SetMipmapNum(1); //PE: mipmaps not needed.
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\bar.png",t.timgbase+1,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\handle.png",t.timgbase+2,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\paneltop.png",t.timgbase+3,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\panelmiddle.png",t.timgbase+4,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\panelbottom.png",t.timgbase+5,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\glass-tube.png",t.timgbase+6,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\dropdown.png",t.timgbase+7,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\fill.png",t.timgbase+8,1 );
	for ( t.t = 0 ; t.t<=  6; t.t++ )
	{
		if (  t.t == 0  )  t.t_s = "red";
		if (  t.t == 1  )  t.t_s = "green";
		if (  t.t == 2  )  t.t_s = "blue";
		if (  t.t == 3  )  t.t_s = "orange";
		if (  t.t == 4  )  t.t_s = "yellow";
		if (  t.t == 5  )  t.t_s = "purple";
		if (  t.t == 6  )  t.t_s = "cyan";
		LoadImage ( cstr(cstr( "languagebank\\neutral\\gamecore\\huds\\sliders\\bar-")+t.t_s+"-start.png").Get(),t.timgbase+11+(t.t*3),1 );
		LoadImage (  cstr(cstr( "languagebank\\neutral\\gamecore\\huds\\sliders\\bar-")+t.t_s+".png").Get(),t.timgbase+12+(t.t*3),1 );
		LoadImage (  cstr(cstr( "languagebank\\neutral\\gamecore\\huds\\sliders\\bar-")+t.t_s+"-end.png").Get(),t.timgbase+13+(t.t*3),1 );
	}
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\minmaxbutton.png",t.timgbase+32,0 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\sliders\\minmaxbuttonlit.png",t.timgbase+33,0 );

	#ifdef VRTECH
	#else
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-health-panel.png",t.timgbase+51,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\horizontal-line.png",t.timgbase+52,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\vertical-line.png",t.timgbase+53,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-colt.png",t.timgbase+54,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-magnum.png",t.timgbase+55,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-uzi.png",t.timgbase+56,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-shotgun.png",t.timgbase+57,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-riffle.png",t.timgbase+58,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-rpg.png",t.timgbase+59,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-grenade.png",t.timgbase+60,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-bow.png",t.timgbase+61,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-mace.png",t.timgbase+62,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-stone.png",t.timgbase+63,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-sword.png",t.timgbase+64,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\icon-staff.png",t.timgbase+65,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-handgun.png",t.timgbase+71,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-handgun.png",t.timgbase+72,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-riffle.png",t.timgbase+73,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-shotgun.png",t.timgbase+74,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-riffle.png",t.timgbase+75,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-rpg.png",t.timgbase+76,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-grenade.png",t.timgbase+77,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-bow.png",t.timgbase+78,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-none.png",t.timgbase+79,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-none.png",t.timgbase+80,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-none.png",t.timgbase+81,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-staff.png",t.timgbase+82,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\health-icon.png",t.timgbase+91,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\lives-icon.png",t.timgbase+92,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\ammohealth\\ammo-icon-infinity.png",t.timgbase+93,1 );
	SetMipmapNum(-1);
	#endif

	image_setlegacyimageloading(false);



	//  Multiple panels allowed
	g.slidersmenumax=0;

	//  Special A.I Drill Down panel
	++g.slidersmenumax;
	t.slidersmenunames.aidrilldown=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=-99;
	t.slidersmenu[g.slidersmenumax].title_s="A.I DETAIL";
	t.slidersmenu[g.slidersmenumax].itemcount=6;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="LUA Engine";
	t.slidersmenuvalue[g.slidersmenumax][1].value=0;
	t.slidersmenuvalue[g.slidersmenumax][1].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][1].expanddetect=-1;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Entity Updates";
	t.slidersmenuvalue[g.slidersmenumax][2].value=0;
	t.slidersmenuvalue[g.slidersmenumax][2].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][2].expanddetect=-1;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="DarkAI Control";
	t.slidersmenuvalue[g.slidersmenumax][3].value=0;
	t.slidersmenuvalue[g.slidersmenumax][3].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][3].expanddetect=-1;
	t.slidersmenuvalue[g.slidersmenumax][4].name_s="DarkAI Update";
	t.slidersmenuvalue[g.slidersmenumax][4].value=0;
	t.slidersmenuvalue[g.slidersmenumax][4].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][4].expanddetect=-1;
	t.slidersmenuvalue[g.slidersmenumax][5].name_s="Character Logic";
	t.slidersmenuvalue[g.slidersmenumax][5].value=0;
	t.slidersmenuvalue[g.slidersmenumax][5].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][5].expanddetect=-1;
	t.slidersmenuvalue[g.slidersmenumax][6].name_s="Attachments";
	t.slidersmenuvalue[g.slidersmenumax][6].value=0;
	t.slidersmenuvalue[g.slidersmenumax][6].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][6].expanddetect=-1;

	//  Performance panel
	++g.slidersmenumax;
	t.slidersmenunames.performance=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=-1;
	t.slidersmenu[g.slidersmenumax].title_s="PERFORMANCE";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=13;
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenu[g.slidersmenumax].ttop=5;
	t.slidersmenu[g.slidersmenumax].tleft=5;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="A.I.";
	t.slidersmenuvalue[g.slidersmenumax][1].value=0;
	t.slidersmenuvalue[g.slidersmenumax][1].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][1].expanddetect=t.slidersmenunames.aidrilldown;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Physics";
	t.slidersmenuvalue[g.slidersmenumax][2].value=0;
	t.slidersmenuvalue[g.slidersmenumax][2].readmodeindex=2;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Vegetation";
	t.slidersmenuvalue[g.slidersmenumax][3].value=0;
	t.slidersmenuvalue[g.slidersmenumax][3].readmodeindex=3;
	t.slidersmenuvalue[g.slidersmenumax][4].name_s="Shadows";
	t.slidersmenuvalue[g.slidersmenumax][4].value=0;
	t.slidersmenuvalue[g.slidersmenumax][4].readmodeindex=3;
	t.slidersmenuvalue[g.slidersmenumax][5].name_s="Sky & Water";
	t.slidersmenuvalue[g.slidersmenumax][5].value=0;
	t.slidersmenuvalue[g.slidersmenumax][5].readmodeindex=3;
	t.slidersmenuvalue[g.slidersmenumax][6].name_s="Rendering";
	t.slidersmenuvalue[g.slidersmenumax][6].value=0;
	t.slidersmenuvalue[g.slidersmenumax][6].readmodeindex=4;
	t.slidersmenuvalue[g.slidersmenumax][7].name_s="Controls";
	t.slidersmenuvalue[g.slidersmenumax][7].value=0;
	t.slidersmenuvalue[g.slidersmenumax][7].readmodeindex=5;
	t.slidersmenuvalue[g.slidersmenumax][8].name_s="Misc";
	t.slidersmenuvalue[g.slidersmenumax][8].value=0;
	t.slidersmenuvalue[g.slidersmenumax][8].readmodeindex=5;
	#ifdef VRTECH
	 t.slidersmenuvalue[g.slidersmenumax][9].name_s="HUD";
	#else
	 t.slidersmenuvalue[g.slidersmenumax][9].name_s="Weapons";
	#endif
	t.slidersmenuvalue[g.slidersmenumax][9].value=0;
	t.slidersmenuvalue[g.slidersmenumax][9].readmodeindex=5;
	t.slidersmenuvalue[g.slidersmenumax][10].name_s="Occlusion";
	t.slidersmenuvalue[g.slidersmenumax][10].value=0;
	t.slidersmenuvalue[g.slidersmenumax][10].readmodeindex=5;
	t.slidersmenuvalue[g.slidersmenumax][11].name_s="Postprocessing";
	t.slidersmenuvalue[g.slidersmenumax][11].value=0;
	t.slidersmenuvalue[g.slidersmenumax][11].readmodeindex=5;
	t.slidersmenuvalue[g.slidersmenumax][12].name_s="Polygons";
	t.slidersmenuvalue[g.slidersmenumax][12].value=0;
	t.slidersmenuvalue[g.slidersmenumax][12].readmodeindex=1;
	t.slidersmenuvalue[g.slidersmenumax][13].name_s="Draw Calls";
	t.slidersmenuvalue[g.slidersmenumax][13].value=0;
	t.slidersmenuvalue[g.slidersmenumax][13].readmodeindex=2;

	//  Visuals panel
	++g.slidersmenumax;
	t.slidersmenunames.visuals=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=2;
	t.slidersmenu[g.slidersmenumax].title_s="VISUAL SETTINGS";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].ttop=5;
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-256-16-5;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=18;
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Fog Nearest";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.visuals.FogNearest_f/100.0;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Fog Distance";
	t.slidersmenuvalue[g.slidersmenumax][2].value=t.visuals.FogDistance_f/500.0;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Fog Red";
	t.slidersmenuvalue[g.slidersmenumax][3].value=t.visuals.FogR_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][4].name_s="Fog Green";
	t.slidersmenuvalue[g.slidersmenumax][4].value=t.visuals.FogG_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][5].name_s="Fog Blue";
	t.slidersmenuvalue[g.slidersmenumax][5].value=t.visuals.FogB_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][6].name_s="Fog Intensity";
	t.slidersmenuvalue[g.slidersmenumax][6].value=t.visuals.FogA_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][7].name_s="Ambience Level";
	t.slidersmenuvalue[g.slidersmenumax][7].value=t.visuals.AmbienceIntensity_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][8].name_s="Ambience Red";
	t.slidersmenuvalue[g.slidersmenumax][8].value=t.visuals.AmbienceRed_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][9].name_s="Ambience Green";
	t.slidersmenuvalue[g.slidersmenumax][9].value=t.visuals.AmbienceGreen_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][10].name_s="Ambience Blue";
	t.slidersmenuvalue[g.slidersmenumax][10].value=t.visuals.AmbienceBlue_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][11].name_s="Surface Level";
	t.slidersmenuvalue[g.slidersmenumax][11].value=t.visuals.SurfaceIntensity_f*33;
	t.slidersmenuvalue[g.slidersmenumax][12].name_s="Surface Red";
	t.slidersmenuvalue[g.slidersmenumax][12].value=t.visuals.SurfaceRed_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][13].name_s="Surface Green";
	t.slidersmenuvalue[g.slidersmenumax][13].value=t.visuals.SurfaceGreen_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][14].name_s="Surface Blue";
	t.slidersmenuvalue[g.slidersmenumax][14].value=t.visuals.SurfaceBlue_f/2.55;
	t.slidersmenuvalue[g.slidersmenumax][15].name_s="Surface Sun Factor";
	t.slidersmenuvalue[g.slidersmenumax][15].value=t.visuals.SurfaceSunFactor_f * 100;
	t.slidersmenuvalue[g.slidersmenumax][16].name_s="Global Specular";
	t.slidersmenuvalue[g.slidersmenumax][16].value=t.visuals.Specular_f * 100;
	t.slidersmenuvalue[g.slidersmenumax][17].name_s="Brightness";
	t.slidersmenuvalue[g.slidersmenumax][17].value=(t.visuals.PostBrightness_f+0.5) * 100;
	t.slidersmenuvalue[g.slidersmenumax][18].name_s="Contrast";
	t.slidersmenuvalue[g.slidersmenumax][18].value=t.visuals.PostContrast_f * 30;

	//  Camera panel
	++g.slidersmenumax;
	t.slidersmenunames.camera=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=2;
	t.slidersmenu[g.slidersmenumax].title_s="CAMERA SETTINGS";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].ttop=5;
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-256-16-5-260;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=4;
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Camera Distance";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.visuals.CameraFAR_f/3000.0;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Camera Horizontal FOV";
	t.slidersmenuvalue[g.slidersmenumax][2].value=(((t.visuals.CameraFOV_f*t.visuals.CameraASPECT_f)-20.0)/90.0)*100.0;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Camera Zoom Percentage";
	t.slidersmenuvalue[g.slidersmenumax][3].value=t.visuals.CameraFOVZoomed_f*100;
	#ifdef VRTECH
	 t.slidersmenuvalue[g.slidersmenumax][4].name_s="HUD Horizontal FOV";
	#else
	 t.slidersmenuvalue[g.slidersmenumax][4].name_s="Weapon Horizontal FOV";
	#endif
	t.slidersmenuvalue[g.slidersmenumax][4].value=(((t.visuals.WeaponFOV_f*t.visuals.CameraASPECT_f)-20.0)/90.0)*100.0;

	//  Water panel
	++g.slidersmenumax;
	t.slidersmenunames.water = g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage = 2;
	t.slidersmenu[g.slidersmenumax].title_s = "WATER SETTINGS";
	t.slidersmenu[g.slidersmenumax].thighlight = -1;
	t.slidersmenu[g.slidersmenumax].ttop = 5;
	t.slidersmenu[g.slidersmenumax].tleft = GetDisplayWidth() - 256 - 16 - 5 - 260 - 260 - 260 -260;
	t.slidersmenu[g.slidersmenumax].titlemargin = 63;
	t.slidersmenu[g.slidersmenumax].leftmargin = 25;
	t.slidersmenu[g.slidersmenumax].itemcount = 13;
	t.slidersmenu[g.slidersmenumax].panelheight = 30 + (t.slidersmenu[g.slidersmenumax].itemcount * 38);
	t.slidersmenuvalue[g.slidersmenumax][1].name_s = "Water Height";
	t.slidersmenuvalue[g.slidersmenumax][1].value = SlidersCutExtendedValues(g.gdefaultwaterheight/10);
	t.slidersmenuvalue[g.slidersmenumax][2].name_s = "Water Red";
	t.slidersmenuvalue[g.slidersmenumax][2].value = SlidersCutExtendedValues(t.visuals.WaterRed_f/2.55);
	t.slidersmenuvalue[g.slidersmenumax][3].name_s = "Water Green";
	t.slidersmenuvalue[g.slidersmenumax][3].value = SlidersCutExtendedValues(t.visuals.WaterGreen_f/2.55);
	t.slidersmenuvalue[g.slidersmenumax][4].name_s = "Water Blue";
	t.slidersmenuvalue[g.slidersmenumax][4].value = SlidersCutExtendedValues(t.visuals.WaterBlue_f/2.55);
	t.slidersmenuvalue[g.slidersmenumax][5].name_s = "Wave Intensity";
	t.slidersmenuvalue[g.slidersmenumax][5].value = SlidersCutExtendedValues(t.visuals.WaterWaveIntensity_f/2);
	t.slidersmenuvalue[g.slidersmenumax][6].name_s = "Water Transparancy";
	t.slidersmenuvalue[g.slidersmenumax][6].value = SlidersCutExtendedValues(t.visuals.WaterTransparancy_f*100.0);
	t.slidersmenuvalue[g.slidersmenumax][7].name_s = "Water Reflection";
	t.slidersmenuvalue[g.slidersmenumax][7].value = SlidersCutExtendedValues(t.visuals.WaterReflection_f*100.0);
	t.slidersmenuvalue[g.slidersmenumax][8].name_s = "Reflection Sparkle Intensity";
	t.slidersmenuvalue[g.slidersmenumax][8].value = SlidersCutExtendedValues(t.visuals.WaterReflectionSparkleIntensity *5.0);
	t.slidersmenuvalue[g.slidersmenumax][9].name_s = "Flow Direction X";
	t.slidersmenuvalue[g.slidersmenumax][9].value = SlidersCutExtendedValues(t.visuals.WaterFlowDirectionX * 10 + 50);
	t.slidersmenuvalue[g.slidersmenumax][10].name_s = "Flow Direction Y";
	t.slidersmenuvalue[g.slidersmenumax][10].value = SlidersCutExtendedValues(t.visuals.WaterFlowDirectionY * 10 + 50);
	t.slidersmenuvalue[g.slidersmenumax][11].name_s = "Water Distortion Waves";
	t.slidersmenuvalue[g.slidersmenumax][11].value = SlidersCutExtendedValues(t.visuals.WaterDistortionWaves*1000);
	t.slidersmenuvalue[g.slidersmenumax][12].name_s = "Water Ripple Speed";
	t.slidersmenuvalue[g.slidersmenumax][12].value = SlidersCutExtendedValues(100-t.visuals.WaterSpeed1);
	t.slidersmenuvalue[g.slidersmenumax][13].name_s = "Water Speed";
	t.slidersmenuvalue[g.slidersmenumax][13].value = SlidersCutExtendedValues(t.visuals.WaterFlowSpeed*10.0);

	//  Post Effects panel
	++g.slidersmenumax;
	t.slidersmenunames.posteffects=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=2;
	t.slidersmenu[g.slidersmenumax].title_s="POST EFFECTS";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=15;
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-256-16-5-260;
	t.slidersmenu[g.slidersmenumax].ttop=GetDisplayHeight()-t.slidersmenu[t.slidersmenunames.posteffects].panelheight-82;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Bloom";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.visuals.bloommode;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Vignette Radius";
	t.slidersmenuvalue[g.slidersmenumax][2].value=t.visuals.VignetteRadius_f*100.0;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Vignette Intensity";
	t.slidersmenuvalue[g.slidersmenumax][3].value=t.visuals.VignetteIntensity_f*100.0;
	t.slidersmenuvalue[g.slidersmenumax][4].name_s="Motion Blur Distance";
	t.slidersmenuvalue[g.slidersmenumax][4].value=t.visuals.MotionDistance_f*100.0;
	t.slidersmenuvalue[g.slidersmenumax][5].name_s="Motion Blur Intensity";
	t.slidersmenuvalue[g.slidersmenumax][5].value=t.visuals.MotionIntensity_f*100.0;
	t.slidersmenuvalue[g.slidersmenumax][6].name_s="Depth Of Field Distance";
	t.slidersmenuvalue[g.slidersmenumax][6].value=t.visuals.DepthOfFieldDistance_f*100;
	t.slidersmenuvalue[g.slidersmenumax][7].name_s="Depth Of Field Intensity";
	t.slidersmenuvalue[g.slidersmenumax][7].value=t.visuals.DepthOfFieldIntensity_f*100;
	t.slidersmenuvalue[g.slidersmenumax][8].name_s="Lightray Length";
	t.slidersmenuvalue[g.slidersmenumax][8].value=t.visuals.LightrayLength_f*100;
	t.slidersmenuvalue[g.slidersmenumax][9].name_s="Lightray Quality";
	t.slidersmenuvalue[g.slidersmenumax][9].value=t.visuals.LightrayQuality_f;
	t.slidersmenuvalue[g.slidersmenumax][10].name_s="Lightray Decay";
	t.slidersmenuvalue[g.slidersmenumax][10].value=t.visuals.LightrayDecay_f*100;
	t.slidersmenuvalue[g.slidersmenumax][11].name_s="SAO Radius";
	t.slidersmenuvalue[g.slidersmenumax][11].value=t.visuals.SAORadius_f*100;
	t.slidersmenuvalue[g.slidersmenumax][12].name_s="SAO Intensity";
	t.slidersmenuvalue[g.slidersmenumax][12].value=t.visuals.SAOIntensity_f*100;
	t.slidersmenuvalue[g.slidersmenumax][13].name_s="Lens Flare Intensity";
	t.slidersmenuvalue[g.slidersmenumax][13].value=t.visuals.LensFlare_f*100;
	t.slidersmenuvalue[g.slidersmenumax][14].name_s = "Saturation";
	t.slidersmenuvalue[g.slidersmenumax][14].value = t.visuals.Saturation_f * 100; 
	t.slidersmenuvalue[g.slidersmenumax][15].name_s = "Sepia";
	t.slidersmenuvalue[g.slidersmenumax][15].value = t.visuals.Sepia_f * 100;

	//  Sky panel
//  `++slidersmenumax

//  `slidersmenunames.sky=slidersmenumax

//  `slidersmenu(slidersmenumax).tabpage=2

//  `slidersmenu(slidersmenumax).title$="SKY SETTINGS"

//  `slidersmenu(slidersmenumax).thighlight=-1

//  `slidersmenu(slidersmenumax).ttop=300

//  `slidersmenu(slidersmenumax).tleft=GetDisplayWidth()-256-16-5-260

//  `slidersmenu(slidersmenumax).titlemargin=63

//  `slidersmenu(slidersmenumax).leftmargin=25

//  `slidersmenu(slidersmenumax).itemcount=3

//  `slidersmenu(slidersmenumax).panelheight=30+(slidersmenu(slidersmenumax).itemcount*38)

//  `slidersmenuvalue(slidersmenumax,1).name$="Sun Position X"

//  `slidersmenuvalue(slidersmenumax,1).value=50+(terrain.sundirectionx#/1000)

//  `slidersmenuvalue(slidersmenumax,2).name$="Sun Position Y"

//  `slidersmenuvalue(slidersmenumax,2).value=0+(terrain.sundirectiony#/1000)

//  `slidersmenuvalue(slidersmenumax,3).name$="Sun Position Z"

//  `slidersmenuvalue(slidersmenumax,3).value=50+(terrain.sundirectionz#/1000)


	//  Quality panel
	++g.slidersmenumax;
	t.slidersmenunames.qualitypanel=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=2;
	t.slidersmenu[g.slidersmenumax].title_s="QUALITY SETTINGS";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].ttop=5;
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-256-16-5-260-260;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=7;
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Terrain LOD Near";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.visuals.TerrainLOD1_f/100;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Terrain LOD Midrange";
	t.slidersmenuvalue[g.slidersmenumax][2].value=t.visuals.TerrainLOD2_f/100;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Terrain LOD Far";
	t.slidersmenuvalue[g.slidersmenumax][3].value=t.visuals.TerrainLOD3_f/100;
	t.slidersmenuvalue[g.slidersmenumax][4].name_s="Terrain Size";
	t.slidersmenuvalue[g.slidersmenumax][4].value=t.visuals.TerrainSize_f;
	t.slidersmenuvalue[g.slidersmenumax][5].name_s="Vegetation Quantity";
	t.slidersmenuvalue[g.slidersmenumax][5].value=t.visuals.VegQuantity_f;
	t.slidersmenuvalue[g.slidersmenumax][6].name_s="Vegetation Width";
	t.slidersmenuvalue[g.slidersmenumax][6].value=t.visuals.VegWidth_f;
	t.slidersmenuvalue[g.slidersmenumax][7].name_s="Vegetation Height";
	t.slidersmenuvalue[g.slidersmenumax][7].value=t.visuals.VegHeight_f;

	//  World Settings panel
	++g.slidersmenumax;
	t.slidersmenunames.worldpanel=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=2;
	t.slidersmenu[g.slidersmenumax].title_s="WORLD SETTINGS";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].ttop=5;
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-256-16-5-260-260-260;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	#ifdef VRTECH
	t.slidersmenu[g.slidersmenumax].itemcount=4;
	#else
	t.slidersmenu[g.slidersmenumax].itemcount=4;
	#endif
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Sky Type";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.visuals.skyindex;
	t.slidersmenuvalue[g.slidersmenumax][1].gadgettype=1;
	t.slidersmenuvalue[g.slidersmenumax][1].gadgettypevalue=1;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Terrain Type";
	t.slidersmenuvalue[g.slidersmenumax][2].value=t.visuals.terrainindex;
	t.slidersmenuvalue[g.slidersmenumax][2].gadgettype=1;
	t.slidersmenuvalue[g.slidersmenumax][2].gadgettypevalue=2;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Vegetation Type";
	t.slidersmenuvalue[g.slidersmenumax][3].value=t.visuals.vegetationindex;
	t.slidersmenuvalue[g.slidersmenumax][3].gadgettype=1;
	t.slidersmenuvalue[g.slidersmenumax][3].gadgettypevalue=3;

	t.slidersmenuvalue[g.slidersmenumax][4].name_s = "LUT Selection";
	t.slidersmenuvalue[g.slidersmenumax][4].value = t.visuals.lutindex;
	t.slidersmenuvalue[g.slidersmenumax][4].gadgettype = 1;
	t.slidersmenuvalue[g.slidersmenumax][4].gadgettypevalue = 8;// 11;

	#ifdef VRTECH
	t.slidersmenuvalue[g.slidersmenumax][4].name_s = "Weather";
	t.slidersmenuvalue[g.slidersmenumax][4].value = t.visuals.iEnvironmentWeather+1;
	t.slidersmenuvalue[g.slidersmenumax][4].gadgettype = 1;
	t.slidersmenuvalue[g.slidersmenumax][4].gadgettypevalue = 40;
	for ( t.tn = 1 ; t.tn<=  4; t.tn++ )
	#else
	for ( t.tn = 1 ; t.tn<=  4; t.tn++ )
	#endif
	{
		t.slidersmenuindex=g.slidersmenumax;
		t.slidersmenuvaluechoice=t.slidersmenuvalue[g.slidersmenumax][t.tn].gadgettypevalue;
		t.slidersmenuvalueindex=t.slidersmenuvalue[g.slidersmenumax][t.tn].value;
		sliders_getnamefromvalue ( );
		t.slidersmenuvalue[g.slidersmenumax][t.tn].value_s=t.slidervaluename_s;
	}

	//  Weapons in-game panel
	++g.slidersmenumax;
	t.slidersmenunames.weapon=g.slidersmenumax;
	if ( g.vrqcontrolmode != 0 || g.gvrmodeoriginal != 0 ) 
	{
		// hide ammo/weapon panel if no weapon specified at start
		t.slidersmenu[g.slidersmenumax].tabpage=-10;
	}
	else
	{
		t.slidersmenu[g.slidersmenumax].tabpage=0;
	}
	t.slidersmenu[g.slidersmenumax].title_s="";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].tleft=5;
	t.slidersmenu[g.slidersmenumax].titlemargin=20;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=2;
	t.slidersmenu[g.slidersmenumax].panelheight=(t.slidersmenu[g.slidersmenumax].itemcount*38)-40;
	t.slidersmenu[g.slidersmenumax].ttop=GetDisplayHeight()-146;
	t.slidersmenu[g.slidersmenumax].readonly=1;
	t.slidersmenu[g.slidersmenumax].customimage=g.slidersmenuimageoffset+51;
	t.slidersmenu[g.slidersmenumax].customimagetype=1;
	t.slidersmenu[g.slidersmenumax].customimagesubtype=2;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Ammo";
	t.slidersmenuvalue[g.slidersmenumax][1].value=0;
	t.slidersmenuvalue[g.slidersmenumax][1].readmodeindex=7;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Clips";
	t.slidersmenuvalue[g.slidersmenumax][2].value=0;
	t.slidersmenuvalue[g.slidersmenumax][2].readmodeindex=6;

	// Player status in-game panel
	++g.slidersmenumax;
	t.slidersmenunames.player=g.slidersmenumax;
	if ( t.player[1].health == 99999 || g.vrqcontrolmode != 0 || g.gvrmodeoriginal != 0 ) 
	{
		//  hide health panel if no health in start marker
		t.slidersmenu[g.slidersmenumax].tabpage=-10;
	}
	else
	{
		t.slidersmenu[g.slidersmenumax].tabpage=0;
	}
	t.slidersmenu[g.slidersmenumax].title_s="";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].titlemargin=20;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=2;
	t.slidersmenu[g.slidersmenumax].panelheight=(t.slidersmenu[g.slidersmenumax].itemcount*38)-40;
	t.slidersmenu[g.slidersmenumax].ttop=GetDisplayHeight()-146;
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-16-230;
	t.slidersmenu[g.slidersmenumax].customimage=g.slidersmenuimageoffset+51;
	t.slidersmenu[g.slidersmenumax].customimagetype=2;
	t.slidersmenu[g.slidersmenumax].customimagesubtype=0;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Lives";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.player[1].lives;
	t.slidersmenuvalue[g.slidersmenumax][1].readmodeindex=5;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Health";
	t.slidersmenuvalue[g.slidersmenumax][2].value=t.player[1].health;
	t.slidersmenuvalue[g.slidersmenumax][2].readmodeindex=4;

	//  Graphics Options panel
	++g.slidersmenumax;
	t.slidersmenunames.graphicoptions=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=2;
	t.slidersmenu[g.slidersmenumax].title_s="GRAPHIC OPTIONS";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=6;
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-256-16-5-260-260-260;
	t.slidersmenu[g.slidersmenumax].ttop=GetDisplayHeight()-t.slidersmenu[g.slidersmenumax].panelheight-87;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Water and Reflections";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.visuals.reflectionmode;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Shadows";
	t.slidersmenuvalue[g.slidersmenumax][2].value=t.visuals.shadowmode;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Lightrays";
	t.slidersmenuvalue[g.slidersmenumax][3].value=t.visuals.lightraymode;
	t.slidersmenuvalue[g.slidersmenumax][4].name_s="Vegetation";
	t.slidersmenuvalue[g.slidersmenumax][4].value=t.visuals.vegetationmode;
	t.slidersmenuvalue[g.slidersmenumax][5].name_s="Occlusion";
	//t.slidersmenuvalue[g.slidersmenumax][5].value=g.globals.occlusionsize/50;
	t.slidersmenuvalue[g.slidersmenumax][5].value=t.visuals.occlusionvalue;
	t.slidersmenuvalue[g.slidersmenumax][6].name_s="Debug Visuals";
	t.slidersmenuvalue[g.slidersmenumax][6].value=t.visuals.debugvisualsmode;

	//  Shader Options panel
	++g.slidersmenumax;
	t.slidersmenunames.shaderoptions=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=2;
	t.slidersmenu[g.slidersmenumax].title_s="SHADER OPTIONS";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].titlemargin=63;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=6;
	if ( g.gpbroverride == 1 ) t.slidersmenu[g.slidersmenumax].itemcount=4;
	t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
	t.slidersmenu[g.slidersmenumax].tleft=GetDisplayWidth()-256-16-5-260-260;
	t.slidersmenu[g.slidersmenumax].ttop=GetDisplayHeight()-t.slidersmenu[t.slidersmenunames.graphicoptions].panelheight-87;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Terrain Shader";
	t.slidersmenuvalue[g.slidersmenumax][1].value=t.visuals.shaderlevels.terrain;
	t.slidersmenuvalue[g.slidersmenumax][1].gadgettype=1;
	t.slidersmenuvalue[g.slidersmenumax][1].gadgettypevalue=4;
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="Entity Shader";
	t.slidersmenuvalue[g.slidersmenumax][2].value=t.visuals.shaderlevels.entities;
	t.slidersmenuvalue[g.slidersmenumax][2].gadgettype=1;
	t.slidersmenuvalue[g.slidersmenumax][2].gadgettypevalue=5;
	t.slidersmenuvalue[g.slidersmenumax][3].name_s="Grass Shader";
	t.slidersmenuvalue[g.slidersmenumax][3].value=t.visuals.shaderlevels.vegetation;
	t.slidersmenuvalue[g.slidersmenumax][3].gadgettype=1;
	t.slidersmenuvalue[g.slidersmenumax][3].gadgettypevalue=6;
	t.slidersmenuvalue[g.slidersmenumax][4].name_s="Lighting";
	t.slidersmenuvalue[g.slidersmenumax][4].value=t.visuals.shaderlevels.lighting;
	t.slidersmenuvalue[g.slidersmenumax][4].gadgettype=1;
	t.slidersmenuvalue[g.slidersmenumax][4].gadgettypevalue=7;
	for ( t.tn = 1 ; t.tn<=  4; t.tn++ )
	{
		t.slidersmenuindex=g.slidersmenumax;
		t.slidersmenuvaluechoice=t.slidersmenuvalue[g.slidersmenumax][t.tn].gadgettypevalue;
		t.slidersmenuvalueindex=t.slidersmenuvalue[g.slidersmenumax][t.tn].value;
		sliders_getnamefromvalue ( );
		t.slidersmenuvalue[g.slidersmenumax][t.tn].value_s=t.slidervaluename_s;
	}
	if ( g.gpbroverride == 0 )
	{
		t.slidersmenuvalue[g.slidersmenumax][5].name_s="Distance Transition";
		t.slidersmenuvalue[g.slidersmenumax][5].value=t.visuals.DistanceTransitionStart_f / 100;
		t.slidersmenuvalue[g.slidersmenumax][6].name_s="Transition Range";
		t.slidersmenuvalue[g.slidersmenumax][6].value=t.visuals.DistanceTransitionRange_f / 10;
	}

	//  YES/NO Message panel
	++g.slidersmenumax;
	t.slidersmenunames.yesnopanel=g.slidersmenumax;
	t.slidersmenu[g.slidersmenumax].tabpage=-9;
	t.slidersmenu[g.slidersmenumax].title_s="LOW FPS DETECTED";
	t.slidersmenu[g.slidersmenumax].thighlight=-1;
	t.slidersmenu[g.slidersmenumax].titlemargin=20;
	t.slidersmenu[g.slidersmenumax].leftmargin=25;
	t.slidersmenu[g.slidersmenumax].itemcount=0;
	t.slidersmenu[g.slidersmenumax].panelheight=55;
	t.slidersmenu[g.slidersmenumax].tleft=(GetDisplayWidth()-256)/2;
	t.slidersmenu[g.slidersmenumax].ttop=(GetDisplayHeight()-128)/2;
	t.slidersmenuvalue[g.slidersmenumax][1].name_s="Do you want to improve your performance?";
	t.slidersmenuvalue[g.slidersmenumax][2].name_s="[YES]            [NO]";
}

void sliders_free ( void )
{
	// no need to free as will be returning to in-game test
	g.slidersmenumax = 0;
}

void sliders_loop ( void )
{

	// Show / Hide LUA Sprites
	if ( g.tabmode != g.lastTabMode )
	{
		if ( g.tabmode > 0 )
			HideOrShowLUASprites ( true );
		else
			HideOrShowLUASprites ( false );
	}

	// store state of tab mode
	g.lastTabMode = g.tabmode;

	// direct or through editor
	if ( (g.tabmode == 2 || g.lowfpswarning == 1) && g.slidersprotoworkmode == 0 ) 
	{
		//PE: Make sure mouse dont leave the current window.
		RECT r;
		GetWindowRect(g_pGlob->hWnd, &r);
		ClipCursor(&r);

		if ( t.game.gameisexe == 1 ) 
		{
			// standalone
			t.tgamemousex_f = MouseX();
			t.tgamemousey_f = MouseY();
			t.tmouseclick = MouseClick();
		}
		else
		{
			#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
			//t.inputsys.xmouse is always at 960 in testgame.
			t.tgamemousex_f = MouseX(); //t.inputsys.xmouse; //((GetChildWindowWidth(-1) + 0.0) / (float)GetDisplayWidth()) * t.inputsys.xmouse;
			t.tgamemousey_f = MouseY(); //t.inputsys.ymouse; //((GetChildWindowHeight(-1) + 0.0) / (float)GetDisplayHeight()) * t.inputsys.ymouse;
			t.tmouseclick = MouseClick(); //t.inputsys.mclick;
			#else
			OpenFileMap (  1, "FPSEXCHANGE" );
			SetEventAndWait (  1 );
			//  from map editor client window (800x600)
			t.tgamemousex_f=GetFileMapDWORD( 1, 0 );
			t.tgamemousey_f=GetFileMapDWORD( 1, 4 );
			t.tgamemousex_f=t.tgamemousex_f/800.0;
			t.tgamemousey_f=t.tgamemousey_f/600.0;
			t.tgamemousex_f=t.tgamemousex_f*(GetDisplayWidth()+0.0);
			t.tgamemousey_f=t.tgamemousey_f*(GetDisplayHeight()+0.0);
			t.tmouseclick = MouseClick();
			//CloseFileMap (  1 );
			#endif
		}
	}
	else
	{
		if ( t.importer.importerActive == 1 || t.characterkit.inUse == 1 ) 
		{
			#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
			//In Editor we need to convert values.
			t.tgamemousex_f = t.inputsys.xmouse *((GetChildWindowWidth(-1) + 0.0) / (float)GetDisplayWidth());
			t.tgamemousey_f = t.inputsys.ymouse *((GetChildWindowHeight(-1) + 0.0) / (float)GetDisplayHeight());
			t.tmouseclick = t.inputsys.mclick;
			#else
			OpenFileMap (  1, "FPSEXCHANGE" );
			SetEventAndWait (  1 );
			//  from map editor client window (800x600)
			t.tgamemousex_f=GetFileMapDWORD( 1, 0 );
			t.tgamemousey_f=GetFileMapDWORD( 1, 4 );
			t.tgamemousex_f=t.tgamemousex_f*(GetChildWindowWidth()/800.0);
			t.tgamemousey_f=t.tgamemousey_f*(GetChildWindowHeight()/600.0);
			t.tmouseclick = t.inputsys.mclick;
			//CloseFileMap (  1 );
			#endif
		}
		else
		{
			t.tgamemousex_f = MouseX();
			t.tgamemousey_f = MouseY();
			t.tmouseclick = MouseClick();
		}
	}

	// control message panel
	t.slidersmenuindex=t.slidersmenunames.yesnopanel;
	if ( g.lowfpswarning == 1 ) 
	{
		if ( t.tmouseclick == 1 ) 
		{
			//  Yes or NO
			if ( t.tgamemousey_f>t.slidersmenu[t.slidersmenuindex].ttop && t.tgamemousey_f<t.slidersmenu[t.slidersmenuindex].ttop+128 ) 
			{
				if ( t.tgamemousex_f>t.slidersmenu[t.slidersmenuindex].tleft && t.tgamemousex_f<t.slidersmenu[t.slidersmenuindex].tleft+256 ) 
				{
					//  clicked YES or NO button?
					if ( t.tgamemousex_f>t.slidersmenu[t.slidersmenuindex].tleft && t.tgamemousex_f<t.slidersmenu[t.slidersmenuindex].tleft+128 ) 
					{
						//  YES
						t.visuals.TerrainLOD1_f=1000.0;
						t.visuals.TerrainLOD2_f=2000.0;
						t.visuals.TerrainLOD3_f=3000.0;
						t.visuals.TerrainSize_f=100.0;
						t.visuals.VegQuantity_f=25.0;
						t.visuals.VegWidth_f=90.0;
						t.visuals.VegHeight_f=70.0;
						t.slidersmenuindex=t.slidersmenunames.qualitypanel;
						t.slidersmenuvalue[t.slidersmenuindex][1].value=t.visuals.TerrainLOD1_f/100;
						t.slidersmenuvalue[t.slidersmenuindex][2].value=t.visuals.TerrainLOD2_f/100;
						t.slidersmenuvalue[t.slidersmenuindex][3].value=t.visuals.TerrainLOD3_f/100;
						t.slidersmenuvalue[t.slidersmenuindex][4].value=t.visuals.TerrainSize_f;
						t.slidersmenuvalue[t.slidersmenuindex][5].value=t.visuals.VegQuantity_f;
						t.slidersmenuvalue[t.slidersmenuindex][6].value=t.visuals.VegWidth_f;
						t.slidersmenuvalue[t.slidersmenuindex][7].value=t.visuals.VegHeight_f;
						t.visuals.reflectionmode=0;
						t.visuals.shadowmode=0;
						t.visuals.bloommode=0;
						t.visuals.lightraymode=0;
						t.visuals.vegetationmode=10;
						t.slidersmenuindex=t.slidersmenunames.graphicoptions;
						t.slidersmenuvalue[t.slidersmenuindex][1].value=t.visuals.reflectionmode;
						t.slidersmenuvalue[t.slidersmenuindex][2].value=t.visuals.shadowmode;
						t.slidersmenuvalue[t.slidersmenuindex][3].value=t.visuals.lightraymode;
						t.slidersmenuvalue[t.slidersmenuindex][4].value=t.visuals.vegetationmode;
						t.visuals.refreshvegetation=1;
						t.visuals.refreshshaders=1;
						//  reduce shader levels
						t.visuals.shaderlevels.terrain=3;
						t.visuals.shaderlevels.entities=2;
						t.visuals.shaderlevels.vegetation=2;
						t.visuals.shaderlevels.lighting=1;
						t.slidersmenuindex=t.slidersmenunames.shaderoptions;
						t.slidersmenuvalue[t.slidersmenuindex][1].value=t.visuals.shaderlevels.terrain;
						t.slidersmenuvalue[t.slidersmenuindex][2].value=t.visuals.shaderlevels.entities;
						t.slidersmenuvalue[t.slidersmenuindex][3].value=t.visuals.shaderlevels.vegetation;
						t.slidersmenuvalue[t.slidersmenuindex][4].value=t.visuals.shaderlevels.lighting;
						for ( t.tn = 1 ; t.tn<=  4; t.tn++ )
						{
							t.slidersmenuvaluechoice=t.slidersmenuvalue[t.slidersmenuindex][t.tn].gadgettypevalue;
							t.slidersmenuvalueindex=t.slidersmenuvalue[t.slidersmenuindex][t.tn].value;
							sliders_getnamefromvalue ( );
							t.slidersmenuvalue[t.slidersmenuindex][t.tn].value_s=t.slidervaluename_s;
						}
						visuals_shaderlevels_update ( );
					}
					else
					{
						//  NO
					}
					//  remove panel from now on
					g.lowfpswarning=2;
				}
			}
		}
	}
	else
	{
	//  control slider panels
	if (  t.tmouseclick == 0 && g.slidersmenufreshclick == 2  )  g.slidersmenufreshclick = 0;
	if (  g.slidersmenufreshclick == 1  )  g.slidersmenufreshclick = 2;
	if (  t.tmouseclick == 1 && g.slidersmenufreshclick == 0  )  g.slidersmenufreshclick = 1;
	// 261115 - first scan to ensure we are not clicking a header for panel dragging
	bool bHoveringOverAPanelHeader = false;
	for ( t.slidersmenuindex = 1 ; t.slidersmenuindex<=  g.slidersmenumax; t.slidersmenuindex++ )
	{
		t.tabviewflag=0;
		if (  t.slidersmenu[t.slidersmenuindex].tabpage == g.tabmode  )  t.tabviewflag = 1;
		if (  t.slidersmenu[t.slidersmenuindex].tabpage == -1 && g.tabmode>0  )  t.tabviewflag = 1;
		if (  t.slidersmenu[t.slidersmenuindex].tabpage == -2 && (g.tabmode == 0 || g.tabmode == 2)  )  t.tabviewflag = 1;
		if (  t.tabviewflag == 1 ) 
		{
			if ( t.slidersmenu[t.slidersmenuindex].thighlight == -1 ) 
			{
				t.tmx=t.tgamemousex_f-t.slidersmenu[t.slidersmenuindex].tleft;
				t.tmy=t.tgamemousey_f-t.slidersmenu[t.slidersmenuindex].ttop;
				t.tmargin=t.slidersmenu[t.slidersmenuindex].titlemargin;
				if ( t.tmy>0 && t.tmy<t.tmargin ) 
				{
					if ( t.tmx>0 && t.tmx<256 ) 
					{
						bHoveringOverAPanelHeader = true;
					}
				}
			}
		}
	}
	// now scan for slider menus
	for ( t.slidersmenuindex = 1 ; t.slidersmenuindex<=  g.slidersmenumax; t.slidersmenuindex++ )
	{
		t.tabviewflag=0;
		if (  t.slidersmenu[t.slidersmenuindex].tabpage == g.tabmode  )  t.tabviewflag = 1;
		if (  t.slidersmenu[t.slidersmenuindex].tabpage == -1 && g.tabmode>0  )  t.tabviewflag = 1;
		if (  t.slidersmenu[t.slidersmenuindex].tabpage == -2 && (g.tabmode == 0 || g.tabmode == 2)  )  t.tabviewflag = 1;
		if (  t.tabviewflag == 1 ) 
		{
			//  get relative mouse position and panel settings
			t.tmx=t.tgamemousex_f-t.slidersmenu[t.slidersmenuindex].tleft;
			t.tmy=t.tgamemousey_f-t.slidersmenu[t.slidersmenuindex].ttop;
			t.tmargin=t.slidersmenu[t.slidersmenuindex].titlemargin;
			t.tpanelheight=t.slidersmenu[t.slidersmenuindex].panelheight;
			t.tconx=t.slidersmenu[t.slidersmenuindex].leftmargin;
			//  alternative slider menu
			t.slidersmenualternativeindex=t.slidersmenuindex;
			if (  t.slidersmenuindex == t.slidersmenunames.performance ) 
			{
				if (  g.sliderspecialview>0 ) 
				{
					//  used for drill-down menus (such as A.I drill down)
					t.slidersmenualternativeindex=g.sliderspecialview;
				}
			}
			t.tnumoptions=t.slidersmenu[t.slidersmenualternativeindex].itemcount;
			//  drag or not
			if (  t.slidersmenu[t.slidersmenuindex].thighlight == -1 ) 
			{
				//  not dragging
				t.thighlight=0;
				if (  t.tmy>0 && t.tmy<t.tpanelheight+t.tmargin ) 
				{
					if (  t.tmx>0 && t.tmx<256 ) 
					{
						//  find if over any slider
						if ( bHoveringOverAPanelHeader==false )
						{
							if ( t.slidersmenu[t.slidersmenuindex].minimised==0 )
							{
						if (  t.slidersmenu[t.slidersmenuindex].readonly == 0 ) 
						{
							for ( t.t = 0 ; t.t<=  t.tnumoptions-1; t.t++ )
							{
								t.tcony=t.tmargin+(t.t*38);
								if (  t.tmy>t.tcony && t.tmy<t.tcony+38 ) 
								{
									t.thighlight=1+t.t;
								}
							}
						}
							}
						}
						// 261115 - find if over minmax button
						if ( t.slidersmenu[t.slidersmenuindex].minmaxbuttonlit != -1 )
						{
							t.slidersmenu[t.slidersmenuindex].minmaxbuttonlit = 0;
							if ( t.tmx>230 && t.tmx<230+13 && t.tmy>=20 && t.tmy<20+13 )
							{
								t.slidersmenu[t.slidersmenuindex].minmaxbuttonlit = 1;
							}
						}
						//  take action
						if (  t.tmouseclick == 0 && t.slidersmenu[t.slidersmenuindex].tclick == 2 ) { t.slidersmenu[t.slidersmenuindex].tclick = 0 ; t.slidersmenu[t.slidersmenuindex].tallowdragging = 0; t.bDraggingHeaderRightNow = false;}
						if (  t.tmouseclick == 1 && t.slidersmenu[t.slidersmenuindex].tclick == 0  )  t.slidersmenu[t.slidersmenuindex].tclick = 1;
						if (  t.slidersmenu[t.slidersmenuindex].tclick == 1 ) 
						{
							if ( t.thighlight>0 && t.bDraggingHeaderRightNow==false ) 
							{
								//  highlight an item
								t.thighlightedmenu=t.slidersmenuindex;
								t.t=t.thighlight;
								if (  t.slidersmenuvalue[t.slidersmenualternativeindex][t.t].gadgettype == 0 ) 
								{
									//  if special expanding slider
									if (  t.slidersmenuvalue[t.slidersmenualternativeindex][t.t].expanddetect != 0 ) 
									{
										//  expand A.I metrics into performance panel
										if (  t.slidersmenuvalue[t.slidersmenualternativeindex][t.t].expanddetect != -1 ) 
										{
											g.sliderspecialview=t.slidersmenuvalue[t.slidersmenualternativeindex][t.t].expanddetect;
										}
										else
										{
											g.sliderspecialview=0;
										}
									}
									else
									{
										// do not adjust metrics view
										if ( t.slidersmenuindex != 2 )
										{
											//  if slider
											if (  t.slidersmenuvalue[t.slidersmenuindex][t.t].useCustomRange  ==  1 ) 
											{
												//  (Dave) to allow for custom value
												t.slidersmenuvalue[t.slidersmenuindex][t.t].value=SlidersAdjustValue(t.slidersmenuvalue[t.slidersmenuindex][t.t].value,t.slidersmenuvalue[t.slidersmenuindex][t.t].valueMin,t.slidersmenuvalue[t.slidersmenuindex][t.t].valueMax,0,100);
											}
											t.tpos_f=t.slidersmenuvalue[t.slidersmenuindex][t.t].value ; t.tpos_f=(188.0/100.0)*t.tpos_f;
											t.tblobx=t.tconx+t.tpos_f;
											if ( t.tmx<t.tblobx-5 ) 
											{
												// step left
												t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value=t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value-10;
												if (  t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value<0  )  t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value = 0;
											}
											else
											{
												if ( t.tmx>t.tblobx+25 ) 
												{
													// step right
													t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value=t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value+10;
													if (  t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value>100  )  t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value = 100;
												}
												else
												{
													//  click to drag
													t.slidersmenu[t.slidersmenuindex].thighlight=t.t;
													t.slidersmenu[t.slidersmenuindex].tdrag=(t.tblobx-t.tmx)/2.0;
												}
											}
											if (  t.slidersmenuvalue[t.slidersmenuindex][t.t].useCustomRange  ==  1 ) 
											{
												//  (Dave) tranfer the bar value back to the custom range
												t.slidersmenuvalue[t.slidersmenuindex][t.t].value=SlidersAdjustValue(t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value,0,100,t.slidersmenuvalue[t.slidersmenuindex][t.t].valueMin,t.slidersmenuvalue[t.slidersmenuindex][t.t].valueMax);
											}
										}
									}
								}
								else
								{
									//  gadget
									if (  t.slidersmenuvalue[t.slidersmenuindex][t.t].gadgettype == 1 ) 
									{
										//  dropdown gadget
										t.slidersmenu[t.slidersmenuindex].thighlight=t.t;
										t.slidersdropdown=t.slidersmenuindex;
										t.slidersmenuvaluechoice=t.slidersmenuvalue[t.slidersmenuindex][t.t].gadgettypevalue;
										t.slidersdropdownchoice=t.slidersmenuvaluechoice;
										sliders_getchoice ( );
										t.slidersdropdownmax=t.sliderschoicemax;
										t.slidersdropdownindex=-1;
										t.rmposx=t.slidersmenu[t.slidersmenuindex].tleft;
										t.rmposy=t.slidersmenu[t.slidersmenuindex].ttop;
										t.rmposytopy=t.rmposy+t.slidersmenu[t.slidersmenuindex].titlemargin;
										t.slidersdropdownleft=t.rmposx+256-32-128;
										t.slidersdropdowntop=t.rmposytopy+((t.t-1)*38)+32;
										g.slidersmenudropdownscroll_f=1;
									}
								}
							}
							if ( t.thighlight <= 0 )
							{
								// drag the panel around
								if (  t.tmy>0 && t.tmy<t.tmargin ) 
								{
									if (  t.importer.importerActive == 0 ) 
									{
										//  dragging header start (dragging not allowed in importer)
										if (  g.slidersmenufreshclick == 1 ) 
										{
											t.slidersmenu[t.slidersmenuindex].tallowdragging=1;
										}
										if (  t.slidersmenu[t.slidersmenuindex].tallowdragging == 1 ) 
										{
											t.tsliderdragx_f=t.slidersmenu[t.slidersmenuindex].tleft-t.tgamemousex_f;
											t.tsliderdragy_f=t.slidersmenu[t.slidersmenuindex].ttop-t.tgamemousey_f;
											t.tsliderdragoriginaltleft = t.slidersmenu[t.slidersmenuindex].tleft;
											t.tsliderdragoriginalttop = t.slidersmenu[t.slidersmenuindex].ttop;
											t.bDraggingHeaderRightNow = true;
											if ( t.slidersmenu[t.slidersmenuindex].bFrozenPanelFromLastCycle==true )
												t.slidersmenu[t.slidersmenuindex].bPermitMovementEvenIfOverlap = true;
											else
												t.slidersmenu[t.slidersmenuindex].bPermitMovementEvenIfOverlap = false;

											t.slidersmenu[t.slidersmenuindex].thighlight=0;
										}
									}
								}
							}
							// 261115 - minimise functionality
							if ( t.slidersmenu[t.slidersmenuindex].minmaxbuttonlit==1 )
							{
								// minimise and maximise panel
								t.slidersmenu[t.slidersmenuindex].minimised = 1 - t.slidersmenu[t.slidersmenuindex].minimised;

								// if expand panel, also ensure we minimise any that get in our way
								if ( t.slidersmenu[t.slidersmenuindex].minimised==0 )
								{
									// work out size of maximised panel
									t.twholepanelheight = t.tmargin - 8;
									t.tpanelheight=((int)(t.slidersmenu[t.slidersmenuindex].panelheight/32))*32;
									t.twholepanelheight += t.tpanelheight + 20;

									// go through all panels, and squash and move overlapped panels
									for ( t.tcheckslidersmenuindex = 1 ; t.tcheckslidersmenuindex<=  g.slidersmenumax; t.tcheckslidersmenuindex++ )
									{
										if (  t.tcheckslidersmenuindex != t.slidersmenuindex ) 
										{
											t.tabviewflag=0;
											if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == g.tabmode  )  t.tabviewflag = 1;
											if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == -1 && g.tabmode>0  )  t.tabviewflag = 1;
											if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == -2 && (g.tabmode == 0 || g.tabmode == 2)  )  t.tabviewflag = 1;
											if (  t.tabviewflag == 1 ) 
											{
												// adjust for minimised panels
												t.twholethispanelheight = t.tmargin - 8;
												if ( t.slidersmenu[t.tcheckslidersmenuindex].minimised==1 )
												{
													t.twholethispanelheight += 8;
												}
												else
												{
													t.tpanelheight=((int)(t.slidersmenu[t.tcheckslidersmenuindex].panelheight/32))*32;
													t.twholethispanelheight += t.tpanelheight + 20;
												}

												// are we in this panel?
												bool bEnteredAnotherPanel = false;
												if ( t.slidersmenu[t.slidersmenuindex].tleft<t.slidersmenu[t.tcheckslidersmenuindex].tleft+256 ) 
													if ( t.slidersmenu[t.slidersmenuindex].tleft+256>t.slidersmenu[t.tcheckslidersmenuindex].tleft ) 
														if (t.slidersmenu[t.slidersmenuindex].ttop<t.slidersmenu[t.tcheckslidersmenuindex].ttop+t.twholethispanelheight ) 
															if ( t.slidersmenu[t.slidersmenuindex].ttop+t.twholepanelheight>t.slidersmenu[t.tcheckslidersmenuindex].ttop ) 
																bEnteredAnotherPanel = true;

												// squash and move
												if ( bEnteredAnotherPanel==true )
												{
													t.slidersmenu[t.tcheckslidersmenuindex].minimised = 1;
													t.slidersmenu[t.tcheckslidersmenuindex].ttop = t.slidersmenu[t.slidersmenuindex].ttop+t.twholepanelheight+1;
													int iMinimisedPanelHeight = t.tmargin;
													if ( t.slidersmenu[t.tcheckslidersmenuindex].ttop > GetDisplayHeight()-iMinimisedPanelHeight )  
														t.slidersmenu[t.tcheckslidersmenuindex].ttop = GetDisplayHeight()-iMinimisedPanelHeight;
												}
											}
										}
									}
								}
							}
							// Ensure it is a single click (then later release)
							t.slidersmenu[t.slidersmenuindex].tclick=2;
						}
					}
				}
			}
			else
			{
				//  dragging
				t.thighlight=t.slidersmenu[t.slidersmenuindex].thighlight;
				if (  t.tmouseclick == 0 && t.slidersmenu[t.slidersmenuindex].thighlight != -1 ) 
				{
					t.slidersmenu[t.slidersmenuindex].thighlight=-1;
					if (  t.slidersdropdown>0 ) 
					{
						if (  t.slidersdropdownindex != -1 ) 
						{
							t.slidersmenuvalue[t.slidersmenuindex][1+t.rmi].value=g.slidersmenudropdownscroll_f+t.slidersdropdownindex;
							t.slidersmenuvaluechoice=t.slidersmenuvalue[t.slidersmenuindex][1+t.rmi].gadgettypevalue;
							t.slidersmenuvalueindex=t.slidersmenuvalue[t.slidersmenuindex][1+t.rmi].value;
							sliders_getnamefromvalue ( );
							t.slidersmenuvalue[t.slidersmenuindex][1+t.rmi].value_s=t.slidervaluename_s;
							t.whichmenuitem = 1+t.rmi;
							sliders_write ( );
						}
						t.slidersdropdownindex=-1;
						t.slidersdropdown=0;
					}
				}
				if (  t.slidersmenu[t.slidersmenuindex].thighlight != -1 ) 
				{
					if (  t.thighlight>0 && t.thighlightedmenu==t.slidersmenuindex ) 
					{
						if (  t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].gadgettype == 0 ) 
						{
							//  slider control
							t.t=t.slidersmenu[t.slidersmenuindex].thighlight;
							t.tnewblobx=((t.tmx-t.tconx)/188.0)*100.0;
							//  Update panel value and call real-time update subroutine
							t.tValue=t.tnewblobx+t.slidersmenu[t.slidersmenuindex].tdrag;
							if (  t.tValue<0  )  t.tValue = 0;
							if (  t.tValue>100  )  t.tValue = 100;
							//  (Dave) take into account if there is a custom value range
							if (  t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].useCustomRange  ==  0 ) 
							{
								t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value=t.tValue;
							}
							else
							{
								t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].value=SlidersAdjustValue(t.tValue,0,100,t.slidersmenuvalue[t.slidersmenuindex][t.t].valueMin,t.slidersmenuvalue[t.slidersmenuindex][t.t].valueMax);
							}
							t.whichmenuitem = t.thighlight;
							sliders_write ( );
						}
						else
						{
							//  gadget
							if (  t.slidersmenuvalue[t.slidersmenuindex][t.thighlight].gadgettype == 1 ) 
							{
								//  dropdown highlighting sub-item
								if (  t.tgamemousey_f >= t.slidersdropdowntop+5 ) 
								{
									t.slidersdropdownindex=(t.tgamemousey_f-t.slidersdropdowntop-5)/16;
									if (  t.slidersdropdownindex<0  )  t.slidersdropdownindex = -1;
									if (  t.slidersdropdownindex >= t.slidersdropdownmax  )  t.slidersdropdownindex = -1;
								}
								else
								{
									t.slidersdropdownindex=-1;
								}
								//  can scroll contents of drop down
								if (  t.tgamemousey_f != 0 ) 
								{
									int iBottomOfScreenInUI = GetChildWindowHeight();
									if (  t.tgamemousey_f >= iBottomOfScreenInUI-40 || t.tgamemousey_f<0 ) 
									{
										g.slidersmenudropdownscroll_f += 0.4f;
										t.tlastchunkcanseemax=2+(t.slidersdropdownmax-((iBottomOfScreenInUI-t.slidersdropdowntop)/16));
										if (  g.slidersmenudropdownscroll_f>t.tlastchunkcanseemax ) 
										{
											g.slidersmenudropdownscroll_f=t.tlastchunkcanseemax;
										}
									}
									else
									{
										if (  t.tgamemousey_f <= t.slidersdropdowntop-5 ) 
										{
											g.slidersmenudropdownscroll_f -= 0.4f ; if (  g.slidersmenudropdownscroll_f<1  )  g.slidersmenudropdownscroll_f = 1;
										}
									}
								}
							}
						}
					}
					else
					{
						//  drag whole panel
						// adjust for minimised panel being moved
						t.twholepanelheight = t.tmargin - 8;
						if ( t.slidersmenu[t.slidersmenuindex].minimised==1 )
						{
							t.twholepanelheight += 8;
						}
						else
						{
							t.tpanelheight=((int)(t.slidersmenu[t.slidersmenuindex].panelheight/32))*32;
							t.twholepanelheight += t.tpanelheight + 20;
						}

						// ensure when getting mouse position, dont get if OFF 3D view area
						if ( t.tgamemousex_f < 40000.0f )
						{
						t.tnewpanelx_f=t.tsliderdragx_f+t.tgamemousex_f;
						t.tnewpanely_f=t.tsliderdragy_f+t.tgamemousey_f;
						}
						if (  t.tnewpanelx_f<0  )  t.tnewpanelx_f = 0;
						if (  t.tnewpanely_f<0  )  t.tnewpanely_f = 0;
						if (  t.tnewpanelx_f>GetDisplayWidth()-256-16  )  t.tnewpanelx_f = GetDisplayWidth()-256-16;
						if (  t.tnewpanely_f>GetDisplayHeight()-t.twholepanelheight  )  t.tnewpanely_f = GetDisplayHeight()-t.twholepanelheight;

						//  ensure new position never overlaps existing panel
						for ( t.tcheckslidersmenuindex = 1 ; t.tcheckslidersmenuindex<=  g.slidersmenumax; t.tcheckslidersmenuindex++ )
						{
							if (  t.tcheckslidersmenuindex != t.slidersmenuindex ) 
							{
								t.tabviewflag=0;
								if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == g.tabmode  )  t.tabviewflag = 1;
								if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == -1 && g.tabmode>0  )  t.tabviewflag = 1;
								if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == -2 && (g.tabmode == 0 || g.tabmode == 2)  )  t.tabviewflag = 1;
								if (  t.tabviewflag == 1 ) 
								{
									// adjust for minimised panels
									t.twholethispanelheight = t.tmargin - 8;
									if ( t.slidersmenu[t.tcheckslidersmenuindex].minimised==1 )
									{
										t.twholethispanelheight += 8;
									}
									else
									{
										t.tpanelheight=((int)(t.slidersmenu[t.tcheckslidersmenuindex].panelheight/32))*32;
										t.twholethispanelheight += t.tpanelheight + 20;
									}

									// are we in this panel?
									bool bEnteredAnotherPanel = false;
									if (  t.tnewpanelx_f<t.slidersmenu[t.tcheckslidersmenuindex].tleft+256 ) 
										if (  t.tnewpanelx_f+256>t.slidersmenu[t.tcheckslidersmenuindex].tleft ) 
											if (  t.tnewpanely_f<t.slidersmenu[t.tcheckslidersmenuindex].ttop+t.twholethispanelheight ) 
												if (  t.tnewpanely_f+t.twholepanelheight>t.slidersmenu[t.tcheckslidersmenuindex].ttop ) 
													bEnteredAnotherPanel = true;

									// yes we are
									if ( bEnteredAnotherPanel==true )
												{
										// determine which edge is best
										int iBestEdge = 0;
										int iBestDistance = 99999;
										int iLeft = abs ( t.slidersmenu[t.slidersmenuindex].tleft - (t.slidersmenu[t.tcheckslidersmenuindex].tleft+256) );
										int iRight = abs ( (t.slidersmenu[t.slidersmenuindex].tleft+256) - t.slidersmenu[t.tcheckslidersmenuindex].tleft );
										int iTop = abs ( t.slidersmenu[t.slidersmenuindex].ttop - (t.slidersmenu[t.tcheckslidersmenuindex].ttop + t.twholethispanelheight) );
										int iBottom = abs ( (t.slidersmenu[t.slidersmenuindex].ttop+t.twholepanelheight) - t.slidersmenu[t.tcheckslidersmenuindex].ttop );
										if ( iLeft < iBestDistance ) { iBestEdge=0; iBestDistance=iLeft; }
										if ( iRight < iBestDistance ) { iBestEdge=1; iBestDistance=iRight; }
										if ( iTop < iBestDistance ) { iBestEdge=2; iBestDistance=iTop; }
										if ( iBottom < iBestDistance ) { iBestEdge=3; iBestDistance=iBottom; }

										// check if moving by X solves it
										if ( iBestEdge==0 || iBestEdge==1 )
										{
											if ( t.tsliderdragoriginaltleft+128 < t.slidersmenu[t.tcheckslidersmenuindex].tleft+128 ) 
											{
												t.tnewpanelx_f = t.slidersmenu[t.tcheckslidersmenuindex].tleft - 256 - 1;
												}
											if ( t.tsliderdragoriginaltleft+128 > t.slidersmenu[t.tcheckslidersmenuindex].tleft+128 ) 
											{
												t.tnewpanelx_f = t.slidersmenu[t.tcheckslidersmenuindex].tleft+256+1;
											}
										}
										else
										{
											if ( t.tsliderdragoriginalttop+(t.twholethispanelheight/2) < t.slidersmenu[t.tcheckslidersmenuindex].ttop+(t.twholethispanelheight/2) ) 
											{
												t.tnewpanely_f = t.slidersmenu[t.tcheckslidersmenuindex].ttop - t.twholepanelheight - 1;
									}
											if ( t.tsliderdragoriginalttop+(t.twholethispanelheight/2) > t.slidersmenu[t.tcheckslidersmenuindex].ttop+(t.twholethispanelheight/2) ) 
											{
												t.tnewpanely_f = t.slidersmenu[t.tcheckslidersmenuindex].ttop+t.twholethispanelheight + 1;
								}
							}
						}
								}
							}
						}

						// one final screen bounds check before we apply to actual panel
						if (  t.tnewpanelx_f<0  )  t.tnewpanelx_f = 0;
						if (  t.tnewpanely_f<0  )  t.tnewpanely_f = 0;
						if (  t.tnewpanelx_f>GetDisplayWidth()-256-16  )  t.tnewpanelx_f = GetDisplayWidth()-256-16;
						if (  t.tnewpanely_f>GetDisplayHeight()-t.twholepanelheight  )  t.tnewpanely_f = GetDisplayHeight()-t.twholepanelheight;
						bool bStillInsideAPanel = false;
						for ( t.tcheckslidersmenuindex = 1 ; t.tcheckslidersmenuindex<=  g.slidersmenumax; t.tcheckslidersmenuindex++ )
						{
							if (  t.tcheckslidersmenuindex != t.slidersmenuindex ) 
							{
								t.tabviewflag=0;
								if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == g.tabmode  )  t.tabviewflag = 1;
								if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == -1 && g.tabmode>0  )  t.tabviewflag = 1;
								if (  t.slidersmenu[t.tcheckslidersmenuindex].tabpage == -2 && (g.tabmode == 0 || g.tabmode == 2)  )  t.tabviewflag = 1;
								if (  t.tabviewflag == 1 ) 
								{
									// adjust for minimised panels
									t.twholethispanelheight = t.tmargin - 8;
									if ( t.slidersmenu[t.tcheckslidersmenuindex].minimised==1 )
									{
										t.twholethispanelheight += 8;
									}
									else
									{
										t.tpanelheight=((int)(t.slidersmenu[t.tcheckslidersmenuindex].panelheight/32))*32;
										t.twholethispanelheight += t.tpanelheight + 20;
									}
									// and check this panel
									if ( t.tnewpanelx_f+1<t.slidersmenu[t.tcheckslidersmenuindex].tleft+256 ) 
										if ( t.tnewpanelx_f+256-8>t.slidersmenu[t.tcheckslidersmenuindex].tleft ) 
											if (t.tnewpanely_f+1<t.slidersmenu[t.tcheckslidersmenuindex].ttop+t.twholethispanelheight ) 
												if ( t.tnewpanely_f+t.twholepanelheight-8>t.slidersmenu[t.tcheckslidersmenuindex].ttop ) 
													bStillInsideAPanel = true;
								}
							}
						}

						// remember this panel overlaps, to allow us to move it around
						// next time around (otherwise this flag above freezes the panel)
						if ( bStillInsideAPanel==true )
							t.slidersmenu[t.slidersmenuindex].bFrozenPanelFromLastCycle = true;
						else
							t.slidersmenu[t.slidersmenuindex].bFrozenPanelFromLastCycle = false;

						// decide if new panel position warranted
						if ( bStillInsideAPanel==false || t.slidersmenu[t.slidersmenuindex].bPermitMovementEvenIfOverlap==true )
						{
							// give panel its new good position							
							t.slidersmenu[t.slidersmenuindex].tleft=t.tnewpanelx_f;
							t.slidersmenu[t.slidersmenuindex].ttop=t.tnewpanely_f;
						}
					}
				}
			}
		}
	}
	}
}

void sliders_readall ( void )
{

	//  Populate slider menu when called
	if (  t.slidersmenuindex == t.slidersmenunames.performance ) 
	{
		//  Performance Readouts
		t.slidersmenu[t.slidersmenuindex].title_s=cstr("METRICS  ")+Str(GetDisplayFPS())+"fps  "+Str(int(t.sky.currenthour_f))+"HR";
		t.game.perf.smoothcounter=t.game.perf.smoothcounter+1;
		if (  t.game.perf.overall>0 && t.game.perf.smoothcounter>5 ) 
		{
			t.game.perf.smoothcounter=0;
			t.toverall_f=t.game.perf.overall;
			t.perftai_f=(t.game.perf.ai+0.0)/t.toverall_f;
			t.perftai1_f=(t.game.perf.ai1+0.0)/t.toverall_f;
			t.perftai2_f=(t.game.perf.ai2+0.0)/t.toverall_f;
			t.perftai3_f=(t.game.perf.ai3+0.0)/t.toverall_f;
			t.perftai4_f=(t.game.perf.ai4+0.0)/t.toverall_f;
			t.perftai5_f=(t.game.perf.ai5+0.0)/t.toverall_f;
			t.perftai6_f=(t.game.perf.ai6+0.0)/t.toverall_f;
			t.perftphysics_f=(t.game.perf.physics+0.0)/t.toverall_f;
			t.perftterrain1_f=(t.game.perf.terrain1+0.0)/t.toverall_f;
			t.perftterrain2_f=(t.game.perf.terrain2+0.0)/t.toverall_f;
			t.perftterrain3_f=(t.game.perf.terrain3+0.0)/t.toverall_f;
			t.perftsynctime_f=(t.game.perf.synctime+0.0)/t.toverall_f;
			t.perftmisc1_f=(t.game.perf.misc+0.0)/t.toverall_f;
			t.perftmisc2_f=(t.game.perf.resttosync+0.0)/t.toverall_f;
			t.perftmisc3_f=(t.game.perf.gun+0.0)/t.toverall_f;
			t.perftmisc4_f=(t.game.perf.occlusion+0.0)/t.toverall_f;
			t.perftmisc5_f=(t.game.perf.postprocessing+0.0)/t.toverall_f;
			t.game.perf.overall=0;
			t.game.perf.ai=0;
			t.game.perf.ai1=0;
			t.game.perf.ai2=0;
			t.game.perf.ai3=0;
			t.game.perf.ai4=0;
			t.game.perf.ai5=0;
			t.game.perf.ai6=0;
			t.game.perf.physics=0;
			t.game.perf.terrain1=0;
			t.game.perf.terrain2=0;
			t.game.perf.terrain3=0;
			t.game.perf.synctime=0;
			t.game.perf.misc=0;
			t.game.perf.resttosync=0;
			t.game.perf.gun=0;
			t.game.perf.occlusion=0;
			t.game.perf.postprocessing=0;
		}
		if (  g.sliderspecialview>0 ) 
		{
			//  A.I drill-down statistics
			t.slidersmenuvalue[g.sliderspecialview][1].value=CurveValue(t.perftai1_f*100.0,t.slidersmenuvalue[g.sliderspecialview][1].value,2.0);
			t.slidersmenuvalue[g.sliderspecialview][2].value=CurveValue(t.perftai2_f*100.0,t.slidersmenuvalue[g.sliderspecialview][2].value,2.0);
			t.slidersmenuvalue[g.sliderspecialview][3].value=CurveValue(t.perftai3_f*100.0,t.slidersmenuvalue[g.sliderspecialview][3].value,2.0);
			t.slidersmenuvalue[g.sliderspecialview][4].value=CurveValue(t.perftai4_f*100.0,t.slidersmenuvalue[g.sliderspecialview][4].value,2.0);
			t.slidersmenuvalue[g.sliderspecialview][5].value=CurveValue(t.perftai5_f*100.0,t.slidersmenuvalue[g.sliderspecialview][5].value,2.0);
			t.slidersmenuvalue[g.sliderspecialview][6].value=CurveValue(t.perftai6_f*100.0,t.slidersmenuvalue[g.sliderspecialview][6].value,2.0);
		}
		else
		{
			//  Default performance statistics
			t.slidersmenuvalue[t.slidersmenuindex][1].value=CurveValue(t.perftai_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][1].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][2].value=CurveValue(t.perftphysics_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][2].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][3].value=CurveValue(t.perftterrain1_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][3].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][4].value=CurveValue(t.perftterrain2_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][4].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][5].value=CurveValue(t.perftterrain3_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][5].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][6].value=CurveValue(t.perftsynctime_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][6].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][7].value=CurveValue(t.perftmisc1_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][7].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][8].value=CurveValue(t.perftmisc2_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][8].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][9].value=CurveValue(t.perftmisc3_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][9].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][10].value=CurveValue(t.perftmisc4_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][10].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][11].value=CurveValue(t.perftmisc5_f*100.0,t.slidersmenuvalue[t.slidersmenuindex][11].value,2.0);
			t.slidersmenuvalue[t.slidersmenuindex][12].value=t.mainstatistic1/2000;
			t.slidersmenuvalue[t.slidersmenuindex][13].value=t.mainstatistic5/3;
		}
	}
	if (  t.slidersmenuindex == t.slidersmenunames.weapon ) 
	{
		if (  g.weaponammoindex>0 ) 
		{
			t.slidersmenuvalue[t.slidersmenuindex][1].value=t.weaponammo[g.weaponammoindex+g.ammooffset];
			t.tfiremode=t.gun[t.gunid].settings.alternate;
			t.tpool=g.firemodes[t.gunid][t.tfiremode].settings.poolindex;
			if (  t.tpool == 0  )  t.tammo = t.weaponclipammo[g.weaponammoindex+g.ammooffset]; else t.tammo = t.ammopool[t.tpool].ammo;
			t.slidersmenuvalue[t.slidersmenuindex][2].value=t.tammo;
		}
		else
		{
			t.slidersmenuvalue[t.slidersmenuindex][1].value=0;
			t.slidersmenuvalue[t.slidersmenuindex][2].value=0;
		}
	}
	if (  t.slidersmenuindex == t.slidersmenunames.player ) 
	{
		//  Player readout
		t.slidersmenuvalue[t.slidersmenuindex][1].value=t.player[1].lives;
		t.slidersmenuvalue[t.slidersmenuindex][2].value=t.player[1].health;
	}
}

void sliders_draw ( void )
{
	// LUA PROMPT IMAGES
	if (  t.promptimage.show == 1 && t.conkit.editmodeactive == 0 ) 
	{
		// image prompt 2D
		if (  ImageExist(t.promptimage.img) == 1 ) 
		{
			if (  t.promptimage.alignment == 0 ) 
			{
				t.tx_f=ImageWidth(t.promptimage.img)/2.0;
				t.ty_f=ImageHeight(t.promptimage.img)/2.0;
			}
			else
			{
				t.tx_f=0 ; t.ty_f=0;
			}
			t.tx_f=((GetDisplayWidth()/100.0)*t.promptimage.x)-t.tx_f;
			t.ty_f=((GetDisplayHeight()/100.0)*t.promptimage.y)-t.ty_f;
			PasteImage (  t.promptimage.img,t.tx_f,t.ty_f,1 );
		}
	}

	//  new crosshair system (avoids motion blur issue)
	if (  t.gunid>0 && t.postprocessings.fadeinvalue_f == 1.0 ) 
	{
		if (  g.firemodes[t.gunid][g.firemode].settings.zoomhidecrosshair == 0 || t.gunzoommode == 0 ) 
		{
			t.timg=t.gun[t.gunid].settings.crosshairimg;
			if (  t.timg>0 ) 
			{
				if (  ImageExist(t.timg) == 1 ) 
				{
					t.tokay=1;
					if (  t.playercontrol.thirdperson.enabled == 1 && t.playercontrol.thirdperson.camerareticle == 0 ) 
					{
						t.tokay=0;
					}
					if (  t.tokay == 1 ) 
					{
						t.timgx=(GetDisplayWidth()-ImageWidth(t.timg))/2;
						if (  t.playercontrol.thirdperson.enabled == 1 ) 
						{
							t.timgy=(GetDisplayHeight()-ImageHeight(t.timg))/4;
							PasteImage (  t.timg,t.timgx,t.timgy,1 );
						}
						else
						{
							t.timgy=(GetDisplayHeight()-ImageHeight(t.timg))/2;
							PasteImage (  t.timg,t.timgx,t.timgy,1 );
						}
					}
				}
			}
		}
	}

	//  LUA PROMPT on HUD (set from within script)
	if (  t.luaglobal.scriptprompttime>0 && t.conkit.editmodeactive == 0 ) 
	{
		if ( (DWORD)(Timer())<t.luaglobal.scriptprompttime+500 ) 
		{
			t.tscriptprompttextsize=t.luaglobal.scriptprompttextsize;
			t.tscriptprompttextsizeyoffset=t.tscriptprompttextsize*20;
			if (  t.tscriptprompttextsize == 0 ) { t.tscriptprompttextsize = 3  ; t.tscriptprompttextsizeyoffset = 50; }
			t.tscriptpromptx=(GetDisplayWidth()-getbitmapfontwidth(t.luaglobal.scriptprompt_s.Get(),t.tscriptprompttextsize))/2;
			//if ( g_VR920RenderStereoNow == true )
			//{
			//	// friendlier text for VR stereo rendering
			//	t.tscriptprompttextsize = 5; // LARGE TEXT - NOT IDEAL THIS!
			//	if ( g_StereoEyeToggle == 0 ) 
			//		t.tscriptpromptx -= 2;
			//	else
			//		t.tscriptpromptx += 2;
			//	pastebitmapfont(t.luaglobal.scriptprompt_s.Get(),t.tscriptpromptx,GetDisplayHeight()/2,t.tscriptprompttextsize,255);
			//}
			//else
			//{
			pastebitmapfont(t.luaglobal.scriptprompt_s.Get(),t.tscriptpromptx,GetDisplayHeight()-t.tscriptprompttextsizeyoffset,t.tscriptprompttextsize,255);
			//}
		}
		else
		{
			t.luaglobal.scriptprompttime=0;
		}
	}

	// manage prompt 3D
	if ( t.luaglobal.scriptprompt3dtime>0 && t.conkit.editmodeactive == 0 ) 
	{
		if ( (DWORD)(Timer())<t.luaglobal.scriptprompt3dtime+500 ) 
			lua_updateprompt3d();
		else
			lua_hideprompt3d();
	}

	// disable all HUDs if meet condition
	bool bAllowFPShileHUDSHidden = false;
	t.tokay=1;
	if ( g.globals.riftmode > 0 )  t.tokay = 0;
	if ( t.conkit.editmodeactive == 1 )  t.tokay = 0;
	if ( t.game.gameisexe == 1 )
	{
		if ( ( g.tabmodehidehuds == 1 || g.ghideallhuds == 1 ) && g.lowfpswarning != 1 && g.tabmode == 0 )  
		{
			bAllowFPShileHUDSHidden = true;
			t.tokay = 0;
		}
	}
	else
	{
		if ( ( g.tabmodehidehuds == 1 || g.ghideallhuds == 1 ) && g.lowfpswarning != 1 && g.tabmodeshowfps != 1 && g.tabmode == 0 )  t.tokay = 0;
	}
	if ( t.importer.importerActive == 1 ) t.tokay = 1;
	if ( t.game.runasmultiplayer == 1 ) g.ghardwareinfomode = 0;

	// also disable if in standalone and trying to use HUD views
	if ( t.game.gameisexe == 1 && g.gprofileinstandalone == 0 ) 
	{
		g.ghardwareinfomode = 0;
		g.tabmode = 0;
	}

	// Special tab mode to show JUST FPS score
	if ( (t.tokay == 1 || bAllowFPShileHUDSHidden == true ) && g.tabmode == 0 && g.tabmodeshowfps != 0 ) 
	{
		pastebitmapfont( cstr(cstr(Str(GetDisplayFPS()))+"fps").Get(),8,8,1,255);
	}

	// If HUDs allowed
	if ( t.tokay == 1 ) 
	{
		//  Special hardware info mode and controls
		if (  g.tabmodeshowfps == 1 && g.ghardwareinfomode != 0 ) 
		{
			//  Gather information
			if (  t.hardwareinfoglobals.gotgraphicsinfo == 0 ) 
			{
				t.hardwareinfoglobals.card_s=CurrentGraphicsCard();
				t.hardwareinfoglobals.dxversion_s=GetDXName();
				t.hardwareinfoglobals.monitorrefresh_s=GetDirectRefreshRate();
				t.hardwareinfoglobals.gotgraphicsinfo=1;
			}

			//  Grey Box (  to contrast Text (  better ) )
			//t.fcol=(128<<24)+(0<<16)+(0<<8)+0;
			GGBoxGradient (  0,GetDisplayHeight()-95,GetDisplayWidth(),GetDisplayHeight(),128,0,0,0);//t.fcol,t.fcol,t.fcol,t.fcol );

			//  Show in realtime (F11 panel)
			pastebitmapfontcenter( cstr(cstr("Hardware Info Mode  (V:")+Str(g.gversion)+")").Get(),GetDisplayWidth()/2,GetDisplayHeight()-90,1,255);
			pastebitmapfontcenter( cstr(cstr("GPU:")+t.hardwareinfoglobals.card_s+"  Resolution:"+Str(GetDisplayWidth())+"x"+Str(GetDisplayHeight())).Get(),GetDisplayWidth()/2,GetDisplayHeight()-65,1,255);
			t.t1_s="terrain";
			t.t2_s="water";
			#ifdef VRTECH
			 t.t3_s="hud";
			#else
			 t.t3_s="gun";
			#endif
			t.t4_s="lmos";
			t.t5_s="sky";
			t.t6_s="physics";
			t.t7_s="ai";
			t.t8_s="grass";
			t.t9_s="entity";
			if (  t.hardwareinfoglobals.noterrain == 1  )  t.t1_s = Upper(t.t1_s.Get());
			if (  t.hardwareinfoglobals.nowater != 0  )  t.t2_s = Upper(t.t2_s.Get());
			if (  t.hardwareinfoglobals.noguns == 1  )  t.t3_s = Upper(t.t3_s.Get());
			if (  t.hardwareinfoglobals.nolmos == 1  )  t.t4_s = Upper(t.t4_s.Get());
			if (  t.hardwareinfoglobals.nosky == 1  )  t.t5_s = Upper(t.t5_s.Get());
			if (  t.hardwareinfoglobals.nophysics == 1  )  t.t6_s = Upper(t.t6_s.Get());
			if (  t.hardwareinfoglobals.noai == 1  )  t.t7_s = Upper(t.t7_s.Get());
			if (  t.hardwareinfoglobals.nograss == 1  )  t.t8_s = Upper(t.t8_s.Get());
			if (  t.hardwareinfoglobals.noentities == 1  )  t.t9_s = Upper(t.t9_s.Get());
			pastebitmapfontcenter( cstr(cstr("[1] ")+t.t1_s+"  [2] "+t.t2_s+"  [3] "+t.t3_s+"  [4] "+t.t4_s+"  [5] "+t.t5_s+"  [6] "+t.t6_s+"  [7] "+t.t7_s+"  [8] "+t.t8_s+"  [9] "+t.t9_s).Get(),GetDisplayWidth()/2,GetDisplayHeight()-40,1,255);
			if (  ScanCode() == 0  )  t.hardwareinfoglobals.togglepress = 0;
			if (  ScanCode()>0 && t.hardwareinfoglobals.togglepress == 0 ) 
			{
				t.hardwareinfoglobals.togglepress=1;
				t.ttkeypressed=ScanCode()-1;
				switch (  t.ttkeypressed ) 
				{
					case 1:
						t.hardwareinfoglobals.noterrain=1-t.hardwareinfoglobals.noterrain;
						if (  t.terrain.TerrainID == 0 ) 
						{
							if (  t.hardwareinfoglobals.noterrain == 0 ) 
							{
								if (  ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
								{
									ShowObject (  t.terrain.terrainobjectindex );
								}
							}
							else
							{
								if (  ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
								{
									HideObject (  t.terrain.terrainobjectindex );
								}
							}
						}
					break;
					case 2:
						if (  t.hardwareinfoglobals.nowater == 2 ) 
						{
							t.hardwareinfoglobals.nowater=0;
						}
						else
						{
							t.hardwareinfoglobals.nowater=1-t.hardwareinfoglobals.nowater;
						}
						terrain_updatewatermechanism ( );
					break;
					case 3:
						t.hardwareinfoglobals.noguns=1-t.hardwareinfoglobals.noguns;
					break;
					case 4:
						t.hardwareinfoglobals.nolmos=1-t.hardwareinfoglobals.nolmos;
						if (  t.hardwareinfoglobals.nolmos == 1 ) 
						{
							lm_hide_lmos ( );
						}
						else
						{
							lm_show_lmos ( );
						}
					break;
					case 5:
						t.hardwareinfoglobals.nosky=1-t.hardwareinfoglobals.nosky;
						if (  t.hardwareinfoglobals.nosky == 1 ) 
						{
							//  hide sky
							if (  ObjectExist(t.terrain.objectstartindex+4) == 1  )  HideObject (  t.terrain.objectstartindex+4 );
							if (  ObjectExist(t.terrain.objectstartindex+8) == 1  )  HideObject (  t.terrain.objectstartindex+8 );
							if (  ObjectExist(t.terrain.objectstartindex+9) == 1  )  HideObject (  t.terrain.objectstartindex+9 );
						}
						else
						{
							//  reshow sky
							if (  ObjectExist(t.terrain.objectstartindex+4) == 1  )  ShowObject (  t.terrain.objectstartindex+4 );
							if (  ObjectExist(t.terrain.objectstartindex+8) == 1  )  ShowObject (  t.terrain.objectstartindex+8 );
							if (  ObjectExist(t.terrain.objectstartindex+9) == 1  )  ShowObject (  t.terrain.objectstartindex+9 );
						}
					break;
					case 6:
						t.hardwareinfoglobals.nophysics=1-t.hardwareinfoglobals.nophysics;
					break;
					case 7:
						t.hardwareinfoglobals.noai=1-t.hardwareinfoglobals.noai;
					break;
					case 8:
						t.hardwareinfoglobals.nograss=1-t.hardwareinfoglobals.nograss;
						if (  t.hardwareinfoglobals.nograss == 1 ) 
						{
							HideVegetationGrid (  );
						}
						else
						{
							ShowVegetationGrid (  );
						}
					break;
					case 9:
						t.hardwareinfoglobals.noentities=1-t.hardwareinfoglobals.noentities;
						if (  t.hardwareinfoglobals.noentities == 1 ) 
						{
							//  hide entities
							for ( t.te = 1 ; t.te<=  g.entityelementlist; t.te++ )
							{
								t.tobj=t.entityelement[t.te].obj;
								t.tentid=t.entityelement[t.te].bankindex;
								if (  t.entityprofile[t.tentid].ismarker == 0 ) 
								{
									if (  t.tobj>0 ) 
									{
										if (  ObjectExist(t.tobj) == 1  )  HideObject (  t.tobj );
									}
								}
							}
							//  hide attachments
							for ( t.tobj = g.entityattachmentsoffset+1 ; t.tobj<=  g.entityattachmentsoffset+g.entityattachmentindex; t.tobj++ )
							{
								if (  ObjectExist(t.tobj) == 1  )  HideObject (  t.tobj );
							}
						}
						else
						{
							//  show entities
							//PE: Need to support draw call optimizer.
							for ( t.te = 1 ; t.te<=  g.entityelementlist; t.te++ )
							{
								t.tentid=t.entityelement[t.te].bankindex;
								if (  t.entityprofile[t.tentid].ismarker == 0 ) 
								{
									if (t.entityelement[t.te].dc_merged == false ) {
										t.tobj = t.entityelement[t.te].obj;
										if (t.tobj > 0)
										{
											if (ObjectExist(t.tobj) == 1)  ShowObject(t.tobj);
										}
									}
								}
							}
							//  show attachments
							for ( t.tobj = g.entityattachmentsoffset+1 ; t.tobj<=  g.entityattachmentsoffset+g.entityattachmentindex; t.tobj++ )
							{
								if (  ObjectExist(t.tobj) == 1  )  ShowObject (  t.tobj );
							}
						}
					break;
				}			
			}

			//  No more slider art in hardware info debug mode
			return;
		}
	}

	// 100316 - draw any scope HUD first (moved out of condition below so it ALWAYS renders for the game if active)
	sliders_scope_draw ( );

	// 080917 - collects sprite immediate draw calls (text,images) so can do in one go 
	ResetSpriteBatcher();

	// only if HUDs allowed to be drawn
	if (  t.tokay == 1 ) 
	{
		//  draw slider menus
		for ( t.slidersmenuindex = 1 ; t.slidersmenuindex<=  g.slidersmenumax; t.slidersmenuindex++ )
		{
			t.tabviewflag=0;
			if ( t.slidersmenu[t.slidersmenuindex].tabpage == g.tabmode  )  t.tabviewflag = 1;
			if ( t.slidersmenu[t.slidersmenuindex].tabpage == -1 && g.tabmode>0 && g.tabmode<3  )  t.tabviewflag = 1;
			if ( t.slidersmenu[t.slidersmenuindex].tabpage == -2 && (g.tabmode == 0 || g.tabmode == 2)  )  t.tabviewflag = 1;
			if ( t.slidersmenu[t.slidersmenuindex].tabpage == -9 && g.lowfpswarning == 1  )  t.tabviewflag = 1;

			if ( t.tabviewflag == 1 ) 
			{
				t.rmposx=t.slidersmenu[t.slidersmenuindex].tleft;
				t.rmposy=t.slidersmenu[t.slidersmenuindex].ttop;
				t.timgbase=g.slidersmenuimageoffset;
				t.tpanely=t.rmposy;
				if ( t.slidersmenu[t.slidersmenuindex].customimage>0 ) 
				{
					// custom panel (ammo, health)
					if ( t.slidersmenu[t.slidersmenuindex].customimagetype == 1 && t.gunid>0 ) 
					{
						//if ( g.quickparentalcontrolmode == 2 || t.gun[t.gunid].weapontype == 51 || g.firemodes[t.gunid][g.firemode].settings.reloadqty == 0 ) 
						if ( g.quickparentalcontrolmode == 2 || t.gun[t.gunid].weapontype == 51 ) 
						{
							// melee has no ammo panel
						}
						else
						{
							// ammo panel (flash if no ammo or weapon jammed)
							if ( SpriteExist(g.ammopanelsprite)  ==  0  )  Sprite (  g.ammopanelsprite,-1000,-1000,t.timgbase+51 );
							t.tflashforjam = 0 ; if ( g.firemodes[t.gunid][g.firemode].settings.jammed == 1 )  t.tflashforjam = 1;
							if (  t.slidersmenuvalue[t.slidersmenuindex][1].value <= 0 || t.tflashforjam == 1 ) 
							{
								t.tDiffuseR = (1+Cos(Timer()/2.5))*100;
								if (  t.slidersmenuvalue[t.slidersmenuindex][2].value <= 0 || t.tflashforjam == 1 ) 
								{
									t.tDiffuseG = 0;
								}
								else
								{
									t.tDiffuseG = t.tDiffuseR;
								}
							}
							else
							{
								t.tDiffuseR = 0;
								t.tDiffuseG = 0;
							}
							SetSpriteDiffuse (  g.ammopanelsprite,t.tDiffuseR,t.tDiffuseG,0 );
							PasteSprite (  g.ammopanelsprite,t.rmposx,t.tpanely );
							PasteImage (  t.timgbase+52,t.rmposx+12,t.tpanely+64,1 );

							t.slidersmenu[t.slidersmenuindex].customimagesubtype=t.gun[t.gunid].statuspanelcode;
							if (  t.slidersmenu[t.slidersmenuindex].customimagesubtype == 100 ) 
							{
								t.timgtouse=g.firemodes[t.gunid][g.firemode].iconimg;
								if (  t.timgtouse == 0  )  t.timgtouse = g.firemodes[t.gunid][0].iconimg;
							}
							else
							{
								t.timgtouse=t.timgbase+54+t.slidersmenu[t.slidersmenuindex].customimagesubtype;
							}
							if (  ImageExist(t.timgtouse) == 1 ) 
							{
								PasteImage (  t.timgtouse,t.rmposx+115-(ImageWidth(t.timgtouse)/2),t.tpanely+75,1 );
							}

							if ( g.firemodes[t.gunid][g.firemode].settings.reloadqty == 0 )
							{
								// use infinity symbol instead of hiding ammo panel
								PasteImage (  t.timgbase+93,(t.rmposx+141)-69,t.tpanely+22,1 );
							}
							else
							{
								//  ammo panel icons
								if (  t.slidersmenu[t.slidersmenuindex].customimagesubtype == 100 ) 
								{
									t.timgtouse=g.firemodes[t.gunid][g.firemode].ammoimg;
									if (  t.timgtouse == 0  )  t.timgtouse = g.firemodes[t.gunid][0].ammoimg;
								}
								else
								{
									t.timgtouse=t.timgbase+71+t.slidersmenu[t.slidersmenuindex].customimagesubtype;
								}
								if (  ImageExist(t.timgtouse) == 1 ) 
								{
									PasteImage (  t.timgtouse,t.rmposx+60-(ImageWidth(t.timgtouse)/2),t.tpanely+20,1 );
								}

								PasteImage (  t.timgbase+53,t.rmposx+141,t.tpanely+22,1 );
								t.tammovalue_s=Str(t.slidersmenuvalue[t.slidersmenuindex][1].value);
								t.twidth=getbitmapfontwidth(t.tammovalue_s.Get(),4);
								pastebitmapfont(t.tammovalue_s.Get(),t.rmposx+136-t.twidth,t.rmposy+19,4,255);
								t.tclipvalue_s=Str(t.slidersmenuvalue[t.slidersmenuindex][2].value);
								pastebitmapfont(t.tclipvalue_s.Get(),t.rmposx+147,t.rmposy+22,3,255);
							}
						}
					}
					if (  t.slidersmenu[t.slidersmenuindex].customimagetype == 2 ) 
					{
						//  health panel
						//  flash panel red if health is low, or solid red when dead, or flash yellow when regenerating
						if (  SpriteExist(g.healthpanelsprite)  ==  0  )  Sprite (  g.healthpanelsprite,-1000,-1000,t.timgbase+51 );
						t.tDiffuseR = 0 ; t.tDiffuseG = 0;
						if (  t.player[1].health < 100 ) 
						{
							if (  t.player[1].health  <=  0 ) 
							{
								t.tDiffuseR = 255;
							}
							else
							{
								t.tDiffuseR = (1+Cos(Timer()/2.5))*100;
							}
						}
						else
						{
							if (  t.playercontrol.regentime>0 ) 
							{
								if (  t.player[1].health < t.playercontrol.startstrength ) 
								{
									if ( (DWORD)(Timer()) > t.playercontrol.regentime+(t.playercontrol.regendelay) ) 
									{
										t.tDiffuseR = (1+Cos(Timer()/4.0))*75;
										t.tDiffuseG = t.tDiffuseR;
									}
								}
							}
						}
						SetSpriteDiffuse (  g.healthpanelsprite,t.tDiffuseR,t.tDiffuseG,0 );
						PasteSprite (  g.healthpanelsprite,t.rmposx,t.tpanely );
						PasteImage (  t.timgbase+52,t.rmposx+12,t.tpanely+64,1 );
						PasteImage (  t.timgbase+53,t.rmposx+141,t.tpanely+22,1 );
						PasteImage (  t.timgbase+91,t.rmposx+46-(ImageWidth(t.timgbase+91)/2),t.tpanely+20,1 );
						if (  t.playercontrol.startlives>0 ) 
						{
							//  if lives specified at start, use the HUD graphics for lives
							PasteImage (  t.timgbase+92,t.rmposx+46-(ImageWidth(t.timgbase+92)/2),t.tpanely+75,1 );
							t.tlivesvalue_s=Str(t.player[1].lives);
							t.twidth=getbitmapfontwidth(t.tlivesvalue_s.Get(),5);
							pastebitmapfont(t.tlivesvalue_s.Get(),t.rmposx+135-t.twidth,t.rmposy+63,5,255);
						}
						t.thealthvalue_s=Str(t.player[1].health);
						t.thealthmax_s=Str(t.playercontrol.startstrength);
						t.twidth=getbitmapfontwidth(t.thealthvalue_s.Get(),4);
						pastebitmapfont(t.thealthvalue_s.Get(),t.rmposx+136-t.twidth,t.rmposy+19,4,255);
						pastebitmapfont(t.thealthmax_s.Get(),t.rmposx+147,t.rmposy+22,3,255);
					}
				}
				else
				{
					//  standard slider panel (TAB TAB) with minmax button
					PasteImage (  t.timgbase+3,t.rmposx,t.tpanely,1 );
					if ( t.slidersmenu[t.slidersmenuindex].minmaxbuttonlit!=-1 ) 
					{
						PasteImage ( t.timgbase+32+t.slidersmenu[t.slidersmenuindex].minmaxbuttonlit, t.rmposx+230, t.tpanely+20, 1 );
					}

					// only show contents of panel if not minimised
					if ( t.slidersmenu[t.slidersmenuindex].minimised==1 )
					{
						// cap minimised panel to make it neat
						PasteImage ( t.timgbase+5, t.rmposx, t.tpanely + 32, 1 );
						pastebitmapfont(t.slidersmenu[t.slidersmenuindex].title_s.Get(),t.rmposx+t.slidersmenu[t.slidersmenuindex].leftmargin,t.rmposy+17,1,255);
					}
					else
					{
					t.tpanely += 32;
					t.tpanelheight=t.slidersmenu[t.slidersmenuindex].panelheight;
					for ( t.tty = 0 ; t.tty<=  t.tpanelheight/32; t.tty++ )
					{
						PasteImage (  t.timgbase+4,t.rmposx,t.tpanely,1 );
						t.tpanely+=32;
					}
					PasteImage (  t.timgbase+5,t.rmposx,t.tpanely,1 );

					#ifdef VRTECH
					#else
					// draw character kit buttons here so they go over the backdrop but under the drop down lists
					if (  t.characterkit.inUse  ==  1 ) 
					{
						characterkit_draw_buttons ( );
					}
					#endif

					// draw contents of panel
					if (  t.slidersmenu[t.slidersmenuindex].itemcount == 0 ) 
					{
						//  message panel
						t.ttnumwidth=getbitmapfontwidth(t.slidersmenu[t.slidersmenuindex].title_s.Get(),1)/2;
						pastebitmapfont(t.slidersmenu[t.slidersmenuindex].title_s.Get(),t.rmposx+128-t.ttnumwidth,t.rmposy+22,1,255);
						t.tvaluestring_s=t.slidersmenuvalue[t.slidersmenuindex][1].name_s;
						t.ttnumwidth=getbitmapfontwidth(t.tvaluestring_s.Get(),2)/2;
						pastebitmapfont(t.tvaluestring_s.Get(),t.rmposx+128-t.ttnumwidth,t.rmposy+44+(20*1),2,255);
						t.tvaluestring_s=t.slidersmenuvalue[t.slidersmenuindex][2].name_s;
						t.ttnumwidth=getbitmapfontwidth(t.tvaluestring_s.Get(),2)/2;
						pastebitmapfont(t.tvaluestring_s.Get(),t.rmposx+128-t.ttnumwidth,t.rmposy+44+(20*2),2,255);
					}
					else
					{
						//  alternative slider menu
						t.slidersmenualternativeindex=t.slidersmenuindex;
						if (  t.slidersmenuindex == t.slidersmenunames.performance ) 
						{
							if (  g.sliderspecialview>0 ) 
							{
								//  used for drill-down menus (such as A.I drill down)
								t.slidersmenualternativeindex=g.sliderspecialview;
							}
						}

						//  controller panel
						t.rmposytopy=t.rmposy+t.slidersmenu[t.slidersmenuindex].titlemargin;
						pastebitmapfont(t.slidersmenu[t.slidersmenualternativeindex].title_s.Get(),t.rmposx+t.slidersmenu[t.slidersmenuindex].leftmargin,t.rmposy+22,1,255);
						for ( t.rmi = 0 ; t.rmi<=  t.slidersmenu[t.slidersmenualternativeindex].itemcount-1; t.rmi++ )
						{
							pastebitmapfont(t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].name_s.Get(),t.rmposx+t.slidersmenu[t.slidersmenuindex].leftmargin,t.rmposytopy+(t.rmi*38),2,255);
							if ( t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].useCustomRange == 0 )
							{
								//  (Dave) to handle sliders that have a custom min and max
								t.tdata=t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].value;
							}
							else
							{
								t.tdata = SlidersAdjustValue ( t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].value, t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].valueMin, t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].valueMax, 0, 100 );
							}
							if (  t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].gadgettype == 0 ) 
							{
								//  slider or readout bar
								if (  t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].readmodeindex == 0 ) 
								{
									//  slider control
									PasteImage (  t.timgbase+1,t.rmposx+25,t.rmposytopy+20+(t.rmi*38),1 );
									t.tpos_f=t.tdata ; t.tpos_f=(188.0/100.0)*t.tpos_f;
									PasteImage (  t.timgbase+2,t.rmposx+t.slidersmenu[t.slidersmenuindex].leftmargin+t.tpos_f,t.rmposytopy+14+(t.rmi*38),1 );
								}
								else
								{
									//  read only resource bar (uses slidersmenualternativeindex)
									t.tcolorofbar=t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].readmodeindex;
									PasteImage (  t.timgbase+6,t.rmposx+25,t.rmposytopy+18+(t.rmi*38),1 );
									if (  t.tdata>0 ) 
									{
										t.tcappeddata=t.tdata;
										if (  t.tcappeddata>100  )  t.tcappeddata = 100;
										t.tfillx=0;
										//  (Dave) color of the bar could go over the max amount, so wrapping it round
										while ( t.tcolorofbar > 7 ) { t.tcolorofbar -= 7 ; }
										t.tfillimg=t.timgbase+11+((t.tcolorofbar-1)*3);
										PasteImage (  t.tfillimg+0,t.rmposx+25+1+t.tfillx,t.rmposytopy+20+1+(t.rmi*38),1 );
										t.tfillx+=5;
										t.tfillwidth=((ImageWidth(t.timgbase+6)-20)/100.0)*t.tcappeddata;
										for ( t.ttfill = 0 ; t.ttfill <= t.tfillwidth ; t.ttfill += 10 )
										{
											PasteImage (  t.tfillimg+1,t.rmposx+25+1+t.tfillx,t.rmposytopy+20+1+(t.rmi*38),1 );
											t.tfillx += 10;
										}
										PasteImage (  t.tfillimg+2,t.rmposx+25+1+t.tfillx,t.rmposytopy+20+1+(t.rmi*38),1 );
									}
								}
								//  (Dave) Check if custom range
								if (  t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].useCustomRange  ==  1  )  t.tdata  =  t.slidersmenuvalue[t.slidersmenualternativeindex][1+t.rmi].value;
								//  Use original values for better accuracy
								if (  t.slidersmenualternativeindex == t.slidersmenunames.performance && 1+t.rmi == 12  )  t.tdata = t.mainstatistic1;
								if (  t.slidersmenualternativeindex == t.slidersmenunames.performance && 1+t.rmi == 13  )  t.tdata = t.mainstatistic5;
								//  Ensure correct FOV is reported from slider
								if (  t.slidersmenualternativeindex == t.slidersmenunames.camera && 1+t.rmi == 2  )  t.tdata = 20+(((t.tdata+0.0)/100.0)*90);
								t.ttnumwidth=getbitmapfontwidth(Str(t.tdata),2);
								pastebitmapfont(Str(t.tdata),t.rmposx+256-25-t.ttnumwidth,t.rmposytopy+(t.rmi*38),2,255);
							}
							else
							{
								if (  t.slidersmenuvalue[t.slidersmenuindex][1+t.rmi].gadgettype == 1 ) 
								{
									//  combobox gadget
									t.tvaluestring_s=t.slidersmenuvalue[t.slidersmenuindex][1+t.rmi].value_s;
									PasteImage (  t.timgbase+7,t.rmposx+256-32-128,t.rmposytopy+(t.rmi*38),1 );
									t.ttnumwidth=getbitmapfontwidth(t.tvaluestring_s.Get(),2)/2;
									pastebitmapfont(t.tvaluestring_s.Get(),t.rmposx+256-32-64-t.ttnumwidth,t.rmposytopy+(t.rmi*38)+9,2,255);
								}
							}
						}
					}
				}
			}
		}
		}

		// Tabs
		importer_tabs_draw ( );

		//  draw dropdown panel
		if (  t.slidersdropdown>0 ) 
		{
			t.slidersmenuindex=t.slidersdropdown;
			t.timgbase=g.slidersmenuimageoffset;
			t.rmi=t.slidersmenu[t.slidersmenuindex].thighlight-1;
			//t.col=192<<24;

			//  Ensure drop downs dont go off screen in the importer and create a drop down list
			if ( t.importer.importerActive == 1 )
			{
				t.tcol = t.col;
				t.tlistmax  =  t.slidersdropdownmax ; if (  t.tlistmax > 10  )  t.tlistmax  =  10;
				t.tboxoverflow = (t.slidersdropdowntop+10+(t.tlistmax*16)) - GetChildWindowHeight();
				if (  t.tboxoverflow > 0  )  t.slidersdropdowntop -= t.tboxoverflow;

				if (  t.inputsys.ymouse*t.tadjustedtoimporterybase_f  <=  t.slidersdropdowntop+(t.importer.dropDownListNumber*16) ) 
				{
					if (  Timer() - t.importer.oldTime > 100 ) 
					{
						--t.importer.dropDownListNumber;
						if (  t.importer.dropDownListNumber < 0 ) 
						{
							t.importer.dropDownListNumber = 0;
						}
						else
						{
							t.slidersdropdowntop+=16;
						}
						t.importer.oldTime = Timer();
					}
				}
				if (  t.inputsys.ymouse*t.tadjustedtoimporterybase_f  >=  t.slidersdropdowntop+10+(t.tlistmax*16)+(t.importer.dropDownListNumber*16) ) 
				{
					if (  Timer() - t.importer.oldTime > 100 ) 
					{
						++t.importer.dropDownListNumber;
						if (  t.importer.dropDownListNumber > t.slidersdropdownmax-9 ) 
						{
							t.importer.dropDownListNumber = t.slidersdropdownmax-9;
						}
						else
						{
							t.slidersdropdowntop -= 16;
						}
						t.importer.oldTime = Timer();
					}
				}

				GGBoxGradient (  t.slidersdropdownleft,t.slidersdropdowntop+(t.importer.dropDownListNumber*16),t.slidersdropdownleft+128,t.slidersdropdowntop+(t.importer.dropDownListNumber*16)+10+(t.tlistmax*16),192,0,0,0);//t.col,t.col,t.col,t.col );
				if (  t.slidersdropdownindex >= 0 ) 
				{
					//t.col as DWORD ; 
					//t.col=(192<<24)+Rgb(180,180,192);
					GGBoxGradient (  t.slidersdropdownleft,t.slidersdropdowntop+5+(t.slidersdropdownindex*16),t.slidersdropdownleft+128,t.slidersdropdowntop+5+(t.slidersdropdownindex*16)+16,192,180,180,192);//t.col,t.col,t.col,t.col );
				}
				for ( t.tn = t.importer.dropDownListNumber ; t.tn<=  t.importer.dropDownListNumber+t.tlistmax-1; t.tn++ )
				{
					t.slidersmenuvaluechoice=t.slidersdropdownchoice;
					t.slidersmenuvalueindex=1+t.tn;
					sliders_getnamefromvalue ( );
					pastebitmapfont(t.slidervaluename_s.Get(),t.slidersdropdownleft+10,t.slidersdropdowntop+5+(t.tn*16),2,255);
				}

				if (  t.importer.dropDownListNumber > 0 ) 
				{
					GGBoxGradient (  t.slidersdropdownleft,t.slidersdropdowntop+(t.importer.dropDownListNumber*16)-16,t.slidersdropdownleft+128,t.slidersdropdowntop+(t.importer.dropDownListNumber*16),192,0,0,0);//t.tcol,t.tcol,t.tcol,t.tcol );
					pastebitmapfont("<",t.slidersdropdownleft+10,t.slidersdropdowntop+(t.importer.dropDownListNumber*16)-16,2,255);
				}
				if (  t.importer.dropDownListNumber+10 < t.slidersdropdownmax ) 
				{
					GGBoxGradient (  t.slidersdropdownleft,t.slidersdropdowntop+(t.importer.dropDownListNumber*16)+10+(t.tlistmax*16),t.slidersdropdownleft+128,t.slidersdropdowntop+(t.importer.dropDownListNumber*16)+10+(t.tlistmax*16)+16,192,0,0,0);//t.tcol,t.tcol,t.tcol,t.tcol );
					pastebitmapfont(">",t.slidersdropdownleft+10,t.slidersdropdowntop+(t.importer.dropDownListNumber*16)+10+(t.tlistmax*16),2,255);
				}
			}
			else
			{
				GGBoxGradient (  t.slidersdropdownleft,t.slidersdropdowntop,t.slidersdropdownleft+128,t.slidersdropdowntop+10+(t.slidersdropdownmax*16),192,0,0,0);//t.col,t.col,t.col,t.col );
				if (  t.slidersdropdownindex >= 0 ) 
				{
					//t.col=(192<<24)+Rgb(180,180,192);
					GGBoxGradient (  t.slidersdropdownleft,t.slidersdropdowntop+5+(t.slidersdropdownindex*16),t.slidersdropdownleft+128,t.slidersdropdowntop+5+(t.slidersdropdownindex*16)+16,192,180,180,192);//t.col,t.col,t.col,t.col );
				}
				for ( t.tn = 0 ; t.tn<=  t.slidersdropdownmax-1; t.tn++ )
				{
					t.slidersmenuvaluechoice=t.slidersdropdownchoice;
					t.slidersmenuvalueindex=g.slidersmenudropdownscroll_f+t.tn;
					if (  t.slidersmenuvalueindex <= t.slidersdropdownmax ) 
					{
						sliders_getnamefromvalue ( );
					}
					else
					{
						t.slidervaluename_s="";
					}
					pastebitmapfont(t.slidervaluename_s.Get(),t.slidersdropdownleft+10,t.slidersdropdowntop+5+(t.tn*16),2,255);
				}
			}
		}
	}

	// 080917 - renders all sprites put into batcher (fewer draw calls) 
	DrawSpriteBatcher();
}

void sliders_getchoice ( void )
{
	//  work out number of items in dropdown Box (  )
	t.sliderschoicemax=0;
	if (  t.slidersmenuvaluechoice == 1  )  t.sliderschoicemax = g.skymax;
	if (  t.slidersmenuvaluechoice == 2  )  t.sliderschoicemax = g.terrainstylemax;
	if (  t.slidersmenuvaluechoice == 3  )  t.sliderschoicemax = g.vegstylemax;
	if (  t.slidersmenuvaluechoice == 4  )  t.sliderschoicemax = 4;
	if (  t.slidersmenuvaluechoice == 5  )  t.sliderschoicemax = 3;
	if (  t.slidersmenuvaluechoice == 6  )  t.sliderschoicemax = 4; // grass techniques
	if (  t.slidersmenuvaluechoice == 7  )  t.sliderschoicemax = 2;//3; // 150917 - added PBR - now using pbroverride
	if (  t.slidersmenuvaluechoice == 8  )  t.sliderschoicemax = g.lutmax;
	#ifdef VRTECH
	if (t.slidersmenuvaluechoice == 40)  t.sliderschoicemax = 5;//Weather
	#endif

	//  If we are in the character kit
	if (  t.slidersmenuvaluechoice == 51  )  t.sliderschoicemax = g.characterkitbodymax;
	if (  t.slidersmenuvaluechoice == 52  )  t.sliderschoicemax = g.characterkitheadmax;
	if (  t.slidersmenuvaluechoice == 53  )  t.sliderschoicemax = t.characterkitfacialhairmax;
	if (  t.slidersmenuvaluechoice == 54 ) 
	{
		if (  t.characterkitcontrol.isMale  ==  1  )  t.sliderschoicemax = t.characterkithatmax; else t.sliderschoicemax = t.characterkitfemalehatmax;
	}
	if (  t.slidersmenuvaluechoice == 55  )  t.sliderschoicemax = t.characterkitweaponmax;
	if (  t.slidersmenuvaluechoice == 56  )  t.sliderschoicemax = t.characterkitprofilemax;

	//  (Dave) If we are in the importer then we can run this code
	if (  t.importer.importerActive  ==  1 ) 
	{
		if (  t.slidersmenuvaluechoice == 101  )  t.sliderschoicemax  =  t.importer.shaderFileCount;
		if (  t.slidersmenuvaluechoice >= 102 && t.slidersmenuvaluechoice<107  )  t.sliderschoicemax  =  2;
		if (  t.slidersmenuvaluechoice == 107  )  t.sliderschoicemax  =  5;
		if (  t.slidersmenuvaluechoice == 109  )  t.sliderschoicemax  =  4;
		if (  t.slidersmenuvaluechoice == 110  )  t.sliderschoicemax  =  6;
		if (  t.slidersmenuvaluechoice == 111  )  t.sliderschoicemax  =  3;
		if (  t.slidersmenuvaluechoice == 112  )  t.sliderschoicemax  =  4;
		if (  t.slidersmenuvaluechoice == 113  )  t.sliderschoicemax  =  2;
		if (  t.slidersmenuvaluechoice == 114  )  t.sliderschoicemax  =  t.importer.scriptFileCount;
		if (  t.slidersmenuvaluechoice == 115  )  t.sliderschoicemax  =  2;
		if (  t.slidersmenuvaluechoice == 116  )  t.sliderschoicemax  =  3;
		if (  t.slidersmenuvaluechoice == 117  )  t.sliderschoicemax  =  3;
	}
}

void sliders_getnamefromvalue ( void )
{
	//  takes slider panel index, and dropdown index and produces correct name representitives of values
	//  slidersmenuindex, slidersmenuvaluechoice, slidersmenuvalueindex
	t.slidervaluename_s="--";
	if (  t.slidersmenuvaluechoice == 1 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.skybank_s) ) 
		{
			t.slidervaluename_s=t.skybank_s[t.slidersmenuvalueindex];
		}
	}
	if (  t.slidersmenuvaluechoice == 2 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.terrainstylebank_s) ) 
		{
			t.slidervaluename_s=t.terrainstylebank_s[t.slidersmenuvalueindex];
		}
	}
	if (  t.slidersmenuvaluechoice == 3 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.vegstylebank_s) ) 
		{
			t.slidervaluename_s=t.vegstylebank_s[t.slidersmenuvalueindex];
		}
	}
	if (  t.slidersmenuvaluechoice == 4 ) 
	{
		t.slidervaluename_s="";
		if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "HIGHEST";
		if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "HIGH";
		if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "MEDIUM";
		if (  t.slidersmenuvalueindex == 4  )  t.slidervaluename_s = "LOWEST";
	}
	if (  t.slidersmenuvaluechoice == 5 ) 
	{
		t.slidervaluename_s="";
		if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "HIGHEST";
		if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "MEDIUM";
		if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "LOWEST";
	}
	if (  t.slidersmenuvaluechoice == 6 ) 
	{
		// grass technique
		t.slidervaluename_s="";
		if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "HIGHEST";
		if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "HIGH";
		if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "MEDIUM";
		if (  t.slidersmenuvalueindex == 4  )  t.slidervaluename_s = "LOWEST";
	}
	if (  t.slidersmenuvaluechoice == 7 ) 
	{
		t.slidervaluename_s="";
		if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "PRE-BAKE";
		if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "REALTIME";
		//if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "REALTIME PBR";
	}
	if (t.slidersmenuvaluechoice == 8)
	{
		if (t.slidersmenuvalueindex <= ArrayCount(t.lutbank_s))
		{
			t.slidervaluename_s = t.lutbank_s[t.slidersmenuvalueindex];
			if (strnicmp(t.slidervaluename_s.Get() + strlen(t.slidervaluename_s.Get()) - 4, ".png", 4) == NULL)
				t.slidervaluename_s = Left(t.slidervaluename_s.Get(), strlen(t.slidervaluename_s.Get()) - 4);
		}
	}

	#ifdef VRTECH
	if (t.slidersmenuvaluechoice == 40)
	{
		t.slidervaluename_s = "";
		if (t.slidersmenuvalueindex == 1)  t.slidervaluename_s = "None";
		if (t.slidersmenuvalueindex == 2)  t.slidervaluename_s = "Light Rain";
		if (t.slidersmenuvalueindex == 3)  t.slidervaluename_s = "Heavy Rain";
		if (t.slidersmenuvalueindex == 4)  t.slidervaluename_s = "Light Snow";
		if (t.slidersmenuvalueindex == 5)  t.slidervaluename_s = "Heavy Snow";
	}
	#endif

	//  if we are in the character kit
	if (  t.slidersmenuvaluechoice == 51 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.characterkitbodybank_s) ) 
		{
			t.slidervaluename_s=t.characterkitbodybank_s[t.slidersmenuvalueindex];
		}
	}
	if (  t.slidersmenuvaluechoice == 52 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.characterkitheadbank_s) ) 
		{
			t.slidervaluename_s=t.characterkitheadbank_s[t.slidersmenuvalueindex];
		}
	}
	if (  t.slidersmenuvaluechoice == 53 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.characterkitfacialhairbank_s) ) 
		{
			t.slidervaluename_s=t.characterkitfacialhairbank_s[t.slidersmenuvalueindex];
		}
	}
	if (  t.slidersmenuvaluechoice == 54 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.characterkithatbank_s) ) 
		{
			if (  t.characterkitcontrol.isMale  ==  1 ) 
			{
				t.slidervaluename_s=t.characterkithatbank_s[t.slidersmenuvalueindex];
			}
			else
			{
				t.slidervaluename_s=t.characterkitfemalehatbank_s[t.slidersmenuvalueindex];
			}
		}
	}
	if (  t.slidersmenuvaluechoice == 55 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.characterkitweaponbank_s) ) 
		{
			t.slidervaluename_s=t.characterkitweaponbank_s[t.slidersmenuvalueindex];
		}
	}
	if (  t.slidersmenuvaluechoice == 56 ) 
	{
		if (  t.slidersmenuvalueindex <= ArrayCount(t.characterkitprofilebank_s) ) 
		{
			t.slidervaluename_s=t.characterkitprofilebank_s[t.slidersmenuvalueindex];
		}
	}

	//  (Dave) If we are in the importer then we can run this code
	if (  t.importer.importerActive  ==  1 ) 
	{
		if (  t.slidersmenuvaluechoice == 101 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "entity_basic.fx";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "character_basic.fx";
			if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "post-bloom.fx";
			if (  t.slidersmenuvalueindex == 4  )  t.slidervaluename_s = "post-none.fx";
			if (  t.slidersmenuvalueindex == 5  )  t.slidervaluename_s = "quad_basic.fx";
			if (  t.slidersmenuvalueindex == 6  )  t.slidervaluename_s = "sky_basic.fx";
			if (  t.slidersmenuvalueindex == 7  )  t.slidervaluename_s = "terrain_basic.fx";
			if (  t.slidersmenuvalueindex == 8  )  t.slidervaluename_s = "vegetation_basic.fx";
			if (  t.slidersmenuvalueindex == 9  )  t.slidervaluename_s = "water_basic.fx";
			if (  t.slidersmenuvalueindex == 10  )  t.slidervaluename_s = "weapon_basic.fx";
			if (  t.slidersmenuvalueindex == 11  )  t.slidervaluename_s = "weapon_legacy.fx";
		}
		if (  t.slidersmenuvaluechoice == 102 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "On";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Off";
		}
		if (  t.slidersmenuvaluechoice == 103 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Yes";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "No";
		}
		if (  t.slidersmenuvaluechoice  >= 104 && t.slidersmenuvaluechoice  <=  108 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Yes";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "No";
		}
	}

	//  (Dave) If we are in the importer then we can run this code
	if (  t.importer.importerActive  ==  1 ) 
	{
		if (  t.slidersmenuvaluechoice == 101 ) 
		{
			for ( int tCount = 1 ; tCount<=  t.importer.shaderFileCount; tCount++ )
			{
				if (  t.slidersmenuvalueindex == tCount  )  t.slidervaluename_s = t.importerShaderFiles[tCount];
			}
		}
		if (  t.slidersmenuvaluechoice == 102 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "On";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Off";
		}
		if (  t.slidersmenuvaluechoice == 103 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Yes";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "No";
		}
		if (  t.slidersmenuvaluechoice  >= 104 && t.slidersmenuvaluechoice < 107 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Yes";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "No";
		}
		if (  t.slidersmenuvaluechoice == 107 ) 
		{
			if (  t.slidersmenuvalueindex  ==  1 ) 
			{
				t.slidervaluename_s="No Objective";
			}
			else
			{
				t.slidervaluename_s=cstr("Objective ") + Str(t.slidersmenuvalueindex-1);
			}
		}
		if (  t.slidersmenuvaluechoice == 109 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Front";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Left";
			if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "Right";
			if (  t.slidersmenuvalueindex == 4  )  t.slidervaluename_s = "Top";
		}
		if (  t.slidersmenuvaluechoice == 110 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Box";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Polygon";
			if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "No collision";
			if (  t.slidersmenuvalueindex == 4  )  t.slidervaluename_s = "Limb One Box";
			if (  t.slidersmenuvalueindex == 5  )  t.slidervaluename_s = "Limb One Poly";
			if (  t.slidersmenuvalueindex == 6  )  t.slidervaluename_s = "Collision Boxes";
		}
		if (  t.slidersmenuvaluechoice == 111 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "None";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Standard";
			if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "Render last";
		}
		if (  t.slidersmenuvaluechoice == 112 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Generic";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Stone";
			if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "Metal";
			if (  t.slidersmenuvalueindex == 4  )  t.slidervaluename_s = "Wood";
		}
		if (  t.slidersmenuvaluechoice == 113 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "Yes";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "No";
		}
		if (  t.slidersmenuvaluechoice == 114 ) 
		{
			for ( int tCount = 1 ; tCount<=  t.importer.scriptFileCount; tCount++ )
			{
				if (  t.slidersmenuvalueindex == tCount  )  t.slidervaluename_s = t.importerScriptFiles[tCount];
			}
		}
		if (  t.slidersmenuvaluechoice == 115 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "None";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Swap Y and Z";
		}
		if (  t.slidersmenuvaluechoice == 116 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "None";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Center Floor";
			if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "Center All";
		}
		if ( t.slidersmenuvaluechoice == 117 ) 
		{
			if (  t.slidersmenuvalueindex == 1  )  t.slidervaluename_s = "No";
			if (  t.slidersmenuvalueindex == 2  )  t.slidervaluename_s = "Yes";
			if (  t.slidersmenuvalueindex == 3  )  t.slidervaluename_s = "Use Uber Anims";
		}
	}
}

void sliders_write (bool bOnlyVisualSettings )
{
	//  Control in-game or editing aspect in real-time (takes slidersmenuindex & thighlight)
	if (  t.slidersmenuindex == t.slidersmenunames.performance ) 
	{
		//  Performance Panel
	}
	if (  t.slidersmenuindex == t.slidersmenunames.visuals ) 
	{
		//  Visual Settings
		//  apply constraints if any
		if (  t.slidersmenuvalue[t.slidersmenuindex][1].value>99 ) 
		{
			t.slidersmenuvalue[t.slidersmenuindex][1].value=99;
		}
		if (  t.slidersmenuvalue[t.slidersmenuindex][2].value<t.slidersmenuvalue[t.slidersmenuindex][1].value+1 ) 
		{
			t.slidersmenuvalue[t.slidersmenuindex][2].value=t.slidersmenuvalue[t.slidersmenuindex][1].value+1;
		}
		//  apply setting to engine values
		t.visuals.FogNearest_f=t.slidersmenuvalue[t.slidersmenuindex][1].value*100.0;
		t.visuals.FogDistance_f=t.slidersmenuvalue[t.slidersmenuindex][2].value*500.0;
		t.visuals.FogR_f=t.slidersmenuvalue[t.slidersmenuindex][3].value*2.55;
		t.visuals.FogG_f=t.slidersmenuvalue[t.slidersmenuindex][4].value*2.55;
		t.visuals.FogB_f=t.slidersmenuvalue[t.slidersmenuindex][5].value*2.55;
		t.visuals.FogA_f=t.slidersmenuvalue[t.slidersmenuindex][6].value*2.55;
		t.visuals.AmbienceIntensity_f=t.slidersmenuvalue[t.slidersmenuindex][7].value*2.55;
		t.visuals.AmbienceRed_f=t.slidersmenuvalue[t.slidersmenuindex][8].value*2.55;
		t.visuals.AmbienceGreen_f=t.slidersmenuvalue[t.slidersmenuindex][9].value*2.55;
		t.visuals.AmbienceBlue_f=t.slidersmenuvalue[t.slidersmenuindex][10].value*2.55;
		t.visuals.SurfaceIntensity_f=t.slidersmenuvalue[t.slidersmenuindex][11].value/33.0;
		t.visuals.SurfaceRed_f=t.slidersmenuvalue[t.slidersmenuindex][12].value*2.55;
		t.visuals.SurfaceGreen_f=t.slidersmenuvalue[t.slidersmenuindex][13].value*2.55;
		t.visuals.SurfaceBlue_f=t.slidersmenuvalue[t.slidersmenuindex][14].value*2.55;
		t.visuals.SurfaceSunFactor_f=t.slidersmenuvalue[t.slidersmenuindex][15].value/100.0;
		t.visuals.Specular_f=t.slidersmenuvalue[t.slidersmenuindex][16].value/100.0;
		t.visuals.PostBrightness_f=(t.slidersmenuvalue[t.slidersmenuindex][17].value/100.0)-0.5;
		t.visuals.PostContrast_f=t.slidersmenuvalue[t.slidersmenuindex][18].value/30.0;
		if (!bOnlyVisualSettings)
		{
			t.storeprojectmodified = 1;
			t.visuals.refreshshaders = 1;
		}
	}
	if ( t.slidersmenuindex == t.slidersmenunames.water ) 
	{
		// Water slider
		g.gdefaultwaterheight = t.slidersmenuvalue[t.slidersmenuindex][1].value*10.0;
		t.visuals.WaterRed_f = t.slidersmenuvalue[t.slidersmenuindex][2].value*2.55;
		t.visuals.WaterGreen_f = t.slidersmenuvalue[t.slidersmenuindex][3].value*2.55;
		t.visuals.WaterBlue_f = t.slidersmenuvalue[t.slidersmenuindex][4].value*2.55;
		t.visuals.WaterWaveIntensity_f = t.slidersmenuvalue[t.slidersmenuindex][5].value*2.0;
		t.visuals.WaterTransparancy_f = t.slidersmenuvalue[t.slidersmenuindex][6].value / 100.0;
		t.visuals.WaterReflection_f = t.slidersmenuvalue[t.slidersmenuindex][7].value / 100.0;
		t.visuals.WaterReflectionSparkleIntensity = t.slidersmenuvalue[t.slidersmenuindex][8].value / 5.0;
		t.visuals.WaterFlowDirectionX = (t.slidersmenuvalue[t.slidersmenuindex][9].value - 50) / 10;
		t.visuals.WaterFlowDirectionY = (t.slidersmenuvalue[t.slidersmenuindex][10].value - 50) / 10;
		t.visuals.WaterDistortionWaves = t.slidersmenuvalue[t.slidersmenuindex][11].value / 1000.0;
		t.visuals.WaterSpeed1 = (t.slidersmenuvalue[t.slidersmenuindex][12].value - 100)*-1;
		t.visuals.WaterFlowSpeed = t.slidersmenuvalue[t.slidersmenuindex][13].value / 10.0;
		if (!bOnlyVisualSettings)
		{
			t.storeprojectmodified = 1;
			t.visuals.refreshshaders = 1;
		}
		//set the waterheight (fix for lua water height command to cover stuff in map editor)
		t.terrain.waterliney_f = g.gdefaultwaterheight;
	}
	if (  t.slidersmenuindex == t.slidersmenunames.camera ) 
	{
		//  Camera settings
		if (  t.slidersmenuvalue[t.slidersmenuindex][1].value<1 ) 
		{
			t.slidersmenuvalue[t.slidersmenuindex][1].value=1;
		}
		t.visuals.CameraFAR_f=t.slidersmenuvalue[t.slidersmenuindex][1].value*3000.0;
		t.visuals.CameraFOV_f=(20+((t.slidersmenuvalue[t.slidersmenuindex][2].value+0.0)/100.0)*90.0)/t.visuals.CameraASPECT_f;
		t.visuals.CameraFOVZoomed_f=t.slidersmenuvalue[t.slidersmenuindex][3].value/100.0;
		t.visuals.WeaponFOV_f=(20+((t.slidersmenuvalue[t.slidersmenuindex][4].value+0.0)/100.0)*90.0)/t.visuals.CameraASPECT_f;
		if (!bOnlyVisualSettings)
		{
			t.storeprojectmodified = 1;
			t.visuals.refreshshaders = 1;
		}
	}
	if (  t.slidersmenuindex == t.slidersmenunames.posteffects ) 
	{
		//  Post Effects
		t.visuals.bloommode=t.slidersmenuvalue[t.slidersmenuindex][1].value;
		t.visuals.VignetteRadius_f=t.slidersmenuvalue[t.slidersmenuindex][2].value/100.0;
		t.visuals.VignetteIntensity_f=t.slidersmenuvalue[t.slidersmenuindex][3].value/100.0;
		t.visuals.MotionDistance_f=t.slidersmenuvalue[t.slidersmenuindex][4].value/100.0;
		t.visuals.MotionIntensity_f=t.slidersmenuvalue[t.slidersmenuindex][5].value/100.0;
		t.visuals.DepthOfFieldDistance_f=t.slidersmenuvalue[t.slidersmenuindex][6].value/100.0;
		t.visuals.DepthOfFieldIntensity_f=t.slidersmenuvalue[t.slidersmenuindex][7].value/100.0;
		t.visuals.LightrayLength_f=t.slidersmenuvalue[t.slidersmenuindex][8].value/100.0;
		t.visuals.LightrayQuality_f=t.slidersmenuvalue[t.slidersmenuindex][9].value;
		t.visuals.LightrayDecay_f=t.slidersmenuvalue[t.slidersmenuindex][10].value/100.0;
		t.visuals.SAORadius_f=t.slidersmenuvalue[t.slidersmenuindex][11].value/100.0;
		t.visuals.SAOIntensity_f=t.slidersmenuvalue[t.slidersmenuindex][12].value/100.0;
		t.visuals.LensFlare_f=t.slidersmenuvalue[t.slidersmenuindex][13].value/100.0;
		t.visuals.Saturation_f = t.slidersmenuvalue[t.slidersmenuindex][14].value / 100.0;
		t.visuals.Sepia_f = t.slidersmenuvalue[t.slidersmenuindex][15].value / 100.0;
		if (!bOnlyVisualSettings)
		{
			t.storeprojectmodified = 1;
			t.visuals.refreshshaders = 1;
		}
	}
//  `if slidersmenuindex=slidersmenunames.sky
	////  Sky settings
	//terrain.sundirectionx#=0+((slidersmenuvalue(slidersmenuindex,1).value-50)*1000)
	//terrain.sundirectiony#=0+((slidersmenuvalue(slidersmenuindex,2).value-0)*1000)
	//terrain.sundirectionz#=0+((slidersmenuvalue(slidersmenuindex,3).value-50)*1000)
	//visuals.refreshshaders=1
//  `endif
	if (  t.slidersmenuindex == t.slidersmenunames.qualitypanel ) 
	{
		//  Quality panel
		if (  t.slidersmenuvalue[t.slidersmenuindex][1].value>98 ) 
		{
			t.slidersmenuvalue[t.slidersmenuindex][1].value=98;
		}
		if (  t.slidersmenuvalue[t.slidersmenuindex][2].value <= t.slidersmenuvalue[t.slidersmenuindex][1].value ) 
		{
			t.slidersmenuvalue[t.slidersmenuindex][2].value=t.slidersmenuvalue[t.slidersmenuindex][1].value+1;
		}
		if (  t.slidersmenuvalue[t.slidersmenuindex][3].value <= t.slidersmenuvalue[t.slidersmenuindex][2].value ) 
		{
			t.slidersmenuvalue[t.slidersmenuindex][3].value=t.slidersmenuvalue[t.slidersmenuindex][2].value+1;
		}
		t.visuals.TerrainLOD1_f=t.slidersmenuvalue[t.slidersmenuindex][1].value*100.0;
		t.visuals.TerrainLOD2_f=t.slidersmenuvalue[t.slidersmenuindex][2].value*100.0;
		t.visuals.TerrainLOD3_f=t.slidersmenuvalue[t.slidersmenuindex][3].value*100.0;
		t.visuals.TerrainSize_f=t.slidersmenuvalue[t.slidersmenuindex][4].value;
		if (!bOnlyVisualSettings && t.visuals.VegQuantity_f != t.slidersmenuvalue[t.slidersmenuindex][5].value  )  t.visuals.refreshvegetation = 1;
		if (!bOnlyVisualSettings && t.visuals.VegWidth_f != t.slidersmenuvalue[t.slidersmenuindex][6].value  )  t.visuals.refreshvegetation = 1;
		if (!bOnlyVisualSettings && t.visuals.VegHeight_f != t.slidersmenuvalue[t.slidersmenuindex][7].value  )  t.visuals.refreshvegetation = 1;
		t.visuals.VegQuantity_f=t.slidersmenuvalue[t.slidersmenuindex][5].value;
		t.visuals.VegWidth_f=t.slidersmenuvalue[t.slidersmenuindex][6].value;
		t.visuals.VegHeight_f=t.slidersmenuvalue[t.slidersmenuindex][7].value;
		if (!bOnlyVisualSettings)
		{
			t.storeprojectmodified = 1;
			t.visuals.refreshshaders = 1;
		}
	}
	if (  t.slidersmenuindex == t.slidersmenunames.worldpanel ) 
	{
		if (  t.slidersmenuvalue[t.slidersmenuindex][1].value<1  )  t.slidersmenuvalue[t.slidersmenuindex][1].value = 1;
		if (  t.slidersmenuvalue[t.slidersmenuindex][2].value<1  )  t.slidersmenuvalue[t.slidersmenuindex][2].value = 1;
		if (  t.slidersmenuvalue[t.slidersmenuindex][3].value<1  )  t.slidersmenuvalue[t.slidersmenuindex][3].value = 1;
		if (  t.visuals.skyindex != t.slidersmenuvalue[t.slidersmenuindex][1].value ) 
		{
			t.visuals.skyindex = t.slidersmenuvalue[t.slidersmenuindex][1].value;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				t.visuals.refreshskysettings = 1;
				t.visuals.refreshshaders = 1;
			}
		}
		if (  t.visuals.terrainindex != t.slidersmenuvalue[t.slidersmenuindex][2].value ) 
		{
			t.visuals.terrainindex=t.slidersmenuvalue[t.slidersmenuindex][2].value;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				t.visuals.refreshterraintexture = 1;
				t.visuals.refreshshaders = 1;
			}
		}
		if (  t.visuals.vegetationindex != t.slidersmenuvalue[t.slidersmenuindex][3].value ) 
		{
			t.visuals.vegetationindex=t.slidersmenuvalue[t.slidersmenuindex][3].value;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				t.visuals.refreshvegtexture = 1;
			}
		}
		if (t.visuals.lutindex != t.slidersmenuvalue[t.slidersmenuindex][4].value)
		{
			t.storeprojectmodified = 1;
			t.visuals.lutindex = t.slidersmenuvalue[t.slidersmenuindex][4].value;
			t.visuals.refreshlutsettings = 1;
			t.visuals.refreshshaders = 1;
		}
		#ifdef VRTECH
		if (t.visuals.iEnvironmentWeather+1 != t.slidersmenuvalue[t.slidersmenuindex][4].value)
		{
			t.visuals.iEnvironmentWeather = t.slidersmenuvalue[t.slidersmenuindex][4].value-1;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
			}
		}
		#endif
	}
	if (  t.slidersmenuindex == t.slidersmenunames.graphicoptions ) 
	{
		//  Graphic Options
		t.visuals.reflectionmode=t.slidersmenuvalue[t.slidersmenuindex][1].value;
		t.visuals.shadowmode=t.slidersmenuvalue[t.slidersmenuindex][2].value;
		t.visuals.lightraymode=t.slidersmenuvalue[t.slidersmenuindex][3].value;
		if (  t.visuals.vegetationmode != t.slidersmenuvalue[t.slidersmenuindex][4].value ) 
		{
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				t.visuals.refreshvegetation = 1;
			}
		}
		t.visuals.vegetationmode = t.slidersmenuvalue[t.slidersmenuindex][4].value;
		if (  t.visuals.occlusionvalue != t.slidersmenuvalue[t.slidersmenuindex][5].value ) 
		{
			t.visuals.occlusionvalue = t.slidersmenuvalue[t.slidersmenuindex][5].value;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				CPU3DSetPolyCount(t.visuals.occlusionvalue);
			}
		}
		t.visuals.debugvisualsmode=t.slidersmenuvalue[t.slidersmenuindex][6].value;
		if (!bOnlyVisualSettings)
		{
			t.visuals.refreshshaders = 1;
		}
	}
	if (  t.slidersmenuindex == t.slidersmenunames.shaderoptions ) 
	{
		if (  t.visuals.shaderlevels.terrain != t.slidersmenuvalue[t.slidersmenuindex][1].value ) 
		{
			t.visuals.shaderlevels.terrain=t.slidersmenuvalue[t.slidersmenuindex][1].value;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				visuals_shaderlevels_terrain_update();
				t.visuals.refreshshaders = 1;
			}
		}
		if (  t.visuals.shaderlevels.entities != t.slidersmenuvalue[t.slidersmenuindex][2].value ) 
		{
			t.visuals.shaderlevels.entities=t.slidersmenuvalue[t.slidersmenuindex][2].value;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				visuals_shaderlevels_entities_update();
				t.visuals.refreshshaders = 1;
			}
		}
		if (  t.visuals.shaderlevels.vegetation != t.slidersmenuvalue[t.slidersmenuindex][3].value ) 
		{
			t.visuals.shaderlevels.vegetation=t.slidersmenuvalue[t.slidersmenuindex][3].value;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				visuals_shaderlevels_vegetation_update();
				t.visuals.refreshshaders = 1;
			}
		}
		if (  t.visuals.shaderlevels.lighting != t.slidersmenuvalue[t.slidersmenuindex][4].value ) 
		{
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				t.visuals.shaderlevels.lighting = t.slidersmenuvalue[t.slidersmenuindex][4].value;
				visuals_shaderlevels_lighting_update();
				//  the above subroutine can change lighting back to REALTIME
				t.slidersmenuvalue[t.slidersmenuindex][4].value = t.visuals.shaderlevels.lighting;
				if (t.slidersmenuvalue[t.slidersmenuindex][4].value >= 2)
				{
					t.slidersmenuvaluechoice = t.slidersmenuvalue[t.slidersmenuindex][4].gadgettypevalue;
					t.slidersmenuvalueindex = t.slidersmenuvalue[t.slidersmenuindex][4].value;
					sliders_getnamefromvalue();
					t.slidersmenuvalue[t.slidersmenuindex][4].value_s = t.slidervaluename_s;
				}
				visuals_shaderlevels_terrain_update();
				visuals_shaderlevels_entities_update();
				visuals_shaderlevels_vegetation_update();
				t.visuals.refreshshaders = 1;
			}
		}
		if (  (t.visuals.DistanceTransitionStart_f != t.slidersmenuvalue[t.slidersmenuindex][5].value*100.0) || (t.visuals.DistanceTransitionRange_f != t.slidersmenuvalue[t.slidersmenuindex][6].value*10.0) ) 
		{
			t.visuals.DistanceTransitionStart_f=t.slidersmenuvalue[t.slidersmenuindex][5].value*100.0;
			t.visuals.DistanceTransitionRange_f=t.slidersmenuvalue[t.slidersmenuindex][6].value*10.0;
			if (!bOnlyVisualSettings)
			{
				t.storeprojectmodified = 1;
				t.visuals.refreshshaders = 1;
			}
		}
	}
	if (  t.slidersmenuindex == t.importer.properties1Index ) 
	{
		if ( t.whichmenuitem==2 )
		{
			// changed shader while in model importer
			importer_changeshader ( t.slidersmenuvalue[t.slidersmenuindex][2].value_s.Get() );
		}
		//if ( t.whichmenuitem==13 || t.whichmenuitem==14 )
		//{
		//	// selected options to force the imported model to reload
		//	g_iFBXGeometryToggleMode = t.slidersmenuvalue[t.slidersmenuindex][13].value-1;
		//	g_iFBXGeometryCenterMesh = t.slidersmenuvalue[t.slidersmenuindex][14].value-1;
		//	g_iTriggerReloadOfImportModel = 1;
		//}
	}
}

void sliders_scope_draw ( void )
{
	//  draw scope (before slider panels)
	if (  t.gunzoommode != 0 ) 
	{
		if (  g.firemodes[t.gunid][0].zoomscope>0 ) 
		{
			t.timgbase=g.firemodes[t.gunid][0].zoomscope;
			Sprite (  t.timgbase,-10000,-10000,t.timgbase );
			t.timgwidth_f=ImageWidth(t.timgbase) ; t.timgheight_f=ImageHeight(t.timgbase);
			t.timgratio_f=t.timgwidth_f/t.timgheight_f;
			t.tsprwidth_f=GetDisplayHeight()*t.timgratio_f;
			SizeSprite ( t.timgbase, t.tsprwidth_f, GetDisplayHeight()+1 );
			PasteSprite ( t.timgbase, (t.tsprwidth_f-GetDisplayWidth())/-2, 0 );
		}
	}
}

float SlidersAdjustValue ( float value_f, float minFrom_f, float maxFrom_f, float minTo_f, float maxTo_f )
{
	float mappedValue_f = 0;
	//  (Dave) Added to allow custom values
	mappedValue_f = minTo_f + ( maxTo_f - minTo_f ) * ( ( value_f - minFrom_f ) / ( maxFrom_f - minFrom_f ) );
	return mappedValue_f;
}

//prevent that slider bar is in other panels when lua commands are used to set the values
float SlidersCutExtendedValues(float value) {
	if (value > 100) return 100;
	else if (value < 0) return 0;
	else return value;
}