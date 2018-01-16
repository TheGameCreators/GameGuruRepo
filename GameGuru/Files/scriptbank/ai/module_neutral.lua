-- COMBATMELEE Manager Module
module_core = require "scriptbank\\ai\\module_core"
module_combatcore = require "scriptbank\\ai\\module_combatcore"

local module_neutral = {}

function module_neutral.init(e)
 module_combatcore.init(e,ai_state_startidle,-2)
end

function module_neutral.main(e)

 -- common vars
 AIObjNo = g_Entity[e]['obj']
 PlayerDist = module_core.getplayerdist(e)

 -- behaviors
 module_combatcore.idle(e,AIObjNo,PlayerDist,ai_state_idle)
 module_combatcore.hurt(e,ai_state_startidle)
 module_combatcore.headshot(e)
 module_combatcore.recover(e,ai_state_startidle)
 
 -- handle debug
 --module_core.debug(e,AIObjNo,PlayerDist)

end

return module_neutral
