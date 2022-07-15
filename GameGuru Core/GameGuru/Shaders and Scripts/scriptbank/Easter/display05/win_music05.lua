-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays a final music

function win_music05_init(e)
end

function win_music05_main(e)
 if g_princess_rescued==1 then
  PlaySound(e,0)
  Destroy(e)
 end
end
