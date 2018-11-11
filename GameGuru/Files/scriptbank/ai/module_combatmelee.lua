-- COMBATMELEE Manager Module
module_core = require "scriptbank\\ai\\module_core"
module_combatcore = require "scriptbank\\ai\\module_combatcore"

local module_combatmelee = {}

function module_combatmelee.init(e,combattype)
 startingstate = ai_state_startidle
 if combattype == ai_combattype_regular then startingstate = ai_state_findpatrolpath end
 if combattype == ai_combattype_patrol then startingstate = ai_state_findpatrolpath end
 module_combatcore.init(e,startingstate,-1)
end

function module_combatmelee.main(e,combattype,movetype)

 -- common vars
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = module_core.getplayerdist(e)
 CanFire = 0
 
 -- handle idle
 module_combatcore.idle(e,AIObjNo,PlayerDist,CanFire,ai_state_startmove,combattype)
 
 -- handle patrol
 if combattype == ai_combattype_regular or combattype == ai_combattype_patrol then
  module_combatcore.patrol(e,AIObjNo,PlayerDist,movetype,CanFire,ai_state_startmove,ai_state_startpatrol,combattype)
 end
 
 -- handle moves
 module_combatcore.homein(e,AIObjNo,PlayerDist,movetype,CanFire,ai_state_recoverstart)
 module_combatcore.sensepunch(e,AIObjNo,PlayerDist,combattype)
 module_combatcore.hurt(e,PlayerDist,ai_state_startmove)
 module_combatcore.headshot(e)
 module_combatcore.soundawareness(e,AIObjNo)

 -- handle patrol logic
 if combattype == ai_combattype_regular or combattype == ai_combattype_patrol then
  if ai_bot_state[e] == ai_state_idle or ai_bot_state[e] == ai_state_move then
   if g_Entity[e]['plrvisible'] == 1 and PlayerDist < AIGetEntityViewRange(AIObjNo) then
    -- record last time seen target
    ai_bot_patroltime[e] = g_Time
   end
   if g_Time > ai_bot_patroltime[e] + 5000 then
    -- after 20 seconds elapsed, return to patrol
    -- PE: only if a ai_bot_pathindex exist , otherwise this will just flicker the idle animation.
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
 
 -- handle attack
 module_combatcore.punch(e,AIObjNo,PlayerDist,combattype,ai_state_startidle) 
 module_combatcore.recover(e,ai_state_startidle)

 -- handle debugging
 --module_core.debug(e,AIObjNo,PlayerDist,combattype)

end

function module_combatmelee.preexit(e,movetype)
 return module_combatcore.preexit(e,movetype)
end

function module_combatmelee.exit(e)
 module_combatcore.exit(e)
end
 
return module_combatmelee
