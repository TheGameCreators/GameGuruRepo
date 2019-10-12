-- Resolutions module

local restoregame = {}

function restoregame.now()
 -- trigger entire game to restore to saved game state
 SetFreezePosition(iPlayerPosX,iPlayerPosY,iPlayerPosZ)
 SetFreezeAngle(iPlayerAngX,iPlayerAngY,iPlayerAngZ)
 TransportToFreezePosition()
 SetPlayerHealth(iPlayerHealth)
 SetPlayerLives(iPlayerLives)
 -- restore weapon stats
 ResetWeaponSystems()
 ChangePlayerWeapon(strPlayerGunName)
 for i = 1, 10, 1 do
  SetWeaponSlot ( i, g_WeaponSlotGot[i], g_WeaponSlotPref[i] )
 end 
 for i = 1, 20, 1 do
  SetWeaponAmmo ( i, g_WeaponAmmo[i] )
  SetWeaponClipAmmo ( i, g_WeaponClipAmmo[i] )
 end 
 for i = 0, 100, 1 do
  SetWeaponPoolAmmo ( i, g_WeaponPoolAmmo[i] )
 end 
 -- restore entity stats
 for i = 1, g_EntityElementMax, 1 do
  if g_Entity[i] ~= nil then
   ResetPosition ( i, g_Entity[i]['x'], g_Entity[i]['y'], g_Entity[i]['z'] )
   ResetRotation ( i, g_Entity[i]['anglex'], g_Entity[i]['angley'], g_Entity[i]['anglez'] )
   SetEntityActive ( i, g_Entity[i]['active'] )
   SetEntityActivated ( i, g_Entity[i]['activated'] )
   SetEntityCollected ( i, g_Entity[i]['collected'] )
   SetEntityHasKey ( i, g_Entity[i]['haskey'] )
   SetEntityHealth ( i, g_Entity[i]['health'] )
   RefreshEntity ( i )
   SetAnimationFrame ( i, g_Entity[i]['frame'] )
   if g_EntityExtra[i]['visible']==1 then
    Show ( i )
   else
    Hide ( i )
   end
   SetEntitySpawnAtStart ( i, g_EntityExtra[i]['spawnatstart'] )
   if g_EntityExtra[i]['spawnatstart']==2 and g_Entity[i]['health'] > 0 then
    Spawn ( i )
   end
  end
 end 
end

return restoregame
