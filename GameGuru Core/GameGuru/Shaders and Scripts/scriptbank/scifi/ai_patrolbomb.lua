-- AI : Biter Behavior
ai_patrolbomb_combatshoot = require "scriptbank\\ai\\module_combatshoot"
function ai_patrolbomb_init(e)
 ai_patrolbomb_combatshoot.init(e,ai_combattype_patrol)
 LoopSound(e,1)
end
function ai_patrolbomb_main(e)
 ai_patrolbomb_combatshoot.main2(e,ai_combattype_patrol,ai_movetype_usespeed,ai_attacktype_nofire,ai_duckstrafe_off,0,0)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 75 then
  SetEntityHealth(e,0)
 end
end
