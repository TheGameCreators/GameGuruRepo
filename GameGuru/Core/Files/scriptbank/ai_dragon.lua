-- AI : Dragon Behavior
function ai_dragon_init(e)
 ai_soldier_state[e] = "unaware"
 SetAnimationFrames(110,610) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
 SetCharacterSoundSet(e) 
end
function ai_dragon_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 400 then
  if ai_soldier_state[e] == "unaware" then
   ai_soldier_state[e] = "takeoff"
   SetAnimationFrames(1111,1141) 
   PlayAnimation(e)
  end
 else
  if ai_soldier_state[e] == "flying" then
   ai_soldier_state[e] = "landing"
   SetAnimationFrames(1204,1233) 
   PlayAnimation(e)
  end
 end
 if ai_soldier_state[e] == "takeoff" then
  if GetAnimationFrame(e) >= 1141 then
   ai_soldier_state[e] = "flying"
   SetAnimationFrames(1142,1172) 
   LoopAnimation(e)
  end
 end
 if ai_soldier_state[e] == "flying" then
  RotateToPlayerSlowly(e,10)
 end
 if ai_soldier_state[e] == "landing" then
  if GetAnimationFrame(e) >= 1233 then
   ai_soldier_state[e] = "unaware"
   SetAnimationFrames(110,610) 
   LoopAnimation(e)
  end
 end
end
