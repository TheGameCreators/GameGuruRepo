-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Enters Checkpoint Zone

function final_assault_music_init(e)
end

function final_assault_music_main(e)
 if g_Entity[e]['plrinzone']==1 then
  PlayFinalAssaultMusic(500)
  Checkpoint(e)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
