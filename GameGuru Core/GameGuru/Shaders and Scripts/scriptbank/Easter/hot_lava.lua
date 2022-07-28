function hot_lava_init(e)
end

function hot_lava_main(e)

PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 125
 then
 HurtPlayer(e,1)
 LoopSound(e,0)
 else
 StopSound(e)
 end
 end

