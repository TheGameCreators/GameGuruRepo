-- LUA Script - precede every function and global member with lowercase name of script

attackstart = {}
attackend = {}
viewrange = {}
damageframestart = {}
damageframeend = {}
lastroar = {}
lastswipe = {}

function ai_fantasycharacter_init(e)
 ai_soldier_state[e] = "patrol"
 ai_soldier_pathindex[e] = -1
 CharacterControlLimbo(e)
 SetAnimationFrames(236,265) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
 attackstart[e]=160
 attackend[e]=189
 damageframestart[e]=170
 damageframeend[e]=180
 lastroar[e] = 0
 lastswipe[e] = 0
 ai_old_health[e] = -1
 SetCharacterSoundSet(e)
end

function ai_fantasycharacter_main(e)
 PlayerDist = GetPlayerDistance(e)
 EntObjNo = g_Entity[e]['obj']
 if viewrange[e]==nil then
  viewrange[e] = AIGetEntityViewRange(EntObjNo)
 end
 if ai_soldier_state[e] == "patrol" then
  if ai_soldier_pathindex[e] == -1 then
   ai_soldier_pathindex[e] = -2
   PathIndex = -1
   PathPointIndex = -1
   pClosest = 99999
   for pa = 1, AIGetTotalPaths(), 1 do
    for po = 1 , AIGetPathCountPoints(pa), 1 do
     pDX = g_Entity[e]['x'] - AIPathGetPointX(pa,po)
     pDZ = g_Entity[e]['z'] - AIPathGetPointZ(pa,po)
     pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDZ*pDZ))
     if pDist < pClosest and pDist < 200 then
      pClosest = pDist
      PathIndex = pa
	  PathPointIndex = po
     end
    end -- po
   end -- pa
   if PathIndex > -1 then
    ai_soldier_pathindex[e] = PathIndex
	ai_path_point_index[e] = PathPointIndex
    CharacterControlArmed(e)
	SetCharacterToWalk(e)
	ModulateSpeed(e,0.65)
	ai_path_point_direction[e] = 1
	ai_path_point_max[e] = AIGetPathCountPoints(ai_soldier_pathindex[e])
   end   
  end
  if ai_soldier_pathindex[e] > -1 then 
   ai_patrol_x[e] = AIPathGetPointX(ai_soldier_pathindex[e],ai_path_point_index[e])
   ai_patrol_z[e] = AIPathGetPointZ(ai_soldier_pathindex[e],ai_path_point_index[e])
   AIEntityGoToPosition(EntObjNo,ai_patrol_x[e],ai_patrol_z[e])
   --AIEntityGoToPosition(EntObjNo,ai_patrol_x[e],g_PlayerPosY,ai_patrol_z[e])
   --SetRotation(e,0,AIGetEntityAngleY(EntObjNo),0)
   --MoveForward(e,AIGetEntitySpeed(EntObjNo)/2.0)
   --AISetEntityPosition(EntObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))   
   tDistX = g_Entity[e]['x'] - ai_patrol_x[e]
   tDistZ = g_Entity[e]['z'] - ai_patrol_z[e]
   DistFromPath = math.sqrt(math.abs(tDistX*tDistX)+math.abs(tDistZ*tDistZ))	
   if DistFromPath < 50 then
	if ai_path_point_direction[e] == 1 then
	 ai_path_point_index[e] = ai_path_point_index[e] + 1
	 if ( ai_path_point_index[e] > ai_path_point_max[e] ) then
	  ai_path_point_index[e] = ai_path_point_max[e] - 1
	  ai_path_point_direction[e] = 0
	 end
	else
	 ai_path_point_index[e] = ai_path_point_index[e] - 1
	 if ( ai_path_point_index[e] < 1 ) then
	  ai_path_point_index[e] = 2
	  ai_path_point_direction[e] = 1
	 end		
	end
   end
  end
 end
 if PlayerDist < viewrange[e] then
  if GetPlayerDistance(e)<100 then
   if ai_soldier_state[e] ~= "attack" then
    AIEntityStop(EntObjNo)
    ai_soldier_state[e] = "attack"
    CharacterControlLimbo(e)
    SetAnimationFrames(160,189) 
    LoopAnimation(e)
    lastswipe[e]=0
   end
  else
   if ai_soldier_state[e] ~= "charge" then
    ai_soldier_state[e] = "charge"
    CharacterControlArmed(e)
    SetCharacterToWalk(e)
    ModulateSpeed(e,1.5)
    RotateToPlayer(e)
   end
  end
  if ai_soldier_state[e] == "charge" then
   AIEntityGoToPosition(EntObjNo,g_PlayerPosX,g_PlayerPosZ)
   --AIEntityGoToPosition(EntObjNo,g_PlayerPosX,g_PlayerPosY,g_PlayerPosZ)
   --SetRotation(e,0,AIGetEntityAngleY(EntObjNo),0)
   --MoveForward(e,AIGetEntitySpeed(EntObjNo)/2.0)
   --AISetEntityPosition(EntObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  end
  if ai_soldier_state[e] == "attack" then
   RotateToPlayer(e)  
   if GetAnimationFrame(e)>damageframestart[e] and GetAnimationFrame(e)<damageframeend[e] then
    if GetPlayerDistance(e)<120 and lastswipe[e]==0 then
	 PlaySound(e,1)
     HurtPlayer(e,g_Entity[e]['health']/4)
	 lastswipe[e]=1
    end
   end
   if GetAnimationFrame(e)<damageframestart[e] then
    lastswipe[e]=0
   end
  end
 end
 if ai_old_health[e]==-1 then
  ai_old_health[e] = g_Entity[e]['health']
 end
 if g_Entity[e]['health'] < ai_old_health[e] then
  ai_old_health[e] = g_Entity[e]['health']
  PlayCharacterSound(e,"onHurt")
 end
 if PlayerDist >= viewrange[e] and ai_soldier_state[e]~="patrol" then
  ai_soldier_state[e] = "patrol"
  CharacterControlLimbo(e)
  SetAnimationFrames(236,265) 
  LoopAnimation(e)
  ModulateSpeed(e,1.0)
  SetAnimationSpeed(e,1.0)
 end
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0)
  ResetLimbHit(e)
 end
end

function ai_fantasycharacter_exit(e)
 PlayCharacterSound(e,"onDeath")
 CollisionOff(e)
end