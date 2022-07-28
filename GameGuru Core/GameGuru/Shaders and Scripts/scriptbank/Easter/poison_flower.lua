egg_status = {}

function poison_flower_init(e)
 egg_status[e] = 0 
end

function poison_flower_main(e)
 if egg_status[e] == 0 then
  if eggradar_addEnemy ~= nil then eggradar_addEnemy(e) end
  egg_status[e] = 1
 end
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist > 2000 then
 Hide(e)
 else
 Show(e)
 end
 if PlayerDist < 300 and g_Entity[e]['animating'] == 0 then
 g_Entity[e]['animating'] = 1
 SetAnimation(1)
 PlayAnimation(e)
 LoopAnimation(e)
 else if PlayerDist > 300 and g_Entity[e]['animating'] == 1  then
 g_Entity[e]['animating'] = 0
 SetAnimation(0)
 PlayAnimation(e)
 LoopAnimation(e)
 else if PlayerDist < 220 then
 HurtPlayer(e,1)
 end
 end
 end
end
 
function poison_flower_exit(e)
PlaySound(e,1)
PlayCharacterSound(e,"onDeath")
ActivateIfUsed(e)
SetAnimationSpeed(e,1.5) 
SetAnimationFrames(741,769) 
PlayAnimation(e)
end


