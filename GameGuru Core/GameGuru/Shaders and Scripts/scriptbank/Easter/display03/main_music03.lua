-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- main music for level3

function main_music03_init(e)
end

function main_music03_main(e)
 if g_Entity[e]['plrinzone']==1 then
  LoopSound(e,0)
  ActivateIfUsed(e)
 end
 if g_Egg1Count==20 and g_Egg2Count==20 and g_Egg3Count==20 or Timesup == 1 then
  StopSound(e,0)
  Destroy(e)
 end
end
