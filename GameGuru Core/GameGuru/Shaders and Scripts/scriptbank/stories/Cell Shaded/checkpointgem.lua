-- LUA Script - precede every function and global member with lowercase name of script + '_main'

function checkpointgem_init(e)
end

function checkpointgem_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 300 and g_PlayerHealth > 0 then
  Spawn(e)
 end
end
