-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function collectable16_init(e)
end

function collectable16_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 and g_PlayerHealth > 0 then
   PlaySound(e,0)
   gCollectedGem015 = 1
   Destroy(e)
   ActivateIfUsed(e)
 end
end
