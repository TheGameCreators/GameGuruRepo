-- LUA Script - precede every function and global member with lowercase name of script
spaceman_onetimespeech = 0
spaceman_frm = 0
uselander_endsequence = 0

function spaceman_init(e)
 CharacterControlLimbo(e)
 SetAnimationSpeed(e,1)
 SetAnimationFrames(3000,3100)
 LoopAnimation(e)
end
function spaceman_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 200 then
  RotateToPlayer(e)
  if spaceman_onetimespeech==0 then
   spaceman_onetimespeech = 1
   PlaySound(e,1)
   SetAnimationSpeed(e,0.9)
   SetAnimationFrames(3110,3420)
   PlayAnimation(e)
  end 
 end 
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
 if uselander_endsequence > 0 then
  Destroy(e)
 end
end
