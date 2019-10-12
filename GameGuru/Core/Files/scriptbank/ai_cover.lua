-- AI : Soldier Guard Behavior
ai_cover_combatshoot = require "scriptbank\\ai\\module_combatshoot"
function ai_cover_init(e)
 ai_cover_combatshoot.init(e,ai_combattype_guard)
end
function ai_cover_main(e)
 ai_cover_combatshoot.main(e,ai_combattype_guard,ai_movetype_usespeed,ai_attacktype_canfire)
end
