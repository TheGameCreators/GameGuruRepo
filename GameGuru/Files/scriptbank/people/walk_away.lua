-- 
g_walk_away_mode = {}

function walk_away_init(e)
 -- initialize character
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 g_walk_away_mode[e] = 0
end
function walk_away_main(e)
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = GetPlayerDistance(e)
 fGoDistance = AIGetEntityViewRange(AIObjNo)
 if fGoDistance < 400 then fGoDistance = 400 end
 fTooCloseDistance = fGoDistance / 4
 if PlayerDist < fTooCloseDistance and g_PlayerHealth > 0 then
  if g_walk_away_mode[e] == 0 or g_walk_away_mode[e] == 2 then
   -- start moving
   g_walk_away_mode[e] = 1
   walk_away_setanim(e,1)
   LoopAnimation(e)
   tDistX = g_PlayerPosX - g_Entity[e]['x']
   tDistZ = g_PlayerPosZ - g_Entity[e]['z']
   tDA = math.atan2(tDistX,tDistZ)
   fX = g_PlayerPosX - (math.sin(tDA) * (100+fGoDistance))
   fZ = g_PlayerPosZ - (math.cos(tDA) * (100+fGoDistance))
   AIEntityGoToPosition(AIObjNo,fX,GetEntityPositionY(e),fZ)
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  end
 end
 if g_walk_away_mode[e] > 0 then
  if PlayerDist < fGoDistance then
   -- keep moving
   SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
   if AIGetEntityIsMoving(AIObjNo) == 1 then
    MoveForward(e,AIGetEntitySpeed(AIObjNo))
   else
    g_walk_away_mode[e] = 2
    MoveForward(e,0.0)
   end
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))   
   -- when out of character range, allow change of direction if player gets close again (above)
   if PlayerDist > fTooCloseDistance then g_walk_away_mode[e] = 2 end
  end
  if PlayerDist >= fGoDistance or AIGetEntityIsMoving(AIObjNo) == 0 then
   -- stop
   g_walk_away_mode[e] = 0
   walk_away_setanim(e,0)
   LoopAnimation(e)
   AIEntityGoToPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   MoveForward(e,0.0)
  end
 end
end
function walk_away_setanim(e,iAnimSlot)
 fStartFrame = GetEntityAnimStart(GetEntityElementBankIndex(e),iAnimSlot)
 fFinishFrame = GetEntityAnimFinish(GetEntityElementBankIndex(e),iAnimSlot)
 SetAnimationFrames(fStartFrame,fFinishFrame)
end
