-- COMBATCORE Common Module
-- anim0  : Idle
-- anim1  : Move
-- anim2  : Punch/Kick/Bite 
-- anim3  : Hurt
-- anim4  : Reload
-- anim5  : Punch/Kick/Bite Damage Frames
-- anim6  : Dying
-- anim7  : Stood to Duck
-- anim8  : Duck Idle
-- anim9  : Duck to Stood
-- anim10 : Run/Dash
-- anim11 : Roll to Duck
-- anim12 : Fast Crouch Dash
-- anim13 : idle sentry
-- anim14 : relaxed move
-- anim15 : strafe left
-- anim16 : strafe right
-- sound0 : Start moving A
-- sound1 : Start strike
-- sound2 : Start moving B
-- sound3 : Get hurt A
-- sound4 : Get hurt B

module_core = require "scriptbank\\ai\\module_core"
module_agro = require "scriptbank\\ai\\module_agro"
module_cameraoverride = require "scriptbank\\ai\\module_cameraoverride"
module_combateffects = require "scriptbank\\ai\\module_combateffects"


ai_bot_newroty = {}
local module_combatcore = {}

function module_combatcore.init(e,startstate,coverstate)
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 ai_bot_oldhealth[e] = g_Entity[e]['health']
 ai_bot_coverindex[e] = coverstate
 ai_bot_state[e] = startstate
 ai_bot_substate[e] = 0
 ai_bot_targetx[e] = nil
 ai_bot_pointtime[e] = -1
 ai_bot_patroltime[e] = 0
 ai_bot_targetx[e] = g_Entity[e]['x']
 ai_bot_targety[e] = g_Entity[e]['y']
 ai_bot_targetz[e] = g_Entity[e]['z']
 ai_bot_closeenoughx[e] = 0
 ai_bot_closeenoughy[e] = 0
 ai_bot_closeenoughz[e] = 0
 ai_bot_gofast[e] = 0
 ai_bot_sighting[e] = 0
 -- populate animation data if entity file does not provide it (except 5 and 6 which are melee)
 if GetEntityAnimationFound(e,0) == 0 then SetEntityAnimation(e,0,100,205) end
 if GetEntityAnimationFound(e,1) == 0 then SetEntityAnimation(e,1,685,707) end
 if GetEntityAnimationFound(e,2) == 0 then SetEntityAnimation(e,2,5511,5553) end
 if GetEntityAnimationFound(e,3) == 0 then SetEntityAnimation(e,3,4812,4850) end
 if GetEntityAnimationFound(e,4) == 0 then SetEntityAnimation(e,4,515,605) end
 if GetEntityAnimationFound(e,7) == 0 then SetEntityAnimation(e,7,1630,1646) end
 if GetEntityAnimationFound(e,8) == 0 then SetEntityAnimation(e,8,1670,1819) end
 if GetEntityAnimationFound(e,9) == 0 then SetEntityAnimation(e,9,1646,1663) end
 if GetEntityAnimationFound(e,10) == 0 then SetEntityAnimation(e,10,795,811) end
 if GetEntityAnimationFound(e,11) == 0 then SetEntityAnimation(e,11,2160,2218) end
 if GetEntityAnimationFound(e,12) == 0 then SetEntityAnimation(e,12,2135,2153) end
 if GetEntityAnimationFound(e,13) == 0 then SetEntityAnimation(e,13,900,999) end
 if GetEntityAnimationFound(e,14) == 0 then SetEntityAnimation(e,14,1290,1320) end
 if GetEntityAnimationFound(e,15) == 0 then SetEntityAnimation(e,15,610,640) end
 if GetEntityAnimationFound(e,16) == 0 then SetEntityAnimation(e,16,645,676) end
end

function module_combatcore.findcover(e,x,y,z)
 CoverIndex = -1
 tdx = x - g_PlayerPosX
 tdy = 0
 tdz = z - g_PlayerPosZ
 tDistToPlayer = math.sqrt ( math.abs(tdx*tdx) + math.abs(tdz*tdz) )
 for prefer = 1, 2, 1 do
  pClosest = 99999
  for ca = 1, AIGetTotalCover(), 1 do
   pDX = x - AICoverGetPointX(ca)
   pDZ = z - AICoverGetPointZ(ca)
   pDistToCover = math.sqrt(math.abs(pDX*pDX)+math.abs(pDZ*pDZ));
   if pDistToCover < pClosest and pDistToCover < tDistToPlayer and pDistToCover < 3000 then
    -- also reject cover that is 'further away' from player than the enemy
    tdx = AICoverGetPointX(ca) - g_PlayerPosX
    tdz = AICoverGetPointZ(ca) - g_PlayerPosZ
    tPlayerCoverDist = math.sqrt ( math.abs(tdx*tdx) + math.abs(tdz*tdz) )
	if tPlayerCoverDist <= tDistToPlayer then
	 -- reject cover if facing away from player direction
     tdx = g_PlayerPosX - AICoverGetPointX(ca)
     tdz = g_PlayerPosZ - AICoverGetPointZ(ca)
     tPlayerCoverAngle = (math.atan2(tdx,tdz)/6.28)*360.0
	 tanglediff = tPlayerCoverAngle - AICoverGetAngle(ca)
	 if tanglediff < -180 then tanglediff=tanglediff+360 end
	 if tanglediff > 180 then tanglediff=tanglediff-360 end
	 if math.abs(tanglediff) < 90 then
      tchoosethis = 0
      if prefer == 1 and ai_cover_slot[ca] == nil then tchoosethis = 1 end
      if prefer == 2 and ai_cover_slot[ca] ~= nil then tchoosethis = 1 end	 
      if tchoosethis == 1 then
       pClosest = pDistToCover
       CoverIndex = ca
	  end
	 end
    end
   end
  end
  if prefer == 1 and CoverIndex ~= -1 then return CoverIndex end
 end
 return CoverIndex
end

function module_combatcore.atcover(e)
 atcoverpos = 0
 CoverIndex = ai_bot_coverindex[e]
 if CoverIndex ~= -1 then
  tdx = AICoverGetPointX(CoverIndex) - g_Entity[e]['x']
  tdy = 0
  tdz = AICoverGetPointZ(CoverIndex) - g_Entity[e]['z']
  tcoverdd = math.sqrt ( math.abs(tdx*tdx) + math.abs(tdz*tdz) )
  if tcoverdd < 125 then
   atcoverpos = 1
  end
 end
 return atcoverpos
end

function module_combatcore.claimcover(e,slot)
 ai_cover_slot[slot] = e
 ai_bot_coverindex[e] = slot
end

function module_combatcore.releasecover(e)
 slot = ai_bot_coverindex[e]
 if slot > 0 then
  ai_cover_slot[slot] = nil
 end
 ai_bot_coverindex[e] = -2
end

function module_combatcore.findandassigncover(e,AIObjNo,PlayerDist)
 tcoverassigned = 0
 if ai_bot_coverindex[e] == -1 then
  coverslot = module_combatcore.findcover(e,g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z'])
  if coverslot > 0 then
   module_combatcore.claimcover(e,coverslot)
   ai_bot_targetx[e] = AICoverGetPointX(ai_bot_coverindex[e])
   ai_bot_targety[e] = g_PlayerPosY
   ai_bot_targetz[e] = AICoverGetPointZ(ai_bot_coverindex[e])
   tcoverassigned = 1
  end
 end
 if ai_bot_coverindex[e] < 0 then
  module_combatcore.releasecover(e)
 end
 if ai_bot_coverindex[e] == -2 and PlayerDist < AIGetEntityViewRange(AIObjNo) then
  ai_bot_targetx[e] = g_PlayerPosX
  ai_bot_targety[e] = g_PlayerPosY
  ai_bot_targetz[e] = g_PlayerPosZ
 end
 return tcoverassigned
end

function module_combatcore.detectplayer(e,AIObjNo,PlayerDist,CanFire,detectstate)
 if (PlayerDist < AIGetEntityViewRange(AIObjNo) and (g_Entity[e]['plrvisible'] == 1 or ai_bot_coverindex[e] == -2)) or g_Entity[e]['activated'] == 2 then
  if g_Entity[e]['activated'] == 2 then 
   ai_bot_targetx[e] = g_PlayerPosX
   ai_bot_targety[e] = g_PlayerPosY
   ai_bot_targetz[e] = g_PlayerPosZ  
   ai_bot_substate[e] = 0
   SetActivated(e,0)
  else
   if module_combatcore.findandassigncover(e,AIObjNo,PlayerDist) == 1 then
    AIEntityGoToPosition(AIObjNo,ai_bot_targetx[e],ai_bot_targety[e],ai_bot_targetz[e])
    ai_bot_state[e] = ai_state_startmove
    ai_bot_substate[e] = 0
   end
  end
  if ai_bot_targetx[e] ~= nil then
   if ai_bot_substate[e] == 0 then
    if module_combatcore.getcomfortdistance(e) < 25 then
	 RotateToPlayerSlowly(e,1.0)
	else
     AIEntityGoToPosition(AIObjNo,ai_bot_targetx[e],ai_bot_targety[e],ai_bot_targetz[e])
     if AIGetEntityIsMoving(AIObjNo) == 1 then
      ai_bot_state[e] = detectstate
      PlaySound(e,0+(math.random(0,1)*2))
     else 
      if g_Entity[e]['plrvisible'] == 1 then
       RotateToPlayer(e)
       if CanFire == 1 then
        module_combatcore.fireweapon(e)
       end
       ai_bot_targetx[e] = g_PlayerPosX
       ai_bot_targety[e] = g_PlayerPosY
       ai_bot_targetz[e] = g_PlayerPosZ
       AIEntityGoToPosition(AIObjNo,ai_bot_targetx[e],ai_bot_targety[e],ai_bot_targetz[e])
	  end
     end
    end
   end
  end
 end
end

function module_combatcore.getcomfortdistance(e)
 dx = ai_bot_closeenoughx[e] - g_PlayerPosX
 dy = ai_bot_closeenoughy[e] - g_PlayerPosY
 dz = ai_bot_closeenoughz[e] - g_PlayerPosZ
 checkdistancefromcomfort = math.sqrt(math.abs(dx*dx)+math.abs(dy*dy)+math.abs(dz*dz))
 return checkdistancefromcomfort
end

function module_combatcore.idle(e,AIObjNo,PlayerDist,CanFire,detectstate,combattype)
 CharacterControlManual(e)
 AISetEntityControl(AIObjNo,AI_MANUAL)
 if ai_bot_state[e] == ai_state_startidle then
  ai_bot_state[e] = ai_state_idle
  if combattype == ai_combattype_regular then
   SetAnimation(13)
  else
   SetAnimation(0)
  end
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
 end
 if ai_bot_state[e] == ai_state_idle then
  module_combatcore.detectplayer(e,AIObjNo,PlayerDist,CanFire,detectstate)
  module_combatcore.donotmove(e) 
 end 
end

function module_combatcore.patrol(e,AIObjNo,PlayerDist,MoveType,CanFire,detectstate,stopstate,combattype)
 if ai_bot_pointtime[e] == -1 then
  ai_bot_pointtime[e] = 0
  StartTimer(e)
 end
 if ai_bot_state[e] == ai_state_findpatrolpath and ai_bot_pointtime[e] == 0 then
  PathIndex = -1
  PointIndex = 2
  pClosest = 99999
  for pa = 1, AIGetTotalPaths(), 1 do
   for po = 1 , AIGetPathCountPoints(pa), 1 do
    pDX = g_Entity[e]['x'] - AIPathGetPointX(pa,po)
    pDY = g_Entity[e]['y'] - AIPathGetPointY(pa,po)
    pDZ = g_Entity[e]['z'] - AIPathGetPointZ(pa,po)
    pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDY*pDY)+math.abs(pDZ*pDZ));
    if pDist < pClosest and pDist < 200 then
     pClosest = pDist
     PathIndex = pa
     PointIndex = po
    end
   end -- po
  end -- pa
  ai_bot_pathindex[e] = PathIndex
  if PathIndex > -1 then
   ai_bot_state[e] = ai_state_startpatrol
   ai_bot_pointdirection[e] = 1
   ai_bot_pointindex[e] = PointIndex
   ai_bot_pointmax[e] = AIGetPathCountPoints(PathIndex)
  else
   ai_bot_state[e] = ai_state_startidle
  end   
 end
 if ai_bot_state[e] == ai_state_startpatrol then
  if ai_bot_pathindex[e] ~= -1 then
   ai_bot_state[e] = ai_state_patrol
   ai_bot_pointtime[e] = g_Time + 100
   AISetEntityMoveBoostPriority(AIObjNo)
   SetAnimationSpeedModulation(e,0.0)
   if combattype == ai_combattype_regular then
    SetAnimation(14)
   else
    SetAnimation(1)
   end
   LoopAnimation(e)
   StartTimer(e)
  else
   ai_bot_state[e] = ai_state_idle
   if combattype == ai_combattype_regular then
    SetAnimation(13)
   else
    SetAnimation(0)
   end
   LoopAnimation(e)
   SetAnimationSpeedModulation(e,1.0)  
  end
 end
 if ai_bot_state[e] == ai_state_patrol then
  patrolx = AIPathGetPointX(ai_bot_pathindex[e],ai_bot_pointindex[e])
  patroly = AIPathGetPointY(ai_bot_pathindex[e],ai_bot_pointindex[e])
  patrolz = AIPathGetPointZ(ai_bot_pathindex[e],ai_bot_pointindex[e])
  module_combatcore.moveandavoid(e,AIObjNo,PlayerDist,MoveType,patrolx,patroly,patrolz,stopstate)
  if AIGetEntityIsMoving(AIObjNo) == 1 then
   if GetAnimationSpeedModulation(e) == 0.0 then
    SetAnimationSpeedModulation(e,0.1)
   else
    tWalkDelta = GetMovementDelta(e) * 2.0
	if tWalkDelta > 0.9 then tWalkDelta = 0.9 end
    SetAnimationSpeedModulation(e,0.1+tWalkDelta)
   end
  end  
  tDistX = g_Entity[e]['x'] - patrolx
  tDistZ = g_Entity[e]['z'] - patrolz
  VertDist = math.abs(g_Entity[e]['y'] - patroly)
  DistFromPath = math.sqrt(math.abs(tDistX*tDistX)+math.abs(tDistZ*tDistZ))	
  if DistFromPath < 25 and VertDist < 95 and g_Time > ai_bot_pointtime[e] then
   ai_bot_pointtime[e] = g_Time + 100
   AISetEntityMoveBoostPriority(AIObjNo)
   StartTimer(e)
   if ai_bot_pointdirection[e] == 1 then
    ai_bot_pointindex[e] = ai_bot_pointindex[e] + 1
    if ai_bot_pointindex[e] > ai_bot_pointmax[e] then
     ai_bot_pointindex[e] = ai_bot_pointmax[e] -1
     ai_bot_pointdirection[e] = 0
    end
   else
    ai_bot_pointindex[e] = ai_bot_pointindex[e] - 1
    if ai_bot_pointindex[e] < 1 then
     ai_bot_pointindex[e] = 2
     ai_bot_pointdirection[e] = 1
    end		
   end
  end
  module_combatcore.detectplayer(e,AIObjNo,PlayerDist,CanFire,detectstate)
 end 
end

function module_combatcore.hunt(e,AIObjNo,PlayerDist,MoveType,CanFire,stopstate)
 if ai_bot_state[e] == ai_state_startmove then
  ai_bot_state[e] = ai_state_move
  if ai_bot_targetx[e] ~= nil then
   tdX = g_Entity[e]['x'] - ai_bot_targetx[e]
   tdZ = g_Entity[e]['z'] - ai_bot_targetz[e]
   tdist = math.sqrt(math.abs(tdX*tdX)+math.abs(tdZ*tdZ))
  else 
   tdist = 0
  end
  if tdist > 200 then 
   ai_bot_gofast[e] = Timer()
  else
   ai_bot_gofast[e] = 0
  end
  if ai_bot_gofast[e] > 0 then
   ModulateSpeed(e,1.5)
   SetAnimation(10)
  else
   ModulateSpeed(e,1.0)
   SetAnimation(1)
  end
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
  StartTimer(e)
 end
 if ai_bot_state[e] == ai_state_move then
  if GetTimer(e) > 500 then
   if ai_bot_coverindex[e] > 0 then
    if module_combatcore.atcover(e) == 1 then
	 if g_Entity[e]['plrdist'] > 400 then
		local rays = module_combatcore.checkentityrays(e,130,0,g_Entity[e]['obj'],57)
		if rays == 0 then 
			if ai_bot_state[e] ~= ai_state_duck then
			 ai_bot_state[e] = ai_state_duckstart
			end
		else --can't see over cover so try roll sideways?
			if ai_bot_state[e] ~= ai_state_rollstart and ai_bot_state[e] ~= ai_state_roll then
				ai_bot_state[e] = ai_state_rollstart
				module_combatcore.handleducking(e,AIObjNo,PlayerDist)
				return
			end 
		end 
	 else
	  -- break from cover, player too close
	  module_combatcore.releasecover(e)
	 end
    end
   end
   if ai_bot_coverindex[e] == -2 then
    if g_Entity[e]['plrvisible'] == 1 and CanFire == 1 then
     ai_bot_state[e] = ai_state_startfireonspot   
    end
   end
   if ai_bot_substate[e] == 0 then
    StartTimer(e)
   end
  end
  if ai_bot_state[e] ~= ai_state_startfireonspot then
   if ai_bot_coverindex[e] == -2 then
    if g_Entity[e]['plrvisible'] == 1 then
     ai_bot_targetx[e] = g_PlayerPosX
     ai_bot_targety[e] = g_PlayerPosY
     ai_bot_targetz[e] = g_PlayerPosZ
	end
   end
   if ai_bot_coverindex[e] > 0 then
    ai_bot_targetx[e] = AICoverGetPointX(ai_bot_coverindex[e])
    ai_bot_targety[e] = g_PlayerPosY
    ai_bot_targetz[e] = AICoverGetPointZ(ai_bot_coverindex[e])
   end
   if ai_bot_targetx[e] ~= nil then
    module_combatcore.moveandavoid(e,AIObjNo,PlayerDist,MoveType,ai_bot_targetx[e],ai_bot_targety[e],ai_bot_targetz[e],stopstate)
    if CanFire == 1 then
     module_combatcore.fireweapon(e)
    end
   end
   if ai_bot_gofast[e] > 0 and Timer() > ai_bot_gofast[e]+500 then
    tWalkDelta = GetMovementDeltaManually(e,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
	if tWalkDelta < 1.0 then
	 StopAnimation(e)
	 SetAnimationFrame(e,GetEntityAnimationStart(e,1))
     SetAnimation(1)
     LoopAnimation(e)
     ModulateSpeed(e,1.0)
	 ai_bot_gofast[e] = 0
	end
   end
  end
 end
 if g_Entity[e]['plrvisible'] == 1 then 
  ai_bot_hunttime[e] = Timer() 
 else
  if ai_bot_hunttime[e] ~= nil then
   if ai_bot_hunttime[e] > 0 and Timer() > ai_bot_hunttime[e]+5000 then
    if ai_bot_state[e] ~= ai_state_duck then
     ai_bot_state[e] = ai_state_startmove
     ai_bot_hunttime[e] = 0
     SetActivated(e,0)
    end
   end
  end
 end
end

function module_combatcore.handleducking(e,AIObjNo,PlayerDist)
 if ai_bot_state[e] == ai_state_crouchdashstart then
  ai_bot_state[e] = ai_state_crouchdash
  ai_bot_substate[e] = 0
  StopAnimation(e)
  SetAnimationFrame(e,GetEntityAnimationStart(e,12))
  SetAnimation(12)
  LoopAnimation(e)
  ModulateSpeed(e,1.25)
  SetAnimationSpeedModulation(e,1.25)
  randomevade = math.random(45,70)
  if math.random(1,2) == 1 then randomevade=randomevade*-1 end
  SetRotation(e,0,AIGetEntityAngleY(AIObjNo)+randomevade,0)
  StartTimer(e)
 else
  if ai_bot_state[e] == ai_state_crouchdash then
   tWalkDelta = GetMovementDeltaManually(e,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   if GetTimer(e) < 250 or (GetTimer(e) < 2000 and PlayerDist > 200 and tWalkDelta > 1.0) then
    if GetTimer(e) < 1750 then
     MoveForward(e,AIGetEntitySpeed(AIObjNo))
     AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
    end
    if GetTimer(e) > 1250 then
 	 RotateToPlayerSlowly(e,10.0)
     ModulateSpeed(e,1.0)
     SetAnimationSpeedModulation(e,1.0)
	end
   else 
    ai_bot_state[e] = ai_state_checkforcover
   end
  end
 end 
 module_combatcore.evasiveactions(e,AIObjNo,PlayerDist)
 module_combatcore.strafeleft(e,0.5)
 module_combatcore.straferight(e,0.5)
 module_combatcore.fireweapon(e)
 if ai_bot_state[e] == ai_state_rollstart then
  randomevade = math.random(65,90)
  if math.random(1,2) == 1 then randomevade=randomevade*-1 end
	local roty = AIGetEntityAngleY(AIObjNo)+randomevade
	local dist = 130
	local rays = module_combatcore.checkentityrays(e,e,dist,roty,g_Entity[e]['obj'],20)
	if rays == 0 then 
		SetRotation(e,0,roty,0)
		ai_bot_state[e] = ai_state_roll
		ai_bot_substate[e] = 0
		SetAnimation(11)
		PlayAnimation(e)
		ModulateSpeed(e,1.25)
		SetAnimationSpeedModulation(e,1.25)
	else 
		ai_bot_state[e] = ai_state_checkforcover
	end 
 else
  if ai_bot_state[e] == ai_state_roll then
   tFrame = GetAnimationFrame(e)
   tStart = GetEntityAnimationStart(e,11)
   tFinish = GetEntityAnimationFinish(e,11)
   if tFrame > tStart + 1 and tFrame < tFinish - 20 then
    MoveForward(e,AIGetEntitySpeed(AIObjNo))
    AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   else 
    if tFrame >= tFinish - 20 then
 	 RotateToPlayerSlowly(e,30.0)
    end
	MoveForward(e,0.0)
   end
   if tFrame >= tFinish then
    ai_bot_state[e] = ai_state_checkforcover
   end
  end
 end 
 if ai_bot_state[e] == ai_state_checkforcover then
  SetAnimationSpeedModulation(e,1.0)
  ModulateSpeed(e,1.0)
  StopAnimation(e)
  if module_combatcore.findandassigncover(e,AIObjNo,PlayerDist) == 1 then
   AIEntityGoToPosition(AIObjNo,ai_bot_targetx[e],ai_bot_targety[e],ai_bot_targetz[e])
   ai_bot_state[e] = ai_state_startmove
   ai_bot_substate[e] = 0
  else
   ai_bot_state[e] = ai_state_duckstart
  end
 end
 if ai_bot_state[e] == ai_state_duckstart then
  ai_bot_state[e] = ai_state_duck
  ai_bot_substate[e] = 0
  RotateToPlayer(e)
  SetAnimation(7)
  PlayAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
 else
  if ai_bot_state[e] == ai_state_duck then
   if ai_bot_substate[e] == 0 then
    tFrame = GetAnimationFrame(e)
    tFinish = GetEntityAnimationFinish(e,7)
    if tFrame >= tFinish then
     ai_bot_substate[e] = 1
     SetAnimation(8)
     PlayAnimation(e)
    end
   end
   -- while ducking
   if ai_bot_substate[e] == 1 then
    -- stay put
    MoveForward(e,0.0)
	-- if see player, rotate slowly to shoot
	if g_Entity[e]['plrvisible'] == 1 then
	 RotateToPlayerSlowly(e,30.0)
     module_combatcore.fireweapon(e)
	end
    -- if player gets too close, end duck and cover
    if g_Entity[e]['plrdist'] < 400 then
     ai_bot_state[e] = ai_state_unduckstart
    end
	-- if player at distance and in cover, pop head up and shoot at random
	if ai_bot_coverindex[e] > 0 and math.random(1,30) == 1 then
	 ai_bot_state[e] = ai_state_unduckstart
	end
   end
   -- while ducking-popupshoot
   if ai_bot_substate[e] == 2 then
    StopAnimation(e)
	SetAnimationFrame(e,GetEntityAnimationStart(e,9))
    SetAnimation(9)
    PlayAnimation(e)
	ai_bot_substate[e] = 3
   else
    if ai_bot_substate[e] == 3 then
     RotateToPlayerSlowly(e,20.0)
     module_combatcore.fireweapon(e)
     tFrame = GetAnimationFrame(e)
     tFinish = GetEntityAnimationFinish(e,9)
     if tFrame >= tFinish then
      StopAnimation(e)
      SetAnimationFrame(e,GetEntityAnimationStart(e,7))
      SetAnimation(7)
      PlayAnimation(e)
 	  ai_bot_substate[e] = 4
     end
    else
     if ai_bot_substate[e] == 4 then
      RotateToPlayerSlowly(e,20.0)
      module_combatcore.fireweapon(e)
      tFrame = GetAnimationFrame(e)
      tFinish = GetEntityAnimationFinish(e,7)
      if tFrame >= tFinish then
 	   ai_bot_substate[e] = 1
       StopAnimation(e)
       SetAnimationFrame(e,GetEntityAnimationStart(e,8))
       SetAnimation(8)
       PlayAnimation(e)
      end
     end   
	end   
   end   
  end
  if ai_bot_state[e] == ai_state_unduckstart then
   ai_bot_state[e] = ai_state_unduck
   SetAnimation(9)
   PlayAnimation(e)
  else
   if ai_bot_state[e] == ai_state_unduck then
    tFrame = GetAnimationFrame(e)
    tFinish = GetEntityAnimationFinish(e,9)
    if tFrame >= tFinish-1 and tFrame <= tFinish then
     ai_bot_state[e] = ai_state_startidle
     module_combatcore.releasecover(e)
     ai_bot_substate[e] = 0
    end
   end
  end
 end
end

function module_combatcore.homein(e,AIObjNo,PlayerDist,MoveType,CanFire,stopstate)
 if ai_bot_state[e] == ai_state_startmove then
  if module_combatcore.getcomfortdistance(e) < 25 then
   -- no need to go to target, happy with close location determined by player position
   ai_bot_state[e] = stopstate
  else
   ai_bot_state[e] = ai_state_move
   SetAnimation(1)
   LoopAnimation(e)
   SetAnimationSpeedModulation(e,1.0)
  end
 end
 if ai_bot_state[e] == ai_state_move then
  -- scan if another bot is crowding player
  tbeingcrowded=0
  for ee = 1 , g_EntityElementMax, 1 do
   if g_Entity[ee] ~= nil then
    pDX = g_Entity[ee]['x'] - g_PlayerPosX
    pDY = g_Entity[ee]['y'] - g_PlayerPosY
    pDZ = g_Entity[ee]['z'] - g_PlayerPosZ
    pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDY*pDY)+math.abs(pDZ*pDZ));
    if e ~= ee and pDist < 100 then
     tbeingcrowded=1
	end
   end
  end 
  if tbeingcrowded == 0 then
   ai_bot_targetx[e] = g_PlayerPosX
   ai_bot_targety[e] = g_PlayerPosY
   ai_bot_targetz[e] = g_PlayerPosZ
  else
   ai_bot_targetx[e] = g_PlayerPosX + (math.sin(e)*50.0)
   ai_bot_targety[e] = g_PlayerPosY
   ai_bot_targetz[e] = g_PlayerPosZ + (math.cos(e)*50.0)
  end
  module_combatcore.moveandavoid(e,AIObjNo,PlayerDist,MoveType,ai_bot_targetx[e],ai_bot_targety[e],ai_bot_targetz[e],stopstate)
 end
end

function module_combatcore.moveandavoid(e,AIObjNo,PlayerDist,MoveType,x,y,z,stopstate)
 movementfrozen = module_combateffects.ismovementfrozen(e)
 if ai_bot_substate[e] == 0 then
  if PlayerDist < 100 then
   if module_core.countaiaroundplayer() > 1 then
    tDistX = x - g_Entity[e]['x']
    tDistZ = z - g_Entity[e]['z']
    tDA = math.atan2(tDistX,tDistZ)
    x = x + (math.sin(tDA) * 50)
    z = z + (math.cos(tDA) * 50)
   end
  end
  AIEntityGoToPosition(AIObjNo,x,y,z)
  SetRotationYSlowly(e,AIGetEntityAngleY(AIObjNo),10.0)
  if movementfrozen == 0 then
   dy = g_PlayerPosY - (g_Entity[e]['y']+50)
   if ai_bot_state[e] == ai_state_patrol then
	stopifclosetofinaldest = 0
   else
    stopifclosetofinaldest = math.sqrt(math.abs(dy*dy))
    if stopifclosetofinaldest < 75 then
     dx = g_PlayerPosX - g_Entity[e]['x']
     dz = g_PlayerPosZ - g_Entity[e]['z']
     stopifclosetofinaldest = math.sqrt(math.abs(dx*dx)+math.abs(dz*dz))
     if stopifclosetofinaldest < 35 then
 	  ai_bot_closeenoughx[e] = g_PlayerPosX 
	  ai_bot_closeenoughy[e] = g_PlayerPosY
	  ai_bot_closeenoughz[e] = g_PlayerPosZ
      AIEntityGoToPosition(AIObjNo,ai_bot_closeenoughx[e],ai_bot_closeenoughy[e],ai_bot_closeenoughz[e])
	  RotateToPlayer(e)
	  stopifclosetofinaldest = 1
	 else 
	  stopifclosetofinaldest = 0 
	 end
	else
     stopifclosetofinaldest = 0
    end
   end
   if AIGetEntityIsMoving(AIObjNo) == 1 and stopifclosetofinaldest == 0 then
    ai_bot_closeenoughx[e] = 0
    ai_bot_closeenoughy[e] = 0
    ai_bot_closeenoughz[e] = 0
    if MoveType == ai_movetype_useanim then
     MoveWithAnimation(e,1)
    else
			local speedmod = 1
			if ai_bot_state[e] == ai_state_patrol then 
				speedmod = 0.5
			end 
     MoveForward(e,AIGetEntitySpeed(AIObjNo)*speedmod)
    end
   else
    MoveForward(e,0.0)
    if GetTimer(e) > 250 then
     ai_bot_state[e] = stopstate
	end
   end
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  end
  if g_Entity[e]['avoid'] == 2 or (g_Entity[e]['avoid'] == 1 and stopstate ~= ai_state_startpatrol) then
   ai_bot_substate[e] = math.random(1,2)
   if ai_bot_substate[e] == 1 then
    tAvoidAngle = AIGetEntityAngleY(AIObjNo)-95
   else
    tAvoidAngle = AIGetEntityAngleY(AIObjNo)+95
   end
   tAvoidAngle = (tAvoidAngle / 360.0) * 6.28
   tAvoidX = GetEntityPositionX(e) + (math.sin(tAvoidAngle) * 30)
   tAvoidZ = GetEntityPositionZ(e) + (math.cos(tAvoidAngle) * 30)
   AIEntityGoToPosition(AIObjNo,tAvoidX,GetEntityPositionY(e),tAvoidZ)
   StartTimer(e)
  end
 end
 if ai_bot_substate[e] > 0 and ai_bot_state[e] ~= ai_state_duck then
  SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
  if movementfrozen == 0 then
   if AIGetEntityIsMoving(AIObjNo) == 1 then
    if MoveType == ai_movetype_useanim then
     MoveWithAnimation(e,1)
    else
     MoveForward(e,AIGetEntitySpeed(AIObjNo))
    end
   else
    MoveForward(e,0.0)
   end
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  end
  if GetTimer(e) > 1000 then
   ai_bot_substate[e] = 0
  end
 end
end

function module_combatcore.donotmove(e)
 MoveForward(e,0)
 ai_bot_substate[e] = 0
end

function module_combatcore.sensepunch(e,AIObjNo,PlayerDist,combattype)
 if PlayerDist < 150 and combattype == ai_combattype_freezermelee and ai_bot_state[e] ~= ai_state_move then
  RotateToPlayer(e)
 end
 if ai_bot_state[e] == ai_state_move or ai_bot_state[e] == ai_state_fireonspot then
  if PlayerDist < 75 and g_PlayerPosY > g_Entity[e]['y'] and g_PlayerPosY < g_Entity[e]['y']+70 then
    -- must also check for line of sight (i.e not through walls) (first and third person)
	thaslineofsight = 0
	tthitvalue = IntersectAll(g_Entity[e]['x'],g_Entity[e]['y']+50,g_Entity[e]['z'],g_PlayerPosX,g_PlayerPosY+50,g_PlayerPosZ,AIObjNo)
	ttpersonobj = 0
	if GetGamePlayerControlThirdpersonEnabled() == 1 then 
     ttpersone = GetGamePlayerControlThirdpersonCharactere()  
     if ttpersone > 0 then 
      ttpersonobj = GetEntityElementObj(ttpersone)
      if GetObjectExist(ttpersonobj) == 0 then ttpersonobj = 0 end
     end
    end
	if tthitvalue == 0 or tthitvalue == playerobj then 
	 thaslineofsight = 1
	end
	if thaslineofsight == 1 then
     ai_bot_state[e] = ai_state_punch
     SetAnimation(2)
     PlayAnimation(e)
     SetAnimationSpeedModulation(e,1.0)
     PlaySound(e,1)
     RotateToPlayer(e)
	 if combattype == ai_combattype_freezermelee then
	  module_cameraoverride.beingattackedby(e,60.0)
	 end
	end
  end
 end
end
 
function module_combatcore.hurt(e,PlayerDist,responsestate)
 if ai_bot_state[e] == ai_state_idle or ai_bot_state[e] == ai_state_patrol or ai_bot_state[e] == ai_state_move or ai_bot_state[e] == ai_state_fireonspot or ai_bot_state[e] == ai_state_recover or ai_bot_state[e] == ai_state_punch or ai_bot_state[e] == ai_state_duck then
  if g_Entity[e]['health'] < ai_bot_oldhealth[e] then
   if ai_bot_state[e] == ai_state_duck then
    ai_bot_state[e] = ai_state_unduckstart
   else
    if (ai_bot_state[e] == ai_state_move or ai_bot_state[e] == ai_state_idle or ai_bot_state[e] == ai_state_fireonspot) and PlayerDist > 300 then
	 if math.random(1,3) == 1 then
      ai_bot_state[e] = ai_state_rollstart
	 else
      ai_bot_state[e] = ai_state_crouchdashstart
	 end
    else
	 if ai_bot_state[e] == ai_state_roll then
	  -- AI is immune to damage when rolling
	  SetEntityHealth(e,ai_bot_oldhealth[e])
	 else
			local flinch = math.random(1,3)
			if flinch == 1 then 
				ai_bot_state[e] = ai_state_hurt
				SetAnimationSpeed(e,3.0)
				SetAnimation(3)
				PlayAnimation(e)
				SetAnimationSpeedModulation(e,1.0)
			end 
	 end
    end
   end
   ai_bot_oldhealth[e] = g_Entity[e]['health']
   ai_bot_angryhurt[e] = 1
   PlaySound(e,math.random(3,4))
   module_agro.alertallwithinradius(g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z'],250.0)
  end
 end
 if ai_bot_state[e] == ai_state_hurt then
  MoveForward(e,0.0)
  RotateToPlayerSlowly(e,10.0)
  tFrame = GetAnimationFrame(e)
  tStart = GetEntityAnimationFinish(e,3)
  if tFrame >= tStart then
   SetAnimationSpeed(e,1.0)
   ai_bot_state[e] = responsestate
  end
 end
end

function module_combatcore.headshot(e)
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0)
  ResetLimbHit(e)
 end
end

function module_combatcore.soundawareness(e,AIObjNo)
 if AIGetEntityHeardSound(AIObjNo) == 1 then
		local botState = ai_bot_state[e]
		if botState == ai_state_idle or
			botState == ai_state_hurt then     
			ai_bot_sighting[e] = Timer()+500    
		elseif
			botstate == ai_state_patrol then
			ai_bot_state[e] = ai_state_startidle
			ai_bot_sighting[e] = Timer()+500
		else
			ai_bot_sighting[e] = 0
		end
	end
	if ai_bot_sighting[e] > 0 and
		Timer() < ai_bot_sighting[e] then
		RotateToPlayerSlowly( e, 10.0 )
	end
end

function module_combatcore.punch(e,AIObjNo,PlayerDist,combattype,afterstate)
 if ai_bot_state[e] == ai_state_punch then
  wemoved = 0
  if combattype == ai_combattype_freezermelee then
   if module_cameraoverride.manageattackcycle(e) == 1 then
    MoveWithAnimation(e,1)
	wemoved = 1
   end
  end
  tFrame = GetAnimationFrame(e)
  tStart = GetEntityAnimationStart(e,2)
  tFinish = GetEntityAnimationFinish(e,2)
  tDamageFramesStart = GetEntityAnimationStart(e,5)
  tDamageFramesFinish = GetEntityAnimationFinish(e,5)
  if tDamageFramesStart > 0 and tDamageFramesFinish > 0 then
   -- override defaults if anim5 specifies specific damage frame range
   tStart = tDamageFramesStart
   tFinish = tDamageFramesFinish
   if combattype == ai_combattype_freezermelee then
    tStart = tStart - 10
    tFinish = tFinish + 30
   else
    if combattype == ai_combattype_bashmelee then
     tStart = tStart - 25
    else
     tStart = tStart - 10
	end
   end
  end
  if PlayerDist < 90 then
   if combattype == ai_combattype_freezermelee then
    if tFrame >= tStart+10 and tFrame <= tFinish-30 then
     if math.random(1,7) == 1 then
      HurtPlayer(e,math.random(1,2))
     end
    end   
   else
    if combattype == ai_combattype_bashmelee then
     if tFrame >= tStart+25 and tFrame <= tStart+35 then
      HurtPlayer(e,math.random(2,3))
	  ForcePlayer ( g_Entity[e]['angley'], 2.0 )
     end
	else
     if tFrame >= tStart+10 and tFrame <= tStart+15 then
      HurtPlayer(e,math.random(2,3))
	  ForcePlayer ( g_Entity[e]['angley'], 3.0 )
     end
	end
   end
  end
  if combattype == ai_combattype_freezermelee then
   if tFrame >= tFinish-30 then
    if module_cameraoverride.finishbeingattacked(e) == 1 then
     ForcePlayer ( g_Entity[e]['angley'], 3.0 )
	end
   else
    if module_cameraoverride.hasowner() == 0 then
	 ai_bot_state[e] = ai_state_recoverstart
    end
   end
  end
  if tFrame >= tFinish then 
   if combattype == ai_combattype_freezermelee or combattype == ai_combattype_bashmelee then
    ai_bot_state[e] = ai_state_recoverstart
   else
    ai_bot_state[e] = afterstate
   end
  end 
  if wemoved == 0 then
   module_combatcore.donotmove(e) 
  end
 end
end

function module_combatcore.recover(e,resumestate)
 if ai_bot_state[e] == ai_state_recoverstart then
  ai_bot_state[e] = ai_state_recover
  StopAnimation(e)
  SetAnimation(0)
  SetAnimationFrame(e,GetEntityAnimationStart(e,0))
  PlayAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
  StartTimer(e)
 end
 if ai_bot_state[e] == ai_state_recover then
  module_combatcore.donotmove(e) 
  if GetTimer(e) > 100 then
   ai_bot_state[e] = resumestate
  end 
 end
end

function module_combatcore.fireonspot(e,AIObjNo)
 if ai_bot_state[e] == ai_state_startfireonspot then
  ai_bot_state[e] = ai_state_fireonspot
  StopAnimation(e)
  SetAnimation(0)
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
  StartTimer(e)
 end
 if ai_bot_state[e] == ai_state_fireonspot then
  if g_Entity[e]['plrvisible'] == 0 and GetTimer(e) > 500 then
		if ai_bot_state[e] ~= ai_state_startidle then 
			ai_bot_state[e] = ai_state_startidle
		end 
  else
   RotateToPlayer(e)
   module_combatcore.fireweapon(e)
   ai_bot_targetx[e] = g_PlayerPosX
   ai_bot_targety[e] = g_PlayerPosY
   ai_bot_targetz[e] = g_PlayerPosZ
   AIEntityGoToPosition(AIObjNo,ai_bot_targetx[e],ai_bot_targety[e],ai_bot_targetz[e]) 
  end 
  module_combatcore.donotmove(e)
 end
end

function module_combatcore.fireweapon(e)
 if ai_bot_state[e] ~= ai_state_startreload and ai_bot_state[e] ~= ai_state_reload and ai_bot_state[e] ~= ai_state_reloadsettle then
  if GetAmmoClip(e) > 0 then
   FireWeapon(e)
  else
   tReloadFinishFrame = GetEntityAnimationFinish(e,4)
   if tReloadFinishFrame > 0 then 
    ai_bot_state[e] = ai_state_startreload 
   end
  end
 end
end

function module_combatcore.reloadweapon(e)
 if ai_bot_state[e] == ai_state_startreload then
  ai_bot_state[e] = ai_state_reload
  SetAnimation(4)
  PlayAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
  tStart = GetEntityAnimationStart(e,4)
  SetAnimationFrame(e,tStart)
  StartTimer(e)
 else
  if ai_bot_state[e] == ai_state_reload then
   if GetTimer(e) > 500 then 
    tFrame = GetAnimationFrame(e)
    tStart = GetEntityAnimationStart(e,4)
    tFinish = GetEntityAnimationFinish(e,4)
	tDiffHalf = (tFinish - tStart) / 3
    if (tFrame >= tFinish - tDiffHalf and tFrame <= tFinish ) or tFinish == 0 then
     ai_bot_state[e] = ai_state_reloadsettle
     StartTimer(e)
    end 
   end
   module_combatcore.donotmove(e)
  else
   if ai_bot_state[e] == ai_state_reloadsettle then
    if GetTimer(e) > 750 then 
	 SetAmmoClip(e,GetAmmoClipMax(e))
     ai_bot_state[e] = ai_state_startidle
    end
    module_combatcore.donotmove(e)
   end
  end
 end
end

function module_combatcore.preexit(e,MoveType)
 tFrame = GetAnimationFrame(e)
 tDyingAnimStart = GetEntityAnimationStart(e,6)
 tDyingAnimFinish = GetEntityAnimationFinish(e,6)
 if tDyingAnimStart > 0 and tDyingAnimFinish > 0 then
  if ai_bot_state[e] ~= ai_state_preexit then
   ai_bot_state[e] = ai_state_preexit
   CharacterControlLimbo(e)
   StopAnimation(e)
   SetAnimation(6)
   PlayAnimation(e)
   SetAnimationFrame(e,tDyingAnimStart)
   SetAnimationSpeedModulation(e,1.0)
  end
 else 
  return 1
 end
 if tFrame >= tDyingAnimFinish-2 then
  return 1
 else
  return 0
 end 
end

function module_combatcore.exit(e)
 StopSound(e,0)
 StopSound(e,1)
 CollisionOff(e)
end

function module_combatcore.evasiveactions(e,AIObjNo,PlayerDist)
	if ai_bot_state[e] ~= ai_state_idle and ai_bot_state[e] ~= ai_state_move and ai_bot_state[e] ~= ai_state_startmove and ai_bot_state[e] ~= ai_state_roll and ai_bot_state[e] ~= ai_state_strafeleft and ai_bot_state[e] ~= ai_state_strafeleftstart and ai_bot_state[e] ~= ai_state_straferightstart and ai_bot_state[e] ~= ai_state_straferight and ai_bot_state[e] ~= ai_state_duckstart and ai_bot_state[e] ~= ai_state_duck and ai_bot_state[e] ~= ai_state_unduckstart then 
		if math.random(1,2) == 1 then 
			local pdist = GetPlayerDistance(e)
			if pdist > 200 then 
				if module_combatcore.playerlooking(e,pdist,10) == 1 then 
					local rays = module_combatcore.checkplayertoentityrays(pdist+5,0,0,60)
					if rays == g_Entity[e]['obj'] or rays == 0 then 
						local temp = math.random(1,6) --set max to 7 or higher to add a chance to roll
						if temp < 7 then 
							if module_combatcore.entitylookingatplayer(e,PlayerDist,10) ~= 1 then 
								temp = 99
							end 
						end 
						if ai_bot_last_sidestep[e] == nil then ai_bot_last_sidestep[e] = 0 end 
						if temp < 4 then 
							if ai_bot_last_sidestep[e] == 0 then 
								ai_bot_last_sidestep[e] = -15
								ai_bot_state[e] = ai_state_strafeleftstart
							else 
								if ai_bot_last_sidestep[e] < 0 then 
									ai_bot_last_sidestep[e] = ai_bot_last_sidestep[e] + 1
								else 
									ai_bot_last_sidestep[e] = ai_bot_last_sidestep[e] - 1
								end 
							end 
						elseif temp < 7 then 
							if ai_bot_last_sidestep[e] == 0 then 
								ai_bot_last_sidestep[e] = 15 
								ai_bot_state[e] = ai_state_straferightstart
							else 
								if ai_bot_last_sidestep[e] < 0 then 
									ai_bot_last_sidestep[e] = ai_bot_last_sidestep[e] + 1
								else 
									ai_bot_last_sidestep[e] = ai_bot_last_sidestep[e] - 1
								end 
							end 
						elseif temp < 8 then 
							ai_bot_state[e] = ai_state_rollstart							
						end 												
					end 
				end 
			end 
		end 
	end 
end

function module_combatcore.strafeleft(e,speedmod)
	if ai_bot_state[e] == ai_state_strafeleftstart then 
		ai_bot_state[e] = ai_state_checkforcover
		randomevade = math.random(45,90)*-1
		local dist = 120
		local rays = module_combatcore.checkentityrays(e,dist,randomevade,g_Entity[e]['obj'],20)
		if rays == 0 then 
			ai_bot_state[e] = ai_state_strafeleft
			ai_bot_substate[e] = 0
			SetAnimationFrame(e,GetEntityAnimationStart(e,15))
			SetAnimation(15)
			PlayAnimation(e)
			g_Entity[e]['animating'] = 15
			ModulateSpeed(e,1)
			SetAnimationSpeedModulation(e,1.5)
			ai_bot_roty[e] = AIGetEntityAngleY(AIObjNo)
			local temprot = AIGetEntityAngleY(AIObjNo)+randomevade
			SetRotation(e,0,temprot,0)
			ai_bot_newroty[e] = temprot
		end 
	end 
	if ai_bot_state[e] == ai_state_strafeleft then
		tFrame = GetAnimationFrame(e)
		tStart = GetEntityAnimationStart(e,15)
		tFinish = GetEntityAnimationFinish(e,15)
		if tFrame >= tStart+5 and tFrame < tFinish-8 and g_Entity[e]['animating'] == 15 then
			SetRotation(e,0,ai_bot_newroty[e],0)
			MoveForward(e,AIGetEntitySpeed(AIObjNo)*speedmod)
		end
		SetRotation(e,0,ai_bot_roty[e],0)
		AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
		if tFrame >= tFinish-2 and tFrame <= tFinish then
			ai_bot_state[e] = ai_state_startfireonspot
			SetAnimationSpeedModulation(e,1)
		end
	end
end
function module_combatcore.straferight(e,speedmod)
	if ai_bot_state[e] == ai_state_straferightstart then 
		ai_bot_state[e] = ai_state_checkforcover
		randomevade = math.random(45,90)
		local dist = 120
		local rays = module_combatcore.checkentityrays(e,dist,randomevade,g_Entity[e]['obj'],20)
		if rays == 0 then 
			ai_bot_state[e] = ai_state_straferight
			ai_bot_substate[e] = 0
			SetAnimationFrame(e,GetEntityAnimationStart(e,16))
			SetAnimation(16)
			PlayAnimation(e)
			g_Entity[e]['animating'] = 16
			ModulateSpeed(e,1)
			SetAnimationSpeedModulation(e,1.5)
			ai_bot_roty[e] = AIGetEntityAngleY(AIObjNo)
			local temprot = AIGetEntityAngleY(AIObjNo)+randomevade
			SetRotation(e,0,temprot,0)
			ai_bot_newroty[e] = temprot
		end 
	end
	if ai_bot_state[e] == ai_state_straferight then
		tFrame = GetAnimationFrame(e)
		tStart = GetEntityAnimationStart(e,16)
		tFinish = GetEntityAnimationFinish(e,16)
		if tFrame >= tStart+5 and tFrame < tFinish-8 then
			SetRotation(e,0,ai_bot_newroty[e],0)
			MoveForward(e,AIGetEntitySpeed(AIObjNo)*speedmod)
		end
		SetRotation(e,0,ai_bot_roty[e],0)
		AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
		if tFrame >= tFinish-2 and tFrame <= tFinish and g_Entity[e]['animating'] == 16 then
			ai_bot_state[e] = ai_state_startfireonspot
			SetAnimationSpeedModulation(e,1)
		end
	end
end
function module_combatcore.getangletopoint(e,x,z)
	if g_Entity[e] ~= nil and x > 0 and z > 0 then
		local destx = x - g_Entity[e]['x']
		local destz = z - g_Entity[e]['z']
		local angle = math.atan2(destx,destz)
		angle = angle * (180.0 / math.pi)
		if angle < 0 then
			angle = 360 + angle
		elseif angle > 360 then
			angle = angle - 360
		end
		return angle
	end
end
function module_combatcore.checkfiredrecently(e,window)
	if last_fired[e] == nil then last_fired[e] = g_Time end 
	local tfired = 0
	if g_Time > last_fired[e]+window then 
		tfired = 0
	else 
		tfired = 1
	end 
	return tfired
end
function module_combatcore.checkentityrays(e,dist,ang,obj,yoff)
	if g_Entity[e]['obj'] ~= nil then 
		if dist == nil then dist = 10 end 
		if ang == nil then ang = 0 end 
		if obj == nil then ang = 0 end 
		new_y = math.rad(GetEntityAngleY(e)+ang)
		x1 = GetEntityPositionX(e)
		y1 = GetEntityPositionY(e)+yoff
		z1 = GetEntityPositionZ(e)
		x2 = x1 + (math.sin(new_y) * dist)
		y2 = y1
		z2 = z1 + (math.cos(new_y) * dist)
		local hit = 0
		hit = RayTerrain(x1,y1,z1,x2,y2,z2)
		if hit == nil then hit = 0 end 
		if hit ~= 1 then 
			hit = IntersectAll(x1,y1,z1,x2,y2,z2,obj)
		end 
		if hit == nil then hit = 0 end 
		return hit
	end 
end
function module_combatcore.checkplayertoentityrays(dist,ang,obj,yoff)
	if dist == nil then dist = 10 end 
	if ang == nil then ang = 0 end 
	if obj == nil then ang = 0 end 
	new_y = math.rad(g_PlayerAngY)+ang
	x1 = g_PlayerPosX
	y1 = g_PlayerPosY+yoff
	z1 = g_PlayerPosZ
	x2 = x1 + (math.sin(new_y) * dist)
	y2 = y1
	z2 = z1 + (math.cos(new_y) * dist)
	local hit = 0
	hit = RayTerrain(x1,y1,z1,x2,y2,z2)
	if hit == nil then hit = 0 end 
	if hit ~= 1 then 
		hit = IntersectAll(x1,y1,z1,x2,y2,z2,obj)
	end 
	if hit == nil then hit = 0 end 
	return hit
end
function module_combatcore.playerlooking(e,dis,v)
	if g_Entity[e] ~= nil then
		if dis == nil then
			dis = 3000
		end
		if v == nil then
			v = 0.5
		end
		if GetPlayerDistance(e) <= dis then
			local destx = g_Entity[e]['x'] - g_PlayerPosX
			local destz = g_Entity[e]['z'] - g_PlayerPosZ
			local angle = math.atan2(destx,destz)
			angle = angle * (180.0 / math.pi)

			if angle <= 0 then
				angle = 360 + angle
			elseif angle > 360 then
				angle = angle - 360
			end
			while g_PlayerAngY < 0 or g_PlayerAngY > 360 do
				if g_PlayerAngY <= 0 then
					g_PlayerAngY = 360 + g_PlayerAngY
				elseif g_PlayerAngY > 360 then
					g_PlayerAngY = g_PlayerAngY - 360
				end
			end
			local L = angle - v
			local R = angle + v
			if L <= 0 then
				L = 360 + L 
			elseif L > 360 then
				L = L - 360
			end
			if R <= 0 then
				R = 360 + R
			elseif R > 360 then
				R = R - 360
			end

			if (L < R and math.abs(g_PlayerAngY) > L and math.abs(g_PlayerAngY) < R) then
				return 1
			elseif (L > R and (math.abs(g_PlayerAngY) > L or math.abs(g_PlayerAngY) < R)) then
				return 1
			else
				return 0
			end
		else
			return 0
		end
	end
end
function module_combatcore.entitylookingatplayer(e,dis,v)
	if g_Entity[e] ~= nil then
		if dis == nil then
			dis = 3000
		end
		if v == nil then
			v = 0.5
		end
		local distx = g_PlayerPosX - g_Entity[e]['x']
		local disty = g_PlayerPosY - g_Entity[e]['y']
		local distz = g_PlayerPosZ - g_Entity[e]['z']
		local tdist = math.sqrt((distx^2)+(disty^2)+(distz^2))
		if tdist <= dis then
			local destx = distx
			local destz = distz
			local angle = math.atan2(destx,destz)
			angle = angle * (180.0 / math.pi)

			if angle <= 0 then
				angle = 360 + angle
			elseif angle > 360 then
				angle = angle - 360
			end
			while g_Entity[e]['angley'] < 0 or g_Entity[e]['angley'] > 360 do
				if g_Entity[e]['angley'] <= 0 then
					g_Entity[e]['angley'] = 360 + g_Entity[e]['angley']
				elseif g_Entity[e]['angley'] > 360 then
					g_Entity[e]['angley'] = g_Entity[e]['angley'] - 360
				end
			end
			local L = angle - v
			local R = angle + v
			if L <= 0 then
				L = 360 + L 
			elseif L > 360 then
				L = L - 360
			end
			if R <= 0 then
				R = 360 + R
			elseif R > 360 then
				R = R - 360
			end

			if (L < R and math.abs(g_Entity[e]['angley']) > L and math.abs(g_Entity[e]['angley']) < R) then
				return 1
			elseif (L > R and (math.abs(g_Entity[e]['angley']) > L or math.abs(g_Entity[e]['angley']) < R)) then
				return 1
			else
				return 0
			end
		else
			return 0
		end
	end
end

return module_combatcore
