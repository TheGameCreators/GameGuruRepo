
function magicexplosionevent_init(e)
end
	 
function magicexplosionevent_main(e)
 if g_projectileevent_explosion > 0 then
  CollisionOff(e)
  SetPosition ( e, g_projectileevent_x, g_projectileevent_y, g_projectileevent_z )
  CollisionOn(e)
  g_projectileevent_explosion = 0
 end
end
