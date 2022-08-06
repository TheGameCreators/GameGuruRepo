-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- plays a final music

function timesup03_init(e)
end

function timesup03_main(e)
 if Timesup == 1 then
  PlaySound(e,0)
  Destroy(e)
 end
end
