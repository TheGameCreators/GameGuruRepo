-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Door Prompts 'Closed' can be opened with entity collected specified by 'USE KEY' 
-- The 'activated' behavior in multiplayer uses SetActivatedWithMP to broadcast change to other
-- players and when if joining existing game, host will send the 'activated' value plus +100
-- therefore triggering the same activity that happened before the user showed up
-- state to ensure user must release E key before can open/close again
door_pressed = 0

function door_init(e)
end
	 
function door_main(e)
 PlayerDist = GetPlayerDistance(e)
 if (PlayerDist < 100 ) and g_PlayerHealth > 0 then
  GetEntityPlayerVisibility(e)
  if 1 then
   if g_Entity[e]['activated'] == 0 then
    if g_Entity[e]['haskey'] == -1 then
     SetActivated(e,1)
    else
	 if g_Entity[e]['plrvisible'] == 1 then 
      if g_Entity[e]['haskey'] == 1 then 
	   if GetGamePlayerStateXBOX() == 1 then
        Prompt("The door is locked. Press Y button to unlock door")
	   else
        Prompt("The door is locked. Press E key to unlock door")
	   end
       if g_KeyPressE == 1 then 
        SetActivated(e,102)
       end
      else
       Prompt("The door is locked. Find a key to unlock door")
      end
	 end
    end
   else 
    if g_Entity[e]['activated'] == 1 then
     -- door is unlocked and closed
     if g_Entity[e]['plrvisible'] == 1 then 
	  if GetGamePlayerStateXBOX() == 1 then
       Prompt("Press Y button to open door")
	  else
       Prompt("Press E to open door")
	  end
      if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and door_pressed == 0 then  
       SetActivatedWithMP(e,202)
	   door_pressed = 1
	  end
	 end
    else
     if g_Entity[e]['activated'] == 2 then
	  -- door is open
	  if g_Entity[e]['plrvisible'] == 1 then 
 	   if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and door_pressed == 0 then
        SetActivatedWithMP(e,201)
	    door_pressed = 1
	   end
	  end
	 end
    end	
   end
  end
 end
 -- proximity independent
 if g_Entity[e]['activated'] == 100 then
  -- this would be the case if this player joined, and this entity activated value was zero
  -- also good place to do stuff when player joins existing game here (as host sends 'activated'+100)
  SetActivated(e,0)
 end
 if g_Entity[e]['activated'] == 101 then
  SetActivated(e,1)
  endframe = GetEntityAnimationFinish(e,1)
  SetAnimationFrame(e,endframe)
  g_Entity[e]['animating'] = 0
 end
 if g_Entity[e]['activated'] == 102 then
  SetActivated(e,2)
  endframe = GetEntityAnimationFinish(e,0)
  SetAnimationFrame(e,endframe)
  g_Entity[e]['animating'] = 0
 end
 if g_Entity[e]['activated'] == 202 then
  -- open door
  SetAnimation(0)
  PlayAnimation(e)
  g_Entity[e]['animating'] = 1
  SetActivated(e,2)
  ActivateIfUsed(e)
  PlaySound(e,0)
  StartTimer(e)
  g_Entity[e]['timer'] = g_Time
 end
 if g_Entity[e]['activated'] == 201 then
  -- close door
  SetAnimation(1)
  PlayAnimation(e)
  g_Entity[e]['animating'] = 1
  SetActivated(e,1)
  PlaySound(e,1)
  CollisionOn(e)
 end	
 if g_Entity[e]['activated'] == 2 then
  -- door collision off after 1 second
  if GetTimer(e)>1000 then
   CollisionOff(e)
  end
 end
 if g_KeyPressE == 0 then 
  door_pressed = 0
 end
end
