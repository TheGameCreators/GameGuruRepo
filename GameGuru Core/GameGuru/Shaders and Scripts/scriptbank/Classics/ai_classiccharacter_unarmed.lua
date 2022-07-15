-- LUA Script - unarmed script for classic characters

-- Classic Legacy Animframes:
--anim0         = 190,209 - spawn in and land
--anim1         = 210,232 - idle
--anim2         = 235,259 - walk
--anim3         = 260,279 - strafe to left
--anim4         = 280,299 - strafe to right
--anim5         = 300,318 - run
--anim6         = 319,355 - throw grenade
--anim11        = 0,19    - fall back on floor
--anim12        = 20,39   - fall back on wall
--anim13        = 493,522 - get up from lying on back
--anim14        = 40,59   - fall forward on floor
--anim15        = 60,79   - fall forward hit wall
--anim16        = 523,552 - get up from lying face down
--anim17        = 120,139 - hit from left
--anim18        = 140,159 - hit from left, slide down wall
--anim20        = 80,99   - hit from right
--anim21        = 100,119 - hit from right, slide down wall
--anim31        = 356,380 - crouch idle
--anim32        = 381,405 - crouch walk
--anim36        = 406,442 - crouch throw grenade
--anim40        = 443,462 - float idle
--anim41        = 463,492 - float move forward
--anim50        = 554,572 - spawn in and land (with pistol)
--anim51        = 573,597 - idle (with pistol)
--anim52        = 598,622 - walk (with pistol)
--anim53        = 623,642 - strafe left (with pistol)
--anim54        = 643,662 - strafe right (with pistol)
--anim55        = 663,681 - run (with pistol)
--anim56        = 682,731 - reload (with pistol)
--anim57        = 160,189 - climb ladder
--anim81        = 732,756 - crouch idle (with pistol)
--anim82        = 757,781 - crouch walk (with pistol)
--anim86        = 782,831 - crouch reload (with pistol)
--anim90        = 832,851 - float idle (with pistol)
--anim91        = 852,881 - float move (with pistol)

viewrange = {}

function ai_classiccharacter_unarmed_init(e)
 CharacterControlLimbo(e)
 ai_soldier_state[e] = "patrol"
 ai_soldier_pathindex[e] = -1
 SetAnimationFrames(211,232) 
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,0.7)
 ai_old_health[e] = -1
 SetCharacterSoundSet(e)
end

function ai_classiccharacter_unarmed_main(e)
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
 if PlayerDist < viewrange[e] and g_PlayerHealth > 0 then
  if ai_soldier_state[e] ~= "stare" then
   AIEntityStop(EntObjNo)
   ai_soldier_state[e] = "stare"
   CharacterControlLimbo(e)
   SetAnimationFrames(210,232) 
   LoopAnimation(e)
  else
   RotateToPlayer(e)
  end
 else
  if ai_soldier_state[e] ~= "patrol" then
   ai_soldier_state[e] = "patrol"
   StopAnimation(e)
   CharacterControlUnarmed(e)
   SetCharacterToWalk(e)
   ModulateSpeed(e,1.0)
  end
 end
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
end

function ai_classiccharacter_unarmed_exit(e)
 PlayCharacterSound(e,"onDeath")
 CollisionOff(e)
end
