-- highlights entity when player in range

local plr_range = 300

function highlight_entity_gold_init(e)
end

function highlight_entity_gold_main(e)
 
	PlayerDist = GetPlayerDistance(e)
	if PlayerDist < plr_range and g_PlayerHealth > 0 then
		SetEntityHighlight(e,5) --1(red),2(pink),3(green),4(blue/green),5(gold)
	else
		if PlayerDist >= plr_range and PlayerDist < plr_range + 50 then
			SetEntityHighlight(e,0) --off
		end
	end

end
