ai_patrolsecurity_combatshoot = require "scriptbank\\ai\\module_combatshoot"
function ai_patrolsecurity_init(e)
 ai_patrolsecurity_combatshoot.init(e,ai_combattype_patrol)
 LoopSound(e,1)
end
function ai_patrolsecurity_main(e)
 ai_patrolsecurity_combatshoot.main2(e,ai_combattype_patrol,ai_movetype_usespeed,ai_attacktype_canfire,ai_duckstrafe_off,0,0)
end
