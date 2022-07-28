-- LUA Script - precede every function and global member with lowercase name of script
function guard_init(e)
 CharacterControlLimbo(e)
 SetAnimationSpeed(e,1)
 SetAnimationFrames(3000,3100)
 LoopAnimation(e)
end
function guard_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 then
  RotateToPlayer(e)
  if g_Entity[e]['activated']==0 then
   SetActivated(e,1)
   PlaySound(e,1)
   SetAnimationSpeed(e,1.0)
   SetAnimationFrames(3380,3420)
   PlayAnimation(e)
  end 
 end 
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
end
