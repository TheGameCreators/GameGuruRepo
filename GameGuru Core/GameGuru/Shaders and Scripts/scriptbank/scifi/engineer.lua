-- LUA Script - precede every function and global member with lowercase name of script
engineer_tool_special = 0
engineer_door_password = 0

function engineer_init(e)
 CharacterControlLimbo(e)
 SetAnimationSpeed(e,1)
 SetAnimationFrames(3000,3100)
 LoopAnimation(e)
end
function engineer_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 150 then
  RotateToPlayer(e)
  if g_Entity[e]['activated']==0 then
   if engineer_tool_special==0 then
    Prompt ( "Press E to speak to engineer" )
   else
    Prompt ( "Press E to give engineer special tool" )
   end
   if g_KeyPressE==1 then
    SetActivated(e,1)
    if engineer_tool_special==0 then
     PlaySound(e,0)
     SetAnimationSpeed(e,1.25)
	else
     PlaySound(e,1)
     SetAnimationSpeed(e,1.25)
     engineer_door_password = 1
	end
    SetAnimationFrames(4260,4464)
    PlayAnimation(e)
   end
  else
   if g_Entity[e]['activated']==1 then
    if GetAnimationFrame(e)>4460 then
	 if engineer_door_password==0 then
 	  SetActivated(e,0)
     else
 	  SetActivated(e,2)
	 end
     SetAnimationSpeed(e,1)
     SetAnimationFrames(3000,3100)
     LoopAnimation(e)
	end
   end
  end 
 end 
 if string.find(string.lower(g_Entity[e]['limbhit']), "head") ~= nil then
  SetEntityHealth(e,0) 
  ResetLimbHit(e)
 end
end
