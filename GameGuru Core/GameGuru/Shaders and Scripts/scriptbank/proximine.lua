-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Detonate proximity mine

proximine_started = {}

function proximine_init(e)
 proximine_started[e]=0
end

function proximine_main(e)
 PlayerDist = GetPlayerDistance(e)
 if g_Entity[e]['health'] > 0 then
  if proximine_started[e] == 0 then
   -- characters or player can trigger proximity mine
   for a = 1, 1000 do
    if ai_soldier_state[a] ~= nil then
     if g_Entity[a] ~= nil then
      if g_Entity[a]['health'] > 0 then
       MineDX = g_Entity[a]['x'] - g_Entity[e]['x'];
       MineDY = g_Entity[a]['y'] - g_Entity[e]['y'];
       MineDZ = g_Entity[a]['z'] - g_Entity[e]['z'];
       MineDist = math.sqrt(math.abs(MineDX*MineDX)+math.abs(MineDY*MineDY)+math.abs(MineDZ*MineDZ));
       if MineDist < 90 then
        proximine_started[e] = GetTimer(e)+2500
        PlaySound(e,0)
       end 
	  end
	 end
    end
   end
   if PlayerDist<90 then 
    proximine_started[e] = GetTimer(e)+2500
    PlaySound(e,0)
   end
  end
 end
 if proximine_started[e]>0 then
  --if g_Entity[e] ~= nil then
  -- Prompt ( "timer=" .. GetTimer(e) .. " started=" .. proximine_started[e] .. " health = " .. g_Entity[e]['health'] )
  --end
  if GetTimer(e) > proximine_started[e] then
   SetEntityHealth(e,0)
   proximine_started[e]=0
  end
 end
 if g_Entity[e]['health'] <= 0 then
  StopSound(e)
 end
end
