-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Player Collects Ammo

function ammo_init(e)
end

function ammo_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 60 and g_PlayerHealth > 0 and g_PlayerThirdPerson==0 then
   PromptLocalForVR(e,"Collected ammo")
   PlaySound(e,0)
   AddPlayerAmmo(e)
   Destroy(e)
   ActivateIfUsed(e)
 end
end
