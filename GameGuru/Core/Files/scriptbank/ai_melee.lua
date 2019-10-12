-- AI : Melee Guard Behavior
module_combatmelee = require "scriptbank\\ai\\module_combatmelee"
function ai_melee_init(e)
 module_combatmelee.init(e,ai_combattype_guard)
end
function ai_melee_main(e)
 module_combatmelee.main(e,ai_combattype_guard)
end
