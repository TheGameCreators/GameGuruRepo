-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- main music for level1

function main_music01_init(e)
end

function main_music01_main(e)
 if g_Egg1Count==20 then
  StopSound(e,0)
  else if g_FirstRun == 1 then
   LoopSound(e,0)
  end
 end
end
