//----------------------------------------------------
//--- GAMEGURU - M-ConstructionKit
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  CONSTRUCTION KIT MODULE - Structure Maker
// 

void conkit_init ( void )
{
	//  First Person 3D Editing init
	conkitedit_init ( );

	//  reset conkit state
	t.conkit.editmodeactive=0;
	t.conkit.make.mode=CONKIT_MODE_OFF;
	t.conkit.make.initialised=0;

	// done
	return;
}

void conkit_loop ( void )
{
	conkitedit_loop();
}

void conkit_quit ( void )
{
	//  this is called when F8 mode is closed or the user returns back to the map editor from test game. It hides all the
	//  various world objects, ensures the current construction site is consolidated and also currently saves out the conkit.dat file
	t.conkit.make.mode=CONKIT_MODE_OFF;
	t.conkit.editmodeactive=0;
}

void conkit_free ( void )
{
	//  First Person 3D Editing free
	if ( t.conkit.make.initialised == 1 ) 
	{
		conkitedit_free ( );
		t.conkit.editmodeactive=0;
		t.conkit.entityeditmode=0;
		conkit_quit ( );
	}
}
