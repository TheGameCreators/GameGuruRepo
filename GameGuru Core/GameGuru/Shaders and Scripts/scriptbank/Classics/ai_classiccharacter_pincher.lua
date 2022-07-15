-- LUA Script - melee attacker script for classic characters

-- PINCHER Animframes:
-- 2511,2560 - IDLE
-- 2580,2610 - WALK
-- 2630,2670 - RUN
-- 2690,2690 - IMPACT FRONT/FALLDOWN
-- 2690,2704 - HURT
-- 2691,2763 - DOUBLESWIPE (2705) (2740)
-- 2781,2824 - SINGLESWIPE (2809)
-- 2840,2840 - LYING ON BACK IDLE
-- 2840,2910 - CREEPY RISE/GETUP
-- 2931,2995 - RAGE
-- 3000,3000 - LIE FACE DOWN IDLE
-- 3001,3047 - GETUP FRONT

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

function ai_classiccharacter_pincher_init(e)
 CharacterControlLimbo(e)
 ai_soldier_state[e] = "patrol"
 ai_soldier_pathindex[e] = -1
 SetAnimationFrames(2,79) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
 roarstart[e]=2931
 roarend[e]=2995
 hurtstart[e]=2690
 hurtend[e]=2704
 attackdistance[e]=90
 attackstart[e]=2781
 attackend[e]=2824
 damageframestart[e]=2809
 damageframeend[e]=2813
 lastroar[e] = 0
 lastswipe[e] = 0
 ai_old_health[e] = -1
 SetCharacterSoundSet(e)
 dyingstart[e]=2690
 dyingend[e]=2690
 SetPreExitValue(e,0)
end

function ai_classiccharacter_pincher_main(e)
 
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
	attacktype[e] = math.random(1,3)
	if attacktype[e] == 1 then
	 attackstart[e]=2781
	 attackend[e]=2824
	 damageframestart[e]=2809
	 damageframeend[e]=2813
	end
	if attacktype[e] == 2 then
     attackstart[e]=2691
     attackend[e]=2763
     damageframestart[e]=2705
     damageframeend[e]=2709
	end
	if attacktype[e] == 3 then
     attackstart[e]=2691
     attackend[e]=2763
     damageframestart[e]=2740
     damageframeend[e]=2744
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

function ai_classiccharacter_pincher_dying(e)
 CollisionOff(e)
 PlayCharacterSound(e,"onDeath")
 StartTimer(e)
 CharacterControlLimbo(e)
 SetAnimationFrames(1,18) 
 SetAnimationFrame(e,1)
 PlayAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
end

function ai_classiccharacter_pincher_preexit(e)
 SetPreExitValue(e,1)
 tFrame = GetAnimationFrame(e)
 if dyingstart[e] > 0 and dyingend[e] > 0 then
  if ai_bot_state[e] ~= ai_state_preexit then
   ai_bot_state[e] = ai_state_preexit
   ai_classiccharacter_pincher_dying(e)
  end
 else 
  SetPreExitValue(e,2)
 end
 if tFrame >= dyingend[e]-2 then
  SetPreExitValue(e,2)
 end 
end

function ai_classiccharacter_pincher_exit(e)
 CollisionOff(e)
 PlayCharacterSound(e,"onDeath")
end