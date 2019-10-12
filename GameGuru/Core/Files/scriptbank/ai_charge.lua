-- AI : Soldier Behavior
ai_charge_combatshoot = require "scriptbank\\ai\\module_combatshoot"
function ai_charge_init(e)
 ai_charge_combatshoot.init(e,ai_combattype_regular)
end
function ai_charge_main(e)
 ai_charge_combatshoot.main(e,ai_combattype_regular,ai_movetype_usespeed)
end
