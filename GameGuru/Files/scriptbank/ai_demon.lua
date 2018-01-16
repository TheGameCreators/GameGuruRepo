-- LUA Script - precede every function and global member with lowercase name of script

attackstart = {}
attackend = {}
damageframestart = {}
damageframeend = {}
lastroar = {}
lastswipe = {}

function ai_demon_init(e)
 ai_soldier_state[e] = "idle"
 SetAnimationFrames(365,449)
 LoopAnimation(e)
 ModulateSpeed(e,1.0)
 SetAnimationSpeed(e,1.0)
 attackstart[e]=709
 attackend[e]=763
 lastroar[e] = 0
 lastswipe[e] = 0
end

function ai_demon_main(e)
 PlayerDist = GetPlayerDistance(e)
 EntObjNo = g_Entity[e]['obj']
 if (PlayerDist < 1000 and ai_soldier_state[e] == "idle" and g_Entity[e]['plrvisible'] == 1) or (AIGetEntityHeardSound(EntObjNo) == 1 and ai_soldier_state[e] == "idle") or (PlayerDist < 100 and ai_soldier_state[e] == "idle") then
  RotateToPlayer(e)
  ai_soldier_state[e] = "detected"
  if GetTimer(e) > lastroar[e] then
   lastroar[e]= GetTimer(e)+3000
   PlaySound(e,0)
  end
 end
 if PlayerDist < 1000 and ai_soldier_state[e] ~= "idle" then
  RotateToPlayer(e)
  if PlayerDist < 70 then
   if ai_soldier_state[e]~="attack" then
    if GetAnimationFrame(e)<attackstart[e] or GetAnimationFrame(e)>attackend[e] then
     AIEntityStop(EntObjNo)
	 ai_soldier_state[e] = "attack"
     CharacterControlLimbo(e)
     randomattack=math.random(1,2)
     if randomattack == 1 then
      attackstart[e]=709
      attackend[e]=735
      damageframestart[e]=715
      damageframeend[e]=720
     end
     if randomattack == 2 then
      attackstart[e]=736
      attackend[e]=763
      damageframestart[e]=745
      damageframeend[e]=752
     end
	 SetAnimationSpeed(e,1.2)
     SetAnimationFrames(attackstart[e],attackend[e])
     PlayAnimation(e)
     StopSound(e,0)
    end
   end
   if ai_soldier_state[e]=="attack" then
    if GetAnimationFrame(e)>damageframestart[e] and GetAnimationFrame(e)<damageframeend[e] then
     if GetPlayerDistance(e)<75 then
      if GetTimer(e) > lastswipe[e] then
       lastswipe[e]= GetTimer(e)+1600
       PlaySound(e,1)
      end
      randomdamage=math.random(3,7)
      HurtPlayer(e,randomdamage)
     end
	end
    if GetAnimationFrame(e)>attackend[e]-2 then
     CharacterControlUnarmed(e)
     ai_soldier_state[e] = "roam"
     SetAnimationSpeed(e,1.2)
     SetAnimationFrames(365,449)
     PlayAnimation(e)
     LoopSound(e,0)
    end
   end
  else 
   if GetPlayerDistance(e)>150 then
    ai_soldier_state[e] = "roam"
    CharacterControlUnarmed(e)
    ModulateSpeed(e,1.2)
    SetAnimationSpeed(e,1.2)
    SetCharacterToRun(e)
    LoopSound(e,0)
   else
    ModulateSpeed(e,1.0)
    SetAnimationSpeed(e,1.0)
   end
   rndx=math.random(1,360)
   rndz=math.random(1,360)
   rndx2=math.sin(rndx)*30
   rndz2=math.cos(rndz)*30
   AIEntityGoToPosition(EntObjNo,g_PlayerPosX+rndx2,g_PlayerPosZ+rndz2) 
  end
 end
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0)
  ResetLimbHit(e)
 end
end

function ai_demon_exit(e)
 StopSound(e,0)
 if lastroar[e] ~= -1 then
  CharacterControlLimbo(e)
  StopAnimation(e)
  SetAnimationFrame(e,547)
  SetAnimationFrames(547,628)
  PlayAnimation(e)
  PlayCharacterSound(e,"onDeath")
  CollisionOff(e)
  lastroar[e]=-1
 end
end
