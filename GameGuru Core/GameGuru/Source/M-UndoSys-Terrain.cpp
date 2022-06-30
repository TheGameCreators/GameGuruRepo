//----------------------------------------------------
//--- GAMEGURU - M-UndoSys Terrain
//----------------------------------------------------

// Includes
#include "gameguru.h"

// Globals
std::stack<sUndoStackItem> g_UndoSysTerrainStack[2];

//
// Specific Calls for Terrain Events
//

// Terrain Sculpt
void undosys_terrain_sculpt (int iSomething)
{
	// create event data
	sUndoSysEventTerrainSculpt* pEvent = new sUndoSysEventTerrainSculpt;
	pEvent->iSomething = iSomething;

	// add to undo stack
	undosys_addevent(eUndoSys_Terrain, eUndoSys_Terrain_Sculpt, pEvent);
}
void undosys_terrain_sculpt_undo (sUndoSysEventTerrainSculpt* pEvent)
{
	// undo the event
	int iObjectID = pEvent->iSomething;
}

// Terrain Paint
void undosys_terrain_paint (int iSomething)
{
	// create event data
	sUndoSysEventTerrainPaint* pEvent = new sUndoSysEventTerrainPaint;
	pEvent->iSomething = iSomething;

	// add to undo stack
	undosys_addevent(eUndoSys_Terrain, eUndoSys_Terrain_Paint, pEvent);
}
void undosys_terrain_paint_undo (sUndoSysEventTerrainPaint* pEvent)
{
	// undo the event
	int iObjectID = pEvent->iSomething;
}
