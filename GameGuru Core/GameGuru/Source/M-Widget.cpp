//----------------------------------------------------
//--- GAMEGURU - M-Widget
//----------------------------------------------------

#include "gameguru.h"

//PE: GameGuru IMGUI.
#include "..\GameGuru\Imgui\imgui.h"
#include "..\GameGuru\Imgui\imgui_impl_win32.h"
#include "..\GameGuru\Imgui\imgui_gg_dx11.h"

// Prototypes
void gridedit_clearentityrubberbandlist ( void );

void widget_init ( void )
{
	//  set widget up
	t.widget.pickedObject = 0;
	t.widget.activeObject = 0;
	t.widget.widgetXObj = g.widgetobjectoffset;
	t.widget.widgetYObj = g.widgetobjectoffset+1;
	t.widget.widgetZObj = g.widgetobjectoffset+2;
	t.widget.widgetXYObj = g.widgetobjectoffset+3;
	t.widget.widgetXZObj = g.widgetobjectoffset+4;
	t.widget.widgetYZObj = g.widgetobjectoffset+5;

	t.widget.widgetXRotObj = g.widgetobjectoffset+6;
	t.widget.widgetYRotObj = g.widgetobjectoffset+7;
	t.widget.widgetZRotObj = g.widgetobjectoffset+8;

	t.widget.widgetXScaleObj = g.widgetobjectoffset+9;
	t.widget.widgetYScaleObj = g.widgetobjectoffset+10;
	t.widget.widgetZScaleObj = g.widgetobjectoffset+11;
	t.widget.widgetXYZScaleObj = g.widgetobjectoffset+12;

	t.widget.widgetXColObj = g.widgetobjectoffset+16;
	t.widget.widgetYColObj = g.widgetobjectoffset+17;
	t.widget.widgetZColObj = g.widgetobjectoffset+18;

	t.widget.widgetPOSObj = g.widgetobjectoffset+19;
	t.widget.widgetROTObj = g.widgetobjectoffset+20;
	t.widget.widgetSCLObj = g.widgetobjectoffset+21;
	t.widget.widgetPRPObj = g.widgetobjectoffset+22;
	t.widget.widgetDUPObj = g.widgetobjectoffset+23;
	t.widget.widgetDELObj = g.widgetobjectoffset+24;
	t.widget.widgetLCKObj = g.widgetobjectoffset+25;

	t.widget.widgetMAXObj = 25;

	t.widget.widgetPlaneObj = g.widgetobjectoffset+26;

	//  Load in media
	LoadObject (  "editors\\gfx\\widget\\widgetX.x",t.widget.widgetXObj );
	LoadObject (  "editors\\gfx\\widget\\widgetY.x",t.widget.widgetYObj );
	LoadObject (  "editors\\gfx\\widget\\widgetZ.x",t.widget.widgetZObj );
	LoadObject (  "editors\\gfx\\widget\\widgetXY.x",t.widget.widgetXYObj );
	LoadObject (  "editors\\gfx\\widget\\widgetXZ.x",t.widget.widgetXZObj );
	LoadObject (  "editors\\gfx\\widget\\widgetYZ.x",t.widget.widgetYZObj );
	LoadObject (  "editors\\gfx\\widget\\widgetXRot.x",t.widget.widgetXRotObj );
	LoadObject (  "editors\\gfx\\widget\\widgetYRot.x",t.widget.widgetYRotObj );
	LoadObject (  "editors\\gfx\\widget\\widgetZRot.x",t.widget.widgetZRotObj );
	LoadObject (  "editors\\gfx\\widget\\widgetXScale.x",t.widget.widgetXScaleObj );
	LoadObject (  "editors\\gfx\\widget\\widgetYScale.x",t.widget.widgetYScaleObj );
	LoadObject (  "editors\\gfx\\widget\\widgetZScale.x",t.widget.widgetZScaleObj );
	MakeObjectCube (  t.widget.widgetXYZScaleObj,2 );
	LoadObject (  "editors\\gfx\\widget\\widgetXCol.x",t.widget.widgetXColObj );
	LoadObject (  "editors\\gfx\\widget\\widgetYCol.x",t.widget.widgetYColObj );
	LoadObject (  "editors\\gfx\\widget\\widgetZCol.x",t.widget.widgetZColObj );
	TextureObject ( t.widget.widgetXObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetYObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetZObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetXYObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetXZObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetYZObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetXRotObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetYRotObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetZRotObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetXScaleObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetYScaleObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetZScaleObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetXColObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetYColObj, g.editorimagesoffset+14 );
	TextureObject ( t.widget.widgetZColObj, g.editorimagesoffset+14 );

	//  Widget Buttons
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\pos.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+1 );
	t.strwork = ""; t.strwork = t.strwork +"languagebank\\"+g.language_s+"\\artwork\\widget\\rot.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+2 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\scl.png";
	LoadImage (t .strwork.Get(),t.widget.imagestart+3 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\prp.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+4 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\dup.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+5 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\del.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+6 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\lck.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+7 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\edt.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+8 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\widget\\sav.png";
	LoadImage ( t.strwork.Get(),t.widget.imagestart+9 );
	t.tbuttscale_f=0.015f;
	MakeObjectBox (  t.widget.widgetPOSObj,4,t.tbuttscale_f*50,t.tbuttscale_f  ); TextureObject (  t.widget.widgetPOSObj,t.widget.imagestart+1 );
	MakeObjectBox (  t.widget.widgetROTObj,4,t.tbuttscale_f*50,t.tbuttscale_f  ); TextureObject (  t.widget.widgetROTObj,t.widget.imagestart+2 );
	MakeObjectBox (  t.widget.widgetSCLObj,4,t.tbuttscale_f*50,t.tbuttscale_f  ); TextureObject (  t.widget.widgetSCLObj,t.widget.imagestart+3 );
	MakeObjectBox (  t.widget.widgetPRPObj,4,t.tbuttscale_f*50,t.tbuttscale_f  ); TextureObject (  t.widget.widgetPRPObj,t.widget.imagestart+4 );
	MakeObjectBox (  t.widget.widgetDUPObj,4,t.tbuttscale_f*50,t.tbuttscale_f  ); TextureObject (  t.widget.widgetDUPObj,t.widget.imagestart+5 );
	MakeObjectBox (  t.widget.widgetDELObj,4,t.tbuttscale_f*50,t.tbuttscale_f  ); TextureObject (  t.widget.widgetDELObj,t.widget.imagestart+6 );
	MakeObjectBox (  t.widget.widgetLCKObj,4,t.tbuttscale_f*50,t.tbuttscale_f  ); TextureObject (  t.widget.widgetLCKObj,t.widget.imagestart+7 );
	t.tmovezup_f=0.0f ; t.tmovestep_f=0.8f;
	OffsetLimb (  t.widget.widgetPOSObj,0,-1.2f,3.0f+(t.tmovestep_f*5),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPOSObj,0,0,0,0 );
	OffsetLimb (  t.widget.widgetROTObj,0,-1.2f,3.0f+(t.tmovestep_f*4),t.tmovezup_f  ); RotateLimb (  t.widget.widgetROTObj,0,0,0,0 );
	OffsetLimb (  t.widget.widgetSCLObj,0,-1.2f,3.0f+(t.tmovestep_f*3),t.tmovezup_f  ); RotateLimb (  t.widget.widgetSCLObj,0,0,0,0 );
	OffsetLimb (  t.widget.widgetPRPObj,0,-1.2f,3.0f+(t.tmovestep_f*2),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPRPObj,0,0,0,0 );
	OffsetLimb (  t.widget.widgetDUPObj,0,-1.2f,3.0f+t.tmovestep_f,t.tmovezup_f  ); RotateLimb (  t.widget.widgetDUPObj,0,0,0,0 );
	OffsetLimb (  t.widget.widgetDELObj,0,-1.2f,2.0f+t.tmovestep_f,t.tmovezup_f  ); RotateLimb (  t.widget.widgetDELObj,0,0,0,0 );
	OffsetLimb (  t.widget.widgetLCKObj,0,-1.2f,2.0f,t.tmovezup_f  ); RotateLimb (  t.widget.widgetLCKObj,0,0,0,0 );

	//  Set up display (lots of stuff is hidden!)
	HideObject (  t.widget.widgetXColObj );
	HideObject (  t.widget.widgetYColObj );
	HideObject (  t.widget.widgetZColObj );

	DisableObjectZDepth (  t.widget.widgetXObj );
	DisableObjectZDepth (  t.widget.widgetYObj );
	DisableObjectZDepth (  t.widget.widgetZObj );
	DisableObjectZDepth (  t.widget.widgetXYObj );
	DisableObjectZDepth (  t.widget.widgetXZObj );
	DisableObjectZDepth (  t.widget.widgetYZObj );
	DisableObjectZDepth (  t.widget.widgetXRotObj );
	DisableObjectZDepth (  t.widget.widgetYRotObj );
	DisableObjectZDepth (  t.widget.widgetZRotObj );
	DisableObjectZDepth (  t.widget.widgetXScaleObj );
	DisableObjectZDepth (  t.widget.widgetYScaleObj );
	DisableObjectZDepth (  t.widget.widgetZScaleObj );
	DisableObjectZDepth (  t.widget.widgetXYZScaleObj );
	DisableObjectZDepth (  t.widget.widgetPOSObj );
	DisableObjectZDepth (  t.widget.widgetROTObj );
	DisableObjectZDepth (  t.widget.widgetSCLObj );
	DisableObjectZDepth (  t.widget.widgetPRPObj );
	DisableObjectZDepth (  t.widget.widgetDUPObj );
	DisableObjectZDepth (  t.widget.widgetDELObj );
	DisableObjectZDepth (  t.widget.widgetLCKObj );

	DisableObjectZRead (  t.widget.widgetXObj );
	DisableObjectZRead (  t.widget.widgetYObj );
	DisableObjectZRead (  t.widget.widgetZObj );
	DisableObjectZRead (  t.widget.widgetXYObj );
	DisableObjectZRead (  t.widget.widgetXZObj );
	DisableObjectZRead (  t.widget.widgetYZObj );
	DisableObjectZRead (  t.widget.widgetXRotObj );
	DisableObjectZRead (  t.widget.widgetYRotObj );
	DisableObjectZRead (  t.widget.widgetZRotObj );
	DisableObjectZRead (  t.widget.widgetXScaleObj );
	DisableObjectZRead (  t.widget.widgetYScaleObj );
	DisableObjectZRead (  t.widget.widgetZScaleObj );
	DisableObjectZRead (  t.widget.widgetXYZScaleObj );
	DisableObjectZRead (  t.widget.widgetPOSObj );
	DisableObjectZRead (  t.widget.widgetROTObj );
	DisableObjectZRead (  t.widget.widgetSCLObj );
	DisableObjectZRead (  t.widget.widgetPRPObj );
	DisableObjectZRead (  t.widget.widgetDUPObj );
	DisableObjectZRead (  t.widget.widgetDELObj );
	DisableObjectZRead (  t.widget.widgetLCKObj );

	SetObjectEmissive (  t.widget.widgetXObj, Rgb(255,0,0) );
	SetObjectEmissive (  t.widget.widgetYObj, Rgb(0,255,0) );
	SetObjectEmissive (  t.widget.widgetZObj, Rgb(0,0,255) );
	SetObjectEmissive (  t.widget.widgetXYObj, Rgb(255,0,0) );
	SetObjectEmissive (  t.widget.widgetXZObj, Rgb(0,255,0) );
	SetObjectEmissive (  t.widget.widgetYZObj, Rgb(0,0,255) );
	SetObjectEmissive (  t.widget.widgetXRotObj, Rgb(255,0,0) );
	SetObjectEmissive (  t.widget.widgetYRotObj, Rgb(0,255,0) );
	SetObjectEmissive (  t.widget.widgetZRotObj, Rgb(0,0,255) );
	SetObjectEmissive (  t.widget.widgetXScaleObj, Rgb(255,0,0) );
	SetObjectEmissive (  t.widget.widgetYScaleObj, Rgb(0,255,0) );
	SetObjectEmissive (  t.widget.widgetZScaleObj, Rgb(0,0,255) );
	SetObjectEmissive (  t.widget.widgetXYZScaleObj, Rgb(255,255,255) );

	SetObjectTransparency (  t.widget.widgetXYObj,6 );
	SetObjectTransparency (  t.widget.widgetXZObj,6 );
	SetObjectTransparency (  t.widget.widgetYZObj,6 );
	SetObjectTransparency (  t.widget.widgetPOSObj,6 );
	SetObjectTransparency (  t.widget.widgetROTObj,6 );
	SetObjectTransparency (  t.widget.widgetSCLObj,6 );
	SetObjectTransparency (  t.widget.widgetPRPObj,6 );

	// 110517 - no more fixed function
	///SetObjectEffect ( t.widget.widgetPOSObj, g.guishadereffectindex );
	///SetObjectEffect ( t.widget.widgetROTObj, g.guishadereffectindex );
	///SetObjectEffect ( t.widget.widgetSCLObj, g.guishadereffectindex );
	///SetObjectEffect ( t.widget.widgetPRPObj, g.guishadereffectindex );
	///SetObjectEffect ( t.widget.widgetDUPObj, g.guishadereffectindex );
	///SetObjectEffect ( t.widget.widgetDELObj, g.guishadereffectindex );
	///SetObjectEffect ( t.widget.widgetLCKObj, g.guishadereffectindex );
	for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
	{
		if ( ObjectExist(g.widgetobjectoffset+t.a) )  
		{
			SetObjectEffect ( g.widgetobjectoffset+t.a, g.guishadereffectindex );
		}
	}

	for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
	{
		if (  ObjectExist(g.widgetobjectoffset+t.a)  )  HideObject (  g.widgetobjectoffset+t.a );
	}

	MakeObjectBox (  t.widget.widgetPlaneObj,100000,100000,1 );
	RotateObject (  t.widget.widgetPlaneObj,-90,0,0 );
	HideObject (  t.widget.widgetPlaneObj );

	//  Offset to Lees rotation objects
	t.twidgetRotStartObject = g.widgetobjectoffset+30;

	//  create widget helper objects
	MakeObjectSphere (  t.twidgetRotStartObject+11,60  ); HideObject (  t.twidgetRotStartObject+11 );
	MakeObjectSphere (  t.twidgetRotStartObject+12,70  ); HideObject (  t.twidgetRotStartObject+12 );
	MakeObjectSphere (  t.twidgetRotStartObject+13,65  ); HideObject (  t.twidgetRotStartObject+13 );
	MakeObjectPlane (  t.twidgetRotStartObject+14,10000,10000  ); HideObject (  t.twidgetRotStartObject+14 );

	//  widget vector and matrices
	t.r=MakeVector3(g.widgetvectorindex+1);
	t.r=MakeMatrix(g.widgetStartMatrix+2);
	t.r=MakeMatrix(g.widgetStartMatrix+3);
	t.r=MakeMatrix(g.widgetStartMatrix+4);
	t.r=MakeMatrix(g.widgetStartMatrix+5);
	t.r=MakeVector3(g.widgetvectorindex+6);
	t.r=MakeVector3(g.widgetvectorindex+7);
	t.r=MakeVector3(g.widgetvectorindex+8);
	t.r=MakeVector3(g.widgetvectorindex+9);
	t.r=MakeVector3(g.widgetvectorindex+10);
	t.r=MakeVector3(g.widgetvectorindex+11);
	t.r=MakeVector3(g.widgetvectorindex+12);

	t.widget.pickedSection = 0;
}

void widget_movezonesandlights ( int e )
{
	t.waypointindex = t.entityelement[e].eleprof.trigger.waypointzoneindex;
	t.thisx_f = t.entityelement[e].x;
	t.thisy_f = t.entityelement[e].y;
	t.thisz_f = t.entityelement[e].z;
	waypoint_movetothiscoordinate ( );

	// also update lights if part of group
	if ( t.entityprofile[t.entityelement[e].bankindex].ismarker == 2 ) 
	{
		int iLightIndex = t.entityelement[e].eleprof.light.index;
		if ( iLightIndex >=0 )
		{
			t.infinilight[iLightIndex].x = t.entityelement[e].x;
			t.infinilight[iLightIndex].y = t.entityelement[e].y;
			t.infinilight[iLightIndex].z = t.entityelement[e].z;
		}
	}
}

void widget_loop ( void )
{
	//  fixed camera projection
	SetCurrentCamera (  0 );
	SetCameraRange (  1,70000 );

	//  real widget mouse X Y
	if (  t.widget.protoineffect == 1 ) 
	{
		t.widgetinputsysxmouse_f=t.inputsys.xmouse;
		t.widgetinputsysymouse_f=t.inputsys.ymouse;
	}
	else
	{
		// only update if mouse within 3D view
		if ( t.inputsys.xmouse != 500000 )
		{
#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
			//PE: imgui Need testing.
			t.widgetinputsysxmouse_f = ((float)t.inputsys.xmouse / (float)GetDisplayWidth()) / ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
			t.widgetinputsysymouse_f = ((float)t.inputsys.ymouse / (float)GetDisplayHeight()) / ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
#else
			//  work out visible part of full backbuffer (i.e. 1212 of 1360)
			t.widgetinputsysxmouse_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
			t.widgetinputsysymouse_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
			//  scale full mouse to fit in visible area
			t.widgetinputsysxmouse_f=((t.inputsys.xmouse+0.0)/800.0)/t.widgetinputsysxmouse_f;
			t.widgetinputsysymouse_f=((t.inputsys.ymouse+0.0)/600.0)/t.widgetinputsysymouse_f;
#endif

			//  then provide in a format for the pick-from-screen command
			#ifdef DX11
			t.widgetinputsysxmouse_f=t.widgetinputsysxmouse_f*(GetDisplayWidth()+0.0);
			t.widgetinputsysymouse_f=t.widgetinputsysymouse_f*(GetDisplayHeight()+0.0);
			#else
			t.widgetinputsysxmouse_f=t.widgetinputsysxmouse_f*(GetChildWindowWidth()+0.0);
			t.widgetinputsysymouse_f=t.widgetinputsysymouse_f*(GetChildWindowHeight()+0.0);
			#endif
		}
	}

	//  highlighter
	if (  t.widget.activeObject>0 ) 
	{
		if (  t.widget.pickedSection != 0  )  t.tdim_f = 25; else t.tdim_f = 50;
		if (  t.widget.pickedSection >= t.widget.widgetPOSObj && t.widget.pickedSection <= t.widget.widgetLCKObj  )  t.tdim_f = 50;
		for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
		{
			if (  ObjectExist(g.widgetobjectoffset+t.a)  )  SetAlphaMappingOn (  g.widgetobjectoffset+t.a,t.tdim_f );
		}
		if (  t.widget.pickedSection != 0 ) 
		{
			t.thighlighterobj = t.widget.pickedSection;
		}
		else
		{
			t.thighlighterobj = PickScreenObject(t.widgetinputsysxmouse_f, t.widgetinputsysymouse_f, g.widgetobjectoffset+0, g.widgetobjectoffset+t.widget.widgetMAXObj);
		}
		if (  t.thighlighterobj>0  )  SetAlphaMappingOn (  t.thighlighterobj,75.0 );
	}
	else
	{
		//  ensure no widget button detection if no active object
		t.thighlighterobj=0;
		t.widget.pickedSection=0;
	}

	//  If no picked object and the widget has been in use, switch it off
	widget_updatewidgetobject ( );

	//  Ensure any old widget object ptr is updated
	if (  t.widget.activeObject>0 ) 
	{
		if (  ObjectExist(t.widget.activeObject) == 0 ) 
		{
			t.widget.activeObject=0;
		}
	}

	//  check if the widget is needed
	if (  t.widget.activeObject  ==  0 ) 
	{

		widget_check_for_new_object_selection ( );

	}
	else
	{
			//  Setup positions for widget objects
			if (  ObjectExist(t.widget.activeObject) == 1 ) 
			{
				for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
				{
					if (  ObjectExist (g.widgetobjectoffset+t.a) )  
					{
						PositionObject (  g.widgetobjectoffset+t.a, CameraPositionX(), CameraPositionY(), CameraPositionZ() );
						PointObject (  g.widgetobjectoffset+t.a,ObjectPositionX(t.widget.activeObject)+t.widget.offsetx,ObjectPositionY(t.widget.activeObject)+t.widget.offsety,ObjectPositionZ(t.widget.activeObject)+t.widget.offsetz );
						MoveObject (  g.widgetobjectoffset+t.a,40 );
						RotateObject (  g.widgetobjectoffset+t.a,0,0,0 );
					}
				}
				//  detect if widget panel off screen, and shift back in
				widget_correctwidgetpanel ( );
				RotateObject (  g.widgetobjectoffset+6, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
				RotateObject (  g.widgetobjectoffset+7, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
				RotateObject (  g.widgetobjectoffset+8, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
				RotateObject (  g.widgetobjectoffset+9, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
				RotateObject (  g.widgetobjectoffset+10, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
				RotateObject (  g.widgetobjectoffset+11, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
				RotateObject (  g.widgetobjectoffset+12, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
			}

			//  If we havent picked a section of the widget, lets test for it
			if ( t.widget.pickedSection == 0 || g.fForceYRotationOfRubberBandFromKeyPress > 0.0f )
			{
				//  Test for the widget selection keys X,C and V
				t.tscancode = t.inputsys.kscancode;
				if ( g.fForceYRotationOfRubberBandFromKeyPress > 0.0f )
				{
					t.toldmode = t.widget.mode;
					t.widget.mode = 1;
					t.widget.mclickpress = 2;
				}
				else
				{
					if (  t.widget.oldScanCode !=  t.tscancode ) 
					{
						t.toldmode = t.widget.mode;
						if (  t.tscancode  ==  WIDGET_KEY_TRANSLATE  )  t.widget.mode  =  0;
						if (  t.tscancode  ==  WIDGET_KEY_ROTATE  )  t.widget.mode  =  1;
						if (  t.tscancode  ==  WIDGET_KEY_SCALE  )  t.widget.mode  =  2;
						if ( t.toldmode != t.widget.mode ) widget_show_widget ( );
					}
				}
				t.widget.oldScanCode = t.tscancode;

				//  some setup code for picking widget section
				if (  t.thighlighterobj == t.widget.widgetPRPObj ) 
				{
					//  click and release
					if (  t.inputsys.mclick == 1 && t.widget.oldMouseClick == 0 && t.widget.mclickpress == 0  )  t.widget.mclickpress = 1;
					if (  t.inputsys.mclick == 0 && t.widget.mclickpress == 1  )  t.widget.mclickpress = 2;
				}
				else
				{
					//  just click
					if (  t.inputsys.mclick == 1 && t.widget.oldMouseClick == 0  )  t.widget.mclickpress = 2;
				}
				if (  t.widget.mclickpress == 2 ) 
				{
					//  See if a section has been chosen
					t.widget.pickedSection = PickScreenObject(t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f, g.widgetobjectoffset+0, g.widgetobjectoffset+t.widget.widgetMAXObj);

					//  as soon as click down, record for possible undo
					if ( t.widget.pickedSection > 0 )
					{
						// but only if we select a gadget (if click nothing we keep older position)
						t.sttentitytoselect=t.tentitytoselect;
						t.tentitytoselect=t.widget.pickedEntityIndex;
						if ( g.entityrubberbandlist.size() > 0 )
							gridedit_moveentityrubberband();
						else
							entity_recordbuffer_move ( );
						t.tentitytoselect=t.sttentitytoselect;
					}

					//  offset+20 is the plane object
					t.widget.mclickpress=0;
					ShowObject (  t.widget.widgetPlaneObj );
					PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
					RotateObject (  t.widget.widgetPlaneObj,-90,0,0 );
					t.a=PickScreenObject (t.widgetinputsysxmouse_f, t.widgetinputsysymouse_f, t.widget.widgetPlaneObj, t.widget.widgetPlaneObj);
					t.toriginalClickX_f = CameraPositionX() + GetPickVectorX();
					t.toriginalClickY_f = CameraPositionY() + GetPickVectorY();
					t.toriginalClickZ_f = CameraPositionZ() + GetPickVectorZ();
					t.toriginalClick2X_f = t.toriginalClickX_f;
					t.toriginalClick2Y_f = t.toriginalClickY_f;
					t.toriginalClick2Z_f = t.toriginalClickZ_f;
					t.tdx_f = t.toriginalClickX_f - ObjectPositionX(t.widget.activeObject);
					t.tdy_f = t.toriginalClickY_f - ObjectPositionY(t.widget.activeObject);
					t.tdz_f = t.toriginalClickZ_f - ObjectPositionZ(t.widget.activeObject);
					t.toriginalDistance_f = Sqrt(t.tdx_f*t.tdx_f + t.tdy_f*t.tdy_f + t.tdz_f*t.tdz_f);
					t.fOriginalDistanceX = t.tdx_f;
					t.fOriginalDistanceZ = t.tdz_f;
					if (  t.widget.pickedSection  ==  t.widget.widgetYScaleObj ) 
					{
						RotateObject (  t.widget.widgetPlaneObj,0,0,0 );
						t.a=PickScreenObject (t.widgetinputsysxmouse_f, t.widgetinputsysymouse_f, t.widget.widgetPlaneObj, t.widget.widgetPlaneObj);
						t.toriginalClick2X_f = CameraPositionX() + GetPickVectorX();
						t.toriginalClick2Y_f = CameraPositionY() + GetPickVectorY();
						t.toriginalClick2Z_f = CameraPositionZ() + GetPickVectorZ();
					}

					if (  t.widget.pickedSection  ==  t.widget.widgetXScaleObj ) 
					{
						if (  t.tdx_f<0  )  t.toriginalDistance_f = t.toriginalDistance_f*-1;
					}
					if (  t.widget.pickedSection  ==  t.widget.widgetYScaleObj ) 
					{
						if (  t.tdy_f<0  )  t.toriginalDistance_f = t.toriginalDistance_f*-1;
					}
					if (  t.widget.pickedSection  ==  t.widget.widgetZScaleObj ) 
					{
						if (  t.tdz_f<0  )  t.toriginalDistance_f = t.toriginalDistance_f*-1;
					}
					t.toriginalScaleX_f = ObjectScaleX(t.widget.activeObject);
					t.toriginalScaleY_f = ObjectScaleY(t.widget.activeObject);
					t.toriginalScaleZ_f = ObjectScaleZ(t.widget.activeObject);
					t.toriginalDistance2_f = Sqrt(t.tdx_f*t.tdx_f);
					if (  t.tdx_f<0  )  t.toriginalDistance2_f = t.toriginalDistance2_f*-1;

					// record all current offsets from primary widget object
					for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
					{
						int e = g.entityrubberbandlist[i].e;
						GGVECTOR3 VecPos;
						VecPos.x = t.entityelement[e].x - ObjectPositionX ( t.widget.activeObject );
						VecPos.y = t.entityelement[e].y - ObjectPositionY ( t.widget.activeObject );
						VecPos.z = t.entityelement[e].z - ObjectPositionZ ( t.widget.activeObject );
						// transform offset with current inversed orientation of primary object
						int tobj = t.entityelement[e].obj;
						if ( tobj > 0 )
						{
							float fDet = 0.0f;
							sObject* pObject = GetObjectData(tobj);
							GGMATRIX inverseMatrix = pObject->position.matObjectNoTran;
							GGMatrixInverse ( &inverseMatrix, &fDet, &inverseMatrix );
							GGVec3TransformCoord ( &VecPos, &VecPos, &inverseMatrix );
							g.entityrubberbandlist[i].x = VecPos.x;
							g.entityrubberbandlist[i].y = VecPos.y;
							g.entityrubberbandlist[i].z = VecPos.z;
						}
					}

					RotateObject (  t.widget.widgetPlaneObj,0,0,0 );
					t.a=PickScreenObject (t.widgetinputsysxmouse_f, t.widgetinputsysymouse_f, t.widget.widgetPlaneObj, t.widget.widgetPlaneObj) ;
					t.toriginalClickX_f = CameraPositionX() + GetPickVectorX();
					t.toriginalClickY_f = CameraPositionY() + GetPickVectorY();
					t.toriginalClickZ_f = CameraPositionZ() + GetPickVectorZ();
					t.tdx_f = t.toriginalClickX_f - ObjectPositionX(t.widget.activeObject);
					t.tdy_f = t.toriginalClickY_f - ObjectPositionY(t.widget.activeObject);
					t.tdz_f = t.toriginalClickZ_f - ObjectPositionZ(t.widget.activeObject);
					t.toriginalTranslateClickY_f = CameraPositionY() + GetPickVectorY() - ObjectPositionY(t.widget.activeObject);

					//  for the YZ position modifier
					RotateObject (  t.widget.widgetPlaneObj,0,90,0 );
					t.a=PickScreenObject (t.widgetinputsysxmouse_f, t.widgetinputsysymouse_f, t.widget.widgetPlaneObj, t.widget.widgetPlaneObj);
					t.toriginalTranslateClickYonZ_f = CameraPositionY() + GetPickVectorY() - ObjectPositionY(t.widget.activeObject);

					//  record entity RY for ragdoll/character rotation code lower down
					t.tlastgoody_f = ObjectAngleY(t.widget.activeObject);

					ShowObject (  t.widget.widgetPlaneObj );
					PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
					RotateObject (  t.widget.widgetPlaneObj, -90,0,0 );
					t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
					t.toriginalTranslateClickX_f = CameraPositionX() + GetPickVectorX() - ObjectPositionX(t.widget.activeObject);
					t.toriginalTranslateClickZ_f = CameraPositionZ() + GetPickVectorZ() - ObjectPositionZ(t.widget.activeObject);

					//  310315 - XZ startclick for XY and ZY modding
					RotateObject (  t.widget.widgetPlaneObj,0,0,0 );
					t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
					t.toriginalTranslateClickX1_f = CameraPositionX() + GetPickVectorX() - ObjectPositionX(t.widget.activeObject);
					RotateObject (  t.widget.widgetPlaneObj,0,90,0 );
					t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
					t.toriginalTranslateClickZ2_f = CameraPositionZ() + GetPickVectorZ() - ObjectPositionZ(t.widget.activeObject);

					HideObject (  t.widget.widgetPlaneObj );

					//  mode 0 = translate, 1 = rotate, 2 = scale
					if (  t.widget.mode  ==  0 || t.widget.mode  ==  2 ) 
					{
						ShowObject (  t.widget.widgetXColObj );
						ShowObject (  t.widget.widgetYColObj );
						ShowObject (  t.widget.widgetZColObj );
						if (  t.widget.mode  ==  0 ) 
						{
							RotateObject (  g.widgetobjectoffset+16, 0,0,0 );
							RotateObject (  g.widgetobjectoffset+17, 0,0,0 );
							RotateObject (  g.widgetobjectoffset+18, 0,0,0 );
						}
						else
						{
							RotateObject (  g.widgetobjectoffset+16, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
							RotateObject (  g.widgetobjectoffset+17, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
							RotateObject (  g.widgetobjectoffset+18, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
						}
					}

					if (  t.widget.mode  ==  0 || t.widget.mode  ==  2 ) 
					{
						HideObject (  t.widget.widgetXColObj );
						HideObject (  t.widget.widgetYColObj );
						HideObject (  t.widget.widgetZColObj );
					}

					if (  t.widget.pickedSection >= t.widget.widgetPOSObj && t.widget.pickedSection <= t.widget.widgetLCKObj ) 
					{
						t.toldmode=t.widget.mode;
						if (  t.widget.pickedSection == t.widget.widgetPOSObj  )  t.widget.mode = 0;
						if (  t.widget.pickedSection == t.widget.widgetROTObj  )  t.widget.mode = 1;
						if (  t.widget.pickedSection == t.widget.widgetSCLObj  )  
						{
							bool bIsThisAnEBE = false;
							if ( t.widget.pickedEntityIndex > 0 ) 
							{
								int iEntID = t.entityelement[t.widget.pickedEntityIndex].bankindex;
								if ( iEntID > 0 ) 
									if ( t.entityprofile[iEntID].isebe != 0 )
										bIsThisAnEBE = true;
							}
							if ( bIsThisAnEBE == true )
							{
								//  Edit EBE construction
								t.widget.propertybuttonselected = 1;
								t.ebe.bReleaseMouseFirst = true;							
							}
							else
							{
								// default is scale
								t.widget.mode = 2;
							}
						}
						if (  t.widget.pickedSection == t.widget.widgetPRPObj ) 
						{
							//  entity properties / Save EBE construction
							t.widget.propertybuttonselected = 2;
						}
						if (  t.widget.pickedSection == t.widget.widgetDUPObj ) 
						{
							//  duplicate / now extract
							t.widget.duplicatebuttonselected=1;
						}
						if (  t.widget.pickedSection == t.widget.widgetDELObj ) 
						{
							//  delete
							t.widget.deletebuttonselected=1;
						}
						if ( t.widget.pickedSection == t.widget.widgetLCKObj ) 
						{
							//  entity lock/unlock
							if ( t.widget.pickedEntityIndex>0 ) 
							{
								int iLoopMax = 1;
								if ( g.entityrubberbandlist.size() > 0 ) iLoopMax = g.entityrubberbandlist.size();
								for ( int i = 0; i < iLoopMax; i++ )
								{
									// get entity index
									int e = t.widget.pickedEntityIndex;
									if ( g.entityrubberbandlist.size() > 0 )
										e = g.entityrubberbandlist[i].e;

									// toggle lock flag
									t.entityelement[e].editorlock = 1 - t.entityelement[e].editorlock;

									// also recreate entity as a clone and set as semi-transparent
									/* this messes up depth render order, totally, best to leave as solid, just locked
									if ( t.entityelement[e].editorlock == 1 ) 
									{
										t.tte=e; t.tobj=t.entityelement[t.tte].obj;
										if ( t.tobj>0 ) 
										{
											if ( ObjectExist(t.tobj) == 1 ) 
											{
												entity_converttoclonetransparent ( );
											}
										}
									}
									*/
								}
								gridedit_clearentityrubberbandlist();
							}
							//  exit widget when lock entity
							t.widget.pickedSection=0;
						}
						if ( t.toldmode != t.widget.mode ) widget_show_widget ( );
					}
					else
					{
						if (  t.widget.pickedSection  >=  g.widgetobjectoffset+16 && t.widget.pickedSection  <=  g.widgetobjectoffset+t.widget.widgetMAXObj ) 
						{
							if (  t.widget.mode  ==  0  )  t.widget.pickedSection -= 16;
							if (  t.widget.mode  ==  2  )  t.widget.pickedSection -= 7;
						}
					}

					//  No section selected, so switch the widget off
					if (  t.widget.pickedSection  ==  0 && g.fForceYRotationOfRubberBandFromKeyPress == 0.0f ) 
					{
						t.widget.activeObject = 0;
						t.widget.pickedObject = 0;
						//  LEE; Can have better code re-use here!
						if (  t.widget.activeObject  ==  0 ) 
						{
							for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
							{
								if (  ObjectExist(g.widgetobjectoffset+t.a) ) 
								{
									PositionObject (  g.widgetobjectoffset+t.a,-100000,-100000,-100000 );
									HideObject (  g.widgetobjectoffset+t.a );
								}
							}
						}
					}

				}
			}
			if ( t.widget.pickedSection == 0 && g.fForceYRotationOfRubberBandFromKeyPress == 0 )
			{
				// code done above, needed here to handle g.fForceYRotationOfRubberBandFromKeyPress logic
			}
			else
			{
				// 201015 - store old active object position (for later if we need to also move rubber band highlighted objects)
				float fOldActiveObjectX = ObjectPositionX ( t.widget.activeObject );
				float fOldActiveObjectY = ObjectPositionY ( t.widget.activeObject );
				float fOldActiveObjectZ = ObjectPositionZ ( t.widget.activeObject );

				if (  t.widget.pickedSection >= t.widget.widgetPOSObj && t.widget.pickedSection <= t.widget.widgetLCKObj ) 
				{
				//  widget button selected
				}
				else
				{
				//  translate
				if (  t.widget.mode == 0 && ObjectExist(t.widget.activeObject) == 1 ) 
				{
					if (  t.widget.pickedSection  ==  t.widget.widgetXObj ) 
					{
						PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
						RotateObject (  t.widget.widgetPlaneObj, -90,0,0 );
						ShowObject (  t.widget.widgetPlaneObj );
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						HideObject (  t.widget.widgetPlaneObj );
						t.tx_f = CameraPositionX() + GetPickVectorX();
						PositionObject (  t.widget.activeObject,t.tx_f - t.toriginalTranslateClickX_f,ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
					}
					if (  t.widget.pickedSection  ==  t.widget.widgetYObj ) 
					{
						PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
						RotateObject (  t.widget.widgetPlaneObj,0,0,0 );
						ShowObject (  t.widget.widgetPlaneObj );
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						HideObject (  t.widget.widgetPlaneObj );
						t.ty_f = CameraPositionY() + GetPickVectorY();
						PositionObject (  t.widget.activeObject,ObjectPositionX(t.widget.activeObject),t.ty_f - t.toriginalTranslateClickY_f,ObjectPositionZ(t.widget.activeObject) );
					}
					if (  t.widget.pickedSection  ==  t.widget.widgetZObj ) 
					{
						PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
						RotateObject (  t.widget.widgetPlaneObj,-90,0,0 );
						ShowObject (  t.widget.widgetPlaneObj );
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						HideObject (  t.widget.widgetPlaneObj );
						t.tz_f = CameraPositionZ() + GetPickVectorZ();
						PositionObject (  t.widget.activeObject,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),t.tz_f - t.toriginalTranslateClickZ_f );
					}
					if (  t.widget.pickedSection  ==  t.widget.widgetXYObj ) 
					{
						PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
						RotateObject (  t.widget.widgetPlaneObj,0,0,0 );
						ShowObject (  t.widget.widgetPlaneObj );
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						t.tx_f = CameraPositionX() + GetPickVectorX();
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						HideObject (  t.widget.widgetPlaneObj );
						t.ty_f = CameraPositionY() + GetPickVectorY();
						PositionObject (  t.widget.activeObject,t.tx_f - t.toriginalTranslateClickX1_f,t.ty_f - t.toriginalTranslateClickY_f,ObjectPositionZ(t.widget.activeObject) );
					}
					if (  t.widget.pickedSection  ==  t.widget.widgetXZObj ) 
					{
						PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
						RotateObject (  t.widget.widgetPlaneObj,-90,0,0 );
						ShowObject (  t.widget.widgetPlaneObj );
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						HideObject (  t.widget.widgetPlaneObj );
						t.tx_f = CameraPositionX() + GetPickVectorX();
						t.tz_f = CameraPositionZ() + GetPickVectorZ();
						PositionObject (  t.widget.activeObject,t.tx_f - t.toriginalTranslateClickX_f,ObjectPositionY(t.widget.activeObject),t.tz_f - t.toriginalTranslateClickZ_f );
					}
					if (  t.widget.pickedSection  ==  t.widget.widgetYZObj ) 
					{
						PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
						RotateObject (  t.widget.widgetPlaneObj,0,90,0 );
						ShowObject (  t.widget.widgetPlaneObj );
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						t.tz_f = CameraPositionZ() + GetPickVectorZ();
						t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
						if (  t.a == 0 ) 
						{
							t.ttdx_f=CameraPositionX()-ObjectPositionX(t.widget.activeObject);
							t.ttdy_f=CameraPositionY()-ObjectPositionY(t.widget.activeObject);
							t.ttdz_f=CameraPositionZ()-ObjectPositionZ(t.widget.activeObject);
							t.ttdist_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.ttdist_f );
						}
						HideObject (  t.widget.widgetPlaneObj );
						t.ty_f = CameraPositionY() + GetPickVectorY();
						PositionObject (  t.widget.activeObject,ObjectPositionX(t.widget.activeObject),t.ty_f - t.toriginalTranslateClickYonZ_f,t.tz_f - t.toriginalTranslateClickZ2_f );
					}
					t.te=t.widget.pickedEntityIndex;
					t.entityelement[t.te].x=ObjectPositionX(t.widget.activeObject);
					t.entityelement[t.te].y=ObjectPositionY(t.widget.activeObject);
					t.entityelement[t.te].z=ObjectPositionZ(t.widget.activeObject);
					t.entityelement[t.te].beenmoved=1;
					// mark as static if it was
					if ( t.entityelement[t.te].staticflag == 1 ) g.projectmodifiedstatic = 1;
					//  update infinilight list with addition
					t.tttentid=t.entityelement[t.te].bankindex;
					if (  t.entityprofile[t.tttentid].ismarker == 2 || t.entityprofile[t.tttentid].ismarker == 5 ) 
					{
						lighting_refresh ( );
					}
				}

				// 201015 - if we need to also move rubber band highlighted objects, do so now
				if ( g.entityrubberbandlist.size() > 0 )
				{
					float fMovedActiveObjectX = ObjectPositionX ( t.widget.activeObject ) - fOldActiveObjectX;
					float fMovedActiveObjectY = ObjectPositionY ( t.widget.activeObject ) - fOldActiveObjectY;
					float fMovedActiveObjectZ = ObjectPositionZ ( t.widget.activeObject ) - fOldActiveObjectZ;
					for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
					{
						int e = g.entityrubberbandlist[i].e;
						int tobj = t.entityelement[e].obj;
						if ( tobj > 0 )
						{
							if ( ObjectExist(tobj) == 1 )
							{
								if ( tobj != t.widget.activeObject )
								{
									// reposition this entity
									PositionObject ( tobj, ObjectPositionX(tobj)+fMovedActiveObjectX, ObjectPositionY(tobj)+fMovedActiveObjectY, ObjectPositionZ(tobj)+fMovedActiveObjectZ );
									t.entityelement[e].x = ObjectPositionX(tobj);
									t.entityelement[e].y = ObjectPositionY(tobj);
									t.entityelement[e].z = ObjectPositionZ(tobj);
									// mark as static if it was
									if ( t.entityelement[e].staticflag == 1 ) g.projectmodifiedstatic = 1;
									// also update zones if part of group
									widget_movezonesandlights ( e );
								}
							}
						}
					}
				}

				//  only allow rot/scl for non markers
				t.te=t.widget.pickedEntityIndex;
				t.tttentid=t.entityelement[t.te].bankindex;
				//  some markers have Y rotation
				t.thaveyrot = 0; 
				if ( t.entityprofile[t.tttentid].ismarker == 1 || t.entityprofile[t.tttentid].ismarker == 6 || t.entityprofile[t.tttentid].ismarker == 7  )  t.thaveyrot = 1;

				//PE: Allow spotlight rotation.
				if (t.entityelement[t.te].eleprof.usespotlighting) t.thaveyrot = 1;

				if ( (t.entityprofile[t.tttentid].ismarker == 0 || t.thaveyrot == 1) && ObjectExist(t.widget.activeObject) == 1 ) 
				{
						// rotate
						if ( t.widget.mode ==  1 || g.fForceYRotationOfRubberBandFromKeyPress > 0.0f ) 
						{
							// 271015 - store old active object angles
							float fOldActiveObjectRX = ObjectAngleX ( t.widget.activeObject );
							float fOldActiveObjectRY = ObjectAngleY ( t.widget.activeObject );
							float fOldActiveObjectRZ = ObjectAngleZ ( t.widget.activeObject );

							//  RotateObject (  on all three axis )
							PickScreen2D23D (  t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,50000 );

							//  control distant mouse pointer object
							t.p0x_f=CameraPositionX();
							t.p0y_f=CameraPositionY();
							t.p0z_f=CameraPositionZ();
							t.px_f=t.p0x_f+GetPickVectorX();
							t.py_f=t.p0y_f+GetPickVectorY();
							t.pz_f=t.p0z_f+GetPickVectorZ();
							PositionObject (  t.twidgetRotStartObject+11,t.px_f,t.py_f,t.pz_f );
							t.p1x_f=ObjectPositionX(t.twidgetRotStartObject+11);
							t.p1y_f=ObjectPositionY(t.twidgetRotStartObject+11);
							t.p1z_f=ObjectPositionZ(t.twidgetRotStartObject+11);

							//  handle rotation modes
							if (  t.widget.pickedSection  ==  t.widget.widgetXRotObj && (t.thaveyrot == 0 || t.entityelement[t.te].eleprof.usespotlighting ) )
							{
								if (  ObjectExist(t.twidgetRotStartObject+14) == 1  )  DeleteObject (  t.twidgetRotStartObject+14 );
								MakeObjectBox (  t.twidgetRotStartObject+14,1,5000000,5000000 );
								HideObject (  t.twidgetRotStartObject+14 );
								PositionObject (  t.twidgetRotStartObject+14,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
								RotateObject (  t.twidgetRotStartObject+14,ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject),ObjectAngleZ(t.widget.activeObject) );
								if (  IntersectObject(t.twidgetRotStartObject+14,t.p0x_f,t.p0y_f,t.p0z_f,t.p1x_f,t.p1y_f,t.p1z_f) != 0 ) 
								{
									t.pinterx_f=ChecklistFValueA(6);
									t.pintery_f=ChecklistFValueB(6);
									t.pinterz_f=ChecklistFValueC(6);
								}
								PositionObject (  t.twidgetRotStartObject+13,t.pinterx_f,t.pintery_f,t.pinterz_f );
							}
							if (  t.widget.pickedSection  ==  t.widget.widgetYRotObj ) 
							{
								if (  ObjectExist(t.twidgetRotStartObject+14) == 1  )  DeleteObject (  t.twidgetRotStartObject+14 );
								MakeObjectBox (  t.twidgetRotStartObject+14,5000000,1,5000000 );
								HideObject (  t.twidgetRotStartObject+14 );
								PositionObject (  t.twidgetRotStartObject+14,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
								RotateObject (  t.twidgetRotStartObject+14,ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject),ObjectAngleZ(t.widget.activeObject) );
								if (  IntersectObject(t.twidgetRotStartObject+14,t.p0x_f,t.p0y_f,t.p0z_f,t.p1x_f,t.p1y_f,t.p1z_f) != 0 ) 
								{
									t.pinterx_f=ChecklistFValueA(6);
									t.pintery_f=ChecklistFValueB(6);
									t.pinterz_f=ChecklistFValueC(6);
								}
								PositionObject (  t.twidgetRotStartObject+13,t.pinterx_f,t.pintery_f,t.pinterz_f );
							}
							if (  t.widget.pickedSection  ==  t.widget.widgetZRotObj && ( t.thaveyrot == 0|| t.entityelement[t.te].eleprof.usespotlighting) )
							{
								if (  ObjectExist(t.twidgetRotStartObject+14) == 1  )  DeleteObject (  t.twidgetRotStartObject+14 );
								MakeObjectBox (  t.twidgetRotStartObject+14,5000000,5000000,1 );
								HideObject (  t.twidgetRotStartObject+14 );
								PositionObject (  t.twidgetRotStartObject+14,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
								RotateObject (  t.twidgetRotStartObject+14,ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject),ObjectAngleZ(t.widget.activeObject) );
								if (  IntersectObject(t.twidgetRotStartObject+14,t.p0x_f,t.p0y_f,t.p0z_f,t.p1x_f,t.p1y_f,t.p1z_f) != 0 ) 
								{
									t.pinterx_f=ChecklistFValueA(6);
									t.pintery_f=ChecklistFValueB(6);
									t.pinterz_f=ChecklistFValueC(6);
								}
								PositionObject (  t.twidgetRotStartObject+13,t.pinterx_f,t.pintery_f,t.pinterz_f );
							}

							//  control rotations
							if (  t.widget.grabbed == 0 ) 
							{
								//  start drag
								t.gmx=t.widgetinputsysxmouse_f;
								t.gmy=t.widgetinputsysymouse_f;
								PositionObject (  t.twidgetRotStartObject+12,t.pinterx_f,t.pintery_f,t.pinterz_f );
								t.fAngleStoreX = ObjectAngleX ( t.widget.activeObject );
								t.fAngleStoreY = ObjectAngleY ( t.widget.activeObject );
								t.fAngleStoreZ = ObjectAngleZ ( t.widget.activeObject );
								t.widget.grabbed=1;
							}
							if (  t.widget.grabbed == 1 ) 
							{
								SetIdentityMatrix (  g.widgetStartMatrix+3 );
								RotateXMatrix (  g.widgetStartMatrix+4,ObjectAngleX(t.widget.activeObject)*0.017444 );
								MultiplyMatrix (  g.widgetStartMatrix+3,g.widgetStartMatrix+3,g.widgetStartMatrix+4 );
								RotateYMatrix (  g.widgetStartMatrix+4,ObjectAngleY(t.widget.activeObject)*0.017444 );
								MultiplyMatrix (  g.widgetStartMatrix+3,g.widgetStartMatrix+3,g.widgetStartMatrix+4 );
								RotateZMatrix (  g.widgetStartMatrix+4,ObjectAngleZ(t.widget.activeObject)*0.017444 );
								MultiplyMatrix (  g.widgetStartMatrix+3,g.widgetStartMatrix+3,g.widgetStartMatrix+4 );
								t.widget.grabbed=2;
							}
							if (  t.widget.grabbed == 2 || g.fForceYRotationOfRubberBandFromKeyPress > 0.0f ) 
							{
								if ( g.fForceYRotationOfRubberBandFromKeyPress > 0.0f )
								{
									// comes from pressing the R key (and 3 and 4 key)
									t.tanglediff_f = g.fForceYRotationOfRubberBandFromKeyPress;
								}
								else
								{
									//  dragging in world space
									//  x-axis oriented by object
									if (  t.widget.pickedSection == t.widget.widgetXRotObj  )  SetVector3 (  g.widgetvectorindex+1,1,0,0 );
									if (  t.widget.pickedSection == t.widget.widgetYRotObj  )  SetVector3 (  g.widgetvectorindex+1,0,1,0 );
									if (  t.widget.pickedSection == t.widget.widgetZRotObj  )  SetVector3 (  g.widgetvectorindex+1,0,0,1 );
									TransformVectorCoordinates3 (  g.widgetvectorindex+1,g.widgetvectorindex+1,g.widgetStartMatrix+3 );
									//  work out grab locations
									SetVector3 (  g.widgetvectorindex+7,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
									SetVector3 (  g.widgetvectorindex+8,ObjectPositionX(t.twidgetRotStartObject+12),ObjectPositionY(t.twidgetRotStartObject+12),ObjectPositionZ(t.twidgetRotStartObject+12) );
									SetVector3 (  g.widgetvectorindex+9,ObjectPositionX(t.twidgetRotStartObject+13),ObjectPositionY(t.twidgetRotStartObject+13),ObjectPositionZ(t.twidgetRotStartObject+13) );
									SubtractVector3 (  g.widgetvectorindex+8,g.widgetvectorindex+8,g.widgetvectorindex+7 );
									SubtractVector3 (  g.widgetvectorindex+9,g.widgetvectorindex+9,g.widgetvectorindex+7 );
									SetVector3 (  g.widgetvectorindex+7,0,0,0 );
									if (  t.tanglediff_f>360  )  t.tanglediff_f = t.tanglediff_f-360;
									if (  t.tanglediff_f<-360  )  t.tanglediff_f = t.tanglediff_f+360;
									//  transform vector 8 and 9 around new UP vector
									SetVector3 (  g.widgetvectorindex+12,0,1,0 );
									CopyMatrix (  g.widgetStartMatrix+4,g.widgetStartMatrix+3 );
									t.r=InverseMatrix(g.widgetStartMatrix+4,g.widgetStartMatrix+4);
									TransformVectorCoordinates3 (  g.widgetvectorindex+8,g.widgetvectorindex+8,g.widgetStartMatrix+4 );
									TransformVectorCoordinates3 (  g.widgetvectorindex+9,g.widgetvectorindex+9,g.widgetStartMatrix+4 );
									if (  t.widget.pickedSection == t.widget.widgetXRotObj ) 
									{
										t.tangle1_f=atan2deg(GetZVector3(g.widgetvectorindex+8),GetYVector3(g.widgetvectorindex+8)) ;
									}
									if (  t.widget.pickedSection == t.widget.widgetYRotObj ) 
									{
										t.tangle1_f=atan2deg(GetXVector3(g.widgetvectorindex+8),GetZVector3(g.widgetvectorindex+8));
									}
									if (  t.widget.pickedSection == t.widget.widgetZRotObj ) 
									{
										t.tangle1_f=atan2deg(GetYVector3(g.widgetvectorindex+8),GetXVector3(g.widgetvectorindex+8)) ;
									}
									if (  t.widget.pickedSection == t.widget.widgetXRotObj ) 
									{
										t.tangle2_f=atan2deg(GetZVector3(g.widgetvectorindex+9),GetYVector3(g.widgetvectorindex+9));
									}
									if (  t.widget.pickedSection == t.widget.widgetYRotObj ) 
									{
										t.tangle2_f=atan2deg(GetXVector3(g.widgetvectorindex+9),GetZVector3(g.widgetvectorindex+9));
									}
									if (  t.widget.pickedSection == t.widget.widgetZRotObj ) 
									{
										t.tangle2_f=atan2deg(GetYVector3(g.widgetvectorindex+9),GetXVector3(g.widgetvectorindex+9)) ;
									}
									t.tanglediff_f=t.tangle2_f-t.tangle1_f;
								}

								// if group rotation, use simpler Y only rotation
								if ( g.entityrubberbandlist.size() > 0 )
								{
									// avoids messy math for now
									t.pVecAnglesx_f = t.fAngleStoreX;
									t.pVecAnglesy_f = t.fAngleStoreY + t.tanglediff_f;
									t.pVecAnglesz_f = t.fAngleStoreZ;
								}
								else
								{
									//  apply rotation
									BuildRotationAxisMatrix (  g.widgetStartMatrix+4,g.widgetvectorindex+1,t.tanglediff_f*0.017444 ) ;
									MultiplyMatrix (  g.widgetStartMatrix+5,g.widgetStartMatrix+3,g.widgetStartMatrix+4 );
									//  convert matrix to euler angles
									t.m00_f = GetMatrixElement(g.widgetStartMatrix+5,0);
									t.m01_f = GetMatrixElement(g.widgetStartMatrix+5,1);
									t.m02_f = GetMatrixElement(g.widgetStartMatrix+5,2);
									t.m12_f = GetMatrixElement(g.widgetStartMatrix+5,6);
									t.m22_f = GetMatrixElement(g.widgetStartMatrix+5,10);
									t.heading_f = atan2deg(t.m01_f,t.m00_f);
									t.attitude_f = atan2deg(t.m12_f,t.m22_f);
									t.bank_f = Asin(-t.m02_f);
									if (  abs ( t.m02_f ) > 1.0 ) 
									{
										t.PI_f = 3.14159265f / 2.0f;
										t.pVecAnglesx_f = 0.0;
										t.pVecAnglesy_f = ( t.PI_f * t.m02_f ) / 0.017444;
										t.pVecAnglesz_f = 0.0;
									}
									else
									{
										t.pVecAnglesx_f = ( t.attitude_f );
										t.pVecAnglesy_f = ( t.bank_f );
										t.pVecAnglesz_f = ( t.heading_f );
									}
								}
								//  update object with regular euler
								RotateObject ( t.widget.activeObject,t.pVecAnglesx_f,t.pVecAnglesy_f,t.pVecAnglesz_f );
							}
							//  transfer final eulers to entity element setting
							if (  t.entityprofile[t.tttentid].ragdoll == 1 ) 
							{
								t.entityelement[t.te].rx=0;
								t.entityelement[t.te].ry=t.tlastgoody_f+t.tanglediff_f;
								t.entityelement[t.te].rz=0;
								RotateObject (  t.widget.activeObject,0,t.entityelement[t.te].ry,0 );
							}
							else
							{
								t.entityelement[t.te].rx=ObjectAngleX(t.widget.activeObject);
								t.entityelement[t.te].ry=ObjectAngleY(t.widget.activeObject);
								t.entityelement[t.te].rz=ObjectAngleZ(t.widget.activeObject);
							}
							// mark as static if it was
							if ( t.entityelement[t.te].staticflag == 1 ) g.projectmodifiedstatic = 1;

							// 271015 - if we need to also rotate rubber band highlighted objects, do so now
							if ( g.entityrubberbandlist.size() > 0 )
							{
								// rotate all the grouped entities and move around Y axis of widget as pivot
								float fMovedActiveObjectRX = ObjectAngleX ( t.widget.activeObject ) - fOldActiveObjectRX;
								float fMovedActiveObjectRY = ObjectAngleY ( t.widget.activeObject ) - fOldActiveObjectRY;
								float fMovedActiveObjectRZ = ObjectAngleZ ( t.widget.activeObject ) - fOldActiveObjectRZ;
								for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
								{
									int e = g.entityrubberbandlist[i].e;
									int tobj = t.entityelement[e].obj;
									if ( tobj > 0 )
									{
										if ( ObjectExist(tobj) == 1 )
										{
											if ( tobj != t.widget.activeObject )
											{
												// 061115 - a fix for entities that have been inverted
												if ( t.entityelement[e].rx==180 && t.entityelement[e].rz==180 )
												{
													t.entityelement[e].rx = 0;
													t.entityelement[e].ry = t.entityelement[e].ry;
													t.entityelement[e].rz = 0;
													RotateObject ( tobj, t.entityelement[e].rx, t.entityelement[e].ry, t.entityelement[e].rz );
												}

												// rotate the entity
												RotateObject ( tobj, ObjectAngleX(tobj)+fMovedActiveObjectRX, ObjectAngleY(tobj)+fMovedActiveObjectRY, ObjectAngleZ(tobj)+fMovedActiveObjectRZ );
												t.entityelement[e].rx = ObjectAngleX(tobj);
												t.entityelement[e].ry = ObjectAngleY(tobj);
												t.entityelement[e].rz = ObjectAngleZ(tobj);
												// mark as static if it was
												if ( t.entityelement[e].staticflag == 1 ) g.projectmodifiedstatic = 1;

												// move the entity around a pivot point
												GGVECTOR3 VecPos;
												VecPos.x = g.entityrubberbandlist[i].x;
												VecPos.y = g.entityrubberbandlist[i].y;
												VecPos.z = g.entityrubberbandlist[i].z;
												sObject* pObject = GetObjectData(tobj);
												GGVec3TransformCoord ( &VecPos, &VecPos, &pObject->position.matObjectNoTran );
												t.entityelement[e].x = ObjectPositionX ( t.widget.activeObject ) + VecPos.x;
												t.entityelement[e].y = ObjectPositionY ( t.widget.activeObject ) + VecPos.y;
												t.entityelement[e].z = ObjectPositionZ ( t.widget.activeObject ) + VecPos.z;
												PositionObject ( tobj, t.entityelement[e].x, t.entityelement[e].y, t.entityelement[e].z );

												// move zones and lights if in group
												widget_movezonesandlights ( e );
											}
										}
									}
								}
							}
						}

						//  scale
						if (  t.entityprofile[t.tttentid].ragdoll == 0 && t.thaveyrot == 0 ) 
						{
							//  only if NOT using ragdoll
							if (  t.widget.mode  ==  2 ) 
							{
								// 271015 - store old active object scales
								float fOldActiveObjectSX = ObjectScaleX ( t.widget.activeObject );
								float fOldActiveObjectSY = ObjectScaleY ( t.widget.activeObject );
								float fOldActiveObjectSZ = ObjectScaleZ ( t.widget.activeObject );

								// 281015 - take scale of object inti account
								float fEntityTotalSize = ObjectSize ( t.widget.activeObject, 0 );
								float fEntityScalingRatio = 100.0f / fEntityTotalSize;

								// scale on X, Y and Z axis
								if (  t.widget.pickedSection  ==  t.widget.widgetXScaleObj ) 
								{
									ShowObject (  t.widget.widgetPlaneObj );
									PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
									RotateObject (  t.widget.widgetPlaneObj,-90,0,0 );
									t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
									HideObject (  t.widget.widgetPlaneObj );
									t.tNewClickX_f = CameraPositionX() + GetPickVectorX();
									t.tNewClickY_f = CameraPositionY() + GetPickVectorY();
									t.tNewClickZ_f = CameraPositionZ() + GetPickVectorZ();
									GGVECTOR3 vecOrig = GGVECTOR3(t.toriginalClick2X_f,t.toriginalClick2Y_f,t.toriginalClick2Z_f);
									GGVECTOR3 vecNew = GGVECTOR3(t.tNewClickX_f,t.tNewClickY_f,t.tNewClickZ_f);
									sObject* pObject = GetObjectData ( t.widget.activeObject );
									float pDet;
									GGMATRIX matInverse;
									GGMatrixInverse ( &matInverse, &pDet, &pObject->position.matRotation );
									vecNew = vecNew - vecOrig;
									GGVec3TransformCoord ( &vecNew, &vecNew, &matInverse );
									t.tnewDistance_f = vecNew.x;
									t.tfactor_f=(t.toriginalScaleX_f/100.0)*2;
									if (  t.tfactor_f<1.0  )  t.tfactor_f = 1.0;
									if (  t.tfactor_f>5.0  )  t.tfactor_f = 5.0;
									t.tscale_f = (t.tnewDistance_f*t.tfactor_f*fEntityScalingRatio)+t.toriginalScaleX_f;
									if (  t.tscale_f < 5  )  t.tscale_f  =  5;
									if (  t.tscale_f > 10000  )  t.tscale_f  =  10000;
									ScaleObject (  t.widget.activeObject,t.tscale_f,ObjectScaleY(t.widget.activeObject),ObjectScaleZ(t.widget.activeObject) );
								}
								if (  t.widget.pickedSection  ==  t.widget.widgetYScaleObj ) 
								{
									ShowObject (  t.widget.widgetPlaneObj );
									PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
									RotateObject (  t.widget.widgetPlaneObj,0,0,0 );
									t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
									HideObject (  t.widget.widgetPlaneObj );
									t.tNewClickX_f = CameraPositionX() + GetPickVectorX();
									t.tNewClickY_f = CameraPositionY() + GetPickVectorY();
									t.tNewClickZ_f = CameraPositionZ() + GetPickVectorZ();
									GGVECTOR3 vecOrig = GGVECTOR3(t.toriginalClick2X_f,t.toriginalClick2Y_f,t.toriginalClick2Z_f);
									GGVECTOR3 vecNew = GGVECTOR3(t.tNewClickX_f,t.tNewClickY_f,t.tNewClickZ_f);
									sObject* pObject = GetObjectData ( t.widget.activeObject );
									float pDet;
									GGMATRIX matInverse;
									GGMatrixInverse ( &matInverse, &pDet, &pObject->position.matRotation );
									vecNew = vecNew - vecOrig;
									GGVec3TransformCoord ( &vecNew, &vecNew, &matInverse );
									t.tnewDistance_f = vecNew.y;
									t.tfactor_f=(t.toriginalScaleY_f/100.0)*2;
									if (  t.tfactor_f<1.0  )  t.tfactor_f = 1.0;
									if (  t.tfactor_f>5.0  )  t.tfactor_f = 5.0;
									t.tscale_f = ((t.tnewDistance_f)*t.tfactor_f*fEntityScalingRatio)+t.toriginalScaleY_f;
									if (  t.tscale_f < 5  )  t.tscale_f  =  5;
									if (  t.tscale_f > 10000  )  t.tscale_f  =  10000;
									ScaleObject (  t.widget.activeObject,ObjectScaleX(t.widget.activeObject),t.tscale_f,ObjectScaleZ(t.widget.activeObject) );
								}
								if (  t.widget.pickedSection  ==  t.widget.widgetZScaleObj ) 
								{
									ShowObject (  t.widget.widgetPlaneObj );
									PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
									RotateObject (  t.widget.widgetPlaneObj,-90,0,0 );
									t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj) ;
									HideObject (  t.widget.widgetPlaneObj );
									t.tNewClickX_f = CameraPositionX() + GetPickVectorX();
									t.tNewClickY_f = CameraPositionY() + GetPickVectorY();
									t.tNewClickZ_f = CameraPositionZ() + GetPickVectorZ();
									GGVECTOR3 vecOrig = GGVECTOR3(t.toriginalClick2X_f,t.toriginalClick2Y_f,t.toriginalClick2Z_f);
									GGVECTOR3 vecNew = GGVECTOR3(t.tNewClickX_f,t.tNewClickY_f,t.tNewClickZ_f);
									sObject* pObject = GetObjectData ( t.widget.activeObject );
									float pDet;
									GGMATRIX matInverse;
									GGMatrixInverse ( &matInverse, &pDet, &pObject->position.matRotation );
									vecNew = vecNew - vecOrig;
									GGVec3TransformCoord ( &vecNew, &vecNew, &matInverse );
									t.tnewDistance_f = vecNew.z;
									t.tfactor_f=(t.toriginalScaleZ_f/100.0)*2;
									if (  t.tfactor_f<1.0  )  t.tfactor_f = 1.0;
									if (  t.tfactor_f>5.0  )  t.tfactor_f = 5.0;
									t.tscale_f = ((t.tnewDistance_f)*t.tfactor_f*fEntityScalingRatio)+t.toriginalScaleZ_f;
									if (  t.tscale_f < 5  )  t.tscale_f  =  5;
									if (  t.tscale_f > 10000  )  t.tscale_f  =  10000;
									ScaleObject (  t.widget.activeObject,ObjectScaleX(t.widget.activeObject),ObjectScaleY(t.widget.activeObject),t.tscale_f );
								}

								// scale on XYZ together
								if (  t.widget.pickedSection  ==  t.widget.widgetXYZScaleObj ) 
								{
									ShowObject (  t.widget.widgetPlaneObj );
									PositionObject (  t.widget.widgetPlaneObj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
									RotateObject (  t.widget.widgetPlaneObj,-90,0,0 );
									t.a=PickScreenObject (t.widgetinputsysxmouse_f,t.widgetinputsysymouse_f,t.widget.widgetPlaneObj,t.widget.widgetPlaneObj);
									HideObject (  t.widget.widgetPlaneObj );
									t.tNewClickX_f = CameraPositionX() + GetPickVectorX();
									t.tNewClickY_f = CameraPositionY() + GetPickVectorY();
									t.tNewClickZ_f = CameraPositionZ() + GetPickVectorZ();
									t.tdx_f = t.tNewClickX_f - ObjectPositionX(t.widget.activeObject);
									t.tdy_f = t.tNewClickY_f - ObjectPositionY(t.widget.activeObject);
									t.tdz_f = t.tNewClickZ_f - ObjectPositionZ(t.widget.activeObject);
									t.tnewDistance_f = Sqrt(t.tdx_f*t.tdx_f + t.tdy_f*t.tdy_f + t.tdz_f*t.tdz_f);
									t.tfactor_f=(t.toriginalScaleX_f/100.0)*2;
									if (  t.tfactor_f<1.0  )  t.tfactor_f = 1.0;
									if (  t.tfactor_f>5.0  )  t.tfactor_f = 5.0;
									t.tscale_f = ((t.tnewDistance_f-t.toriginalDistance_f)*t.tfactor_f*fEntityScalingRatio)+t.toriginalScaleX_f;
									t.tScaleY_f = (t.toriginalScaleY_f / t.toriginalScaleX_f) * t.tscale_f;
									t.tScaleZ_f = (t.toriginalScaleZ_f / t.toriginalScaleX_f) * t.tscale_f;
									if (  t.tscale_f < 5  )  t.tscale_f  =  5;
									if (  t.tscale_f > 10000  )  t.tscale_f  =  10000;
									if (  t.tScaleY_f < 5  )  t.tScaleY_f  =  5;
									if (  t.tScaleY_f > 10000  )  t.tScaleY_f  =  10000;
									if (  t.tScaleZ_f < 5  )  t.tScaleZ_f  =  5;
									if (  t.tScaleZ_f > 10000  )  t.tScaleZ_f  =  10000;
									ScaleObject (  t.widget.activeObject,t.tscale_f,t.tScaleY_f,t.tScaleZ_f );
								}

								//  transfer final scale to entity element setting
								t.entityelement[t.te].scalex = ObjectScaleX(t.widget.activeObject)-100.0;
								t.entityelement[t.te].scaley = ObjectScaleY(t.widget.activeObject)-100.0;
								t.entityelement[t.te].scalez = ObjectScaleZ(t.widget.activeObject)-100.0;
								// mark as static if it was
								if ( t.entityelement[t.te].staticflag == 1 ) g.projectmodifiedstatic = 1;

								// 271015 - if we need to also scale rubber band highlighted objects, do so now
								if ( g.entityrubberbandlist.size() > 0 )
								{
									float fMovedActiveObjectSX = ObjectScaleX ( t.widget.activeObject ) - fOldActiveObjectSX;
									float fMovedActiveObjectSY = ObjectScaleY ( t.widget.activeObject ) - fOldActiveObjectSY;
									float fMovedActiveObjectSZ = ObjectScaleZ ( t.widget.activeObject ) - fOldActiveObjectSZ;
									for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
									{
										int e = g.entityrubberbandlist[i].e;
										int tobj = t.entityelement[e].obj;
										if ( tobj > 0 )
										{
											if ( ObjectExist(tobj) == 1 )
											{
												if ( tobj != t.widget.activeObject )
												{
													ScaleObject ( tobj, ObjectScaleX(tobj)+fMovedActiveObjectSX, ObjectScaleY(tobj)+fMovedActiveObjectSY, ObjectScaleZ(tobj)+fMovedActiveObjectSZ );
													t.entityelement[e].scalex = ObjectScaleX(tobj) - 100;
													t.entityelement[e].scaley = ObjectScaleY(tobj) - 100;
													t.entityelement[e].scalez = ObjectScaleZ(tobj) - 100;
													// mark as static if it was
													if ( t.entityelement[e].staticflag == 1 ) g.projectmodifiedstatic = 1;
												}
											}
										}
									}
								}
							}
						}
					}
				}

				//  update widget objects
				for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
				{
					if (  ObjectExist (g.widgetobjectoffset+t.a) )  
					{
						PositionObject (  g.widgetobjectoffset+t.a, CameraPositionX(), CameraPositionY(), CameraPositionZ() );
						//  070415 - apply offset from actual click 3D position when selected widget object
						PointObject (  g.widgetobjectoffset+t.a,ObjectPositionX(t.widget.activeObject)+t.widget.offsetx,ObjectPositionY(t.widget.activeObject)+t.widget.offsety,ObjectPositionZ(t.widget.activeObject)+t.widget.offsetz );
						MoveObject (  g.widgetobjectoffset+t.a,40 );
						RotateObject (  g.widgetobjectoffset+t.a,0,0,0 );
					}
					RotateObject (  g.widgetobjectoffset+6, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
					RotateObject (  g.widgetobjectoffset+7, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
					RotateObject (  g.widgetobjectoffset+8, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
					RotateObject (  g.widgetobjectoffset+9, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
					RotateObject (  g.widgetobjectoffset+10, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
					RotateObject (  g.widgetobjectoffset+11, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
					RotateObject (  g.widgetobjectoffset+12, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
				}
				//  detect if widget panel off screen, and shift back in
				widget_correctwidgetpanel ( );

				//  user has let go of the mouse, reset section chosen
				if (  t.inputsys.mclick  ==  0 ) 
				{
					t.widget.pickedSection = 0;
					t.widget.grabbed = 0;
				}
			}

			//  update waypoint object when widget moves entity
			t.ttte=t.widget.pickedEntityIndex;
			if (  t.ttte>0 ) 
			{
				t.waypointindex=t.entityelement[t.ttte].eleprof.trigger.waypointzoneindex;
				if (  t.waypointindex>0 ) 
				{
					t.thisx_f=t.entityelement[t.ttte].x;
					t.thisy_f=t.entityelement[t.ttte].y;
					t.thisz_f=t.entityelement[t.ttte].z;
					waypoint_movetothiscoordinate ( );
				}
			}

	}
	t.widget.oldMouseClick = t.inputsys.mclick;

	//  restore camera range
	editor_refreshcamerarange ( );

	// 061115 - ensure the R key press is cancelled
	g.fForceYRotationOfRubberBandFromKeyPress = 0.0f;
}

void widget_correctwidgetpanel ( void )
{
	//  reverse widget when facing away from user
	if (  WrapValue(CameraAngleY())>180 || WrapValue(CameraAngleY())<1  )  t.txflip = 0; else t.txflip = 1;
	if (  t.txflip == 1 ) 
	{
		YRotateObject (  t.widget.widgetXObj,180 );
		YRotateObject (  t.widget.widgetXYObj,180 );
	}
	else
	{
		YRotateObject (  t.widget.widgetXObj,0 );
		YRotateObject (  t.widget.widgetXYObj,0 );
	}
	if (  WrapValue(CameraAngleY())>270 || WrapValue(CameraAngleY())<90 ) 
	{
		YRotateObject (  t.widget.widgetZObj,180 );
		if ( t.txflip == 1 ) 
			YRotateObject (  t.widget.widgetXZObj,90 );
		else
			YRotateObject (  t.widget.widgetXZObj,0 );
		YRotateObject (  t.widget.widgetYZObj,0 );
	}
	else
	{
		YRotateObject (  t.widget.widgetZObj,0 );
		if (  t.txflip == 1 ) 
		{
			YRotateObject (  t.widget.widgetXZObj,180 );
		}
		else
		{
			YRotateObject (  t.widget.widgetXZObj,270 );
		}
		YRotateObject (  t.widget.widgetYZObj,180 );
	}

	//  align panels better
	for ( t.a = t.widget.widgetPOSObj ; t.a<=  t.widget.widgetLCKObj; t.a++ )
	{
		SetObjectToCameraOrientation (  t.a );
		MoveObjectRight (  t.a,5 );
		MoveObject (  t.a,-5 );
	}

	//  default location is top right (needs proper maffs here!)
	t.tshiftscrx_f=0 ; t.tshiftscry_f=0;
	t.tareawidth_f=(GetDisplayWidth()-66);
	t.tareaheight_f=(GetDisplayHeight()-18);
	t.tmousemodifierx_f=(GetChildWindowWidth()+0.0)/(GetWindowWidth()+0.0);
	t.tmousemodifiery_f=(GetChildWindowHeight()+0.0)/(GetWindowHeight()+0.0);
	t.tmaxwidthhere_f=t.tareawidth_f*t.tmousemodifierx_f;
	t.tmaxheighthere_f=t.tareaheight_f*t.tmousemodifiery_f;
	//  move menu when near right and top
	t.tmenux_f=GetScreenX(t.widget.widgetPOSObj);
	t.tmenuy_f=GetScreenY(t.widget.widgetPOSObj);
	if (  t.tmenux_f>t.tmaxwidthhere_f*0.9 ) 
	{
		t.tshiftscrx_f=t.tshiftscrx_f-9;
	}
	if (  t.tmenuy_f<190.0*t.tmousemodifierx_f ) 
	{
		t.tshiftscry_f=t.tshiftscry_f-10;
	}
	//  apply overall shift when in lower left quadrant
	if (  t.tmenux_f<(t.tareawidth_f/2.0) ) 
	{
		t.tshiftscrx_f=t.tshiftscrx_f+(((t.tareawidth_f/2.0)-t.tmenux_f)*0.007);
	}
	if (  t.tmenuy_f>(t.tareaheight_f/2.0) ) 
	{
		t.tshiftscry_f=t.tshiftscry_f+((t.tmenuy_f-(t.tareaheight_f/2.0))*0.007);
	}

	//  position final widget panel resting place
	for ( t.a = t.widget.widgetPOSObj ; t.a<=  t.widget.widgetLCKObj; t.a++ )
	{
		MoveObjectRight (  t.a,t.tshiftscrx_f );
		MoveObjectUp (  t.a,t.tshiftscry_f );
	}

	//  if widget core not in screen, hide widget menu altogether
	if (  GetInScreen(t.widget.widgetXZObj) == 1 ) 
	{
		for ( t.a = t.widget.widgetPOSObj ; t.a<=  t.widget.widgetLCKObj; t.a++ )
		{
			ShowLimb (  t.a,0 );
		}
	}
	else
	{
		for ( t.a = t.widget.widgetPOSObj ; t.a<=  t.widget.widgetLCKObj; t.a++ )
		{
			HideLimb (  t.a,0 );
		}
	}

	// if camera BELOW vertical of widget, HIDE the XZ gadget as it messes up badly
	//if (  t.widget.mode  ==  0 ) 
	//{
	//	if ( CameraPositionY(0) < ObjectPositionY(t.widget.widgetXZObj) )
	//		HideObject ( t.widget.widgetXZObj );
	//	else
	//		ShowObject ( t.widget.widgetXZObj );
	//}
}

void widget_updatewidgetobject ( void )
{

	//  If no picked object and the widget has been in use, switch it off
	if (  t.widget.pickedObject == 0 ) 
	{
		if (  t.widget.activeObject>0 ) 
		{
			t.widget.pickedEntityIndex=0;
			t.widget.grabbed=0;
			t.widget.pickedSection=0;
			t.widget.mode=0;
			for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
			{
				if (  ObjectExist(g.widgetobjectoffset+t.a)  )  HideObject (  g.widgetobjectoffset+t.a );
			}
			t.widget.activeObject=0;
		}
	}
	else
	{
		if (  t.widget.pickedObject != t.widget.activeObject  )  t.widget.activeObject = 0;
	}

return;

}

void widget_check_for_new_object_selection ( void )
{
	//  check if an object has been chosen
	if (  t.widget.activeObject == 0 ) 
	{
		if (  t.widget.pickedObject != 0 ) 
		{
			t.widget.activeObject=t.widget.pickedObject;
			t.widget.pickedSection=0;
			t.widget.grabbed=0;
			t.widget.mode=0;
			widget_show_widget ( );
		}
	}
	t.widget.oldActiveObject=t.widget.activeObject;
return;

}

void widget_show_widget ( void )
{

	//  Show the widget in all its glory
	if (  t.widget.activeObject == 0  )  return;
	if (  ObjectExist(t.widget.activeObject) == 0  )  return;

	//  what is shown varies on which mode is picked (translate, rotate or scale)
	t.oldry_f = ObjectAngleY(t.widget.activeObject);
	for ( t.a = 0 ; t.a<=  t.widget.widgetMAXObj; t.a++ )
	{
		if (  ObjectExist(g.widgetobjectoffset+t.a)  )  HideObject(g.widgetobjectoffset+t.a);
	}

	// detect if selected in an EBE entity
	bool bIsEBEWidget = false;
	t.ttte=t.widget.pickedEntityIndex;
	if (  t.ttte>0 ) 
	{
		int iEntID = t.entityelement[t.ttte].bankindex;
		if ( iEntID > 0 ) 
			if ( t.entityprofile[iEntID].isebe != 0 )
				bIsEBEWidget = true;
	}

	// normally scale and property (but EBE can change this texture)
	TextureObject ( t.widget.widgetSCLObj, 0, t.widget.imagestart+3 );
	TextureObject ( t.widget.widgetPRPObj, 0, t.widget.imagestart+4 );

	// rubber band or selected parent
	bool bRealRubberBand = false;
	if ( g.entityrubberbandlist.size() > 0 )
	{
		bRealRubberBand = true;
		if ( t.widget.pickedEntityIndex > 0 )
		{
			bool bPartOfParentChildGroup = false;
			editor_rec_checkifindexinparentchain ( t.widget.pickedEntityIndex, &bPartOfParentChildGroup );
			if ( bPartOfParentChildGroup == true )
				bRealRubberBand = false;
		}
	}

	// show all or just POS
	if ( bRealRubberBand == true )
	{
		// Rubber band select POS, DELETE and LOCK only
		t.a = t.widget.widgetPOSObj; if ( ObjectExist(t.a) == 1  ) ShowObject ( t.a );
		t.a = t.widget.widgetROTObj; if ( ObjectExist(t.a) == 1  ) ShowObject ( t.a );
		t.a = t.widget.widgetSCLObj; if ( ObjectExist(t.a) == 1  ) ShowObject ( t.a );
		t.a = t.widget.widgetDELObj; if ( ObjectExist(t.a) == 1  ) ShowObject ( t.a );
		t.a = t.widget.widgetLCKObj; if ( ObjectExist(t.a) == 1  ) ShowObject ( t.a );

		// move POS button alongside DEL and EXT
		OffsetLimb ( t.widget.widgetPOSObj,0,-1.2f,3.0f+(t.tmovestep_f*3),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPOSObj,0,0,0,0 );
		OffsetLimb ( t.widget.widgetROTObj,0,-1.2f,3.0f+(t.tmovestep_f*2),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPOSObj,0,0,0,0 );
		OffsetLimb ( t.widget.widgetSCLObj,0,-1.2f,3.0f+(t.tmovestep_f*1),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPOSObj,0,0,0,0 );
	}
	else
	{
		// POS, ROT, SCALE, etc
		for ( t.a = t.widget.widgetPOSObj ; t.a<=  t.widget.widgetLCKObj; t.a++ )
		{
			if (  ObjectExist(t.a) == 1  )  ShowObject (  t.a );
		}

		// hide if EBE widget
		if ( bIsEBEWidget == true ) 
		{
			OffsetLimb ( t.widget.widgetPOSObj,0,-1.2f,3.0f+(t.tmovestep_f*5),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPOSObj,0,0,0,0 );
			OffsetLimb ( t.widget.widgetROTObj,0,-1.2f,3.0f+(t.tmovestep_f*4),t.tmovezup_f  ); RotateLimb (  t.widget.widgetROTObj,0,0,0,0 );
			OffsetLimb ( t.widget.widgetSCLObj,0,-1.2f,3.0f+(t.tmovestep_f*3),t.tmovezup_f  ); RotateLimb (  t.widget.widgetSCLObj,0,0,0,0 );
			//OffsetLimb ( t.widget.widgetPOSObj,0,-1.2f,3.0f+(t.tmovestep_f*4),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPOSObj,0,0,0,0 );
			//OffsetLimb ( t.widget.widgetROTObj,0,-1.2f,3.0f+(t.tmovestep_f*3),t.tmovezup_f  ); RotateLimb (  t.widget.widgetROTObj,0,0,0,0 );
			TextureObject ( t.widget.widgetSCLObj, 0, t.widget.imagestart+8 ); // EDIT
			TextureObject ( t.widget.widgetPRPObj, 0, t.widget.imagestart+9 ); // SAVE
		}
		else
		{
			// correct POS position
			OffsetLimb ( t.widget.widgetPOSObj,0,-1.2f,3.0f+(t.tmovestep_f*5),t.tmovezup_f  ); RotateLimb (  t.widget.widgetPOSObj,0,0,0,0 );
			OffsetLimb ( t.widget.widgetROTObj,0,-1.2f,3.0f+(t.tmovestep_f*4),t.tmovezup_f  ); RotateLimb (  t.widget.widgetROTObj,0,0,0,0 );
			OffsetLimb ( t.widget.widgetSCLObj,0,-1.2f,3.0f+(t.tmovestep_f*3),t.tmovezup_f  ); RotateLimb (  t.widget.widgetSCLObj,0,0,0,0 );
		}
	}

	if (  t.widget.mode  ==  0 ) 
	{
		for ( t.a = 0 ; t.a<=  5; t.a++ )
		{
			if (  ObjectExist(g.widgetobjectoffset+t.a) == 1 ) 
			{
				ShowObject (  g.widgetobjectoffset+t.a );
				PositionObject (  g.widgetobjectoffset+t.a, CameraPositionX(), CameraPositionY(), CameraPositionZ() );
				PointObject (  g.widgetobjectoffset+t.a,ObjectPositionX(t.widget.activeObject)+t.widget.offsetx,ObjectPositionY(t.widget.activeObject)+t.widget.offsety,ObjectPositionZ(t.widget.activeObject)+t.widget.offsetz );
				MoveObject (  g.widgetobjectoffset+t.a,40 );
				RotateObject (  g.widgetobjectoffset+t.a,0,0,0 );
			}
		}
	}

	if (  t.widget.mode  ==  1 ) 
	{
		for ( t.a = 6 ; t.a<=  8; t.a++ )
		{
			if (  ObjectExist(g.widgetobjectoffset+t.a) == 1 ) 
			{
				ShowObject (  g.widgetobjectoffset+t.a );
				PositionObject (  g.widgetobjectoffset+t.a, CameraPositionX(), CameraPositionY(), CameraPositionZ() );
				PointObject (  g.widgetobjectoffset+t.a,ObjectPositionX(t.widget.activeObject)+t.widget.offsetx,ObjectPositionY(t.widget.activeObject)+t.widget.offsety,ObjectPositionZ(t.widget.activeObject)+t.widget.offsetz );
				MoveObject (  g.widgetobjectoffset+t.a,40 );
			}
		}
		if ( ObjectExist(g.widgetobjectoffset+6) == 1 ) 
		{
			ScaleObject (  g.widgetobjectoffset+6,50,100,100 );
			ScaleObject (  g.widgetobjectoffset+7,100,50,100 );
			ScaleObject (  g.widgetobjectoffset+8,100,100,50 );
			RotateObject (  g.widgetobjectoffset+6, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
			RotateObject (  g.widgetobjectoffset+7, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
			RotateObject (  g.widgetobjectoffset+8, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
		}
		if ( g.entityrubberbandlist.size() > 0 || bIsEBEWidget == true )
		{
			// hide X and Z adjustment (for now)
			HideObject ( g.widgetobjectoffset + 6 );
			RotateObject (  g.widgetobjectoffset+7, 0, ObjectAngleY(t.widget.activeObject), 0 );
			HideObject ( g.widgetobjectoffset + 8 );
		}
	}

	if (  t.widget.mode  ==  2 ) 
	{
		for ( t.a = 9 ; t.a<=  12; t.a++ )
		{
			if (  ObjectExist(g.widgetobjectoffset+t.a) == 1 ) 
			{
				ShowObject (  g.widgetobjectoffset+t.a );
				PositionObject (  g.widgetobjectoffset+t.a, CameraPositionX(), CameraPositionY(), CameraPositionZ() );
				PointObject (  g.widgetobjectoffset+t.a,ObjectPositionX(t.widget.activeObject)+t.widget.offsetx,ObjectPositionY(t.widget.activeObject)+t.widget.offsety,ObjectPositionZ(t.widget.activeObject)+t.widget.offsetz );
				MoveObject (  g.widgetobjectoffset+t.a,40 );
				RotateObject (  g.widgetobjectoffset+t.a,0,0,0 );
			}
		}
		if (  ObjectExist(g.widgetobjectoffset+9) == 1 ) 
		{
			RotateObject (  g.widgetobjectoffset+9, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
			RotateObject (  g.widgetobjectoffset+10, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
			RotateObject (  g.widgetobjectoffset+11, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
			RotateObject (  g.widgetobjectoffset+12, ObjectAngleX(t.widget.activeObject),ObjectAngleY(t.widget.activeObject), ObjectAngleZ(t.widget.activeObject) );
		}
	}

	//  hide any buttons and widgets if entity is a 'waypoint zone type'
	t.ttte=t.widget.pickedEntityIndex;
	if (  t.ttte>0 ) 
	{
		t.tttwi=t.entityelement[t.ttte].eleprof.trigger.waypointzoneindex;
		if (  t.tttwi>0 ) 
		{
			//  hide top three buttons
			if (  ObjectExist(t.widget.widgetPOSObj) == 1 ) 
			{
				HideObject (  t.widget.widgetPOSObj );
				HideObject (  t.widget.widgetROTObj );
				HideObject (  t.widget.widgetSCLObj );
				//  hide widget gadgets
				for ( t.a = 6 ; t.a<=  12; t.a++ )
				{
					HideObject (  g.widgetobjectoffset+t.a );
				}
			}
		}
	}
}

void widget_switchoff ( void )
{
	t.widget.pickedSection=0;
	t.widget.activeObject=0;
	t.widget.pickedObject=0;
	t.widget.mode=0;
	widget_hide ( );
}

void widget_hide ( void )
{
	for ( t.a = 0 ; t.a<=  40; t.a++ )
	{
		if (  ObjectExist (g.widgetobjectoffset+t.a)  )  HideObject (  g.widgetobjectoffset+t.a );
	}
	t.widget.mode=0;
}

void widget_free ( void )
{
	//  clean up widget files (will need to call widget_init again before using the widget)
	for ( t.a = 0 ; t.a<=  40; t.a++ )
	{
		if (  ObjectExist (g.widgetobjectoffset+t.a)  )  DeleteObject (  g.widgetobjectoffset+t.a ) ;
	}
	for ( t.a = 1 ; t.a<=  12; t.a++ )
	{
		t.r=DeleteVector3(g.widgetvectorindex+t.a);
	}
	for ( t.a = 1 ; t.a<=  5; t.a++ )
	{
		if (  MatrixExist(g.widgetStartMatrix+t.a)  )  DeleteMatrix (  g.widgetStartMatrix+t.a );
	}
	t.widget.mode=0;
return;
}
