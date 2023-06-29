-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects Health Booster (this will increase the player's health, even if it's full)

function healthboost_init(e)
local item_name
end

function healthboost_init_name(e,name)
item_name = name
end

function healthboost_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 and g_PlayerHealth > 0 then
	Prompt("Collected "..item_name)
	PlaySound(e,0)
	SetPlayerHealth(g_PlayerHealth+50)
	--AddPlayerHealth(e)
	Destroy(e)
	ActivateIfUsed(e)
 end
end
