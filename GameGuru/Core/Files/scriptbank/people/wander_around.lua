-- 
g_wander_around_mode = {}
g_wander_around_basex = {}
g_wander_around_basez = {}

function wander_around_init(e)
 -- initialize character
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 g_wander_around_mode[e] = 0
 g_wander_around_basex[e] = g_Entity[e]['x']
 g_wander_around_basez[e] = g_Entity[e]['z']
end
function wander_around_main(e)
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = GetPlayerDistance(e)
 fGoDistance = AIGetEntityViewRange(AIObjNo) * 2
 if fGoDistance > 1000 then fGoDistance = 1000 end
 if PlayerDist < fGoDistance and g_PlayerHealth > 0 then
  if g_wander_around_mode[e] == 0 then
   -- start moving
   g_wander_around_mode[e] = 1
   wander_around_setanim(e,1)
   LoopAnimation(e)
   tDA = math.random(6.28)
   fX = g_wander_around_basex[e] - (math.sin(tDA) * (100+math.random(fGoDistance)))
   fZ = g_wander_around_basez[e] - (math.cos(tDA) * (100+math.random(fGoDistance)))
   AIEntityGoToPosition(AIObjNo,fX,GetEntityPositionY(e),fZ)
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  end
 end
 if g_wander_around_mode[e] > 0 then
  if PlayerDist < fGoDistance then
   -- keep moving
   SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
   if AIGetEntityIsMoving(AIObjNo) == 1 then
    MoveForward(e,AIGetEntitySpeed(AIObjNo))
   end
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))   
  end
  if PlayerDist >= fGoDistance or AIGetEntityIsMoving(AIObjNo) == 0 then
   -- stop
   g_wander_around_mode[e] = 0
   wander_around_setanim(e,0)
   LoopAnimation(e)
   AIEntityGoToPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   MoveForward(e,0.0)
  end
 end
end
function wander_around_setanim(e,iAnimSlot)
 fStartFrame = GetEntityAnimStart(GetEntityElementBankIndex(e),iAnimSlot)
 fFinishFrame = GetEntityAnimFinish(GetEntityElementBankIndex(e),iAnimSlot)
 SetAnimationFrames(fStartFrame,fFinishFrame)
end
