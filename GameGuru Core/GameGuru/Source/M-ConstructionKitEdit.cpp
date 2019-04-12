//----------------------------------------------------
//--- GAMEGURU - M-ConstructionKitEdit
//----------------------------------------------------

#include "gameguru.h"

// 
//  CONSTRUCTION KIT EDIT MODULE - First Person 3D Editing
// 


//Subroutines

//Dave - pass on status of f9 mode to the occluder
extern bool g_occluderf9Mode;
extern bool RayCollisionDoBoxCheckFirst;

void conkitedit_init ( void )
{

	//  non yet

return;

}

void conkitedit_free ( void )
{


	//  reset highlighter on exit
	conkitedit_resetentityhighlighted ( );

	//  reset entity edit mode on exit
	t.conkit.entityeditmode=0;
	t.terrain.entityeditmodecursorR=0.0;
	t.terrain.entityeditmodecursorG=0.0;

	characterkit_makeF9Cursor ( );
	t.characterkitcontrol.oldF9CursorEntid = 0;

return;

}

void conkitedit_updateeditmodecursorcolor ( void )
{
	if (  t.conkit.entityeditmode == 1 ) 
	{
		t.terrain.entityeditmodecursorR=0.0 ; t.terrain.entityeditmodecursorG=0.0;
	}
	else
	{
		t.terrain.entityeditmodecursorR=1.0 ; t.terrain.entityeditmodecursorG=1.0;
	}
return;

}

void conkitedit_resetentityhighlighted ( void )
{
	if (  t.conkit.edit.entityhighlighted>0 ) 
	{
		t.tobj=t.entityelement[t.conkit.edit.entityhighlighted].obj;
		if (  t.tobj>0 ) 
		{
			if (  ObjectExist(t.tobj) == 1 ) 
			{
				SetAlphaMappingOn (  t.tobj,100 );
			}
		}
		t.conkit.edit.entityhighlighted=0;
	}
}

void conkitedit_loop ( void )
{
	// called every cycle
	if (  t.game.runasmultiplayer  ==  1  )  t.conkit.editmodeactive = 0;
	
	// let the occluder know the status of f9 mode
	if (  t.conkit.editmodeactive == 1 ) 
	{
		HideOrShowLUASprites ( true );
		g_occluderf9Mode = true;

		// let the occluder thread know it is okay to begin
		if ( g_hOccluderBegin ) SetEvent ( g_hOccluderBegin );

		//Ensure characters are switched to use the normal shader rather than the static one
		int tobj;
		for ( int c = 0 ; c < (int)t.characterBasicEntityList.size() ; c++ )
		{		
			tobj = t.entityelement[t.characterBasicEntityList[c]].obj;
			if ( tobj > 0 && t.entityelement[t.characterBasicEntityList[c]].active == 1 )
			{
				if ( ObjectExist ( tobj ) == 1 )
				{

					if ( !t.characterBasicEntityListIsSetToCharacter[c] )
					{
						SetObjectEffect( tobj , t.characterBasicShaderID );
						t.characterBasicEntityListIsSetToCharacter[c] = true;						
					}
				}
			}
		}
		if ( t.haveSetupShaderSwitching )
		{
			t.haveSetupShaderSwitching = false;
			game_setup_character_shader_entities(false);
		}
	}
	else
		g_occluderf9Mode = false;

	if (  t.conkit.editmodeactive == 1 ) 
	{		
		//  character creator stuff
		characterkit_checkForCharacters ( );
		characterkit_updateAllCharacterCreatorEntitiesInMap ( );
		characterkit_makeF9Cursor ( );

		//  Machine Independent Speed for EDITING
		t.ts_f=(Timer()-t.tsl_f)/50.0 ; t.tsl_f=Timer();

		//  Get input keys and clear k$
		input_getdirectcontrols ( );
		t.inputsys.k_s="";

		// Suppliment keys via universal KeyState ( command )
		if (  KeyState(g.keymap[2]) == 1 || t.conkit.forceaction == 2  )  t.inputsys.k_s = "1";
		if (  KeyState(g.keymap[3]) == 1 || t.conkit.forceaction == 3  )  t.inputsys.k_s = "2";
		if (  KeyState(g.keymap[4]) == 1 || t.conkit.forceaction == 4  )  t.inputsys.k_s = "3";
		if (  KeyState(g.keymap[5]) == 1 || t.conkit.forceaction == 5  )  t.inputsys.k_s = "4";
		if (  KeyState(g.keymap[6]) == 1 || t.conkit.forceaction == 6  )  t.inputsys.k_s = "5";
		if (  KeyState(g.keymap[7]) == 1 || t.conkit.forceaction == 7  )  t.inputsys.k_s = "6";
		if (  KeyState(g.keymap[8]) == 1 || t.conkit.forceaction == 8  )  t.inputsys.k_s = "7";
		if (  KeyState(g.keymap[9]) == 1 || t.conkit.forceaction == 9  )  t.inputsys.k_s = "8";
		if (  KeyState(g.keymap[10]) == 1 || t.conkit.forceaction == 10  )  t.inputsys.k_s = "9";
		if (  KeyState(g.keymap[11]) == 1 || t.conkit.forceaction == 11  )  t.inputsys.k_s = "0";
		if (  KeyState(g.keymap[12]) == 1  )  t.inputsys.k_s = "-";
		if (  KeyState(g.keymap[74]) == 1  )  t.inputsys.k_s = "-";
		if (  KeyState(g.keymap[13]) == 1  )  t.inputsys.k_s = "=";
		if (  KeyState(g.keymap[78]) == 1  )  t.inputsys.k_s = "=";
		if (  KeyState(g.keymap[26]) == 1  )  t.inputsys.k_s = "[";
		if (  KeyState(g.keymap[27]) == 1  )  t.inputsys.k_s = "]";
		if (  KeyState(g.keymap[38]) == 1  )  t.inputsys.k_s = "l";
		if (  KeyState(g.keymap[34]) == 1  )  t.inputsys.k_s = "g";
		if (  KeyState(g.keymap[50]) == 1  )  t.inputsys.k_s = "m";
		if (  KeyState(g.keymap[18]) == 1  )  t.inputsys.k_s = "e";
		if (  KeyState(g.keymap[19]) == 1  )  t.inputsys.k_s = "r";
		if (  KeyState(g.keymap[20]) == 1  )  t.inputsys.k_s = "t";
		if (  KeyState(g.keymap[51]) == 1  )  t.inputsys.k_s = ",";
		if (  KeyState(g.keymap[52]) == 1  )  t.inputsys.k_s = ".";
		if (  KeyState(g.keymap[21]) == 1  )  t.inputsys.k_s = "y";

		//  entity edit mode
		if (  t.inputsys.k_s == "e" || t.inputsys.k_s == "t" || t.inputsys.k_s == "," || t.inputsys.k_s == "." || t.inputsys.k_s == "y" || t.conkit.edit.entitytakenfromlevel == 1 ) 
		{
			if (  t.conkit.edit.entityeditmodepress == 0 ) 
			{
				t.conkit.edit.entityeditmodepress=1;
				if (  t.inputsys.k_s == "e" ) 
				{
					t.tokay=0;
					if (  g.entidmaster>0 ) 
					{
						for ( t.tentid = 1 ; t.tentid<=  g.entidmaster; t.tentid++ )
						{
							if (  t.entityprofile[t.tentid].ismarker == 0 && t.entitybank_s[t.tentid] != ""  )  t.tokay = t.tentid;
						}
					}
					if (  t.tokay>0 ) 
					{
						t.conkit.entityeditmode=1;
						t.conkit.edit.entityindex=t.tokay;
						t.conkit.edit.scalex=100;
						t.conkit.edit.scaley=100;
						t.conkit.edit.scalez=100;
						t.conkit.edit.entityheightadjust=0;
						t.conkit.edit.entityeditgrabbed=0;
					}
					else
					{
						t.visuals.generalpromptstatetimer=Timer()+3000;
						t.visuals.generalprompt_s="You must first add some entities via the main editor";
						t.visuals.generalpromptalignment=1;
					}
				}
				if (  t.inputsys.k_s == "t" ) 
				{
					if (  ObjectExist(t.conkit.objectstartnumber+1) == 1  )  DeleteObject (  t.conkit.objectstartnumber+1 );
					t.conkit.edit.entityeditgrabbed=0;
					conkitedit_resetentityhighlighted ( );
					t.conkit.entityeditmode=0;
				}
				if (  t.inputsys.k_s == "," || t.inputsys.k_s == "." ) 
				{
					//  scroll back/fore an item
					if (  t.conkit.edit.entityeditgrabbed == 0 ) 
					{
						t.conkit.edit.entityeditgrabbed=1;
					}
					else
					{
						t.tcount=1+t.conkit.edit.entityindexmax;
						do
						{
							if (  t.inputsys.k_s == "." ) 
							{
								t.conkit.edit.entityindex=t.conkit.edit.entityindex+1;
								if (  t.conkit.edit.entityindex>t.conkit.edit.entityindexmax  )  t.conkit.edit.entityindex = 1;
							}
							else
							{
								t.conkit.edit.entityindex=t.conkit.edit.entityindex-1;
								if (  t.conkit.edit.entityindex<1  )  t.conkit.edit.entityindex = t.conkit.edit.entityindexmax;
							}
							--t.tcount;
							t.tentid=t.conkit.edit.entityindex;
						} while ( !(  (t.entityprofile[t.tentid].ismarker == 0 && t.entitybank_s[t.tentid] != "") || t.tcount == 0 ) );
					}
					t.conkit.entityeditmode=1;
					if (  t.conkit.edit.entityeditgrabbed == 1 ) 
					{
						t.tentid=t.conkit.edit.entityindex;
						if (  t.entityprofile[t.tentid].ismarker != 0 || t.entitybank_s[t.tentid] == "" ) 
						{
							t.conkit.edit.entityeditgrabbed=0;
						}
						else
						{
							//  acquire default eleprof settings from parent object
							t.entid=t.conkit.edit.entityindex;
							entity_fillgrideleproffromprofile ( );
							if (  t.entityprofile[t.entid].scale>0 ) 
							{
								t.conkit.edit.scalex=t.entityprofile[t.entid].scale;
								t.conkit.edit.scaley=t.entityprofile[t.entid].scale;
								t.conkit.edit.scalez=t.entityprofile[t.entid].scale;
							}
							else
							{
								t.conkit.edit.scalex=100;
								t.conkit.edit.scaley=100;
								t.conkit.edit.scalez=100;
							}
							t.conkit.edit.rotx = 0;
							t.conkit.edit.roty = 0;
							t.conkit.edit.rotz = 0;
							t.conkit.edit.posoffx = 0;
							t.conkit.edit.posoffz = 0;
							t.conkit.edit.posoffy = BT_GetGroundHeight(t.terrain.TerrainID,t.inputsys.localx_f,t.inputsys.localy_f);
							t.conkit.edit.entityheightadjust=0;
							t.conkit.edit.entityeditstaticmode=t.entityprofile[t.entid].defaultstatic;
							conkitedit_updateeditmodecursorcolor ( );
							t.conkit.edit.entityeditposoffground=0;
						}
					}
				}
				if (  t.conkit.edit.entitytakenfromlevel == 1 ) 
				{
					//  delete highlighted one as we extract it from level
					if (  t.conkit.edit.entityhighlighted>0 ) 
					{
						t.tentitytoselect=t.conkit.edit.entityhighlighted;
						t.entid=t.entityelement[t.tentitytoselect].bankindex;
						if (  t.entityelement[t.tentitytoselect].usingphysicsnow == 1 ) 
						{
							t.tphyobj=t.entityelement[t.tentitytoselect].obj;
							physics_disableobject ( );
							t.entityelement[t.tentitytoselect].usingphysicsnow=0;
						}
						t.entityelement[t.tentitytoselect].editorlock=0;
						t.tEntityAngle_f = ObjectAngleY(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.scalex = ObjectScaleX(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.scaley = ObjectScaleY(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.scalez = ObjectScaleZ(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.rotx = ObjectAngleX(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.roty = ObjectAngleY(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.rotz = ObjectAngleZ(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.posoffx = ObjectPositionX(t.entityelement[t.tentitytoselect].obj)-t.inputsys.localx_f;
						t.conkit.edit.posoffy = ObjectPositionY(t.entityelement[t.tentitytoselect].obj);
						t.conkit.edit.posoffz = ObjectPositionZ(t.entityelement[t.tentitytoselect].obj)-t.inputsys.localy_f;
						//  230315 - ensure any AI is also removed as ingame
						for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
						{
							if (  t.tentitytoselect == t.charanimstates[g.charanimindex].e ) 
							{
								t.charanimstates[g.charanimindex].e=0 ; t.charanimstates[g.charanimindex].obj=0;
							}
						}
						//  remove any attachments too
						if (  t.entityelement[t.tentitytoselect].attachmentobj>0 ) 
						{
							HideObject (  t.entityelement[t.tentitytoselect].attachmentobj );
							t.entityelement[t.tentitytoselect].attachmentobj=0;
						}
						entity_deleteentityfrommap ( );
						g.projectmodified=1 ; gridedit_changemodifiedflag ( );
						g.projectmodifiedstatic = 0; if ( t.entityelement[t.tentitytoselect].staticflag == 1 ) g.projectmodifiedstatic = 1;
						conkitedit_resetentityhighlighted ( );
					}
					t.conkit.edit.entitytakenfromlevel=0;
					t.conkit.edit.entityeditgrabbed=1;
					t.conkit.entityeditmode=1;
					t.conkit.edit.entityeditposoffground=1;
				}
				if (  t.inputsys.k_s == "y" ) 
				{
					//  toggle static and dynamic mode for entity edit mode placement
					t.conkit.edit.entityeditstaticmode=1-t.conkit.edit.entityeditstaticmode;
				}
				else
				{
					//  all other keys recreate cursor object
					if (  t.conkit.entityeditmode == 1 ) 
					{
						if (  ObjectExist(t.conkit.objectstartnumber+1) == 1  )  DeleteObject (  t.conkit.objectstartnumber+1 );
						if (  t.conkit.edit.entityeditgrabbed == 1 ) 
						{
							t.obj=t.conkit.objectstartnumber+1;
							t.tupdatee=-1 ; t.tentid=t.conkit.edit.entityindex;
							entity_createobj ( );
							t.conkit.edit.entityheightadjust=0;
							ScaleObject (  t.obj,t.conkit.edit.scalex,t.conkit.edit.scaley,t.conkit.edit.scalez );
							PositionObject (  t.obj,t.conkit.edit.posoffx+t.inputsys.localx_f,t.conkit.edit.posoffy,t.conkit.edit.posoffz+t.inputsys.localy_f );
							RotateObject (  t.obj,t.conkit.edit.rotx,t.conkit.edit.roty,t.conkit.edit.rotz );
						}
					}
					else
					{
						if (  ObjectExist(t.conkit.objectstartnumber+1) == 1  )  DeleteObject (  t.conkit.objectstartnumber+1 );
					}
				}
				conkitedit_updateeditmodecursorcolor ( );
			}
		}
		else
		{
			t.conkit.edit.entityeditmodepress=0;
		}
		if (  t.conkit.entityeditmode == 1 ) 
		{
			//  position work entity around
			if (  t.conkit.edit.entityeditgrabbed == 1 ) 
			{
				if (  ObjectExist(t.conkit.objectstartnumber+1) == 1 ) 
				{
					t.obj=t.conkit.objectstartnumber+1;
					if (  t.conkit.edit.entityeditposoffground == 1 ) 
					{
						t.conkit.edit.posoffy=t.conkit.edit.posoffy+t.conkit.edit.entityheightadjust;
						t.conkit.edit.entityheightadjust=0;
						PositionObject (  t.obj,t.conkit.edit.posoffx+t.inputsys.localx_f,t.conkit.edit.posoffy,t.conkit.edit.posoffz+t.inputsys.localy_f );
					}
					else
					{
						t.tentitydefaultheight_f=t.entityprofile[t.conkit.edit.entityindex].defaultheight;
						PositionObject (  t.obj,t.inputsys.localx_f,BT_GetGroundHeight(t.terrain.TerrainID,t.inputsys.localx_f,t.inputsys.localy_f)+t.conkit.edit.entityheightadjust+t.tentitydefaultheight_f,t.inputsys.localy_f );
					}
					//  set the entity static or dynamic color
					if (  t.conkit.edit.entityeditstaticmode == 1 ) 
					{
						SetAlphaMappingOn (  t.obj,101 );
					}
					else
					{
						SetAlphaMappingOn (  t.obj,103 );
					}
				}
			}
			//  detect when highlighting an item
			if ( t.conkit.edit.entityeditgrabbed == 0 && Timer() - t.conkit.edit.timeDelay > 250 && ObjectExist (t.conkit.objectstartnumber+0) == 1 ) 
			{
				t.founde=0;
				t.trayx1_f=CameraPositionX(t.terrain.gameplaycamera);
				t.trayy1_f=CameraPositionY(t.terrain.gameplaycamera);
				t.trayz1_f=CameraPositionZ(t.terrain.gameplaycamera);
				PositionObject (  t.conkit.objectstartnumber+0,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				SetObjectToCameraOrientation (  t.conkit.objectstartnumber+0 );
				MoveObject (  t.conkit.objectstartnumber+0,1000 );
				t.tClosestDist_f=99999;
				//Let the ray check know we want to check box collision first
				RayCollisionDoBoxCheckFirst = true;
				for ( t.e = 1 ; t.e<=  g.entityelementmax; t.e++ )
				{
					t.obj=t.entityelement[t.e].obj;
					if (  t.obj>0 ) 
					{
						t.entid = t.entityelement[t.e].bankindex;
						if ( t.entid>0 ) 
						{
							if ( t.entityprofile[t.entid].isebe == 0 )
							{
								if (  t.entityelement[t.e].editorlock == 0 || (t.entityelement[t.e].editorlock == 1 && t.inputsys.keyspace == 1) ) // 010416 - && t.inputsys.keyshift == 1) ) 
								{
									if (  ObjectExist(t.obj) == 1 ) 
									{
										t.trayx2_f=ObjectPositionX(t.conkit.objectstartnumber+0);
										t.trayy2_f=ObjectPositionY(t.conkit.objectstartnumber+0);
										t.trayz2_f=ObjectPositionZ(t.conkit.objectstartnumber+0);
										t.tdist_f=IntersectObject(t.obj,t.trayx1_f,t.trayy1_f,t.trayz1_f,t.trayx2_f,t.trayy2_f,t.trayz2_f);
										if (  t.tdist_f>0 && t.tdist_f<t.tClosestDist_f ) 
										{
											//  ray intersected this object, so select it
											t.founde=t.e ; t.tClosestDist_f=t.tdist_f;
										}
									}
								}
							}
						}
					}
				}
				RayCollisionDoBoxCheckFirst = false;
				if ( t.conkit.edit.entityhighlighted != t.founde ) 
				{
					conkitedit_resetentityhighlighted ( );
				}
				if ( t.founde>0 ) 
				{
					t.obj=t.entityelement[t.founde].obj;
					if (  t.obj>0 ) 
					{
						if (  ObjectExist(t.obj) == 1 ) 
						{
							if (  t.entityelement[t.founde].staticflag == 1 ) 
							{
								SetAlphaMappingOn (  t.obj,101 );
							}
							else
							{
								SetAlphaMappingOn (  t.obj,103 );
							}
						}
					}
				}
				t.conkit.edit.entityhighlighted=t.founde;
			}
			//  control left/right click
			if ( t.inputsys.mclick == 0  )  t.conkit.edit.entityeditaddpress = 0;
			if ( t.inputsys.mclick == 1 ) 
			{
				if (  t.conkit.edit.entityeditaddpress == 0 ) 
				{
					t.conkit.edit.entityeditaddpress=1;
					if (  t.conkit.edit.entityeditgrabbed == 1 ) 
					{
						//  add this entity to scene
						if (  ObjectExist(t.conkit.objectstartnumber+1) == 1 ) 
						{
							t.gridentity=t.conkit.edit.entityindex;
							t.gridentityeditorfixed=0;
							t.gridentitystaticmode=t.conkit.edit.entityeditstaticmode;
							t.gridentityposy_f=ObjectPositionY(t.conkit.objectstartnumber+1);
							t.gridentityposx_f=ObjectPositionX(t.conkit.objectstartnumber+1);
							t.gridentityposz_f=ObjectPositionZ(t.conkit.objectstartnumber+1);
							t.gridentityrotatex_f=ObjectAngleX(t.conkit.objectstartnumber+1);
							t.gridentityrotatey_f=ObjectAngleY(t.conkit.objectstartnumber+1);
							t.gridentityrotatez_f=ObjectAngleZ(t.conkit.objectstartnumber+1);
							//  scalexyz are full OBJ SCALES (100=full)
							t.gridentityscalex_f=t.conkit.edit.scalex;
							t.gridentityscaley_f=t.conkit.edit.scaley;
							t.gridentityscalez_f=t.conkit.edit.scalez;
							t.entid=t.conkit.edit.entityindex;
							entity_addentitytomap ( );
							g.projectmodified=1 ; gridedit_changemodifiedflag ( );
							g.projectmodifiedstatic = 0; if ( t.gridentitystaticmode == 1 ) g.projectmodifiedstatic = 1;
							//  must add to permanant stored entity array (so does not delete on exit)
							if (  t.tupdatee>t.storedentityelementlist ) 
							{
								t.storedentityelementlist=g.entityelementlist;
								Dim (  t.storedentityelement,t.storedentityelementlist );
							}
							t.storedentityelement[t.tupdatee]=t.entityelement[t.tupdatee];
							//  add physics
							t.e=t.tupdatee  ; physics_prepareentityforphysics ( );
							//  ensure when leave F9 mode, entity is activated for game use
							t.entityelement[t.tupdatee].active=2;
						}
					}
					else
					{
						//  select entity if highlighting on
						if (  t.conkit.edit.entityhighlighted>0 ) 
						{
							//  acquire eleprof settings from modified data in entityelement
							t.founde=t.conkit.edit.entityhighlighted;
							t.grideleprof=t.entityelement[t.founde].eleprof;
							t.conkit.edit.entityindex=t.entityelement[t.founde].bankindex;
							t.conkit.edit.entitytakenfromlevel=1;
							t.conkit.edit.entityeditmodepress=0;
							t.conkit.edit.entityeditgrabbed=0;
							t.conkit.edit.entityeditstaticmode=t.entityelement[t.founde].staticflag;
							conkitedit_updateeditmodecursorcolor ( );
						}
					}
				}
			}
			if (  t.inputsys.mclick == 2 ) 
			{
				//  delete entity from cursor
				if (  t.conkit.edit.entityeditgrabbed == 1 ) 
				{
					if (  ObjectExist(t.conkit.objectstartnumber+1) == 1  )  DeleteObject (  t.conkit.objectstartnumber+1 );
					t.conkit.edit.entityeditgrabbed=0;
				}
			}
		}

		//  entity rotation and height control
		if (  t.conkit.entityeditmode == 1 && t.conkit.edit.entityeditgrabbed == 1 ) 
		{
			//  rotation
			t.trotnowx_f=0.0 ; t.trotnowy_f=0.0 ; t.trotnowz_f=0.0 ; t.trotnow=0;
			if (  t.inputsys.k_s == "r" ) {  t.trotnowy_f = 1  ; t.trotnow = 1 ; t.gridentityrotateaxis = 1; }
			if (  t.inputsys.k_s == "1" ) {  t.trotnowx_f = 1  ; t.trotnow = 1 ; t.gridentityrotateaxis = 0; }
			if (  t.inputsys.k_s == "2" ) {  t.trotnowx_f = -1  ; t.trotnow = 1 ; t.gridentityrotateaxis = 0; }
			if (  t.inputsys.k_s == "3" ) {  t.trotnowy_f = 1  ; t.trotnow = 1 ; t.gridentityrotateaxis = 1; }
			if (  t.inputsys.k_s == "4" ) {  t.trotnowy_f = -1  ; t.trotnow = 1 ; t.gridentityrotateaxis = 1; }
			if (  t.inputsys.k_s == "5" ) {  t.trotnowz_f = 1  ; t.trotnow = 1 ; t.gridentityrotateaxis = 2; }
			if (  t.inputsys.k_s == "6" ) {  t.trotnowz_f = -1  ; t.trotnow = 1 ; t.gridentityrotateaxis = 2; }
			if (  t.trotnow == 1 ) 
			{
				t.trotnowx_f=t.trotnowx_f*g.timeelapsed_f;
				t.trotnowy_f=t.trotnowy_f*g.timeelapsed_f;
				t.trotnowz_f=t.trotnowz_f*g.timeelapsed_f;
				if (  t.inputsys.keyshift == 1 ) 
				{
					t.trotnowx_f=t.trotnowx_f*10;
					t.trotnowy_f=t.trotnowy_f*10;
					t.trotnowz_f=t.trotnowz_f*10;
				}
				else
				{
					if (  t.inputsys.keycontrol == 1 ) 
					{
						t.trotnowx_f=t.trotnowx_f*0.1;
						t.trotnowy_f=t.trotnowy_f*0.1;
						t.trotnowz_f=t.trotnowz_f*0.1;
					}
				}
				if (  ObjectExist(t.conkit.objectstartnumber+1) == 1 ) 
				{
					RotateObject (  t.conkit.objectstartnumber+1,ObjectAngleX(t.conkit.objectstartnumber+1)+t.trotnowx_f,ObjectAngleY(t.conkit.objectstartnumber+1)+t.trotnowy_f,ObjectAngleZ(t.conkit.objectstartnumber+1)+t.trotnowz_f );
				}
			}
			if (  t.inputsys.keyshift == 0 ) 
			{
				if (  t.inputsys.k_s == "0" ) 
				{
					if (  t.gridentityrotateaxis == 0  )  RotateObject (  t.conkit.objectstartnumber+1,0,ObjectAngleY(t.conkit.objectstartnumber+1)+t.trotnowy_f,ObjectAngleZ(t.conkit.objectstartnumber+1)+t.trotnowz_f );
					if (  t.gridentityrotateaxis == 1  )  RotateObject (  t.conkit.objectstartnumber+1,ObjectAngleX(t.conkit.objectstartnumber+1)+t.trotnowx_f,0,ObjectAngleZ(t.conkit.objectstartnumber+1)+t.trotnowz_f );
					if (  t.gridentityrotateaxis == 2  )  RotateObject (  t.conkit.objectstartnumber+1,ObjectAngleX(t.conkit.objectstartnumber+1)+t.trotnowx_f,ObjectAngleY(t.conkit.objectstartnumber+1)+t.trotnowy_f,0 );
				}
			}
			else
			{
				if (  t.inputsys.k_s == "0" ) 
				{
					RotateObject (  t.conkit.objectstartnumber+1,0,0,0 );
				}
			}
			//  height control
			t.tposnowy_f=0.0 ; t.tposnow=0;
			if (  t.inputsys.kscancode == 209 ) { t.tposnowy_f = -1  ; t.tposnow = 1 ; t.conkit.edit.entityeditposoffground = 1; }
			if (  t.inputsys.kscancode == 201 ) { t.tposnowy_f = 1  ; t.tposnow = 1 ; t.conkit.edit.entityeditposoffground = 1; }
			t.tposnowy_f=t.tposnowy_f*g.timeelapsed_f;
			if (  t.inputsys.keyshift == 1 ) 
			{
				t.tposnowy_f=t.tposnowy_f*10.0;
			}
			else
			{
				if (  t.inputsys.keycontrol == 1 ) 
				{
					t.tposnowy_f=t.tposnowy_f*0.1;
				}
			}
			if (  t.tposnow == 1 ) 
			{
				t.conkit.edit.posoffy = ObjectPositionY(t.conkit.objectstartnumber+1);
				t.conkit.edit.entityheightadjust=t.conkit.edit.entityheightadjust+t.tposnowy_f;
			}
			//  return key (find Floor ( /top of other entity) )
			if (  t.inputsys.keyreturn == 1 ) 
			{
				//  code partly from gridedit but for direct-obj
				t.conkit.edit.entityeditposoffground=0;
				t.tbestdist_f=99999 ; t.tbesty_f=0;
				t.tcurrentposy_f=ObjectPositionY(t.conkit.objectstartnumber+1)+t.conkit.edit.entityheightadjust;
				t.tto_f=t.tcurrentposy_f-200.0;
				if (  t.inputsys.keyshift == 0 ) 
				{
					t.tfrom_f=t.tcurrentposy_f+500.0;
				}
				else
				{
					t.tfrom_f=t.tcurrentposy_f+75.0;
				}
				for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
				{
					t.obj=t.entityelement[t.e].obj;
					if (  t.obj>0 ) 
					{
						if (  ObjectExist(t.obj) == 1 ) 
						{
							if (  GetVisible(t.obj) == 1 ) 
							{
								if (  IntersectObject(t.obj,ObjectPositionX(t.conkit.objectstartnumber+1),t.tfrom_f,ObjectPositionZ(t.conkit.objectstartnumber+1),ObjectPositionX(t.conkit.objectstartnumber+1),t.tto_f,ObjectPositionZ(t.conkit.objectstartnumber+1)) != 0 ) 
								{
									t.tdist_f=abs(ChecklistFValueB(6)-t.tfrom_f);
									if (  t.tdist_f<t.tbestdist_f ) 
									{
										t.tbesty_f=ChecklistFValueB(6);
										t.tbestdist_f=t.tdist_f;
									}
								}
							}
						}
					}
				}
				if (  t.tbestdist_f == 99999 ) 
				{
					t.conkit.edit.entityheightadjust=0;
					t.conkit.edit.entityeditposoffground=0;
				}
				else
				{
					t.conkit.edit.entityeditposoffground=1;
					if (  t.terrain.TerrainID>0 ) 
					{
						t.conkit.edit.posoffy=BT_GetGroundHeight(t.terrain.TerrainID,t.inputsys.localx_f,t.inputsys.localy_f);
						t.conkit.edit.entityheightadjust=t.tbesty_f-t.conkit.edit.posoffy;
					}
					else
					{
						t.conkit.edit.posoffy=1000;
						t.conkit.edit.entityheightadjust=t.tbesty_f-t.conkit.edit.posoffy;
					}
				}
			}
		}

		//  gravity control
		if (  t.inputsys.k_s == "g" ) 
		{
			if (  t.playercontrol.gravityactivepress == 0 ) 
			{
				t.playercontrol.gravityactivepress=1;
				t.playercontrol.gravityactive=1-t.playercontrol.gravityactive;
				t.playercontrol.lockatheight=0;
				if (  t.playercontrol.gravityactive == 1 ) 
				{
					ODESetWorldGravity (  0,-20,0 );
				}
				else
				{
					ODESetWorldGravity (  0,0,0 );
				}
			}
		}
		else
		{
			t.playercontrol.gravityactivepress=0;
		}
		if (  t.inputsys.k_s == "l" ) 
		{
			if (  t.playercontrol.lockatheightpress == 0 ) 
			{
				t.playercontrol.lockatheightpress=1;
				t.playercontrol.lockatheight=1-t.playercontrol.lockatheight;
			}
		}
		else
		{
			t.playercontrol.lockatheightpress=0;
		}

		//  PGUP/PGDN (and mouse scroll)
		if (  t.inputsys.k_s == "[" || t.inputsys.k_s == "]" || t.inputsys.zmouselast != t.inputsys.zmouse || t.inputsys.k_s == "m" ) 
		{
			if (  t.playercontrol.controlheightpress == 0 ) 
			{
				t.playercontrol.controlheightpress=1;
				if (  t.playercontrol.controlheight == 0 ) 
				{
					t.playercontrol.lockatheight=1;
					t.playercontrol.gravityactive=0;
					ODESetWorldGravity (  0,0,0 );
				}
				if (  t.inputsys.k_s == "m" ) 
				{
					t.playercontrol.controlheight=3;
				}
				else
				{
					if (  t.inputsys.zmouselast != t.inputsys.zmouse ) 
					{
						if (  t.inputsys.zmouselast>t.inputsys.zmouse  )  t.playercontrol.controlheight = 1;
						if (  t.inputsys.zmouselast<t.inputsys.zmouse  )  t.playercontrol.controlheight = 2;
					}
					else
					{
						if (  t.inputsys.k_s == "]"  )  t.playercontrol.controlheight = 1;
						if (  t.inputsys.k_s == "["  )  t.playercontrol.controlheight = 2;
					}
				}
			}
			t.inputsys.zmouselast=t.inputsys.zmouse;
		}
		else
		{
			if ( t.playercontrol.controlheight!=0 && t.playercontrol.controlheightcooldown==0.0f )
			{
				t.playercontrol.controlheightcooldown=50.0f;
			}
			t.playercontrol.controlheightpress=0;
			t.playercontrol.controlheight=0;
		}

		//  update terrain shader highlight cursor
		if (  ObjectExist(t.conkit.objectstartnumber+0) == 0 ) 
		{
			MakeObjectCube (  t.conkit.objectstartnumber+0,0 );
			HideObject (  t.conkit.objectstartnumber+0 );
		}
		if (  ObjectExist(t.conkit.objectstartnumber+0) == 1 ) 
		{
			//  position edit cursor object
			if (  t.playercontrol.gravityactive == 1 ) 
			{
				PositionObject (  t.conkit.objectstartnumber+0,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				RotateObject (  t.conkit.objectstartnumber+0,CameraAngleX(t.terrain.gameplaycamera),CameraAngleY(t.terrain.gameplaycamera),0 );
				MoveObject (  t.conkit.objectstartnumber+0,500 );
				t.trayx1_f=CameraPositionX(t.terrain.gameplaycamera);
				t.trayy1_f=CameraPositionY(t.terrain.gameplaycamera);
				t.trayz1_f=CameraPositionZ(t.terrain.gameplaycamera);
				t.trayx2_f=ObjectPositionX(t.conkit.objectstartnumber+0);
				t.trayy2_f=ObjectPositionY(t.conkit.objectstartnumber+0);
				t.trayz2_f=ObjectPositionZ(t.conkit.objectstartnumber+0);
				t.trayxi_f=(t.trayx2_f-t.trayx1_f)/100.0;
				t.trayyi_f=(t.trayy2_f-t.trayy1_f)/100.0;
				t.trayzi_f=(t.trayz2_f-t.trayz1_f)/100.0;
				t.tcamerafooty_f=t.trayy1_f-70.0;
				while (  t.trayy2_f<t.tcamerafooty_f ) 
				{
					t.trayx2_f=t.trayx2_f-t.trayxi_f;
					t.trayy2_f=t.trayy2_f-t.trayyi_f;
					t.trayz2_f=t.trayz2_f-t.trayzi_f;
				}
			}
			else
			{
				if (  t.terrain.superflat  ==  0 ) 
				{
					//  cast a spread of rays for our smoothing effect
					t.trayx1_f=CameraPositionX(t.terrain.gameplaycamera);
					t.trayy1_f=CameraPositionY(t.terrain.gameplaycamera);
					t.trayz1_f=CameraPositionZ(t.terrain.gameplaycamera);
					t.trayx2_f = 0; t.trayy2_f = 0; t.trayz2_f = 0;
					for ( t.tRay = 1 ; t.tRay<=  CONKITEDIT_CURSORRAYS; t.tRay++ )
					{
						t.tAng_f = (t.tRay - 1 - CONKITEDIT_CURSORRAYS/2.0) * ((CONKITEDIT_CURSORRAYSPREAD * 2.0)/CONKITEDIT_CURSORRAYS);
						PositionObject (  t.conkit.objectstartnumber+0,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
						RotateObject (  t.conkit.objectstartnumber+0,CameraAngleX(t.terrain.gameplaycamera)+t.tAng_f , CameraAngleY(t.terrain.gameplaycamera),0 );
						MoveObject (  t.conkit.objectstartnumber+0,CONKITEDIT_CURSORRAYDIST );
						t.tobjx_f=ObjectPositionX(t.conkit.objectstartnumber+0);
						t.tobjy_f=ObjectPositionY(t.conkit.objectstartnumber+0);
						t.tobjz_f=ObjectPositionZ(t.conkit.objectstartnumber+0);
						if (  ODERayTerrain(t.trayx1_f,t.trayy1_f,t.trayz1_f,t.tobjx_f,t.tobjy_f,t.tobjz_f) == 1 ) 
						{
							t.trayx2_f = t.trayx2_f + ODEGetRayCollisionX();
							t.trayz2_f = t.trayz2_f + ODEGetRayCollisionZ();
						}
						else
						{
							t.trayx2_f = t.trayx2_f + t.tobjx_f;
							t.trayz2_f = t.trayz2_f + t.tobjz_f;
						}
					}
					t.trayx2_f = t.trayx2_f / CONKITEDIT_CURSORRAYS;
					t.trayz2_f = t.trayz2_f / CONKITEDIT_CURSORRAYS;
				}
				else
				{
					//  for superflat terrain we can more easily calculate where the cursor should be and there is no hill smoothing to worry about
					t.tdist_f = CONKITEDIT_CURSORRAYDIST;
					if (  CameraAngleX(t.terrain.gameplaycamera) > 0 ) 
					{
						t.tdist_f = (CameraPositionY(t.terrain.gameplaycamera) - TERRAIN_SUPERFLAT_HEIGHT)/Tan(CameraAngleX(t.terrain.gameplaycamera));
						if (  t.tdist_f < 0  )  t.tdist_f  =  0;
						if (  t.tdist_f > CONKITEDIT_CURSORRAYDIST  )  t.tdist_f  =  CONKITEDIT_CURSORRAYDIST;
					}
					t.trayx2_f = CameraPositionX(t.terrain.gameplaycamera) + Sin(CameraAngleY()) * t.tdist_f;
					t.trayz2_f = CameraPositionZ(t.terrain.gameplaycamera) + Cos(CameraAngleY()) * t.tdist_f;
				}
			}
			//  place terrain highlighter at edit cursor position
			t.inputsys.localx_f=t.trayx2_f ; t.inputsys.localy_f=t.trayz2_f;
			terrain_cursor_nograsscolor ( );
		}

		//  use terrain controls to sculpt/paint
		t.terrain.lastxmouse=-1 ; t.terrain.lastymouse=-1;
		t.terrain.zoom_f=0.10f  ; terrain_editcontrol ( );

		//  update terrain physics on the fly
		if (  t.terrain.dirtyterrain == 1 ) 
		{
			if (  1 ) 
			{
				t.tgenerateterraindirtyregiononly=1;
				physics_createterraincollision ( );
				t.tgenerateterraindirtyregiononly=0;
			}
			else
			{
				//  heightmap based terrain (old)
				if (  MemblockExist(30) == 0  )  MakeMemblock (  30,(1024*1024*4) );
				if (  MemblockExist(30) == 1 ) 
				{
					t.pos=0;
					for ( t.z = t.terrain.dirtyz1 ; t.z<=  t.terrain.dirtyz2; t.z++ )
					{
						for ( t.x = t.terrain.dirtyx1 ; t.x<=  t.terrain.dirtyx2; t.x++ )
						{
							t.h_f=BT_GetGroundHeight(t.terrain.TerrainID,(t.x*50.0),(t.z*50.0));
							WriteMemblockFloat (  30,t.pos,t.h_f );
							t.pos += 4;
						}
					}
					t.memblockptr=GetMemblockPtr(30);
					ODEUpdateStaticTerrain (  t.tphysicsterrainobjstart,1024,1024,t.memblockptr,t.terrain.dirtyx1,t.terrain.dirtyz1,t.terrain.dirtyx2,t.terrain.dirtyz2 );
				}
			}
			t.terrain.dirtyterrain=0;
		}

		//  update grass memblock on the fly
		if (  t.terrain.grassregionupdate == 1 ) 
		{
			terrain_fastveg_updatedirtyregionfast ( );
			t.terrain.grassregionupdate=0;
		}

		//  draw any HUDs related to CONKIT
		terrain_getpaintmode ( );
		if (  ImageExist(g.editorimagesoffset+21) == 1 ) 
		{
			if (  t.conkit.entityeditmode == 1 ) 
			{
				PasteImage (  g.editorimagesoffset+22,8,8 );
			}
			else
			{
				PasteImage (  g.editorimagesoffset+21,8,8 );
			}
		}
		t.tprompt_s=t.mode_s;
		t.tposx=GetDisplayWidth()-getbitmapfontwidth(t.tprompt_s.Get(),1)-16;
		pastebitmapfont(t.tprompt_s.Get(),t.tposx,8,1,255);
		if (  t.playercontrol.gravityactive == 1 ) 
		{
			t.tprompt_s="Gravity On";
		}
		else
		{
			t.tprompt_s="Gravity Off";
			if (  t.playercontrol.lockatheight == 1 ) 
			{
				t.tprompt_s=t.tprompt_s+" Height Locked";
			}
		}
		t.tposx=GetDisplayWidth()-getbitmapfontwidth(t.tprompt_s.Get(),1)-16;
		pastebitmapfont(t.tprompt_s.Get(),t.tposx,8+32,1,255);

		//  draw entity edit cursor (if required)
		if (  t.conkit.entityeditmode == 1 ) 
		{
			PasteSprite (  124,GetDisplayWidth()/2,GetDisplayHeight()/2 );
		}

		//  detect switch OFF
		if (  (KeyState(g.keymap[67]) == 1 || t.conkit.forceaction == 67) && t.conkit.edit.conkitkeypressed == 0  )  t.conkit.edit.conkitkeypressed = 1;
		if (  (KeyState(g.keymap[67]) == 0 && t.conkit.forceaction != 67) && t.conkit.edit.conkitkeypressed == 1 ) 
		{
			conkitedit_switchoff ( );
		}
	}
	else
	{
		//  detect switch ON (only if not standalone)
		if (  t.game.gameisexe == 0 && t.game.runasmultiplayer  ==  0 && (g.vrglobals.GGVREnabled == 0 || g.vrglobals.GGVRUsingVRSystem == 0 ) ) 
		{
			if (  (KeyState(g.keymap[67]) == 1 || t.conkit.forceaction == 68) && t.conkit.edit.conkitkeypressed == 0  )  t.conkit.edit.conkitkeypressed = 1;
			if (  (KeyState(g.keymap[67]) == 0 && t.conkit.forceaction != 68) && t.conkit.edit.conkitkeypressed == 1 ) 
			{
				//  disable third person controls during 3D EDIT MODE
				if (  t.playercontrol.thirdperson.enabled == 1 ) 
				{
					t.tobj=t.entityelement[t.playercontrol.thirdperson.charactere].obj;
					if ( t.tobj>0 ) { if ( ObjectExist(t.tobj)==1 ) { HideObject( t.tobj ) ; } }
					t.tobj=t.entityelement[t.playercontrol.thirdperson.charactere].attachmentobj;
					if ( t.tobj>0 ) { if ( ObjectExist(t.tobj)==1 ) { HideObject( t.tobj ) ; } }
				}
				t.playercontrol.thirdperson.enabledstorefortestgameedit=t.playercontrol.thirdperson.enabled;
				t.playercontrol.thirdperson.enabledstorefortestgameeditgunid=t.gunid;
				t.playercontrol.thirdperson.enabled=0;

				//  first make sure conkit is switch off
				if ( t.conkit.make.mode > CONKIT_MODE_OFF ) conkit_quit ( );

				//  Set any death-haze-red ness to OFF when in F9 edit mode
				t.playercontrol.redDeathFog_f=0.0;
				SetVector4 (  g.terrainvectorindex,t.playercontrol.redDeathFog_f,0,0,0 );
				t.tColorVector=g.terrainvectorindex ; postprocess_setscreencolor ( );

				//  F9 means we can modify level, so ensure this is flagged
				g.projectmodified=1 ; gridedit_changemodifiedflag ( );
				g.projectmodifiedstatic = 1;
				t.storeprojectmodified = 1; // ensure if use F9, we save if immediately exit editor

				//  prepare entity edit cursor
				t.imgx_f=ImageWidth(g.editorimagesoffset+10);
				t.imgy_f=ImageHeight(g.editorimagesoffset+10);
				Sprite (  124,-10000,-10000,g.editorimagesoffset+10 );
				SizeSprite (  124,t.imgx_f,t.imgy_f );
				OffsetSprite (  124,t.imgx_f/2,t.imgy_f/2 );

				//  230315 - reset all entity clones back to instances for editing (will restore later)
				for ( t.e = 1 ; t.e<=  g.entityelementmax; t.e++ )
				{
					t.tentid = t.entityelement[t.e].bankindex;
					if ( t.entityprofile[t.tentid].isebe == 0 )
					{
						if ( t.entityelement[t.e].active==1 )
						{
							t.entityelement[t.e].active = 4;
						}
						if (  t.entityprofile[t.tentid].ismarker == 0 ) 
						{
							if (  t.entityelement[t.e].active != 0 && t.entityelement[t.e].health>0 ) 
							{
								t.tokay=1;
								if (  t.playercontrol.thirdperson.enabledstorefortestgameedit == 1 ) 
								{
									if (  t.e == t.playercontrol.thirdperson.charactere  )  t.tokay = 0;
								}
								int tobj = t.entityelement[t.e].obj;
								if ( t.tokay == 1 && tobj > 0 ) 
								{
									// only if visible (ignores yet to be spawned entities)
									if ( GetVisible ( tobj )==1 )
									{
										// turn to instancing for editing
										t.tte = t.e ; entity_converttoinstance ( );

										// hide attachments
										if (  t.entityelement[t.e].attachmentobj>0 ) 
										{
											if (  ObjectExist(t.entityelement[t.e].attachmentobj) == 1 ) 
											{
												HideObject (  t.entityelement[t.e].attachmentobj );
											}
										}

										// 171115 - but only if alive in game
										if ( t.entityprofile[t.tentid].ischaracter==1 )
										{
											if ( t.entityelement[t.e].health > 0 )
											{
												t.entityelement[t.e].active = 3;
											}
										}
									}
								}
							}
						}
					}
				}

				if (  t.playercontrol.jetpackmode == 2  )  t.playercontrol.jetpackmode = 3 ; else t.playercontrol.jetpackmode = 0;
				g.remembergunid = t.gunid;
				g.autoloadgun = 0;
				gun_change ( );
				terrain_whitewashwatermask ( );
				t.terrain.RADIUS_f=100;
				t.conkit.editmodeactive=1;
				t.conkit.edit.conkitkeypressed=0;
				t.inputsys.zmouselast=t.inputsys.zmouse;
				if (  t.conkit.edit.entityindex == 0  )  t.conkit.edit.entityindex = 1;
				t.conkit.edit.entityindexmax=g.entidmaster;
				t.conkit.edit.entityeditgrabbed=0;
				t.conkit.entityeditmode=0;
				conkitedit_updateeditmodecursorcolor ( );
				//  ensure occlusion not hooked to entities we might delete/move
				//Dave - dont clear the list, we can carry on using occlusion after
				//if (  g.globals.occlusionmode == 1  )  CPU3DClear (  );
				//  silence any sounds when in F9 mode
				for ( t.s = 1 ; t.s<=  60000; t.s++ )
				{
					if (  SoundExist(t.s) == 1 ) 
					{
						SetSoundVolume (  t.s,0 );
					}
				}
				//  switch of water physics effect on player
				ODESetDynamicCharacterController (  t.aisystem.objectstartindex,-10000,0,0,0,0,0,0 );
			}
		}

	}

	//  Wipe after one cycle
	t.conkit.forceaction=0;

return;

}

void conkitedit_switchoff ( void )
{
	// Show LUA Sprites
	HideOrShowLUASprites ( false );

	//  restore third person mode
	t.playercontrol.thirdperson.enabled=t.playercontrol.thirdperson.enabledstorefortestgameedit;
	if (  t.playercontrol.thirdperson.enabledstorefortestgameedit == 1 ) 
	{
		//  restore third person game play
		t.tobj=t.entityelement[t.playercontrol.thirdperson.charactere].obj;
		if ( t.tobj>0 ) { if ( ObjectExist(t.tobj)==1 ) { ShowObject( t.tobj ) ; } }
		t.tobj=t.entityelement[t.playercontrol.thirdperson.charactere].attachmentobj;
		if ( t.tobj>0 ) { if ( ObjectExist(t.tobj)==1 ) { ShowObject( t.tobj ) ; } }
		//  and restore weapon
		g.autoloadgun=t.playercontrol.thirdperson.enabledstorefortestgameeditgunid;
		gun_change ( );
	}
	else
	{
		// 161115 - restore any gun for FPS mode
		g.autoloadgun = g.remembergunid;
		g.remembergunid = 0;
	}

	//  turns off conkit edit mode
	if (  SpriteExist(124) == 1  )  DeleteSprite (  124 );
	terrain_cursor_off ( );
	t.conkit.entityeditmode=0;
	t.conkit.editmodeactive=0;
	t.conkit.edit.conkitkeypressed=0;
	t.playercontrol.lockatheight=0;
	t.playercontrol.gravityactive=1;
	ODESetWorldGravity (  0,-20,0 );
	conkitedit_resetentityhighlighted ( );
	if (  ObjectExist(t.conkit.objectstartnumber+1) == 1  )  DeleteObject (  t.conkit.objectstartnumber+1 );
	t.conkit.edit.entityeditgrabbed=0;
	//  restore any sounds when in F9 mode
	for ( t.s = 1 ; t.s<=  60000; t.s++ )
	{
		if (  SoundExist(t.s) == 1 ) 
		{
			SetSoundVolume (  t.s,100.0 * t.audioVolume.soundFloat );
		}
	}
	//  restore water physics effect on player
	if (  t.hardwareinfoglobals.nowater != 0 ) 
	{
		ODESetDynamicCharacterController (  t.aisystem.objectstartindex,-10000,0,0,0,0,0,0 );
	}
	else
	{
		ODESetDynamicCharacterController (  t.aisystem.objectstartindex,480,0,0,0,0,0,0 );
	}
	//  Ensure editor does not have a selection when return
	t.gridentity=0;

	//  Ensure any newly introduced entities are incorporated into game
	entity_bringnewentitiestolife ( );

	//  Uses ragdollified to ignore dead characters
	for ( t.e = 1 ; t.e<=  g.entityelementmax; t.e++ )
	{
		if (  t.entityelement[t.e].active == 0 ) 
		{
			t.entityelement[t.e].ragdollified=1;
			t.entityelement[t.e].active=100;
		}
	}
	darkai_setup_characters ( );
	for ( t.e = 1 ; t.e<=  g.entityelementmax; t.e++ )
	{
		if (  t.entityelement[t.e].active == 100 ) 
		{
			t.entityelement[t.e].ragdollified=0;
			t.entityelement[t.e].active=0;
		}
	}

	// 021215 - ensure animation smooth array is reset so objects are treated to fresh anims (all turned to instances in F9 mode)
	darkai_resetsmoothanims();

	// 131115 - reactivate gun index if third person controller used
	if ( t.playercontrol.thirdperson.enabled == 1 )
	{
		t.gunid = t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.hasweapon;
		t.currentgunobj = t.gun[t.gunid].obj;
		if ( t.currentgunobj > 0 )
			if ( ObjectExist ( t.currentgunobj )==1 )
				HideObject ( t.currentgunobj );
	}
}
