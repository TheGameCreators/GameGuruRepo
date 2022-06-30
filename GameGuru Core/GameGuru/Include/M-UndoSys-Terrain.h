//----------------------------------------------------
//--- GAMEGURU - M-UndoSys Terrain
//----------------------------------------------------

// Includes
#include <stack>
#include <vector>
#include <deque>
#include "K3D_Vector3D.h"
#include "M-UndoSys.h"

// Externs
extern std::stack<sUndoStackItem> g_UndoSysTerrainStack[2];

// Terrain Sculpt
struct sUndoSysEventTerrainSculpt
{
	int iSomething;
};
void undosys_terrain_sculpt (int iSomething);
void undosys_terrain_sculpt_undo (sUndoSysEventTerrainSculpt* pEvent);

// Terrain Paint
struct sUndoSysEventTerrainPaint
{
	int iSomething;
};
void undosys_terrain_paint (int iSomething);
void undosys_terrain_paint_undo (sUndoSysEventTerrainPaint* pEvent);
