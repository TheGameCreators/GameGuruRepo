-- CORE Common Module

local module_core = {}

function module_core.getplayerdist(e)
 PlayerDist = GetPlayerDistance(e)
 if g_PlayerHealth <= 0 then PlayerDist = 99999 end
 return PlayerDist
end

function module_core.debug(e,AIObjNo,PlayerDist,combattype)
 if ai_bot_state ~= nil then
  if ai_bot_state[e] ~= nil then
   AH=0
   if ai_bot_angryhurt[e] ~= nil then AH=1 end
   PromptLocal ( e, "H=" .. g_Entity[e]['health'] .. " CT=" .. combattype .. " STATE=" .. ai_bot_state[e] .. "(" .. ai_state_debug[1+ai_bot_state[e]] .. ") SUB=" .. ai_bot_substate[e] .. " DIST=" .. PlayerDist .. " AVOID=" .. g_Entity[e]['avoid'] .. " VIS=" .. g_Entity[e]['plrvisible'] .. " COVER=" .. ai_bot_coverindex[e] .. "(" .. AICoverGetIfUsed(ai_bot_coverindex[e]) .. ")" .. " AH=" .. AH )
  else
   PromptLocal ( e, "No ai_bot_state[e]" )
  end
 else
  PromptLocal ( e, "No ai_bot_state" )
 end
end
 
return module_core
