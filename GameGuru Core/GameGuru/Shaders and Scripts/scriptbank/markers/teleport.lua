--
function teleport_init(e)
end
function teleport_main(e)
	PlayerDist = GetPlayerDistance(e)
	if PlayerDist <= 150 then
		PlaySound(e,0)
		TransportToIfUsed(e)
	end
end
		