-- AI : Biter Behavior
module_combatmelee = require "scriptbank\\ai\\module_combatmelee"
function ai_meleebiter_init(e)
 module_combatmelee.init(e,ai_combattype_freezermelee)
end
function ai_meleebiter_main(e)
 module_combatmelee.main(e,ai_combattype_freezermelee,ai_movetype_useanim)
end
