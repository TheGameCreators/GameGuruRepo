function objects_distance_init(e)
end

function objects_distance_main(e)

PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 800
 then
 Show(e)
 else
 Hide(e)
 end
 end

