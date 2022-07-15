-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- main music zone for level4

function main_music04_init(e)
end

function main_music04_main(e)
 if g_Entity[e]['plrinzone']==1 then
  LoopSound(e,0)
  ActivateIfUsed(e)
 end
 if g_Egg1Count==20 and g_Egg2Count==20 and g_Egg3Count==20 and g_Egg4Count==20 then
  StopSound(e,0)
 end
end
