-- Agro Manager Module

local module_agro = {}

function module_agro.alertallwithinradius(posx, posy, posz, radius)
 for ee = 1, 1000 do
  if ai_bot_state[ee] ~= nil then
   if g_Entity[ee] ~= nil then
    if g_Entity[ee]['health'] > 0 then
     tDX = g_Entity[ee]['x'] - posx
     tDY = g_Entity[ee]['y'] - posy
     tDZ = g_Entity[ee]['z'] - posz
     tDist = math.sqrt(math.abs(tDX*tDX)+math.abs(tDY*tDY)+math.abs(tDZ*tDZ));
     if tDist < radius then
 	  SetActivated(ee,2)
      ai_bot_angryhurt[ee] = 1
	 end
    end 
   end
  end
 end 
end

return module_agro
