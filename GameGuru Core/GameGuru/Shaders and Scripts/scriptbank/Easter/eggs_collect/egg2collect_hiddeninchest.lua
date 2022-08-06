-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays eggs left to collect

egg_status = {}

function egg2collect_hiddeninchest_init(e)
 egg_status[e] = 0
end

function egg2collect_hiddeninchest_main(e)
 if egg_status[e] == 0 then
  eggradar_addObjective(e)
  egg_status[e] = 1
 end
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 and chest_opened == 1 then
  PlaySound(e,0)
  if g_Egg2Count < 20 then 
   g_Egg2Count=g_Egg2Count+1
  end
  Destroy(e)
 end  
end
