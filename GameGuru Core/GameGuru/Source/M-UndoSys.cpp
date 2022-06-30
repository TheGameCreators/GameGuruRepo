//----------------------------------------------------
//--- GAMEGURU - M-UndoSys
//----------------------------------------------------

// Includes
#include "gameguru.h"

// Globals
eUndoMasterList g_UndoList;
std::stack<sUndoMasterStackItem> g_UndoSysMasterStack[2];
bool g_UndoSysMasterCollectingMultipleEvents = false;
int g_UndoSysMasterMultipleEventsCount = 0;

// 
// UNDO SYSTEM CORE
// 

void undosys_setlist(eUndoMasterList eList)
{
	g_UndoList = eList;
}

void undosys_multiplevents_start(void)
{
	// called before multiple events to be added as one action
	if (g_UndoSysMasterCollectingMultipleEvents == false)
	{
		g_UndoSysMasterCollectingMultipleEvents = true;
		g_UndoSysMasterMultipleEventsCount = 0;
	}
}

void undosys_multiplevents_finish(void)
{
	// called when finished all events to go under the last master stack item added
	if (g_UndoSysMasterCollectingMultipleEvents == true)
	{
		//PE: This crash if g_UndoSysMasterStack[g_UndoList].size = 0;
		//sUndoMasterStackItem lastmasteritem = g_UndoSysMasterStack[g_UndoList].top();
		g_UndoSysMasterCollectingMultipleEvents = false;
		g_UndoSysMasterMultipleEventsCount = 0;
	}
}

void undosys_addevent ( eUndoMasterItemType mastertype, eUndoEventType eventtype, void* pEventData )
{
	// add item to master list stack
	sUndoMasterStackItem masteritem;
	if (g_UndoSysMasterCollectingMultipleEvents == true)
	{
		// can have multiple events in type stacks represented by one item in master stack (for multiple events as one user action)
		g_UndoSysMasterMultipleEventsCount++;
		if (g_UndoSysMasterMultipleEventsCount == 1)
		{
			// first one of multiple events
			masteritem.type = mastertype;
			masteritem.count = 1;
			g_UndoSysMasterStack[g_UndoList].push(masteritem);
		}
		else
		{
			// ensure master type does not change INSIDE the multiple events collection
			masteritem = g_UndoSysMasterStack[g_UndoList].top();
			if (masteritem.type != mastertype)
			{
				MessageBoxA (NULL, "Cannot mix undo system master types in multiple events capture", "", MB_OK);
			}
			else
			{
				// if another event added, replace last master item with new count
				g_UndoSysMasterStack[g_UndoList].pop();
				masteritem.type = mastertype;
				masteritem.count = g_UndoSysMasterMultipleEventsCount;
				g_UndoSysMasterStack[g_UndoList].push(masteritem);
			}
		}
	}
	else
	{
		// regular single master item on stack per event
		masteritem.type = mastertype;
		masteritem.count = 1;
		g_UndoSysMasterStack[g_UndoList].push(masteritem);
	}

	// create item for this event
	sUndoStackItem UndoStackItem;
	UndoStackItem.event = eventtype;
	UndoStackItem.pEventData = pEventData;

	// add item to relevant type stack
	switch (masteritem.type)
	{
		case eUndoSys_Object:	g_UndoSysObjectStack[g_UndoList].push(UndoStackItem); break;
		case eUndoSys_Terrain:  g_UndoSysTerrainStack[g_UndoList].push(UndoStackItem); break;
	}
}

/*
void* undosys_getlasteventdata(eUndoMasterItemType mastertype, eUndoEventType eventtype)
{
	sUndoMasterStackItem lastmasteritem = g_UndoSysMasterStack[g_UndoList].top();
	if (lastmasteritem.type == mastertype)
	{
		sUndoStackItem lasteventitem;
		switch (lastmasteritem.type)
		{
			case eUndoSys_Object:	lasteventitem = g_UndoSysObjectStack[g_UndoList].top(); break;
			case eUndoSys_Terrain:  lasteventitem = g_UndoSysTerrainStack[g_UndoList].top(); break;
		}
		if (lasteventitem.event == eventtype)
		{
			return lasteventitem.pEventData;
		}
	}
	return NULL;
}

void undosys_eraselastevent (void)
{
	// if something on master stack
	if (g_UndoSysMasterStack[g_UndoList].size() == 0)
		return;

	// pull from master list stack
	sUndoMasterStackItem masteritem;
	masteritem = g_UndoSysMasterStack[g_UndoList].top();
	g_UndoSysMasterStack[g_UndoList].pop();

	// pull event from specific undo stack
	sUndoStackItem item;
	switch (masteritem.type)
	{
	case eUndoSys_Object:
		item = g_UndoSysObjectStack[g_UndoList].top();
		g_UndoSysObjectStack[g_UndoList].pop();
		break;

	case eUndoSys_Terrain:
		item = g_UndoSysTerrainStack[g_UndoList].top();
		g_UndoSysTerrainStack[g_UndoList].pop();
		break;
	}

	// erase event data, not using the item, just erasing it
	if (item.pEventData)
	{
		delete item.pEventData;
		item.pEventData = NULL;
	}
}
*/

void undosys_undoredoevent_core ( eUndoMasterList eList, eUndoMasterList eListForRedo )
{
	// if something on master stack
	if (g_UndoSysMasterStack[eList].size() == 0)
		return;

	// pull from master list stack
	sUndoMasterStackItem masteritem;
	masteritem = g_UndoSysMasterStack[eList].top();
	g_UndoSysMasterStack[eList].pop();

	// start the redo stack entry
	if (masteritem.count > 1)
	{
		undosys_setlist(eListForRedo);
		undosys_multiplevents_start();
		undosys_setlist(eList);
	}

	// master stack item can specify multiple events
	for (int n = 0; n < masteritem.count; n++)
	{
		// pull event from specific undo stack
		sUndoStackItem item;
		switch (masteritem.type)
		{
		case eUndoSys_Object:
			item = g_UndoSysObjectStack[eList].top();
			g_UndoSysObjectStack[eList].pop();
			break;

		case eUndoSys_Terrain:
			item = g_UndoSysTerrainStack[eList].top();
			g_UndoSysTerrainStack[eList].pop();
			break;
		}

		// execute undo code for this specific event
		switch (masteritem.type)
		{
		case eUndoSys_Object:
		{
			// create a redo action before actually do the undo
			undosys_setlist(eListForRedo);
			extern void entity_createtheredoaction (eUndoEventType eventtype, void* pEventData);
			entity_createtheredoaction(item.event, item.pEventData);
			undosys_setlist(eList);

			// perform the undo action for this event
			extern void entity_performtheundoaction (eUndoEventType eventtype, void* pEventData);
			entity_performtheundoaction (item.event, item.pEventData);
			break;
		}
		case eUndoSys_Terrain:
		{
			break;
		}
		}
	}

	// finish the redo stack entry
	if (masteritem.count > 1)
	{
		undosys_setlist(eListForRedo);
		undosys_multiplevents_finish();
		undosys_setlist(eList);
	}
}

void undosys_clearredostack (void)
{
	// this is called when user makes a new manual action that invalidates all past 'future events'
	int redostacksize = g_UndoSysMasterStack[eUndoSys_RedoList].size();
	for (int n = 0; n < redostacksize; n++) g_UndoSysMasterStack[eUndoSys_RedoList].pop();

	// also need to clear type specific stacks and remove event data allocations
	redostacksize = g_UndoSysObjectStack[eUndoSys_RedoList].size();
	for (int n = 0; n < redostacksize; n++)
	{
		sUndoStackItem eventitem = g_UndoSysObjectStack[eUndoSys_RedoList].top();
		if (eventitem.pEventData) delete eventitem.pEventData;
		g_UndoSysObjectStack[eUndoSys_RedoList].pop();
	}
	redostacksize = g_UndoSysTerrainStack[eUndoSys_RedoList].size();
	for (int n = 0; n < redostacksize; n++)
	{
		sUndoStackItem eventitem = g_UndoSysObjectStack[eUndoSys_RedoList].top();
		if (eventitem.pEventData) delete eventitem.pEventData;
		g_UndoSysTerrainStack[eUndoSys_RedoList].pop();
	}
}

void undosys_undoevent (void)
{
	// undo from undo stack, and adds to redo stack
	undosys_undoredoevent_core (eUndoSys_UndoList, eUndoSys_RedoList);
	undosys_setlist(eUndoSys_UndoList);
}

void undosys_redoevent (void)
{
	// undo from redo stack, and adds to undo stack
	undosys_undoredoevent_core (eUndoSys_RedoList, eUndoSys_UndoList);
	undosys_setlist(eUndoSys_UndoList);
}