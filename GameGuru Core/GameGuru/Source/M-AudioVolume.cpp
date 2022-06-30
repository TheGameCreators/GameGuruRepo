//----------------------------------------------------
//--- GAMEGURU - M-AudioVolume
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  Character Sound
// 

//  globals; t.audioVolume.music, t.audioVolume.sound, both percentage

void audio_volume_init ( void )
{
	t.audioVolume.music = 100;
	t.audioVolume.sound = 100;
	t.audioVolume.musicFloat = 1.0;
	t.audioVolume.soundFloat = 1.0;
}

void audio_volume_update ( void )
{
	// convert to 0 - 1.0 to enable us to multiply volumes in game
	t.audioVolume.musicFloat = t.audioVolume.music;
	t.audioVolume.musicFloat = (( t.audioVolume.musicFloat * 0.30 ) + 70.0 ) * 0.01;
	t.audioVolume.soundFloat = t.audioVolume.sound;
	t.audioVolume.soundFloat = (( t.audioVolume.soundFloat * 0.30 ) + 70.0 ) * 0.01;

	//  change volume of all sounds and music
	for ( t.tSound = 1 ; t.tSound<=  65535; t.tSound++ )
	{
		if (  SoundExist(t.tSound) == 1 ) 
		{
			if (  t.tSound >= g.musicsoundoffset && t.tSound <= g.musicsoundoffsetend ) 
			{
				//  handled in music code	
			}
			else
			{
				SetSoundVolume (  t.tSound, (100.0 * t.audioVolume.soundFloat) );
			}
		}
	}
}

void audio_volume_free ( void )
{
	//  nothing needed
}
