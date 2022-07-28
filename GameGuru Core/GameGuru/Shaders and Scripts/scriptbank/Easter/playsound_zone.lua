-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Sound Zone

function playsound_zone_init(e)
end

function playsound_zone_main(e)
 if g_Entity[e]['plrinzone']==1 then
  LoopSound(e,0)
  else
  StopSound(e)
 end
end
