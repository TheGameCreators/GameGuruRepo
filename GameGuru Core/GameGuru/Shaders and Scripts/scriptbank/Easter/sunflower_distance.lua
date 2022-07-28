function sunflower_distance_init(e)
end

function sunflower_distance_main(e)

PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 2000
 then
 Show(e)
 else
 Hide(e)
 end
 end

