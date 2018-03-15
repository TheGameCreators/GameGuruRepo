-- COMBATCORE Common Module
-- anim0 : Idle
-- anim1 : Move
-- anim2 : Punch/Kick/Bite 
-- anim3 : Hurt
-- anim4 : Reload
-- sound0 : Start moving A
-- sound1 : Start strike
-- sound2 : Start moving B
-- sound3 : Get hurt A
-- sound4 : Get hurt B

module_agro = require "scriptbank\\ai\\module_agro"
module_cameraoverride = require "scriptbank\\ai\\module_cameraoverride"

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
end

function module_combatcore.findcover(e,x,y,z)
 CoverIndex = -1
 pClosest = 99999
 for ca = 1, AIGetTotalCover(), 1 do
  pDX = x - AICoverGetPointX(ca)
  pDZ = z - AICoverGetPointZ(ca)
  pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDZ*pDZ));
  if pDist < pClosest and pDist < 1000 then
   pClosest = pDist
   CoverIndex = ca
  end
 end
 if CoverIndex > -1 then
  tdx = x - g_PlayerPosX
  tdy = 0
  tdz = z - g_PlayerPosZ
  tbotdd = math.sqrt ( math.abs(tdx*tdx) + math.abs(tdz*tdz) )
  tdx = AICoverGetPointX(CoverIndex) - g_PlayerPosX
  tdy = 0
  tdz = AICoverGetPointZ(CoverIndex) - g_PlayerPosZ
  tcoverdd = math.sqrt ( math.abs(tdx*tdx) + math.abs(tdz*tdz) )
  if tbotdd < tcoverdd then
   CoverIndex = -1
  end
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
  if tcoverdd < 50 then
   atcoverpos = 1
  end
 end
 return atcoverpos
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
   if ai_bot_targetx[e] == nil then
    if ai_bot_coverindex[e] ~= -2 then
     ai_bot_coverindex[e] = module_combatcore.findcover(e,g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z'])
     if ai_bot_coverindex[e] > 0 then
      ai_bot_targetx[e] = AICoverGetPointX(ai_bot_coverindex[e])
      ai_bot_targety[e] = g_PlayerPosY
      ai_bot_targetz[e] = AICoverGetPointZ(ai_bot_coverindex[e])
     end
    end
    if ai_bot_coverindex[e] < 0 then
     ai_bot_coverindex[e] = -2
    end
   end
   if ai_bot_coverindex[e] == -2 and PlayerDist < AIGetEntityViewRange(AIObjNo) then
    ai_bot_targetx[e] = g_PlayerPosX
    ai_bot_targety[e] = g_PlayerPosY
    ai_bot_targetz[e] = g_PlayerPosZ
   end
  end
  if ai_bot_targetx[e] ~= nil then
   if ai_bot_substate[e] == 0 then
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

function module_combatcore.idle(e,AIObjNo,PlayerDist,CanFire,detectstate)
 CharacterControlManual(e)
 AISetEntityControl(AIObjNo,AI_MANUAL)
 if ai_bot_state[e] == ai_state_startidle then
  ai_bot_state[e] = ai_state_idle
  SetAnimation(0)
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
 end
 if ai_bot_state[e] == ai_state_idle then
  module_combatcore.detectplayer(e,AIObjNo,PlayerDist,CanFire,detectstate)
  module_combatcore.donotmove(e) 
 end 
end

function module_combatcore.patrol(e,AIObjNo,PlayerDist,MoveType,CanFire,detectstate,stopstate)
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
   SetAnimationSpeedModulation(e,0.0)
   StartTimer(e)
  else
   ai_bot_state[e] = ai_state_idle
   SetAnimation(0)
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
    SetAnimation(1)
    LoopAnimation(e)
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
 else
  SetAnimationSpeedModulation(e,1.0)
 end 
end

function module_combatcore.hunt(e,AIObjNo,PlayerDist,MoveType,CanFire,stopstate)
 if ai_bot_state[e] == ai_state_startmove then
  ai_bot_state[e] = ai_state_move
  SetAnimation(1)
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
  StartTimer(e)
 end
 if ai_bot_state[e] == ai_state_move then
  if GetTimer(e) > 500 then
   if ai_bot_coverindex[e] > 0 then
    if module_combatcore.atcover(e) == 1 then
     ai_bot_coverindex[e] = -2
    end
   end
   if ai_bot_coverindex[e] < 0 then
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
  end
 end
 if g_Entity[e]['plrvisible'] == 1 then 
  ai_bot_hunttime[e] = Timer() 
 else
  if ai_bot_hunttime[e] ~= nil then
   if ai_bot_hunttime[e] > 0 and Timer() > ai_bot_hunttime[e]+500 then
    ai_bot_state[e] = ai_state_startmove
    ai_bot_coverindex[e] = -1
    ai_bot_hunttime[e] = 0
    SetActivated(e,0)
   end
  end
 end
end

function module_combatcore.homein(e,AIObjNo,PlayerDist,MoveType,CanFire,stopstate)
 if ai_bot_state[e] == ai_state_startmove then
  ai_bot_state[e] = ai_state_move
  SetAnimation(1)
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
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
 if ai_bot_substate[e] == 0 then
  if PlayerDist < 100 then
   tDistX = x - g_Entity[e]['x']
   tDistZ = z - g_Entity[e]['z']
   tDA = math.atan2(tDistX,tDistZ)
   x = x + (math.sin(tDA) * 50)
   z = z + (math.cos(tDA) * 50)
  end
  AIEntityGoToPosition(AIObjNo,x,y,z)
  SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
  if AIGetEntityIsMoving(AIObjNo) == 1 then
   if MoveType == ai_movetype_useanim then
    MoveWithAnimation(e,1)
   else
    MoveForward(e,AIGetEntitySpeed(AIObjNo))
   end
  else
   MoveForward(e,0.0)
   if GetTimer(e) > 250 then
    ai_bot_state[e] = stopstate
   end
  end
  AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
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
 if ai_bot_substate[e] > 0 then
  SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
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
 if ai_bot_state[e] == ai_state_move or ai_bot_state[e] == ai_state_fireonspot then
  if PlayerDist < 70 and g_PlayerPosY > g_Entity[e]['y'] and g_PlayerPosY < g_Entity[e]['y']+70 then
   if combattype ~= ai_combattype_freezermelee or (combattype == ai_combattype_freezermelee and module_cameraoverride.hasowner() == 0) then
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
   else
    ai_bot_state[e] = ai_state_recoverstart
   end  
  end
 end
end
 
function module_combatcore.hurt(e,responsestate)
 if ai_bot_state[e] == ai_state_idle or ai_bot_state[e] == ai_state_patrol or ai_bot_state[e] == ai_state_move or ai_bot_state[e] == ai_state_fireonspot or ai_bot_state[e] == ai_state_recover or ai_bot_state[e] == ai_state_punch then
  if g_Entity[e]['health'] < ai_bot_oldhealth[e] then
   ai_bot_state[e] = ai_state_hurt
   ai_bot_oldhealth[e] = g_Entity[e]['health']
   ai_bot_angryhurt[e] = 1
   if ai_bot_coverindex[e] == -1 then ai_bot_coverindex[e] = -2 end
   SetAnimationSpeed(e,1.5)
   SetAnimation(3)
   PlayAnimation(e)
   SetAnimationSpeedModulation(e,1.0)
   PlaySound(e,math.random(3,4))
   module_agro.alertallwithinradius(g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z'],250.0)
   RotateToPlayer(e)
  end
 end
 if ai_bot_state[e] == ai_state_hurt then
  tFrame = GetAnimationFrame(e)
  tStart = GetEntityAnimationFinish(e,3)
  if tFrame >= tStart then
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
  if ai_bot_state[e] == ai_state_idle then
   RotateToPlayer(e)
  end
  if ai_bot_state[e] == ai_state_patrol then
   ai_bot_state[e] = ai_state_startidle
   RotateToPlayer(e)
  end
 end
end
 
function module_combatcore.punch(e,PlayerDist,combattype,afterstate)
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
    module_cameraoverride.finishbeingattacked(e)
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
   ai_bot_state[e] = ai_state_startidle
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
 if ai_bot_state[e] ~= ai_state_startreload and ai_bot_state[e] ~= ai_state_reload then
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
  StartTimer(e)
 else
  if ai_bot_state[e] == ai_state_reload then
   if GetTimer(e) > 500 then 
    tFrame = GetAnimationFrame(e)
    tFinish = GetEntityAnimationFinish(e,4)
    if tFrame >= tFinish or tFinish == 0 then
     ai_bot_state[e] = ai_state_startidle
     SetAmmoClip(e,GetAmmoClipMax(e))
    end 
   end
   module_combatcore.donotmove(e)
  end
 end
end

return module_combatcore
