function objects_distance_2_init(e)
end

function objects_distance_2_main(e)

PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 2000
 then
 Show(e)
 else
 Hide(e)
 end
 end

