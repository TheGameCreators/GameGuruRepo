-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects Ammo
local plr_highlight_ammo_range = 500

function ammo_highlight_init(e)
end

function ammo_highlight_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < plr_highlight_ammo_range then
	SetEntityHighlight(e,1) --1(red),2(pink),3(green),4(blue/green),5(gold)
 end
 if PlayerDist < 60 and g_PlayerHealth > 0 then
   Prompt("Collected ammo")
   PlaySound(e,0)
   AddPlayerAmmo(e)
   Destroy(e)
   ActivateIfUsed(e)
 end
end
