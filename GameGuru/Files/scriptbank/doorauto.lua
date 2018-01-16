-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function doorauto_init(e)
end
function doorauto_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 100 and g_PlayerHealth > 0 then
  GetEntityPlayerVisibility(e)
  if g_Entity[e]['plrvisible'] == 1 then
   if g_Entity[e]['activated'] == 0 then
    SetActivated(e,1)
	SetAnimation(0)
	PlayAnimation(e)
	g_Entity[e]['animating'] = 1
	ActivateIfUsed(e)
    PlaySound(e,0)
	StartTimer(e)
   end
  end
  if g_Entity[e]['activated'] == 1 then
   if GetTimer(e)>1000 then
 	CollisionOff(e)
    SetActivated(e,2)
   end
  end
 else
  if g_Entity[e]['activated'] == 2 then
   SetAnimation(1)
   PlayAnimation(e)
   g_Entity[e]['animating'] = 1
   SetActivated(e,0)
   PlaySound(e,1)
   CollisionOn(e)
  end
 end
end
