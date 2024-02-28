-- LUA Script - precede every function and global member with lowercase name of script + '_main'


function hurt_init(e)
 StartTimer(e)
end

function hurt_main(e)
 PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 70 then
  if g_PlayerPosY < g_Entity[e]['y']+38.5 then
   if GetTimer(e) > 100 then
    HurtPlayer(-1,1)
	StartTimer(e)
   end
  end
 end
end
