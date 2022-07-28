-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- main music zone for level5

function main_music02_05_init(e)
end

function main_music02_05_main(e)
 if skydome == 1 then
  LoopSound(e,0)
 else
  StopSound(e,0)
 end
end
