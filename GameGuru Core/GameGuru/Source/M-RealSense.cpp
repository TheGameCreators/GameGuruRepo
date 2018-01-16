//----------------------------------------------------
//--- GAMEGURU - M-RealSense
//----------------------------------------------------

#include "gameguru.h"

// 
//  RealSense Module
// 

void realsense_init ( void )
{

	/*      
	//  RealSense database
	t.realsense.grammermax=10;
	Dim (  realsensecommands_s(t.realsense.grammermax) );
	realsensecommands_s(1) = "Select Shotgun";
	realsensecommands_s(2) = "Hide Weapon";
	realsensecommands_s(3) = "Edit Mode";
	realsensecommands_s(4) = "Enter Edit Mode";
	realsensecommands_s(5) = "Play Game";
	realsensecommands_s(6) = "Enter Play Mode";
	realsensecommands_s(7) = "Terrain Mode Sculpt";
	realsensecommands_s(8) = "Paint Mode Pavement";
	realsensecommands_s(9) = "Paint Mode Rock";
	realsensecommands_s(10) = "Paint Mode Grass";
	t.realsense.currentCommand_s = "No Command";

	//  RealSense Init
	t.realsense.init = PC INIT(g.FEATURES);
	t.realsense.grammer = PC VOICE CREATE GRAMMER();
	for ( t.i = 1 ; t.i<=  t.realsense.grammermax; t.i++ )
	{
		PC VOICE ADD GRAMMER t.realsense.grammer, t.i, realsensecommands_s(t.i);
	}
	PC VOICE SET GRAMMER t.realsense.grammer;
	*/    

return;

}

void realsense_loop ( void )
{

	/*      
	//  Sync (  each cycle )
	if (  t.realsense.init == 0 ) 
	{

		//  read RealSense sensors
		snc = PC SYNCHRONISE();
		snc = PC SYNCHRONISE AUDIO()	;
		t.realsense.disconnected = PC IS DISCONNECTED();
		if (  t.realsense.disconnected  ==  1 ) 
		{
		}
		t.realsense.bMassX_f = PC GET BODY MASS X();
		t.realsense.bMassY_f = PC GET BODY MASS Y();
		cmd = PC VOICE GET COMMAND();
		while (  cmd ) 
		{
			t.realsense.id = PC VOICE GET COMMAND ID();
			t.realsense.confidence = PC VOICE GET COMMAND CONFIDENCE();
			t.realsense.currentCommand_s = realsensecommands_s(t.realsense.id);
			cmd = PC VOICE GET NEXT COMMAND();
		}

		//  act on Sensor feedback
		if (  ReturnKey() == 1 ) 
		{
			//  calibration when in Run-Time
			t.realsense.bMassCalibrateX_f=t.realsense.bMassX_f;
		}
		tBMTrelativedifferenceX_f=t.realsense.bMassX_f-t.realsense.bMassCalibrateX_f;
		t.playercontrol.leanoverangle_f=90;
		tBMTDestinationX_f=tBMTrelativedifferenceX_f;
		t.playercontrol.leanover_f=CurveValue(tBMTDestinationX_f,t.playercontrol.leanover_f,50.0);

		//  Act on Voice Control
		if (  t.realsense.id>0 ) 
		{
			if (  t.realsense.id == 1 ) 
			{
				//  Select Shotgun
				for ( twsi = 1 ; twsi<=  10; twsi++ )
				{
					t.tweaponindex=t.weaponslot[twsi].got;
					if (  t.tweaponindex>0  )  break;
				}
				if (  t.tweaponindex>0 ) 
				{
					t.player[t.plrid].command.newweapon=t.tweaponindex;
				}
			}
			if (  t.realsense.id == 2 ) 
			{
				//  Hide Weapon
				if (  t.gunmode<31 || t.gunmode>35 ) 
				{
					if (  t.gunid != 0 ) 
					{
						t.gunmode=31 ; t.gunselectionafterhide=0;
					}
				}
			}
			if (  t.conkit.editmodeactive == 0 ) 
			{
				if (  t.realsense.id == 3 || t.realsense.id == 4 ) 
				{
					//  Edit Mode
					t.conkit.forceaction=68;
				}
			}
			else
			{
				if (  t.realsense.id == 5 || t.realsense.id == 6 ) 
				{
					//  Play Game
					t.conkit.forceaction=67;
				}
				if (  t.realsense.id == 7 ) 
				{
					//  Terrain Mode Sculpt
					t.conkit.forceaction=2;
				}
				if (  t.realsense.id == 8 ) 
				{
					//  Paint Mode Pavement
					t.conkit.forceaction=5;
				}
				if (  t.realsense.id == 9 ) 
				{
					//  Paint Mode Rock
					t.conkit.forceaction=6;
				}
				if (  t.realsense.id == 10 ) 
				{
					//  Paint Mode Grass
					t.conkit.forceaction=9;
				}
			}
			//lua_prompt ( );
			t.realsense.id=0;
		}

	}
	else
	{
		//  no leaning
		t.playercontrol.leanoverangle_f=0;
		t.playercontrol.leanover_f=0;
	}
	*/    

return;

}

void realsense_free ( void )
{

	/*      
	//  free if created
	if (  t.realsense.init  ==  0 ) 
	{
		PC CLOSE;
	}
	*/    

return;

}
