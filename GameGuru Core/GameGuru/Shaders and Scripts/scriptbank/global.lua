-- 
-- LUA Globals Common Header
--

-- Constants
AI_MANUAL    = 0 -- Preferred Mode With Direct Script Control 
AI_AUTOMATIC = 1 -- Discontinued Legacy Mode

-- Globals (built-in)
g_Entity = {}
g_EntityExtra = {}
g_DebugStringPeek = ""

-- New AI Globals
ai_state_startidle, ai_state_idle, ai_state_findpatrolpath, ai_state_startpatrol, ai_state_patrol, ai_state_startmove, ai_state_move, ai_state_avoid, ai_state_hurt, ai_state_punch, ai_state_recoverstart, ai_state_recover, ai_state_startfireonspot, ai_state_fireonspot, ai_state_startreload, ai_state_reload, ai_state_reloadsettle, ai_state_disable, ai_state_duckstart, ai_state_duck, ai_state_unduckstart, ai_state_unduck, ai_state_rollstart, ai_state_roll, ai_state_crouchdashstart, ai_state_crouchdash, ai_state_checkforcover, ai_state_strafeleftstart, ai_state_strafeleft, ai_state_straferightstart, ai_state_straferight, ai_state_preexit = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
ai_state_debug = { "startidle", "idle", "findpatrolpath", "startpatrol", "patrol", "startmove", "move", "avoid", "hurt", "punch", "recoverstart", "recover", "startfireonspot", "fireonspot", "startreload", "reload", "reloadsettle", "disable", "duckstart", "duck", "unduckstart", "unduck",  "rollstart", "roll", "crouchdashstart", "crouchdash", "ai_state_checkforcover", "strafeleftstart", "strafeleft", "straferightstart", "straferight", "preexit" }
ai_combattype_regular, ai_combattype_patrol, ai_combattype_guard, ai_combattype_freezermelee, ai_combattype_bashmelee = 0, 1, 2, 3, 4
ai_movetype_usespeed, ai_movetype_useanim = 0, 1
ai_attacktype_nofire, ai_attacktype_canfire = 0, 1
ai_duckstrafe_off, ai_duckstrafe_on = 0, 1
ai_bot_state = {}
ai_bot_substate = {}
ai_bot_oldhealth = {}
ai_bot_targetx = {}
ai_bot_targety = {}
ai_bot_targetz = {}
ai_bot_closeenoughx = {}
ai_bot_closeenoughy = {}
ai_bot_closeenoughz = {}
ai_bot_pathindex = {}
ai_bot_pointindex = {}
ai_bot_pointdirection = {}
ai_bot_pointmax = {}
ai_bot_pointtime = {}
ai_bot_coverindex = {}
ai_bot_angryhurt = {}
ai_bot_patroltime = {}
ai_bot_hunttime = {}
ai_bot_gofast = {}
ai_bot_sighting = {}
ai_bot_roty = {}
ai_bot_last_sidestep = {}
ai_bot_last_fired = {}
ai_cover_slot = {}

-- Weapon name global
weapon_name = {}

-- Globals (passed-in)
g_GameStateChange = 0
g_PlayerPosX = 0
g_PlayerPosY = 0
g_PlayerPosZ = 0
g_PlayerAngX = 0
g_PlayerAngY = 0
g_PlayerAngZ = 0
g_PlayerObjNo = 0
g_PlayerHealth = 0
g_PlayerLives = 0
g_PlayerFlashlight = 0
g_PlayerGunCount = 0
g_PlayerGunID = 0
g_PlayerGunName = ""
g_PlayerGunMode = 0
g_PlayerGunFired = 0
g_PlayerGunAmmoCount = 0
g_PlayerGunClipCount = 0
g_PlayerGunZoomed = 0
g_PlayerThirdPerson = 0
g_PlayerController = 0
g_PlayerFOV = 34
g_PlayerLastHitTime = 0
g_PlayerDeadTime = 0
g_Time = 0
g_TimeElapsed = 0
g_KeyPressE = 0
g_KeyPressQ = 0
g_KeyPressW = 0
g_KeyPressA = 0
g_KeyPressS = 0
g_KeyPressD = 0
g_KeyPressR = 0
g_KeyPressF = 0
g_KeyPressC = 0
g_KeyPressJ = 0
g_MouseClickControl = 0
g_KeyPressSPACE = 0
g_KeyPressSHIFT = 0
g_Scancode = 0
g_InKey = ""
g_LevelFilename = ""
g_MouseX = 0
g_MouseY = 0
g_MouseWheel = 0
g_MouseClick = 0
g_EntityElementMax = 0
g_PlayerUnderwaterMode = 0

-- Default Playable Map Size
g_mapsizeminx = 100
g_mapsizemaxx = 51100
g_mapsizeminz = 100
g_mapsizemaxz = 51100

function PlayableArea(minx,maxx,minz,maxz)
g_mapsizeminx = minx
g_mapsizemaxx = maxx
g_mapsizeminz = minz
g_mapsizemaxz = maxz
end

-- Checkpoint and soundloop states
g_CheckpointX = 0
g_CheckpointY = 0
g_CheckpointZ = 0
g_CheckpointAngle = 0
g_SoundState = {}

-- Mappable in-game keys
g_PlrKeyW = 0
g_PlrKeyA = 0
g_PlrKeyS = 0
g_PlrKeyD = 0
g_PlrKeyQ = 0
g_PlrKeyE = 0
g_PlrKeyF = 0
g_PlrKeyC = 0
g_PlrKeyZ = 0
g_PlrKeyR = 0
g_PlrKeySPACE = 0
g_PlrKeyRETURN = 0
g_PlrKeySHIFT = 0
g_PlrKeySHIFT2 = 0
g_PlrKeyJ = 0

-- Weapon stats
g_WeaponSlotGot = {}
g_WeaponSlotPref = {}
g_WeaponAmmo = {}
g_WeaponClipAmmo = {}
g_WeaponPoolAmmo = {}

-- Radar globals
g_objectiveCount = 0
radarObjectives = {}

-- Multiplayer
mp_isServer = 0
mp_playerNames = {}
mp_playerKills = {}
mp_playerDeaths = {}
mp_playerConnected = {}
mp_playerTeam = {}
mp_isConnectedToSteam = 0
mp_gameMode = 0
mp_servertimer = 0
mp_showscores = 0
mp_teambased = 0
mp_friendlyfireoff = 0
mp_me = 0;
mp_coop = 0;
mp_enemiesLeftToKill = 100;

-- GameLoop Globals, Init and Loop
g_gameloop_StartHealth = 0
g_gameloop_RegenRate = 0
g_gameloop_RegenSpeed = 0
g_gameloop_RegenDelay = 0

-- Globals to track projectile explosion event
g_projectileevent_explosion = 0
g_projectileevent_name = ""
g_projectileevent_x = 0
g_projectileevent_y = 0
g_projectileevent_z = 0
g_projectileevent_radius = 0
g_projectileevent_damage = 0
g_projectileevent_entityhit = 0

function GameLoopInit(sth,rra,rsp,rde)
 g_gameloop_StartHealth = sth
 g_gameloop_RegenRate = rra
 g_gameloop_RegenSpeed = rsp
 g_gameloop_RegenDelay = rde
 gameloop = require "scriptbank\\gameloop"
 gameloop.init()
end

function GlobalLoop(gameloopflag)
 -- monitor game state changes
 if gameloopflag == 1 then
  if g_GameStateChange > 0 then
   gamedata = require "titlesbank\\gamedata"
   if gamedata.load(g_GameStateChange) == 1 then
    if strLevelFilename ~= g_LevelFilename then
	 -- need to load correct level for save position first
	 JumpToLevel(strLevelFilename)
	else
	 RestoreGameFromSlot(0)
	 g_GameStateChange = 0
	 TriggerFadeIn()
     restoregame = require "titlesbank\\restoregame"
     restoregame.now()
	end
   end
  end
 end
 -- call per-game loop script (player health control)
 gameloop = require "scriptbank\\gameloop"
 gameloop.main()
end

function GameLoopQuit()
 gameloop = require "scriptbank\\gameloop"
 gameloop.quit()
end

function GameLoopSaveStats()
 file = io.open("savegames\\standalonelevelstats.dat", "w+")
 io.output(file)
 io.write("v1\n")
 io.write(g_PlayerHealth .. "\n")
 io.write(g_PlayerLives .. "\n")
 WeaponID = GetPlayerWeaponID()
 io.write(WeaponID .. "\n")
 for index = 1, 10, 1 do
  WeaponID = GetWeaponSlot ( index )
  io.write(WeaponID .. "\n")
  WeaponAmmoQty = GetWeaponAmmo ( index )
  io.write(WeaponAmmoQty .. "\n")
  WeaponAmmoClipQty = GetWeaponClipAmmo ( index )
  io.write(WeaponAmmoClipQty .. "\n")
  WeaponAmmoPoolQty = GetWeaponPoolAmmo ( index )
  io.write(WeaponAmmoPoolQty .. "\n")
 end
 io.close(file)
end

function GameLoopLoadStats()
 file = io.open("savegames\\standalonelevelstats.dat", "r")
 if file ~= nil then
  io.input(file)
  versionString = io.read()
  g_PlayerHealth = tonumber(io.read())
  g_PlayerLives = tonumber(io.read())
  SetPlayerHealth(g_PlayerHealth)
  SetPlayerLives(g_PlayerLives)
  CurrentlyHeldWeaponID = tonumber(io.read())
  for index = 1, 10, 1 do
   WeaponID = tonumber(io.read())
   if WeaponID > 0 then
    SetWeaponSlot ( index, WeaponID, WeaponID )
    ChangePlayerWeaponID(WeaponID)
   else
    SetWeaponSlot ( index, 0, 0 )
   end
   WeaponAmmoQty = tonumber(io.read())
   SetWeaponAmmo ( index, WeaponAmmoQty ) 
   WeaponAmmoClipQty = tonumber(io.read())
   SetWeaponClipAmmo ( index, WeaponAmmoClipQty )
   WeaponAmmoPoolQty = tonumber(io.read())
   SetWeaponPoolAmmo ( index, WeaponAmmoPoolQty )
  end
  io.close(file)
  ChangePlayerWeaponID(CurrentlyHeldWeaponID)
 end
end

function PlayerControl()
 gameplayercontrol = require "scriptbank\\gameplayercontrol"
 gameplayercontrol.main()
end

-- Common Updater Functions (called by Engine)

function UpdateEntitySMALL(e,object,x,y,z)
 g_Entity[e] = {x=x; y=y; z=z; obj=object;}
end

function UpdateEntity(e,object,x,y,z,rx,ry,rz,ave,act,col,key,zon,ezon,plrvis,ani,hea,frm,pdst,avd,lmb,lmi)
 g_Entity[e] = {x=x; y=y; z=z; anglex=rx; angley=ry; anglez=rz; active=ave; activated=act; animating=ani; collected=col; haskey=key; plrinzone=zon; entityinzone=ezon; plrvisible=plrvis; obj=object; health=hea; frame=frm; timer=0; plrdist=pdst; avoid=avd; limbhit=lmb; limbhitindex=lmi; }
 g_EntityExtra[e] = {visible=1; spawnatstart=1;}
end

function UpdateEntityRT(e,object,x,y,z,rx,ry,rz,ave,act,col,key,zon,ezon,plrvis,hea,frm,pdst,avd,lmb,lmi)
 g_Entity[e]['x'] = x;
 g_Entity[e]['y'] = y;
 g_Entity[e]['z'] = z;
 g_Entity[e]['anglex'] = rx;
 g_Entity[e]['angley'] = ry;
 g_Entity[e]['anglez'] = rz;
 g_Entity[e]['obj'] = object;
 g_Entity[e]['active'] = ave;
 g_Entity[e]['activated'] = act;
 g_Entity[e]['collected'] = col;
 g_Entity[e]['haskey'] = key;
 g_Entity[e]['plrinzone'] = zon;
 g_Entity[e]['entityinzone'] = ezon;
 g_Entity[e]['plrvisible'] = plrvis;
 g_Entity[e]['health'] = hea;
 g_Entity[e]['frame'] = frm;
 g_Entity[e]['plrdist'] = pdst;
 g_Entity[e]['avoid'] = avd;
 g_Entity[e]['limbhit'] = lmb;
 g_Entity[e]['limbhitindex'] = lmi;
end

function UpdateEntityAnimatingFlag(e,ani)
 if g_Entity[e] ~= nil then
  g_Entity[e]['animating'] = ani
 end
end

function UpdateWeaponStatsItem(mode,index,value)
 if mode==1 then g_WeaponSlotGot[index] = value end
 if mode==2 then g_WeaponSlotPref[index] = value end
 if mode==3 then g_WeaponAmmo[index] = value end
 if mode==4 then g_WeaponClipAmmo[index] = value end
 if mode==5 then g_WeaponPoolAmmo[index] = value end
end

-- Macro Functions
local sqrt = math.sqrt
local abs  = math.abs

function GetPlayerDistance( e )
    local Ent = g_Entity[ e ]
    local PDX, PDY, PDZ = Ent.x - g_PlayerPosX, 
                          Ent.y - g_PlayerPosY,
						  Ent.z - g_PlayerPosZ;
  
    if abs( PDY ) > 100 then PDY = PDY * 4 end
  
    return sqrt( PDX*PDX + PDY*PDY + PDZ*PDZ )
end

-- Common Action Functions (called by LUA)
function Prompt(str)
 SendMessageS("prompt",str);
end
function PromptDuration(str,v)
 SendMessageS("promptduration",v,str);
end
function PromptTextSize(v)
 SendMessageI("prompttextsize",v);
end
function PromptLocal(e,str)
 SendMessageS("promptlocal",e,str);
end
function PromptLocalForVR(e,str,vrmode)
 SendMessageF("promptlocalforvrmode",vrmode);
 SendMessageS("promptlocalforvr",e,str);
end

function GetFileExists(filename)
    local file = io.open(filename, "r")
    if file then
        file:close()
        return 1
    else
        return 0
    end
end

function SetFogNearest(v)
 SendMessageF("setfognearest",v)
end
function SetFogDistance(v)
 SendMessageF("setfogdistance",v)
end
function SetFogRed(v)
 SendMessageF("setfogred",v)
end
function SetFogGreen(v)
 SendMessageF("setfoggreen",v)
end
function SetFogBlue(v)
 SendMessageF("setfogblue",v)
end
function SetFogIntensity(v)
 SendMessageF("setfogintensity",v)
end
function SetAmbienceIntensity(v)
 SendMessageF("setambienceintensity",v)
end
function SetAmbienceRed(v)
 SendMessageF("setambiencered",v)
end
function SetAmbienceGreen(v)
 SendMessageF("setambiencegreen",v)
end
function SetAmbienceBlue(v)
 SendMessageF("setambienceblue",v)
end
function SetSurfaceIntensity(v)
 SendMessageF("setsurfaceintensity",v/33.0)
end
function SetSurfaceRed(v)
 SendMessageF("setsurfacered",v)
end
function SetSurfaceGreen(v)
 SendMessageF("setsurfacegreen",v)
end
function SetSurfaceBlue(v)
 SendMessageF("setsurfaceblue",v)
end
function SetSurfaceSunFactor(v)
 SendMessageF("setsurfacesunfactor",v)
end
function SetGlobalSpecular(v)
 SendMessageF("setglobalspecular",v)
end
function SetBrightness(v)
 -- range of -0.5(0%) to 0.5(100%)
 SendMessageF("setbrightness",v)
end
function SetContrast(v)
 -- range of 0(0%) to 3.33(100%)
 SendMessageF("setcontrast",v)
end
function SetPostBloom(v)
 SendMessageF("setpostbloom",v)
end
function SetPostSepia(v)
 SendMessageF("setsepia",v)
end
function SetPostSaturation(v)
 SendMessageF("setsaturation",v)
end
function SetPostVignetteRadius(v)
 SendMessageF("setpostvignetteradius",v)
end
function SetPostVignetteIntensity(v)
 SendMessageF("setpostvignetteintensity",v)
end
function SetPostMotionDistance(v)
 SendMessageF("setpostmotiondistance",v)
end
function SetPostMotionIntensity(v)
 SendMessageF("setpostmotionintensity",v)
end
function SetPostDepthOfFieldDistance(v)
 SendMessageF("setpostdepthoffielddistance",v)
end
function SetPostDepthOfFieldIntensity(v)
 SendMessageF("setpostdepthoffieldintensity",v)
end
function SetPostLightRayLength(v)
 SendMessageF("setpostlightraylength",v)
end
function SetPostLightRayQuality(v)
 SendMessageF("setpostlightrayquality",v)
end
function SetPostLightRayDecay(v)
 SendMessageF("setpostlightraydecay",v)
end
function SetPostSAORadius(v)
 SendMessageF("setpostsaoradius",v)
end
function SetPostSAOIntensity(v)
 SendMessageF("setpostsaointensity",v)
end
function SetPostLensFlareIntensity(v)
 SendMessageF("setpostlensflareintensity",v)
end
function SetOptionReflection(v)
 SendMessageF("setoptionreflection",v)
end
function SetOptionShadows(v)
 SendMessageF("setoptionshadows",v)
end
function SetOptionLightRays(v)
 SendMessageF("setoptionlightrays",v)
end
function SetOptionVegetation(v)
 SendMessageF("setoptionvegetation",v)
end
function SetOptionOcclusion(v)
 SendMessageF("setoptionocclusion",v)
end
function SetCameraPanelDistance(v)
 SendMessageF("setcameradistance",v)
end
function SetCameraPanelFOV(v)
 SendMessageF("setcamerafov",v)
end
function SetCameraPanelZoomPercentage(v)
 SendMessageF("setcamerazoompercentage",v)
end
function SetCameraPanelWeaponFOV(v)
 SendMessageF("setcameraweaponfov",v)
end
function SetTerrainLODNear(v)
 SendMessageF("setterrainlodnear",v)
end
function SetTerrainLODMid(v)
 SendMessageF("setterrainlodmid",v)
end
function SetTerrainLODFar(v)
 SendMessageF("setterrainlodfar",v)
end
function SetTerrainSize(v)
 SendMessageF("setterrainsize",v)
end
function SetVegetationQuantity(v)
 SendMessageF("setvegetationquantity",v)
end
function SetVegetationWidth(v)
 SendMessageF("setvegetationwidth",v)
end
function SetVegetationHeight(v)
 SendMessageF("setvegetationheight",v)
end

function JumpToLevelIfUsed(e)
 SendMessageS("jumptolevel",e,"")
end
function JumpToLevel(levelname)
 SendMessageS("jumptolevel",0,levelname)
end
function FinishLevel()
 SendMessageI("finishlevel",0);
end

function HideTerrain()
 SendMessageF("hideterrain",0)
end
function ShowTerrain()
 SendMessageF("showterrain",0)
end
function HideWater()
 SendMessageF("hidewater",0)
end
function ShowWater()
 SendMessageF("showwater",0)
end
function HideHuds()
 SendMessageF("hidehuds",0)
end
function ShowHuds()
 SendMessageF("showhuds",0)
end
function FreezeAI()
 SendMessageF("freezeai",0)
end
function UnFreezeAI()
 SendMessageF("unfreezeai",0)
end
function FreezePlayer()
 SendMessageF("freezeplayer",0)
end
function UnFreezePlayer()
 SendMessageF("unfreezeplayer",0)
end
function SetFreezePosition(x,y,z)
 SendMessageF("setfreezepositionx",x)
 SendMessageF("setfreezepositiony",y)
 SendMessageF("setfreezepositionz",z)
end
function SetFreezeAngle(ax,ay,az)
 SendMessageF("setfreezepositionax",ax)
 SendMessageF("setfreezepositionay",ay)
 SendMessageF("setfreezepositionaz",az)
end
function TransportToFreezePosition()
 SendMessageI("transporttofreezeposition",0)
end
function TransportToFreezePositionOnly()
 SendMessageI("transporttofreezeposition",1)
end
function ActivateMouse()
 SendMessageF("activatemouse",0)
end
function DeactivateMouse()
 SendMessageF("deactivatemouse",0)
end
function SetPlayerHealth(v)
 SendMessageF("setplayerhealth",v)
end
function SetPlayerLives(v)
 SendMessageF("setplayerlives",v)
end
function SetEntityHealth(e,v)
 SendMessageI("setentityhealth",e,v)
end
function SetEntityHealthSilent(e,v)
 SendMessageI("setentityhealthsilent",e,v)
end
function SetEntityRagdollForce(e,limb,x,y,z,v)
 SendMessageF("setforcex",x);
 SendMessageF("setforcey",y);
 SendMessageF("setforcez",z);
 SendMessageI("setforcelimb",e,limb);
 SendMessageI("ragdollforce",e,v);
end
function StartParticleEmitter(e)
 SendMessageF("startparticleemitter",e,0);
end
function StopParticleEmitter(e)
 SendMessageF("stopparticleemitter",e,0);
end

function StartTimer(e)
 g_Entity[e]['timer'] = g_Time
end
function GetTimer(e)
 return g_Time - g_Entity[e]['timer'];
end
function Destroy(e)
 SendMessageI("destroy",e);
end
function CollisionOn(e)
 SendMessageI("collisionon",e);
end
function CollisionOff(e)
 SendMessageI("collisionoff",e);
end
function GetEntityPlayerVisibility(e)
 SendMessageI("getentityplrvisible",e);
end
function GetEntityInZone(e)
 SendMessageI("getentityinzone",e);
end
function Hide(e)
 SendMessageI("hide",e);
end
function Show(e)
 SendMessageI("show",e);
end
function Spawn(e)
 SendMessageI("spawn",e);
end
function SetActivated(e,v)
 g_Entity[e]['activated'] = v
 SendMessageI("setactivated",e,v);
end
function ResetLimbHit(e)
 g_Entity[e]['limbhit'] = 0
 SendMessageI("resetlimbhit",e,0);
end
function ActivateIfUsed(e)
 SendMessageI("activateifused",e);
end
function SpawnIfUsed(e)
 SendMessageI("spawnifused",e);
end
function TransportToIfUsed(e)
 SendMessageI("transporttoifused",e);
end
function RefreshEntity(e)
 SendMessageI("refreshentity",e);
end
function Collected(e)
 SendMessageI("collected",e,1);
end
function MoveUp(e,v)
 SendMessageF("moveup",e,v);
end
function MoveDown( e, v )
MoveUp( e, -v )
end
function MoveForward(e,v)
 SendMessageF("moveforward",e,v);
end
function MoveBackward(e,v)
 SendMessageF("movebackward",e,v);
end
function SetHoverFactor(e,v)
 SendMessageF("sethoverfactor",e,v);
end

function SetPosition(e,x,y,z)
 SendMessageF("setpositionx",e,x);
 SendMessageF("setpositiony",e,y);
 SendMessageF("setpositionz",e,z);
end
function ResetPosition(e,x,y,z)
 SendMessageF("resetpositionx",e,x);
 SendMessageF("resetpositiony",e,y);
 SendMessageF("resetpositionz",e,z);
end
function SetRotation(e,x,y,z)
 SendMessageF("setrotationx",e,x);
 SendMessageF("setrotationy",e,y);
 SendMessageF("setrotationz",e,z);
end
function ResetRotation(e,x,y,z)
 SendMessageF("resetrotationx",e,x);
 SendMessageF("resetrotationy",e,y);
 SendMessageF("resetrotationz",e,z);
end
function ModulateSpeed(e,v)
 SendMessageF("modulatespeed",e,v);
end
function RotateX(e,v)
 SendMessageF("rotatex",e,v);
end
function RotateY(e,v)
 SendMessageF("rotatey",e,v);
end
function RotateZ(e,v)
 SendMessageF("rotatez",e,v);
end
function SetLimbIndex(e,v)
 SendMessageI("setlimbindex",e,v);
end
function RotateLimbX(e,v)
 SendMessageF("rotatelimbx",e,v);
end
function RotateLimbY(e,v)
 SendMessageF("rotatelimby",e,v);
end
function RotateLimbZ(e,v)
 SendMessageF("rotatelimbz",e,v);
end
function Scale(e,v)
 SendMessageF("scale",e,v);
end
function SetAnimation(e)
 SendMessageI("setanimation",e);
end
function SetAnimationFrames(e,v)
 SendMessageI("setanimationframes",e,v);
end
function PlayAnimation(e)
 SendMessageI("playanimation",e);
end
function LoopAnimation(e)
 SendMessageI("loopanimation",e);
end
function StopAnimation(e)
 SendMessageI("stopanimation",e);
end
function MoveWithAnimation(e,v)
 SendMessageI("movewithanimation",e,v);
end
function SetAnimationSpeed(e,v)
 SendMessageF("setanimationspeed",e,v);
end
function SetAnimationFrame(e,v)
 SendMessageF("setanimationframe",e,v);
end
function GetAnimationFrame(e)
 return g_Entity[e]['frame']
end

function GravityOff(e)
 SendMessageI("setnogravity",e,1);
end
function GravityOn(e)
 SendMessageI("setnogravity",e,0);
end
function LookAtPlayer(e)
 SendMessageI("lookatplayer",e);
end
function RotateToPlayerWithOffset(e,angleoffset)
 SendMessageF("rotatetoplayerwithoffset",e,angleoffset);
end
function RotateToPlayer(e)
 SendMessageI("rotatetoplayer",e,100);
end
function RotateToCamera(e)
 SendMessageI("rotatetocamera",e,100);
end
function RotateToPlayerSlowly(e,v)
 SendMessageI("rotatetoplayer",e,v);
end
function AddPlayerWeapon(e)
 SendMessageI("addplayerweapon",e);
end
function ChangePlayerWeapon(str)
 SendMessageS("changeplayerweapon",str);
end
function ChangePlayerWeaponID(id)
 SendMessageI("changeplayerweaponid",id);
end
function ReplacePlayerWeapon(e)
 SendMessageI("replaceplayerweapon",e);
end
function AddPlayerAmmo(e)
 SendMessageI("addplayerammo",e);
end
function AddPlayerHealth(e)
 SendMessageI("addplayerhealth",e);
end
function SetPlayerPower(e,v)
 SendMessageI("setplayerpower",e,v);
end
function AddPlayerPower(e,v)
 SendMessageI("addplayerpower",e,v);
end
function RemovePlayerWeapons(e)
 SendMessageI("removeplayerweapons",e);
end
function AddPlayerJetPack(e,fuel)
 SendMessageI("addplayerjetpack",e,fuel);
end
function Checkpoint(e)
 SendMessageI("checkpoint",e);
end
function GetPlayerInZone(e)
 return g_Entity[e]['plrinzone'];
end
function SetSound(e,v)
 SendMessageI("setsound",e,v);
end
function PlaySound(e,v)
 SendMessageI("playsound",e,v);
end
function PlaySoundIfSilent(e,v)
 SendMessageI("playsoundifsilent",e,v);
end
function PlayNon3DSound(e,v)
 SendMessageI("playnon3dsound",e,v);
end
function LoopNon3DSound(e,v)
 SendMessageI("loopnon3dsound",e,v);
end
function LoopSound(e,v)
 SendMessageI("loopsound",e,v);
end
function StopSound(e,v)
 SendMessageI("stopsound",e,v);
end
function SetSoundSpeed(freq)
 SendMessageI("setsoundspeed",freq);
end
function SetSoundVolume(vol)
 SendMessageI("setsoundvolume",vol);
end
function PlayVideo(e,v)
 SendMessageI("playvideo",e,v);
end
function PlayVideoNoSkip(e,v)
 SendMessageI("playvideonoskip",e,v);
end
function StopVideo(e,v)
 SendMessageI("stopvideo",e,v);
end
function FireWeapon(e)
 SendMessageI("fireweapon",e);
end
function FireWeaponInstant(e)
 SendMessageI("fireweaponinstant",e);
end
function HurtPlayer(e,v)
 SendMessageI("hurtplayer",e,v);
end
function DrownPlayer(e,v)
 SendMessageI("drownplayer",e,v);
end
function SwitchScript(e,str)
 SendMessageS("switchscript",e,str);
end
function SetCharacterSoundSet(e)
 SendMessageI("setcharactersoundset",e);
end
function SetCharacterSound(e,str)
 SendMessageS("setcharactersound",e,str);
end
function PlayCharacterSound(e,str)
 SendMessageS("playcharactersound",e,str);
end
function PlayCombatMusic(playTime,fadeTime)
 music_play_timecue(2,playTime,fadeTime);
end
function PlayFinalAssaultMusic(fadeTime)
 music_play_cue(3,fadeTime);
end
function DisableMusicReset(v)
 SendMessageI("disablemusicreset",v);
end
function HideLight(e)
 SendMessageI("setlightvisible",e,0);
end
function ShowLight(e)
 SendMessageI("setlightvisible",e,1);
end
function LoadImages(str,v)
 SendMessageS("loadimages",v,str);
end
function SetImagePosition(x,y)
 SendMessageF("setimagepositionx",x);
 SendMessageF("setimagepositiony",y);
end
function ShowImage(i)
 SendMessageI("showimage",i);
end
function HideImage(i)
 SendMessageI("hideimage",i);
end
function SetImageAlignment(i)
 SendMessageI("setimagealignment",i);
end
function Text(x,y,size,txt)
 SendMessageF("textx",x);
 SendMessageF("texty",y);
 SendMessageI("textsize",size);
 SendMessageS("texttxt",txt);
end
function TextCenterOnX(x,y,size,txt)
 SendMessageF("textx",x);
 SendMessageF("texty",y);
 SendMessageI("textsize",size);
 SendMessageI("textcenterx",0);
 SendMessageS("texttxt",txt);
end
function TextColor(x,y,size,txt,r,g,b)
 SendMessageF("textx",x);
 SendMessageF("texty",y);
 SendMessageI("textsize",size);
 SendMessageI("textred",r);
 SendMessageI("textgreen",g);
 SendMessageI("textblue",b);
 SendMessageS("texttxt",txt);
end
function TextCenterOnXColor(x,y,size,txt,r,g,b)
 SendMessageF("textx",x);
 SendMessageF("texty",y);
 SendMessageI("textsize",size);
 SendMessageI("textcenterx",0);
 SendMessageI("textred",r);
 SendMessageI("textgreen",g);
 SendMessageI("textblue",b);
 SendMessageS("texttxt",txt);
end
function Panel(x,y,x2,y2)
 SendMessageF("panelx",x);
 SendMessageF("panely",y);
 SendMessageF("panelx2",x2);
 SendMessageF("panely2",y2);
 SendMessageI("dopanel",0,0);
end
function PanelLocal(e,mode,x,y,x2,y2)
 SendMessageF("panelx",x);
 SendMessageF("panely",y);
 SendMessageF("panelx2",x2);
 SendMessageF("panely2",y2);
 SendMessageI("dopanel",e,mode);
end
function SetSkyTo(str)
 SendMessageS("setskyto",str);
end
function SetLutTo(str)
 SendMessageS("setlutto",str);
end

-- Common Multiplayer

function MP_IsServer()
 -- 1 = is the server, 0 = is not the server
 return mp_isServer;
end

function SetMultiplayerGameMode(mode)
 SendMessageI("mpgamemode",mode);
 mp_gameMode = mode;
end

function GetMultiplayerGameMode(mode)
 return mp_gameMode;
end

function SetServerTimer(t)
	SendMessageI("setservertimer",t);
	mp_servertimer = t;
end

function GetServerTimer(t)
	return mp_servertimer;
end

function GetServerTimerPassed()
	return os.time()-mp_servertimer;
end

function ServerRespawnAll()
	SendMessageI("serverrespawnall",0);
end

function ServerEndPlay()
	SendMessageI("serverendplay",0);
end

function GetShowScores()
	return mp_showscores;
end

function SetServerKillsToWin(v)
	SendMessageI("setserverkillstowin",v);
end

function GetInKey()
	return g_InKey;
end

function GetScancode()
	return g_Scancode;
end

function GetMultiplayerTeamBased()
	return mp_teambased;
end

function SetMultiplayerGameFriendlyFireOff()
	mp_friendlyfireoff = 1;
end

function SetNameplatesOff()
	SendMessageI("nameplatesoff",v);
end

function GetCoOpMode()
	return mp_coop
end

function GetCoOpEnemiesAlive()
	return mp_enemiesLeftToKill
end

-- TITLE/MENU/PAGE COMMANDS

function StartGame()
 SendMessage("startgame");
end
function LoadGame()
 SendMessage("loadgame");
end
function SaveGame()
 SendMessage("savegame");
end
function QuitGame()
 SendMessage("quitgame");
end
function LeaveGame()
 SendMessage("leavegame");
end
function ResumeGame()
 SendMessage("resumegame");
end
function SwitchPage(strPageName)
 SendMessageS("switchpage",strPageName);
end
function SwitchPageBack()
 SendMessage("switchpageback");
end
function LevelFilenameToLoad(strLevelName)
 SendMessageS("levelfilenametoload",strLevelName); 
end
function TriggerFadeIn()
 SendMessage("triggerfadein");
end

function SetGameQuality(v)
 SendMessageI("setgamequality",v);
end
function SetPlayerFOV(v)
 g_PlayerFOV = v
 SendMessageI("setplayerfov",v);
end
function SetGameSoundVolume(v)
 SendMessageI("setgamesoundvolume",v);
end
function SetGameMusicVolume(v)
 SendMessageI("setgamemusicvolume",v);
end
function SetLoadingResource(i,v)
 SendMessageI("setloadingresource",i,v);
end

-- Ancient LUA commands (superseded by removing hard coded systems)
function CharacterControlManual(e)
 SendMessageI("charactercontrolmanual",e);
end
function CharacterControlLimbo(e)
 SendMessageI("charactercontrollimbo",e);
end
function CharacterControlUnarmed(e)
 SendMessageI("charactercontrolunarmed",e);
end
function CharacterControlArmed(e)
 SendMessageI("charactercontrolarmed",e);
end
function CharacterControlFidget(e)
 SendMessageI("charactercontrolfidget",e);
end
function CharacterControlDucked(e)
 SendMessageI("charactercontrolducked",e);
end
function CharacterControlStand(e)
 SendMessageI("charactercontrolstand",e);
end
function SetCharacterToWalk(e)
 SendMessageI("setcharactertowalkrun",e,0);
end
function SetCharacterToRun(e)
 SendMessageI("setcharactertowalkrun",e,1);
end
function SetCharacterToStrafeLeft(e)
 SendMessageI("setcharactertostrafe",e,0);
end
function SetCharacterToStrafeRight(e)
 SendMessageI("setcharactertostrafe",e,1);
end
function SetCharacterVisionDelay(e,v)
 SendMessageI("setcharactervisiondelay",e,v);
end
function LockCharacterPosition(e,v)
 SendMessageI("setlockcharacter",e,1);
end
function UnlockCharacterPosition(e,v)
 SendMessageI("setlockcharacter",e,0);
end

--Ancient AI Globals (old legacy globals no longer supported with newer scripts)
AI_CLOSEST_TO_PLAYER = 50
ai_old_health = {}
ai_soldier_state = {}
ai_soldier_pathindex = {}
ai_combat_mode = {}
ai_combat_state_delay = {}
ai_combat_old_time = {}
ai_combat_turn_delay = 1
ai_combat_cover_delay = 4
ai_combat_delay_after_finding = 2
ai_next_aggro_delay = 0
ai_start_x = {}
ai_start_z = {}
ai_dest_x = {}
ai_dest_z = {}
ai_ran_to_cover = {}
ai_alerted_mode = {}
ai_alerted_state_delay = {}
ai_alerted_old_time = {}
ai_alerted_spoken = {}
ai_returning_home = {}
ai_alert_x = -10000
ai_alert_z = -10000
ai_alert_counter = 0
ai_alert_entity = 0
ai_path_point_index = {}
ai_path_point_direction = {}
ai_path_point_max = {}
ai_starting_heath = {}
ai_patrol_x = {}
ai_patrol_z = {}
ai_aggro_x = nil
ai_aggro_z = nil
ai_aggro_entity = nil
ai_aggro_range = 400
PlayerDist = 0

--[[

Direct call LUA Commands:

GetInternalSoundState: GetInternalSoundState( index ) -- Get the internal sound state of the sound at index
SetInternalSoundState: SetInternalSoundState( index ) -- Set and play/loop the internal sound state of the sound at index
SetCheckpoint: SetCheckpoint(x,y,z,angle) -- Refresh the current checkpoint used by the player with a new coordinate and angle

UpdateWeaponStats: UpdateWeaponStats() -- Call this to instantly update all g_Weapon* data
ResetWeaponSystems: ResetWeaponSystems ( ) -- resets any projectiles currently active in game

SetWeaponSlot: SetWeaponSlot ( index, got flag, preference flag ) -- Sets the weapon data directly, index is 1 through 10. The got flag value is the weapon ID of the weapon you have in that slot. By setting it, you can effectively grant the player that weapon without them having to pick it up, but you should ensure that weapon is placed somewhere in the level so it can load it the particulars.  The preference flag value also takes a Weapon ID and is used when you want to make sure when a weapon is collected, it will go to that slot, so  SetWeaponSlot ( 9, 0, 12 ) will make sure that when you collect Weapon ID 12 it will automatically be assigned to slot 9.

FirePlayerWeapon(#) -- set to 1 to force player to fire current weapon
GetWeaponSlotGot(weapon slot) - Returns weapon ID of slot number (1-9)
GetWeaponSlotNoSelect() -- To be documented.

GetWeaponAmmo: quantity = GetWeaponAmmo ( index ) -- Gets the weapon data directly, index is 1 through 10
SetWeaponAmmo: SetWeaponAmmo ( index, ammo quantity ) -- Sets the weapon data directly, index is 1 through 10
GetWeaponClipAmmo: quantity = GetWeaponClipAmmo ( index ) -- Gets the weapon data directly, index is 1 through 10
SetWeaponClipAmmo: SetWeaponClipAmmo ( index, clip quantity ) -- Sets the weapon data directly, index is 1 through 10
GetWeaponPoolAmmo: quantity = GetWeaponPoolAmmo ( index ) -- Gets the weapon data directly, index is 1 through 10
SetWeaponPoolAmmo: SetWeaponPoolAmmo ( index, pool ammo quantity ) -- Sets the weapon data directly, index is 1 through 10
GetWeaponSlot: GetWeaponSlot ( index ) -- Gets WeaponID from slot, index is 1 through 10
GetPlayerWeaponID: WeaponID = GetPlayerWeaponID ( ) -- Returns the WeaponID the player is currently carrying
GetWeaponID: GetWeaponID ( GunNameString ) -- Gets WeaponID associated with current GunNameString
GetEntityWeaponID: GetEntityWeaponID ( e ) -- Gets the WeaponID associated with the entity specified
SetWeaponDamage: SetWeaponDamage ( WeaponID, FireModeIndex, Value ) -- Sets damage value of weapon
SetWeaponAccuracy: SetWeaponAccuracy ( WeaponID, FireModeIndex, Value ) -- Sets accuracy value of weapon
SetWeaponReloadQuantity: SetWeaponReloadQuantity ( WeaponID, FireModeIndex, Value ) -- Sets reload quantity value of weapon
SetWeaponFireIterations: SetWeaponFireIterations ( WeaponID, FireModeIndex, Value ) -- Sets shot iterations value of weapon
SetWeaponRange: SetWeaponRange ( WeaponID, FireModeIndex, Value ) -- Sets range value of weapon
SetWeaponDropoff: SetWeaponDropoff ( WeaponID, FireModeIndex, Value ) -- Sets dropoff value of weapon
SetWeaponSpotLighting: SetWeaponSpotLighting ( WeaponID, FireModeIndex, Value ) -- Sets whether weapon uses spot light effect
GetWeaponDamage: Value = GetWeaponDamage ( WeaponID, FireModeIndex ) -- Get damage value of weapon
GetWeaponAccuracy: Value = GetWeaponAccuracy ( WeaponID, FireModeIndex ) -- Get accuracy value of weapon
GetWeaponReloadQuantity: Value = GetWeaponReloadQuantity ( WeaponID, FireModeIndex ) -- Get reload quantity value of weapon
GetWeaponFireIterations: Value = GetWeaponFireIterations ( WeaponID, FireModeIndex ) -- Get shot iterations value of weapon
GetWeaponRange: Value = GetWeaponRange ( WeaponID, FireModeIndex ) -- Get range value of weapon
GetWeaponDropoff: Value = GetWeaponDropoff ( WeaponID, FireModeIndex ) -- Get dropoff value of weapon
GetWeaponSpotLighting: Value = GetWeaponSpotLighting ( WeaponID, FireModeIndex ) -- Get whether weapon uses spot light effect

WrapAngle : V = WrapAngle ( Angle, Dest, Smoothing ) will smoothly change Angle to Dest, returning in V. Smoothing 0 to 1.
SetCameraOverride : SetCameraOverride ( i ) where i is 0-off, 1-position only, 2-angle only, 3-position and angle
SetCameraPosition : SetCameraPosition ( c, x, y, z ) where c should be zero and XYZ are 3D coordinates
SetCameraAngle : SetCameraAngle ( c, x, y, z ) where c should be zero and XYZ are euler angles
GetCameraPositionX : V = GetCameraPositionX ( c ) where V is the X coordinate of the specified camera
GetCameraPositionY : V = GetCameraPositionY ( c ) where V is the Y coordinate of the specified camera
GetCameraPositionZ : V = GetCameraPositionZ ( c ) where V is the Z coordinate of the specified camera
GetCameraAngleX : V = GetCameraAngleX ( c ) where V is the X angle of the specified camera
GetCameraAngleY : V = GetCameraAngleY ( c ) where V is the Y angle of the specified camera
GetCameraAngleZ : V = GetCameraAngleZ ( c ) where V is the Z angle of the specified camera

ForcePlayer : ForcePlayer ( angle, velocity ) where angle is the angle and physics velocity to push the player

SetEntityActive: SetEntityActive ( e, active flag ) -- Sets the entity data directly, e is entity index
SetEntityActivated: SetEntityActivated ( e, activated flag ) -- Sets the entity data directly, e is entity index
SetEntityCollected: SetEntityCollected ( e, collected flag ) -- Sets the entity data directly, e is entity index
SetEntityHasKey : SetEntityHasKey ( e, haskey flag ) -- Sets the entity data directly, e is entity index
GetEntityVisibility : vis = GetEntityVisibility ( e ) -- where e is the entity number and return vis (0-hidden,1-shown)
GetEntityActive : flag = GetEntityActive ( e ) -- returns 1 if the entity is active and not in process of dying
GetEntityPositionX : x = GetEntityPositionX ( e ) -- where e is the entity number and returns the X position
GetEntityPositionY : y = GetEntityPositionY ( e ) -- where e is the entity number and returns the Y position
GetEntityPositionZ : z = GetEntityPositionZ ( e ) -- where e is the entity number and returns the Z position
GetEntityAngleX : x = GetEntityAngleX ( e ) -- where e is the entity number and returns the X angle
GetEntityAngleY : y = GetEntityAngleY ( e ) -- where e is the entity number and returns the Y angle
GetEntityAngleZ : z = GetEntityAngleZ ( e ) -- where e is the entity number and returns the Z angle
GetAnimationSpeed : speed = GetAnimationSpeed ( e ) -- where e is the entity number and speed is the ANIMSPEED of the entity
SetAnimationSpeedModulation : SetAnimationSpeedModulation ( e, speed ) -- where e is the entity number and speed is animation speed modulator
GetAnimationSpeedModulation : speed = GetAnimationSpeedModulation ( e ) -- where e is the entity number and speed is the animation speed modulator
GetMovementDelta : delta = GetMovementDelta ( e ) -- where e is the entity number and delta is the movement distance since the last cycle

SetEntityString : SetEntityString ( e, slot, string, loadSound ) -- where e is the entity number and slot (0-4) to write the string into
                                                                 -- set loadSound to 1 to also relplace the sound sample in the specified
																 -- slot with the one pointed to by 'string'
GetEntityString : GetEntityString ( e, slot ) -- where e is the entity number and slot (0-4) is the sound slot index

GetLimbName : name = GetLimbName ( obj, limbindex ) -- returns name of limb specified by object number and limb index

GetEntitySpawnAtStart : state = GetEntitySpawnAtStart ( e ) -- returns the state of the spawn (0-dont spawn at start, 1-spawn at start, 2-spawned during game)
GetEntityFilePath : string = GetEntityFilePath ( e ) -- returns the entity file path to be used for helping inventory image systems
GetEntityAnimationStart : frame = GetEntityAnimationStart ( e, animsetindex ) -- returns frame (Y) stored in FPE under animX = Y,Z where X is animsetindex
GetEntityAnimationFinish : frame = GetEntityAnimationFinish ( e, animsetindex ) -- returns frame (Z) stored in FPE under animX = Y,Z where X is animsetindex
GetEntityAnimationFound : flag = GetEntityAnimationFound ( e, animsetindex ) -- returns a 1 if this value was set in FPE anim system (not CSI)
GetEntityFootfallMax : max = GetEntityFootfallMax ( e ) -- returns the max count of footfalls specified by entity FPE
GetEntityFootfallKeyframe : keyframe = GetEntityFootfallKeyframe ( e, footfallindex, leftorright ) -- returns the keyframe of the footfall specified, targeting left(0) or right(1) foot

GetAmmoClipMax : ammoclipmax = GetAmmoClipMax ( e ) -- returns the maximum units per ammo clip for the held weapon of the entity
GetAmmoClip : ammoclip = GetAmmoClip ( e ) -- returns the current ammo in clip for the held weapon of the entity
SetAmmoClip : SetAmmoClip ( e, ammoquantity ) -- sets the ammo clip quantity for the held weapon of the entity
	
FreezeEntity : FreezeEntity ( e, mode ) -- where e is the entity index and mode is reserved. Will freeze entity physics position
UnFreezeEntity : UnFreezeEntity ( e ) -- where e is the entity index to be unfrozen from a previous call to FreezeEntity

GetTerrainHeight : height = GetTerrainHeight(x,z) -- where X and Z are the coordinates on the terrain you want the height from

GetFogNearest : value = GetFogNearest ( ) -- gets the setting value currently used in the game
GetFogDistance : value = GetFogDistance ( ) -- gets the setting value currently used in the game
GetFogRed : value = GetFogRed ( ) -- gets the setting value currently used in the game
GetFogGreen : value = GetFogGreen ( ) -- gets the setting value currently used in the game
GetFogBlue : value = GetFogBlue ( ) -- gets the setting value currently used in the game
GetFogIntensity : value = GetFogIntensity ( ) -- gets the setting value currently used in the game
GetAmbienceIntensity : value = GetAmbienceIntensity ( ) -- gets the setting value currently used in the game
GetAmbienceRed : value = GetAmbienceRed ( ) -- gets the setting value currently used in the game
GetAmbienceGreen : value = GetAmbienceGreen ( ) -- gets the setting value currently used in the game
GetAmbienceBlue : value = GetAmbienceBlue ( ) -- gets the setting value currently used in the game
GetSurfaceRed : value = GetSurfaceRed ( ) -- gets the setting value currently used in the game
GetSurfaceGreen : value = GetSurfaceGreen ( ) -- gets the setting value currently used in the game
GetSurfaceBlue : value = GetSurfaceBlue ( ) -- gets the setting value currently used in the game
GetSurfaceIntensity : value = GetSurfaceIntensity ( ) -- gets the setting value currently used in the game
GetPostVignetteRadius : value = GetPostVignetteRadius ( ) -- gets the setting value currently used in the game
GetPostVignetteIntensity : value = GetPostVignetteIntensity ( ) -- gets the setting value currently used in the game
GetPostMotionDistance : value = GetPostMotionDistance ( ) -- gets the setting value currently used in the game
GetPostMotionIntensity : value = GetPostMotionIntensity ( ) -- gets the setting value currently used in the game
GetPostDepthOfFieldDistance : value = GetPostDepthOfFieldDistance ( ) -- gets the setting value currently used in the game
GetPostDepthOfFieldIntensity : value = GetPostDepthOfFieldIntensity ( ) -- gets the setting value currently used in the game

LoadImage: myImage = LoadImage ( "myFolder\\myImage.png" ) -- Anywhere inside GG Files folder (however it is best to put into scriptbank images folder for making standalones)
GetImageWidth: myWidth = GetImageWidth ( 1 ) -- get percentage width of image
GetImageHeight: myHeight = GetImageHeight ( 1 ) -- get percentage height of image
CreateSprite: mySprite = CreateSprite ( myImage )
PasteSprite: PasteSprite ( mySprite ) -- pastes the sprite at its current location (good for pasting sprite BEHIND text)
PasteSpritePosition: PasteSpritePosition ( mySprite , x , y ) -- as above but pasted at a specified XY coordinate
DeleteSprite: DeleteSprite ( mySprite )
SetSpritePosition: SetSpritePosition ( mySprite , x , y )
SetSpriteSize: SetSpriteSize ( mySprite , sizeX , sizeZ ) -- passing -1 as one of the params ensure the sprite retains its aspect ratio
SetSpriteDepth: SetSpriteDepth ( mySprite , 10 ) (0 is front, 100 is back)
SetSpriteColor: SetSpriteColor ( mySprite , red , green, blue, alpha )
SetSpriteAngle: SetSpriteAngle ( mySprite , 180 )
SetSpriteOffset: SetSpriteOffset ( mySprite , 5 , -1 ) -- would be the centre for a 10% width image assigned to a sprite, passing -1 as one of the params ensure the sprite retains its aspect ratio
SetSpriteImage: SetSpriteImage ( mySprite , myImage )

LoadGlobalSound: LoadGlobalSound ( filename, iID ) -- where iID is an index greater than zero and filename points to a WAV file in your game installation	
DeleteGlobalSound: DeleteGlobalSound ( iID ) -- where iID is the index of the sound loaded in the load command
PlayGlobalSound: PlayGlobalSound ( iID ) -- where iID is the index of the sound to be played
LoopGlobalSound: LoopGlobalSound ( iID ) -- where iID is the index of the sound to be looped
StopGlobalSound: StopGlobalSound ( iID ) -- where iID is the index of the sound to be stopped
SetGlobalSoundSpeed: SetGlobalSoundSpeed ( iID, speed ) -- where iID is the index of the sound to change the speed of
SetGlobalSoundVolume: SetGlobalSoundVolume ( iID, volume ) -- where iID is the index of the sound to change the volume of
GetGlobalSoundExist: GetGlobalSoundExist ( iID ) -- where iID is the index of the sound to check existence of
GetGlobalSoundPlaying: GetGlobalSoundPlaying ( iID ) -- where iID is the index of the sound to check if playing
GetGlobalSoundLooping: GetGlobalSoundLooping ( iID ) -- where iID is the index of the sound to check if looping

PlayRawSound: PlayRawSound ( iID ) -- where iID is the raw engine sound index
LoopRawSound: LoopRawSound ( iID ) -- where iID is the raw engine sound index
StopRawSound: StopRawSound ( iID ) -- where iID is the raw engine sound index
SetRawSoundVolume: SetRawSoundVolume ( iID, iVolume ) -- where iID is the raw engine sound index and iVolume from 0 to 100
RawSoundExist: iFlag = RawSoundExist ( iID ) -- where iID is the raw engine sound index
RawSoundPlaying: iFlag = RawSoundPlaying ( iID ) -- where iID is the raw engine sound index

GetTimeElapsed: delta = GetTimeElapsed() -- returns the float time slice of the current game cycle

SetGamePlayerControlJetpackMode: SetGamePlayerControlJetpackMode ( iValue ) -- sets the player control data to specified value
GetGamePlayerControlJetpackMode: iValue = GetGamePlayerControlJetpackMode() -- gets the specified player control data value
SetGamePlayerControlJetpackFuel: SetGamePlayerControlJetpackFuel ( iValue ) -- sets the player control data to specified value
GetGamePlayerControlJetpackFuel: iValue = GetGamePlayerControlJetpackFuel() -- gets the specified player control data value
SetGamePlayerControlJetpackHidden: SetGamePlayerControlJetpackHidden ( iValue ) -- sets the player control data to specified value
GetGamePlayerControlJetpackHidden: iValue = GetGamePlayerControlJetpackHidden() -- gets the specified player control data value
SetGamePlayerControlJetpackCollected: SetGamePlayerControlJetpackCollected ( iValue ) -- sets the player control data to specified value
GetGamePlayerControlJetpackCollected: iValue = GetGamePlayerControlJetpackCollected() -- gets the specified player control data value
SetGamePlayerControlSoundStartIndex: SetGamePlayerControlSoundStartIndex ( iValue ) -- sets the player control data to specified value
GetGamePlayerControlSoundStartIndex: iValue = GetGamePlayerControlSoundStartIndex() -- gets the specified player control data value
SetGamePlayerControlJetpackParticleEmitterIndex: SetGamePlayerControlJetpackParticleEmitterIndex ( iValue ) -- sets the player control data to specified value
GetGamePlayerControlJetpackParticleEmitterIndex: iValue = GetGamePlayerControlJetpackParticleEmitterIndex() -- gets the specified player control data value
SetGamePlayerControlJetpackThrust: SetGamePlayerControlJetpackThrust ( iValue ) -- sets the player control data to specified value
GetGamePlayerControlJetpackThrust: iValue = GetGamePlayerControlJetpackThrust() -- gets the specified player control data value

SetPlayerWeapons: SetPlayerWeapons(0) disabled player weapons, SetPlayerWeapons(1) restores them
SetAttachmentVisible: SetAttachmentVisible(e,1). 1 sets the entities attachment to be visible (such as their weapon), 0 switches it off
SetFlashLight: SetFlashLight(1), 1 switches the flash light on, 0 off
SetFlashLightKeyEnabled: SetFlashLightKeyEnabled(1) - 1 for on, 0 for off. Disables the flash light key from being used
SetOcclusion: SetOcclusion(100), control the occluder from script! set the occlude from 0 (off) to 100 (max). 10 and under for minimal popping

SetFont: SetFont ( "myFont : 1) To change in game font. Fonts 1-3 are the default ones used.
NOTE: You can place your texture atlas bitmap font file in the Files\fontbank\ folder with the name FPSCR-Font-XX.png where XX is the unique name for your font. When you want to use it, simply call the command SetFont ( "XX", YY ) where XX is the unique name above and YY is the index you want to 'overwrite'. Remember to include the 'FPSCR-Font-XX-Subimages.fnt' file which describes the coordinates within the texture atlas image for the specific bitmap fonts contained therein.

GetFirstEntitySpawn: GetFirstEntitySpawn() -- useful for scripts (such as the radar) that need to deal with entities that can come into existence at any time (returns 0 if no new entity)
GetNextEntitySpawn: GetNextEntitySpawn() -- returns the next new spawn in the list, 0 if the end of the list is reached

GetDeviceWidth: GetDeviceWidth() -- returns the display width in pixels
GetDeviceHeight: GetDeviceHeight() -- returns the current display height in pixels

GetTimeElapsed: GetTimeElapsed() -- returns the time elapsed since the last cycle
GetKeyState: x=GetKeyState(y) -- returns 1 in x when keycode specified in y is pressed
Timer: x=Timer() -- returns the global time in milliseconds since the system started
MouseMoveX: x=MouseMoveX() -- returns the mouse delta since the last time it was called
MouseMoveY: y=MouseMoveY() -- returns the mouse delta since the last time it was called
GetDesktopWidth: GetDesktopWidth() -- returns the current desktop width
GetDesktopHeight: GetDesktopHeight() -- returns the current desktop height
CurveValue: x=CurveValue(dest,current,smooth) -- returns the smoothed value based on the smooth factor
CurveAngle: a=CurveAngle(dest,current,smooth) -- as CurveValue but handles angles from 0-360 degrees
PositionMouse: PositionMouse(x,y) -- repositions the hardware mouse pointer in real-time (screen size coords)
GetDynamicCharacterControllerDidJump: x=GetDynamicCharacterControllerDidJump() -- returns 1 if controller jumped
GetCharacterControllerDucking: x=GetCharacterControllerDucking() -- returns 1 if the player has been forced to duck
WrapValue: x=WrapValue(y) -- takes the value y and wraps it to an angle between 0-360 degrees
GetElapsedTime: x=GetElapsedTime() -- returns the elapsed delta time since the last game cycle
GetPlrObjectPositionX: x=GetPlrObjectPositionX() -- returns the raw x position of the visible player object
GetPlrObjectPositionY: x=GetPlrObjectPositionY() -- returns the raw y position of the visible player object
GetPlrObjectPositionZ: x=GetPlrObjectPositionZ() -- returns the raw z position of the visible player object
GetPlrObjectAngleX: x=GetPlrObjectAngleX() -- returns the raw x angle of the visible player object
GetPlrObjectAngleY: x=GetPlrObjectAngleY() -- returns the raw y angle of the visible player object
GetPlrObjectAngleZ: x=GetPlrObjectAngleZ() -- returns the raw z angle of the visible player object
GetGroundHeight: y=GetGroundHeight(x,z) -- returns the terrain height of the coordinates specified by x and z
NewXValue: x=NewXValue(current,angle,distance) -- projects a new x position from the specified angle and distance 
NewZValue: z=NewZValue(current,angle,distance) -- projects a new z position from the specified angle and distance
ControlDynamicCharacterController: ControlDynamicCharacterController(iObj, -- controls the physics capsule of the player
 fAngleY
 fAngleX
 fSpeed
 fJump
 fDucking
 fPushAngle
 fPushForce
 fThrustUpwards )
GetCharacterHitFloor: x=GetCharacterHitFloor() -- returns a value if the player hits the floor
GetCharacterFallDistance: x=GetCharacterFallDistance() -- returns the distance the player fell when it hit the floor
RayTerrain: r=RayTerrain(x,y,z,x2,y2,z2) -- returns 1 if the ray cast hits the terrain geometry
GetRayCollisionX: x=GetRayCollisionX() -- returns the X position of the terrain hit position
GetRayCollisionY: y=GetRayCollisionY() -- returns the Y position of the terrain hit position
GetRayCollisionZ: z=GetRayCollisionZ() -- returns the Z position of the terrain hit position
IntersectAll: x=IntersectAll(x1,y1,z1,x2,y2,z2,IgnoreObj) -- returns 1 if the ray cast hits ANY entity or lightmapped geometry
IntersectStatic: x=IntersectStatic(x1,y1,z1,x2,y2,z2,IgnoreObj) -- returns 1 if the ray cast hits non-animating entity or lightmapped geometry
GetIntersectCollisionX: x=GetIntersectCollisionX() -- returns the X position of the entity hit position
GetIntersectCollisionY: y=GetIntersectCollisionY() -- returns the Y position of the entity hit position
GetIntersectCollisionZ: z=GetIntersectCollisionZ() -- returns the Z position of the entity hit position
GetIntersectCollisionNX: x=GetIntersectCollisionNX() -- returns the X normal of the entity hit surface
GetIntersectCollisionNY: y=GetIntersectCollisionNY() -- returns the Y normal of the entity hit surface
GetIntersectCollisionNZ: z=GetIntersectCollisionNZ() -- returns the Z normal of the entity hit surface
PositionCamera: PositionCamera(cam,x,y,z) -- positions the specified camera at the xyz position
PointCamera: PointCamera(cam,x,y,z) -- angles the specified camera towards the xyz position
MoveCamera: MoveCamera(cam,step) -- moves the specified camera at its current angle by the step amount
GetObjectExist: x=GetObjectExist(obj) -- returns if the specified object exists
SetObjectFrame: SetObjectFrame(obj,x) -- sets the animation frame of the specified object 
GetObjectFrame: x=GetObjectFrame(obj) -- returns the animation frame of the specified object 
SetObjectSpeed: SetObjectSpeed(obj,x) -- sets the animation speed of the specified object 
GetObjectSpeed: x=GetObjectSpeed(obj) -- returns the animation speed of the specified object 
PositionObject: PositionObject(obj,x,y,z) -- sets the position of the specified object
RotateObject: RotateObject(obj,x,y,z) -- sets the angle of the specified object
GetObjectAngleX: x=GetObjectAngleX(obj) -- returns the X position of the specified object
GetObjectAngleY: y=GetObjectAngleY(obj) -- returns the Y position of the specified object
GetObjectAngleZ: z=GetObjectAngleZ(obj) -- returns the Z position of the specified object
RunCharLoop: RunCharLoop() -- runs the legacy animation system to control character index GetGamePlayerStateCharAnimIndex()
TriggerWaterRipple: TriggerWaterRipple(x,y,z) -- triggers a ripple decal animation at the xyz position 
PlayFootfallSound: snd=PlayFootfallSound(type,x,y,z,lastsnd) -- triggers footfall sound and returns raw sound index used
ResetUnderwaterState: ResetUnderwaterState() -- resets the underwater sub-system when player emerges from water
SetUnderwaterOn: SetUnderwaterOn() -- use this when the player goes underwater for visual changes
SetUnderwaterOff: SetUnderwaterOff() -- use this when the player goes above water for visual changes

------- Particle system commands --------- 
ParticlesGetFreeEmitter: emitterId = ParticlesGetFreeEmitter() -- where emitterId is the index of the particle emitter

ParticlesAddEmitter: 
	ParticlesAddEmitter(emitterId,               -- create a particle emitter with the following parameters..
						animationSpeed,          -- 
						startsOffRandomAngle,    -- 0 no, 1 yes
						offsetMinX,              -- All these look really daunting if you aren't aware of how particles systems work
						offsetMinY,              -- but basically they are all ranges of values between which the created particles 
						offsetMinZ,              -- will spawn with or end with, so for example if you specify a minimum x offset of 
						offsetMaxX,              -- -100 and a maximum x offset of +100 then all particles generated will be given a
						offsetMaxY,              -- random starting position within that range of either the player position or an 
						offsetMaxZ,              -- entity position (if using the ParticlesAddEmitterEx variant, see below)
						scaleStartMin,           --
						scaleStartMax,           --
						scaleEndMin,             --
						scaleEndMax,             --
						movementSpeedMinX,       --
						movementSpeedMinY,       --
						movementSpeedMinZ,       --
						movementSpeedMaxX,       --
						movementSpeedMaxY,       --
						movementSpeedMaxZ,       --
						rotateSpeedMinZ,         --
						rotateSpeedMaxZ,         --
						lifeMin,                 --
						lifeMax,                 --
						alphaStartMin,           -- Alpha range is 0 .. 100, i.e. 0 = invisible ... 100 = fully opaque
						alphaStartMax,           --
						alphaEndMin,             --
						alphaEndMax,             --
						frequency                -- frequency with which particles are spawned in milliseconds
					   )
 
ParticlesAddEmitterEx:  Same as ParticlesAddEmitter with 4 extra parameters (i.e. following the frequency parameter)
						entityid,				 -- particles will be spawned at the location of the given entity ( or -1 )
						limbindex,			     -- particles spawned at limb if applicable ( or 0 )
						particleimage,			 -- specified imageFile value will be used ( see ParticlesLoadImage )
						imageframe               -- for custom images specifies number of frames ( 64, 16 or 4 )
						
ParticlesDeleteEmitter: ParticlesDeleteEmitter(emitterId) -- where emitterId is the index of the particle emitter

ParticlesLoadImage:  Example imageFile = ParticlesLoadImage( "effectbank\\particles\\flowerpuff.dds" )
					Allows dynamic loading of custom image files which can then be passed into ParticlesAddEmitterEx
					Note: sprite sheets have to be square, e.g. 2x2 or 6x6 (4 frames and 36 frames respectively), maximum
					size is 64x64 (4096 frames)
					
ParticlesLoadEffect: Example effectId = ParticlesLoadEffect( "effectbank\\reloaded\\decal_basic_additive.fx", emitterId )
                    Changes the shader effect used for a given particle emitter

					
-- the following commands allow on-the-fly altering of specific particle creation parameters
ParticlesSpawnParticle( emitterId, xPos, yPos, zPos ) -- forces the emitter to spawn a particle at the specified position
ParticlesSetFrames( emitterId, animationSpeed, startFrame, endFrame )
ParticlesSetSpeed( emitterId, movementSpeedMinX, movementSpeedMinY, movementSpeedMinZ,
		                      movementSpeedMaxX, movementSpeedMaxY, movementSpeedMaxZ )
ParticlesSetOffset( emitterId,  offsetMinX, offsetMinY, offsetMinZ,
                                offsetMaxX, offsetMaxY, offsetMaxZ ) 
ParticlesSetScale( emitterId, scaleStartMin, scaleStartMax, scaleEndMin, scaleEndMax )  
ParticlesSetAlpha( emitterId, alphaStartMin, alphaStartMax, alphaEndMin, alphaEndMax )
ParticlesSetAngle( emitterId, xAngle, yAngle, zAngle )  - Euler angles 

-- the '0' parameters below are not yet fully implemented in the engine, basically they are placeholders for the future
ParticlesSetRotation:   ParticlesSetRotation( emitterId, 0, 0, rotateSpeedMinZ, 0, 0, rotateSpeedMaxZ )
ParticlesSetLife:		ParticlesSetLife( emitterId, lifeMin, lifeMax, maxParticles, 0, maxPerFrame ) 
-- maxParticles default is 100, maxPerFrame default is 50

-- This command mimics a basic wind effect, all particles will be effected by this ..
ParticlesSetWindVector: ParticlesSetWindVector( windX, windZ ) -- values are X & Z units per frame
-- unless specifically omitted sith this command
ParticlesSetNoWind( emitterId )  -- All particles created by the specified emitter will not be affected by wind.

-- This command mimics a basic gravity component
ParticlesSetGravity( emitterId, startG, endG ) -- values are Y units per frame

-----------------------------------------

GetGamePlayerControlJetpackMode: GetGamePlayerControlJetpackMode() -- command used by the default player control mechanism
GetGamePlayerControlJetpackFuel: GetGamePlayerControlJetpackFuel() -- command used by the default player control mechanism
GetGamePlayerControlJetpackHidden: GetGamePlayerControlJetpackHidden() -- command used by the default player control mechanism
GetGamePlayerControlJetpackCollected: GetGamePlayerControlJetpackCollected() -- command used by the default player control mechanism
GetGamePlayerControlSoundStartIndex: GetGamePlayerControlSoundStartIndex() -- command used by the default player control mechanism
GetGamePlayerControlJetpackParticleEmitterIndex: GetGamePlayerControlJetpackParticleEmitterIndex() -- command used by the default player control mechanism
GetGamePlayerControlJetpackThrust: GetGamePlayerControlJetpackThrust() -- command used by the default player control mechanism
GetGamePlayerControlStartStrength: GetGamePlayerControlStartStrength() -- command used by the default player control mechanism
GetGamePlayerControlIsRunning: GetGamePlayerControlIsRunning() -- command used by the default player control mechanism
GetGamePlayerControlFinalCameraAngley: GetGamePlayerControlFinalCameraAngley() -- command used by the default player control mechanism
GetGamePlayerControlCx: GetGamePlayerControlCx() -- command used by the default player control mechanism
GetGamePlayerControlCy: GetGamePlayerControlCy() -- command used by the default player control mechanism
GetGamePlayerControlCz: GetGamePlayerControlCz() -- command used by the default player control mechanism
GetGamePlayerControlBasespeed: GetGamePlayerControlBasespeed() -- command used by the default player control mechanism
GetGamePlayerControlCanRun: GetGamePlayerControlCanRun() -- command used by the default player control mechanism
GetGamePlayerControlMaxspeed: GetGamePlayerControlMaxspeed() -- command used by the default player control mechanism
GetGamePlayerControlTopspeed: GetGamePlayerControlTopspeed() -- command used by the default player control mechanism
GetGamePlayerControlMovement: GetGamePlayerControlMovement() -- command used by the default player control mechanism
GetGamePlayerControlMovey: GetGamePlayerControlMovey() -- command used by the default player control mechanism
GetGamePlayerControlLastMovement: GetGamePlayerControlLastMovement() -- command used by the default player control mechanism
GetGamePlayerControlFootfallCount: GetGamePlayerControlFootfallCount() -- command used by the default player control mechanism
GetGamePlayerControlLastMovement: GetGamePlayerControlLastMovement() -- command used by the default player control mechanism
GetGamePlayerControlGravityActive: GetGamePlayerControlGravityActive() -- command used by the default player control mechanism
GetGamePlayerControlPlrHitFloorMaterial: GetGamePlayerControlPlrHitFloorMaterial() -- command used by the default player control mechanism
GetGamePlayerControlUnderwater: GetGamePlayerControlUnderwater() -- command used by the default player control mechanism
GetGamePlayerControlJumpMode: GetGamePlayerControlJumpMode() -- command used by the default player control mechanism
GetGamePlayerControlJumpModeCanAffectVelocityCountdown: GetGamePlayerControlJumpModeCanAffectVelocityCountdown() -- command used by the default player control mechanism
GetGamePlayerControlSpeed: GetGamePlayerControlSpeed() -- command used by the default player control mechanism
GetGamePlayerControlAccel: GetGamePlayerControlAccel() -- command used by the default player control mechanism
GetGamePlayerControlSpeedRatio: GetGamePlayerControlSpeedRatio() -- command used by the default player control mechanism
GetGamePlayerControlWobble: GetGamePlayerControlWobble() -- command used by the default player control mechanism
GetGamePlayerControlWobbleSpeed: GetGamePlayerControlWobbleSpeed() -- command used by the default player control mechanism
GetGamePlayerControlWobbleHeight: GetGamePlayerControlWobbleHeight() -- command used by the default player control mechanism
GetGamePlayerControlJumpmax: GetGamePlayerControlJumpmax() -- command used by the default player control mechanism
GetGamePlayerControlPushangle: GetGamePlayerControlPushangle() -- command used by the default player control mechanism
GetGamePlayerControlPushforce: GetGamePlayerControlPushforce() -- command used by the default player control mechanism
GetGamePlayerControlFootfallPace : GetGamePlayerControlFootfallPace() -- command used by the default player control mechanism
GetGamePlayerControlFinalCameraAngley: GetGamePlayerControlFinalCameraAngley() -- command used by the default player control mechanism
GetGamePlayerControlLockAtHeight: GetGamePlayerControlLockAtHeight() -- command used by the default player control mechanism
GetGamePlayerControlControlHeight: GetGamePlayerControlControlHeight() -- command used by the default player control mechanism
GetGamePlayerControlControlHeightCooldown: GetGamePlayerControlControlHeightCooldown() -- command used by the default player control mechanism
GetGamePlayerControlStoreMovey: GetGamePlayerControlStoreMovey() -- command used by the default player control mechanism
GetGamePlayerControlPlrHitFloorMaterial: GetGamePlayerControlPlrHitFloorMaterial() -- command used by the default player control mechanism
GetGamePlayerControlHurtFall: GetGamePlayerControlHurtFall() -- command used by the default player control mechanism
GetGamePlayerControlLeanoverAngle: GetGamePlayerControlLeanoverAngle() -- command used by the default player control mechanism
GetGamePlayerControlLeanover: GetGamePlayerControlLeanover() -- command used by the default player control mechanism
GetGamePlayerControlCameraShake: GetGamePlayerControlCameraShake() -- command used by the default player control mechanism
GetGamePlayerControlFinalCameraAnglex: GetGamePlayerControlFinalCameraAnglex() -- command used by the default player control mechanism
GetGamePlayerControlFinalCameraAngley: GetGamePlayerControlFinalCameraAngley() -- command used by the default player control mechanism
GetGamePlayerControlFinalCameraAnglez: GetGamePlayerControlFinalCameraAnglez() -- command used by the default player control mechanism
GetGamePlayerControlCamRightMouseMode: GetGamePlayerControlCamRightMouseMode() -- command used by the default player control mechanism
GetGamePlayerControlCamCollisionSmooth: GetGamePlayerControlCamCollisionSmooth() -- command used by the default player control mechanism
GetGamePlayerControlCamCurrentDistance: GetGamePlayerControlCamCurrentDistance() -- command used by the default player control mechanism
GetGamePlayerControlCamDoFullRayCheck: GetGamePlayerControlCamDoFullRayCheck() -- command used by the default player control mechanism
GetGamePlayerControlLastGoodcx: GetGamePlayerControlLastGoodcx() -- command used by the default player control mechanism
GetGamePlayerControlLastGoodcy: GetGamePlayerControlLastGoodcy() -- command used by the default player control mechanism
GetGamePlayerControlLastGoodcz: GetGamePlayerControlLastGoodcz() -- command used by the default player control mechanism
GetGamePlayerControlCamDoFullRayCheck: GetGamePlayerControlCamDoFullRayCheck() -- command used by the default player control mechanism
GetGamePlayerControlFlinchx: GetGamePlayerControlFlinchx() -- command used by the default player control mechanism
GetGamePlayerControlFlinchy: GetGamePlayerControlFlinchy() -- command used by the default player control mechanism
GetGamePlayerControlFlinchz: GetGamePlayerControlFlinchz() -- command used by the default player control mechanism
GetGamePlayerControlFlinchCurrentx: GetGamePlayerControlFlinchCurrentx() -- command used by the default player control mechanism
GetGamePlayerControlFlinchCurrenty: GetGamePlayerControlFlinchCurrenty() -- command used by the default player control mechanism
GetGamePlayerControlFlinchCurrentz: GetGamePlayerControlFlinchCurrentz() -- command used by the default player control mechanism
GetGamePlayerControlFootfallType: GetGamePlayerControlFootfallType() -- command used by the default player control mechanism
GetGamePlayerControlRippleCount: GetGamePlayerControlRippleCount() -- command used by the default player control mechanism
GetGamePlayerControlLastFootfallSound: GetGamePlayerControlLastFootfallSound() -- command used by the default player control mechanism
GetGamePlayerControlInWaterState: GetGamePlayerControlInWaterState() -- command used by the default player control mechanism
GetGamePlayerControlDrownTimestamp: GetGamePlayerControlDrownTimestamp() -- command used by the default player control mechanism
GetGamePlayerControlDeadTime: GetGamePlayerControlDeadTime() -- command used by the default player control mechanism
GetGamePlayerControlSwimTimestamp: GetGamePlayerControlSwimTimestamp() -- command used by the default player control mechanism
GetGamePlayerControlRedDeathFog: GetGamePlayerControlRedDeathFog() -- command used by the default player control mechanism
GetGamePlayerControlHeartbeatTimeStamp: GetGamePlayerControlHeartbeatTimeStamp() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonEnabled: GetGamePlayerControlThirdpersonEnabled() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCharacterIndex: GetGamePlayerControlThirdpersonCharacterIndex() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraFollow: GetGamePlayerControlThirdpersonCameraFollow() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraFocus: GetGamePlayerControlThirdpersonCameraFocus() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCharactere: GetGamePlayerControlThirdpersonCharactere() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraFollow: GetGamePlayerControlThirdpersonCameraFollow() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonShotFired: GetGamePlayerControlThirdpersonShotFired() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraDistance: GetGamePlayerControlThirdpersonCameraDistance() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraSpeed: GetGamePlayerControlThirdpersonCameraSpeed() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraLocked: GetGamePlayerControlThirdpersonCameraLocked() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraHeight: GetGamePlayerControlThirdpersonCameraHeight() -- command used by the default player control mechanism
GetGamePlayerControlThirdpersonCameraShoulder: GetGamePlayerControlThirdpersonCameraShoulder() -- command used by the default player control mechanism
SetGamePlayerControlJetpackMode: SetGamePlayerControlJetpackMode() -- command used by the default player control mechanism
SetGamePlayerControlJetpackFuel: SetGamePlayerControlJetpackFuel() -- command used by the default player control mechanism
SetGamePlayerControlJetpackHidden: SetGamePlayerControlJetpackHidden() -- command used by the default player control mechanism
SetGamePlayerControlJetpackCollected: SetGamePlayerControlJetpackCollected() -- command used by the default player control mechanism
SetGamePlayerControlSoundStartIndex: SetGamePlayerControlSoundStartIndex() -- command used by the default player control mechanism
SetGamePlayerControlJetpackParticleEmitterIndex: SetGamePlayerControlJetpackParticleEmitterIndex() -- command used by the default player control mechanism
SetGamePlayerControlJetpackThrust: SetGamePlayerControlJetpackThrust() -- command used by the default player control mechanism
SetGamePlayerControlStartStrength: SetGamePlayerControlStartStrength() -- command used by the default player control mechanism
SetGamePlayerControlIsRunning: SetGamePlayerControlIsRunning() -- command used by the default player control mechanism
SetGamePlayerControlFinalCameraAngley: SetGamePlayerControlFinalCameraAngley() -- command used by the default player control mechanism
SetGamePlayerControlCx: SetGamePlayerControlCx() -- command used by the default player control mechanism
SetGamePlayerControlCy: SetGamePlayerControlCy() -- command used by the default player control mechanism
SetGamePlayerControlCz: SetGamePlayerControlCz() -- command used by the default player control mechanism
SetGamePlayerControlBasespeed: SetGamePlayerControlBasespeed() -- command used by the default player control mechanism
SetGamePlayerControlCanRun: SetGamePlayerControlCanRun() -- command used by the default player control mechanism
SetGamePlayerControlMaxspeed: SetGamePlayerControlMaxspeed() -- command used by the default player control mechanism
SetGamePlayerControlTopspeed: SetGamePlayerControlTopspeed() -- command used by the default player control mechanism
SetGamePlayerControlMovement: SetGamePlayerControlMovement() -- command used by the default player control mechanism
SetGamePlayerControlMovey: SetGamePlayerControlMovey() -- command used by the default player control mechanism
SetGamePlayerControlLastMovement: SetGamePlayerControlLastMovement() -- command used by the default player control mechanism
SetGamePlayerControlFootfallCount: SetGamePlayerControlFootfallCount() -- command used by the default player control mechanism
SetGamePlayerControlLastMovement: SetGamePlayerControlLastMovement() -- command used by the default player control mechanism
SetGamePlayerControlGravityActive: SetGamePlayerControlGravityActive() -- command used by the default player control mechanism
SetGamePlayerControlPlrHitFloorMaterial: SetGamePlayerControlPlrHitFloorMaterial() -- command used by the default player control mechanism
SetGamePlayerControlUnderwater: SetGamePlayerControlUnderwater() -- command used by the default player control mechanism
SetGamePlayerControlJumpMode: SetGamePlayerControlJumpMode() -- command used by the default player control mechanism
SetGamePlayerControlJumpModeCanAffectVelocityCountdown: SetGamePlayerControlJumpModeCanAffectVelocityCountdown() -- command used by the default player control mechanism
SetGamePlayerControlSpeed: SetGamePlayerControlSpeed() -- command used by the default player control mechanism
SetGamePlayerControlAccel: SetGamePlayerControlAccel() -- command used by the default player control mechanism
SetGamePlayerControlSpeedRatio: SetGamePlayerControlSpeedRatio() -- command used by the default player control mechanism
SetGamePlayerControlWobble: SetGamePlayerControlWobble() -- command used by the default player control mechanism
SetGamePlayerControlWobbleSpeed: SetGamePlayerControlWobbleSpeed() -- command used by the default player control mechanism
SetGamePlayerControlWobbleHeight: SetGamePlayerControlWobbleHeight() -- command used by the default player control mechanism
SetGamePlayerControlJumpmax: SetGamePlayerControlJumpmax() -- command used by the default player control mechanism
SetGamePlayerControlPushangle: SetGamePlayerControlPushangle() -- command used by the default player control mechanism
SetGamePlayerControlPushforce: SetGamePlayerControlPushforce() -- command used by the default player control mechanism
SetGamePlayerControlFootfallPace: SetGamePlayerControlFootfallPace() -- command used by the default player control mechanism
SetGamePlayerControlFinalCameraAngley: SetGamePlayerControlFinalCameraAngley() -- command used by the default player control mechanism
SetGamePlayerControlLockAtHeight: SetGamePlayerControlLockAtHeight() -- command used by the default player control mechanism
SetGamePlayerControlControlHeight: SetGamePlayerControlControlHeight() -- command used by the default player control mechanism
SetGamePlayerControlControlHeightCooldown: SetGamePlayerControlControlHeightCooldown() -- command used by the default player control mechanism
SetGamePlayerControlStoreMovey: SetGamePlayerControlStoreMovey() -- command used by the default player control mechanism
SetGamePlayerControlPlrHitFloorMaterial: SetGamePlayerControlPlrHitFloorMaterial() -- command used by the default player control mechanism
SetGamePlayerControlHurtFall: SetGamePlayerControlHurtFall() -- command used by the default player control mechanism
SetGamePlayerControlLeanoverAngle: SetGamePlayerControlLeanoverAngle() -- command used by the default player control mechanism
SetGamePlayerControlLeanover: SetGamePlayerControlLeanover() -- command used by the default player control mechanism
SetGamePlayerControlCameraShake: SetGamePlayerControlCameraShake() -- command used by the default player control mechanism
SetGamePlayerControlFinalCameraAnglex: SetGamePlayerControlFinalCameraAnglex() -- command used by the default player control mechanism
SetGamePlayerControlFinalCameraAngley: SetGamePlayerControlFinalCameraAngley() -- command used by the default player control mechanism
SetGamePlayerControlFinalCameraAnglez: SetGamePlayerControlFinalCameraAnglez() -- command used by the default player control mechanism
SetGamePlayerControlCamRightMouseMode: SetGamePlayerControlCamRightMouseMode() -- command used by the default player control mechanism
SetGamePlayerControlCamCollisionSmooth: SetGamePlayerControlCamCollisionSmooth() -- command used by the default player control mechanism
SetGamePlayerControlCamCurrentDistance: SetGamePlayerControlCamCurrentDistance() -- command used by the default player control mechanism
SetGamePlayerControlCamDoFullRayCheck: SetGamePlayerControlCamDoFullRayCheck() -- command used by the default player control mechanism
SetGamePlayerControlLastGoodcx: SetGamePlayerControlLastGoodcx() -- command used by the default player control mechanism
SetGamePlayerControlLastGoodcy: SetGamePlayerControlLastGoodcy() -- command used by the default player control mechanism
SetGamePlayerControlLastGoodcz: SetGamePlayerControlLastGoodcz() -- command used by the default player control mechanism
SetGamePlayerControlCamDoFullRayCheck: SetGamePlayerControlCamDoFullRayCheck() -- command used by the default player control mechanism
SetGamePlayerControlFlinchx: SetGamePlayerControlFlinchx() -- command used by the default player control mechanism
SetGamePlayerControlFlinchy: SetGamePlayerControlFlinchy() -- command used by the default player control mechanism
SetGamePlayerControlFlinchz: SetGamePlayerControlFlinchz() -- command used by the default player control mechanism
SetGamePlayerControlFlinchCurrentx: SetGamePlayerControlFlinchCurrentx() -- command used by the default player control mechanism
SetGamePlayerControlFlinchCurrenty: SetGamePlayerControlFlinchCurrenty() -- command used by the default player control mechanism
SetGamePlayerControlFlinchCurrentz: SetGamePlayerControlFlinchCurrentz() -- command used by the default player control mechanism
SetGamePlayerControlFootfallType: SetGamePlayerControlFootfallType() -- command used by the default player control mechanism
SetGamePlayerControlRippleCount: SetGamePlayerControlRippleCount() -- command used by the default player control mechanism
SetGamePlayerControlLastFootfallSound: SetGamePlayerControlLastFootfallSound() -- command used by the default player control mechanism
SetGamePlayerControlInWaterState: SetGamePlayerControlInWaterState() -- command used by the default player control mechanism
SetGamePlayerControlDrownTimestamp: SetGamePlayerControlDrownTimestamp() -- command used by the default player control mechanism
SetGamePlayerControlDeadTime: SetGamePlayerControlDeadTime() -- command used by the default player control mechanism
SetGamePlayerControlSwimTimestamp: SetGamePlayerControlSwimTimestamp() -- command used by the default player control mechanism
SetGamePlayerControlRedDeathFog: SetGamePlayerControlRedDeathFog() -- command used by the default player control mechanism
SetGamePlayerControlHeartbeatTimeStamp: SetGamePlayerControlHeartbeatTimeStamp() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonEnabled: SetGamePlayerControlThirdpersonEnabled() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCharacterIndex: SetGamePlayerControlThirdpersonCharacterIndex() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraFollow: SetGamePlayerControlThirdpersonCameraFollow() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraFocus: SetGamePlayerControlThirdpersonCameraFocus() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCharactere: SetGamePlayerControlThirdpersonCharactere() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraFollow: SetGamePlayerControlThirdpersonCameraFollow() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonShotFired: SetGamePlayerControlThirdpersonShotFired() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraDistance: SetGamePlayerControlThirdpersonCameraDistance() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraSpeed: SetGamePlayerControlThirdpersonCameraSpeed() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraLocked: SetGamePlayerControlThirdpersonCameraLocked() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraHeight: SetGamePlayerControlThirdpersonCameraHeight() -- command used by the default player control mechanism
SetGamePlayerControlThirdpersonCameraShoulder: SetGamePlayerControlThirdpersonCameraShoulder() -- command used by the default player control mechanism
SetGamePlayerStateGunMode: SetGamePlayerStateGunMode() -- command used by the default player control mechanism
GetGamePlayerStateGunMode: GetGamePlayerStateGunMode() -- command used by the default player control mechanism
SetGamePlayerStateFiringMode: SetGamePlayerStateFiringMode() -- command used by the default player control mechanism
GetGamePlayerStateFiringMode: GetGamePlayerStateFiringMode() -- returns a value of 1 when the LMB is pressed, even if the player has not got any weapons
GetGamePlayerStateWeaponAmmoIndex: GetGamePlayerStateWeaponAmmoIndex() -- command used by the default player control mechanism
GetGamePlayerStateAmmoOffset: GetGamePlayerStateAmmoOffset() -- command used by the default player control mechanism
GetGamePlayerStateGunMeleeKey: GetGamePlayerStateGunMeleeKey() -- command used by the default player control mechanism
GetGamePlayerStateBlockingAction: GetGamePlayerStateBlockingAction() -- command used by the default player control mechanism
SetGamePlayerStateGunShootNoAmmo: SetGamePlayerStateGunShootNoAmmo() -- command used by the default player control mechanism
GetGamePlayerStateGunShootNoAmmo: GetGamePlayerStateGunShootNoAmmo() -- command used by the default player control mechanism
SetGamePlayerStateUnderwater: SetGamePlayerStateUnderwater() -- command used by the default player control mechanism
GetGamePlayerStateUnderwater: GetGamePlayerStateUnderwater() -- command used by the default player control mechanism
SetGamePlayerStateRightMouseHold: SetGamePlayerStateRightMouseHold() -- command used by the default player control mechanism
GetGamePlayerStateRightMouseHold: GetGamePlayerStateRightMouseHold() -- command used by the default player control mechanism
SetGamePlayerStateXBOX: SetGamePlayerStateXBOX() -- command used by the default player control mechanism
GetGamePlayerStateXBOX: GetGamePlayerStateXBOX() -- command used by the default player control mechanism
JoystickX: JoystickX() -- returns a value between -1000 and +1000 representing X axis of controller
JoystickY: JoystickY() -- returns a value between -1000 and +1000 representing Y axis of controller
JoystickZ: JoystickZ() -- returns a value between -1000 and +1000 representing Trigger of controller
SetGamePlayerStateGunZoomMode: SetGamePlayerStateGunZoomMode() -- command used by the default player control mechanism
GetGamePlayerStateGunZoomMode: GetGamePlayerStateGunZoomMode() -- command used by the default player control mechanism
SetGamePlayerStateGunZoomMag: SetGamePlayerStateGunZoomMag() -- command used by the default player control mechanism
GetGamePlayerStateGunZoomMag: GetGamePlayerStateGunZoomMag() -- command used by the default player control mechanism
SetGamePlayerStateGunReloadNoAmmo: SetGamePlayerStateGunReloadNoAmmo() -- command used by the default player control mechanism
GetGamePlayerStateGunReloadNoAmmo: GetGamePlayerStateGunReloadNoAmmo() -- command used by the default player control mechanism
SetGamePlayerStatePlrReloading: SetGamePlayerStatePlrReloading() -- command used by the default player control mechanism
GetGamePlayerStatePlrReloading: GetGamePlayerStatePlrReloading() -- command used by the default player control mechanism
SetGamePlayerStateGunAltSwapKey1: SetGamePlayerStateGunAltSwapKey1() -- command used by the default player control mechanism
GetGamePlayerStateGunAltSwapKey1: GetGamePlayerStateGunAltSwapKey1() -- command used by the default player control mechanism
SetGamePlayerStateGunAltSwapKey2: SetGamePlayerStateGunAltSwapKey2() -- command used by the default player control mechanism
GetGamePlayerStateGunAltSwapKey2: GetGamePlayerStateGunAltSwapKey2() -- command used by the default player control mechanism
SetGamePlayerStateWeaponKeySelection: SetGamePlayerStateWeaponKeySelection() -- command used by the default player control mechanism
GetGamePlayerStateWeaponKeySelection: GetGamePlayerStateWeaponKeySelection() -- command used by the default player control mechanism
SetGamePlayerStateWeaponIndex: SetGamePlayerStateWeaponIndex() -- command used by the default player control mechanism
GetGamePlayerStateWeaponIndex: GetGamePlayerStateWeaponIndex() -- command used by the default player control mechanism
SetGamePlayerStateCommandNewWeapon: SetGamePlayerStateCommandNewWeapon() -- command used by the default player control mechanism
GetGamePlayerStateCommandNewWeapon: GetGamePlayerStateCommandNewWeapon() -- command used by the default player control mechanism
SetGamePlayerStateGunID: SetGamePlayerStateGunID() -- command used by the default player control mechanism
GetGamePlayerStateGunID: GetGamePlayerStateGunID() -- command used by the default player control mechanism
SetGamePlayerStateGunSelectionAfterHide: SetGamePlayerStateGunSelectionAfterHide() -- command used by the default player control mechanism
GetGamePlayerStateGunSelectionAfterHide: GetGamePlayerStateGunSelectionAfterHide() -- command used by the default player control mechanism
SetGamePlayerStateGunBurst: SetGamePlayerStateGunBurst() -- command used by the default player control mechanism
GetGamePlayerStateGunBurst: GetGamePlayerStateGunBurst() -- command used by the default player control mechanism
JoystickHatAngle: JoystickHatAngle() -- command used by the default player control mechanism
JoystickFireXL: JoystickFireXL() -- command used by the default player control mechanism
JoystickTwistX: JoystickTwistX() -- command used by the default player control mechanism
JoystickTwistY: JoystickTwistY() -- command used by the default player control mechanism
JoystickTwistZ: JoystickTwistZ() -- command used by the default player control mechanism
SetGamePlayerStatePlrZoomInChange: SetGamePlayerStatePlrZoomInChange() -- command used by the default player control mechanism
GetGamePlayerStatePlrZoomInChange: GetGamePlayerStatePlrZoomInChange() -- command used by the default player control mechanism
SetGamePlayerStatePlrZoomIn: SetGamePlayerStatePlrZoomIn() -- command used by the default player control mechanism
GetGamePlayerStatePlrZoomIn: GetGamePlayerStatePlrZoomIn() -- command used by the default player control mechanism
SetGamePlayerStateLuaActiveMouse: SetGamePlayerStateLuaActiveMouse() -- command used by the default player control mechanism
GetGamePlayerStateLuaActiveMouse: GetGamePlayerStateLuaActiveMouse() -- command used by the default player control mechanism
SetGamePlayerStateRealFov: SetGamePlayerStateRealFov() -- command used by the default player control mechanism
GetGamePlayerStateRealFov: GetGamePlayerStateRealFov() -- command used by the default player control mechanism
SetGamePlayerStateDisablePeeking: SetGamePlayerStateDisablePeeking() -- command used by the default player control mechanism
GetGamePlayerStateDisablePeeking: GetGamePlayerStateDisablePeeking() -- command used by the default player control mechanism
SetGamePlayerStatePlrHasFocus: SetGamePlayerStatePlrHasFocus() -- command used by the default player control mechanism
GetGamePlayerStatePlrHasFocus: GetGamePlayerStatePlrHasFocus() -- command used by the default player control mechanism
SetGamePlayerStateGameRunAsMultiplayer: SetGamePlayerStateGameRunAsMultiplayer() -- command used by the default player control mechanism
GetGamePlayerStateGameRunAsMultiplayer: GetGamePlayerStateGameRunAsMultiplayer() -- command used by the default player control mechanism
SetGamePlayerStateSteamWorksRespawnLeft: SetGamePlayerStateSteamWorksRespawnLeft() -- command used by the default player control mechanism
GetGamePlayerStateSteamWorksRespawnLeft: GetGamePlayerStateSteamWorksRespawnLeft() -- command used by the default player control mechanism
SetGamePlayerStateTabMode: SetGamePlayerStateTabMode() -- command used by the default player control mechanism
GetGamePlayerStateTabMode: GetGamePlayerStateTabMode() -- command used by the default player control mechanism
SetGamePlayerStateLowFpsWarning: SetGamePlayerStateLowFpsWarning() -- command used by the default player control mechanism
GetGamePlayerStateLowFpsWarning: GetGamePlayerStateLowFpsWarning() -- command used by the default player control mechanism
SetGamePlayerStateCameraFov: SetGamePlayerStateCameraFov() -- command used by the default player control mechanism
GetGamePlayerStateCameraFov: GetGamePlayerStateCameraFov() -- command used by the default player control mechanism
SetGamePlayerStateCameraFovZoomed: SetGamePlayerStateCameraFovZoomed() -- command used by the default player control mechanism
GetGamePlayerStateCameraFovZoomed: GetGamePlayerStateCameraFovZoomed() -- command used by the default player control mechanism
SetGamePlayerStateMouseInvert: SetGamePlayerStateMouseInvert() -- command used by the default player control mechanism
GetGamePlayerStateMouseInvert: GetGamePlayerStateMouseInvert() -- command used by the default player control mechanism
SetGamePlayerStateSlowMotion: SetGamePlayerStateSlowMotion() -- command used by the default player control mechanism
GetGamePlayerStateSlowMotion: GetGamePlayerStateSlowMotion() -- command used by the default player control mechanism
SetGamePlayerStateSmoothCameraKeys: SetGamePlayerStateSmoothCameraKeys() -- command used by the default player control mechanism
GetGamePlayerStateSmoothCameraKeys: GetGamePlayerStateSmoothCameraKeys() -- command used by the default player control mechanism
SetGamePlayerStateCamMouseMoveX: SetGamePlayerStateCamMouseMoveX() -- command used by the default player control mechanism
GetGamePlayerStateCamMouseMoveX: GetGamePlayerStateCamMouseMoveX() -- command used by the default player control mechanism
SetGamePlayerStateCamMouseMoveY: SetGamePlayerStateCamMouseMoveY() -- command used by the default player control mechanism
GetGamePlayerStateCamMouseMoveY: GetGamePlayerStateCamMouseMoveY() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilX: SetGamePlayerStateGunRecoilX() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilX: GetGamePlayerStateGunRecoilX() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilY: SetGamePlayerStateGunRecoilY() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilY: GetGamePlayerStateGunRecoilY() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilAngleX: SetGamePlayerStateGunRecoilAngleX() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilAngleX: GetGamePlayerStateGunRecoilAngleX() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilAngleY: SetGamePlayerStateGunRecoilAngleY() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilAngleY: GetGamePlayerStateGunRecoilAngleY() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilCorrectY: SetGamePlayerStateGunRecoilCorrectY() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilCorrectY: GetGamePlayerStateGunRecoilCorrectY() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilCorrectX: SetGamePlayerStateGunRecoilCorrectX() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilCorrectX: GetGamePlayerStateGunRecoilCorrectX() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilCorrectAngleY: SetGamePlayerStateGunRecoilCorrectAngleY() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilCorrectAngleY: GetGamePlayerStateGunRecoilCorrectAngleY() -- command used by the default player control mechanism
SetGamePlayerStateGunRecoilCorrectAngleX: SetGamePlayerStateGunRecoilCorrectAngleX() -- command used by the default player control mechanism
GetGamePlayerStateGunRecoilCorrectAngleX: GetGamePlayerStateGunRecoilCorrectAngleX() -- command used by the default player control mechanism
SetGamePlayerStateCamAngleX: SetGamePlayerStateCamAngleX() -- command used by the default player control mechanism
GetGamePlayerStateCamAngleX: GetGamePlayerStateCamAngleX() -- command used by the default player control mechanism
SetGamePlayerStateCamAngleY: SetGamePlayerStateCamAngleY() -- command used by the default player control mechanism
GetGamePlayerStateCamAngleY: GetGamePlayerStateCamAngleY() -- command used by the default player control mechanism
SetGamePlayerStatePlayerDucking: SetGamePlayerStatePlayerDucking() -- command used by the default player control mechanism
GetGamePlayerStatePlayerDucking: GetGamePlayerStatePlayerDucking() -- command used by the default player control mechanism
SetGamePlayerStateEditModeActive: SetGamePlayerStateEditModeActive() -- command used by the default player control mechanism
GetGamePlayerStateEditModeActive: GetGamePlayerStateEditModeActive() -- command used by the default player control mechanism
SetGamePlayerStatePlrKeyShift: SetGamePlayerStatePlrKeyShift() -- command used by the default player control mechanism
GetGamePlayerStatePlrKeyShift: GetGamePlayerStatePlrKeyShift() -- command used by the default player control mechanism
SetGamePlayerStatePlrKeyShift2: SetGamePlayerStatePlrKeyShift2() -- command used by the default player control mechanism
GetGamePlayerStatePlrKeyShift2: GetGamePlayerStatePlrKeyShift2() -- command used by the default player control mechanism
SetGamePlayerStatePlrKeyControl: SetGamePlayerStatePlrKeyControl() -- command used by the default player control mechanism
GetGamePlayerStatePlrKeyControl: GetGamePlayerStatePlrKeyControl() -- command used by the default player control mechanism
SetGamePlayerStateNoWater: SetGamePlayerStateNoWater() -- command used by the default player control mechanism
GetGamePlayerStateNoWater: GetGamePlayerStateNoWater() -- command used by the default player control mechanism
SetGamePlayerStateWaterlineY: SetGamePlayerStateWaterlineY() -- command used by the default player control mechanism
GetGamePlayerStateWaterlineY: GetGamePlayerStateWaterlineY() -- command used by the default player control mechanism
SetGamePlayerStateFlashlightKeyEnabled: SetGamePlayerStateFlashlightKeyEnabled() -- command used by the default player control mechanism
GetGamePlayerStateFlashlightKeyEnabled: GetGamePlayerStateFlashlightKeyEnabled() -- command used by the default player control mechanism
SetGamePlayerStateFlashlightControl: SetGamePlayerStateFlashlightControl() -- command used by the default player control mechanism
GetGamePlayerStateFlashlightControl: GetGamePlayerStateFlashlightControl() -- command used by the default player control mechanism
SetGamePlayerStateMoving: SetGamePlayerStateMoving() -- command used by the default player control mechanism
GetGamePlayerStateMoving: GetGamePlayerStateMoving() -- command used by the default player control mechanism
SetGamePlayerStateTerrainHeight: SetGamePlayerStateTerrainHeight() -- command used by the default player control mechanism
GetGamePlayerStateTerrainHeight: GetGamePlayerStateTerrainHeight() -- command used by the default player control mechanism
SetGamePlayerStateJetpackVerticalMove: SetGamePlayerStateJetpackVerticalMove() -- command used by the default player control mechanism
GetGamePlayerStateJetpackVerticalMove: GetGamePlayerStateJetpackVerticalMove() -- command used by the default player control mechanism
SetGamePlayerStateTerrainID: SetGamePlayerStateTerrainID() -- command used by the default player control mechanism
GetGamePlayerStateTerrainID: GetGamePlayerStateTerrainID() -- command used by the default player control mechanism
SetGamePlayerStateEnablePlrSpeedMods: SetGamePlayerStateEnablePlrSpeedMods() -- command used by the default player control mechanism
GetGamePlayerStateEnablePlrSpeedMods: GetGamePlayerStateEnablePlrSpeedMods() -- command used by the default player control mechanism
SetGamePlayerStateRiftMode: SetGamePlayerStateRiftMode() -- command used by the default player control mechanism
GetGamePlayerStateRiftMode: GetGamePlayerStateRiftMode() -- command used by the default player control mechanism
SetGamePlayerStatePlayerX: SetGamePlayerStatePlayerX() -- command used by the default player control mechanism
GetGamePlayerStatePlayerX: GetGamePlayerStatePlayerX() -- command used by the default player control mechanism
SetGamePlayerStatePlayerY: SetGamePlayerStatePlayerY() -- command used by the default player control mechanism
GetGamePlayerStatePlayerY: GetGamePlayerStatePlayerY() -- command used by the default player control mechanism
SetGamePlayerStatePlayerZ: SetGamePlayerStatePlayerZ() -- command used by the default player control mechanism
GetGamePlayerStatePlayerZ: GetGamePlayerStatePlayerZ() -- command used by the default player control mechanism
SetGamePlayerStateTerrainPlayerX: SetGamePlayerStateTerrainPlayerX() -- command used by the default player control mechanism
GetGamePlayerStateTerrainPlayerX: GetGamePlayerStateTerrainPlayerX() -- command used by the default player control mechanism
SetGamePlayerStateTerrainPlayerY: SetGamePlayerStateTerrainPlayerY() -- command used by the default player control mechanism
GetGamePlayerStateTerrainPlayerY: GetGamePlayerStateTerrainPlayerY() -- command used by the default player control mechanism
SetGamePlayerStateTerrainPlayerZ: SetGamePlayerStateTerrainPlayerZ() -- command used by the default player control mechanism
GetGamePlayerStateTerrainPlayerZ: GetGamePlayerStateTerrainPlayerZ() -- command used by the default player control mechanism
SetGamePlayerStateTerrainPlayerAX: SetGamePlayerStateTerrainPlayerAX() -- command used by the default player control mechanism
GetGamePlayerStateTerrainPlayerAX: GetGamePlayerStateTerrainPlayerAX() -- command used by the default player control mechanism
SetGamePlayerStateTerrainPlayerAY: SetGamePlayerStateTerrainPlayerAY() -- command used by the default player control mechanism
GetGamePlayerStateTerrainPlayerAY: GetGamePlayerStateTerrainPlayerAY() -- command used by the default player control mechanism
SetGamePlayerStateTerrainPlayerAZ: SetGamePlayerStateTerrainPlayerAZ() -- command used by the default player control mechanism
GetGamePlayerStateTerrainPlayerAZ: GetGamePlayerStateTerrainPlayerAZ() -- command used by the default player control mechanism
SetGamePlayerStateAdjustBasedOnWobbleY: SetGamePlayerStateAdjustBasedOnWobbleY() -- command used by the default player control mechanism
GetGamePlayerStateAdjustBasedOnWobbleY: GetGamePlayerStateAdjustBasedOnWobbleY() -- command used by the default player control mechanism
SetGamePlayerStateFinalCamX: SetGamePlayerStateFinalCamX() -- command used by the default player control mechanism
GetGamePlayerStateFinalCamX: GetGamePlayerStateFinalCamX() -- command used by the default player control mechanism
SetGamePlayerStateFinalCamY: SetGamePlayerStateFinalCamY() -- command used by the default player control mechanism
GetGamePlayerStateFinalCamY: GetGamePlayerStateFinalCamY() -- command used by the default player control mechanism
SetGamePlayerStateFinalCamZ: SetGamePlayerStateFinalCamZ() -- command used by the default player control mechanism
GetGamePlayerStateFinalCamZ: GetGamePlayerStateFinalCamZ() -- command used by the default player control mechanism
SetGamePlayerStateShakeX: SetGamePlayerStateShakeX() -- command used by the default player control mechanism
GetGamePlayerStateShakeX: GetGamePlayerStateShakeX() -- command used by the default player control mechanism
SetGamePlayerStateShakeY: SetGamePlayerStateShakeY() -- command used by the default player control mechanism
GetGamePlayerStateShakeY: GetGamePlayerStateShakeY() -- command used by the default player control mechanism
SetGamePlayerStateShakeZ: SetGamePlayerStateShakeZ() -- command used by the default player control mechanism
GetGamePlayerStateShakeZ: GetGamePlayerStateShakeZ() -- command used by the default player control mechanism
SetGamePlayerStateImmunity: SetGamePlayerStateImmunity() -- command used by the default player control mechanism
GetGamePlayerStateImmunity: GetGamePlayerStateImmunity() -- command used by the default player control mechanism
SetGamePlayerStateCharAnimIndex: SetGamePlayerStateCharAnimIndex() -- command used by the default player control mechanism
GetGamePlayerStateCharAnimIndex: GetGamePlayerStateCharAnimIndex() -- command used by the default player control mechanism

GetGamePlayerStatePlrLogic: GetGamePlayerStatePlrLogic() -- returns state of the player logic (FreezePlayer sets this to zero)

SetGamePlayerStateIsMelee: SetGamePlayerStateIsMelee() -- by default sets current weapon, can specify optional gunid and firemode
GetGamePlayerStateIsMelee: GetGamePlayerStateIsMelee() -- command used by the default player control mechanism
SetGamePlayerStateAlternate: SetGamePlayerStateAlternate() -- by default sets current weapon, can specify optional gunid and firemode
GetGamePlayerStateAlternate: GetGamePlayerStateAlternate() -- command used by the default player control mechanism
SetGamePlayerStateModeShareMags: SetGamePlayerStateModeShareMags() -- by default sets current weapon, can specify optional gunid and firemode
GetGamePlayerStateModeShareMags: GetGamePlayerStateModeShareMags() -- command used by the default player control mechanism
SetGamePlayerStateAlternateIsFlak: SetGamePlayerStateAlternateIsFlak() -- by default sets current weapon, can specify optional gunid and firemode
GetGamePlayerStateAlternateIsFlak: GetGamePlayerStateAlternateIsFlak() -- command used by the default player control mechanism
SetGamePlayerStateAlternateIsRay: SetGamePlayerStateAlternateIsRay() -- by default sets current weapon, can specify optional gunid and firemode
GetGamePlayerStateAlternateIsRay: GetGamePlayerStateAlternateIsRay() -- command used by the default player control mechanism
SetFireModeSettingsReloadQty: SetFireModeSettingsReloadQty() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsReloadQty: GetFireModeSettingsReloadQty() -- command used by the default player control mechanism
SetFireModeSettingsIsEmpty: SetFireModeSettingsIsEmpty() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsIsEmpty: GetFireModeSettingsIsEmpty() -- command used by the default player control mechanism
SetFireModeSettingsJammed: SetFireModeSettingsJammed() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsJammed: GetFireModeSettingsJammed() -- command used by the default player control mechanism
SetFireModeSettingsJamChance: SetFireModeSettingsJamChance() -- by default sets current weapon, can specify optional gunid and firemodem
GetFireModeSettingsJamChance: GetFireModeSettingsJamChance() -- command used by the default player control mechanism
SetFireModeSettingsMinTimer: SetFireModeSettingsMinTimer() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsMinTimer: GetFireModeSettingsMinTimer() -- command used by the default player control mechanism
SetFireModeSettingsAddTimer: SetFireModeSettingsAddTimer() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsAddTimer: GetFireModeSettingsAddTimer() -- command used by the default player control mechanism
SetFireModeSettingsShotsFired: SetFireModeSettingsShotsFired() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsShotsFired: GetFireModeSettingsShotsFired() -- command used by the default player control mechanism
SetFireModeSettingsCoolTimer: SetFireModeSettingsCoolTimer() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsCoolTimer: GetFireModeSettingsCoolTimer() -- command used by the default player control mechanism
SetFireModeSettingsOverheatAfter: SetFireModeSettingsOverheatAfter() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsOverheatAfter: GetFireModeSettingsOverheatAfter() -- command used by the default player control mechanism
SetFireModeSettingsJamChanceTime: SetFireModeSettingsJamChanceTime() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsJamChanceTime: GetFireModeSettingsJamChanceTime() -- command used by the default player control mechanism
SetFireModeSettingsCoolDown: SetFireModeSettingsCoolDown() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsCoolDown: GetFireModeSettingsCoolDown() -- command used by the default player control mechanism
SetFireModeSettingsNoSubmergedFire: SetFireModeSettingsNoSubmergedFire() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsNoSubmergedFire: GetFireModeSettingsNoSubmergedFire() -- command used by the default player control mechanism
SetFireModeSettingsSimpleZoom: SetFireModeSettingsSimpleZoom() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsSimpleZoom: GetFireModeSettingsSimpleZoom() -- command used by the default player control mechanism
SetFireModeSettingsForceZoomOut: SetFireModeSettingsForceZoomOut() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsForceZoomOut: GetFireModeSettingsForceZoomOut() -- command used by the default player control mechanism
SetFireModeSettingsZoomMode: SetFireModeSettingsZoomMode() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsZoomMode: GetFireModeSettingsZoomMode() -- command used by the default player control mechanism
SetFireModeSettingsSimpleZoomAnim: SetFireModeSettingsSimpleZoomAnim() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsSimpleZoomAnim: GetFireModeSettingsSimpleZoomAnim() -- command used by the default player control mechanism
SetFireModeSettingsPoolIndex: SetFireModeSettingsPoolIndex() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsPoolIndex: GetFireModeSettingsPoolIndex() -- command used by the default player control mechanism
SetFireModeSettingsPlrTurnSpeedMod: SetFireModeSettingsPlrTurnSpeedMod() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsPlrTurnSpeedMod: GetFireModeSettingsPlrTurnSpeedMod() -- command used by the default player control mechanism
SetFireModeSettingsZoomTurnSpeed: SetFireModeSettingsZoomTurnSpeed() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsZoomTurnSpeed: GetFireModeSettingsZoomTurnSpeed() -- command used by the default player control mechanism
SetFireModeSettingsPlrJumpSpeedMod: SetFireModeSettingsPlrJumpSpeedMod() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsPlrJumpSpeedMod: GetFireModeSettingsPlrJumpSpeedMod() -- command used by the default player control mechanism
SetFireModeSettingsPlrEmptySpeedMod: SetFireModeSettingsPlrEmptySpeedMod() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsPlrEmptySpeedMod: GetFireModeSettingsPlrEmptySpeedMod() -- command used by the default player control mechanism
SetFireModeSettingsPlrMoveSpeedMod: SetFireModeSettingsPlrMoveSpeedMod() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsPlrMoveSpeedMod: GetFireModeSettingsPlrMoveSpeedMod() -- command used by the default player control mechanism
SetFireModeSettingsZoomWalkSpeed: SetFireModeSettingsZoomWalkSpeed() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsZoomWalkSpeed: GetFireModeSettingsZoomWalkSpeed() -- command used by the default player control mechanism
SetFireModeSettingsPlrReloadSpeedMod: SetFireModeSettingsPlrReloadSpeedMod() -- default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsPlrReloadSpeedMod: GetFireModeSettingsPlrReloadSpeedMod() -- command used by the default player control mechanism
SetFireModeSettingsHasEmpty: SetFireModeSettingsHasEmpty() -- by default sets current weapon, can specify optional gunid and firemode
GetFireModeSettingsHasEmpty: GetFireModeSettingsHasEmpty() -- command used by the default player control mechanism
GetFireModeSettingsActionBlockStart: GetFireModeSettingsActionBlockStart() -- command used by the default player control mechanism

SetGamePlayerStateGunSound: SetGamePlayerStateGunSound() -- command used by the default player control mechanism
GetGamePlayerStateGunSound: GetGamePlayerStateGunSound() -- command used by the default player control mechanism
SetGamePlayerStateGunAltSound: SetGamePlayerStateGunAltSound() -- command used by the default player control mechanism
GetGamePlayerStateGunAltSound: GetGamePlayerStateGunAltSound() -- command used by the default player control mechanism
CopyCharAnimState: CopyCharAnimState() -- command used by the default player control mechanism
SetCharAnimStatePlayCsi: SetCharAnimStatePlayCsi() -- command used by the default player control mechanism
GetCharAnimStatePlayCsi: GetCharAnimStatePlayCsi() -- command used by the default player control mechanism
SetCharAnimStateOriginalE: SetCharAnimStateOriginalE() -- command used by the default player control mechanism
GetCharAnimStateOriginalE: GetCharAnimStateOriginalE() -- command used by the default player control mechanism
SetCharAnimStateObj: SetCharAnimStateObj() -- command used by the default player control mechanism
GetCharAnimStateObj: GetCharAnimStateObj() -- command used by the default player control mechanism
SetCharAnimStateAnimationSpeed: SetCharAnimStateAnimationSpeed() -- command used by the default player control mechanism
GetCharAnimStateAnimationSpeed: GetCharAnimStateAnimationSpeed() -- command used by the default player control mechanism
SetCharAnimStateE: SetCharAnimStateE() -- command used by the default player control mechanism
GetCharAnimStateE: GetCharAnimStateE() -- command used by the default player control mechanism
GetCsiStoodVault: GetCsiStoodVault() -- command used by the default player control mechanism
GetCharSeqTrigger: GetCharSeqTrigger() -- command used by the default player control mechanism
GetEntityElementBankIndex: GetEntityElementBankIndex() -- command used by the default player control mechanism
GetEntityElementObj: GetEntityElementObj() -- command used by the default player control mechanism
GetEntityElementRagdollified: GetEntityElementRagdollified() -- command used by the default player control mechanism
GetEntityElementSpeedModulator: GetEntityElementSpeedModulator() -- command used by the default player control mechanism
GetEntityProfileJumpModifier: GetEntityProfileJumpModifier() -- command used by the default player control mechanism
GetEntityProfileStartOfAIAnim: GetEntityProfileStartOfAIAnim() -- command used by the default player control mechanism
GetEntityProfileJumpHold: GetEntityProfileJumpHold() -- command used by the default player control mechanism
GetEntityProfileJumpResume: GetEntityProfileJumpResume() -- command used by the default player control mechanism
SetCharAnimControlsLeaping: SetCharAnimControlsLeaping() -- command used by the default player control mechanism
GetCharAnimControlsLeaping: GetCharAnimControlsLeaping() -- command used by the default player control mechanism
SetCharAnimControlsMoving: SetCharAnimControlsMoving() -- command used by the default player control mechanism
GetCharAnimControlsMoving: GetCharAnimControlsMoving() -- command used by the default player control mechanism
GetEntityAnimStart: GetEntityAnimStart() -- command used by the default player control mechanism
GetEntityAnimFinish: GetEntityAnimFinish() -- command used by the default player control mechanism

Understanding PREEXIT system
----------------------------
SetPreExitValue: SetPreExitValue(e,value) -- where e is the entity and a value of 2 will exit preexit function
function ai_melee_animmove_init(e)
 SetPreExitValue(e,0) -- set this to zero to clear out this value for the entity
end
function ai_melee_animmove_preexit(e)
 SetPreExitValue(e,1) -- setting this to one will ensure we keep calling preexit over and over
 if module_combatmelee.preexit(e,ai_movetype_useanim) == 1 then
  SetPreExitValue(e,2) -- when we set this to two, we instruct the engine to stop calling preexit and go to _exit
 end
end
function ai_melee_animmove_exit(e)
 module_combatmelee.exit(e)
end

These AI commands represent a subset of the DarkAI system (further help referenced at Docs\DarkAI Documentation):

NEW MANUAL MODE COMMANDS
AIEntityGoToPosition: AIEntityGoToPosition(obj,x,z) -- where obj is the object number of the entity.Use this for legacy AI bot movement
AIEntityGoToPosition: AIEntityGoToPosition(obj,x,z,containerindex) -- where obj is the object number of the entity. New decoupled AI bot manual control
AISetEntityControl: AISetEntityControl(obj,controlmode) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIEntityStop: AIEntityStop(obj) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIGetEntityCanSee: AIGetEntityCanSee(obj,x,z,groundflag) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIGetEntityViewRange: AIGetEntityViewRange(obj) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIGetEntityHeardSound: AIGetEntityHeardSound(obj) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AICouldSee: AICouldSee(obj,x,y,z) -- where obj is the object number of the entity. XYZ of the point in space tested line of sight by the AI
AIGetEntityIsMoving: AIGetEntityIsMoving(obj) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AISetEntityPosition : AISetEntityPosition(obj,x,y,z) -- where obj is the object number of the entity and XYZ are the new coordinates
AISetEntityTurnSpeed : AISetEntityTurnSpeed(obj,speed) -- where obj is the object number and speed is the turning speed for navigating entities
AIGetEntitySpeed: speed = AIGetEntitySpeed(obj) -- where obj is the object number of the entity and returns the movement speed of the entity
AIGetTotalPaths: AIGetTotalPaths() -- returns the number of created paths
AIGetPathCountPoints: AIGetPathCountPoints(pathnumber) -- where pathnumber is the internal number of the path
AIPathGetPointX: AIPathGetPointX(pathnumber,pointindex) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIPathGetPointY: AIPathGetPointY(pathnumber,pointindex) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIPathGetPointZ: AIPathGetPointZ(pathnumber,pointindex) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIGetTotalCover: AIGetTotalCover() -- returns the number of created cover zones
AICoverGetPointX: AICoverGetPointX(coverindex) -- where coverindex is the index of the cover marker and returns the X position
AICoverGetPointY: AICoverGetPointY(coverindex) -- where coverindex is the index of the cover marker and returns the Y position
AICoverGetPointZ: AICoverGetPointZ(coverindex) -- where coverindex is the index of the cover marker and returns the Z position
AICoverGetAngle: AICoverGetAngle(coverindex) -- where coverindex is the index of the cover marker and returns the Y angle position
AICoverGetIfUsed : AICoverGetIfUsed(coverindex) -- where coverindex is the index of the cover marker and returns string from IFUSED field

SetRotationYSlowly : SetRotationYSlowly(e,destangle,smoothvalue) -- where e is the entity and a smooth value of 100 is immediate, where 50 takes twice as long to rotate to destangle

OLD LEGACY AUTOMATIC COMMANDS
AIEntityAssignPatrolPath AIEntityAssignPatrolPath(obj,pathid) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIEntityAddTarget: AIEntityAddTarget(obj,targetid) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIEntityRemoveTarget: AIEntityRemoveTarget(obj,targetid) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIEntityMoveToCover: AIEntityMoveToCover(obj,x,z) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions
AIGetEntityCanFire: AIGetEntityCanFire(obj) -- where obj is the object number of the entity. See DarkAI docs for parameter descriptions

***** VR and Headtracking
GetHeadTracker : GetHeadTracker() -- returns a value of one if the head tracker is connected
ResetHeadTracker : ResetHeadTracker() -- resets the tracker to signal device is facing forward
GetHeadTrackerYaw : GetHeadTrackerYaw() - returns the yaw of the head tracker if attached
GetHeadTrackerPitch : GetHeadTrackerPitch() - returns the pitch of the head tracker if attached
GetHeadTrackerRoll : GetHeadTrackerRoll() - returns the roll of the head tracker if attached

GetGamePlayerStateMotionController -- to be documented.
GetGamePlayerStateMotionControllerType -- to be documented.
MotionControllerThumbstickX -- to be documented.
MotionControllerThumbstickY -- to be documented.
CombatControllerTrigger -- to be documented.
CombatControllerGrip -- to be documented.
CombatControllerThumbstickX -- to be documented.
CombatControllerThumbstickY -- to be documented.

MotionControllerBestX -- to be documented.
MotionControllerBestY -- to be documented.
MotionControllerBestZ -- to be documented.
MotionControllerBestAngleX -- to be documented.
MotionControllerBestAngleY -- to be documented.
MotionControllerBestAngleZ -- to be documented.
MotionControllerLaserGuidedEntityObj -- to be documented.

Prompt3D : Prompt3D(text,duration) -- renders a 3D text panel in front of camera showing 'text'
PositionPrompt3D : PositionPrompt3D(x,y,z,angle) -- repositions 3D text panel to any XYZ world coordinate

ScaleObject : ScaleObject( obj, x, y, z ) -- Scales object in all axis (Note: uses object id not entity!)

GetEntityWeight : weight = GetEntityWeight( e ) -- returns the Physics weight value of the entity
GetObjectCentre(object id) -- returns center of object
GetLimbName : name = GetLimbName ( obj, limbindex ) -- returns name of limb specified by object number and limb index
GetLimbPosAng(obj,limbindex) -- returns the position and angle of a limb: x,y,z,anglex,angley,anglez.

SetSkyTo : SetSkyTo ( str ) -- where str is the folder name of the sky you want to change to

SetLutTo : SetLutTo ( "str" ) -- where str is the name of the post process filter effect to change to ("none" is default / none)
GetLut : GetLut() -- returns the name of the loaded LUT
SetPostSaturation : SetPostSaturation ( v ) -- where v is the value between 0 and 100 for post process saturation (30 is default)
GetPostSaturation : value = GetPostSaturation ( ) -- gets the saturation value currently used in the game
SetPostSepia : SetPostSepia ( v ) -- where v is the value between 0 and 100 for post process sepia effect (0 is default / none)
GetPostSepia : value = GetPostSepia ( ) -- gets the sepia value currently used in the game

***** The following five functions return multiple values, if you do not need them all just replace 
***** the ones you don't need with '_' for example : _, _, _, Ax, Ay, Az = GetEntityPosAng( e ) would
***** just give you last three of the 6 values returned
GetObjectPosAng : x, y, z, Ax, Ay, Az = GetObjectPosAng( obj ) -- returns position and Euler angles of object
GetEntityPosAng : x, y, z, Ax, Ay, Az = GetEntityPosAng( e )   -- returns position and Euler angles of entity
GetObjectScales : xs, ys, zs = GetObjectScales( obj ) -- returns scale values of object in all axis (Note: uses object id not entity!)
GetEntityWeight : weight = GetEntityWeight( e ) -- returns the Physics weight value of the entity

***** Collision box is defined by coordinates of two opposing corners, from these it is easy to calculate the size of the object
GetObjectCollBox : xmin, ymin, zmin, xmax, ymax, zmax = GetObjectColBox( obj ) -- returns collision cube of object
GetEntityCollBox : xmin, ymin, zmin, xmax, ymax, zmax = GetEntityColBox( e )   -- returns collision cube of entity. Example: local x1, y1, z1, x2, y2, z2 = GetEntityCollBox(e)

***** Lua control of dynamic light, you get the light number using entity e number then use that in the other light functions
***** for example; lightNum = GetEntityLightNumber( e )  then  x, y, z = GetLightPosition( lightNum )
GetEntityLightNumber : lightNum = GetEntityLightNumber( e ) -- returns the internal light number held by the entity
GetLightPosition : x, y, z = GetLightPosition( lightNum ) -- returns the XYZ position of the dynamic light specified
GetLightAngle : xv, yv, zv = GetLightAngle( LightNum ) -- returns the angle vector of the dynamic light specified
GetLightRGB : r, g, b = GetLightRGB( lightNum ) -- returns the RGB color of the dynamic light specified
GetLightRange : range = GetLightRange ( lightNum ) -- returns the range value of the dynamic light specified
SetLightPosition : SetLightPosition ( lightNum, x, y, z ) -- sets the new position of the specified dynamic light
SetLightAngle : SetLightAngle( lightNum, xv, yv, zv ) -- sets the angle vector of the specified light ( e.g. 0, 0, 1 would be 'North' or +Z axis )
SetLightRGB : SetLightRGB ( lightNum, r, g, b ) -- sets the new color of the specified dynamic light
SetLightRange : SetLightRange ( lightNum, range ) -- sets the new range (1 to 10000) of the specified light

GetEntityMarkerMode(e) -- returns marker mode (1=player,2=lights,3=trigger zone,4=decal particle emitter,5=entity lights,6=checkpoint zone,7=multiplayer start,8=floor zone,9=cover zone,10=new particle emitter)

***Water Shader Settings*** Look into the shader for informations about these values(open effectbank/reloaded/water_basic.fx with i.e. notepad)
SetWaterHeight(value) -- sets water setting attributes
SetWaterColor(red,green,blue) -- sets water setting attributes
SetWaterWaveIntensity(value) -- sets water setting attributes
SetWaterTransparancy(value) -- sets water setting attributes
SetWaterReflection(value) -- sets water setting attributes
SetWaterReflectionSparkleIntensity(value) -- sets water setting attributes
SetWaterFlowDirection(x,y,speed) small tip: -1 =east/north; 1=west/south - speed is a multiplier of the flowdirection(higher => fast flow)
SetWaterDistortionWaves(value) -- sets water setting attributes
SetRippleWaterSpeed(value) -- sets water setting attributes
GetWaterHeight() -- gets water setting attributes
GetWaterWaveIntensity() -- gets water setting attributes
GetWaterShaderColorRed() -- gets water setting attributes
GetWaterShaderColorGreen() -- gets water setting attributes
GetWaterShaderColorBlue() -- gets water setting attributes
GetWaterTransparancy() -- gets water setting attributes
GetWaterReflection() -- gets water setting attributes
GetWaterReflectionSparkleIntensity() -- gets water setting attributes
GetWaterFlowDirectionX() -- gets water setting attributes
GetWaterFlowDirectionY() -- gets water setting attributes
GetWaterFlowSpeed() -- gets water setting attributes
GetWaterDistortionWaves() -- gets water setting attributes
GetRippleWaterSpeed() -- gets water setting attributes

***** Entity Texture
SetEntityHighlight(e,v) -- set an overlay colour on entity, v = 0(none/remove),1(red),2(pink),3(green),4(blue/green),5(gold)
ChangeTexture(e,texturepath) -- Change the texture of the entity. For example: ChangeTexture(e,"entitybank\\Industrial\\Gas Tank_D.dds") OR ChangeTexture(e,"entitybank\\Industrial\\Gas Tank_D.dds",2) to include material index if multi texture object.
PlayVideoOnEntity(e,videopath,v) -- entity e number, video file path, [optional] loopvideo = 1/0 
StopVideoOnEntity(e) -- Stop video texture
IsVideoOnEntityPlaying(e) -- Returns if video texture is playing on entity.

***** Dymanic sun commands.
SetSunDirection(x,y,z) -- The direction will be multiplyid into a position so you can use ranges like -10 to 10 or similar.

See simulated_sky_demo.lua for command example usage

SetStarDensity(v) 0 to 100
SetSolarLatitude(v) -90 to 90
SetSolarDay(v) 1 to 31
SetSolarMonth(v) 1 to 12
SetSolarTime(v) 0 to 24
SetSunTime(time_hours,day,month,latitude)
SetCloudCoverage(v) 0 to 100
SetCloudSpeed(v) 0 to 100
SetCloudDensity(v) 0 to 100
SetCloudTint(v) 0 to 100 -- lower darker
SetSkyEffects(v) 0 to 100 -- note - values greater than zero and less than 50 enable aurora at night, > 50 enables lightning flashes any time of day

GetStarDensity() -- return star density.
GetSkyEffects() -- return sky effects.
GetCloudCoverage() -- return cloud coverage.
GetCloudSpeed() -- return cloud speed.
GetCloudDensity() -- return cloud density.
GetCloudTint() -- return cloud tint.
GetSolarLatitude() -- return solar latitude.
GetSolarDay() -- return solar day.
GetSolarMonth() -- return solar month.
GetSolarTime() -- return solar time.

***** Physics commands
PushObject -- To be documented
ConstrainObjMotion -- To be documented
ConstrainObjRotation -- To be documented
CreateSingleHinge -- To be documented
CreateSingleJoint -- To be documented
CreateDoubleHinge -- To be documented
CreateDoubleJoint -- To be documented
CreateSliderDouble -- To be documented
RemoveObjectConstraints -- To be documented
RemoveConstraint -- To be documented
PhysicsRayCast -- To be documented
SetObjectDamping -- To be documented
SetHingeLimits -- To be documented
GetHingeAngle -- To be documented
SetHingeMotor -- To be documented
SetSliderMotor -- To be documented
SetBodyScaling -- To be documented
SetSliderLimits -- To be documented
GetSliderPosition -- To be documented

***** Collision detection commands 
GetObjectNumCollisions -- To be documented
GetObjectCollisionDetails -- To be documented
GetTerrainNumCollisions -- To be documented
GetTerrainCollisionDetails -- To be documented
AddObjectCollisionCheck -- To be documented
RemoveObjectCollisionCheck -- To be documented

***** quaternion commands
QuatToEuler -- To be documented
EulerToQuat -- To be documented
QuatMultiply -- To be documented
QuatSLERP -- To be documented
QuatLERP -- To be documented

***** utility
MsgBox("") -- Display pop-up "Lua Message" confirmation dialogue box. Do not run this on loop!
Reset(Fade() -- To be documented

--]]
