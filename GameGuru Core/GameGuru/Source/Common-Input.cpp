//----------------------------------------------------
//--- GAMEGURU - Common-Input
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  Common Input Functions
// 

void input_getdirectcontrols ( void )
{
	//  Direct input
	t.inputsys.activemouse=1;
	t.inputsys.xmouse=MouseX();
	t.inputsys.ymouse=MouseY();
	t.inputsys.zmouse=MouseZ();

	//Try breakpoint in MouseMoveX();
	t.inputsys.xmousemove=MouseMoveX();
	t.inputsys.ymousemove=MouseMoveY();
	t.inputsys.wheelmousemove=MouseMoveZ();
	t.inputsys.mclick=MouseClick();
	t.inputsys.k_s=Lower(Inkey());
	t.inputsys.kscancode=ScanCode();

	//  Control keys direct from keyboard
	t.inputsys.keyreturn=ReturnKey();
	t.inputsys.keyshift = ShiftKey();
	t.inputsys.keytab = KeyState(15);
	t.inputsys.keyleft=LeftKey();
	t.inputsys.keyright=RightKey();
	t.inputsys.keyup=UpKey();
	t.inputsys.keydown=DownKey();
	t.inputsys.keycontrol=ControlKey();
	t.inputsys.keyspace=SpaceKey();

	//  extra mappings
	input_extramappings ( );
}

void input_extramappings ( void )
{
	//  map numpad -/+ to querty -/= keys
	if (  t.inputsys.kscancode == 109  )  t.inputsys.kscancode = 189;
	if (  t.inputsys.kscancode == 107  )  t.inputsys.kscancode = 187;

	//  map ']' to regular ']' (some keyboard have / at 220)
	if (  t.inputsys.kscancode == 221  )  t.inputsys.kscancode = 220;
}

int control_mouseclick_mode ( int mode )
{
	int rreturnvalue = 0;
	int treturnvalue = 0;
	int returnvalue = 0;
	returnvalue = MouseClick();
	if ( g.gxbox == 1 ) 
	{
		//  intercept with controller input
		if ( g.gxboxcontrollertype == 0 ) 
		{
			if ( JoystickZ()<-500  )  returnvalue = 1;
		}
		if ( g.gxboxcontrollertype == 1 ) 
		{
			if ( JoystickFireXL(5) == 1 || JoystickFireXL(7) == 1  ) returnvalue = 1;
		}
		if ( g.gxboxcontrollertype == 2 ) 
		{
			// Dual Action F310
			if ( JoystickFireXL(5) == 1 || JoystickFireXL(7) == 1  ) returnvalue = 1;
		}
		if ( mode == 0 ) 
		{
			if ( JoystickFireA() == 1 ) returnvalue = 1;
			if ( JoystickFireB() == 1 ) returnvalue = 1;
			if ( JoystickFireC() == 1 ) returnvalue = 1;
			if ( JoystickFireD() == 1 ) returnvalue = 1;
		}
		if ( mode == 1 ) 
		{
			// only one main controller button for left click
			if ( JoystickFireB() == 1 ) returnvalue = 1;
		}
		if ( mode == 2 ) 
		{
			// only one Logitech F310 controller button for left click
			if ( JoystickFireB() == 1 ) returnvalue = 1;
		}
	}
	if ( g.fpgcgenre == 0 ) 
	{
		//  game creator is non-shooting, so LEFT is PICKUP/DROP, RIGHT=NULL
		treturnvalue = returnvalue;
		if ( returnvalue == 1 ) treturnvalue = 2;
		if ( returnvalue == 2 ) rreturnvalue = 0;
		returnvalue=treturnvalue;
	}
	return returnvalue;
}

int control_mouseclick ( void )
{
	int tresult = 0;
	tresult=control_mouseclick_mode(0);
	return tresult;
}

int inputmousex ( void )
{
	return g.mymousex;
}

int inputmousey ( void )
{
	return g.mymousey;
}

void inputpositionmouse ( int x, int y )
{
	g.mymousex=x;
	g.mymousey=y;
	//  Keep windows mouse center of primary screen (avoids multimonitor issues)
	PositionMouse (  g.mymousex,g.mymousey );
}

void inputupdatemouse ( void )
{
	g.mymousex=g.mymousex+MouseMoveX();
	g.mymousey=g.mymousey+MouseMoveY();
	if (  g.mymousex<0  )  g.mymousex = 0;
	if (  g.mymousex>GetDisplayWidth()-1  )  g.mymousex = GetDisplayWidth()-1;
	if (  g.mymousey<0  )  g.mymousey = 0;
	if (  g.mymousey>GetDisplayHeight()-1  )  g.mymousey = GetDisplayHeight()-1;
}

