-- LUA Script - globalsoundtests used to try out all Global Sound commands

function globalsoundtests_init(e)
 LoadGlobalSound ( "audiobank\\misc\\ammo.wav", 1 )
 LoadGlobalSound ( "audiobank\\misc\\bite.wav", 2 )
 LoadGlobalSound ( "audiobank\\misc\\checkpoint.wav", 3 )
 LoadGlobalSound ( "audiobank\\misc\\explode.wav", 4 )
 LoadGlobalSound ( "audiobank\\misc\\weapon.wav", 5 )
 LoadGlobalSound ( "audiobank\\misc\\health.wav", 6 )
 DeleteGlobalSound ( 6 )
 LoadGlobalSound ( "audiobank\\misc\\steam.wav", 6 )
end

function globalsoundtests_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 200 and g_PlayerHealth > 0 then
  Prompt ( "Press Keys 1-6 to play global sounds. 7 to stop all" )
  if g_Scancode == 2 then PlayGlobalSound ( 1 ) end
  if g_Scancode == 3 then PlayGlobalSound ( 2 ) end
  if g_Scancode == 4 then PlayGlobalSound ( 3 ) end
  if g_Scancode == 5 then
   if GetGlobalSoundExist ( 4 ) == 1 then
    PlayGlobalSound ( 4 ) 
    SetGlobalSoundSpeed ( 4, 66000 )
   end
  end
  if g_Scancode == 6 then 
   if GetGlobalSoundPlaying ( 5 ) == 0 then
    PlayGlobalSound ( 5 ) 
    SetGlobalSoundVolume ( 5, 90 )
   end
  end
  if g_Scancode == 7 then
   if GetGlobalSoundLooping ( 6 ) == 0 then
    LoopGlobalSound ( 6 ) 
   end
  end
  if g_Scancode == 8 then 
   StopGlobalSound ( 1 ) 
   StopGlobalSound ( 2 ) 
   StopGlobalSound ( 3 ) 
   StopGlobalSound ( 4 ) 
   StopGlobalSound ( 5 ) 
   StopGlobalSound ( 6 ) 
  end
 end
end
