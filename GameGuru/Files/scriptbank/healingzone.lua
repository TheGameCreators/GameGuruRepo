-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects Health

function healingzone_init(e)
end

function healingzone_main(e)
 if g_Entity[e]['plrinzone']==1 and g_PlayerHealth > 0 and g_PlayerHealth < g_gameloop_StartHealth then
   SetPlayerHealth(g_PlayerHealth+1)
   ActivateIfUsed(e)
 end
end