-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Door Prompts 'Closed' can be opened with entity collected specified by 'USE KEY' 

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
        SetActivated(e,1)
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
	   SetAnimation(0)
	   PlayAnimation(e)
	   g_Entity[e]['animating'] = 1
       SetActivated(e,2)
	   ActivateIfUsed(e)
       PlaySound(e,0)
	   StartTimer(e)
	   g_Entity[e]['timer'] = g_Time
	   door_pressed = 1
	  end
	 end
    else
     if g_Entity[e]['activated'] == 2 then
	  -- door is open
	  if g_Entity[e]['plrvisible'] == 1 then 
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
  end
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
