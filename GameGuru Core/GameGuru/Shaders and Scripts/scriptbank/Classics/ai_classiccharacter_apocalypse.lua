-- LUA Script - melee attacker script for classic characters

-- APOCALYPSE Animframes:
-- 8,28      - RUN
-- 38,98     - IDLE 1
-- 105,164   - IDLE 2
-- 172,231   - IDLE 3
-- 239,298   - IDLE 4
-- 304,392   - DOUBLE ARM SLASH (330) (350)
-- 407,459   - SINGLE ARM SLASH (434)
-- 473,536   - OVERHEAD SLASH LEFT (503)
-- 551,615   - OVERHEAD SLASH RIGHT (581)
-- 629,687   - RAGE 1
-- 701,760   - RAGE 2
-- 740,751   - HURT
-- 773,849   - BOUNCE FALL
-- 869,932   - FACEPLANT
-- 946,1001  - FALLBACK
-- 1030,1101 - FALLSIDE
-- 1107,1137 - WALK 
-- 1158,1188 - WALK LEAN BACK
-- 1208,1238 - WALK ARMS OUT
-- 1259,1306 - MARCH
-- 1334,1355 - RUN 1
-- 1362,1383 - RUN 2
-- 1394,1464 - BODY HIT 1
-- 1504,1595 - BODY HIT 2
-- 1610,1658 - BODY HIT 3
-- 1685,1757 - HEADSHOT 1
-- 1780,1883 - HEADSHOT 2
-- 1900,2011 - HEADSHOT 3
-- 2020,2148 - LEFT ARM HIT 1
-- 2215,2291 - LEFT ARM HIT 2
-- 2350,2415 - LEFT LEG DIE 1
-- 2500,2669 - LEFT LEG DIE 2
-- 2780,2870 - RIGHT ARM HIT 1
-- 2980,3087 - RIGHT ARM HIT 2
-- 3140,3205 - RIGHT LEG DIE 1
-- 3300,3469 - RIGHT LEG DIE 2
-- 3578,3647 - LEFT LEG HIT TO PRONE
-- 3709,3779 - RIGHT LEG HIT TO PRONE
-- 3818,3818 - PRONE
-- 3819,3864 - UP FROM PRONE
-- 3865,3912 - PRONE ATTACK
-- 3913,3967 - PRONE DIE
-- 3973,3993 - CRAWL REACH
-- 3994,4013 - CRAWL PULL

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

function ai_classiccharacter_apocalypse_init(e)
 CharacterControlLimbo(e)
 ai_soldier_state[e] = "patrol"
 ai_soldier_pathindex[e] = -1
 SetAnimationFrames(2,79) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
 roarstart[e]=629
 roarend[e]=687
 hurtstart[e]=740
 hurtend[e]=751
 attackdistance[e]=85
 attackstart[e]=304
 attackend[e]=392
 damageframestart[e]=330
 damageframeend[e]=334
 lastroar[e] = 0
 lastswipe[e] = 0
 ai_old_health[e] = -1
 SetCharacterSoundSet(e)
 dyingstart[e]=946
 dyingend[e]=9999
 SetPreExitValue(e,0)
end

function ai_classiccharacter_apocalypse_main(e)

 -- Death Animation
 if ai_soldier_state[e] ~= "deathanim" then
 
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
	attacktype[e] = math.random(1,5)
	if attacktype[e] == 1 then
	 attackstart[e]=304
	 attackend[e]=392
	 damageframestart[e]=330
	 damageframeend[e]=334
	end
	if attacktype[e] == 2 then
	 attackstart[e]=304
	 attackend[e]=392
	 damageframestart[e]=350
	 damageframeend[e]=354
	end
	if attacktype[e] == 3 then
     attackstart[e]=407
     attackend[e]=459
     damageframestart[e]=434
     damageframeend[e]=437
	end
	if attacktype[e] == 4 then
     attackstart[e]=473
     attackend[e]=536
     damageframestart[e]=503
     damageframeend[e]=507
	end
	if attacktype[e] == 5 then
     attackstart[e]=551
     attackend[e]=615
     damageframestart[e]=581
     damageframeend[e]=585
	end
	SetAnimationSpeed(e,1.5)
    SetAnimationFrames(attackstart[e],attackend[e]) 
    PlayAnimation(e)
	lastswipe[e] = 0
   else
    if lastswipe[e]==0 and GetAnimationFrame(e)<attackend[e]-1 then
	 lastswipe[e]=1
	end
    if lastswipe[e]==1 and GetAnimationFrame(e)>=attackend[e]-1 then
     ai_soldier_state[e] = "patrol"
	 SetAnimationSpeed(e,1.0)
 	 StopAnimation(e)
	 lastswipe[e]=2
    end    
   end
  else
   if ai_soldier_state[e] == "attack" then
    if GetAnimationFrame(e)<attackstart[e] or GetAnimationFrame(e)>=attackend[e]-1 then
     ai_soldier_state[e] = "patrol"
	 SetAnimationSpeed(e,1.0)
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
        ai_soldier_state[e] = "charge"
       end
	  end
	  if ai_soldier_state[e] ~= "hurtanim" then
	   if (GetAnimationFrame(e) > roarend[e]-2 and GetAnimationFrame(e) < roarend[e]+20) or lastroar[e] == -2 then
        ai_soldier_state[e] = "charge"
        CharacterControlUnarmed(e)
   	    chargemode[e] = math.random(1,2)
	    if chargemode[e]==1 then
         SetCharacterToWalk(e)
	    end
	    if chargemode[e]==2 then
         SetCharacterToRun(e)
	    end
        ModulateSpeed(e,1.0)
	    lastroar[e] = -2
	   end
	  end
	  if lastroar[e] > 0 then
       AIEntityStop(EntObjNo)
       ai_soldier_state[e] = "roar"
       CharacterControlLimbo(e)
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
   CharacterControlUnarmed(e)
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
 
 else --DeathAnim Else

  -- wait until death anim over
  if GetTimer(e)>2000 then
   SetEntityHealth(e,0)
  end
  
 end --DeathAnim Endif
  
end

function ai_classiccharacter_apocalypse_dying(e)
 CollisionOff(e)
 lwrlimb = string.lower(g_Entity[e]['limbhit'])
 if string.len(lwrlimb) > 3 then
  if string.find(lwrlimb, "head") ~= nil then hitarea = 1 end
   if string.find(lwrlimb, "spine") ~= nil then hitarea = 99 end
   if string.find(lwrlimb, "l_thigh") ~= nil then hitarea = 2 end
   if string.find(lwrlimb, "l_calf") ~= nil then hitarea = 2 end
   if string.find(lwrlimb, "l_foot") ~= nil then hitarea = 2 end
   if string.find(lwrlimb, "r_thigh") ~= nil then hitarea = 3 end
   if string.find(lwrlimb, "r_calf") ~= nil then hitarea = 3 end
   if string.find(lwrlimb, "r_foot") ~= nil then hitarea = 3 end
   if string.find(lwrlimb, "l_upperarm") ~= nil then hitarea = 4 end
   if string.find(lwrlimb, "l_forearm") ~= nil then hitarea = 4 end
   if string.find(lwrlimb, "l_hand") ~= nil then hitarea = 4 end
   if string.find(lwrlimb, "l_finger") ~= nil then hitarea = 4 end
   if string.find(lwrlimb, "r_upperarm") ~= nil then hitarea = 5 end
   if string.find(lwrlimb, "r_forearm") ~= nil then hitarea = 5 end
   if string.find(lwrlimb, "r_hand") ~= nil then hitarea = 5 end
   if string.find(lwrlimb, "r_finger") ~= nil then hitarea = 5 end
 end
 ResetLimbHit(e)
 PlayCharacterSound(e,"onDeath")
  StartTimer(e)
  CharacterControlLimbo(e)
  if hitarea == 1 then
   randhit = math.random(1,3)
   if randhit == 1 then dyingend[e]=1757  SetAnimationFrames(1685,1757)  SetAnimationFrame(e,1685) end
   if randhit == 2 then dyingend[e]=1883  SetAnimationFrames(1780,1883)  SetAnimationFrame(e,1780) end
   if randhit == 3 then dyingend[e]=2011  SetAnimationFrames(1900,2011)  SetAnimationFrame(e,1900) end
  end
  if hitarea == 2 then
   randhit = math.random(1,2)
   if randhit == 1 then dyingend[e]=2415  SetAnimationFrames(2350,2415)  SetAnimationFrame(e,2350) end
   if randhit == 2 then dyingend[e]=2669  SetAnimationFrames(2500,2669)  SetAnimationFrame(e,2500) end
  end
  if hitarea == 3 then
   randhit = math.random(1,2)
   if randhit == 1 then dyingend[e]=3205  SetAnimationFrames(3140,3205)  SetAnimationFrame(e,3140) end
   if randhit == 2 then dyingend[e]=3469  SetAnimationFrames(3300,3469)  SetAnimationFrame(e,3300) end
  end
  if hitarea == 4 then
   randhit = math.random(1,2)
   if randhit == 1 then dyingend[e]=2148  SetAnimationFrames(2020,2148)  SetAnimationFrame(e,2020) end
   if randhit == 2 then dyingend[e]=2291  SetAnimationFrames(2215,2291)  SetAnimationFrame(e,2215) end
  end
  if hitarea == 5 then
   randhit = math.random(1,2)
   if randhit == 1 then dyingend[e]=2870  SetAnimationFrames(2780,2870)  SetAnimationFrame(e,2780) end
   if randhit == 2 then dyingend[e]=3087  SetAnimationFrames(2980,3087)  SetAnimationFrame(e,2980) end
  end
  if hitarea == 99 then
   randhit = math.random(1,4)
   if randhit == 1 then dyingend[e]=1464  SetAnimationFrames(1394,1464)  SetAnimationFrame(e,1394) end
   if randhit == 2 then dyingend[e]=1595  SetAnimationFrames(1504,1595)  SetAnimationFrame(e,1504) end
   if randhit == 3 then dyingend[e]=1658  SetAnimationFrames(1610,1658)  SetAnimationFrame(e,1610) end
   if randhit == 4 then dyingend[e]=849  SetAnimationFrames(773,849)  SetAnimationFrame(e,773) end
  end
  PlayAnimation(e)
  ModulateSpeed(e,1.0)
  SetAnimationSpeed(e,1.0)
end

function ai_classiccharacter_apocalypse_preexit(e)
 SetPreExitValue(e,1)
 tFrame = GetAnimationFrame(e)
 if dyingstart[e] > 0 and dyingend[e] > 0 then
  if ai_bot_state[e] ~= ai_state_preexit then
   ai_bot_state[e] = ai_state_preexit
   ai_classiccharacter_apocalypse_dying(e)
  end
 else 
  SetPreExitValue(e,2)
 end
 if tFrame >= dyingend[e]-2 then
  SetPreExitValue(e,2)
 end 
end

function ai_classiccharacter_apocalypse_exit(e)
 CollisionOff(e)
 PlayCharacterSound(e,"onDeath")
end