-- LUA Script - precede every function and global member with lowercase name of script + '_main'

-- global array to hold frame value of door
doorautolegacy = {}

function doorautolegacy_init(e)
 -- set entity for manual frame control
 CharacterControlLimbo(e)
 doorautolegacy[e] = 0
end

function doorautolegacy_main(e)
 -- detect player proximity
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 100 and g_PlayerHealth > 0 then
  -- only if door visible to player
  GetEntityPlayerVisibility(e)
  if g_Entity[e]['plrvisible'] == 1 then
   -- trigger door to open
   if g_Entity[e]['activated'] == 0 then
	doorautolegacy[e] = 0
	SetAnimationFrame(e,doorautolegacy[e])
    SetActivated(e,1)
	ActivateIfUsed(e)
    PlaySound(e,0)
   end
  end
 else
  -- when out of range, trigger door to close
  if g_Entity[e]['activated'] == 2 then
   SetActivated(e,3)
   PlaySound(e,1)
   CollisionOn(e)
  end
 end
 -- handle door opening frame animation
 if g_Entity[e]['activated'] == 1 then
  doorautolegacy[e] = doorautolegacy[e] + 0.1
  if GetAnimationFrame(e)>GetEntityAnimationFinish(e,0) then
   doorautolegacy[e] = GetEntityAnimationFinish(e,0)
   CollisionOff(e)
   SetActivated(e,2)
  end
  SetAnimationFrame(e,doorautolegacy[e])
 end
 -- handle door closing frame animation
 if g_Entity[e]['activated'] == 3 then
  doorautolegacy[e] = doorautolegacy[e] - 0.1
  if GetAnimationFrame(e)<GetEntityAnimationStart(e,0) then
   doorautolegacy[e] = GetEntityAnimationStart(e,0)
   SetActivated(e,0)
  end
  SetAnimationFrame(e,doorautolegacy[e])
 end
end
