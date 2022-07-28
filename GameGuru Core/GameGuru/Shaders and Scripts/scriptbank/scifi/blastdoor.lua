-- LUA Script - precede every function and global member with lowercase name of script + '_main'
engineer_door_password = 0

function blastdoor_init(e)
 StartTimer(e)
end
function blastdoor_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 100 and g_PlayerHealth > 0 then
  GetEntityPlayerVisibility(e)
  if g_Entity[e]['plrvisible'] == 1 then
   if g_Entity[e]['activated'] == 0 and engineer_door_password==0 then
    if GetTimer(e)>5000 then
     PlaySound(e,0)
     StartTimer(e)
	end
   end
   if g_Entity[e]['activated'] == 0 and engineer_door_password==1 then
    SetActivated(e,1)
	SetAnimation(0)
	PlayAnimation(e)
	g_Entity[e]['animating'] = 1
	ActivateIfUsed(e)
    PlaySound(e,1)
	StartTimer(e)
   else
    --Prompt ( " act=" .. g_Entity[e]['activated'] .. " timer=" .. GetTimer(e) )
    if g_Entity[e]['activated'] == 1 then
     if GetTimer(e)>3000 then
      CollisionOff(e)
      SetActivated(e,2)
     end
    end
   end
  end
 else
  if g_Entity[e]['activated'] == 2 then
   SetAnimation(1)
   PlayAnimation(e)
   g_Entity[e]['animating'] = 1
   SetActivated(e,3)
   PlaySound(e,1)
   StartTimer(e)
  end
 end
 if g_Entity[e]['activated'] == 3 then
  if GetTimer(e)>3000 then
   CollisionOn(e)
   SetActivated(e,0)
  end
 end
 --Prompt ( "activated = " .. g_Entity[e]['activated'] )
end
