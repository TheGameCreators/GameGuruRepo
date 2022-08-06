-- LUA Script - precede every function and global member with lowercase name of script + '_main'
doorautolegacy = {}

function doorremote_init(e)
 doorautolegacy[e] = 0
end

function doorremote_main(e)
 if g_PlayerHealth > 0 and g_Entity[e]['activated'] == 1 then
   if g_Entity[e]['activated'] == 1 and GetAnimationFrame(e)<GetEntityAnimationFinish(e,0)  then
	doorautolegacy[e] = 0
	SetAnimationFrame(e,doorautolegacy[e])
    SetActivated(e,2)
	ActivateIfUsed(e)
    PlaySound(e,0)
   end
   if  g_Entity[e]['activated'] == 1 then
	if GetAnimationFrame(e) == GetEntityAnimationFinish(e,0)  then
 	 doorautolegacy[e] = GetEntityAnimationFinish(e,0)
	 SetAnimationFrame(e,doorautolegacy[e])
   	 SetActivated(e,4)
	 ActivateIfUsed(e)
   	 PlaySound(e,0)
	end
  end
 end
 if g_Entity[e]['activated'] == 2 then
  CollisionOff(e)
  doorautolegacy[e] = doorautolegacy[e] + 0.5
  if GetAnimationFrame(e)>GetEntityAnimationFinish(e,0) then
   doorautolegacy[e] = GetEntityAnimationFinish(e,0)
   CollisionOff(e)
   SetActivated(e,0)
  end
  SetAnimationFrame(e,doorautolegacy[e])
 end
 if g_Entity[e]['activated'] == 4 then
  doorautolegacy[e] = doorautolegacy[e] - 0.5
  CollisionOn(e)
  if GetAnimationFrame(e)<GetEntityAnimationStart(e,0) then
   doorautolegacy[e] = GetEntityAnimationStart(e,0)
   SetActivated(e,0)
  end
  SetAnimationFrame(e,doorautolegacy[e])
 end
end
