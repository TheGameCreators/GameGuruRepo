-- LUA Script - precede every function and global member with lowercase name of script + '_main'

engineer_tool_special = 0

function tool_special_init(e)
end

function tool_special_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 and g_PlayerHealth > 0 then
   PromptDuration("You collected a rather special looking tool",3000)
   engineer_tool_special = 1
   PlaySound(e,0)
   Destroy(e)
   ActivateIfUsed(e)
 end
end
