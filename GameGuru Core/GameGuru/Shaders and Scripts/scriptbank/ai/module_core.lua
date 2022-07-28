-- CORE Common Module

local module_core = {}

function module_core.getplayerdist(e)
 PlayerDist = GetPlayerDistance(e)
 if g_PlayerHealth <= 0 then PlayerDist = 99999 end
 return PlayerDist
end

function module_core.countaiaroundplayer()
 tcountai = 0
 for ee = 1 , g_EntityElementMax, 1 do
  if g_Entity[ee] ~= nil then
   if ai_bot_state[ee] ~= nil then
    pDX = g_Entity[ee]['x'] - g_PlayerPosX
    pDY = g_Entity[ee]['y'] - g_PlayerPosY
    pDZ = g_Entity[ee]['z'] - g_PlayerPosZ
    pDist = math.sqrt(math.abs(pDX*pDX)+math.abs(pDY*pDY)+math.abs(pDZ*pDZ));
    if pDist < 100 then
     tcountai = tcountai + 1
    end
   end
  end
 end
 return tcountai
end

function module_core.debug(e,AIObjNo,PlayerDist,combattype)
 if ai_bot_state ~= nil then
  if ai_bot_state[e] ~= nil then
   AH=0
   if ai_bot_angryhurt[e] ~= nil then AH=1 end
   PromptLocal ( e, "H=" .. g_Entity[e]['health'] .. " CT=" .. combattype .. " STATE=" .. ai_bot_state[e] .. "(" .. ai_state_debug[1+ai_bot_state[e]] .. ") SUB=" .. ai_bot_substate[e] .. " DIST=" .. PlayerDist .. " AVOID=" .. g_Entity[e]['avoid'] .. " VIS=" .. g_Entity[e]['plrvisible'] .. " COVER=" .. ai_bot_coverindex[e] .. "(" .. AICoverGetIfUsed(ai_bot_coverindex[e]) .. ")" .. " AH=" .. AH .. " SP=" .. ai_bot_gofast[e] )
  else
   PromptLocal ( e, "No ai_bot_state[e]" )
  end
 else
  PromptLocal ( e, "No ai_bot_state" )
 end
end
 
return module_core
