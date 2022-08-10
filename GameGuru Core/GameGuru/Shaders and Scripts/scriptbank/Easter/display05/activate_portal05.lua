-- LUA Script - precede every function and global member with lowercase name of script + '_main'

portal_activated = 0

function activate_portal05_init(e)
end

function activate_portal05_main(e)
 if g_Egg1Count==7 and g_Egg2Count==3 and g_Egg3Count==7 and g_Egg4Count==3 and g_Egg5Count==3 and portal_activated == 0 then
  ActivateIfUsed(e)
  PlaySound(e,0)
  portal_activated = 1
  Destroy(e)
 end
end
