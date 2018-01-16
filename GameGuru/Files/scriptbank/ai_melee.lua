-- AI : Melee Behavior
module_combatmelee = require "scriptbank\\ai\\module_combatmelee"
function ai_melee_init(e)
 module_combatmelee.init(e,1)
end
function ai_melee_main(e)
 module_combatmelee.main(e,1)
end
