-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Door Prompts 'Closed' can be opened with entity collected specified by 'USE KEY' 

-- state to ensure user must release E key before can open/close again
door_pressed = 0

function missiondoor_init(e)
end
	 
function missiondoor_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 100 and KillCount < 5 then
  Prompt("Complete your objective first!")
 end

 if PlayerDist < 100 and KillCount == 5 then
  if g_Entity[e]['activated'] == 0 then
   if g_Entity[e]['haskey'] == -1 then
    SetActivated(e,1)
   else
    if g_Entity[e]['haskey'] == 1 then 
     Prompt("The door is locked. Press E key to unlock door")
     if g_KeyPressE == 1 then 
      SetActivated(e,1)
     end
    else
     Prompt("The door is locked. Find a key to unlock door")
    end
   end
  else
   if g_Entity[e]['activated'] == 1 then
    -- door is unlocked and closed
 	Prompt("Press E to open door")
    if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and door_pressed == 0 then 
	 SetAnimation(0)
	 PlayAnimation(e)
	 g_Entity[e]['animating'] = 1
     SetActivated(e,2)
	 ActivateIfUsed(e)
     PlaySound(e,0)
	 StartTimer(e)
	 door_pressed = 1
	end
   else
    if g_Entity[e]['activated'] == 2 then
	 -- door collision off after 1 second
	 if GetTimer(e)>1000 then
 	  CollisionOff(e)
	 end
	 -- door is open
 	 if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and door_pressed == 0 then
 	  SetAnimation(1)
	  PlayAnimation(e)
 	  g_Entity[e]['animating'] = 1
      SetActivated(e,1)
      PlaySound(e,1)
      CollisionOn(e)
	  door_pressed = 1
	 end
	end
   end
  end
 end
 if g_KeyPressE == 0 then 
  door_pressed = 0
 end
end
