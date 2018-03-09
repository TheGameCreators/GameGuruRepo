//----------------------------------------------------
//--- GAMEGURU - Common-Sounds
//----------------------------------------------------

#include "gameguru.h"

// 
//  Common Code - Sound Functions
// 

int soundfileexist ( char* tfile_s )
{
	int soundexists = 0;
	cstr tfileogg_s =  "";
	soundexists=0;
	if (  FileExist(tfile_s) == 1 ) 
	{
		soundexists=1;
	}
	else
	{
		//  try as OGG
		tfileogg_s=Left(tfile_s,Len(tfile_s)-4);
		tfileogg_s += ".ogg";
		if (  FileExist(tfileogg_s.Get()) == 1 ) 
		{
			soundexists=1;
		}
	}
//endfunction soundexists
	return soundexists
;
}

float soundtruevolume ( int tvolume_f )
{
	if (  t.audioVolume.soundFloat>0 ) 
	{
		tvolume_f=tvolume_f*t.audioVolume.soundFloat;
	}
	else
	{
		tvolume_f=0.0;
	}
//endfunction tvolume#
	return tvolume_f
;
}

int loadinternalsoundcorecloneflag ( char* tfile_s, int mode, int clonesoundindex )
{
	cstr tfiletoload_s =  "";
	int tspecialogg = 0;
	cstr tfileogg_s =  "";
	unsigned char skipclone = 0;
	int soundid = 0;

//  Terry fix for build game crash when cloning sounds.
//bool skipclone;
skipclone = 0;

//  The file to actually load (can change)
tfiletoload_s=tfile_s;

//  Default return
soundid=0;

//  No scan for existing - all sounds unique for overlay play
int tt;
tt=g.soundbankmax+1;
if (  tt>g.soundbankmax ) 
{
	//  as we change WAV to OGG during build process, end game can try to load WAV but
	//  only the OGG file exists in the standalone media, so detect any OGG and switch to that
	if (  FileExist(tfile_s) == 0 ) 
	{
		tfileogg_s=Left(tfile_s,Len(tfile_s)-4);
		tfileogg_s += ".ogg";
		if (  FileExist(tfileogg_s.Get()) == 1 ) 
		{
			tfiletoload_s=tfileogg_s;
		}
	}
	tspecialogg = 0 ; if ( cstr( Right(Lower(tfiletoload_s.Get()),4)) == ".ogg"  )  tspecialogg = 1;
	if (  cstr(Right(Lower(tfile_s),4)) == ".wav" || tspecialogg == 1 ) 
	{
		if (  FileExist(tfiletoload_s.Get()) == 1 ) 
		{
			//  add this sound to game sound bank
			++g.soundbankmax;
			Dim (  t.soundbank_s,g.soundbankmax  );
			soundid=g.soundbankoffset+g.soundbankmax;
			if (  clonesoundindex > 0 ) 
			{
				//  special mode which clones existing sound (saves memory)
				CloneSound (  soundid,clonesoundindex );
				t.soundbank_s[g.soundbankmax]="cloned sound";
			}
			else
			{
				if (  mode == 1 && tspecialogg == 0 ) 
				{
					Load3DSound (  tfiletoload_s.Get(),soundid,1 );
					if (  SoundExist(soundid) == 0  )  LoadSound (  tfiletoload_s.Get(),soundid,0,1 );
				}
				else
				{
					LoadSound (  tfiletoload_s.Get(),soundid,0,1 );
				}
				if (  SoundExist(soundid) == 0 ) 
				{
					//  sound could not loaded
					t.soundbank_s[g.soundbankmax]="";
					--g.soundbankmax;
					soundid=0;
				}
				else
				{
					//  success
					t.soundbank_s[g.soundbankmax]=tfile_s;
				}
			}
		}
	}
}

//endfunction soundid
	return soundid
;
}

int loadinternalsoundcore ( char* tfile_s, int mode )
{
	int soundid = 0;
	soundid=loadinternalsoundcorecloneflag(tfile_s,mode,0);
//endfunction soundid
	return soundid
;
}

int loadinternalsound ( char* tfile_s )
{
	int soundid = 0;
	soundid=loadinternalsoundcore(tfile_s,0);
//endfunction soundid
	return soundid
;
}

int changeplrforsound ( int tplrid, int tsnd )
{
	int tsoundsetindex = 0;
	int tscansoundid = 0;
	int tscanplrid = 0;
	int scanplrid = 0;
	if (  tplrid>1 && g.gmultiplayergame == 0 ) 
	{
		if (  t.playersound[tplrid][tsnd] == 0 ) 
		{
			//  look for available sound that matches sound set index
			tsoundsetindex=t.playersoundsetindex[tplrid];
			for ( tscanplrid = 1 ; tscanplrid<=  g.playermax; tscanplrid++ )
			{
				tscansoundid=t.playersound[tscanplrid][tsnd];
				if (  tscansoundid>0 ) 
				{
					if (  t.playersoundsetindex[tscanplrid] == tsoundsetindex ) 
					{
						if (  SoundExist(tscansoundid) == 1 ) 
						{
							if (  SoundPlaying(tscansoundid) == 0 ) 
							{
								tplrid=tscanplrid ; scanplrid=g.playermax;
							}
						}
					}
				}
			}
		}
	}
//endfunction tplrid
	return tplrid
;
}

void deleteinternalsound ( int soundid )
{
	int sndid = 0;
	sndid = soundid - g.soundbankoffset;
	if (  sndid>0 && sndid <= ArrayCount(t.soundbank_s) ) 
	{
		if (  SoundExist(soundid) == 1  )  DeleteSound (  soundid );
		t.soundbank_s[sndid]="";
	}
//endfunction

}

// `broadcast from old AI system

// `function managebroadcastsound()

//if bcsoundstrength#>0.0
// dec bcsoundstrength#,1.0
// if (  bcsoundstrength#<0  )  bcsoundstrength# = 0.0
//endif
// `endfunction

// `function broadcast3dsound(x#,y#,z#,strength#,obj)

//if strength#>bcsoundstrength#
// bcsoundstrength#=strength#
// bcsoundx#=x# ; bcsoundy#=y# ; bcsoundz#=z#
//endif
// `endfunction

// `function playinternalBC3dsound(id,x#,y#,z#,strength#)

//if id>0
// if SoundExist(id)=1
//  broadcast3dsound(x#,y#,z#,strength#,0)
//  PositionSound (  id,x#/10.0,y#/3.0,z#/10.0 )
//  PlaySound (  id )
// endif
//endif
// `endfunction


void playinternalsound ( int id )
{
	if (  id>0 ) 
	{
		if (  SoundExist(id) == 1 ) 
		{
			if (  SoundPlaying(id) == 0 ) 
			{
				PlaySound (  id );
			}
		}
	}
//endfunction

}

void playinternalsoundinterupt ( int id )
{
	if (  id>0 ) 
	{
		if (  SoundExist(id) == 1 ) 
		{
			PlaySound (  id );
			SetSoundVolume (  id,soundtruevolume(100.0) );
		}
	}
//endfunction

}

void playinternal3dsound ( int id, int x_f, int y_f, int z_f )
{
	if (  id>0 ) 
	{
		if (  SoundExist(id) == 1 ) 
		{
			PositionSound (  id,x_f,y_f,z_f );
			PlaySound (  id );
			SetSoundVolume (  id,soundtruevolume(100.0) );
		}
	}
//endfunction

}

void playinternalBC3dsound ( int id, int x_f, int y_f, int z_f, int strength_f )
{
	//  strength# can alter volume..todo
	playinternal3dsound(id,x_f,y_f,z_f);
//endfunction

}

void loopinternal3dsound ( int id, int x_f, int y_f, int z_f )
{
	if (  id>0 ) 
	{
		if (  SoundExist(id) == 1 ) 
		{
			PositionSound (  id,x_f,y_f,z_f );
			SetSoundVolume (  id,soundtruevolume(100.0) );
			if (  SoundPlaying(id) == 0  )  LoopSound (  id );
		}
	}
}

void posinternal3dsound ( int id, int x_f, int y_f, int z_f )
{
	if (  id>0 ) 
	{
		if (  SoundExist(id) == 1 ) 
		{
			PositionSound (  id,x_f,y_f,z_f );
		}
	}
}

void playinternal3dsoundfactor ( int id, int x_f, int y_f, int z_f, int factor_f )
{
	if (  id>0 ) 
	{
		if (  SoundExist(id) == 1 ) 
		{
//    `position sound id,x#/10.0,y#/3.0,z#/10.0

			PositionSound (  id,x_f,y_f,z_f );
			factor_f=(factor_f*2.0)+80.0;
			if (  factor_f<0  )  factor_f = 0;
			if (  factor_f>100  )  factor_f = 100;
			SetSoundVolume (  id,soundtruevolume(factor_f) );
			PlaySound (  id );
		}
	}
//endfunction

}


//Music system


void music_init ( void )
{

	//  make sure nothing is playing and values are reset
	for ( t.m = 1 ; t.m<=  MUSICSYSTEM_MAXTRACKS; t.m++ )
	{
		music_resettrack(t.m);
	}
	g.musicsystem.percentageVolume = 100;
	g.musicsystem.fadetime = 0;
	g.musicsystem.default = 0;

return;

}

void music_loop ( void )
{
	//  process all our music tracks
	t.tTime = Timer();
	for ( t.m = 1 ; t.m<=  MUSICSYSTEM_MAXTRACKS; t.m++ )
	{
		if (  t.musictrack[t.m].state > 0 ) 
		{
			t.tSndID = g.musicsoundoffset + t.m - 1;
			if (  SoundExist(t.tSndID)  ==  1 ) 
			{
				//  if the music is on a Timer ( , see if this time has elapsed, and if it has, return to the default track )
				if ( t.musictrack[t.m].playtime > 0 ) 
				{
					t.timePlayed = t.tTime - t.musictrack[t.m].playtimestamp;
					if (  t.timePlayed > t.musictrack[t.m].playtime - MUSICSYSTEM_TRANSITIONTIMEBUFFER ) 
					{
						t.musictrack[t.m].playtime = 0;
						t.musictrack[t.m].playtimestamp = 0;

						//  if we have the default track cued up, we're using the interval system, so don't change tracks right now. If we don't
						//  have a track cued up, we need to stop this one and start the default track
						if (  t.musictrack[t.m].cuetrack  ==  0 ) 
						{
							if (  g.musicsystem.default > 0 ) 
							{
								music_stoptrack(t.m,t.musictrack[t.m].playtimefade*3);
								music_play(g.musicsystem.default,t.musictrack[t.m].playtimefade);
							}
							else
							{
								music_stoptrack(t.m,t.musictrack[t.m].playtimefade);
							}
						}
					}
				}
				else
				{

					//  if the music has been told to play another music at it's next interval, check its interval and see if we should play it.
					if (  t.musictrack[t.m].cuetrack > 0 ) 
					{
						if (  t.musictrack[t.m].interval > 0 ) 
						{
							t.tTimePlayed = t.tTime - t.musictrack[t.m].startstamp;
							t.tTimePastInterval = mod(t.tTimePlayed,t.musictrack[t.m].interval);
							if (  t.tTimePastInterval < MUSICSYSTEM_TRANSITIONTIMEBUFFER || t.musictrack[t.m].interval - t.tTimePastInterval < MUSICSYSTEM_TRANSITIONTIMEBUFFER ) 
							{

								//  we're within the magic 'zone' where we can transition into our cue track
								music_play(t.musictrack[t.m].cuetrack,0);
								music_stoptrack(t.m,t.musictrack[t.m].cuefade);
								t.musictrack[t.m].cuetrack = 0;
								t.musictrack[t.m].cuefade = 0;
							}
						}
					}
				}

				//  we may need to retrigger the playing of this sound if it's almost at the end
				if (  t.tTime > t.musictrack[t.m].startstamp + t.musictrack[t.m].length - MUSICSYSTEM_TRANSITIONTIMEBUFFER ) 
				{
					PlaySound (  t.tSndID );
					t.musictrack[t.m].startstamp = t.tTime;
				}

				//  we may need to fade the volume towards the target volume
				if (  t.musictrack[t.m].fadetime > 0 ) 
				{
					if (  t.tTime > t.musictrack[t.m].fadestamp + t.musictrack[t.m].fadetime ) 
					{
						t.musictrack[t.m].fadetime = 0;
						t.musictrack[t.m].currentVolume = t.musictrack[t.m].targetVolume;
					}
					else
					{
						t.tVolumechange_f = t.musictrack[t.m].targetVolume - t.musictrack[t.m].previousVolume;
						t.musictrack[t.m].currentVolume = t.musictrack[t.m].previousVolume + (t.tTime - t.musictrack[t.m].fadestamp)/(t.musictrack[t.m].fadetime*1.0) * t.tVolumechange_f;
					}
				}

				//  always set the volume. This will mean it always respects the global volume set in the settings menus
				SetSoundVolume (  t.tSndID,music_getplaybackvolume(t.musictrack[t.m].currentVolume) );

				//  if this sound is stopping and it's volume is below the minimum, stop the sound
				if (  t.musictrack[t.m].state  ==  2 ) 
				{
					if (  t.musictrack[t.m].currentVolume  <=  1 ) 
					{
						music_resettrack(t.m);
					}
				}
			}
		}
	}

return;

}

void music_fadeAllMusicToNewVolume ( void )
{

	//  if the master volume is adjusted we can set all tracks to fade to this new volume
	t.tTime = Timer();
	for ( t.m = 1 ; t.m<=  MUSICSYSTEM_MAXTRACKS; t.m++ )
	{
		if (  t.musictrack[t.m].state  ==  1 ) 
		{
			if (  t.musictrack[t.m].targetVolume  !=  g.musicsystem.percentageVolume ) 
			{
				t.musictrack[t.m].previousVolume = t.musictrack[t.m].currentVolume;
				t.musictrack[t.m].targetVolume = g.musicsystem.percentageVolume;
				t.musictrack[t.m].fadetime = g.musicsystem.fadetime;
				t.musictrack[t.m].fadestamp = t.tTime;
			}
		}
	}

return;

}

void music_resettrack ( int m )
{
	int tSndID = 0;

	//  sets a track back to default start state
	if (  m < 1 || m > MUSICSYSTEM_MAXTRACKS  )  return;
	t.musictrack[m].state = 0;
	t.musictrack[m].previousVolume = 0;
	t.musictrack[m].currentVolume = g.musicsystem.percentageVolume;
	t.musictrack[m].targetVolume = g.musicsystem.percentageVolume;
	t.musictrack[m].fadetime = 0;
	t.musictrack[m].cuetrack = 0;
	t.musictrack[m].cuefade = 0;
	t.musictrack[m].startstamp = 0;
	t.musictrack[m].fadestamp = 0;
	t.musictrack[m].playtimestamp = 0;
	t.musictrack[m].playtime = 0;
	t.musictrack[m].playtimefade = 0;
	tSndID = g.musicsoundoffset + m - 1;
	if (  SoundExist(tSndID)  ==  0  )  return;
	SetSoundVolume (  tSndID,music_getplaybackvolume(0) );
	StopSound (  tSndID );

//endfunction

}

void music_stoptrack ( int m, int tFadeOut )
{
	int tSndID = 0;

	//  set a track for fading out and stopping
	if (  m < 1 || m > MUSICSYSTEM_MAXTRACKS  )  return;
	if (  t.musictrack[m].state  ==  0  )  return;
	if (  t.musictrack[m].state  ==  2  )  return;
	tSndID = g.musicsoundoffset + m - 1;
	if (  SoundExist(tSndID)  ==  1 ) 
	{
		if (  tFadeOut  <=  0 ) 
		{
			music_resettrack(m);
		}
		else
		{
			t.musictrack[m].fadetime = tFadeOut;
			t.musictrack[m].fadestamp = Timer();
			t.musictrack[m].previousVolume = t.musictrack[m].currentVolume;
			t.musictrack[m].targetVolume = 0;
			t.musictrack[m].state = 2;
		}
	}
//endfunction

}

void music_stopall ( int tFadeOut )
{
	int m = 0;

	//  stop all music
	for ( m = 1 ; m<=  MUSICSYSTEM_MAXTRACKS; m++ )
	{
		music_stoptrack(m,tFadeOut);
	}

//endfunction

}

void music_play ( int m, int tFadeTime )
{
	int tSndID = 0;
	int tTime = 0;

	// 271115 - if already in playing state, do not repeat as it will make music stutter!
	if ( t.musictrack[m].state == 1 ) return;

	//  start music and fade in
	tSndID = g.musicsoundoffset + m - 1;
	if (  SoundExist(tSndID)  ==  0  )  return;
	t.musictrack[m].targetVolume = g.musicsystem.percentageVolume;
	t.musictrack[m].previousVolume = 0;
	tTime = Timer();
	t.musictrack[m].startstamp = tTime;
	t.musictrack[m].fadestamp = tTime;
	t.musictrack[m].fadetime = tFadeTime;
	t.musictrack[m].state = 1;
	if (  tFadeTime  ==  0 ) 
	{
		SetSoundVolume (  tSndID, music_getplaybackvolume(g.musicsystem.percentageVolume) );
	}
	else
	{
		SetSoundVolume (  tSndID, music_getplaybackvolume(0) );
	}
	PlaySound (  tSndID );

//endfunction

}

int music_getplaybackvolume ( int tVol )
{
	int tReturnVol = 0;
	//  returns the actual volume to play back the sound, respecting the tracks percentage volume and the global setting for music volume
	tReturnVol = MUSICSYSTEM_MINVOLUME + (MUSICSYSTEM_MAXVOLUME - MUSICSYSTEM_MINVOLUME) * (tVol/100.0) * t.audioVolume.musicFloat;
	return tReturnVol;
}

void music_resetall ( void )
{

	//  resets all music tracks (normally called on death)
	for ( t.m = 1 ; t.m<=  MUSICSYSTEM_MAXTRACKS; t.m++ )
	{
		music_resettrack(t.m);
	}

return;

}

void music_playdefault ( void )
{

	//  sets the default music playing again, normally after respawn
	music_play(g.musicsystem.default,10000);

}

//
// Footfall Sounds for Player and Characters
//

void sound_footfallsound ( int footfalltype, float fX, float fY, float fZ, int* lastfootfallsound )
{
	t.trndsnd = Rnd(3);
	if ( t.trndsnd == 0 ) t.tsnd = t.material[footfalltype].tred0id;
	if ( t.trndsnd == 1 ) t.tsnd = t.material[footfalltype].tred1id;
	if ( t.trndsnd == 2 ) t.tsnd = t.material[footfalltype].tred2id;
	if ( t.trndsnd == 3 ) t.tsnd = t.material[footfalltype].tred3id;
	if ( t.tsnd>0 ) 
	{
		if ( t.trndsnd == *lastfootfallsound ) 
		{
			t.trndsnd = t.trndsnd+1 ; if ( t.trndsnd>3 ) t.trndsnd = 0;
			if ( t.trndsnd == 0 ) t.tsnd = t.material[footfalltype].tred0id;
			if ( t.trndsnd == 1 ) t.tsnd = t.material[footfalltype].tred1id;
			if ( t.trndsnd == 2 ) t.tsnd = t.material[footfalltype].tred2id;
			if ( t.trndsnd == 3 ) t.tsnd = t.material[footfalltype].tred3id;
		}
		*lastfootfallsound = t.trndsnd;

		//  play this material sound (will play tsnd+0 through tsnd+4)
		t.tsoundtrigger = t.tsnd; t.tvol_f = 90;
		t.tspd_f = t.material[footfalltype].freq;
		t.tsx_f = fX;
		t.tsy_f = fY;
		t.tsz_f = fZ;
		material_triggersound ( );
	}
}
