-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays a final music

function win_music01_init(e)
end

function win_music01_main(e)
 if g_Egg1Count==20 then
  PlaySound(e,0)
  Destroy(e)
 end
end
