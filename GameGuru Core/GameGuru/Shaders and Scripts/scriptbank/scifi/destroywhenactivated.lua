-- LUA Script - precede every function and global member with lowercase name of script + '_main'
function destroywhenactivated_init(e)
end
function destroywhenactivated_main(e)
 if g_Entity[e]['activated']==1 then
  PlaySound(e,0)
  Destroy(e)
  ActivateIfUsed(e)
 end
end
