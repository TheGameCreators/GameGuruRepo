-- LUA Script - precede every function and global member with lowercase name of script + '_main'

myPlayerShield = 0
function increaseshield_init(e)
end
function increaseshield_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 60 and g_PlayerHealth > 0 then
   PromptDuration("Your shield increased in power",3000)
   PlaySound(e,0)
   myPlayerShield = myPlayerShield + 50
   Destroy(e)
   ActivateIfUsed(e)
 end
end
