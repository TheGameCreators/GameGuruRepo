-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- main music for level2

function main_music02_init(e)
end

function main_music02_main(e)
 if g_Egg1Count==20 and g_Egg2Count==20 then
  StopSound(e,0)
 else if g_EKey2 == 1 then
  LoopSound(e,0)
 end
 end
end
