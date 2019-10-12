-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays gems left to collect

function gemcollect_init(e)
end

function gemcollect_main(e)
 
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 then
   PlaySound(e,0)
   GemCount=GemCount+1
     Destroy(e)
   end
   
end
