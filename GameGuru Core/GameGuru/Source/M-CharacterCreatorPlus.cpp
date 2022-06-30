//----------------------------------------------------
//--- GAMEGURU - M-CharacterCreatorPlus
//----------------------------------------------------

// Includes
#include "stdafx.h"
#include "gameguru.h"
#include "M-CharacterCreatorPlusTTS.h"
#include "CCameraC.h"

//PE: GameGuru IMGUI.
#include "..\Imgui\imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "..\Imgui\imgui_internal.h"
#include "..\Imgui\imgui_impl_win32.h"
#include "..\Imgui\imgui_gg_dx11.h"

// Defines
#define CCPMODELEXT ".x"

// Globals
bool g_bCharacterCreatorPlusActivated = false;
sCharacterCreatorPlus g_CharacterCreatorPlus;
bool g_bCharacterCreatorPrepAnims = false;

bool g_bLegsChangeCascade = false;
bool g_bFeetChangeCascade = false;

bool g_charactercreatorplus_preloading = false;
char g_charactercreatorplus_path[MAX_PATH];
int g_charactercreatorplus_part = 0;
char g_charactercreatorplus_tag[MAX_PATH];

static std::map<std::string, std::string> CharacterCreatorHeadGear_s;
static std::map<std::string, std::string> CharacterCreatorHair_s;
static std::map<std::string, std::string> CharacterCreatorHead_s;
static std::map<std::string, std::string> CharacterCreatorEyeglasses_s;
static std::map<std::string, std::string> CharacterCreatorFacialHair_s;
static std::map<std::string, std::string> CharacterCreatorBody_s;
static std::map<std::string, std::string> CharacterCreatorLegs_s;
static std::map<std::string, std::string> CharacterCreatorFeet_s;

static std::map<std::string, std::string> g_charactercreatorplus_annotation_list;
static std::map<std::string, std::string> CharacterCreatorAnnotatedHeadGear_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedHair_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedHead_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedEyeglasses_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedFacialHair_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedBody_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedFeet_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedLegs_s;

static std::map<std::string, std::string> g_charactercreatorplus_annotationtag_list;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagHeadGear_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagHair_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagHead_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagEyeglasses_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagFacialHair_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagBody_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagFeet_s;
static std::map<std::string, std::string> CharacterCreatorAnnotatedTagLegs_s;

static std::map<std::string, std::string> CharacterCreatorType_s;
int iCharObj = 0, iCharObjHeadGear = 0, iCharObjHair = 0, iCharObjHead = 0, iCharObjEyeglasses = 0, iCharObjFacialHair = 0, iCharObjLegs = 0, iCharObjFeet = 0;

bool bCharObjVisible = false;
char CCP_Type[260] = "adult male";
char CCP_Name[260] = "\0";
static char CCP_Script[260] = "people\\patrol.lua";
char CCP_Path[260] = "entitybank\\user\\charactercreatorplus\\";
char CCP_SpeakText[1024] = "Hello there, I am a new character!\n";
wchar_t CCP_SpeakText_w[1024];

int CCP_Speak_Rate = 0;
char cSelectedLegsFilter[260] = "\0";
char cSelectedFeetFilter[260] = "\0";
char cSelectedICCode[260] = "\0";
char cSelectedHeadGear[260] = "\0";
char cSelectedHair[260] = "\0";
char cSelectedHead[260] = "\0";
char cSelectedEyeglasses[260] = "\0";
char cSelectedFacialHair[260] = "\0";
char cSelectedBody[260] = "\0";
char cSelectedLegs[260] = "\0";
char cSelectedFeet[260] = "\0";
ISpObjectToken * CCP_SelectedToken = 0;
LPSTR pCCPVoiceSet = "";
ImVec4 vColorSelected[5];
float oldx_f, oldy_f, oldz_f, oldangx_f, oldangy_f;
float editoroldx_f=0, editoroldy_f, editoroldz_f, editoroldangx_f, editoroldangy_f, editoroldmode_f;
int iDelayThumbs = 99; //0; //Icon removed
int iDelayExecute = 0;
int iThumbsOffsetY = 0;
float fCharObjectY = 600.0f;
float ccpTargetX, ccpTargetY, ccpTargetZ, ccpTargetAX, ccpTargetAY;
float ccpObjTargetX, ccpObjTargetY, ccpObjTargetZ, ccpObjTargetAX, ccpObjTargetAY, ccpObjTargetAZ;
float fCCPRotateY = 0.0f;

extern bool bTriggerMessage;
extern char cTriggerMessage[MAX_PATH];
void DisplaySmallImGuiMessage(char *text);
bool bMessageDisplayed = false;

extern preferences pref;


void charactercreatorplus_preloadinitialcharacter ( void )
{
	#ifdef PRODUCTV3
	image_preload_files_start();
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male body 02_color.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male body 02_ao.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male body 02_normal.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male body 02_metalness.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male body 02_gloss.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male body 02_mask.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male head 01_color.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male head 01_ao.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male head 01_normal.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male head 01_metalness.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male head 01_gloss.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male head 01_mask.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male legs 02_color.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male legs 02_ao.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male legs 02_normal.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male legs 02_metalness.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male legs 02_gloss.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male legs 02_mask.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male feet 01_color.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male feet 01_ao.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male feet 01_normal.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male feet 01_metalness.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male feet 01_gloss.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male feet 01_mask.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male hair 01_color.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male hair 01_ao.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male hair 01_normal.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male hair 01_metalness.dds");
	image_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male hair 01_gloss.dds");
	image_preload_files_finish();
	object_preload_files_start();
	object_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male body 02.x");
	object_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male head 01.x");
	object_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male legs 02.x");
	object_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male feet 01.x");
	object_preload_files_add("charactercreatorplus\\parts\\adult male\\adult male hair 01.x");
	object_preload_files_finish();
	#else
	#endif
}

void charactercreatorplus_GetDefaultCharacterPartNum (int iBase, int iPart, LPSTR pPartNumStr, LPSTR pPartNumVariantStr = NULL)
{
	LPSTR pPart = "";
	LPSTR pPartVariant = "";
	if (iBase == 1)
	{
		// male
		if (iPart == 1) { pPart = "13"; pPartVariant = "13c"; }
		if (iPart == 2) { pPart = "07"; pPartVariant = "07"; }
		if (iPart == 3) { pPart = "12"; pPartVariant = "12c"; }
		if (iPart == 4) { pPart = "10"; pPartVariant = "10"; }
		if (iPart == 5) { pPart = "10"; pPartVariant = "10"; }
	}
	if (iBase == 2)
	{
		// female
		if (iPart == 1) { pPart = "07"; pPartVariant = "07"; }
		if (iPart == 2) { pPart = "07"; pPartVariant = "07"; }
		if (iPart == 3) { pPart = "07"; pPartVariant = "07"; }
		if (iPart == 4) { pPart = "05"; pPartVariant = "05"; }
		if (iPart == 5) { pPart = "06"; pPartVariant = "06"; }
	}
	if (iBase == 3)
	{
		// zombie
		if (iPart == 1) { pPart = "01"; pPartVariant = "01"; }
		if (iPart == 2) { pPart = "01"; pPartVariant = "01"; }
		if (iPart == 3) { pPart = "01"; pPartVariant = "01"; }
		if (iPart == 4) { pPart = "01"; pPartVariant = "01"; }
		if (iPart == 5) { pPart = "01"; pPartVariant = "01"; }
	}
	strcpy (pPartNumStr, pPart);
	if ( pPartNumVariantStr ) strcpy (pPartNumVariantStr, pPartVariant);
}

void charactercreatorplus_preloadallcharacterbasedefaults(void)
{
	#ifndef PRODUCTV3
	int iBaseCount = 3;
	#else
	int iBaseCount = 4;
	#endif
	for (int base = 1; base <= iBaseCount; base++)
	{
		LPSTR pBase = NULL;
		char pRelFile[MAX_PATH];
		if (base == 1) pBase = "adult male";
		if (base == 2) pBase = "adult female";
		#ifdef PRODUCTV3
		if (base == 3) pBase = "child female";
		if (base == 4) pBase = "child male";
		#else
		if (base == 3) pBase = "zombie male";
		#endif
		for (int part = 1; part <= 5; part++)
		{
			char pPart[1024];
			strcpy(pPart, "");
			#ifdef PRODUCTV3
			if (part == 1) strcpy(pPart, "body 02");
			if (part == 2) strcpy(pPart, "head 01");
			if (part == 3) strcpy(pPart, "legs 02");
			if (part == 4) strcpy(pPart, "feet 01");
			if (part == 5) strcpy(pPart, "hair 01");
			#else
			if (base == 3)
			{
				// zombie
				if (part == 1) strcpy(pPart, "body 01");
				if (part == 2) strcpy(pPart, "head 01");
				if (part == 3) strcpy(pPart, "legs 01");
				if (part == 4) strcpy(pPart, "feet 01");
				if (part == 5) strcpy(pPart, "hair 01");
			}
			else
			{
				// normal male and female
				if (part == 1) strcpy(pPart, "body 03");
				if (part == 2) strcpy(pPart, "head 04");
				if (part == 3) strcpy(pPart, "legs 03");
				if (part == 4) strcpy(pPart, "feet 01");
				if (part == 5) strcpy(pPart, "hair 01");
			}
			#endif
			for (int item = 1; item <= 6; item++)
			{
				LPSTR pItem = NULL;
				if (item == 1) pItem = "_color.dds";
				if (item == 2) pItem = "_ao.dds";
				if (item == 3) pItem = "_normal.dds";
				if (item == 4) pItem = "_metalness.dds";
				if (item == 5) pItem = "_gloss.dds";
				if (item == 6) pItem = "_mask.dds";
				sprintf(pRelFile, "charactercreatorplus\\parts\\%s\\%s %s%s", pBase, pBase, pPart, pItem);
				image_preload_files_add(pRelFile);
			}
			sprintf(pRelFile, "charactercreatorplus\\parts\\%s\\%s %s%s", pBase, pBase, pPart, CCPMODELEXT);
			object_preload_files_add(pRelFile);
		}
	}
}

void charactercreatorplus_preloadallcharacterpartchoices ( void )
{
	image_preload_files_start();
	object_preload_files_start();
	static std::map<std::string, std::string> CharacterCreatorCurrent_s;
	for (int part_loop = 0; part_loop < 8; part_loop++) 
	{
		if (part_loop == 0) CharacterCreatorCurrent_s = CharacterCreatorHeadGear_s;
		if (part_loop == 1) CharacterCreatorCurrent_s = CharacterCreatorHair_s;
		if (part_loop == 2) CharacterCreatorCurrent_s = CharacterCreatorHead_s;
		if (part_loop == 3) CharacterCreatorCurrent_s = CharacterCreatorEyeglasses_s;
		if (part_loop == 4) CharacterCreatorCurrent_s = CharacterCreatorFacialHair_s;
		if (part_loop == 5) CharacterCreatorCurrent_s = CharacterCreatorBody_s;
		if (part_loop == 6) CharacterCreatorCurrent_s = CharacterCreatorLegs_s;
		if (part_loop == 7) CharacterCreatorCurrent_s = CharacterCreatorFeet_s;
		if (!CharacterCreatorCurrent_s.empty())
		{
			for (std::map<std::string, std::string>::iterator it = CharacterCreatorCurrent_s.begin(); it != CharacterCreatorCurrent_s.end(); ++it)
			{
				std::string full_path = it->second;
				std::string name = it->first;
				char pFullBaseFilename[2048];
				strcpy ( pFullBaseFilename, full_path.c_str() );
				strcat ( pFullBaseFilename, name.c_str());

				// ignore None entries in the list
				if (strnicmp(name.c_str(), "None", 4) != NULL)
				{
					// detect color variant
					char pFullBaseVariantFilename[2048];
					strcpy(pFullBaseVariantFilename, pFullBaseFilename);
					char pLastLetter = pFullBaseFilename[strlen(pFullBaseFilename) - 1];
					if (pLastLetter >= 'a' && pLastLetter <= 'z')
					{
						// using a color variant
						strcpy(pFullBaseVariantFilename, pFullBaseFilename);
						pFullBaseFilename[strlen(pFullBaseFilename) - 1] = 0;
					}
					char pWorkFile[2038];
					strcpy(pWorkFile, pFullBaseFilename); strcat(pWorkFile, CCPMODELEXT);
					object_preload_files_add(pWorkFile);
					strcpy(pWorkFile, pFullBaseVariantFilename); strcat(pWorkFile, "_color.dds");
					image_preload_files_add(pWorkFile);
					strcpy(pWorkFile, pFullBaseFilename); strcat(pWorkFile, "_ao.dds");
					image_preload_files_add(pWorkFile);
					strcpy(pWorkFile, pFullBaseFilename); strcat(pWorkFile, "_normal.dds");
					image_preload_files_add(pWorkFile);
					strcpy(pWorkFile, pFullBaseFilename); strcat(pWorkFile, "_metalness.dds");
					image_preload_files_add(pWorkFile);
					strcpy(pWorkFile, pFullBaseFilename); strcat(pWorkFile, "_gloss.dds");
					image_preload_files_add(pWorkFile);
					strcpy(pWorkFile, pFullBaseFilename); strcat(pWorkFile, "_mask.dds");
					image_preload_files_add(pWorkFile);
				}
			}
		}
	}
	// also add in any base model defaults
	charactercreatorplus_preloadallcharacterbasedefaults();
	image_preload_files_finish();
	object_preload_files_finish();
}

void charactercreatorplus_imgui(void)
{
	extern bool bImGuiGotFocus;
	extern bool bForceKey;
	extern cstr csForceKey;
	extern bool bEntity_Properties_Window;
	bool once_camera_adjust = false;
	if (g_bCharacterCreatorPlusActivated) 
	{
		if (g_CharacterCreatorPlus.bInitialised) 
		{
			// handle thread dependent triggers (smooth UI) 
			charactercreatorplus_waitforpreptofinish();

			// handle in-level visuals
			if (!bCharObjVisible) 
			{
				editoroldmode_f = t.editorfreeflight.mode;
				editoroldx_f = t.editorfreeflight.c.x_f;
				editoroldy_f = t.editorfreeflight.c.y_f;
				editoroldz_f = t.editorfreeflight.c.z_f;
				editoroldangx_f = t.editorfreeflight.c.angx_f;
				editoroldangy_f = t.editorfreeflight.c.angy_f;

				ShowObject(iCharObj);
				bCharObjVisible = true;

				//Hide entities.
				//Exit properties.
				bForceKey = true;
				csForceKey = "e";

				t.inputsys.dowaypointview = 1;
				t.inputsys.domodeentity = 1;

				widget_hide();
				ebe_hide();
				terrain_paintselector_hide();
				editor_restoreentityhighlightobj();
				gridedit_clearentityrubberbandlist();
				waypoint_hideall();

				ccpTargetX = t.editorfreeflight.c.x_f;
				ccpTargetY = t.editorfreeflight.c.y_f;
				ccpTargetZ = t.editorfreeflight.c.z_f;
				ccpTargetAX = t.editorfreeflight.c.angx_f;
				ccpTargetAY = t.editorfreeflight.c.angy_f;

				float terrain_height = BT_GetGroundHeight(t.terrain.TerrainID, ccpTargetX, ccpTargetZ, 1);
				fCharObjectY = terrain_height;
				
				float oangx = ObjectAngleX(iCharObj);
				float oangz = ObjectAngleZ(iCharObj);

				//PE: a simple z,x mouse from center of screen.
				float placeatx_f, placeatz_f;
				extern ImVec2 OldrenderTargetSize;
				extern ImVec2 OldrenderTargetPos;
				extern ImVec2 renderTargetAreaSize;
				extern ImVec2 renderTargetAreaPos;
				extern bool bWaypointDrawmode;

				ImVec2 vCenterPos = { (OldrenderTargetSize.x*0.5f) + OldrenderTargetPos.x , (OldrenderTargetSize.y*0.45f) + OldrenderTargetPos.y };

				int omx = t.inputsys.xmouse, omy = t.inputsys.ymouse, oldgridentitysurfacesnap = t.gridentitysurfacesnap, oldonedrag = t.onedrag;;
				bool owdm = bWaypointDrawmode;

				//Always target terrain only.
				float RatioX = ((float)GetDisplayWidth() / (float)renderTargetAreaSize.x) * ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
				float RatioY = ((float)GetDisplayHeight() / (float)renderTargetAreaSize.y) * ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
				t.inputsys.xmouse = (vCenterPos.x - renderTargetAreaPos.x) * RatioX;
				t.inputsys.ymouse = (vCenterPos.y - renderTargetAreaPos.y) * RatioY;

				t.gridentitysurfacesnap = 0; t.onedrag = 0; bWaypointDrawmode = false;

				input_calculatelocalcursor();

				if( !(t.inputsys.picksystemused == 1 || t.inputsys.localcurrentterrainheight_f < 100.0f))
				{
					ccpTargetX = t.inputsys.localx_f;
					ccpTargetZ = t.inputsys.localy_f;
				}

				t.onedrag = oldonedrag;
				bWaypointDrawmode = owdm;
				t.gridentitysurfacesnap = oldgridentitysurfacesnap;
				t.inputsys.xmouse = omx;
				t.inputsys.ymouse = omy;
				//Restore real input.
				input_calculatelocalcursor();

				terrain_height = BT_GetGroundHeight(t.terrain.TerrainID, ccpTargetX, ccpTargetZ, 1);
				fCharObjectY = terrain_height;

				t.editorfreeflight.c.x_f = ccpTargetX;
				t.editorfreeflight.c.z_f = ccpTargetZ;

				SetObjectToCameraOrientation(iCharObj);
				PositionObject(iCharObj, ccpTargetX, fCharObjectY, ccpTargetZ);
				RotateObject(iCharObj, oangx, ObjectAngleY(iCharObj), oangz);
				MoveObject(iCharObj, 120);

				terrain_height = BT_GetGroundHeight(t.terrain.TerrainID, ObjectPositionX(iCharObj), ObjectPositionZ(iCharObj), 1);
				fCharObjectY = terrain_height;
				PositionObject(iCharObj, ObjectPositionX(iCharObj), fCharObjectY, ObjectPositionZ(iCharObj));

				ccpObjTargetX = ObjectPositionX(iCharObj);
				ccpObjTargetY = ObjectPositionY(iCharObj);
				ccpObjTargetZ = ObjectPositionZ(iCharObj);
				ccpObjTargetAX = ObjectAngleX(iCharObj);
				ccpObjTargetAY = ObjectAngleY(iCharObj);
				ccpObjTargetAZ = ObjectAngleZ(iCharObj);

				t.editorfreeflight.mode = 1;
				t.editorfreeflight.c.y_f = fCharObjectY+60;
				t.editorfreeflight.c.angx_f = 11;
				t.editorfreeflight.s = t.editorfreeflight.c;

				once_camera_adjust = true;

				//  "hide" all entities in map by moving them out the way
				for (t.tcce = 1; t.tcce <= g.entityelementlist; t.tcce++)
				{
					t.tccentid = t.entityelement[t.tcce].bankindex;
					if (t.tccentid > 0)
					{
						t.tccsourceobj = t.entityelement[t.tcce].obj;
						if (ObjectExist(t.tccsourceobj) == 1)
						{
							PositionObject(t.tccsourceobj, 0, 0, 0);
						}
					}
				}

				fCCPRotateY = ccpObjTargetAY = ObjectAngleY(iCharObj);
				//PE: Make sure we are in slider range.
				if (fCCPRotateY < 0.0) fCCPRotateY += 360.0;
				if (fCCPRotateY > 360.0) fCCPRotateY -= 360.0;
				// refresh thumbnail
				iDelayThumbs = 0; // Regen thumnbnail
			}

			//Display sky for better look.
			if (ObjectExist(t.terrain.objectstartindex + 4) == 1)
			{
				PositionObject(t.terrain.objectstartindex + 4, CameraPositionX(0), CameraPositionY(0), CameraPositionZ(0));
				SetAlphaMappingOn(t.terrain.objectstartindex + 4, 100.0*t.sky.alpha1_f);
				ShowObject(t.terrain.objectstartindex + 4);
			}

			if (iDelayExecute == 1) 
			{
				//PE: Change type.
				charactercreatorplus_refreshtype();
				iDelayThumbs = 0; // Regen thumnbnail
				iDelayExecute = 0;
			}
			// generate thumbnail
			if (iDelayThumbs <= 6) 
			{
				extern bool g_bNoSwapchainPresent;
				if (iDelayThumbs == 3)
				{
					iDelayThumbs++;
				}
				else if (iDelayThumbs == 5) 
				{
					t.editorfreeflight.mode = 1;
					oldx_f = t.editorfreeflight.c.x_f;
					oldy_f = t.editorfreeflight.c.y_f;
					oldz_f = t.editorfreeflight.c.z_f;
					oldangx_f = t.editorfreeflight.c.angx_f;
					oldangy_f = t.editorfreeflight.c.angy_f;

					float new_th = BT_GetGroundHeight(t.terrain.TerrainID, GGORIGIN_X, GGORIGIN_Z, 1);

					PositionObject(iCharObj, GGORIGIN_X, new_th, GGORIGIN_Z);
					RotateObject(iCharObj, 0, 15, 0);

					t.editorfreeflight.c.x_f = GGORIGIN_X;
					t.editorfreeflight.c.y_f = new_th + 65.0f;
					t.editorfreeflight.c.z_f = GGORIGIN_Z - 240;
					t.editorfreeflight.c.angx_f = 0.0f;
					t.editorfreeflight.c.angy_f = 0;
					t.editorfreeflight.s = t.editorfreeflight.c;

					if (ObjectExist(t.terrain.objectstartindex + 4) == 1)
					{
						PositionObject(t.terrain.objectstartindex + 4, t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
						SetAlphaMappingOn(t.terrain.objectstartindex + 4, 100.0*t.sky.alpha1_f);
						ShowObject(t.terrain.objectstartindex + 4);
					}

					//Remove flicker when generating new thumb.
					g_bNoSwapchainPresent = true; //dont present backbuffer to HWND.

					PositionCamera(t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
					RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);

					// raise ambience for shot
					for (int iShaderIndex = 0; iShaderIndex < 2; iShaderIndex++)
					{
						if (iShaderIndex == 0) t.effectid = g.thirdpersonentityeffect;
						if (iShaderIndex == 1) t.effectid = g.thirdpersoncharactereffect;
						if (GetEffectExist(t.effectid) == 1)
						{
							SetVector4(g.terrainvectorindex, 0.9f, 0.9f, 0.9f, 1);
							SetEffectConstantV(t.effectid, "AmbiColorOverride", g.terrainvectorindex);
							SetVector4(g.terrainvectorindex, 1, 1, 1, 0);
							SetEffectConstantV(t.effectid, "AmbiColor", g.terrainvectorindex);
							SetEffectConstantF ( t.effectid,"SurfaceSunFactor", 1.0f );
							SetVector4 (  g.terrainvectorindex, 1.4f, 1.4f, 1.4f, 0.0f );
							SetEffectConstantV (  t.effectid,"SurfColor",g.terrainvectorindex );
						}
					}

					extern bool bImGuiInTestGame;
					bImGuiInTestGame = true; //just reuse this to prevent imgui rendering.
					FastSync();
					bImGuiInTestGame = false;

					// restore ambience for shot
					for (int iShaderIndex = 0; iShaderIndex < 2; iShaderIndex++)
					{
						if (iShaderIndex == 0) t.effectid = g.thirdpersonentityeffect;
						if (iShaderIndex == 1) t.effectid = g.thirdpersoncharactereffect;
						if (GetEffectExist(t.effectid) == 1)
						{
							SetVector4 (g.terrainvectorindex,t.visuals.AmbienceIntensity_f/255.0,t.visuals.AmbienceIntensity_f/255.0,t.visuals.AmbienceIntensity_f/255.0,t.visuals.AmbienceIntensity_f/255.0 );
							SetEffectConstantV ( t.effectid,"AmbiColorOverride",g.terrainvectorindex );
							SetVector4 (g.terrainvectorindex,t.visuals.AmbienceRed_f/255.0,t.visuals.AmbienceGreen_f/255.0,t.visuals.AmbienceBlue_f/255.0,0 );
							SetEffectConstantV ( t.effectid,"AmbiColor",g.terrainvectorindex );
							SetVector4 (  g.terrainvectorindex,t.visuals.SurfaceRed_f/255.0,t.visuals.SurfaceGreen_f/255.0,t.visuals.SurfaceBlue_f/255.0, 0.0f );
							SetEffectConstantV (  t.effectid,"SurfColor",g.terrainvectorindex );
							SetEffectConstantF ( t.effectid,"SurfaceSunFactor",t.visuals.SurfaceSunFactor_f );
						}
					}

					//Restore camera.
					t.editorfreeflight.c.x_f = oldx_f;
					t.editorfreeflight.c.y_f = oldy_f;
					t.editorfreeflight.c.z_f = oldz_f;
					t.editorfreeflight.c.angx_f = oldangx_f;
					t.editorfreeflight.c.angy_f = oldangy_f;

					PositionCamera(t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
					RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);

					PositionObject(iCharObj, ccpObjTargetX, ccpObjTargetY, ccpObjTargetZ);
					RotateObject(iCharObj, ccpObjTargetAX, ccpObjTargetAY, ccpObjTargetAZ);

					//Delayed  hide waypoints , as selecting entity mode will enable it.
					waypoint_hideall();
					t.inputsys.dowaypointview = 1;

					// delete previous thumbnail
					if (GetImageExistEx(g.importermenuimageoffset + 50))
					{
						DeleteImage(g.importermenuimageoffset + 50);
					}

					// we can't grab from the backbuffer when we use a camera image.
					extern DBPRO_GLOBAL CCameraManager m_CameraManager;
					DBPRO_GLOBAL tagCameraData* m_mycam;
					m_mycam = m_CameraManager.GetData(0);
					float thumbnail_dimension = 64;// 64; // are we prepared for V3 to use larger thumbnails (i.e. 128x128)? //PE: Sure, also a must for HDPI (modern laptops).
					if (m_mycam) 
					{
						extern GlobStruct* g_pGlob;
						LPGGSURFACE	pTmpSurface = g_pGlob->pCurrentBitmapSurface;

						g_pGlob->pCurrentBitmapSurface = m_mycam->pCameraToImageSurface;

						//PE: TODO this should be in percent , or atleast be based on the backbuffer size ?
						float fHalfThumb = (thumbnail_dimension*0.5);
						float fCamWidth = m_mycam->viewPort3D.Width*0.5;
						float fCamHeight = m_mycam->viewPort3D.Height*0.5;
						ImVec2 grab = ImVec2(fCamWidth, fCamHeight);
						grab.x += 3.0f;
						grab.y -= 10.0f;
						grab.y += iThumbsOffsetY;
						GrabImage(g.importermenuimageoffset + 50, grab.x - fHalfThumb, grab.y - fHalfThumb, grab.x + fHalfThumb, grab.y + fHalfThumb);

						g_pGlob->pCurrentBitmapSurface = pTmpSurface;
					}
					iDelayThumbs++;
				}
				else if (iDelayThumbs == 6)
				{
					// final stage
					g_bNoSwapchainPresent = false; //reenable backbuffer to hwnd
					iDelayThumbs++;
				}
				else
				{
					iDelayThumbs++;
				}
			}

			// handle preparing of animation data

			//Enable this to disable all movement ... when g_bCharacterCreatorPlusActivated
			extern int iGenralWindowsFlags;
			ImGui::Begin("Character Creator##PropertiesWindow", &g_bCharacterCreatorPlusActivated, iGenralWindowsFlags);

			if (once_camera_adjust)
			{
				extern ImVec2 OldrenderTargetSize;
				extern ImVec2 OldrenderTargetPos;
				extern ImVec2 renderTargetAreaSize;
				PositionCamera(t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
				RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);

				float camxadjust = renderTargetAreaSize.x - (ImGui::GetWindowPos().x - OldrenderTargetPos.x);

				if (camxadjust > 100.0f && camxadjust < GetDisplayWidth()) {
					camxadjust -= 100.0;
					camxadjust *= 0.068;
					MoveCameraLeft(g_pGlob->dwCurrentSetCameraID, -camxadjust);
					t.editorfreeflight.c.x_f = CameraPositionX();
					t.editorfreeflight.c.z_f = CameraPositionZ();;
				}
				once_camera_adjust = false;
			}
			int media_icon_size = 64; //96
			float col_start = 80.0f;
			ImGui::PushItemWidth(ImGui::GetFontSize()*10.0);

			if (ImGui::StyleCollapsingHeader("Name And Type", ImGuiTreeNodeFlags_DefaultOpen)) {
				
				float w = ImGui::GetWindowContentRegionWidth();
				//Icon removed
				//if (GetImageExistEx(g.importermenuimageoffset + 50)) {
				//
				//	ImVec2 ocp = ImGui::GetCursorPos();
				//	ImGui::SetCursorPos(ocp + ImVec2(w - 10.0f - media_icon_size, 0.0f));
				//	ImGui::ImgBtn(g.importermenuimageoffset + 50, ImVec2(media_icon_size, media_icon_size), ImColor(0, 0, 0, 255), ImColor(255, 255, 255, 255));
				//	ImGui::SetCursorPos(ocp);
				//}

				ImGui::Indent(10);
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 13)); //3
				ImGui::Text("Name");
				ImGui::SameLine();
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
				ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));
				//ImGui::PushItemWidth(-10 - media_icon_size - 10); //Icon removed
				ImGui::PushItemWidth(-10);
				//PE: TEST: We now check IsAnyItemActive() and disable input, so this check is not needed (keep until proper test)
				//if (!ImGui::IsWindowHovered())
				//	ImGui::InputText("##NameCCP", &CCP_Name[0], 250, ImGuiInputTextFlags_ReadOnly);
				//else

				ImGui::InputText("##NameCCP", &CCP_Name[0], 250);

				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set Character Name");

				ImGui::PopItemWidth();

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
				ImGui::Text("Type");
				ImGui::SameLine();
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
				ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));
				#ifdef PRODUCTV3
				 const char* items[] = { "Adult Male", "Adult Female", "Child Female", "Child Male" };
				#else
				 const char* items[] = { "Adult Male", "Adult Female", "Zombie Male" };
				#endif

				int item_current_type_selection = 0;
				for (int i = 0; i < 4; i++) 
				{
					if (pestrcasestr(CCP_Type, items[i])) 
					{
						item_current_type_selection = i;
						break;
					}
				}

				//ImGui::PushItemWidth(-10 - media_icon_size - 10); //Icon removed.
				ImGui::PushItemWidth(-10);
				if (ImGui::Combo("##TypeCCP", &item_current_type_selection, items, IM_ARRAYSIZE(items)))
				{
					strcpy(CCP_Type, items[item_current_type_selection]);
					iThumbsOffsetY = 0;
					if (item_current_type_selection == 2 || item_current_type_selection == 3) iThumbsOffsetY = 50;
					iDelayExecute = 1;
					DisplaySmallImGuiMessage("Loading ...");
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select Character Type");

				ImGui::PopItemWidth();

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 8)); //3

				ImGui::Indent(-10);
			}

			if (ImGui::StyleCollapsingHeader("Customize", ImGuiTreeNodeFlags_DefaultOpen))
			{
				static std::map<std::string, std::string> CharacterCreatorCurrent_s;
				static std::map<std::string, std::string> CharacterCreatorCurrentAnnotated_s;	
				static std::map<std::string, std::string> CharacterCreatorCurrentAnnotatedTag_s;		
				cstr field_name;
				char* combo_buffer = NULL;
				char* combo_annotated_buffer = NULL;
				int part_number = 0;
				for (int part_loop = 0; part_loop < 8; part_loop++) 
				{
					if (part_loop == 0)
					{
						CharacterCreatorCurrent_s = CharacterCreatorHeadGear_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedHeadGear_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagHeadGear_s;
						field_name = "Head Gear";
						LPSTR pAnnotatedLabel = "None";
						if ( strnicmp (cSelectedHeadGear, "None", 4)!=NULL ) pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedHeadGear);
						combo_buffer = cSelectedHeadGear;
						combo_annotated_buffer = pAnnotatedLabel;
						part_number = part_loop;
					}
					if (part_loop == 1)
					{
						CharacterCreatorCurrent_s = CharacterCreatorHair_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedHair_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagHair_s;
						field_name = "Hair";
						LPSTR pAnnotatedLabel = "None";
						if (strnicmp(cSelectedHair, "None", 4) != NULL) pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedHair);
						combo_buffer = cSelectedHair;
						combo_annotated_buffer = pAnnotatedLabel;
						part_number = part_loop;
					}
					if (part_loop == 2)
					{
						CharacterCreatorCurrent_s = CharacterCreatorHead_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedHead_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagHead_s;
						field_name = "Head";
						LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedHead);
						combo_buffer = cSelectedHead;
						combo_annotated_buffer = pAnnotatedLabel;
						part_number = part_loop;
					}
					if (part_loop == 3)
					{
						CharacterCreatorCurrent_s = CharacterCreatorEyeglasses_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedEyeglasses_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagEyeglasses_s;
						field_name = "Wearing";
						LPSTR pAnnotatedLabel = "None";
						if (strnicmp(cSelectedEyeglasses, "None", 4) != NULL) pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedEyeglasses);
						combo_buffer = cSelectedEyeglasses;
						combo_annotated_buffer = pAnnotatedLabel;
						part_number = part_loop;
					}
					if (part_loop == 4)
					{
						CharacterCreatorCurrent_s = CharacterCreatorFacialHair_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedFacialHair_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagFacialHair_s;
						field_name = "Facial Hair";
						LPSTR pAnnotatedLabel = "None";
						if (strnicmp(cSelectedFacialHair, "None", 4) != NULL) pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedFacialHair);
						combo_buffer = cSelectedFacialHair;
						combo_annotated_buffer = pAnnotatedLabel;
						part_number = part_loop;
					}
					if (part_loop == 5)
					{
						CharacterCreatorCurrent_s = CharacterCreatorBody_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedBody_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagBody_s;
						field_name = "Body";
						LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedBody); 
						combo_buffer = cSelectedBody;
						combo_annotated_buffer = pAnnotatedLabel;						
						part_number = part_loop;
					}
					if (part_loop == 6) 
					{
						CharacterCreatorCurrent_s = CharacterCreatorLegs_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedLegs_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagLegs_s;
						field_name = "Legs";
						// before allowng selected legs through, check they comply with our cSelectedLegsFilter filter
						bool bAllow = false;
						LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedLegs); 
						if (strlen(cSelectedLegsFilter) == 0 && strnicmp ( cSelectedLegs + strlen(cSelectedLegs) - 2, "01", 2 ) != NULL) bAllow = true;
						if (strlen(cSelectedLegsFilter) > 0 && pAnnotatedLabel && strstr(pAnnotatedLabel, cSelectedLegsFilter) != NULL) bAllow = true;
						if ( bAllow == true )
						{
							// no filter so allow, or filter matches, so also allow
						}
						else
						{
							// this current legs selection no longer matches filter, so change to one that does
							// starting with the top-most item and working down
							std::map<std::string, std::string>::iterator annotated = CharacterCreatorCurrentAnnotated_s.begin(); 
							for (std::map<std::string, std::string>::iterator it = CharacterCreatorCurrent_s.begin(); it != CharacterCreatorCurrent_s.end(); ++it)
							{
								std::string thisname = it->first;
								std::string thistag = annotated->second;
								bool bThisAllow = false;
								LPSTR pThisName = (char*)thisname.c_str();
								if (strlen(cSelectedLegsFilter) == 0 && strnicmp ( pThisName + strlen(pThisName) - 2, "01", 2 ) != NULL) bThisAllow = true;
								if (strlen(cSelectedLegsFilter) > 0 && strstr(thistag.c_str(), cSelectedLegsFilter) != NULL) bThisAllow = true;
								if ( bThisAllow == true )
								{
									// found first (or one matching the filter)
									strcpy(cSelectedLegs, thisname.c_str());
									strcpy(cSelectedFeetFilter, "");
									g_bLegsChangeCascade = true;
									g_bFeetChangeCascade = true;
									break;
								}
								annotated++;
							}
							pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedLegs);
						}
						// continue with selected legs as normal now
						combo_buffer = cSelectedLegs;
						combo_annotated_buffer = pAnnotatedLabel;						
						part_number = part_loop;
					}
					if (part_loop == 7) 
					{
						CharacterCreatorCurrent_s = CharacterCreatorFeet_s;
						CharacterCreatorCurrentAnnotated_s = CharacterCreatorAnnotatedFeet_s;
						CharacterCreatorCurrentAnnotatedTag_s = CharacterCreatorAnnotatedTagFeet_s;
						field_name = "Feet";
						// before allowng selected feet through, check they comply with our filter
						bool bAllow = false;
						LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedFeet);
						LPSTR pAnnotatedLabelTag = charactercreatorplus_findannotationtag(cSelectedFeet); 
						if (strlen(cSelectedFeetFilter) == 0 && (pAnnotatedLabelTag==NULL || strlen(pAnnotatedLabelTag) == 0) ) bAllow = true;
						if (strlen(cSelectedFeetFilter) > 0 && pAnnotatedLabelTag && strstr(pAnnotatedLabelTag, cSelectedFeetFilter) != NULL) bAllow = true;
						if ( bAllow == true )
						{
							// no filter so allow, or filter matches, so also allow
						}
						else
						{
							// this current feet selection no longer matches filter, so change to one that does
							// starting with the top-most item and working down
							std::map<std::string, std::string>::iterator annotatedtag = CharacterCreatorCurrentAnnotatedTag_s.begin(); 
							for (std::map<std::string, std::string>::iterator it = CharacterCreatorCurrent_s.begin(); it != CharacterCreatorCurrent_s.end(); ++it)
							{
								std::string thisname = it->first;
								std::string thistag = annotatedtag->second;
								bool bThisAllow = false;
								LPSTR pThisName = (char*)thisname.c_str();
								if (strlen(cSelectedFeetFilter) == 0 && strlen(thistag.c_str())==0) bThisAllow = true;
								if (strlen(cSelectedFeetFilter) > 0 && strstr(thistag.c_str(), cSelectedFeetFilter) != NULL) bThisAllow = true;
								if ( bThisAllow == true )
								{
									// found first (or one matching the filter)
									strcpy(cSelectedFeet, thisname.c_str());
									g_bFeetChangeCascade = true;
									break;
								}
								annotatedtag++;
							}
							pAnnotatedLabel = charactercreatorplus_findannotation(cSelectedFeet);
						}
						combo_buffer = cSelectedFeet;
						combo_annotated_buffer = pAnnotatedLabel;						
						part_number = part_loop;
					}
					if (!CharacterCreatorCurrent_s.empty() && CharacterCreatorCurrent_s.size()>1) 
					{
						ImGui::Indent(10);

						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
						ImGui::Text(field_name.Get());
						ImGui::SameLine();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));

						ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));

						float Color_gadget_size = ImGui::GetFontSize()*2.0;

						ImGui::PushItemWidth(-10);

						cstr unique_label = "##CCP";
						unique_label += field_name;
						if (ImGui::BeginCombo(unique_label.Get(), combo_annotated_buffer)) // The second parameter is the label previewed before opening the combo.
						{
							std::map<std::string, std::string>::iterator annotated = CharacterCreatorCurrentAnnotated_s.begin(); 
							std::map<std::string, std::string>::iterator annotatedtag = CharacterCreatorCurrentAnnotatedTag_s.begin(); 
							for (std::map<std::string, std::string>::iterator it = CharacterCreatorCurrent_s.begin(); it != CharacterCreatorCurrent_s.end(); ++it)
							{
								std::string full_path = it->second;
								std::string name = it->first;

								// only allow if part has no filter or filter within name
								bool bThisAllow = false;
								if (part_number == 6 || part_number == 7)
								{
									LPSTR pThisName = (char*)name.c_str();
									if (part_number == 6)
									{
										// only allow specific legs
										LPSTR pThisAnnotatedName = (char*)annotated->second.c_str();
										if (strlen(cSelectedLegsFilter) == 0 && strnicmp(pThisName + strlen(pThisName) - 2, "01", 2) != NULL) bThisAllow = true;
										if (strlen(cSelectedLegsFilter) > 0 && strstr(pThisAnnotatedName, cSelectedLegsFilter) != NULL) bThisAllow = true;
									}
									if (part_number == 7)
									{
										// only allow specific feet
										LPSTR pThisAnnotatedTagName = (char*)annotatedtag->second.c_str();
										if (strlen(cSelectedFeetFilter) == 0 && (strlen(pThisAnnotatedTagName) == 0 )) bThisAllow = true;
										if (strlen(cSelectedFeetFilter) > 0 && strstr(pThisAnnotatedTagName, cSelectedFeetFilter) != NULL) bThisAllow = true;
									}
								}
								else
								{
									// all other parts have a free pass!
									bThisAllow = true;
								}
								if (bThisAllow == true)
								{
									// mark the one selected
									bool is_selected = false;
									if (strcmp(name.c_str(), combo_buffer) == 0)
										is_selected = true;

									// the label we see
									std::string annotated_label = annotated->second;
									std::string annotatedtag_label = annotatedtag->second;

									// when something selected
									if (ImGui::Selectable(annotated_label.c_str(), is_selected))
									{
										// we need to wait for any previously requested preloads to exist before we can go on to make the character
										object_preload_files_wait(); // dont need to wait for image preload, image preload is thread safe and can overlap normal DX operations
										// Change Character. full_path.c_str()
										strcpy(combo_buffer, name.c_str());
										// instead of instant change, record change we want and fire off some preloads for smooth UI
										charactercreatorplus_preparechange((char *)full_path.c_str(), part_number, (char *)annotatedtag_label.c_str());
										if (part_number == 2)
										{
											// and set the IC for reference when its time to save the character assembly info
											strcpy(cSelectedICCode, (char *)annotatedtag_label.c_str());
										}
										if (part_number == 5)
										{
											// if body requires NO LEGS (or something else), set this condition for other dropdowns
											// and force any current legs to conform
											strcpy(cSelectedLegsFilter, (char *)annotatedtag_label.c_str());
										}
										if (part_number == 6)
										{
											// if legs requires NO FEET (or something else), set this condition for other dropdowns
											// and force any current feet to conform
											strcpy(cSelectedFeetFilter, (char *)annotatedtag_label.c_str());
										}
										// Update thumbnail after change.
										iDelayThumbs = 0; // Regen thumnbnail
									}
									if (is_selected)
										ImGui::SetItemDefaultFocus();
								}

								// advance annotated list with real item list
								annotated++;
								annotatedtag++;
							}
							ImGui::EndCombo();
						}

						if (ImGui::IsItemHovered()) {
							cstr unique_tooltip = "Select Character ";
							unique_tooltip += field_name;
							ImGui::SetTooltip(unique_tooltip.Get());
						}

						ImGui::PopItemWidth();
						ImGui::Indent(-10);

						// also update character during a cascade (body changes legs, which changes feet)
						if ((g_bLegsChangeCascade == true || g_bFeetChangeCascade == true) && g_charactercreatorplus_preloading == false)
						{
							// and ensure all thread activity ends before we push this (or it may delay long enough to change base type again!)
							image_preload_files_wait();
							object_preload_files_wait();

							// a faster single pass option
							std::map<std::string, std::string>::iterator it = CharacterCreatorCurrent_s.begin();
							std::string full_path = it->second;
							charactercreatorplus_preparechange((char*)full_path.c_str(), 67, "");
							g_bLegsChangeCascade = false;
							g_bFeetChangeCascade = false;
						}
						CharacterCreatorCurrent_s.clear();
					}
				}


				//ZJ: Updated to new gadget style (below).
				//PE: Rotate bCharObjVisible.
				//ImGui::Indent(10);
				//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
				//ImGui::Text("Rotate");
				//ImGui::SameLine();
				//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
				//ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));
				//float Color_gadget_size = ImGui::GetFontSize()*2.0;

				//ImGui::PushItemWidth(-10);
				//if (ImGui::SliderFloat("##fCCPRotate:", &fCCPRotateY, 0.0, 360.0)) {
				//	RotateObject(iCharObj, ObjectAngleX(iCharObj), fCCPRotateY, ObjectAngleZ(iCharObj));
				//	ccpObjTargetAY = fCCPRotateY;
				//}
				//if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rotate Character");
				//ImGui::PopItemWidth();
				//ImGui::Indent(-10);

				//	Rotate the character.
				ImGui::TextCenter("Rotate");
				ImGui::Indent(10.0f);
				if (ImGui::MaxSliderInputFloat("##CharacterRotation", &fCCPRotateY, 0.0f, 360.0f, "Rotate Character", 0.0f, 360.0f))
				{
					RotateObject(iCharObj, ObjectAngleX(iCharObj), fCCPRotateY, ObjectAngleZ(iCharObj));
					ccpObjTargetAY = fCCPRotateY;
				}
				ImGui::Indent(-10.0f);

			}


			if (ImGui::StyleCollapsingHeader("Character Details", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(10);

				//Only if we actually have sapi and a installed lang pack.
				if (g_voiceList_s.size() > 0) {

					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
					ImGui::Text("Voice");
					ImGui::SameLine();
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
					//Combo
					ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));
					ImGui::PushItemWidth(-10);
					if (ImGui::BeginCombo("##SelectVoiceCCP", pCCPVoiceSet)) // The second parameter is the label previewed before opening the combo.
					{
						int size = g_voiceList_s.size();
						for (int vloop = 0; vloop < size; vloop++) {

							bool is_selected = false;
							if (strcmp(g_voiceList_s[vloop].Get(), pCCPVoiceSet) == 0)
								is_selected = true;

							if (ImGui::Selectable(g_voiceList_s[vloop].Get(), is_selected)) {
								//Change Voice set
								pCCPVoiceSet = g_voiceList_s[vloop].Get();
								CCP_SelectedToken = g_voicetoken[vloop];
							}
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select Voice Set");

					ImGui::PopItemWidth();
				}

				ImGui::Indent(-10); //unindent before center.
			}

			if (ImGui::StyleCollapsingHeader("Save Character", ImGuiTreeNodeFlags_DefaultOpen)) {

				ImGui::Indent(10);

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
				ImGui::Text("Path");
				ImGui::SameLine();
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));

				ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));
				//ImGui::PushItemWidth(-10);

				float path_gadget_size = ImGui::GetFontSize()*2.0;

				ImGui::PushItemWidth(-10 - path_gadget_size);
				//PE: TEST: We now check IsAnyItemActive() and disable input, so this check is not needed (keep until proper test)
				//if (!ImGui::IsWindowHovered())
				//	ImGui::InputText("##InputPathCCP", &CCP_Path[0], 250, ImGuiInputTextFlags_ReadOnly);
				//else

				ImGui::InputText("##InputPathCCP", &CCP_Path[0], 250);
				
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set Where to Save Your Character");

				ImGui::PopItemWidth();
				//	Let the user know they set an invalid save file path.
				if (ImGui::BeginPopup("##CCPInvalidSavePath"))
				{
					ImGui::Text("Path must be within 'Max\\Files\\entitybank\\user\\'");
					ImGui::EndPopup();
				}

				ImGui::SameLine();
				ImGui::PushItemWidth(path_gadget_size);
				if (ImGui::StyleButton("...##ccppath")) {
					//PE: filedialogs change dir so.
					cStr tOldDir = GetDir();
					char * cFileSelected;
					cstr fulldir = tOldDir + "\\entitybank\\user\\"; //"\\entitybank\\user\\charactercreatorplus\\";
					cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_DIR, "All\0*.*\0", fulldir.Get(), "", true, NULL);

					SetDir(tOldDir.Get());

					if (cFileSelected && strlen(cFileSelected) > 0) {

						//	Check that the new path still contains the entitybank folder.
						char* cCropped = strstr(cFileSelected, "\\entitybank\\user");
						if (cCropped)
						{
							//	New location contains entitybank folder, so change the import path.
							strcpy(CCP_Path, cFileSelected);
						}
						else
						{
							ImGui::OpenPopup("##CCPInvalidSavePath");
						}
					}
					// ZJ: Implementing in the same way as the importer, for consistency.
					//if (cFileSelected && strlen(cFileSelected) > 0) {
					//	strcpy(CCP_Path, cFileSelected);
					//}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select Where to Save Your Character");

				ImGui::PopItemWidth();

				ImGui::Indent(-10); //unindent before center.
				float save_gadget_size = ImGui::GetFontSize()*10.0;
				float w = ImGui::GetWindowContentRegionWidth();
				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (save_gadget_size*0.5) , 0.0f));

				if (ImGui::StyleButton("Save Character##butsave", ImVec2(save_gadget_size,0)) )
				{
					if (strlen(CCP_Name) > 0) 
					{
						if (strlen(CCP_Path) > 0) 
						{
							// save character FPE
							g_CharacterCreatorPlus.obj.settings.script_s = CCP_Script;
							g_CharacterCreatorPlus.obj.settings.voice_s = pCCPVoiceSet;
							g_CharacterCreatorPlus.obj.settings.iSpeakRate = CCP_Speak_Rate;
							int iCharObj = g.characterkitobjectoffset + 1;
							cstr pFillFilename = cstr(CCP_Path) + CCP_Name + ".dbo";
							if ( charactercreatorplus_savecharacterentity ( iCharObj, pFillFilename.Get(), g.importermenuimageoffset + 50 ) == true )
							{
								strcpy(cTriggerMessage,"Character Saved");
								bTriggerMessage = true;

							}
						}
						else 
						{
							strcpy(cTriggerMessage, "Please select a path where you like the character saved.");
							bTriggerMessage = true;
						}
					}
					else 
					{
						strcpy(cTriggerMessage, "You must give your character a name before you can save it.");
						bTriggerMessage = true;
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Save Your Character");

			}

			if (!pref.bHideTutorials)
			{
				if (ImGui::StyleCollapsingHeader("Tutorial (this feature is incomplete)", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent(10);
					void SmallTutorialVideo(char *tutorial, char* combo_items[] = NULL, int combo_entries = 0, int iVideoSection = 0);
					cstr cShowTutorial = "03 - Add character and set a path";
					char* tutorial_combo_items[] = { "01 - Getting started", "02 - Creating terrain", "03 - Add character and set a path" };
					SmallTutorialVideo(cShowTutorial.Get(), tutorial_combo_items, ARRAYSIZE(tutorial_combo_items), SECTION_CHARACTER_CREATOR);
					float but_gadget_size = ImGui::GetFontSize()*12.0;
					float w = ImGui::GetWindowContentRegionWidth() - 10.0;
					ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (but_gadget_size*0.5), 0.0f));
					#ifdef INCLUDESTEPBYSTEP
					if (ImGui::StyleButton("View Step by Step Tutorial", ImVec2(but_gadget_size, 0)))
					{
						// pre-select tutorial 03
						extern bool bHelpVideo_Window;
						extern bool bHelp_Window;
						extern char cForceTutorialName[1024];
						bHelp_Window = true;
						bHelpVideo_Window = true;
						extern bool bSetTutorialSectionLeft;
						bSetTutorialSectionLeft = false;
						strcpy(cForceTutorialName, cShowTutorial.Get());
					}
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Start Step by Step Tutorial");
					#endif

					ImGui::Indent(-10);
				}
			}

			// insert a keyboard shortcut component into panel
			UniversalKeyboardShortcut(eKST_CharacterCreator);

			ImGui::PopItemWidth();

			void CheckMinimumDockSpaceSize(float minsize);
			CheckMinimumDockSpaceSize(250.0f);

			if (ImGui::GetCurrentWindow()->ScrollbarSizes.x > 0) {
				//Hitting exactly at the botton could cause flicker, so add some additional lines when scrollbar on.
				ImGui::Text("");
				ImGui::Text("");
			}

			ImGui::End();
		}
	}
	else 
	{
		if ( g_CharacterCreatorPlus.bInitialised ) 
		{
			//Make sure we hide ccp
			if (bCharObjVisible && ObjectExist(iCharObj)) 
			{

				// first, erase preloaded files we dont need any more (and load in basics for when return to CCP)
				image_preload_files_reset();
				object_preload_files_wait(); // If it is still working on loading, it will crash when data is reset.
				object_preload_files_reset();
				charactercreatorplus_preloadinitialcharacter();

				// hide character creator model
				HideObject(iCharObj);
				bCharObjVisible = false;

				t.inputsys.dowaypointview = 0;

				//Restore.
				waypoint_restore();

				t.gridentityhidemarkers = 0;
				editor_updatemarkervisibility();
				editor_refresheditmarkers();

				//  put all entities back where they were
				for (t.tcce = 1; t.tcce <= g.entityelementlist; t.tcce++)
				{
					t.tccentid = t.entityelement[t.tcce].bankindex;
					if (t.tccentid > 0)
					{
						t.tccsourceobj = t.entityelement[t.tcce].obj;
						if (ObjectExist(t.tccsourceobj) == 1)
						{
							PositionObject(t.tccsourceobj, t.entityelement[t.tcce].x, t.entityelement[t.tcce].y, t.entityelement[t.tcce].z);
						}
					}
				}
				//if (ObjectExist(t.terrain.objectstartindex + 4)) {
				//	HideObject(t.terrain.objectstartindex + 4);
				//}

				//Restore editor camera.
				if (editoroldx_f != 0) //PE: Somehow editoroldx_f was 0 ?
				{
					t.editorfreeflight.mode = editoroldmode_f;
					t.editorfreeflight.c.x_f = editoroldx_f;
					t.editorfreeflight.c.y_f = editoroldy_f;
					t.editorfreeflight.c.z_f = editoroldz_f;
					t.editorfreeflight.c.angx_f = editoroldangx_f;
					t.editorfreeflight.c.angy_f = editoroldangy_f;
					PositionCamera(t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
					RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);
				}
			}
		}
	}
}

void charactercreatorplus_preparechange(char *path, int part, char* tag)
{
	g_charactercreatorplus_preloading = true;
	strcpy(g_charactercreatorplus_path, path);
	g_charactercreatorplus_part = part;
	strcpy(g_charactercreatorplus_tag, tag);
	DisplaySmallImGuiMessage("Loading ...");
}

void charactercreatorplus_waitforpreptofinish(void)
{
	if (g_charactercreatorplus_preloading == true)
	{
		DisplaySmallImGuiMessage("Loading ...");
	}
	if (g_charactercreatorplus_preloading == true)
	{
		if (image_preload_files_in_progress()==false && object_preload_files_in_progress()==false)
		{
			image_preload_files_wait();
			object_preload_files_wait();
			charactercreatorplus_change(g_charactercreatorplus_path, g_charactercreatorplus_part, g_charactercreatorplus_tag);
			charactercreatorplus_preloadallcharacterpartchoices();
			g_charactercreatorplus_preloading = false;
		}
	}
}

void charactercreatorplus_refreshskincolor(void)
{
	// only needed to do for body, legs and feet
	int iCharTexture = g.charactercreatorEditorImageoffset + 1;
	int iCharLegsTexture = g.charactercreatorEditorImageoffset + 61;
	int iCharFeetTexture = g.charactercreatorEditorImageoffset + 71;
	int iCharSkinTexture = g.charactercreatorEditorImageoffset + 101;
	
	// find a free work memblock
	int iMemblockAlbedoID = 32; while (MemblockExist(iMemblockAlbedoID) == 1) iMemblockAlbedoID++;
	int iMemblockMaskID = 33; while (MemblockExist(iMemblockMaskID) == 1) iMemblockMaskID++;
	int iMemblockSkinID = 34; while (MemblockExist(iMemblockSkinID) == 1) iMemblockSkinID++;

	// for each relevant body part
	for ( int partnum = 0; partnum < 3; partnum++ )
	{
		// determine which body part to work on
		int iPartObj, iAlbedoTexture;
		if (partnum == 0) { iPartObj = iCharObj; iAlbedoTexture = iCharTexture + 0; }
		if (partnum == 1) { iPartObj = iCharObjLegs; iAlbedoTexture = iCharLegsTexture + 0; }
		if (partnum == 2) { iPartObj = iCharObjFeet; iAlbedoTexture = iCharFeetTexture + 0; }
		int iMaskTexture;
		if (partnum == 0) { iMaskTexture = iCharTexture + 5; }
		if (partnum == 1) { iMaskTexture = iCharLegsTexture + 5; }
		if (partnum == 2) { iMaskTexture = iCharFeetTexture + 5; }

		// ensure mask available
		if (ImageExist(iMaskTexture))
		{
			// make sure images are not compressed (i.e. DXT1-5), but regular XRGB so we can read them
			char pNewTempAlbedoTextureFile[MAX_PATH];
			sprintf(pNewTempAlbedoTextureFile, "charactercreatorplus\\skins\\tempfinalalbedo.png");
			GG_GetRealPath(pNewTempAlbedoTextureFile, 1);
			SaveImage(pNewTempAlbedoTextureFile, iAlbedoTexture);
			DeleteImage(iAlbedoTexture);
			LoadImage(pNewTempAlbedoTextureFile, iAlbedoTexture);
			char pNewTempMaskTextureFile[MAX_PATH];
			sprintf(pNewTempMaskTextureFile, "charactercreatorplus\\skins\\tempfinalmask.png");
			GG_GetRealPath(pNewTempMaskTextureFile, 1);
			SaveImage(pNewTempMaskTextureFile, iMaskTexture);
			DeleteImage(iMaskTexture);
			LoadImage(pNewTempMaskTextureFile, iMaskTexture);

			// load image data into appropriate memblocks
			if (MemblockExist(iMemblockAlbedoID) == 1) DeleteMemblock(iMemblockAlbedoID);
			if (MemblockExist(iMemblockMaskID) == 1) DeleteMemblock(iMemblockMaskID);
			if (MemblockExist(iMemblockSkinID) == 1) DeleteMemblock(iMemblockSkinID);
			CreateMemblockFromImage(iMemblockAlbedoID, iAlbedoTexture);
			CreateMemblockFromImage(iMemblockMaskID, iMaskTexture);
			CreateMemblockFromImage(iMemblockSkinID, iCharSkinTexture);

			// skin mask may have different resolution
			int imgSkinMaskWidth = ReadMemblockDWord(iMemblockMaskID, 0);
			int imgSkinMaskHeight = ReadMemblockDWord(iMemblockMaskID, 4);
			int imgSkinMaskDepth = ReadMemblockDWord(iMemblockMaskID, 8);
			int imgSkinMaskSize = imgSkinMaskWidth * imgSkinMaskHeight * imgSkinMaskDepth;
			int imgSkinMaskOffset = 4 * 3;

			// and skin ref may be different size also (usually 2K)
			int imgSkinWidth = ReadMemblockDWord(iMemblockSkinID, 0);
			int imgSkinHeight = ReadMemblockDWord(iMemblockSkinID, 4);
			int imgSkinDepth = ReadMemblockDWord(iMemblockSkinID, 8);
			int imgSkinSize = imgSkinWidth * imgSkinHeight * imgSkinDepth;
			int imgSkinOffset = 4 * 3;

			// create new albedo from mask and skin texture
			int imgWidth = ReadMemblockDWord(iMemblockAlbedoID, 0);
			int imgHeight = ReadMemblockDWord(iMemblockAlbedoID, 4);
			int imgDepth = ReadMemblockDWord(iMemblockAlbedoID, 8);
			int imgSize = imgWidth * imgHeight * imgDepth;
			int imgOffset = 4 * 3;

			// used to calculate actual skinmask coords offset
			float imgSkinMaskXDiv = (float)imgSkinMaskWidth / (float)imgWidth;
			float imgSkinMaskYDiv = (float)imgSkinMaskHeight / (float)imgHeight;
			float imgSkinXDiv = (float)imgSkinWidth / (float)imgWidth;
			float imgSkinYDiv = (float)imgSkinHeight / (float)imgHeight;

			// replace all skin pixels using mask
			for (int y = 0; y < imgHeight - 1; y++)
			{
				for (int x = 0; x < imgWidth - 1; x++)
				{
					// skin mask can be different size than color texture (i.e. 1K mask + 4K color texture)
					if (imgSize != imgSkinMaskSize)
					{
						int iXOffset = x*imgSkinMaskXDiv;
						int iYOffset = y*imgSkinMaskYDiv;
						int iXYOffset = iXOffset + (iYOffset*(imgSkinMaskWidth-1));
						imgSkinMaskOffset = (4 * 3) + (iXYOffset *4);
					}
					else
					{
						imgSkinMaskOffset = imgOffset;
					}
					if (imgSize != imgSkinSize)
					{
						int iXOffset = x * imgSkinXDiv;
						int iYOffset = y * imgSkinYDiv;
						int iXYOffset = iXOffset + (iYOffset*(imgSkinWidth-1));
						imgSkinOffset = (4 * 3) + (iXYOffset * 4);
					}
					else
					{
						imgSkinOffset = imgOffset;
					}
					int pixelCol = ReadMemblockDWord(iMemblockAlbedoID, imgOffset);
					int maskColR = ReadMemblockByte(iMemblockMaskID, imgSkinMaskOffset+0);
					int maskColG = ReadMemblockByte(iMemblockMaskID, imgSkinMaskOffset+1);
					int maskColB = ReadMemblockByte(iMemblockMaskID, imgSkinMaskOffset+2);
					int maskColA = ReadMemblockByte(iMemblockMaskID, imgSkinMaskOffset+3);
					int skinCol = ReadMemblockDWord(iMemblockSkinID, imgSkinOffset);
					if (maskColR != 255 || maskColG != 255 || maskColB != 255 || maskColA != 255)
					{
						// for some reason detecting red at zero did not work
						// need some more time to find out why the mask memblock returning strange values
						// i.e. 255,255,247,255 for the non-red part (255,255,255,255 for the rest)
					}
					else
					{
						// if mask red channel full, allow skin pixel to bake into main character texture
						WriteMemblockDWord(iMemblockAlbedoID, imgOffset, skinCol);
					}
					imgOffset = imgOffset + 4;
				}
			}

			// delete temp files
			DeleteFileA(pNewTempAlbedoTextureFile);
			DeleteFileA(pNewTempMaskTextureFile);

			// save the file
			sprintf(pNewTempAlbedoTextureFile, "charactercreatorplus\\skins\\tempfinalalbedo%d.dds", partnum);
			GG_GetRealPath(pNewTempAlbedoTextureFile, 1);
			DeleteImage(iAlbedoTexture);
			CreateImageFromMemblock(iAlbedoTexture, iMemblockAlbedoID);
			SaveImage(pNewTempAlbedoTextureFile, iAlbedoTexture);


			// load the file and apply to the object
			LoadImage(pNewTempAlbedoTextureFile, iAlbedoTexture);
			TextureObject(iPartObj, 0, iAlbedoTexture);
		}
	}
	if (MemblockExist(iMemblockAlbedoID) == 1) DeleteMemblock(iMemblockAlbedoID);
	if (MemblockExist(iMemblockMaskID) == 1) DeleteMemblock(iMemblockMaskID);
	if (MemblockExist(iMemblockSkinID) == 1) DeleteMemblock(iMemblockSkinID);
}


void charactercreatorplus_change(char *path, int part, char* tag)
{
	// need legacy loading for image reskinning work
	image_setlegacyimageloading(true);

	// part = 67 is a special code to update both legs and feet (for cascade system when body/legs force other part changes)
	int iCharTexture = g.charactercreatorEditorImageoffset + 1;
	int iCharHeadGearTexture = g.charactercreatorEditorImageoffset + 11;
	int iCharHairTexture = g.charactercreatorEditorImageoffset + 21;
	int iCharHeadTexture = g.charactercreatorEditorImageoffset + 31;
	int iCharEyeglassesTexture = g.charactercreatorEditorImageoffset + 41;
	int iCharFacialHairTexture = g.charactercreatorEditorImageoffset + 51;
	int iCharLegsTexture = g.charactercreatorEditorImageoffset + 61;
	int iCharFeetTexture = g.charactercreatorEditorImageoffset + 71;

	// skin override texture
	int iCharSkinTexture = g.charactercreatorEditorImageoffset + 101;

	// free main character object to recreate here
	if (ObjectExist(iCharObj)) DeleteObject(iCharObj);

	// load generic white and black texture
	int iCharTextureWhite = g.charactercreatorEditorImageoffset + 0;
	if (!GetImageExistEx(iCharTextureWhite)) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharTextureWhite);
	int iCharTextureBlack = g.charactercreatorEditorImageoffset - 1;
	if (!GetImageExistEx(iCharTextureBlack)) LoadImage("effectbank\\reloaded\\media\\blank_black.dds", iCharTextureBlack);

	// final part name
	cstr final_name = path, tmp;

	// detect color variant
	int iEnd = 0;
	char cLast = 0;
	char cUseHeadGear[260];
	char cUseHair[260];
	char cUseHead[260];
	char cUseEyeglasses[260];
	char cUseFacialHair[260];
	char cUseBody[260];
	char cUseLegs[260];
	char cUseFeet[260];
	strcpy ( cUseHeadGear, cSelectedHeadGear );
	strcpy ( cUseHair, cSelectedHair);
	strcpy ( cUseHead, cSelectedHead);
	strcpy(cUseEyeglasses, cSelectedEyeglasses);
	strcpy(cUseFacialHair, cSelectedFacialHair);
	strcpy ( cUseBody, cSelectedBody );
	strcpy ( cUseLegs, cSelectedLegs );
	strcpy ( cUseFeet, cSelectedFeet );
	char cSelectedVariantHeadGear[260];
	char cSelectedVariantHair[260];
	char cSelectedVariantHead[260];
	char cSelectedVariantEyeglasses[260];
	char cSelectedVariantFacialHair[260];
	char cSelectedVariantBody[260];
	char cSelectedVariantLegs[260];
	char cSelectedVariantFeet[260];
	strcpy ( cSelectedVariantHeadGear, cSelectedHeadGear );
	strcpy ( cSelectedVariantHair, cSelectedHair);
	strcpy ( cSelectedVariantHead, cSelectedHead);
	strcpy(cSelectedVariantEyeglasses, cSelectedEyeglasses);
	strcpy(cSelectedVariantFacialHair, cSelectedFacialHair);
	strcpy ( cSelectedVariantBody, cSelectedBody );
	strcpy ( cSelectedVariantLegs, cSelectedLegs );
	strcpy ( cSelectedVariantFeet, cSelectedFeet );
	iEnd = strlen(cSelectedHeadGear) - 1; cLast = cSelectedHeadGear[iEnd]; if (iEnd > 3 && cLast >= 'a' && cLast <= 'z') { cUseHeadGear[iEnd] = 0; }
	iEnd = strlen(cSelectedHair) - 1; cLast = cSelectedHair[iEnd]; if (iEnd > 3 && cLast >= 'a' && cLast <= 'z') { cUseHair[iEnd] = 0; }
	iEnd = strlen(cSelectedHead) - 1; cLast = cSelectedHead[iEnd]; if (iEnd > 3 && cLast >= 'a' && cLast <= 'z') { cUseHead[iEnd] = 0; }
	iEnd = strlen(cSelectedEyeglasses) - 1; cLast = cSelectedEyeglasses[iEnd]; if (iEnd > 3 && cLast >= 'a' && cLast <= 'z') { cUseEyeglasses[iEnd] = 0; }
	iEnd = strlen(cSelectedFacialHair) - 1; cLast = cSelectedFacialHair[iEnd]; if (iEnd > 3 && cLast >= 'a' && cLast <= 'z') { cUseFacialHair[iEnd] = 0; }
	iEnd = strlen(cSelectedBody)-1; cLast = cSelectedBody[iEnd]; if (iEnd > 3 && cLast  >= 'a' && cLast <= 'z' ) { cUseBody[iEnd] = 0; }
	iEnd = strlen(cSelectedLegs)-1; cLast = cSelectedLegs[iEnd]; if (iEnd > 3 && cLast  >= 'a' && cLast <= 'z' ) { cUseLegs[iEnd] = 0; }
	iEnd = strlen(cSelectedFeet)-1; cLast = cSelectedFeet[iEnd]; if (iEnd > 3 && cLast  >= 'a' && cLast <= 'z' ) { cUseFeet[iEnd] = 0; }

	//Load all objects.
	if (strnicmp(cUseHeadGear, "None", 4) != NULL)
	{
		tmp = final_name + cUseHeadGear + CCPMODELEXT;
		LoadObject(tmp.Get(), iCharObjHeadGear);
	}
	else
	{
		if (ObjectExist(iCharObjHeadGear)) DeleteObject(iCharObjHeadGear);
	}
	if (strnicmp(cUseHair, "None", 4) != NULL)
	{
		tmp = final_name + cUseHair + CCPMODELEXT;
		LoadObject(tmp.Get(), iCharObjHair);
	}
	else
	{
		if (ObjectExist(iCharObjHair)) DeleteObject(iCharObjHair);
	}
	tmp = final_name + cUseHead + CCPMODELEXT;
	LoadObject(tmp.Get(), iCharObjHead);
	if (strnicmp(cUseEyeglasses, "None", 4) != NULL)
	{
		tmp = final_name + cUseEyeglasses + CCPMODELEXT;
		LoadObject(tmp.Get(), iCharObjEyeglasses);
	}
	else
	{
		if (ObjectExist(iCharObjEyeglasses)) DeleteObject(iCharObjEyeglasses);
	}
	if (strnicmp(cUseFacialHair, "None", 4) != NULL)
	{
		tmp = final_name + cUseFacialHair + CCPMODELEXT;
		LoadObject(tmp.Get(), iCharObjFacialHair);
	}
	else
	{
		if (ObjectExist(iCharObjFacialHair)) DeleteObject(iCharObjFacialHair);
	}
	tmp = final_name + cUseBody + CCPMODELEXT;
	LoadObject(tmp.Get(), iCharObj);
	tmp = final_name + cUseLegs + CCPMODELEXT;
	LoadObject(tmp.Get(), iCharObjLegs);
	tmp = final_name + cUseFeet + CCPMODELEXT;
	LoadObject(tmp.Get(), iCharObjFeet);

	// load all required textures
	if ((part == 0 || part==-1) && ObjectExist(iCharObjHeadGear)==1 )
	{ 
		//HeadGear
		for (int a = 0; a < 5; a++)
			if (GetImageExistEx(iCharHeadGearTexture + a)) DeleteImage(iCharHeadGearTexture + a);

		tmp = final_name + cSelectedVariantHeadGear + "_color.dds";
		LoadImage(tmp.Get(), iCharHeadGearTexture + 0);
		tmp = final_name + cUseHeadGear + "_ao.dds";
		LoadImage(tmp.Get(), iCharHeadGearTexture + 1);
		if (ImageExist(iCharHeadGearTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharHeadGearTexture + 1);
		tmp = final_name + cUseHeadGear + "_normal.dds";
		LoadImage(tmp.Get(), iCharHeadGearTexture + 2);
		tmp = final_name + cUseHeadGear + "_metalness.dds";
		LoadImage(tmp.Get(), iCharHeadGearTexture + 3);
		tmp = final_name + cUseHeadGear + "_gloss.dds";
		LoadImage(tmp.Get(), iCharHeadGearTexture + 4);
	}
	if ((part == 1 || part==-1) && ObjectExist(iCharObjHair) == 1)
	{ 
		//Hair
		for (int a = 0; a < 5; a++)
			if (GetImageExistEx(iCharHairTexture + a)) DeleteImage(iCharHairTexture + a);

		tmp = final_name + cSelectedVariantHair + "_color.dds";
		LoadImage(tmp.Get(), iCharHairTexture + 0);
		tmp = final_name + cUseHair + "_ao.dds";
		LoadImage(tmp.Get(), iCharHairTexture + 1);
		if (ImageExist(iCharHairTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharHairTexture + 1);
		tmp = final_name + cUseHair + "_normal.dds";
		LoadImage(tmp.Get(), iCharHairTexture + 2);
		tmp = final_name + cUseHair + "_metalness.dds";
		LoadImage(tmp.Get(), iCharHairTexture + 3);
		tmp = final_name + cUseHair + "_gloss.dds";
		LoadImage(tmp.Get(), iCharHairTexture + 4);
	}
	if ((part == 2 || part==-1) && ObjectExist(iCharObjHead) == 1)
	{
		//Head
		for (int a = 0; a < 5; a++)
			if (GetImageExistEx(iCharHeadTexture + a)) DeleteImage(iCharHeadTexture + a);

		tmp = final_name + cSelectedVariantHead + "_color.dds";
		LoadImage(tmp.Get(), iCharHeadTexture + 0);
		tmp = final_name + cUseHead + "_ao.dds";
		LoadImage(tmp.Get(), iCharHeadTexture + 1);
		if (ImageExist(iCharHeadTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharHeadTexture + 1);
		tmp = final_name + cUseHead + "_normal.dds";
		LoadImage(tmp.Get(), iCharHeadTexture + 2);
		tmp = final_name + cUseHead + "_metalness.dds";
		LoadImage(tmp.Get(), iCharHeadTexture + 3);
		tmp = final_name + cUseHead + "_gloss.dds";
		LoadImage(tmp.Get(), iCharHeadTexture + 4);

		// in addition, load in skin type to apply to rest of body
		if (tag && strnicmp(tag, "IC", 2) == NULL)
		{
			if (GetImageExistEx(iCharSkinTexture) == 1) DeleteImage(iCharSkinTexture);
			char pRelPathToSkinTexture[MAX_PATH];
			sprintf(pRelPathToSkinTexture, "charactercreatorplus\\skins\\%s.dds", tag);
			if (FileExist(pRelPathToSkinTexture) == 0) sprintf(pRelPathToSkinTexture, "charactercreatorplus\\skins\\%s.png", tag);
			LoadImage(pRelPathToSkinTexture, iCharSkinTexture);
		}
	}
	if ((part == 3 || part==-1) && ObjectExist(iCharObjEyeglasses) == 1)
	{
		// Eyeglasses
		for (int a = 0; a < 5; a++)
			if (GetImageExistEx(iCharEyeglassesTexture + a)) DeleteImage(iCharEyeglassesTexture + a);

		tmp = final_name + cSelectedVariantEyeglasses + "_color.dds";
		LoadImage(tmp.Get(), iCharEyeglassesTexture + 0);
		tmp = final_name + cUseEyeglasses + "_ao.dds";
		LoadImage(tmp.Get(), iCharEyeglassesTexture + 1);
		if (ImageExist(iCharEyeglassesTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharEyeglassesTexture + 1);
		tmp = final_name + cUseEyeglasses + "_normal.dds";
		LoadImage(tmp.Get(), iCharEyeglassesTexture + 2);
		tmp = final_name + cUseEyeglasses + "_metalness.dds";
		LoadImage(tmp.Get(), iCharEyeglassesTexture + 3);
		tmp = final_name + cUseEyeglasses + "_gloss.dds";
		LoadImage(tmp.Get(), iCharEyeglassesTexture + 4);
	}
	if ((part == 4 || part==-1) && ObjectExist(iCharObjFacialHair) == 1)
	{
		// Facial Hair
		for (int a = 0; a < 5; a++)
			if (GetImageExistEx(iCharFacialHairTexture + a)) DeleteImage(iCharFacialHairTexture + a);

		tmp = final_name + cSelectedVariantFacialHair + "_color.dds";
		LoadImage(tmp.Get(), iCharFacialHairTexture + 0);
		tmp = final_name + cUseFacialHair + "_ao.dds";
		LoadImage(tmp.Get(), iCharFacialHairTexture + 1);
		if (ImageExist(iCharFacialHairTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharFacialHairTexture + 1);
		tmp = final_name + cUseFacialHair + "_normal.dds";
		LoadImage(tmp.Get(), iCharFacialHairTexture + 2);
		tmp = final_name + cUseFacialHair + "_metalness.dds";
		LoadImage(tmp.Get(), iCharFacialHairTexture + 3);
		tmp = final_name + cUseFacialHair + "_gloss.dds";
		LoadImage(tmp.Get(), iCharFacialHairTexture + 4);
	}
	if ((part == 5 || part==-1) && ObjectExist(iCharObj) == 1)
	{ 
		//Body
		for (int a = 0; a < 6; a++)
			if (GetImageExistEx(iCharTexture + a)) DeleteImage(iCharTexture + a);

		tmp = final_name + cSelectedVariantBody + "_color.dds";
		LoadImage(tmp.Get(), iCharTexture + 0);
		tmp = final_name + cUseBody + "_ao.dds";
		LoadImage(tmp.Get(), iCharTexture + 1);
		if (ImageExist(iCharTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharTexture + 1);
		tmp = final_name + cUseBody + "_normal.dds";
		LoadImage(tmp.Get(), iCharTexture + 2);
		tmp = final_name + cUseBody + "_metalness.dds";
		LoadImage(tmp.Get(), iCharTexture + 3);
		tmp = final_name + cUseBody + "_gloss.dds";
		LoadImage(tmp.Get(), iCharTexture + 4);
		tmp = final_name + cUseBody + "_mask.dds";
		LoadImage(tmp.Get(), iCharTexture + 5);
	}
	if ((part == 6 || part==-1 || part==67) && ObjectExist(iCharObjLegs) == 1)
	{ 
		//Legs
		for (int a = 0; a < 6; a++)
			if (GetImageExistEx(iCharLegsTexture + a)) DeleteImage(iCharLegsTexture + a);

		tmp = final_name + cSelectedVariantLegs + "_color.dds";
		LoadImage(tmp.Get(), iCharLegsTexture + 0);
		tmp = final_name + cUseLegs + "_ao.dds";
		LoadImage(tmp.Get(), iCharLegsTexture + 1);
		if (ImageExist(iCharLegsTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharLegsTexture + 1);
		tmp = final_name + cUseLegs + "_normal.dds";
		LoadImage(tmp.Get(), iCharLegsTexture + 2);
		tmp = final_name + cUseLegs + "_metalness.dds";
		LoadImage(tmp.Get(), iCharLegsTexture + 3);
		tmp = final_name + cUseLegs + "_gloss.dds";
		LoadImage(tmp.Get(), iCharLegsTexture + 4);
		tmp = final_name + cUseLegs + "_mask.dds";
		LoadImage(tmp.Get(), iCharLegsTexture + 5);
	}
	if ((part == 7 || part==-1 || part==67) && ObjectExist(iCharObjFeet) == 1)
	{ 
		//Feet
		for (int a = 0; a < 6; a++)
			if (GetImageExistEx(iCharFeetTexture + a)) DeleteImage(iCharFeetTexture + a);

		tmp = final_name + cSelectedVariantFeet + "_color.dds";
		LoadImage(tmp.Get(), iCharFeetTexture + 0);
		tmp = final_name + cUseFeet + "_ao.dds";
		LoadImage(tmp.Get(), iCharFeetTexture + 1);
		if (ImageExist(iCharFeetTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharFeetTexture + 1);
		tmp = final_name + cUseFeet + "_normal.dds";
		LoadImage(tmp.Get(), iCharFeetTexture + 2);
		tmp = final_name + cUseFeet + "_metalness.dds";
		LoadImage(tmp.Get(), iCharFeetTexture + 3);
		tmp = final_name + cUseFeet + "_gloss.dds";
		LoadImage(tmp.Get(), iCharFeetTexture + 4);
		tmp = final_name + cUseFeet + "_mask.dds";
		LoadImage(tmp.Get(), iCharFeetTexture + 5);
	}

	// texture parts
	if (ObjectExist(iCharObjHeadGear) == 1)
	{
		TextureObject(iCharObjHeadGear, 0, iCharHeadGearTexture + 0);
		TextureObject(iCharObjHeadGear, 1, iCharHeadGearTexture + 1);
		TextureObject(iCharObjHeadGear, 2, iCharHeadGearTexture + 2);
		TextureObject(iCharObjHeadGear, 3, iCharHeadGearTexture + 3);
		TextureObject(iCharObjHeadGear, 4, iCharHeadGearTexture + 4);
		TextureObject(iCharObjHeadGear, 5, iCharTextureBlack);
		TextureObject(iCharObjHeadGear, 7, iCharSkinTexture);
		SetObjectEffect(iCharObjHeadGear, g.thirdpersoncharactereffect);
	}
	if (ObjectExist(iCharObjHair) == 1)
	{
		TextureObject(iCharObjHair, 0, iCharHairTexture + 0);
		TextureObject(iCharObjHair, 1, iCharHairTexture + 1);
		TextureObject(iCharObjHair, 2, iCharHairTexture + 2);
		TextureObject(iCharObjHair, 3, iCharHairTexture + 3);
		TextureObject(iCharObjHair, 4, iCharHairTexture + 4);
		TextureObject(iCharObjHair, 5, iCharTextureBlack);
		TextureObject(iCharObjHair, 7, iCharSkinTexture);
		SetObjectEffect(iCharObjHair, g.thirdpersoncharactereffect);
	}
	TextureObject(iCharObjHead, 0, iCharHeadTexture + 0);
	TextureObject(iCharObjHead, 1, iCharHeadTexture + 1);
	TextureObject(iCharObjHead, 2, iCharHeadTexture + 2);
	TextureObject(iCharObjHead, 3, iCharHeadTexture + 3);
	TextureObject(iCharObjHead, 4, iCharHeadTexture + 4);
	TextureObject(iCharObjHead, 5, iCharTextureBlack);
	TextureObject(iCharObjHead, 7, iCharSkinTexture);
	SetObjectEffect(iCharObjHead, g.thirdpersoncharactereffect);
	if (ObjectExist(iCharObjEyeglasses) == 1)
	{
		TextureObject(iCharObjEyeglasses, 0, iCharEyeglassesTexture + 0);
		TextureObject(iCharObjEyeglasses, 1, iCharEyeglassesTexture + 1);
		TextureObject(iCharObjEyeglasses, 2, iCharEyeglassesTexture + 2);
		TextureObject(iCharObjEyeglasses, 3, iCharEyeglassesTexture + 3);
		TextureObject(iCharObjEyeglasses, 4, iCharEyeglassesTexture + 4);
		TextureObject(iCharObjEyeglasses, 5, iCharTextureBlack);
		TextureObject(iCharObjEyeglasses, 7, iCharSkinTexture);
		SetObjectEffect(iCharObjEyeglasses, g.thirdpersoncharactereffect);
	}
	if (ObjectExist(iCharObjFacialHair) == 1)
	{
		TextureObject(iCharObjFacialHair, 0, iCharFacialHairTexture + 0);
		TextureObject(iCharObjFacialHair, 1, iCharFacialHairTexture + 1);
		TextureObject(iCharObjFacialHair, 2, iCharFacialHairTexture + 2);
		TextureObject(iCharObjFacialHair, 3, iCharFacialHairTexture + 3);
		TextureObject(iCharObjFacialHair, 4, iCharFacialHairTexture + 4);
		TextureObject(iCharObjFacialHair, 5, iCharTextureBlack);
		TextureObject(iCharObjFacialHair, 7, iCharSkinTexture);
		SetObjectEffect(iCharObjFacialHair, g.thirdpersoncharactereffect);
	}
	TextureObject(iCharObj, 0, iCharTexture + 0);
	TextureObject(iCharObj, 1, iCharTexture + 1);
	TextureObject(iCharObj, 2, iCharTexture + 2);
	TextureObject(iCharObj, 3, iCharTexture + 3);
	TextureObject(iCharObj, 4, iCharTexture + 4);
	TextureObject(iCharObj, 5, iCharTexture + 5);
	TextureObject(iCharObj, 7, iCharSkinTexture);
	SetObjectEffect(iCharObj, g.thirdpersoncharactereffect);
	TextureObject(iCharObjLegs, 0, iCharLegsTexture + 0);
	TextureObject(iCharObjLegs, 1, iCharLegsTexture + 1);
	TextureObject(iCharObjLegs, 2, iCharLegsTexture + 2);
	TextureObject(iCharObjLegs, 3, iCharLegsTexture + 3);
	TextureObject(iCharObjLegs, 4, iCharLegsTexture + 4);
	TextureObject(iCharObjLegs, 5, iCharLegsTexture + 5);
	TextureObject(iCharObjLegs, 7, iCharSkinTexture);
	SetObjectEffect(iCharObjLegs, g.thirdpersoncharactereffect);
	TextureObject(iCharObjFeet, 0, iCharFeetTexture + 0);
	TextureObject(iCharObjFeet, 1, iCharFeetTexture + 1);
	TextureObject(iCharObjFeet, 2, iCharFeetTexture + 2);
	TextureObject(iCharObjFeet, 3, iCharFeetTexture + 3);
	TextureObject(iCharObjFeet, 4, iCharFeetTexture + 4);
	TextureObject(iCharObjFeet, 5, iCharFeetTexture + 5);
	TextureObject(iCharObjFeet, 7, iCharSkinTexture);
	SetObjectEffect(iCharObjFeet, g.thirdpersoncharactereffect);


	// double hair trick allows hair to render correctly
	int iDoubleHair = g.characterkitobjectoffset + 15;

	// stitch model together
	if (ObjectExist(iCharObjHeadGear) == 1)
	{
		StealMeshesFromObject(iCharObj, iCharObjHeadGear);
		DeleteObject(iCharObjHeadGear);
	}
	StealMeshesFromObject(iCharObj, iCharObjHead);
	DeleteObject(iCharObjHead);
	StealMeshesFromObject(iCharObj, iCharObjLegs);
	DeleteObject(iCharObjLegs);
	StealMeshesFromObject(iCharObj, iCharObjFeet);
	DeleteObject(iCharObjFeet);
	if (ObjectExist(iCharObjHair) == 1)
	{
		// ensure hair has no culling and semi-transparent
		SetObjectTransparency(iCharObjHair, 2);
		SetObjectCull(iCharObjHair, 0);
		// double hair trick ensures polygons rendered out of order dont leave 'mostly transparent' pixels blocking hair behind them
		CloneObject(iDoubleHair, iCharObjHair);
		DisableObjectZWriteEx(iCharObjHair,true);
		StealMeshesFromObject(iCharObj, iCharObjHair);
		DeleteObject(iCharObjHair);
		// the double hair mesh is drawn again but this time with Zdepthwrite enabled so backface more distant hair polygons dont render over nearer ones
		SetObjectCull(iDoubleHair, 0);
		SetObjectTransparency(iDoubleHair, 2);
		StealMeshesFromObject(iCharObj, iDoubleHair);
		DeleteObject(iDoubleHair);
	}
	if (ObjectExist(iCharObjEyeglasses) == 1)
	{
		SetObjectCull(iCharObjEyeglasses, 0);
		SetObjectTransparency(iCharObjEyeglasses, 2);
		DisableObjectZWriteEx(iCharObjEyeglasses,true);
		StealMeshesFromObject(iCharObj, iCharObjEyeglasses);
		DeleteObject(iCharObjEyeglasses);
	}
	if (ObjectExist(iCharObjFacialHair) == 1)
	{
		SetObjectTransparency(iCharObjFacialHair, 2);
		SetObjectCull(iCharObjFacialHair, 0);
		DisableObjectZWriteEx(iCharObjFacialHair, true);
		StealMeshesFromObject(iCharObj, iCharObjFacialHair);
		DeleteObject(iCharObjFacialHair);
	}

	// as character parts have no animations, wipe out ones they do have
	// and replace with the latest animation set for this base mesh
	tmp = final_name + "default animations" + CCPMODELEXT;
	if (FileExist(tmp.Get()))
	{
		AppendObject(tmp.Get(), iCharObj, 0);
	}
	// and trigger animation to be prepped
	g_bCharacterCreatorPrepAnims = true;


	// this sequence is duplicated during the init, see if can merge into single function at some point!

	// position final stitched character in scene
	float terrain_height = BT_GetGroundHeight(t.terrain.TerrainID, GGORIGIN_X, GGORIGIN_Z, 1);
	fCharObjectY = terrain_height;
	PositionObject(iCharObj, ccpObjTargetX, ccpObjTargetY, ccpObjTargetZ);
	RotateObject(iCharObj, ccpObjTargetAX, ccpObjTargetAY, ccpObjTargetAZ);
	SetObjectArtFlags(iCharObj, (1 << 1) + (0), 0);
	LoopObject(iCharObj, 15, 55);

	// set object default animation speed
	SetObjectSpeed(iCharObj, 100);

	// finished legacy loading requirements
	image_setlegacyimageloading(false);
}

void charactercreatorplus_loadannotationlist ( void )
{
	// prepare destination file
	LPSTR pFilename = "Annotates.txt";
	if (FileExist(pFilename) == 0) pFilename = "devannotates.txt";
	g_charactercreatorplus_annotation_list.clear();
	g_charactercreatorplus_annotationtag_list.clear();
	if (FileExist(pFilename) == 1)
	{
		OpenToRead(1, pFilename);
		while (FileEnd(1) == 0)
		{
			// get line by line
			cstr line_s = ReadString(1);
			LPSTR pLine = line_s.Get();

			// get field name
			char pFieldName[260];
			char pFieldValue[260];
			strcpy(pFieldName, pLine);
			LPSTR pEquals = strstr(pFieldName, "=");
			if (pEquals)
			{
				LPSTR pEqualsPos = pEquals;

				// field name - eat spaces at end of field name
				pEquals--;
				while (pEquals > pFieldName && *pEquals == 32) pEquals--;
				*(pEquals + 1) = 0;

				// rest is field value
				strcpy(pFieldValue, pEqualsPos + 2);

				// strip off any tags
				char tag[260];
				strcpy(tag, "");
				for (int tagn = 0; tagn < strlen(pFieldValue); tagn++)
				{
					if (pFieldValue[tagn] == '[')
					{
						strcpy(tag, pFieldValue+tagn+1);
						pFieldValue[tagn] = 0;
					}
				}
				while (strlen(tag) > 0 && (tag[strlen(tag) - 1] == ' ' || tag[strlen(tag) - 1] == ']')) tag[strlen(tag) - 1] = 0;

				// add to good list of annotations
				g_charactercreatorplus_annotation_list.insert(std::make_pair(pFieldName, pFieldValue));
				g_charactercreatorplus_annotationtag_list.insert(std::make_pair(pFieldName, tag));
			}
		}

		// close file handling
		CloseFile(1);
	}
}

LPSTR charactercreatorplus_findannotation ( LPSTR pSearchStr )
{
	LPSTR pNewString = NULL;
	for ( std::map<std::string, std::string>::iterator it = g_charactercreatorplus_annotation_list.begin(); it != g_charactercreatorplus_annotation_list.end(); ++it)
	{
		std::string field = it->first;
		if ( stricmp ( field.c_str(), pSearchStr ) == NULL )
		{
			pNewString = (LPSTR)it->second.c_str();
			break;
		}
	}
	return pNewString;
}

LPSTR charactercreatorplus_findannotationtag ( LPSTR pSearchStr )
{
	LPSTR pNewString = NULL;
	for ( std::map<std::string, std::string>::iterator it = g_charactercreatorplus_annotationtag_list.begin(); it != g_charactercreatorplus_annotationtag_list.end(); ++it)
	{
		std::string field = it->first;
		if ( stricmp ( field.c_str(), pSearchStr ) == NULL )
		{
			pNewString = (LPSTR)it->second.c_str();
			break;
		}
	}
	return pNewString;
}

void charactercreatorplus_refreshtype(void)
{
	// need legacy loading for image reskinning work
	image_setlegacyimageloading(true);

	// ensure we have all textures of character creator init before we start
	image_preload_files_wait();
	object_preload_files_wait();

	// reset character base mesh and textures based on type stored in CCP_Type
	CharacterCreatorHeadGear_s.clear();
	CharacterCreatorHair_s.clear();
	CharacterCreatorHead_s.clear();
	CharacterCreatorEyeglasses_s.clear();
	CharacterCreatorFacialHair_s.clear();
	CharacterCreatorBody_s.clear();
	CharacterCreatorLegs_s.clear();
	CharacterCreatorFeet_s.clear();
	CharacterCreatorAnnotatedHeadGear_s.clear();
	CharacterCreatorAnnotatedHair_s.clear();
	CharacterCreatorAnnotatedHead_s.clear();
	CharacterCreatorAnnotatedEyeglasses_s.clear();
	CharacterCreatorAnnotatedFacialHair_s.clear();
	CharacterCreatorAnnotatedBody_s.clear();
	CharacterCreatorAnnotatedLegs_s.clear();
	CharacterCreatorAnnotatedFeet_s.clear();
	CharacterCreatorAnnotatedTagHeadGear_s.clear();
	CharacterCreatorAnnotatedTagHair_s.clear();
	CharacterCreatorAnnotatedTagHead_s.clear();
	CharacterCreatorAnnotatedTagEyeglasses_s.clear();
	CharacterCreatorAnnotatedTagFacialHair_s.clear();
	CharacterCreatorAnnotatedTagBody_s.clear();
	CharacterCreatorAnnotatedTagLegs_s.clear();
	CharacterCreatorAnnotatedTagFeet_s.clear();

	// work out path to parts
	char pPartsPath[260];
	strcpy(pPartsPath, "charactercreatorplus\\parts\\");
	strcat(pPartsPath, CCP_Type);
	strcat(pPartsPath, "\\");

	// some lists can have a NONE value, which does not need a part model
	CharacterCreatorHeadGear_s.insert(std::make_pair("None", pPartsPath));
	CharacterCreatorHair_s.insert(std::make_pair("None", pPartsPath));
	CharacterCreatorEyeglasses_s.insert(std::make_pair("None", pPartsPath));
	CharacterCreatorFacialHair_s.insert(std::make_pair("None", pPartsPath));
	CharacterCreatorAnnotatedHeadGear_s.insert(std::make_pair("", "None"));
	CharacterCreatorAnnotatedHair_s.insert(std::make_pair("", "None"));
	CharacterCreatorAnnotatedEyeglasses_s.insert(std::make_pair("", "None"));
	CharacterCreatorAnnotatedFacialHair_s.insert(std::make_pair("", "None"));

	// ensure annotates and tags size matches
	CharacterCreatorAnnotatedTagHeadGear_s.insert(std::make_pair("", ""));
	CharacterCreatorAnnotatedTagHair_s.insert(std::make_pair("", ""));
	CharacterCreatorAnnotatedTagEyeglasses_s.insert(std::make_pair("", ""));
	CharacterCreatorAnnotatedTagFacialHair_s.insert(std::make_pair("", ""));

	// free any old character objects
	iCharObj = g.characterkitobjectoffset + 1;
	iCharObjHeadGear = g.characterkitobjectoffset + 2;
	iCharObjHair = g.characterkitobjectoffset + 3;
	iCharObjHead = g.characterkitobjectoffset + 4;
	iCharObjEyeglasses = g.characterkitobjectoffset + 5;
	iCharObjFacialHair = g.characterkitobjectoffset + 6;
	iCharObjLegs = g.characterkitobjectoffset + 7;
	iCharObjFeet = g.characterkitobjectoffset + 8;
	if (ObjectExist(iCharObj) == 1) DeleteObject(iCharObj);
	if (ObjectExist(iCharObjHeadGear) == 1) DeleteObject(iCharObjHeadGear);
	if (ObjectExist(iCharObjHair) == 1) DeleteObject(iCharObjHair);
	if (ObjectExist(iCharObjHead) == 1) DeleteObject(iCharObjHead);
	if (ObjectExist(iCharObjEyeglasses) == 1) DeleteObject(iCharObjEyeglasses);
	if (ObjectExist(iCharObjFacialHair) == 1) DeleteObject(iCharObjFacialHair);
	if (ObjectExist(iCharObjLegs) == 1) DeleteObject(iCharObjLegs);
	if (ObjectExist(iCharObjFeet) == 1) DeleteObject(iCharObjFeet);

	// default body part choices
	strcpy(cSelectedFeetFilter, "");
	strcpy(cSelectedICCode, "IC1a");
	strcpy(cSelectedHeadGear, "None");
	strcpy(cSelectedHair, "");
	strcpy(cSelectedHead, "");
	strcpy(cSelectedEyeglasses, "None");
	strcpy(cSelectedFacialHair, "None");
	strcpy(cSelectedBody, "");
	strcpy(cSelectedLegs, "");
	strcpy(cSelectedFeet, "");

	// which legs part to use
	strcpy(cSelectedLegsFilter, "");
	LPSTR pOptionalLegsChoice = "02"; 

	// scan for all character parts
	cstr olddir_s = GetDir();
	char pTempStr[260];
	strcpy(pTempStr, "charactercreatorplus\\parts\\");
	strcat(pTempStr, CCP_Type);
	SetDir(pTempStr);
	charactercreatorplus_loadannotationlist();
	ChecklistForFiles();
	for (int c = 1; c <= ChecklistQuantity(); c++)
	{
		cStr tfile_s = Lower(ChecklistString(c));
		if (tfile_s != "." && tfile_s != "..")
		{
			char *find = NULL;
			if (strcmp(Right(tfile_s.Get(), 10), "_color.dds") == 0)
			{
				// base filename
				char tmp[260];
				strcpy(tmp, tfile_s.Get());
				tmp[strlen(tmp) - 10] = 0; // remove _color.dds

				// determine which list it goes into
				if (pestrcasestr(tfile_s.Get(), " body "))
				{
					CharacterCreatorBody_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedBody_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedBody_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagBody_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagBody_s.insert(std::make_pair(tmp, ""));

					bool bBodyFilterThisOut = false;
					if (stricmp(CCP_Type, "zombie male") != NULL)
					{
						if (tmp[strlen(tmp) - 1] != '1' && tmp[strlen(tmp) - 1] != 'a') bBodyFilterThisOut = true;
					}
					if (strlen(cSelectedBody) == 0 && bBodyFilterThisOut == false)
					{
						// for body, do we need no legs?
						if (pAnnotatedTagLabel)
						{
							if (strstr(pAnnotatedTagLabel, "No Legs") != NULL)
							{
								strcpy(cSelectedLegsFilter, "No Legs");
								pOptionalLegsChoice = "01";
							}
						}
						
						// and this is the default body
						strcpy(cSelectedBody, tmp);
					}
				}
				else if (pestrcasestr(tfile_s.Get(), " feet "))
				{
					CharacterCreatorFeet_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedFeet_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedFeet_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagFeet_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagFeet_s.insert(std::make_pair(tmp, ""));
					if (strlen(cSelectedFeet) == 0) strcpy(cSelectedFeet, tmp);
				}
				else if (pestrcasestr(tfile_s.Get(), " hair "))
				{
					CharacterCreatorHair_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedHair_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedHair_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagHair_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagHair_s.insert(std::make_pair(tmp, ""));
					if (strlen(cSelectedHair) == 0) strcpy(cSelectedHair, tmp);
				}
				else if (pestrcasestr(tfile_s.Get(), " head "))
				{
					CharacterCreatorHead_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedHead_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedHead_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagHead_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagHead_s.insert(std::make_pair(tmp, ""));
					if (strlen(cSelectedHead) == 0) strcpy(cSelectedHead, tmp);
				}
				else if (pestrcasestr(tfile_s.Get(), " legs "))
				{
					CharacterCreatorLegs_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedLegs_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedLegs_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagLegs_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagLegs_s.insert(std::make_pair(tmp, ""));

					// if require no legs, always choose 01 for default
					if (strlen(cSelectedLegs) == 0)
					{
						bool bLegsFilterThisOut = false;
						if (stricmp(CCP_Type, "zombie male") != NULL)
						{
							bLegsFilterThisOut = true;
							if (strstr(pOptionalLegsChoice, "01") != NULL)
							{
								if (strlen(cSelectedLegs) == 0 && tmp[strlen(tmp) - 1] == '1') bLegsFilterThisOut = false;
							}
							else
							{
								if (strlen(cSelectedLegs) == 0 && tmp[strlen(tmp) - 1] != '1') bLegsFilterThisOut = false;
							}
						}
						if (bLegsFilterThisOut == false)
						{
							strcpy(cSelectedLegs, tmp);
						}
					}
				}
				else if (pestrcasestr(tfile_s.Get(), " headgear "))
				{
					CharacterCreatorHeadGear_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedHeadGear_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedHeadGear_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagHeadGear_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagHeadGear_s.insert(std::make_pair(tmp, ""));
					if (strlen(cSelectedHeadGear) == 0) strcpy(cSelectedHeadGear, tmp);
				}
				else if (pestrcasestr(tfile_s.Get(), " facialhair "))
				{
					CharacterCreatorFacialHair_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedFacialHair_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedFacialHair_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagFacialHair_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagFacialHair_s.insert(std::make_pair(tmp, ""));
					if (strlen(cSelectedFacialHair) == 0) strcpy(cSelectedFacialHair, tmp);
				}
				else if (pestrcasestr(tfile_s.Get(), " eyeglasses "))
				{
					CharacterCreatorEyeglasses_s.insert(std::make_pair(tmp, pPartsPath));// "charactercreatorplus\\parts\\adult male\\"));
					LPSTR pAnnotatedLabel = charactercreatorplus_findannotation(tmp);
					if (pAnnotatedLabel)
						CharacterCreatorAnnotatedEyeglasses_s.insert(std::make_pair(tmp, pAnnotatedLabel));
					else
						CharacterCreatorAnnotatedEyeglasses_s.insert(std::make_pair(tmp, tmp));
					LPSTR pAnnotatedTagLabel = charactercreatorplus_findannotationtag(tmp);
					if (pAnnotatedTagLabel)
						CharacterCreatorAnnotatedTagEyeglasses_s.insert(std::make_pair(tmp, pAnnotatedTagLabel));
					else
						CharacterCreatorAnnotatedTagEyeglasses_s.insert(std::make_pair(tmp, ""));
					if (strlen(cSelectedEyeglasses) == 0) strcpy(cSelectedEyeglasses, tmp);
				}
			}
		}
	}
	SetDir(olddir_s.Get());

	// default selection for each CCP type
	sprintf(cSelectedBody, "%s body 02", CCP_Type);
	sprintf(cSelectedHead, "%s head 01", CCP_Type);
	sprintf(cSelectedLegs, "%s legs 02", CCP_Type);
	sprintf(cSelectedFeet, "%s feet 01", CCP_Type);
	sprintf(cSelectedHair, "%s hair 01", CCP_Type);

	// output location for character
	LPSTR pCharacterFinal = "entitybank\\user\\charactercreatorplus\\character.dbo";

	// reset color selections
	for (int a = 0; a < 5; a++) vColorSelected[a] = ImVec4(0.0, 0.0, 0.0, 1.0);

	// generic textures
	int iCharTextureWhite = g.charactercreatorEditorImageoffset + 0;
	LoadImage("effectbank\\reloaded\\media\\blank_O.DDS", iCharTextureWhite);
	int iCharTextureBlack = g.charactercreatorEditorImageoffset + 1;
	if (!GetImageExistEx(iCharTextureBlack)) LoadImage("effectbank\\reloaded\\media\\blank_black.dds", iCharTextureBlack);

	// work out path to part files
	strcpy(pTempStr, "charactercreatorplus\\parts\\");
	strcat(pTempStr, CCP_Type);
	strcat(pTempStr, "\\");
	strcat(pTempStr, CCP_Type);
	strcat(pTempStr, " ");

	// texture index references
	int iCharTexture = g.charactercreatorEditorImageoffset + 1;
	int iCharHeadGearTexture = g.charactercreatorEditorImageoffset + 11;
	int iCharHairTexture = g.charactercreatorEditorImageoffset + 21;
	int iCharHeadTexture = g.charactercreatorEditorImageoffset + 31;
	int iCharEyeglassesTexture = g.charactercreatorEditorImageoffset + 41;
	int iCharFacialHairTexture = g.charactercreatorEditorImageoffset + 51;
	int iCharLegsTexture = g.charactercreatorEditorImageoffset + 61;
	int iCharFeetTexture = g.charactercreatorEditorImageoffset + 71;

	// load default skin type texture IC1a
	std::map<std::string, std::string>::iterator it = CharacterCreatorAnnotatedTagHead_s.begin();
	LPSTR pCorrectICCode = "IC1c"; // default
	for (it = CharacterCreatorAnnotatedTagHead_s.begin(); it != CharacterCreatorAnnotatedTagHead_s.end(); ++it)
	{
		std::string thistag = it->first;
		if (strnicmp (thistag.c_str(), cSelectedHead, strlen(cSelectedHead)) == NULL)
		{
			// found correct skin for this head
			LPSTR pNewSkinCode = (LPSTR)it->second.c_str();
			if (strlen(pNewSkinCode) > 0) pCorrectICCode = pNewSkinCode;
			break;
		}
	}
	int iCharSkinTexture = g.charactercreatorEditorImageoffset + 101;
	if (GetImageExistEx(iCharSkinTexture) == 1) DeleteImage(iCharSkinTexture);
	char pRelPathToSkinTexture[MAX_PATH];
	sprintf(pRelPathToSkinTexture, "charactercreatorplus\\skins\\%s.dds", pCorrectICCode);
	if (FileExist(pRelPathToSkinTexture) == 0) sprintf(pRelPathToSkinTexture, "charactercreatorplus\\skins\\%s.png", pCorrectICCode);
	LoadImage(pRelPathToSkinTexture, iCharSkinTexture);

	// Default body
	bool bNonStandardCharacter = false;
	#ifdef PRODUCTV3
	 LPSTR pDefaultBody = "02";
	 LPSTR pDefaultHair = "01";
	 LPSTR pDefaultHead = "01";
	 LPSTR pDefaultLegs = "02";
	 LPSTR pDefaultFeet = "01";
	#else
	// need a better way than this 
	LPSTR pDefaultBody = "03";
	LPSTR pDefaultHair = "01";
	LPSTR pDefaultHead = "04";
	LPSTR pDefaultLegs = "03";
	LPSTR pDefaultFeet = "01";
	if (stricmp(CCP_Type, "zombie male") == NULL)
	{
		pDefaultBody = "01";
		pDefaultHair = "01";
		pDefaultHead = "01";
		pDefaultLegs = "01";
		bNonStandardCharacter = true;
	}
	#endif
	LPSTR pDefaultBodyVariant = pDefaultBody;
	LPSTR pDefaultHairVariant = pDefaultHair;
	LPSTR pDefaultHeadVariant = pDefaultHead;
	LPSTR pDefaultLegsVariant = pDefaultLegs;
	LPSTR pDefaultFeetVariant = pDefaultFeet;

	// load all body parts
	for (int iPartID = 0; iPartID < 8; iPartID++)
	{
		// work out which part
		int iThisObj = 0;
		int iThisTexture = 0;
		LPSTR pPartName = "";
		LPSTR pPartNum = "";
		LPSTR pPartNumVariant = "";
		if (iPartID == 0) { iThisObj = iCharObj; iThisTexture = iCharTexture; pPartName = "body"; pPartNum = pDefaultBody; pPartNumVariant = pDefaultBodyVariant; }
		if (iPartID == 1) { iThisObj = iCharObjHeadGear; iThisTexture = iCharHeadGearTexture; pPartName = ""; pPartNum = ""; pPartNumVariant = ""; }
		if (iPartID == 2) { iThisObj = iCharObjHair; iThisTexture = iCharHairTexture; pPartName = "hair"; pPartNum = pDefaultHair; pPartNumVariant = pDefaultHairVariant; }
		if (iPartID == 3) { iThisObj = iCharObjHead; iThisTexture = iCharHeadTexture; pPartName = "head"; pPartNum = pDefaultHead; pPartNumVariant = pDefaultHeadVariant; }
		if (iPartID == 4) { iThisObj = iCharObjEyeglasses; iThisTexture = iCharEyeglassesTexture; pPartName = ""; pPartNum = ""; pPartNumVariant = ""; }
		if (iPartID == 5) { iThisObj = iCharObjFacialHair; iThisTexture = iCharFacialHairTexture; pPartName = ""; pPartNum = ""; pPartNumVariant = ""; }
		if (iPartID == 6) 
		{ 
			iThisObj = iCharObjLegs; 
			iThisTexture = iCharLegsTexture; 
			pPartName = "legs"; 
			pPartNum = pDefaultLegs; 
			pPartNumVariant = pDefaultLegsVariant; 	
			std::map<std::string, std::string>::iterator it = CharacterCreatorAnnotatedTagBody_s.begin();
			LPSTR pCorrpBodyNoLegsTag = (LPSTR)it->second.c_str();
			if ( strstr ( pCorrpBodyNoLegsTag, "No Legs" ) != NULL )
				pPartNum = "01"; 
		}
		if (iPartID == 7) { iThisObj = iCharObjFeet; iThisTexture = iCharFeetTexture; pPartName = "feet"; pPartNum = pDefaultFeet; pPartNumVariant = pDefaultFeetVariant; }

		// load part object, textures and apply effect
		if (strlen(pPartName) > 0)
		{
			char pPartFile[260];
			strcpy(pPartFile, pPartName);
			strcat(pPartFile, " ");
			strcat(pPartFile, pPartNum); // i.e. 01
			LoadObject(cstr(cstr(pTempStr) + pPartFile + CCPMODELEXT).Get(), iThisObj);
			LoadImage(cstr(cstr(pTempStr) + pPartFile + "_color.dds").Get(), iThisTexture + 0);
			LoadImage(cstr(cstr(pTempStr) + pPartFile + "_ao.dds").Get(), iThisTexture + 1);
			LoadImage(cstr(cstr(pTempStr) + pPartFile + "_normal.dds").Get(), iThisTexture + 2);
			LoadImage(cstr(cstr(pTempStr) + pPartFile + "_metalness.dds").Get(), iThisTexture + 3);
			LoadImage(cstr(cstr(pTempStr) + pPartFile + "_gloss.dds").Get(), iThisTexture + 4);
			if (ImageExist(iThisTexture + 1) == 0) LoadImage("effectbank\\reloaded\\media\\blank_O.dds", iThisTexture + 1);
			LoadImage(cstr(cstr(pTempStr) + pPartFile + "_mask.dds").Get(), iThisTexture + 5);
			TextureObject(iThisObj, 0, iThisTexture + 0);
			TextureObject(iThisObj, 1, iThisTexture + 1);
			TextureObject(iThisObj, 2, iThisTexture + 2);
			TextureObject(iThisObj, 3, iThisTexture + 3);
			TextureObject(iThisObj, 4, iThisTexture + 4);
			TextureObject(iThisObj, 5, iThisTexture + 5);
			TextureObject(iThisObj, 7, iCharSkinTexture);
			SetObjectEffect(iThisObj, g.thirdpersoncharactereffect);
		}
	}


	// double hair trick allows hair to render correctly
	int iDoubleHair = g.characterkitobjectoffset + 15;

	// meshes are useless once they have been stolen from (preload system allows fresh loading to be near instant however)
	if (ObjectExist(iCharObjHeadGear) == 1)
	{
		StealMeshesFromObject(iCharObj, iCharObjHeadGear);
		DeleteObject(iCharObjHeadGear);
	}
	StealMeshesFromObject(iCharObj, iCharObjHead);
	DeleteObject(iCharObjHead);
	StealMeshesFromObject(iCharObj, iCharObjLegs);
	DeleteObject(iCharObjLegs);
	StealMeshesFromObject(iCharObj, iCharObjFeet);
	DeleteObject(iCharObjFeet);
	if (ObjectExist(iCharObjHair) == 1)
	{
		// ensure hair has no culling and semi-transparent
		SetObjectTransparency(iCharObjHair, 2);
		SetObjectCull(iCharObjHair, 0);
		// double hair trick ensures polygons rendered out of order dont leave 'mostly transparent' pixels blocking hair behind them
		CloneObject(iDoubleHair, iCharObjHair);
		DisableObjectZWriteEx(iCharObjHair,true);
		StealMeshesFromObject(iCharObj, iCharObjHair);
		DeleteObject(iCharObjHair);
		// the double hair mesh is drawn again but this time with Zdepthwrite enabled so backface more distant hair polygons dont render over nearer ones
		SetObjectCull(iDoubleHair, 0);
		SetObjectTransparency(iDoubleHair, 2);
		StealMeshesFromObject(iCharObj, iDoubleHair);
		DeleteObject(iDoubleHair);
	}
	if (ObjectExist(iCharObjEyeglasses) == 1)
	{
		SetObjectCull(iCharObjEyeglasses, 0);
		DisableObjectZWriteEx(iCharObjEyeglasses, true);
		SetObjectTransparency(iCharObjEyeglasses, 2);
		StealMeshesFromObject(iCharObj, iCharObjEyeglasses);
		DeleteObject(iCharObjEyeglasses);
	}
	if (ObjectExist(iCharObjFacialHair) == 1)
	{
		SetObjectTransparency(iCharObjFacialHair, 2);
		SetObjectCull(iCharObjFacialHair, 0);
		StealMeshesFromObject(iCharObj, iCharObjFacialHair);
		DeleteObject(iCharObjFacialHair);
	}

	// as character parts have no animations, wipe out ones they do have
	// and replace with the latest animation set for this base mesh
	cstr final_name = "charactercreatorplus\\parts\\";
	final_name = final_name + CCP_Type;
	final_name = final_name +"\\default animations" + CCPMODELEXT;
	if (FileExist(final_name.Get()))
	{
		AppendObject(final_name.Get(), iCharObj, 0);
	}
	// and trigger animation to be prepped
	g_bCharacterCreatorPrepAnims = true;



	// place character in scene
	float terrain_height = BT_GetGroundHeight(t.terrain.TerrainID, GGORIGIN_X, GGORIGIN_Z, 1);
	fCharObjectY = terrain_height;
	PositionObject(iCharObj, ccpObjTargetX, ccpObjTargetY, ccpObjTargetZ);
	RotateObject(iCharObj, ccpObjTargetAX, ccpObjTargetAY, ccpObjTargetAZ);
	SetObjectArtFlags(iCharObj, (1 << 1) + (0), 0);
	LoopObject(iCharObj, 15, 55);

	// set default object animation speed
	SetObjectSpeed(iCharObj, 100);

	// if it highly likely new users will go from male, female, girl and boy,
	// so anticipate this by preloading the default sets of each just after a type selected
	// preloadallcharacterpartchoices includes !!charactercreatorplus_preloadallcharacterbasedefaults!!
	charactercreatorplus_preloadallcharacterpartchoices(); 

	// finished legacy loading
	image_setlegacyimageloading(false);
}

void charactercreatorplus_init(void)
{
	// Initialisation prompt 
	timestampactivity ( 0, "Start character creator plus initialisation" );
	g_CharacterCreatorPlus.bInitialised = true;

	// create voice list for choices
	if (CreateListOfVoices() > 0) 
	{
		pCCPVoiceSet = g_voiceList_s[0].Get();
		CCP_SelectedToken = g_voicetoken[0];
	}

	// hide other editor stuff
	terrain_paintselector_hide();
	waypoint_hideall ( );

	// load in base mesh list, objects and texture for initial character
	charactercreatorplus_refreshtype();

	// finished
	timestampactivity ( 0, "Finished character creator plus initialisation" );
}

void charactercreatorplus_free(void)
{
	// show editor stuff
	waypoint_showall();

	// deactivate character creator plus
	g_bCharacterCreatorPlusActivated = false;
}

bool charactercreatorplus_savecharacterentity ( int iCharObj, LPSTR pOptionalDBOSaveFile, int iThumbnailImage )
{
	// saves character FPE, DBO and BMP from current character
	char pEntityName[1024];
	cstr FPEFile_s, BMPFile_s, DBOFile_s;
	strcpy ( pEntityName, "" );
	if ( pOptionalDBOSaveFile != NULL )
	{
		strcpy ( pEntityName, pOptionalDBOSaveFile );
		pEntityName[strlen(pEntityName)-4] = 0;
		FPEFile_s = cstr(pEntityName)+".fpe";
		BMPFile_s = cstr(pEntityName)+".bmp";
		DBOFile_s = cstr(pEntityName)+".dbo";
	}

	// store old dir
	cstr olddir_s = GetDir();

	// check if user folder exists, if not create it
	if ( PathExist( cstr( g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() )  ==  0 ) 
	{
		MakeDirectory ( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() );
	}
	if ( PathExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus").Get() )  ==  0 ) 
	{
		MakeDirectory ( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus").Get() );
	}

	// allow mouse for file dialog
	ShowMouse (  );

	// save dialog
	cstr SaveFile_s = "";
	if ( pOptionalDBOSaveFile == NULL )
	{
		int iInSaveDialog = 1;
		while ( iInSaveDialog == 1 ) 
		{
			SaveFile_s = openFileBox("FPSC Entity (.fpe)|*.fpe|All Files|*.*|", cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus\\").Get(), "Save Character", ".fpe", CHARACTERKITSAVEFILE);
			if ( SaveFile_s == "Error" ) return false;
			iInSaveDialog = 0;
		}
		strcpy ( pEntityName, SaveFile_s.Get() );
		pEntityName[strlen(pEntityName)-4] = 0;
		FPEFile_s = cstr(pEntityName)+".fpe";
		BMPFile_s = cstr(pEntityName)+".bmp";
		DBOFile_s = cstr(pEntityName)+".dbo";
	}

	// get character name
	cstr CharacterName_s = "";
	for ( int n = strlen(pEntityName); n >= 1; n-- )
	{
		if ( pEntityName[n] == '\\' || pEntityName[n] == '/' ) 
		{
			CharacterName_s = pEntityName + n + 1;
			break;
		}
	}


	// save DBO at specified location
	if ( FileExist(DBOFile_s.Get()) == 1 ) DeleteAFile ( DBOFile_s.Get() );
	SaveObject ( DBOFile_s.Get(), iCharObj );

	// character template
    cstr copyFrom_s = g.fpscrootdir_s+"\\Files\\entitybank\\Characters\\Uber Character.fpe";

	// prepare destination file
    cstr copyTo_s = FPEFile_s;
	if ( FileExist(FPEFile_s.Get()) == 1 ) DeleteAFile ( FPEFile_s.Get() );
	if ( FileOpen(1) == 1 ) CloseFile(1);
	if ( FileOpen(2) == 1 ) CloseFile(2);
	OpenToRead ( 1, copyFrom_s.Get() );
	OpenToWrite ( 2, copyTo_s.Get() );

	// go through all source FPE
	int iCount = 0;
	while ( FileEnd(1) == 0 ) 
	{
		// get line by line
		bool bSkipWritingReadLine = false;
		cstr line_s = ReadString ( 1 );

		// update description
		if ( cstr(Lower(Left(line_s.Get(),4))) == "desc" ) line_s = cstr("desc             = ") + CharacterName_s;

		// write out how this character was made up		
		if ( cstr(Lower(Left(line_s.Get(),11))) == "ccpassembly" )
		{
			cstr pCCPAssemblyString = "";
			for (int partscan = 0; partscan < 8; partscan++)
			{
				char pTrunc[260];
				if (partscan == 0) strcpy(pTrunc, cSelectedHeadGear);
				if (partscan == 1) strcpy(pTrunc, cSelectedHair);
				if (partscan == 2)
				{
					strcpy(pTrunc, cSelectedHead);
					strcat(pTrunc, " [");
					strcat(pTrunc, cSelectedICCode);
					strcat(pTrunc, "]");
				}
				if (partscan == 3) strcpy(pTrunc, cSelectedEyeglasses);
				if (partscan == 4) strcpy(pTrunc, cSelectedFacialHair);
				if (partscan == 5) strcpy(pTrunc, cSelectedBody);
				if (partscan == 6) strcpy(pTrunc, cSelectedLegs);
				if (partscan == 7) strcpy(pTrunc, cSelectedFeet);
				if (stricmp(pTrunc, "none") != NULL)
				{
					pCCPAssemblyString += pTrunc;
					if (partscan < 7) pCCPAssemblyString += ",";
				}
			}
			WriteString ( 2, cstr(cstr("ccpassembly      = ") + pCCPAssemblyString).Get() );
			bSkipWritingReadLine = true;
		}

		// replace some fields for destination
		if ( cstr(Lower(Left(line_s.Get(),6))) == "aimain" )
		{
			WriteString ( 2, cstr(cstr("aimain           = ") + g_CharacterCreatorPlus.obj.settings.script_s).Get() );
			WriteString ( 2, cstr(cstr("voice            = ") + g_CharacterCreatorPlus.obj.settings.voice_s).Get() );
			WriteString ( 2, cstr(cstr("speakrate        = ") + cstr(g_CharacterCreatorPlus.obj.settings.iSpeakRate)).Get() );
			bSkipWritingReadLine = true;
		}
		if ( cstr(Lower(Left(line_s.Get(),5))) == "model" )
		{
			WriteString ( 2, cstr(cstr("model            = ") + CharacterName_s + ".dbo").Get() );
			bSkipWritingReadLine = true;
		}
		if ( cstr(Lower(Left(line_s.Get(),8))) == "textured" )
		{
			WriteString ( 2, cstr(cstr("textured         = ") + "").Get() );
			bSkipWritingReadLine = true;
			//PE: Missing "textureref1 = " for save standalone.
		}

		// write line (changed or not) to the destination FPE
		if ( bSkipWritingReadLine == false )
		{
			WriteString ( 2, line_s.Get() );
		}
	}


	// close file handling
	CloseFile ( 1 );
	CloseFile ( 2 );
	
	// save thumbnail file
	if (iThumbnailImage > 0)
	{
		char pRealBMPFile[MAX_PATH];
		strcpy(pRealBMPFile, BMPFile_s.Get());
		GG_GetRealPath(pRealBMPFile, 1);
		if (FileExist(pRealBMPFile) == 1) DeleteAFile(pRealBMPFile);
		SaveImage(pRealBMPFile, iThumbnailImage);
	}

	// restore old dir
	SetDir ( olddir_s.Get() );

	// success
	return true;
}

void charactercreatorplus_loop(void)
{
	if ( g_CharacterCreatorPlus.bInitialised == false )
	{
		// set up for character editing
		charactercreatorplus_init();
	}
	else
	{
	}
}

// Character Creator for Multiplayer

void characterkitplus_chooseOnlineAvatar ( void )
{
	// new 'simple' method, select from file requester 
	char pOldDir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, pOldDir);
	SetDir(g.fpscrootdir_s.Get());
	SetDir("Files\\entitybank\\user\\charactercreatorplus");
	char pCCPDir[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, pCCPDir);
	t.tpickedAvatar_s = openFileBox("Character Files|*.bmp|", pCCPDir, "Select Character", ".bmp", 1);
	if (t.tpickedAvatar_s == "Error")
	{
		SetDir(pOldDir);
		return;
	}
	if (FileExist(t.tpickedAvatar_s.Get()) == 1)
	{
		t.tpickedAvatar_s = Right(t.tpickedAvatar_s.Get(), (Len(t.tpickedAvatar_s.Get()) - Len(pCCPDir))-1 );
		t.tpickedAvatar_s = Left(t.tpickedAvatar_s.Get(), Len(t.tpickedAvatar_s.Get()) - 4);
		SetDir(pOldDir);
		characterkitplus_saveAvatarInfo();
	}
	SetDir(pOldDir);
}

void characterkitplus_saveAvatarInfo ( void )
{
	t.tavatarstring_s = "";
	if ( FileExist( cstr(cstr("entitybank\\user\\charactercreatorplus\\")+t.tpickedAvatar_s+".fpe").Get() )  ==  1 ) 
	{
		// grab the character creator string
		t.tavatarstring_s = "";
		if ( FileOpen(1) == 1 ) CloseFile ( 1 );
		OpenToRead ( 1, cstr(cstr("entitybank\\user\\charactercreatorplus\\")+t.tpickedAvatar_s+".fpe").Get() );
		while ( FileEnd(1) == 0 ) 
		{
			t.tline_s = ReadString ( 1 );
			t.tcciStat_s = Lower(FirstToken( t.tline_s.Get(), " "));
			if ( t.tcciStat_s == "ccpassembly" ) 
			{
				LPSTR pStr = t.tline_s.Get();
				for (int n = 0; n < strlen(pStr); n++)
				{
					if (pStr[n] == '=')
					{
						n++; if (pStr[n] == ' ') n++;
						t.tavatarstring_s = pStr + n;
					}
				}
				break;
			}
		}
		CloseFile (  1 );

		// write out multiplayeravatar.dat file
		if ( FileExist( cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() ) == 1 ) DeleteAFile ( cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
		OpenToWrite ( 1, cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
		WriteString ( 1,t.tavatarstring_s.Get() );
		WriteString ( 1,t.tpickedAvatar_s.Get() );
		CloseFile ( 1 );

		// load in the avatar for multiplayer
		characterkitplus_loadMyAvatarInfo();
	}
}

void characterkitplus_checkAvatarExists ( void )
{
	if ( FileOpen(1) == 1 ) CloseFile ( 1 );
	if ( FileExist( cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() ) == 1 ) 
	{
		OpenToRead ( 1, cstr (g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
		g.mp.myAvatar_s = ReadString ( 1 );
		g.mp.myAvatarHeadTexture_s = ReadString ( 1 );
		g.mp.myAvatarName_s = g.mp.myAvatarHeadTexture_s;
		/* old system
		// head texture
		if ( g.mp.myAvatarHeadTexture_s != "" ) 
		{
			g.mp.myAvatarHeadTexture_s = g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus\\"+ g.mp.myAvatarHeadTexture_s + "_cc.dds";
			if ( FileExist(g.mp.myAvatarHeadTexture_s.Get()) == 0 )
			{
				g.mp.myAvatarHeadTexture_s = "";
				CloseFile ( 1 );
				DeleteAFile (  cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
				return;
			}
		}
		*/
		CloseFile (  1 );
	}
}

void characterkitplus_loadMyAvatarInfo ( void )
{
	// blank out the data first
	g.mp.myAvatar_s = "";
	g.mp.myAvatarName_s = "";
	g.mp.myAvatarHeadTexture_s = "";
	g.mp.haveSentMyAvatar = 0;
	t.bTriggerAvatarRescanAndLoad = false;
	for ( t.c = 0 ; t.c <= MP_MAX_NUMBER_OF_PLAYERS-1; t.c++ )
	{
		t.mp_playerAvatars_s[t.c] = "";
		t.mp_playerAvatarOwners_s[t.c] = "";
		t.mp_playerAvatarLoaded[t.c] = false;
	}

	// open multiplayer avatar file
	if ( FileExist( cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() ) == 1 ) 
	{
		OpenToRead ( 1, cstr (g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
		g.mp.myAvatar_s = ReadString ( 1 );
		g.mp.myAvatarHeadTexture_s = ReadString ( 1 );
		g.mp.myAvatarName_s = g.mp.myAvatarHeadTexture_s;
		/* old system
		// store the name of the head texture
		if ( g.mp.myAvatarHeadTexture_s != "" ) 
		{
			g.mp.myAvatarHeadTexture_s = g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator\\"+ g.mp.myAvatarHeadTexture_s + "_cc.dds";
			if ( FileExist(g.mp.myAvatarHeadTexture_s.Get()) == 0 ) g.mp.myAvatarHeadTexture_s = "";
		}
		*/
		CloseFile ( 1 );

		if ( t.tShowAvatarSprite == 1 ) 
		{
			t.tShowAvatarSprite = 0;
			if ( g.charactercreatorEditorImageoffset > 1 ) 
			{
				if ( ImageExist(g.charactercreatorEditorImageoffset) == 1 ) DeleteImage ( g.charactercreatorEditorImageoffset );
				if ( FileExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus\\"+g.mp.myAvatarName_s+".bmp").Get() ) == 1 ) 
				{
					LoadImage ( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus\\"+g.mp.myAvatarName_s+".bmp").Get(), g.charactercreatorEditorImageoffset );
				}
			}
		}
	}
}

void characterkitplus_makeMultiplayerCharacterCreatorAvatar ( void )
{
	// delete it if it exists (it shouldn't, but just in case)
	if ( FileExist(t.avatarFile_s.Get()) == 1 ) DeleteAFile ( t.avatarFile_s.Get() );

	// Store old dir
	t.tolddir_s=GetDir();

	// Check if user folder exists, if not create it
	if ( PathExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() ) == 0 ) 
	{
		MakeDirectory ( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() );
	}
	if ( PathExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus").Get() ) == 0 ) 
	{
		MakeDirectory ( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreatorplus").Get() );
	}

	// create correct name
	t.tSaveFile_s = t.avatarFile_s;
	t.tname_s = t.tSaveFile_s;
	if ( cstr(Lower(Right(t.tname_s.Get(),4))) == ".fpe"  )  t.tname_s = Left(t.tname_s.Get(),Len(t.tname_s.Get())-4);
	for ( t.tloop = Len(t.tname_s.Get()) ; t.tloop >= 1 ; t.tloop+= -1 )
	{
		if ( cstr(Mid(t.tname_s.Get(),t.tloop)) == "\\" || cstr(Mid(t.tname_s.Get(),t.tloop)) == "/" ) 
		{
			t.tname_s = Right(t.tname_s.Get(),Len(t.tname_s.Get())- t.tloop);
			break;
		}
	}

	// template reference
	#ifdef PHOTONMP
	 t.tcopyfrom_s = g.fpscrootdir_s+"\\Files\\entitybank\\Characters\\Uber Character.fpe";
	#else
	 t.tcopyfrom_s = g.fpscrootdir_s+"\\Files\\entitybank\\Characters\\Uber Soldier.fpe";
	#endif
	t.tcopyto_s = t.tSaveFile_s;
	if ( cstr(Lower(Right(t.tcopyto_s.Get(),4))) != ".fpe" ) t.tcopyto_s = t.tcopyto_s + ".fpe";

	// now modify the copy
	if ( FileOpen(1) ==  1 ) CloseFile ( 1 );
	if ( FileOpen(2) ==  1 ) CloseFile ( 2 );
	OpenToRead ( 1, t.tcopyfrom_s.Get() );
	OpenToWrite ( 2, t.tcopyto_s.Get() );
	while ( FileEnd(1) == 0 ) 
	{
		// line by line
		t.ts_s = ReadString ( 1 );
		if ( cstr(Lower(Left(t.ts_s.Get(),4))) == "desc" ) t.ts_s = cstr("desc           =  ") + t.tname_s;

		// replace model specified 
		if ( cstr(Lower(Left(t.ts_s.Get(),5))) == "model" ) t.ts_s = cstr("model          =  ") + t.tname_s + ".dbo";

		// replace ccpassembly field
		if ( cstr(Lower(Left(t.ts_s.Get(),11))) == "ccpassembly" ) t.ts_s = cstr(cstr("ccpassembly      = ") + t.avatarString_s).Get();

		// write back out
		WriteString ( 2, t.ts_s.Get() );
	}
	CloseFile ( 1 );
	CloseFile ( 2 );

	// created FPE, but have no DBO, so need to create one here
	char pTmpDBO[MAX_PATH];
	strcpy(pTmpDBO, t.tcopyto_s.Get());
	pTmpDBO[strlen(pTmpDBO)-4] = 0;
	strcat(pTmpDBO, ".dbo");

	// setup character parts, then trigger its creation (adult male hair 11,adult male head 01,adult male body 03,adult male legs 04e,adult male feet 04)
	iCharObj = g.characterkitobjectoffset + 1;
	iCharObjHeadGear = g.characterkitobjectoffset + 2;
	iCharObjHair = g.characterkitobjectoffset + 3;
	iCharObjHead = g.characterkitobjectoffset + 4;
	iCharObjEyeglasses = g.characterkitobjectoffset + 5;
	iCharObjFacialHair = g.characterkitobjectoffset + 6;
	iCharObjLegs = g.characterkitobjectoffset + 7;
	iCharObjFeet = g.characterkitobjectoffset + 8;
	strcpy(cSelectedLegsFilter, "");
	strcpy(cSelectedFeetFilter, "");
	strcpy(cSelectedICCode, "IC1a");
	strcpy(cSelectedHeadGear, "None");
	strcpy(cSelectedHair, "None");
	strcpy(cSelectedHead, "None");
	strcpy(cSelectedEyeglasses, "None");
	strcpy(cSelectedFacialHair, "None");
	strcpy(cSelectedBody, "None");
	strcpy(cSelectedLegs, "None");
	strcpy(cSelectedFeet, "None");
	char pICTag[MAX_PATH];
	strcpy(pICTag, "IC1a");
	char pBasePath[MAX_PATH];
	strcpy(pBasePath, "");
	char pAvatarAssembly[MAX_PATH];
	strcpy(pAvatarAssembly, t.avatarString_s.Get());
	int n = 0;
	while ( n <= strlen(pAvatarAssembly) )
	{
		bool bLastItem = false;
		if (n == strlen(pAvatarAssembly)) bLastItem = true;
		if ((n < strlen(pAvatarAssembly) && pAvatarAssembly[n] == ',') || bLastItem == true)
		{
			// get each part name
			char pPartName[MAX_PATH];
			strcpy(pPartName, pAvatarAssembly);
			pPartName[n] = 0;

			// find the part type
			if (strstr(pPartName, "headgear") != 0) strcpy(cSelectedHeadGear, pPartName);
			if (strstr(pPartName, "hair") != 0) strcpy(cSelectedHair, pPartName);
			if (strstr(pPartName, "head") != 0)
			{
				// head assembly name can include IC code (for avatar recreation)
				// get base
				strcpy(pBasePath, "charactercreatorplus\\parts\\");
				strcat(pBasePath, pPartName);
				LPSTR pHeadToken = strstr(pBasePath, " head");
				if (pHeadToken) *pHeadToken = 0;
				strcat(pBasePath, "\\");

				// find IC part
				LPSTR pICToken = strstr(pPartName, "[");
				if (pICToken)
				{
					// cut off IC part for selectedhead string
					strcpy(cSelectedHead, pPartName);
					LPSTR pICCutOff = strstr(cSelectedHead, " [");
					if (pICCutOff) *pICCutOff = 0;

					// and keep the IC part for the tag
					strcpy(pICTag, pICToken+1);
					if (pICTag[strlen(pICTag) - 1] == ']') 
						pICTag[strlen(pICTag) - 1] = 0;
				}
				else
				{
					strcpy(cSelectedHead, pPartName);
				}
			}
			if (strstr(pPartName, "eyeglasses") != 0) strcpy(cSelectedEyeglasses, pPartName);
			if (strstr(pPartName, "facialhair") != 0) strcpy(cSelectedFacialHair, pPartName);
			if (strstr(pPartName, "body") != 0) strcpy(cSelectedBody, pPartName);
			if (strstr(pPartName, "legs") != 0) strcpy(cSelectedLegs, pPartName);
			if (strstr(pPartName, "feet") != 0) strcpy(cSelectedFeet, pPartName);

			// prepare to get next one
			if (bLastItem == false)
			{
				strcpy(pAvatarAssembly, pAvatarAssembly + n + 1);
				n = 0;
			}
			else
			{
				strcpy(pAvatarAssembly, "");
				n = 1;
			}
		}
		else
		{
			n++;
		}
	}
	// now make a character object from parts
	image_preload_files_wait();
	object_preload_files_wait();
	charactercreatorplus_change(pBasePath, -1, pICTag);
	if (ObjectExist(iCharObj) == 1)
	{
		// save DBO at specified location
		if ( FileExist(pTmpDBO) == 1 ) DeleteAFile ( pTmpDBO );
		SaveObject(pTmpDBO, iCharObj);

		// and finally delete the unneeded object
		DeleteObject(iCharObj);
	}

	// Restore old dir
	SetDir ( t.tolddir_s.Get() );
}

void characterkitplus_removeMultiplayerCharacterCreatorAvatar ( void )
{
	// remove temp FPE file
	if ( FileExist(t.avatarFile_s.Get()) == 1 ) DeleteAFile ( t.avatarFile_s.Get() );

	// remove temp DBO file
	char pTmpDBO[MAX_PATH];
	strcpy(pTmpDBO, t.avatarFile_s.Get());
	pTmpDBO[strlen(pTmpDBO)-4] = 0;
	strcat(pTmpDBO, ".dbo");
	if ( FileExist(pTmpDBO) == 1 ) DeleteAFile ( pTmpDBO );
}
