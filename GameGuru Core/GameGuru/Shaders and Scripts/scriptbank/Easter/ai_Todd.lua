-- LUA Script - precede every function and global member with lowercase name of script

function ai_Todd_init(e)
 CharacterControlUnarmed(e)
end

function ai_Todd_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 200 then
  LookAtPlayer(e)
 end 
 
function ai_Todd_exit(e)
PlayCharacterSound(e,"onDeath")
ai_soldier_state[e] = "dead"
ModulateSpeed(e,1.0)
CharacterControlLimbo(e)
SetAnimationSpeed(e,1.0) 
SetAnimationFrames(808,852) 
PlayAnimation(e)
CollisionOff(e)
end
end
