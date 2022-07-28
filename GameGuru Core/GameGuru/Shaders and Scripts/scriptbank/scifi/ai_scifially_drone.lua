-- LUA Script - precede every function and global member with lowercase name of script

allyfriendly = {}
allyfound = {}
allyhover = {}
allyadvanced = {}
allyparticles = {}
lastenemyscan = {}

function ai_scifially_drone_init(e)
 CharacterControlManual(e)
 AIObjNo = g_Entity[e]['obj']
 AISetEntityControl(AIObjNo,AI_MANUAL)
 ai_soldier_state[e] = "waitingstart"
 allyfound[e] = 0
 allyhover[e] = 0
 -- set allyadvanced to 1 for SEPT18 PublicPreview onwards for particles and hover control
 allyadvanced[e] = 0
 lastenemyscan[e] = 0
 allyfriendly[e] = 1
 ai_old_health[e] = -1
 allyparticles[e] = 0
 if allyadvanced[e] == 1 then
  allyparticles[e] = ParticlesGetFreeEmitter()
  if ( allyparticles[e]>0 ) then 
   toffsetminx=-2
   toffsetminy=-10
   toffsetminz=-2
   toffsetmaxx=2
   toffsetmaxy=-15
   toffsetmaxz=2
   tscalestartmin=5
   tscalestartmax=10
   tscaleendmin=90
   tscaleendmax=100
   tmoveminx=0
   tmoveminy=-0.2
   tmoveminz=0
   tmovemaxx=0
   tmovemaxy=-0.3
   tmovemaxz=0
   trotatespeedmin=-0.1
   trotatespeedmax=0.1
   tlifemin=1000
   tlifemax=2000
   talphastartmin=40
   talphastartmax=75
   talphaendmin=0
   talphaendmax=0
   tfrequency=50
   tentlimbindex=0
   tparticleimage = ParticlesLoadImage("scriptbank\\scifi\\droneparticle.dds")
   tparticleframecount = 16
   ParticlesAddEmitterEx(allyparticles[e],1/64.0,1,toffsetminx,toffsetminy,toffsetminz,toffsetmaxx,toffsetmaxy,toffsetmaxz,tscalestartmin,tscalestartmax,tscaleendmin,tscaleendmax,tmoveminx,tmoveminy,tmoveminz,tmovemaxx,tmovemaxy,tmovemaxz,trotatespeedmin,trotatespeedmax,tlifemin,tlifemax,talphastartmin,talphastartmax,talphaendmin,talphaendmax,tfrequency,e,tentlimbindex,tparticleimage,tparticleframecount)
  end
 end
end

function ai_scifially_drone_enemydist(ee,e)
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

function ai_scifially_drone_main(e)
 PlayerDist = GetPlayerDistance(e)
 AIObjNo = g_Entity[e]['obj']
 if ai_soldier_state[e] == "waitingstart" then
  ModulateSpeed(e,1.0)
  ai_soldier_state[e] = "waiting"
 end
 if ai_soldier_state[e] == "waiting" then
  -- hover around player when waiting
  allyhover[e]=allyhover[e]+0.1
  if allyhover[e] > 6.28 then allyhover[e] = allyhover[e] - 6.28 end
  if allyadvanced[e] == 1 then SetHoverFactor(e,65+(math.cos(allyhover[e])*10)) else MoveUp(e,0) end
  if allyfound[e]==0 then
   if PlayerDist < AIGetEntityViewRange(AIObjNo) and g_Entity[e]['plrvisible'] == 1 then
    PlaySound(e,0)
    allyfound[e]=1 
   end
  else
   RotateToPlayerSlowly(e,10.0)  
   if GetPlayerDistance(e)>400 then
    ai_soldier_state[e] = "followingstart"
	allyfound[e]=2
   else
    if GetPlayerDistance(e)>200 then
     ai_soldier_state[e] = "followingstart"
	 allyfound[e]=1
    end
   end
  end
 end
 if ai_soldier_state[e] == "followingstart" then
  if allyfound[e] == 2 then
   ModulateSpeed(e,3.0)
  else
   ModulateSpeed(e,1.0)
  end
  ai_soldier_state[e] = "following"
  if allyparticles[e] > 0 then
   ParticlesSetScale(allyparticles[e],50,60,140,150)
   ParticlesSetLife(allyparticles[e],3000,4000,100,0)
  end
 end
 if ai_soldier_state[e] == "following" then
  if allyadvanced[e] == 1 then SetHoverFactor(e,65) else MoveUp(e,0) end
  allyhover[e]=6.28/4.0
  AIEntityGoToPosition(AIObjNo,g_PlayerPosX,g_PlayerPosY,g_PlayerPosZ) 
  if GetPlayerDistance(e)<150 or g_PlayerHealth<=0 then
   AIEntityGoToPosition(AIObjNo,g_Entity[e]['x'],g_Entity[e]['y'],g_Entity[e]['z']) 
   ai_soldier_state[e] = "waitingstart"
   if allyparticles[e] > 0 then
    ParticlesSetScale(allyparticles[e],5,10,90,100)
    ParticlesSetLife(allyparticles[e],1000,2000,100,0)
   end
  else
   RotateToPlayerSlowly(e,20.0)
   if AIGetEntityIsMoving(AIObjNo) == 1 then
    MoveForward(e,AIGetEntitySpeed(AIObjNo))
   end
   AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
  end
  if allyfound[e] == 1 and GetPlayerDistance(e)>400 then
   ai_soldier_state[e] = "followingstart"
   allyfound[e]=2
  end
  if allyfound[e] == 2 and GetPlayerDistance(e)<225 then
   ai_soldier_state[e] = "followingstart"
   allyfound[e]=1
  end
 end
 if ai_soldier_state[e] == "waiting" or ai_soldier_state[e] == "following" then
  if lastenemyscan[e] < 0 then
   ee = lastenemyscan[e]*-1
   if g_Entity[ee]['health'] <= 0 then
    lastenemyscan[e] = 0
   end   
  end
  if allyfound[e] ~= 0 and lastenemyscan[e] >= 0 then
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
  ModulateSpeed(e,3.0)
  ai_soldier_state[e] = "attackenemy"
 end
 if ai_soldier_state[e] == "attackenemy" then
  if allyadvanced[e] == 1 then SetHoverFactor(e,50) else MoveUp(e,0) end
  ee = lastenemyscan[e]*-1
  AIEntityGoToPosition(AIObjNo,g_Entity[ee]['x'],g_Entity[ee]['y'],g_Entity[ee]['z'] ) 
  ddd = ai_scifially_drone_enemydist(ee,e)
  if ddd < 90 then
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
     MoveForward(e,AIGetEntitySpeed(AIObjNo))
    end
    AISetEntityPosition(AIObjNo,GetEntityPositionX(e),GetEntityPositionY(e),GetEntityPositionZ(e))
   end
  end
 end
 if ai_soldier_state[e] == "attackstrikestart" then
  ModulateSpeed(e,1.0)
  PlaySound(e,2)
  ai_soldier_state[e] = "attackstrike"
  StartTimer(e)
 end
 if ai_soldier_state[e] == "attackstrike" then
  MoveUp(e,0.0)
  if lastenemyscan[e] < 0 then 
   ee = lastenemyscan[e]*-1
   ddx = g_Entity[ee]['x'] - g_Entity[e]['x']
   ddz = g_Entity[ee]['z'] - g_Entity[e]['z']
   dda = math.deg(math.atan2(ddx,ddz))
   SetRotation(e,0,dda,0)
   if GetTimer(e) > 500 then
    ai_soldier_state[e] = "attackenemystart"
    ddd = ai_scifially_drone_enemydist(ee,e)
    if ddd < 90 then
     PlaySoundIfSilent(e,2)
     if g_Entity[ee]['health'] > 25 then
      SetEntityHealth(ee,g_Entity[ee]['health']-25)
	 else
      SetEntityHealth(ee,0)
	  lastenemyscan[e] = 0
      ai_soldier_state[e] = "waitingstart"
 	 end
	end
   end
  else
   ai_soldier_state[e] = "waitingstart"
  end
 end
 if ai_old_health[e]==-1 then
  ai_old_health[e] = g_Entity[e]['health']
 end
 if g_Entity[e]['health'] < ai_old_health[e] then
  ai_old_health[e] = g_Entity[e]['health']
  PlaySound(e,3)
 end
 --PromptLocal ( e, ai_soldier_state[e] .. " target=" .. lastenemyscan[e] )
end

function ai_scifially_drone_exit(e)
 PlaySound(e,3)
 CollisionOff(e)
end
