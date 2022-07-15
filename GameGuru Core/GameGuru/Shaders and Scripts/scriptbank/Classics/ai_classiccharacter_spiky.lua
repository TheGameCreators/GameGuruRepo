-- LUA Script - melee attacker script for classic characters

-- SPIKY Animframes:
-- 2530,2670 - IDLE
-- 2700,2757 - WALK
-- 2830,2851 - RUN
-- 2960,3026 - BITE 1 (2987)
-- 3060,3126 - BITE 2 (3097)
-- 3160,3210 - RAGE/SPIKES
-- 3204,3212 - HURT
-- 3303,3376 - GETUP FRONT
-- 3410,3438 - IMPACT FRONT/FALLDOWN
-- 3303,3303 - LYING ON BACK IDLE
-- 3303,3376 - CREEPY RISE/GETUP
-- 3600,3640 - SPIKES

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
dyingstart = {}
dyingend = {}

function ai_classiccharacter_spiky_init(e)
 CharacterControlLimbo(e)
 ai_soldier_state[e] = "patrol"
 ai_soldier_pathindex[e] = -1
 SetAnimationFrames(2,79) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
 roarstart[e]=3160
 roarend[e]=3210
 hurtstart[e]=3204
 hurtend[e]=3212
 attackdistance[e]=85
 attackstart[e]=2960
 attackend[e]=3026
 damageframestart[e]=2987
 damageframeend[e]=2991
 lastroar[e] = 0
 lastswipe[e] = 0
 ai_old_health[e] = -1
 SetCharacterSoundSet(e)
 dyingstart[e]=3410
 dyingend[e]=3438
 SetPreExitValue(e,0)
end

function ai_classiccharacter_spiky_main(e)

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
	attacktype[e] = math.random(1,2)
	if attacktype[e] == 1 then
	 attackstart[e]=2960
	 attackend[e]=3026
	 damageframestart[e]=2987
	 damageframeend[e]=2991
	end
	if attacktype[e] == 2 then
     attackstart[e]=3060
     attackend[e]=3126
     damageframestart[e]=3087
     damageframeend[e]=3092
    end
	SetAnimationSpeed(e,1.0)
    SetAnimationFrames(attackstart[e],attackend[e]) 
    PlayAnimation(e)
	lastswipe[e] = 0
   else
    if lastswipe[e]==0 and GetAnimationFrame(e)<attackend[e]-1 then
	 lastswipe[e]=1
	end
    if lastswipe[e]==1 and GetAnimationFrame(e)>=attackend[e]-1 then
     ai_soldier_state[e] = "patrol"
 	 StopAnimation(e)
	 lastswipe[e]=2
    end    
   end
  else
   if ai_soldier_state[e] == "attack" then
    if GetAnimationFrame(e)<attackstart[e] or GetAnimationFrame(e)>=attackend[e]-1 then
     ai_soldier_state[e] = "patrol"
 	 StopAnimation(e)
    end
   else
    if ai_soldier_state[e] ~= "charge" then
 	 if lastroar[e]==0 or lastroar[e] > g_Time then 
	  if lastroar[e] == 0 then lastroar[e] = g_Time + 1000 end
      AIEntityGoToPosition(EntObjNo,g_PlayerPosX,g_PlayerPosZ)
	 else
	  if ai_soldier_state[e] == "hurtanim" then
	   if GetAnimationFrame(e) > hurtend[e]-2 then
        ai_soldier_state[e] = "dazedreadytochargenext"
       end
	  end
	  if ai_soldier_state[e] ~= "hurtanim" then
	   if (GetAnimationFrame(e) > roarend[e]-2 and GetAnimationFrame(e) < roarend[e]+20) or lastroar[e] == -2 then
        ai_soldier_state[e] = "charge"
        CharacterControlUnarmed(e)
   	    chargemode[e] = math.random(1,4)
	    if chargemode[e]==1 then
         SetCharacterToWalk(e)
         ModulateSpeed(e,1.0)
 		 SetAnimationSpeed(e,1.0)
	    else
         SetCharacterToRun(e)
         ModulateSpeed(e,1.5)
 		 SetAnimationSpeed(e,1.0)
	    end
	    lastroar[e] = -2
	   end
	  end
	  if lastroar[e] > 0 then
       AIEntityStop(EntObjNo)
       ai_soldier_state[e] = "roar"
       CharacterControlLimbo(e)
	   SetAnimationSpeed(e,1.0)
       SetAnimationFrames(roarstart[e],roarend[e]) 
       PlayAnimation(e) 
 	   RotateToPlayer(e) 
  	   PlaySound(e,1)
	   lastroar[e] = -1
	  end
     end
	end
   end
  end
  if ai_soldier_state[e] == "charge" then
   RotateToPlayer(e)  
   AIEntityGoToPosition(EntObjNo,g_PlayerPosX,g_PlayerPosZ) 
  end
  if ai_soldier_state[e] == "attack" then
   RotateToPlayer(e)  
   if GetAnimationFrame(e)>damageframestart[e] and GetAnimationFrame(e)<damageframeend[e] then
    if GetPlayerDistance(e)<attackdistance[e]+40 then
     HurtPlayer(e,1)
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
 
end

function ai_classiccharacter_spiky_dying(e)
 tDyingAnimStart = dyingstart[e]
 tDyingAnimFinish = dyingend[e]
 CharacterControlLimbo(e)
 StopAnimation(e)
 SetAnimationFrames(tDyingAnimStart,tDyingAnimFinish)
 PlayAnimation(e)
 SetAnimationFrame(e,tDyingAnimStart)
 SetAnimationSpeedModulation(e,1.0)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
end

function ai_classiccharacter_spiky_preexit(e)
 SetPreExitValue(e,1)
 tFrame = GetAnimationFrame(e)
 if dyingstart[e] > 0 and dyingend[e] > 0 then
  if ai_bot_state[e] ~= ai_state_preexit then
   ai_bot_state[e] = ai_state_preexit
   ai_classiccharacter_spiky_dying(e)
  end
 else 
  SetPreExitValue(e,2)
 end
 if tFrame >= dyingend[e]-2 then
  SetPreExitValue(e,2)
 end 
end

function ai_classiccharacter_spiky_exit(e)
 CollisionOff(e)
 PlayCharacterSound(e,"onDeath")
end
