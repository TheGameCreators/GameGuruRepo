-- LUA Script - melee attacker script for classic characters

-- MEDUSA Animframes:
-- 5,5     ; IDLE
-- 5,53    ; GETUP CREEPY RISE WHEN CLOSE
-- 58,97   ; WALK 1
-- 102,200 ; WALK 2
-- 241,289 ; OVERHEAD ATTACK (270)
-- 341,390 ; SWIPE (364)
-- 451,458 ; HURT
-- 480,772 ; INCINERATE
-- 801,860 ; LYING DOWN IN IDLE POSE

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

function ai_classiccharacter_medusa_init(e)
 CharacterControlLimbo(e)
 ai_soldier_state[e] = "sleeping"
 ai_soldier_pathindex[e] = -1
 SetAnimationFrames(5,5) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,0.9)
 roarstart[e]=5
 roarend[e]=53
 hurtstart[e]=451
 hurtend[e]=468
 attackdistance[e]=100
 attackstart[e]=241
 attackend[e]=289
 damageframestart[e]=270
 damageframeend[e]=275
 lastroar[e] = 0
 lastswipe[e] = 0
 ai_old_health[e] = -1
 SetCharacterSoundSet(e)
 dyingstart[e]=480
 dyingend[e]=772
 SetPreExitValue(e,0)
end

function ai_classiccharacter_medusa_main(e)

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
	attacktype[e] = math.random(1,2)
	if attacktype[e] == 1 then
	 attackstart[e]=241
	 attackend[e]=289
	 damageframestart[e]=270
	 damageframeend[e]=275
	end
	if attacktype[e] == 2 then
     attackstart[e]=341
     attackend[e]=390
     damageframestart[e]=364
     damageframeend[e]=368
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
	    else
         SetCharacterToRun(e)
 		 SetAnimationSpeed(e,2.0)
	    end
        ModulateSpeed(e,1.0)
	    lastroar[e] = -2
	   end
	  end
	  if lastroar[e] > 0 then
       AIEntityStop(EntObjNo)
       ai_soldier_state[e] = "roar"
       CharacterControlLimbo(e)
	   SetAnimationSpeed(e,0.9)
       SetAnimationFrames(roarstart[e],roarend[e]) 
       PlayAnimation(e) 
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
     HurtPlayer(e,2)
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
 if PlayerDist >= viewrange[e] and ai_soldier_state[e]~="patrol" and ai_soldier_state[e]~="sleeping" then
  ai_soldier_state[e] = "patrol"
  CharacterControlUnarmed(e)
  SetCharacterToWalk(e)
  ModulateSpeed(e,1.0)
 end 
 
 else --DeathAnim Else

  -- wait until death anim over
  if GetTimer(e)>2000 then
   SetEntityHealth(e,0)
   ResetLimbHit(e)
  end
  
 end --DeathAnim Endif
 
end

function ai_classiccharacter_medusa_exit(e)
 ai_soldier_state[e] = "deathanim"
 PlayCharacterSound(e,"onDeath")
 StartTimer(e)
 CharacterControlLimbo(e)
 SetAnimationFrames(480,772) 
 SetAnimationFrame(e,480)
 PlayAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,2.0)
 CollisionOff(e)
end
