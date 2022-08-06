-- AI : Biter Behavior
ai_patrol_combatshoot = require "scriptbank\\ai\\module_combatshoot"
function ai_patrol_init(e)
 ai_patrol_combatshoot.init(e,ai_combattype_patrol)
end
function ai_patrol_main(e)
 ai_patrol_combatshoot.main(e,ai_combattype_patrol,ai_movetype_usespeed,ai_attacktype_canfire)
end
