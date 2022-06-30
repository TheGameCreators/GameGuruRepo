//----------------------------------------------------
//--- GAMEGURU - M-UndoSys Object
//----------------------------------------------------

// Includes
#include "gameguru.h"

// Globals
std::stack<sUndoStackItem> g_UndoSysObjectStack[2];
bool g_UndoSysObjectIsBeingMoved = false; // when click an object to highlight it, the delete/add is just an internal process to move object
bool g_UndoSysObjectRememberBeforeMove = false;
int g_UndoSysObjectRememberBeforeMoveE;
float g_UndoSysObjectRememberBeforeMovePX;
float g_UndoSysObjectRememberBeforeMovePY;
float g_UndoSysObjectRememberBeforeMovePZ;
float g_UndoSysObjectRememberBeforeMoveRX;
float g_UndoSysObjectRememberBeforeMoveRY;
float g_UndoSysObjectRememberBeforeMoveRZ;
float g_UndoSysObjectRememberBeforeMoveSX;
float g_UndoSysObjectRememberBeforeMoveSY;
float g_UndoSysObjectRememberBeforeMoveSZ;

//
// Specific Calls for Object Events
//

// Object Add
void undosys_object_add (int te)
{
	// create event data
	sUndoSysEventObjectAdd* pEvent = new sUndoSysEventObjectAdd;
	pEvent->iEntityElementIndex = te;

	// add to undo stack
	undosys_addevent(eUndoSys_Object, eUndoSys_Object_Add, pEvent);
}

// Object Delete
void undosys_object_delete (int te, int waypointzoneindex, int editorfixed, int maintype, int bankindex, int staticflag, int iHasParentIndex, float x, float y, float z, float rx, float ry, float rz, float scalex, float scaley, float scalez )
{
	// create event data
	sUndoSysEventObjectDelete* pEvent = new sUndoSysEventObjectDelete;
	pEvent->e = te;
	pEvent->gridentity = bankindex;
	pEvent->grideleprof_trigger_waypointzoneindex = waypointzoneindex;
	pEvent->gridentityeditorfixed = editorfixed;
	pEvent->entitymaintype = maintype;
	pEvent->entitybankindex = bankindex;
	pEvent->gridentitystaticmode = staticflag;
	pEvent->gridentityhasparent = iHasParentIndex;
	pEvent->gridentityposx_f = x;
	pEvent->gridentityposy_f = y;
	pEvent->gridentityposz_f = z;
	pEvent->gridentityrotatex_f = rx;
	pEvent->gridentityrotatey_f = ry;
	pEvent->gridentityrotatez_f = rz;
	pEvent->gridentityscalex_f = scalex;
	pEvent->gridentityscaley_f = scaley;
	pEvent->gridentityscalez_f = scalez;

	// add to undo stack
	undosys_addevent(eUndoSys_Object, eUndoSys_Object_Delete, pEvent);
}

// Object Change Pos/Rot/Scl
void undosys_object_changeposrotscl (int te, float x, float y, float z, float rx, float ry, float rz, float scalex, float scaley, float scalez )
{
	// create event data
	sUndoSysEventObjectChangePosRotScl* pEvent = new sUndoSysEventObjectChangePosRotScl;
	pEvent->e = te;
	pEvent->posx_f = x;
	pEvent->posy_f = y;
	pEvent->posz_f = z;
	pEvent->rotatex_f = rx;
	pEvent->rotatey_f = ry;
	pEvent->rotatez_f = rz;
	pEvent->scalex_f = scalex;
	pEvent->scaley_f = scaley;
	pEvent->scalez_f = scalez;

	// add to undo stack
	undosys_addevent(eUndoSys_Object, eUndoSys_Object_ChangePosRotScl, pEvent);
}

/*
bool undosys_object_changeposrotscl_erasefromstackifsame (int te, float x, float y, float z, float rx, float ry, float rz, float scalex, float scaley, float scalez)
{
	// get event data from last item on stack (special case)
	sUndoSysEventObjectChangePosRotScl* pLastEvent = (sUndoSysEventObjectChangePosRotScl*)undosys_getlasteventdata(eUndoSys_Object, eUndoSys_Object_ChangePosRotScl);
	if (pLastEvent)
	{
		if (pLastEvent->e == te && pLastEvent->posx_f == x && pLastEvent->posy_f == y && pLastEvent->posz_f == z
			&& pLastEvent->rotatex_f == rx && pLastEvent->rotatey_f == ry && pLastEvent->rotatez_f == rz
			&& pLastEvent->scalex_f == scalex && pLastEvent->scaley_f == scaley && pLastEvent->scalez_f == scalez)
		{
			// erase from stack, the move is redundant (no change)
			undosys_eraselastevent();
			return true;
		}
	}
	return false;
}
*/
