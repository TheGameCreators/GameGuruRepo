-- LUA Script - melee attacker script for classic characters

-- CHAINSAWBRUTE Animframes:
-- 554,664   - roar - start chainsaw
-- 700,721   - run - run with chainsaw
-- 802,851   - attack (824)
-- 901,959   - attack (924-950)
-- 1020,1085 - hurt
-- 1202,1268 - fall to knees
-- 1269,1300 - knees idle
-- 1400,1435 - got chainsaw - idle
-- 1436,1656 - got chainsaw - chop head off
-- 1657,1757 - fall to floor
-- 210,232   - idle

attackdistance = {}
attackstart = {}
attackend = {}
roarstart = {}
roarend = {}
hurtstart = {}
hurtend = {}
viewrange = {}
damageframestart = {}
damageframeend = {}
chargemode = {}
attacktype = {}
lastroar = {}
lastswipe = {}
chainsaw_stage = {}
local gs = {}

function ai_classiccharacter_chainsawbrute_init(e)
 CharacterControlLimbo(e)
 LoadGlobalSound("audiobank\\Classics\\chainsaw\\chainsaw-cutting.wav",gs[e])
 ai_soldier_state[e] = "patrol"
 ai_soldier_pathindex[e] = -1
 SetAnimationFrames(211,232) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,0.7)
 roarstart[e]=554
 roarend[e]=664
 hurtstart[e]=1020
 hurtend[e]=1085
 attackdistance[e]=75
 attackstart[e]=802
 attackend[e]=851
 damageframestart[e]=824
 damageframeend[e]=829
 lastroar[e] = 0
 lastswipe[e] = 0
 ai_old_health[e] = -1
 chainsaw_stage[e] = 0
 SetCharacterSoundSet(e)
end

function ai_classiccharacter_chainsawbrute_main(e)
 PlayerDist = GetPlayerDistance(e)
 EntObjNo = g_Entity[e]['obj']
 if viewrange[e]==nil then
  viewrange[e] = AIGetEntityViewRange(EntObjNo)
 end
 if ai_soldier_state[e] == "patrol" then
  if ai_soldier_pathindex[e] == -1 then
   ai_soldier_pathindex[e] = -2
   CharacterControlUnarmed(e)
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
	ModulateSpeed(e,1.0)
	SetCharacterToWalk(e)
	ai_path_point_direction[e] = 1
	ai_path_point_max[e] = AIGetPathCountPoints(ai_soldier_pathindex[e])
   end   
  end
  if ai_soldier_pathindex[e] > -1 then 
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
  if GetPlayerDistance(e)<attackdistance[e] and g_PlayerHealth>0 then
   if ai_soldier_state[e] ~= "attack" then
    AIEntityStop(EntObjNo)
    ai_soldier_state[e] = "attack"
    CharacterControlLimbo(e)
    SetAnimationSpeed(e,1.0)
	attacktype[e] = math.random(1,2)
	if attacktype[e] == 1 then
	 attackstart[e]=802
	 attackend[e]=851
	 damageframestart[e]=824
	 damageframeend[e]=829
	end
	if attacktype[e] == 2 then
 	 if GetGlobalSoundPlaying(gs[e]) == 0 then 
	  PlayGlobalSound(gs[e])
	  SetGlobalSoundVolume(gs[e],100-(GetPlayerDistance(e)/75))
	 end		
     attackstart[e]=901
     attackend[e]=959
     damageframestart[e]=924
     damageframeend[e]=950
	end
    SetAnimationFrames(attackstart[e],attackend[e]) 
    PlayAnimation(e)
	if attacktype[e] == 2 then 
 	 SetAnimationSpeed(e,0.3)
	else 
	 SetSoundSpeed(50000)
	 LoopSound(e,1)
	 SetSoundVolume(90-(GetPlayerDistance(e)/75))
	end 
	lastswipe[e] = 0
   else
    if lastswipe[e]==0 and GetAnimationFrame(e)<attackend[e]-1 then
	 lastswipe[e]=1
	end
    if lastswipe[e]==1 and GetAnimationFrame(e)>=attackend[e]-1 then
     ai_soldier_state[e] = "patrol"
 	 StopAnimation(e)
	 SetSoundSpeed(44000)
	 StopGlobalSound(gs[e])
	 lastswipe[e]=2
    end    
   end
  else
   if ai_soldier_state[e] == "attack" then
    if GetAnimationFrame(e)<attackstart[e] or GetAnimationFrame(e)>=attackend[e]-1 then
	 ai_soldier_state[e] = "patrol"
	 SetAnimationSpeed(e,0.7)
	 StopAnimation(e)
	 SetSoundSpeed(44000)
	 LoopSound(e,1)
	 SetSoundVolume(90-(GetPlayerDistance(e)/75))
    end
   else
    if ai_soldier_state[e] ~= "charge" then
 	 if lastroar[e]==0 or lastroar[e] > g_Time then 
	  if lastroar[e] == 0 then lastroar[e] = g_Time + 1000 end
      AIEntityGoToPosition(EntObjNo,g_PlayerPosX,g_PlayerPosZ)
	 else
	  if lastroar[e] > 0 then
       AIEntityStop(EntObjNo)
       ai_soldier_state[e] = "roar"
       CharacterControlLimbo(e)
       SetAnimationSpeed(e,1.0)
       SetAnimationFrames(roarstart[e],roarend[e]) 
       PlayAnimation(e) 
	   SetAnimationSpeed(e,1.0)
 	   RotateToPlayer(e) 
	   lastroar[e] = -1
	  end
	  if ai_soldier_state[e] == "hurtanim" then
	   if GetAnimationFrame(e) > hurtend[e]-2 then
        ai_soldier_state[e] = "dazedreadytochargenext"
       end
	  end
	  if ai_soldier_state[e] ~= "hurtanim" then
       if chainsaw_stage[e] == 0 and GetAnimationFrame(e) >= 587 then
	    chainsaw_stage[e] = 1
        PlayCharacterSound(e,"onAggro")
	   end
       if chainsaw_stage[e] == 1 and GetAnimationFrame(e) >= 631 then
	    chainsaw_stage[e] = 2
        PlayCharacterSound(e,"onAggro")
	   end
       if chainsaw_stage[e] == 2 and GetAnimationFrame(e) >= 656 then
	    chainsaw_stage[e] = 3
		SetSoundSpeed(44000)
   	    LoopSound(e,1)
		SetSoundVolume(90-(GetPlayerDistance(e)/75))
	   end
	   if GetAnimationFrame(e) > roarend[e]-2 or lastroar[e] == -2 then
        SetAnimationSpeed(e,1.2)
        ai_soldier_state[e] = "charge"
        CharacterControlUnarmed(e)
        SetCharacterToRun(e)
        ModulateSpeed(e,1.4)
	    lastroar[e] = -2
	   end
	  end
     end
	end
   end
  end
  if ai_soldier_state[e] == "charge" then
   CharacterControlUnarmed(e)
   RotateToPlayer(e)  
   AIEntityGoToPosition(EntObjNo,g_PlayerPosX,g_PlayerPosZ) 
  end
  if ai_soldier_state[e] == "attack" then
   RotateToPlayer(e)  
   if GetAnimationFrame(e)>damageframestart[e] and GetAnimationFrame(e)<damageframeend[e] then
    if GetPlayerDistance(e)<attackdistance[e]+40 then
	 if attacktype[e] == 2 then 
	  StopSound(e,1)
 	 end
     HurtPlayer(e,1)
	else 
	 if attacktype[e] == 2 then 
	  StopGlobalSound(gs[e])
 	 end
    end
   end
  end
 end 
 if ai_old_health[e]==-1 then
  ai_old_health[e] = g_Entity[e]['health']
 end
 if g_Entity[e]['health'] < ai_old_health[e] then
  ai_old_health[e] = g_Entity[e]['health']
  AIEntityStop(EntObjNo)
  ai_soldier_state[e] = "hurtanim"
  CharacterControlLimbo(e)
  SetAnimationSpeed(e,0.7)
  SetAnimationFrames(hurtstart[e],hurtend[e]) 
  PlayAnimation(e) 
  PlayCharacterSound(e,"onHurt")
 end
 if PlayerDist >= viewrange[e] and ai_soldier_state[e]~="patrol" then
  ai_soldier_state[e] = "patrol"
  CharacterControlUnarmed(e)
  SetCharacterToWalk(e)
  ModulateSpeed(e,1.0)
 end 
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
 SetSoundVolume(95-(GetPlayerDistance(e)/75))
 if g_PlayerHealth < 1 then 
  ai_classiccharacter_chainsawbrute_init(e)
 end
end

function ai_classiccharacter_chainsawbrute_exit(e)
 PlayCharacterSound(e,"onDeath")
 StopGlobalSound(gs[e])
 CollisionOff(e)
 StopSound(e,1)
end
