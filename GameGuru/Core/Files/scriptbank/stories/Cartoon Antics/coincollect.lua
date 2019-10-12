-- LUA Script - precede every function and global member with lowercase name of script + '_main'
-- Displays coins left to collect

function coincollect_init(e)
end

function coincollect_main(e)
 
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 80 then
   PlaySound(e,0)
   CoinCount=CoinCount+1
     Destroy(e)
   end
   
end
