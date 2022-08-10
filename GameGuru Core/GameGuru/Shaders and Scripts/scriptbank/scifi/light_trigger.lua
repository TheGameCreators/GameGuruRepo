-- LUA Script - precede every function and global member with lowercase name of script + '_main'
uselander_endsequence = 0

function light_trigger_init(e)
end
function light_trigger_main(e)
 if g_Entity[e]['activated'] == 0 then
  SetActivated(e,1)
  HideLight(e)
 else
  if uselander_endsequence > 0 then
   ShowLight(e)
  end
 end
end
