-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- main music zone for level5

function main_music01_05_init(e)
end

function main_music01_05_main(e)
 if g_princess_rescued==1 or skydome == 1 then
  StopSound(e,0)
 else if g_EKey6 == 1 then
  LoopSound(e,0)
 end
 end
end
