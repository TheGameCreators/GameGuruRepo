-- LUA Script - precede every function and global member with lowercase name of script + '_main'
g_teleporter_transittime = 0
g_teleporter_state = {}

function teleporter_init(e)
 -- initialise transporter within level
 g_teleporter_state[e] = 0
end

function teleporter_main(e)
 PlayerDist = GetPlayerDistance(e)
 if g_teleporter_state[e] == 0 and PlayerDist > 50 and PlayerDist < 120 then
  -- transporter aware of players proximity and powers up
  if g_Time > g_teleporter_transittime + 500 then
   g_teleporter_state[e] = 1
  end
 end
 if g_teleporter_state[e] == 1 and PlayerDist < 50 then
  -- transport system active
  g_teleporter_state[e] = 2
  -- find neighboring transporter
  for twopasses = 1, 2 do
   if twopasses == 1 then
    astart = e+1 
	afinish = g_EntityElementMax 
   end
   if twopasses == 2 then
    astart = 1
	afinish = e-1
   end
   for a = astart, afinish do
    if g_Entity[a] ~= nil then
     if g_Entity[a]['health'] > 0 then 
      if g_teleporter_state[a] ~= nil then
       -- transport player here
       PlaySound(e,0)
	   SetFreezePosition(g_Entity[a]['x'],g_Entity[a]['y'],g_Entity[a]['z'])
	   TransportToFreezePositionOnly()
	   g_teleporter_transittime = g_Time
	   twopasses = 3
      end
     end
    end
   end
  end
 end
 if g_teleporter_state[e] >= 1 and PlayerDist >= 120 then
  -- transporter powers down and cools down
  g_teleporter_state[e] = 0
 end
end
