-- LUA Script - precede every function and global member with lowercase name of script + '_main'
guard_chat_talktoalien = 0

function artefact_init(e)
end
function artefact_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 and g_PlayerHealth > 0 then
   PromptDuration("You collected a strange alien artifact",3000)
   PlaySound(e,0)
   Destroy(e)
   ActivateIfUsed(e)
   guard_chat_talktoalien = 3
 end
end
