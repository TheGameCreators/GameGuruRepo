-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function tool_init(e)
end

function tool_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 and g_PlayerHealth > 0 then
   PromptDuration("You collected a tool",3000)
   PlaySound(e,0)
   Destroy(e)
   ActivateIfUsed(e)
 end
end
