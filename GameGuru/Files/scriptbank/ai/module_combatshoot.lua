-- COMBATSHOOT Manager Module
module_core = require "scriptbank\\ai\\module_core"
module_agro = require "scriptbank\\ai\\module_agro"
module_combatcore = require "scriptbank\\ai\\module_combatcore"
local module_combatshoot = {}

function module_combatshoot.init(e,combattype)
 if combattype == ai_combattype_regular then startingstate = ai_state_findpatrolpath end
 if combattype == ai_combattype_patrol then startingstate = ai_state_findpatrolpath end
 if combattype == ai_combattype_guard then startingstate = ai_state_startidle end
 module_combatcore.init(e,startingstate,-1)
end

function module_combatshoot.main(e,combattype,movetype,attacktype)
 
 -- DEBUG OPTION - make player undetectable so can test AI waypoints
 --g_Entity[e]['plrvisible'] = 0
 
 -- common vars
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = module_core.getplayerdist(e)
 
 -- attack types
 if attacktype == ai_attacktype_canfire then
  CanFire = 1
 else
  CanFire = 0 
 end
 
 -- handle idle
 if ai_bot_angryhurt[e] ~= nil then combattype = ai_combattype_regular end
 if combattype == ai_combattype_guard then
  module_combatcore.idle(e,AIObjNo,PlayerDist,CanFire,ai_state_startfireonspot,combattype)
 else
  module_combatcore.idle(e,AIObjNo,PlayerDist,CanFire,ai_state_startmove,combattype)
 end
 
 -- handle AI that moves
 if combattype == ai_combattype_regular or combattype == ai_combattype_patrol then
  module_combatcore.patrol(e,AIObjNo,PlayerDist,movetype,CanFire,ai_state_startmove,ai_state_startpatrol,combattype)
  module_combatcore.hunt(e,AIObjNo,PlayerDist,movetype,CanFire,ai_state_startidle)
  module_combatcore.handleducking(e,AIObjNo,PlayerDist)
  module_combatcore.soundawareness(e,AIObjNo)
 end
 
 -- handle patrol logic
 if combattype == ai_combattype_regular or combattype == ai_combattype_patrol then
  if ai_bot_state[e] == ai_state_idle or ai_bot_state[e] == ai_state_move then
   if g_Entity[e]['plrvisible'] == 1 and PlayerDist < AIGetEntityViewRange(AIObjNo) then
    -- record last time seen target
    ai_bot_patroltime[e] = g_Time
   end
   if g_Time > ai_bot_patroltime[e] + 5000 then
    -- after X seconds elapsed, return to patrol
    -- only if a ai_bot_pathindex exist , otherwise this will just flicker the idle animation.
    if ai_bot_pathindex[e] ~= -1 then
		ai_bot_state[e] = ai_state_startpatrol
		ai_bot_coverindex[e] = -1
		ai_bot_targetx[e] = nil
		ai_bot_patroltime[e] = g_Time
	end
   end
  else
   ai_bot_patroltime[e] = g_Time
  end
 end 
 
 -- handle attacks
 if combattype == ai_combattype_regular then
  module_combatcore.sensepunch(e,AIObjNo,PlayerDist,combattype)
 end
 
 -- handle hurt response
 if combattype == ai_combattype_guard then
  module_combatcore.hurt(e,PlayerDist,ai_state_startfireonspot)
 else
  module_combatcore.hurt(e,PlayerDist,ai_state_startidle)
 end
 
 -- handle events
 module_combatcore.headshot(e)
 module_combatcore.punch(e,AIObjNo,PlayerDist,combattype,ai_state_startfireonspot)
 module_combatcore.recover(e,ai_state_startpatrol)
 module_combatcore.fireonspot(e,AIObjNo)
 module_combatcore.reloadweapon(e)
 
 -- handle debugging
 --module_core.debug(e,AIObjNo,PlayerDist,combattype)

end

function module_combatshoot.preexit(e,movetype)
 return module_combatcore.preexit(e,movetype)
end

function module_combatshoot.exit(e)
 module_combatcore.exit(e)
end

return module_combatshoot
