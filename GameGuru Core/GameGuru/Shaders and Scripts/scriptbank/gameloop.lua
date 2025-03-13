-- GameLoop module
module_cameraoverride = require "scriptbank\\ai\\module_cameraoverride"

gameloop_RegenTickTime = 0

local gameloop = {}

function gameloop.init()
 -- insert your own inits here
 gameloop_RegenTickTime = 0
end

function gameloop.main()
 -- player handle health 
 if g_PlayerHealth > 0 and g_PlayerHealth < g_gameloop_StartHealth and g_PlayerDeadTime == 0 then
  -- dont regen health if dead or at max health
  if g_PlayerLastHitTime > 0 then
   -- handle player health regeneration
   if g_Time > g_PlayerLastHitTime + g_gameloop_RegenDelay and  GetGamePlayerControlInWaterState() < 3 then
    if g_Time > gameloop_RegenTickTime then
	 gameloop_RegenTickTime = g_Time + g_gameloop_RegenSpeed
	 newHealth = g_PlayerHealth + g_gameloop_RegenRate
	 if newHealth > g_gameloop_StartHealth then
	  newHealth = g_gameloop_StartHealth
	 end
	 SetPlayerHealth ( newHealth )
	end
   end
  end
 end
 -- more externalised mechanics to come..
end

function gameloop.quit()
 module_cameraoverride.restoreandreset()
end

return gameloop
