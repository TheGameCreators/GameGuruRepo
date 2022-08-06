-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays eggs left to collect

egg_status = {}

function egg3collect_init(e)
 egg_status[e] = 0
end

function egg3collect_main(e)
 if egg_status[e] == 0 then
  if eggradar_addObjective ~= nil then
   eggradar_addObjective(e)
  end
  egg_status[e] = 1
 end
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 then
  PlaySound(e,0)
  if g_Egg3Count ~= nil then
   if g_Egg3Count < 20 then 
    g_Egg3Count=g_Egg3Count+1
   end
  end
  Destroy(e)
 end  
end
