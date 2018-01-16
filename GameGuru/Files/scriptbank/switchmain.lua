-- LUA Script - precede every function and global member with lowercase name of script + '_main'
switchautolegacy = {}

function switchmain_init(e)
 switchautolegacy[e] = 0
end

function switchmain_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 100 and g_PlayerHealth > 0 then
  GetEntityPlayerVisibility(e)
  if g_Entity[e]['plrvisible'] == 1 and (g_Entity[e]['activated'] == 0 or g_Entity[e]['activated'] == 2) then
   if GetGamePlayerStateXBOX() == 1 then
    Prompt("Press Y button to use")
   else
    Prompt ("Press E to use")
   end
   if g_Entity[e]['activated'] == 0 and g_KeyPressE == 1 then
	switchautolegacy[e] = 0
	SetAnimationFrame(e,switchautolegacy[e])
    SetActivated(e,1)
	ActivateIfUsed(e)
    PlaySound(e,0)
   end
   if g_Entity[e]['activated'] == 2 and g_KeyPressE == 1 then
	switchautolegacy[e] = 28
	SetAnimationFrame(e,switchautolegacy[e])
    SetActivated(e,3)
	ActivateIfUsed(e)
    PlaySound(e,0)
   end
  end
 end
 if g_Entity[e]['activated'] == 1 then
  switchautolegacy[e] = switchautolegacy[e] + 0.5
  if GetAnimationFrame(e)>28 then
   switchautolegacy[e] = 28
   SetActivated(e,2)
  end
  SetAnimationFrame(e,switchautolegacy[e])
 end
 if g_Entity[e]['activated'] == 3 then
  switchautolegacy[e] = switchautolegacy[e] - 0.5
  if GetAnimationFrame(e)<1 then
   switchautolegacy[e] = 1
   SetActivated(e,0)
  end
  SetAnimationFrame(e,switchautolegacy[e])
 end
end
