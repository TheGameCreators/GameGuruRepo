//----------------------------------------------------
//--- GAMEGURU - M-Waypoint
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

#ifdef ENABLEIMGUI
//PE: GameGuru IMGUI.
#include "..\Imgui\imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "..\Imgui\imgui_internal.h"
#include "..\Imgui\imgui_impl_win32.h"
#include "..\Imgui\imgui_gg_dx11.h"

bool bWaypointDrawmode = false;
int iDrawPoints = 0;
int iWaypointDeleteMode = 1;
int iSpreadingInterval = 108;
extern bool bWaypoint_Window;
extern bool bImGuiGotFocus;
extern preferences pref;
extern bool bForceKey;
extern cstr csForceKey;

extern bool bHelp_Window;
extern bool bHelpVideo_Window;
extern char cForceTutorialName[1024];
bool bOnlyFollowTerrain = false;

#endif



//Subroutines for waypoint system

#ifdef ENABLEIMGUI
void waypoint_imgui_loop(void)
{
	if (bWaypointDrawmode)
		waypoint_drawmode_loop();

	if (bWaypoint_Window && t.grideditselect == 6 ) 
	{
		//Make sure window is setup in docking space.
		extern int iGenralWindowsFlags;
		ImGui::Begin("Waypoints##WaypointsToolsWindow", &bWaypoint_Window, iGenralWindowsFlags);

		float w = ImGui::GetWindowContentRegionWidth();
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		int current_mode = 0;
		float media_icon_size = 40.0f;
		float plate_width = (media_icon_size + 6.0) * 4.0f;
		int icon_size = 60;
		ImVec2 iToolbarIconSize = { (float)icon_size, (float)icon_size };
		ImVec2 tool_selected_padding = { 1.0, 1.0 };
		ImVec4 tool_selected_col = ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram];
		if (pref.current_style == 3)
			tool_selected_col = ImGui::GetStyle().Colors[ImGuiCol_Button];

		if (bWaypointDrawmode)
			current_mode = TOOL_DRAWWAYPOINTS;
		else
			current_mode = TOOL_WAYPOINTS;

		if (ImGui::StyleCollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {

			float control_width = (icon_size) * 2.0f + 6.0; ////PE: 3.0f - Old add waypoint , removed for now.
			int indent = (w*0.5) - (control_width*0.5);
			if (indent < 10)
				indent = 10;
			ImGui::Indent(indent);

			if (current_mode == TOOL_WAYPOINTS)	window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
			if (ImGui::ImgBtn(TOOL_WAYPOINTS, iToolbarIconSize, ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false)) {
				bWaypointDrawmode = false;
				bForceKey = true;
				csForceKey = "p";
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Waypoint Editing Mode (P)");
			ImGui::SameLine();

			if (current_mode == TOOL_DRAWWAYPOINTS) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
			if (ImGui::ImgBtn(TOOL_DRAWWAYPOINTS, iToolbarIconSize, ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false)) {
				//Draw waypoint mode

				bWaypointDrawmode = true;
				iWaypointDeleteMode = 1;
				iDrawPoints = 0;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Draw Waypoint Path");

			//PE: Old add waypoint , removed for now.
//			ImGui::SameLine();
//			if (current_mode == TOOL_NEWWAYPOINTS) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
//			if (ImGui::ImgBtn(TOOL_NEWWAYPOINTS, iToolbarIconSize, ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false)) {
//				bForceKey = true;
//				csForceKey = "p";
//				t.inputsys.domodewaypointcreate = 1;
//				iWaypointDeleteMode = 1;
//				bWaypointDrawmode = false;
//			}
//			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Create New Waypoint");

			ImGui::Indent(-indent);
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));

		}
		if (ImGui::StyleCollapsingHeader("Mode", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));

			int control_image_size = 34;
			float control_width = (control_image_size + 3.0) * 5.0f + 6.0;
			int indent = (w*0.5) - (control_width*0.5);
			if (indent < 10)
				indent = 10;
			ImGui::Indent(indent);

			if (iWaypointDeleteMode == 1)
			{
				ImVec2 padding = { 3.0, 3.0 };
				const ImRect image_bb((window->DC.CursorPos - padding), window->DC.CursorPos + padding + ImVec2(control_image_size, control_image_size));
				window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
			}
			if (ImGui::ImgBtn(EBE_CONTROL1, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false)) {
				//Paint mode.
				iWaypointDeleteMode = 1;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Edit Mode");
			ImGui::SameLine();

			if (iWaypointDeleteMode != 1)
			{
				ImVec2 padding = { 3.0, 3.0 };
				const ImRect image_bb((window->DC.CursorPos - padding), window->DC.CursorPos + padding + ImVec2(control_image_size, control_image_size));
				window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
			}
			if (ImGui::ImgBtn(EBE_CONTROL2, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false)) {
				//Remove mode.
				iWaypointDeleteMode = 0;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete Mode");
			ImGui::SameLine();

			ImVec2 cp = ImGui::GetCursorPos();

			//ImGui::Text("Brush Size:");
			ImGui::SetItemAllowOverlap();
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(cp.x, cp.y + (ImGui::GetFontSize() * 1.5)));
			ImGui::PushItemWidth((control_image_size + 6.0) * 3.0);
			ImGui::SetWindowFontScale(0.5);
			ImVec4 oldFrameBg = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
			ImVec4 oldBorder = ImGui::GetStyle().Colors[ImGuiCol_Border];

			ImGui::GetStyle().Colors[ImGuiCol_FrameBg].w *= 0.25;
			ImGui::GetStyle().Colors[ImGuiCol_Border].w *= 0.25;

			if (ImGui::SliderInt("##Spreadingsize", &iSpreadingInterval, 20, 200, "")) { //g.fTerrainBrushSizeMax
				if (iSpreadingInterval < 10) iSpreadingInterval = 10;
				if (iSpreadingInterval > 200) iSpreadingInterval = 300;
			}
			ImGui::GetStyle().Colors[ImGuiCol_FrameBg].w = oldFrameBg.w;
			ImGui::GetStyle().Colors[ImGuiCol_Border].w = oldBorder.w;
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Spreading Intervals %d", iSpreadingInterval);
			ImGui::PopItemWidth();
			ImGui::SetWindowFontScale(1.0);

			ImGui::SetCursorPos(cp);

			ImGui::SetItemAllowOverlap();
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 5));
			#ifdef PRODUCTV3
			 //if (ImGui::RoundButton("Spreading Interval Low", ImVec2(control_image_size, control_image_size - 8.0), 6)) {
			 if (ImGui::ImgBtn(TOOL_DOTCIRCLE_S, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false))
			#else
			 if (ImGui::ImgBtn(TOOL_CIRCLE_S, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false))
			#endif
			{
				iSpreadingInterval = 37;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Spreading Interval Low");

			ImGui::SameLine();

			ImGui::SetItemAllowOverlap();
			#ifdef PRODUCTV3
			 //if (ImGui::RoundButton("Spreading Interval Medium", ImVec2(control_image_size, control_image_size - 8.0), 10)) 
			 if (ImGui::ImgBtn(TOOL_DOTCIRCLE_M, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false))
			#else
			 if (ImGui::ImgBtn(TOOL_CIRCLE_M, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false))
			#endif
			{
				iSpreadingInterval = 108;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Spreading Interval Medium");

			ImGui::SameLine();

			ImGui::SetItemAllowOverlap();
			#ifdef PRODUCTV3
			 //if (ImGui::RoundButton("Spreading Interval High", ImVec2(control_image_size, control_image_size - 8.0), 14)) 
			 if (ImGui::ImgBtn(TOOL_DOTCIRCLE, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false))
			#else
			 if (ImGui::ImgBtn(TOOL_CIRCLE, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false, false, false, false))
			#endif
			{
				iSpreadingInterval = 180;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Spreading Interval High");
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 5));
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 8));
			ImGui::Indent(-indent);
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
		}

		if (!pref.bHideTutorials)
		{

			if (ImGui::StyleCollapsingHeader("Tutorial (this feature is incomplete)", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(10);
				cstr cShowTutorial = "01 - Getting started";
				void SmallTutorialVideo(char *tutorial, char* combo_items[] = NULL, int combo_entries = 0, int iVideoSection = 0);
				char* tutorial_combo_items[] = { "01 - Getting started", "02 - Creating terrain", "03 - Add character and set a path" };
				SmallTutorialVideo(cShowTutorial.Get(), tutorial_combo_items, ARRAYSIZE(tutorial_combo_items), SECTION_WAYPOINTS);
				float but_gadget_size = ImGui::GetFontSize()*12.0;
				float w = ImGui::GetWindowContentRegionWidth() - 10.0;
				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (but_gadget_size*0.5), 0.0f));
				#ifdef INCLUDESTEPBYSTEP
				if (ImGui::StyleButton("View Step by Step Tutorial", ImVec2(but_gadget_size, 0)))
				{
					// pre-select tutorial 03
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

		if (ImGui::StyleCollapsingHeader("Keyboard Shortcuts ???", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(10);
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.0f, 4.0f));


			ImGui::Checkbox("Draw to Terrain Height", &bOnlyFollowTerrain);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Draw Waypoint to Terrain Height");

			// context help button
			float button_gadget_size = ImGui::GetFontSize()*10.0;
			float w = ImGui::GetWindowContentRegionWidth();
			ImGui::Text("Left Mouse Button to Move Points.");
			ImGui::Text("Shift + Left Mouse Button to Clone.");
			ImGui::Text("Shift + Right Mouse Button to Delete.");
			ImGui::Indent(-10);
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
		}

		ImRect bbwin(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
		if (ImGui::IsMouseHoveringRect(bbwin.Min, bbwin.Max))
		{
			bImGuiGotFocus = true;
		}
		if (ImGui::IsAnyItemFocused()) {
			bImGuiGotFocus = true;
		}

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

float placeatx_f, placeatz_f;
float last_placeatx_f, last_placeatz_f;
int iNewWaypointPath = -1;

void waypoint_drawmode_loop(void)
{

	if (bWaypointDrawmode && t.inputsys.keypress == 0)
	{
		placeatx_f = t.inputsys.localx_f;
		placeatz_f = t.inputsys.localy_f;
		g.waypointeditheight_f = t.inputsys.localcurrentterrainheight_f;

		if (bOnlyFollowTerrain)
		{
			if (t.terrain.TerrainID > 0 || ObjectExist(t.terrain.terrainobjectindex))
			{
				//t.terrain.terrainobjectindex
				g.waypointeditheight_f = BT_GetGroundHeight(t.terrain.TerrainID, placeatx_f, placeatz_f);
			}
			else
			{
				g.waypointeditheight_f = g.gdefaultterrainheight;
			}
		}

		if (ObjectExist(g.editorwaypointoffset + 0) == 0)
		{
			float fSphereSize = 25.0f;
			MakeObjectSphere(g.editorwaypointoffset + 0, fSphereSize);
			SetObjectCollisionOff(g.editorwaypointoffset + 0);
			SetAlphaMappingOn(g.editorwaypointoffset + 0, 25);
			DisableObjectZRead(g.editorwaypointoffset + 0);
			HideObject(g.editorwaypointoffset + 0);
			SetObjectMask(g.editorwaypointoffset + 0, 1);
			SetObjectEffect(g.editorwaypointoffset + 0, g.guishadereffectindex);
			SetObjectEmissive(g.editorwaypointoffset + 0, Rgb(255, 255, 255));
			SetObjectMask(g.editorwaypointoffset + 0, 1);
		}
		if (ObjectExist(g.editorwaypointoffset + 0) == 1)
		{
			PositionObject(g.editorwaypointoffset + 0, placeatx_f, t.inputsys.localcurrentterrainheight_f + 5, placeatz_f);
			ShowObject(g.editorwaypointoffset + 0);
		}
		if (iDrawPoints == 0 && t.mclick == 1)
		{
			t.inputsys.domodewaypointcreate = 0;
			t.inputsys.keypress = 1; t.inputsys.domodewaypoint = 1; t.grideditselect = 6;
			t.waypointeditstyle = 1; t.waypointeditstylecolor = 0; t.waypointeditentity = 0;
			t.mx_f = placeatx_f;
			t.mz_f = placeatz_f;
			waypoint_createnew();
			iDrawPoints++;

			last_placeatx_f = placeatx_f;
			last_placeatz_f = placeatz_f;

			iNewWaypointPath = t.waypointindex;
		}
		else if (iDrawPoints > 0 && t.mclick == 1)
		{
			int iOffset = iSpreadingInterval;
			if( (placeatx_f > last_placeatx_f+iOffset) || (placeatx_f < last_placeatx_f - iOffset) ||
				(placeatz_f > last_placeatz_f + iOffset) || (placeatz_f < last_placeatz_f - iOffset) )
			{
				t.waypointiovercursor = g.waypointcoordmax;
				t.w = g.waypointcoordmax + 1;
				t.waypointindex = iNewWaypointPath;

				if (t.w >= g.waypointcoordmax)
				{
					//  at end
					++g.waypointcoordmax;
					Dim(t.waypointcoord, g.waypointcoordmax);
					t.w = g.waypointcoordmax;
				}

				t.waypointcoord[t.w].x = placeatx_f;
				t.waypointcoord[t.w].y = g.waypointeditheight_f;
				t.waypointcoord[t.w].z = placeatz_f;
				t.waypointcoord[t.w].link = 0;
				t.waypointcoord[t.w].index = t.waypointindex;
				t.waypoint[t.waypointindex].count = t.waypoint[t.waypointindex].count + 1;
				t.waypoint[t.waypointindex].finish = t.waypoint[t.waypointindex].finish + 1;

				for (t.twayp = 1; t.twayp <= g.waypointmax; t.twayp++)
				{
					if (t.twayp != t.waypointindex)
					{
						if (t.w <= t.waypoint[t.twayp].start)  t.waypoint[t.twayp].start = t.waypoint[t.twayp].start + 1;
						if (t.w <= t.waypoint[t.twayp].finish)  t.waypoint[t.twayp].finish = t.waypoint[t.twayp].finish + 1;
					}
					for (t.tw = t.waypoint[t.twayp].start; t.tw <= t.waypoint[t.twayp].finish; t.tw++)
					{
						if (t.waypointcoord[t.tw].link > 0)
						{
							if (t.w <= t.waypointcoord[t.tw].link)  t.waypointcoord[t.tw].link = t.waypointcoord[t.tw].link + 1;
						}
					}
				}
				if (t.waypoint[t.waypointindex].count > 0)
				{
					createwaypointobj(g.editorwaypointoffset + t.waypointindex, t.waypointindex);
				}
				else
				{
					if (ObjectExist(g.editorwaypointoffset + t.waypointindex) == 1)
					{
						DeleteObject(g.editorwaypointoffset + t.waypointindex);
					}
				}
				last_placeatx_f = placeatx_f;
				last_placeatz_f = placeatz_f;
			}
		}
		else if (iDrawPoints > 0 && t.mclick == 0)
		{
			HideObject(g.editorwaypointoffset + 0);
			bWaypointDrawmode = false;
		}
	}
}
#endif

void waypoint_savedata ( void )
{
	//  Save list
	//if (  FileExist("levelbank\\testmap\\map.way") == 1  )  DeleteAFile (  "levelbank\\testmap\\map.way" );
	//OpenToWrite (  1,"levelbank\\testmap\\map.way" );
	cstr waypointSaveData_s = g.mysystem.levelBankTestMap_s + "map.way";
	if ( FileExist(waypointSaveData_s.Get()) == 1  )  DeleteAFile ( waypointSaveData_s.Get() );
	OpenToWrite ( 1, waypointSaveData_s.Get() );
	//  strands
	WriteLong ( 1, g.waypointmax );
	if (  g.waypointmax>0 ) 
	{
		for ( t.w = 1 ; t.w<=  g.waypointmax; t.w++ )
		{
			t.a=t.waypoint[t.w].style ; WriteLong (  1,t.a );
			t.a=t.waypoint[t.w].fillcolor ; WriteLong (  1,t.a );
			t.a=t.waypoint[t.w].linkedtoentityindex ; WriteLong (  1,t.a );
			t.a=t.waypoint[t.w].count ; WriteLong (  1,t.a );
			t.a=t.waypoint[t.w].start ; WriteLong (  1,t.a );
			t.a=t.waypoint[t.w].finish ; WriteLong (  1,t.a );
		}
	}
	//  coords
	//  Dave fix, on some older maps waypointcoordmax was a negative number
	//  which would cause it to show up as 4gb when diming
	//  so now we check if it is negative and correct it
	if (  g.waypointcoordmax < 0  )  g.waypointcoordmax  =  0;
	WriteLong (  1,g.waypointcoordmax );
	if (  g.waypointcoordmax>0 ) 
	{
		for ( t.w = 1 ; t.w<=  g.waypointcoordmax; t.w++ )
		{
			t.a_f=t.waypointcoord[t.w].x ; WriteFloat (  1,t.a_f );
			t.a_f=t.waypointcoord[t.w].y ; WriteFloat (  1,t.a_f );
			t.a_f=t.waypointcoord[t.w].z ; WriteFloat (  1,t.a_f );
			t.a=t.waypointcoord[t.w].link ; WriteLong (  1,t.a );
			t.a=t.waypointcoord[t.w].index ; WriteLong (  1,t.a );
		}
	}
	CloseFile (  1 );
}

void waypoint_validate_style2(void)
{
	//Validate style 2. must have a entity pointing to the waypointdata.
	if (g.waypointmax > 0)
	{
		for (t.waypointindex = 1; t.waypointindex <= g.waypointmax; t.waypointindex++)
		{
			if (t.waypoint[t.waypointindex].count > 0 && t.waypoint[t.waypointindex].style == 2)
			{
				bool bWaypointFound = false;
				for (t.e = 1; t.e <= g.entityelementlist; t.e++)
				{
					int iEntID = t.entityelement[t.e].bankindex;
					if (iEntID > 0)
					{
						if (t.entityprofile[iEntID].ismarker == 3 || t.entityprofile[iEntID].ismarker == 6 || t.entityprofile[iEntID].ismarker == 8)
						{
							if (t.entityelement[t.e].eleprof.trigger.waypointzoneindex == t.waypointindex)
							{
								bWaypointFound = true;
								break;
							}
						}
					}
				}
				if (!bWaypointFound)
				{
					//Remove waypoint.
					t.waypoint[t.waypointindex].style = 0;
					t.waypoint[t.waypointindex].fillcolor = 0;
					t.waypoint[t.waypointindex].linkedtoentityindex = 0;
					t.waypoint[t.waypointindex].count = 0;
					t.waypoint[t.waypointindex].start = 0;
					t.waypoint[t.waypointindex].finish = 0;
				}
			}
		}
	}
}

void waypoint_loaddata ( void )
{
	//  Free any old
	waypoint_deleteall ( );

	//  load list
	if (  FileExist( cstr(t.levelmapptah_s+"\\map.way").Get() ) == 1 ) 
	{
		t.filename_s=t.levelmapptah_s+"map.way";
		OpenToRead (  1,t.filename_s.Get() );
			//  strands
			g.waypointmax = ReadLong ( 1 );
			if (  g.waypointmax>0 ) 
			{
				UnDim (  t.waypoint );
				Dim (  t.waypoint,g.waypointmax+1  );
				for ( t.w = 1 ; t.w<=  g.waypointmax; t.w++ )
				{
					t.a = ReadLong ( 1 ); t.waypoint[t.w].style=t.a;
					t.a = ReadLong ( 1 ); t.waypoint[t.w].fillcolor=t.a;
					t.a = ReadLong ( 1 ); t.waypoint[t.w].linkedtoentityindex=t.a;
					t.a = ReadLong ( 1 ); t.waypoint[t.w].count=t.a;
					t.a = ReadLong ( 1 ); t.waypoint[t.w].start=t.a;
					t.a = ReadLong ( 1 ); t.waypoint[t.w].finish=t.a;
				}
			}
			//  coords
			g.waypointcoordmax = ReadLong ( 1 );
			//  Dave fix, on some older maps waypointcoordmax was a negative number
			//  which would cause it to show up as 4gb when diming
			//  so now we check if it is negative and correct it
			if (  g.waypointcoordmax < 0  )  g.waypointcoordmax  =  0;

			if (  g.waypointcoordmax>0 ) 
			{
				UnDim (  t.waypointcoord );
				Dim (  t.waypointcoord,g.waypointcoordmax+1 );
				for ( t.w = 1 ; t.w<=  g.waypointcoordmax; t.w++ )
				{
					t.a_f = ReadFloat ( 1 ); t.waypointcoord[t.w].x=t.a_f;
					t.a_f = ReadFloat ( 1 ); t.waypointcoord[t.w].y=t.a_f;
					t.a_f = ReadFloat ( 1 ); t.waypointcoord[t.w].z=t.a_f;
					t.a = ReadLong ( 1 ); t.waypointcoord[t.w].link=t.a;
					t.a = ReadLong ( 1 ); t.waypointcoord[t.w].index=t.a;
				}
			}
		CloseFile (  1 );
	}
	else
	{
		// 010316 - V1.13b1 - if waypoint data entirely missing, need to scan 
		// all entities to remove any reference to zones that use them
		for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
		{
			int iEntID = t.entityelement[t.e].bankindex;
			if ( iEntID > 0 )
			{
				if ( t.entityprofile[iEntID].ismarker==3 || t.entityprofile[iEntID].ismarker==6 || t.entityprofile[iEntID].ismarker==8 )
				{
					// entities rely on waypoints, and no waypoint data, so erase these entities
					t.entityelement[t.e].bankindex = 0;
					t.entityelement[t.e].active = 0;
				}
			}
		}
	}

	//  also ensure data is not pointing to larger arrays than we have (old legacy editing rogue data)
	if ( g.waypointmax>0 ) 
	{
		waypoint_validate_style2();

		for ( t.waypointindex = 1; t.waypointindex <= g.waypointmax; t.waypointindex++ )
		{
			if (  t.waypoint[t.waypointindex].count>0 ) 
			{
				if (  t.waypoint[t.waypointindex].start>g.waypointcoordmax || t.waypoint[t.waypointindex].finish>g.waypointcoordmax ) 
				{
					//  waypoint indexes exceed current arrays - diable waypoint
					t.waypoint[t.waypointindex].style=0;
					t.waypoint[t.waypointindex].fillcolor=0;
					t.waypoint[t.waypointindex].linkedtoentityindex=0;
					t.waypoint[t.waypointindex].count=0;
					t.waypoint[t.waypointindex].start=0;
					t.waypoint[t.waypointindex].finish=0;
				}
			}
		}
		for ( t.w = 1 ; t.w<=  g.waypointcoordmax; t.w++ )
		{
			if (  t.waypointcoord[t.w].link>g.waypointcoordmax ) 
			{
				t.waypointcoord[t.w].link=0;
			}
			if (  t.waypointcoord[t.w].index>g.waypointmax ) 
			{
				t.waypointcoord[t.w].index=0;
			}
		}
	}

	//  also ensure waypoint table not corrupt, so check sizes and ranges
	if (  g.waypointmax>0 ) 
	{
		for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
		{
			t.tcountnow=t.waypoint[t.waypointindex].finish-t.waypoint[t.waypointindex].start;
			if (  t.waypoint[t.waypointindex].count>0 && t.waypoint[t.waypointindex].count != t.tcountnow ) 
			{
				//  reconstruct table data from waypoint coord data
				t.waypoint[t.waypointindex].start=-1;
				for ( t.w = 1 ; t.w<=  g.waypointcoordmax; t.w++ )
				{
					if (  t.waypointcoord[t.w].index == t.waypointindex ) 
					{
						if (  t.waypoint[t.waypointindex].start == -1 ) 
						{
							t.waypoint[t.waypointindex].start=t.w;
						}
					}
					else
					{
						if (  t.waypoint[t.waypointindex].start != -1 ) 
						{
							break;
						}
					}
				}
				if (  t.waypoint[t.waypointindex].start != -1 ) 
				{
					t.waypoint[t.waypointindex].finish=t.w-1;
				}
			}
			if (  t.waypoint[t.waypointindex].start == -1 ) 
			{
				t.waypoint[t.waypointindex].start=0;
				t.waypoint[t.waypointindex].finish=0;
				t.waypoint[t.waypointindex].count=0;
			}
		}
	}
}

void waypoint_recreateobjs ( void )
{
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		if (  t.waypoint[t.waypointindex].count>0 ) 
		{
			createwaypointobj(g.editorwaypointoffset+t.waypointindex,t.waypointindex);
		}
	}
}

void waypoint_createnew ( void )
{
	// Create a new waypoint strand (or find empty one)
	if ( g.waypointcoordmax < 0 ) g.waypointcoordmax = 0;
	for ( t.tw = 1 ; t.tw <= g.waypointmax; t.tw++ )
	{
		if ( t.waypoint[t.tw].count == 0 ) 
		{
			t.waypointindex=t.tw ; break;
		}
	}
	if ( t.tw>g.waypointmax ) 
	{
		++g.waypointmax;
		Dim (  t.waypoint,g.waypointmax  );
		t.waypointindex=g.waypointmax;
	}

	// Set Waypoint style and data
	t.waypoint[t.waypointindex].style=t.waypointeditstyle;
	t.waypoint[t.waypointindex].fillcolor=t.waypointeditstylecolor;
	t.waypoint[t.waypointindex].linkedtoentityindex=t.waypointeditentity;

	// Waypoint style (1-path, 2-zone, 3-navmeshzone)
	if ( t.waypoint[t.waypointindex].style == 1 ) 
	{
		// Path Waypoint Style
		t.waypoint[t.waypointindex].count=1;
		++g.waypointcoordmax;
		t.w=g.waypointcoordmax;
		Dim (  t.waypointcoord,g.waypointcoordmax  );
		t.waypoint[t.waypointindex].start=t.w;
		t.waypoint[t.waypointindex].finish=t.w;
		t.waypointcoord[t.w].x=t.mx_f;
		t.waypointcoord[t.w].z=t.mz_f;
		t.waypointcoord[t.w].y=g.waypointeditheight_f+5;
		t.waypointcoord[t.w].index=t.waypointindex;
		t.waypointcoord[t.w].link=0;
	}
	if ( t.waypoint[t.waypointindex].style == 2 || t.waypoint[t.waypointindex].style == 3 ) 
	{
		// Zone Waypoint Style
		t.ttaa_f=0.0;
		int iNodeCount = 6;
		if ( t.waypoint[t.waypointindex].style == 3 )
		{
			// for Floor Zone make a square
			iNodeCount = 5;
			t.ttaa_f = -45.0f;
		}
		t.waypoint[t.waypointindex].count = iNodeCount;
		t.waypoint[t.waypointindex].start = g.waypointcoordmax+1;
		for ( t.ttii = 1 ; t.ttii<=  t.waypoint[t.waypointindex].count; t.ttii++ )
		{
			++g.waypointcoordmax;
			Dim (  t.waypointcoord,g.waypointcoordmax  );
			t.w=g.waypointcoordmax;
			t.waypointcoord[t.w].x=NewXValue(t.mx_f,t.ttaa_f,100.0);
			t.waypointcoord[t.w].z=NewZValue(t.mz_f,t.ttaa_f,100.0);
			t.waypointcoord[t.w].y=g.waypointeditheight_f + 5.0f;
			t.waypointcoord[t.w].index=t.waypointindex;
			t.waypointcoord[t.w].link=0;
			t.ttaa_f=t.ttaa_f+(360.0/(t.waypoint[t.waypointindex].count-1));
		}
		t.waypoint[t.waypointindex].finish=t.w;
	}

	// Create the above waypoint style
	if ( t.gdontcreatewaypointobjects == 0 ) 
	{
		if ( t.waypoint[t.waypointindex].count>0 ) 
		{
			createwaypointobj(g.editorwaypointoffset+t.waypointindex,t.waypointindex);
		}
	}

	// Create highlight sphere for easier selection
	if ( t.gdontcreatewaypointobjects == 0 ) 
	{
		if ( ObjectExist(g.editorwaypointoffset+0) == 0 ) 
		{
			float fSphereSize = 25.0f;
			MakeObjectSphere ( g.editorwaypointoffset+0,fSphereSize );
			SetObjectCollisionOff ( g.editorwaypointoffset+0 );
			SetAlphaMappingOn ( g.editorwaypointoffset+0,25 );
			DisableObjectZRead ( g.editorwaypointoffset+0 );
			HideObject ( g.editorwaypointoffset+0 );
			SetObjectMask ( g.editorwaypointoffset+0, 1 );
			SetObjectEffect ( g.editorwaypointoffset+0, g.guishadereffectindex );
			SetObjectEmissive ( g.editorwaypointoffset+0, Rgb(255,255,255) );
			SetObjectMask(g.editorwaypointoffset + 0, 1);
		}
	}
}

void waypoint_createallobjs ( void )
{
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		t.obj=g.editorwaypointoffset+t.waypointindex;
		if (  t.gdontcreatewaypointobjects == 0 ) 
		{
			if (  t.waypoint[t.waypointindex].count>0 ) 
			{
				createwaypointobj(t.obj,t.waypointindex);
			}
			else
			{
				if (  ObjectExist(t.obj) == 1  )  DeleteObject (  t.obj );
			}
		}
	}
}

void waypoint_deleteall ( void )
{
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		t.obj=g.editorwaypointoffset+t.waypointindex;
		if (  ObjectExist(t.obj) == 1  )  DeleteObject (  t.obj );
	}
	g.waypointmax=0;
	g.waypointcoordmax=0;
}

void waypoint_hideall ( void )
{
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		t.obj=g.editorwaypointoffset+t.waypointindex;
		if (  ObjectExist(t.obj) == 1  )  HideObject (  t.obj );
	}

	if (ObjectExist(g.editorwaypointoffset + 0) == 1) {
		HideObject(g.editorwaypointoffset + 0);
	}
}

void waypoint_hideallpaths ( void )
{
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		if (  t.waypoint[t.waypointindex].style == 1 ) 
		{
			t.obj=g.editorwaypointoffset+t.waypointindex;
			if (  ObjectExist(t.obj) == 1  )  HideObject (  t.obj );
		}
	}
	if (ObjectExist(g.editorwaypointoffset + 0) == 1) {
		HideObject(g.editorwaypointoffset + 0);
	}

}

void waypoint_showall ( void )
{
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		t.obj=g.editorwaypointoffset+t.waypointindex;
		if (  ObjectExist(t.obj) == 1  )  ShowObject (  t.obj );
	}
}

void waypoint_showallpaths ( void )
{
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		if (  t.waypoint[t.waypointindex].style == 1 ) 
		{
			t.obj=g.editorwaypointoffset+t.waypointindex;
			if (  ObjectExist(t.obj) == 1  )  ShowObject (  t.obj );
		}
	}
}

void waypoint_mousemanage ( void )
{
	#ifdef VRTECH
	if (bWaypointDrawmode || t.inputsys.mclick == 2 )
		return;
	#endif

	// Grid the waypoint coordinate input
	float fOrigMX = t.mx_f;
	float fOrigMY = g.waypointeditheight_f;
	float fOrigMZ = t.mz_f;
	t.mx_f=int(t.mx_f/5)*5 ; t.mz_f=int(t.mz_f/5)*5;

	// Simply find waypoint over cursor
	t.waypointiovercursor=0;
	t.waypointindexovercursor=0;
	t.tbest_f=999999 ; t.tbestwaypointindex=0 ; t.tbestwaypointiovercursor=0;
	for ( t.twaypointindex = 1 ; t.twaypointindex <= g.waypointmax; t.twaypointindex++ )
	{
		if ( t.waypoint[t.twaypointindex].count>0 ) 
		{
			for ( t.w = t.waypoint[t.twaypointindex].start ; t.w <= t.waypoint[t.twaypointindex].finish; t.w++ )
			{
				// do intersect scan to see if we are pointing to this node
				t.x1_f = CameraPositionX();
				t.y1_f = CameraPositionY();
				t.z1_f = CameraPositionZ();

				// adjust destination vector with bullet inaccuacies
				if ( ObjectExist( g.waypointdetectworkobject ) == 0 ) MakeObjectCube ( g.waypointdetectworkobject, 25.0f );
				PositionObject ( g.waypointdetectworkobject, t.waypointcoord[t.w].x, t.waypointcoord[t.w].y, t.waypointcoord[t.w].z );
				HideObject ( g.waypointdetectworkobject );

				// scan to see if waypoint node temp object intersected
				float fDistance = IntersectObject ( g.waypointdetectworkobject, t.x1_f, t.y1_f, t.z1_f, fOrigMX, fOrigMY, fOrigMZ );
				if ( fDistance > 0.0f )
				{
					if ( fDistance < t.tbest_f ) 
					{
						t.tbest_f = fDistance; 
						t.tbestwaypointindex = t.twaypointindex; 
						t.tbestwaypointiovercursor = t.w;
					}
				}
			}
		}
	}
	if ( t.tbest_f<9999 ) 
	{
		if ( t.tbestwaypointiovercursor != 0 ) 
		{
			t.waypointindexovercursor=t.tbestwaypointindex;
			t.waypointiovercursor=t.tbestwaypointiovercursor;
		}
	}

	// mclickdone is for singlr mouse clicks
	t.mclickdone=0;
	if ( t.mclick == 0 ) t.mclickpressed = 0;
	if ( t.mclick > 0 && t.mclickpressed == 0 ) { t.mclickdone = t.mclick ; t.mclickpressed = 1; }

	bool bWaypointActive = true;
	// move existing new waypoint
	if (bWaypointActive && t.mclick > 0 )
	{
		#ifdef VRTECH
		if (iWaypointDeleteMode == 0 && t.grideditselect == 6 ) {
			if (t.onedrag == 0)
			{
				if (t.tentitytoselect == 0)
				{
					//  Find waypoint
					if (t.waypointiovercursor > 0 && t.waypointindexovercursor > 0)
					{
						t.onedrag = t.waypointiovercursor; t.onedragmode = t.mclick; t.onedragx_f = t.mx_f; t.onedragz_f = t.mz_f;
						t.waypointindex = t.waypointindexovercursor;
					}
				}
			}
		}
		else if (iWaypointDeleteMode != 0 || t.grideditselect == 5 )
		#else
		if(1)
		#endif
		{
			if (t.onedrag == 0)
			{
				if (t.tentitytoselect == 0)
				{
					//  Find waypoint
					if (t.waypointiovercursor > 0 && t.waypointindexovercursor > 0)
					{
						t.onedrag = t.waypointiovercursor; t.onedragmode = t.mclick; t.onedragx_f = t.mx_f; t.onedragz_f = t.mz_f;
						t.waypointindex = t.waypointindexovercursor;
					}
				}
			}
			else
			{
				//  Drag waypoint around
				if (t.onedragmode == 1 && (t.mx_f != t.onedragx_f || t.mz_f != t.onedragz_f))
				{
					t.w = t.onedrag;
					t.tmovedeltax_f = t.mx_f - t.waypointcoord[t.w].x;
					t.tmovedeltay_f = g.waypointeditheight_f - t.waypointcoord[t.w].y;
					t.tmovedeltaz_f = t.mz_f - t.waypointcoord[t.w].z;

					#ifndef PRODUCTCLASSIC
					if (bOnlyFollowTerrain)
					{
						if (t.terrain.TerrainID > 0)
						{
							g.waypointeditheight_f = BT_GetGroundHeight(t.terrain.TerrainID, t.mx_f, t.mz_f);
						}
						else
						{
							g.waypointeditheight_f = g.gdefaultterrainheight;
						}
					}
					#endif

					t.waypointcoord[t.w].x = t.mx_f;
					t.waypointcoord[t.w].y = g.waypointeditheight_f;
					t.waypointcoord[t.w].z = t.mz_f;
					//  if zone-style, connect start and finish nodes
					if (t.waypoint[t.waypointindex].style == 2 || t.waypoint[t.waypointindex].style == 3)
					{
						//  closed loop (first is last)
						if (t.waypoint[t.waypointindex].start == t.w)
						{
							t.tow = t.waypoint[t.waypointindex].finish;
							t.waypointcoord[t.tow].x = t.waypointcoord[t.w].x;
							t.waypointcoord[t.tow].y = t.waypointcoord[t.w].y;
							t.waypointcoord[t.tow].z = t.waypointcoord[t.w].z;
						}
						if (t.waypoint[t.waypointindex].finish == t.w)
						{
							t.tow = t.waypoint[t.waypointindex].start;
							t.waypointcoord[t.tow].x = t.waypointcoord[t.w].x;
							t.waypointcoord[t.tow].y = t.waypointcoord[t.w].y;
							t.waypointcoord[t.tow].z = t.waypointcoord[t.w].z;
						}
						//  update center and adjust linked entity
						t.e = t.waypoint[t.waypointindex].linkedtoentityindex;
						waypoint_moveentitycenter();
					}
					//  Recreate waypoint object
					createwaypointobj(g.editorwaypointoffset + t.waypointindex, t.waypointindex);
					//  Drag linked waypoint too
					t.linkto = t.waypointcoord[t.w].link;
					if (t.linkto > 0)
					{
						t.waypointcoord[t.linkto].x = t.mx_f;
						t.waypointcoord[t.linkto].y = g.waypointeditheight_f;
						t.waypointcoord[t.linkto].z = t.mz_f;
						for (t.twayp = 1; t.twayp <= g.waypointmax; t.twayp++)
						{
							for (t.w = t.waypoint[t.twayp].start; t.w <= t.waypoint[t.twayp].finish; t.w++)
							{
								if (t.w == t.linkto)  createwaypointobj(g.editorwaypointoffset + t.twayp, t.twayp);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// Release waypoint
		t.onedrag=0;
	}

	//  Delete last waypoint selected
	#ifdef VRTECH
	if (  ((t.inputsys.keyshift == 1 && t.mclickdone == 2) || (iWaypointDeleteMode == 0 && t.mclickdone == 1 && t.grideditselect == 6 ) ) && t.waypointiovercursor > 0 )
	#else
	if (  t.inputsys.keyshift == 1 && t.mclickdone == 2 && t.waypointiovercursor>0 ) 
	#endif
	{
		//  if this is a zone style waypoint we want to always have a minimum of 4 waypoint nodes!
		if (  t.waypoint[t.waypointindex].count>4 || ( t.waypoint[t.waypointindex].style != 2 && t.waypoint[t.waypointindex].style != 3) ) 
		{
			//  Delete waypoint
			t.w=t.waypointiovercursor;
			if (  t.w == t.waypoint[t.waypointindex].start ) 
			{
				#ifdef VRTECH
				bool bSure = true;
				if ((iWaypointDeleteMode == 0 && t.mclickdone == 1) && t.waypoint[t.waypointindex].finish >= t.waypoint[t.waypointindex].start+1) {
					//Dont allow this in "deletemode"
					//To stop deleting by accident.
					//Only allow if there is 2 points left.
					int iAction = askBoxCancel("This will delete the whole path, are you sure?", "Confirmation"); //1==Yes 2=Cancel 0=No
					if (iAction != 1)
						bSure = false;

				}
				if(bSure)
				#else
				if ( 1 )
				#endif
				{
					waypoint_delete();

					//  if we're deleting the whole waypoint path we'll need to also delete the entity that may be with it
					t.tentitytoselect = t.waypoint[t.waypointindex].linkedtoentityindex;
					if (t.tentitytoselect > 0)
					{
						t.tDontDeleteWPFlag = 1;
						entity_deleteentityfrommap();
					}
				}
			}
			else
			{
				//  delete node from waypoint
				t.w=t.waypointiovercursor;
				while (  t.w<g.waypointcoordmax ) 
				{
					t.waypointcoord[t.w].x=t.waypointcoord[t.w+1].x;
					t.waypointcoord[t.w].y=t.waypointcoord[t.w+1].y;
					t.waypointcoord[t.w].z=t.waypointcoord[t.w+1].z;
					t.waypointcoord[t.w].link=t.waypointcoord[t.w+1].link;
					t.waypointcoord[t.w].index=t.waypointcoord[t.w+1].index;
					++t.w;
				}
				t.waypointcoord[t.w].x=0;
				t.waypointcoord[t.w].y=0;
				t.waypointcoord[t.w].z=0;
				t.waypointcoord[t.w].link=0;
				t.waypointcoord[t.w].index=0;
				--g.waypointcoordmax;
				t.waypoint[t.waypointindex].count=t.waypoint[t.waypointindex].count-1;
				t.waypoint[t.waypointindex].finish=t.waypoint[t.waypointindex].finish-1;
				t.w=t.waypointiovercursor;
				for ( t.twayp = 1 ; t.twayp<=  g.waypointmax; t.twayp++ )
				{
					for ( t.tw = t.waypoint[t.twayp].start ; t.tw<=  t.waypoint[t.twayp].finish; t.tw++ )
					{
						if (  t.w<t.waypointcoord[t.tw].link  )  t.waypointcoord[t.tw].link = t.waypointcoord[t.tw].link-1;
					}
					if (  t.twayp != t.waypointindex ) 
					{
						if (  t.w<t.waypoint[t.twayp].start  )  t.waypoint[t.twayp].start = t.waypoint[t.twayp].start-1;
						if (  t.w<t.waypoint[t.twayp].finish  )  t.waypoint[t.twayp].finish = t.waypoint[t.twayp].finish-1;
					}
				}
			}
			if (  t.waypoint[t.waypointindex].count>0 ) 
			{
				createwaypointobj(g.editorwaypointoffset+t.waypointindex,t.waypointindex);
			}
			else
			{
				if (  ObjectExist(g.editorwaypointoffset+t.waypointindex) == 1 ) 
				{
					DeleteObject (  g.editorwaypointoffset+t.waypointindex );
				}
			}
		}
		t.waypointiovercursor=0;
		t.mdoubleclick=0;
		t.onedragmode=0;
		t.onedrag=0;
	}

	//  Insert waypoint at selected
	if (  t.inputsys.keyshift == 1 && t.mclickdone == 1 && t.waypointiovercursor>0 ) 
	{
		t.w=t.waypointiovercursor+1;
		if (  t.w >= g.waypointcoordmax ) 
		{
			//  at end
			++g.waypointcoordmax;
			Dim (  t.waypointcoord,g.waypointcoordmax  );
			t.w=g.waypointcoordmax;
		}
		else
		{
			//  shuffle
			++g.waypointcoordmax;
			Dim (  t.waypointcoord,g.waypointcoordmax  );
			t.w=g.waypointcoordmax;
			while (  t.w>t.waypointiovercursor+1 ) 
			{
				t.waypointcoord[t.w].x=t.waypointcoord[t.w-1].x;
				t.waypointcoord[t.w].y=t.waypointcoord[t.w-1].y;
				t.waypointcoord[t.w].z=t.waypointcoord[t.w-1].z;
				t.waypointcoord[t.w].link=t.waypointcoord[t.w-1].link;
				t.waypointcoord[t.w].index=t.waypointcoord[t.w-1].index;
				--t.w;
			}
			t.w=t.waypointiovercursor+1;
		}
		t.waypointcoord[t.w].x=t.waypointcoord[t.w-1].x+10;
		t.waypointcoord[t.w].y=t.waypointcoord[t.w-1].y;
		t.waypointcoord[t.w].z=t.waypointcoord[t.w-1].z+10;
		t.waypointcoord[t.w].link=0;
		t.waypointcoord[t.w].index=t.waypointindex;
		t.waypoint[t.waypointindex].count=t.waypoint[t.waypointindex].count+1;
		t.waypoint[t.waypointindex].finish=t.waypoint[t.waypointindex].finish+1;
		for ( t.twayp = 1 ; t.twayp<=  g.waypointmax; t.twayp++ )
		{
			if (  t.twayp != t.waypointindex ) 
			{
				if (  t.w <= t.waypoint[t.twayp].start  )  t.waypoint[t.twayp].start = t.waypoint[t.twayp].start+1;
				if (  t.w <= t.waypoint[t.twayp].finish  )  t.waypoint[t.twayp].finish = t.waypoint[t.twayp].finish+1;
			}
			for ( t.tw = t.waypoint[t.twayp].start ; t.tw<=  t.waypoint[t.twayp].finish; t.tw++ )
			{
				if (  t.waypointcoord[t.tw].link>0 ) 
				{
					if (  t.w <= t.waypointcoord[t.tw].link  )  t.waypointcoord[t.tw].link = t.waypointcoord[t.tw].link+1;
				}
			}
		}
		if (  t.waypoint[t.waypointindex].count>0 ) 
		{
			createwaypointobj(g.editorwaypointoffset+t.waypointindex,t.waypointindex);
		}
		else
		{
			if (  ObjectExist(g.editorwaypointoffset+t.waypointindex) == 1 ) 
			{
				DeleteObject (  g.editorwaypointoffset+t.waypointindex );
			}
		}
		t.waypointiovercursor=0;
		t.onedragmode=0;
		t.onedrag=0;
	}

	//  Show selected waypoint
	if (  ObjectExist(g.editorwaypointoffset+0) == 1 ) 
	{
		if (  t.waypointiovercursor>0 ) 
		{
			t.w=t.waypointiovercursor;
			PositionObject (  g.editorwaypointoffset+0,t.waypointcoord[t.w].x,t.waypointcoord[t.w].y,t.waypointcoord[t.w].z );
			ShowObject (  g.editorwaypointoffset+0 );
		}
		else
		{
			HideObject (  g.editorwaypointoffset+0 );
		}
	}
}

void waypoint_delete ( void )
{

	//  shuffle deleted coord data to erase
	t.twcoordindex=t.w;
	t.tsize0=0+t.waypoint[t.waypointindex].finish-t.waypoint[t.waypointindex].start;
	t.tsize1=1+t.waypoint[t.waypointindex].finish-t.waypoint[t.waypointindex].start;
	while (  t.w<g.waypointcoordmax-t.tsize0 ) 
	{
		t.waypointcoord[t.w].x=t.waypointcoord[t.w+t.tsize1].x;
		t.waypointcoord[t.w].y=t.waypointcoord[t.w+t.tsize1].y;
		t.waypointcoord[t.w].z=t.waypointcoord[t.w+t.tsize1].z;
		t.waypointcoord[t.w].link=t.waypointcoord[t.w+t.tsize1].link;
		t.waypointcoord[t.w].index=t.waypointcoord[t.w+t.tsize1].index;
		++t.w;
	}
	//  blank coord data left over
	for ( t.tw = t.w ; t.tw<=  t.w+t.tsize0; t.tw++ )
	{
		t.waypointcoord[t.tw].x=0;
		t.waypointcoord[t.tw].y=0;
		t.waypointcoord[t.tw].z=0;
		t.waypointcoord[t.tw].link=0;
		t.waypointcoord[t.tw].index=0;
	}
	//  reduce to new max value for coord data
	g.waypointcoordmax -= t.tsize1;
	//  update waypoint headers to GetPoint (  new coord indexes )
	t.w=t.twcoordindex;
	for ( t.twayp = 1 ; t.twayp<=  g.waypointmax; t.twayp++ )
	{
		if (  t.twayp != t.waypointindex ) 
		{
			if (  t.w<t.waypoint[t.twayp].start  )  t.waypoint[t.twayp].start = t.waypoint[t.twayp].start-t.tsize1;
			if (  t.w<t.waypoint[t.twayp].finish  )  t.waypoint[t.twayp].finish = t.waypoint[t.twayp].finish-t.tsize1;
		}
	}
	//  delete wayppint-sequence itself
	t.waypoint[t.waypointindex].count=0;
	t.waypoint[t.waypointindex].start=0;
	t.waypoint[t.waypointindex].finish=0;

	//  remove object and reset
	if (  ObjectExist(g.editorwaypointoffset+t.waypointindex) == 1 ) 
	{
		DeleteObject (  g.editorwaypointoffset+t.waypointindex );
	}
	t.waypointiovercursor=0;
}

void waypoint_findcenter ( void )
{
	//  find center of waypoint zone
	t.tavx_f=0.0 ; t.tavy_f=0.0 ; t.tavz_f=0.0;
	for ( t.ttw = t.waypoint[t.waypointindex].start ; t.ttw<=  t.waypoint[t.waypointindex].finish; t.ttw++ )
	{
		t.tavx_f=t.tavx_f+t.waypointcoord[t.ttw].x;
		t.tavy_f=t.tavy_f+t.waypointcoord[t.ttw].y;
		t.tavz_f=t.tavz_f+t.waypointcoord[t.ttw].z;
	}
	t.tavx_f=t.tavx_f/t.waypoint[t.waypointindex].count;
	t.tavy_f=t.tavy_f/t.waypoint[t.waypointindex].count;
	t.tavz_f=t.tavz_f/t.waypoint[t.waypointindex].count;
}

void waypoint_movetogrideleprof ( void )
{
	//  waypoint zone index
	t.waypointindex=t.grideleprof.trigger.waypointzoneindex;
	t.thisx_f=t.gridentityposx_f;
	t.thisy_f=t.gridentityposy_f;
	t.thisz_f=t.gridentityposz_f;
	waypoint_movetothiscoordinate ( );
}

void waypoint_movetothiscoordinate ( void )
{
	//  waypoint zone index
	waypoint_findcenter ( );

	//  now normalise coordinates around 0,0,0 and add entity edit object position
	for ( t.ttw = t.waypoint[t.waypointindex].start ; t.ttw<=  t.waypoint[t.waypointindex].finish; t.ttw++ )
	{
		t.waypointcoord[t.ttw].x=t.thisx_f+(t.waypointcoord[t.ttw].x-t.tavx_f);
		t.waypointcoord[t.ttw].y=t.thisy_f+(t.waypointcoord[t.ttw].y-t.tavy_f);
		t.waypointcoord[t.ttw].z=t.thisz_f+(t.waypointcoord[t.ttw].z-t.tavz_f);
	}

	//  refresh waypoint object
	t.obj=g.editorwaypointoffset+t.waypointindex;
	if (  ObjectExist(t.obj) == 1  )  DeleteObject (  t.obj );
	createwaypointobj(t.obj,t.waypointindex);
}

void waypoint_moveentitycenter ( void )
{
	//  when move a single waypoint coordinate, need to adjust entity marker object position
	t.waypointindex=t.entityelement[t.e].eleprof.trigger.waypointzoneindex;
	waypoint_findcenter ( );

	//  entity has new object position
	t.entityelement[t.e].x=t.tavx_f;
	t.entityelement[t.e].y=t.tavy_f;
	t.entityelement[t.e].z=t.tavz_f;

	//  update entity object
	t.obj=t.entityelement[t.e].obj;
	if (  t.obj>0 ) 
	{
		if (  ObjectExist(t.obj) == 1 ) 
		{
			PositionObject (  t.obj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
		}
	}
}

void createwaypointobj ( int obj, int waypointindex )
{
	int lastdiffuse = 0;
	int tcolorcycle = 0;
	int polycount = 0;
	int polyindex = 0;
	DWORD diffuse = 0;
	float tsize_f = 0;
	float fade_f = 0;
	float c1x_f = 0;
	float c1y_f = 0;
	float c1z_f = 0;
	float c2x_f = 0;
	float c2y_f = 0;
	float c2z_f = 0;
	float c3x_f = 0;
	float c3y_f = 0;
	float c3z_f = 0;
	float c4x_f = 0;
	float c4y_f = 0;
	float c4z_f = 0;
	float c5x_f = 0;
	float c5y_f = 0;
	float c5z_f = 0;
	float c6x_f = 0;
	float c6y_f = 0;
	float c6z_f = 0;
	float fwa_f = 0;
	float fwx_f = 0;
	float fwy_f = 0;
	float fwz_f = 0;
	float pwa_f = 0;
	float wa_f = 0;
	float wx_f = 0;
	float wy_f = 0;
	float wz_f = 0;
	if (  obj <= g.editorwaypointoffsetmax ) 
	{
		int lastdiffuse = 0;
		int tcolorcycle = 0;
		int polycount = 0;
		int polyindex = 0;
		DWORD diffuse = 0;
		float tsize_f = 0;
		float fade_f = 0;
		float c1x_f = 0;
		float c1y_f = 0;
		float c1z_f = 0;
		float c2x_f = 0;
		float c2y_f = 0;
		float c2z_f = 0;
		float c3x_f = 0;
		float c3y_f = 0;
		float c3z_f = 0;
		float c4x_f = 0;
		float c4y_f = 0;
		float c4z_f = 0;
		float c5x_f = 0;
		float c5y_f = 0;
		float c5z_f = 0;
		float c6x_f = 0;
		float c6y_f = 0;
		float c6z_f = 0;
		float fwa_f = 0;
		float fwx_f = 0;
		float fwy_f = 0;
		float fwz_f = 0;
		float pwa_f = 0;
		float wa_f = 0;
		float wx_f = 0;
		float wy_f = 0;
		float wz_f = 0;
		int w = 0;
		if (  t.waypoint[waypointindex].count>0 ) 
		{
			float fHeightFromFloor = 6.0f;
			float fLineThickness = 5.0f;
			polycount=((t.waypoint[waypointindex].count-1)*2);
			polycount+=(t.waypoint[waypointindex].count*2);
			if ( t.waypoint[waypointindex].style == 2 || t.waypoint[waypointindex].style == 3 )  polycount = polycount+t.waypoint[waypointindex].count-2;
			t.waypoint[waypointindex].polycount=polycount;
			makepolymesh(obj,polycount);
			w=t.waypoint[waypointindex].start;
			fwx_f=t.waypointcoord[w].x;
			fwy_f=t.waypointcoord[w].y+fHeightFromFloor;
			fwz_f=t.waypointcoord[w].z;
			polyindex=0;
			for ( w = t.waypoint[waypointindex].start+1 ; w<=  t.waypoint[waypointindex].finish; w++ )
			{
				wx_f=t.waypointcoord[w].x;
				wy_f=t.waypointcoord[w].y+fHeightFromFloor;
				wz_f=t.waypointcoord[w].z;
				wa_f=atan2deg(wx_f-fwx_f,wz_f-fwz_f)-90;
				pwa_f=wa_f ; fwa_f=wa_f;
				fade_f=1.0;
				if ( t.waypoint[waypointindex].style == 2 || t.waypoint[waypointindex].style == 3 ) 
				{
					// zones have white lines
					diffuse=Rgb(155*fade_f,155*fade_f,155*fade_f);
				}
				else
				{
					tcolorcycle=t.waypointcoord[w].index-((t.waypointcoord[w].index/6)*6);
					if (  tcolorcycle == 0  )  diffuse = Rgb(0*fade_f,0*fade_f,155*fade_f);
					if (  tcolorcycle == 1  )  diffuse = Rgb(155*fade_f,0,0);
					if (  tcolorcycle == 2  )  diffuse = Rgb(0,155*fade_f,0);
					if (  tcolorcycle == 3  )  diffuse = Rgb(155*fade_f,155*fade_f,0);
					if (  tcolorcycle == 4  )  diffuse = Rgb(155*fade_f,0,155*fade_f);
					if (  tcolorcycle == 5  )  diffuse = Rgb(0,155*fade_f,155*fade_f);
				}
				if (  w == t.waypoint[waypointindex].start+1  )  lastdiffuse = diffuse;
				c1x_f=NewXValue(fwx_f,fwa_f,fLineThickness) ; c1z_f=NewZValue(fwz_f,fwa_f,fLineThickness) ; c1y_f=fwy_f;
				c2x_f=NewXValue(fwx_f,fwa_f,-fLineThickness) ; c2z_f=NewZValue(fwz_f,fwa_f,-fLineThickness) ; c2y_f=fwy_f;
				c3x_f=NewXValue(wx_f,pwa_f,fLineThickness) ; c3z_f=NewZValue(wz_f,pwa_f,fLineThickness) ; c3y_f=wy_f;
				c4x_f=NewXValue(wx_f,pwa_f,-fLineThickness) ; c4z_f=NewZValue(wz_f,pwa_f,-fLineThickness) ; c4y_f=wy_f;
				if (  polyindex+1<t.waypoint[waypointindex].polycount ) 
				{
					addpolytomesh(obj,polyindex+0,lastdiffuse,diffuse,diffuse,c1x_f,c1y_f,c1z_f,c3x_f,c3y_f,c3z_f,c4x_f,c4y_f,c4z_f);
					addpolytomesh(obj,polyindex+1,lastdiffuse,lastdiffuse,diffuse,c2x_f,c2y_f,c2z_f,c1x_f,c1y_f,c1z_f,c4x_f,c4y_f,c4z_f);
				}
				fwx_f=wx_f ; fwy_f=wy_f ; fwz_f=wz_f ; fwa_f=wa_f ; lastdiffuse=diffuse ; polyindex += 2;
			}
			//  stars
			for ( w = t.waypoint[waypointindex].start ; w<=  t.waypoint[waypointindex].finish; w++ )
			{
				wx_f=t.waypointcoord[w].x;
				wy_f=t.waypointcoord[w].y+6;
				wz_f=t.waypointcoord[w].z;
				if (  t.waypointcoord[w].link>0 ) 
				{
					diffuse=Rgb(0,255,255);
				}
				else
				{
					diffuse=Rgb(255,255,0);
				}
				if (  w == t.waypoint[waypointindex].start  )  tsize_f = 1.5; else tsize_f = 1.0;
				c1x_f=NewXValue(wx_f,55,8*tsize_f) ; c1z_f=NewZValue(wz_f,55,8*tsize_f) ; c1y_f=wy_f;
				c2x_f=NewXValue(wx_f,-55,8*tsize_f) ; c2z_f=NewZValue(wz_f,-55,8*tsize_f) ; c2y_f=wy_f;
				c3x_f=NewXValue(wx_f,180,10*tsize_f) ; c3z_f=NewZValue(wz_f,180,10*tsize_f) ; c3y_f=wy_f;
				c4x_f=NewXValue(wx_f,235,8*tsize_f) ; c4z_f=NewZValue(wz_f,235,8*tsize_f) ; c4y_f=wy_f;
				c5x_f=NewXValue(wx_f,125,8*tsize_f) ; c5z_f=NewZValue(wz_f,125,8*tsize_f) ; c5y_f=wy_f;
				c6x_f=NewXValue(wx_f,0,10*tsize_f) ; c6z_f=NewZValue(wz_f,0,10*tsize_f) ; c6y_f=wy_f;
				if (  polyindex+1<t.waypoint[waypointindex].polycount ) 
				{
					addpolytomesh(obj,polyindex+0,diffuse,diffuse,diffuse,c1x_f,c1y_f,c1z_f,c2x_f,c2y_f,c2z_f,c3x_f,c3y_f,c3z_f);
					addpolytomesh(obj,polyindex+1,diffuse,diffuse,diffuse,c4x_f,c4y_f,c4z_f,c5x_f,c5y_f,c5z_f,c6x_f,c6y_f,c6z_f);
				}
				polyindex += 2;
			}
			//  zone style also shades interior
			if ( t.waypoint[waypointindex].style == 2 || t.waypoint[waypointindex].style == 3 ) 
			{
				w=t.waypoint[waypointindex].start;
				c1x_f=t.waypointcoord[w].x;
				c1y_f=t.waypointcoord[w].y+6;
				c1z_f=t.waypointcoord[w].z;
				fade_f=0.5;
				if (  t.waypoint[waypointindex].fillcolor == 1  )  diffuse = Rgb(255*fade_f,128*fade_f,0*fade_f);
				if (  t.waypoint[waypointindex].fillcolor == 2  )  diffuse = Rgb(183*fade_f,220*fade_f,244*fade_f);
				if (  t.waypoint[waypointindex].fillcolor == 3  )  diffuse = Rgb(0*fade_f,128*fade_f,255*fade_f);
				if (  t.waypoint[waypointindex].fillcolor == 4  )  diffuse = Rgb(0*fade_f,255*fade_f,0*fade_f);
				if (  t.waypoint[waypointindex].fillcolor == 5  )  diffuse = Rgb(64*fade_f,64*fade_f,64*fade_f);
				if (  t.waypoint[waypointindex].fillcolor == 6  )  diffuse = Rgb(255*fade_f,0*fade_f,0*fade_f);
				if (  t.waypoint[waypointindex].fillcolor == 7  )  diffuse = Rgb(0*fade_f,0*fade_f,255*fade_f);
				for ( w = t.waypoint[waypointindex].start+1 ; w<=  t.waypoint[waypointindex].finish-1; w++ )
				{
					c2x_f=t.waypointcoord[w].x;
					c2y_f=t.waypointcoord[w].y+6;
					c2z_f=t.waypointcoord[w].z;
					c3x_f=t.waypointcoord[w+1].x;
					c3y_f=t.waypointcoord[w+1].y+6;
					c3z_f=t.waypointcoord[w+1].z;
					if (  polyindex+0<t.waypoint[waypointindex].polycount ) 
					{
						addpolytomesh(obj,polyindex+0,diffuse,diffuse,diffuse,c1x_f,c1y_f,c1z_f,c2x_f,c2y_f,c2z_f,c3x_f,c3y_f,c3z_f);
					}
					++polyindex;
				}
			}
			finalisepolymesh(obj);
		}
		else
		{
			if ( ObjectExist(obj) == 1 ) DeleteObject ( obj );
		}
	}
}

void makepolymesh ( int obj, int polycount )
{
	int headersize = 0;
	int memblock = 0;
	int vertsize = 0;
	int memsize = 0;
	int verts = 0;
	int fvf = 0;

	//  Free old memblock
	memblock = 11 ; if (  MemblockExist(memblock) == 1  )  DeleteMemblock (  memblock );

	//  Make a Simple Cube Mesh Memblock
	headersize=12 ; fvf=338 ; vertsize=36 ; verts=polycount*3;
	memsize=headersize+(vertsize*verts);
	MakeMemblock (  memblock,memsize );

	//  Set header info
	WriteMemblockDWord (  memblock,0,fvf );
	WriteMemblockDWord (  memblock,4,vertsize );
	WriteMemblockDWord (  memblock,8,verts );
}

void addverttomesh(int obj, int vertindex, unsigned int color, float fX, float fY, float fZ)
{
	// Create a vertex point
	int memblock = 11, vertsize = 36;
	int pos = 12 + vertindex * vertsize;
	WriteMemblockFloat (memblock, pos + 0, fX);
	WriteMemblockFloat (memblock, pos + 4, fY);
	WriteMemblockFloat (memblock, pos + 8, fZ);
	WriteMemblockFloat (memblock, pos + 12, 0);
	WriteMemblockFloat (memblock, pos + 16, 1);
	WriteMemblockFloat (memblock, pos + 20, 0);
	WriteMemblockDWord (memblock, pos + 24, color);
	WriteMemblockFloat (memblock, pos + 28, 0);
	WriteMemblockFloat (memblock, pos + 32, 0);
}

void addpolytomesh ( int obj, int polyindex, int diffuse1, int diffuse2, int diffuse3, int x1_f, int y1_f, int z1_f, int x2_f, int y2_f, int z2_f, int x3_f, int y3_f, int z3_f )
{
	int memblock = 0;
	int vertsize = 0;
	float nx_f = 0;
	float ny_f = 0;
	float nz_f = 0;
	int pos = 0;
	float x_f = 0;
	float y_f = 0;
	float z_f = 0;
	int tt = 0;
	int diff;
	int v = 0;

	// Create vertex data (3 make a triangle polygon)
	memblock=11 ; vertsize=36;
	for ( tt = 0 ; tt <= 2; tt++ )
	{
		//  Corners of poly
		if (  tt == 0 ) { x_f = x1_f ;y_f = y1_f;z_f = z1_f ; diff = diffuse1; }
		if (  tt == 1 ) { x_f = x2_f ;y_f = y2_f;z_f = z2_f ; diff = diffuse2; }
		if (  tt == 2 ) { x_f = x3_f ;y_f = y3_f;z_f = z3_f ; diff = diffuse3; }
		v=(polyindex*3)+tt;

		//  Position of vertex in memblock
		pos=12+(v*vertsize);

		//  Create some random vertex data
		nx_f=0 ; ny_f=1.0 ; nz_f=0;

		//  Set vertex position
		WriteMemblockFloat (  memblock,pos+0,x_f );
		WriteMemblockFloat (  memblock,pos+4,y_f );
		WriteMemblockFloat (  memblock,pos+8,z_f );

		//  Set vertex normals
		WriteMemblockFloat (  memblock,pos+12,nx_f );
		WriteMemblockFloat (  memblock,pos+16,ny_f );
		WriteMemblockFloat (  memblock,pos+20,nz_f );

		//  Set vertex diffuse
		WriteMemblockDWord (  memblock,pos+24,diff );

		//  Set vertex texture UV coords
		WriteMemblockFloat (  memblock,pos+28,0 );
		WriteMemblockFloat (  memblock,pos+32,0 );
	}
}

void finalisepolymesh ( int obj )
{
	int memblock = 0;

	// Make a new object from the memblock
	if ( ObjectExist(obj) == 1 ) DeleteObject ( obj );
	if ( GetMeshExist(2) == 1 ) DeleteMesh ( 2 );
	memblock=11; CreateMeshFromMemblock ( 2, memblock );
	MakeObject ( obj, 2, 0 ); 
	SetObject ( obj, 1, 0, 0, 0, 0, 0, 0 );
	SetObjectCull ( obj, 0 );
	TextureObject ( obj, g.editorimagesoffset+13 );
	SetObjectCollisionOff ( obj );
	DisableObjectZWrite ( obj );
	SetObjectLight ( obj, 0 );
	SetSphereRadius ( obj, 0 );
	SetObjectMask ( obj, 1 );

	// 150817 - GUI shader with DIFFUSE element included
	SetObjectEffect ( obj, g.guidiffuseshadereffectindex );


	// set alpha and transparency of this object
	SetObjectTransparency ( obj, 2 );
	SetAlphaMappingOn (obj, 75);// 25 );
}

void waypoint_reset ( void )
{
	//  reset all waypoints ready for new level
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		t.waypoint[t.waypointindex].active=1;
		t.obj=g.editorwaypointoffset+t.waypointindex;
		if (  ObjectExist(t.obj) == 1  )  HideObject (  t.obj );
	}
}

void waypoint_restore ( void )
{
	//  Waypoint restore for return to editor
	for ( t.waypointindex = 1 ; t.waypointindex<=  g.waypointmax; t.waypointindex++ )
	{
		t.waypoint[t.waypointindex].active=1;
		t.obj=g.editorwaypointoffset+t.waypointindex;
		if (  ObjectExist(t.obj) == 1  )  ShowObject (  t.obj );
	}
}

void waypoint_hide ( void )
{
	t.waypoint[t.waypointindex].active=0;
	t.ttobj=g.editorwaypointoffset+t.waypointindex;
	if (  ObjectExist(t.ttobj) == 1  )  HideObject (  t.ttobj );
}

int waypoint_getmax ( void )
{
	return g.waypointmax;
}

int waypoint_ispointinzoneex ( int iWPIndex, float fX, float fY, float fZ, int iStyleFloorZoneOnly )
{
	t.tokay=0;
	if ( iStyleFloorZoneOnly == 0 )
	{
		if ( ispointinzone(iWPIndex,fX,fZ) == 1 ) 
			t.tokay=1;
	}
	else
	{
		if ( t.waypoint[iWPIndex].style == 3 && t.waypoint[iWPIndex].count > 0 )
		{
			// also check waypoint layer vs fY passed in
			int e = t.waypoint[iWPIndex].linkedtoentityindex;
			if ( e > 0 )
				if ( fY >= t.entityelement[e].y-25.0f && fY <= t.entityelement[e].y+65.0f )
					if ( ispointinzone(iWPIndex,fX,fZ) == 1 ) 
						t.tokay=1;
		}
	}
	return t.tokay;
}

void waypoint_ispointinzone ( void )
{
	waypoint_ispointinzoneex ( t.waypointindex, t.tpointx_f, -1, t.tpointz_f, 0 );
}

int ispointinzone ( int waypointindex, int tpointx_f, int tpointz_f )
{
	int nvert = 0;
	float ca_f = 0;
	float cb_f = 0;
	float cd_f = 0;
	float ce_f = 0;
	int cc = 0;
	int cf = 0;
	int wi = 0;
	int wj = 0;
	int c = 0;
	int j = 0;
	int i = 0;
	nvert=t.waypoint[waypointindex].count;
	i=0;
	j=nvert-1;
	c=0;
	while (  i<nvert ) 
	{
		wi=t.waypoint[waypointindex].start+i;
		wj=t.waypoint[waypointindex].start+j;
		ca_f=(t.waypointcoord[wi].z>=tpointz_f);
		cb_f=(t.waypointcoord[wj].z>=tpointz_f);
		cc = ca_f != cb_f;
		cd_f=tpointx_f;
		ce_f=(t.waypointcoord[wj].x-t.waypointcoord[wi].x) * (tpointz_f-t.waypointcoord[wi].z) / (t.waypointcoord[wj].z-t.waypointcoord[wi].z) + t.waypointcoord[wi].x;
		cf=cd_f<=ce_f;
		if (  cc == 1 && cf == 1 ) 
		{
			c = 1-c;
		}
		j=i;
		++i;
	}
	return c;
}
