-- LUA Script - precede every function and global member with lowercase name of script
guard_chat_talktoalien = 0

function guard_chat_init(e)
 CharacterControlLimbo(e)
 SetAnimationSpeed(e,1)
 SetAnimationFrames(3000,3100)
 LoopAnimation(e)
end
function guard_chat_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 then
  RotateToPlayer(e)
  if g_Entity[e]['activated']==0 then
   if guard_chat_talktoalien==0 then
    Prompt ( "Press E to speak with guard" )
   else
    Prompt ( "Press E to tell guard about the temple alien" )
   end
   if g_KeyPressE==1 then
    SetActivated(e,1)
    if guard_chat_talktoalien==0 then
     PlaySound(e,0)
     SetAnimationSpeed(e,1.25)
     SetAnimationFrames(4360,4464)
     PlayAnimation(e)
	else
 	 ActivateIfUsed(e)
     guard_chat_talktoalien = 2
	 SetActivated(e,2)
	 CharacterControlUnarmed(e)
	 EntObjNo = g_Entity[e]['obj']
	 AIEntityGoToPosition(EntObjNo,g_Entity[e]['x'],g_Entity[e]['z']+200)
	end
   end
  end 
 end 
 if g_Entity[e]['activated']==1 then
  if GetAnimationFrame(e)>4460 then
   SetActivated(e,0)
   SetAnimationSpeed(e,1)
   SetAnimationFrames(3000,3100)
   LoopAnimation(e)
  end
 end
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
end
