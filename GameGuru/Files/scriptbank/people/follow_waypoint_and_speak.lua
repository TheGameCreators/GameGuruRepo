-- 
g_follow_waypoint_and_speak_mode = {}
g_follow_waypoint_and_speak_pathindex = {}
g_follow_waypoint_and_speak_pathpointindex = {}
g_follow_waypoint_and_speak_pathpointdirection = {}
g_follow_waypoint_and_speak_name = {}

function follow_waypoint_and_speak_init_name(e,name)
 -- initialize character
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 g_follow_waypoint_and_speak_mode[e] = -1
 g_follow_waypoint_and_speak_pathindex[e] = 0
 g_follow_waypoint_and_speak_pathpointindex[e] = 0
 g_follow_waypoint_and_speak_pathpointdirection[e] = 0
 g_follow_waypoint_and_speak_name[e] = name
end
function follow_waypoint_and_speak_main(e)
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = GetPlayerDistance(e)
 fGoDistance = AIGetEntityViewRange(AIObjNo)
 if fGoDistance < 400 then fGoDistance = 400 end
 fSpeakDistance = 200
 if PlayerDist < fGoDistance and g_PlayerHealth > 0 then
  if g_follow_waypoint_and_speak_mode[e] == -1 then
   -- find closest waypoint
   PathIndex = -1
   PointIndex = 2
   pClosest = 99999
   for pa = 1, AIGetTotalPaths(), 1 do
    for po = 1, AIGetPathCountPoints(pa), 1 do
     pDX = g_Entity[e]['x'] - AIPathGetPointX(pa,po)
     pDY = g_Entity[e]['y'] - AIPathGetPointY(pa,po)
     pDZ = g_Entity[e]['z'] - AIPathGetPointZ(pa,po)
     pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDY*pDY)+math.abs(pDZ*pDZ));
     if pDist < pClosest and pDist < 200 then
      pClosest = pDist
      PathIndex = pa
      PointIndex = po
     end
    end
   end
   g_follow_waypoint_and_speak_pathindex[e] = PathIndex
   g_follow_waypoint_and_speak_pathpointindex[e] = PointIndex
   g_follow_waypoint_and_speak_pathpointdirection[e] = 0
   -- start moving to point within waypoint
   g_follow_waypoint_and_speak_mode[e] = 2
   follow_waypoint_and_speak_setanim(e,1)
   LoopAnimation(e)
  end
 end
 if g_follow_waypoint_and_speak_mode[e] == 2 then
  -- go to the point
  patrolx = AIPathGetPointX(g_follow_waypoint_and_speak_pathindex[e],g_follow_waypoint_and_speak_pathpointindex[e])
  patroly = AIPathGetPointY(g_follow_waypoint_and_speak_pathindex[e],g_follow_waypoint_and_speak_pathpointindex[e])
  patrolz = AIPathGetPointZ(g_follow_waypoint_and_speak_pathindex[e],g_follow_waypoint_and_speak_pathpointindex[e])
  AIEntityGoToPosition(AIObjNo,patrolx,patroly,patrolz)
  AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  g_follow_waypoint_and_speak_mode[e] = 1
 end
 if g_follow_waypoint_and_speak_mode[e] == 1 then
  pDX = g_Entity[e]['x'] - AIPathGetPointX(g_follow_waypoint_and_speak_pathindex[e],g_follow_waypoint_and_speak_pathpointindex[e])
  pDZ = g_Entity[e]['z'] - AIPathGetPointZ(g_follow_waypoint_and_speak_pathindex[e],g_follow_waypoint_and_speak_pathpointindex[e])
  if math.abs(pDX)+math.abs(pDZ) < 50 then
   -- move to next point
   if g_follow_waypoint_and_speak_pathpointindex[e] >= AIGetPathCountPoints(g_follow_waypoint_and_speak_pathindex[e]) then
    -- stop and trigger speak
    follow_waypoint_and_speak_setanim(e,4)
    LoopAnimation(e)
    AIEntityGoToPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
    AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
    MoveForward(e,0.0)
    g_follow_waypoint_and_speak_mode[e] = 3
   else
    g_follow_waypoint_and_speak_pathpointindex[e] = g_follow_waypoint_and_speak_pathpointindex[e] + 1
    g_follow_waypoint_and_speak_mode[e] = 2
   end
  else
   -- keep moving
   SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
   MoveForward(e,AIGetEntitySpeed(AIObjNo))
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))   
  end
 end
 if g_follow_waypoint_and_speak_mode[e] == 3 then
  if PlayerDist < fSpeakDistance and g_PlayerHealth > 0 then
   RotateToPlayer(e)
   PromptLocalForVR(e,g_follow_waypoint_and_speak_name[e],2)
  end
 end
end
function follow_waypoint_and_speak_setanim(e,iAnimSlot)
 fStartFrame = GetEntityAnimStart(GetEntityElementBankIndex(e),iAnimSlot)
 fFinishFrame = GetEntityAnimFinish(GetEntityElementBankIndex(e),iAnimSlot)
 SetAnimationFrames(fStartFrame,fFinishFrame)
end
