-- AI : Soldier Behavior
ai_soldier_combatshoot = require "scriptbank\\ai\\module_combatshoot"
function ai_soldier_init(e)
	ai_soldier_combatshoot.init(e,ai_combattype_regular)
end
function ai_soldier_main(e)
	ai_soldier_combatshoot.main(e,ai_combattype_regular,ai_movetype_usespeed,ai_attacktype_canfire)
end
function ai_soldier_exit(e)
	module_combatcore.releasecover(e)
end
