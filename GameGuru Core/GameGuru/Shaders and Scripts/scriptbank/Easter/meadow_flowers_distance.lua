function meadow_flowers_distance(e)
end

function meadow_flowers_distance_main(e)

PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 1000
 then
 Show(e)
 else
 Hide(e)
 end
 end

