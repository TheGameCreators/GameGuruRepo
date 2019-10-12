-- 
g_get_close_and_speak_mode = {}
g_get_close_and_speak_name = {}

function get_close_and_speak_init_name(e,name)
 -- initialize character
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 g_get_close_and_speak_mode[e] = 0
 g_get_close_and_speak_name[e] = name
end
function get_close_and_speak_main(e)
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = GetPlayerDistance(e)
 fGoDistance = AIGetEntityViewRange(AIObjNo)
 if fGoDistance < 400 then fGoDistance = 400 end
 fCloseDistance = 100
 if PlayerDist < fGoDistance and g_PlayerHealth > 0 then
  if PlayerDist < fCloseDistance+50 then
   RotateToPlayer(e)
   PromptLocalForVR(e,g_get_close_and_speak_name[e],2)
  else
   if g_get_close_and_speak_mode[e] == 0 then
    -- start moving
    g_get_close_and_speak_mode[e] = 1
    get_close_and_speak_setanim(e,1)
    LoopAnimation(e)
    tDistX = g_PlayerPosX - g_Entity[e]['x']
    tDistZ = g_PlayerPosZ - g_Entity[e]['z']
    tDA = math.atan2(tDistX,tDistZ)
    fX = g_PlayerPosX - (math.sin(tDA) * (fCloseDistance))
    fZ = g_PlayerPosZ - (math.cos(tDA) * (fCloseDistance))
    AIEntityGoToPosition(AIObjNo,g_PlayerPosX,g_PlayerPosY,g_PlayerPosZ)
    AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   end
  end  
 end
 if g_get_close_and_speak_mode[e] > 0 then
  if PlayerDist > fCloseDistance and AIGetEntityIsMoving(AIObjNo) == 1 then
   -- keep moving closer
   SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
   MoveForward(e,AIGetEntitySpeed(AIObjNo))
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))   
  else
   -- stop
   get_close_and_speak_setanim(e,4)
   LoopAnimation(e)
   AIEntityGoToPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   MoveForward(e,0.0)
   g_get_close_and_speak_mode[e] = 0
  end
 end
end
function get_close_and_speak_setanim(e,iAnimSlot)
 fStartFrame = GetEntityAnimStart(GetEntityElementBankIndex(e),iAnimSlot)
 fFinishFrame = GetEntityAnimFinish(GetEntityElementBankIndex(e),iAnimSlot)
 SetAnimationFrames(fStartFrame,fFinishFrame)
end
