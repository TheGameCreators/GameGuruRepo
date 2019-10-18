
// PE: changed to be used in GameGuru

#include <algorithm>
#include <string>
#include <vector>

#define ENABLEIMGUI
#define USERENDERTARGET
//#define USEOLDIDE
//#define PETESTSETUP

//#define USETOOLBARHEADER
//#define CENTERETOOLBAR
#define USETOOLBARCOLORS
#define USETOOLBARGRADIENT
#define ADDGGTOOLBAR

//PE: icons use tons of memory i have 12000 so...
#define DYNAMICLOADUNLOAD

//Image range reserved for UIV3
#define UIV3IMAGES 3000

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
	const char *default_name);

namespace ImGui {
	bool BeginPopupContextItemAGK(const char* str_id = NULL, int mouse_button = 1);
	bool windowTabVisible(void);
	int windowTabFlags(void);
	int windowDockNodeId(void);

	bool ImgBtn(int iImageID, const ImVec2& btn_size = ImVec2(0, 0), const ImVec4& bg_col = ImColor(255, 255, 255, 0),
		const ImVec4& drawCol_normal = ImColor(220, 220, 220, 220),
		const ImVec4& drawCol_hover = ImColor(255, 255, 255, 255),
		const ImVec4& drawCol_Down = ImColor(180, 180, 160, 255), int frame_padding = -1, int atlasindex = 0, int atlasrows = 0, int atlascolumns = 0, bool nowhite = false , bool gratiant = false );

	bool ImgBtnBack(int iImageID, const ImVec2& btn_size = ImVec2(0, 0), const ImVec4& bg_col = ImColor(255, 255, 255, 0),
		const ImVec4& drawCol_normal = ImColor(220, 220, 220, 220),
		const ImVec4& drawCol_hover = ImColor(255, 255, 255, 255),
		const ImVec4& drawCol_Down = ImColor(180, 180, 160, 255), int frame_padding = 0);

	void ToggleButton(const char* str_id, bool* v);
	void ImRotateStart(void);
	ImVec2 ImRotationCenter(void);
	void ImRotateEnd(float rad, ImVec2 center);
	void ImGui_GG_HideWindow(ImGuiViewport* viewport);
	void ImGui_GG_ShowWindow(ImGuiViewport* viewport);
	void HideAllViewPortWindows(void);
	void ShowAllViewPortWindows(void);
}

void myDarkStyle(ImGuiStyle* dst);
void myLightStyle(ImGuiStyle* dst);
void myStyle(ImGuiStyle* dst);
void myStyle2(ImGuiStyle* dst);
void myStyle3(ImGuiStyle* dst);
void ChangeGGFont(const char *cpcustomfont, int iIDEFontSize);
bool NoCaseLess(const std::string &a, const std::string &b);
void GetMainEntityList(char* folder_s, char* rel_s, void *pFolder);
void replaceAll(std::string& str, const std::string& from, const std::string& to);

