
// PE: changed to be used in GameGuru

#include <algorithm>
#include <string>
#include <vector>
#include "preprocessor-flags.h"
#include "preprocessor-moreflags.h"

	#define TABENTITYNAME "Entities##LeftPanel"
	#define TABEDITORNAME "Editor##GGRenderarget"
	#define SMALLFONTSIZE 0.8
	#define SMALLESTFONTSIZE 0.7

//We need to do a reset layout when a new window is added to a new version , so increase this if this is the case.
#define V3VERSION 62
//PE: Increase MAXVERSION to set preference to defaults and reset imgui windows settings (for Friday builds).
//PE: This do not reset important user features like "Write folder" ...
#define MAXVERSION 17

#define ENABLEIMGUI
#define USERENDERTARGET
#define ALLOW_WEATHER_IN_EDITOR
#define ADDGGTOOLBAR

//PE: icons use tons of memory i have 12000 so...
#define DYNAMICLOADUNLOAD

//Image range reserved for UIV3
#define UIV3IMAGES 43000

#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;

IMGUI_IMPL_API bool     ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
IMGUI_IMPL_API void     ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_IMPL_API void     ImGui_ImplDX11_InvalidateDeviceObjects();
IMGUI_IMPL_API bool     ImGui_ImplDX11_CreateDeviceObjects();

#define TOOL_SHAPE UIV3IMAGES+1
#define TOOL_LEVELMODE UIV3IMAGES+2
#define TOOL_STOREDLEVEL UIV3IMAGES+3
#define TOOL_BLENDMODE UIV3IMAGES+4
#define TOOL_RAMPMODE UIV3IMAGES+5
#define TOOL_PAINTTEXTURE UIV3IMAGES+6
#define TOOL_PAINTGRASS UIV3IMAGES+7
#define TOOL_ENTITY UIV3IMAGES+8
#define TOOL_MARKERS UIV3IMAGES+9
#define TOOL_WAYPOINTS UIV3IMAGES+10
#define TOOL_NEWWAYPOINTS UIV3IMAGES+11
#define TOOL_TESTGAME UIV3IMAGES+12
#define TOOL_VRMODE UIV3IMAGES+13
#define TOOL_SOCIALVR UIV3IMAGES+14

#define TOOL_HEADER UIV3IMAGES+15

#define TOOL_NEWLEVEL UIV3IMAGES+16
#define TOOL_LOADLEVEL UIV3IMAGES+17
#define TOOL_SAVELEVEL UIV3IMAGES+18

#define ROUNDING_OVERLAY UIV3IMAGES+19

#define EBE_BLOCK UIV3IMAGES+20
#define EBE_COLUMN UIV3IMAGES+21
#define EBE_CUBE UIV3IMAGES+22
#define EBE_FLOOR UIV3IMAGES+23
#define EBE_NEW UIV3IMAGES+24
#define EBE_ROW UIV3IMAGES+25
#define EBE_STAIRS UIV3IMAGES+26
#define EBE_WALL UIV3IMAGES+27
#define EBE_EBE UIV3IMAGES+28


#define EBE_CONTROL1 UIV3IMAGES+30
#define EBE_CONTROL2 UIV3IMAGES+31
#define EBE_CONTROL3 UIV3IMAGES+32
#define EBE_CONTROL4 UIV3IMAGES+33
#define EBE_CONTROL5 UIV3IMAGES+34
#define EBE_CONTROL6 UIV3IMAGES+35
#define EBE_THUMB UIV3IMAGES+36


#define TOOL_BUILDER UIV3IMAGES+40
#define TOOL_CCP UIV3IMAGES+41
#define TOOL_IMPORT UIV3IMAGES+42

#define MEDIA_PLAY UIV3IMAGES+43
#define MEDIA_PAUSE UIV3IMAGES+44
#define MEDIA_REFRESH UIV3IMAGES+45
#define TUTORIAL_POINTER UIV3IMAGES+46
#define MEDIA_RECORD UIV3IMAGES+47
#define MEDIA_RECORDING UIV3IMAGES+48
#define MEDIA_RECORDPROCESSING UIV3IMAGES+49

#define HELPMENU_IMAGE UIV3IMAGES+50
#define ABOUT_LOGO UIV3IMAGES+51
#define ABOUT_HB UIV3IMAGES+52
#define ABOUT_TGC UIV3IMAGES+53

#define TUTORIAL_POINTERUP UIV3IMAGES+54

#define TOOL_SHAPE_UP UIV3IMAGES+55
#define TOOL_SHAPE_DOWN UIV3IMAGES+56

#define TOOL_DRAWWAYPOINTS UIV3IMAGES+57

#define TOOL_DOTCIRCLE UIV3IMAGES+58
#define TOOL_DOTCIRCLE_S UIV3IMAGES+59
#define TOOL_DOTCIRCLE_M UIV3IMAGES+60

#define TOOL_CIRCLE UIV3IMAGES+61
#define TOOL_CIRCLE_S UIV3IMAGES+62
#define TOOL_CIRCLE_M UIV3IMAGES+63

#define TOOL_ENT_EDIT UIV3IMAGES+71
#define TOOL_ENT_EXTRACT UIV3IMAGES+72
#define TOOL_ENT_DUPLICATE UIV3IMAGES+73
#define TOOL_ENT_LOCK UIV3IMAGES+74
#define TOOL_ENT_FINDFLOOR UIV3IMAGES+75
#define TOOL_ENT_DELETE UIV3IMAGES+76
#define TOOL_ENT_SEARCH UIV3IMAGES+77

#define TOOL_VISUALS UIV3IMAGES+80
#define TOOL_CAMERA UIV3IMAGES+81
#define TOOL_CAMERALIGHT UIV3IMAGES+82
#define TOOL_GOBACK UIV3IMAGES+83
#define MEDIA_MAXIMIZE UIV3IMAGES+84
#define MEDIA_MINIMIZE UIV3IMAGES+85

#define ENV_SUN UIV3IMAGES+86
#define ENV_RAIN UIV3IMAGES+87
#define ENV_SNOW UIV3IMAGES+88
#define ENV_WEATHER UIV3IMAGES+89

#define TOOL_RPG UIV3IMAGES+90
#define TOOL_PUZZLE UIV3IMAGES+91
#define TOOL_SHOOTER UIV3IMAGES+92

#define TOOL_TRIGGERZONE UIV3IMAGES+93
#define TOOL_FLAG UIV3IMAGES+94

#define TOOL_PENCIL UIV3IMAGES+95

#define UI3D_DOTOBJECTS UIV3IMAGES+96
#define UI3D_DOTMIDDLEOBJECTS UIV3IMAGES+97

#define SHAPE_CIRCLE UIV3IMAGES+100
#define SHAPE_SQUARE UIV3IMAGES+101

#define KEY_ALT UIV3IMAGES+105
#define KEY_BACKSPACE UIV3IMAGES+106
#define KEY_KEYBOARD UIV3IMAGES+107
#define KEY_CONTROL UIV3IMAGES+108
#define KEY_MINUS UIV3IMAGES+109
#define KEY_PLUS UIV3IMAGES+110
#define KEY_SHIFT UIV3IMAGES+111
#define MOUSE_LMB UIV3IMAGES+112
#define MOUSE_RMB UIV3IMAGES+113
//#define KEY_CONTROL_2 UIV3IMAGES+114 // used ?
#define KEY_SEPARATOR UIV3IMAGES+114
#define KEY_SEPARATOR_SMALL UIV3IMAGES+115

#define MEDIA_FAVORITE UIV3IMAGES+116

#define TOOL_GROUPEDIT UIV3IMAGES+117
#define TOOL_GROUP UIV3IMAGES+118
#define TOOL_UNGROUP UIV3IMAGES+119

#define TOOL_TRASHCAN UIV3IMAGES+120
#define TOOL_LOCK UIV3IMAGES+121
#define TOOL_UNLOCK UIV3IMAGES+122

#define MEDIA_FAVORITE_DIS UIV3IMAGES+123
#define KEY_MAXIMIZE UIV3IMAGES+124
#define MOUSE_MMB UIV3IMAGES+125

#define KEY_R UIV3IMAGES+126
#define KEY_DELETE UIV3IMAGES+127
#define KEY_Y UIV3IMAGES+128
#define KEY_RETURN UIV3IMAGES+129
#define KEY_PGUP UIV3IMAGES+130
#define KEY_PGDN UIV3IMAGES+131
#define KEY_F UIV3IMAGES+132
#define KEY_G UIV3IMAGES+133

#define OBJECT_MOVE_XZ UIV3IMAGES+134
#define OBJECT_MOVE_Y UIV3IMAGES+135
#define KEY_TAB UIV3IMAGES+136

#define KEY_CONTROL_SHIFT UIV3IMAGES+137

#define ICON_INFO UIV3IMAGES+138
#define OBJECT_MOVE_SURFACESCAN UIV3IMAGES+139
#define OBJECT_MOVE_FINDFLOOR UIV3IMAGES+140
#define OBJECT_MOVE_LOCK UIV3IMAGES+141
#define OBJECT_MOVE_UNLOCK UIV3IMAGES+142

#define TOOL_ENT_FILTER UIV3IMAGES+143

#define MARKETPLACE_GGMAX UIV3IMAGES+144
#define MARKETPLACE_GCSTORE UIV3IMAGES+145
#define MARKETPLACE_SKETCHFAB UIV3IMAGES+146
#define MARKETPLACE_FILLER UIV3IMAGES+147
#define MARKETPLACE_IMPORT UIV3IMAGES+148
#define WELCOME_HEADER UIV3IMAGES+149
#define WELCOME_FILLERROUNDED UIV3IMAGES+150

#define FILETYPE_OGG UIV3IMAGES+151
#define FILETYPE_MP3 UIV3IMAGES+152
#define FILETYPE_WAV UIV3IMAGES+153
#define MARKETPLACE_SHOCKWAVESOUND UIV3IMAGES+154

#define FILETYPE_VIDEO UIV3IMAGES+155
#define FILETYPE_SCRIPT UIV3IMAGES+156
#define PLAYER_START UIV3IMAGES+157
#define FILETYPE_PARTICLE UIV3IMAGES+158

#define ENTITY_PARTICLE UIV3IMAGES+159
#define ENTITY_LIGHT UIV3IMAGES+160
#define ENTITY_WIN UIV3IMAGES+161
#define ENTITY_IMAGE UIV3IMAGES+162
#define ENTITY_MUSIC UIV3IMAGES+163
#define ENTITY_SOUND UIV3IMAGES+164
#define ENTITY_TEXT UIV3IMAGES+165
#define ENTITY_VIDEO UIV3IMAGES+166

#define MARKETPLACE_COMMUNITY UIV3IMAGES+167

#define LIGHT_POINT UIV3IMAGES+168
#define LIGHT_SPOT UIV3IMAGES+169

#define ENTITY_START UIV3IMAGES+170
#define ENTITY_CHECKPOINT UIV3IMAGES+171

#define FILTER_CHAR_ON UIV3IMAGES+173
#define FILTER_CHAR_OFF UIV3IMAGES+174
#define FILTER_SCENARY_ON UIV3IMAGES+175
#define FILTER_SCENARY_OFF UIV3IMAGES+176
#define FILTER_FAVORITE_ON UIV3IMAGES+177
#define FILTER_FAVORITE_OFF UIV3IMAGES+178
#define FILTER_HUD_ON UIV3IMAGES+179
#define FILTER_HUD_OFF UIV3IMAGES+180
#define FILTER_ELEMENTS_ON UIV3IMAGES+181
#define FILTER_ELEMENTS_OFF UIV3IMAGES+182
#define FILTER_USER_ON UIV3IMAGES+183
#define FILTER_USER_OFF UIV3IMAGES+184
#define FILTER_DLUA_ON UIV3IMAGES+185
#define FILTER_DLUA_OFF UIV3IMAGES+186

#define MEDIA_PIN UIV3IMAGES+187
#define MEDIA_UNPIN UIV3IMAGES+188

#define TOOL_GROUPSAVE UIV3IMAGES+189

#define ENTITY_FLAG UIV3IMAGES+190
#define ENTITY_GUNS UIV3IMAGES+191
#define ENTITY_AMMO UIV3IMAGES+192
#define ENTITY_ENEMIES UIV3IMAGES+193
#define ENTITY_ALLIES UIV3IMAGES+194

#define INFOIMAGE UIV3IMAGES+195
#define TOOL_SMARTOBJECT UIV3IMAGES+196

//198,197 also used in backdrop.
#define BACKDROPMAGE UIV3IMAGES+199
#define SKYBOX_ICONS UIV3IMAGES+200
#define MARKETPLACE_ICONS UIV3IMAGES+300
#define ENTITY_CACHE_ICONS UIV3IMAGES+1000
#define ENTITY_CACHE_ICONS_LARGE UIV3IMAGES+2000
#define PROPERTIES_CACHE_ICONS UIV3IMAGES+3000
#define IMPORTER_ALL_MESH UIV3IMAGES + 4000

bool overWriteFileBox(char * file);
bool CancelQuit();
bool askBox(char * ask, char *title);
bool changedFileBox(char * file);
void BoxerInfo(char * text, const char *heading);
void DebugInfo(char * text, const char *heading);
int askBoxCancel(char * ask, char *title);

enum {
	NOC_FILE_DIALOG_OPEN = 1 << 0,   // Create an open file dialog.
	NOC_FILE_DIALOG_SAVE = 1 << 1,   // Create a save file dialog.
	NOC_FILE_DIALOG_DIR = 1 << 2,   // Open a directory.
	NOC_FILE_DIALOG_OVERWRITE_CONFIRMATION = 1 << 3,
};
const char *noc_file_dialog_open(int flags,
	const char *filters,
	const char *default_path,
	const char *default_name, 
	bool bUseDefaultPath = false, 
	const char* pTitle = NULL);

namespace ImGui {
	bool BeginPopupContextItemAGK(const char* str_id = NULL, int mouse_button = 1);
	const char* CalcWordWrapPositionB(float scale, const char* text, const char* text_end, float wrap_width, float line_start);
	void TextCenter(const char* fmt, ...);
	bool StyleButtonEx(const char* label, const ImVec2& size_arg = ImVec2(0, 0), bool bDisabled = false);
	bool StyleButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0));
	bool StyleButtonDark(const char* label, const ImVec2& size_arg = ImVec2(0, 0));
	bool HyberlinkButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0));
	bool MinMaxButtonEx(const char* str_id, ImGuiDir dir);
	void RenderArrowOutLine(ImDrawList* draw_list, ImVec2 pos, ImU32 col, ImGuiDir dir, float scale);
	bool StyleCollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0);
	bool IsLastSliderHovered(void);
	bool MaxSliderInputFloat(const char* label, float* v, float v_min, float v_max, const char* tooltip, int startval=0, float maxval = 100.0f, int numericboxwidth = 30.0f);
	bool MaxSliderInputFloat2(const char* label, float* v, float v_min, float v_max, const char* tooltip, int startval = 0, float maxval = 100.0f, int numericboxwidth = 30.0f);
	bool MaxSliderInputInt(const char* label, int* v, int v_min, int v_max, const char* tooltip);
	bool MaxSliderInputRangeFloat(const char* label, float* v, float* v2, float v_min, float v_max, const char* tooltip);
	bool windowTabVisible(void);
	int windowTabFlags(void);
	int windowDockNodeId(void);
	void SetBlurMode(bool blur);

	bool ImgBtn(int iImageID, const ImVec2& btn_size = ImVec2(0, 0), const ImVec4& bg_col = ImColor(255, 255, 255, 0),
		const ImVec4& drawCol_normal = ImColor(220, 220, 220, 220),
		const ImVec4& drawCol_hover = ImColor(255, 255, 255, 255),
		const ImVec4& drawCol_Down = ImColor(180, 180, 160, 255), int frame_padding = -1, int atlasindex = 0, int atlasrows = 0, int atlascolumns = 0, bool nowhite = false, bool gratiant = false, bool center_image = false, bool noalpha = false, bool useownid = false, bool boost25 = false);
	bool ImgBtnBack(int iImageID, const ImVec2& btn_size = ImVec2(0, 0), const ImVec4& bg_col = ImColor(255, 255, 255, 0),
		const ImVec4& drawCol_normal = ImColor(220, 220, 220, 220),
		const ImVec4& drawCol_hover = ImColor(255, 255, 255, 255),
		const ImVec4& drawCol_Down = ImColor(180, 180, 160, 255), int frame_padding = 0);

	void ToggleButton(const char* str_id, bool* v);
	bool RoundButton(const char* str_id,ImVec2 size, float radius);
	bool MaxIsItemFocused(void);
	void ImRotateStart(void);
	ImVec2 ImRotationCenter(void);
	void ImRotateEnd(float rad, ImVec2 center);
	void ImGui_GG_HideWindow(ImGuiViewport* viewport);
	void ImGui_GG_ShowWindow(ImGuiViewport* viewport);
	void HideAllViewPortWindows(void);
	void ShowAllViewPortWindows(void);
	bool BeginTimeline(const char* str_id, float max_value);
	bool TimelineEvent(const char* str_id, float* values);
	void EndTimeline(void);
	bool RangeSlider(const char* str_id, float & val1, float & val2, float max_value, bool bDisplayValues = true);
}

#define REMEMBERLASTFILES 5
#define MAXSEARCHHISTORY 7

#define SECTION_VISUALS 1
#define SECTION_LEVEL_ENTITIES 2
#define SECTION_ENTITY_TOOLS 3
#define SECTION_SCULPT_TERRAIN 4
#define SECTION_PAINT_TERRAIN 5
#define SECTION_ADD_VEGETATION 6
#define SECTION_WAYPOINTS 7
#define SECTION_CHARACTER_CREATOR 8
#define SECTION_STRUCTURE_EDITOR 9
#define SECTION_IMPORTER 10
#define SECTION_WEATHER 11

#define SECTION_SHOOTERGENRE 12
#define SECTION_RPGGENRE 13
#define SECTION_PUZZLEGENRE 14


struct preferences {
	char szCheckFile[15] = "GAMEGURU-PREFS";
	int launched = 0;
	int current_style = 3; //Default to light style
	ImVec2 vStartResolution = { 1280,800 };
	char last_open_files[10][260] = { "\0","\0","\0","\0","\0","\0","\0","\0","\0","\0" };
	int save_layout = true;
	int dummy_filler = 0;
	int current_version = 0;
	char search_history[15][260] = { "\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0" };
	char filler[260];
	int iMaximized = 1;
	bool bHideTutorials = false;
	bool bMultiplyOpenHeaders = false;
	bool bAutoClosePropertySections = true;
	bool bDisableMultipleViewport = false;
	int dummy_filler2 = 0; //PE: Importent we align to 32bit when using bools here. thats why we have fillers.
	int iAllowUndocking = false;
	ImVec4 tint_style = ImVec4(0.0, 0.0, 0.0, 0.0);
	ImVec4 shade_style = ImVec4(0.0, 0.0, 0.0, 0.0);
	char small_search_history[15][260] = { "\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0" };
	int iTurnOffUITransparent = false;
	int iPlayedVideoSection[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int iResetAutoRunVideosOnNextStartup = 0;
	int iEnableCustomColors = 0;
	int iEnableDragDropEntityMode = 1;
	ImVec4 title_style = ImVec4(0.0, 0.0, 0.0, 0.0);
	int iEnableAdvancedSky = 0;
	int iEnableAdvancedWater = 0;
	int iEnableAdvancedPostProcessing = 0;
	int iEnableAdvancedShadows = 0;
	int iEnableArcRelationshipLines = 1;
	int iEnableRelationPopupWindow = 0;
	int iEnableAxisRotationShortcuts = 0;
	int iObjectEnableAdvanced = 0;
	int iEnableDragDropWidgetSelect = 0;
	int iEnableDragDropMousePickSystem = 0;
	int iEnableEditorOutlineSelection = 1;
	int iEnableSingleRightPanelAdvanced = 0;
	char cCustomWriteFolder[260] = { "\0" };
	int iEnableDeveloperProperties = 0;
	int iEnableIdentityProperties = 0;
	int iDragCameraMovement = 1;
	int bAutoOpenMenuItems = true;
	int iGameCreaterStore = 0;
	char cDefaultImportPath[260] = { "\0" };
	int iFullscreenPreviewAdvanced = 0;
	char last_import_files[10][260] = { "\0","\0","\0","\0","\0","\0","\0","\0","\0","\0" };
	int iDisplayWelcomeScreen = 1;
	float vSaved_Light_Palette_R[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	float vSaved_Light_Palette_G[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	float vSaved_Light_Palette_B[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	int iSaved_Light_Type[16] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	int iSaved_Light_Range[16] = { 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	float fSaved_Light_ProbeScale[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };

	//Particles.
	char Saved_Particle_Name[16][260] = { "\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0" };
	int Saved_bParticle_Preview[16] = { true,true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };
	int Saved_bParticle_Show_At_Start[16] = { true,true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };
	int Saved_bParticle_Looping_Animation[16] = { true,true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };
	int Saved_bParticle_Full_Screen[16] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	float Saved_fParticle_Fullscreen_Duration[16] = { 10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f,10.0f };
	float Saved_fParticle_Fullscreen_Fadein[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	float Saved_fParticle_Fullscreen_Fadeout[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	char Saved_Particle_Fullscreen_Transition[16][260] = { "\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0" };
	float Saved_fParticle_Speed[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	float Saved_fParticle_Opacity[16] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	int iEnableAdvancedEntityList = 0;
	float fHighLightThickness = 1.0;

	int iTurnOffEditboxTooltip = false;
	int iImporterDome = 1;
	int iEnableAutoExposureInEditor = 0;
	char cRememberLastSearchObjects[260] = { "\0" };
	int iSetColumnsEntityLib = 3;
	int iCheckboxFilters[10] = { 1,1,1,1,1,1,1,1,1,1 };
};

void coreResetIMGUIFunctionalityPrefs(void);

void myDarkStyle(ImGuiStyle* dst);
void myLightStyle(ImGuiStyle* dst);
void myStyle(ImGuiStyle* dst);
void myStyle2(ImGuiStyle* dst);
void myStyle2_colors_only(void);
void myStyle3(ImGuiStyle* dst);
void TintCurrentStyle(void);
void myStyleBlue(ImGuiStyle* dst);

void ChangeGGFont(const char *cpcustomfont, int iIDEFontSize);
bool NoCaseLess(const std::string &a, const std::string &b);
void GetMainEntityList(char* folder_s, char* rel_s, void *pFolder ,char* folder_name_start="", bool bForceToTop=false, int foldertype = 0);

int cstring_cmp_folder(const void *a, const void *b);
void RefreshEntityFolder(char* folder_s, void *pFolder);

// done in cStr
//void replaceAll(std::string& str, const std::string& from, const std::string& to);

void FindAWriteablefolder(void);
void AddPayLoad(ImGuiPayload* payload, bool addtocursor = false);

enum eKeyboardShortcutType
{
	eKST_Sculpt,
	eKST_Paint,
	eKST_AddVeg,
	eKST_ObjectMode,
	eKST_CharacterCreator,
	eKST_ObjectLibrary,
	eKST_Last
};
void UniversalKeyboardShortcut(eKeyboardShortcutType KST);
