-- AI : Soldier Behavior
ai_wanderer_combatshoot = require "scriptbank\\ai\\module_combatshoot"
function ai_wanderer_init(e)
 ai_wanderer_combatshoot.init(e,ai_combattype_regular)
end
function ai_wanderer_main(e)
 ai_wanderer_combatshoot.main(e,ai_combattype_regular,ai_movetype_usespeed,ai_attacktype_nofire)
end
