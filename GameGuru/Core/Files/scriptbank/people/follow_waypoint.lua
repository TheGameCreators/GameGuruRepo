-- 
g_follow_waypoint_mode = {}
g_follow_waypoint_pathindex = {}
g_follow_waypoint_pathpointindex = {}
g_follow_waypoint_pathpointdirection = {}

function follow_waypoint_init(e)
 -- initialize character
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 g_follow_waypoint_mode[e] = -1
 g_follow_waypoint_pathindex[e] = 0
 g_follow_waypoint_pathpointindex[e] = 0
 g_follow_waypoint_pathpointdirection[e] = 0
end
function follow_waypoint_main(e)
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = GetPlayerDistance(e)
 fGoDistance = AIGetEntityViewRange(AIObjNo)
 if fGoDistance < 400 then fGoDistance = 400 end
 if PlayerDist < fGoDistance and g_PlayerHealth > 0 then
  if g_follow_waypoint_mode[e] == -1 then
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
   g_follow_waypoint_pathindex[e] = PathIndex
   g_follow_waypoint_pathpointindex[e] = PointIndex
   g_follow_waypoint_pathpointdirection[e] = 0
   -- start moving to point within waypoint
   g_follow_waypoint_mode[e] = 2
   follow_waypoint_setanim(e,1)
   LoopAnimation(e)
  end
 end
 if g_follow_waypoint_mode[e] == 2 then
  -- go to the point
  patrolx = AIPathGetPointX(g_follow_waypoint_pathindex[e],g_follow_waypoint_pathpointindex[e])
  patroly = AIPathGetPointY(g_follow_waypoint_pathindex[e],g_follow_waypoint_pathpointindex[e])
  patrolz = AIPathGetPointZ(g_follow_waypoint_pathindex[e],g_follow_waypoint_pathpointindex[e])
  AIEntityGoToPosition(AIObjNo,patrolx,patroly,patrolz)
  AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  g_follow_waypoint_mode[e] = 1
 end
 if g_follow_waypoint_mode[e] == 1 then
  pDX = g_Entity[e]['x'] - AIPathGetPointX(g_follow_waypoint_pathindex[e],g_follow_waypoint_pathpointindex[e])
  pDZ = g_Entity[e]['z'] - AIPathGetPointZ(g_follow_waypoint_pathindex[e],g_follow_waypoint_pathpointindex[e])
  if math.abs(pDX)+math.abs(pDZ) < 50 then
   -- move to next point
   if g_follow_waypoint_pathpointdirection[e] == 0 then
	if g_follow_waypoint_pathpointindex[e] >= AIGetPathCountPoints(g_follow_waypoint_pathindex[e]) then
	 g_follow_waypoint_pathpointdirection[e] = 1
	else
     g_follow_waypoint_pathpointindex[e] = g_follow_waypoint_pathpointindex[e] + 1
	end
   else
	if g_follow_waypoint_pathpointindex[e] <= 1 then
	 g_follow_waypoint_pathpointdirection[e] = 0
	else
     g_follow_waypoint_pathpointindex[e] = g_follow_waypoint_pathpointindex[e] - 1
	end
   end
   g_follow_waypoint_mode[e] = 2
  else
   -- keep moving
   SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
   MoveForward(e,AIGetEntitySpeed(AIObjNo))
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))   
  end
 end
end
function follow_waypoint_setanim(e,iAnimSlot)
 fStartFrame = GetEntityAnimStart(GetEntityElementBankIndex(e),iAnimSlot)
 fFinishFrame = GetEntityAnimFinish(GetEntityElementBankIndex(e),iAnimSlot)
 SetAnimationFrames(fStartFrame,fFinishFrame)
end
