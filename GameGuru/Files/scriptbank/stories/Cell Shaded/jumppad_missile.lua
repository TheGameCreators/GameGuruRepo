

function jumppad_missile_init(e)
weapon_name[e] = "jumppad missile"
CollisionOff(e)
end


function jumppad_missile_main(e)
PlayerDist = GetPlayerDistance(e)
 if PlayerDist < 100 then
 Hide(e)
 end
end