-- LUA Script - precede every function and global member with lowercase name of script + '_main'

gate_pressed = 0

function castle_gate_02_05_init(e)
end
	 
function castle_gate_02_05_main(e)
 PlayerDist = GetPlayerDistance(e)
 if (PlayerDist < 100 ) and g_PlayerHealth > 0 then
  GetEntityPlayerVisibility(e)
  if 1 then
   if g_Entity[e]['activated'] == 0 then
    if g_Egg1Count==-1 and g_Egg2Count==-1 and g_Egg3Count==-1 and g_Egg4Count==-1 and g_Egg5Count==-1 then
     SetActivated(e,1)
    else
	 if g_Entity[e]['plrvisible'] == 1 then 
      if g_Egg1Count==20 and g_Egg2Count==20 and g_Egg3Count==20 and g_Egg4Count==20 and g_Egg5Count==20 then 
       Prompt("Press E to unlock gate")
       if g_KeyPressE == 1 then 
        SetActivated(e,1)
       end
      else
       Prompt("The gate is locked. Collect all eggs.")
      end
	 end
    end
   else
    if g_Entity[e]['activated'] == 1 then
     -- gate is unlocked and closed
     if g_Entity[e]['plrvisible'] == 1 then 
      Prompt("Press E to open gate")
      if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and gate_pressed == 0 then  
	   SetAnimation(0)
	   PlayAnimation(e)
	   g_Entity[e]['animating'] = 1
       SetActivated(e,2)
	   ActivateIfUsed(e)
       PlaySound(e,0)
	   gate_pressed = 1
	  end
	 end
    else
     if g_Entity[e]['activated'] == 2 then
	  -- gate is open
	  if g_Entity[e]['plrvisible'] == 1 then 
	  Prompt("Press E to close gate")
 	   if g_KeyPressE == 1 and g_Entity[e]['animating'] == 0 and gate_pressed == 0 then
 	    SetAnimation(1)
	    PlayAnimation(e)
 	    g_Entity[e]['animating'] = 1
        SetActivated(e,1)
        PlaySound(e,1)
        CollisionOn(e)
	    gate_pressed = 1
	   end
	  end
	 end
    end
   end
  end
 end
 if g_Entity[e]['activated'] == 2 then
  if GetAnimationFrame(e)>35 then
   CollisionOff(e)
  end
 end
 if g_KeyPressE == 0 then 
  gate_pressed = 0
 end
end
