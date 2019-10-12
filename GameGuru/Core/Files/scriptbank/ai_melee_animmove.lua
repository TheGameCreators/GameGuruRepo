-- AI : Melee Guard Behavior
module_combatmelee = require "scriptbank\\ai\\module_combatmelee"
function ai_melee_animmove_init(e)
 module_combatmelee.init(e,ai_combattype_freezermelee,ai_movetype_useanim)
 SetPreExitValue(e,0)
end
function ai_melee_animmove_main(e)
 module_combatmelee.main(e,ai_combattype_freezermelee,ai_movetype_useanim)
end
function ai_melee_animmove_preexit(e)
 SetPreExitValue(e,1)
 if module_combatmelee.preexit(e,ai_movetype_useanim) == 1 then
  SetPreExitValue(e,2)
 end
end
function ai_melee_animmove_exit(e)
 module_combatmelee.exit(e)
end
