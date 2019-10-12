-- LUA Script - precede every function and global member with lowercase name of script + '_main'

g_weaponboostcolt1911_usedupgrade = 0

function weaponboostcolt1911_init(e)
end

function weaponboostcolt1911_main(e)
 PlayerDist = GetPlayerDistance(e)
 WeaponID = GetWeaponID("colt1911")
 if PlayerDist < 70 and g_PlayerHealth > 0 then
  -- Deterine style of upgrade prompt
  if g_weaponboostcolt1911_usedupgrade == 0 then
   -- Not used upgrade system yet
   if GetWeaponSlot(1) == WeaponID then
    Prompt("Press E to boost Colt 1911 attributes, including your primary weapon!")
   else
    Prompt("Press E to boost all Colt 1911 attributes in the level")
   end
   -- Action to upgrade weapon
   if g_KeyPressE == 1 then
    -- Modify name and attributes of Colt 1911
    SetWeaponDamage(WeaponID,0,GetWeaponDamage(WeaponID,0)+100)
    SetWeaponAccuracy(WeaponID,0,GetWeaponAccuracy(WeaponID,0)/2)
    SetWeaponReloadQuantity(WeaponID,0,GetWeaponReloadQuantity(WeaponID,0)*3)
    SetWeaponFireIterations(WeaponID,0,GetWeaponFireIterations(WeaponID,0)+5)
    SetWeaponRange(WeaponID,0,GetWeaponRange(WeaponID,0)+1000)
    SetWeaponDropoff(WeaponID,0,GetWeaponDropoff(WeaponID,0)+10)
    SetWeaponSpotLighting(WeaponID,0,1-GetWeaponSpotLighting(WeaponID,0))
    -- Finished upgrading    
	g_weaponboostcolt1911_usedupgrade = 1
    PlaySound(e,0)
    ActivateIfUsed(e)
   end
  else
   -- Used upgrade system - once only
  end
 end
 if PlayerDist < 200 and g_PlayerHealth > 0 then
  if g_weaponboostcolt1911_usedupgrade == 0 and PlayerDist < 70 then
   -- Do not show when get close and not upgraded
  else
   Prompt("Colt 1911 Attributes : " .. GetWeaponDamage(WeaponID,0) .. "-" .. GetWeaponAccuracy(WeaponID,0) .. "-" .. GetWeaponReloadQuantity(WeaponID,0) .. "-" .. GetWeaponFireIterations(WeaponID,0) .. "-" .. GetWeaponRange(WeaponID,0) .. "-" .. GetWeaponDropoff(WeaponID,0) .. "-" .. GetWeaponSpotLighting(WeaponID,0) )
  end
 end
end
