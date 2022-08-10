-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays a final music

function win_music02_init(e)
end

function win_music02_main(e)
 if g_Egg1Count==20 and g_Egg2Count==20 then
  PlaySound(e,0)
  Destroy(e)
 end
end
