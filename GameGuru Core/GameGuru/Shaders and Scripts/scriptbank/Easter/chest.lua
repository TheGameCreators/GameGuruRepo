chest_pressed = 0
chest_opened = 0

function chest_init(e)
end
	 
function chest_main(e)
PlayerDist = GetPlayerDistance(e)
 if (PlayerDist < 120 ) and g_PlayerHealth > 0 then
  GetEntityPlayerVisibility(e)
  if 1 then
  if g_KeyPressE == 0 then 
  chest_pressed = 0
 end
if
 GetAnimationFrame(e)==49 and PlayerDist < 120 then
 chest_opened = 1
 else
 chest_opened = 0
 end

 if g_Entity[e]['activated'] == 0 then
    if g_Entity[e]['haskey'] == -1 then
     SetActivated(e,1)
    else
	 if g_Entity[e]['plrvisible'] == 1 then 
      if g_Entity[e]['haskey'] == 1 then 
       Prompt("The chest is locked. Press E key to unlock chest")
       if g_KeyPressE == 1 then 
        SetActivated(e,1)
       end
      else
       Prompt("The chest is locked. Find a key to unlock chest")
      end
	 end
    end
   else
    if g_Entity[e]['activated'] == 1 then
    if g_Entity[e]['plrvisible'] == 1 then 
      Prompt("Press E to open chest")
      if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and chest_pressed == 0 then 
      SetAnimationSpeed(e,2)	  
	   SetAnimation(0)
	   PlayAnimation(e)
	   g_Entity[e]['animating'] = 1
       SetActivated(e,2)
	   ActivateIfUsed(e)
       PlaySound(e,0)
	   StartTimer(e)
	   chest_pressed = 1
	  end
	 end
    else
     if g_Entity[e]['activated'] == 2 then
	  if g_Entity[e]['plrvisible'] == 1 then 
	  Prompt("Press E to close chest")
 	   if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and chest_pressed == 0 then
	   SetAnimationSpeed(e,2)
 	    SetAnimation(1)
	    PlayAnimation(e)
 	    g_Entity[e]['animating'] = 1
        SetActivated(e,1)
        PlaySound(e,1)
        CollisionOn(e)
	    chest_pressed = 1
	   end
	  end
	 end
    end
   end
  end
 end
 if g_Entity[e]['activated'] == 2 then
 if GetTimer(e)>1000 then
   CollisionOn(e)
  end
 end
end
