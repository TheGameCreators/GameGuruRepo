-- LUA Script - melee attacker script for classic characters

-- ELHUMONGO Animframes:
-- pound 30,163 (85)
-- smash 180,285 (228)
-- stomp 300,400 (345)
-- idle 420,470
-- look_around 500,600
-- jaw_wag 605,680
-- roar 720,827
-- walk 1001,1091
-- rage 1200,1310
-- fall_sequence 1400,1791
-- fall_to_kneel 1400,1520
-- kneel_idle 1520,1670
-- getup_no_hit 1700,1791
-- getup_hit 1860,1951
-- rage_2 2000,2051
-- hurt 2100,2137
-- death 2100,2316
-- bull_charge_start 2400,2450
-- bull_charge 2500,2524
-- bull_charge_impact 2601,2720
-- throw_sequence 2800,2982
-- walk_slower 3102,3228

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

function ai_classiccharacter_elhumongo_init(e)
 CharacterControlLimbo(e)
 ai_soldier_state[e] = "patrol"
 ai_soldier_pathindex[e] = -1
 SetAnimationFrames(2,79) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,0.6)
 roarstart[e]=720
 roarend[e]=827
 hurtstart[e]=582
 hurtend[e]=625
 attackdistance[e]=150
 attackstart[e]=30
 attackend[e]=163
 damageframestart[e]=85
 damageframeend[e]=90
 lastroar[e] = 0
 lastswipe[e] = 0
 ai_old_health[e] = -1
 SetCharacterSoundSet(e)
 dyingstart[e]=2100
 dyingend[e]=2316
 SetPreExitValue(e,0)
end

function ai_classiccharacter_elhumongo_main(e)

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
	attacktype[e] = math.random(1,3)
	if attacktype[e] == 1 then
	 attackstart[e]=30
	 attackend[e]=163
	 damageframestart[e]=85
	 damageframeend[e]=90
	end
	if attacktype[e] == 2 then
     attackstart[e]=180
     attackend[e]=285
     damageframestart[e]=228
     damageframeend[e]=235
	end
	if attacktype[e] == 3 then
     attackstart[e]=300
     attackend[e]=400
     damageframestart[e]=345
     damageframeend[e]=350
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
	   if GetAnimationFrame(e) > roarend[e]-2 or lastroar[e] == -2 then
        ai_soldier_state[e] = "charge"
        CharacterControlUnarmed(e)
   	    chargemode[e] = math.random(1,2)
	    if chargemode[e]==1 then
		 SetAnimationSpeed(e,0.6)
         SetCharacterToWalk(e)
         ModulateSpeed(e,1.0)
	    end
	    if chargemode[e]==2 then
		 SetAnimationSpeed(e,1.0)
         SetCharacterToRun(e)
         ModulateSpeed(e,1.5)
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
     HurtPlayer(e,3)
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
   ResetLimbHit(e)
  end
  
 end --DeathAnim Endif

end

function ai_classiccharacter_elhumongo_exit(e)
 PlayCharacterSound(e,"onDeath")
 StartTimer(e)
 CharacterControlLimbo(e)
 SetAnimationFrames(2100,2316) 
 SetAnimationFrame(e,2100)
 PlayAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
 CollisionOff(e)
end
