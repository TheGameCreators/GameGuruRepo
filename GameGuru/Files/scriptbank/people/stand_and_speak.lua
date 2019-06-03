-- 
g_stand_and_speak_name = {}

function stand_and_speak_init_name(e,name)
 g_stand_and_speak_name[e] = name
end
function stand_and_speak_main(e)
 PlayerDist = GetPlayerDistance(e)
 fCloseDistance = AIGetEntityViewRange(AIObjNo) / 4
 if fCloseDistance < 150 then fCloseDistance = 150 end
 if PlayerDist < fCloseDistance and g_PlayerHealth > 0 then
  RotateToPlayer(e)
  if g_stand_and_speak_name[e] ~= nil then
   PromptLocalForVR(e,g_stand_and_speak_name[e],2)
  end
 end
end
