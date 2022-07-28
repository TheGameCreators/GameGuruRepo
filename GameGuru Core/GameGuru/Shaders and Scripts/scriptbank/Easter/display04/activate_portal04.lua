-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function activate_portal04_init(e)
end

function activate_portal04_main(e)
 if g_Egg1Count==13 and g_Egg2Count==13 and g_Egg3Count==10 and g_Egg4Count==15 then
  ActivateIfUsed(e)
  PlaySound(e,0)
  Destroy(e)
 end
end
