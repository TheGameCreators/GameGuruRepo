-- LUA Script - precede every function and global member with lowercase name of script
guard_chat_talktoalien = 0
alien_artefactImage = 0
alien_artefactSprite = 0
alien_artefactAngle = 0
alien_artefactProgress = 0

function alien_init(e)
 alien_artefactImage = LoadImage("scriptbank\\scifi\\media\\artefact-vision.png")
 alien_artefactSprite = CreateSprite ( alien_artefactImage )
 SetSpriteDepth ( alien_artefactSprite, 100 )
 SetSpriteSize ( alien_artefactSprite, 40, 40 )
 SetSpriteOffset ( alien_artefactSprite, 20, 20 )
 SetSpritePosition ( alien_artefactSprite, -100, -100 ) 
 CharacterControlLimbo(e)
 SetAnimationSpeed(e,1)
 SetAnimationFrames(3000,3100)
 LoopAnimation(e) 
end
function alien_main(e)
 -- use sprite as a vision from alien
 if alien_artefactProgress>0 then 
  SetSpritePosition ( alien_artefactSprite, 50, 50 ) 
  alien_artefactProgress = alien_artefactProgress + 2
  if alien_artefactProgress <= 255 then
   SetSpriteColor ( alien_artefactSprite, 255, 255, 255, alien_artefactProgress )
  else
   if alien_artefactProgress <= 256+255 then
    SetSpriteColor ( alien_artefactSprite, 255, 255, 255, 255-(alien_artefactProgress-256) )
   else
    alien_artefactProgress = 0
    SetSpritePosition ( alien_artefactSprite, -100, -100 )
   end
  end
  alien_artefactAngle = alien_artefactAngle + 0.2
  SetSpriteAngle( alien_artefactSprite, alien_artefactAngle )
 end
 -- alien logic
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 then
  RotateToPlayer(e)
  if g_Entity[e]['activated']==0 then
   if guard_chat_talktoalien<=1 then
    Prompt ( "Press E to speak to temple alien" )
   else
    Prompt ( "Press E to return artefact to temple alien" )
   end
   if g_KeyPressE==1 then
    if guard_chat_talktoalien<=2 then
     PlaySound(e,0)
	 guard_chat_talktoalien = 1
     alien_artefactProgress = 1
     alien_artefactAngle = -20
	else
     PlaySound(e,1)
     if guard_chat_talktoalien==3 then
	  guard_chat_talktoalien = 4
	 end
	end
    SetAnimationSpeed(e,0.5)
    SetAnimationFrames(4680,4745)
    PlayAnimation(e)
    SetActivated(e,3)
	StartTimer(e)
   end
  end 
 end
 if g_Entity[e]['activated']==1 then
  if GetAnimationFrame(e)>4740 then
   if guard_chat_talktoalien==4 then
	SetActivated(e,2)
	ActivateIfUsed(e)
    SetAnimationSpeed(e,1.0)
	CharacterControlUnarmed(e)
	EntObjNo = g_Entity[e]['obj']
	AIEntityGoToPosition(EntObjNo,g_Entity[e]['x'],g_Entity[e]['z']-1000)
   else
 	SetActivated(e,0)
    SetAnimationSpeed(e,1)
    SetAnimationFrames(3000,3100)
    LoopAnimation(e)
   end
  end
 end
 if g_Entity[e]['activated']==3 then
  SetActivated(e,1)
 end
 if g_Entity[e]['activated']==2 then
  -- alien has walked away
 else
  if guard_chat_talktoalien==4 then
   -- alien doing final talk
  else
   if GetTimer(e)>5000 then
    CharacterControlLimbo(e)
    EntObjNo = g_Entity[e]['obj']
    AIEntityStop(EntObjNo)
    SetActivated(e,0)
    SetAnimationFrames(3000,3100)
    LoopAnimation(e)
    StartTimer(e)
   end
  end
 end
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
end
