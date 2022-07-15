-- LUA Script - precede every function and global member with lowercase name of script + '_main'
uselander_endsequence = 0
liftoff_currenty = 0
liftoff_startposy = 500

function liftoff_init(e)
 liftoff_startposy = g_Entity[e]['y']
end
function liftoff_main(e)
 if liftoff_currenty==0 then
  liftoff_currenty = liftoff_startposy
  Hide(e)
 end
 if uselander_endsequence > 0 then
  if liftoff_currenty==liftoff_startposy then
   Show(e)
   ActivateIfUsed(e)  
  end
  liftoff_currenty = liftoff_currenty + 1
  SetPosition ( e, g_Entity[e]['x'], liftoff_currenty, g_Entity[e]['z'] )
 end
end
