//----------------------------------------------------
//--- GAMEGURU - M-CharacterSound
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  Character Sound
// 

void character_sound_init ( void )
{

	g.characterSoundCount = g.charactersoundoffset;
	g.characterSoundBankCount = 0;
	g.characterSoundStackSize = 0;
	g.characterSoundCurrentPlayingNumber = 0;
	g.characterSoundCurrentPlayingType_s = "";
	g.characterSoundPrevPickedNumber = 0;

	for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK-1; t.c++ )
	{
		t.characterSoundName[t.c] = "";
		for ( t.p = 0 ; t.p<=  CHARACTERSOUND_SIZE-1; t.p++ )
		{
			t.characterSound[t.c][t.p][0] = 0;
		}
	}

	//  set all elements to -1 (no bank)
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entityelement[t.e].characterSoundBankNumber = -1;
	}

return;

}

void character_sound_load ( void )
{

//  `rem Return if nothing

	if (  t.s_s  ==  ""  )  return;
	
	t.tCharacterSoundName;
	t.tCharacterSoundName = t.s_s;

	//  Check if the sound bank has already been loaded in
	t.tfound = -1;

	for ( t.c = 0 ; t.c<=  g.characterSoundBankCount-1; t.c++ )
	{
		if (  t.tfound  ==  -1 ) 
		{
			if (  t.tCharacterSoundName  ==  t.characterSoundName[t.c]  )  t.tfound  =  t.c;
		}
	}

	//  No existing bank found, so load it in
	if (  t.tfound  ==  -1 ) 
	{
		if (  g.characterSoundBankCount  >=  CHARACTERSOUND_MAX_BANK-1  )  return;
	
		t.tfound = g.characterSoundBankCount;
		t.characterSoundName[t.tfound] = t.tCharacterSoundName;
		++g.characterSoundBankCount;

		t.tFolder_s = "";
		t.tFolder_s = t.tFolder_s + "audiobank\\character\\" + t.tCharacterSoundName + "\\";

		t.tBroken = 0;
		t.characterSound[t.tfound][ON_AGGRO][0] = 0;
		for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS-1; t.c++ )
		{
			if (  t.tBroken  ==  0 ) 
			{
				t.tfile_s = t.tFolder_s+"onAggro\\" + Str(t.c) + ".wav";
				if (  FileExist(t.tfile_s.Get()) == 1 ) 
				{
					Load3DSound (  t.tfile_s.Get(),g.characterSoundCount );
					t.characterSound[t.tfound][ON_AGGRO][t.c+1] = g.characterSoundCount;
					++g.characterSoundCount;
					t.characterSound[t.tfound][ON_AGGRO][0] = t.characterSound[t.tfound][ON_AGGRO][0] + 1;
				}
				else
				{
					t.tBroken = 1;
				}
			}
		}

		t.tBroken = 0;
		t.characterSound[t.tfound][ON_ALERT][0] = 0;
		for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS-1; t.c++ )
		{
				if (  t.tBroken  ==  0 ) 
				{
					t.tfile_s = t.tFolder_s+"onAlert\\" + Str(t.c) + ".wav";
					if (  FileExist ( t.tfile_s.Get() ) ) 
					{
						Load3DSound (  t.tfile_s.Get(),g.characterSoundCount );
//       `load sound tFile$,characterSoundCount

						t.characterSound[t.tfound][ON_ALERT][t.c+1] = g.characterSoundCount;
						++g.characterSoundCount;
						t.characterSound[t.tfound][ON_ALERT][0] = t.characterSound[t.tfound][ON_ALERT][0] + 1;
					}
					else
					{
						t.tBroken = 1;
					}
				}
		}

		t.tBroken = 0;
		t.characterSound[t.tfound][ON_DEATH][0] = 0;
		for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS-1; t.c++ )
		{
				if (  t.tBroken  ==  0 ) 
				{
					t.tfile_s = t.tFolder_s+"onDeath\\" + Str(t.c) + ".wav";
					if (  FileExist ( t.tfile_s.Get() ) ) 
					{
						Load3DSound (  t.tfile_s.Get(),g.characterSoundCount );
//       `load sound tFile$,characterSoundCount

						t.characterSound[t.tfound][ON_DEATH][t.c+1] = g.characterSoundCount;
						++g.characterSoundCount;
						t.characterSound[t.tfound][ON_DEATH][0] = t.characterSound[t.tfound][ON_DEATH][0] + 1;
					}
					else
					{
						t.tBroken = 1;
					}
				}
		}

		t.tBroken = 0;
		t.characterSound[t.tfound][ON_HURT][0] = 0;
		for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS-1; t.c++ )
		{
				if (  t.tBroken  ==  0 ) 
				{
					t.tfile_s = t.tFolder_s+"onHurt\\" + Str(t.c) + ".wav";
					if (  FileExist ( t.tfile_s.Get() ) ) 
					{
						Load3DSound (  t.tfile_s.Get(),g.characterSoundCount );
//       `load sound tFile$,characterSoundCount

						t.characterSound[t.tfound][ON_HURT][t.c+1] = g.characterSoundCount;
						++g.characterSoundCount;
						t.characterSound[t.tfound][ON_HURT][0] = t.characterSound[t.tfound][ON_HURT][0] + 1;
					}
					else
					{
						t.tBroken = 1;
					}
				}
		}

		t.tBroken = 0;
		t.characterSound[t.tfound][ON_HURT_PLAYER][0] = 0;
		for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS-1; t.c++ )
		{
				if (  t.tBroken  ==  0 ) 
				{
					t.tfile_s = t.tFolder_s+"onHurtPlayer\\" + Str(t.c) + ".wav";
					if (  FileExist ( t.tfile_s.Get() ) ) 
					{
						Load3DSound (  t.tfile_s.Get(),g.characterSoundCount );
//       `load sound tFile$,characterSoundCount

						t.characterSound[t.tfound][ON_HURT_PLAYER][t.c+1] = g.characterSoundCount;
						++g.characterSoundCount;
						t.characterSound[t.tfound][ON_HURT_PLAYER][0] = t.characterSound[t.tfound][ON_HURT_PLAYER][0] + 1;
					}
					else
					{
						t.tBroken = 1;
					}
				}
		}

		t.tBroken = 0;
		t.characterSound[t.tfound][ON_IDLE][0] = 0;
		for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS-1; t.c++ )
		{
				if (  t.tBroken  ==  0 ) 
				{
					t.tfile_s = t.tFolder_s+"onIdle\\" + Str(t.c) + ".wav";
					if (  FileExist ( t.tfile_s.Get() ) ) 
					{
						Load3DSound (  t.tfile_s.Get(),g.characterSoundCount );
//       `load sound tFile$,characterSoundCount

						t.characterSound[t.tfound][ON_IDLE][t.c+1] = g.characterSoundCount;
						++g.characterSoundCount;
						t.characterSound[t.tfound][ON_IDLE][0] = t.characterSound[t.tfound][ON_IDLE][0] + 1;
					}
					else
					{
						t.tBroken = 1;
					}
				}
		}

		t.tBroken = 0;
		t.characterSound[t.tfound][ON_INTERACT][0] = 0;
		for ( t.c = 0 ; t.c<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS-1; t.c++ )
		{
				if (  t.tBroken  ==  0 ) 
				{
					t.tfile_s = t.tFolder_s+"onInteract\\" + Str(t.c) + ".wav";
					if (  FileExist ( t.tfile_s.Get() ) ) 
					{
						Load3DSound (  t.tfile_s.Get(),g.characterSoundCount );
//       `load sound tFile$,characterSoundCount

						t.characterSound[t.tfound][ON_INTERACT][t.c+1] = g.characterSoundCount;
						++g.characterSoundCount;
						t.characterSound[t.tfound][ON_INTERACT][0] = t.characterSound[t.tfound][ON_INTERACT][0] + 1;
					}
					else
					{
						t.tBroken = 1;
					}
				}
		}

	}

	//  store sound bank in entityelement
	t.entityelement[t.e].characterSoundBankNumber = t.tfound;

return;

}

void character_soundset ( void )
{
	//PE: soundset_s has been removed from chars.
	//PE: For now always use male, until this is converted into a new lua setup.
	#ifdef VRTECH
	//	t.s_s=t.entityelement[t.e].eleprof.soundset_s;
	t.s_s = "male";
	#else
	t.s_s=t.entityelement[t.e].eleprof.soundset_s;
	#endif
	if (  Len(t.s_s.Get())<2  )  t.s_s = "soldier";
	character_sound_load ( );
}

void character_sound_play ( void )
{

	if (  t.entityelement[t.e].characterSoundBankNumber  ==  -1  )  return;

	t.s_s=Lower(t.s_s.Get());

	if (  g.characterSoundCurrentPlayingNumber > 0 ) 
	{
		//  place on stack if we are currently playing a sound - if we have room
		//  only different types of sound will be placed on the stack, so we dont get 5 alerts qued etc
		if (  SoundPlaying(g.characterSoundCurrentPlayingNumber)  ==  1 ) 
		{
			//  check we dont have the same type in the list already
			if (  g.characterSoundCurrentPlayingType_s  ==  t.s_s  )  t.tfound  =  1; else t.tfound  =  0;
			for ( t.c = 0 ; t.c<=  g.characterSoundStackSize; t.c++ )
			{
					if (  t.tfound  ==  0 ) 
					{
						//C++ISSUE - characterSoundStackType$ should of been an array, so i am fixing a bug here, but hopefully it wont have any knock on effects
						if (  t.characterSoundStackType_s[t.c] == t.s_s  )  t.tfound  =  1;
					}
			}

			if (  t.tfound  ==  0 ) 
			{
				t.tSlotAvailable = 0;
				if (  g.characterSoundStackSize > 0 ) 
				{
					t.tSlotAvailable=g.characterSoundStackSize;
					++g.characterSoundStackSize;
					if (  g.characterSoundStackSize > CHARACTERSOUND_STACK_SIZE-1  )  g.characterSoundStackSize  =  CHARACTERSOUND_STACK_SIZE-1;
				}
					t.characterSoundStackEntity[t.tSlotAvailable] = t.e;
					t.characterSoundStackType_s[t.tSlotAvailable] = t.s_s;
				return;
			}
			return;
		}
		else
		{
			g.characterSoundCurrentPlayingNumber = 0;
		}
	}

		if ( strcmp ( t.s_s.Get() ,  "onaggro" ) == 0 ) t.onType  =  ON_AGGRO;
		else if ( strcmp ( t.s_s.Get() , "onalert" ) == 0 ) t.onType  =  ON_ALERT ;  
		else if ( strcmp ( t.s_s.Get() , "ondeath" ) == 0 ) t.onType  =  ON_DEATH ;  
		else if ( strcmp ( t.s_s.Get() , "onhurt" ) == 0 ) t.onType  =  ON_HURT ;  
		else if ( strcmp ( t.s_s.Get() , "onhurtplayer" ) == 0 ) t.onType  =  ON_HURT_PLAYER ;  
		else if ( strcmp ( t.s_s.Get() , "onidle" ) == 0 ) t.onType  =  ON_IDLE ;  
		else if ( strcmp ( t.s_s.Get() , "oninteract" ) == 0 ) t.onType  =  ON_INTERACT ;

//entity_lua_findcharanimstate ( );
	if (  1 ) 
	{

		if (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][0] < 1  )  return;
		t.tPicked = Rnd(t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][0]-1)+1;
		//  if we have a choice, lets not pick the same one as previous
		if (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][0] > 1 ) 
		{
			while (  g.characterSoundPrevPickedNumber  ==  t.tPicked ) 
			{
				t.tPicked = Rnd(t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][0]-1)+1;
			}
		}

		g.characterSoundPrevPickedNumber = t.tPicked;

		if (  SoundPlaying (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked] )  ==  0 ) 
		{
			if ( t.charanimstate.obj > 0 )
			{
				if ( ObjectExist ( t.charanimstate.obj ) == 1 )
				{
					PositionSound (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked], ObjectPositionX(t.charanimstate.obj),ObjectPositionY(t.charanimstate.obj),ObjectPositionZ(t.charanimstate.obj) );
					PlaySound (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked] );
					g.characterSoundCurrentPlayingNumber = t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked];
					g.characterSoundCurrentPlayingType_s = t.s_s;
				}
			}
		}

	}

return;

}

void character_sound_update ( void )
{

	if (  g.characterSoundStackSize  ==  0  )  return;
	if (  g.characterSoundCurrentPlayingNumber > 0 ) 
	{
		//  if sound still playing, return
		if (  SoundPlaying(g.characterSoundCurrentPlayingNumber)  ==  1  )  return;
	}
	g.characterSoundCurrentPlayingNumber = 0;

	//  grab the next one on the stack and play it
	t.e = t.characterSoundStackEntity[0];
	t.s_s = t.characterSoundStackType_s[0];
	--g.characterSoundStackSize;
	for ( t.c = 0 ; t.c<=  g.characterSoundStackSize; t.c++ )
	{
		t.characterSoundStackEntity[t.c] = t.characterSoundStackEntity[t.c+1];
		t.characterSoundStackType_s[t.c] = t.characterSoundStackType_s[t.c+1];
	}

		if ( strcmp ( t.s_s.Get() , "onaggro" ) == 0 ) t.onType  =  ON_AGGRO ;  
		else if ( strcmp ( t.s_s.Get() , "onalert" ) == 0 ) t.onType  =  ON_ALERT ;  
		else if ( strcmp ( t.s_s.Get() , "ondeath" ) == 0 ) t.onType  =  ON_DEATH ;  
		else if ( strcmp ( t.s_s.Get() , "onhurt" ) == 0 ) t.onType  =  ON_HURT ;  
		else if ( strcmp ( t.s_s.Get() , "onhurtplayer" ) == 0 ) t.onType  =  ON_HURT_PLAYER ;  
		else if ( strcmp ( t.s_s.Get() , "onidle" ) == 0 ) t.onType  =  ON_IDLE ;  
		else if ( strcmp ( t.s_s.Get() , "oninteract" ) == 0 ) t.onType  =  ON_INTERACT ;  

//entity_lua_findcharanimstate ( );
	if (  1 ) 
	{

		if (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][0] < 1  )  return;
		t.tPicked = Rnd(t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][0]-1)+1;
		if (  SoundPlaying ( t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked])  ==  0 ) 
		{
			PositionSound (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked], ObjectPositionX(t.charanimstate.obj),ObjectPositionY(t.charanimstate.obj),ObjectPositionZ(t.charanimstate.obj) );
			PlaySound (  t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked] );
			g.characterSoundCurrentPlayingNumber = t.characterSound[t.entityelement[t.e].characterSoundBankNumber][t.onType][t.tPicked];
			g.characterSoundCurrentPlayingType_s = t.s_s;
		}

	}

return;

}

void character_sound_free ( void )
{

	for ( t.tBankCount = 0 ; t.tBankCount<=  CHARACTERSOUND_MAX_BANK-1; t.tBankCount++ )
	{
		for ( t.tTypeCount = 0 ; t.tTypeCount<=  CHARACTERSOUND_SIZE-1; t.tTypeCount++ )
		{
			for ( t.tSoundCount = 1 ; t.tSoundCount<=  CHARACTERSOUND_MAX_BANK_MAX_SOUNDS; t.tSoundCount++ )
			{

					if (  t.characterSound[t.tBankCount][t.tTypeCount][t.tSoundCount] > 0 ) 
					{
						if (  SoundExist(t.characterSound[t.tBankCount][t.tTypeCount][t.tSoundCount]) ) 
						{
							if (  SoundPlaying(t.characterSound[t.tBankCount][t.tTypeCount][t.tSoundCount]) ) 
							{
								StopSound (  t.characterSound[t.tBankCount][t.tTypeCount][t.tSoundCount] );
							}
							DeleteSound (  t.characterSound[t.tBankCount][t.tTypeCount][t.tSoundCount] );
							t.characterSound[t.tBankCount][t.tTypeCount][t.tSoundCount] = 0;
						}
					}

			}
		}
	}

}
