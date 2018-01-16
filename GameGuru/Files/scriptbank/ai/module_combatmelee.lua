-- COMBATMELEE Manager Module
module_core = require "scriptbank\\ai\\module_core"
module_combatcore = require "scriptbank\\ai\\module_combatcore"

local module_combatmelee = {}

function module_combatmelee.init(e,combattype)
 startingstate = ai_state_startidle
 module_combatcore.init(e,startingstate,-1)
end

function module_combatmelee.main(e,combattype,movetype)

 -- common vars
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = module_core.getplayerdist(e)
 CanFire = 0
 
 -- behaviors
 module_combatcore.idle(e,AIObjNo,PlayerDist,CanFire,ai_state_startmove)
 module_combatcore.homein(e,AIObjNo,PlayerDist,movetype,CanFire,ai_state_recoverstart)
 module_combatcore.sensepunch(e,AIObjNo,PlayerDist,combattype)
 module_combatcore.hurt(e,ai_state_startmove)
 module_combatcore.headshot(e)
 module_combatcore.soundawareness(e,AIObjNo)
 module_combatcore.punch(e,PlayerDist,combattype,ai_state_startidle) 
 module_combatcore.recover(e,ai_state_startidle)

 -- handle debugging
 --module_core.debug(e,AIObjNo,PlayerDist,combattype)

end

return module_combatmelee
