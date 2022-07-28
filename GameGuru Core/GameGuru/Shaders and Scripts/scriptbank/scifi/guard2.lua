-- LUA Script - precede every function and global member with lowercase name of script
function guard2_init(e)
 CharacterControlLimbo(e)
 SetAnimationSpeed(e,1)
 SetAnimationFrames(3000,3100)
 LoopAnimation(e)
end
function guard2_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 then
  RotateToPlayer(e)
  if g_Entity[e]['activated']==0 then
   SetActivated(e,1)
   PlaySound(e,1)
   SetAnimationSpeed(e,0.65)
   SetAnimationFrames(4540,4605)
   PlayAnimation(e)
  end 
 end 
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
end
