-- LUA Script - precede every function and global member with lowercase name of script

attackstart = {}
attackend = {}
damageframestart = {}
damageframeend = {}
lastroar = {}
lastswipe = {}
egg_status = {}

function poison_flower_walkable_init(e)
 ai_soldier_state[e] = "idle"
 CharacterControlLimbo(e)
 SetAnimationFrames(0,199)
 LoopAnimation(e)
 attackstart[e]=256
 attackend[e]=314
 damageframestart[e]=256
 damageframeend[e]=314
 lastroar[e]=0
 lastswipe[e]=0
 egg_status[e] = 0 
end

function poison_flower_walkable_main(e)
 if egg_status[e] == 0 then

if eggradar_addEnemy ~= nil then eggradar_addEnemy(e) end
  egg_status[e] = 1
 end
 PlayerDist = GetPlayerDistance(e)
 EntObjNo = g_Entity[e]['obj']
 if (PlayerDist < 1500 and ai_soldier_state[e] == "idle" and g_Entity[e]['plrvisible'] == 1) or (PlayerDist < 100 and ai_soldier_state[e] == "idle") then
  RotateToPlayer(e)
  if GetTimer(e) > lastroar[e] then
   lastroar[e]=GetTimer(e)+5000
  end
 end
 if PlayerDist < 1000 and g_Entity[e]['plrvisible']==1 then
  RotateToPlayer(e)
  if PlayerDist < 65 and g_Entity[e]['plrvisible']==1 then
   if ai_soldier_state[e]~="attack" then
    SetAnimationSpeed(e,0.7)
    if GetAnimationFrame(e)<attackstart[e] or GetAnimationFrame(e)>attackend[e] then
     AIEntityStop(EntObjNo)
	 ai_soldier_state[e] = "attack"
     CharacterControlLimbo(e)
     SetAnimationFrames(attackstart[e],attackend[e])
     PlayAnimation(e)
	 g_Entity[e]['animating'] = 1
    end
   else
    if ai_soldier_state[e]=="attack" then
     if GetAnimationFrame(e)>damageframestart[e] and GetAnimationFrame(e)<damageframeend[e] then
      if GetPlayerDistance(e)<65 then
       if GetTimer(e) > lastswipe[e] then
        lastswipe[e]= GetTimer(e)+1000
        PlaySound(e,0)
        randomdamage=math.random(20,50)
        HurtPlayer(e,randomdamage)
       end
      end 
  	 end
     if g_Entity[e]['animating']==0 then
      ai_soldier_state[e] = "idle"
      CharacterControlLimbo(e)
      SetAnimationFrames(0,199)
      PlayAnimation(e)
      RotateToPlayer(e)
     end
    end
   end
  else 
   if ai_soldier_state[e]~="roam" then
    ai_soldier_state[e] = "roam"
	PlaySound(e,0)
    CharacterControlUnarmed(e)
    ModulateSpeed(e,1.0)
    SetAnimationSpeed(e,1.5)
    SetCharacterToWalk(e)
   end
   rndx=math.random(1,360)
   rndz=math.random(1,360)
   rndx2=math.sin(rndx)*30
   rndz2=math.cos(rndz)*30
   AIEntityGoToPosition(EntObjNo,g_PlayerPosX+rndx2,g_PlayerPosZ+rndz2) 
  end
 else
  if PlayerDist >= 1500 and ai_soldier_state[e] ~= "idle" then
   ai_soldier_state[e] = "idle"
   CharacterControlLimbo(e)
   SetAnimationFrames(0,199)
   LoopAnimation(e)
   ModulateSpeed(e,1.0)
   SetAnimationSpeed(e,1.0)
  end
 end
 --PromptLocal(e,"ZOMBIEWALK : Dist=" .. PlayerDist .. " State=" .. ai_soldier_state[e] .. " Vis=" .. g_Entity[e]['plrvisible'] .. " Frm=" .. GetAnimationFrame(e) )
end

function poison_flower_walkable_exit(e)
PlaySound(e,1)
ai_soldier_state[e] = "dead"
ActivateIfUsed(e)
ModulateSpeed(e,1.0)
CharacterControlLimbo(e)
SetAnimationSpeed(e,1.5) 
SetAnimationFrames(741,769) 
PlayAnimation(e)
CollisionOff(e)
if GetAnimationFrame(e)==769 then
Destroy(e)
end
end
