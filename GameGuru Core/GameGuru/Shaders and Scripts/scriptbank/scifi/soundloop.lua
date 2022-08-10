-- LUA Script - precede every function and global member with lowercase name of script + '_main'
function soundloop_init(e)
end
function soundloop_main(e)
 if g_Entity[e]['activated']==0 then
  SetActivated(e,1)
  LoopSound(e,0)
 end
end
