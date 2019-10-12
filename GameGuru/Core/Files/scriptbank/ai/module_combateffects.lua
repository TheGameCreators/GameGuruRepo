-- Combat Effects Manager Module

g_module_combateffects_areaoffreeze = 0
g_module_combateffects_areaoffreezex = 0
g_module_combateffects_areaoffreezey = 0
g_module_combateffects_areaoffreezez = 0
g_module_combateffects_areaoffreezeradius = 0
g_module_combateffects_areaoffreezeperiod = 0
g_module_combateffects_areaoffreezedamagetimer = 0
g_module_combateffects_areaoffreezedamage = 0

local module_combateffects = {}

function module_combateffects.ismovementfrozen(e)
 -- determine an area of freeze
 if g_projectileevent_explosion == 1 then
  if g_projectileevent_name == "fantasy\\magicbolt" then
   g_module_combateffects_areaoffreeze = 1
   g_module_combateffects_areaoffreezex = g_projectileevent_x
   g_module_combateffects_areaoffreezey = g_projectileevent_y
   g_module_combateffects_areaoffreezez = g_projectileevent_z
   g_module_combateffects_areaoffreezeradius = g_projectileevent_radius
   g_module_combateffects_areaoffreezeperiod = Timer()+(3000)
   g_module_combateffects_areaoffreezedamagetimer = Timer() + 100
   g_module_combateffects_areaoffreezedamage = 1
  end
  g_projectileevent_explosion = 0
 end
 if g_module_combateffects_areaoffreeze == 1 then
  if Timer() > g_module_combateffects_areaoffreezeperiod then
   g_module_combateffects_areaoffreeze = 0
  end
 end
 -- apply damage to entity within areaoffreeze
 if g_module_combateffects_areaoffreeze == 1 and g_module_combateffects_areaoffreezedamagetimer > 0 then
  if Timer() > g_module_combateffects_areaoffreezedamagetimer then
   g_module_combateffects_areaoffreezedamagetimer = Timer() + 100
   newentityhealth = g_Entity[e]['health'] - g_module_combateffects_areaoffreezedamage
   SetEntityHealth ( e, newentityhealth )
   ai_bot_oldhealth[e] = newentityhealth
  end
 end 
 -- is this entity frozen (from magic spell)
 movementfrozen = 0
 if g_module_combateffects_areaoffreeze == 1 then
  tDX = g_Entity[e]['x'] - g_module_combateffects_areaoffreezex
  tDY = g_Entity[e]['y'] - g_module_combateffects_areaoffreezey
  tDZ = g_Entity[e]['z'] - g_module_combateffects_areaoffreezez
  tDist = math.sqrt(math.abs(tDX*tDX)+math.abs(tDY*tDY)+math.abs(tDZ*tDZ));
  if tDist < g_module_combateffects_areaoffreezeradius then
   movementfrozen = 1
  end
 end
 return movementfrozen
end

return module_combateffects
