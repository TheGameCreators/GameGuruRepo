-- AI : Melee Behavior
module_combatmelee = require "scriptbank\\ai\\module_combatmelee"
function ai_meleepatrol_init(e)
 module_combatmelee.init(e,ai_combattype_patrol)
end
function ai_meleepatrol_main(e)
 module_combatmelee.main(e,ai_combattype_patrol)
end
