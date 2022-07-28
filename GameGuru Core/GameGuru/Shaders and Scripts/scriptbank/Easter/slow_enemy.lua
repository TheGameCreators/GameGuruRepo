-- LUA Script --

defeated = 0
attackstart = {}
attackend = {}
damageframestart = {}
damageframeend = {}
lastroar = {}
lastswipe = {}
egg_status = {}

function slow_enemy_init(e)
ai_soldier_pathindex[e] = -1;
 ai_soldier_state[e] = "patrol";
 CharacterControlLimbo(e)
 SetAnimationFrames(0,199)
 LoopAnimation(e)
 attackstart[e]=256
 attackend[e]=314
 damageframestart[e]=256
 damageframeend[e]=314
 lastroar[e]=0
 lastswipe[e]=0
 egg_status[e] = 0 
end

function slow_enemy_main(e)
 if egg_status[e] == 0 then
  if eggradar_addEnemy ~= nil then eggradar_addEnemy(e) end
  egg_status[e] = 1
 end
 PlayerDist = GetPlayerDistance(e)
 EntObjNo = g_Entity[e]['obj']
 if (PlayerDist < 1500 and ai_soldier_state[e] == "idle" and g_Entity[e]['plrvisible'] == 1) or (PlayerDist < 100 and ai_soldier_state[e] == "idle") then
  RotateToPlayer(e)
  if GetTimer(e) > lastroar[e] then
   lastroar[e]=GetTimer(e)+5000
  end
 end
 if PlayerDist < 2000 and g_Entity[e]['plrvisible']==1 then
  RotateToPlayer(e)
  if PlayerDist < 65 and g_Entity[e]['plrvisible']==1 then
   if ai_soldier_state[e]~="attack" then
    SetAnimationSpeed(e,0.8)
    if GetAnimationFrame(e)<attackstart[e] or GetAnimationFrame(e)>attackend[e] then
     AIEntityStop(EntObjNo)
	 ai_soldier_state[e] = "attack"
     CharacterControlLimbo(e)
     SetAnimationFrames(attackstart[e],attackend[e])
     PlayAnimation(e)
	 g_Entity[e]['animating'] = 1
    end
   else
    if ai_soldier_state[e]=="attack" then
     if GetAnimationFrame(e)>damageframestart[e] and GetAnimationFrame(e)<damageframeend[e] then
      if GetPlayerDistance(e)<65 then
       if GetTimer(e) > lastswipe[e] then
        lastswipe[e]= GetTimer(e)+1000
        PlaySound(e,0)
        randomdamage=math.random(20,50)
        HurtPlayer(e,randomdamage)
       end
      end 
  	 end
     if g_Entity[e]['animating']==0 then
      ai_soldier_state[e] = "idle"
      CharacterControlLimbo(e)
      SetAnimationFrames(0,199)
      PlayAnimation(e)
      RotateToPlayer(e)
     end
    end
   end
  else 
   if ai_soldier_state[e]~="roam" then
    ai_soldier_state[e] = "roam"
	PlaySound(e,0)
    CharacterControlUnarmed(e)
    ModulateSpeed(e,1.0)
    SetAnimationSpeed(e,1.5)
    SetCharacterToWalk(e)
   end
   rndx=math.random(1,360)
   rndz=math.random(1,360)
   rndx2=math.sin(rndx)*30
   rndz2=math.cos(rndz)*30
   AIEntityGoToPosition(EntObjNo,g_PlayerPosX+rndx2,g_PlayerPosZ+rndz2) 
  end
 else
  if PlayerDist >= 1500 and ai_soldier_state[e] ~= "idle" then
   ai_soldier_state[e] = "idle"
   CharacterControlLimbo(e)
   SetAnimationFrames(0,199)
   LoopAnimation(e)
   ModulateSpeed(e,1.0)
   SetAnimationSpeed(e,1.0)
  end
 end
 if ai_soldier_state[e] == "patrol" then
 if ai_soldier_pathindex[e] == -1 then
   ai_soldier_pathindex[e] = -2
   CharacterControlArmed(e)
   PathIndex = -1;
   pClosest = 99999;
   for pa = 1, AIGetTotalPaths(), 1 do
    for po = 1 , AIGetPathCountPoints(pa), 1 do
     pDX = g_Entity[e]['x'] - AIPathGetPointX(pa,po);
     pDZ = g_Entity[e]['z'] - AIPathGetPointZ(pa,po);
     pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDZ*pDZ));
     if pDist < pClosest and pDist < 200 then
      pClosest = pDist;
      PathIndex = pa;
     end
    end 
   end 
   if PathIndex > -1 then
    ai_soldier_pathindex[e] = PathIndex;
	ai_path_point_index[e] = 2
	ModulateSpeed(e,1.0)
	SetCharacterToWalk(e)
	ai_path_point_direction[e] = 1
	ai_path_point_max[e] = AIGetPathCountPoints(ai_soldier_pathindex[e])
   end   
  end
if ai_returning_home[e] == 1 then
    tDistX = g_Entity[e]['x'] - ai_start_x[e];
    tDistZ = g_Entity[e]['z'] - ai_start_z[e];
	DistToStart = math.sqrt(math.abs(tDistX*tDistX)+math.abs(tDistZ*tDistZ))
	if DistToStart < 200 or GetTimer(e) > ai_combat_state_delay[e] then
		ai_soldier_pathindex[e] = -1
		SetCharacterToWalk(e)
		CharacterControlUnarmed(e)
		AIEntityStop(EntObjNo);
		ModulateSpeed(e,1.0)
		ai_returning_home[e] = 0
		ai_soldier_state[e] = "alerted"
		ai_alerted_mode[e] = 0
	else
		AIEntityGoToPosition(EntObjNo,ai_start_x[e],ai_start_z[e])
	end
	
	elseif ai_soldier_pathindex[e] > -1 then 

	ai_patrol_x[e] = AIPathGetPointX(ai_soldier_pathindex[e],ai_path_point_index[e])
	ai_patrol_z[e] = AIPathGetPointZ(ai_soldier_pathindex[e],ai_path_point_index[e])
	AIEntityGoToPosition(EntObjNo,ai_patrol_x[e],ai_patrol_z[e])
    tDistX = g_Entity[e]['x'] - ai_patrol_x[e]
    tDistZ = g_Entity[e]['z'] - ai_patrol_z[e]
	DistFromPath = math.sqrt(math.abs(tDistX*tDistX)+math.abs(tDistZ*tDistZ))	
	
	if DistFromPath < 50 then
		if ai_path_point_direction[e] == 1 then
			ai_path_point_index[e] = ai_path_point_index[e] + 1
			if ( ai_path_point_index[e] > ai_path_point_max[e] ) then
				ai_path_point_index[e] = ai_path_point_max[e] -1
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
end

function slow_enemy_exit(e)
PlaySound(e,1)
ai_soldier_state[e] = "dead"
defeated=defeated+1
ActivateIfUsed(e)
ModulateSpeed(e,1.0)
CharacterControlLimbo(e)
SetAnimationSpeed(e,1.0) 
SetAnimationFrames(316,360) 
PlayAnimation(e)
CollisionOff(e)
end
