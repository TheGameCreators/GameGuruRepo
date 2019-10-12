-- Constants
--MUSIC_TRACK1 = "audiobank\\atmospheric\\assylumatmos.ogg";
MUSIC_TRACK1 = "";
MUSIC_TRACK1_INTERVAL = 3692;
MUSIC_TRACK1_LENGTH = 38000;

--MUSIC_TRACK2 = "audiobank\\atmospheric\\jungle-wind.ogg";
MUSIC_TRACK2 = "";
MUSIC_TRACK2_INTERVAL = 3692;
MUSIC_TRACK2_LENGTH = 60000;

--MUSIC_TRACK3 = "audiobank\\atmospheric\\WindNearSea.ogg";
MUSIC_TRACK3 = "";
MUSIC_TRACK3_INTERVAL = 3692;
MUSIC_TRACK3_LENGTH = 51705;

--MUSIC_TRACK4 = "audiobank\\atmospheric\\Desert.wav";
MUSIC_TRACK4 = "";
MUSIC_TRACK4_INTERVAL = 0;
MUSIC_TRACK4_LENGTH = 5000;

MUSIC_TRACK_DEFAULT = 0;

-- init when level first runs, load our music and play the default track
function music_init()
	music_load(1, MUSIC_TRACK1, MUSIC_TRACK1_INTERVAL, MUSIC_TRACK1_LENGTH);
	music_load(2, MUSIC_TRACK2, MUSIC_TRACK2_INTERVAL, MUSIC_TRACK2_LENGTH);
	music_load(3, MUSIC_TRACK3, MUSIC_TRACK3_INTERVAL, MUSIC_TRACK3_LENGTH);
	music_load(4, MUSIC_TRACK4, MUSIC_TRACK4_INTERVAL, MUSIC_TRACK4_LENGTH);
	music_set_default(MUSIC_TRACK_DEFAULT);
	if MUSIC_TRACK_DEFAULT > 0 then
		music_play_instant(MUSIC_TRACK_DEFAULT,10000);
		music_set_volume(100,100)
	end	
end

-- load in and setup a piece of music
function music_load(id,str,interval,length)
	if str ~= "" then
		SendMessageS("musicload",id,str);
		SendMessageI("musicsetinterval",id,interval);
		SendMessageI("musicsetlength",id,length);
	end
end

-- plays m fading in over fadetime, stopping any other music over fadetime * 3
function music_play(m,fadeTime)
	music_set_fadetime(fadeTime);
	SendMessageI("musicplayfade",m);	
end

-- plays m fading in over fadetime, stopping any other music instantly
function music_play_instant(m,fadeTime)
	music_set_fadetime(fadeTime);
	SendMessageI("musicplayinstant",m);	
end

-- plays m at the next interval of the current music, starting at full volume and fading out the current music by fadeTime
function music_play_cue(m,fadeTime)
	music_set_fadetime(fadeTime);
	SendMessageI("musicplaycue",m);
end

-- as music_play, but plays m for time playTime before returning to play the default track
function music_play_time(m,playTime,fadeTime)
	music_set_fadetime(fadeTime);
	SendMessageI("musicplaytime",m,playTime);
end

-- as music_play_cue, but plays m for time playTime before returning to play the default track, using timing intervals
function music_play_timecue(m,playTime,fadeTime)
	music_set_fadetime(fadeTime);
	SendMessageI("musicplaytimecue",m,playTime);
end

-- stops the music playing in time fadeTime
function music_stop(fadeTime)
	music_set_fadetime(fadeTime);
	SendMessage("musicstop");
end

-- sets the global music volume and fades the playing track to this volume over fadeTime
function music_set_volume(v,fadeTime)
	music_set_fadetime(fadeTime);
	SendMessageI("musicsetvolume",v);
end

-- sets the default music track m (the track returned to after another track is played for a finite period of time)
function music_set_default(m)
	SendMessageI("musicsetdefault",m);
end

-- sets the global fadetime for all subsequent commands that use fade
function music_set_fadetime(f)
	SendMessageI("musicsetfadetime",f);
end