-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function skydome_05_init(e)
end

function skydome_05_main(e)
 if skydome==0 then
  Hide(e)
 else
  Show(e)
 end
end
