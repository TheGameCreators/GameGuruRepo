-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects Health

function health_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 then
	if g_PlayerHealth > 0 and g_PlayerHealth < g_gameloop_StartHealth then
	Prompt("Collected "..item_name)
	PlaySound(e,0)
	AddPlayerHealth(e)
	Destroy(e)
	ActivateIfUsed(e)
	else
	Prompt("Your health is full.")
	end
 end
end
