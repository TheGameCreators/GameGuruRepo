-- LUA Script - precede every function and global member with lowercase name of script

allyfriendly = {}
lastroar = {}
lastswipe = {}
lastfidget = {}
lastenemyscan = {}

function ai_fantasyally_imp_init(e)
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 ai_soldier_state[e] = "waitingstart"
 lastroar[e] = 0
 lastswipe[e] = -1
 lastfidget[e] = 1000
 lastenemyscan[e] = 0
 allyfriendly[e] = 1
 ai_old_health[e] = -1
end

function ai_fantasyally_imp_enemydist(ee,e)
 if g_Entity[ee]['health'] > 0 then
  ddx = math.abs(g_Entity[ee]['x'] - g_Entity[e]['x'])
  ddy = math.abs(g_Entity[ee]['y'] - g_Entity[e]['y'])
  ddz = math.abs(g_Entity[ee]['z'] - g_Entity[e]['z'])
  ddd = math.sqrt((ddx*ddx)+(ddy*ddy)+(ddz*ddz))
 else
  ddd = 999999
 end
 return ddd
end

function ai_fantasyally_imp_main(e)
 lidlestart=10
 lidleend=100
 lfidgetstart=101
 lfidgetend=360
 lmovingstart=370
 lmovingend=420
 lrunningstart=565
 lrunningend=597
 lattackstart=995
 lattackend=1022
 ldamageframestart=1000
 ldamageframeend=1012
 PlayerDist = GetPlayerDistance(e)
 AIObjNo = g_Entity[e]['obj']
 if ai_soldier_state[e] == "waitingstart" then
  SetAnimationFrames(lidlestart,lidleend) 
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
  ModulateSpeed(e,1.0)
  ai_soldier_state[e] = "waiting"
 end
 if ai_soldier_state[e] == "waiting" then
  if lastroar[e]==0 then
   if PlayerDist < AIGetEntityViewRange(AIObjNo) and g_Entity[e]['plrvisible'] == 1 then
    PlaySound(e,0)
    StartTimer(e)
    lastroar[e]=1 
   end
  else
   RotateToPlayer(e)  
   if GetPlayerDistance(e)>400 then
    ai_soldier_state[e] = "followingstart"
	lastroar[e]=2
   else
    if GetPlayerDistance(e)>200 then
     ai_soldier_state[e] = "followingstart"
	 lastroar[e]=1
    end
   end
   if GetTimer(e) > lastfidget[e] then
    ai_soldier_state[e] = "fidgetstart"
	lastfidget[e] = math.random(5,10)*1000    
    StartTimer(e)
   end
  end
 end
 if ai_soldier_state[e] == "fidgetstart" then
  SetAnimationFrames(lfidgetstart,lfidgetend) 
  LoopAnimation(e)
  SetAnimationSpeedModulation(e,1.0)
  ModulateSpeed(e,1.0)
  ai_soldier_state[e] = "fidget"
 else
  if ai_soldier_state[e] == "fidget" then
   if GetAnimationFrame(e)>=lfidgetend-10 then
    ai_soldier_state[e] = "waitingstart"
   end
  end  
 end
 if ai_soldier_state[e] == "followingstart" then
  if lastroar[e] == 2 then
   SetAnimationFrames(lrunningstart,lrunningend) 
   SetAnimationSpeedModulation(e,0.33)
   ModulateSpeed(e,3.0)
  else
   SetAnimationFrames(lmovingstart,lmovingend) 
   SetAnimationSpeedModulation(e,1.0)
   ModulateSpeed(e,1.0)
  end
  LoopAnimation(e)
  ai_soldier_state[e] = "following"
 end
 if ai_soldier_state[e] == "following" then
  AIEntityGoToPosition(AIObjNo,g_PlayerPosX,g_PlayerPosY,g_PlayerPosZ) 
  if GetPlayerDistance(e)<150 or g_PlayerHealth<=0 then
   AIEntityGoToPosition(AIObjNo,g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z']) 
   ai_soldier_state[e] = "waitingstart"
  else
   SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
   if AIGetEntityIsMoving(AIObjNo) == 1 then
    MoveForward(e,AIGetEntitySpeed(AIObjNo))
   end
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  end
  if lastroar[e] == 1 and GetPlayerDistance(e)>400 then
   ai_soldier_state[e] = "followingstart"
   lastroar[e]=2
  end
  if lastroar[e] == 2 and GetPlayerDistance(e)<225 then
   ai_soldier_state[e] = "followingstart"
   lastroar[e]=1
  end
 end
 if ai_soldier_state[e] == "waiting" or ai_soldier_state[e] == "following" then
  if lastenemyscan[e] < 0 then
   ee = lastenemyscan[e]*-1
   if g_Entity[ee]['health'] <= 0 then
    lastenemyscan[e] = 0
   end   
  end
  if lastroar[e] ~= 0 and lastenemyscan[e] >= 0 then
   lastenemyscan[e] = lastenemyscan[e] + 1
   if lastenemyscan[e] > 100 then
    lastenemyscan[e] = 0
	lclosestee = -1
	lclosestdist = 99999
	lthirdpersone = GetGamePlayerControlThirdpersonCharactere()
    for ee = 1, g_EntityElementMax, 1 do
     if ee ~= e and ee ~= lthirdpersone and (ai_soldier_state[ee] ~= nil or ai_bot_state[ee] ~= nil) then 
	  if g_Entity[ee]['health'] > 0 then 
       ddx = math.abs(g_Entity[ee]['x'] - g_PlayerPosX)
       ddy = math.abs(g_Entity[ee]['y'] - g_PlayerPosY)
       ddz = math.abs(g_Entity[ee]['z'] - g_PlayerPosZ)
       ddd = math.sqrt((ddx*ddx)+(ddy*ddy)+(ddz*ddz))
       if ddd < 500 then
	    if allyfriendly[ee] == nil and ddd < lclosestdist then
		 lclosestdist = ddd
		 lclosestee = ee
		end
       end
	  end
	 end
    end
	if lclosestee > 0 then
     ai_soldier_state[e] = "attackenemystart"
     lastenemyscan[e] = lclosestee * -1
	end
   end
  end
 end
 if ai_soldier_state[e] == "attackenemystart" then
  SetAnimationFrames(lrunningstart,lrunningend) 
  SetAnimationSpeedModulation(e,0.33)
  ModulateSpeed(e,3.0)
  LoopAnimation(e)
  ai_soldier_state[e] = "attackenemy"
 end
 if ai_soldier_state[e] == "attackenemy" then
  ee = lastenemyscan[e]*-1
  AIEntityGoToPosition(AIObjNo,g_Entity[ee]['x'],g_Entity[ee]['y'],g_Entity[ee]['z'] ) 
  ddd = ai_fantasyally_imp_enemydist(ee,e)
  if ddd < 40 then
   AIEntityGoToPosition(AIObjNo,g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z']) 
   ai_soldier_state[e] = "attackstrikestart"
  else
   if ddd >= 500 then
    AIEntityGoToPosition(AIObjNo,g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z']) 
    ai_soldier_state[e] = "waitingstart"
	lastenemyscan[e] = 0
   else
    SetRotation(e,0,AIGetEntityAngleY(AIObjNo),0)
    if AIGetEntityIsMoving(AIObjNo) == 1 then
	 if ddd > 75 then
      MoveForward(e,AIGetEntitySpeed(AIObjNo))
	 else
      MoveForward(e,AIGetEntitySpeed(AIObjNo)*0.2)
	 end
    end
    AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   end
  end
 end
 if ai_soldier_state[e] == "attackstrikestart" then
  SetAnimationFrames(lattackstart,lattackend) 
  SetAnimationSpeedModulation(e,0.5)
  ModulateSpeed(e,1.0)
  LoopAnimation(e)
  PlaySound(e,1)
  ai_soldier_state[e] = "attackstrike"
  lastswipe[e] = 0
 end
 if ai_soldier_state[e] == "attackstrike" then
  if lastenemyscan[e] < 0 then 
   ee = lastenemyscan[e]*-1
   ddx = g_Entity[ee]['x'] - g_Entity[e]['x']
   ddz = g_Entity[ee]['z'] - g_Entity[e]['z']
   dda = math.deg(math.atan2(ddx,ddz))
   SetRotation(e,0,dda,0)
   if GetAnimationFrame(e)>ldamageframestart and GetAnimationFrame(e)<ldamageframeend then
    ddd = ai_fantasyally_imp_enemydist(ee,e)
    if ddd < 60 then
     PlaySoundIfSilent(e,2)
     if g_Entity[ee]['health'] > 2 then
      SetEntityHealth(ee,g_Entity[ee]['health']-2)
	 else
      SetEntityHealth(ee,0)
	  lastenemyscan[e] = 0
	 end
	end
   end
  end
  if lastswipe[e]==0 and GetAnimationFrame(e)<lattackend-1 then
   lastswipe[e]=1
  end
  if lastswipe[e]==1 and GetAnimationFrame(e)>=lattackend-1 then
   if lastenemyscan[e] < 0 then
    ai_soldier_state[e] = "attackenemystart"
   else
    ai_soldier_state[e] = "waitingstart"
   end
   StopAnimation(e)
   lastswipe[e]=2
  end
 end
 if ai_old_health[e]==-1 then
  ai_old_health[e] = g_Entity[e]['health']
 end
 if g_Entity[e]['health'] < ai_old_health[e] then
  ai_old_health[e] = g_Entity[e]['health']
  PlaySound(e,3)
 end
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
 --PromptLocal ( e, ai_soldier_state[e] .. " target=" .. lastenemyscan[e] )
end

function ai_fantasyally_imp_exit(e)
 PlaySound(e,3)
 CollisionOff(e)
end
